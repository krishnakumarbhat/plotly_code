/*===================================================================================*\
Disclaimer:
This file is intended as an example showing how to integrate the tracker, it is not to be used as is for production.
All values used/assigned below are example values from previous programs, they are not to be reused.
\*===================================================================================*/


/*===================================================================================*\
 * FILE:  CAF_Parameters.h
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains declarations of Car specific parameters
 * These structures should be changed to match the available data from the customer
 * These structures contain the minimum required signals
 *
 * Applicable Standards (in order of precedence: highest first):
 * ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 * ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/
#ifndef CAF_PARAMETERS_H
#define CAF_PARAMETERS_H

#include "f360_reuse.h"
#include "T360_Types.h"

typedef struct
{
   float Sensor_offset_long;
   float Sensor_offset_lat;
   float Sensor_offset_vert;
   float Sensor_offset_az_angle;
   float Sensor_offset_el_angle;
   int32_t Sensor_polarity;
}CAF_Sensor_Param_T;

typedef struct
{
   float dist_rear_axle_to_vcs_m;       // [m] distance bewteen center of rear axis to VCS origin
   float rear_cornering_compliance;     // [rad*m/s*s] The rear cornering compliance.
   float steer_gear_ratio;              // Ratio of steering wheel angle to wheel angle
   float wheelbase_m;                   // [m] wheelbase in meters
   float understeer_coefficient;        // Understeer coefficient
   float vehicle_width_m;               // [m] Width of the vehicle
   float vehicle_length_m;              // [m] Length of the vehicle
   bool f_enable_internal_reflections_func;
   bool f_enable_internal_reflections_func_trailer;
}CAF_Host_Param_T;

typedef struct
{
   CAF_Host_Param_T host;
   CAF_Sensor_Param_T sensor[NUM_TOTAL_RADAR_SENSORS];
}CAF_Param_T;

#endif // CAF_PARAMETERS_H
