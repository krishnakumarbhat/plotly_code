/*===================================================================================*\
* FILE: f360_nees_cfmi_helpers.cpp
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
*   Helpers function common for different nees cost function initialization modules
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

#include "f360_nees_cfmi_helpers.h"
#include "f360_math_func.h"
#include "f360_uncertainty_propagation.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Calc_Single_NEES_Value_For_Pos_Diff
   *===========================================================================
   * RETURN VALUE:
   * bool indication if velocity is valid
   *
   * PARAMETERS:
   * const F360_VCS_Velocity_T& ref_vel
   * const F360_VCS_Velocity_T& vel_hyp
   * float32_t const (&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * NEES cost value for single velocity estimation based on position difference
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Single_NEES_Value_For_Pos_Diff(
      const F360_VCS_Velocity_T& ref_vel,
      const F360_VCS_Velocity_T& vel_hyp,
      float32_t const (&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION])
   {
      float32_t nees_value;
      const float32_t determinant = F360_2d_Matrix_Determinant(vel_hyp_cov);

      const bool f_valid = (determinant > F360_MIN_DENOMINATOR);
      if (f_valid)
      {
         F360_VCS_Velocity_T vel_diff;
         vel_diff.longitudinal = ref_vel.longitudinal - vel_hyp.longitudinal;
         vel_diff.lateral = ref_vel.lateral - vel_hyp.lateral;

         nees_value = ((vel_diff.longitudinal * vel_diff.longitudinal * vel_hyp_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]) -
            (2.0F * vel_diff.longitudinal * vel_diff.lateral * vel_hyp_cov[F360_2D_IDX_X][F360_2D_IDX_Y]) +
            (vel_diff.lateral * vel_diff.lateral * vel_hyp_cov[F360_2D_IDX_X][F360_2D_IDX_X])) / determinant;
      }
      else
      {
         nees_value = 1.0F / F360_MIN_DENOMINATOR;
      }
      return nees_value;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Single_NEES_Value_For_Cloud
   *===========================================================================
   * RETURN VALUE:
   * float32_t NEES cost value
   *
   * PARAMETERS:
   *
   * const F360_VCS_Velocity_T& VCS_velocity,
   * const float32_t cos_azimuth,
   * const float32_t sin_azimuth,
   * const float32_t range_rate_compensated,
   * const float32_t range_rate_var
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * NEES Constant Velocity Cost function based on cloud
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Single_NEES_Value_For_Cloud(const F360_VCS_Velocity_T& VCS_velocity,
      const float32_t cos_azimuth,
      const float32_t sin_azimuth,
      const float32_t range_rate_compensated,
      const float32_t range_rate_var)
   {
      float32_t nees_value;

      if (range_rate_var > F360_MIN_DENOMINATOR)
      {
         const float32_t estimated_range_rate_error = VCS_velocity.longitudinal * cos_azimuth
            + VCS_velocity.lateral * sin_azimuth
            - range_rate_compensated;

         nees_value = (estimated_range_rate_error * estimated_range_rate_error) / range_rate_var;
      }
      else
      {
         nees_value = 1.0F / F360_MIN_DENOMINATOR;
      }

      return nees_value;
   }

   /*===========================================================================*\
   * FUNCTION: Add_Into_NEES_CFMI_Determinant
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
   * const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Add two NEES Cost determinant structures
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Add_Into_NEES_CFMI_Determinant(F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
      const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants)
   {
      destination_nees_cost_determinants.n_dets += source_nees_cost_determinants.n_dets;
      destination_nees_cost_determinants.information += source_nees_cost_determinants.information;
      destination_nees_cost_determinants.Sxx += source_nees_cost_determinants.Sxx;
      destination_nees_cost_determinants.Sxy += source_nees_cost_determinants.Sxy;
      destination_nees_cost_determinants.Sx += source_nees_cost_determinants.Sx;
      destination_nees_cost_determinants.Syy += source_nees_cost_determinants.Syy;
      destination_nees_cost_determinants.Syx += source_nees_cost_determinants.Syx;
      destination_nees_cost_determinants.Sy += source_nees_cost_determinants.Sy;
   }

   /*===========================================================================*\
   * FUNCTION: Add_Into_NEES_CFMI_Determinant_Weighted
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
   * const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants
   * const float32_t weight
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Add two NEES Cost determinant structures, weighted version
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Add_Into_NEES_CFMI_Determinant_Weighted(F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
      const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants,
      const float32_t weight)
   {
      destination_nees_cost_determinants.n_dets += source_nees_cost_determinants.n_dets;
      destination_nees_cost_determinants.information += source_nees_cost_determinants.information * weight;
      destination_nees_cost_determinants.Sxx += source_nees_cost_determinants.Sxx * weight;
      destination_nees_cost_determinants.Sxy += source_nees_cost_determinants.Sxy * weight;
      destination_nees_cost_determinants.Sx += source_nees_cost_determinants.Sx * weight;
      destination_nees_cost_determinants.Syy += source_nees_cost_determinants.Syy * weight;
      destination_nees_cost_determinants.Syx += source_nees_cost_determinants.Syx * weight;
      destination_nees_cost_determinants.Sy += source_nees_cost_determinants.Sy * weight;
   }


   /*===========================================================================*\
   * FUNCTION: Estimate_Plausibility_For_NEES_CFMI_Velocity
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Velocity_T& velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Estimate plausibility for given estimation.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Estimate_Plausibility_For_NEES_CFMI_Velocity(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Velocity_T& velocity)
   {

      // Dets.
      if (nees_cfmi_information.dets_num > 0U)
      {
         const float32_t dets_inlier_ratio = velocity.weight_dets_sum / static_cast<float32_t>(nees_cfmi_information.dets_num);
         float32_t max_dets_inliers_ratio = static_cast<float32_t>(nees_cfmi_information.init_dets_inliers_num) / static_cast<float32_t>(nees_cfmi_information.dets_num);
         max_dets_inliers_ratio *= calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio;
         max_dets_inliers_ratio *= calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale;

         const float32_t min_inliers_ratio = 0.0F;
         velocity.plausibility_cloud = F360_Linear_Equation_With_Saturation(dets_inlier_ratio,
            min_inliers_ratio, max_dets_inliers_ratio,
            F360_MIN_PROBABILITY, F360_MAX_PROBABILITY);
      }
      else
      {
         velocity.plausibility_cloud = F360_MAX_PROBABILITY;
      }

      // Position difference.
      if (nees_cfmi_information.vels_num > 0U)
      {
         const float32_t vels_inlier_ratio = velocity.weight_vels_sum / static_cast<float32_t>(nees_cfmi_information.vels_num);
         float32_t max_vels_inliers_ratio = static_cast<float32_t>(nees_cfmi_information.expected_vels_inliers_num) / static_cast<float32_t>(nees_cfmi_information.vels_num);
         max_vels_inliers_ratio *= calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio;
         max_vels_inliers_ratio *= calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale;

         const float32_t min_inliers_ratio = 0.0F;
         velocity.plausibility_pos_diff = F360_Linear_Equation_With_Saturation(vels_inlier_ratio,
            min_inliers_ratio, max_vels_inliers_ratio,
            F360_MIN_PROBABILITY, F360_MAX_PROBABILITY);
      }
      else
      {
         velocity.plausibility_pos_diff = F360_MAX_PROBABILITY;
      }

      if (nees_cfmi_information.dominant_velocity.f_valid)
      {
         velocity.plausibility_dominant_vel = velocity.weight_dominant_vel;
      }

      // Overall plausibility
      const float32_t position_based_plausability = std::max(velocity.plausibility_pos_diff, velocity.plausibility_dominant_vel);
      velocity.plausibility = velocity.plausibility_cloud * position_based_plausability;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Velocity_T& const velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate velocity covariance (if possible) based on velocity determinants
   * and update input data structure with calculated results
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(F360_NEES_CFMI_Velocity_T& velocity)
   {
      velocity.f_cov_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(velocity.determinants, velocity.vel_cov);

      if (!velocity.f_cov_valid)
      {
         velocity.f_valid = false;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Raw_Vel_Cov_From_Determinants
   *===========================================================================
   * RETURN VALUE:
   * bool - flag indicating if covariance was calculated
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Determinants_T& determinants - input structure with determinants
   * float32_t (&cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] - output array with calculated covariance
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate velocity covariance (if possible) based on determinants and fill up
   * the array (passed as second parameter) with the result
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants,
      float32_t (&out_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION])
   {
      bool f_valid;

      const float32_t V_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);

      if (V_determinant > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_V_determinant = 1.0F / V_determinant;
         out_cov[F360_2D_IDX_X][F360_2D_IDX_X] = determinants.Syy * one_over_V_determinant;
         out_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = -(determinants.Sxy * one_over_V_determinant);
         out_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = -(determinants.Syx * one_over_V_determinant);
         out_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = determinants.Sxx * one_over_V_determinant;
         f_valid = true;
      }
      else
      {
         f_valid = false;
      }
      return f_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Vel_From_Determinants
   *===========================================================================
   * RETURN VALUE:
   * bool f_valid
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Determinants_T& determinants,
   * F360_VCS_Velocity_T& VCS_velocity)
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate velocity (if possible) based on nees CFMI determinants
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Calc_Vel_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants,
      F360_VCS_Velocity_T& VCS_velocity)
   {
      bool f_valid;

      if (determinants.n_dets > 1U)
      {
         const float32_t V_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);
         if (V_determinant > F360_MIN_DENOMINATOR)
         {
            f_valid = true;
            VCS_velocity.longitudinal = ((determinants.Sx * determinants.Syy) - (determinants.Sxy * determinants.Sy)) / V_determinant;
            VCS_velocity.lateral = ((determinants.Sxx * determinants.Sy) - (determinants.Sx * determinants.Syx)) / V_determinant;
         }
         else
         {
            // Velocity estimation not possible.
            f_valid = false;
         }
      }
      else
      {
         // Velocity estimation not possible.
         f_valid = false;
      }
      return f_valid;
   }


   /*===========================================================================*\
   * FUNCTION: Calc_Vel_Determinant_From_NEES_CFMI_Determinants
   *===========================================================================
   * RETURN VALUE:
   * float32_t V_determinant
   *
   * PARAMETERS:
   * const F360_NEES_CFMI_Determinants_T& determinants
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate velocity determinant based on nees CFMI determinants
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Vel_Determinant_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants)
   {
      const float32_t V_determinant = (determinants.Sxx * determinants.Syy) - (determinants.Sxy * determinants.Syx);
      return V_determinant;
   }

   /*===========================================================================*\
* FUNCTION: Pos_Diff_Vel_Estimate
*===========================================================================
* RETURN VALUE:
* bool indication if velocity is valid
*
* PARAMETERS:
*
* const Point& point_A,
* const Point& point_B,
* const float32_t dt,
* F360_VCS_Velocity_T & velocity
*
* DEVIATIONS FROM STANDARDS:
* None.
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* Velocity estimation based on positions difference
*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
*
\*===========================================================================*/
   bool Pos_Diff_Vel_Estimate(
      const Point& point_A,
      const Point& point_B,
      const float32_t dt,
      F360_VCS_Velocity_T & velocity)
   {
      const bool f_valid = (std::abs(dt) > F360_MIN_DENOMINATOR);
      if (f_valid)
      {
         velocity.longitudinal = (point_A.x - point_B.x) / dt;
         velocity.lateral = (point_A.y - point_B.y) / dt;
      }
      return f_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Pos_Diff_Cov_Estimate
   *===========================================================================
   * RETURN VALUE:
   * bool indication if velocity is valid
   *
   * PARAMETERS:
   *
   * const float32_t point_A_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * const float32_t point_B_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * const float32_t dt,
   * float32_t output_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION])
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Covariance estimation for position difference based velocity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Pos_Diff_Cov_Estimate(
      const float32_t(&point_A_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t(&point_B_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t dt,
      float32_t(&output_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION])
   {
      const bool f_valid = (std::abs(dt) > F360_MIN_DENOMINATOR);
      if (f_valid)
      {
         const float32_t dt_constant = 1.0F / dt;
         Add_Uncertainty_2d(point_A_cov, point_B_cov, output_cov);
         Constant_Uncertainty_Propagation_2d(dt_constant, output_cov);
      }

      return f_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_NEES_CFMI_Determinants_Pos_Diff
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Velocity_T& single_vel
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate NEES cost determinants in case of position difference
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calc_NEES_CFMI_Determinants_Pos_Diff(F360_NEES_CFMI_Pos_Diff_Velocity_T& single_vel)
   {
      if (single_vel.determinant > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_determinant = 1.0F / single_vel.determinant;

         single_vel.pos_diff_determinants.n_dets = 1U;
         single_vel.pos_diff_determinants.Sxx = single_vel.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] * one_over_determinant;
         single_vel.pos_diff_determinants.Sxy = -(single_vel.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] * one_over_determinant);
         single_vel.pos_diff_determinants.Sx = ((single_vel.vel.longitudinal * single_vel.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]) -
            (single_vel.vel.lateral * single_vel.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y])) * one_over_determinant;
         single_vel.pos_diff_determinants.Syy = single_vel.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] * one_over_determinant;
         single_vel.pos_diff_determinants.Sy = ((single_vel.vel.lateral * single_vel.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X]) -
            (single_vel.vel.longitudinal * single_vel.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y]))  * one_over_determinant;

         single_vel.pos_diff_determinants.Syx = single_vel.pos_diff_determinants.Sxy;

         single_vel.pos_diff_determinants.information = single_vel.cov_trace * one_over_determinant; // Quick Fisher information calculation.
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_If_Cluster_Has_Any_Det_In_All_Looks
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Cluster_T& cluster_to_init,
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Detection_Hist_T& detection_hist
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check if at least one detection in cluster is in all looks.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Check_If_Cluster_Has_Any_Det_In_All_Looks(
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Hist_T& detection_hist
   )
   {
      bool is_cluster_in_all_looks = false;
      const uint32_t num_dets = static_cast<uint32_t>(cluster_to_init.ndets);
      for (uint32_t index = 0U; index < num_dets; index++)
      {
         const uint32_t det_index = static_cast<uint32_t>(cluster_to_init.detids[index]) - 1U;
         const rspp_variant_A::RSPP_Detection_T &detection = raw_detection_list.detections[det_index];
         const int32_t sensor_index = detection.raw.sensor_id - 1;

         const bool f_range_in_all_looks = (detection.raw.range < sensors[sensor_index].constant.range_limits[0]) &&
            (detection.raw.range < sensors[sensor_index].constant.range_limits[1]) &&
            (detection.raw.range < sensors[sensor_index].constant.range_limits[2]) &&
            (detection.raw.range < sensors[sensor_index].constant.range_limits[3]);

         if (f_range_in_all_looks)
         {
            is_cluster_in_all_looks = true;
            break;
         }
      }

      if (!is_cluster_in_all_looks)
      {
         const uint32_t num_old_dets = static_cast<uint32_t>(cluster_to_init.num_old_dets);
         for (uint32_t index = 0U; index < num_old_dets; index++)
         {
            const uint32_t det_index = static_cast<uint32_t>(cluster_to_init.old_det_idx[index]);
            const F360_Detection_Hist_Data_T& detection = detection_hist.det_data[det_index];
            if (detection.f_is_range_in_all_looks)
            {
               is_cluster_in_all_looks = true;
               break;
            }
         }
      }
      return is_cluster_in_all_looks;
   }
}
