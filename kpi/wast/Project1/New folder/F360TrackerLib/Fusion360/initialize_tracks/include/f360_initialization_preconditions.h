#ifndef F360_INITIALIZATION_PRECONDITIONS_H
#define F360_INITIALIZATION_PRECONDITIONS_H
/*===========================================================================*\
* FILE: f360_initialization_preconditions.h
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declaration for Initialization_Preconditions.
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
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_cluster.h"
#include "f360_tracker_info.h"

namespace f360_variant_A
{
   bool Initialization_Preconditions(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T& tracker_info
   );
}
#endif
