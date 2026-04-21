/*===========================================================================*\
* FILE: rspp_calibrations.h
*============================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains RSPP_Calibrations structure declaration
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#ifndef RSPP_CALIBRATIONS_H
#define RSPP_CALIBRATIONS_H

#include "f360_reuse.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   typedef struct RSPP_Calibrations_Tag
   {
      // Vcs longitudinal sorted detection rerence points in longitudinal direction in ascending order
      // As detections are sorted the first one that have a long pos greater than a reference point is "tagged" by
      // saving its detection index in tracker info structure. The array vcs_long_sorted_ref_points contains reference
      // points that must be defined in ascending order.
      float32_t vcs_long_sorted_ref_points[MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS];

      // Sensor signal uncertainties and other
      float32_t fov_interior_limit;
      float32_t k_range_rate_std;
      float32_t k_range_std;

      // Calculating detection moving threshold
      float32_t k_det_mov_slope;                  // [-] linear function slope value.
      float32_t k_det_mov_intercept;              // [-] linear function intercept value.
      float32_t k_det_mov_min;                    // [m/s] minimum moving threshold below which range rate threshold is saturated.
      float32_t k_det_mov_max;                    // [m/s] maximum moving threshold above which range rate threshold is saturated.
      float32_t k_det_mov_low_az_conf_penalty;    // [m/s] penalty value if det azimuth confidence is low (<= 3).
      float32_t k_det_az_error_stat_mov_penalty;  // [m/s] penalty value if det that flag f_azimuth_error_stat_mov is true.
      float32_t k_det_motion_sigma_th;            // [-] threshold for stationary motion clasicication.

      // Mulitpath detection filter
      float32_t k_min_host_speed_for_bad_az_filter;           // [m/s] Minimum speed of host to filter out detections with bad azimuth if fraction is large enough.
      float32_t k_max_fraction_of_bad_azimuth_dets_default;   // [-] Default max fraction of detections with worst azimuth confidence from a sensor to allow usage of these detections.
      float32_t k_max_fraction_of_bad_azimuth_dets_srr5;      // [-] Max fraction of detections with worst azimuth confidence from a SRR5 sensor to allow usage of these detections.
      uint32_t k_min_num_valid_dets_for_bad_az_filter;     // [-] Minimum number of detections per sensor to filter out detections with bad azimuth if fraction is large enough.

      // Detection motion classification constants
      float32_t k_dmc_base_thr;                          // [m/s] Base range-rate for threshold limitation
      float32_t k_dmc_host_speed_offset;                 // [m/s] Host speed offset for speed component in the threshold limitation function
      float32_t k_dmc_host_speed_scale_factor;           // [-]   Scale factor for squared host speed in the threshold limitation function
      float32_t k_dmc_host_curvature_scale_factor;       // [-]   Scale factor for squared host curvature in the threshold limitation function
      float32_t k_dmc_bypass_det_range_thr_sq;           // [m^2] Sqared range for bypassing the threshold limitation function
      float32_t k_dmc_range_rate_aliasing_buffer;        // [m/s] Buffer for calculating expected aliased compensated range-rate

      // Sensor preprocessing
      float32_t k_min_host_speed_for_check_det_az_conf_and_elevation;  // [m/s] Sensor preprocessing module: Minimum host speed for checking detection azimuth confidence and elevation
      float32_t k_mrr3_max_range;                                      // [m] Sensor preprocessing module: maximum range of MRR3 sensor
      float32_t k_mrr3_max_abs_elev_angle;                             // Sensor preprocessing module: maximum absolute value of elevation angle for MRR4 detection
      float32_t k_srr4_max_elevation;                                  // Sensor preprocessing module: maximum elevation for SRR4 detection
      float32_t k_srr4_max_azimuth_super_res;                          // Sensor preprocessing module: maximum azimuth for SRR4 super resolution detection
      float32_t k_srr4_min_range_az_conf;                              // Sensor preprocessing module: minimum range for SRR4 detection with low azimuth confidence
      float32_t k_srr4_min_azimuth_az_conf;                            // Sensor preprocessing module: minimum azimuth for SRR4 detection with low azimuth confidence and high azimuth
      float32_t k_srr4_max_range_rate_comp_az_conf;                    // Sensor preprocessing module: maximum range rate compensated for SRR4 detection with low azimuth confidence
      float32_t k_sens_vcs_mounting_az_std;                            // STD of az alignment estimate
      float32_t k_max_threshold_range_rate_compensated;                // maximum value of the compensated range rate
      int8_t k_mrr3_conf_thresh;                                   // [-] Sensor preprocessing module: confidence threshold for MRR3 detection

      // MRR360 bandaid for filtering away detections with big absolute elevation angle
      bool f_mrr360_filter_away_big_elev_angle;  // Parameter for filtering away detections with absolute elevation angle bigger than k_mrr360_max_abs_elev_angle_rad
      float32_t k_mrr360_max_abs_elev_angle_rad;        // [rad] maximum absolut elevation angle allowed for detections to be used

      bool k_inputs_preprocessing_inside_tracker; // TODO: DFT-2097 this should be removed once Inputs Preprocessing is extracted from tracker
      
      // Detection double bounce detector
      float k_db_max_range; // [m] Maximum range of detection to check it for double bounce
      uint32_t k_db_max_nr_multi_bounces; // [-] Maximum number of bounces to consider
      float k_db_range_threshold_frac; // [-] Fraction of secondary detection range to consider as threshold for double bounce. Greater range gives a more generous range gate to satisfy double bounce range condition 
      float k_db_min_range_threshold; // [m] Minimum range tolerance limit relative predicted range
      float k_db_max_range_threshold; // [m] Maximum range tolerance limit relative predicted range
      float k_db_range_rate_threshold; // [m/s] Range rate threshold for considering if a detection is double bounce
      float k_db_azimuth_thres_k; // [1/m] K value for deriving azimuth threshold as a linear function of range
      float k_db_azimuth_thres_m; // [m] M value for deriving azimuth threshold as a linear function of range
      float k_db_min_azimuth_thres; // [rad] Min azimuth threshold for double bounce
      float k_db_max_azimuth_thres; // [rad] Max azimuth threshold for double bounce
   
      uint8_t padding[3];

   } RSPP_Calibrations_T;

   void Initialize_RSPP_Calibrations(RSPP_Calibrations_T& rspp_calibrations);
}
#endif
