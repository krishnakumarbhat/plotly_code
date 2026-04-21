#ifndef F360_IS_CLUSTER_VS_OBJECTS_PRIORITIZATION_SUCCEED_H
#define F360_IS_CLUSTER_VS_OBJECTS_PRIORITIZATION_SUCCEED_H
/*===========================================================================*\
* FILE: f360_is_cluster_vs_object_prioritization_succeed.h
*============================================================================
* Copyright © 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declarationsfor Is_Cluster_Vs_Object_Prioritization_Succeed.
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Requirements Document(s):
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

#include "f360_tracker_info.h"
#include "f360_cluster.h"

namespace f360_variant_A
{
   bool Is_Cluster_Vs_Object_Prioritization_Succeed(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Cluster_T& cluster
   );
}
#endif
