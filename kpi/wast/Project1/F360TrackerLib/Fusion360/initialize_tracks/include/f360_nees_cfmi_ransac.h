/*===================================================================================*\
* FILE: f360_nees_cfmi_ransac.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   RANSAC(s) functions for NEES cost function initialization.
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_NEES_CFMI_RANSAC_H
#define F360_NEES_CFMI_RANSAC_H

#include "f360_reuse.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   void Get_CV_NEES_CFMI_RANSAC_Vel_Hyp(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& best_nees_velocity);

   void NEES_CFMI_CV_RANSAC_Iteration_With_Comparison(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_VCS_Velocity_T & ref_vel,
      const float32_t(&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      F360_NEES_CFMI_Inliers_T& inliers,
      F360_NEES_CFMI_Velocity_T& best_nees_cfmi_velocity);

   bool Compare_CV_RANSAC_Iterations(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Velocity_T& velocity,
      const F360_NEES_CFMI_Velocity_T& best_velocity);

   void CV_RANSAC_Preconditions(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   void Estimate_Velocity_After_Ransac_Runs(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& best_nees_velocity);
}
#endif
