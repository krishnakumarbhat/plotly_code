/*===================================================================================*\
* FILE:  f360_populate_internal_clusters_log.cpp
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*------------------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*/

#include <cstring>
#include <algorithm>
#include "f360_iterator.h"
#include "f360_populate_internal_clusters_log.h"

namespace f360_variant_A
{
   void Populate_Internal_Clusters_Data(F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Tracker_Info_T& tracker_info, const F360_Internal_Cluster_T(&cluster_log)[NUMBER_OF_CLUSTERS])
   {
      // Clear ids
      std::fill(cmn::begin(tracker_info.active_cluster_ids), cmn::end(tracker_info.active_cluster_ids), static_cast<int16_t>(0));
      std::fill(cmn::begin(tracker_info.inactive_cluster_ids), cmn::end(tracker_info.inactive_cluster_ids), static_cast<int16_t>(0));

      tracker_info.num_active_clusters = 0;
      for (uint16_t i = 0U; i < NUMBER_OF_CLUSTERS; i++)
      {
         if ((0.0F <= cluster_log[i].time_since_created) && (0 < cluster_log[i].id))
         {
            const int16_t cluster_idx = cluster_log[i].id - 1;

            clusters[cluster_idx].vcs_position.y = cluster_log[i].vcs_lat_posn;
            clusters[cluster_idx].vcs_position.x = cluster_log[i].vcs_long_posn;
            clusters[cluster_idx].rep_vcs_az = cluster_log[i].rep_vcs_az;
            clusters[cluster_idx].rep_rdotcomp = cluster_log[i].rep_rdotcomp;
            clusters[cluster_idx].exist_prob = cluster_log[i].exist_prob;
            clusters[cluster_idx].time_since_created = cluster_log[i].time_since_created;
            clusters[cluster_idx].time_since_cluster_updated = cluster_log[i].time_since_cluster_updated;
            clusters[cluster_idx].time_since_measurement = cluster_log[i].time_since_measurement;
            clusters[cluster_idx].priority = cluster_log[i].priority;
            clusters[cluster_idx].id = cluster_log[i].id;
            clusters[cluster_idx].motion_status = static_cast<F360_Cluster_Motion_Status_T>(cluster_log[i].motion_status);
            clusters[cluster_idx].num_types_of_dets[0] = cluster_log[i].num_types_of_dets[0];
            clusters[cluster_idx].num_types_of_dets[1] = cluster_log[i].num_types_of_dets[1];
            clusters[cluster_idx].num_of_cluster_merges = cluster_log[i].num_of_cluster_merges;
            clusters[cluster_idx].f_dealiased = (1U == cluster_log[i].f_dealiased);
            clusters[cluster_idx].f_to_be_killed = (1U == cluster_log[i].f_to_be_killed);
            clusters[cluster_idx].low_rcs_dets_cnt = cluster_log[i].low_rcs_dets_cnt;

            clusters[cluster_idx].ndets = 0;
            clusters[cluster_idx].num_old_dets = 0;
            clusters[cluster_idx].cos_vcs_az = F360_Cosf(clusters[cluster_idx].rep_vcs_az);
            clusters[cluster_idx].sin_vcs_az = F360_Sinf(clusters[cluster_idx].rep_vcs_az);

            tracker_info.active_cluster_ids[tracker_info.num_active_clusters] = cluster_idx + 1;
            tracker_info.num_active_clusters++;
         }
      }

      int16_t n_inactive_clusters = 0;
      for (int16_t i = 0; i < static_cast<int16_t>(NUMBER_OF_CLUSTERS); i++)
      {
         if (0.0F > clusters[i].time_since_created)
         {
            tracker_info.inactive_cluster_ids[n_inactive_clusters] = i + 1;
            n_inactive_clusters++;
         }
      }
   }

   void Populate_Internal_Clusters_Log_Data(F360_Internal_Cluster_T(&cluster_log)[NUMBER_OF_CLUSTERS],
      const F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS], const int16_t num_active_clusters,
      const int16_t(&active_cluster_ids)[NUMBER_OF_CLUSTERS])
   {
      for (int16_t i = 0; i < num_active_clusters; i++)
      {
         const int16_t cluster_idx = active_cluster_ids[i] - 1;

         cluster_log[i].vcs_lat_posn = clusters[cluster_idx].vcs_position.y;
         cluster_log[i].vcs_long_posn = clusters[cluster_idx].vcs_position.x;
         cluster_log[i].rep_vcs_az = clusters[cluster_idx].rep_vcs_az;
         cluster_log[i].rep_rdotcomp = clusters[cluster_idx].rep_rdotcomp;
         cluster_log[i].exist_prob = clusters[cluster_idx].exist_prob;
         cluster_log[i].time_since_created = clusters[cluster_idx].time_since_created;
         cluster_log[i].time_since_cluster_updated = clusters[cluster_idx].time_since_cluster_updated;
         cluster_log[i].time_since_measurement = clusters[cluster_idx].time_since_measurement;
         cluster_log[i].priority = clusters[cluster_idx].priority;
         cluster_log[i].id = clusters[cluster_idx].id;
         cluster_log[i].motion_status = static_cast<int8_t>(clusters[cluster_idx].motion_status);
         cluster_log[i].num_types_of_dets[0] = clusters[cluster_idx].num_types_of_dets[0];
         cluster_log[i].num_types_of_dets[1] = clusters[cluster_idx].num_types_of_dets[1];
         cluster_log[i].num_of_cluster_merges = clusters[cluster_idx].num_of_cluster_merges;
         cluster_log[i].f_dealiased = clusters[cluster_idx].f_dealiased ? 1U : 0U;
         cluster_log[i].f_to_be_killed = clusters[cluster_idx].f_to_be_killed ? 1U : 0U;
         cluster_log[i].low_rcs_dets_cnt = clusters[cluster_idx].low_rcs_dets_cnt;
      }
   }
}
