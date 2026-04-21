/*=========================================================================
*  FILE: f360_concrete_wall_side.h
*=========================================================================
* Copyright (C) 2021 Aptiv. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------
*
*  DESCRIPTION:
*    This file contains Concrete_Wall_Side_T class declaration
*
*========================================================================*/
#ifndef F360_CONCRETE_WALL_SIDE_H
#define F360_CONCRETE_WALL_SIDE_H

#include "f360_concrete_wall_sensor.h"
#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_static_env_poly_types.h"

namespace f360_variant_A
{
   class Concrete_Wall_Side_T
   {
      enum CW_Side_Status_T : uint8_t
      {
         CW_NOT_VALID = 0,
         CW_VALID = 1
      };
      static const uint32_t cwd_max_num_sensors_per_side = 3U;

   public:
      Concrete_Wall_Side_T();
      void Fuse_CWD_Side_Data(const Concrete_Wall_Sensor_T(&concrete_wall_sensors)[MAX_NUMBER_OF_SENSORS], const float32_t abs_host_speed, const F360_Calibrations_T &calib);
      void Append_Sensor_Idx(const uint32_t sensor_idx);
      void Reset_Side_CWD();
      void Map_CWD_Side_Data_To_Static_Env_Poly(Static_Env_Poly_T &stat_env_poly, const float32_t host_curvature_rear) const;
      void Invalidate_Status() { status = CW_NOT_VALID; }

   private:
      float32_t long_validity_min;
      float32_t long_validity_max;
      float32_t fused_lat_posn_estimate;
      float32_t fused_lat_posn_confid;
      uint32_t sensors_count;
      uint32_t sensor_idxs[cwd_max_num_sensors_per_side];
      CW_Side_Status_T status;

   public:  // Below functions are for data logging purpose
      uint32_t Get_Sensors_Count() const { return sensors_count; }
      void Set_Sensors_Count(const uint32_t sensors_count_input) { this->sensors_count = sensors_count_input; }

      uint32_t Get_Sensor_Idx_From_Array(const uint32_t idx_in_array)const { return sensor_idxs[idx_in_array]; }
      void Set_Sensor_Idx_To_Array(const uint32_t sens_idx, const uint32_t idx_in_array) { this->sensor_idxs[idx_in_array] = sens_idx; }
   };
}
#endif
