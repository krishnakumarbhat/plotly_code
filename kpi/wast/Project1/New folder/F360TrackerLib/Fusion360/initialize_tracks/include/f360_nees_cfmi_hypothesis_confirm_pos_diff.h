/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_confirm_pos_diff.h
*====================================================================================
* Copyright © 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains declarations of functions: Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_T_NEES_CFMI_HYPOTHESIS_CONFIRM_POS_DIFF_H
#define F360_T_NEES_CFMI_HYPOTHESIS_CONFIRM_POS_DIFF_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   using Is_Velocity_RR_Confidence_Level_Correct_Predicate_F = bool (*)(const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity);

   void Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);
}
#endif
