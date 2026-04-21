/*===================================================================================*\
* FILE: f360_get_new_cluster_id.cpp
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
*   The file contains the definition of Get_New_Cluster_ID function
*
* ABBREVIATIONS:
*
*
* TRACEABILITY INFO:
*   Design Document(s): f360_get_new_object_trk_id.cpp
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
#ifndef F360_GET_NEW_CLUSTER_ID_H
#define F360_GET_NEW_CLUSTER_ID_H

#include "f360_reuse.h"
#include "f360_tracker_info.h"

namespace f360_variant_A
{
   void Get_New_Cluster_ID(
      int16_t & new_id,
      F360_Tracker_Info_T & tracker_info);
}
#endif
