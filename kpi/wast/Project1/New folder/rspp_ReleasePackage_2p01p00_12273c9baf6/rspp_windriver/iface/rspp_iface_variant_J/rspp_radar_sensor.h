#ifndef RSPP_RADAR_SENSOR_VARIANT_J_H
#define RSPP_RADAR_SENSOR_VARIANT_J_H
/*===================================================================================*\
* FILE: rspp_radar_sensor.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_velocity.h"
#include "rspp_range_type.h"
#include "rspp_look_type.h"
#include "rspp_reuse.h"
#include "rspp_sensor_mounting_position.h"
#include "rspp_mounting_location.h"
#include "rspp_sensor_type.h"
#include "rspp_look_ID.h"

namespace rspp_variant_J
{
   // Function for mapping Look ID to Look Type (Added since Look Type was removed from the interface)
   inline RSPP_Det_Look_Type_T Get_Look_Type(const RSPP_Det_Look_ID_T look_ID)
   {
      RSPP_Det_Look_Type_T look_type;
      switch (look_ID)
      {
      case RSPP_DET_LOOK_ID_0:
      {
         look_type = RSPP_DET_LOOK_TYPE_LONG;
         break;
      }
      case RSPP_DET_LOOK_ID_1:
      {
         look_type = RSPP_DET_LOOK_TYPE_MEDIUM;
         break;
      }
      case RSPP_DET_LOOK_ID_2:
      {
         look_type = RSPP_DET_LOOK_TYPE_LONG;
         break;
      }
      case RSPP_DET_LOOK_ID_3:
      {
         look_type = RSPP_DET_LOOK_TYPE_MEDIUM;
         break;
      }
      default:
      {
         look_type = RSPP_DET_LOOK_TYPE_INVALID;
         break;
      }
      }

      return look_type;
   }

   // Function for mapping Look ID to Range Type (Added since Range Type was removed from the interface)
   inline RSPP_Det_Range_Type_T Get_Range_Type(const RSPP_Det_Look_ID_T look_ID)
   {
      RSPP_Det_Range_Type_T range_type;
      switch (look_ID)
      {
      case RSPP_DET_LOOK_ID_0:
      {
         range_type = RSPP_DET_RANGE_TYPE_LONG;
         break;
      }
      case RSPP_DET_LOOK_ID_1:
      {
         range_type = RSPP_DET_RANGE_TYPE_LONG;
         break;
      }
      case RSPP_DET_LOOK_ID_2:
      {
         range_type = RSPP_DET_RANGE_TYPE_MEDIUM;
         break;
      }
      case RSPP_DET_LOOK_ID_3:
      {
         range_type = RSPP_DET_RANGE_TYPE_MEDIUM;
         break;
      }
      default:
      {
         range_type = RSPP_DET_RANGE_TYPE_INVALID;
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
      float32_t ant_sens_SCS_azim[18];   // common azimuth angle knots for all antenna sensitivity maps of the sensor
      float32_t ant_sens_SCS_sq_rng_90[18]; // border of antenna sensitivity region for a given probability of detection
      float32_t ant_sens_SCS_sq_rng_50[18]; // border of antenna sensitivity region for a given probability of detection

      // Internal reflection calibrations
      F360_Internal_Reflections_Calib_T internal_reflections; // Contains enable/disable & tuning parameters for the internal reflections functionality

      float32_t interior_fov[RSPP_DET_NUM_LOOK_ID];
      float32_t left_fov_normal[RSPP_DET_NUM_LOOK_ID];
      float32_t right_fov_normal[RSPP_DET_NUM_LOOK_ID];

      // Mounting properties
      RSPP_Sensor_Mounting_Position_T mounting_position;

      // Detection properties
      float32_t range_limits[RSPP_DET_NUM_LOOK_ID];
      float32_t fov_min_az_rad[RSPP_DET_NUM_LOOK_ID];
      float32_t fov_max_az_rad[RSPP_DET_NUM_LOOK_ID];
      float32_t fov_min_el_rad[RSPP_DET_NUM_LOOK_ID];
      float32_t fov_max_el_rad[RSPP_DET_NUM_LOOK_ID];
      float32_t min_aliaised_range_rate[RSPP_DET_NUM_LOOK_ID];
      float32_t v_wrapping[RSPP_DET_NUM_LOOK_ID]; // Range rate dealiasing interval
      float32_t r_wrapping[RSPP_DET_NUM_LOOK_ID]; // Range offset after doppler unfolding; 0 for FMCW and nonzero for SFW

      uint32_t id; // Sensor index (given by the tracker wrapper)

      int32_t polarity; // 1 = normal, -1 = flipped

      // Version info
      uint32_t sensor_sw_version;

      // CDC data
      bool f_read_cdc_data;

      // Mounting properties
      RSPP_Mounting_Location_T mounting_location;

      // Sensor properties
      RSPP_Sensor_Type_T sensor_type;
      uint8_t padding[5];
   };

   struct VariableProps_T
   {
      uint64_t timestamp_us;
      
      // Motion info
      RSPP_VCS_Velocity_T vcs_velocity;
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

      RSPP_Det_Look_ID_T look_id;

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
