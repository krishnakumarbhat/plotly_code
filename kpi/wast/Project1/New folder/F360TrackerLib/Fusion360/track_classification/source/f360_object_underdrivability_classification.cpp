/*===================================================================================*\
 * FILE:  f360_object_underdrivability_classification.cpp
 *====================================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains the implementation of Object_Underdriviability_Classification()
 * 
 * Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 **/

#include "f360_object_underdrivability_classification.h"
#include "f360_assign_underdrivability_status_to_tracks_ocg.h"
#include "f360_classification_underdrivability_moving.h"
#include "f360_get_wall_time.h"

namespace f360_variant_A
{
   static float32_t Init_Object_Underdrivability_Classification_Timing_Info(F360_TRKR_TIMING_INFO_T& timing_info);

   /*===========================================================================*\
   * FUNCTION: Object_Underdrivability_Classification()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Tracker_Info_T& tracker_info
   * const OCG_Outputs_T* const p_occupancy_grid
   * const F360_Host_T& host
   * const rspp_variant_A::RSPP_Detection_List_T& dets_raw
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
   * Main function for determination of underdrivability status for objects
   *
   * PRECONDITIONS:
   * None
   * 
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/

   void Object_Underdrivability_Classification(
      const F360_Tracker_Info_T& tracker_info,
      const ocg::OCG_Outputs_T* const p_occupancy_grid,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      
      const float32_t start_time = Init_Object_Underdrivability_Classification_Timing_Info(timing_info);

      F360_OCG_INTERNAL_T ocg_internal = {};
      const bool f_ocg_is_available = Preprocess_OCG(host, p_occupancy_grid, static_cast<float32_t>(tracker_info.time_us), ocg_internal);
      

      // Do underdrivability classification of objects
      for (int32_t obj_idx = 0; obj_idx < tracker_info.num_active_objs; obj_idx++)
      {
         F360_Object_Track_T& curr_obj = object_tracks[tracker_info.active_obj_ids[obj_idx] - 1];

         if (curr_obj.f_moving)
         {
            // Perform classification of object underdrivability status
            Assign_Underdrivability_Status_To_Moving_Object(calib, curr_obj, timing_info);
         }
         else
         {
            if (f_ocg_is_available)
            {
               // Occupancy grid is available, map ocg underdrivability status to that of object underdrivability status
               Assign_Underdrivability_Status_To_Stationary_Object(calib, *p_occupancy_grid, ocg_internal, host, curr_obj, timing_info);
            }
            else
            {
               // No OCG input is available. Set the default UD status
               curr_obj.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
               curr_obj.probability_underdrivable = 0.0F;
            }
         }
      }
      timing_info.object_underdrivability_classification = start_time - get_wall_time();
   }

   /*===========================================================================*\
   * FUNCTION: Init_Object_Underdrivability_Classification_Timing_Info
   *===========================================================================
   * RETURN VALUE:
   * float32_t start_time
   *
   * PARAMETERS:
   * F360_TRKR_TIMING_INFO_T* const timing_info
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
   * Initialize timing variables and start timers
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static float32_t Init_Object_Underdrivability_Classification_Timing_Info(F360_TRKR_TIMING_INFO_T& timing_info)
   {
      const float32_t start_time = get_wall_time();
      timing_info.assign_underdrivability_status_to_tracks_ocg = 0.0F; // Initialize to zero. This timer will be iteratively updated for each object inside of Assign_Underdrivability_Status_To_Tracks_OCG()
      timing_info.determine_underdrivability_for_movable = 0.0F; // Initialize to zero. This timer will be iteratively updated for each object inside of Determine_Underdrivability_For_Movable()

      return start_time;
   }

}
