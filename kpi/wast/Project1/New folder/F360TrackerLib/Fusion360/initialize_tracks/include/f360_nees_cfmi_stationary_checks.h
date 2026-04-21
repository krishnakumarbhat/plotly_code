/*===================================================================================*\
* FILE: f360_nees_cfmi_stationary_checks.h
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
*   Functions to check if object is stationary or not for NEES cost function initialization
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
#ifndef F360_NEES_CFMI_STATIONARY_CHECKS_H
#define F360_NEES_CFMI_STATIONARY_CHECKS_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_host_props.h"
namespace f360_variant_A
{
   bool Check_Stationary_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   void Stationary_NEES_Test(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);
}


#endif
