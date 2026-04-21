/*===================================================================================*\
* FILE: f360_clustering_detections.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* The file contains the definition of functions for clustering of detections
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_clustering_detections.h"
#include "f360_dbscan.h"
#include "f360_get_new_cluster_id.h"
#include "f360_get_wall_time.h"
#include "f360_initialize_clusters.h"
#include "f360_math_func.h"
#include "f360_calculate_curvi_position.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Cluster_Moving_Detections()
   *===========================================================================
   * RETURN VALUE:
   * Number of moving clusters
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * const F360_Calibrations_T &calibrations,
   * F360_Tracker_Info_T &tracker_info,
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
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
   * This function clusters moving detections
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Cluster_Moving_Detections(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      F360_Tracker_Info_T &tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info)
   {
      const float32_t moving_dets_clustering_radius = 2.0F; // TODO: move to calibs later
      const uint32_t moving_dets_min_pts_in_cluster = 1U; // TODO: move to calibs later
      const float32_t start_time = get_wall_time();
      F360_Clustering_Configuration_T cluster_moving_config;

      cluster_moving_config.Cluster_Distance_Sq_Function = &Clustering_Distance_Position_Rangerate;
      cluster_moving_config.Detection_Cluster_Check = &Cluster_Moving_Check;
      cluster_moving_config.clustering_radius = moving_dets_clustering_radius;
      cluster_moving_config.clustering_radius_sq = moving_dets_clustering_radius * moving_dets_clustering_radius;
      cluster_moving_config.min_pts_in_cluster = moving_dets_min_pts_in_cluster;

      Cluster_Detections(raw_detection_list, sensors, calibrations, host, cluster_moving_config, tracker_info, detection_props, clusters);

      timing_info.cluster_moving_detections = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Cluster_Left_Over_Detections()
   *===========================================================================
   * RETURN VALUE:
   * Number of non-moving clusters
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * const F360_Calibrations_T &calibrations,
   * F360_Tracker_Info_T &tracker_info,
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
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
   * This function updates cluster of non-moving detections.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Cluster_Leftover_Detections(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      F360_Tracker_Info_T &tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info
   )
   {
      const float32_t leftover_dets_clustering_radius = 1.5F; // TODO: move to calibs later
      const uint32_t leftover_dets_min_pts_in_cluster = 1U; // TODO: move to calibs later
      const float32_t start_time = get_wall_time();
      F360_Clustering_Configuration_T cluster_leftovers_config;

      cluster_leftovers_config.Cluster_Distance_Sq_Function = &Clustering_Distance_Position;
      cluster_leftovers_config.Detection_Cluster_Check = &Cluster_Leftovers_Check;
      cluster_leftovers_config.clustering_radius = leftover_dets_clustering_radius;
      cluster_leftovers_config.clustering_radius_sq = leftover_dets_clustering_radius * leftover_dets_clustering_radius;
      cluster_leftovers_config.min_pts_in_cluster = leftover_dets_min_pts_in_cluster;

      Cluster_Detections(raw_detection_list, sensors, calibrations, host, cluster_leftovers_config, tracker_info, detection_props, clusters);

      timing_info.cluster_leftover_detections = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Clustering_Distance_Postion_Rangerate()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Props_T &det_1
   * const F360_Detection_Props_T &det_2
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
   * This function does distance measure for clustering function.  Includes range rate gating.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Clustering_Distance_Position_Rangerate(
      const F360_Detection_Props_T &det_1, 
      const F360_Detection_Props_T &det_2)
   {
      // TODO: implement logic to dynamically adjust range_rate_gate to current scenario
      const float32_t range_rate_gate = 2.0F;
      return (std::abs(det_1.range_rate_dealiased - det_2.range_rate_dealiased) <= range_rate_gate) ? Clustering_Distance_Position(det_1, det_2) : INFTY;
   }

     /*===========================================================================*\
     * FUNCTION: Clustering_Distance_Postion()
     *===========================================================================
     * RETURN VALUE:
     * None
     *
     * PARAMETERS:
     * const F360_Detection_Props_T &det_1
     * const F360_Detection_Props_T &det_2
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
     * This function does distance measure for clustering function based on positon only.
     *
     * PRECONDITIONS:
     *
     * POSTCONDITIONS:
     * None
     *
     \*===========================================================================*/
   float32_t Clustering_Distance_Position(
      const F360_Detection_Props_T &det_1, 
      const F360_Detection_Props_T &det_2)
   {
      float32_t distance_diff = det_1.vcs_position.x - det_2.vcs_position.x;
      float32_t sq_distance = distance_diff * distance_diff;
      distance_diff = det_1.vcs_position.y - det_2.vcs_position.y;
      sq_distance += distance_diff * distance_diff;

      return sq_distance;
   }

     /*===========================================================================*\
     * FUNCTION: Cluster_Leftovers_Check()
     *===========================================================================
     * RETURN VALUE:
     * None
     *
     * PARAMETERS:
     * const F360_Calibrations_T &calib
     * const F360_Detection_Props_T &det_p
     * const rspp_variant_A::RSPP_Detection_T &det
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
     * This function checks if input detection should be included in leftovers clustering process
     *
     * PRECONDITIONS:
     *
     * POSTCONDITIONS:
     * None
     *
     \*===========================================================================*/
   bool Cluster_Leftovers_Check(
      const F360_Calibrations_T& calib,
      const F360_Radar_Sensor_T& sensor,
      const F360_Detection_Props_T& det_p,
      const rspp_variant_A::RSPP_Detection_T& det,
      const float32_t host_vcs_speed)
   {
      const bool result = (0 == det_p.object_track_id)
                           && (0 == det_p.cluster_id)
                           && (F360_DETECTION_WHEELSPIN_TYPE_INVALID == det_p.wheel_spin_type)
                           && (!det_p.f_double_bounce)
                           && (!det_p.f_close_target)
                           && (!det_p.f_det_pair)
                           && (!det_p.f_FOV_edge)
                           && (!det_p.f_water_spray)
                           && (det_p.f_ok_to_use)
                           && (!det.raw.f_host_veh_clutter)
                           && (!det.raw.f_bistatic)
                           && (!((fabsf(det.raw.elevation) > calib.k_mrr360_max_abs_elev_angle_rad) && 
                              (sensor.constant.sensor_type == F360_SENSOR_TYPE_MRR360_RADAR) &&
                              (host_vcs_speed > calib.k_mrr360_min_host_speed_el_check)));
      return result;
   }

   /*===========================================================================*\
   * FUNCTION: Cluster_Moving_Check()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib,
   * const F360_Detection_Props_T &det_p
   * const rspp_variant_A::RSPP_Detection_T &det
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
   * This function checks if input detection should be included in moving clustering process
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Cluster_Moving_Check(
      const F360_Calibrations_T &calib,
      const F360_Radar_Sensor_T &sensor,
      const F360_Detection_Props_T &det_p,
      const rspp_variant_A::RSPP_Detection_T &det,
      const float32_t host_vcs_speed)
   {
      const bool f_preconditions = ((calib.is_separate_clustering && (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == det.processed.motion_status))
                                     || (!calib.is_separate_clustering)) && (0 == det_p.object_track_id);

      const bool f_valid_combined = (F360_DETECTION_WHEELSPIN_TYPE_INVALID == det_p.wheel_spin_type)
                                       && (!det_p.f_double_bounce)
                                       && (!det_p.f_close_target)
                                       && (!det_p.f_det_pair)
                                       && (!det_p.f_FOV_edge)
                                       && (det_p.f_ok_to_use)
                                       && (!det_p.f_water_spray)
                                       && (F360_INVALID_UNSIGNED_ID == det_p.on_sep_id)
                                       && (!det.raw.f_host_veh_clutter)
                                       && (!det.raw.f_bistatic)
                                       && (!((fabsf(det.raw.elevation) > calib.k_mrr360_max_abs_elev_angle_rad) && 
                                          (sensor.constant.sensor_type == F360_SENSOR_TYPE_MRR360_RADAR) &&
                                          (host_vcs_speed > calib.k_mrr360_min_host_speed_el_check)));
      
      const bool result = f_preconditions && ((det_p.f_valid_for_liberal_tracking) || f_valid_combined);
      return result;
   }

   /*===========================================================================*\
   * FUNCTION: Cluster_Detections()
   *===========================================================================
   * RETURN VALUE:
   * Number of clusters
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Calibrations_T& calibrations
   * const F360_Host_T& host
   * const F360_Clustering_Configuration_T& clustering_config
   * F360_Tracker_Info_T& tracker_info
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]
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
   * This function clusters detections according to the cluster_configuration
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Cluster_Detections(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Clustering_Configuration_T& clustering_config,
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS])
   {
      int16_t valid_det_sorted_idxs[MAX_NUMBER_OF_DETECTIONS];
      int16_t valid_det_count = 0;
      bool valid_dets[MAX_NUMBER_OF_DETECTIONS] = {};

      if (raw_detection_list.number_of_valid_detections > 0U)
      {
         Find_And_Prioritize_Detections(raw_detection_list, sensors, calibrations, host, clustering_config, detection_props, valid_det_count, valid_det_sorted_idxs, valid_dets);
      }

      if (0 < valid_det_count)
      {
         F360_Local_Clusters_T local_cluster_data = {};
         local_cluster_data.num_clusters = 0U;

         DBscan(tracker_info, valid_dets, static_cast<uint16_t>(valid_det_count), detection_props, raw_detection_list, clustering_config, valid_det_sorted_idxs, local_cluster_data);

         Initialize_Clusters(raw_detection_list, calibrations, sensors, local_cluster_data, tracker_info, detection_props, clusters);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Find_And_Prioritize_Detections()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Calibrations_T& calibrations,
   * const F360_Host_T& host,
   * const F360_Clustering_Configuration_T& clustering_config,
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * int32_t &valid_det_count,
   * int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS],
   * bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS]
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
   * This function sorts list of detection indexes based on detection
   * vcs position. There are five priority zones in vcs coordinates:
   * first - in front of the host within pos.y threshold
   * second - behind the host within pos.y threshold
   * third - within semicircle radius in front of the host
   * fourth - rest of detections in front of the host
   * fifth - rest of detections behind the host
   * 
   * Within single zone detections are sorted by longitudinal position.
   * 
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Find_And_Prioritize_Detections(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Clustering_Configuration_T& clustering_config,
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      int16_t &valid_det_count,
      int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS],
      bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS])
   {
      
      constexpr float32_t pos_y_threshold = 20.0F;
      int16_t fourth_fifth_priority_zone[MAX_NUMBER_OF_DETECTIONS];
      int16_t third_priority_zone[MAX_NUMBER_OF_DETECTIONS];
      int16_t first_second_priority_zone_det_count = 0;
      int16_t fourth_fifth_priority_zone_det_count = 0;
      int16_t third_priority_zone_det_count = 0;

      constexpr float32_t det_starting_xpos = 0.0F;

      // Guaranteed to be either < det_starting_pos
      // Or the detection with minimum overshooting xpos relative det_starting_xpos
      int16_t det_idx = static_cast<int16_t>(Get_First_Relevant_Long_Sorted_Det_Idx(det_starting_xpos, raw_detection_list));
      
      // Find the first detection with longpos > det_starting_xpos
      for (uint16_t i = 0U; i < MAX_NUMBER_OF_DETECTIONS; i++)
      {
         if (det_idx != F360_INVALID_ID)
         {
            if (raw_detection_list.detections[det_idx].processed.vcs_position_x > det_starting_xpos)
            {
               break;
            }
            det_idx = raw_detection_list.detections[det_idx].processed.next_sorted_idx;
         }
         else
         {
            break;
         }
      }

      const int16_t det_idx_starting_pos = det_idx;

      for (uint16_t i = 0U; i < MAX_NUMBER_OF_DETECTIONS; i++)
      {
         if (det_idx != F360_INVALID_ID)
         {
            const int32_t sensor_idx = raw_detection_list.detections[det_idx].raw.sensor_id - 1;
            const bool f_det_valid_for_clustering = clustering_config.Detection_Cluster_Check(calibrations, sensors[sensor_idx], detection_props[det_idx], raw_detection_list.detections[det_idx], host.vcs_speed);

            if (f_det_valid_for_clustering)
            {
               const float32_t curvi_lat_pos = Calculate_Curvi_Lat_Pos(host, raw_detection_list.detections[det_idx].processed.vcs_position_x, raw_detection_list.detections[det_idx].processed.vcs_position_y);
               const bool f_det_within_first_zone = std::fabs(curvi_lat_pos) < pos_y_threshold;

               //first priority zone
               if (f_det_within_first_zone)
               {
                  sorted_det_idxs[first_second_priority_zone_det_count] = det_idx;
                  first_second_priority_zone_det_count++;
                  valid_dets[det_idx] = true;
               }

               //third priority zone
               else if (Is_Detection_In_Third_Priority_Zone(raw_detection_list, host, det_idx))
               {
                  third_priority_zone[third_priority_zone_det_count] = det_idx;
                  third_priority_zone_det_count++;
                  valid_dets[det_idx] = true;
               }

               //fourth priority zone
               else
               {
                  fourth_fifth_priority_zone[fourth_fifth_priority_zone_det_count] = det_idx;
                  fourth_fifth_priority_zone_det_count++;
                  valid_dets[det_idx] = true;
               }
            }
            det_idx = raw_detection_list.detections[det_idx].processed.next_sorted_idx;
         }
         else
         {
            break;
         }
      }

      //detection in front of starting_pos detection are iterated, now loop backwards over detections behind
      det_idx = raw_detection_list.detections[det_idx_starting_pos].processed.prev_sorted_idx;

      for (uint16_t i = 0U; i < MAX_NUMBER_OF_DETECTIONS; i++)
      {
         if (det_idx != F360_INVALID_ID)
         {
            const int32_t sensor_idx = raw_detection_list.detections[det_idx].raw.sensor_id - 1;
            const bool f_det_valid_for_clustering = clustering_config.Detection_Cluster_Check(calibrations, sensors[sensor_idx], detection_props[det_idx], raw_detection_list.detections[det_idx], host.vcs_speed);

            if (f_det_valid_for_clustering)
            {
               const bool f_det_within_second_zone = std::fabs(raw_detection_list.detections[det_idx].processed.vcs_position_y) < pos_y_threshold;

               //second priority zone
               if (f_det_within_second_zone)
               {
                  sorted_det_idxs[first_second_priority_zone_det_count] = det_idx;
                  first_second_priority_zone_det_count++;
                  valid_dets[det_idx] = true;
               }

               //fifth priority zone
               else
               {
                  fourth_fifth_priority_zone[fourth_fifth_priority_zone_det_count] = det_idx;
                  fourth_fifth_priority_zone_det_count++;
                  valid_dets[det_idx] = true;
               }
            }
            det_idx = raw_detection_list.detections[det_idx].processed.prev_sorted_idx;
         }
         else
         {
            break;
         }
      }

      //concate detection lists
      valid_det_count = first_second_priority_zone_det_count + third_priority_zone_det_count + fourth_fifth_priority_zone_det_count;
      for (int32_t i = 0; i < third_priority_zone_det_count; i++)
      {
         sorted_det_idxs[(first_second_priority_zone_det_count + i)] = third_priority_zone[i];
      }
      for (int32_t i = 0; i < fourth_fifth_priority_zone_det_count; i++)
      {
         sorted_det_idxs[(first_second_priority_zone_det_count + third_priority_zone_det_count + i)] = fourth_fifth_priority_zone[i];
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Detection_In_Third_Priority_Zone()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Host_T& host,
   * const int16_t &det_idx
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
   * This function checks if given detection is inside third priority zone thresholds.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Detection_In_Third_Priority_Zone(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Host_T& host,
      const int16_t &det_idx)
   {
      constexpr float32_t third_priority_zone_ttc_sq = 25.0F;
      constexpr float32_t min_semicircle_radius = 20.0F;
      const float32_t det_sq_dist = F360_Get_Hypotenuse_Squared(raw_detection_list.detections[det_idx].processed.vcs_position_x, raw_detection_list.detections[det_idx].processed.vcs_position_y);
      const float32_t semicircle_sq_radius = third_priority_zone_ttc_sq * host.vcs_speed * host.vcs_speed;
      const float32_t saturated_semicircle_sq_radius = Clamp(semicircle_sq_radius, min_semicircle_radius, INFTY);
      return det_sq_dist < saturated_semicircle_sq_radius;
   }
}
