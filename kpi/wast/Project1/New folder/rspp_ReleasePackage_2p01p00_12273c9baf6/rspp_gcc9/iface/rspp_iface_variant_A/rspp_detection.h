#ifndef RSPP_DETECTION_VARIANT_A_H
#define RSPP_DETECTION_VARIANT_A_H
/*===================================================================================*\
* FILE: rspp_detection.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_reuse.h"
#include "rspp_detection_motion_status.h"


namespace rspp_variant_A
{
   typedef enum RSPP_Azimuth_Confidence_Tag
   {
      RSPP_CONF_AZIMUTH_HIGH = 0,
      RSPP_CONF_AZIMUTH_MIDHIGH = 1,
      RSPP_CONF_AZIMUTH_MIDLOW = 2,
      RSPP_CONF_AZIMUTH_LOW = 3,
   }RSPP_Azimuth_Confidence_T;

   typedef struct Raw_Detection_Tag
   {
      float32_t range;                             // [m] Range of detection
      float32_t std_range;                         // [m] Standard deviation of range of detection
      float32_t range_rate;                        // [m/s] Range rate of detection
      float32_t std_range_rate;                    // [m/s] Standard deviation of range rate of detections
      float32_t azimuth;                           // [rad] Raw Azimuth angle. Not compensated for alignment or polarity
      float32_t std_azimuth;                       // [rad] Standard deviation of Azimuth angle.
      float32_t elevation;                         // [rad] Raw Elevation angle. Not compensated for alignment or polarity
      float32_t std_elevation;                     // [rad] Standard deviation of Elevation angle.
      float32_t snr;                               // [-] Signal to noise ratio
      float32_t rcs;                               // [dB/m^2] RCS normalized rcs of detection
      int32_t sensor_id;                       // Id of the sensor that the detection came from 
      int32_t det_id;                          // Id from the sensor that the detection came from (Unique only to that sensor) 
      int8_t confid_azimuth;                   // Confidence on azimuth, 0 = best, 3 = worst
      int8_t confid_elevation;                 // Confidence on elevation, 0 = best, 3 = worst
      bool f_super_res;                     // Flag indicating that super resolution branch have been used by the sensors angle finding algo
      bool f_host_veh_clutter;              // Flag indicating that this detection stems from host vehicle itself
      bool f_nd_target;                     // Flag indicating that this detection seems to stem from a small target close to a larger target
      bool f_bistatic;                      // Flag indicating that this detection is a bistatic detection
      uint8_t padding[2];

   } Raw_Detection_T;

   typedef struct Processed_Detection_Tag
   {
      float32_t vcs_position_x;                    // longitudinal position in Vehicle Coordinate System (VCS) [m]
      float32_t vcs_position_y;                    // lateral position in Vehicle Coordinate System (VCS) [m]
      float32_t vcs_position_z;                    // z coordinate in Vehicle Coordinate System (VCS) [m], Note: The zero plane is defined at ground level and negative above ground
      float32_t vcs_position_cov_scm[2][2];        // [m^2] position covariance in VCS  as computed by sensor capability module
      float32_t range_rate_compensated;            // (Raw detection extension) range rate after host motion compensation (like Over-The-Ground range rate, extesion - raw detection has the same signal) [m/s]
      float32_t std_range_rate_compensated_scm;    // STD of compensated range rate as computed by sensor capability module [m/s]
      float32_t vcs_az;                            // [rad] azimuth aligned with VCS (corrected by boresight angle in VCS)
      float32_t vcs_el;                            // NOTE: This signal is to be compensated by elevation misaligment in future releases. For now it is populated by the raw elevation angle but compensated by polarity [rad]
      float32_t cos_vcs_az;                        // [-] cosine of vcs azimuth
      float32_t sin_vcs_az;                        // [-] sine of vcs azimuth
      float32_t std_vcs_az_scm;                    // [rad] STD of vcs azimuth as computed by sensor capability module 
      float32_t vcs_cross_covariances_scm[5];      // cross covariances between; Element 0: VCS position x and VCS azimuth, Element 1: VCS position x and compensated range rate, Element 2: VCS position y and VCS azimuth, Element 3: VCS position y and compensated range rate, Element 4: VCS azimuth and compensated range rate
      int16_t next_sorted_idx;                 // index of the next sorted detection (with higher VCS longitudinal position) [-]
      int16_t prev_sorted_idx;                 // index of the previous sorted detection (with lower VCS longitudinal position) [-]
      int32_t global_id;                       // non-repeatable identifier [-]
      int8_t motion_status;                    // [-] -1: invalid, 0: stationary, 1: moving, 2: ambiguous status of detection motion
      bool f_ok_to_use;                     // flag indicating that detection pass plausibility checks [-]
      bool f_azimuth_error_stat_mov;        // [-] flag indicating that detection with suspicious elevation angle and low azimuth confidence (countermeasure)
      bool f_double_bounce;                 // flag indicating that detection is multi-bounce (at the double range and range rate of other detection) (countermeasure) [-]
   } Processed_Detection_T;

   typedef struct RSPP_Detection_Tag
   {
      Raw_Detection_T raw;
      Processed_Detection_T processed;
   } RSPP_Detection_T;
   
   static_assert(144 == sizeof(RSPP_Detection_T), "sizeof(RSPP_Detection_T) not as expected. Remember to align padding if needed");
}
#endif
