/*===================================================================================*\
* FILE: f360_track_grouping.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Track_Grouping function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "f360_track_grouping.h"

#include "f360_try_to_merge_two_objects.h"
#include "f360_get_wall_time.h"
#include "f360_compute_split_logic_signals.h"
#include "f360_split_objects_in_orth_direction.h"

namespace f360_variant_A
{
   static bool Is_Long_Dist_Between_Objects_Within_Thr(
      const float32_t first_obj_vcs_long_posn,
      const float32_t second_obj_vcs_long_posn,
      const float32_t max_distance);

   /*===========================================================================*\
   * FUNCTION: Track_Grouping()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const F360_Host_T& host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list
   * const F360_Globals_T& globals
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Tracker_Info_T & tracker_info
   * F360_TRKR_TIMING_INFO_T & timing_info
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
   * Functions iterate after longitudinally sorted objects and checks whether 
   * any two of them could be merged and merge them if it is needed.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Track_Grouping(
      const F360_Calibrations_T & calib,
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Host_T& host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list,
      const F360_Globals_T& globals,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Tracker_Info_T& tracker_info,
      F360_TRKR_TIMING_INFO_T & timing_info)
   {
      const float32_t start_time = get_wall_time();
      const F360_Object_Track_T* obj1 = tracker_info.vcslong_sorted_start;
      for (int32_t i = 0; i < (tracker_info.num_active_objs - 1); i++)
      {        
         if (NULL == obj1)
         {
            break;
         }
         const int32_t idx1 = obj1->id - 1;
         if (obj1->f_moving)
         {
            Try_To_Merge_With_Other_Object(sensors, raw_detection_list, calib, static_env_polys, idx1, host, globals, object_tracks, detection_props, tracker_info);
         }
         obj1 = tracker_info.vcslong_sorted_next_track[idx1];
      }

      Compute_Split_Logic_Signals(detection_props, calib, tracker_info, host.dist_rear_axle_to_vcs_m, object_tracks);

      Split_Objects_In_Orth_Direction(
         host,
         calib,
         static_env_polys,
         raw_detection_list,
         sensors,
         globals,
         object_tracks,
         tracker_info,
         detection_props);

      timing_info.track_grouping = get_wall_time() - start_time;

   }

   /*===========================================================================*\
   * FUNCTION: Try_To_Merge_With_Other_Object()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list
   * const F360_Calibrations_T & calib
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
   * const int32_t idx1
   * const F360_Host_T & host
   * const F360_Globals_T& globals
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Tracker_Info_T & tracker_info
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
   * Function takes first object and compare it with next objects on 
   * longitudinally sorted targets list and verify if any object could be merged 
   * with the first object. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Try_To_Merge_With_Other_Object(
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list,
      const F360_Calibrations_T & calib,
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const int32_t idx1,
      const F360_Host_T & host,
      const F360_Globals_T& globals,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Tracker_Info_T& tracker_info)
   {
      int32_t idx2 = idx1;
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         bool f_take_next_object;
         const F360_Object_Track_T* const obj2 = tracker_info.vcslong_sorted_next_track[idx2];
         if (NULL != obj2)
         {
            idx2 = obj2->id - 1;

            if(Is_Long_Dist_Between_Objects_Within_Thr(object_tracks[idx1].bbox.Get_Center().x, object_tracks[idx2].bbox.Get_Center().x, 2.0F * object_tracks[idx1].bbox.Get_Length()))
            {
               int32_t kill_idx = F360_INVALID_ID;
               if (obj2->f_moving)
               {
                   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, static_env_polys, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);
               }
               f_take_next_object = (idx1 != kill_idx);
            }
            else
            {
               f_take_next_object = false;
            }
         }
         else
         {
            f_take_next_object = false;
         }
         if (!f_take_next_object)
         {
            break;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Long_Dist_Between_Objects_Within_Thr()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const float32_t first_obj_vcs_long_posn
   * const float32_t second_obj_vcs_long_posn
   * const float32_t max_distanc
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
   * Functions check whether two objects meet longitudinal distance thresholds from
   * each other. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static bool Is_Long_Dist_Between_Objects_Within_Thr(
      const float32_t first_obj_vcs_long_posn,
      const float32_t second_obj_vcs_long_posn,
      const float32_t max_distance)
   {
      return ((second_obj_vcs_long_posn - first_obj_vcs_long_posn) <= max_distance);
   }
}
