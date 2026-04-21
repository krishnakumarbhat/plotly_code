/*===================================================================================*\
* FILE: f360_clustering.cpp
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
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
*   This file contains Clustering() function definition.
*
* ABBREVIATIONS:
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
*
* DEVIATIONS FROM STANDARDS:
*
*
\*==========================================================================================*/


/******************************
* Includes
*******************************/

#include "f360_clustering.h"
#include "f360_clustering_detections.h"
#include "f360_get_wall_time.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Clustering()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calibrations,
   * F360_Tracker_Info_T &tracker_info,
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
   * F360_TRKR_TIMING_INFO_T &timing_info
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Group non-associated detections which are close to each other into clusters. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Clustering(
      const F360_Calibrations_T &calibrations,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Host_T& host,
      F360_Tracker_Info_T &tracker_info,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Cluster_T (&clusters)[NUMBER_OF_CLUSTERS],
      F360_TRKR_TIMING_INFO_T &timing_info
   )
   {
      const float32_t start_time = get_wall_time();

      // Clustering of detections with no track association
      Cluster_Moving_Detections(raw_detection_list, sensors, calibrations, host, tracker_info, detection_props, clusters, timing_info);

      Cluster_Leftover_Detections(raw_detection_list, sensors, calibrations, host, tracker_info, detection_props, clusters, timing_info);

      timing_info.clustering = get_wall_time() - start_time;
   }
}
