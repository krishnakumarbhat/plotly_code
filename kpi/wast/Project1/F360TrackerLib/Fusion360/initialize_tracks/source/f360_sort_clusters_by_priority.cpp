/*===========================================================================*\
* FILE: f360_sort_clusters_by_priority.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function definition for Sort_Clusters_By_Priority.
*
*
*   Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_initialization_preconditions.h"
#include "f360_nees_cfmi_preconditions.h"
#include "f360_calculate_priority.h"
#include "f360_tracker_info.h"
#include "f360_math_func.h"
#include "f360_sort_clusters_by_priority.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Sort_Clusters_By_Priority
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Detection_Hist_T& det_hist,
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Host_T& host,
   * const F360_Tracker_Info_T& tracker_info,
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
   * uint32_t (&sorted_clusters_id_by_prior)[NUMBER_OF_CLUSTERS],
   * uint32_t& num_predond_valid_clusters
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function which is sorting clusters by priority
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Sort_Clusters_By_Priority(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Hist_T& det_hist,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Host_T& host,
      const F360_Tracker_Info_T& tracker_info,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      int32_t (&sorted_clusters_id_by_prior)[NUMBER_OF_CLUSTERS],
      uint32_t& num_predond_valid_clusters)
   {
      float32_t precond_valid_cluster_prior[NUMBER_OF_CLUSTERS] = {};
      uint32_t sorted_idx[NUMBER_OF_CLUSTERS] = {};
      int32_t predond_valid_clusters_id[NUMBER_OF_CLUSTERS] = {};

      for (int32_t i = 0; i < tracker_info.num_active_clusters; i++)
      {
         F360_Cluster_T& p_current_cluster = clusters[tracker_info.active_cluster_ids[i] - 1];

         if (Initialization_Preconditions(calibrations, p_current_cluster, sensors, det_props, raw_detection_list, det_hist, tracker_info))
         {
            sorted_idx[num_predond_valid_clusters] = num_predond_valid_clusters;
            predond_valid_clusters_id[num_predond_valid_clusters] = p_current_cluster.id;

            p_current_cluster.priority = Calculate_Priority(calibrations, host, calibrations.k_priority_default_cluster_movable, 
               calibrations.k_priority_default_cluster_confidence, p_current_cluster.vcs_position.x, p_current_cluster.vcs_position.y);

            precond_valid_cluster_prior[num_predond_valid_clusters] = p_current_cluster.priority;
            num_predond_valid_clusters++;
         }
         else
         {
            p_current_cluster.priority = 0.0F;
         }
      }
      if ((num_predond_valid_clusters > 1U) && (static_cast<uint32_t>(tracker_info.num_active_objs) + num_predond_valid_clusters >= tracker_info.variant.num_tracks))
      {
         (void)F360_Sort(precond_valid_cluster_prior, num_predond_valid_clusters, false, sorted_idx);
      }
      for (uint32_t i = 0U; i < num_predond_valid_clusters; i++)
      {
         sorted_clusters_id_by_prior[i] = predond_valid_clusters_id[sorted_idx[i]];
      }
   }

}
