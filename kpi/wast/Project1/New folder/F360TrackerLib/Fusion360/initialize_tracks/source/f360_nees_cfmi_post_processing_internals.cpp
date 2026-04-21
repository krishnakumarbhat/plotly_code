/*===================================================================================\
 * FILE: f360_nees_cfmi_post_processing_internals.cpp
 *====================================================================================
 * Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   Internal post-processing function for nees cost function initialization
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_post_processing_internals.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_NEES_CFMI_Init_Scheme
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate init scheme based on executed algorithms and velocity status
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_NEES_CFMI_Init_Scheme(F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      if (nees_cfmi_init_info.f_valid)
      {
         if (nees_cfmi_init_info.f_stationary)
         {
            nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_STATIONARY;
         }
         else if (nees_cfmi_init_info.f_ransac_valid)
         {
            if (nees_cfmi_init_info.f_m_estimator_valid)
            {
               nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_CV_RANSAC_M_ESTIMATOR;
            }
            else
            {
               nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_CV_RANSAC;
            }
         }
         else if (nees_cfmi_init_info.f_m_estimator_valid)
         {
            nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_CV_M_ESTIMATOR;
         }
         else
         {
            nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_CV_ANALYTIC;
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Velocity_Plausibility_Checks
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Set f_vel_plausible flag based on some sanity checks and plausibility measures.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Velocity_Plausibility_Checks(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity)
   {
      if (nees_cfmi_velocity.f_valid)
      {
         if (nees_cfmi_velocity.plausibility > calibrations.k_nees_cfmi_vel_plaus_thr)
         {
            nees_cfmi_velocity.f_plausible = true;
         }
         else
         {
            // Calculate expected plausibility (squared) for given covariance trace for higher covariance
            const float32_t expected_plausibility_squared = F360_Linear_Equation_With_Saturation(nees_cfmi_velocity.vel_cov_trace,
               calibrations.k_nees_cfmi_vel_plaus_cov_trace_low_plaus, calibrations.k_nees_cfmi_vel_plaus_cov_trace_high_plaus,
               calibrations.k_nees_cfmi_vel_plaus_cov_low_plaus * calibrations.k_nees_cfmi_vel_plaus_cov_low_plaus,
               calibrations.k_nees_cfmi_vel_plaus_cov_high_plaus * calibrations.k_nees_cfmi_vel_plaus_cov_high_plaus);

            const float32_t plausibility_squared = nees_cfmi_velocity.plausibility * nees_cfmi_velocity.plausibility;

            nees_cfmi_velocity.f_plausible = plausibility_squared > expected_plausibility_squared;
         }
      }
      else
      {
         nees_cfmi_velocity.f_plausible = false;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Initialization_Sanity_Checks
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Set f_fault_detected flag based on some sanity checks, invalidate init info
   * in case of detected fault.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Initialization_Sanity_Checks(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
      {
         // Valid speed check
         const float32_t max_valid_speed_squared = calibrations.k_nees_cfmi_vel_sanity_max_speed * calibrations.k_nees_cfmi_vel_sanity_max_speed;
         const float32_t speed_squared = (nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal * nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal) +
            (nees_cfmi_init_info.VCS_vel_hyp.vel.lateral * nees_cfmi_init_info.VCS_vel_hyp.vel.lateral);
         const bool f_speed_invalid = (speed_squared > max_valid_speed_squared);

         // Valid vel_cov check
         const bool f_vel_cov_invalid = ((0U == nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff) &&
            (calibrations.k_nees_cfmi_vel_sanity_max_vel_cov_trace < nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace));

         nees_cfmi_init_info.f_fault_detected = (f_speed_invalid || f_vel_cov_invalid);
      }
   }
}

