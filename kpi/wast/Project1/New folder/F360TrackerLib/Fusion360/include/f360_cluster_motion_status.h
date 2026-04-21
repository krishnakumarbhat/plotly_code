/*===================================================================================*\
* FILE: f360_cluster_motion_status.h
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
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
*   This file contains enumeration F360_Cluster_Motion_Status_T declaration
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
#ifndef F360_CLUSTER_MOTION_STATUS_H
#define F360_CLUSTER_MOTION_STATUS_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   typedef enum F360_Cluster_Motion_Status_Tag : int8_t
   {
      F360_CLUSTER_MOTION_STATUS_INVALID = -1,
      F360_CLUSTER_MOTION_STATUS_STATIONARY = 0,
      F360_CLUSTER_MOTION_STATUS_MOVING = 1,
      F360_CLUSTER_MOTION_STATUS_AMBIGUOUS = 2
   }F360_Cluster_Motion_Status_T;
}
#endif
