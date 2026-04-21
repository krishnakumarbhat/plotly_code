/*===================================================================================\
 * FILE: rspp_calibrations.cpp
 *====================================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains function definition for Initialize_RSPP_Calibrations.
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

#include "rspp_calibrations.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_RSPP_Calibrations()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * RSPP_Calibrations_T &rspp_calibrations
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
   * This function initializes default values for rspp_calibrations
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Initialize_RSPP_Calibrations(RSPP_Calibrations_T& rspp_calibrations)
   {
      rspp_calibrations.k_inputs_preprocessing_inside_tracker = true;

      // MRR360 bandaid for filtering away detections with big absolute elevation angle
      rspp_calibrations.k_mrr360_max_abs_elev_angle_rad = F360_DEG2RAD(6.0F);

      //MRR360 filter on by default in F360 core releases
      rspp_calibrations.f_mrr360_filter_away_big_elev_angle = true;

      // Vcs longitudinal sorted detections reference points, needs to be in ascending order
      const float32_t vcs_sorted_ref_points[MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS] = {
         -100.0F, -90.0F, -80.0F, -70.0F, -60.0F, -50.0F,
         -40.0F, -30.0F, -20.0F, -10.0F, 0.0F, 10.0F,
         20.0F, 30.0F, 40.0F, 50.0F, 60.0F, 70.0F,
         80.0F, 90.0F, 100.0F, 110.0F, 120.0F, 130.0F,
         140.0F, 150.0F, 160.0F, 170.0F, 180.0F, 190.0F, 200.0F };
      for (uint32_t i = 0U; i < MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS; i++)
      {
         rspp_calibrations.vcs_long_sorted_ref_points[i] = vcs_sorted_ref_points[i];
      }

      rspp_calibrations.fov_interior_limit = 1.1345F; // 65 degrees
      rspp_calibrations.k_range_rate_std = 0.06F; // Detection range rate std, taken from radar data sheets.

      // Sensor preprocessing: Check_Detection_Azimuth_Confidence_and_Elevation
      rspp_calibrations.k_min_host_speed_for_check_det_az_conf_and_elevation = 2.0F;
      rspp_calibrations.k_mrr3_max_range = 20.0F;
      rspp_calibrations.k_mrr3_conf_thresh = 2;
      rspp_calibrations.k_mrr3_max_abs_elev_angle = F360_DEG2RAD(5.5F);
      rspp_calibrations.k_srr4_max_elevation = F360_DEG2RAD(5.0F);
      rspp_calibrations.k_srr4_max_azimuth_super_res = F360_DEG2RAD(20.0F);
      rspp_calibrations.k_srr4_min_range_az_conf = 15.0F;
      rspp_calibrations.k_srr4_min_azimuth_az_conf = F360_DEG2RAD(45.0F);
      rspp_calibrations.k_srr4_max_range_rate_comp_az_conf = 5.0F;
      rspp_calibrations.k_max_threshold_range_rate_compensated = 300.0F;

      // Sensor preprocessing: Sensor Capability Detections
      rspp_calibrations.k_sens_vcs_mounting_az_std = F360_DEG2RAD(0.15F);

      // Mulitpath detection filter
      rspp_calibrations.k_min_num_valid_dets_for_bad_az_filter = 50U;
      rspp_calibrations.k_min_host_speed_for_bad_az_filter = 1.0F;
      rspp_calibrations.k_max_fraction_of_bad_azimuth_dets_default = 1.0F;
      rspp_calibrations.k_max_fraction_of_bad_azimuth_dets_srr5 = 0.33F;

      // Calculating detection moving threshold
      rspp_calibrations.k_det_mov_slope = 0.04F;
      rspp_calibrations.k_det_mov_intercept = 0.18F;
      rspp_calibrations.k_det_mov_min = 0.3F;
      rspp_calibrations.k_det_mov_max = 1.5F;
      rspp_calibrations.k_det_mov_low_az_conf_penalty = 1.0F;
      rspp_calibrations.k_det_az_error_stat_mov_penalty = 3.0F;

      rspp_calibrations.k_det_motion_sigma_th = 3.0F;

      // Detection motion classification threshold limitation
      rspp_calibrations.k_dmc_base_thr = 1.5F;
      rspp_calibrations.k_dmc_host_speed_offset = 15.0F;
      rspp_calibrations.k_dmc_host_speed_scale_factor = 0.02F;
      rspp_calibrations.k_dmc_host_curvature_scale_factor = 500.0F;
      rspp_calibrations.k_dmc_bypass_det_range_thr_sq = 25.0F;
      rspp_calibrations.k_dmc_range_rate_aliasing_buffer = 5.0F;
   }
}
