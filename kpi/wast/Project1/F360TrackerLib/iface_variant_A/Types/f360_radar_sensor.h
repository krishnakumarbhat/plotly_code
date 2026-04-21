/*===================================================================================*\
* FILE: f360_radar_sensor.h
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
*   This file contains F360_Radar_Sensor_T structure declaration
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): defineFusion360Types.m
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
#ifndef F360_RADAR_SENSOR_VARIANT_A_H
#define F360_RADAR_SENSOR_VARIANT_A_H

#include "f360_velocity.h"
#include "f360_range_type.h"
#include "f360_look_type.h"
#include "f360_reuse.h"
#include "f360_sensor_mounting_position.h"
#include "f360_mounting_location.h"
#include "f360_sensor_type.h"
#include "f360_look_ID.h"
#include "T360_Types.h"

namespace f360_variant_A
{
   // Function for mapping Look ID to Look Type (Added since Look Type was removed from the interface)
   inline F360_Det_Look_Type_T Get_Look_Type(const F360_Det_Look_ID_T look_ID)
   {
      F360_Det_Look_Type_T look_type;
      switch (look_ID)
      {
      case F360_DET_LOOK_ID_0:
      {
         look_type = F360_DET_LOOK_TYPE_LONG;
         break;
      }
      case F360_DET_LOOK_ID_1:
      {
         look_type = F360_DET_LOOK_TYPE_MEDIUM;
         break;
      }
      case F360_DET_LOOK_ID_2:
      {
         look_type = F360_DET_LOOK_TYPE_LONG;
         break;
      }
      case F360_DET_LOOK_ID_3:
      {
         look_type = F360_DET_LOOK_TYPE_MEDIUM;
         break;
      }
      default:
      {
         look_type = F360_DET_LOOK_TYPE_INVALID;
         break;
      }
      }

      return look_type;
   }

   // Function for mapping Look ID to Range Type (Added since Range Type was removed from the interface)
   inline F360_Det_Range_Type_T Get_Range_Type(const F360_Det_Look_ID_T look_ID)
   {
      F360_Det_Range_Type_T range_type;
      switch (look_ID)
      {
      case F360_DET_LOOK_ID_0:
      {
         range_type = F360_DET_RANGE_TYPE_LONG;
         break;
      }
      case F360_DET_LOOK_ID_1:
      {
         range_type = F360_DET_RANGE_TYPE_LONG;
         break;
      }
      case F360_DET_LOOK_ID_2:
      {
         range_type = F360_DET_RANGE_TYPE_MEDIUM;
         break;
      }
      case F360_DET_LOOK_ID_3:
      {
         range_type = F360_DET_RANGE_TYPE_MEDIUM;
         break;
      }
      default:
      {
         range_type = F360_DET_RANGE_TYPE_INVALID;
         break;
      }
      }

      return range_type;
   }

   struct F360_Internal_Reflections_Calib_T
   {
       float32_t min_host_vel;           // [m/s] identification of internal reflections only updates its buffer slots if host is moving at least this fast

       float32_t occurrence_lowerlimit;  // [fraction of tracker iterations] identification of internal reflections resets buffer slots below this occurrence rate
       float32_t occurrence_threshold;   // [fraction of tracker iterations] identification of internal reflections considers a buffer slot a reflection if it occurred at least this often (as a fraction of occurrence_count)

       float32_t rcs_tolerance;    // [dB/m^2] identification of internal reflections counts recurrence of detections based on this rcs tolerance
       float32_t azimuth_tolerance;      // [rad] identification of internal reflections counts recurrence of detections based on this azimuth tolerance
       float32_t range_tolerance;        // [m/s] identification of internal reflections counts recurrence of detections based on this range tolerance

       float32_t max_abs_range_rate;     // [m/s] identification of internal reflections only treats detections below abs(range_rate)
       float32_t rcs_max;          // [dB/m^2] identification of internal reflections only treats detections below this rcs
       float32_t range_max;              // [m] identification of internal reflections only treats detections within this range

       uint16_t age_threshold;        // [tracker iterations] Mininmum number of tracker iterations before a buffer slot is evaluated if it is an internal reflection

       bool f_enable;             // [-] When set to true, the internal reflections functionality will be active for this sensor

       uint8_t padding[1];
   };

   struct ConstantProps_T
   {
       // Antenna sensitivity map
       float32_t ant_sens_SCS_azim[NUM_VTX_ANT_SENS_POLY];   // common azimuth angle knots for all antenna sensitivity maps of the sensor
       float32_t ant_sens_SCS_sq_rng_90[NUM_VTX_ANT_SENS_POLY]; // border of antenna sensitivity region for a given probability of detection
       float32_t ant_sens_SCS_sq_rng_50[NUM_VTX_ANT_SENS_POLY]; // border of antenna sensitivity region for a given probability of detection

       // Internal reflection calibrations
       F360_Internal_Reflections_Calib_T internal_reflections; // Contains enable/disable & tuning parameters for the internal reflections functionality

       float32_t interior_fov[F360_DET_NUM_LOOK_ID];
       float32_t left_fov_normal[F360_DET_NUM_LOOK_ID];
       float32_t right_fov_normal[F360_DET_NUM_LOOK_ID];

       // Mounting properties
       F360_Sensor_Mounting_Position_T mounting_position;

       // Detection properties
       float32_t range_limits[F360_NUM_LOOK_ID];
       float32_t fov_min_az_rad[F360_NUM_LOOK_ID];
       float32_t fov_max_az_rad[F360_NUM_LOOK_ID];
       float32_t fov_min_el_rad[F360_NUM_LOOK_ID];
       float32_t fov_max_el_rad[F360_NUM_LOOK_ID];
       float32_t min_aliaised_range_rate[F360_NUM_LOOK_ID];
       float32_t v_wrapping[F360_NUM_LOOK_ID]; // Range rate dealiasing interval
       float32_t r_wrapping[F360_NUM_LOOK_ID]; // Range offset after doppler unfolding; 0 for FMCW and nonzero for SFW

       uint32_t id; // Sensor index (given by the tracker wrapper)
       int32_t polarity; // 1 = normal, -1 = flipped

       // Version info
       uint32_t sensor_sw_version;

       // CDC data
       bool f_read_cdc_data;

       // Mounting properties
       F360_Mounting_Location_T mounting_location;

       // Sensor properties
       F360_Sensor_Type_T sensor_type;
       uint8_t padding[5];
   };

   struct VariableProps_T
   {
       uint64_t timestamp_us;

       // Motion info
       F360_VCS_Velocity_T vcs_velocity;
       float32_t vacs_boresight_az_estimated;
       float32_t vacs_boresight_el_estimated;

       uint32_t number_of_valid_detections;

       // Diag
       float32_t yaw_rate_calc_dps;
       float32_t vehicle_speed_calc_mps;
       
       float32_t time_since_measurement_s;

       // Index of first detection from this sensor in the full tracker detection list
       int32_t first_detection_list_idx;

       uint16_t look_index;

       bool is_valid;

       // Antenna sensitivity map
       bool f_ant_sens_available;
       bool f_ant_sens_degraded;

       F360_Det_Look_ID_T look_id;
       
       uint8_t padding[6];
   };

   typedef struct F360_Radar_Sensor_Tag
   {
      ConstantProps_T constant;
      VariableProps_T variable;
   } F360_Radar_Sensor_T;

   static_assert(528 == sizeof(F360_Radar_Sensor_T), "sizeof(F360_Radar_Sensor_T) not as expected. Remember to align padding if needed");
}

#endif
