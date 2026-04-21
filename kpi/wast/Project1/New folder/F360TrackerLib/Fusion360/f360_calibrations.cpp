/*===================================================================================\
 * FILE: f360_calibrations.cpp
 *====================================================================================
 * Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains function definition for Initialize_Tracker_Calibrations.
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

#include <cstring>
#include "f360_calibrations.h"
#include "f360_math.h"
#include <algorithm>
#include "f360_iterator.h"
#include <cassert>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Tracker_Calibrations()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Calibrations_T &calibrations
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
   * This function initializes default values for calibrations
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Initialize_Tracker_Calibrations(F360_Calibrations_T &calibrations)
   {
      calibrations.initial_speed_var_radial_init = 26.0F;
      calibrations.initial_heading_var_radial_init = 0.03F;
      calibrations.is_separate_clustering = true;
      calibrations.is_robust_fit = true;
      calibrations.max_change_in_fraction_of_dets = 0.1F;
      calibrations.is_fraction_check_for_guardrail = false;
      calibrations.low_confidence_level_thresh = 0.3F;
      calibrations.speed_var_non_zero_yaw_rate = 1.0F;
      calibrations.host_vicinity_vcs_x_range = 10.0F;
      calibrations.host_vicinity_vcs_y_range = 9.0F;
      calibrations.fast_moving_thresh = 3.0F;

      calibrations.is_posn_gate_used_for_pseudo_msmt = false;
      calibrations.is_esr_mr_exclueded_for_orth_posn = true;
      calibrations.hide_tracks_outside_guardrail = false;

      // CTCA Time update
      calibrations.k_max_allowed_curv_variance = 0.04F;
      calibrations.k_min_acc_for_increasing_acc_noise = 0.5F;
      calibrations.k_max_acc_for_increasing_acc_noise = 2.0F;
      calibrations.k_max_acc_scale_factor = 20.0F;
      calibrations.k_normal_noise_acc = 2.5F;
      calibrations.k_normal_noise_speed = 0.0F;
      calibrations.k_normal_noise_pos_para = 0.2F;
      calibrations.k_q_tuning_orth_direction_speed_breakpoint = 7.0F;
      calibrations.k_normal_noise_curv = 10.0F;
      calibrations.k_normal_noise_hdg = 0.0F;
      calibrations.k_normal_noise_pos_orth = 0.05F;

      // CTCA measurement update
      calibrations.k_ctca_msmnt_update_max_reverse_abs_spd = 3.0F;

      // CCA measurement update
      calibrations.k_cca_msmnt_update_vel_var = 1.0F;
      calibrations.k_speed_th_for_saturating_r = 0.0001F;
      calibrations.k_max_innov_dist_thres = 0.3F;
      calibrations.k_max_innov_dist_yaw_rate_gain = -0.12F / F360_DEG2RAD(15.0F);
      calibrations.k_min_decrease_factor = 0.0F;

      // CTCA time and measurement update for preventing overshooting of speed and tangential acceleration estimates at hard breaking
      calibrations.k_abs_acc_threshold_for_breaking = 0.5F;
      calibrations.k_abs_speed_threshold_for_stopping = 0.5F;

      // Track validity
      calibrations.k_tv_min_host_speed_moving = 1.0F;
      calibrations.k_tv_min_vel_of_reflected_trk = 1.0F;
      calibrations.k_tv_max_age_of_reflected_trk = 1.0F;
      calibrations.k_tv_refl_trk_offset_from_reflector_center_sq = 6.25F;
      calibrations.k_tv_max_error_in_orientation = 2.5F;
      calibrations.k_tv_max_heading_error = 0.25F;
      calibrations.k_tv_max_dist_of_reflector = 10.0F;
      calibrations.k_tv_reflector_size_extension = 1.5F;
      calibrations.k_tv_error_in_angle = 0.4F;
      calibrations.k_tv_max_frac_error_in_speed = 0.1F;
      calibrations.k_tv_dets_exp_filter_const = 0.95F; //F360_Expf(-T/tau) where T is a fixed time step between samples and tau is desired time constant

      calibrations.k_tv_refl_gr_trk_max_diff_x = 5.5F;
      calibrations.k_tv_refl_gr_trk_max_diff_y = 5.5F;
      calibrations.k_tv_refl_gr_trk_max_diff_rel_vel = 0.2F;
      calibrations.k_tv_refl_gr_trk_max_diff_heading = F360_DEG2RAD(20.0F);
      calibrations.k_tv_refl_gr_trk_min_bbox_lat_margin = 1.0F;
      calibrations.k_tv_refl_gr_trk_bbox_lat_margin = 2.0F;
      calibrations.k_tv_refl_gr_trk_straight_mov_head_th = F360_DEG2RAD(30.0F);
      calibrations.k_tv_refl_gr_trk_min_sep_lon_pos = -80.0F;
      calibrations.k_tv_refl_gr_trk_max_sep_lon_pos = 80.0F;
      calibrations.k_tv_refl_gr_trk_min_host_to_ghost_lon_pos_diff = 1e-6F;

      calibrations.host_vehicle_width = 2.0F;
      calibrations.host_vehicle_length = 4.0F;
      calibrations.k_vp_vehicle_next_to_ego_max_lat_dist = 5.0F;
      calibrations.k_vp_vehicle_next_to_ego_max_long_dist = 5.0F;
      calibrations.k_vp_vehicle_next_to_ego_max_abs_heading = 0.785398163397448F;
      calibrations.k_vp_vehicle_next_to_ego_lat_pos_offset = 0.5F;
      calibrations.k_vp_vehicle_next_to_ego_long_pos_offset = 0.5F;
      calibrations.k_min_speed_for_updating_heading = 2.0F;
      calibrations.rdot_interval_compatability_dealiasing_gate = 3.0F;
      calibrations.max_age_of_older_amb_dets = 0.350F;    // Expected value (0.200F) + const to compensate difference of tracker time and measurement time (~0.150F)
      calibrations.max_age_of_older_no_amb_dets = 0.550F;    // Expected value (0.400F) + const to compensate difference of tracker time and measurement time (~0.150F)
      calibrations.max_age_of_older_dets_lr = 0.750F; // Expected value (0.600F) + const to compensate difference of tracker time and measurement time (~0.150F)

      calibrations.k_conf_overlapping_reduction_factor = 0.9F; // Confidence reduction factor for objects that are overlapping
      calibrations.k_hyst_time_for_coasted_objects = 0.055F; // Hysteresis for when newly COASTED objects are treated similarly to UPDATED objects
      calibrations.k_range_rate_std = 0.06F; // Detection range rate std, taken from radar data sheets.

      calibrations.k_rdot_half_gate = 3.0F;
      calibrations.k_radial_gate = 1.5F;
      calibrations.k_max_crossing_speed = 10.0F;
      calibrations.k_cross_radial_expand_factor = 0.13F;

      calibrations.k_init_trk_preconditions_min_current_dets = 1;
      calibrations.k_init_trk_preconditions_min_mov_dets_sr = 5;
      calibrations.k_init_trk_preconditions_min_mov_amb_dets = 6; // In case of moving and stationary detections combined more detections are needed
      calibrations.k_init_trk_preconditions_min_amb_dets = 4;
      calibrations.k_init_trk_preconditions_min_mov_dets_lr = 4;

      calibrations.k_init_trk_init_confidence = 0.15F;
      calibrations.k_init_trk_max_frac_az_error_stat_mov = 0.5F;

      calibrations.k_init_trk_fast_moving_thresh = 5.55F; // (20 km/h)
      calibrations.k_init_trk_stationary_length = 1.0F;
      calibrations.k_init_trk_stationary_width = 1.0F;
      calibrations.k_init_trk_slow_moving_length = 3.0F;
      calibrations.k_init_trk_slow_moving_width = 1.0F;
      calibrations.k_init_trk_fast_moving_length = 6.0F;
      calibrations.k_init_trk_fast_moving_width = 2.0F;

      calibrations.k_init_cca_range_var = 1.0F;
      calibrations.k_init_cca_az_var = F360_DEG2RAD(2.0F) * F360_DEG2RAD(2.0F);
      calibrations.k_init_vel_errcov_scale_factor = 3.0F;
      calibrations.k_init_cca_min_acc_var = 0.1F;
      calibrations.k_init_cca_max_acc_var = 15.0F;
      calibrations.k_init_cca_k_obj_speed_for_max_acc_var = 10.0F;

      calibrations.f_nees_cfmi_extended_preconditions = false;

      calibrations.k_nees_cfmi_extended_prec_max_dets = 6;
      calibrations.k_nees_cfmi_extended_prec_2_dets_max_std = F360_KPH2MPS(0.1F);
      calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = F360_KPH2MPS(0.1F);
      calibrations.k_nees_cfmi_extended_prec_4_dets_max_std = F360_KPH2MPS(1.0F);
      calibrations.k_nees_cfmi_extended_prec_5_dets_max_std = F360_KPH2MPS(10.0F);
      calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = F360_KPH2MPS(25.0F);

      calibrations.k_nees_cfmi_dt_to_split_dets = F360_MS2S(25.0F);

      calibrations.k_nees_cfmi_extended_object_pos_std = 0.75F;
      calibrations.k_nees_cfmi_process_noise_acc_std = 1.0F;
      calibrations.k_nees_cfmi_process_noise_yaw_rate_std_deg = 10.0F;
      calibrations.k_nees_cfmi_rrate_comp_std = 0.1F;

      calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate = 2U;

      calibrations.k_nees_cfmi_stat_check_max_mov_dets = 2U;
      calibrations.k_nees_cfmi_stat_check_max_mov_dets_ratio = 0.5F;

      calibrations.k_nees_cfmi_init_cl_sl = 3.0F;
      calibrations.k_nees_cfmi_init_stat_pd_sl = 3.0F;
      calibrations.k_nees_cfmi_init_mov_pd_sl = 3.5F;

      calibrations.k_nees_cfmi_min_weight_for_inlier = 0.01F;

      calibrations.k_sigma_level_dominant_velocity = 5.0F;

      calibrations.k_nees_cfmi_vh_check_cloud_sl_pd = 4.0F;
      calibrations.k_nees_cfmi_vh_check_cloud_sl_cl = 4.0F;
      calibrations.k_nees_cfmi_vh_check_cloud_sl_do = 5.0F;
      calibrations.k_nees_cfmi_vh_check_cpd_sl_pd = 4.0F;
      calibrations.k_nees_cfmi_vh_check_cpd_sl_cl = 4.0F;
      calibrations.k_nees_cfmi_vh_check_cpd_sl_do = 5.0F;
      calibrations.k_nees_cfmi_vh_check_dom_sl_pd = 4.0F;
      calibrations.k_nees_cfmi_vh_check_dom_sl_cl = 4.0F;
      calibrations.k_nees_cfmi_vh_check_dom_sl_do = 5.0F;
      constexpr float32_t F360_SQRT2 = 1.41421356237309504880F; // square root of 2
      calibrations.k_nees_cfmi_vh_check_ransac_sl_pd = 3.5F * F360_SQRT2;
      calibrations.k_nees_cfmi_vh_check_ransac_sl_cl = 5.0F;
      calibrations.k_nees_cfmi_vh_check_ransac_sl_do = 5.0F;
      calibrations.k_nees_cfmi_vh_check_radial_sl_pd = 3.0F;
      calibrations.k_nees_cfmi_vh_check_radial_sl_cl = 3.0F;
      calibrations.k_nees_cfmi_vh_check_radial_sl_do = 5.0F;
      calibrations.k_nees_cfmi_vh_check_high_plausibility = 0.99F;
      calibrations.k_nees_cfmi_vh_check_min_pd_plaus = 0.4F;
      calibrations.k_nees_cfmi_vh_check_min_cl_plaus = 0.5F;
      calibrations.k_nees_cfmi_vh_check_min_overall_plaus = 0.3F;

      calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 0.814F; // Expected value of bisquare weight in case of 3-sigma threshold (one variable).
      calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 1.0F;
      calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 0.728F; // Expected value of bisquare weight in case of 3.5-sigma threshold (two variables).
      calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 1.0F;

      calibrations.k_nees_cfmi_ransac_expected_success = 0.999F;
      calibrations.k_nees_cfmi_ransac_max_iterations = 18U;
      calibrations.k_nees_cfmi_ransac_min_iterations = 9U;
      calibrations.k_nees_cfmi_ransac_plausibility_diff_thr = 0.001F;

      calibrations.k_nees_cfmi_min_rr_plaus_thr_low_conf = 0.01F;
      calibrations.k_nees_cfmi_min_rr_plaus_thr_med_conf = 0.3F;
      calibrations.k_nees_cfmi_min_rr_plaus_thr_high_conf = 0.6F;
      calibrations.k_nees_cfmi_rr_std_for_vh_confirm = 0.1F;
      calibrations.k_nees_cfmi_sigma_level_for_vh_confirm = 4.0F;

      calibrations.k_nees_cfmi_cloud_hyp_max_vel_std_kph = 200.0F;

      calibrations.k_nees_cfmi_m_est_max_iter = 10U;

      calibrations.k_nees_cfmi_m_est_msl_min_pd_plaus = 0.5F;
      calibrations.k_nees_cfmi_m_est_msl_min_cl_plaus = 0.5F;
      calibrations.k_nees_cfmi_m_est_msl_pd_sl_for_min_plaus = 3.5F;
      calibrations.k_nees_cfmi_m_est_msl_pd_sl_neutral = 3.0F;
      calibrations.k_nees_cfmi_m_est_msl_pd_sl_for_max_over_inliers = 2.0F;
      calibrations.k_nees_cfmi_m_est_msl_cl_sl_for_min_plaus = 3.0F;
      calibrations.k_nees_cfmi_m_est_msl_cl_sl_for_max_plaus = 2.5F;
      calibrations.k_nees_cfmi_m_est_msl_low_pass_filter_alpha = 0.5F;

      calibrations.k_nees_cfmi_m_est_conv_check_min_speed = F360_KPH2MPS(2.0F);
      calibrations.k_nees_cfmi_m_est_conv_check_max_vel_diff = F360_KPH2MPS(0.5F);
      calibrations.k_nees_cfmi_m_est_conv_check_max_rel_vel_diff = 0.01F;

      calibrations.k_nees_cfmi_m_est_plaus_min_pd_plaus = 0.4F;
      calibrations.k_nees_cfmi_m_est_plaus_min_cl_plaus = 0.7F;
      calibrations.k_nees_cfmi_m_est_plaus_min_overall_plaus = 0.55F;

      calibrations.k_nees_cfmi_vel_cov_host_inf_factor = 1.0F;
      calibrations.k_nees_cfmi_vel_cov_biger_eig_bias = F360_KPH2MPS(5.0F);
      calibrations.k_nees_cfmi_vel_cov_lower_eig_bias = F360_KPH2MPS(0.1F);
      calibrations.k_nees_cfmi_vel_cov_bias_factor = 0.2F;

      calibrations.k_nees_cfmi_stat_nees_test_alpha_for_stat = 1e-8F;
      calibrations.k_nees_cfmi_stat_nees_test_alpha_for_mov = 1e-3F;
      calibrations.k_nees_cfmi_stat_nees_test_bias = F360_KPH2MPS(1.0F);
      calibrations.k_nees_cfmi_stat_nees_test_bias_factor = 1.0F;

      calibrations.k_nees_cfmi_stat_nees_max_test_var = 64.0F;

      calibrations.k_nees_cfmi_stat_dom_hyp_sigma_level = 4.0F;
      calibrations.k_nees_cfmi_stat_dom_hyp_max_speed = F360_KPH2MPS(5.0F);
      calibrations.k_nees_cfmi_stat_dom_hyp_min_dt = F360_MS2S(15.0F);

      calibrations.k_nees_cfmi_stat_inliers_check_sl_cl = 6.0F;
      calibrations.k_nees_cfmi_stat_inliers_check_sl_pd = 3.0F;
      calibrations.k_nees_cfmi_stat_inliers_check_min_det_plaus = 0.5F;

      calibrations.k_nees_cfmi_vel_plaus_thr = 0.999F;
      calibrations.k_nees_cfmi_vel_plaus_cov_trace_low_plaus = F360_KPH2MPS(10.0F) *  F360_KPH2MPS(10.0F);
      calibrations.k_nees_cfmi_vel_plaus_cov_trace_high_plaus = F360_KPH2MPS(50.0F) *  F360_KPH2MPS(50.0F);
      calibrations.k_nees_cfmi_vel_plaus_cov_low_plaus = 0.57F;
      calibrations.k_nees_cfmi_vel_plaus_cov_high_plaus = 0.9F;

      calibrations.k_priority_default_cluster_confidence = 0.5F;
      calibrations.k_priority_default_cluster_movable = true;
      calibrations.k_priority_distance_coefficient = 0.25F;
      calibrations.k_priority_confidence_coefficient = 0.1F;
      calibrations.k_priority_headway_coefficient = 0.15F;
      calibrations.k_priority_f_movable_coefficient = 0.5F;
      calibrations.k_priority_distance_for_min_priority_inverse = 1.0F / 200.0F;
      calibrations.k_priority_headway_for_min_priority_inverse = 1.0F / 10.0F;
      calibrations.k_priority_lat_penalty_max_dist_inverse = 1.0F / 20.0F;

      calibrations.k_nees_cfmi_vel_sanity_max_speed = F360_KPH2MPS(300.0F);
      calibrations.k_nees_cfmi_vel_sanity_max_vel_cov_trace = F360_KPH2MPS(100.0F) * F360_KPH2MPS(100.0F);

      calibrations.f_nees_cfmi_init_with_not_plausible_vel = false;

      calibrations.k_obj_init_min_number_of_correct_dets = 4;
      calibrations.k_obj_init_min_correct_dets_ratio = 0.35F;

      calibrations.k_cluster_ep_max_det_num_for_ep_sat = 5U;
      calibrations.k_cluster_ep_min_det_num_for_ep_sat = 0U;

      calibrations.k_moving_clusters_dist_coarse_gate = 8.0F;
      calibrations.k_stat_clusters_dist_sq_coarse_gate = 4.0F;
      calibrations.k_moving_clusters_dist_sq_coarse_gate_1 = 90.0F;
      calibrations.k_moving_clusters_dist_sq_coarse_gate_2 = 156.0F;
      calibrations.k_moving_clusters_time_diff_coarse_gate = 0.1F;
      calibrations.k_max_dealiased_range_rate_diff = 3.0F;

      //Determine reflected object
      calibrations.k_mirror_prob_threshold = 0.30F;
      calibrations.k_reflective_guardrail_track_min_host_speed = 2.0F;
      calibrations.k_reflected_object_max_mirror_probability = 1.0F;

      // Measurement update covariance parameters
      calibrations.k_ref_msmt_cov_cca = 0.9F;
      calibrations.k_ref_msmt_cov_ctca = 0.5F;
      calibrations.k_min_num_selected_dets_per_sensor_for_binning = 5U;

      // Reference point paramters - F360Version
      calibrations.k_normal_obj_ref_pnt_hysteresis_factor = 1.05F;
      calibrations.k_normal_obj_length_thr = 6.5F;
      calibrations.k_long_obj_ref_pnt_hysteresis_factor = 1.005F;
      calibrations.k_long_obj_length_thr = 11.5F;
      calibrations.k_ref_point_state_update_speed_threshold = 0.01F;
      calibrations.k_sin_max_pointing_error_sq = 0.001217974870088F; // Corresponds to sin(2deg)^2
      calibrations.k_frac_of_moved_dist = 0.1F;

      // Determine to freeze track
      calibrations.rdot_comp_mov_obj_tresh = 1.0F;
      calibrations.pred_abs_vcs_xvel_tresh = 0.5F;
      calibrations.max_abs_vcs_yvel_tresh = 1.0F;
      calibrations.max_abs_cos_azim_vcs_for_det_outside_cone_of_silence = std::abs(F360_Cosf(F360_DEG2RAD(60.0F)));
      calibrations.min_object_age_for_object_close_to_stat_host = 0.5F;

      // Guardrail based angel jump detector
      calibrations.k_angle_jump_range_tolerance = 1.0F;
      calibrations.k_angle_jump_max_abs_range_rate = 2.0F;
      calibrations.k_angle_jump_min_abs_azimuth_vcs = F360_DEG2RAD(40.0F);
      calibrations.k_angle_jump_max_abs_azimuth_vcs = F360_DEG2RAD(140.0F);
      calibrations.k_angle_jump_max_range = 15.0F;
      calibrations.k_angle_jump_long_search_margin = 5.0F;

      //Mark stationary bounced detections
      calibrations.k_stat_bounce_min_host_speed = 10.0F;
      calibrations.k_stat_bounce_max_trk_long_posn = -15.0F;
      calibrations.k_stat_bounce_max_trk_lat_dist = 6.0F;
      calibrations.k_stat_bounce_max_trk_heading = F360_DEG2RAD(20.0F);
      calibrations.k_stat_bounce_azimuth_border_ext = F360_DEG2RAD(2.0F);
      calibrations.k_stat_bounce_range_rate_diff_thr = 1.0F;
      calibrations.k_stat_bounce_min_det_long_posn = -110.0F;

      // Filter out low quality detections on or behind guardrail
      calibrations.k_azimuth_conf_low_quality_detection_filter = 3;
      calibrations.k_max_rcs_thr_low_quality_detection_filter = -10.0F;

      calibrations.k_mrr360_max_abs_elev_angle_rad = F360_DEG2RAD(6.0F);
      calibrations.k_mrr360_min_host_speed_el_check = 2.0F;

      // Parameters for Mark_Detections_With_Neighbours
      calibrations.max_dist_for_neighbor_detections = 3.0F;

      // Parameters for Detect_Wheel_Spin_Pairs
      calibrations.k_max_wheel_spin_dist_sq = (0.2F);
      calibrations.k_min_wheel_spin_doppler_spread = (3.0F);
      calibrations.k_max_abs_vcs_long_posn_for_wheelspin_pair = (30.0F);
      calibrations.k_max_abs_vcs_lat_posn_for_wheelspin_pair = (20.0F);
      calibrations.k_max_azimuth_difference_for_wheelspin_pair = (0.06F);
      calibrations.k_wheelspin_pair_max_close_det_iterations = 3U;
      calibrations.k_max_wheel_spin_dets_to_mark = 10U;

      // Parameters for Mark_Detections_Wheel_Spin_From_Objects
      calibrations.k_max_abs_vcs_long_posn_for_wheelspin = 30.0F;
      calibrations.k_min_speed_fast_moving = 3.0F;
      calibrations.k_min_rr_diff_wheelspin = 2.0F;
      calibrations.k_ws_lat_buffer_zone_oncoming = 0.40F;
      calibrations.k_ws_lat_buffer_zone = 0.3F;
      calibrations.k_ws_long_buffer_zone = 0.3F;

      // Parameters for Detect_NearBy_Wheel_Spins
      calibrations.k_nbws_max_lat_pos = 5.0F;
      calibrations.k_nbws_min_lat_pos = -5.0F;
      calibrations.k_nbws_max_long_pos = 14.0F;
      calibrations.k_nbws_min_long_pos = -14.0F;
      calibrations.k_nbws_lat_marking_th = 0.2F;
      calibrations.k_nbws_long_marking_th = 0.5F;
      calibrations.k_nbws_lat_asc_th = 0.3F;
      calibrations.k_nbws_long_asc_th = 0.3F;
      calibrations.k_nbws_max_num_clusters = 50U;

      // Parameters for Mark_Dets_As_Close_Target_And_Farside
      calibrations.k_ct_and_fcm_max_abs_heading_diff_to_host = F360_DEG2RAD(30.0F);
      calibrations.k_ct_and_fcm_max_dist_for_close_target_sq = 36.0F;
      calibrations.k_ct_orth_buffer_zone_factor = 2.0F;
      calibrations.k_ct_para_buffer_zone_factor = 0.5F;

      // Parameters for Cond_Deassoc_Low_RR_Dets
      calibrations.k_cond_deassoc_min_obj_spd_for_deassoc = 2.0F;
      calibrations.k_cond_deassoc_det_comp_rr_max = 0.2F;
      calibrations.k_cond_deassoc_fraction_of_width_to_deassoc = 0.8F;

      // Existence Probability
      calibrations.k_ep_init_p_det_sensor = 1.0F;
      calibrations.k_ep_p_measurement_with_no_new_meas = 0.01F;
      calibrations.k_ep_clutter_prob_with_meas = 0.05F;
      calibrations.k_ep_clutter_prob_with_no_meas = 0.05F;
      calibrations.k_ep_min_allowed_exist_prob = 0.01F;
      calibrations.k_ep_sensor_degradation_factor = 0.7F;
      calibrations.k_ep_prob_track_state_exp_scale = 0.4F;
      calibrations.k_ep_prob_track_state_exp_offset = 0.1F;
      calibrations.k_ep_bottom_saturation_of_normalized_variance = 0.2F * 0.2F;
      calibrations.k_ep_variance_th_pos_xy = 2.0F * 2.0F;
      calibrations.k_ep_variance_th_heading = 0.2F * 0.2F;
      calibrations.k_ep_variance_th_velocity = 1.5F * 1.5F;
      calibrations.k_ep_variance_th_curvature = 0.04F * 0.04F;
      calibrations.k_ep_variance_th_accel = 1.0F * 1.0F;
      calibrations.k_ep_variance_th_tan_accel = 1.5F * 1.5F;
      calibrations.k_ep_prob_track_state_init_value = 0.80F;
      calibrations.k_ep_init_factor_min_val = 0.3F;
      calibrations.k_ep_init_factor_max_val = 1.0F;
      calibrations.k_ep_init_factor_exponent = 2U;

      //Post Update track Adjustments
      calibrations.k_puta_max_vcs_xposn_for_ghost_NU_2_C = 50.0F;
      calibrations.k_puta_max_vcs_yposn_for_ghost_NU_2_C = 30.0F;
      calibrations.k_puta_overlapping_tracks_max_speed_diff = 3.0F;
      calibrations.k_puta_overlapping_tracks_max_heading_diff = F360_DEG2RAD(30.0F);
      calibrations.k_puta_overlapping_tracks_high_conf_thr = 0.8F;
      calibrations.k_puta_overlapping_tracks_low_conf_thr = 0.5F;
      calibrations.k_puta_overlapping_tracks_long_thr = 25.0F;

      calibrations.k_puta_obj_size_acc_filt_coef_innov_coasting_obj = 0.9F;
      calibrations.k_puta_obj_size_acc_filt_coef_innov_updated_obj = 0.9F;
      calibrations.k_puta_obj_size_acc_innov_no_update_length = 6.0F;
      calibrations.k_puta_obj_size_acc_innov_no_update_width = 2.0F;

      calibrations.k_puta_min_object_confidence = 0.5F;
      calibrations.k_puta_min_object_time = 0.4F;
      calibrations.k_puta_large_distance = 1e10F;
      calibrations.k_puta_orientation_diff_threshold = F360_DEG2RAD(90.0F);

      // Track Downselection
      calibrations.k_track_downselect_max_priority = 1e+6F;
      calibrations.k_track_downselect_max_vcs_x_range_to_preserve = 15.0F;
      calibrations.k_track_downselect_max_vcs_y_range_to_preserve = 9.0F;
      calibrations.k_track_downselect_min_negative_ttc = -100.0F;
      calibrations.k_track_downselect_dets_threshold = 1.25F;
      calibrations.k_track_downselect_dets_threshold_low = 0.5F;
      calibrations.k_track_downselect_average_dets_thresh = 1.0F;
      calibrations.k_track_downselect_min_time_filter_dets_thresh = 2.5F;
      calibrations.k_track_downselect_confidence_level_lowering_factor = 0.9F;
      calibrations.k_track_downselect_upper_ttc_saturation_level = 10.0F;
      calibrations.k_track_downselect_lower_ttc_saturation_level = 7.0F;
      calibrations.k_track_downselect_low_priority_level_for_nonmoveable_tracks = 250.0F;
      calibrations.k_track_downselect_confidence_thresh = 0.4F;
      calibrations.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object = 0.5236F;
      calibrations.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object = 5.55F;
      calibrations.k_track_downselect_max_confidence_for_possibly_ghost = 0.6F;

      // Track grouping
      calibrations.k_track_grouping_half_long_extension = 0.75F;
      calibrations.k_track_grouping_half_lat_extension = 0.0F;
      calibrations.k_track_grouping_hdg_gate = F360_DEG2RAD(30.0F);
      calibrations.k_track_grouping_speed_gate = 1.0F;
      calibrations.k_track_grouping_curvature_gate = 0.02F;
      calibrations.merging_m2m_distance_threshold = 2.2F;
      calibrations.merging_lateral_det_spread_threshold = 4.0F;
      calibrations.merging_m2m_max_obj_speed = F360_KPH2MPS(25.0F);
      calibrations.k_orth_split_width_threshold = 1.8F;

      //Calculate Time To Collision (ttc)
      calibrations.k_calc_ttc_min_thresh_projected_velocity = 1.0F;
      calibrations.k_calc_ttc_max_thresh_projected_velocity = 1e+6F;
      calibrations.k_calc_ttc_min_obj_track_range = 0.1F;

      // radar phenomena
      calibrations.rp_max_object_lateral_distance = 8.0F;
      calibrations.rp_max_abs_pointing_disagreement = F360_DEG2RAD(10.0F);
      calibrations.rp_object_max_longitudinal_margin = 2.0F;
      calibrations.rp_min_confidence_level = 0.5F;

      // object based angle jump detector
      calibrations.obj_aj_det_range_gap = 15.25F;
      calibrations.obj_aj_max_allowed_rr_diff = 0.3F;
      calibrations.obj_aj_azimuth_jump_value = F360_DEG2RAD(60.0F);
      calibrations.obj_aj_border_half_width = 0.25F;
      calibrations.obj_aj_obj_length_reduction_factor = 0.1F;
      calibrations.obj_aj_max_obj_length_reduction = 1.0F;
      calibrations.obj_aj_suspected_rr_handicap = 0.1F;
      calibrations.obj_aj_max_double_range_hypothesis = 10.0F;

      // object based multibounce detector
      calibrations.mb_max_num_bounces = 4U;
      calibrations.mb_range_rate_diff_th = 1.0F;
      calibrations.mb_max_det_range = 15.0F;
      calibrations.mb_restricted_area_width = 0.5F;

      // Association: Parameters for detection to track association
      calibrations.k_nonmoveable_target_diameter = 0.5F;
      calibrations.k_min_assoc_gate_extension_non_moveable = 0.1F;
      calibrations.k_max_assoc_gate_extension_non_moveable = 1.75F;
      calibrations.k_max_assoc_gate_radius_non_moveable = 2.0F;
      calibrations.k_obj_dist_for_min_assoc_gate_extension_non_moveable = 0.0F;
      calibrations.k_obj_dist_for_max_assoc_gate_extension_non_moveable = 50.0F;
      calibrations.k_spd_dependent_assoc_gate_extension_factor_non_moveable = 0.5F;
      calibrations.k_min_speed_for_increasing_occluded_long_assoc_buffer = 5.55F; // about 20kph
      calibrations.k_max_speed_for_saturating_occluded_long_assoc_buffer_increase = 8.3F; // about 30kph
      calibrations.k_max_occluded_long_buffer_increase = 1.0F;
      calibrations.k_min_assoc_gate_long_buffer_moveable_objs = 0.4F;
      calibrations.k_min_assoc_gate_lat_buffer_moveable_objs = 0.4F;
      calibrations.k_range_rate_score_threshold = 2.0F;
      calibrations.k_range_buffer_max_dist = 100.0F;
      calibrations.k_range_buffer_min_val = 0.0F;
      calibrations.k_range_buffer_max_val = 1.0F;
      calibrations.k_az_buffer_max_dist = 90.0F;
      calibrations.k_az_buffer_min_val = 0.0F;
      calibrations.k_az_buffer_max_val = 5.0F;
      calibrations.k_SEP_association_zone_extension = 6.0F;
      calibrations.k_slow_moving_ctca_min_uncertainty_assoc_buffer = 0.9F;
      calibrations.k_max_uncertainty_assoc_buffer = 2.0F;
      calibrations.k_slope_coeff_uncertainty_assoc_buffer = 0.4F;
      calibrations.k_offset_coeff_uncertainty_assoc_buffer = 0.5F;
      calibrations.k_min_rr_diff_from_stationary_hypothesis = 0.2F;

      // Association: Parameters for calculating association score, detection inside bounding box
      calibrations.k_base_score_inside_bbox = 0.0F;
      calibrations.k_para_diff_weight_inside_box = 0.15F;
      calibrations.k_orth_diff_weight_inside_box = 0.15F;
      calibrations.k_rdot_diff_weight_inside_box = 0.7F;
      calibrations.k_dist_weight_inside_solid_circle = 0.3F;
      calibrations.k_rdot_diff_weight_inside_solid_circel = 0.7F;
      calibrations.k_base_score_bbox_center = 0.75F;

      // Association: Parameters for calculating association score, detection inside extended bounding box
      calibrations.k_base_score_inside_ext_bbox = 1.0F;
      calibrations.k_para_diff_weight_inside_ext_box = 0.35F;
      calibrations.k_orth_diff_weight_inside_ext_box = 0.35F;
      calibrations.k_rdot_diff_weight_inside_ext_box = 0.3F;
      calibrations.k_dist_weight_inside_ext_circle = 0.7F;
      calibrations.k_rdot_diff_weight_inside_ext_circel = 0.3F;

      // Association: Score for detections that match in range rate but is outside extended bounding box
      calibrations.k_score_outside_ext_bbox = 5.0F;

      // Association: Parameters for calculating range rate threshold
      calibrations.k_vcs_distance_sqr_thr = 1600.0F; // 40 * 40 meters
      calibrations.k_rr_thr_factor_far_away_coasted = 2.0F;
      calibrations.k_rr_thr_factor_fov_edge = 0.2F;
      calibrations.k_speed_threshold = F360_KPH2MPS(25.0F);

      // Association: detection inlier selection
      calibrations.k_rr_error_statistics_forgetting_factor = 0.83F;
      calibrations.k_max_number_of_historic_dets_obj_non_movable = 10.0F;
      calibrations.k_max_number_of_historic_dets_obj_movable = 4.0F;
      calibrations.k_max_historic_rr_error_variance = 2.0F;
      calibrations.k_min_range_rate_error_threshold = 0.2F;

      // Object tracks properties
      calibrations.k_underdrive_min_trk_long_posn = -5.0F;

      calibrations.k_underdrive_min_zone_long_posn = 0.0F;
      calibrations.k_underdrive_lat_buffer_factor = 1.4F;

      // Underdrivability for moving objects
      calibrations.ud_mov_forgetting_factor = 0.97F;
      calibrations.ud_mov_height_threshold = 6.5F; 
      calibrations.ud_mov_cnt_consecutive_scans = 10U;
      calibrations.ud_mov_max_n_historic_dets = 80.0F;
      calibrations.ud_mov_posx_min_limit = 0.0F;
      calibrations.ud_mov_posx_max_limit = 100.0F;
      calibrations.ud_mov_prob_can_pass_under = 1.0F;
      calibrations.ud_mov_prob_can_not_pass_under = 0.0F;
      calibrations.ud_mov_prob_not_to_consider = 0.0F;

      // Occupancy Grid related caliberations
      calibrations.k_ocg_underdrive_small_curvature_th = 1e-4F;
      calibrations.f_ocg_use_curvilinear_simplification = true;

      // Pseudo position estimation
      calibrations.k_pseudo_pos_gain_weighted_mean = 30.0F;
      calibrations.k_pseudo_pos_var_bias_para = 2.0F;
      calibrations.k_pseudo_pos_var_bias_orth = 1.0F;
      calibrations.k_pseudo_pos_gain_simple_estimate = 5.0F;
      calibrations.k_pseudo_pos_dist_diff_gain = 5.0F;
      calibrations.k_pseudo_pos_dist_diff_thr = 0.9F;
      calibrations.k_pseudo_pos_max_variance_threshold = 30.0F;

      // Pseudo centroid position
      calibrations.k_pcp_offset_length_factor = 0.4F;
      calibrations.k_pcp_offset_width_factor = 0.43F;

      // Uncertainty bias for pseudo position measurement
      calibrations.k_pseudo_pos_cov_matrix_bias = 1.0F;
      calibrations.k_pseudo_pos_cov_matrix_bias_non_movable = 1.0F * 1.0F;

      calibrations.k_time_since_init_th_to_enable_outlier_mitigation_cca = 0.75F;

      // Paramerters for pseudo position in FOVE cases
      calibrations.k_pseudo_pos_high_uncertainity = 10000.0F;
      calibrations.k_fov_normal_rotation_angle = F360_DEG2RAD(21.5F);

      // Polar uncertainty
      calibrations.k_range_var = 0.5F * 0.5F;
      calibrations.k_az_var = F360_DEG2RAD(2.0F) * F360_DEG2RAD(2.0F);
      calibrations.k_raw_pseudo_pos_cov_max_saturation_distance = 100.0F * 100.0F;

      // Determine to use pseudo heading estimate parameters

      // Calculate pseudo heading estimate

      // M-estimator selection criteria

      // Water spray detectors
      calibrations.k_ws_min_speed = 5.0F;
      calibrations.k_ws_max_det_rcs = -24.5F;

      // Probabilities of detections
      calibrations.k_probability_of_detection_default = 0.7F;
      calibrations.k_probability_of_detection_init = 0.1F;
      calibrations.k_probability_of_detection_low = 0.5F;
      calibrations.k_probability_of_detection_medium = 0.7F;
      calibrations.k_probability_of_detection_high = 0.9F;
      calibrations.k_detection_probability_degradation_factor = 0.5F;

      // Sensor preprocessing: calculate_position_covariance_matrix_single_detection
      calibrations.k_default_detection_azimuth_std = F360_DEG2RAD(0.3F);
      calibrations.k_default_detection_range_std = 0.15F;

      // Host water spray detector
      calibrations.k_hws_para_box_host_speed_factor = 0.4F;
      calibrations.k_hws_ortho_box_host_speed_factor = 0.15F;

      // Object based water spray detector
      calibrations.k_ows_min_long_pos = -25.0F;
      calibrations.k_ows_max_long_pos = 50.0F;
      calibrations.k_ows_max_lat_pos = 25.0F;
      calibrations.k_ows_para_box_obj_speed_factor = 0.4F;
      calibrations.k_ows_ortho_box_obj_speed_factor = 0.1F;
      calibrations.k_ows_range_rate_min_factor = 0.1F;
      calibrations.k_ows_range_rate_max_factor = 0.7F;
      calibrations.k_ows_zone_lng_ext_threshold = 8.0F;
      calibrations.k_ows_zone_halfwidth_ext_threshold = 1.3F;

      // Host vehicle clutter handling
      calibrations.max_range_flagging_hvc_dets = 0.30F;

      // Detection double bounce detector
      calibrations.k_db_max_range = 40.0F;
      calibrations.k_db_max_nr_multi_bounces = 3U;
      calibrations.k_db_range_threshold_frac = 0.15F;
      calibrations.k_db_min_range_threshold = 0.2F;
      calibrations.k_db_max_range_threshold = 3.0F;
      calibrations.k_db_range_rate_threshold = 2.0F;
      calibrations.k_db_azimuth_thres_k = -0.0052F;
      calibrations.k_db_azimuth_thres_m = 0.1222F;
      calibrations.k_db_min_azimuth_thres = F360_DEG2RAD(2.0F);
      calibrations.k_db_max_azimuth_thres = F360_DEG2RAD(5.0F);

      // Longi stat curves
      calibrations.k_lsc_min_long_pos = -250.0F;
      calibrations.k_lsc_max_long_pos = 250.0F;
      calibrations.k_lsc_long_pos_gate = 5.0F;
      calibrations.k_lsc_lat_pos_gate = 1.2F;
      calibrations.k_speed_extend_long_lat_pos_gate = F360_KPH2MPS(80.0F);
      calibrations.k_lsc_min_points_in_cluster = 3U;
      calibrations.k_lsc_long_merging_gate = 3.0F;
      calibrations.k_lsc_lat_merging_gate = 3.0F;
      calibrations.k_lsc_cluster_merge_thr = 1.2F;
      calibrations.k_lsc_length_score_gain = 100.0F;
      calibrations.k_lsc_max_a_coeff = 1.0F;
      calibrations.k_distance_to_circle_thr = 2.0F;

      // Static Environment Polynomials
      calibrations.k_sep_p2_coeff_poly_linear_thr = 0.0005F;
      calibrations.k_sep_max_k_coeff_for_lateral_line = 1000.0F;
      calibrations.k_sep_det_on_poly_thr = 0.75F;
      calibrations.k_sep_obj_on_poly_thr = 0.75F;

      // Termination of coasting objects
      calibrations.k_max_conf_objtrk_coast_time = 0.55F;
      calibrations.k_max_coast_time_mirror = 0.25F;
      calibrations.k_max_coast_time_outside_fov = 0.055F;

      // Object size estimation
      calibrations.k_movable_max_target_width = 2.5F;
      calibrations.k_fast_movable_max_target_length = 25.0F;
      calibrations.k_slow_movable_max_target_length = 4.0F;
      calibrations.k_min_CTCA_target_width = 1.0F;
      calibrations.k_min_CTCA_target_length = 2.5F;
      calibrations.k_fast_time_factor_length = 0.04F;
      calibrations.k_slow_time_factor_length = 3.0F;
      calibrations.k_fast_time_factor_width = 0.25F;
      calibrations.k_slow_time_factor_width = 3.0F;
      calibrations.k_min_aspect_ratio = 0.15F;
      calibrations.k_max_aspect_ratio = 0.667F;
      calibrations.k_min_num_dets_size_update = 1U;
      calibrations.k_fraction_of_dets_not_used_for_wid_update = 0.2F;
      calibrations.k_size_filter_length_uncertainty = 0.2F;
      calibrations.k_size_filter_width_uncertainty = 0.2F;

      // Object size update
      calibrations.k_size_update_min_det_range = 5.0F;
      calibrations.k_size_update_base_measurement_uncertainty = 5.0F;
      calibrations.k_size_update_base_process_noise = 1e-3F;
      calibrations.k_size_update_speed_threshold_low_speed_process_noise = 2.0F;
      calibrations.k_size_update_low_speed_process_noise = 1e-6F;
      calibrations.k_size_update_min_speed_to_update_nonvisible_side = 5.0F;
      calibrations.k_size_update_process_noise_pruning = 0.1F;
      calibrations.k_min_num_dets_to_decrease_meas_uncertainty = 5U;

      // Object motion classification
      calibrations.k_object_motion_sigma_ctca_th = 3.0F;
      calibrations.k_object_motion_min_speed = 0.5F;
      calibrations.k_object_motion_min_consec_moving_cnt_movable_th = 2;
      calibrations.k_object_motion_min_consec_moving_cnt_th = 3;
      calibrations.k_object_motion_min_consec_moving_cnt_high_yaw_th = 5;

      calibrations.k_object_motion_min_consec_stopped = 140;
      calibrations.k_object_motion_queue_zone_host_stationary_speed_threshold = 0.5F;
      calibrations.k_object_motion_queue_zone_long_dist = 30.0F;
      calibrations.k_object_motion_queue_zone_long_dist_host_stationary = 15.0F;
      calibrations.k_object_motion_queue_zone_lat_dist = 4.0F; 
      calibrations.k_object_motion_max_abs_orient_diff_to_host = F360_DEG2RAD(65.0F);
      calibrations.k_object_motion_min_consec_stop_time_th = 3.0F;

      calibrations.k_object_motion_min_host_yaw_rate_th = F360_DEG2RAD(2.0F);

      calibrations.k_object_motion_min_moving_dets_percentage_th = 0.25F;
      calibrations.k_object_motion_parallel_moving_heading_th = F360_DEG2RAD(10.0F);
      calibrations.k_object_motion_parallel_moving_speed_diff_th = 0.25F;
      calibrations.k_object_motion_parallel_moving_lat_posn_th = 5.0F;
      calibrations.k_object_motion_parallel_moving_lon_posn_th = 5.0F;

      calibrations.k_object_motion_nees_min_p_value_th = 0.001F;
      calibrations.k_object_motion_nees_max_test_variable = 64.0F;
      calibrations.k_object_motion_nees_range_scaling_factor = 0.1F;

      calibrations.k_object_motion_cross_moving_min_abs_vcs_heading_th = F360_DEG2RAD(10.0F);
      calibrations.k_object_motion_cross_moving_max_abs_vcs_azimuth_th = F360_DEG2RAD(45.0F);

      calibrations.k_object_motion_occluded_speed_scale_factor = 2.5F;
      calibrations.k_occlusion_zone_long_left_rear_stationary_check = 0.0F;
      calibrations.k_occlusion_zone_lat_left_rear_stationary_check = -10.0F;
      calibrations.k_occlusion_zone_long_right_front_stationary_check = 10.0F;
      calibrations.k_occlusion_zone_lat_right_front_stationary_check = 10.0F;

      //Liberal tracking
      calibrations.liberal_tracking_min_host_speed = F360_KPH2MPS(50.0F);
      calibrations.liberal_tracking_dets_rrate_thr = -5.0F;
      calibrations.liberal_tracking_trapezoid_zone_top_lon_pos = -55.0F;
      calibrations.liberal_tracking_trapezoid_zone_top_len = 16.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_lon_pos = -120.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_len = 32.0F;

      calibrations.liberal_tracking_obj_relative_velocity_lon_thr = 5.0F;
      calibrations.liberal_tracking_obj_heading_thr = F360_DEG2RAD(30.0F);

      calibrations.liberal_tracking_obj_relative_velocity_lon_thr = 5.0F;
      calibrations.liberal_tracking_initial_length = 4.5F;
      calibrations.liberal_tracking_initial_len2 = 0.5F;
      calibrations.liberal_tracking_min_velocity_plausibility = 0.1F;
      calibrations.liberal_tracking_velocity_plausibility_scaling_factor = 0.5F;
      calibrations.liberal_tracking_sigma_level_pos_diff = 2.0F;
      calibrations.liberal_tracking_sigma_level_cloud = 5.0F;

      // Confidence overall
      calibrations.k_conf_overall_timeout_to_start_decay = 0.125F;

      calibrations.k_conf_overall_difference_thresh_long_posn_h = 2.0F;
      calibrations.k_conf_overall_difference_thresh_long_posn_m = 1.0F;
      calibrations.k_conf_overall_difference_thresh_long_posn_l = 0.5F;
      calibrations.k_conf_overall_difference_thresh_lat_posn_h = 2.0F;
      calibrations.k_conf_overall_difference_thresh_lat_posn_m = 1.0F;
      calibrations.k_conf_overall_difference_thresh_lat_posn_l = 0.5F;

      calibrations.k_conf_overall_difference_thresh_long_vel_h = 1.0F;
      calibrations.k_conf_overall_difference_thresh_long_vel_m = 0.5F;
      calibrations.k_conf_overall_difference_thresh_long_vel_l = 0.25F;
      calibrations.k_conf_overall_difference_thresh_lat_vel_h = 1.0F;
      calibrations.k_conf_overall_difference_thresh_lat_vel_m = 0.5F;
      calibrations.k_conf_overall_difference_thresh_lat_vel_l = 0.25F;

      calibrations.k_conf_overall_difference_thresh_speed_h = 1.5F;
      calibrations.k_conf_overall_difference_thresh_speed_m = 0.75F;
      calibrations.k_conf_overall_difference_thresh_speed_l = 0.375F;

      // Unreliable low confidence track
      calibrations.k_low_conf_unreliability_max_ttc = 10.0F;
      calibrations.k_conf_downselection_exclusion_box_lat = 8.0F;
      calibrations.k_conf_downselection_exclusion_box_long = 10.0F;
      calibrations.k_low_conf_unreliability_min_heading = F360_DEG2RAD(30.0F);
      calibrations.k_low_conf_max_allowed_host_speed_in_cta_scenarios = 0.2F;
      calibrations.k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios = F360_DEG2RAD(90.0F);
      calibrations.k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios = F360_DEG2RAD(15.0F);

      // Object class determination
      calibrations.k_ad_oc_mean_length_pedestrian = 0.8F;
      calibrations.k_ad_oc_standard_deviation_length_pedestrian = 0.4F;
      calibrations.k_ad_oc_mean_width_pedestrian = 0.8F;
      calibrations.k_ad_oc_standard_deviation_width_pedestrian = 0.3F;
      calibrations.k_ad_oc_mean_speed_pedestrian = 1.4F;
      calibrations.k_ad_oc_standard_deviation_speed_pedestrian = 1.2F;

      calibrations.k_ad_oc_mean_length_2wheel = 1.0F;
      calibrations.k_ad_oc_standard_deviation_length_2wheel = 0.5F;
      calibrations.k_ad_oc_mean_width_2wheel = 1.15F;
      calibrations.k_ad_oc_standard_deviation_width_2wheel = 0.2F;
      calibrations.k_ad_oc_mean_speed_2wheel = 25.0F;
      calibrations.k_ad_oc_standard_deviation_speed_2wheel = 14.0F;

      calibrations.k_ad_oc_mean_length_car = 5.3F;
      calibrations.k_ad_oc_standard_deviation_length_car = 1.9F;
      calibrations.k_ad_oc_mean_width_car = 2.0F;
      calibrations.k_ad_oc_standard_deviation_width_car = 0.3F;
      calibrations.k_ad_oc_mean_speed_car = 25.0F;
      calibrations.k_ad_oc_standard_deviation_speed_car = 14.0F;

      calibrations.k_ad_oc_mean_length_truck = 18.0F;
      calibrations.k_ad_oc_standard_deviation_length_truck = 4.0F;
      calibrations.k_ad_oc_mean_width_truck = 2.2F;
      calibrations.k_ad_oc_standard_deviation_width_truck = 0.3F;
      calibrations.k_ad_oc_mean_speed_truck = 22.0F;
      calibrations.k_ad_oc_standard_deviation_speed_truck = 11.0F;

      calibrations.k_ad_oc_boundary_lowspeed = 2.8F;
      calibrations.k_ad_oc_boundary_highspeed = 22.0F;

      calibrations.k_ad_oc_apriori_lowspeed_pedestrian = 0.25F;
      calibrations.k_ad_oc_apriori_lowspeed_2wheel = 0.25F;
      calibrations.k_ad_oc_apriori_lowspeed_car = 0.25F;
      calibrations.k_ad_oc_apriori_lowspeed_truck = 0.25F;

      calibrations.k_ad_oc_apriori_highspeed_pedestrian = 0.04F;
      calibrations.k_ad_oc_apriori_highspeed_2wheel = 0.32F;
      calibrations.k_ad_oc_apriori_highspeed_car = 0.32F;
      calibrations.k_ad_oc_apriori_highspeed_truck = 0.32F;

      calibrations.k_ad_oc_step_decrease_prob_unknown = 0.1F;

      calibrations.k_ad_oc_min_thres_vel_2wheel = 8.3F;
      calibrations.k_ad_oc_max_thres_vel_2wheel = 11.1F;
      calibrations.k_ad_oc_weight_frac_2wheel = 0.25F;

      calibrations.k_ad_oc_min_prob_winner_class = 0.65F;
      calibrations.k_ad_oc_lowered_min_prob_winner_class = 0.45F;

      calibrations.k_ad_oc_min_pedestrian_speed = 0.3F;
      calibrations.k_ad_oc_max_pedestrian_speed = 3.0F;
      calibrations.k_ad_oc_max_stationary_speed = 0.55F;
      calibrations.k_ad_oc_prob_decrease = 0.3F;
      calibrations.k_ad_oc_min_prob_pedestrian = 0.9F;

      // Predict existence probability
      calibrations.k_p_persist_outside_long_range = 0.66F;
      calibrations.k_p_persist_inside_long_range = 0.99F;

      // Bistatic
      calibrations.k_bistatic_cond_assoc_area_min_lat = -5.0F;
      calibrations.k_bistatic_cond_assoc_area_max_lat = 5.0F;
      calibrations.k_bistatic_cond_assoc_area_min_lon = -12.0F;
      calibrations.k_bistatic_cond_assoc_area_max_lon = 30.0F;
      calibrations.k_bistatic_lat_th_extension = 0.7F;

      // Host mirror track
      calibrations.k_host_refl_min_obj_long_pos = -15.0F;
      calibrations.k_host_refl_max_obj_long_pos = 5.0F;

      calibrations.k_host_refl_half_host_length = 2.5F;

      calibrations.k_host_refl_bbox_lat_ext = 1.75F;
      calibrations.k_host_refl_bbox_long_ext = 1.0F;

      calibrations.k_host_refl_filtering_distance = 20.0F;

      calibrations.k_host_refl_lowspeed_host_speed_th = 6.0F;
      calibrations.k_host_refl_lowspeed_speed_diff_th = 1.0F;
      calibrations.k_host_refl_lowspeed_heading_th = 0.1F;

      calibrations.k_host_refl_highspeed_heading_th = 0.18F;
      calibrations.k_host_refl_highspeed_min_speed_diff_th = 1.0F;
      calibrations.k_host_refl_highspeed_speed_diff_ramp_coef = 0.4F;
      calibrations.k_host_refl_highspeed_max_speed_diff_th = 7.0F;

      calibrations.k_ws_bbox_len_extension_factor = 0.3F;
      calibrations.k_ws_bbox_wid_extension_factor = 0.1F;

      // Object confidence and raw confidence level
      calibrations.k_conf_updated_tracks_filter_const = 0.2F;
      calibrations.k_conf_raw_weight_dets = F360_Logf(0.1F);
      calibrations.k_conf_raw_max_value_not_reduced_dets = 1.0F - F360_Expf(calibrations.k_conf_raw_weight_dets * 1.0F); // max allowed raw confidence level with only associated detections
                                                                                                                         // is equal to confidence with a single reduced detection

      calibrations.k_conf_coasted_min_time_since_init = 1.0F;
      calibrations.k_conf_coasted_min_time_trusted_track = 3.0F;
      calibrations.k_conf_coasted_min_long_posn_tructed_track = 30.0F;
      calibrations.k_conf_coasted_max_long_posn_tructed_track = 100.0F;
      calibrations.k_conf_coasted_min_average_confidence_level = 0.7F;

      // Flagging of azimuth range rate outliers
      calibrations.k_az_rdot_max_sq_dist = 49.0F;
      calibrations.k_az_rdot_max_az_diff = F360_DEG2RAD(3.0F);
      calibrations.k_az_rdot_min_rdot_diff = 0.25F;

      // Dead zone
      calibrations.k_use_dead_zone_in_stationkeeping_scenarions = true;
      calibrations.k_dead_zone_min_host_speed = 5.0F;
      calibrations.k_dead_zone_max_obj_vcs_lat_pos = 5.0F;
      calibrations.k_dead_zone_max_obj_vcs_heading = 0.5F;
      calibrations.k_dead_zone_max_rel_vel_diff = 0.2F;
      calibrations.k_dead_zone_assoc_gates_additional_enhacementl = 1.0F;
      calibrations.k_dead_zone_long_limit_extension = 4.0F;

      // CWD - Concrete Wall Detector
      calibrations.k_cwd_max_diff_from_aggregated_lat_pos = 0.2F;
      calibrations.k_cwd_min_lat_posn_for_cw_sensor = 0.5F;
      calibrations.k_cwd_min_longitudinal_validity_limit = 2.0F;
      calibrations.k_cwd_max_longitudinal_validity_limit = 10.0F;
      calibrations.k_host_speed_for_min_long_validity_limit = 2.77F;
      calibrations.k_host_speed_for_max_long_validity_limit = 13.88F;
      calibrations.k_cwd_max_lateral_range = 999.0F;
      calibrations.k_cwd_sensor_zone_half_length = 0.26F;
      calibrations.k_cwd_one_sensor_at_side_conf_factor = 0.8F;
      calibrations.k_cwd_side_dist_span_low_limit = 0.3F;
      calibrations.k_cwd_side_dist_span_high_limit = 0.6F;
      calibrations.k_cwd_spread_weight = 3.0F;
      calibrations.k_cwd_max_lateral_spread = 1.0F;
      calibrations.k_cwd_min_host_speed_for_cwd = 1.5F;
      calibrations.k_cwd_min_conf_to_confirm_cw_by_single_sensor = 0.60001F;
      calibrations.k_cwd_one_sensor_long_validity_offset = 2.0F;
      calibrations.k_cwd_min_num_samples_for_outlier_finding = 3U;

      // Occlusion
      calibrations.k_occlusion_enabled = true;
      calibrations.k_occlusion_confidence_level = 0.9F;
      calibrations.k_occlusion_confidence_level_if_front_rear_not_visible = 0.7F;
      calibrations.k_occlusion_min_long = -60.0F;
      calibrations.k_occlusion_max_long = 60.0F;
      calibrations.k_occlusion_min_lat = -15.0F;
      calibrations.k_occlusion_max_lat = 15.0F;
      calibrations.k_occlusion_range_uncertainty_th = 0.5F;

      // Split logic
      calibrations.k_orth_split_min_speed = 5.0F;
      calibrations.k_orth_split_orth_delta_filter_const = 0.2F;
      calibrations.k_orth_split_orth_gap_filter_prop_const = 0.06F;
      calibrations.k_orth_split_orth_gap_filter_max_dets = 5U;
      calibrations.k_orth_split_min_orth_gap_for_split = 3.0F;
      calibrations.k_orth_split_width_gain = 1.2F;
      calibrations.k_orth_split_max_distance_sq = 80.0F * 80.0F;
      calibrations.k_orth_split_min_distance_sq = 15.0F * 15.0F;
      calibrations.k_pos_delta_heading_filter_constant = 0.1F;

      // Multi Path detector - low level logic
      calibrations.k_mp_object_reflector_size_extension = 0.1F;

      // Multi Path detector - high level logic
      calibrations.k_mp_max_allowed_host_speed_to_use_MP = 2.0F;
      calibrations.k_mp_default_mirror_probability = 1.0F;

      // Multi Path detector - reflector selector
      calibrations.k_mp_half_long_zone = 20.0F;
      calibrations.k_mp_half_lat_zone = 30.0F;

      // RCS filter (Radar Cross Section filter)
      calibrations.k_average_rcs_filter_constant = 0.15F;

      // Overall Confidence Blocker
      calibrations.k_ocb_cnt_delta_midlow_rcs = 1U;
      calibrations.k_ocb_cnt_delta_low_rcs_or_mult_dets = 2U;
      calibrations.k_ocb_cnt_max = 10U;
      calibrations.k_ocb_rcs_thresh_midlow_rcs = -18.0F;
      calibrations.k_ocb_rcs_thresh_low_rcs = -23.0F;
      calibrations.k_ocb_rcs_thresh_hi_rcs = 5.0F;
      calibrations.k_ocb_max_range = 10.0F;
      calibrations.k_ocb_max_range_rate = 2.0F;

      calibrations.k_cca_low_speed_th_to_ramp_down_proceess_noise = 1.5F;
      calibrations.k_cca_high_speed_th_to_ramp_down_proceess_noise = 6.0F;
      calibrations.q_cca_pos_para_high_speed = 2.0F;
      calibrations.q_cca_acc_para_high_speed = 25.0F;
      calibrations.q_cca_pos_orth_high_speed = 2.0F;
      calibrations.q_cca_acc_orth_high_speed = 2.0F;
      calibrations.q_cca_pos_low_speed = 0.3F;
      calibrations.q_cca_vel = 0.0F;
      calibrations.q_cca_acc_low_speed = 0.05F;
      calibrations.q_cca_pnt = 0.0F;
      calibrations.q_cca_heading_rate = 0.05F;
      calibrations.k_speed_th_to_slow_down_cca_filter = 4.0F;
      calibrations.init_cca_pnt_filter_cov[0][0] = 0.6F;
      calibrations.init_cca_pnt_filter_cov[0][1] = 0.0F;
      calibrations.init_cca_pnt_filter_cov[1][0] = 0.0F;
      calibrations.init_cca_pnt_filter_cov[1][1] = 0.01F;
      calibrations.k_cca_to_ctca_time_since_init_th = 2.0F;
      calibrations.k_max_num_cca_updates_since_init_to_limit_acc = 10U;
      calibrations.k_cca_heading_rate_high_speed_breakpoint = 7.0F;
      calibrations.k_cca_heading_rate_low_speed_breakpoint = 4.0F;
      calibrations.k_cca_maximum_heading_rate = 1.0F;
      calibrations.k_cca_to_ctca_curvature_th = 0.002F;
      calibrations.k_max_accepted_vel_error = 0.25F;
      calibrations.k_cca_min_speed_to_update_pnt = 0.001F;

      // Sanity check given calibrations
      assert(calibrations.k_orth_split_min_speed >= calibrations.k_min_speed_for_updating_heading);
   }
}
