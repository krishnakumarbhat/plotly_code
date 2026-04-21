/*===================================================================================*\
* FILE: f360_nees_cfmi_m_estimator.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Estimate_Vel_By_CV_NEES_M_Estimator function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "f360_nees_cfmi_m_estimator.h"

#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_inliers_check.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_uncertainty_propagation.h"
#include "f360_reuse.h"
#include <algorithm>

namespace f360_variant_A
{
   static void Modify_Sigma_Levels_For_M_Estimator(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   static bool CV_M_Estimator_Convergence_Check(const F360_Calibrations_T& calibrations,
      const F360_VCS_Velocity_T& prev_vel,
      const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   /*===========================================================================*\
   * FUNCTION: Estimate_Vel_By_CV_NEES_M_Estimator
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * M-estimator implementation for Constant velocity NEES Cost function velocity estimation
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Estimate_Vel_By_CV_NEES_M_Estimator(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      const uint32_t max_number_of_iteration = 10U;
      const uint32_t number_of_iteration = std::min(max_number_of_iteration, calibrations.k_nees_cfmi_m_est_max_iter); //DFD-1878 to decide if use the calibration directly


      for (uint32_t loop_index = 0U; loop_index < number_of_iteration; loop_index++)
      {
         const F360_VCS_Velocity_T prev_vel = nees_cfmi_init_info.VCS_vel_hyp.vel;

         NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, nees_cfmi_init_info.inliers,
            nees_cfmi_init_info.VCS_vel_hyp.vel, nees_cfmi_init_info.VCS_vel_hyp.vel_cov,
            nees_cfmi_init_info.sigma_level_pos_diff, nees_cfmi_init_info.sigma_level_cloud,
            calibrations.k_sigma_level_dominant_velocity, calibrations.k_nees_cfmi_min_weight_for_inlier);

         // Estimate velocity
         nees_cfmi_init_info.VCS_vel_hyp = {};
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);
         Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(nees_cfmi_init_info.VCS_vel_hyp);

         const bool f_is_next_iteration_needed = (0U < loop_index) ? CV_M_Estimator_Convergence_Check(calibrations, prev_vel, nees_cfmi_init_info) : true;

         if (f_is_next_iteration_needed)
         {
            // Evaluate velocity: needed for sigma level modification
            NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);
            Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);

            Modify_Sigma_Levels_For_M_Estimator(calibrations, nees_cfmi_information, nees_cfmi_init_info);
         }
         else
         {
            break;
         }
      }
      // Evaluate final estimate
      nees_cfmi_init_info.VCS_vel_hyp.Reset_Plausibility();
      NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, nees_cfmi_init_info.inliers,
         nees_cfmi_init_info.VCS_vel_hyp.vel, nees_cfmi_init_info.VCS_vel_hyp.vel_cov,
         nees_cfmi_init_info.sigma_level_pos_diff, nees_cfmi_init_info.sigma_level_cloud,
         calibrations.k_sigma_level_dominant_velocity, calibrations.k_nees_cfmi_min_weight_for_inlier);

      NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);

      // Plausibility checks
      Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);
      nees_cfmi_init_info.VCS_vel_hyp.f_valid = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);

      nees_cfmi_init_info.f_m_estimator_valid = nees_cfmi_init_info.VCS_vel_hyp.f_valid;

   }


   /*===========================================================================*\
   * FUNCTION: Modify_Sigma_Levels_For_M_Estimator
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Modify Sigma Levels for Position Difference and Cloud
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Modify_Sigma_Levels_For_M_Estimator(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      float32_t sigma_level_pos_diff;
      float32_t sigma_level_cloud;
      float32_t sigma_level_pos_diff_filtered;
      float32_t sigma_level_cloud_filtered;

      // Position difference
      if (nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff > nees_cfmi_information.expected_vels_inliers_num)
      {
         const float32_t extended_vels_num = static_cast<float32_t>(nees_cfmi_information.vels_num) + 1.0F;
         // There is too many inliers, sigma level need to be reduced
         sigma_level_pos_diff = F360_Linear_Equation_With_Saturation(static_cast<float32_t>(nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff),
                         static_cast<float32_t>(nees_cfmi_information.expected_vels_inliers_num), extended_vels_num,
            calibrations.k_nees_cfmi_m_est_msl_pd_sl_neutral, calibrations.k_nees_cfmi_m_est_msl_pd_sl_for_max_over_inliers);
      }
      else
      {
         sigma_level_pos_diff = F360_Linear_Equation_With_Saturation(nees_cfmi_init_info.VCS_vel_hyp.plausibility_pos_diff,
            calibrations.k_nees_cfmi_m_est_msl_min_pd_plaus, F360_MAX_PROBABILITY,
            calibrations.k_nees_cfmi_m_est_msl_pd_sl_for_min_plaus, calibrations.k_nees_cfmi_m_est_msl_pd_sl_neutral);
      }

      // Cloud
      sigma_level_cloud = F360_Linear_Equation_With_Saturation(nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud,
         calibrations.k_nees_cfmi_m_est_msl_min_cl_plaus, F360_MAX_PROBABILITY,
         calibrations.k_nees_cfmi_m_est_msl_cl_sl_for_min_plaus, calibrations.k_nees_cfmi_m_est_msl_cl_sl_for_max_plaus);

      // Filtering
      sigma_level_pos_diff_filtered = F360_Low_Pass_Filter_First_Order(sigma_level_pos_diff, nees_cfmi_init_info.sigma_level_pos_diff,
         calibrations.k_nees_cfmi_m_est_msl_low_pass_filter_alpha);
      sigma_level_cloud_filtered = F360_Low_Pass_Filter_First_Order(sigma_level_cloud, nees_cfmi_init_info.sigma_level_cloud,
         calibrations.k_nees_cfmi_m_est_msl_low_pass_filter_alpha);

      // Sigma level should not increase
      nees_cfmi_init_info.sigma_level_pos_diff = std::min(sigma_level_pos_diff_filtered, nees_cfmi_init_info.sigma_level_pos_diff);
      nees_cfmi_init_info.sigma_level_cloud = std::min(sigma_level_cloud_filtered, nees_cfmi_init_info.sigma_level_cloud);
   }

   /*===========================================================================*\
   * FUNCTION: CV_M_Estimator_Convergence_Check
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_next_iteration_needed
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_VCS_Velocity_T& prev_diff,
   * const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Convergence Check for M-estimator
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool CV_M_Estimator_Convergence_Check(const F360_Calibrations_T& calibrations,
      const F360_VCS_Velocity_T& prev_vel,
      const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      bool f_is_next_iteration_needed;


      f_is_next_iteration_needed = (nees_cfmi_init_info.VCS_vel_hyp.f_valid);
      if (f_is_next_iteration_needed)  // If data is not valid set the flag to false
      {
         F360_VCS_Velocity_T vel_diff;
         vel_diff.longitudinal = prev_vel.longitudinal - nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal;
         vel_diff.lateral = prev_vel.lateral - nees_cfmi_init_info.VCS_vel_hyp.vel.lateral;

         const float32_t vel_diff_square = (vel_diff.longitudinal * vel_diff.longitudinal) + (vel_diff.lateral * vel_diff.lateral);
         const float32_t max_vel_diff_square = calibrations.k_nees_cfmi_m_est_conv_check_max_vel_diff * calibrations.k_nees_cfmi_m_est_conv_check_max_vel_diff;

         f_is_next_iteration_needed = (vel_diff_square > max_vel_diff_square);

         if (f_is_next_iteration_needed) // If true check for  relative_vel_square_diff > max_relative_vel_diff_square
         {
            const float32_t min_vel_square = calibrations.k_nees_cfmi_m_est_conv_check_min_speed * calibrations.k_nees_cfmi_m_est_conv_check_min_speed;
            const float32_t prev_vel_square = (prev_vel.longitudinal * prev_vel.longitudinal) + (prev_vel.lateral * prev_vel.lateral);
            float32_t relative_vel_square_diff;

            if (prev_vel_square > min_vel_square)
            {
               relative_vel_square_diff = vel_diff_square / prev_vel_square;
            }
            else
            {
               relative_vel_square_diff = 1.0F; // In case of low velocity comparing relative one does not make sense
            }

            const float32_t max_relative_vel_diff_square = calibrations.k_nees_cfmi_m_est_conv_check_max_rel_vel_diff * calibrations.k_nees_cfmi_m_est_conv_check_max_rel_vel_diff;

            f_is_next_iteration_needed = (relative_vel_square_diff > max_relative_vel_diff_square);
         }
      }

      return f_is_next_iteration_needed;
   }

   /*===========================================================================*\
   * FUNCTION: CV_M_Estimator_Plausibility_Check
   *===========================================================================
   * RETURN VALUE:
   * bool f_plausible
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Plausibility checks for M-estimator.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool CV_M_Estimator_Plausibility_Check(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity)
   {
      // For stationary tracks position difference doesn't have to be valid
       const bool f_plausible =
          ((nees_cfmi_velocity.plausibility > calibrations.k_nees_cfmi_m_est_plaus_min_overall_plaus) ||
             ((nees_cfmi_velocity.plausibility_cloud > calibrations.k_nees_cfmi_m_est_plaus_min_cl_plaus) &&
                ((nees_cfmi_velocity.plausibility_pos_diff > calibrations.k_nees_cfmi_m_est_plaus_min_pd_plaus)))) &&
          (nees_cfmi_velocity.num_cloud >= nees_cfmi_information.min_dets_inliers_num) &&
          (nees_cfmi_velocity.num_pos_diff >= nees_cfmi_information.min_vels_inliers_num);
      return f_plausible;
   }
}
