/*===================================================================================*\
* FILE: f360_nees_cfmi_m_estimator.h
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
*   M-estimator(s) for NEES cost function
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
#ifndef F360_NEES_CFMI_M_ESTIMATOR_H
#define F360_NEES_CFMI_M_ESTIMATOR_H

#include "f360_nees_cfmi_structs.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   void Estimate_Vel_By_CV_NEES_M_Estimator(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   bool CV_M_Estimator_Plausibility_Check(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity);
}
#endif
