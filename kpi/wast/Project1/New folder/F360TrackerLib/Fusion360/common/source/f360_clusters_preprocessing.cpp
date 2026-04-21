/*===========================================================================*\
* FILE: f360_clusters_preprocessing.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function definition of Clusters_Preprocessing,
* Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion and helper functions.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/
#include "f360_math.h"
#include "f360_kill_obj_trk.h"
#include "f360_math_func.h"
#include "f360_clear_cluster.h"
#include "f360_detection_hist.h"
#include "f360_kill_cluster.h"
#include "f360_terminate_clusters.h"
#include "f360_host_props.h"
#include "f360_norm_heading_angle.h"
#include "f360_iterator.h"
#include "f360_clusters_preprocessing.h"
#include <algorithm>
#include <cstring>

namespace f360_variant_A
{
   static void Remove_Old_Dets_From_Clusters(
      const F360_Calibrations_T &calib,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Hist_T &det_hist,
      F360_Tracker_Info_T &tracker_info
   );


   /*===========================================================================*\
   * FUNCTION: Remove_Old_Dets_From_Clusters()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS]
   * F360_Detection_Hist_T &det_hist
   * F360_Tracker_Info_T &tracker_info
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function removes old detections from clusters and kills clusters with no associated detection.
   * TODO: This functions should be refactored in DFT-341
   *
   \*===========================================================================*/
   static void Remove_Old_Dets_From_Clusters(
      const F360_Calibrations_T &calib,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Hist_T &det_hist,
      F360_Tracker_Info_T &tracker_info)
   {
      const int16_t num_clusters = tracker_info.num_active_clusters;
      
      int16_t active_clusters[NUMBER_OF_CLUSTERS];
      (void)std::copy(cmn::begin(tracker_info.active_cluster_ids), cmn::end(tracker_info.active_cluster_ids), cmn::begin(active_clusters));

      for (int16_t i = 0; i < num_clusters; i++)
      {
         const int16_t cluster_idx = active_clusters[i] - 1;
         int16_t new_ndets = 0;
         int16_t new_detvec[MAX_DETS_IN_OBJ_TRK] = {};

         // Remove old detections from cluster old_det_idx
         for (int16_t j = 0; j < clusters[cluster_idx].num_old_dets; j++)
         {
            const int16_t hist_det_idx = clusters[cluster_idx].old_det_idx[j];

            if (Is_Det_Valid_To_Keep(calib, det_hist.det_data[hist_det_idx]))
            {
               new_detvec[new_ndets] = hist_det_idx;
               new_ndets++;
            }
            else
            {
               det_hist.det_data[hist_det_idx] = {};
               det_hist.f_idx_occupied[hist_det_idx] = false;
               det_hist.n_occupied--;
            }
         }
         clusters[cluster_idx].num_old_dets = new_ndets;

         (void)std::copy(cmn::begin(new_detvec), cmn::end(new_detvec), cmn::begin(clusters[cluster_idx].old_det_idx));

         if (clusters[cluster_idx].num_old_dets == 0)
         {
            kill_cluster(clusters[cluster_idx], det_hist, tracker_info);
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const F360_Host_Props_T &host_props - a reference to host properties data structure
   * F360_Cluster_T &cluster - a reference to a single cluster
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
   * The host vehicle could possibly move (translate and rotate) between two tracker 
   * iterations. Since the VCS is fixed in host, the VCS system in one tracker 
   * iteration could therefore differ from the VCS in another tracker iteration. This
   * function corrects the estimated cluster VCS position and VCS azimuth according
   * to the host delta motion between two tracker iterations.
   *
   * PRECONDITIONS:
   * None
   * 
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(
      const F360_Host_Props_T &host_props,
      F360_Cluster_T &cluster)
   {
         // Update cluster vcs position
         float32_t vcs_long_pos_accounted_for_host_translation;
         float32_t vcs_lat_pos_accounted_for_host_translation;
         F360_Translate_2D_Position(cluster.vcs_position.x, cluster.vcs_position.y,
            -host_props.delta_position.x, -host_props.delta_position.y,
            vcs_long_pos_accounted_for_host_translation, vcs_lat_pos_accounted_for_host_translation);
         F360_Rotate_2D_Vector(vcs_long_pos_accounted_for_host_translation, vcs_lat_pos_accounted_for_host_translation,
            host_props.cos_delta_pointing, -host_props.sin_delta_pointing,
            cluster.vcs_position.x, cluster.vcs_position.y);

         // Update cluster vcs heading
         cluster.rep_vcs_az = Normalize_Heading_Angle(cluster.rep_vcs_az - host_props.delta_pointing, 0.0F);
   }

   /*===========================================================================*\
   * FUNCTION: Clusters_Preprocessing()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib - Calibration structure
   * const F360_Host_Props_T &host_props - Host properties
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS] - Clusters
   * F360_Detection_Hist_T &det_hist - Historical detection structure
   * F360_Tracker_Info_T &tracker_info - Tracker info 
   * 
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function kills clusters with flag f_to_be_killed set to true, removes 
   * old detections from clusters and kills these with no associated ones. This 
   * function also corrects the VCS properties of clusters according to the host
   * delta motion between two tracker iterations.
   *
   \*===========================================================================*/
   void Clusters_Preprocessing(
      const F360_Calibrations_T &calib,
      const F360_Host_Props_T &host_props,
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Hist_T &det_hist,
      F360_Tracker_Info_T &tracker_info)
   {
      // Terminate clusters with f_to_be_killed flag set to true
      Terminate_Clusters(clusters, det_hist, tracker_info);

      // Remove old detections from clusters
      Remove_Old_Dets_From_Clusters(calib, clusters, det_hist, tracker_info);

      // Adjusts the cluster VCS properties based on on how much host has moved since previous filter iteration
      for (uint32_t i = 0U; i < static_cast<uint32_t>(tracker_info.num_active_clusters); i++)
      {
         const uint32_t cluster_idx = static_cast<uint32_t>(tracker_info.active_cluster_ids[i]) - 1U;
         Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(host_props, clusters[cluster_idx]);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Det_Valid_To_Keep()
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_det_valid_to_keep
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib,
   * const F360_Detection_Hist_Data_T &det_hist
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function checks if detections valid to be kept based on its motion status, age and range
   * 
   \*===========================================================================*/
   bool Is_Det_Valid_To_Keep(
      const F360_Calibrations_T &calib,
      const F360_Detection_Hist_Data_T &det_hist)
   {
      const float32_t delta_time = det_hist.time_since_meas;

      const bool f_is_det_amb_and_not_old = (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS == det_hist.motion_status) && (delta_time < calib.max_age_of_older_amb_dets);
      const bool f_is_det_not_amb_stat_and_not_old = (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS != det_hist.motion_status) && (delta_time < calib.max_age_of_older_no_amb_dets);
      const bool f_is_det_in_all_looks_and_not_old = (!det_hist.f_is_range_in_all_looks) && (delta_time < calib.max_age_of_older_dets_lr);

      const bool f_is_det_valid_to_keep = ((f_is_det_amb_and_not_old || f_is_det_not_amb_stat_and_not_old) || f_is_det_in_all_looks_and_not_old);
      
      return f_is_det_valid_to_keep;
   }
}
