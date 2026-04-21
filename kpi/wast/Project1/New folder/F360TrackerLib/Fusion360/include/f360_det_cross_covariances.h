/*===================================================================================*\
* FILE: f360_det_cross_covariances.h
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
*   F360 cross covariances array definitions for detections
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
#ifndef F360_DET_CROSS_COVARIANCES_H
#define F360_DET_CROSS_COVARIANCES_H

namespace f360_variant_A
{
   enum F360_DET_CROSS_COVARIANCES
   {
      F360_DET_CROSS_COV_VCS_X_VCS_AZ = 0,
      F360_DET_CROSS_COV_VCS_X_VCS_COMP_RANGE_RATE = 1,
      F360_DET_CROSS_COV_VCS_Y_VCS_AZ = 2,
      F360_DET_CROSS_COV_VCS_Y_VCS_COMP_RANGE_RATE = 3,
      F360_DET_CROSS_COV_VCS_AZ_VCS_COMP_RANGE_RATE = 4
   };
}
#endif
