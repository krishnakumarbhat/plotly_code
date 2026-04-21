/*===================================================================================*\
* FILE: f360_try_to_merge_two_objects.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definitions of Try_To_Merge_With_Other_Object() function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/
#include "f360_try_to_merge_two_objects.h"

#include "f360_are_two_objects_preconditions_ok_for_merge.h"
#include "f360_update_merged_objects_properties.h"
#include "f360_move_dets_from_killed_to_kept_object.h"
#include "f360_verify_object_size.h"
#include "f360_calculate_merged_object_dimensions.h"
#include "f360_kill_obj_trk.h"
#include "f360_dimensions.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Try_To_Merge_Two_Objects()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T & host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list
   * const F360_Calibrations_T & calib
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const int32_t idx1
   * const int32_t idx2
   * const F360_Globals_T& globals
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Tracker_Info_T & tracker_info
   * int32_t & kill_idx
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
   * Functions check whether two object could be merged and merges them if it 
   * is possible. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Try_To_Merge_Two_Objects(
      const F360_Host_T & host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list,
      const F360_Calibrations_T & calib,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const int32_t idx1,
      const int32_t idx2,
      const F360_Globals_T& globals,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Tracker_Info_T & tracker_info,
      int32_t & kill_idx)
   {
      if (Are_Two_Objects_Preconditions_OK_For_Merge(calib, object_tracks[idx1], object_tracks[idx2], detection_props))
      {
         int32_t keep_idx;
         Choose_Obj_Idx_To_Be_Kept(idx1, idx2, object_tracks[idx1], object_tracks[idx2], keep_idx, kill_idx);
         F360_Dimensions_T dimensions = Calculate_Merged_Object_Dimensions(object_tracks[keep_idx], object_tracks[kill_idx]);

         if (Verify_Object_Size(object_tracks[keep_idx], object_tracks[kill_idx], calib, dimensions))
         {
            Update_Merged_Objects_Properties(object_tracks[kill_idx], calib, sep, host, sensors, globals, dimensions, object_tracks[keep_idx]);

            Move_Dets_From_Killed_To_Kept_Object(tracker_info, sensors, raw_detection_list, calib, object_tracks[kill_idx], object_tracks[keep_idx], detection_props);

            // Detections associated to killed object should not be cleared since they have been moved to the kept object.
            Kill_Obj_Trk(kill_idx + 1, object_tracks, tracker_info);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Choose_Obj_Idx_To_Be_Kept()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const int32_t idx1
   * const int32_t idx2
   * const F360_Object_Track_T& obj1,
   * const F360_Object_Track_T& obj2,
   * uint32_t & keep_idx
   * uint32_t & kill_idx
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
   * Function decides, which object ID shall be assigned to newly merged object.
   * For objects that are old enough, 
   *     - the CTCA object is selected if the other is CCA.
   *     - If both have the same filter type, the closest object is selected.
   * If either of the objects is not old enough,
   *     - the oldest is selected.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Choose_Obj_Idx_To_Be_Kept(
      const int32_t idx1,
      const int32_t idx2,
      const F360_Object_Track_T& obj1,
      const F360_Object_Track_T& obj2,
      int32_t & keep_idx,
      int32_t & kill_idx)
   {
      constexpr float32_t min_obj_age_for_keep_obj_selection = 2.0F;
      const bool f_both_objects_old_enough = (obj1.time_since_cluster_created > min_obj_age_for_keep_obj_selection) && (obj2.time_since_cluster_created > min_obj_age_for_keep_obj_selection);
      if (f_both_objects_old_enough)
      {
         constexpr float32_t k_slow_moving_merge_speed_threshold = 3.0F;
         const bool f_both_objects_slow_moving = (obj1.speed < k_slow_moving_merge_speed_threshold) && (obj2.speed < k_slow_moving_merge_speed_threshold);
         // If the objects have different filter types and are slow moving, the CTCA object is kept
         if (f_both_objects_slow_moving && (F360_TRACKER_TRKFLTR_CTCA == obj1.trk_fltr_type) && (F360_TRACKER_TRKFLTR_CCA == obj2.trk_fltr_type))
         {
            keep_idx = idx1;
            kill_idx = idx2;
         }
         else if (f_both_objects_slow_moving && (F360_TRACKER_TRKFLTR_CTCA == obj2.trk_fltr_type) && (F360_TRACKER_TRKFLTR_CCA == obj1.trk_fltr_type))
         {
            keep_idx = idx2;
            kill_idx = idx1;
         }
         else
         {
            // If both objects are old enough and of the same filter type, keep object with closest reference point
            const float32_t obj1_dist_sq = obj1.vcs_position.x * obj1.vcs_position.x + obj1.vcs_position.y * obj1.vcs_position.y;
            const float32_t obj2_dist_sq = obj2.vcs_position.x * obj2.vcs_position.x + obj2.vcs_position.y * obj2.vcs_position.y;

            if (obj2_dist_sq < obj1_dist_sq)
            {
               keep_idx = idx2;
               kill_idx = idx1;
            }
            else
            {
               keep_idx = idx1;
               kill_idx = idx2;
            }
         }
      }
      else
      {
         // If any of the objects are newly created, keep the oldest
         if (obj2.time_since_cluster_created > obj1.time_since_cluster_created)
         {
            keep_idx = idx2;
            kill_idx = idx1;
         }
         else
         {
            keep_idx = idx1;
            kill_idx = idx2;
         }
      }

   }
}
