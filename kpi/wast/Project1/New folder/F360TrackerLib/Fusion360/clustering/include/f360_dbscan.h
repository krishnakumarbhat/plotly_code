/*===================================================================================*\
* FILE: f360_dbscan.h
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
*   This file contains function signature of DBSCAN related functions
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
#ifndef F360_DBSCAN_H
#define F360_DBSCAN_H

#include "f360_reuse.h"
#include "f360_clustering_configuration.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_local_clusters.h"
#include "f360_tracker_info.h"

namespace f360_variant_A
{
   void DBscan(
      const F360_Tracker_Info_T& tracker_info,
      const bool(&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const uint16_t num_det_pts,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const F360_Clustering_Configuration_T &cluster_config,
      const int16_t(&sorted_det_idxs)[MAX_NUMBER_OF_DETECTIONS],
      F360_Local_Clusters_T& output_data);

   uint16_t Cluster_Region_Query(
      const bool (&valid_dets)[MAX_NUMBER_OF_DETECTIONS],
      const uint16_t detection_index,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const F360_Clustering_Configuration_T &cluster_config,
      uint16_t(&neighbor_pts)[MAX_NUMBER_OF_DETECTIONS]);
}
#endif
