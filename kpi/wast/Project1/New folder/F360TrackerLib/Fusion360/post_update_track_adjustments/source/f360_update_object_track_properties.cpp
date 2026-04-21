/*===================================================================================*\
* FILE:  f360_update_object_track_properties.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Update_Object_Track_Properties() function
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_update_object_track_properties.h"
#include "f360_reuse.h"
#include "f360_math_func.h"
#include "f360_get_wall_time.h"
#include "f360_calc_obj_size_accuracy.h"
#include "f360_mark_det_to_use_for_dim_update.h"
#include "f360_mark_detections_with_neighbors.h"
#include "f360_calc_obj_size.h"
#include "f360_calc_obj_height.h"
#include "f360_update_object_average_rcs.h"
#include "f360_calc_heading_from_pos_diff.h"
#include "f360_update_object_reference_point.h"
#include "f360_sorted_tracks_mgmt.h"



namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_Object_Track_Properties()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib
   * const F360_Globals_T & globals
   * const F360_Tracker_Info_T & tracker_info
   * const rspp_variant_A::RSPP_Detection_List_T & raw_detect_list
   * const F360_Host_T & host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
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
   * This function updates object track properties such as objects dimensions, 
   * pointing and heading angle, total number of reduced detections, reference
   * point, linear moving, object height, and status and average RCS value.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Object_Track_Properties(
      const F360_Calibrations_T & calib,
      const F360_Globals_T & globals,
      const rspp_variant_A::RSPP_Detection_List_T & raw_detect_list,
      const F360_Host_T & host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_TRKR_TIMING_INFO_T & timing_info)
   {      
      const float32_t start_time = get_wall_time();

      for (int32_t num_obj_active = 0; num_obj_active < tracker_info.num_active_objs; num_obj_active++)
      {
         const int32_t obj_trk_idx = tracker_info.active_obj_ids[num_obj_active] - 1;
         F360_Object_Track_T& obj = object_tracks[obj_trk_idx];
   
         Calc_Heading_From_Pos_Diff(obj, calib);

         // Update total number of reduced dets
         obj.total_reduced_dets = obj.total_reduced_dets + obj.num_rr_inlier_dets;

         // Check which associated detections should be used in track object to update track dimensions
         Mark_Det_To_Use_For_Dim_Update(obj, calib, raw_detect_list, det_props);

         Update_Object_Average_Rcs(raw_detect_list.detections, calib, obj);

         // If there are at least 2 associated detections, update size.
         if ((obj.status == F360_OBJECT_STATUS_UPDATED) && (obj.ndets >= 2U))
         {
            float32_t measured_length;
            float32_t measured_width;

            Calc_Obj_Size(det_props, raw_detect_list, calib, measured_length, measured_width, obj);
            Calc_Obj_Size_Accuracy(measured_length, measured_width, calib, obj);
         }

         // Calculate current object height
         Calc_Obj_Height(raw_detect_list.detections, calib, obj);

         Update_Object_Reference_Point(host.dist_rear_axle_to_vcs_m, true, false, calib, sensors, globals, obj);
      }

      Sorted_Tracks_Re_Sort(tracker_info);

      timing_info.obj_trk_properties = get_wall_time() - start_time;
   }
}
