/*===================================================================================*\
* FILE: f360_clear_cluster.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function definition of Clear_Cluster
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include <cstring>
#include "f360_clear_cluster.h"
#include <algorithm>
#include "f360_iterator.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Clear_Cluster
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Cluster_T & cluster - Cluster to be cleared
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function clears all the fields of a cluster
   *
   \*===========================================================================*/

   void Clear_Cluster(
      F360_Cluster_T & cluster)
   {
      cluster.vcs_position.x = 0.0F;
      cluster.vcs_position.y = 0.0F;
      cluster.rep_vcs_az = 0.0F;
      cluster.cos_vcs_az = 0.0F;
      cluster.sin_vcs_az = 0.0F;
      cluster.rep_rdotcomp = 0.0F;
      cluster.exist_prob = 0.0F;
      cluster.ndets = 0;
      std::fill(cmn::begin(cluster.detids), cmn::end(cluster.detids), static_cast<int16_t>(0));
      cluster.num_old_dets = 0;
      std::fill(cmn::begin(cluster.old_det_idx), cmn::end(cluster.old_det_idx), static_cast<int16_t>(0));
      std::fill(cmn::begin(cluster.num_types_of_dets), cmn::end(cluster.num_types_of_dets), static_cast<int16_t>(0));
      cluster.f_dealiased = false;
      cluster.f_to_be_killed = false;
      cluster.low_rcs_dets_cnt = 0U;
      cluster.motion_status = F360_CLUSTER_MOTION_STATUS_INVALID;
      cluster.time_since_measurement = -1.0F;
      cluster.time_since_created = -1.0F;
      cluster.time_since_cluster_updated = -1.0F;
      cluster.num_of_cluster_merges = 0U;
      cluster.priority = 0.0F;

   }
}
