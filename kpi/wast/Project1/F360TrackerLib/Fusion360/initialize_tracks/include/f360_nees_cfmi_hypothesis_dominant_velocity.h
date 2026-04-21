/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_dominant_velocity.h
*====================================================================================
* Copyright - 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains declarations of functions: Get_NEES_CFMI_Information_Dominant_Velocity() and Check_Dominant_Velocity_Preconditions().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_T_NEES_CFMI_HYPOTHESIS_DOMINANT_VELOCITY_H
#define F360_T_NEES_CFMI_HYPOTHESIS_DOMINANT_VELOCITY_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_detection_hist.h"
#include "f360_detection_time_slots.h"

namespace f360_variant_A
{
   void Get_NEES_CFMI_Information_Dominant_Velocity(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Time_Slots_T& det_hist_time_slots,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   bool Check_Dominant_Velocity_Preconditions(
      const F360_Detection_Time_Slots_T& det_hist_time_slots);
}
#endif
