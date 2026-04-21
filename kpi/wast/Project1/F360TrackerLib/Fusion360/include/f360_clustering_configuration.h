/*===================================================================================*\
* FILE: f360_clustering_configuration.h
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
*  structure that configures clustering algorithm (for now DBSCAN)
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): LOOK_ID.m
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
#ifndef F360_CLUSTER_CONFIGURATION_H
#define F360_CLUSTER_CONFIGURATION_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_detection_props.h"
#include "rspp_detection.h"
#include "f360_radar_sensor.h"

namespace f360_variant_A
{
   using Detection_Cluster_Check_T = bool(*)(const F360_Calibrations_T &calib, const F360_Radar_Sensor_T& sensor, const F360_Detection_Props_T& det_p, 
      const rspp_variant_A::RSPP_Detection_T &det, const float32_t host_vcs_speed);
   using Clustering_Distance_Function_T = float32_t(*)(const F360_Detection_Props_T &det_1, const F360_Detection_Props_T &det_2);

   typedef struct F360_Clustering_Configuration_Tag
   {
      Clustering_Distance_Function_T Cluster_Distance_Sq_Function;
      Detection_Cluster_Check_T Detection_Cluster_Check;
      float32_t clustering_radius;
      float32_t clustering_radius_sq;
      uint32_t min_pts_in_cluster;
   }F360_Clustering_Configuration_T;

   // 64-bit
   static_assert((4 == sizeof(void*)) || (32 == sizeof(F360_Clustering_Configuration_T)), "sizeof(F360_Clustering_Configuration_T) not as expected. Remember to align padding if needed");

   // 32-bit
   static_assert((8 == sizeof(void*)) || (20 == sizeof(F360_Clustering_Configuration_T)), "sizeof(F360_Clustering_Configuration_T) not as expected. Remember to align padding if needed");
}
#endif
