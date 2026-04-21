/*===================================================================================*\
* FILE:  f360_calculate_priority.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function shared between object and cluster for calculating priority value
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_calculate_priority.h"
#include "f360_calculate_curvi_position.h"
#include "f360_math_func.h"
#include <algorithm>

namespace f360_variant_A
{
   static float32_t Calculate_Headway_Priority(
      const F360_Host_T & host_props,
      const F360_Calibrations_T & calibs,
      const float32_t longitudal_pos,
      const float32_t lateral_pos);

   /*===========================================================================*\
   * FUNCTION: Calculate_Priority()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const F360_Calibrations_T* const calibs,
   * const F360_Host_T* const host_props,
   * const bool f_movable,
   * const float32_t confidence,
   * const float32_t longitudal_pos,
   * const float32_t lateral_pos
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates basic priority value for the cluster or object with
   * given number of detection, position and velocity
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   * confidence is in range <0, 1>
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calculate_Priority(
      const F360_Calibrations_T & calibs,
      const F360_Host_T & host_props,
      const bool f_movable,
      const float32_t confidence,
      const float32_t longitudal_pos,
      const float32_t lateral_pos)
   {
      const float32_t sum_of_weights = calibs.k_priority_distance_coefficient
         + calibs.k_priority_confidence_coefficient
         + calibs.k_priority_headway_coefficient
         + calibs.k_priority_f_movable_coefficient;

      float32_t priority;
      if (sum_of_weights > F360_MIN_DENOMINATOR)
      {
         const float32_t headway_priority = Calculate_Headway_Priority(host_props, calibs, longitudal_pos, lateral_pos);
         const float32_t movable_priority = f_movable ? F360_MAX_PRIORITY : F360_MIN_PRIORITY;
         const float32_t manhattan_distance = std::abs(longitudal_pos) + std::abs(lateral_pos);
         const float32_t distance_priority = std::max(F360_MIN_PRIORITY, (F360_MAX_PRIORITY - (manhattan_distance * calibs.k_priority_distance_for_min_priority_inverse)));

         priority = confidence * calibs.k_priority_confidence_coefficient;
         priority += distance_priority * calibs.k_priority_distance_coefficient;
         priority += headway_priority * calibs.k_priority_headway_coefficient;
         priority += movable_priority * calibs.k_priority_f_movable_coefficient;
         priority /= sum_of_weights;
      }
      else
      {
         priority = 0.0F;
      }

      return priority;
   }

    /*===========================================================================*\
    * FUNCTION: Calculate_Headway_Priority()
    *===========================================================================
    * RETURN VALUE:
    * none
    *
    * PARAMETERS:
    * const F360_Host_T* const host_props,
    * const F360_Calibrations_T* const calibs,
    * const float32_t longitudal_pos,
    * const float32_t lateral_pos
    *
    * EXTERNAL REFERENCES:
    * None.
    *
    * DEVIATIONS FROM STANDARDS:
    * None.
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * This function calculates an object's headway priority.
    *
    * PRECONDITIONS:
    * confidence is in range <0, 1>
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   static float32_t Calculate_Headway_Priority(
      const F360_Host_T & host_props,
      const F360_Calibrations_T & calibs,
      const float32_t longitudal_pos,
      const float32_t lateral_pos)
   {
      float32_t headway_priority;
      if ((longitudal_pos > 0.0F) && (host_props.vcs_speed > F360_MIN_DENOMINATOR))
      {
         const float32_t headway = longitudal_pos / host_props.vcs_speed;
         const float32_t curvi_lat_pos = Calculate_Curvi_Lat_Pos(host_props, longitudal_pos, lateral_pos);
         const float32_t headway_lon_priority = std::max(F360_MIN_PRIORITY, (F360_MAX_PRIORITY - (headway * calibs.k_priority_headway_for_min_priority_inverse)));
         const float32_t headway_lat_penalty = std::min(F360_MAX_PRIORITY, ((std::abs(curvi_lat_pos) * calibs.k_priority_lat_penalty_max_dist_inverse)));
         headway_priority = std::max(F360_MIN_PRIORITY, headway_lon_priority - headway_lat_penalty);
      }
      else
      {
         headway_priority = F360_MIN_PRIORITY;
      }
      return headway_priority;
   }
}
