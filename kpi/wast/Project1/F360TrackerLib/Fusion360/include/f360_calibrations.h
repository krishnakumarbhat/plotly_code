/*===========================================================================*\
* FILE: f360_calibrations.h
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains F360_Calibrations structure declaration
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#ifndef F360_CALIBRATIONS_H
#define F360_CALIBRATIONS_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_sensor_type.h"
#include "f360_conf.h"

namespace f360_variant_A
{
   typedef struct F360_Calibrations_Tag
   {
      float32_t initial_speed_var_radial_init;
      float32_t initial_heading_var_radial_init;
      float32_t max_change_in_fraction_of_dets;
      float32_t speed_var_non_zero_yaw_rate;
      float32_t host_vicinity_vcs_x_range;
      float32_t host_vicinity_vcs_y_range;
      float32_t fast_moving_thresh;
      float32_t low_confidence_level_thresh;
      float32_t snr_valid_thresh; // do not use SNR when its value is below this threshold
      // Time Update
      // CTCA
      float32_t k_max_allowed_curv_variance; // [1/m^2] Maximum curvature variance allowed in time update. If resulting curvature variance is larger then it is saturated.
      float32_t k_min_acc_for_increasing_acc_noise; // [m/s^2] Minimum absolute tangential acceleration for when to start ramping up acceleration Q variance in CTCA time update (i.e. for acceleration below this value Q will be at lowest level)
      float32_t k_max_acc_for_increasing_acc_noise; // [m/s^2] Maximum absolute tangential acceleration for when to stop ramping up acceleration Q variance in CTCA time update (i.e. for acceleration above this value Q will be at highest level)
      float32_t k_max_acc_scale_factor; // [-] Maximum scale factor for acceleration Q variance in CTCA time update
      float32_t k_normal_noise_acc; // [m^2/s^4] Acceleration process noise calibration parameter (for Q matrix)
      float32_t k_normal_noise_speed; // [m^2/s^2] Speed process noise calibration parameter (for Q matrix)
      float32_t k_normal_noise_pos_para; // [m^2] Para position process noise calibration parameter (for Q matrix)
      float32_t k_q_tuning_orth_direction_speed_breakpoint; // [m/s] Parameter used for Q matrix tuning of the orth/non-tangential behaviour of the object. Speed threshold that corresponds to when the assumption of "maximum possible curvature is constant and determined my min turning radius" transitions into assumption of "maximum possible curvature is determined by maximum possible lateral acceleration"
      float32_t k_normal_noise_curv; // [1/m^2] Curvature process noise calibration parameter (for Q matrix)
      float32_t k_normal_noise_hdg; // [rad^2] Heading process noise calibration parameter (for Q matrix)
      float32_t k_normal_noise_pos_orth; // [m^2] Orth position process noise calibration parameter (for Q matrix)
      // Other
      float32_t k_conf_overlapping_reduction_factor;
      float32_t k_hyst_time_for_coasted_objects;
      float32_t k_range_rate_std;

      // CTCA measurement update
      float32_t k_ctca_msmnt_update_max_reverse_abs_spd; // [m/s] Maximum absolute value of speed to be considered as a reversing object.

      // CCA measurement update
      float32_t k_cca_msmnt_update_vel_var; // [(m / s)^2] Assumed variance of velocity measuremenmt used in CCA pointing, yaw rate filter measurement update
      float32_t k_speed_th_for_saturating_r; // Speed threshold for saturating R in CCA pointing, yaw rate filter measurement update
      float32_t k_max_innov_dist_thres; // [m] Maximum position innovation allowed for non-moveable CCA objects before the maximum limitation on how much the position measurement is allowed to impact the velocity estimate is reached.
      float32_t k_max_innov_dist_yaw_rate_gain; // [ms/rad] Gain on host yaw rate to compute "total k_max_innov_dist_thres". The gain multiplied with host yaw rate plus k_max_innov_dist_thres yields the "total k_max_innov_dist_thres".
      float32_t k_min_decrease_factor; // [-] Minimum decrease factor of Kalman gain elements to limit on how much the position measurement is allowed to impact the velocity estimate for non-moveable CCA objects. Should be in the range of [0, 1]

      // CTCA and CCA time and measurement update for preventing overshooting of velocity and acceleration estimates at hard breaking
      float32_t k_abs_acc_threshold_for_breaking; // [m/s^2] Threshold on absolute value of acceleration for assuming an object is braking.
      float32_t k_abs_speed_threshold_for_stopping; // [m/s] Threshold on object absolute value of speed for considering it being stopped

      // Track validity
      float32_t k_tv_refl_gr_trk_max_diff_x; // [m] maximum x_vcs distance of an object track from source position to consider it as a source of reflected ghost candidate
      float32_t k_tv_refl_gr_trk_max_diff_y; // [m] maximum y_vcs distance of an object track from source position to consider it as a source of reflected ghost candidate
      float32_t k_tv_min_host_speed_moving; // [m/s] minimum host speed to run reflection at intersection algorithm
      float32_t k_tv_min_vel_of_reflected_trk; // [m/s] minimum reflected track speed to run reflection at intersection algorithm
      float32_t k_tv_max_age_of_reflected_trk; // [s] reflected track maximum age (time) to run reflection at intersection algorithm
      float32_t k_tv_refl_trk_offset_from_reflector_center_sq; // [m^2] minimal squared range from reflected object to reflection point
      float32_t k_tv_max_error_in_orientation; // [rad] maximum value of error in orientation used for calculating error in angle if needed
      float32_t k_tv_max_heading_error; // [rad] maximum heading error for determining if obejct is reflection
      float32_t k_tv_max_dist_of_reflector; // [m] maxiumum distance to an object that can be reflector candidate
      float32_t k_tv_reflector_size_extension; // [m] size extension for possible reflector object.
      float32_t k_tv_error_in_angle; // [rad] defualt angle error value for calcualtion of the mirror line slope
      float32_t k_tv_max_frac_error_in_speed; // [-] maximum fracion of speed for reflected object verification
      float32_t k_tv_refl_gr_trk_max_diff_rel_vel; // [m/s] maximum relative velocity difference between an object track and ghost candidate to consider it as a source of reflection
      float32_t k_tv_refl_gr_trk_max_diff_heading; // [rad] maximum heading difference between an object track and ghost candidate to consider it as a source of reflection
      float32_t k_tv_refl_gr_trk_min_bbox_lat_margin; // [m] minimum tcs lateral extension of source candadiate bbox in which the hypothetic source point has to consider reflection
      float32_t k_tv_refl_gr_trk_bbox_lat_margin; // [m] tcs lateral extension of source candadiate bbox in which the hypothetic source point has to consider reflection
      float32_t k_tv_refl_gr_trk_straight_mov_head_th; // [rad] max allowed abs heading of source candidate consider it as a source of reflected ghost candidate
      float32_t k_tv_refl_gr_trk_min_sep_lon_pos; // [m] lower bound of x_vcs interval on the guardrail in which reflective objects are analyzed
      float32_t k_tv_refl_gr_trk_max_sep_lon_pos; // [m] upper bound of x_vcs interval on the guardrail in which reflective objects are analyzed
      float32_t k_tv_refl_gr_trk_min_host_to_ghost_lon_pos_diff; // [m] minimum x_vcs distance between the host and ghost candidate to continue reflection analysis
      float32_t k_tv_dets_exp_filter_const; // [s] Time constant for first order exponential low pass filter

      float32_t host_vehicle_width;
      float32_t host_vehicle_length;

      float32_t k_vp_vehicle_next_to_ego_max_lat_dist;
      float32_t k_vp_vehicle_next_to_ego_max_long_dist;

      float32_t k_vp_vehicle_next_to_ego_max_abs_heading;

      float32_t k_vp_vehicle_next_to_ego_lat_pos_offset;
      float32_t k_vp_vehicle_next_to_ego_long_pos_offset;

      float32_t k_min_speed_for_updating_heading; // [m/s] Minimum speed for updating heading (e.g. in measurement update and split logic where heading is computed from position difference)

      // Sensor preprocessing
      float32_t rdot_interval_compatability_dealiasing_gate; // [m/s] Range rate dealiased gate for two range rate intervals to be compatable
      float32_t max_age_of_older_amb_dets;       // Sensor preprocessing module: Maximum time of keeping ambiguous historical detections
      float32_t max_age_of_older_no_amb_dets;    // Sensor preprocessing module: Maximum time of keeping non ambiguous historical detections
      float32_t max_age_of_older_dets_lr;        // Sensor preprocessing module: Maximum time of keeping long range historical detections
      float32_t k_probability_of_detection_default; // Sensor preprocessing module: Default probability of detection
      float32_t k_probability_of_detection_init; // Sensor preprocessing module: Initial probability of detection
      float32_t k_probability_of_detection_low; // Sensor preprocessing module: Low probability of detection
      float32_t k_probability_of_detection_medium; // Sensor preprocessing module: Medium probability of detection
      float32_t k_probability_of_detection_high; // Sensor preprocessing module: High probability of detection
      float32_t k_detection_probability_degradation_factor; // Sensor preprocessing module: Factor for lowering probability of detection in case of degraded antenna sensitivity
      float32_t k_default_detection_azimuth_std;// Sensor preprocessing module: Default value for standard deviation of detection azimuth
      float32_t k_default_detection_range_std; // [m] Sensor preprocessing module: Default value for standard deviation of detection range

      // Initialization calibrations.
      int32_t k_init_trk_preconditions_min_current_dets; // Initialize Tracks module: Preconditions: Minimal number of detection from current instance to satisfy preconditions.
      int32_t k_init_trk_preconditions_min_mov_dets_sr; // Initialize Tracks module: Preconditions: Minimal number of moving detection to satisfy preconditions.
      int32_t k_init_trk_preconditions_min_mov_amb_dets; // Initialize Tracks module: Preconditions: Minimal number of moving + stationary detection to satisfy preconditions.
      int32_t k_init_trk_preconditions_min_amb_dets; // Initialize Tracks module: Preconditions: Minimal number of ambiguous detection to satisfy preconditions.
      int32_t k_init_trk_preconditions_min_mov_dets_lr; // Initialize Tracks module: Preconditions: Minimal number of moving detection to satisfy preconditions for long range.

      float32_t k_init_trk_init_confidence; // Initialize Tracks module: Confidence of initialized objects.
      float32_t k_init_trk_max_frac_az_error_stat_mov; // Initialize Tracks module: Maximum fraction of possible azimuth error.

      float32_t k_init_trk_fast_moving_thresh; // [m/s] Speed threshold for an object to be considered fast moving during initial size determination.
      float32_t k_init_trk_stationary_length;  // [m] Initial length for a stationary object.
      float32_t k_init_trk_stationary_width;   // [m] Initial width for a stationary object.
      float32_t k_init_trk_slow_moving_length; // [m] Initial length for a slow moving object.
      float32_t k_init_trk_slow_moving_width;  // [m] Initial width for a slow moving object.
      float32_t k_init_trk_fast_moving_length; // [m] Initial length for a fast moving object.
      float32_t k_init_trk_fast_moving_width;  // [m] Initial width for a fast moving object.

      float32_t k_init_cca_range_var; // [m^2] Assumed range variance during initialization of position uncertainty for CCA objects
      float32_t k_init_cca_az_var; // [rad^2] Assumed azimuth variance during initialization of position uncertainty for CCA objects
      float32_t k_init_vel_errcov_scale_factor; // [-] Increase factor for velocity covariance when a new CCA object is born
      float32_t k_init_cca_min_acc_var; // [(m/s^2)^2] Minimum possible acceleration variance for newly initialized CCA objects
      float32_t k_init_cca_max_acc_var; // [(m/s^2)^2] Maximum possible acceleration variance for newly initialized CCA objects
      float32_t k_init_cca_k_obj_speed_for_max_acc_var; // [m/s] Object speed at which a newly initialized CCA object will get maximum possible initial acceleration variance

      // NEES CFMI module calibrations.
      int32_t k_nees_cfmi_extended_prec_max_dets; // NEES CFMI module: Maximum number of detection to run extended NEES CFMI preconditions.
      float32_t k_nees_cfmi_extended_prec_2_dets_max_std;  // NEES CFMI module: Maximum estimated velocity std to accept precondition for 2 detections.
      float32_t k_nees_cfmi_extended_prec_3_dets_max_std;  // NEES CFMI module: Maximum estimated velocity std to accept precondition for 3 detections.
      float32_t k_nees_cfmi_extended_prec_4_dets_max_std;  // NEES CFMI module: Maximum estimated velocity std to accept precondition for 4 detections.
      float32_t k_nees_cfmi_extended_prec_5_dets_max_std;  // NEES CFMI module: Maximum estimated velocity std to accept precondition for 5 detections.
      float32_t k_nees_cfmi_extended_prec_6_and_more_dets_max_std;  // NEES CFMI module: Maximum estimated velocity std to accept precondition for 6 and more detections.

      float32_t k_nees_cfmi_dt_to_split_dets; // NEES CFMI module: delta time to consider detections as coming form different time instances [s].

      float32_t k_nees_cfmi_extended_object_pos_std; // NEES CFMI module: Standard deviation of extended object assumption [m].
      float32_t k_nees_cfmi_process_noise_acc_std; // NEES CFMI module: Standard deviation of acceleration process noise [m/s^2].
      float32_t k_nees_cfmi_process_noise_yaw_rate_std_deg; // NEES CFMI module: Standard deviation of yaw rate process noise [deg/s].
      float32_t k_nees_cfmi_rrate_comp_std; // NEES CFMI module: standard deviation of detections compensated range rate which is used for NEES initialization [m/s].

      uint32_t k_nees_cfmi_min_estimates_num_for_vel_estimate; // NEES CFMI module: Minimum number of input estimates to estimate velocity.

      uint32_t k_nees_cfmi_stat_check_max_mov_dets; // NEES CFMI module: Stationary check: Maximum number of moving detections to check if object is stationary.
      float32_t k_nees_cfmi_stat_check_max_mov_dets_ratio; // NEES CFMI module: Stationary check: Maximum ratio of moving detections to check if object is stationary.

      float32_t k_nees_cfmi_init_cl_sl; // NEES CFMI module: Initial cloud sigma level.
      float32_t k_nees_cfmi_init_stat_pd_sl; // NEES CFMI module: Initial position difference sigma level for moving objects (assumed).
      float32_t k_nees_cfmi_init_mov_pd_sl; // NEES CFMI module: Initial position difference sigma level for stationary objects (assumed).

      float32_t k_nees_cfmi_min_weight_for_inlier; // NEES CFMI module: Minimum weight to treat estimate as inlier.

      float32_t k_sigma_level_dominant_velocity;  // NEES CFMI module: Dominant velocity sigma level.

      float32_t k_nees_cfmi_vh_check_cloud_sl_pd; // NEES CFMI module: Velocity Hypotheses checks: Cloud check: Sigma level for position difference source.
      float32_t k_nees_cfmi_vh_check_cloud_sl_cl; // NEES CFMI module: Velocity Hypotheses checks: Cloud check: Sigma level for cloud source.
      float32_t k_nees_cfmi_vh_check_cloud_sl_do; // NEES CFMI module: Velocity Hypotheses checks: Cloud check: Sigma level for dominant velocity source.
      float32_t k_nees_cfmi_vh_check_cpd_sl_pd; // NEES CFMI module: Velocity Hypotheses checks: Confirmed Position Difference check: Sigma level for position difference source.
      float32_t k_nees_cfmi_vh_check_cpd_sl_cl; // NEES CFMI module: Velocity Hypotheses checks: Confirmed Position Difference check: Sigma level for cloud source.
      float32_t k_nees_cfmi_vh_check_cpd_sl_do; // NEES CFMI module: Velocity Hypotheses checks: Confirmed Position Difference check: Sigma level for dominant velocity source.
      float32_t k_nees_cfmi_vh_check_dom_sl_pd; // NEES CFMI module: Velocity Hypotheses checks: Dominant check: Sigma level for position difference source.
      float32_t k_nees_cfmi_vh_check_dom_sl_cl; // NEES CFMI module: Velocity Hypotheses checks: Dominant check: Sigma level for cloud source.
      float32_t k_nees_cfmi_vh_check_dom_sl_do;  // NEES CFMI module: Velocity Hypotheses checks: Dominant check: Sigma level for dominant velocity source.
      float32_t k_nees_cfmi_vh_check_ransac_sl_pd; // NEES CFMI module: Velocity Hypotheses checks: RANSAC check: Sigma level for position difference source.
      float32_t k_nees_cfmi_vh_check_ransac_sl_cl; // NEES CFMI module: Velocity Hypotheses checks: RANSAC check: Sigma level for cloud source.
      float32_t k_nees_cfmi_vh_check_ransac_sl_do;  // NEES CFMI module: Velocity Hypotheses checks: RANSAC check: Sigma level for dominant velocity source.
      float32_t k_nees_cfmi_vh_check_radial_sl_pd; // NEES CFMI module: Velocity Hypotheses checks: Radial check: Sigma level for position difference source.
      float32_t k_nees_cfmi_vh_check_radial_sl_cl; // NEES CFMI module: Velocity Hypotheses checks: Radial check: Sigma level for cloud source.
      float32_t k_nees_cfmi_vh_check_radial_sl_do; // NEES CFMI module: Velocity Hypotheses checks: Radial check: Sigma level for dominant velocity source.
      float32_t k_nees_cfmi_vh_check_high_plausibility; // NEES CFMI module: Velocity Hypotheses checks: Minimum valid plausibility to break Velocity Hypotheses check loop
      float32_t k_nees_cfmi_vh_check_min_pd_plaus; // NEES CFMI module: Velocity Hypotheses checks: Minimum position difference plausibility to treat velocity hypothesis as valid.
      float32_t k_nees_cfmi_vh_check_min_cl_plaus; // NEES CFMI module: Velocity Hypotheses checks: Minimum cloud plausibility to treat velocity hypothesis as valid.
      float32_t k_nees_cfmi_vh_check_min_overall_plaus; // NEES CFMI module: Velocity Hypotheses checks: Minimum overall plausibility to treat velocity hypothesis as valid.

      float32_t k_nees_cfmi_plaus_expected_cl_in_ratio; // NEES CFMI module: Velocity Hypotheses checks: Plausibility Estimation: Expected ratio of cloud inliers.
      float32_t k_nees_cfmi_plaus_expected_cl_in_ratio_scale; // NEES CFMI module: Velocity Hypotheses checks: Plausibility Estimation: Expected ratio of cloud inliers - additional scale.
      float32_t k_nees_cfmi_plaus_expected_pd_in_ratio; // NEES CFMI module: Velocity Hypotheses checks: Plausibility Estimation: Expected ratio of position difference inliers.
      float32_t k_nees_cfmi_plaus_expected_pd_in_ratio_scale; // NEES CFMI module: Velocity Hypotheses checks: Plausibility Estimation: Expected ratio of position difference inliers - additional scale.

      float32_t k_nees_cfmi_ransac_expected_success; // NEES CFMI module: RANSAC: expected RANSAC success probability.
      uint32_t k_nees_cfmi_ransac_max_iterations; // NEES CFMI module: RANSAC: maximum number of RANSAC iterations.
      uint32_t k_nees_cfmi_ransac_min_iterations; // NEES CFMI module: RANSAC: minimum number of RANSAC iterations (to calculate number of iterations by expected success probability.
      float32_t k_nees_cfmi_ransac_plausibility_diff_thr; // NEES CFMI module: RANSAC: min plausibility difference to treat hypothesis as better without extra checks

      float32_t k_nees_cfmi_min_rr_plaus_thr_low_conf; // NEES CFMI module: Range rates plausibility threshold for low confidence level
      float32_t k_nees_cfmi_min_rr_plaus_thr_med_conf; // NEES CFMI module: Range rates plausibility threshold for medium confidence level
      float32_t k_nees_cfmi_min_rr_plaus_thr_high_conf; // NEES CFMI module: Range rates plausibility threshold for high confidence level
      float32_t k_nees_cfmi_rr_std_for_vh_confirm; // NEES CFMI module: Range rate standard deviation used for VH confirmation based on range rates. [m/s]
      float32_t k_nees_cfmi_sigma_level_for_vh_confirm; // NEES CFMI module: Sigma level used for VH confirmation based on range rates.

      float32_t k_nees_cfmi_cloud_hyp_max_vel_std_kph; // NEES CFMI module: Cloud velocity hypothesis validation based on raw covariance: maximum std [km/h].

      uint32_t k_nees_cfmi_m_est_max_iter; // NEES CFMI module: M-Estimator: Maximum number of iterations.

      float32_t k_nees_cfmi_m_est_msl_min_pd_plaus; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_min_cl_plaus; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal cloud plausibility.
      float32_t k_nees_cfmi_m_est_msl_pd_sl_for_min_plaus; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Sigma level for minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_pd_sl_neutral; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_pd_sl_for_max_over_inliers; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_cl_sl_for_min_plaus; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_cl_sl_for_max_plaus; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.
      float32_t k_nees_cfmi_m_est_msl_low_pass_filter_alpha; // NEES CFMI module: M-Estimator: Modify Sigma Levels: Minimal position difference plausibility.

      float32_t k_nees_cfmi_m_est_conv_check_min_speed; // NEES CFMI module: M-Estimator: Convergence Check: Minimum speed to calculate relative velocity difference [m/s].
      float32_t k_nees_cfmi_m_est_conv_check_max_vel_diff; // NEES CFMI module: M-Estimator: Convergence Check: Maximum velocity difference for convergence [m/s].
      float32_t k_nees_cfmi_m_est_conv_check_max_rel_vel_diff; // NEES CFMI module: M-Estimator: Convergence Check: Maximum relative velocity difference for convergence [%].

      float32_t k_nees_cfmi_m_est_plaus_min_pd_plaus; // NEES CFMI module: M-Estimator: Minimum position difference plausibility to treat velocity as valid.
      float32_t k_nees_cfmi_m_est_plaus_min_cl_plaus; // NEES CFMI module: M-Estimator: Minimum cloud plausibility to treat velocity as valid.
      float32_t k_nees_cfmi_m_est_plaus_min_overall_plaus; // NEES CFMI module: M-Estimator: Minimum overall plausibility to treat velocity as valid.

      float32_t k_nees_cfmi_vel_cov_host_inf_factor; // NEES CFMI module: Velocity Covariance: Host velocity covariance influence on object initial covariance.
      float32_t k_nees_cfmi_vel_cov_biger_eig_bias; // NEES CFMI module: Velocity Covariance: Covariance bias for higher eigenvalue [m/s].
      float32_t k_nees_cfmi_vel_cov_lower_eig_bias; // NEES CFMI module: Velocity Covariance: Covariance bias for lower eigenvalue [m/s].
      float32_t k_nees_cfmi_vel_cov_bias_factor; // NEES CFMI module: Velocity Covariance: Factor of extension of covariance (for bias estimation).

      float32_t k_nees_cfmi_stat_nees_test_alpha_for_stat;  // NEES CFMI module: Stationary NEES test: alpha level to reject for stationary NEES test for stationary 'hypothesis' objects.
      float32_t k_nees_cfmi_stat_nees_test_alpha_for_mov;  // NEES CFMI module: Stationary NEES test: alpha level to reject for stationary NEES test for moving 'hypothesis' objects.
      float32_t k_nees_cfmi_stat_nees_test_bias;  // NEES CFMI module: Stationary NEES test: Bias of covariance for this test.
      float32_t k_nees_cfmi_stat_nees_test_bias_factor;  // NEES CFMI module: Stationary NEES test: Factor of bias of covariance for this test.

      float32_t k_nees_cfmi_stat_nees_max_test_var; // NEES CFMI module: Stationary NEES test: Maximum test variable value

      float32_t k_nees_cfmi_stat_dom_hyp_sigma_level;  // NEES CFMI module: Stationary hypothesis check: Dominant velocity sigma level (threshold) for NEES test.
      float32_t k_nees_cfmi_stat_dom_hyp_max_speed;  // NEES CFMI module: Stationary hypothesis check: Dominant velocity maximum speed to consider NEES test.
      float32_t k_nees_cfmi_stat_dom_hyp_min_dt;  // NEES CFMI module: Stationary hypothesis check: Dominant velocity minimum dt to consider NEES test.

      float32_t k_nees_cfmi_stat_inliers_check_sl_cl;  // NEES CFMI module: Stationary hypothesis check: Inliers check: Sigma level for cloud.
      float32_t k_nees_cfmi_stat_inliers_check_sl_pd;  // NEES CFMI module: Stationary hypothesis check: Inliers check: Sigma level for position difference.
      float32_t k_nees_cfmi_stat_inliers_check_sl_do;  // NEES CFMI module: Stationary hypothesis check: Inliers check: Sigma level for dominant velocity.
      float32_t k_nees_cfmi_stat_inliers_check_min_det_plaus;  // NEES CFMI module: Stationary hypothesis check: Inliers check: Min Cloud plausibility to accept hypothesis.

      float32_t k_nees_cfmi_vel_plaus_thr; // NEES CFMI module: Velocity Plausibility: Plausibility threshold to validate velocity without extra checks.
      float32_t k_nees_cfmi_vel_plaus_cov_trace_low_plaus; // NEES CFMI module: Velocity Plausibility: Covariance trace for low plausibility.
      float32_t k_nees_cfmi_vel_plaus_cov_trace_high_plaus; // NEES CFMI module: Velocity Plausibility: Covariance trace for high plausibility.
      float32_t k_nees_cfmi_vel_plaus_cov_low_plaus; // NEES CFMI module: Velocity Plausibility: Covariance trace low plausibility.
      float32_t k_nees_cfmi_vel_plaus_cov_high_plaus; // NEES CFMI module: Velocity Plausibility: Covariance trace high plausibility.
      float32_t k_nees_cfmi_vel_sanity_max_speed; // NEES CFMI module: Velocity Sanity check: Maximum valid speed.
      float32_t k_nees_cfmi_vel_sanity_max_vel_cov_trace; // NEES CFMI module: Velocity Sanity check: Maximum valid covariance trace.

      bool f_nees_cfmi_extended_preconditions; // NEES CFMI module: Flag indicating if extra preconditions should be executed.
      bool f_nees_cfmi_init_with_not_plausible_vel; // Flag indicating if object should be initialized when velocity is valid but not plausible.
      uint8_t padding_ness[2];

      int32_t k_obj_init_min_number_of_correct_dets; //minimal number of correct detection (non angle jumps) to consider cluser being unaffected
      float32_t k_obj_init_min_correct_dets_ratio; //minimal ration of correct detctions (non angle jumps) to all detections to consider cluser being unaffected

      // Clusters Grouping parameters
      float32_t k_rdot_half_gate;
      float32_t k_radial_gate; // [m] Size of world-az axis of elliptical fine position gate
      float32_t k_max_crossing_speed; // [m/sec] Rate at which orthogonal-to-world-az axis of elliptical fine position gate
      float32_t k_cross_radial_expand_factor; // [-] Factor for calculating cross radial gate for clusters
      float32_t k_stat_clusters_dist_sq_coarse_gate; // [m^2] Squared distance gate for clusters
      float32_t k_moving_clusters_dist_coarse_gate; // [m] Longitudinal distance gate for clusters
      float32_t k_moving_clusters_dist_sq_coarse_gate_1; // [m^2] Smaller squared distance gate for clusters with lower time_since_measurement diff
      float32_t k_moving_clusters_dist_sq_coarse_gate_2; // [m^2] Larger squared distance gate for clusters with higher time_since_measurement diff
      float32_t k_moving_clusters_time_diff_coarse_gate; // [ms] Time difference gate between two clusters
      float32_t k_max_dealiased_range_rate_diff; // [m/s] Maximum difference in dealiased calculated target range rate and detection range rate to dealiase detection.

      // Priority parameters.
      float32_t k_priority_default_cluster_confidence; // Priority Function: default confidence for clusters.
      float32_t k_priority_distance_coefficient; // Priority Function: this coefficient is multiplied by distance priority, and then it's used for priority normalization
      float32_t k_priority_confidence_coefficient; // Priority Function: this coefficient is multiplied by number of detections priority, and then it's used for priority normalization
      float32_t k_priority_headway_coefficient; // Priority Function: this coefficient is multiplied by headway priority, and then it's used for priority normalization
      float32_t k_priority_f_movable_coefficient; // Priority Function: this coefficient is multiplied by movable priority, and then it's used for priority normalization
      float32_t k_priority_distance_for_min_priority_inverse; // Priority Function: inverse of distance after object do not gain distance priority (F360_MIN_PRIORITY)
      float32_t k_priority_headway_for_min_priority_inverse; // Priority Function: inverse of headway time that object do not gain priority (F360_MIN_PRIORITY)
      float32_t k_priority_lat_penalty_max_dist_inverse; // Priority Function: inverse of maximum lateral penalty for lateral distance in curvilinear coordinate system  (F360_MIN_PRIORITY)
      bool k_priority_default_cluster_movable; // Priority Function: default movable status for clusters.
      uint8_t padding_priority[3];

      // Measurement update parameters
      float32_t k_ref_msmt_cov_cca; // Reference variance for range rate compensated measurements in CCA measurement update.
      float32_t k_ref_msmt_cov_ctca; // Reference variance for range rate compensated measurements in CTCA measurement update.
      uint32_t k_min_num_selected_dets_per_sensor_for_binning; // If an object is eligible for binning & has more detections than this from a single sensor, bin detections

      // Reference point update parameters
      float32_t k_normal_obj_ref_pnt_hysteresis_factor; // [-] Hysteresis factor for switching from old reference point to new.
      float32_t k_normal_obj_length_thr; // [m] Hysteresis factor for switching from old reference point to new.
      float32_t k_long_obj_length_thr; // [-] Hysteresis factor for switching from old reference point to new.
      float32_t k_long_obj_ref_pnt_hysteresis_factor; // [m] Hysteresis factor for switching from old reference point to new.
      float32_t k_ref_point_state_update_speed_threshold;  // [m/s] Speed threshold used when determining whether to do state update after reference point change.
      float32_t k_sin_max_pointing_error_sq; // [-] Square of sinus of assumed maximum object pointing error used for increaing position uncertanty after reference point switch
      float32_t k_frac_of_moved_dist; // [-] Scaling parameter that is multplied with the moved distance of the reference point in order to determin how much position uncertainty should be increased after reference point switch

      // Determine reflected object
      float32_t k_mirror_prob_threshold; // Threshold if object should be reduced or not based on its mirror probability, if mirror probablity is higher than this value the object will not be reduced
      float32_t k_reflective_guardrail_track_min_host_speed; // [m/s] Track Validity module: Minimum host speed for which Is_Reflective_Guardrail_Track is run
      float32_t k_reflected_object_max_mirror_probability; // [-] Track Validity module: Maximum mirror probability assigned to an object that is suspected of being a reflection

      // Determine to freeze track
      float32_t rdot_comp_mov_obj_tresh; // Threshold for verifying whether any det is moving direclty
      float32_t pred_abs_vcs_xvel_tresh; // Threshold for verifying whether any det is moving indirectly
      float32_t max_abs_vcs_yvel_tresh; // Constant for extending detection range rate compensated basing on detection vcs azimuth
      float32_t max_abs_cos_azim_vcs_for_det_outside_cone_of_silence; // Threshold for verifying whether detection is in cone of silence
      float32_t min_object_age_for_object_close_to_stat_host; // Minimal age to classify an object as "close to host" when host is stationary

      // Guardrail based angle jump detector
      float32_t k_angle_jump_range_tolerance; // [m] detection range gap condition -> (dist_to_guardrial - tollerance, dist_to_guardrial + tollerance)
      float32_t k_angle_jump_max_abs_range_rate; // [m/s] maximum allowed detection range rate
      float32_t k_angle_jump_min_abs_azimuth_vcs; // [rad] minimum detection azimuth (in vcs) condtion to filter out detection from detector
      float32_t k_angle_jump_max_abs_azimuth_vcs; // [rad] maximum detection azimuth (in vcs) condtion to filter out detection from detector
      float32_t k_angle_jump_max_range; // [m] maximum detection's range for angle jump checking
      float32_t k_angle_jump_long_search_margin; // [m] longitudinal search margin for SEP

      // Mark stationary bounced detections
      float32_t k_stat_bounce_min_host_speed; // [m/s] Minimum host speed for lookng for stationary bounce detections.
      float32_t k_stat_bounce_max_trk_long_posn; // [m] Maximum longitudinal position of true object for which tracker is lookng for stationary bounce detections.
      float32_t k_stat_bounce_max_trk_lat_dist; // [m] Maximum lateral distance from host of true object for which tracker is lookng for stationary bounce detections.
      float32_t k_stat_bounce_max_trk_heading; // [rad] Maximum heading of true object for which tracker is lookng for stationary bounce detections.
      float32_t k_stat_bounce_azimuth_border_ext;// [rad] Extension of left and right azimuth borders of zone where algorithm looks for stationary bounced detections.
      float32_t k_stat_bounce_range_rate_diff_thr; // [m/s] Maximum difference between reported detection range rate and expected detection range rate to flag detection as stationary bounce.
      float32_t k_stat_bounce_min_det_long_posn; // [m] Minumum longitudinal position of detecion, which may be flagged as stationary bounce.

      // Filter out low quality detections on or behind guardrail
      float32_t k_max_rcs_thr_low_quality_detection_filter; // [dB] Detection maximum rcs for filtering away detections behind/on guardrail with low rcs and low azimuth confidence.
      int8_t k_azimuth_conf_low_quality_detection_filter; // [-] Detection azimuth confidence for filtering away detections behind/on guardrail with low rcs and low azimuth confidence.
      uint8_t padding_low_quality_detection[3];

      // MRR360 bandaid for filtering away detections with big absolute elevation angle
      float32_t k_mrr360_max_abs_elev_angle_rad;
      float32_t k_mrr360_min_host_speed_el_check;

      // Parameters for Mark_Detections_With_Neighbours
      float32_t max_dist_for_neighbor_detections; // [m] Maximum distance between detections to classify them as neighbors

      // Parameters for Detect_Wheel_Spin_Pairs
      float32_t k_max_wheel_spin_dist_sq;  // [m^2] max squared position difference to consider two detections close
      float32_t k_min_wheel_spin_doppler_spread;  // [m/s] min diff in range rate between close detections to consider them wheel spin
      float32_t k_max_abs_vcs_long_posn_for_wheelspin_pair; // [m] Maximum absolute longitude pos to check for wheelspin pair
      float32_t k_max_abs_vcs_lat_posn_for_wheelspin_pair; // [m] Maximum absolute latitude pos to check for wheelspin pair
      float32_t k_max_azimuth_difference_for_wheelspin_pair; // [rad] max diff in azimuth to consider two detections close
      uint8_t k_wheelspin_pair_max_close_det_iterations; // Maximum number of iterations to search for close detection to a wheelspin pair
      uint8_t padding_wheelspin[3];
      uint32_t k_max_wheel_spin_dets_to_mark; //max number of detection pairs to be marked as wheel spin

      // Parameters for Mark_Detections_Wheel_Spin_From_Objects
      float32_t k_max_abs_vcs_long_posn_for_wheelspin;  // [m]   Maximum distance between object and host to consider wheelspin countermeasure
      float32_t max_heading_angle_for_close_target;     // [rad] Maximum heading angle diff between object and host for a close target.
      float32_t k_parallel_ang_threshold;               // [rad] Maximum heading angle diff between object and host. If exceeded bounding box is not extended
      float32_t k_min_speed_fast_moving;                // [m/s] Minimum object speed to flag object as fast moving
      float32_t k_min_rr_diff_wheelspin;                // [m/s] Minimum allowed difference between detection RR and object predicted RR, otherwise not marked as WS
      float32_t k_ws_lat_buffer_zone_oncoming;          // [m]   Extension of bounding box in lateral direction for an oncoming object with motion parallel to host
      float32_t k_ws_lat_buffer_zone;                   // [m]   Extension of bounding box in lateral direction for an object with motion parallel to host
      float32_t k_ws_long_buffer_zone;                  // [m]   Extension of bounding box in longitudinal direction

      // Parameters for Detect_Near_By_Wheel_Spins
      float32_t k_nbws_max_lat_pos;      // [m] maxiumum lateral position of detection to be used by algorithm (are of interest)
      float32_t k_nbws_min_lat_pos;      // [m] minimum lateral position of detection to be used by algorithm (are of interest)
      float32_t k_nbws_max_long_pos;     // [m] maxiumum longitudinal position of detection to be used by algorithm (are of interest)
      float32_t k_nbws_min_long_pos;     // [m] minimum longitudinal position of detection to be used by algorithm (are of interest)
      float32_t k_nbws_lat_marking_th;   // [m] lateral distance threshold from cluster to detection for marking
      float32_t k_nbws_long_marking_th;  // [m] longitudinal distance threshold from cluster to detection for marking
      float32_t k_nbws_lat_asc_th;       // [m] lateral distance threshold from cluster to detection for cluster association
      float32_t k_nbws_long_asc_th;      // [m] longitudinal distance threshold from cluster to detection for cluster association

      // Parameters for Mark_Dets_As_Farside_And_Close_Target
      float32_t k_ct_and_fcm_max_abs_heading_diff_to_host; // [rad] Maximum absolute value of heading difference between host and track to be considered as parallel
      float32_t k_ct_and_fcm_max_dist_for_close_target_sq; // [m^2] Maximum distance squared that an object can be at to be considered for function Mark_Dets_As_Farside_And_Close_Target
      float32_t k_ct_orth_buffer_zone_factor;              // Factor multiplied by edge distance to determine how much the ct box should be extended in orth direction
      float32_t k_ct_para_buffer_zone_factor;              // Factor multiplied by edge distance to determine how much the ct box should be extended in para direction

      // Parameters for Cond_Deassoc_Low_RR_Dets
      float32_t k_cond_deassoc_min_obj_spd_for_deassoc; // [m/s] Minimum object speed to consider deassociation of detections
      float32_t k_cond_deassoc_det_comp_rr_max; // [m/s] Maximum detection compensated range rate to consider deassociation of detection
      float32_t k_cond_deassoc_fraction_of_width_to_deassoc; // [] Fraction of width that detection has to be from the visible orth edge to be deassociated

      // Existance Probability
      float32_t k_ep_init_p_det_sensor; // [-] default probability that object is detected by sensor
      float32_t k_ep_p_measurement_with_no_new_meas; // [-] probability of getting good measurement when object is not updated (coasted)
      float32_t k_ep_clutter_prob_with_meas; // [-] probability that new information from sensor is reported as false positive (ghost).
      float32_t k_ep_clutter_prob_with_no_meas;  // [-] probability factor when no new information is associated witch track object
      float32_t k_ep_min_allowed_exist_prob;  // [-] minimal probability value
      float32_t k_ep_sensor_degradation_factor; // [-] If sensor performance degraded (bad data quality) then this calibration is used to probability calculation.
      float32_t k_ep_prob_track_state_exp_scale;  // [-] scaling factor for quality of state estimate of the track
      float32_t k_ep_prob_track_state_exp_offset;  // [-] offset for approximation of sum(inv(Var_i))
      float32_t k_ep_bottom_saturation_of_normalized_variance; // [-] low state variance saturation level
      float32_t k_ep_variance_th_pos_xy; // [m^2] threshold for position variance
      float32_t k_ep_variance_th_heading; // [rad^2] threshold for heading variance
      float32_t k_ep_variance_th_velocity; // [(m/s)^2] threshold for velocity variance
      float32_t k_ep_variance_th_curvature; // [(1/m)^2] threshold for curvature variance
      float32_t k_ep_variance_th_accel; // [(m/s^2)^2] threshold for acceleration variance
      float32_t k_ep_variance_th_tan_accel; // [(1/s^2)^2] threshold for tangential acceleration variance
      float32_t k_ep_prob_track_state_init_value; // [-] init value for quality of the state estimate of the track.
      float32_t k_ep_init_factor_min_val; // [-] minimum value of penalty factor for init existence probability calculation
      float32_t k_ep_init_factor_max_val; // [-] maximum value of penalty factor for init existence probability calculation
      uint32_t k_ep_init_factor_exponent; // [-] exponent vlaue for init existence probability calculation

      // Track Downselection
      float32_t k_track_downselect_max_priority;  //Maximum allowed priority value.
      float32_t k_track_downselect_max_vcs_x_range_to_preserve;  // Used to determine distance from host where tracks have high priority [m].
      float32_t k_track_downselect_max_vcs_y_range_to_preserve;  // Used to determine distance from host where tracks have high priority [m].
      float32_t k_track_downselect_min_negative_ttc; // Min negative time to conntact - ttc [s].
      float32_t k_track_downselect_dets_threshold;   // Treshold for filtered over the time number of detection.
      float32_t k_track_downselect_dets_threshold_low;  //Low dets treshold for filtered over the time number of detection.
      float32_t k_track_downselect_average_dets_thresh; // Average dets treshold for filtered over the time number of detection.
      float32_t k_track_downselect_min_time_filter_dets_thresh;  // Min time treshold for filtering numer of detections [s].
      float32_t k_track_downselect_confidence_level_lowering_factor;   // Factor for lowering confidence in some specal cases.
      float32_t k_track_downselect_upper_ttc_saturation_level;   // upper ttc saturation level for mooving objects [s].
      float32_t k_track_downselect_lower_ttc_saturation_level;   // lower ttc saturation level for mooving objects [s].
      float32_t k_track_downselect_low_priority_level_for_nonmoveable_tracks;   // Low priority level sor nonmoveable tracks.
      float32_t k_track_downselect_confidence_thresh;   // Confidence threshold used to determine track priority level.
      float32_t k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object;  // maximum allowed yawrate for non fast yawing object behind guardrail [rad/s].
      float32_t k_track_downselect_max_allowed_speed_for_non_fast_yawing_object;  // maximum allowed speed for non fast yawing object behind guardrail [m/s].
      float32_t k_track_downselect_max_confidence_for_possibly_ghost; // [-] Track Downselection module: maximum allowed confidence level for an object possibly considered as ghost

      // Track grouping
      float32_t k_track_grouping_half_long_extension; // [m] Length extension (half value) of bounding box for merging test
      float32_t k_track_grouping_half_lat_extension;  // [m] Width extension (half value) of bounding box for merging test
      float32_t k_track_grouping_hdg_gate;     // [rad] Heading gate for grouping two objects
      float32_t k_track_grouping_speed_gate;   // [m/s] Speed gate for grouping two objects
      float32_t k_track_grouping_curvature_gate;   // [rad/m] Curvature gate for grouping two objects
      float32_t merging_m2m_distance_threshold;                  // [m] maximum metal to metal distance between 2 objects to allow merge
      float32_t merging_lateral_det_spread_threshold;            // [m] maximum lateral spread of detection between 2 objects to allow merge
      float32_t merging_m2m_max_obj_speed;                       // [m/s] maximum objects speed for checking metal to metal and lateral spread conditions
      float32_t k_orth_split_width_threshold;                    // [m] Maximum width for objects after split

      // Calculate Time To Collision (ttc)
      float32_t k_calc_ttc_min_thresh_projected_velocity;  // Minimal value of velocity projected in to host direction [m/s].
      float32_t k_calc_ttc_max_thresh_projected_velocity;  // Maximal value of velocity projected in to host direction [m/s].
      float32_t k_calc_ttc_min_obj_track_range;   //Minimal range from host to object track for calculating of ttc [m];

      // Post Update Track Adjustments
      float32_t k_puta_max_vcs_xposn_for_ghost_NU_2_C; // [m] maximum allowed longitudinal distance to set f_ghost_NU_2_C flag
      float32_t k_puta_max_vcs_yposn_for_ghost_NU_2_C; // [m] maximum allowed lateral distance to set f_ghost_NU_2_C flag
      float32_t k_puta_overlapping_tracks_max_speed_diff; // [m/s] Maximum speed difference between the two objects to be considered as overlapping.
      float32_t k_puta_overlapping_tracks_max_heading_diff; // [rad] Maximum heading difference between the two objects to be considered as overlapping.
      float32_t k_puta_overlapping_tracks_high_conf_thr; // [-] Minimum required confidence threshold for the object with the higher confidence in overlapping logic
      float32_t k_puta_overlapping_tracks_low_conf_thr; // [-] Maximum required confidence threshold for the object with the lower confidence in overlapping logic
      float32_t k_puta_overlapping_tracks_long_thr; // [m] Maximum absolute difference in longitudinal position between the two relevant objects to continue more detailed checks for overlapping tracks

      float32_t k_puta_obj_size_acc_filt_coef_innov_coasting_obj; // [-] Filter coefficient in object size accuracy for computing innovation for coasting object
      float32_t k_puta_obj_size_acc_filt_coef_innov_updated_obj; // [-] Filter coefficient in object size accuracy for computing innovation for updated object
      float32_t k_puta_obj_size_acc_innov_no_update_length; // [-] Filter coefficient in object size accuracy for computing innovation length
      float32_t k_puta_obj_size_acc_innov_no_update_width; // [-] Filter coefficient in object size accuracy for computing innovation width

      float32_t k_puta_min_object_confidence; // [-] Minimum object confidence level to be treated as trusted track.
      float32_t k_puta_min_object_time; // [s] Minimum object time since cluster was created to treat it as trusted track.
      float32_t k_puta_large_distance; // [m]  Large distance between two object, which is set when object to compare is not trusted.
      float32_t k_puta_orientation_diff_threshold; // Maximum orientation difference to allow kill object.

      // Vcs longitudinal sorted detection rerence points in longitudinal direction in ascending order
      // As detections are sorted the first one that have a long pos greater than a reference point is "tagged" by
      // saving its detection index in tracker info structure. The array vcs_long_sorted_ref_points contains reference
      // points that must be defined in ascending order.

      // Association: Parameters for detection to track association
      float32_t k_min_assoc_gate_extension_non_moveable; // [m] Minimum possible extension of the circular association gate used for non-moveable objects
      float32_t k_max_assoc_gate_extension_non_moveable; // [m] Maximim possible extension of the circular association gate used for non-moveable objects
      float32_t k_max_assoc_gate_radius_non_moveable; // [m] Maximim possible radius of the circular association gate used for non-moveable objects
      float32_t k_obj_dist_for_min_assoc_gate_extension_non_moveable; // [m] Object distance at which the extension of the circular association gate used for non-moveable objects should be minimum possible
      float32_t k_obj_dist_for_max_assoc_gate_extension_non_moveable; // [m] Object distance at which the extension of the circular association gate used for non-moveable objects should be maximum possible
      float32_t k_spd_dependent_assoc_gate_extension_factor_non_moveable; // [1/s] Multiplication factor for the speed dependent part of the extension of the circular association gate used for non-moveable objects
      float32_t k_min_speed_for_increasing_occluded_long_assoc_buffer; // [m/s] For objects with speeds larger than this the association gates in the less visible para direction will be increased
      float32_t k_max_speed_for_saturating_occluded_long_assoc_buffer_increase; // [m/s] For objects with speeds larger than this the increase of the association gates in the less visible para direction will be saturated at maximum level
      float32_t k_max_occluded_long_buffer_increase; // [m] The maximum possible increase of the association gates in the less visible para direction of an object
      float32_t k_min_assoc_gate_long_buffer_moveable_objs; // [m] Minimum size of the longitudinal association buffer for moveable objects
      float32_t k_min_assoc_gate_lat_buffer_moveable_objs; // [m] Minimum size of the laterall association buffer for moveable objects
      float32_t k_range_rate_score_threshold; // Threshold on range rate for association in the general case [m/s]
      float32_t k_range_buffer_max_dist; // [m] Maximum saturation distance for calculating range buffer
      float32_t k_range_buffer_min_val; // [m] Buffer value at minimum saturation distance for calculating range buffer
      float32_t k_range_buffer_max_val; // [m] Buffer value at maximum saturation distance for calculating range buffer
      float32_t k_az_buffer_max_dist; // [m] Maximum saturation distance for calculating azimuth buffer
      float32_t k_az_buffer_min_val; // [m] Buffer value at zero distance for calculating azimuth buffer
      float32_t k_az_buffer_max_val; // [m] Buffer value at maximum saturation distance for calculating azimuth buffer
      float32_t k_SEP_association_zone_extension; // [m] Extension of SEP bounding box in all directions in VCS.
      float32_t k_slow_moving_ctca_min_uncertainty_assoc_buffer; // [m] Minimum value of the uncertainty contribution to the extended bounding box for slow moving CTCA objects.
      float32_t k_max_uncertainty_assoc_buffer; // [m] Maximum value of the uncertainty dependent contribution to the extended bounding box.
      float32_t k_slope_coeff_uncertainty_assoc_buffer; // [m] Contribution from object position covariance to the extended bounding box corresponds to a linear function. The slope coefficient is given by this calibration parameter.
      float32_t k_offset_coeff_uncertainty_assoc_buffer; // [m] Contribution from object position covariance to the extended bounding box corresponds to a linear function. The offset coefficient is given by this calibration parameter.
      float32_t k_min_rr_diff_from_stationary_hypothesis; // [m/s] Minimum difference between moving and stationary range rate hypotheses.

      // Association: Parameters for calculating association detection score
      float32_t k_base_score_inside_bbox; // Minimum score for a detection inside an objects bounding box
      float32_t k_para_diff_weight_inside_box; // Weight for detection inside a moveable objects bounding box in para direction
      float32_t k_orth_diff_weight_inside_box; // Weight for detection inside a moveable objects bounding box in ortho direction
      float32_t k_rdot_diff_weight_inside_box; // Weight for a detections range rate inside a moveable objects bounding box
      float32_t k_dist_weight_inside_solid_circle; // Weight for detection inside a non-moveable objects outline (circel)
      float32_t k_rdot_diff_weight_inside_solid_circel; // Weight for a detections range rate inside a non-moveable objects outline (circel)
      float32_t k_base_score_inside_ext_bbox; // Minimum score for a detection inside an objects extended bounding box
      float32_t k_para_diff_weight_inside_ext_box; // Weight for detection inside a moveable objects extended bounding box in para direction
      float32_t k_orth_diff_weight_inside_ext_box; // Weight for detection inside a moveable objects extended bounding box in ortho direction
      float32_t k_rdot_diff_weight_inside_ext_box; // Weight for a detections range rate inside a moveable objects extended bounding box
      float32_t k_dist_weight_inside_ext_circle; // Weight for detection inside a non-moveable objects association gate but outside of its outline (circel)
      float32_t k_rdot_diff_weight_inside_ext_circel; // Weight for a detections range rate inside a non-moveable objects association gate but outside of its outline (circel)
      float32_t k_score_outside_ext_bbox; // Score to use if detection is outside an objects extended bounding box
      float32_t k_base_score_bbox_center; // Base Score at the bounding box center

      // Association: Parameters for calculating range rate threshold
      float32_t k_vcs_distance_sqr_thr; // Distance squared from VCS for an object to be considered as "far away"
      float32_t k_rr_thr_factor_far_away_coasted; // Scaling factor of range rate threshold for objects coasting and "far away"
      float32_t k_rr_thr_factor_fov_edge; // Scaling factor of range rate threshold if detection is in the edge of FoV
      float32_t k_speed_threshold; // Speed threshold to allow expansion of range rate threshold for far away coasted objects

      // Association: Parameters for detection inlier selection
      float32_t k_rr_error_statistics_forgetting_factor; // [-] Determines the intertia of the mean and variance filter for range rate errors. Low forgetting factor means more history is ignored.
      float32_t k_max_number_of_historic_dets_obj_non_movable; // [-] Maximum number of historic dets after forgetting factor is applied for non-movable objects.
      float32_t k_max_number_of_historic_dets_obj_movable; // [-] Maximum number of historic dets after forgetting factor is applied for movable objects.
      float32_t k_max_historic_rr_error_variance; // [m^2/s^2] An object's maximum variance of historic range rate errors.
      float32_t k_min_range_rate_error_threshold; // [m/s] Minimum threshold for accepting range rate inliers.

      // Object tracks properties
      float32_t k_underdrive_min_trk_long_posn; // [m] maximal longitudinal position of considered tracks

      float32_t k_underdrive_min_zone_long_posn; // [m] minimal longitudinal position of tracks that can be in zone FOV
      float32_t k_underdrive_lat_buffer_factor; // Number that multiplies to the max_ocg_cell_width, and the result is used as a lateral threshold for ocg ud classification 

      // Underdrivability for moving objects
      float32_t ud_mov_forgetting_factor; // [-] Determines the intertia of the mean height. Low forgetting factor means more history is ignored.
      float32_t ud_mov_height_threshold;  // [m] Height threshold above which the object is suspected as underdrivable
      uint32_t ud_mov_cnt_consecutive_scans; // [-] Determines the number of scans for which object has to have historic height mean higher than threshold for underdrivability status to change
      float32_t ud_mov_max_n_historic_dets; // [-] Determines the max number of historical detections 
      float32_t ud_mov_posx_min_limit; // [m] Determines the minimum position x limit where we start to determine ud status for moving objects
      float32_t ud_mov_posx_max_limit; // [m] Determines the maximum position x limit to which we determine ud status for moving objects
      float32_t ud_mov_prob_can_pass_under; // [-] Defines the probability that the object is underdrivable when it has status "UNDERDRIVABLE_STATUS_CAN_PASS_UNDER"
      float32_t ud_mov_prob_can_not_pass_under; // [-] Defines the probability that the object is underdrivable when it has status "UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER"
      float32_t ud_mov_prob_not_to_consider; // [-] Defines the probability that the object is underdrivable when it has status "UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER"

      // Occupancy Grid related calibrations
      float32_t k_ocg_underdrive_small_curvature_th;  // [m^(-1)] minimal curvature of predicted host path to consider it as straight line
      bool f_ocg_use_curvilinear_simplification;  // Flag that indicates if the small curvature based simplication needs to used for object UD status assignment

      // radar phenomena
      float32_t rp_max_object_lateral_distance; // [m] max object lateral postion to be considered
      float32_t rp_max_abs_pointing_disagreement; // [rad] max absolute object pointing to be considered
      float32_t rp_object_max_longitudinal_margin; // [m] object longitudinal gap between sensor long position and object bumpers position to be considered
      float32_t rp_min_confidence_level; // [-] object minimum confidence level to be considered as reference object

      //object based angle jump detector
      float32_t obj_aj_det_range_gap; // [m] detection range gap
      float32_t obj_aj_max_allowed_rr_diff; // [m/s] max allowed difference between orignal diff and suspect diff (used to check suspect and original similarity)
      float32_t obj_aj_azimuth_jump_value; // [rad] value of angle jump (azimuth)
      float32_t obj_aj_border_half_width; // [m] half of border width that is use for detection filtration
      float32_t obj_aj_obj_length_reduction_factor; // reduction factor of object length used for detection filtration
      float32_t obj_aj_max_obj_length_reduction; // [m] maximum value of object length reduction (twice, front and back)
      float32_t obj_aj_suspected_rr_handicap; // [m/s] handicap range rate difference for suspected detection (it helps to not avoid marking det as angle jump when its "new" and origin range rates are similar)
      float32_t obj_aj_max_double_range_hypothesis; // [m] max detection range for double (doublebounce) hypothesis

      // object based multibounce detector
      float32_t mb_range_rate_diff_th;      //[m/s] range rate diff threshold between estimated range rate and range rate expected from object velocity
      float32_t mb_max_det_range;           //[m] maximum detection range to be checked by the algorithm
      float32_t mb_restricted_area_width;   //[m] width of resticted area
      uint8_t mb_max_num_bounces;        //[-] maximum bounce factor. Above this detetions are not considered as multibounces
      uint8_t padding_mb[3];

      // Pseudo position estimation
      float32_t k_pseudo_pos_gain_weighted_mean; // [-] Gain on variance from edge weighted mean
      float32_t k_pseudo_pos_var_bias_para; // [m^2] Bias to be added to the calculated para variance from edge weighted mean
      float32_t k_pseudo_pos_var_bias_orth; // [m^2] Bias to be added to the calculated orth variance from edge weighted mean
      float32_t k_pseudo_pos_gain_simple_estimate; // [-] Gain on previous error covariance when no estimation has been done for edge
      float32_t k_pseudo_pos_max_variance_threshold; // [m^2] Saturation threshold for pseudo position variances
      float32_t k_pseudo_pos_dist_diff_gain; // [-] Gain to be multiplied with distance (between time prediction and pseudo position) function for adding covariance to pseudo position measurement
      float32_t k_pseudo_pos_dist_diff_thr; // [m] Threshold in distance (between time prediction and pseudo position) function for adding covariance to pseudo position measurement. If distance is smaller than this value, no extra covariance is added.

      // Pseudo centroid position
      float32_t k_pcp_offset_length_factor;            // [-] Offset factor to object pseudo centroid position in para direction
      float32_t k_pcp_offset_width_factor;             // [-] Offset factor to object pseudo centroid position in orth direction

      // Uncertainty bias for pseudo position measurement
      float32_t k_pseudo_pos_cov_matrix_bias;              // [m^2] Bias added to pseudo pos variance when estimated edge is visible
      float32_t k_pseudo_pos_cov_matrix_bias_non_movable;  // [m^2] Bias added to pseudo pos variance for non movable objects (with reference point center)

      float32_t k_time_since_init_th_to_enable_outlier_mitigation_cca; // [s] Threshold on minimum time since initialization ob object for enabling the outlier mitigation function for pseudo pos covariance estimation for CCA objects

      // Paramerters for pseudo position in FOVE cases
      float32_t k_pseudo_pos_high_uncertainity;             // [m^2] Pseudo pos meascov when an object is partiallly outside the FOV, such that, a specific position element in TCS (x or y) has close to no impact during msmt update
      float32_t k_fov_normal_rotation_angle;               //  [rad] The angle that describes the rotation of the FOV normals, in order to shrink the FOV

      // Polar uncertainty used in pseudo position measurements
      float32_t k_range_var;                                  // [m^2] Polar pseudo position measurement variance in range direction.
      float32_t k_az_var;                                     // [rad^2] Polar pseudo position measurement variance in azimuth direction.
      float32_t k_raw_pseudo_pos_cov_max_saturation_distance; // [m] Range saturation limit for pseudo measurement variance.

      // Water spray detectors
      float32_t k_ws_min_speed; // [m/s] Min speed of object or host to search for water spray detections
      float32_t k_ws_max_det_rcs; // [dB] Maximum rcs of detection for it to be checked for water spray.

      // Host water spray parameters
      float32_t k_hws_para_box_host_speed_factor; // [1/s] Factor to multiply by speed host to derive box in para direction where to search for water spray detections. The greater the speed the larger the box.
      float32_t k_hws_ortho_box_host_speed_factor; // [1/s] Factor to multiply by speed host to derive box in ortho direction where to search for water spray detections. The greater the speed the larger the box.

      // Object based water spray detector
      float32_t k_ows_min_long_pos; // [m] Min value of VCS zone to search for water spray detections
      float32_t k_ows_max_long_pos; // [m] Max value of VCS zone to search for water spray detections
      float32_t k_ows_max_lat_pos; // [m] Max lateral of VCS zone to search for water spray detections. Symmetric on both sides of host.
      float32_t k_ows_para_box_obj_speed_factor; // [1/s] Factor to multiply by speed host to derive box in para direction where to search for water spray detections. The greater the speed the larger the box.
      float32_t k_ows_ortho_box_obj_speed_factor; // [1/s] Factor to multiply by speed host to derive box in ortho direction where to search for water spray detections. The greater the speed the larger the box.
      float32_t k_ows_range_rate_min_factor; // [-] Factor used for verifying range rate fitness (calculating interval lower limit)
      float32_t k_ows_range_rate_max_factor; // [-] Factor used for verifying range rate fitness (calculating interval upper limit)
      float32_t k_ows_zone_lng_ext_threshold; // [m] Left ceiling for zone area longitudinal extension
      float32_t k_ows_zone_halfwidth_ext_threshold; // [m] Left ceiling for zone area width extension

      // Host vehicle clutter handling
      float32_t max_range_flagging_hvc_dets; // [m] maximum range for flagging host vehicle clutter (HVC) detections as not f_ok_to_use

      // Detection double bounce detector
      float32_t k_db_max_range; // [m] Maximum range of detection to check it for double bounce
      uint32_t k_db_max_nr_multi_bounces; // [-] Maximum number of bounces to consider
      float32_t k_db_range_threshold_frac; // [-] Fraction of secondary detection range to consider as threshold for double bounce. Greater range gives a more generous range gate to satisfy double bounce range condition
      float32_t k_db_min_range_threshold; // [m] Minimum range tolerance limit relative predicted range
      float32_t k_db_max_range_threshold; // [m] Maximum range tolerance limit relative predicted range
      float32_t k_db_range_rate_threshold; // [m/s] Range rate threshold for considering if a detection is double bounce
      float32_t k_db_azimuth_thres_k; // [1/m] K value for deriving azimuth threshold as a linear function of range
      float32_t k_db_azimuth_thres_m; // [m] M value for deriving azimuth threshold as a linear function of range
      float32_t k_db_min_azimuth_thres; // [rad] Min azimuth threshold for double bounce
      float32_t k_db_max_azimuth_thres; // [rad] Max azimuth threshold for double bounce

      // Termination of coasting objects
      float32_t k_max_conf_objtrk_coast_time; // [s] Maximum coasting time for a confirmed track before it is terminated
      float32_t k_max_coast_time_mirror; // [s] Maximum coasting time for a mirror track before it is terminated
      float32_t k_max_coast_time_outside_fov; // [s] Maximum coasting time outside FOV (for front radar only configuration) 

      // Object motion thresholds
      float32_t k_object_motion_sigma_ctca_th; // [-] threshold for stationary CTCA object motion clasicication.
      float32_t k_object_motion_min_speed; // [m/s] threshold indicating minimal object speed to treat it as moving.

      int32_t k_object_motion_min_consec_moving_cnt_movable_th; // [-] Threshold indicating minimal count of consecutive scans to treat as moving if it was movable.
      int32_t k_object_motion_min_consec_moving_cnt_th; // [-] Threshold indicating minimal count of consecutive scans to treat as moving
      int32_t k_object_motion_min_consec_moving_cnt_high_yaw_th; // [-] Threshold indicating minimal count of consecutive scans to treat as moving if host yaw rate is high
      float32_t k_object_motion_min_host_yaw_rate_th; // [rad/s] Threshold for treating host as turning

      int32_t k_object_motion_min_consec_stopped; // [-] Threshold indicating minimal count of consecutive scans to treat as stopped
      float32_t k_object_motion_queue_zone_host_stationary_speed_threshold; // [m/s] Speed threshold below which the host is considered to be stationary
      float32_t k_object_motion_queue_zone_long_dist; // [m] Queue zone longitudinal distance - to prevent close-to-host objects from switching to non-moveable
      float32_t k_object_motion_queue_zone_long_dist_host_stationary; // [m] Queue zone longitudinal distance - when host is stationary
      float32_t k_object_motion_queue_zone_lat_dist; // [m] Queue zone lateral distance - to prevent close-to-host objects from switching to non-moveable
      float32_t k_object_motion_max_abs_orient_diff_to_host; // [rad] Maximum allowed absolute orientation diff from the parallel to host direction (0/180deg)
      float32_t k_object_motion_min_consec_stop_time_th; // [s] threshold indicating minimal time interval between two consecutive stops (f_moving from true to false)

      float32_t k_object_motion_min_moving_dets_percentage_th; // [%/100] Minimal percentage of moving detections to treat object as moving
      float32_t k_object_motion_parallel_moving_heading_th; // [rad] Maximal object vcs heading to analyse it in parallel check
      float32_t k_object_motion_parallel_moving_speed_diff_th; // [m/s] Maximal absolute velocity diff between object and host
      float32_t k_object_motion_parallel_moving_lat_posn_th; // [m] Maximum absolute vcs lateral position of track to treat is as parallel moving
      float32_t k_object_motion_parallel_moving_lon_posn_th; // [m] Maximum absolute vcs longitudinal position of track to treat is as parallel moving

      float32_t k_object_motion_nees_min_p_value_th; // [-] Minimal P value used to reject stationary NEES hypothesis
      float32_t k_object_motion_nees_max_test_variable; // [-] Maximum test variable
      float32_t k_object_motion_nees_range_scaling_factor; // [-] Factor used to lower threshold of p value to accept stationary NEES hypothesis

      float32_t k_object_motion_cross_moving_min_abs_vcs_heading_th; // [rad] Minimal absolute object vcs heading to analyse it as cross moving
      float32_t k_object_motion_cross_moving_max_abs_vcs_azimuth_th; // [rad] Minimal absolute object vcs azimuth to analyse it as cross moving

      float32_t k_object_motion_occluded_speed_scale_factor; // [-] Scaling factor used to increase the occluded cross moving weight
      float32_t k_occlusion_zone_long_left_rear_stationary_check;  // [m] Longitudinal position of rear left corner of occlusion zone used for determining moving speed threshold in motion classification
      float32_t k_occlusion_zone_lat_left_rear_stationary_check;  // [m] Lateral position of rear left corner of occlusion zone used for determining moving speed threshold in motion classification
      float32_t k_occlusion_zone_long_right_front_stationary_check; // [m] Longitudinal position of front right corner of occlusion zone used for determining moving speed threshold in motion classification
      float32_t k_occlusion_zone_lat_right_front_stationary_check; // [m] Lateral position of front right corner of occlusion zone used for determining moving speed threshold in motion classification

      // Object size estimation
      float32_t k_movable_max_target_width; // [m] Max width of movable objects classified as vehicular track
      float32_t k_fast_movable_max_target_length; // [m] Max length of movable objects classified as vehicular track
      float32_t k_slow_movable_max_target_length; // [m] Max length of movable objects not classified as vehicular track
      float32_t k_min_CTCA_target_width;       // [m] Min width of movable objects
      float32_t k_min_CTCA_target_length;      // [m] Min length of movable objects
      float32_t k_nonmoveable_target_diameter;    // [m] diameter of non-moveable objects
      float32_t k_fast_time_factor_length;  // [-] Factor to derive low-pass filter constant when object length increases
      float32_t k_slow_time_factor_length;  // [-] Factor to derive low-pass filter constant when object length decreases
      float32_t k_fast_time_factor_width;   // [-] Factor to derive low-pass filter constant when object width increases
      float32_t k_slow_time_factor_width;   // [-] Factor to derive low-pass filter constant when object width decreases
      float32_t k_min_aspect_ratio; // [-] Min ratio between length and width
      float32_t k_max_aspect_ratio; // [-] Max ratio between length and width
      uint32_t k_min_num_dets_size_update; // [-] Min number of detections needed to do size estimation
      float32_t k_fraction_of_dets_not_used_for_wid_update;      // [-] Fraction of detections which wouldn't be used for wid1/wid2 update.
      float32_t k_size_filter_length_uncertainty;   // [m] Default uncertainty scalar for object size filter in length direction
      float32_t k_size_filter_width_uncertainty;    // [m] Default uncertainty scalar for object size filter in width direction

      // Object size update
      float32_t k_size_update_min_det_range;                           // [m] Minimum range of closest detection to allow size update of non-visible side
      float32_t k_size_update_base_measurement_uncertainty;            // [m^2] Default measurement variance used for both length and width KF update
      float32_t k_size_update_base_process_noise;                      // [m^2] Default process noise for both length and width used in the KF update
      float32_t k_size_update_speed_threshold_low_speed_process_noise; // [m/s] Upper speed threshold to use low speed process noise in KF update for both length and width
      float32_t k_size_update_low_speed_process_noise;                 // [m^2] Process noise for slow moving objects used in both length and width KF update
      float32_t k_size_update_min_speed_to_update_nonvisible_side;     // [m/s] Minimum speed threshold to allow size update of non-visible side
      float32_t k_size_update_process_noise_pruning;                   // Factor to make filter slower by reducing process noise when estimated side is not visible
      uint32_t k_min_num_dets_to_decrease_meas_uncertainty;         // [-] Minimum number of associated detections to allow decrease in measurement uncertainty


      // Liberal tracking
      float32_t liberal_tracking_min_host_speed; // [m/s] Liberal tracking only considered when host speed is above that value
      float32_t liberal_tracking_dets_rrate_thr; // [m/s] Only detections with range rates below that value can be marked as valid for liberal tracking
      float32_t liberal_tracking_trapezoid_zone_top_lon_pos; // [m] Longitudinal position of top edge of trapezoid zone used for liberal tracking
      float32_t liberal_tracking_trapezoid_zone_top_len; // [m] Length of top edge of trapezoid zone used for liberal tracking
      float32_t liberal_tracking_trapezoid_zone_bot_lon_pos; // [m] Longitudinal position of bottom edge of trapezoid zone used for liberal tracking
      float32_t liberal_tracking_trapezoid_zone_bot_len; // [m] Length position of bottom edge of trapezoid zone used for liberal tracking

      float32_t liberal_tracking_obj_relative_velocity_lon_thr; // [m/s] Only objects with relative longitudinal velocity above that value can be marked as valid for liberal tracking
      float32_t liberal_tracking_obj_heading_thr; // [rad] Only objects with heading between (-threshold, threshold) can be marked as valid for liberal tracking
      float32_t liberal_tracking_initial_length; // [m] Initial length for objects initialized by clusters which are valid for liberal tracking.
      float32_t liberal_tracking_initial_len2; // [m] Initial length from reference point to front edge of bbox for objects initialized by clusters which are valid for liberal tracking.
      float32_t liberal_tracking_min_velocity_plausibility; // [m] Minimal value for velocity plausibility when liberal initialization conditions are met.
      float32_t liberal_tracking_velocity_plausibility_scaling_factor; // [m] Scaling factor for velocity plausibility in case of liberal initialization.
      float32_t liberal_tracking_sigma_level_pos_diff; // [-] Sigma level for position diff based velocities used in inliers check in case of liberal initialization.
      float32_t liberal_tracking_sigma_level_cloud; // [-] Sigma level for detection rrates inliers check in case of liberal initialization.

      // Confidence overall
      float32_t k_conf_overall_timeout_to_start_decay; // [s] time a track may coast before the state confidence starts to decay.

      float32_t k_conf_overall_difference_thresh_long_posn_h; // [m] High difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_long_posn_m; // [m] Medium difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_long_posn_l; // [m] Low difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_lat_posn_h; // [m] High difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_lat_posn_m; // [m] Medium difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_lat_posn_l; // [m] Low difference threshold. Used to in confidence state machine to manipulate internal state confidence.

      float32_t k_conf_overall_difference_thresh_long_vel_h; // [m/s] High difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_long_vel_m; // [m/s] Medium difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_long_vel_l; // [m/s] Low difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_lat_vel_h; // [m/s] High difference threshold. Used to in confidence state machine to manipulate internal state confidence.

      float32_t k_conf_overall_difference_thresh_lat_vel_m; // [m/s] Medium difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_lat_vel_l; // [m/s] Low difference threshold. Used to in confidence state machine to manipulate internal state confidence.

      float32_t k_conf_overall_difference_thresh_speed_h; // [m/s] High difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_speed_m; // [m/s] Medium difference threshold. Used to in confidence state machine to manipulate internal state confidence.
      float32_t k_conf_overall_difference_thresh_speed_l; // [m/s] Low difference threshold. Used to in confidence state machine to manipulate internal state confidence.

      // Unreliable low confidence track
      float32_t k_low_conf_unreliability_max_ttc; // [s] Maximum time to cross to consider not downselecting a target based on overall confidence.
      float32_t k_conf_downselection_exclusion_box_lat; // [m] Lateral exclusion zone where tracks will be downselected regardless of overall confidence level.
      float32_t k_conf_downselection_exclusion_box_long; // [m] Longitudinal exclusion zone where tracks will be downselected regardless of overall confidence level.
      float32_t k_low_conf_unreliability_min_heading; // [RAD] Minimum heading for excluding from exclusion zone (so to include object for test for overall confidence).
      float32_t k_low_conf_max_allowed_host_speed_in_cta_scenarios; // [m/s] Track Downselection module: Maximum allowed host speed in CTA scenarios
      float32_t k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios; // [rad] Track Downselection module: Expected absolute object heading (vcs) in CTA scenarios
      float32_t k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios; // [rad] Track Downselection module: Maximum allowed absolute object heading difference in CTA scenarios

      // Longi stat curves
      float32_t k_lsc_min_long_pos; // [m] Minimum longitudinal position of objects to be used by longi stat curves.
      float32_t k_lsc_max_long_pos; // [m] Maximum longitudinal position of objects to be used by longi stat curves.
      float32_t k_lsc_long_pos_gate; // [m] Maximum distance between objects in longitudinal direction to cluster objects to be used by longi stat curves
      float32_t k_lsc_lat_pos_gate; // [m] Maximum absolute distance between objects in lateral direction to cluster objects to be used by longi stat curves
      float32_t k_speed_extend_long_lat_pos_gate; // [m/s] The speed at which clustering gate extends in long and lat directions to cluster objects to be used by longi stat curves
      uint32_t k_lsc_min_points_in_cluster; // [-] Minimum objects in a cluster to be eligible to fit polynomial to. NOTE: This must be >= LSC_NR_POLY_COEFF_SLOTS

      float32_t k_lsc_lat_merging_gate; // [m] Crude gate in lateral direction to try merging two clusters
      float32_t k_lsc_long_merging_gate; // [m] Crude gate in longitudinal direction to try merging two clusters
      float32_t k_lsc_cluster_merge_thr; // [m] Gate on how close two clusters' end points are to execute a merge of the two clusters
      float32_t k_lsc_length_score_gain; // [-] Gain factor on the longitudinal spread of a cluster to boost a long curves prio. Calculated as: score =  k_lsc_length_score_gain * (1 / (x_max-x_min))
      float32_t k_lsc_max_a_coeff; // [-] Threshold on "a" coefficient of polynomial to sanity check curves. Too high a value indicates bad longitudinal distribution of cluster and curve is not valid

      float32_t k_distance_to_circle_thr; // [m]  Minimum distance to the host projected path where the guard rail is allowed to be created 

      // Static Environment Polynomials
      float32_t k_sep_p2_coeff_poly_linear_thr; // [1/m] Threshold on "p2" coefficient of polynomial to treat the polynomial as a linear line
      float32_t k_sep_max_k_coeff_for_lateral_line; // [-] Maximum slope of line to consider as only extending in lateral direction
      float32_t k_sep_det_on_poly_thr; // [m] Distance threshold for a detection to be flagged as "on" static environment polynomial
      float32_t k_sep_obj_on_poly_thr; // [m] Distance threshold for an object's centroid to be flagged as "on" static environment polynomial
      CONF3_T k_low_conf_unreliability_min_overall_conf; // [ENUM] Minimum overall confidence level to allow downselection.
      uint8_t padding_sep[3];

      // Object class determination
      float32_t k_ad_oc_mean_length_pedestrian; // [m] Assumed mean length of pedestrian used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_length_pedestrian; // [m] Assumed mean standard deviation of pedestrian length used to calculate PDF value
      float32_t k_ad_oc_mean_width_pedestrian; // [m] Assumed mean width of pedestrian used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_width_pedestrian; // [m] Assumed mean standard deviation of pedestrian width used to calculate PDF value
      float32_t k_ad_oc_mean_speed_pedestrian; // [m/s] Assumed mean speed of pedestrian used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_speed_pedestrian; // [m/s] Assumed mean standard deviation of pedestrian speed used to calculate PDF value

      float32_t k_ad_oc_mean_length_2wheel; // [m] Assumed mean length of 2wheel (bicycle, motorcycle) used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_length_2wheel; // [m] Assumed mean standard deviation of 2wheel (bicycle, motorcycle) length used to calculate PDF value
      float32_t k_ad_oc_mean_width_2wheel; // [m] Assumed mean width of 2wheel (bicycle, motorcycle) used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_width_2wheel; // [m] Assumed mean standard deviation of 2wheel (bicycle, motorcycle) width used to calculate PDF value
      float32_t k_ad_oc_mean_speed_2wheel; // [m/s] Assumed mean speed of 2wheel (bicycle, motorcycle) used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_speed_2wheel; // [m/s] Assumed mean standard deviation of 2wheel (bicycle, motorcycle) speed used to calculate PDF value

      float32_t k_ad_oc_mean_length_car; // [m] Assumed mean length of car used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_length_car; // [m] Assumed mean standard deviation of car length used to calculate PDF value
      float32_t k_ad_oc_mean_width_car; // [m] Assumed mean width of car used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_width_car; // [m] Assumed mean standard deviation of car width used to calculate PDF value
      float32_t k_ad_oc_mean_speed_car; // [m/s] Assumed mean speed of car used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_speed_car; // [m/s] Assumed mean standard deviation of car speed used to calculate PDF value

      float32_t k_ad_oc_mean_length_truck; // [m] Assumed mean length of truck used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_length_truck; // [m] Assumed mean standard deviation of truck length used to calculate PDF value
      float32_t k_ad_oc_mean_width_truck; // [m] Assumed mean width of truck used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_width_truck; // [m] Assumed mean standard deviation of truck width used to calculate PDF value
      float32_t k_ad_oc_mean_speed_truck; // [m/s] Assumed mean speed of truck used to calculate PDF value
      float32_t k_ad_oc_standard_deviation_speed_truck; // [m/s] Assumed mean standard deviation of truck speed used to calculate PDF value

      float32_t k_ad_oc_boundary_lowspeed; // [m/s] Maximal speed of host to use apriori probabilities defined for slow mooving host
      float32_t k_ad_oc_boundary_highspeed; // [m/s] Minimal speed of host to use apriori probabilities defined for fast moving host

      float32_t k_ad_oc_apriori_lowspeed_pedestrian; // [-] Apriori probability of slow moving pedestrian
      float32_t k_ad_oc_apriori_lowspeed_2wheel; // [-] Apriori probability of slow moving 2wheel
      float32_t k_ad_oc_apriori_lowspeed_car; // [-] Apriori probability of slow moving car
      float32_t k_ad_oc_apriori_lowspeed_truck;// [-] Apriori probability of slow moving truck

      float32_t k_ad_oc_apriori_highspeed_pedestrian; // [-] Apriori probability of fast moving pedestrian
      float32_t k_ad_oc_apriori_highspeed_2wheel; // [-] Apriori probability of fast moving 2wheel
      float32_t k_ad_oc_apriori_highspeed_car; // [-] Apriori probability of fast moving car
      float32_t k_ad_oc_apriori_highspeed_truck; // [-] Apriori probability of fast moving truck

      float32_t k_ad_oc_step_decrease_prob_unknown; // [-] Value used to lower object undetermined probability in several cycles after its creation

      float32_t k_ad_oc_min_thres_vel_2wheel; // [m/s] Threshold used to split 2wheel probability to bicycle and motorbike when object is slow moving
      float32_t k_ad_oc_max_thres_vel_2wheel; // [m/s] Threshold used to split 2wheel probability to bicycle and motorbike when object is fast moving
      float32_t k_ad_oc_weight_frac_2wheel; // [-] Weight of bicycle in 2wheel split

      float32_t k_ad_oc_min_prob_winner_class; // [-] Minimum probability of winner class at which object class enum is set
      float32_t k_ad_oc_lowered_min_prob_winner_class; // [-] Lowered minimum probability of winner class at which object class enum is set

      float32_t k_ad_oc_min_pedestrian_speed; // [m/s] Minimum speed for an object to be considered as pedestrian
      float32_t k_ad_oc_max_pedestrian_speed; // [m/s] Maximum speed for an object to be considered as pedestrian
      float32_t k_ad_oc_max_stationary_speed; // [m/s] Maximum speed for an object to be considered as stationary
      float32_t k_ad_oc_prob_decrease; // [-] Factor for decreasing probability of object being of a given class
      float32_t k_ad_oc_min_prob_pedestrian; // [-] Minimum probability for determining if the slow moving object is a pedestrian

      // Predict existence probability
      float32_t k_p_persist_outside_long_range; // [-] Default persistence probability of a track outside the long look range of the sensor
      float32_t k_p_persist_inside_long_range; // [-] Default persistence probability of a track inside the long look range of the sensor

      // Bistatic
      float32_t k_bistatic_cond_assoc_area_min_lat; // [m] Minimal lateral position of bistatic detection to analyse it whether it is okay to use
      float32_t k_bistatic_cond_assoc_area_max_lat; // [m] Maximal lateral position of bistatic detection to analyse it whether it is okay to use
      float32_t k_bistatic_cond_assoc_area_min_lon; // [m] Minimal longitudinal position of bistatic detection to analyse it whether it is okay to use
      float32_t k_bistatic_cond_assoc_area_max_lon; // [m] Maximal longitudinal position of bistatic detection to analyse it whether it is okay to use
      float32_t k_bistatic_lat_th_extension; // [m] Lateral threshold extension used to accept bistatic detections as okay to use

      // Host mirror track
      float32_t k_host_refl_min_obj_long_pos; // [m] Minimal longitudinal position of object to be analysed whether it is a host mirror track
      float32_t k_host_refl_max_obj_long_pos; // [m] Maximal longitudinal position of object to be analysed whether it is a host mirror track

      float32_t k_host_refl_half_host_length; // [m] Assumed length of host length divided by two

      float32_t k_host_refl_bbox_lat_ext; // [m] Lateral extension of object bbox
      float32_t k_host_refl_bbox_long_ext; // [m] Longitudinal extension of object bbox

      float32_t k_host_refl_filtering_distance; // [m] Longitudinal distance for filtering SEPs crossing host's path

      float32_t k_host_refl_lowspeed_host_speed_th; // [m/s] Maximum host speed to use thresholds defined for slow moving host
      float32_t k_host_refl_lowspeed_speed_diff_th; // [m/s] Maximum diff of host vs object speed when host is slow moving
      float32_t k_host_refl_lowspeed_heading_th; // [rad] Maximum heading of object when host is slowly moving

      float32_t k_host_refl_highspeed_heading_th; // [rad] Maximum heading of object when host is fast moving
      float32_t k_host_refl_highspeed_min_speed_diff_th; // [m/s] Minimal object vs host speed diff when host is fast moving
      float32_t k_host_refl_highspeed_speed_diff_ramp_coef; // [-] Scaling factor used to increase maximum host vs object speed diff
      float32_t k_host_refl_highspeed_max_speed_diff_th; // [m/s] Maximal object vs host speed diff when host is fast moving

      float32_t k_ws_bbox_len_extension_factor; // [-] length extension factor for calculating extended bounding box for water spray detections
      float32_t k_ws_bbox_wid_extension_factor; // [-] width extension factor for calculating extended bounding box for water spray detections

      // Object confidence and raw confidence level
      float32_t k_conf_updated_tracks_filter_const; // [-] Constant used to determine lowpass filter coefficents for objects that were recently updated.
      float32_t k_conf_raw_weight_dets; // [-] Coefficient used to calculate raw confidence level of updated tracks. It determines impact of number of dets on raw confidence level
      float32_t k_conf_raw_max_value_not_reduced_dets; // [-] Max value of raw confidence when object has only associated, but no reduced detections

      float32_t k_conf_coasted_min_time_since_init; // [s] Minimal time since coasted track initialization to use medium confidence coefficients
      float32_t k_conf_coasted_min_time_trusted_track; // [s] Minimum time since coasted track initialization to consider it as tructed track
      float32_t k_conf_coasted_min_long_posn_tructed_track; // [m] Minimum longitudinal distance of object to consider it as trusted track
      float32_t k_conf_coasted_max_long_posn_tructed_track; // [m] Maximum longitudinal distance of object to consider it as trusted track
      float32_t k_conf_coasted_min_average_confidence_level; // [-] Minimum average confidence level of object to use medium confidence coefficients

      // Flagging of azimuth range rate outliers
      float32_t k_az_rdot_max_sq_dist; // [m^2] Maximum squared distance of object from approximated host center to consider flagging detection as range rate outliers
      float32_t k_az_rdot_max_az_diff; // [rad] Maximum spread in azimuth between detections to flag range rate outliers
      float32_t k_az_rdot_min_rdot_diff; // [m/s] Minimum deviation from velocity profile for a detection to be flagged as a range rate outlier

      // Dead zone
      float32_t k_dead_zone_min_host_speed; // [m/s] Minimal host speed to mark objects entering dead zone
      float32_t k_dead_zone_max_obj_vcs_lat_pos; // [m] Maximal object vcs lateral position
      float32_t k_dead_zone_max_obj_vcs_heading; // [m] Maximal object vcs lateral position
      float32_t k_dead_zone_max_rel_vel_diff; // [-] <0, 1> Maximal relative host and object velocity diff to extend association gates
      float32_t k_dead_zone_assoc_gates_additional_enhacementl; // [m] Additional enhancement used to increase longitudinal association gates of object
      float32_t k_dead_zone_long_limit_extension; // [m] Extension for calculating limit of extended dead zone.
      bool k_use_dead_zone_in_stationkeeping_scenarions; // [-] Switch used to turn on marking tracks as in dead zone
      uint8_t padding_dead_zone[3];

      // Occlusion
      float32_t k_occlusion_confidence_level; // [-] Min confidence level for occlusion for track to be relevant
      float32_t k_occlusion_confidence_level_if_front_rear_not_visible; // [-] Min confidence level for occlusion for track to be relevant
      float32_t k_occlusion_min_long; // [m] Minimal longitudinal position of object to be analysed by occlusion detector
      float32_t k_occlusion_max_long; // [m] Maximal longitudinal position of object to be analysed by occlusion detector
      float32_t k_occlusion_min_lat; // [m] Minimal lateral position of object to be analysed by occlusion detector
      float32_t k_occlusion_max_lat; // [m] Maximal lateral position of object to be analysed by occlusion detector
      float32_t k_occlusion_range_uncertainty_th; // [m] Threshold used to lower tested point range due to uncertainties
      bool k_occlusion_enabled; // [-] Flag used to turn on and off usage of occlusion
      uint8_t padding_occlusion[3];

      // Split logic
      float32_t k_orth_split_min_speed; // [m/s] Minimum speed of an object to perform a split. Should be greater or equal to calibration k_min_speed_for_updating_heading
      float32_t k_orth_split_orth_delta_filter_const; // [-] Low pass filter constant for filtered max delta/spread distance between associated detections of an object.
      float32_t k_orth_split_orth_gap_filter_prop_const; // [-] Proportionality constant that is multiplied with number of associated detections to compute low pass filter constant for filtered max gap distance between associated detections of an object.
      uint32_t k_orth_split_orth_gap_filter_max_dets; // [-] Maximum number of detections that is allowed to determine filter constant for max gap between associated detections in orth direction.
      float32_t k_orth_split_min_orth_gap_for_split; // [m] Threshold on objects low pass filtered max detection gap in orthogonal direction value. Signal over this level triggers an orthogonal split.
      float32_t k_orth_split_width_gain; // [-] Gain to determine the updated width after split based on the objects property "orth_delta_filtered"
      float32_t k_orth_split_max_distance_sq; // [m^2] Squared distance limit from host center where we start updating the objects filtered split signals. If object is further away the split signals are reset.
      float32_t k_orth_split_min_distance_sq; // [m^2] Squared distance limit from host center where we start freezing the objects filtered split signals. If objects are closer the the split signals are frozen.
      float32_t k_pos_delta_heading_filter_constant; // [-] Low pass filter constant for filtering an objects heading solely derived from centroid position change between scans

      // CWD - Concrete Wall Detector
      float32_t k_cwd_max_diff_from_aggregated_lat_pos;    // [m] Max difference of lateral position from cw_aggregated_lateral_pos.
      float32_t k_cwd_min_lat_posn_for_cw_sensor;          // [m] Min lateral sensor mounting position to be relevant for CWD.
      float32_t k_cwd_min_longitudinal_validity_limit;     // [m] Min longitudinal validity CWD estimation, limit is a linear function of host speed
      float32_t k_cwd_max_longitudinal_validity_limit;     // [m] Max longitudinal validity CWD estimation, limit is a linear function of host speed
      float32_t k_host_speed_for_min_long_validity_limit;  // [m/s] Host speed for CWD to use minimum longitudinal limit
      float32_t k_host_speed_for_max_long_validity_limit;  // [m/s] Host speed for CWD to use maximum longitudinal limit
      float32_t k_cwd_max_lateral_range;                   // [m] Max range for CWD.
      float32_t k_cwd_sensor_zone_half_length;             // [m] Half width of sensor zone. Zone is defined as: sensor sensor_x_posn +/- k_cwd_sensor_zone_half_length.
      float32_t k_cwd_one_sensor_at_side_conf_factor;      // [-] Factor for modifying confidence when CWD estimation is available only from one sensor at side.
      float32_t k_cwd_side_dist_span_low_limit;            // [m] Low limit for distance span for data fusion at host side.
      float32_t k_cwd_side_dist_span_high_limit;           // [m] High limit for distance span for data fusion at host side.
      float32_t k_cwd_spread_weight;                       // [-] Lateral spread weight used to calculating spread based confidence.
      float32_t k_cwd_max_lateral_spread;                  // [-] Max lateral spread of data in samples buffer to calculate aggregated lateral position from them.
      float32_t k_cwd_min_host_speed_for_cwd;              // [m/s] Minimum host speed for CW reporting.
      float32_t k_cwd_min_conf_to_confirm_cw_by_single_sensor; // [-] Minimum confidence required to confirm CW presence by only one sensor
      float32_t k_cwd_one_sensor_long_validity_offset;      // [m] Min/Max longitudinal validity interval for CW estimate based only on one sensor data.
      uint32_t k_cwd_min_num_samples_for_outlier_finding;// [-] Minimal number of samples in buffer for outlier samples finding.

      // Multi Path detector - low level logic
      float32_t k_mp_object_reflector_size_extension;   // [m] object type reflector extension in size

      // Multi Path detector - high level logic
      float32_t k_mp_max_allowed_host_speed_to_use_MP;      // [m/s] maximum allowed host speed, when checking if object is multipath.
      float32_t k_mp_default_mirror_probability;      // [-] default mirror probability that is assigned to object classified as multipath

      // Multi Path detector - reflector selector
      float32_t k_mp_half_long_zone;     // [m] Half of longitudinal size of outer zone where MP detector works
      float32_t k_mp_half_lat_zone;      // [m] Half of lateral size of outer zone where MP detector works

      // Average object RCS value
      float32_t k_average_rcs_filter_constant; // [-] Low pass filter constant for filtered average object RCS value

      // Overall Confidence Blocker
      uint8_t k_ocb_cnt_delta_midlow_rcs; // [-] Amount to change counter
      uint8_t k_ocb_cnt_delta_low_rcs_or_mult_dets; // [-] Amount to change counter
      uint8_t k_ocb_cnt_max; // [-] Max count that is allowed
      float32_t k_ocb_rcs_thresh_midlow_rcs; // [dBsm] Detection RCS threshold to increase count
      float32_t k_ocb_rcs_thresh_low_rcs; // [dBsm] Secondary detection RCS threshold to increase count
      float32_t k_ocb_rcs_thresh_hi_rcs; // [dBsm] Secondary detection RCS threshold to decrease count
      float32_t k_ocb_max_range; // [m] Max detection range to pass check
      float32_t k_ocb_max_range_rate; // [m] Max detection range to pass check

      // Parameters for Detect_Near_By_Wheel_Spins
      uint16_t k_nbws_max_num_clusters; // [m] max number of clusters to be checked

      // Clustering parameters
      uint8_t k_cluster_ep_max_det_num_for_ep_sat;     // upper saturation on number of detections for existence probability.
      uint8_t k_cluster_ep_min_det_num_for_ep_sat;     // bottom saturation on number of detections for existence probability.

      bool is_separate_clustering;
      bool is_robust_fit;
      bool is_enabled_azimuth_error_detection;
      bool is_fraction_check_for_guardrail;
      bool is_posn_gate_used_for_pseudo_msmt;
      bool is_esr_mr_exclueded_for_orth_posn;
      bool hide_tracks_outside_guardrail;

      // CCA filter tunings
      float32_t k_cca_low_speed_th_to_ramp_down_proceess_noise; // [m/s] For speeds below this the CCA object obtains the slowest process noise tuning in KF time update.
      float32_t k_cca_high_speed_th_to_ramp_down_proceess_noise; // [m/s] For speeds aboce this the CCA object obtains the fastest process noise tuning in KF time update.
      float32_t q_cca_pos_para_high_speed; // [m^2] Time continous process noise variance on position state in object para direction. Applied to fast moving CCA objects in KF time update.
      float32_t q_cca_acc_para_high_speed; // [(m/s^2)^2] Time continous process noise variance on acceleration state in object para direction. Applied to fast moving CCA objects in KF time update.
      float32_t q_cca_pos_orth_high_speed; // [m^2] Time continous process noise variance on position state in object orth direction. Applied to fast moving CCA objects in KF time update.
      float32_t q_cca_acc_orth_high_speed; // [(m/s^2)^2] Time continous process noise variance on acceleration state in object orth direction. Applied to fast moving CCA objects in KF time update.
      float32_t q_cca_pos_low_speed; // [m^2] Time continous process noise variance on position state in object para direction. Applied to slow moving CCA objects in KF time update.
      float32_t q_cca_vel; // [(m/s)^2] Time continous process noise variance on velocity state in object para direction. Applied to fast moving CCA objects in KF time update.
      float32_t q_cca_acc_low_speed; // [(m/s^2)^2] Time continous process noise variance on acceleration state in object para direction. Applied to slow moving CCA objects in KF time update.
      float32_t q_cca_pnt;  // [rad^2] Time continous process noise variance on pointing state. For CCA Pointing yaw rate KF filter.
      float32_t q_cca_heading_rate; // [(rad/s)^2] Time continous process noise variance on heading rate state. For CCA Pointing heading rate KF filter.
      float32_t k_speed_th_to_slow_down_cca_filter; // Speed threshold to determine when to slow down the CCA pointing, yaw rate filter.
      float32_t init_cca_pnt_filter_cov[2][2]; // Initial state covariance matrix for CCA pointing yaw rate filter
      float32_t k_cca_to_ctca_time_since_init_th; // Minimum time before a newly initialized CCA object can transition to CTCA
      uint8_t k_max_num_cca_updates_since_init_to_limit_acc; // Number of updates to limit how measurements are allowed to drive estimated acceleration for young CCA objects
      float32_t k_cca_heading_rate_high_speed_breakpoint; // [m/s] Largest speed for which CCA objects can achieve the maximum yaw rate. For speeds higher than this the maximum yaw rate is smaller.
      float32_t k_cca_heading_rate_low_speed_breakpoint; // [m/s] Smallest speed for which CCA objects can achieve the maximum yheading rate. For speeds lower than this the maximum heading rate is smaller.
      float32_t k_cca_maximum_heading_rate; // [rad/s] Maximum possible yaw rate of a CCA object
      float32_t k_cca_to_ctca_curvature_th; // [1/m] Maximum curvature to allow transision from CCA to CTCA filter type
      float32_t k_max_accepted_vel_error; // [m/s] Used for computing maximum allowed heading pointing disagreement error for CCA-> CTCA transition. Corresponds to maximum allowed speed error when approximating vcs_heading with vcs_pointing during the transition.
      float32_t k_cca_min_speed_to_update_pnt; // [m/s] Threshold for minumum speed of CCA object to update the pointing of the object

      uint8_t padding[3];
   } F360_Calibrations_T;

   void Initialize_Tracker_Calibrations(F360_Calibrations_T& calibrations);
}
#endif
