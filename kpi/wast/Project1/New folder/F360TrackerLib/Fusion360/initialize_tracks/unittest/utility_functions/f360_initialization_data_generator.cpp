/*===========================================================================*\
* FILE: f360_initialization_data_generator.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains tracks, clusters, detections data generation for unit testing
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

#include "f360_initialize_tracks.h"
#include "f360_math_func.h"
#include "f360_initialization_data_generator.h"
#include "f360_data_generator.h"
#include "f360_initialization_data_generator_support_functions.h"

namespace f360_variant_A
{
   void Define_Test_Vector_4_Dealiased_and_2_NotDealiased_Clusters(
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS]
   )
   {
      uint16_t clust_idx = 1;
      clusters[clust_idx].id = 2;
      clusters[clust_idx].vcs_position.x = 11.4F;
      clusters[clust_idx].f_dealiased = true;
      clusters[clust_idx].ndets = 4;
      clusters[clust_idx].num_types_of_dets[0] = 4;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }

      clust_idx = 3;
      clusters[clust_idx].id = 4;
      clusters[clust_idx].vcs_position.x = 11.0F;
      clusters[clust_idx].f_dealiased = true;
      clusters[clust_idx].ndets = 4;
      clusters[clust_idx].num_types_of_dets[0] = 4;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }

      clust_idx = 5;
      clusters[clust_idx].id = 6;
      clusters[clust_idx].vcs_position.x = 5.5F;
      clusters[clust_idx].f_dealiased = false;
      clusters[clust_idx].ndets = 2;
      clusters[clust_idx].num_types_of_dets[0] = 1;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }

      clust_idx = 6;
      clusters[clust_idx].id = 7;
      clusters[clust_idx].vcs_position.x = 23.3F;
      clusters[clust_idx].f_dealiased = true;
      clusters[clust_idx].ndets = 4;
      clusters[clust_idx].num_types_of_dets[0] = 4;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }

      clust_idx = 9;
      clusters[clust_idx].id = 10;
      clusters[clust_idx].vcs_position.x = 9.9F;
      clusters[clust_idx].f_dealiased = false;
      clusters[clust_idx].ndets = 2;
      clusters[clust_idx].num_types_of_dets[0] = 1;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }

      clust_idx = 15;
      clusters[clust_idx].id = 16;
      clusters[clust_idx].vcs_position.x = 20.0F;
      clusters[clust_idx].f_dealiased = true;
      clusters[clust_idx].ndets = 4;
      clusters[clust_idx].num_types_of_dets[0] = 4;
      for (int32_t index = 0U; index < clusters[clust_idx].ndets; index++)
      {
         clusters[clust_idx].detids[index] = index + 1;
      }
   }

   void Define_Test_Vector_4_Obj_Tracks(
      F360_Object_Track_T (&obj_track)[NUMBER_OF_OBJECT_TRACKS]
   )
   {
      uint16_t id = 1;
      obj_track[id].vcs_position.x = 5.0F;
      obj_track[id].vcs_position.y = 2.0F;
      obj_track[id].Set_Bbox_Orientation(Angle{ 0.0F });
      obj_track[id].vcs_velocity.longitudinal = 10.0f;
      obj_track[id].vcs_velocity.lateral = 0.0f;
      obj_track[id].f_moving = true;
      obj_track[id].Update_Bbox_Size(3.0F, 7.0F);

      id = 2;
      obj_track[id].vcs_position.x = 15.0F;
      obj_track[id].vcs_position.y = 12.0F;
      obj_track[id].Set_Bbox_Orientation(Angle{ -F360_PI_2 });
      obj_track[id].vcs_velocity.longitudinal = -10.0f;
      obj_track[id].vcs_velocity.lateral = 0.0f;
      obj_track[id].f_moving = true;
      obj_track[id].Update_Bbox_Size(4.0F, 7.0F);

      id = 3;
      obj_track[id].vcs_position.x = -10.0F;
      obj_track[id].vcs_position.y = 0.0F;
      obj_track[id].Set_Bbox_Orientation(Angle{ 0.0F });
      obj_track[id].vcs_velocity.longitudinal = 10.0f;
      obj_track[id].vcs_velocity.lateral = 0.0f;
      obj_track[id].f_moving = true;
      obj_track[id].Update_Bbox_Size(6.0F, 3.0F);

      id = 4;
      obj_track[id].vcs_position.x = 0.0F;
      obj_track[id].vcs_position.y = -20.0F;
      obj_track[id].Set_Bbox_Orientation(Angle{ F360_PI / 4.0F });
      obj_track[id].vcs_velocity.longitudinal = 0.0f;
      obj_track[id].vcs_velocity.lateral = 10.0f;
      obj_track[id].f_moving = true;
      obj_track[id].Update_Bbox_Size(4.0F, 2.0F);
   }

   void Define_Test_Vector_N_Obj_Tracks(
      F360_Object_Track_T(&obj_track)[NUMBER_OF_OBJECT_TRACKS],
      uint16_t tracks_to_generate
   )
   {
      if (tracks_to_generate > NUMBER_OF_OBJECT_TRACKS)
      {
         tracks_to_generate = NUMBER_OF_OBJECT_TRACKS;
      }

      for (uint16_t idx = 0U; idx < tracks_to_generate; ++idx)
      {
         // Generate some simple deterministic randomness to diversify values
         obj_track[idx].id = idx + 1;
         float32_t length = 5.0F + F360_Fmodf(idx * 0.02F, 0.8F) - F360_Fmodf(idx * 0.03F, 0.6F);
         float32_t width = 2.5F + F360_Fmodf(idx * 0.01F, 0.5F) - F360_Fmodf(idx * 0.05F, 0.4F);
         obj_track[idx].vcs_position.x = 5.0F - F360_Fmodf(idx * 0.5F, 10.0F);
         obj_track[idx].vcs_position.y = 2.0F + F360_Fmodf(idx * 0.2F, 8.0F);
         obj_track[idx].Set_Bbox_Orientation(Angle{ 0.0f + F360_Fmodf(idx * 0.1F, 1.0F) });
         obj_track[idx].vcs_velocity.longitudinal = 10.0f + F360_Fmodf(idx * 0.2F, 12.0F);
         obj_track[idx].vcs_velocity.lateral = 0.0f - F360_Fmodf(idx * 0.5F, 4.0F);
         obj_track[idx].f_moving = true;
         obj_track[idx].priority = F360_Saturate(0.1F + F360_Fmodf(idx * 0.1F, 0.9F), 0.0F, 1.0F);
         obj_track[idx].Update_Bbox_Size(length, width);
      }
   }

   void Define_Test_Vector_4_Valid_Moving_Hist_Dets_Different_Positions(
      F360_Detection_Hist_T &det_hist
   )
   {
      det_hist.n_occupied = 5;
      det_hist.max_occupation = 5;

      uint16_t id = 0U;
      float32_t expected_pos_lat = 11.0F;
      float32_t expected_pos_long = 10.0F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 10.0F;
      float32_t dt = 0.3F;
      float32_t std = 1.0F;
      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 1;
      expected_pos_lat = 1.5F;
      expected_pos_long = 5.2F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 2;
      expected_pos_lat = 19.9F;
      expected_pos_long = 20.0F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.5F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 3;
      expected_pos_lat = 29.9F;
      expected_pos_long = 30.0F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.5F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 4;
      expected_pos_lat = 22.0F;
      expected_pos_long = 12.9F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.9F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
   }

   void Define_Test_Vector_3_Static_Valid_Hist_Dets(
      F360_Detection_Hist_T &det_hist
   )
   {
      det_hist.n_occupied = 3;
      det_hist.max_occupation = 3;

      uint16_t id = 0U;
      float32_t expected_pos_lat = 10.0F;
      float32_t expected_pos_long = 0.0F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 0.0F;
      float32_t dt = 0.3F;
      float32_t std = 1.0F;
      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 1;
      expected_pos_lat = 10.0F;
      expected_pos_long = 0.3F;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.9F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      id = 2;
      expected_pos_lat = 10.3F;
      expected_pos_long = 0.0F;
      vel_lat = 0.0F;
      vel_long = 0.0F;
      dt = 0.9F;
      std = 1.0F;

      det_hist.f_idx_occupied[id] = true;
      det_hist.det_data[id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
   }

   void Define_Test_Vector_4_Valid_Moving_Dets_Different_Positions(
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   )
   {
      uint16_t det_id = 0U;
      float32_t expected_pos_lat = 0.0F;
      float32_t expected_pos_long = 10.0F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 20.0F;
      float32_t dt = 0.0F;
      float32_t std = 1.0F;
      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

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

      det_id = 1;
      expected_pos_lat = 10.0F;
      expected_pos_long = 10.3F;
      vel_lat = 0.0F;
      vel_long = 20.0F;
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

      det_id = 2;
      expected_pos_lat = 10.0F;
      expected_pos_long = -10.3F;
      vel_lat = 0.0F;
      vel_long = 20.0F;
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

      det_id = 3;
      expected_pos_lat = -10.0F;
      expected_pos_long = -10.3F;
      vel_lat = 0.0F;
      vel_long = 20.0F;
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

   void Define_Test_Vector_3_Dets_RangeRate_Diff(
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   )
   {
      uint16_t det_id = 0U;
      float32_t expected_pos_lat = 0.0F;
      float32_t expected_pos_long = 10.0F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 20.0F;
      float32_t dt = 0.0F;
      float32_t std = 1.0F;
      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

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

      det_id = 1;
      expected_pos_lat = 0.0F;
      expected_pos_long = 10.3F;
      vel_lat = 0.0F;
      vel_long = 30.0F;
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

      det_id = 2;
      expected_pos_lat = 0.3F;
      expected_pos_long = 10.0F;
      vel_lat = 0.0F;
      vel_long = 40.0F;
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

   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.5F;
      clusters[clust_idx_1].vcs_position.y = 0.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first detection to cluster
      uint16_t det_idx = Get_Det_First_Free_Idx(raw_detections);
      clusters[clust_idx_1].detids[0] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 10.0F;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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

      // det hist creation
      // create and associate first hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      uint16_t det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[0] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      // create and associate second hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[1] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
   }

   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = -20.0F;
      clusters[clust_idx_1].vcs_position.y = -10.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first detection to cluster
      uint16_t det_idx = Get_Det_First_Free_Idx(raw_detections);
      clusters[clust_idx_1].detids[0] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 15.0F;
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
      raw_detections.detections[det_idx].raw.sensor_id = 3;

      // create and associate second detection to cluster
      det_idx = det_idx + 1U;
      clusters[clust_idx_1].detids[1] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 15.0F;
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
      raw_detections.detections[det_idx].raw.sensor_id = 3;

      // det hist creation and association
      // create and associate first hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      uint16_t det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[0] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      vel_lat = 0.0F;
      vel_long = 15.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      // create and associate second hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[1] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 15.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
   }

   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Unsuccesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.5F;
      clusters[clust_idx_1].vcs_position.y = 0.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first detection to cluster
      uint16_t det_idx = Get_Det_First_Free_Idx(raw_detections);
      clusters[clust_idx_1].detids[0] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 10.0F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 5.0F;
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
      det_idx = det_idx + 1;
      clusters[clust_idx_1].detids[1] = det_idx + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x + 5.0F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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

      // det hist creation and association
      // create and associate first hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      uint16_t det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[0] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x + 10.0F;
      vel_lat = 0.0F;
      vel_long = 15.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      // create and associate second hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      clusters[clust_idx_1].old_det_idx[1] = det_hist_id;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 20;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
   }

   void Define_Test_Vector_Cluster_With_3_Dets_and_3_Hist_Dets_1_Det_1_Hist_Det_with_azimuth_error_stat_mov_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.5F;
      clusters[clust_idx_1].vcs_position.y = 0.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first detection to cluster
      uint16_t det_id = Get_Det_First_Free_Idx(raw_detection_list);
      clusters[clust_idx_1].detids[0] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 10.0F;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.5;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      // det hist creation
      // create and associate first hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      uint16_t det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[0] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y + 0.5F;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.3F;
      std = 1.0F;
      
      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      // create and associate second hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[1] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.3F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );

      // create and associate third hist detection to cluster
      det_hist.n_occupied++;
      det_hist.max_occupation++;

      det_hist_id = det_hist.n_occupied - 1;
      clusters[clust_idx_1].old_det_idx[2] = det_hist_id;
      clusters[clust_idx_1].num_old_dets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.5F;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x + 0.5F;
      vel_lat = 0.0F;
      vel_long = 10.0F;
      dt = 0.4F;
      std = 1.0F;

      det_hist.det_data[det_hist_id] = Gen_Single_Hist_Det(
         motion_status,
         expected_pos_lat,
         expected_pos_long,
         vel_lat,
         vel_long,
         dt,
         std
      );
      det_hist.det_data[det_hist_id].f_azimuth_error_stat_mov = true;
   }

   void Define_Test_Vector_Cluster_With_4_Dets_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.5F;
      clusters[clust_idx_1].vcs_position.y = 0.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first detection to cluster
      uint16_t det_id = Get_Det_First_Free_Idx(raw_detection_list);
      clusters[clust_idx_1].detids[0] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 10.0F;
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
      raw_detection_list.detections[det_id] = dets_info.first;
      det_props[det_id] = dets_info.second;

      raw_detection_list.detections[det_id].processed.vcs_az = Get_Azimuth_From_Cart_Pos(det_props[det_id].vcs_position.y, det_props[det_id].vcs_position.x);
      raw_detection_list.detections[det_id].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[det_id].processed.vcs_az);
      raw_detection_list.detections[det_id].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[det_id].processed.vcs_az);

      // create and associate second detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[1] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      raw_detection_list.detections[det_id] = dets_info.first;
      det_props[det_id] = dets_info.second;

      raw_detection_list.detections[det_id].processed.vcs_az = Get_Azimuth_From_Cart_Pos(det_props[det_id].vcs_position.y, det_props[det_id].vcs_position.x);
      raw_detection_list.detections[det_id].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[det_id].processed.vcs_az);
      raw_detection_list.detections[det_id].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[det_id].processed.vcs_az);

      // create and associate third detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[2] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.4;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      raw_detection_list.detections[det_id] = dets_info.first;
      det_props[det_id] = dets_info.second;
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      raw_detection_list.detections[det_id].processed.vcs_az = Get_Azimuth_From_Cart_Pos(det_props[det_id].vcs_position.y, det_props[det_id].vcs_position.x);
      raw_detection_list.detections[det_id].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[det_id].processed.vcs_az);
      raw_detection_list.detections[det_id].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[det_id].processed.vcs_az);

      // create and associate fourth detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[3] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.5;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      raw_detection_list.detections[det_id] = dets_info.first;
      det_props[det_id] = dets_info.second;
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      raw_detection_list.detections[det_id].processed.vcs_az = Get_Azimuth_From_Cart_Pos(det_props[det_id].vcs_position.y, det_props[det_id].vcs_position.x);
      raw_detection_list.detections[det_id].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[det_id].processed.vcs_az);
      raw_detection_list.detections[det_id].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[det_id].processed.vcs_az);
   }

   void Define_Test_Vector_Cluster_With_3_Moving_Dets_4_Stationary_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.5F;
      clusters[clust_idx_1].vcs_position.y = 0.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_MOVING;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      // create and associate first moving detection to cluster
      uint16_t det_id = Get_Det_First_Free_Idx(raw_detection_list);
      clusters[clust_idx_1].detids[0] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
      float32_t expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      float32_t expected_pos_long = clusters[clust_idx_1].vcs_position.x - 0.5F;
      float32_t vel_lat = 0.0F;
      float32_t vel_long = 10.0F;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
      expected_pos_lat = clusters[clust_idx_1].vcs_position.y - 0.4;
      expected_pos_long = clusters[clust_idx_1].vcs_position.x;
      vel_lat = 0.0F;
      vel_long = 10.0F;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
      // create and associate fourth stationary detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[3] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      // create and associate fifth stationary detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[4] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      // create and associate sixth stationary detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[5] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;

      // create and associate seventh stationary detection to cluster
      det_id = det_id + 1U;
      clusters[clust_idx_1].detids[6] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      raw_detection_list.detections[det_id].processed.f_azimuth_error_stat_mov = true;
   }

   void Define_Test_Vector_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Host_Props_T &host_props
   )
   {
      host_props.cos_heading = 1.0F;

      tracker_info.num_active_clusters++;
      tracker_info.active_cluster_ids[tracker_info.num_active_clusters - 1] = tracker_info.num_active_clusters;

      uint16_t clust_idx_1 = tracker_info.num_active_clusters - 1;
      clusters[clust_idx_1].id = tracker_info.num_active_clusters;
      clusters[clust_idx_1].vcs_position.x = 10.0F;
      clusters[clust_idx_1].vcs_position.y = 10.0F;
      clusters[clust_idx_1].f_dealiased = true;
      clusters[clust_idx_1].num_types_of_dets[1] = 0;
      clusters[clust_idx_1].motion_status = F360_CLUSTER_MOTION_STATUS_STATIONARY;

      rspp_variant_A::RSPP_Detection_Motion_Status_T motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
      // create and associate first detection to cluster
      uint16_t det_id = Get_Det_First_Free_Idx(raw_detection_list);
      clusters[clust_idx_1].detids[0] = det_id + 1;
      clusters[clust_idx_1].ndets++;
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
      clusters[clust_idx_1].num_types_of_dets[0]++;
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
}
