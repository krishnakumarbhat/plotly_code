/*===================================================================================*\
 * FILE:  f360_classification_underdrivability_moving.cpp
 *====================================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains the implementation of Determine_Underdrivability_For_Movable()
 * 
 * Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 **/

#include "f360_classification_underdrivability_moving.h"
#include "f360_get_wall_time.h"
#include <numeric>

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: Assign_Underdrivability_Status_To_Moving_Object()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib
   * F360_Object_Track_T& object
   * F360_TRKR_TIMING_INFO_T& timing_info
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
   * Main function assigning underdrivability status to moving objects.
   * Historical height mean and no. of scan indexes above the height threshold is considered.
   *
   * PRECONDITIONS:
   * None
   * 
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/

   void Assign_Underdrivability_Status_To_Moving_Object(
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & object,
      F360_TRKR_TIMING_INFO_T& timing_info)

   {
      const float32_t start_time = get_wall_time();

      if (object.otg_height > calib.ud_mov_height_threshold)
      {
         object.ud_mov_cnt_underdrivable++;
      }
      else
      {
         object.ud_mov_cnt_underdrivable = 0U;
      }

      if ((calib.ud_mov_posx_min_limit <= object.vcs_position.x) && (object.vcs_position.x <= calib.ud_mov_posx_max_limit))
      {
         if (object.ud_mov_cnt_underdrivable > calib.ud_mov_cnt_consecutive_scans)
         {
            object.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER;
            object.probability_underdrivable = calib.ud_mov_prob_can_pass_under;
         }
         else
         {
            object.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER;
            object.probability_underdrivable = calib.ud_mov_prob_can_not_pass_under;
         }
      }
      else
      {
         object.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
         object.probability_underdrivable = calib.ud_mov_prob_not_to_consider;
      }

      timing_info.determine_underdrivability_for_movable += get_wall_time() - start_time;
    }
}
