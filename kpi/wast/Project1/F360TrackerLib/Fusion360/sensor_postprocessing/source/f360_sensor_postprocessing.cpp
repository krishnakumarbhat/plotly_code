/*===================================================================================*\
* FILE: f360_sensor_postprocessing.cpp
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contain definitions for senor post-processing main function and some support functions
*
* ABBREVIATIONS:
*   OTG	Over-The-ground
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
*
* DEVIATIONS FROM STANDARDS:
*
*
\*==========================================================================================*/


/******************************
* Includes
*******************************/

#include <algorithm>
#include "f360_reuse.h"
#include "f360_iterator.h"
#include "f360_constants.h"
#include "f360_sensor_postprocessing.h"
#include "f360_kill_cluster.h"
#include "f360_get_wall_time.h"


namespace f360_variant_A
{
   static void Update_Detection_History(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Detection_Hist_T& detection_hist,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      const F360_Tracker_Info_T& tracker_info,
      F360_TRKR_TIMING_INFO_T & timing_info
   );

   static bool Find_Next_Hist_Det_Idx(
      uint16_t& hist_det_idx,
      const int16_t cluster_idx,
      const F360_Tracker_Info_T& tracker_info,
      F360_Detection_Hist_T& detection_hist,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]
   );


   /*===========================================================================*\
   * FUNCTION: Sensor_Postprocessing()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Tracker_Info_T* const tracker_info,
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_Detection_Hist_T & detection_hist,
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
   * F360_TRKR_TIMING_INFO_T &timing_info
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
   * This function is the main function for performing sensor post-processing
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Sensor_Postprocessing(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Detection_Hist_T& detection_hist,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info)
   {
      const float32_t start_time = get_wall_time();
      Update_Detection_History(det_props, raw_detection_list, sensors, detection_hist, clusters, tracker_info, timing_info);
      timing_info.sensor_postprocessing = get_wall_time() - start_time;
   }


   static void Update_Detection_History(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Detection_Hist_T& detection_hist,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      const F360_Tracker_Info_T& tracker_info,
      F360_TRKR_TIMING_INFO_T & timing_info)
   {
      const int16_t num_clusters = tracker_info.num_active_clusters;
      int16_t active_clusters[NUMBER_OF_CLUSTERS];

      const float32_t start_time = get_wall_time();

      (void)std::copy(cmn::begin(tracker_info.active_cluster_ids), cmn::end(tracker_info.active_cluster_ids), cmn::begin(active_clusters));
      for (int16_t i = 0; i < num_clusters; i++)
      {
         const int16_t cluster_idx = active_clusters[i] - 1;

         // Add "new" dets to cluster old_det_idx
         for (int16_t j = 0; j < clusters[cluster_idx].ndets; j++)
         {
            if ((detection_hist.n_occupied < static_cast<int32_t>(tracker_info.variant.num_hist_dets)) ||
               (clusters[cluster_idx].num_old_dets > 0))
            {

               uint16_t hist_det_idx = tracker_info.variant.num_hist_dets; // Uninitialized value (too big)
               if (Find_Next_Hist_Det_Idx(hist_det_idx, cluster_idx, tracker_info, detection_hist, clusters))
               {
                  const uint16_t det_idx = static_cast<uint16_t>(clusters[cluster_idx].detids[j]) - 1U;
                  const rspp_variant_A::RSPP_Detection_T& detection = raw_detection_list.detections[det_idx];
                  const int32_t sensor_idx = detection .raw.sensor_id - 1;
                  const F360_Det_Look_ID_T look_id = sensors[sensor_idx].variable.look_id;
                  const bool f_range_in_all_looks = (detection.raw.range < F360_Min_Kth_Element(sensors[sensor_idx].constant.range_limits, 2U));

                  detection_hist.det_data[hist_det_idx].vcs_position.x = det_props[det_idx].vcs_position.x;
                  detection_hist.det_data[hist_det_idx].vcs_position.y = det_props[det_idx].vcs_position.y;

                  detection_hist.det_data[hist_det_idx].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_X] =
                     det_props[det_idx].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_X];
                  detection_hist.det_data[hist_det_idx].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_Y] =
                     det_props[det_idx].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_Y];
                  detection_hist.det_data[hist_det_idx].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_X] =
                     det_props[det_idx].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_X];
                  detection_hist.det_data[hist_det_idx].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_Y] =
                     det_props[det_idx].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_Y];

                  detection_hist.det_data[hist_det_idx].rdot = det_props[det_idx].range_rate_dealiased;
                  detection_hist.det_data[hist_det_idx].rdot_comp = det_props[det_idx].range_rate_compensated;
                  detection_hist.det_data[hist_det_idx].vcs_az = detection.processed.vcs_az;
                  detection_hist.det_data[hist_det_idx].look_type = Get_Look_Type(look_id);
                  detection_hist.det_data[hist_det_idx].range_type = Get_Range_Type(look_id);
                  detection_hist.det_data[hist_det_idx].time_since_meas = sensors[sensor_idx].variable.time_since_measurement_s;
                  detection_hist.det_data[hist_det_idx].v_wrapping = sensors[sensor_idx].constant.v_wrapping[look_id];
                  detection_hist.det_data[hist_det_idx].r_wrapping = sensors[sensor_idx].constant.r_wrapping[look_id];
                  detection_hist.det_data[hist_det_idx].motion_status = detection.processed.motion_status;
                  detection_hist.det_data[hist_det_idx].f_dealiased = det_props[det_idx].f_dealiased;
                  detection_hist.det_data[hist_det_idx].f_is_range_in_all_looks = f_range_in_all_looks;
                  detection_hist.det_data[hist_det_idx].f_potential_angle_jump = det_props[det_idx].f_potential_angle_jump;
                  detection_hist.det_data[hist_det_idx].cluster_idx = cluster_idx;
                  detection_hist.det_data[hist_det_idx].wheel_spin_type = det_props[det_idx].wheel_spin_type;
               }
            }
         }
         // Clean up information from previous tracker loop
         clusters[cluster_idx].ndets = 0;
         std::fill(cmn::begin(clusters[cluster_idx].detids), cmn::end(clusters[cluster_idx].detids), static_cast<int16_t>(0));
      }

      timing_info.update_det_hist = get_wall_time() - start_time;
   }


   static bool Find_Next_Hist_Det_Idx(
      uint16_t& hist_det_idx,
      const int16_t cluster_idx,
      const F360_Tracker_Info_T& tracker_info,
      F360_Detection_Hist_T& detection_hist,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS])
   {

      bool f_update_det_data = false;
      if ((clusters[cluster_idx].num_old_dets < static_cast<int16_t>(tracker_info.variant.num_dets_in_track)) &&
         (detection_hist.n_occupied < static_cast<int16_t>(tracker_info.variant.num_hist_dets)))
      {
         for (uint16_t k = 0U; ((k < tracker_info.variant.num_hist_dets) && (!f_update_det_data)); k++)
         {
            // Find first empty space in detection_hist
            if (!detection_hist.f_idx_occupied[k])
            {
               hist_det_idx = k;
               detection_hist.n_occupied++;
               detection_hist.f_idx_occupied[k] = true;
               detection_hist.max_occupation = detection_hist.max_occupation < detection_hist.n_occupied ? detection_hist.n_occupied : detection_hist.max_occupation;
               clusters[cluster_idx].old_det_idx[clusters[cluster_idx].num_old_dets] = static_cast<int16_t>(hist_det_idx);
               clusters[cluster_idx].num_old_dets++;
               f_update_det_data = true;
            }
         }
      }
      else
      {
         // Remove the oldest historical detection associated to the cluster and insert the new one
         hist_det_idx = static_cast<uint16_t>(clusters[cluster_idx].old_det_idx[0]);
         (void)std::copy(&clusters[cluster_idx].old_det_idx[1],
                         cmn::end(clusters[cluster_idx].old_det_idx),
                         cmn::begin(clusters[cluster_idx].old_det_idx));
         clusters[cluster_idx].old_det_idx[clusters[cluster_idx].num_old_dets - 1] = static_cast<int16_t>((hist_det_idx));
         f_update_det_data = true;
      }

      return f_update_det_data && (tracker_info.variant.num_hist_dets > hist_det_idx);

   }
}
