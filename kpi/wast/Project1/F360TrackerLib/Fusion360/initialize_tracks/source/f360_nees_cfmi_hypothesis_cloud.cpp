/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_cloud.cpp
*====================================================================================
* Copyright © 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains definitions of functions: Get_NEES_CFMI_Information_Cloud_VH().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#include "f360_nees_cfmi_hypothesis_cloud.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"

namespace f360_variant_A
{
/*===========================================================================*\
* FUNCTION: Get_NEES_CFMI_Information_Cloud_VH
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
* Calculate Velocity Hypothesis from detections based on cloud algorithm.
*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
*
\*===========================================================================*/
   void Get_NEES_CFMI_Information_Cloud_VH(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t dets_num = nees_cfmi_information.dets_num;
      const bool number_of_dets_is_ok = (0U < dets_num) && (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET);
      if (number_of_dets_is_ok)
      {
         F360_NEES_CFMI_Pos_Diff_Velocity_T& cloud_hyp_vel = nees_cfmi_information.cloud_hyp_vel;
         // Check which condition should be used for velocity hypothesis calculation
         for (uint32_t index = 0U; index < dets_num; index++)
         {
            if (nees_cfmi_information.detections[index].f_inlier)
            {
               Add_Into_NEES_CFMI_Determinant(cloud_hyp_vel.pos_diff_determinants,
                  nees_cfmi_information.detections[index].cloud_determinants);

               cloud_hyp_vel.time_since_meas += nees_cfmi_information.detections[index].time_since_meas;
            }
         }

         // Estimate velocity only if number of velocity hypotheses is higher than one
         const uint32_t min_dets_num = calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate;
         const bool number_of_dets_is_sufficient = cloud_hyp_vel.pos_diff_determinants.n_dets >= min_dets_num;
         if (number_of_dets_is_sufficient)
         {
            const float32_t one_over_n_dets = 1.0F / static_cast<float32_t>(cloud_hyp_vel.pos_diff_determinants.n_dets);

            bool f_vel_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(
               cloud_hyp_vel.pos_diff_determinants,
               cloud_hyp_vel.vel_cov);

            if (f_vel_valid)
            {
               f_vel_valid = Calc_Vel_From_NEES_CFMI_Determinants(
                  cloud_hyp_vel.pos_diff_determinants,
                  cloud_hyp_vel.vel);
            }

            cloud_hyp_vel.cov_trace = Trace_Of_2d_Covariance(cloud_hyp_vel.vel_cov);

            const float32_t max_cov_std = F360_KPH2MPS(calibrations.k_nees_cfmi_cloud_hyp_max_vel_std_kph);
            const float32_t max_cov_trace = max_cov_std * max_cov_std;
            const bool cov_trace_is_ok = cloud_hyp_vel.cov_trace < max_cov_trace;

            const bool hyp_is_valid = (f_vel_valid && cov_trace_is_ok);
            cloud_hyp_vel.f_valid = hyp_is_valid;
            cloud_hyp_vel.determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(cloud_hyp_vel.pos_diff_determinants);

            cloud_hyp_vel.time_since_meas = cloud_hyp_vel.time_since_meas * one_over_n_dets;
            cloud_hyp_vel.f_inlier = cloud_hyp_vel.f_valid;
         }
      }
   }

}
