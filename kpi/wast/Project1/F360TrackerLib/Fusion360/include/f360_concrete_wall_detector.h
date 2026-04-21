/*=========================================================================
*  FILE: f360_concrete_wall_detector.h
*=========================================================================
* Copyright (C) 2021 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------
*
*  DESCRIPTION:
*    This file contains F360_Concrete_Wall_Detector_T class declaration
*
*========================================================================*/
#ifndef F360_CONCRETE_WALL_DETECTOR_H
#define F360_CONCRETE_WALL_DETECTOR_H

#include "f360_concrete_wall_sensor.h"
#include "f360_concrete_wall_side.h"
#include "f360_radar_sensor_props.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_calibrations.h"
#include "f360_timing_info.h"

namespace f360_variant_A
{
   class F360_Concrete_Wall_Detector_T
   {
   public:
      F360_Concrete_Wall_Detector_T();
      ~F360_Concrete_Wall_Detector_T();
      bool Is_Initialized() const;
      void Run_Measurements(
         const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
         const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
         const F360_Calibrations_T &calib,
         const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
         const float32_t abs_host_speed,
         F360_TRKR_TIMING_INFO_T& timing_info);

      void Map_Data_To_Static_Env_Poly(Static_Env_Poly_T(&stat_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS], const float32_t host_curvature_rear) const;
      void Init(const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS], const F360_Calibrations_T &calib);
      void Init_Long_Zones(const float32_t sensor_zone_half_length);

   private:
      struct Long_Zone_T
      {
         float32_t x_min;
         float32_t x_max;
         uint32_t sensor_idxs[MAX_NUMBER_OF_SENSORS];   // indices in concrete_wall_sensors[] array.
         uint32_t num_sensors_in_zone;
      };

      void Reset_All_CWD_Sensors(const F360_Calibrations_T& calib);
      void Set_Zone_Beginning(const uint32_t sens_idx, const uint32_t zone_idx, const float32_t sensor_zone_half_length);
      void Set_Zone_Ending(const uint32_t sens_idx, const uint32_t zone_idx, const float32_t sensor_zone_half_length);
      void Add_Sensor_Idx_To_Zone_Definition(const uint32_t sens_idx, const uint32_t zone_idx);
      void Set_Extreme_Dets_Idxs_For_Each_Sensor_Zone(
         const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
         const F360_Calibrations_T &calib);
      
      bool Are_Sensors_Zones_Overlapping(const uint32_t curr_sens_idx, const uint32_t next_sens_idx, const float32_t sensor_zone_half_width) const;
      Concrete_Wall_Sensor_T cwd_sensors[MAX_NUMBER_OF_SENSORS];   // CWD data for each sensor
      Concrete_Wall_Side_T cw_left;
      Concrete_Wall_Side_T cw_right;
      Long_Zone_T long_zones[MAX_NUMBER_OF_SENSORS];
      uint32_t cwd_sensors_count;
      uint32_t long_sorted_sensors_idxs[MAX_NUMBER_OF_SENSORS];
      uint32_t cwd_long_zones_count;
      bool f_initialized;               //Flag indicating that object was initialized.

   public:     // Below functions are for data logging purpose
      const Concrete_Wall_Side_T& Get_CWD_Left_Side() const { return this->cw_left; }
      const Concrete_Wall_Side_T& Get_CWD_Right_Side() const { return this->cw_right; }
      const Concrete_Wall_Sensor_T& Get_CWD_Sensor(const uint32_t sens_idx) const { return this->cwd_sensors[sens_idx]; }
      void Set_CWD_Sensor(const uint32_t sens_idx, const Concrete_Wall_Sensor_T& cwd_sensor_input) { this->cwd_sensors[sens_idx] = cwd_sensor_input; }
      uint32_t Get_CWD_Sensors_Count() const { return this->cwd_sensors_count; }
      void Set_CWD_Sensors_Count(const uint32_t cwd_sensors_count_input) { this->cwd_sensors_count = cwd_sensors_count_input; }
      uint32_t Get_CWD_Long_Zones_Count() const { return this->cwd_long_zones_count; }
      void Set_CWD_Long_Zones_Count(const uint32_t cwd_long_zones_count_input) { this->cwd_long_zones_count = cwd_long_zones_count_input; }
      bool Get_f_initialized() const { return this->f_initialized; }
      void Set_f_initialized(const bool f_initialized_input) { this->f_initialized = f_initialized_input; }

   };   
}
#endif
