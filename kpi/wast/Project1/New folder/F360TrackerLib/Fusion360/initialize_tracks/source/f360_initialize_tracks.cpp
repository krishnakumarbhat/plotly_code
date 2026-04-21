/*===================================================================================*\
* FILE: f360_initialize_tracks.cpp
*====================================================================================
* Copyright 2020 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
*------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains F360_Tracker declaration of Initialize_Tracks() function
*   for tracked objects state initialization.
*
* ABBREVIATIONS:
*   NONE
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]*
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/


/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_initialize_tracks.h"
#include "f360_sorted_tracks_mgmt.h"
#include "f360_get_wall_time.h"
#include "f360_sort_clusters_by_priority.h"
#include "f360_is_cluster_vs_object_prioritization_succeed.h"
#include "f360_fill_init_obj_track_props.h"
#include "f360_allocate_id_for_initialized_object.h"
#include "f360_sort_priority.h"
#include "f360_associate_detection_to_object.h"
#include "f360_is_cluster_visible.h"
#include "f360_nees_cfmi_try_to_init.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"

namespace f360_variant_A
{

   static void Post_Init_Obj_Track(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T& p_current_cluster,
      F360_Object_Track_T& object_track_to_init);

   static void Move_Cluster_Detections_To_Object(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Cluster_T& initialized_cluster,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& object_track_to_init);

   /*===========================================================================*\
   * FUNCTION: Initialize_Tracks()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T& host,
   * const F360_Host_Props_T& host_props,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
   * const F360_Detection_Hist_T& det_hist,
   * const F360_Globals_T& globals,
   * const F360_Calibrations_T& calibrations,
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
   * const Occlusion_T& occlusion,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
   * F360_Tracker_Info_T& tracker_info,
   * F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_TRKR_TIMING_INFO_T& tracker_time_info
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
   * This function initialize objects based on gathered cluster data.
   * In next steps new objects parameters are filled and adequate structure of tracker info are updated.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Initialize_Tracks(
      const F360_Host_T& host,
      const F360_Host_Props_T& host_props,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      const F360_Detection_Hist_T& det_hist,
      const F360_Globals_T& globals,
      const F360_Calibrations_T& calibrations,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const Occlusion_T& occlusion,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_TRKR_TIMING_INFO_T& tracker_time_info)
   {
      const float32_t start_time = get_wall_time();

      uint32_t num_predond_valid_clusters = 0U;
      int32_t sorted_clusters_id_by_prior[NUMBER_OF_CLUSTERS];

      Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detections, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

      //initialize stationary object velocity from host
      F360_NEES_CFMI_Pos_Diff_Velocity_T stationary_velocity = {};
      Prepare_Stationary_Info(host_props, stationary_velocity);


      for (uint32_t unsorted_cluster_idx = 0U; unsorted_cluster_idx < num_predond_valid_clusters; unsorted_cluster_idx++)
      {
         const int32_t cluster_idx = sorted_clusters_id_by_prior[unsorted_cluster_idx] - 1;
         F360_Cluster_T& current_cluster = clusters[cluster_idx];

         if (!Is_Cluster_Vs_Object_Prioritization_Succeed(tracker_info, current_cluster))
         {
            break;
         }

         F360_Tracked_Object_Init_Info_T init_info = {};
         Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detections, det_hist, stationary_velocity, init_info);

         if (init_info.f_success && (init_info.f_valid_for_liberal_tracking || Is_Cluster_Visible(occlusion, raw_detections, current_cluster)))
         {
            const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, object_tracks, det_props);

            F360_Object_Track_T& object_track_to_init = object_tracks[new_id - 1];

            Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, current_cluster, sep, tracker_info.num_unique_objs, sensors, det_props, object_track_to_init);

            Post_Init_Obj_Track(raw_detections, tracker_info, det_props,
               current_cluster, object_track_to_init);
         }
      }
      tracker_time_info.initialize_tracks = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Post_Init_Obj_Track
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * F360_Tracker_Info_T& tracker_info,
   * F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T& p_current_cluster,
   * F360_Object_Track_T& object_track_to_init,
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function for updating some track's information after initialization
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   static void Post_Init_Obj_Track(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T& p_current_cluster,
      F360_Object_Track_T& object_track_to_init)
   {
      Move_Cluster_Detections_To_Object(tracker_info, p_current_cluster, raw_detect_list, det_props, object_track_to_init);

      p_current_cluster.f_to_be_killed = true;

      Sort_Priority_With_New_Track(tracker_info, &object_track_to_init);
      Sorted_Tracks_Insert(tracker_info, &object_track_to_init);
   }

   /*===========================================================================*\
    * FUNCTION: Move_Cluster_Detections_To_Object
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const F360_Tracker_Info_T& tracker_info,
    * const F360_Cluster_T& initialized_cluster,
    * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
    * F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
    * F360_Object_Track_T& object_track_to_init
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
    * Moving association information from cluster to object
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   static void Move_Cluster_Detections_To_Object(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Cluster_T& initialized_cluster,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& object_track_to_init)
   {
      for (int32_t slot_index = 0; slot_index < initialized_cluster.ndets; slot_index++)
      {
         const int32_t det_id = initialized_cluster.detids[slot_index];
         const int32_t det_idx = det_id - 1;
         F360_Detection_Props_T& single_det = det_props[det_idx];
         const bool f_detection_associated_to_object = Associate_Detection_To_Object(tracker_info, raw_detect_list.detections[det_idx], object_track_to_init, single_det, static_cast<uint32_t>(det_id));
         single_det.f_rr_inlier = f_detection_associated_to_object;
      }
      object_track_to_init.num_rr_inlier_dets = object_track_to_init.ndets;
   }

   void Prepare_Stationary_Info(const F360_Host_Props_T& host_props,
      F360_NEES_CFMI_Pos_Diff_Velocity_T &stationary_velocity)
   {
      stationary_velocity.f_valid = true;
      stationary_velocity.f_inlier = true;
      stationary_velocity.vel.longitudinal = 0.0F;
      stationary_velocity.vel.lateral = 0.0F;

      stationary_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] = host_props.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      stationary_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = host_props.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      stationary_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = host_props.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      stationary_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = host_props.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];

      stationary_velocity.cov_trace = Trace_Of_2d_Covariance(stationary_velocity.vel_cov);
      stationary_velocity.determinant = F360_2d_Matrix_Determinant(host_props.vel_cov);

      Calc_NEES_CFMI_Determinants_Pos_Diff(stationary_velocity);
   }

}
