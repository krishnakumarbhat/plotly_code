/*===================================================================================*\
Disclaimer:
This file is intended as an example showing how to integrate the tracker, it is not to be used as is for production.
All values used/assigned below are example values from previous programs, they are not to be reused.
\*===================================================================================*/

/*===================================================================================*\
 * FILE:  tracker_IAL.h
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains declarations of input data to f360_tracker
 * These structures should be changed to match the available data from the customer
 * These structures contain the minimum required signals
 *
 * Applicable Standards (in order of precedence: highest first):
 * ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 * ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/
#ifndef TRACKER_IAL_H
#define TRACKER_IAL_H

#include "f360_reuse.h"
#include "T360_Types.h"

typedef struct SRR_Sens_Data_Tag
{
   uint64_t timestamp_us;
   uint32_t number_of_valid_detections;
   uint16_t look_index;
   uint8_t look_id;
   bool f_full_blockage;
   float vacs_boresight_az_estimated;
   float vacs_boresight_el_estimated;
   float speed_correction_factor;
}SRR_Sens_Data_T;

typedef struct SRR_Det_Data_Tag
{
   int32_t det_id;           // Id from the sensor that the detection came from (Unique only to that sensor) 
   float  range;             // [m] Range of detection
   float  std_range;         // [m] Standard deviation of range of detection
   float  range_rate;        // [m/s] Range rate of detections
   float  std_range_rate;    // [m/s] Standard deviation of range rate of detections
   float  azimuth;           // [rad] Raw Azimuth angle. Neither polarity nor alignment are compensated. Positive to the right
   float  std_azimuth;       // [rad] Standard deviation of Azimuth angle.
   float  elevation;         // [rad] Raw Elevation angle. Neither polarity nor alignment are compensated. Positive to the right
   float  std_elevation;     // [rad] Standard deviation of Elevation angle.
   float  snr;               // [-] Signal to noise ratio 
   float  amplitude;         // [dB/m^2] RCS normalized amplitude of detection 
   int8_t  confid_azimuth;   // Confidence on azimuth, 0 = best, 3 = worst
   int8_t  confid_elevation; // Confidence on elevation, 0 = best, 3 = worst
   bool  f_super_res;        // Flag indicating that super resolution branch have been used by the sensors angle finding algo
   bool  f_host_veh_clutter; // Flag indicating that this detection stems from host vehicle itself
   bool  f_nd_target;        // Flag indicating that this detection seems to stem from a small target close to a larger target
   bool  f_bistatic;         // Flag indicating that this detection is a bistatic detection
}SRR_Det_Data_T;

typedef struct SRR_Sensor_Info_Tag
{
   SRR_Sens_Data_T sens_data;
   SRR_Det_Data_T det_data[MAX_SRR_RADAR_DETS_PER_SENSOR];
}SRR_Sensor_Info_T;

typedef struct Ego_Vehicle_Info_Tag   /*Offset Start byte of the Structure Ego_Vehicle_Info_Core0 is 156 bytes*/
{
   // VSE
   uint32_t vehicle_index;      //!< index of the current vehicle iteration.
   float timestamp_s;           //!< [s] Timestamp of the raw vehicle data
   float speed;                 //!< [m/s] corrected signed speed of host at center of rear axis (speed = raw_speed * speed_correction_factor)
   float acceleration;          //!< [m/s^2] signed acceleration of host at center of rear axis
   float curvature_rear;        //!< [m] curvature at center of rear axis
   float vcs_speed;             //!< [m/s] signed speed of host at VCS origin
   float vcs_lat_acceleration;  //!< [m/s^2] lateral acceleration of host at VCS origin
   float vcs_long_acceleration; //!< [m/s^2] longitudinal acceleration of host at VCS origin
   float vcs_sideslip;          //!< [rad] angle between VCS longitudinal direction and velocity vector of vcs_speed
   float yaw_rate_rad;          //!< [rad/s] yaw-rate of host vehicle (compensated for yawrate bias), a turn to the right is positive

   float speed_correction_factor;

   // Host raw
   float raw_speed;             //!< [m/s] signed speed of host at center of rear axis provided by vehicle
   float steering_angle_rad;    //!< [rad] vehicle steering angle, (+) increase clockwise to the right
   float road_wheel_angle_rad;   // [rad] vehicle road wheel angle, (+) increase clockwise to the right
   float raw_yaw_rate_rad;      //!< [rad/s] yaw-rate of host vehicle, a turn to the right is positive
   uint8_t prndl;               //!< [0-6] (enum F360_PRNDL_STATE) Current gear selected; 0-PARK, 1-REVERSE, 2-NEUTRAL, 3-DRIVE, 4-FOURTH,
                                //!< 5-THIRD, 6-LOW
   bool f_reverse_gear;         //!< flag indicating vehicle is in reverse
   bool f_trailer_presence_hardware;      //!< flag indicating a trailer is connected to host

   uint8_t speed_qf;
   uint8_t yaw_rate_qf;
   uint8_t steering_wheel_angle_qf;
   uint8_t road_wheel_angle_qf;
   uint8_t lat_accel_qf;
   uint8_t long_accel_qf;
}Ego_Vehicle_Info_T;

typedef struct Tracker_IAL_Tag
{
   uint64_t tracker_timestamp_us;
   SRR_Sensor_Info_T srr_sensor_info[NUM_SRR_SENSORS];
   Ego_Vehicle_Info_T vehicle_info;
}Tracker_IAL_T;

#endif // TRACKER_IAL_H
