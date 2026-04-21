#ifndef RSPP_HOST_H
#define RSPP_HOST_H
/*===================================================================================*\
* FILE: rspp_host.h
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

struct RSPP_Host_T
{
   uint32_t vehicle_index;

   float32_t speed;
   float32_t vcs_speed;
   float32_t acceleration;
   float32_t vcs_lat_acceleration;
   float32_t vcs_long_acceleration;
   float32_t yaw_rate_rad;                 // Compensated for yaw_rate bias
   float32_t vcs_sideslip;
   float32_t curvature_rear;
   float32_t dist_rear_axle_to_vcs_m;      // [m] The distance from the rear axle to VCS origin
   float32_t rear_cornering_compliance;
   float32_t speed_correction_factor;

   // enum F360_QUALITY_FACTOR: 0 - UNDEF, 1 - TEMP_UNDEF, 2 - INACCURATE, 3 - ACCURATE
   uint8_t speed_qf;
   uint8_t yaw_rate_qf;
   uint8_t lat_accel_qf;
   uint8_t long_accel_qf;

   bool f_trailer_presence_hardware; // 1: trailer presence detected by the host vehicle electronically
};

#endif
