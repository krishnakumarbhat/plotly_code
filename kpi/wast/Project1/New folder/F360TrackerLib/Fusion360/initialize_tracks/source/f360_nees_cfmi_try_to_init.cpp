/*===========================================================================*\
* FILE: f360_nees_cfmi_try_to_init.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   Functions for NEES cost function minimization initialization - try to initialize object
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_try_to_init.h"

#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_hypotheses_selector.h"
#include "f360_nees_cfmi_inliers_check.h"
#include "f360_nees_cfmi_information_processing.h"
#include "f360_nees_cfmi_m_estimator.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_nees_cfmi_post_processing.h"
#include "f360_nees_cfmi_ransac.h"
#include "f360_nees_cfmi_stationary_checks.h"
#include "f360_nees_cfmi_vel_hyp_source.h"
#include "f360_uncertainty_propagation.h"
#include "f360_mark_for_liberal_tracking.h"
#include "f360_host.h"
#include "f360_math_func.h"
#include "f360_detection_history_time_slots_processing.h"
#include "f360_nees_cfmi_info_pos_diff.h"

namespace f360_variant_A
{

   static void Get_NEES_CFMI_VH_For_Multi_Time_Instance_Path(const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   static void Get_NEES_CFMI_VH_For_One_Time_Instance_Path(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);


/*===========================================================================*\
   * FUNCTION: Is_Not_Overlapping_With_Host
   *===========================================================================
   * RETURN VALUE:
   * bool f_success
   *
   * PARAMETERS:
   * const Point vcs_pos
   * const F360_Host_T& host
   *
   * EXTERNAL REFERENCES: tryToInitObjTrk.m
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * constructor
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Is_Not_Overlapping_With_Host(
      const Point vcs_pos,
      const F360_Host_T& host)
   {
      const float32_t min_dist_from_host = 1.5F;

      const float32_t half_host_dist_rear_axle_to_vcs_m = host.dist_rear_axle_to_vcs_m * 0.5F;

      const bool f_success = (std::abs(vcs_pos.x + half_host_dist_rear_axle_to_vcs_m) > half_host_dist_rear_axle_to_vcs_m) ||
                               (std::abs(vcs_pos.y) > min_dist_from_host);
      return  f_success;
   }
   /*===========================================================================*\
    * FUNCTION: Calc_Frac_Of_Az_Error_Stat_Mov
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    *
    * const F360_Cluster_T& cluster,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
    * const F360_Detection_Hist_T& detection_hist,
    * const RSPP_Detection_List_T& raw_detect_list
    *
    * EXTERNAL REFERENCES:
    * None.
    *
    * DEVIATIONS FROM STANDARDS:
    * None.
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * Calc_Frac_Of_Az_Error_Stat_Mov function
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
   \*===========================================================================*/
   static float32_t Calc_Frac_Of_Az_Error_Stat_Mov(
      const F360_Cluster_T& cluster,
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list)
   {
      float32_t frac_f_az_error;

      if ((cluster.ndets + cluster.num_old_dets) > 0)
      {
         int16_t cnt_f_az_error = 0;
         for (int16_t index = 0; index < cluster.ndets; index++)
         {
            const int16_t det_idx = cluster.detids[index] - 1;
            if (raw_detect_list.detections[det_idx].processed.f_azimuth_error_stat_mov)
            {
               cnt_f_az_error++;
            }
         }

         for (int16_t index = 0; index < cluster.num_old_dets; index++)
         {
            const int16_t det_idx = cluster.old_det_idx[index];
            if (detection_hist.det_data[det_idx].f_azimuth_error_stat_mov)
            {
               cnt_f_az_error++;
            }
         }
         const int16_t overall_number_of_dets = cluster.ndets + cluster.num_old_dets;
         frac_f_az_error = static_cast<float32_t>(cnt_f_az_error) / static_cast<float32_t>(overall_number_of_dets);
      }
      else
      {
         frac_f_az_error = 0.0F;
      }
      return frac_f_az_error;
   }


 /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Cov_Higher_Eig_Vectors_Sin_Cos
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&VCS_vel_cov_raw)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   * float32_t& sin_heva
   * float32_t& cos_heva
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * https://en.wikipedia.org/wiki/Eigenvalue_algorithm
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void NEES_CFMI_Cov_Higher_Eig_Vectors_Sin_Cos(
      const float32_t(&VCS_vel_cov_raw)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      float32_t& sin_heva,
      float32_t& cos_heva)
   {
      const float32_t cov_trace = Trace_Of_2d_Covariance(VCS_vel_cov_raw);
      const float32_t cov_det = F360_2d_Matrix_Determinant(VCS_vel_cov_raw);
      float32_t higher_eig;
      float32_t gap;
      float32_t higher_eig_vector_x;
      float32_t higher_eig_vector_y;
      float32_t higher_eig_vector_magnitude;

      // Calculate eigenvalue values
      gap = (cov_trace * cov_trace) - (4.0F * cov_det);
      if (gap > F360_EPSILON)
      {
         higher_eig = (cov_trace + F360_Sqrtf(gap)) * 0.5F;
      }
      else
      {
         // Safe state
         higher_eig = std::abs(cov_trace * 0.5F);
      }

      higher_eig_vector_x = VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_Y];
      higher_eig_vector_y = higher_eig - VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_X];

      // Only sinus and cosine is needed so it can be calculated based on magnitude
      higher_eig_vector_magnitude = F360_Get_Hypotenuse(higher_eig_vector_x, higher_eig_vector_y);

      // Bias definition and conversion from eigenvalue matrix to VCS (rotate by -angle)
      if (higher_eig_vector_magnitude > F360_MIN_DENOMINATOR)
      {
         sin_heva = higher_eig_vector_y / higher_eig_vector_magnitude;
         cos_heva = higher_eig_vector_x / higher_eig_vector_magnitude;
      }
      else
      {
         sin_heva = 0.0F;
         cos_heva = 0.0F;
      }
   }

/*===========================================================================*\
   * FUNCTION: NEES_CFMI_Estimate_Velocity_Covariance_Bias
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations
   * const float32_t (&VCS_vel_cov_raw)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   * const float32_t sin_heva
   * const float32_t cos_heva
   * float32_t (&bias_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Estimate velocity covariance bias.
   * Estimated velocity is for sure biased due to error in variable of cloud algorithm
   * For this source bias is more significant for cross radial component.
   * Cross radial component corresponds to higher uncertainty component in covariance
   * matrix after eigenvalue decomposition (Smaller uncertainty corresponds to radial unc.)
   * Higher eigenvalue and eigenvector for higher eigenvalue need to be calculated to find
   * covariance matrix angle so bias can be defined for eigenvalue (diagonal) matrix and after
   * that rotated into VCS.
   * https://en.wikipedia.org/wiki/Eigenvalue_algorithm
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void NEES_CFMI_Estimate_Velocity_Covariance_Bias(
      const F360_Calibrations_T& calibrations,
      const float32_t (&VCS_vel_cov_raw)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t sin_heva,
      const float32_t cos_heva,
      float32_t (&bias_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION])
   {
      // Initialize cov matrices
      float32_t prop_bias_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
      float32_t eig_bias_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
      float32_t rotation_matrix[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};

      prop_bias_cov[F360_2D_IDX_X][F360_2D_IDX_X] = VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_X];
      prop_bias_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_Y];
      prop_bias_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = VCS_vel_cov_raw[F360_2D_IDX_Y][F360_2D_IDX_X];
      prop_bias_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = VCS_vel_cov_raw[F360_2D_IDX_Y][F360_2D_IDX_Y];

      // Bias definition and conversion from eigenvalue matrix to VCS (rotate by -angle)
      eig_bias_cov[F360_2D_IDX_X][F360_2D_IDX_X] =
         calibrations.k_nees_cfmi_vel_cov_biger_eig_bias * calibrations.k_nees_cfmi_vel_cov_biger_eig_bias;
      eig_bias_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] =
         calibrations.k_nees_cfmi_vel_cov_lower_eig_bias * calibrations.k_nees_cfmi_vel_cov_lower_eig_bias;

      rotation_matrix[F360_2D_IDX_X][F360_2D_IDX_X] = cos_heva;
      rotation_matrix[F360_2D_IDX_X][F360_2D_IDX_Y] = -sin_heva;
      rotation_matrix[F360_2D_IDX_Y][F360_2D_IDX_X] = sin_heva;
      rotation_matrix[F360_2D_IDX_Y][F360_2D_IDX_Y] = cos_heva;

      Uncertainty_Propagation_2d(rotation_matrix, eig_bias_cov, bias_cov);

      // Add proportional part
      Constant_Uncertainty_Propagation_2d(calibrations.k_nees_cfmi_vel_cov_bias_factor, prop_bias_cov);

      Add_Into_Uncertainty_2d(prop_bias_cov, bias_cov);
   }

   /*===========================================================================*\
   * FUNCTION: Get_Plausibility_Scale
   *===========================================================================
   * RETURN VALUE:
   * float32_t plausibility scale
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Init_Info_T& nees_cfmi_information,
   *
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Post-processing of velocity information
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
  float32_t Get_Plausibility_Scale(const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      float32_t plausibility_scale;

      if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
      {
         // Extend variance based on plausibility
         // For stationary hypothesis use only cloud, for the rest use overall plausibility
         if (nees_cfmi_init_info.f_stationary_hyp_valid)
         {
            if (nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud > F360_MIN_DENOMINATOR)
            {
               plausibility_scale = 1.0F / nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud;
            }
            else
            {
               plausibility_scale = 1.0F / F360_MIN_DENOMINATOR;
            }
         }
         else
         {
            if (nees_cfmi_init_info.VCS_vel_hyp.plausibility > F360_MIN_DENOMINATOR)
            {
               plausibility_scale = 1.0F / nees_cfmi_init_info.VCS_vel_hyp.plausibility;
            }
            else
            {
               plausibility_scale = 1.0F / F360_MIN_DENOMINATOR;
            }
         }
      }
      else
      {
         plausibility_scale = 1.0F;
      }
      return plausibility_scale;
   }

/*===========================================================================*\
   * FUNCTION: NEES_CFMI_Estimate_Velocity_Covariance
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations
   * const F360_Host_Props_T& host_props
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Estimate velocity covariance
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void NEES_CFMI_Estimate_Velocity_Covariance(
      const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {

      // Calculate covariance
      Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(nees_cfmi_init_info.VCS_vel_hyp);

      float32_t VCS_vel_cov_raw[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
      float32_t sin_higher_eig_raw_cov;
      float32_t cos_higher_eig_raw_cov;
      VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      VCS_vel_cov_raw[F360_2D_IDX_X][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      VCS_vel_cov_raw[F360_2D_IDX_Y][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      VCS_vel_cov_raw[F360_2D_IDX_Y][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];
      NEES_CFMI_Cov_Higher_Eig_Vectors_Sin_Cos(VCS_vel_cov_raw, sin_higher_eig_raw_cov, cos_higher_eig_raw_cov);

      // If velocity valid modify uncertainty else keep init one
      if (nees_cfmi_init_info.VCS_vel_hyp.f_valid == true)
      {
         // Extend covariance based on plausibility
         const float32_t plausibility_scale = Get_Plausibility_Scale(nees_cfmi_init_info);
         Constant_Uncertainty_Propagation_2d(plausibility_scale, nees_cfmi_init_info.VCS_vel_hyp.vel_cov);

         // Biases
         float32_t bias_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
         float32_t host_influence_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
         NEES_CFMI_Estimate_Velocity_Covariance_Bias(calibrations, VCS_vel_cov_raw,
            sin_higher_eig_raw_cov, cos_higher_eig_raw_cov, bias_cov);
         host_influence_cov[F360_2D_IDX_X][F360_2D_IDX_X] = host_props.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
         host_influence_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = host_props.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
         host_influence_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = host_props.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
         host_influence_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = host_props.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];
         Constant_Uncertainty_Propagation_2d(calibrations.k_nees_cfmi_vel_cov_host_inf_factor, host_influence_cov);

         Add_Into_Uncertainty_2d(bias_cov, nees_cfmi_init_info.VCS_vel_hyp.vel_cov);
         Add_Into_Uncertainty_2d(host_influence_cov, nees_cfmi_init_info.VCS_vel_hyp.vel_cov);
      }
   }
/*===========================================================================*\
   * FUNCTION:Initialize_Stationary_Vel
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *  const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   *  F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Initialize velocity and covariance with default stationary value
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Initialize_Stationary_Vel(
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
         nees_cfmi_init_info.f_stationary = true;
         nees_cfmi_init_info.VCS_vel_hyp.vel = stationary_velocity.vel;
         nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] = stationary_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
         nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = stationary_velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
         nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = stationary_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
         nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = stationary_velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];

         nees_cfmi_init_info.VCS_vel_hyp.plausibility = F360_MAX_PROBABILITY;
         nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud = F360_MAX_PROBABILITY;
         nees_cfmi_init_info.VCS_vel_hyp.plausibility_pos_diff = F360_MAX_PROBABILITY;

   }

/*===========================================================================*\
   * FUNCTION:Get_NEES_CFMI_Vel_Hypo
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
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
   * Initialize velocity and covariance with default stationary value
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Get_NEES_CFMI_Vel_Hypo(const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      if (nees_cfmi_information.vels_num > 0U)
      {
         Get_NEES_CFMI_VH_For_Multi_Time_Instance_Path(calibrations, nees_cfmi_information, nees_cfmi_init_info);
      }
      else
      {
         Get_NEES_CFMI_VH_For_One_Time_Instance_Path(nees_cfmi_information, nees_cfmi_init_info);
      }

   }

/*===========================================================================*\
   * FUNCTION: Prepare_Info_NEES_CFMI
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   *  const F360_Calibrations_T& calibrations,
   *  const F360_Cluster_T& cluster_to_init,
   *  const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   *  const RSPP_Detection_List_T& raw_detection_list,
   *  const F360_Detection_Hist_T& detection_hist,
   *  F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info,
   *  F360_NEES_CFMI_Information_T nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Extract the info from dets and host, prepare for the estimation
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Prepare_Info_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Hist_T& detection_hist,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Information_T& nees_cfmi_information
      )
   {
      Get_NEES_CFMI_Information(calibrations, cluster_to_init, det_props, detection_hist, raw_detection_list, sensors, nees_cfmi_information);
      Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   }

/*===========================================================================*\
   * FUNCTION: Estimation_NEES_CFMI
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   *  const F360_Calibrations_T& calibrations,
   *  const F360_Host_Props_T& host_props,
   *  const F360_Host_T& host,
   *  const F360_Cluster_T& cluster_to_init,
   *  const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   *  F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info,
   *  F360_NEES_CFMI_Information_T nees_cfmi_information,
   *  F360_Tracked_Object_Init_Info_T& init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Estimate the velocity, covariance, size and related flags for different hypothesis
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Estimation_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      const F360_Host_T& host,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Tracked_Object_Init_Info_T& init_info)
   {
      //TODO: DEX-3259 liberal to be removed
      const bool cluster_valid_for_liberal_tracking = Check_Cluster_For_Liberal_Tracking(calibrations, host, det_props, cluster_to_init);


      // stationary hypothesis.
      if (nees_cfmi_init_info.f_stationary_hyp &&  Check_Stationary_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info))
      {

         // set stationary velocity
         Initialize_Stationary_Vel(stationary_velocity, nees_cfmi_init_info);

      }

      //TODO: DEX-3259 liberal hypotheisis to be removed
      else if (cluster_valid_for_liberal_tracking)
      {
         init_info.f_valid_for_liberal_tracking = true;
         Liberal_Initialization(calibrations, nees_cfmi_information, nees_cfmi_init_info);
      }

      // non-stationary hypothesis
      else
      {
         Get_NEES_CFMI_Vel_Hypo(calibrations, nees_cfmi_information, nees_cfmi_init_info);

         // M-estimator to improve robustness.
         if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
         {

            Estimate_Vel_By_CV_NEES_M_Estimator(calibrations, nees_cfmi_information, nees_cfmi_init_info);
            NEES_CFMI_Estimate_Velocity_Covariance(calibrations, host_props, nees_cfmi_init_info);

            // Check if is stationary based on the final velocity
            Stationary_NEES_Test(calibrations, host_props, nees_cfmi_init_info);
            if (!nees_cfmi_init_info.f_stationary_test_rejected) // stationary
            {
               // Overwrite with stationary velocity
               Initialize_Stationary_Vel(stationary_velocity, nees_cfmi_init_info);
            }
         }

      }

      NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   }
   /*===========================================================================*\
   * FUNCTION: Try_To_Init_Obj_Track_By_NEES
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Host_Props_T& host_props,
   * const F360_Object_Track_T& cluster_to_init,
   * const F360_Detection_Props_T& det_props,
   * const RSPP_Detection_List_T& raw_detection_list,
   * const F360_Detection_Hist_T& detection_hist,
   * F360_Tracked_Object_Init_Info_T& init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Try to initialize Object using Normalized Estimated Error Squared (NEES)
   * minimization.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Try_To_Init_Obj_Track_By_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      const F360_Host_T& host,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_Tracked_Object_Init_Info_T& init_info)
   {
      F360_NEES_CFMI_Information_T nees_cfmi_information = {};
      F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};


      Prepare_Info_NEES_CFMI(calibrations, cluster_to_init, det_props,raw_detection_list, sensors, detection_hist, nees_cfmi_init_info, nees_cfmi_information);

      // Estimate velocity and related info. init_info won't be updated here after liberal tracking is removed
      Estimation_NEES_CFMI(calibrations, host_props, host, cluster_to_init, det_props, stationary_velocity, nees_cfmi_init_info, nees_cfmi_information, init_info);

      // Output
      Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster_to_init, detection_hist, raw_detection_list, host, init_info);


   }

   /*===========================================================================*\
   * FUNCTION: Init_NEES_CFMI_Init_Info
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
   * Initialize structure from NEES cost information
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Init_NEES_CFMI_Init_Info(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      const float k_default_weight = 1.0F;
      const uint32_t dets_num = nees_cfmi_information.dets_num;
      const uint32_t vels_num = nees_cfmi_information.vels_num;

      // Stationary check.
      if ((nees_cfmi_information.moving_dets_num > calibrations.k_nees_cfmi_stat_check_max_mov_dets) &&
         (nees_cfmi_information.moving_dets_ratio > calibrations.k_nees_cfmi_stat_check_max_mov_dets_ratio))
      {
         nees_cfmi_init_info.f_stationary_hyp = false;
         nees_cfmi_init_info.sigma_level_pos_diff = calibrations.k_nees_cfmi_init_mov_pd_sl;
      }
      else
      {
         nees_cfmi_init_info.f_stationary_hyp = true;
         nees_cfmi_init_info.sigma_level_pos_diff = calibrations.k_nees_cfmi_init_stat_pd_sl;
      }
      nees_cfmi_init_info.sigma_level_cloud = calibrations.k_nees_cfmi_init_cl_sl;

      for (uint32_t index = 0U; index < dets_num; index++)
      {
         if (nees_cfmi_information.detections[index].f_inlier)
         {
            nees_cfmi_init_info.inliers.f_dets_valid[index] = true;
            nees_cfmi_init_info.inliers.det_weights[index] = k_default_weight;
         }
      }

      for (uint32_t index = 0U; index < vels_num; index++)
      {
         if (nees_cfmi_information.velocities[index].f_inlier)
         {
            nees_cfmi_init_info.inliers.f_vels_valid[index] = true;
            nees_cfmi_init_info.inliers.vel_weights[index] = k_default_weight;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_VH_For_Multi_Time_Instance_Path
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
   * Get Normalized Estimated Error Squared Cost Function Minimization Initialization
   * Velocity Hypothesis for multi time instance path
   * (with valid position difference hypothesis)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Get_NEES_CFMI_VH_For_Multi_Time_Instance_Path(const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      static const uint32_t F360_NEES_CFMI_Vel_Hyp_Source_Valid_Num = 5U;
      // Go through all hypotheses and check if any of them is valid.
      for (uint32_t index = 0U; index < F360_NEES_CFMI_Vel_Hyp_Source_Valid_Num; index++)
      {
         F360_NEES_CFMI_Velocity_T current_hypothesis = {};
         // Estimate - custom for each hypothesis.
         NEES_CFMI_Velocity_Hypothesis_Selector(calibrations,
            static_cast<F360_NEES_CFMI_Vel_Hyp_Source_T>(index),
            nees_cfmi_information,
            nees_cfmi_init_info,
            current_hypothesis);

         if (current_hypothesis.f_valid)
         {
            // Evaluate - same for each hypothesis.
            NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information,
               nees_cfmi_init_info.inliers,
               current_hypothesis.vel,
               nees_cfmi_information.mean_vel_cov,
               nees_cfmi_information.mean_rr_comp_var,
               nees_cfmi_init_info.sigma_level_pos_diff,
               nees_cfmi_init_info.sigma_level_cloud,
               calibrations.k_sigma_level_dominant_velocity,
               calibrations.k_nees_cfmi_min_weight_for_inlier);

            NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, nees_cfmi_init_info.inliers, current_hypothesis);
            Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, current_hypothesis);

            if (current_hypothesis.plausibility > nees_cfmi_init_info.VCS_vel_hyp.plausibility)
            {
               nees_cfmi_init_info.vel_hyp_source = static_cast<F360_NEES_CFMI_Vel_Hyp_Source_T>(index);

               nees_cfmi_init_info.VCS_vel_hyp = current_hypothesis;

               Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(nees_cfmi_init_info.VCS_vel_hyp);
               nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace = Trace_Of_2d_Covariance(nees_cfmi_init_info.VCS_vel_hyp.vel_cov);

               if (nees_cfmi_init_info.VCS_vel_hyp.plausibility > calibrations.k_nees_cfmi_vh_check_high_plausibility)
               {

                  break; // No need to look for next hypothesis.
               }

               nees_cfmi_init_info.VCS_vel_hyp.f_valid = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations,
                  nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);

            }
         }
      }



   }

   /*===========================================================================*\
   * FUNCTION: Best_NEES_CFMI_VH_Plausbility_Checks
   *===========================================================================
   * RETURN VALUE:
   * bool plausibility flag
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_NEES_CFMI_Velocity_T& vel_hypothesis
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check if velocity hypothesis is plausible or not
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Best_NEES_CFMI_VH_Plausbility_Checks(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Velocity_T& vel_hypothesis)
   {
      bool f_plausible;
      f_plausible = (((vel_hypothesis.plausibility > calibrations.k_nees_cfmi_vh_check_min_overall_plaus) ||
         ((vel_hypothesis.plausibility_cloud > calibrations.k_nees_cfmi_vh_check_min_cl_plaus) &&
         (vel_hypothesis.plausibility_pos_diff > calibrations.k_nees_cfmi_vh_check_min_pd_plaus))) &&
         (vel_hypothesis.num_cloud >= nees_cfmi_information.min_dets_inliers_num) &&
         (vel_hypothesis.num_pos_diff >= nees_cfmi_information.min_vels_inliers_num));

      return f_plausible;
   }


   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_VH_For_One_Time_Instance_Path
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Get Normalized Estimated Error Squared Cost Function Minimization Initialization
   * Velocity Hypothesis for one time instance path (no position difference hypothesis)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Get_NEES_CFMI_VH_For_One_Time_Instance_Path(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);
      Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(nees_cfmi_init_info.VCS_vel_hyp);
      nees_cfmi_init_info.vel_hyp_source = F360_NEES_CFMI_VEL_HYP_SOURCE_CLOUD;
   }

   /*===========================================================================*\
   * FUNCTION: Update_Track_Init_Info_By_NEES_CFMI_Init_Info
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   *  const F360_Calibrations_T& calibrations,
   *  const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
   *  const F360_Cluster_T& cluster,
   *  const F360_Detection_Hist_T& detection_hist,
   *  const RSPP_Detection_List_T& raw_detect_list,
   *  const F360_Host_T& host,
   *  F360_Tracked_Object_Init_Info_T& init_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Update Init info (global) by NEES CFMI init info (local)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Track_Init_Info_By_NEES_CFMI_Init_Info(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      const F360_Cluster_T& cluster,
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Host_T& host,
      F360_Tracked_Object_Init_Info_T& init_info)

   {
      init_info.f_success = nees_cfmi_init_info.f_valid && (calibrations.f_nees_cfmi_init_with_not_plausible_vel || nees_cfmi_init_info.f_plausible);

      init_info.init_scheme = nees_cfmi_init_info.init_scheme;
      init_info.init_vel_source = nees_cfmi_init_info.vel_hyp_source;

      init_info.det_bbox = nees_cfmi_init_info.det_bbox;
      init_info.VCS_position_cov[F360_2D_IDX_X][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      init_info.VCS_position_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      init_info.VCS_position_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      init_info.VCS_position_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];

      init_info.VCS_velocity = nees_cfmi_init_info.VCS_vel_hyp.vel;
      init_info.VCS_velocity_plausability = nees_cfmi_init_info.VCS_vel_hyp.plausibility;

      init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = nees_cfmi_init_info.VCS_vel_hyp.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];

      if (init_info.f_success)
      {
         init_info.frac_az_error_stat_mov = Calc_Frac_Of_Az_Error_Stat_Mov(cluster, detection_hist, raw_detect_list);
         init_info.f_success = Is_Not_Overlapping_With_Host(init_info.det_bbox.Get_Center(), host);
      }
   }


   /*===========================================================================*\
   * FUNCTION: Liberal_Initialization
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
   * Initialize velocity with radial assumtion.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Liberal_Initialization(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      const float32_t rrate_var = calibrations.k_range_rate_std * calibrations.k_range_rate_std;
      NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information,
         nees_cfmi_init_info.inliers,
         nees_cfmi_information.radial_velocity.vel,
         nees_cfmi_information.mean_vel_cov,
         rrate_var,
         calibrations.liberal_tracking_sigma_level_pos_diff,
         calibrations.liberal_tracking_sigma_level_cloud,
         calibrations.k_sigma_level_dominant_velocity,
         calibrations.k_nees_cfmi_min_weight_for_inlier);

      NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, nees_cfmi_init_info.inliers, nees_cfmi_init_info.VCS_vel_hyp);
      Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, nees_cfmi_init_info.VCS_vel_hyp);
      Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(nees_cfmi_init_info.VCS_vel_hyp);

      nees_cfmi_init_info.VCS_vel_hyp.plausibility = 0.0F; //Bug found during code refactoring, will be removed with liberal tracking in DEX-3259
      nees_cfmi_init_info.f_valid = nees_cfmi_information.radial_velocity.f_valid;
      nees_cfmi_init_info.VCS_vel_hyp.f_valid = nees_cfmi_information.radial_velocity.f_valid;
      nees_cfmi_init_info.VCS_vel_hyp.vel = nees_cfmi_information.radial_velocity.vel;
      nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace = Trace_Of_2d_Covariance(nees_cfmi_information.radial_velocity.vel_cov);
      nees_cfmi_init_info.f_plausible = true;
      nees_cfmi_init_info.vel_hyp_source = F360_NEES_CFMI_VEL_HYP_SOURCE_RANGE_RATE;
      nees_cfmi_init_info.init_scheme = F360_TRACK_NEES_LIBERAL;

   }
}
