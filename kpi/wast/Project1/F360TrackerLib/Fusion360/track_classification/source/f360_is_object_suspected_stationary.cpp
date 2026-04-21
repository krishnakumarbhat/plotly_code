/*===================================================================================*\
* FILE:  f360_check_if_object_is_suspected_stationary.cpp
*====================================================================================

*Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.

* Confidential - Restricted Aptiv information. Do not disclose."

*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains implementation of Is_Object_Suspected_Stationary() function.
*
*

* Applicable Standards (in order of precedence: highest first):

*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]

*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]

***/

#include "f360_is_object_suspected_stationary.h"
#include "f360_is_object_suspected_stationary_helpers.h"
#include "f360_math_func.h"
#include "f360_bounding_box.h"
#include "f360_determine_cross_moving_obj.h"

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: Is_Object_Suspected_Stationary
   *===========================================================================
   * RETURN VALUE:
   * bool f_stationary_suspected - flag indicating whether object is suspected of being stationary.
   *
   * PARAMETERS:
   *  F360_Object_Track_T& object,
   *  const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
   *  const F360_Host_T& host,
   *  const F360_Calibrations_T& calib,
   *  const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   *  const Occlusion_T& occlusion
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
   * Function checks if object is suspected of being stationary.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Object_Suspected_Stationary(
      F360_Object_Track_T& object,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const Occlusion_T& occlusion)
   {
      bool f_stationary_suspected;

      const float32_t moving_speed_threshold = Calc_Moving_Speed_Threshold(raw_detect_list, sensors, calib, object, occlusion);

      if (std::abs(object.speed) <= moving_speed_threshold)
      {
         f_stationary_suspected = true;
      }
      else if (F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type) // CTCA has ONE state reprecenting object speed (object.speed) so Is_Object_Stationary_By_Vel_Sigma() should be used
      {
         const bool f_moving_suspected =
            (!Is_Object_Stationary_By_Num_Dets(object, raw_detect_list, calib.k_object_motion_min_moving_dets_percentage_th)) ||
            (!Is_Object_Stationary_By_Vel_Sigma(object, calib)) ||
            (Is_Object_Parallel_Moving(object, host, calib)) ||
            (object.f_moveable && Is_Object_Cross_Moving(object, calib));

         f_stationary_suspected = !f_moving_suspected;
      }
      else if ((F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type)) // CCA has TWO states reprecenting object speed (object.vcs_velocity.longitudinal and object.vce_velocity.lateral) so Is_Object_Stationary_By_Vel_NEES() should be used
      {
         const bool f_moving_suspected =
            (!Is_Object_Stationary_By_Num_Dets(object, raw_detect_list, calib.k_object_motion_min_moving_dets_percentage_th)) ||
            (!Is_Object_Stationary_By_Vel_NEES(object, calib)) ||
            (Is_Object_Parallel_Moving(object, host, calib)) ||
            (object.f_moveable && Is_Object_Cross_Moving(object, calib));

         f_stationary_suspected = !f_moving_suspected;
      }
      else
      {
         f_stationary_suspected = true;
      }

      return f_stationary_suspected;
   }
}
