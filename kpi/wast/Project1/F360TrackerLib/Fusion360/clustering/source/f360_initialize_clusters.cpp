/*===================================================================================*\
* FILE: f360_initialize_clusters.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* The file contains the definition of Initialize_Clusters function
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

/*===========================================================================*\
* Includes
\*===========================================================================*/
#include "f360_initialize_clusters.h"

#include "f360_math.h"

#include "f360_clear_cluster.h"
#include "f360_calculate_priority.h"
#include "f360_dbscan.h"
#include "f360_get_new_cluster_id.h"
#include "f360_get_wall_time.h"
#include "f360_kill_cluster.h"
#include "f360_math_func.h"
#include "rspp_detection_list.h"
#include "f360_norm_heading_angle.h"
#include "f360_sorted_clusters_mgmt.h"
#include "f360_sort_priority.h"
#include "f360_math_func.h"
#include "f360_host_props.h"
#include "f360_uncertainty_propagation.h"
#include <algorithm>

namespace f360_variant_A
{
   /******************************
   * File scope functions declarations
   *******************************/
   static void Update_Cluster_Timestamp(
      F360_Cluster_T& cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list);

   static void Rewrite_Data_From_Preinit_Cluster_To_Cluster(
      const F360_Cluster_T &preinit_cluster,
      F360_Cluster_T &cluster);

   static void Init_Cluster(
      const F360_Cluster_T &preinit_cluster,
      const uint16_t first_det_idx_in_cluster,
      const uint16_t(&array_of_det_idxs_in_clusters)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T &cluster);

   /*===========================================================================*\
   * FUNCTION: Initialize_Clusters()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * const F360_Calibrations_T& calibrations,
   * F360_Local_Clusters_T &local_clusters_data,
   * F360_Tracker_Info_T &tracker_info,
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]
   *
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
   *
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Initialize_Clusters(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Calibrations_T& calibrations,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Local_Clusters_T &local_clusters_data,
      F360_Tracker_Info_T &tracker_info,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS])
   {
      uint16_t first_det_idx_in_cluster = 0U;  // This is pointer to the index of first detection of given local cluster
      float32_t cluster_vcs_long[MAX_TRACKER_POSN_CLUSTERS]; // each clustering function can initialize MAX_TRACKER_POSN_CLUSTERS clusters
      uint16_t num_initialized_local_clusters = 0U;
      int16_t new_clusters_ids[MAX_TRACKER_POSN_CLUSTERS] = {};

      for (uint16_t local_cluster_index = 0U; local_cluster_index < local_clusters_data.num_clusters; local_cluster_index++)
      {
         const uint16_t number_of_dets_in_local_cluster = local_clusters_data.num_dets_in_clusters[local_cluster_index];
         const uint16_t max_dets_in_obj_track = static_cast<uint16_t>(tracker_info.variant.num_dets_in_track);

         F360_Cluster_T new_cluster;
         Preinitialize_Cluster(first_det_idx_in_cluster, local_clusters_data.array_of_det_idxs_in_clusters, raw_detection_list, calibrations,
            max_dets_in_obj_track, detection_props, number_of_dets_in_local_cluster, new_cluster);

         int16_t new_global_cluster_id = 0;
         Get_New_Cluster_ID(new_global_cluster_id, tracker_info);

         if (0 < new_global_cluster_id)
         {
            const uint16_t new_global_cluster_index = static_cast<uint16_t>(new_global_cluster_id) - 1U;

            Init_Cluster(new_cluster, first_det_idx_in_cluster, local_clusters_data.array_of_det_idxs_in_clusters,
               raw_detection_list, sensors, detection_props, clusters[new_global_cluster_index]);

            Compute_Detection_Position_Covariance_Matrix(clusters[new_global_cluster_index], calibrations, raw_detection_list, detection_props);

            cluster_vcs_long[num_initialized_local_clusters] = clusters[new_global_cluster_index].vcs_position.x;
            new_clusters_ids[num_initialized_local_clusters] = new_global_cluster_id;
            num_initialized_local_clusters++;
         }

         first_det_idx_in_cluster += local_clusters_data.num_dets_in_clusters[local_cluster_index];
      }

      if (num_initialized_local_clusters > 0U)
      {
         Sorted_Clusters_Insert_Batch(tracker_info, clusters, cluster_vcs_long, new_clusters_ids, num_initialized_local_clusters);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Preinitialize_Cluster()
   *===========================================================================
   * RETURN VALUE:
   * cluster priority in range <0.0, 100.0>
   *
   * PARAMETERS:
   * const uint32_t first_det_idx_in_cluster,
   * const uint32_t (&array_of_det_idxs_in_clusters)[MAX_NUMBER_OF_DETECTIONS],
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * const F360_Calibrations_T& calibrations,
   * F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * uint32_t number_of_dets_in_cluster,
   * F360_Cluster_T &cluster
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
   *
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   * number_of_dets_in_cluster > 0
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Preinitialize_Cluster(
      const uint16_t first_det_idx_in_cluster,
      const uint16_t (&array_of_det_idxs_in_clusters)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Calibrations_T& calibrations,
      const uint16_t max_dets_in_obj_track,
      F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
      uint16_t number_of_dets_in_cluster,
      F360_Cluster_T &cluster)
   {
      float32_t n_dets_inv;
      int16_t num_moving_dets = 0;

      // consider just up to MAX_DETS_IN_OBJ_TRK dets
      number_of_dets_in_cluster = std::min(number_of_dets_in_cluster, max_dets_in_obj_track);

      if (number_of_dets_in_cluster > 0U)
      {
         n_dets_inv = 1.0F / static_cast<float32_t>(number_of_dets_in_cluster);
      }
      else
      {
         n_dets_inv = 1.0F;
      }

      Clear_Cluster(cluster);

      for (uint16_t det_iter = 0U; det_iter < number_of_dets_in_cluster; det_iter++) // def offset -> look how to make it more clear
      {
         const uint16_t det_index = array_of_det_idxs_in_clusters[first_det_idx_in_cluster + det_iter];
         cluster.rep_rdotcomp += raw_detection_list.detections[det_index].raw.range_rate;
         cluster.vcs_position.x += det_p[det_index].vcs_position.x;
         cluster.vcs_position.y += det_p[det_index].vcs_position.y;

         if (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == raw_detection_list.detections[det_index].processed.motion_status)
         {
            num_moving_dets++;
         }
      }
      cluster.ndets = static_cast<int16_t>(number_of_dets_in_cluster);
      cluster.vcs_position.x *= n_dets_inv;
      cluster.vcs_position.y *= n_dets_inv;
      cluster.rep_rdotcomp *= n_dets_inv;
      cluster.num_types_of_dets[0] = num_moving_dets;
      cluster.num_types_of_dets[1] = cluster.ndets - num_moving_dets;
      cluster.priority = 0.0F;

      if (number_of_dets_in_cluster > 0U)
      {
         // If the cluster only has one detection, and a number of conditions are fulfilled, initialize the counter
         const rspp_variant_A::RSPP_Detection_T &detection = raw_detection_list.detections[array_of_det_idxs_in_clusters[first_det_idx_in_cluster]];
         if ((1 == cluster.ndets) && (detection.raw.range < calibrations.k_ocb_max_range) && (std::abs(detection.raw.range_rate) < calibrations.k_ocb_max_range_rate))
         {
            // Depending on the RCS, two different values are used
            if (detection.raw.rcs < calibrations.k_ocb_rcs_thresh_low_rcs)
            {
               cluster.low_rcs_dets_cnt = calibrations.k_ocb_cnt_delta_low_rcs_or_mult_dets;
            }
            else if (detection.raw.rcs < calibrations.k_ocb_rcs_thresh_midlow_rcs)
            {
               cluster.low_rcs_dets_cnt = calibrations.k_ocb_cnt_delta_midlow_rcs;
            }
            else
            {
               // Do nothing
            }
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Rewrite_Data_From_Preinit_Cluster_To_Cluster()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Cluster_T &preinit_cluster,
   * F360_Cluster_T &cluster
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
   * This function rewrites data from preinitialized cluster to the new cluster
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Rewrite_Data_From_Preinit_Cluster_To_Cluster(
      const F360_Cluster_T &preinit_cluster,
      F360_Cluster_T &cluster)
   {
      cluster.ndets = preinit_cluster.ndets;
      cluster.vcs_position.x = preinit_cluster.vcs_position.x;
      cluster.vcs_position.y = preinit_cluster.vcs_position.y;
      cluster.rep_rdotcomp = preinit_cluster.rep_rdotcomp;
      cluster.priority = preinit_cluster.priority;
      cluster.num_types_of_dets[0] = preinit_cluster.num_types_of_dets[0];
      cluster.num_types_of_dets[1] = preinit_cluster.num_types_of_dets[1];
      cluster.low_rcs_dets_cnt = preinit_cluster.low_rcs_dets_cnt;
   }


   /*===========================================================================*\
   * FUNCTION: Init_Cluster()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Cluster_T &preinit_cluster,
   * const uint32_t first_det_idx_in_cluster,
   * const uint32_t (&array_of_det_idxs_in_clusters)[MAX_NUMBER_OF_DETECTIONS],
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T &cluster
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
   *
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Init_Cluster(
      const F360_Cluster_T &preinit_cluster,
      const uint16_t first_det_idx_in_cluster,
      const uint16_t (&array_of_det_idxs_in_clusters)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T &cluster)
   {
      bool f_dealiased = false;
      float32_t sum_vcs_az = 0.0F;
      float32_t sum_dets_exist_prob = 0.0F;
      float32_t ndet_inv;
      float32_t cluster_exist_prob;

      Clear_Cluster(cluster);
      Rewrite_Data_From_Preinit_Cluster_To_Cluster(preinit_cluster, cluster);

      // Extra validation for number of detections.
      assert((static_cast<uint32_t>(cluster.ndets) <= MAX_DETS_IN_OBJ_TRK) && (static_cast<uint32_t>(cluster.ndets) > 0U));

      for (uint16_t det_iter = 0U; det_iter < static_cast<uint16_t>(cluster.ndets); det_iter++)
      {
         const uint16_t det_index = array_of_det_idxs_in_clusters[det_iter + first_det_idx_in_cluster];

         det_p[det_index].cluster_id = cluster.id;
         cluster.detids[det_iter] = static_cast<int16_t>(det_index) + 1;

         f_dealiased = f_dealiased || det_p[det_index].f_dealiased;
         const float32_t reference_vcs_az = raw_detection_list.detections[array_of_det_idxs_in_clusters[first_det_idx_in_cluster]].processed.vcs_az;
         sum_vcs_az += Normalize_Heading_Angle(raw_detection_list.detections[det_index].processed.vcs_az, reference_vcs_az);
         sum_dets_exist_prob += det_p[det_index].probability_of_detection;
      }
      ndet_inv = 1.0F / static_cast<float32_t>(cluster.ndets);
      cluster.rep_vcs_az = Normalize_Heading_Angle(sum_vcs_az * ndet_inv, 0.0F);
      cluster.cos_vcs_az = F360_Cosf(cluster.rep_vcs_az);
      cluster.sin_vcs_az = F360_Sinf(cluster.rep_vcs_az);

      cluster.time_since_created = 0.0F;
      cluster.time_since_cluster_updated = 0.0F;
      Update_Cluster_Timestamp(cluster, sensors, raw_detection_list);

      cluster_exist_prob = sum_dets_exist_prob * ndet_inv;
      cluster_exist_prob = F360_Power_Scale_Prob(cluster_exist_prob, static_cast<uint32_t>(cluster.ndets));
      cluster.exist_prob = cluster_exist_prob;

      cluster.f_dealiased = f_dealiased;
      cluster.num_of_cluster_merges = 0U;
      cluster.motion_status = F360_CLUSTER_MOTION_STATUS_AMBIGUOUS;
   }

   /*===========================================================================*\
   * FUNCTION: Update_Cluster_Timestamp()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Cluster_T & const cluster,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list
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
   * This function updates cluster time since measurement timestamp.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Cluster_Timestamp(
      F360_Cluster_T& cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list)
   {
      cluster.time_since_measurement = 0.0F;
      for (int16_t j = 0; j < cluster.ndets; j++)
      {
         const int16_t det_idx = cluster.detids[j] - 1;
         const int32_t sensor_idx = raw_detect_list.detections[det_idx].raw.sensor_id - 1;
         const float time_since_measurement = sensors[sensor_idx].variable.time_since_measurement_s;

         if (0 == j)
         {
            cluster.time_since_measurement = time_since_measurement;
         }
         else if (time_since_measurement < cluster.time_since_measurement)
         {
            cluster.time_since_measurement = time_since_measurement;
         }
         else
         {
            //Do nothing.
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Compute_Detection_Position_Covariance_Matrix
   *===========================================================================*/
   void Compute_Detection_Position_Covariance_Matrix(
      const F360_Cluster_T& cluster_to_init,
      const F360_Calibrations_T& calib,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      for (int16_t i = 0; i < cluster_to_init.ndets; i++)
      {
         const int16_t det_idx = cluster_to_init.detids[i] - 1;
         const rspp_variant_A::RSPP_Detection_T& detection = raw_detection_list.detections[det_idx];
         F360_Detection_Props_T& detection_prop = detection_props[det_idx];
         float32_t polar_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];

         // https://jiraprod.aptiv.com/browse/DFT-1970
         // TODO: Different sensors has different accuracy. Maybe even every single detection can has assigned range and azimuth std.
         // TODO: In case of elevation signal available 3d uncertainty propagation would be needed.
         // TODO: Consider unbiased transformation.
         polar_cov[F360_2D_IDX_X][F360_2D_IDX_X] = calib.k_default_detection_range_std * calib.k_default_detection_range_std;
         polar_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.0F;
         polar_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = 0.0F;
         polar_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = calib.k_default_detection_azimuth_std * calib.k_default_detection_azimuth_std;

         float32_t jacobian_matrix[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
         const float32_t sin_azimuth = detection.processed.sin_vcs_az;
         const float32_t cos_azimuth = detection.processed.cos_vcs_az;
         jacobian_matrix[F360_2D_IDX_X][F360_2D_IDX_X] = cos_azimuth;
         jacobian_matrix[F360_2D_IDX_X][F360_2D_IDX_Y] = -detection.raw.range * sin_azimuth;
         jacobian_matrix[F360_2D_IDX_Y][F360_2D_IDX_X] = sin_azimuth;
         jacobian_matrix[F360_2D_IDX_Y][F360_2D_IDX_Y] = detection.raw.range * cos_azimuth;

         Uncertainty_Propagation_2d(jacobian_matrix, polar_cov, detection_prop.position_cov_nees);
      }
   }
}
