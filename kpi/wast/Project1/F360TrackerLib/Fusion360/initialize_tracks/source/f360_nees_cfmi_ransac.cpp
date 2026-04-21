/*===================================================================================*\
* FILE: f360_nees_cfmi_ransac.cpp
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
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
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_ransac.h"

#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_inliers_check.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_random_number_generation.h"
#include "f360_uncertainty_propagation.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Get_CV_NEES_CFMI_RANSAC_Vel_Hyp
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& best_nees_cfmi_init_info
   * F360_NEES_CFMI_Velocity_T& best_nees_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Main RANSAC function
   *
   * PRECONDITIONS:
   * best_nees_velocity is empty
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_CV_NEES_CFMI_RANSAC_Vel_Hyp(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& best_nees_velocity)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t max_vel_index = vels_num - 1U;

      CV_RANSAC_Preconditions(calibrations, nees_cfmi_information, nees_cfmi_init_info);
      const uint32_t max_ransac_iteration = nees_cfmi_init_info.max_number_of_ransac_iterations;

      best_nees_velocity = F360_NEES_CFMI_Velocity_T();

      if ((max_vel_index < F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) && (max_ransac_iteration <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL))
      {
         if (vels_num > max_ransac_iteration)
         {
            // Classic RANSAC
            const uint32_t random_seed = 1U;
            Init_Random_Number_Generator(random_seed);

            for (uint32_t ransac_index = 0U; ransac_index < max_ransac_iteration; ransac_index++)
            {
               const uint32_t vels_num_index = Generate_Random_Index(max_vel_index);

               NEES_CFMI_CV_RANSAC_Iteration_With_Comparison(calibrations,
                  nees_cfmi_information,
                  nees_cfmi_information.velocities[vels_num_index].vel,
                  nees_cfmi_information.velocities[vels_num_index].vel_cov,
                  nees_cfmi_init_info.inliers,
                  best_nees_velocity);
            }
         }
         else
         {
            for (uint32_t vels_num_index = 0U; vels_num_index <= max_vel_index; vels_num_index++)
            {
               NEES_CFMI_CV_RANSAC_Iteration_With_Comparison(calibrations,
                  nees_cfmi_information,
                  nees_cfmi_information.velocities[vels_num_index].vel,
                  nees_cfmi_information.velocities[vels_num_index].vel_cov,
                  nees_cfmi_init_info.inliers,
                  best_nees_velocity);
            }
         }
         Estimate_Velocity_After_Ransac_Runs(calibrations, nees_cfmi_information, nees_cfmi_init_info, best_nees_velocity);

         nees_cfmi_init_info.f_ransac_valid = best_nees_velocity.f_valid;
      }

   }

   /*===========================================================================*\
   * FUNCTION: Estimate_Velocity_After_Ransac_Runs
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
   * F360_NEES_CFMI_Velocity_T& best_nees_velocity
   *
   * ABSTRACT:
   * Estimate velocity after RANSAC iterations.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Estimate_Velocity_After_Ransac_Runs(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& best_nees_velocity)
   {
      if (best_nees_velocity.f_valid)
      {
         const float32_t rr_var_extension = std::min(best_nees_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X],
            best_nees_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]);

         NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information,
            nees_cfmi_init_info.inliers,
            best_nees_velocity.vel,
            rr_var_extension,
            calibrations.k_nees_cfmi_vh_check_ransac_sl_pd,
            calibrations.k_nees_cfmi_vh_check_ransac_sl_cl,
            calibrations.k_nees_cfmi_vh_check_ransac_sl_do,
            calibrations.k_nees_cfmi_min_weight_for_inlier);

         best_nees_velocity = F360_NEES_CFMI_Velocity_T();
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, best_nees_velocity);
      }
   }

   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_CV_RANSAC_Iteration_With_Comparison
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_VCS_Velocity_T & const ref_vel,
   * const float32_t (&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * F360_NEES_CFMI_Inliers_T& inliers,
   * F360_NEES_CFMI_Velocity_T& best_nees_cfmi_velocity,
   *
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * RANSAC Iteration with comparison with best iteration
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_CV_RANSAC_Iteration_With_Comparison(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_VCS_Velocity_T & ref_vel,
      const float32_t(&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      F360_NEES_CFMI_Inliers_T & inliers,
      F360_NEES_CFMI_Velocity_T& best_nees_cfmi_velocity)
   {
      F360_NEES_CFMI_Velocity_T temp_nees_cfmi_velocity = {};

      temp_nees_cfmi_velocity.vel = ref_vel;
      temp_nees_cfmi_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] = ref_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      temp_nees_cfmi_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = ref_vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      temp_nees_cfmi_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = ref_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      temp_nees_cfmi_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = ref_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];
      temp_nees_cfmi_velocity.f_valid = true;

      const float32_t rr_var_extension = std::min(ref_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X],
         ref_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]);

      // Inliers check with RR extension
      NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information,
         inliers,
         ref_vel,
         rr_var_extension,
         calibrations.k_nees_cfmi_vh_check_ransac_sl_pd,
         calibrations.k_nees_cfmi_vh_check_ransac_sl_cl,
         calibrations.k_nees_cfmi_vh_check_ransac_sl_do,
         calibrations.k_nees_cfmi_min_weight_for_inlier);

      NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, inliers, temp_nees_cfmi_velocity);
      Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, temp_nees_cfmi_velocity);

      // Check if current estimation is better
      const bool f_is_current_estimate_better = Compare_CV_RANSAC_Iterations(calibrations, temp_nees_cfmi_velocity, best_nees_cfmi_velocity);
      if (f_is_current_estimate_better)
      {
         best_nees_cfmi_velocity = temp_nees_cfmi_velocity;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Compare_CV_RANSAC_Iterations
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_current_estimate_better
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Velocity_T& velocity
   * const F360_NEES_CFMI_Velocity_T& best_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check if current estimation is better than the best one
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Compare_CV_RANSAC_Iterations(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Velocity_T& velocity,
      const F360_NEES_CFMI_Velocity_T& best_velocity)
   {
      bool f_is_current_estimate_better;
      const float32_t plausibility_diff = velocity.plausibility - best_velocity.plausibility;
      const float32_t plausibility_diff_thr = calibrations.k_nees_cfmi_ransac_plausibility_diff_thr;

      if (plausibility_diff > plausibility_diff_thr)
      {
         f_is_current_estimate_better = true;
      }
      else if (plausibility_diff < (-plausibility_diff_thr))
      {
         f_is_current_estimate_better = false;
      }
      else
      {
         f_is_current_estimate_better = (velocity.information_pos_diff > best_velocity.information_pos_diff);
      }

      return f_is_current_estimate_better;
   }

   /*===========================================================================*\
   * FUNCTION: CV_RANSAC_Preconditions
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_inf
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   *
   * Calculate maximum number of RANSAC iterations based on (1-p) = (1-w^n)^k
   *  p - expected probability of successful estimation
   *  w - expected inliers ratio
   *  n - number of point needed to estimate object (here 1)
   *  k - number of RANSAC iterations
   *
   *  k = log(1-p) / log(1-w^n)
   *  https://en.wikipedia.org/wiki/Random_sample_consensus
   *  Calculate expected success probability in case that maximum number
   *  of iterations are reached.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void CV_RANSAC_Preconditions(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      const uint32_t min_ransac_iterations = calibrations.k_nees_cfmi_ransac_min_iterations;

      if (nees_cfmi_information.vels_num <= min_ransac_iterations)
      {
         nees_cfmi_init_info.max_number_of_ransac_iterations = nees_cfmi_information.vels_num;
      }
      else
      {
         // Calculate expected number of iterations based on assumed probability of success
         const float32_t denominator = F360_Safe_Logf(F360_MAX_PROBABILITY - nees_cfmi_information.expected_vels_inliers_ratio);

         if (std::abs(denominator) > F360_MIN_DENOMINATOR)
         {
            const float32_t expected_ransac_success = calibrations.k_nees_cfmi_ransac_expected_success;
            const float32_t numerator = F360_Safe_Logf(F360_MAX_PROBABILITY - expected_ransac_success);
            const uint32_t ransac_iterations = static_cast<uint32_t>(F360_Ceilf(numerator / denominator));

            // Check if estimated number of iteration is not higher that number of detections

            if (ransac_iterations < min_ransac_iterations)
            {
               // Better to use minimal number of iterations since that would provide better robustness
               nees_cfmi_init_info.max_number_of_ransac_iterations = min_ransac_iterations;

            }
            else if (ransac_iterations <= calibrations.k_nees_cfmi_ransac_max_iterations)
            {
               nees_cfmi_init_info.max_number_of_ransac_iterations = ransac_iterations;
            }
            else // Number of RANSAC iterations need to be saturated
            {
               nees_cfmi_init_info.max_number_of_ransac_iterations = calibrations.k_nees_cfmi_ransac_max_iterations;

            }
         }
         else
         {
            // Should not go there since input for log should always be below 1
            nees_cfmi_init_info.max_number_of_ransac_iterations = std::min(nees_cfmi_information.vels_num, calibrations.k_nees_cfmi_ransac_max_iterations);
         }
      }
   }
}
