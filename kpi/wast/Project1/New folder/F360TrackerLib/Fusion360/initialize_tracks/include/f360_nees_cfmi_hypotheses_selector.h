/*===================================================================================*\
* FILE: f360_nees_cfmi_hypotheses_selector.h
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
*   Hypotheses used for starting point for minimization of nees cost function
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
#ifndef F360_NEES_CFMI_HYPOTHESES_SELECTOR_H
#define F360_NEES_CFMI_HYPOTHESES_SELECTOR_H

#include "f360_reuse.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_nees_cfmi_vel_hyp_source.h"
#include "f360_calibrations.h"
namespace f360_variant_A
{
   void NEES_CFMI_Velocity_Hypothesis_Selector(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Vel_Hyp_Source_T vel_hyp_index,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& vel_hyp);

   void Check_CV_Dominant_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& dominant_vel_hyp);

   void Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& conf_pos_diff_vel_hyp);

   void Check_CV_Cloud_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& cloud_vel_hyp);

   void Check_CV_Radial_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& radial_vel_hyp);
}
#endif
