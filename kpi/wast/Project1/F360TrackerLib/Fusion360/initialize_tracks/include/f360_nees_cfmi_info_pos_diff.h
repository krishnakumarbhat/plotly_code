/*===================================================================================*\
* FILE: f360_nees_cfmi_info_pos_diff.h
*====================================================================================
* Copyright - 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains declarations of functions: Get_NEES_CFMI_Information_Pos_Diff_Current_Dets() and Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_NEES_CFMI_INFO_POS_DIFF_H
#define F360_NEES_CFMI_INFO_POS_DIFF_H

#include "f360_calibrations.h"
#include "f360_cluster.h"
#include "f360_detection_props.h"
#include "f360_detection_hist.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{

   void Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Time_Slots_T& det_hist_time_slots,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   bool Check_Dets_Pos_Diff_Preconditions(const F360_Detection_Time_Slots_T& det_hist_time_slots);

   float32_t Calculate_Single_Det_Plausibility_For_VH(
      const F360_VCS_Velocity_T& vel_hyp,
      const float32_t(&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t range_rate_comp,
      const float32_t range_rate_comp_var,
      const float32_t cos_azimuth,
      const float32_t sin_azimuth,
      const float32_t sigma_squered_thr);

   void Update_NEES_Pos_Diff_Slot(
      const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Detection_T& detection_A,
      const F360_NEES_CFMI_Detection_T& detection_B,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);
}
#endif
