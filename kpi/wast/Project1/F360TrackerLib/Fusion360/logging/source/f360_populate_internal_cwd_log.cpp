/*===================================================================================*\
* FILE:  f360_populate_internal_cwd_log.cpp
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*------------------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*/
#include "f360_populate_internal_cwd_log.h"

namespace f360_variant_A
{
   void Populate_Internal_CWD_Data(Static_Env_T& static_environment,
      const F360_Internal_CWD_T(&cwd_log)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations)
   {
      F360_Concrete_Wall_Detector_T cwd = static_environment.Get_CWD();
      cwd.Init(r_sensors, calibrations);
      cwd.Init_Long_Zones(calibrations.k_cwd_sensor_zone_half_length);

      for (uint32_t sens_idx = 0U; sens_idx < static_cast<uint32_t>(MAX_NUMBER_OF_SENSORS); ++sens_idx)
      {
         const bool f_measurement_valid = (1U == cwd_log[sens_idx].laterally_closest_measurement_is_valid);
         if (f_measurement_valid)
         {
            Concrete_Wall_Sensor_T cwd_sensor = cwd.Get_CWD_Sensor(sens_idx);
            cwd_sensor.Set_Laterally_Closest_Measurement(
               cwd_log[sens_idx].laterally_closest_measurement_lateral_position, f_measurement_valid);

            Concrete_Wall_Measurements_T det_vcs_lat_pos_buffer = cwd_sensor.Get_CW_Measurements();
            for (uint32_t sample_idx = 0U; sample_idx < 5U; ++sample_idx)
            {
               const bool f_sample_valid = (1U == cwd_log[sens_idx].measurement_is_valid[sample_idx]);
               if (f_sample_valid)
               {
                  Concrete_Wall_Sample_T sample = {};
                  sample.lateral_position = cwd_log[sens_idx].measurement_lateral_position[sample_idx];
                  sample.is_valid = f_sample_valid;
                  sample.padding[0] = 0U;
                  sample.padding[1] = 0U;
                  sample.padding[2] = 0U;
                  det_vcs_lat_pos_buffer.Push(sample);
               }
            }
            cwd_sensor.Set_CW_Measurements(det_vcs_lat_pos_buffer);
            cwd.Set_CWD_Sensor(sens_idx, cwd_sensor);
         }
      }
      static_environment.Set_CWD(cwd);
   }

   void Populate_Internal_CWD_Log_Data(F360_Internal_CWD_T(&cwd_log)[MAX_NUMBER_OF_SENSORS],
      const Static_Env_T& static_environment)
   {
      const uint32_t cwd_sensors_count = static_environment.Get_CWD().Get_CWD_Sensors_Count();
      for (uint32_t sens_idx = 0U; sens_idx < cwd_sensors_count; sens_idx++)
      {
         Concrete_Wall_Measurements_T::Concrete_Wall_Samples_T samples = static_environment.Get_CWD().Get_CWD_Sensor(sens_idx).Get_CW_Measurements().Get_Samples();
         for (uint32_t sample_idx = 0U; sample_idx < 5U; sample_idx++)
         {
            cwd_log[sens_idx].measurement_lateral_position[sample_idx] = samples[sample_idx].lateral_position;
            cwd_log[sens_idx].measurement_is_valid[sample_idx] = samples[sample_idx].is_valid ? 1U : 0U;
         }
         cwd_log[sens_idx].laterally_closest_measurement_lateral_position = static_environment.Get_CWD().Get_CWD_Sensor(sens_idx).Get_Laterally_Closest_Measurement().lateral_position;
         cwd_log[sens_idx].laterally_closest_measurement_is_valid = static_environment.Get_CWD().Get_CWD_Sensor(sens_idx).Get_Laterally_Closest_Measurement().is_valid ? 1U : 0U;
      }
   }
}
