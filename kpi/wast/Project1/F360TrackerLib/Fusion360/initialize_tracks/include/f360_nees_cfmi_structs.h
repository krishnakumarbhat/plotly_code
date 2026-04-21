/*===================================================================================*\
* FILE: f360_nees_cfmi_structs.h
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
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
*   definition of nees_cfmi_information structure
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
#ifndef F360_NEES_CFMI_STRUCTS_H
#define F360_NEES_CFMI_STRUCTS_H

#include "f360_reuse.h"
#include "f360_point.h"
#include "f360_velocity.h"
#include "f360_matrix_dimension.h"
#include "f360_detection_time_slots.h"
#include "f360_nees_cfmi_vel_hyp_source.h"
#include "f360_track_init.h"
#include "f360_bounding_box.h"

namespace f360_variant_A
{
   enum F360_NEES_CFMI_Vel_RR_Conf_T : uint8_t // Enumeration for Velocity Hypothesis range rate evaluation confidence level.
   {
      F360_NEES_CFMI_VEL_RR_CONF_INVALID = 0,
      F360_NEES_CFMI_VEL_RR_CONF_LOW = 1,
      F360_NEES_CFMI_VEL_RR_CONF_MEDIUM = 2,
      F360_NEES_CFMI_VEL_RR_CONF_HIGH = 3
   };

   struct F360_NEES_CFMI_Detection_Idx_T // Structure used for unified index format for current and historical detections.
   {
      uint32_t idx; // Index of detection's position in source array (detection props or historical detections).
      uint32_t nees_idx; // Index of detection's position in array of NEES detections
      bool f_historical; // Flag indicating if detection is historical or not.

      F360_NEES_CFMI_Detection_Idx_T() : idx(), nees_idx(), f_historical(){}
      F360_NEES_CFMI_Detection_Idx_T(const uint32_t index, const uint32_t nees_index, const bool f_hist) : idx(index), nees_idx(nees_index), f_historical(f_hist){}
   };

   struct F360_NEES_CFMI_Determinants_T // Structure used for accumulation determinants used for velocity initialization.
   {
      // Sx = Vx * Sxx + Vy * Sxy - Vx derivative equation
      // Sy = Vx * Syx + Vy * Syy - Vy derivative equation
      uint32_t n_dets; // Number of detections.
      float32_t Sxx; // Sum of factors corresponding to Vx in Vx derivative equation.
      float32_t Sxy; // Sum of factors corresponding to Vy in Vx derivative equation.
      float32_t Sx;  // Sum of factors corresponding to free expression in Vx derivative equation.
      float32_t Syy; // Sum of factors corresponding to Vy in Vy derivative equation.
      float32_t Syx; // Sum of factors corresponding to Vx in Vy derivative equation.
      float32_t Sy;  // Sum of factors corresponding to free expression in Vy derivative equation.
      float32_t information; // Information in factors (does not have to be Fisher Information).

      F360_NEES_CFMI_Determinants_T() : n_dets(), Sxx(), Sxy(), Sx(), Syy(), Syx(), Sy(), information() {}
   };

   struct F360_NEES_CFMI_Detection_T // Structure used for accumulation information of single detection (internal input).
   {
      F360_NEES_CFMI_Detection_Idx_T idx; // Index structure of detection.
      float32_t time_since_meas; // Time-stamp of measurement.
      float32_t timestamp_diff;  // time difference between time since measurement of detection and current time index timestamp
      Point pos; // Position in VCS.
      float32_t pos_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // 2D covariance matrix of position
      float32_t range_rate_comp; // Range rate compensated by host movement.
      float32_t range_rate_comp_var; // Estimated Variance of range rate compensated by host movement.
      float32_t sin_vcs_az; // Sine of VCS azimuth.
      float32_t cos_vcs_az; // Cosine of VCS azimuth.
      F360_NEES_CFMI_Determinants_T cloud_determinants; // Determinants associated to this detection.
      bool f_valid; // Flag indication if it is valid.
      bool f_inlier; // Flag indication if it is inlier (initially).


      F360_NEES_CFMI_Detection_T() :
         idx(),
         time_since_meas(),
         timestamp_diff(),
         pos(),
         pos_cov(),
         range_rate_comp(),
         range_rate_comp_var(),
         sin_vcs_az(),
         cos_vcs_az(),
         cloud_determinants(),
         f_valid(),
         f_inlier()
      {}
   };

   struct F360_NEES_CFMI_Pos_Diff_Velocity_T // Structure used for accumulation information of single velocity based on position difference (internal input).
   {
      F360_NEES_CFMI_Detection_Idx_T det_A_idx; // Index structure of detection: Newer detection.
      F360_NEES_CFMI_Detection_Idx_T det_B_idx; // Index structure of detection: Older detection.
      float32_t time_since_meas; // Time-stamp of measurement: preferable middle of detections timestamps.
      F360_VCS_Velocity_T vel; // Estimated velocity in VCS.
      float32_t vel_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Estimated velocity covariance in VCS.
      float32_t dt; // Time difference between measurements.
      float32_t cov_trace; // Trace of covariance matrix.
      float32_t determinant; // Determinant of position difference based NEES value determinants
      float32_t rr_plausbility; // Overall plausibility of associated detections range rates: confirmation that VH comes from valid RR.
      F360_NEES_CFMI_Determinants_T pos_diff_determinants; // Determinants associated to this velocity.
      F360_NEES_CFMI_Vel_RR_Conf_T rr_confidence_level; // Enumeration for Velocity Hypothesis range rate evaluation confidence level.
      bool f_valid; // Flag indication if it is valid.
      bool f_inlier; // Flag indication if it is inlier (initially).
      bool f_rr_confirmed; // Flag indication if associated detections confirms this velocity hypothesis.

      F360_NEES_CFMI_Pos_Diff_Velocity_T() :
         det_A_idx(),
         det_B_idx(),
         time_since_meas(),
         vel(),
         vel_cov(),
         dt(),
         cov_trace(),
         determinant(),
         rr_plausbility(),
         pos_diff_determinants(),
         rr_confidence_level(),
         f_valid(),
         f_inlier(),
         f_rr_confirmed()
      {}
   };

   struct F360_NEES_CFMI_Information_T // Structure used for accumulating information needed for initialization (internal input).
   {
      Point pos_center; // Position of center based on current detections (in VCS).
      float32_t pos_center_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Covariance of position of center based on current detections (in VCS).
      float32_t center_time_since_meas; // Time since measurement of center.

      float32_t mean_vel_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Mean of velocity covariances.
      float32_t mean_rr_comp_var; // Mean range rate compensated variance.


      float32_t min_time_since_meas; // Minimum time since measurement of valid detections.

      uint32_t dets_num; // Number of valid detections (for cloud).
      uint32_t current_dets_count; // Number of current detections.
      uint32_t old_dets_count; // Number of historical detections.
      uint32_t vels_num; // Number of valid velocities (position difference).

      uint32_t moving_dets_num; // Number of detections classified as moving.
      float32_t moving_dets_ratio; // Ratio of detections classified as moving.

      uint32_t init_dets_inliers_num; // Initial number of detection inliers - based on preliminary conditions.

      F360_NEES_CFMI_Detection_T detections[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET]; // Detections associated and accumulated into matrix.
      F360_NEES_CFMI_Pos_Diff_Velocity_T velocities[F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL]; // Velocities associated and accumulated into matrix.

      F360_NEES_CFMI_Pos_Diff_Velocity_T cloud_hyp_vel; // Velocity Hypothesis based on Cloud.
      F360_NEES_CFMI_Pos_Diff_Velocity_T confirm_pos_diff_hyp_vel; // Confirmed Position Difference Hypothesis Velocity: hypothesis from confirmed position difference VH.
      F360_NEES_CFMI_Pos_Diff_Velocity_T dominant_velocity; // Dominant Velocity: newest detections vs the oldest detections.
      F360_NEES_CFMI_Pos_Diff_Velocity_T radial_velocity; // Radial Velocity: velocity based on radial motion assumption.
      F360_NEES_CFMI_Pos_Diff_Velocity_T stationary_velocity; // Stationary Velocity: hypothesis that object is stationary.

      uint32_t expected_vels_inliers_num; // Expected number of position difference based velocity hypotheses
      float32_t expected_vels_inliers_ratio; // Expected ratio of position difference based velocity hypotheses
      uint32_t expected_vels_inliers_num_in_slot[F360_NEES_COST_FUNCTION_INFORMATION_VEL_SLOTS]; // Expected number of position difference based velocity hypotheses in one slot.

      uint32_t min_dets_inliers_num; // Minimum number of detection inliers to accept velocity estimation.
      uint32_t min_vels_inliers_num; // Minimum number of velocity inliers to accept velocity estimation.

      uint32_t num_of_confirm_pos_diff_hyp_low; // Number of position difference velocity hypotheses low confidence level.
      uint32_t num_of_confirm_pos_diff_hyp_med; // Number of position difference velocity hypotheses medium confidence level.
      uint32_t num_of_confirm_pos_diff_hyp_high; // Number of position difference velocity hypotheses high confidence level.

      bool f_is_any_det_in_all_looks; // If range of any detection is in all looks

      F360_NEES_CFMI_Information_T() :
         pos_center(),
         pos_center_cov(),
         center_time_since_meas(),
         mean_vel_cov(),
         mean_rr_comp_var(),
         min_time_since_meas(),
         dets_num(),
         current_dets_count(),
         old_dets_count(),
         vels_num(),
         moving_dets_num(),
         moving_dets_ratio(),
         init_dets_inliers_num(),
         detections(),
         velocities(),
         cloud_hyp_vel(),
         confirm_pos_diff_hyp_vel(),
         dominant_velocity(),
         radial_velocity(),
         stationary_velocity(),
         expected_vels_inliers_num(),
         expected_vels_inliers_ratio(),
         expected_vels_inliers_num_in_slot(),
         min_dets_inliers_num(),
         min_vels_inliers_num(),
         num_of_confirm_pos_diff_hyp_low(),
         num_of_confirm_pos_diff_hyp_med(),
         num_of_confirm_pos_diff_hyp_high(),
         f_is_any_det_in_all_looks()
      {}
   };

   struct F360_NEES_CFMI_Velocity_T // Structure used for accumulation information of single velocity hypothesis.
   {
      uint32_t num_all; // Number of estimates used (inliers)
      uint32_t num_cloud; // Number of cloud based estimates used (inliers)
      uint32_t num_pos_diff; // Number of position difference based estimates used (inliers)

      float32_t weight_sum; // Sum of weights.
      float32_t weight_dets_sum; // Sum of weights of cloud source.
      float32_t weight_vels_sum; // Sum of weights of position difference source.
      float32_t weight_dominant_vel; // Sum of weights of dominant velocity source.

      float32_t plausibility; // Overall plausibility.
      float32_t plausibility_cloud; // Plausibility of cloud source.
      float32_t plausibility_pos_diff; // Plausibility of position difference source.
      float32_t plausibility_dominant_vel; // Plausibility of dominant velocity source.

      float32_t information_cloud; // Accumulated information of cloud source based on range rate variance (not Fisher)
      float32_t information_pos_diff; // Accumulated Fisher information of position difference source.

      float32_t V_determinant; // Determinant used for velocity estimation.
      F360_VCS_Velocity_T vel; // Estimated velocity in VCS.
      float32_t vel_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Estimated velocity covariance in VCS.
      float32_t vel_cov_trace; // Trace of covariance matrix.
      F360_NEES_CFMI_Determinants_T determinants; // Determinants associated to this velocity.

      bool f_valid; // Flag indication if it is valid.
      bool f_plausible; // Flag indication if it is plausible and valid.
      bool f_cov_valid; // Flag indication if covariance is valid.

      void Reset_Plausibility();

      F360_NEES_CFMI_Velocity_T() :
         num_all(),
         num_cloud(),
         num_pos_diff(),
         weight_sum(),
         weight_dets_sum(),
         weight_vels_sum(),
         weight_dominant_vel(),
         plausibility(),
         plausibility_cloud(),
         plausibility_pos_diff(),
         plausibility_dominant_vel(),
         information_cloud(),
         information_pos_diff(),
         V_determinant(),
         vel(),
         vel_cov(),
         vel_cov_trace(),
         determinants(),
         f_valid(),
         f_plausible(),
         f_cov_valid()
      {}
   };

   struct F360_NEES_CFMI_Inliers_T // Structure used for storing inliers information.
   {
      float32_t det_weights[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET]; // Weight values for detections.
      float32_t vel_weights[F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL]; // Weight values for velocities.
      float32_t dominant_vel_weight;   // Weight value for dominant velocity.
      bool f_dets_valid[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET]; // Flags indicating if detection is inlier.
      bool f_vels_valid[F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL]; // Flags indicating if velocity is inlier.
      bool f_dominant_vel_valid;  // Flags indicating if dominant velocity is valid.

      F360_NEES_CFMI_Inliers_T() :
         det_weights(),
         vel_weights(),
         dominant_vel_weight(),
         f_dets_valid(),
         f_vels_valid(),
         f_dominant_vel_valid()
      {}
   };

   struct F360_NEES_CFMI_Init_Info_T // Structure used for accumulating initialization information. (internal output)
   {
      // Internal variables.

      uint32_t max_number_of_ransac_iterations; // Maximum number of RANSAC iterations.

      float32_t sigma_level_cloud; // Sigma level for cloud source for M-estimator.
      float32_t sigma_level_pos_diff; // Sigma level for position difference source for M-estimator.

      float32_t stationary_test_nees_value; // Stationary NEES test value [sigma].
      float32_t stationary_test_p_value; // Stationary NEES test p-value.

      F360_NEES_CFMI_Velocity_T VCS_vel_hyp; // Structure for velocity hypothesis estimation.
      F360_NEES_CFMI_Inliers_T inliers; // Inliers structure.


      BoundingBox det_bbox; // Bounding box containing the cluster detections
      float32_t VCS_pos_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Final position covariance estimate in VCS.

      // Output information section.
      F360_Track_Init_T init_scheme; // Initialization scheme.
      F360_NEES_CFMI_Vel_Hyp_Source_T vel_hyp_source; // Source of velocity initialization.

      // Base section - used for velocity estimation itself.
      bool f_valid; // Flag indicating that it is valid.
      bool f_plausible; // Flag indicating that velocity is plausible.
      bool f_fault_detected; // Flag indicating that fault was detected, sanity checks failed.

      // Section of controlling flags.
      bool f_stationary_hyp; // Flag indicating that it is possible that object is stationary, false indicating that there is no such possibility.
      bool f_stationary_hyp_valid; // Flag indicating that stationary hypothesis was confirmed.
      bool f_stationary; // Flag indicating object is stationary.

      bool f_cloud_vh_valid; // Flag indicating that cloud velocity hypothesis was valid.

      bool f_dominant_valid; // Flag indicating that dominant hypothesis was valid.

      bool f_confirm_pos_diff_hyp_vel_valid; // Flag indicating that velocity hypothesis from confirmed position difference VH was valid.

      bool f_ransac_valid; // Flag indicating that RANSAC estimate is valid.

      bool f_radial_vh_valid; // Flag indicating that radial velocity hypothesis was valid.

      bool f_m_estimator_valid; // Flag indicating if M-estimator estimate is valid.

      bool f_stationary_test_rejected; // Flag indicating that stationary NEES test was rejected - object can be treat as moving.

      F360_NEES_CFMI_Init_Info_T() :
         max_number_of_ransac_iterations(),
         sigma_level_cloud(),
         sigma_level_pos_diff(),
         stationary_test_nees_value(),
         stationary_test_p_value(),
         VCS_vel_hyp(),
         inliers(),
         det_bbox(),
         VCS_pos_cov(),
         init_scheme(F360_TRACK_INVALID),
         vel_hyp_source(F360_NEES_CFMI_VEL_HYP_SOURCE_NONE),
         f_valid(),
         f_plausible(),
         f_fault_detected(),
         f_stationary_hyp(),
         f_stationary_hyp_valid(),
         f_stationary(),
         f_cloud_vh_valid(),
         f_dominant_valid(),
         f_confirm_pos_diff_hyp_vel_valid(),
         f_ransac_valid(),
         f_radial_vh_valid(),
         f_m_estimator_valid(),
         f_stationary_test_rejected()
      {}
   };
}
#endif

