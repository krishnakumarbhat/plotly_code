/*===================================================================================*\
* FILE: f360_nees_cfmi_inliers_check.cpp
*====================================================================================
* Copyright © 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential  Restricted Aptiv information. Do not disclose.
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
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/


#include "f360_nees_cfmi_inliers_check.h"

#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Inliers_T& inliers,
   * const F360_VCS_Velocity_T& ref_vel,
   * const float32_t rr_var_extension,
   * const float32_t sigma_level_pos_diff,
   * const float32_t sigma_level_cloud,
   * const float32_t min_weight
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Simple NEES based inliers check for given reference velocity,
   * position difference and cloud variance is taken into account,
   * additional range rate variance can be defined to extend variance for cloud nees.
   * Constant Velocity model is assumed.
   * Only inliers flag and inliers weight is updated.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t rr_var_extension,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight)
   {


      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) &&
         (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         float32_t nees_value;
         float32_t raw_weight;
         // Two loops to determine inliers
         for (uint32_t vel_index = 0U; vel_index < vels_num; vel_index++)
         {
            const float32_t max_nees_value_pos_diff = sigma_level_pos_diff * sigma_level_pos_diff;
            const  F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.velocities[vel_index];
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, current_nees_vel.vel_cov);
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_pos_diff);

            inliers.vel_weights[vel_index] = raw_weight;
            inliers.f_vels_valid[vel_index] = (raw_weight > min_weight);
         }
         for (uint32_t det_index = 0U; det_index < dets_num; det_index++)
         {
            const F360_NEES_CFMI_Detection_T& current_nees_det = nees_cfmi_information.detections[det_index];
            nees_value = Calc_Single_NEES_Value_For_Cloud(ref_vel,
               current_nees_det.cos_vcs_az, current_nees_det.sin_vcs_az,
               current_nees_det.range_rate_comp, current_nees_det.range_rate_comp_var + rr_var_extension);
            const float32_t max_nees_value_cloud = sigma_level_cloud * sigma_level_cloud;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_cloud);

            inliers.det_weights[det_index] = raw_weight;
            inliers.f_dets_valid[det_index] = (raw_weight > min_weight);
         }

         if (nees_cfmi_information.dominant_velocity.f_valid)
         {
            const  F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.dominant_velocity;
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, current_nees_vel.vel_cov);
            const float32_t max_nees_value_dominant_vel_diff = sigma_level_dominant_vel * sigma_level_dominant_vel;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_dominant_vel_diff);
            inliers.dominant_vel_weight = raw_weight;
            inliers.f_dominant_vel_valid = (raw_weight > min_weight);
         }

      }
   }

   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Simple_NEES_Inliers_Check
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Inliers_T& inliers,
   * const F360_VCS_Velocity_T& ref_vel,
   * const float32_t sigma_level_pos_diff,
   * const float32_t sigma_level_cloud,
   * const float32_t min_weight
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Simple NEES based inliers check for given reference velocity,
   * position difference and cloud variance is taken into account
   * but reference velocity uncertainty not.
   * Constant Velocity model is assumed.
   * Only inliers flag and inliers weight is updated.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_CV_Simple_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) &&
         (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         float32_t nees_value;
         float32_t raw_weight;
         // Two loops to determine inliers
         for (uint32_t vel_index = 0U; vel_index < vels_num; vel_index++)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.velocities[vel_index];
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, current_nees_vel.vel_cov);

            const float32_t max_nees_value_pos_diff = sigma_level_pos_diff * sigma_level_pos_diff;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_pos_diff);

            inliers.vel_weights[vel_index] = raw_weight;
            inliers.f_vels_valid[vel_index] = (raw_weight > min_weight);
         }
         for (uint32_t det_index = 0U; det_index < dets_num; det_index++)
         {
            const F360_NEES_CFMI_Detection_T& current_nees_det = nees_cfmi_information.detections[det_index];
            nees_value = Calc_Single_NEES_Value_For_Cloud(ref_vel, current_nees_det.cos_vcs_az, current_nees_det.sin_vcs_az, current_nees_det.range_rate_comp, current_nees_det.range_rate_comp_var);

            const float32_t max_nees_value_cloud = sigma_level_cloud * sigma_level_cloud;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_cloud);

            inliers.det_weights[det_index] = raw_weight;
            inliers.f_dets_valid[det_index] = (raw_weight > min_weight);
         }

         if (nees_cfmi_information.dominant_velocity.f_valid)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.dominant_velocity;
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, current_nees_vel.vel_cov);
            const float32_t max_nees_value_dominant_vel_diff = sigma_level_dominant_vel * sigma_level_dominant_vel;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_dominant_vel_diff);
            inliers.dominant_vel_weight = raw_weight;
            inliers.f_dominant_vel_valid = (raw_weight > min_weight);
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Inliers_T& inliers,
   * const F360_VCS_Velocity_T& ref_vel,
   * const float32_t(&constant_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * const float32_t constant_rr_var,
   * const float32_t sigma_level_pos_diff,
   * const float32_t sigma_level_cloud,
   * const float32_t min_weight
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * NEES based inliers check for given reference velocity,
   * and given constant velocity covariance for position difference nees check
   * and constant range rate variance for range rates check.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t(&constant_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t constant_rr_var,
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) &&
         (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         float32_t nees_value;
         float32_t raw_weight;
         // Two loops to determine inliers.
         for (uint32_t vel_index = 0U; vel_index < vels_num; vel_index++)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.velocities[vel_index];
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, constant_vel_cov);
            const float32_t max_nees_value_pos_diff = sigma_level_pos_diff * sigma_level_pos_diff;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_pos_diff);
            inliers.vel_weights[vel_index] = raw_weight;
            inliers.f_vels_valid[vel_index] = (raw_weight > min_weight);
         }
         for (uint32_t det_index = 0U; det_index < dets_num; det_index++)
         {
            const F360_NEES_CFMI_Detection_T& current_nees_det = nees_cfmi_information.detections[det_index];
            nees_value = Calc_Single_NEES_Value_For_Cloud(ref_vel, current_nees_det.cos_vcs_az, current_nees_det.sin_vcs_az,
               current_nees_det.range_rate_comp, constant_rr_var);
            const float32_t max_nees_value_cloud = sigma_level_cloud * sigma_level_cloud;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_cloud);
            inliers.det_weights[det_index] = raw_weight;
            inliers.f_dets_valid[det_index] = (raw_weight > min_weight);
         }

         if (nees_cfmi_information.dominant_velocity.f_valid)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.dominant_velocity;
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, constant_vel_cov);
            const float32_t max_nees_value_dominant_vel_diff = sigma_level_dominant_vel * sigma_level_dominant_vel;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_dominant_vel_diff);
            inliers.dominant_vel_weight = raw_weight;
            inliers.f_dominant_vel_valid  = (raw_weight > min_weight);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_CV_Full_NEES_Inliers_Check
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Inliers_T& inliers,
   * const F360_VCS_Velocity_T&  ref_vel,
   * const float32_t (&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * const float32_t sigma_level_pos_diff,
   * const float32_t sigma_level_cloud,
   * const float32_t min_weight)
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Full NEES based inliers check for given reference velocity,
   * position difference and cloud variance is taken into account
   * and reference velocity uncertainty as well.
   * Constant Velocity model is assumed.
   * Only inliers flag and inliers weight is updated.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_CV_Full_NEES_Inliers_Check(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Inliers_T& inliers,
      const F360_VCS_Velocity_T& ref_vel,
      const float32_t (&ref_vel_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t sigma_level_pos_diff,
      const float32_t sigma_level_cloud,
      const float32_t sigma_level_dominant_vel,
      const float32_t min_weight)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) &&
         (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         float32_t nees_value;
         float32_t raw_weight;
         float32_t pos_diff_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
         // Two loops to determine inliers.
         for (uint32_t vel_index = 0U; vel_index < vels_num; vel_index++)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.velocities[vel_index];
            Add_Uncertainty_2d(ref_vel_cov, current_nees_vel.vel_cov, pos_diff_cov);
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, pos_diff_cov);

            const float32_t max_nees_value_pos_diff = sigma_level_pos_diff * sigma_level_pos_diff;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_pos_diff);

            inliers.vel_weights[vel_index] = raw_weight;
            inliers.f_vels_valid[vel_index] = (raw_weight > min_weight);
         }
         for (uint32_t det_index = 0U; det_index < dets_num; det_index++)
         {
            const F360_NEES_CFMI_Detection_T& current_nees_det = nees_cfmi_information.detections[det_index];

            const float32_t range_rate_ref_var = Vel_Cov_2_Range_Rate_Var(ref_vel_cov, current_nees_det.cos_vcs_az, current_nees_det.sin_vcs_az);
            nees_value = Calc_Single_NEES_Value_For_Cloud(ref_vel, current_nees_det.cos_vcs_az, current_nees_det.sin_vcs_az,
               current_nees_det.range_rate_comp, current_nees_det.range_rate_comp_var + range_rate_ref_var);

            const float32_t max_nees_value_cloud = sigma_level_cloud * sigma_level_cloud;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_cloud);

            inliers.det_weights[det_index] = raw_weight;
            inliers.f_dets_valid[det_index] = (raw_weight > min_weight);
         }

         if (nees_cfmi_information.dominant_velocity.f_valid)
         {
            const F360_NEES_CFMI_Pos_Diff_Velocity_T& current_nees_vel = nees_cfmi_information.dominant_velocity;
            nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel.vel, pos_diff_cov);
            const float32_t max_nees_value_dominant_vel_diff = sigma_level_dominant_vel * sigma_level_dominant_vel;
            raw_weight = F360_Bisquare_Weight(nees_value, max_nees_value_dominant_vel_diff);
            inliers.dominant_vel_weight = raw_weight;
            inliers.f_dominant_vel_valid = (raw_weight > min_weight);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_NEES_CFMI_Inliers_T& inliers,
   * F360_NEES_CFMI_Velocity_T& velocity,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Accumulate inliers number, weights and information
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Inliers_T& inliers,
      F360_NEES_CFMI_Velocity_T& velocity)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL) &&
         (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         float32_t raw_weight;
         // Two loops to determine inliers.
         for (uint32_t vel_index = 0U; vel_index < vels_num; vel_index++)
         {

            if (inliers.f_vels_valid[vel_index])
            {
               // Inlier.
               raw_weight = inliers.vel_weights[vel_index];

               velocity.num_pos_diff++;
               velocity.num_all++;
               velocity.weight_sum += raw_weight;
               velocity.weight_vels_sum += raw_weight;
               velocity.information_pos_diff += raw_weight * nees_cfmi_information.velocities[vel_index].pos_diff_determinants.information;
            }
         }
         for (uint32_t det_index = 0U; det_index < dets_num; det_index++)
         {
            if (inliers.f_dets_valid[det_index])
            {
               // Inlier.
               raw_weight = inliers.det_weights[det_index];

               velocity.num_cloud++;
               velocity.num_all++;
               velocity.weight_sum += raw_weight;
               velocity.weight_dets_sum += raw_weight;
               velocity.information_cloud += raw_weight * nees_cfmi_information.detections[det_index].cloud_determinants.information;
            }
         }
         velocity.weight_dominant_vel = inliers.dominant_vel_weight;
      }
   }
}


