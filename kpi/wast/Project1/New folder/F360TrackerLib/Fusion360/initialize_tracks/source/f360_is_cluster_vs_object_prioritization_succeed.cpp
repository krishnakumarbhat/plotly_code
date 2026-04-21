/*===========================================================================*\
* FILE: f360_is_cluster_vs_object_prioritization_succeed.cpp
*============================================================================
* Copyright © 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contain function definition for Is_Cluster_Vs_Object_Prioritization_Succeed.
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [30-Mar-2018]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_is_cluster_vs_object_prioritization_succeed.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Cluster_Vs_Object_Prioritization_Succeed 
   *===========================================================================
   * RETURN VALUE:
   * Bool
   *
   * PARAMETERS:
   *  const F360_Tracker_Info_T& tracker_info
   *  const F360_Cluster_T& cluster
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function is returning false if current cluster's priority is lower that the lowest priority of all objects, with objects saturation. Otherwise it returns true.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   bool Is_Cluster_Vs_Object_Prioritization_Succeed(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Cluster_T& cluster)
   {
      bool f_cluster_prioritization_succeed;

      if (tracker_info.num_active_objs >= static_cast<int32_t>(tracker_info.variant.num_tracks))
      {
         assert(tracker_info.p_lowest_priority_track != NULL);

         f_cluster_prioritization_succeed = (cluster.priority > tracker_info.p_lowest_priority_track->priority);
      }
      else
      {
         f_cluster_prioritization_succeed = true;
      }

      return f_cluster_prioritization_succeed;
   }
}
