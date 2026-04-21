/*===================================================================================*\
* FILE: f360_detection_2_cluster_type.h
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
*   This file contains enum F360_DETECTION_2_CLUSTER_TYPE - each point in classified according to the localization in a cluster;
*    CORE_POINT - points in inner cluster with multiple neighbors
*    BORDER_POINT - points in outer cluster
*    NOISE - points with not enough neighbors to create a cluster
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
#ifndef F360_DETECTION_2_CLUSTER_TYPE_H
#define F360_DETECTION_2_CLUSTER_TYPE_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   typedef enum F360_Detection_2_Cluster_Type_Tag : uint8_t
   {
      F360_DETECTION_2_CLUSTER_TYPE_UNCLASSIFIED = (0),
      F360_DETECTION_2_CLUSTER_TYPE_NOISE = (1),
      F360_DETECTION_2_CLUSTER_TYPE_BORDER = (2),
      F360_DETECTION_2_CLUSTER_TYPE_CORE = (3)
   } F360_Detection_2_Cluster_Type_T;
}
#endif
