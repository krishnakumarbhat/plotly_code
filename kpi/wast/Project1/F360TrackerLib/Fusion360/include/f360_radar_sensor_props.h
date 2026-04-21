/*===================================================================================*\
* FILE: f360_radar_sensor_props.h
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
*   This file contains F360_Sensor_Props_T structure declaration
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
#ifndef F360_RADAR_SENSOR_PROPS_H
#define F360_RADAR_SENSOR_PROPS_H

#include "f360_reuse.h"
#include "f360_velocity.h"
#include "f360_internal_reflection_buffer_slot.h"
#include "f360_constants.h"
#include "f360_look_ID.h"

namespace f360_variant_A
{
   typedef struct F360_Radar_Sensor_Props_Tag
   {
      // Time properties
      float32_t time_since_measurement_s;

      // Index of first detection from this sensor in the full tracker detection list
      int32_t first_detection_list_idx;

      float32_t interior_fov[F360_DET_NUM_LOOK_ID];
      float32_t left_fov_normal[F360_DET_NUM_LOOK_ID];
      float32_t right_fov_normal[F360_DET_NUM_LOOK_ID];

      Internal_Reflection_Buffer_Slot internal_reflections_buffer[INTERNAL_REFLECTIONS_BUFFER_SIZE]; // Buffer containing internal reflection patterns

      // GD info 
      int32_t next_to_sensor_object_track_id; // Object Track id of the vehicle next to the sensor (-1 for none)
      float32_t next_to_sensor_object_track_min_long_pos; // [m]
      float32_t next_to_sensor_object_track_max_long_pos; // [m]
      float32_t next_to_sensor_object_track_min_lat_pos;  // [m]
      float32_t next_to_sensor_object_track_max_lat_pos;  // [m]
      bool f_object_track_next_to_sensor; // Flag indicating there is a vehicle next to the sensor
      uint8_t padding[3];

   } F360_Radar_Sensor_Props_T;

   static_assert(
      (sizeof(F360_Radar_Sensor_Props_T::internal_reflections_buffer) + 80) == sizeof(F360_Radar_Sensor_Props_T), 
      "sizeof(F360_Radar_Sensor_Props_T) not as expected. Remember to align padding if needed");
}
#endif
