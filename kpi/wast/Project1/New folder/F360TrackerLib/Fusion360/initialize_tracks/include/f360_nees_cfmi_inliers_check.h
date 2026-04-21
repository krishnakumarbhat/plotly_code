/*===================================================================================*\
* FILE: f360_nees_cfmi_inliers_check.h
*====================================================================================
* Copyright - 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   RANSAC(s) functions for NEES cost function initialization.
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
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_NEES_CFMI_INLIERS_CHECK_H
#define F360_NEES_CFMI_INLIERS_CHECK_H

#include "f360_reuse.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t rr_var_extension,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight);

   void NEES_CFMI_CV_Simple_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight);

   void NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t(&constant_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t constant_rr_var,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight);

   void NEES_CFMI_CV_Full_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t (&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight);

   void NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Inliers_T& inliers,
      F360_NEES_CFMI_Velocity_T& velocity);
}
#endif


