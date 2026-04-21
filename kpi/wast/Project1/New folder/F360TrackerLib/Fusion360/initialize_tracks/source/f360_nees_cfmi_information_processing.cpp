/*===================================================================================*\
* FILE: f360_nees_cfmi_information_processing.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains functions declarations for processing NEES cost function information processing:
*     - Get_NEES_CFMI_Information
*     - Calculate_Detections_Mean_Center
*     - Calculate_Moving_Detections_Ratio
*     - Calculate_Detections_And_Velocities_Inliers_Ratio
*     - Calculate_Mean_Range_Rate_Comp_Variance
*     - Calculate_Mean_Velocity_Covariance
*     - Calculate_Moving_Detections_Ratio
*     - Calculate_Detections_And_Velocities_Inliers_Ratio
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_nees_cfmi_information_processing.h"
#include "f360_detection_history_time_slots_processing.h"
#include "f360_detection_time_slots.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_math_func.h"
#include "f360_uncertainty_propagation.h"
#include "f360_nees_cfmi_info_cloud.h"
#include "f360_nees_cfmi_info_pos_diff.h"
#include "f360_nees_cfmi_hypothesis_confirm_pos_diff.h"
#include "f360_nees_cfmi_hypothesis_dominant_velocity.h"
#include "f360_nees_cfmi_hypothesis_cloud.h"
#include "f360_nees_cfmi_hypothesis_radial.h"
#include "f360_matrix_dimension.h"
#include <algorithm>

namespace f360_variant_A
{
   static void Get_NEES_CFMI_Information_Other(const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      //Postprocessing
      Calculate_Moving_Detections_Ratio(nees_cfmi_information);
      Calculate_Detections_And_Velocities_Inliers_Ratio(calibrations, nees_cfmi_information);
      Calculate_Mean_Range_Rate_Comp_Variance(nees_cfmi_information);
      Calculate_Mean_Velocity_Covariance(nees_cfmi_information);
   }

   static void Get_NEES_CFMI_Information_Dets(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      F360_Detection_Time_Slots_T det_hist_time_slots = {};

      //Preprocessing
      Calculate_Detections_Mean_Center(calibrations, cluster_to_init, raw_detection_list, det_props, sensors, nees_cfmi_information);

      //Get NEES determints for both current dets and old dets
      Get_NEES_CFMI_Information_Cloud_Current_Dets(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, nees_cfmi_information);
      Get_NEES_CFMI_Information_Cloud_Old_Dets(calibrations, cluster_to_init, detection_hist, nees_cfmi_information);


      Split_Dets_Into_Time_Slots(nees_cfmi_information, det_hist_time_slots, calibrations.k_nees_cfmi_dt_to_split_dets);


      nees_cfmi_information.f_is_any_det_in_all_looks = Check_If_Cluster_Has_Any_Det_In_All_Looks(cluster_to_init, raw_detection_list, sensors, detection_hist);
      Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities(calibrations, det_hist_time_slots, nees_cfmi_information);

      //Get dominant_velocity for both Check_Stationary_NEES_CFMI_Hypothesis and Check_CV_Dominant_NEES_CFMI_Hypothesis
      Get_NEES_CFMI_Information_Dominant_Velocity(calibrations, det_hist_time_slots, nees_cfmi_information);
   }

   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations,
   * const F360_Cluster_T& cluster_to_init,
   * const F360_Detection_Props_T& det_props,
   * const F360_Detection_Hist_T& detection_hist,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * ABSTRACT:
   * Get NEES cost information - extract relevant information to speed up NEES
   * cost function minimization algorithms.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_NEES_CFMI_Information(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      //Prepare det info
      Get_NEES_CFMI_Information_Dets(calibrations, cluster_to_init, det_props, detection_hist, raw_detection_list, sensors, nees_cfmi_information);

      //TODO: DEX-3259 Move to VH selector after liberal removed
      Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

      Get_NEES_CFMI_Information_Other(calibrations, nees_cfmi_information);

   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Detections_Mean_Center
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations,
   * const F360_Cluster_T& cluster_to_init,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * ABSTRACT:
   * Calculate mean center position of cluster detections
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Detections_Mean_Center(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t num_dets = static_cast<uint32_t>(cluster_to_init.ndets);

      float32_t sum_x = 0.0F;
      float32_t sum_y = 0.0F;
      float32_t sum_time_since_measurement = 0.0F;
      float32_t position_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
      float32_t extended_object_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
      Init_2d_Covariance_By_Std(calibrations.k_nees_cfmi_extended_object_pos_std, extended_object_cov);

      for (uint32_t index = 0U; index < num_dets; index++)
      {
         const uint32_t det_index = static_cast<uint32_t>(cluster_to_init.detids[index]) - 1U;
         const rspp_variant_A::RSPP_Detection_T& single_det_raw = raw_detect_list.detections[det_index];
         const F360_Detection_Props_T& single_det = det_props[det_index];
         const int32_t sensor_index = single_det_raw.raw.sensor_id - 1;
         const float32_t time_since_measurement = sensors[sensor_index].variable.time_since_measurement_s;

         sum_x += single_det.vcs_position.x;
         sum_y += single_det.vcs_position.y;
         sum_time_since_measurement += time_since_measurement;
         Add_Into_Uncertainty_2d(single_det.position_cov_nees, position_cov);
         nees_cfmi_information.min_time_since_meas = (nees_cfmi_information.min_time_since_meas > F360_EPSILON) ? std::min(nees_cfmi_information.min_time_since_meas, time_since_measurement) : time_since_measurement;
      }

      const float32_t one_over_num_dets = 1.0F / static_cast<float32_t>(num_dets);
      nees_cfmi_information.pos_center.x = sum_x * one_over_num_dets;
      nees_cfmi_information.pos_center.y = sum_y * one_over_num_dets;
      nees_cfmi_information.center_time_since_meas = sum_time_since_measurement * one_over_num_dets;
      Constant_Uncertainty_Propagation_2d(one_over_num_dets, position_cov);
      Add_Uncertainty_2d(position_cov, extended_object_cov, nees_cfmi_information.pos_center_cov);
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Moving_Detections_Ratio
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * ABSTRACT:
   * Calculate ratio of moving detections to all processed detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Moving_Detections_Ratio(F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      if (nees_cfmi_information.dets_num > 0U)
      {
         nees_cfmi_information.moving_dets_ratio = static_cast<float32_t>(nees_cfmi_information.moving_dets_num) / static_cast<float32_t>(nees_cfmi_information.dets_num);
      }
      else
      {
         nees_cfmi_information.moving_dets_ratio = 0.0F;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Detections_And_Velocities_Inliers_Ratio
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * ABSTRACT:
   * Calculate inliers ratio of detections' range rates and position based velocities.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Detections_And_Velocities_Inliers_Ratio(const F360_Calibrations_T& calibrations, F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      nees_cfmi_information.min_dets_inliers_num = std::min(calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate, nees_cfmi_information.init_dets_inliers_num);
      nees_cfmi_information.min_vels_inliers_num = std::min(calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate, nees_cfmi_information.expected_vels_inliers_num);

      if (nees_cfmi_information.vels_num > 0U)
      {
         nees_cfmi_information.expected_vels_inliers_ratio = static_cast<float32_t>(nees_cfmi_information.expected_vels_inliers_num) / static_cast<float32_t>(nees_cfmi_information.vels_num);
      }
      else
      {
         nees_cfmi_information.expected_vels_inliers_ratio = 0.0F;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Mean_Range_Rate_Comp_Variance
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * ABSTRACT:
   * Calculate mean of processed detection range rate variances.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Mean_Range_Rate_Comp_Variance(F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t dets_num = nees_cfmi_information.dets_num;

      if ((0U < dets_num) && (dets_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET))
      {
         for (uint32_t index = 0U; index < dets_num; index++)
         {
            nees_cfmi_information.mean_rr_comp_var += nees_cfmi_information.detections[index].range_rate_comp_var;
         }
         const float32_t one_over_dets_num = 1.0F / static_cast<float32_t>(dets_num);
         nees_cfmi_information.mean_rr_comp_var *= one_over_dets_num;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Mean_Velocity_Covariance
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * ABSTRACT:
   * Calculate mean of position differences based velocitys covariance matrices.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Mean_Velocity_Covariance(F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t vels_num = nees_cfmi_information.vels_num;

      if ((0U < vels_num) && (vels_num <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL))
      {
         for (uint32_t index = 0U; index < vels_num; index++)
         {
            Add_Into_Uncertainty_2d(nees_cfmi_information.velocities[index].vel_cov, nees_cfmi_information.mean_vel_cov);
         }
         const float32_t one_over_vels_num = 1.0F / static_cast<float32_t>(vels_num);

         nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] *= one_over_vels_num;
         nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] *= one_over_vels_num;
         nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] *= one_over_vels_num;
         nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] *= one_over_vels_num;

      }
   }

}
