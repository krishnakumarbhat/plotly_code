/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_radial.cpp
*====================================================================================
* Copyright © 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains definitions of functions: Get_NEES_CFMI_Information_Radial_VH().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#include "f360_nees_cfmi_hypothesis_radial.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information_Radial_VH
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
   * Calculate Velocity Hypothesis from detections based on radial motion assumption.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_NEES_CFMI_Information_Radial_VH(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t dets_num = nees_cfmi_information.dets_num;
      const bool number_of_dets_is_ok = (0U < dets_num) && (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET);

      if (number_of_dets_is_ok)
      {
         uint32_t n_dets = 0U;
         float32_t weight_sum = 0.0F;

         for (uint32_t index = 0U; index < dets_num; index++)
         {
            const F360_NEES_CFMI_Detection_T &detection = nees_cfmi_information.detections[index];
            const bool det_is_ok = (detection.f_inlier) && (detection.range_rate_comp_var > F360_MIN_DENOMINATOR);
            if (det_is_ok)
            {
               n_dets++;
               // Covariance as for cloud
               F360_NEES_CFMI_Pos_Diff_Velocity_T &radial_hyp_vel = nees_cfmi_information.radial_velocity;
               Add_Into_NEES_CFMI_Determinant(radial_hyp_vel.pos_diff_determinants,
                  detection.cloud_determinants);

               // Weighting by var
               const float32_t weight = 1.0F / detection.range_rate_comp_var;
               weight_sum += weight;
               radial_hyp_vel.vel.longitudinal += detection.cos_vcs_az * detection.range_rate_comp * weight;
               radial_hyp_vel.vel.lateral += detection.sin_vcs_az * detection.range_rate_comp * weight;
               radial_hyp_vel.time_since_meas += detection.time_since_meas;
            }
         }

         // Estimate velocity only if number of velocity hypotheses is higher than one
         const uint32_t min_dets_num = calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate;
         if ((n_dets > 0U) && (n_dets >= min_dets_num) && (weight_sum > F360_MIN_DENOMINATOR))
         {
            const float32_t one_over_n_dets = 1.0F / static_cast<float32_t>(n_dets);
            const float32_t one_over_weight_sum = 1.0F / weight_sum;

            F360_NEES_CFMI_Pos_Diff_Velocity_T &radial_hyp_vel = nees_cfmi_information.radial_velocity;
            const bool f_vel_cov_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(
               radial_hyp_vel.pos_diff_determinants,
               radial_hyp_vel.vel_cov);

            radial_hyp_vel.cov_trace = Trace_Of_2d_Covariance(radial_hyp_vel.vel_cov);

            radial_hyp_vel.vel.longitudinal *= one_over_weight_sum;
            radial_hyp_vel.vel.lateral *= one_over_weight_sum;

            radial_hyp_vel.f_valid = f_vel_cov_valid;
            radial_hyp_vel.determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(radial_hyp_vel.pos_diff_determinants);

            radial_hyp_vel.time_since_meas = radial_hyp_vel.time_since_meas * one_over_n_dets;
            radial_hyp_vel.f_inlier = radial_hyp_vel.f_valid;
         }
      }
   }
}
