#ifndef F360_IS_CLUSTER_VISIBLE_H
#define F360_IS_CLUSTER_VISIBLE_H
/*===================================================================================*\
* FILE: f360_is_cluster_visible.h
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains declaration of Is_Cluster_Visible function
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "rspp_detection_list.h"
#include "f360_occlusion.h"
#include "f360_cluster.h"

namespace f360_variant_A
{
   bool Is_Cluster_Visible(const Occlusion_T& occlusion,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      const F360_Cluster_T& current_cluster);
}
#endif
