/*===================================================================================*\
* FILE: f360_merge_two_clusters.cpp
* ====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
* -----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function to merge two clusters
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

/******************************
* Includes
*******************************/
#include <algorithm>

#include "f360_math.h"
#include "f360_iterator.h"
#include "f360_math_func.h"
#include "f360_merge_two_clusters.h"
#include "f360_get_unique_rdot_interval_ids.h"
#include "f360_is_two_look_type_ok_combine.h"
#include "f360_sorted_clusters_mgmt.h"

namespace f360_variant_A
{
   static float32_t Dealias_Range_Rates_In_A_Cluster(
      const F360_Cluster_T & cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T & det_hist,
      const float32_t dealiasing_interval_width,
      const float32_t dealiasing_interval,
      float32_t cluster_rdotcomp,
      int32_t n_rdot_ests);

   static bool Collect_Dealiased_Dets_In_Newer_Cluster(
      const F360_Tracker_Info_T& tracker_info,
      F360_Cluster_T& cluster_older,
      F360_Cluster_T& cluster_newer,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T& det_hist);

   static void Update_Cluster_Det_Types(
      F360_Cluster_T& cluster,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Hist_T & det_hist);

   static void Dealias_Detections_To_Target_Range_Rate(
      const F360_Cluster_T & cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T & det_hist,
      const float32_t target_range_rate,
      const float32_t k_max_dealiased_range_rate_diff);

   static inline void F360_Compensate_Det_Range(
      const float32_t dealiasing_interval,
      const float32_t r_wrapping,
      const rspp_variant_A::RSPP_Detection_T &detection,
      F360_Detection_Props_T &detection_prop);
      
   static inline void F360_Compensate_Detection_Hist_Range(
      const float32_t dealiasing_interval,
      F360_Detection_Hist_Data_T &det_data);

   static void Manage_Low_RCS_Dets_Counters(F360_Cluster_T& cluster_newer, const F360_Cluster_T& cluster_older);

   /*===========================================================================*\
    * FUNCTION: Merge_Two_Clusters()
    *===========================================================================
    * RETURN VALUE:
    * f_need_to_kill_id_older
    *
    * PARAMETERS:
    * const F360_Tracker_Info_T& tracker_info,
    * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
    * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
    * F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]
    * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
    * F360_Detection_Hist_T & det_hist
    * const int32_t cluster_id_1
    * const int32_t cluster_id_2
    * const float32_t rngrate_interval_width_1
    * const float32_t rngrate_interval_width_2
    * const float32_t interval_1
    * const float32_t interval_2
    * const float32_t k_max_dealiased_range_rate_diff
    *
    * EXTERNAL REFERENCES: mergeTwoUnconfTrks.m
    * None.
    *
    * DEVIATIONS FROM STANDARDS:
    * None.
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * This function merge two unconform tracks
    *
    * PRECONDITIONS:
    * None.
    *
    * POSTCONDITIONS:
    * None.
    *
    \*===========================================================================*/
   void Merge_Two_Clusters(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T & det_hist,
      const int16_t cluster_id_1,
      const int16_t cluster_id_2,
      const float32_t rngrate_interval_width_1,
      const float32_t rngrate_interval_width_2,
      const float32_t interval_1,
      const float32_t interval_2,
      const float32_t k_max_dealiased_range_rate_diff)
   {
      float32_t cluster_rdotcomp_older;
      float32_t cluster_rdotcomp_newer;

      float32_t rngrate_interval_width_older;
      float32_t rngrate_interval_width_newer;
      float32_t interval_older;
      float32_t interval_newer;

      int16_t cluster_newer_idx;
      int16_t cluster_older_idx;
      bool f_need_to_kill_id_older;


      if (clusters[cluster_id_1 - 1].time_since_cluster_updated > clusters[cluster_id_2 - 1].time_since_cluster_updated)
      {
         cluster_newer_idx = cluster_id_2 - 1;
         cluster_older_idx = cluster_id_1 - 1;
         interval_newer = interval_2;
         interval_older = interval_1;
         rngrate_interval_width_newer = rngrate_interval_width_2;
         rngrate_interval_width_older = rngrate_interval_width_1;
      }
      else
      {
         cluster_newer_idx = cluster_id_1 - 1;
         cluster_older_idx = cluster_id_2 - 1;
         interval_newer = interval_1;
         interval_older = interval_2;
         rngrate_interval_width_newer = rngrate_interval_width_1;
         rngrate_interval_width_older = rngrate_interval_width_2;
      }

      F360_Cluster_T& cluster_newer = clusters[cluster_newer_idx];
      F360_Cluster_T& cluster_older = clusters[cluster_older_idx];

      // Dealias detections with the specific range-rate interval width.
      cluster_rdotcomp_older = Dealias_Range_Rates_In_A_Cluster(cluster_older, sensors, raw_detection_list, detection_props, det_hist, rngrate_interval_width_older, interval_older, 0.0F, 0);
      cluster_rdotcomp_newer = Dealias_Range_Rates_In_A_Cluster(cluster_newer, sensors, raw_detection_list, detection_props, det_hist, rngrate_interval_width_newer, interval_newer, 0.0F, 0);

      cluster_newer.rep_rdotcomp = (cluster_rdotcomp_newer + cluster_rdotcomp_older) * 0.5F;

      // Attempt dealiasing of detections with different range-rate intervals
      Dealias_Detections_To_Target_Range_Rate(cluster_older, sensors, raw_detection_list, detection_props, det_hist, cluster_newer.rep_rdotcomp, k_max_dealiased_range_rate_diff);
      Dealias_Detections_To_Target_Range_Rate(cluster_newer, sensors, raw_detection_list, detection_props, det_hist, cluster_newer.rep_rdotcomp, k_max_dealiased_range_rate_diff);

      f_need_to_kill_id_older = Collect_Dealiased_Dets_In_Newer_Cluster(tracker_info, cluster_older, cluster_newer, detection_props, det_hist);

      Update_Cluster_Det_Types(cluster_older, raw_detection_list, det_hist);
      Update_Cluster_Det_Types(cluster_newer, raw_detection_list, det_hist);

      cluster_newer.exist_prob = F360_Accumulate_Probabilities(cluster_older.exist_prob, cluster_newer.exist_prob);

      cluster_newer.f_dealiased = true;
      cluster_older.f_dealiased = false;
      cluster_newer.time_since_created = (cluster_older.time_since_created > cluster_newer.time_since_created) ? cluster_older.time_since_created : cluster_newer.time_since_created;

      if (f_need_to_kill_id_older)
      {
         // kill the older track in caller function
         cluster_older.f_to_be_killed = true;
      }

      Manage_Low_RCS_Dets_Counters(cluster_newer, cluster_older);
      
      cluster_older.num_of_cluster_merges++;
      cluster_newer.num_of_cluster_merges++;
   }

   static float32_t Dealias_Range_Rates_In_A_Cluster(
      const F360_Cluster_T & cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T & det_hist,
      const float32_t dealiasing_interval_width,
      const float32_t dealiasing_interval,
      float32_t cluster_rdotcomp,
      int32_t n_rdot_ests)
   {

      if (!cluster.f_dealiased)
      {
         // correct range rate for look types confirmed to be dealiased
         for (int16_t i = 0; i < cluster.ndets; i++)
         {
            const int16_t det_idx = cluster.detids[i] - 1;
            const int32_t sens_idx = raw_detection_list.detections[det_idx].raw.sensor_id - 1;
            const F360_Det_Look_ID_T look_id = sensors[sens_idx].variable.look_id;
            const float32_t range_rate_interval_width = sensors[sens_idx].constant.v_wrapping[look_id];

            if (F360_EPSILON > std::abs(dealiasing_interval_width - range_rate_interval_width))
            {
               detection_props[det_idx].range_rate_compensated = detection_props[det_idx].range_rate_compensated + (dealiasing_interval_width * dealiasing_interval);
               detection_props[det_idx].range_rate_dealiased = detection_props[det_idx].range_rate_dealiased + (dealiasing_interval_width * dealiasing_interval);
               F360_Compensate_Det_Range(dealiasing_interval, 
                                         sensors[sens_idx].constant.r_wrapping[look_id], 
                                         raw_detection_list.detections[det_idx],
                                         detection_props[det_idx]);
               detection_props[det_idx].f_dealiased = true;
               cluster_rdotcomp += detection_props[det_idx].range_rate_compensated;
               n_rdot_ests += 1;
            }
         }

         for (int16_t i = 0; i < cluster.num_old_dets; i++)
         {
            const int16_t det_idx = cluster.old_det_idx[i];
            if (F360_EPSILON > std::abs(dealiasing_interval_width - det_hist.det_data[det_idx].v_wrapping))
            {
               det_hist.det_data[det_idx].rdot = det_hist.det_data[det_idx].rdot + (dealiasing_interval_width * dealiasing_interval);
               det_hist.det_data[det_idx].rdot_comp = det_hist.det_data[det_idx].rdot_comp + (dealiasing_interval_width * dealiasing_interval);

               F360_Compensate_Detection_Hist_Range(dealiasing_interval, det_hist.det_data[det_idx]);
               det_hist.det_data[det_idx].f_dealiased = true;
               cluster_rdotcomp += det_hist.det_data[det_idx].rdot_comp;
               n_rdot_ests += 1;
            }
         }
      }

      return ((n_rdot_ests > 0) ? (cluster_rdotcomp/ static_cast<float32_t>(n_rdot_ests)) : cluster.rep_rdotcomp);
   }

   /*===========================================================================*\
   * FUNCTION: Collect_Dealiased_Dets_In_Newer_Cluster()
   *===========================================================================
   * RETURN VALUE:
   * bool f_need_to_kill_id_older
   *
   * PARAMETERS:
   * F360_Cluster_T& cluster_older
   * F360_Cluster_T& cluster_newer
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Detection_Hist_T& det_hist
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
   * This function assigne dealiased detections from two clusters to newer claster
   * and assignes non dealiased detections to old cluster. 
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static bool Collect_Dealiased_Dets_In_Newer_Cluster(
      const F360_Tracker_Info_T& tracker_info,
      F360_Cluster_T& cluster_older,
      F360_Cluster_T& cluster_newer,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T& det_hist)
   {
      uint16_t cnt_detid_newer = 0U;
      uint16_t cnt_detid_older = 0U;
      uint16_t cnt_old_det_idx_newer = 0U;
      uint16_t cnt_old_det_idx_older = 0U;
      bool f_need_to_kill_id_older;

      int16_t detids_newer[MAX_DETS_IN_OBJ_TRK] = {};
      int16_t detids_older[MAX_DETS_IN_OBJ_TRK] = {};
      int16_t old_det_idx_newer[MAX_DETS_IN_OBJ_TRK] = {};
      int16_t old_det_idx_older[MAX_DETS_IN_OBJ_TRK] = {};

      // Determine where detections go from newer cluster
      for (int16_t i = 0; i < cluster_newer.ndets; i++)
      {
         const int16_t det_idx = cluster_newer.detids[i] - 1;
         if (detection_props[det_idx].f_dealiased)
         {
            detids_newer[cnt_detid_newer] = det_idx + 1;
            cnt_detid_newer++;
         }
         else
         {
            detection_props[det_idx].cluster_id = cluster_older.id;
            detids_older[cnt_detid_older] = det_idx + 1;
            cnt_detid_older++;
         }
      }
      for (int16_t i = 0; i < cluster_newer.num_old_dets; i++)
      {
         const int16_t det_idx = cluster_newer.old_det_idx[i];
         if (det_hist.det_data[det_idx].f_dealiased)
         {
            old_det_idx_newer[cnt_old_det_idx_newer] = det_idx;
            cnt_old_det_idx_newer++;
         }
         else
         {
            det_hist.det_data[det_idx].cluster_idx = cluster_older.id - 1;
            old_det_idx_older[cnt_old_det_idx_older] = det_idx;
            cnt_old_det_idx_older++;
         }
      }

      // Determine where detections go from older cluster
      for (int16_t i = 0; i < cluster_older.ndets; i++)
      {
         const int16_t det_idx = cluster_older.detids[i] - 1;
         if (detection_props[det_idx].f_dealiased && (tracker_info.variant.num_dets_in_track > cnt_detid_newer))
         {
            detection_props[det_idx].cluster_id = cluster_newer.id;
            detids_newer[cnt_detid_newer] = det_idx + 1;
            cnt_detid_newer++;
         }
         else if (tracker_info.variant.num_dets_in_track > cnt_detid_older)
         {
            detids_older[cnt_detid_older] = det_idx + 1;
            cnt_detid_older++;
         }
         else
         {
            // Drop this detection
            detection_props[det_idx].cluster_id = 0;
         }
      }
      for (int16_t i = 0; i < cluster_older.num_old_dets; i++)
      {
         const int16_t det_idx = cluster_older.old_det_idx[i];
         if (det_hist.det_data[det_idx].f_dealiased && (tracker_info.variant.num_dets_in_track > cnt_old_det_idx_newer))
         {
            det_hist.det_data[det_idx].cluster_idx = cluster_newer.id - 1;
            old_det_idx_newer[cnt_old_det_idx_newer] = det_idx;
            cnt_old_det_idx_newer++;
         }
         else if (tracker_info.variant.num_dets_in_track > cnt_old_det_idx_older)
         {
            old_det_idx_older[cnt_old_det_idx_older] = det_idx;
            cnt_old_det_idx_older++;
         }
         else
         {
            // Drop this detection
            det_hist.f_idx_occupied[det_idx] = false;
            det_hist.det_data[det_idx] = {};
         }
      }

      // Update clusters with new detection associations
      cluster_newer.ndets = static_cast<int16_t>(cnt_detid_newer);
      cluster_newer.num_old_dets = static_cast<int16_t>(cnt_old_det_idx_newer);
      cluster_older.ndets = static_cast<int16_t>(cnt_detid_older);
      cluster_older.num_old_dets = static_cast<int16_t>(cnt_old_det_idx_older);

      (void)std::copy(cmn::begin(detids_newer), cmn::end(detids_newer), cmn::begin(cluster_newer.detids));
      (void)std::copy(cmn::begin(detids_older), cmn::end(detids_older), cmn::begin(cluster_older.detids));
      (void)std::copy(cmn::begin(old_det_idx_newer), cmn::end(old_det_idx_newer), cmn::begin(cluster_newer.old_det_idx));
      (void)std::copy(cmn::begin(old_det_idx_older), cmn::end(old_det_idx_older), cmn::begin(cluster_older.old_det_idx));

      f_need_to_kill_id_older = (0 == (cluster_older.ndets + cluster_older.num_old_dets));
      return f_need_to_kill_id_older;
   }

   /*===========================================================================*\
   * FUNCTION: Dealias_Detections_To_Target_Range_Rate()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Cluster_T & cluster,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Detection_Hist_T & det_hist,
   * const float32_t target_range_rate,
   * const float32_t k_max_dealiased_range_rate_diff
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
   * This function check whether detection should be dealiased based 
   * on this detection range rate, target range rate and range rate interval width.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static void Dealias_Detections_To_Target_Range_Rate(
      const F360_Cluster_T & cluster,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T & det_hist,
      const float32_t target_range_rate,
      const float32_t k_max_dealiased_range_rate_diff)
   {

      float32_t rdot_diff;
      float32_t dealiased_rdot_diff;
      float32_t n_alias_intervals;

      // correct range rate for look types confirmed to be dealiased
      for (int16_t i = 0; i < cluster.ndets; i++)
      {
         const int16_t det_idx = cluster.detids[i] - 1;
         const int32_t sens_idx = raw_detection_list.detections[det_idx].raw.sensor_id - 1;
         const F360_Det_Look_ID_T look_id = sensors[sens_idx].variable.look_id;
         const float32_t range_rate_interval_width = sensors[sens_idx].constant.v_wrapping[look_id];

         if ((!detection_props[det_idx].f_dealiased) && (range_rate_interval_width > 0.0F))
         {
            rdot_diff = target_range_rate - detection_props[det_idx].range_rate_compensated;
            n_alias_intervals = F360_Roundf(rdot_diff / range_rate_interval_width);

            dealiased_rdot_diff = rdot_diff - n_alias_intervals * range_rate_interval_width;
            if (std::abs(dealiased_rdot_diff) < k_max_dealiased_range_rate_diff)
            {
               detection_props[det_idx].range_rate_compensated += n_alias_intervals * range_rate_interval_width;
               detection_props[det_idx].range_rate_dealiased += n_alias_intervals * range_rate_interval_width;
               F360_Compensate_Det_Range(n_alias_intervals, 
                                         sensors[sens_idx].constant.r_wrapping[look_id], 
                                         raw_detection_list.detections[det_idx],
                                         detection_props[det_idx]);
               detection_props[det_idx].f_dealiased = true;
            }
         }
      }

      for (int16_t i = 0; i < cluster.num_old_dets; i++)
      {
         const int16_t det_idx = cluster.old_det_idx[i];
         if ((!det_hist.det_data[det_idx].f_dealiased) && (det_hist.det_data[det_idx].v_wrapping > 0.0F))
         {
            rdot_diff = target_range_rate - det_hist.det_data[det_idx].rdot_comp;
            n_alias_intervals = F360_Roundf(rdot_diff / det_hist.det_data[det_idx].v_wrapping);

            dealiased_rdot_diff = rdot_diff - n_alias_intervals * det_hist.det_data[det_idx].v_wrapping;
            if (std::abs(dealiased_rdot_diff) < k_max_dealiased_range_rate_diff)
            {
               det_hist.det_data[det_idx].rdot_comp += n_alias_intervals * det_hist.det_data[det_idx].v_wrapping;
               det_hist.det_data[det_idx].rdot += n_alias_intervals * det_hist.det_data[det_idx].v_wrapping;
               F360_Compensate_Detection_Hist_Range(n_alias_intervals, det_hist.det_data[det_idx]);
               det_hist.det_data[det_idx].f_dealiased = true;
            }
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Update_Cluster_Det_Types()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * F360_Cluster_T& cluster
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * const F360_Detection_Hist_T & det_hist
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
   * This function updates detection motion status base on dealiased range rate.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static void Update_Cluster_Det_Types(
      F360_Cluster_T& cluster,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Hist_T & det_hist)
   {
      std::fill(cmn::begin(cluster.num_types_of_dets), cmn::end(cluster.num_types_of_dets), static_cast<int16_t>(0));

      for (int16_t i = 0; i < cluster.ndets; i++)
      {
         const int16_t det_idx = cluster.detids[i] - 1;
         if (raw_detect_list.detections[det_idx].processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING)
         {
            cluster.num_types_of_dets[0]++;
         }
         else
         {
            cluster.num_types_of_dets[1]++;
         }
      }
      for (int16_t i = 0; i < cluster.num_old_dets; i++)
      {
         const int16_t det_idx = cluster.old_det_idx[i];
         if (det_hist.det_data[det_idx].motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING)
         {
            cluster.num_types_of_dets[0]++;
         }
         else
         {
            cluster.num_types_of_dets[1]++;
         }
      }
   }

   static void Manage_Low_RCS_Dets_Counters(F360_Cluster_T& cluster_newer, const F360_Cluster_T& cluster_older)
   {
      /* The older cluster should not have any detections associated from the current time,
      and the newer one should have exactly one. The result is always stored in the newer cluster. */
      if ((cluster_newer.low_rcs_dets_cnt > 0U) && (cluster_older.low_rcs_dets_cnt > 0U)
         && (0 == cluster_older.ndets) && (1 == cluster_newer.ndets))
      {
         cluster_newer.low_rcs_dets_cnt += cluster_older.low_rcs_dets_cnt;
      }
      else
      {
         cluster_newer.low_rcs_dets_cnt = 0U;
      }
   }


   /*===========================================================================*\
   * FUNCTION: F360_Compensate_Det_Range()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   *  const float32_t dealiasing_interval,
   *  const float32_t r_wrapping,
   *  const rspp_variant_A::RSPP_Detection_T &detection,
   *  F360_Detection_Props_T &detection_prop
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
   * This function updates detection's range_dealiased and vcs position after
   * range rate dealiasing (SFW)
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static inline void F360_Compensate_Det_Range(
      const float32_t dealiasing_interval,
      const float32_t r_wrapping,
      const rspp_variant_A::RSPP_Detection_T &detection,
      F360_Detection_Props_T &detection_prop)
   {
      const float32_t delta_range = r_wrapping * dealiasing_interval;
      const Point cur_vcs_position = detection_prop.vcs_position;
      detection_prop.range_dealiased = detection.raw.range + delta_range;
      detection_prop.vcs_position.Set_Position(
         cur_vcs_position.x + delta_range * detection.processed.cos_vcs_az,
         cur_vcs_position.y + delta_range * detection.processed.sin_vcs_az);
   }

   /*===========================================================================*\
   * FUNCTION: F360_Compensate_Detection_Hist_Range()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const float32_t dealiasing_interval,
   * F360_Detection_Hist_Data_T &det_data
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
   * This function updates detection hist's vcs position after
   * range rate dealiasing (SFW)
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static inline void F360_Compensate_Detection_Hist_Range(
      const float32_t dealiasing_interval,
      F360_Detection_Hist_Data_T &det_data)
   {
      const float32_t delta_range = det_data.r_wrapping * dealiasing_interval;
      const Point cur_vcs_position = det_data.vcs_position;
      det_data.vcs_position.Set_Position(
         cur_vcs_position.x + delta_range * F360_Cosf(det_data.vcs_az),
         cur_vcs_position.y + delta_range * F360_Sinf(det_data.vcs_az));
   }
}
