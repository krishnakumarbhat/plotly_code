/*===========================================================================*\
* FILE: f360_downselect_longi_stat_clusters.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Downselect_Longi_Stat_Clusters()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_downselect_longi_stat_clusters.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
 /*===========================================================================*\
 * FUNCTION: Downselect_Longi_Stat_Clusters()
 * ===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   const uint16_t nr_valid_clusters,
 *   const F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS],
 *   const F360_Calibrations_T& calibs,
 *   uint16_t& nr_downselected_clusters,
 *   F360_Longi_Stat_Cluster_T(&downselected_clusters)[MAX_NR_OF_LONGI_STAT_CURVES]
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
 * This function downselects which clusters should be used to fit a polynomial to and
 * thus become valid longi stat curves for this tracker iteration.
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
 \*===========================================================================*/
   void Downselect_Longi_Stat_Clusters(
      const uint16_t nr_valid_clusters,
      const F360_Longi_Stat_Cluster_T(&valid_clusters)[NR_LONGI_STAT_CLUSTERS],
      const F360_Calibrations_T& calibs,
      uint16_t& nr_downselected_clusters,
      F360_Longi_Stat_Cluster_T(&downselected_clusters)[MAX_NR_OF_LONGI_STAT_CURVES])
   {
      
      float32_t cluster_score_array[NR_LONGI_STAT_CLUSTERS] = {};
      for (uint16_t i = 0U; i < nr_valid_clusters; i++)
      {
         cluster_score_array[i] = Calc_Longi_Stat_Cluster_Score(valid_clusters[i], calibs);
      }

      uint32_t cluster_score_array_sorted[NR_LONGI_STAT_CLUSTERS] = {};
      (void)F360_Sort(cluster_score_array, static_cast<uint32_t>(nr_valid_clusters), true, cluster_score_array_sorted);

      nr_downselected_clusters = 0U;
      for (uint32_t i = 0U; (i < nr_valid_clusters) && (i < MAX_NR_OF_LONGI_STAT_CURVES); i++)
      {
         downselected_clusters[nr_downselected_clusters] = valid_clusters[cluster_score_array_sorted[i]];
         nr_downselected_clusters++;
      }
   }

 /*===========================================================================*\
 * FUNCTION: Calc_Longi_Stat_Cluster_Score()
 * ===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   const F360_Longi_Stat_Cluster_T& lsc_cluster
 *   const F360_Calibrations_T& calibs
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
 * This function calculates an "importance score" of a longi stat cluster that
 * wants to create a polynomial. Score is dependent on distance from VCS origin.
 * Function uses logic gates for the only three possible scenarios. 
 *  - Both end points of cluster is negative, curve is behind host
 *  - Both end points of cluster is positive, curve is in front of host
 *  - Lower end point is negative and upper end point is positive, curve is adjacent to host
 * 
 * The clusters longitudinal spread is weigthed in. A longer longi stat curve
 * is considered more important. 
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
 \*===========================================================================*/
   float32_t Calc_Longi_Stat_Cluster_Score(
      const F360_Longi_Stat_Cluster_T& lsc_cluster,
      const F360_Calibrations_T& calibs)
   {
      float32_t cluster_score;

      const float32_t x_max = Get_Cluster_Max_Long_Pos(lsc_cluster);
      const float32_t x_min = Get_Cluster_Min_Long_Pos(lsc_cluster);
      const float32_t delta_x = x_max - x_min;
      if (delta_x > F360_EPSILON)
      {
         // Put punishing score on short curves
         cluster_score = calibs.k_lsc_length_score_gain / delta_x;

         // Distance score is evaluated as either direct lateral distance from VCS or to closest valid x-point
         if ((x_min < 0.0F) && (x_max > 0.0F))
         {
            cluster_score += std::abs(lsc_cluster.lat_mean);
         }
         else if (x_min > 0.0F)
         {
            // Cluster is in positive longitudinal quadrants
            cluster_score += F360_Get_Hypotenuse(x_min, lsc_cluster.lat_mean);
         }
         else
         {
            // Cluster is in negative longitudinal quadrants
            cluster_score += F360_Get_Hypotenuse(x_max, lsc_cluster.lat_mean);
         }

      }
      else
      {
         cluster_score = INFTY;
      }

      return cluster_score;
   }
}

