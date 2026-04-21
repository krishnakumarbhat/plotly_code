/*===================================================================================*\
* FILE: f360_dbscan.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* The file contains implementation of DB-SCAN functionality
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

/******************************
* Includes
*******************************/
#include "f360_dbscan.h"
#include "f360_detection_2_cluster_type.h"
#include "f360_math_func.h"
#include <algorithm>

/******************************
*   Function prototypes
*******************************/
namespace f360_variant_A
{
   static void Add_New_Point_If_Inlier(
      const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const int16_t det_pt_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const F360_Detection_Props_T &det_base,
      uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS],
      uint16_t& neighbor_pt_index);

   static void Cluster_Spread(
      const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const uint16_t detection_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      bool(&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS],
      uint16_t &base_cluster_num_neighbor_pts,
      uint16_t(&base_neighbor_pts)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_2_Cluster_Type_T(&det_pt_type)[MAX_NUMBER_OF_DETECTIONS]);

   static F360_Detection_2_Cluster_Type_T Cluster_Expand(
      const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const uint16_t detection_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const uint16_t cluster_id,
      uint16_t(&det_pts_cluster_id)[MAX_NUMBER_OF_DETECTIONS],
      bool(&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_2_Cluster_Type_T(&det_pt_type)[MAX_NUMBER_OF_DETECTIONS],
      F360_Local_Clusters_T &output_data);

   /*===========================================================================*\
   * FUNCTION: DBscan()
   *===========================================================================
   * RETURN VALUE:
   * Number of Clusters
   *
   * PARAMETERS:
   * const F360_Tracker_Info_T& tracker_info
   * const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS]
   * const uint16_t num_valid_dets
   * const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detections
   * const F360_Clustering_Configuration_T &cluster_config
   * const int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Local_Clusters_T &output_data
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
   * This function does the DBSCAN
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void DBscan(
      const F360_Tracker_Info_T& tracker_info,
      const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const uint16_t num_valid_dets,
      const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const F360_Clustering_Configuration_T &cluster_config,
      const int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS],
      F360_Local_Clusters_T &output_data)
   {
      uint16_t new_cluster_id = 1U;

      bool det_pt_visited[MAX_NUMBER_OF_DETECTIONS] = {}; // Array to keep track of points that have been visited
      uint16_t det_pts_cluster_id[MAX_NUMBER_OF_DETECTIONS] = {};
      F360_Detection_2_Cluster_Type_T det_pt_type[MAX_NUMBER_OF_DETECTIONS] = {};

      output_data.num_of_associated_dets = 0U;
      output_data.num_clusters = 0U;
      output_data.num_dets_in_clusters[output_data.num_clusters] = 0U;
      output_data.array_of_det_idxs_in_clusters[output_data.num_clusters] = 0U;

      for (uint32_t i = 0U; i < num_valid_dets; i++)
      {
         const uint16_t detection_index = sorted_det_idxs[i];
         // If this point not visited yet, mark it as visited
         if ((!det_pt_visited[detection_index]) && valid_dets[detection_index])
         {
            const F360_Detection_2_Cluster_Type_T current_det_type = Cluster_Expand(valid_dets, cluster_config, detection_index, det_p, raw_detections, new_cluster_id, det_pts_cluster_id, det_pt_visited, det_pt_type, output_data);
            if (current_det_type == F360_DETECTION_2_CLUSTER_TYPE_CORE)
            {
               new_cluster_id++;
               output_data.num_clusters++;
               output_data.num_dets_in_clusters[output_data.num_clusters] = 0U;
               if (new_cluster_id > tracker_info.variant.num_posn_clusters)
               {
                  break;
               }
            }
         }
      }
      output_data.num_clusters = new_cluster_id - 1U;
   }

   /*===========================================================================*\
   * FUNCTION: Add_New_Point_If_Inlier()
   *===========================================================================
   * RETURN VALUE:
   * Number of neighbor points
   *
   * PARAMETERS:
   * const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
   * const int16_t detection_index,
   * const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * const F360_Clustering_Configuration_T &cluster_config,
   * const F360_Detection_Props_T &det_base,
   * uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS]
   * uint16_t& neighbor_pt_index
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
   * This function compares the distance between 2 points
   * if distance^2 < radius^2:
   *    add to array of neighbour points and increment counter
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Add_New_Point_If_Inlier(
      const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const int16_t det_pt_index,
      const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const F360_Detection_Props_T &det_base,
      uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS],
      uint16_t& neighbor_pt_index)
   {
      if (true == valid_dets[det_pt_index])
      {
         if (cluster_config.Cluster_Distance_Sq_Function(det_p[det_pt_index], det_base) < cluster_config.clustering_radius_sq)
         {
            neighbor_pts[neighbor_pt_index] = static_cast<uint16_t>(det_pt_index);
            neighbor_pt_index++;
         }
      }
   }

    /*===========================================================================*\
    * FUNCTION: Cluster_Region_Query()
    *===========================================================================
    * RETURN VALUE:
    * Number of neighbor points
    *
    * PARAMETERS:
    * const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS]
    * const uint16_t detection_index
    * const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS]
    * const rspp_variant_A::RSPP_Detection_List_T &raw_detections
    * const F360_Clustering_Configuration_T &cluster_config
    * uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS]
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
    * This function finds neighbor points for a given detection point.
    *
    * PRECONDITIONS:
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   uint16_t Cluster_Region_Query(
      const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const uint16_t detection_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const F360_Clustering_Configuration_T &cluster_config,
      uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS])
   {
      uint16_t neighbor_pt_index = 0U;
      int16_t det_pt_index_forward = static_cast<int16_t>(detection_index);
      for (uint16_t i = 0U; i < raw_detections.number_of_valid_detections; i++)
      {
         det_pt_index_forward = raw_detections.detections[det_pt_index_forward].processed.next_sorted_idx;      // iterate forward
         if ((det_pt_index_forward == F360_INVALID_ID) ||
            (std::abs(det_p[det_pt_index_forward].vcs_position.x - det_p[detection_index].vcs_position.x) > cluster_config.clustering_radius))
         {
            break;
         }
         Add_New_Point_If_Inlier(valid_dets, det_pt_index_forward, det_p, cluster_config, det_p[detection_index], neighbor_pts, neighbor_pt_index);
      }

      int16_t det_pt_index_bacwards = static_cast<int16_t>(detection_index);
      for (uint16_t i = 0U; i < raw_detections.number_of_valid_detections; i++)
      {
         det_pt_index_bacwards = raw_detections.detections[det_pt_index_bacwards].processed.prev_sorted_idx;    // iterate backwards
         if ((det_pt_index_bacwards == F360_INVALID_ID) || (std::abs(det_p[det_pt_index_bacwards].vcs_position.x - det_p[detection_index].vcs_position.x) > cluster_config.clustering_radius))
         {
            break;
         }
         Add_New_Point_If_Inlier(valid_dets, det_pt_index_bacwards, det_p, cluster_config, det_p[detection_index], neighbor_pts, neighbor_pt_index);
      }

      return neighbor_pt_index;
   }

     /*===========================================================================*\
     * FUNCTION: Cluster_Spread()
     *===========================================================================
     * RETURN VALUE:
     * Number of Clusters
     *
     * PARAMETERS:
     * const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS]
     * const F360_Clustering_Configuration_T &cluster_config
     * const uint16_t detection_index
     * const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS]
     * const rspp_variant_A::RSPP_Detection_List_T &raw_detections
     * bool(&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS]
     * uint16_t &base_cluster_num_neighbor_pts
     * uint16_t(&base_neighbor_pts)[MAX_NUMBER_OF_DETECTIONS]
     * F360_Detection_2_Cluster_Type_T(&det_pt_type)[MAX_NUMBER_OF_DETECTIONS]
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
     * This function is call for every CORE POINT associated to cluster;
     * For given detection and base cluster -> find all neighbours for given detection,
     * downselect those not associated to base cluster and add them to it.
     *
     * PRECONDITIONS:
     * All the Pointers should Point to valid structures.
     *
     * POSTCONDITIONS:
     * None
     *
     \*===========================================================================*/
   static void Cluster_Spread(
      const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const uint16_t detection_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      bool(&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS],
      uint16_t &base_cluster_num_neighbor_pts,
      uint16_t(&base_neighbor_pts)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_2_Cluster_Type_T(&det_pt_type)[MAX_NUMBER_OF_DETECTIONS])
   {

      uint32_t num_neighbor_pts = 0U;
      uint16_t neighbor_pts[MAX_NUMBER_OF_DETECTIONS] = {};

      det_pt_visited[detection_index] = true;
      // Find the neighbors of this neighbor and if it has
      // enough neighbors add them to the neighbourPts list
      
      num_neighbor_pts = Cluster_Region_Query(valid_dets, detection_index, det_p, raw_detections, cluster_config, neighbor_pts);

      if (num_neighbor_pts >= cluster_config.min_pts_in_cluster)
      {
         det_pt_type[detection_index] = F360_DETECTION_2_CLUSTER_TYPE_CORE;
         for (uint32_t neighbor_count = 0U; neighbor_count < num_neighbor_pts; neighbor_count++)
         {
            const uint16_t* const p_neighbor_id = std::find(&base_neighbor_pts[0], &base_neighbor_pts[base_cluster_num_neighbor_pts], neighbor_pts[neighbor_count]);
            // Basically checking any no point was found - then this point should be added
            if (p_neighbor_id == &base_neighbor_pts[base_cluster_num_neighbor_pts])
            {
               base_neighbor_pts[base_cluster_num_neighbor_pts] = neighbor_pts[neighbor_count];
               base_cluster_num_neighbor_pts++;
            }
         }
      }
      else
      {
         det_pt_type[detection_index] = F360_DETECTION_2_CLUSTER_TYPE_BORDER;
      }
   }

    /*===========================================================================*\
    * FUNCTION: Cluster_Expand()
    *===========================================================================
    * RETURN VALUE:
    * Number of Clusters
    *
    * PARAMETERS:
    * const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS]
    * const F360_Clustering_Configuration_T &cluster_config
    * const uint16_t detection_index
    * const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS]
    * const rspp_variant_A::RSPP_Detection_List_T &raw_detections
    * const uint16_t cluster_id
    * uint16_t (&det_pts_cluster_id)[MAX_NUMBER_OF_DETECTIONS]
    * bool (&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS]
    * F360_Detection_2_Cluster_Type_T (&det_pt_type)[MAX_NUMBER_OF_DETECTIONS]
    * F360_Local_Clusters_T &output_data
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
    * This function Cluster_Expand given point with all its neighbour points (according to the distance function);
    * for all found neighbours it calls Cluster_Spread function
    *
    * PRECONDITIONS:
    * All the Pointers should Point to valid structures.
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   static F360_Detection_2_Cluster_Type_T Cluster_Expand(
      const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Clustering_Configuration_T &cluster_config,
      const uint16_t detection_index,
      const F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const uint16_t cluster_id,
      uint16_t (&det_pts_cluster_id)[MAX_NUMBER_OF_DETECTIONS],
      bool (&det_pt_visited)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_2_Cluster_Type_T (&det_pt_type)[MAX_NUMBER_OF_DETECTIONS],
      F360_Local_Clusters_T &output_data)
   {
      uint16_t neighbor_pts[MAX_NUMBER_OF_DETECTIONS] = {};

      det_pt_visited[detection_index] = true;
      // Find the neighbor points
      uint16_t num_neighbor_pts = Cluster_Region_Query(valid_dets, detection_index, det_p, raw_detections, cluster_config, neighbor_pts);

      if (num_neighbor_pts < (cluster_config.min_pts_in_cluster - 1U))
      {
         det_pts_cluster_id[detection_index] = 0U; // Not enough points to form a cluster. Mark the point as noise.
         det_pt_type[detection_index] = F360_DETECTION_2_CLUSTER_TYPE_NOISE;
      }
      else
      {
         // and mark the point as being a member of cluster num_cluster.
         det_pts_cluster_id[detection_index] = cluster_id;
         det_pt_type[detection_index] = F360_DETECTION_2_CLUSTER_TYPE_CORE;

         output_data.array_of_det_idxs_in_clusters[output_data.num_of_associated_dets] = detection_index;
         output_data.num_of_associated_dets++;
         output_data.num_dets_in_clusters[output_data.num_clusters]++;

         // For each point P' in neighbor_pts
         uint16_t neighbor_pt_index = 0U;
         while (neighbor_pt_index < num_neighbor_pts) 
         {
            const uint16_t neighbor_point = neighbor_pts[neighbor_pt_index];

            // If this neighbor has not been visited, mark it as visited
            if (false == det_pt_visited[neighbor_point])
            {
               Cluster_Spread(valid_dets, cluster_config, neighbor_point, det_p, raw_detections, det_pt_visited, num_neighbor_pts, neighbor_pts, det_pt_type);
            }
            // If this neighbour not yet a member of any cluster add it to this cluster.
            if (0U == det_pts_cluster_id[neighbor_point])
            {
               det_pts_cluster_id[neighbor_point] = cluster_id;

               output_data.array_of_det_idxs_in_clusters[output_data.num_of_associated_dets] = neighbor_point;
               output_data.num_of_associated_dets++;
               output_data.num_dets_in_clusters[output_data.num_clusters]++;
            }
            neighbor_pt_index++;
         }
      }
      return det_pt_type[detection_index];
   }

}
