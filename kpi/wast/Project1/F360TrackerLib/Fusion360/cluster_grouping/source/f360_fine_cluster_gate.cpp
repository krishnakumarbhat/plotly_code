/*===================================================================================*\
* FILE:  f360_fine_cluster_gate.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains the definition of the function Fine_Cluster_Gate() which can be used
* to check if two clusters are close to each other in position. The two clusters may be of
* different age and the possible position difference of the clusters caused by this is 
* accounted for by utilizing the range rate of the older cluster together with the age 
* difference
* 
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_math.h"
#include "f360_fine_cluster_gate.h"
#include "f360_is_pos_dist_inside_ellipse.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   static bool Calc_Dist_Gate_Of_Two_Clusters(
      const F360_Calibrations_T& calib,
      const F360_Cluster_T& cluster_older,
      const F360_Cluster_T& cluster_newer,
      const float32_t rdot_interval_older,
      const float32_t n_aliased_older
   );

   bool Fine_Cluster_Gate(
      const F360_Calibrations_T& calib,
      const F360_Cluster_T& cluster1,
      const F360_Cluster_T& cluster2,
      const float32_t rdot_interval_1,
      const float32_t rdot_interval_2,
      const float32_t n_aliased_1,
      const float32_t n_aliased_2
   )
   {
      bool f_success;

      if (cluster1.time_since_cluster_updated > cluster2.time_since_cluster_updated)
      {
         f_success = Calc_Dist_Gate_Of_Two_Clusters(calib, cluster1, cluster2, rdot_interval_1, n_aliased_1);
      }
      else
      {
         f_success = Calc_Dist_Gate_Of_Two_Clusters(calib, cluster2, cluster1, rdot_interval_2, n_aliased_2);
      }

      return f_success;
   }

   static bool Calc_Dist_Gate_Of_Two_Clusters(
      const F360_Calibrations_T& calib,
      const F360_Cluster_T& cluster_older,
      const F360_Cluster_T& cluster_newer,
      const float32_t rdot_interval_older,
      const float32_t n_aliased_older
   )
   {
      const float32_t cross_radial_gate = calib.k_cross_radial_expand_factor * F360_Get_Hypotenuse((cluster_older.vcs_position.x + cluster_newer.vcs_position.x) * 0.5F,
         (cluster_older.vcs_position.y + cluster_newer.vcs_position.y) * 0.5F);

      const float32_t delta_time = cluster_older.time_since_cluster_updated - cluster_newer.time_since_cluster_updated;
      const float32_t temp_rdot_comp = cluster_older.rep_rdotcomp + (n_aliased_older * rdot_interval_older);
      const float32_t trk_az_rep_cos = cluster_older.cos_vcs_az;
      const float32_t trk_az_rep_sin = cluster_older.sin_vcs_az;

      const float32_t rdot_dist = delta_time * temp_rdot_comp;
      const float32_t delta_x = cluster_older.vcs_position.x + rdot_dist * trk_az_rep_cos - cluster_newer.vcs_position.x;
      const float32_t delta_y = cluster_older.vcs_position.y + rdot_dist * trk_az_rep_sin - cluster_newer.vcs_position.y;

      /* Position gating.
      * Account for uncertainty in cross - radial motion(e.g., use elliptical gate ? ).
      * FOR NOW, use tilted ellipse in world coordinates for gating. Elipse is rotated by old cluster azimuth with
      * the(minor) axis, in the world - az direction and the(major) axis, orthogonal to world - az, has a size which increases with
      * time difference between the two clusters. Center of elipse is considered as old cluster's position. 
      * Checking distance is between old cluster to new cluster with time difference consideration
      */
      const bool f_success = Is_Pos_Dist_Inside_Ellipse(delta_x, delta_y, trk_az_rep_cos, trk_az_rep_sin,
         (cross_radial_gate + calib.k_max_crossing_speed * delta_time), calib.k_radial_gate);

      return f_success;
   }
}
