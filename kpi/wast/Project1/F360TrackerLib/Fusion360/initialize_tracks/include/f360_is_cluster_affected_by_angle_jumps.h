#ifndef F360_IS_CLUSTER_AFFECTED_BY_ANGLE_JUMPS_H
#define F360_IS_CLUSTER_AFFECTED_BY_ANGLE_JUMPS_H
/*===========================================================================*\
* FILE: f360_is_cluster_affected_by_angle_jumps.h
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
*   This file contains function declaration for Is_Cluster_Affected_By_Angle_Jumps.
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

#include "f360_calibrations.h"
#include "f360_detection_hist.h"
#include "f360_detection_props.h"
#include "f360_cluster.h"

namespace f360_variant_A
{
   bool Is_Cluster_Affected_By_Angle_Jumps(
      const F360_Cluster_T& cluster,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& detection_hist,
      const F360_Calibrations_T& calibrations
   );
}
#endif
