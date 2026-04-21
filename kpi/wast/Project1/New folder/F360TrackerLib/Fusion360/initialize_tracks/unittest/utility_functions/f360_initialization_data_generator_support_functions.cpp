/*===========================================================================*\
* FILE: f360_initialization_data_generator_support_functions.cpp
*============================================================================
* Copyright ? 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential ? Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains support functions for tracks, clusters, detections data generation for unit testing
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [30-Mar-2018]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_initialization_data_generator_support_functions.h"

#include "f360_data_generator.h"
#include "f360_initialize_tracks.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   float32_t Get_Norm_From_Cart_Pos(
      float32_t posn_lat,
      float32_t posn_long
   )
   {
      float32_t norm = F360_Get_Hypotenuse(posn_lat, posn_long);
      return norm;
   }

   float32_t Get_RangeRate_From_Cart_Coordinates(
      float32_t posn_lat,
      float32_t posn_long,
      float32_t vel_lat,
      float32_t vel_long
   )
   {
      float32_t azimuth = Get_Azimuth_From_Cart_Pos(posn_lat, posn_long);

      float32_t rrange = F360_Cosf(azimuth) * vel_long + F360_Sinf(azimuth) * vel_lat;
      return rrange;
   }

   float32_t Move_Pos_By_Step(
      float32_t posn,
      float32_t vel,
      float32_t time_step
   )
   {
      float32_t new_posn = posn + vel * time_step;
      return new_posn;
   }

   F360_Detection_Hist_Data_T Gen_Single_Hist_Det(
      const rspp_variant_A::RSPP_Detection_Motion_Status_T &motion_status,
      float32_t expected_pos_lat,
      float32_t expected_pos_long,
      float32_t vel_lat,
      float32_t vel_long,
      float32_t dt,
      float32_t pos_std
   )
   {
      F360_Detection_Hist_Data_T singe_det_hist = {};
      singe_det_hist.vcs_position.x = expected_pos_long;
      singe_det_hist.vcs_position.y = expected_pos_lat;
      singe_det_hist.time_since_meas = dt;

      singe_det_hist.vcs_position.y = Move_Pos_By_Step(singe_det_hist.vcs_position.y, -vel_lat, dt);
      singe_det_hist.vcs_position.x = Move_Pos_By_Step(singe_det_hist.vcs_position.x, -vel_long, dt);
      
      singe_det_hist.vcs_az = Get_Azimuth_From_Cart_Pos(singe_det_hist.vcs_position.y, singe_det_hist.vcs_position.x);
      singe_det_hist.rdot_comp = Get_RangeRate_From_Cart_Coordinates(singe_det_hist.vcs_position.y, singe_det_hist.vcs_position.x, vel_lat, vel_long);
      
      singe_det_hist.motion_status = motion_status;
      singe_det_hist.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      singe_det_hist.position_cov_nees[0][0] = pos_std * pos_std;
      singe_det_hist.position_cov_nees[0][1] = 0.0F;
      singe_det_hist.position_cov_nees[1][0] = 0.0F;
      singe_det_hist.position_cov_nees[1][1] = pos_std * pos_std;
      return singe_det_hist;
   }

   std::pair<rspp_variant_A::RSPP_Detection_T,F360_Detection_Props_T> Gen_Single_Det(
      const rspp_variant_A::RSPP_Detection_Motion_Status_T &motion_status,
      float32_t expected_pos_lat,
      float32_t expected_pos_long,
      float32_t vel_lat,
      float32_t vel_long,
      float32_t dt,
      float32_t pos_std,
      uint16_t global_id
   )
   {
      std::pair<rspp_variant_A::RSPP_Detection_T,F360_Detection_Props_T> singe_det{};
      singe_det.second.vcs_position.x = expected_pos_long;
      singe_det.second.vcs_position.y = expected_pos_lat;
      singe_det.first.processed.global_id = global_id;

      singe_det.second.vcs_position.y = Move_Pos_By_Step(singe_det.second.vcs_position.y, -vel_lat, dt);
      singe_det.second.vcs_position.x = Move_Pos_By_Step(singe_det.second.vcs_position.x, -vel_long, dt);

      singe_det.second.range_rate_compensated = Get_RangeRate_From_Cart_Coordinates(singe_det.second.vcs_position.y, singe_det.second.vcs_position.x, vel_lat, vel_long);

      singe_det.first.processed.motion_status = motion_status;
      singe_det.second.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      singe_det.second.position_cov_nees[0][0] = pos_std * pos_std;
      singe_det.second.position_cov_nees[0][1] = 0.0F;
      singe_det.second.position_cov_nees[1][0] = 0.0F;
      singe_det.second.position_cov_nees[1][1] = pos_std * pos_std;
      return singe_det;
   }

   int16_t Get_Det_First_Free_Idx(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list
   )
   {
      uint16_t free_det_idx = 0;

      for (uint16_t det_idx = 0; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      {
         if (raw_detection_list.detections[det_idx].processed.global_id == 0)
         {
            free_det_idx = det_idx;
            break;
         }
      }
      return free_det_idx;
   }

   void Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = cluster_pos_long;
      clusters[clust_idx_1].vcs_position.y = cluster_pos_lat;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[0] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_STATIONARY;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
      // create and associate first detection to cluster
      uint16_t det_id = Get_Det_First_Free_Idx(raw_detection_list);
      clusters[clust_idx_1].detids[0] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 0.0;
      float32_t dt = 0.0F;
      float32_t std = 1.0F;
      
      auto dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_id + 1
      );
      det_props[det_id] = dets_info.second;

      // create and associate second detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[1] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.0F;
      std = 1.0F;

      dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_id + 1
      );
      det_props[det_id] = dets_info.second;

      // create and associate third detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[2] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.5;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.0F;
      std = 1.0F;

      dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_id + 1
      );
      det_props[det_id] = dets_info.second;
   }

   void Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = cluster_pos_long;
      clusters[clust_idx_1].vcs_position.y = cluster_pos_lat;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[0] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_STATIONARY;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
      // create and associate first detection to cluster
      uint16_t det_idx = Get_Det_First_Free_Idx(raw_detections);
      clusters[clust_idx_1].detids[0] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 0.0;
      float32_t dt = 0.0F;
      float32_t std = 1.0F;

      auto dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_idx + 1
      );
      raw_detections.detections[det_idx] = dets_info.first;
      det_props[det_idx] = dets_info.second;
      raw_detections.detections[det_idx].raw.sensor_id = 1;

      // create and associate second detection to cluster
      det_idx = det_idx + 1U;
      clusters[clust_idx_1].detids[1] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.0F;
      std = 1.0F;

      dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_idx + 1
      );
      raw_detections.detections[det_idx] = dets_info.first;
      det_props[det_idx] = dets_info.second;
      raw_detections.detections[det_idx].raw.sensor_id = 1;

      // create and associate third detection to cluster
      det_idx = det_idx + 1U;
      clusters[clust_idx_1].detids[2] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.5;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.0F;
      std = 1.0F;

      dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_idx + 1
      );
      raw_detections.detections[det_idx] = dets_info.first;
      det_props[det_idx] = dets_info.second;

      // create and associate fourth detection to cluster
      det_idx = det_idx + 1U;
      clusters[clust_idx_1].detids[3] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[1]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.3;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x + 0.3;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.0F;
      std = 1.0F;

      dets_info = Gen_Single_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std,
         det_idx + 1
      );
      raw_detections.detections[det_idx] = dets_info.first;
      det_props[det_idx] = dets_info.second;
   }

   void Fill_Clusters_With_Moving_Cluster_With_4_Moving_Dets_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat,
      float32_t expected_vel_long,
      float32_t expected_vel_lat
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      F360_Cluster_T &cluster = clusters[clust_idx_1];
      
      cluster.id = tracker_info.num_active_clusters;
      cluster.vcs_position.x = cluster_pos_long;
      cluster.vcs_position.y = cluster_pos_lat;
      cluster.f_dealiased = true;
      cluster.num_types_of_dets[0] = 0;
      cluster.motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create 4 dets and associate them to cluster
      float32_t lat_offsets[] = { -0.35F, -0.15F, 0.1F, 0.35F };
      float32_t long_offsets[] = { -0.3F, -0.2F, 0.2F, 0.3F };

      const uint16_t first_det_index = Get_Det_First_Free_Idx(raw_detections);
      for (int det_counter = 0; det_counter < 4; det_counter++)
      {
         const uint16_t det_index = first_det_index +  det_counter;
         cluster.detids[det_counter] = det_index + 1;
         cluster.ndets++;
         cluster.num_types_of_dets[1]++;
         float32_t expected_pos_lat = cluster.vcs_position.y + lat_offsets[det_counter];
         float32_t expected_pos_long = cluster.vcs_position.x + long_offsets[det_counter];

         float32_t dt = 0.0F;
         float32_t std = 1.0F;

         auto dets_info = Gen_Single_Det(
            motion_status,
            expected_pos_lat,
            expected_pos_long,
            expected_vel_lat,
            expected_vel_long,
            dt,
            std,
            det_index + 1
         );
         raw_detections.detections[det_index] = dets_info.first;
         det_props[det_index] = dets_info.second;
         raw_detections.detections[det_index].raw.sensor_id = 1;

         raw_detections.detections[det_index].processed.vcs_az = Get_Azimuth_From_Cart_Pos(det_props[det_index].vcs_position.y, det_props[det_index].vcs_position.x);
         raw_detections.detections[det_index].processed.sin_vcs_az = F360_Sinf(raw_detections.detections[det_index].processed.vcs_az);
         raw_detections.detections[det_index].processed.cos_vcs_az = F360_Cosf(raw_detections.detections[det_index].processed.vcs_az);
      }
   }
}
