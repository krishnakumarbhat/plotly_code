/*===========================================================================*\
* FILE: f360_sensor_capability.h
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declarations for computing and propagating sensor uncertainties
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/
#ifndef SENSOR_CAPABILITY_H
#define SENSOR_CAPABILITY_H
/******************************
* Includes
*******************************/
#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_host.h"
#include "f360_host_props.h"
#include "f360_radar_sensor.h"
#include "rspp_detection_list.h"
#include "f360_detection_props.h"
#include "rspp_calibrations.h"
#include "f360_globals.h"
#include "f360_timing_info.h"

/******************************
* Declarations
*******************************/
namespace f360_variant_A
{
   void Sensor_Capability_Host(
      const float32_t elapsed_time,
      const F360_Host_T &host,
      const float32_t max_otg_speed,
      F360_Host_Props_T &host_props
   );

   void Calculate_Detections_Uncertainties(
      const F360_Host_T& host,
      const F360_Host_Props_T& host_props,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const RSPP_Calibrations_T& rspp_calibrations,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list
   );

   void Sensor_Capability(
      const uint32_t sensor_number_of_detections,
      const F360_Radar_Sensor_T &sensor,
      const F360_Host_T &host,
      const F360_Host_Props_T &host_props,
      const uint32_t (&sensor_det_idx)[MAX_DETS_FOR_SINGLE_SENSOR],
      const RSPP_Calibrations_T & rspp_calibrations,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list
   );

   float32_t Compute_Detection_VCS_Azimuth_Variance(
      const float32_t &det_azimuth_var,
      const RSPP_Calibrations_T &rspp_calibrations
   );

   void Get_Sensor_VCS_Mounting_Position(
      const F360_Radar_Sensor_T &sensor,
      float32_t (&sensor_vcs_mounting_pos)[2]
   );

   void Get_Sensor_VCS_Velocity(
      const F360_Radar_Sensor_T &sensor,
      float32_t (&sensor_vcs_vel_vec)[2]
   );

   void Assign_Detection_Per_Sensor(
      const rspp_variant_A::RSPP_Detection_T &raw_detection,
      const uint32_t det_idx,
      uint32_t (&det_idx_per_sensor)[MAX_NUMBER_OF_SENSORS][MAX_DETS_FOR_SINGLE_SENSOR],
      uint32_t (&ndets_per_sensor)[MAX_NUMBER_OF_SENSORS]
   );

   float32_t Compute_Raw_Host_Speed_Uncertainty(
      const F360_Host_T &host,
      const float32_t max_otg_speed
   );

   void Get_Host_Velocity_Uncertainty(
      const F360_Host_T &host,
      const float32_t host_speed_var,
      const float32_t host_yaw_rate_var,
      const float32_t (&translation_vec)[2],
      float32_t (&velocity_cov)[2][2]
   );

   void Compute_Raw_Detection_Uncertainty(
      const rspp_variant_A::RSPP_Detection_T &det,
      const F360_Radar_Sensor_T &sensor,
      const RSPP_Calibrations_T &rspp_calibrations,
      float32_t &range_var,
      float32_t &azimuth_var,
      float32_t &range_rate_var
   );

   void Get_Limits_For_FOV(
      const F360_Det_Range_Type_T range_type,
      const F360_Radar_Sensor_T &sensor,
      float32_t &min_interior_fov,
      float32_t &max_interior_fov
   );

   void Compute_Azimuth_Std_Vec_Based_On_Sensor_Type(
      const F360_Sensor_Type_T &sensor_type,
      float32_t (&std_vec)[7]
   );

   void Compute_Azimuth_Breakpoints(
      const float32_t &min_fov,
      const float32_t &az_safety_margin,
      const float32_t &min_interior_fov,
      const float32_t &frac_az,
      const float32_t &max_interior_fov,
      const float32_t &max_fov,
      float32_t (&az_breakpoint_vec)[7]
   );


   void Get_Uncertainty_Of_Compensated_Range_Rate(
      const float32_t cos_det_az,
      const float32_t sin_det_az,
      const float32_t (&sens_vel)[2],
      const float32_t var_det_rng_rate,
      const float32_t var_det_az,
      const float32_t (&cov_sens_vel)[2][2],
      float32_t &var_comp_rng_rate
   );

   void Get_Uncertainty_Of_Detection_Position(
      const float32_t det_rng,
      const float32_t cos_det_az,
      const float32_t sin_det_az,
      const float32_t var_det_rng,
      const float32_t var_det_az,
      float32_t (&cov_det_pos)[2][2]
   );

   void Compute_Detection_Cross_Covariances(
      const float32_t (&sens_vel_vcs)[2],
      const float32_t var_azimuth_vcs,
      rspp_variant_A::RSPP_Detection_T& detection
   );
}
#endif
