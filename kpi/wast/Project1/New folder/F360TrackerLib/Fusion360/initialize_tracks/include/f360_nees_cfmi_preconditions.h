/*===================================================================================*\
* FILE: f360_nees_cfmi_preconditions.h
*====================================================================================
* Copyright 2020 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   Precondition functions for NEES cost function initialization
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_NEES_CFMI_PRECONDITIONS_H
#define F360_NEES_CFMI_PRECONDITIONS_H

#include "f360_reuse.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_detection_hist.h"
#include "f360_cluster.h"
#include "f360_calibrations.h"
#include "f360_tracker_info.h"

namespace f360_variant_A
{
   bool NEES_CFMI_Preconditions(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T& tracker_info);
}
#endif

