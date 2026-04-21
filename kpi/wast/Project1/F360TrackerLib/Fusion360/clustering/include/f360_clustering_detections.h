/*===================================================================================*\
* FILE: f360_clustering_detections.h
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
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
*   This file contains function signature of detections clustering functions
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
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef CLUSTERING_DETECTIONS_H
#define CLUSTERING_DETECTIONS_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_cluster.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_detection_props.h"
#include "f360_timing_info.h"
#include "f360_tracker_info.h"
#include "f360_clustering_configuration.h"
#include "f360_host_props.h"
#include "f360_host.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   void Cluster_Moving_Detections(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      F360_Tracker_Info_T &tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info
   );

   void Cluster_Leftover_Detections(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      F360_Tracker_Info_T &tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info
   );

   float32_t Clustering_Distance_Position_Rangerate(
      const F360_Detection_Props_T &det_1,
      const F360_Detection_Props_T &det_2);

   float32_t Clustering_Distance_Position(
      const F360_Detection_Props_T &det_1,
      const F360_Detection_Props_T &det_2);

   bool Cluster_Leftovers_Check(
      const F360_Calibrations_T &calib,
      const F360_Radar_Sensor_T &sensor,
      const F360_Detection_Props_T &det_p,
      const rspp_variant_A::RSPP_Detection_T &det,
      const float32_t host_vcs_speed);

   bool Cluster_Moving_Check(
      const F360_Calibrations_T &calib,
      const F360_Radar_Sensor_T &sensor,
      const F360_Detection_Props_T &det_p,
      const rspp_variant_A::RSPP_Detection_T &det,
      const float32_t host_vcs_speed);

   void Cluster_Detections(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Clustering_Configuration_T& clustering_config,
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]);

   void Find_And_Prioritize_Detections(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Clustering_Configuration_T& clustering_config,
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      int16_t& valid_det_count,
      int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS],
      bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS]);

   bool Is_Detection_In_Third_Priority_Zone(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list, 
      const F360_Host_T& host, 
      const int16_t& det_idx);
}
#endif
