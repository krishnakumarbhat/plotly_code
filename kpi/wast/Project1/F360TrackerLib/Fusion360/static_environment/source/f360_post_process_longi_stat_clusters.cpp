/*===========================================================================*\
* FILE: f360_post_process_longi_stat_clusters.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Post_Process_Longi_Stat_Clusters()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_post_process_longi_stat_clusters.h"
#include "f360_iterator.h"
#include "f360_math.h"
#include "f360_math_func.h"
#include <algorithm>


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Post_Process_Longi_Stat_Clusters()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   *   const F360_Calibrations_T& calibs,
   *   uint16_t&nr_valid_clusters,
   *   F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS]
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
   * Performs post processing on clusters of longitudinal sorted objects.
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Post_Process_Longi_Stat_Clusters(
      const F360_Calibrations_T& calibs,
      uint16_t&nr_valid_clusters,
      F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS])
   {
      Merge_Longi_Stat_Clusters(calibs, nr_valid_clusters, valid_clusters);
   }

   /*===========================================================================*\
   * FUNCTION: Merge_Longi_Stat_Clusters()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   *   const F360_Calibrations_T& calibs,
   *   uint16_t&nr_valid_clusters,
   *   F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS]
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
   * This function iterates over clusters and attempts to 
   * merge clusters. Function is done when no more merges was done by
   * Try_To_Merge_Longi_Stat_Clusters() function.
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Merge_Longi_Stat_Clusters(
      const F360_Calibrations_T& calibs,
      uint16_t& nr_valid_clusters,
      F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS])
   {

      uint16_t nr_next_clusters = nr_valid_clusters;
      F360_Longi_Stat_Cluster_T next_clusters[NR_LONGI_STAT_CLUSTERS] = {};
      (void)std::copy(&valid_clusters[0], &valid_clusters[nr_next_clusters], cmn::begin(next_clusters));

      // Find max number of comparisons that can be done using an arithmetic sum formula
      const uint16_t max_iterations = (nr_next_clusters * (nr_next_clusters - 1U)) / 2U;

      uint16_t next_idx = 1U;
      F360_Longi_Stat_Cluster_T clusters[NR_LONGI_STAT_CLUSTERS] = {};
      for (uint16_t iter = 0U; iter < max_iterations; iter++)
      {
         const uint16_t nr_clusters = nr_next_clusters;
         (void)std::copy(&next_clusters[0], &next_clusters[nr_clusters], cmn::begin(clusters));

         const bool f_merge_occurred = Try_To_Merge_Longi_Stat_Clusters(nr_clusters, clusters, calibs, next_idx, nr_next_clusters, next_clusters);

         if (!f_merge_occurred)
         {
            //If no merge occurred, the merge logic is done
            break;
         }
      }

      // Update cluster array after merge logic have completed
      nr_valid_clusters = nr_next_clusters;
      (void)std::copy(&next_clusters[0], &next_clusters[nr_valid_clusters], cmn::begin(valid_clusters));
   }

   /*===========================================================================*\
   * FUNCTION: Try_To_Merge_Longi_Stat_Clusters()
   * ===========================================================================
   * RETURN VALUE:
   * bool f_merged_occurred
   *
   * PARAMETERS:
   *   const uint16_t nr_clusters,
   *   const F360_Longi_Stat_Cluster_T(&clusters)[NR_LONGI_STAT_CLUSTERS],
   *   const F360_Calibrations_T& calibs,
   *   uint16_t& next_idx,
   *   uint16_t& nr_next_clusters,
   *   F360_Longi_Stat_Cluster_T(&next_clusters)[NR_LONGI_STAT_CLUSTERS]
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
   * Returns true if two clusters was merged. If clusters are merged the function 
   * also arranges the updated cluster information in "next clusters" that should
   * be used in next iteration. Idea is that this function should be called until 
   * it returns false. Meaning that algo has run through all clusters and no more
   * merges was done.
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Try_To_Merge_Longi_Stat_Clusters(
      const uint16_t nr_clusters,
      const F360_Longi_Stat_Cluster_T(&clusters)[NR_LONGI_STAT_CLUSTERS],
      const F360_Calibrations_T& calibs,
      uint16_t& next_idx,
      uint16_t& nr_next_clusters,
      F360_Longi_Stat_Cluster_T(&next_clusters)[NR_LONGI_STAT_CLUSTERS])
   {
      bool f_merge_occurred = false;

      for (uint16_t i = 0U; i < (nr_clusters - 1U); i++)
      {
         // This cluster will always have a lower x_min value than cluster k since cluster have been created in ascending longitudinal VCS order
         const F360_Longi_Stat_Cluster_T cluster_i = clusters[i];

         for (uint16_t k = next_idx; k < nr_clusters; k++)
         {
            const F360_Longi_Stat_Cluster_T cluster_k = clusters[k];

            if ((std::abs(cluster_i.lat_mean - cluster_k.lat_mean) < calibs.k_lsc_lat_merging_gate) &&
               (std::abs(Get_Cluster_Max_Long_Pos(cluster_i) - Get_Cluster_Min_Long_Pos(cluster_k)) < calibs.k_lsc_long_merging_gate))
            {
               // Clusters have roughly the same start/end position, do detailed check on the mean lateral position
               // of the objects in each cluster close to the end points
               const F360_Object_Track_T* obj_i = cluster_i.last_object;
               const F360_Object_Track_T* obj_k = cluster_k.first_object;

               float32_t cluster_i_lat_pos_array[NUMBER_OF_OBJECT_TRACKS] = {};
               float32_t cluster_k_lat_pos_array[NUMBER_OF_OBJECT_TRACKS] = {};
               for (uint32_t cluster_point = 0U; cluster_point < calibs.k_lsc_min_points_in_cluster; cluster_point++)
               {
                  cluster_i_lat_pos_array[cluster_point] = obj_i->bbox.Get_Center().y;
                  cluster_k_lat_pos_array[cluster_point] = obj_k->bbox.Get_Center().y;

                  obj_i = obj_i->lsc_prev_in_cluster;
                  obj_k = obj_k->lsc_next_in_cluster;
               }
               const float32_t mean_cluster_i = F360_Mean(cluster_i_lat_pos_array, calibs.k_lsc_min_points_in_cluster);
               const float32_t mean_cluster_k = F360_Mean(cluster_k_lat_pos_array, calibs.k_lsc_min_points_in_cluster);

               if (std::abs(mean_cluster_i - mean_cluster_k) < calibs.k_lsc_cluster_merge_thr)
               {
                  f_merge_occurred = true;
                  next_idx = k;
                  // Merge clusters
                  Merge_Longi_Stat_Cluster_Pair(nr_clusters, clusters, i, k, nr_next_clusters, next_clusters);

                  // We need another merge try iteration, break inner loop
                  break;
               }
            }
         }

         if (f_merge_occurred)
         {
            // Inner loop have merged two clusters, we need to run a new merge try iteration
            break;
         }
         else
         {
            // Inner loop did not merge two clusters, keep checking by increasing outer loop
            // and resetting inner loop counter
            next_idx = (i + 2U);
         }
      }

      return f_merge_occurred;

   }

   /*===========================================================================*\
   * FUNCTION: Merge_Longi_Stat_Cluster_Pair()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *   const uint16_t nr_clusters
   *   const F360_Longi_Stat_Cluster_T(&clusters)[NR_LONGI_STAT_CLUSTERS]
   *   const uint16_t primary_cluster_idx
   *   const uint16_t secondary_cluster_idx
   *   uint16_t &nr_next_clusters
   *   F360_Longi_Stat_Cluster_T(&next_clusters)[NR_LONGI_STAT_CLUSTERS]
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
   * This function merges two clusters. The primary cluster will inherit all objects
   * of secondary cluster. Primary cluster properties are updated and secondary
   * cluster is removed from the array of interesting clusters for next iteration.
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Merge_Longi_Stat_Cluster_Pair(
      const uint16_t nr_clusters,
      const F360_Longi_Stat_Cluster_T(&clusters)[NR_LONGI_STAT_CLUSTERS],
      const uint16_t primary_cluster_idx,
      const uint16_t secondary_cluster_idx,
      uint16_t&nr_next_clusters,
      F360_Longi_Stat_Cluster_T(&next_clusters)[NR_LONGI_STAT_CLUSTERS])
   {

      // Stitch together the clustered objects from each cluster
      F360_Object_Track_T* const object_primary = clusters[primary_cluster_idx].last_object;
      F360_Object_Track_T* const object_secondary = clusters[secondary_cluster_idx].first_object;
      object_primary->lsc_next_in_cluster = object_secondary;
      object_secondary->lsc_prev_in_cluster = object_primary;
      next_clusters[primary_cluster_idx].nr_objects += clusters[secondary_cluster_idx].nr_objects;
      next_clusters[primary_cluster_idx].first_object = clusters[primary_cluster_idx].first_object;
      next_clusters[primary_cluster_idx].last_object = clusters[secondary_cluster_idx].last_object;

      // Update lateral mean
      const float32_t nr_obj_primary = static_cast<float32_t>(clusters[primary_cluster_idx].nr_objects);
      const float32_t nr_obj_secondary = static_cast<float32_t>(clusters[secondary_cluster_idx].nr_objects);
      
      next_clusters[primary_cluster_idx].lat_mean = ((nr_obj_primary * clusters[primary_cluster_idx].lat_mean) +
         (nr_obj_secondary * clusters[secondary_cluster_idx].lat_mean)) /
         (nr_obj_primary + nr_obj_secondary);

      // Remove secondary cluster
      for (uint16_t i = secondary_cluster_idx; i < (nr_clusters - 1U); i++)
      {
         next_clusters[i] = clusters[i + 1U];
      }
      nr_next_clusters--;

   }
}

