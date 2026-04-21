/*===================================================================================*\
* FILE: f360_nees_cfmi_minimization.h
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
*   Function for minimization of defined cost function
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
#ifndef F360_NEES_CFMI_MINIMIZATION_H
#define F360_NEES_CFMI_MINIMIZATION_H

#include "f360_reuse.h"
#include "f360_nees_cfmi_structs.h"
namespace f360_variant_A
{
   void Estimate_Vel_By_CV_NEES_Min_Analytical(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Inliers_T& inliers,
      F360_NEES_CFMI_Velocity_T& velocity);
}
#endif
