/*===================================================================================*\
* FILE: f360_detection.h
*====================================================================================
* Copyright (C) 2020 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains F360_Detection_T structure declaration.
*   It contains raw detection signals collected from sensors and also new signals added by Radar Sensor-preprocessing component.
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): defineFusion360Types.m
*
*   Requirements Document(s):
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_DETECTION_VARIANT_A_H
#define F360_DETECTION_VARIANT_A_H

#include "emblib_f360_types.h"
#include "f360_reuse.h"

namespace f360_variant_C
{
typedef struct F360_Detection_Tag
{
   F360_FPN_T range_rate_dealiased;   // range rate after dealiasing - compensate measurement limitations (valid when f_dealiased
                                      // flag is true, extesion - raw detection has the same signal) [m/s]
   F360_FPN_T range_rate_compensated; // (Raw detection extension) range rate after host motion compensation (like Over-The-Ground
                                      // range rate, extesion - raw detection has the same signal) [m/s]
   F360_FPN_T std_range_rate_compensated_scm; // STD of compensated range rate as computed by sensor capability module [m/s]
   F360_SI16N_T next_sorted_idx;              // index of the next sorted detection (with higher VCS longitudinal position) [-]
   F360_SI16N_T prev_sorted_idx;              // index of the previous sorted detection (with lower VCS longitudinal position) [-]
   F360_SI32N_T sensor_id;                    // Id of the sensor that the detection came from
   F360_SI32N_T det_id;                       // Id from the sensor that the detection came from (Unique only to that sensor)
   F360_FPN_T range;                          // [m] Range of detection
   F360_FPN_T std_range;                      // [m] Standard deviation of range of detection
   F360_FPN_T range_rate;                     // [m/s] Range rate of detections
   F360_FPN_T rngrate_interval_width;         // [m/s] interval of un-ambiguous range rate measurement
   F360_FPN_T std_range_rate;                 // [m/s] Standard deviation of range rate of detections
   F360_FPN_T azimuth_raw;   // [rad] "Raw" Azimuth angle. Compensated for polarity, but NOT for alignment. Positive to the right
   F360_FPN_T elevation_raw; // [rad] "Raw" Elevation angle. Compensated for polarity, but NOT for alignment. Positive to the right
   F360_FPN_T azimuth;       // [rad] Corrected Azimuth angle. Compensated for both polarity and alignment. Positive to the right
   F360_FPN_T std_azimuth;   // [rad] Standard deviation of Azimuth angle.
   F360_FPN_T vcs_az;        // [rad] azimuth aligned with VCS (corrected by boresight angle in VCS)
   F360_FPN_T cos_vcs_az;    // [-] cosine of vcs azimuth
   F360_FPN_T sin_vcs_az;    // [-] sine of vcs azimuth
   F360_FPN_T std_vcs_az_scm; // [rad] STD of vcs azimuth as computed by sensor capability module
   F360_FPN_T elevation;     // [rad] Corrected Elevation angle. Compensated for both polarity and alignment. Positive to the right
   F360_FPN_T std_elevation; // [rad] Standard deviation of Elevation angle.
   F360_FPN_T snr;           // [-] Signal to noise ratio
   F360_FPN_T amplitude;     // [dB/m^2] RCS normalized amplitude of detection
   F360_FPN_T
   vcs_cross_covariances_scm[5]; // cross covariances between; Element 0: VCS position x and VCS azimuth, Element 1: VCS
                                 // position x and compensated range rate, Element 2: VCS position y and VCS azimuth, Element 3:
                                 // VCS position y and compensated range rate, Element 4: VCS azimuth and compensated range rate
   F360_SI8N_T confid_azimuth;   // Confidence on azimuth, 0 = best, 3 = worst
   F360_SI8N_T confid_elevation; // Confidence on elevation, 0 = best, 3 = worst
   F360_BOOLN_T f_super_res;     // Flag indicating that super resolution branch have been used by the sensors angle finding algo
   F360_BOOLN_T f_host_veh_clutter; // Flag indicating that this detection stems from host vehicle itself
   F360_BOOLN_T f_nd_target; // Flag indicating that this detection seems to stem from a small target close to a larger target
   F360_BOOLN_T f_bistatic;  // Flag indicating that this detection is a bistatic detection
   F360_BOOLN_T
   f_dealiased; // flag indicating that detection's range rate is dealiased (extesion - raw detection has the same signal)  [-]
   F360_BOOLN_T f_ok_to_use;             // flag indicating that detection pass plausibility checks [-]
   F360_BOOLN_T f_is_range_in_all_looks; // flag indicating that detection range is within all radar looks [-]
   F360_UI8N_T padding;
} F360_Detection_T;

static_assert(124 == sizeof(F360_Detection_T), "sizeof(F360_Detection_T) not as expected. Remember to align padding if needed");
} // namespace f360_variant_C

#endif
