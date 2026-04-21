/*===========================================================================*\
* FILE: f360_sort_clusters_by_priority.h
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declaration for Sort_Clusters_By_Priority.
*
* 
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/
#ifndef F360_SORT_CLUSTERS_BY_PRIORITY_H
#define F360_SORT_CLUSTERS_BY_PRIORITY_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_detection_hist.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_host.h"
#include "f360_tracker_info.h"
#include "f360_cluster.h"

namespace f360_variant_A
{
   void Sort_Clusters_By_Priority(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Hist_T& det_hist,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Host_T& host,
      const F360_Tracker_Info_T& tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      int32_t(&sorted_clusters_id_by_prior)[NUMBER_OF_CLUSTERS],
      uint32_t& num_predond_valid_clusters);
}

#endif
