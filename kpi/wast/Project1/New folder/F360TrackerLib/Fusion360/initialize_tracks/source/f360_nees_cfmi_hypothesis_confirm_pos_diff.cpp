/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_confirm_pos_diff.cpp
*====================================================================================
* Copyright © 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains definitions of functions: Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_nees_cfmi_hypothesis_confirm_pos_diff.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"

namespace f360_variant_A
{
   static void Update_Confirm_Pos_Diff_Hyp(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity,
      F360_NEES_CFMI_Pos_Diff_Velocity_T & confirm_pos_diff_hyp);

   static void Calc_Num_Of_Pos_Diff_Hyp_For_Valid_Level(F360_NEES_CFMI_Information_T& nees_cfmi_information);

   static bool Check_Velocity_RR_Confidence_HIGH_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity);

   static bool Check_Velocity_RR_Confidence_HIGH_MEDIUM_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity);

   static bool Check_Velocity_RR_Confidence_LOW_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity);

   static void Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const Is_Velocity_RR_Confidence_Level_Correct_Predicate_F Is_Velocity_RR_Confidence_Level_Correct_Predicate,
      const F360_NEES_CFMI_Vel_RR_Conf_T rr_update_confidence_level,
      F360_NEES_CFMI_Pos_Diff_Velocity_T& confirm_pos_diff_hyp_vel);

   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate Velocity Hypothesis from confirmed position difference velocity hypotheses.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;

      if ((vels_num > 0U) && (vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL)) // preliminary condition
      {
         Calc_Num_Of_Pos_Diff_Hyp_For_Valid_Level(nees_cfmi_information);

         const uint32_t num_of_at_least_high = nees_cfmi_information.num_of_confirm_pos_diff_hyp_high;       // Number confirmed position difference VH with at least high confidence level.
         const uint32_t num_of_at_least_medium = num_of_at_least_high + nees_cfmi_information.num_of_confirm_pos_diff_hyp_med;  // Number confirmed position difference VH with at least medium confidence level.
         const uint32_t num_of_at_least_low = num_of_at_least_medium + nees_cfmi_information.num_of_confirm_pos_diff_hyp_low;   // Number confirmed position difference VH with at least low confidence level.

         const uint32_t min_possible_vel_num = calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate;
         const uint32_t expected_vels_inliers_num_thr = nees_cfmi_information.expected_vels_inliers_num - 1U;
         const uint32_t min_vel_num = (expected_vels_inliers_num_thr > min_possible_vel_num) ? expected_vels_inliers_num_thr : min_possible_vel_num;

         F360_NEES_CFMI_Pos_Diff_Velocity_T& confirm_pos_diff_hyp_vel = nees_cfmi_information.confirm_pos_diff_hyp_vel;

         // Check which condition should be used for velocity hypothesis calculation.
         if (num_of_at_least_high >= min_vel_num)
         {
            Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(nees_cfmi_information, &Check_Velocity_RR_Confidence_HIGH_Level, F360_NEES_CFMI_VEL_RR_CONF_HIGH, confirm_pos_diff_hyp_vel);
         }
         else if (num_of_at_least_medium >= min_vel_num)
         {
            Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(nees_cfmi_information, &Check_Velocity_RR_Confidence_HIGH_MEDIUM_Level, F360_NEES_CFMI_VEL_RR_CONF_MEDIUM, confirm_pos_diff_hyp_vel);
         }
         else if (num_of_at_least_low >= min_vel_num)
         {
            Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(nees_cfmi_information, &Check_Velocity_RR_Confidence_LOW_Level, F360_NEES_CFMI_VEL_RR_CONF_LOW, confirm_pos_diff_hyp_vel);
         }
         else if (num_of_at_least_low >= min_possible_vel_num) // Extra condition for very low number of detection.
         {
            Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(nees_cfmi_information, &Check_Velocity_RR_Confidence_LOW_Level, F360_NEES_CFMI_VEL_RR_CONF_INVALID, confirm_pos_diff_hyp_vel);
         }
         else
         {
            // Do nothing, there is no sufficient number of detections to calculate velocity hypothesis.
         }

         /* Estimate velocity only if number of velocity hypotheses is higher than one. */
         if (confirm_pos_diff_hyp_vel.pos_diff_determinants.n_dets >= min_possible_vel_num)
         {
            const float32_t one_over_n_dets = 1.0F / static_cast<float32_t>(confirm_pos_diff_hyp_vel.pos_diff_determinants.n_dets);
            bool f_vel_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(
               confirm_pos_diff_hyp_vel.pos_diff_determinants,
               confirm_pos_diff_hyp_vel.vel_cov);

            if (f_vel_valid)
            {
               f_vel_valid = Calc_Vel_From_NEES_CFMI_Determinants(
                  confirm_pos_diff_hyp_vel.pos_diff_determinants,
                  confirm_pos_diff_hyp_vel.vel);
            }

            confirm_pos_diff_hyp_vel.f_valid = f_vel_valid;
            confirm_pos_diff_hyp_vel.determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(confirm_pos_diff_hyp_vel.pos_diff_determinants);

            confirm_pos_diff_hyp_vel.time_since_meas = confirm_pos_diff_hyp_vel.time_since_meas * one_over_n_dets;
            confirm_pos_diff_hyp_vel.rr_plausbility = confirm_pos_diff_hyp_vel.rr_plausbility * one_over_n_dets;
            confirm_pos_diff_hyp_vel.dt = confirm_pos_diff_hyp_vel.dt * one_over_n_dets;

            confirm_pos_diff_hyp_vel.f_inlier = confirm_pos_diff_hyp_vel.f_valid;

            confirm_pos_diff_hyp_vel.cov_trace = Trace_Of_2d_Covariance(confirm_pos_diff_hyp_vel.vel_cov);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Update_Confirm_Pos_Diff_Hyp
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Velocity_T& single_velocity,
   * F360_NEES_CFMI_Velocity_T & confirm_pos_diff_hyp
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Update Confirmed Position Difference Hypothesis by single velocity hypothesis.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Confirm_Pos_Diff_Hyp(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity,
      F360_NEES_CFMI_Pos_Diff_Velocity_T& confirm_pos_diff_hyp)
   {
      Add_Into_NEES_CFMI_Determinant_Weighted(confirm_pos_diff_hyp.pos_diff_determinants,
         single_velocity.pos_diff_determinants,
         single_velocity.rr_plausbility);

      // Extra information for weighting.
      confirm_pos_diff_hyp.time_since_meas += single_velocity.time_since_meas;
      confirm_pos_diff_hyp.rr_plausbility += single_velocity.rr_plausbility;
      confirm_pos_diff_hyp.dt += single_velocity.dt;
   }

   /*===========================================================================*\
   * FUNCTION: Check_Velocity_RR_Confidence_HIGH_Level
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Checking if velocity's range rate confidence has particular level.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Check_Velocity_RR_Confidence_HIGH_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity)
   {
      return (single_velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_HIGH);
   }

   /*===========================================================================*\
   * FUNCTION: Check_Velocity_RR_Confidence_HIGH_MEDIUM_Level
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Checking if velocity's range rate confidence has particular level.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Check_Velocity_RR_Confidence_HIGH_MEDIUM_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity)
   {
      return ((single_velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_HIGH) || (single_velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_MEDIUM));
   }

   /*===========================================================================*\
   * FUNCTION: Check_Velocity_RR_Confidence_LOW_Level
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Checking if velocity's range rate confidence has particular level.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Check_Velocity_RR_Confidence_LOW_Level(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& single_velocity)
   {
      return (single_velocity.rr_confidence_level != F360_NEES_CFMI_VEL_RR_CONF_INVALID);
   }

   /*===========================================================================*\
   * FUNCTION: All_Velocities_Update_Confirm_Pos_Diff_Hyp
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information
   * const Is_Velocity_RR_Confidence_Level_Correct_Predicate_F Is_Velocity_RR_Confidence_Level_Correct_Predicate,
   * const F360_NEES_CFMI_Vel_RR_Conf_T rr_update_confidence_level
   * F360_NEES_CFMI_Pos_Diff_Velocity_T& confirm_pos_diff_hyp_vel
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Update Confirmed Position Difference Hypothesis for all velocity hypothesis.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Vel_Hypothesis_By_All_Pos_Diff_Vels(
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const Is_Velocity_RR_Confidence_Level_Correct_Predicate_F Is_Velocity_RR_Confidence_Level_Correct_Predicate,
      const F360_NEES_CFMI_Vel_RR_Conf_T rr_update_confidence_level,
      F360_NEES_CFMI_Pos_Diff_Velocity_T& confirm_pos_diff_hyp_vel)
   {
      for (uint32_t index = 0U; index < nees_cfmi_information.vels_num; index++)
      {
         if (Is_Velocity_RR_Confidence_Level_Correct_Predicate(nees_cfmi_information.velocities[index]))
         {
            Update_Confirm_Pos_Diff_Hyp(nees_cfmi_information.velocities[index], confirm_pos_diff_hyp_vel);
         }
      }
      confirm_pos_diff_hyp_vel.rr_confidence_level = rr_update_confidence_level;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Num_Of_Pos_Diff_Hyp_For_Valid_Level
   *===========================================================================
   * RETURN VALUE:
   * F360_NEES_CFMI_Vel_RR_Conf_T confidence level
   *
   * PARAMETERS:
   *
   *   F360_NEES_CFMI_Information_T *nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Update number of velocity hypotheses with corresponding confidence level.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Calc_Num_Of_Pos_Diff_Hyp_For_Valid_Level(F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      for (uint32_t index = 0U; index < vels_num; index++)
      {
         switch (nees_cfmi_information.velocities[index].rr_confidence_level)
         {
         case F360_NEES_CFMI_VEL_RR_CONF_LOW:
            nees_cfmi_information.num_of_confirm_pos_diff_hyp_low++;
            break;
         case F360_NEES_CFMI_VEL_RR_CONF_MEDIUM:
            nees_cfmi_information.num_of_confirm_pos_diff_hyp_med++;
            break;
         case F360_NEES_CFMI_VEL_RR_CONF_HIGH:
            nees_cfmi_information.num_of_confirm_pos_diff_hyp_high++;
            break;
         default:
            /* Do nothing.*/
            break;
         }
      }
   }
}
