/*===================================================================================*\
 * FILE: f360_host_calib.h
 *====================================================================================
 * Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
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
 *   This file contains static host structure related declaration
 *
 * ABBREVIATIONS:
 *  None
 *
 * TRACEABILITY INFO:
 *   Design Document(s):  PRNDL_STATE.m
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
#ifndef F360_HOST_CALIB_VARIANT_I_H
#define F360_HOST_CALIB_VARIANT_I_H

#include "f360_reuse.h"

namespace f360_variant_I
{
   typedef struct F360_Host_Calib_Tag
   {
      /* Vehicle properties */
      float dist_rear_axle_to_vcs_m;       // [m] distance bewteen center of rear axis to VCS origin
      float rear_cornering_compliance;     // [rad*m/s*s] The rear cornering compliance.
      float steer_gear_ratio;              // Ratio of steering wheel angle to wheel angle
      float wheelbase_m;                   // [m] wheelbase in meters
      float understeer_coefficient;        // Understeer coefficient
      float vehicle_width_m;               // [m] Width of the vehicle
      float vehicle_length_m;              // [m] Length of the vehicle
      float cog_x;                         // [m] Center of gravity
      float cog_y;                         // [m] Center of gravity
      float front_wheel_radius_m;          // [m] Wheel radius when vehicle is loaded to GVW
      float front_track_width_m;           // [m] Front Track Width
      uint32_t raw_host_signal_latency_ms; // [ms] latency of raw host signal
      bool f_enable_internal_reflections_func;         // [-] Flag to enable the internal reflections function using general tuning
      bool f_enable_internal_reflections_func_trailer; // [-] Flag to allow the internal reflections function to be enabled when a trailer is connected to host using trailer tuning
      uint8_t padding[2];
   } F360_Host_Calib_T;

   static_assert(sizeof(F360_Host_Calib_T) == 52, "F360_Host_Calib_T: Wrong size.");
}

#endif
