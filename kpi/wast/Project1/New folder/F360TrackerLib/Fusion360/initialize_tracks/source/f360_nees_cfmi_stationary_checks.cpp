/*===================================================================================*\
* FILE: f360_nees_cfmi_stationary_checks.cpp
*====================================================================================
* Copyright 2021 Aptiv Technologies, Inc., All Rights Reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
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
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/
#include "f360_nees_cfmi_stationary_checks.h"

#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_inliers_check.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_nees_cfmi_ransac.h"
#include "f360_statistical_tests.h"
#include "f360_uncertainty_propagation.h"
#include <algorithm>

namespace f360_variant_A
{
   static void Check_Stationary_Dominant_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   static void Check_Stationary_Inliers_Check(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   /*===========================================================================*\
   * FUNCTION: Check_Stationary_NEES_CFMI_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check stationary hypothesis
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Check_Stationary_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      if (stationary_velocity.f_valid)
      {
         Check_Stationary_Dominant_Hypothesis(calibrations, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);

         // If Dominant hypothesis failed check for RANSAC iteration.
         if (!nees_cfmi_init_info.VCS_vel_hyp.f_valid)
         {
            Check_Stationary_Inliers_Check(calibrations, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);
         }

         if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
         {
            nees_cfmi_init_info.f_stationary_hyp_valid = true;
            nees_cfmi_init_info.vel_hyp_source = F360_NEES_CFMI_VEL_HYP_SOURCE_STATIONARY;
         }
      }
      return nees_cfmi_init_info.f_stationary_hyp_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Stationary_NEES_Test
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Host_T& host_props,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check if hypothesis that velocity is stationary can be rejected.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Stationary_NEES_Test(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      F360_VCS_Velocity_T VCS_stationary_velocity;
      float32_t stationary_test_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
      float32_t stationary_test_cov_bias[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];

      // Extend uncertainty by host motion uncertainty
      Add_Uncertainty_2d(nees_cfmi_init_info.VCS_vel_hyp.vel_cov, host_props.vel_cov, stationary_test_cov);

      // Extend by Bias
      Constant_Uncertainty_Propagation_2d(calibrations.k_nees_cfmi_stat_nees_test_bias_factor, stationary_test_cov);
      Init_2d_Covariance_By_Std(calibrations.k_nees_cfmi_stat_nees_test_bias, stationary_test_cov_bias);
      Add_Into_Uncertainty_2d(stationary_test_cov_bias, stationary_test_cov);

      // Stationary velocity
      VCS_stationary_velocity.longitudinal = 0.0F;
      VCS_stationary_velocity.lateral = 0.0F;

      if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
      {
         float32_t alpha_level;

         if (nees_cfmi_init_info.f_stationary_hyp)
         {
            alpha_level = calibrations.k_nees_cfmi_stat_nees_test_alpha_for_stat;
         }
         else
         {
            alpha_level = calibrations.k_nees_cfmi_stat_nees_test_alpha_for_mov;
         }
         nees_cfmi_init_info.stationary_test_nees_value = Calc_Single_NEES_Value_For_Pos_Diff(VCS_stationary_velocity, nees_cfmi_init_info.VCS_vel_hyp.vel, stationary_test_cov);
         nees_cfmi_init_info.stationary_test_p_value = F360_Chi_Square_Test_Two_Degree_P_Value(nees_cfmi_init_info.stationary_test_nees_value, calibrations.k_nees_cfmi_stat_nees_max_test_var);
         nees_cfmi_init_info.f_stationary_test_rejected = F360_Try_To_Reject_Test(nees_cfmi_init_info.stationary_test_p_value, alpha_level);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Stationary_Dominant_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check stationary hypothesis
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Check_Stationary_Dominant_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      // First check for dominant
      if ((nees_cfmi_information.dominant_velocity.f_valid) && (nees_cfmi_information.dominant_velocity.dt > calibrations.k_nees_cfmi_stat_dom_hyp_min_dt))
      {
         const float32_t dominant_speed_squared = (nees_cfmi_information.dominant_velocity.vel.longitudinal * nees_cfmi_information.dominant_velocity.vel.longitudinal) +
            (nees_cfmi_information.dominant_velocity.vel.lateral * nees_cfmi_information.dominant_velocity.vel.lateral);
         const float32_t max_dominant_speed_squared = calibrations.k_nees_cfmi_stat_dom_hyp_max_speed * calibrations.k_nees_cfmi_stat_dom_hyp_max_speed;
         if (dominant_speed_squared < max_dominant_speed_squared)
         {
            float32_t dominant_vel_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
            Add_Uncertainty_2d(stationary_velocity.vel_cov, nees_cfmi_information.dominant_velocity.vel_cov, dominant_vel_cov);
            const float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(stationary_velocity.vel,
               nees_cfmi_information.dominant_velocity.vel,
               dominant_vel_cov);
            const float32_t max_dominant_nees = calibrations.k_nees_cfmi_stat_dom_hyp_sigma_level * calibrations.k_nees_cfmi_stat_dom_hyp_sigma_level;
            if (nees_value < max_dominant_nees)
            {
               nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
            }
         }
         nees_cfmi_init_info.f_dominant_valid = nees_cfmi_init_info.VCS_vel_hyp.f_valid;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Stationary_Inliers_Check
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check stationary based on inliers check.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Check_Stationary_Inliers_Check(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      const float32_t rr_var_extension = std::min(stationary_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X],
         stationary_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]);

      // Inliers check with RR extension
      nees_cfmi_init_info.VCS_vel_hyp = {};
      NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information,
         nees_cfmi_init_info.inliers,
         stationary_velocity.vel,
         rr_var_extension,
         calibrations.k_nees_cfmi_stat_inliers_check_sl_pd,
         calibrations.k_nees_cfmi_stat_inliers_check_sl_cl,
         calibrations.k_nees_cfmi_stat_inliers_check_sl_do,
         calibrations.k_nees_cfmi_min_weight_for_inlier);

      NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);
      Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);

      // Plausibility checks
      nees_cfmi_init_info.VCS_vel_hyp.f_valid =
         (nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud > calibrations.k_nees_cfmi_stat_inliers_check_min_det_plaus) &&
         (nees_cfmi_init_info.VCS_vel_hyp.num_cloud >= nees_cfmi_information.min_dets_inliers_num);

      if (!nees_cfmi_init_info.VCS_vel_hyp.f_valid)
      {
         nees_cfmi_init_info.VCS_vel_hyp = {};
      }
   }
}
