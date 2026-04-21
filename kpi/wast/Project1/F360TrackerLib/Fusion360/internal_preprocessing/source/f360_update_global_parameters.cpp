/*===================================================================================*\
 * FILE: f360_update_global_parameters.cpp
 *====================================================================================
* Copyright (C) 2019-2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 * This file contains functions descriptions for
 * Update_Global_Parameters()
 * Calculate_Shrinked_FOV_Normals()
 * Check_Sensor_Configurations()
 * 
 * ABBREVIATIONS:
 *
 * Applicable Standards (in order of precedence: highest first):
 * ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [06-Sep-2020]
 * ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
 \*====================================================================================*/

#include "f360_reuse.h"
#include "f360_update_global_parameters.h"
#include "f360_calc_obj_mov_stat_thresh.h"
#include "f360_constants.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_Global_Parameters()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T& host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Calibrations_T &calibrations
   * F360_Globals_T& globals
   * F360_TRKR_TIMING_INFO_T& timing_info
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function updates global parameters
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Update_Global_Parameters(
      const F360_Host_T& host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      F360_Globals_T& globals,
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
       Check_Sensor_Configuration(sensors, globals);

       Calculate_Shrinked_FOV_Normals(sensors, calibrations, globals);

       globals.obj_mov_stat_spd_thresh = Calc_Obj_Mov_Stat_Thresh(host.vcs_speed, &timing_info);
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Shrinked_FOV_Normals()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Calibrations_T &calibrations
   * F360_Globals_T& globals
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates the rotated FOV normals
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Calculate_Shrinked_FOV_Normals(
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      F360_Globals_T& globals)
   {
       for (uint32_t current_sensor_idx = 0U; current_sensor_idx < MAX_NUMBER_OF_SENSORS; current_sensor_idx++)
       {
           const F360_Radar_Sensor_T& current_sensor = sensors[current_sensor_idx];

           if (current_sensor.variable.is_valid)
           {
               // Currently, all sensor normals are rotated by the same angle
               // So, if different rotation angle is needed for another sensors, that condition should be added here

              float32_t left_fov_normal_vector[2]; // index 0 is the x-component, index 1 is the y-component
              float32_t right_fov_normal_vector[2];

              if (F360_DET_RANGE_TYPE_LONG == Get_Range_Type(current_sensor.variable.look_id))
              {
                 left_fov_normal_vector[0] = current_sensor.constant.left_fov_normal[F360_LOOK_ID_0];
                 left_fov_normal_vector[1] = current_sensor.constant.left_fov_normal[F360_LOOK_ID_1];

                 right_fov_normal_vector[0] = current_sensor.constant.right_fov_normal[F360_LOOK_ID_0];
                 right_fov_normal_vector[1] = current_sensor.constant.right_fov_normal[F360_LOOK_ID_1];
              }
              else
              {
                 left_fov_normal_vector[0] = current_sensor.constant.left_fov_normal[F360_LOOK_ID_2];
                 left_fov_normal_vector[1] = current_sensor.constant.left_fov_normal[F360_LOOK_ID_3];

                 right_fov_normal_vector[0] = current_sensor.constant.right_fov_normal[F360_LOOK_ID_2];
                 right_fov_normal_vector[1] = current_sensor.constant.right_fov_normal[F360_LOOK_ID_3];
              }

               F360_Rotate_2D_Vector(
                   left_fov_normal_vector[0],
                   left_fov_normal_vector[1],
                   F360_Cosf(calibrations.k_fov_normal_rotation_angle),
                   F360_Sinf(calibrations.k_fov_normal_rotation_angle),
                   globals.rotated_left_fov_normal[current_sensor_idx][0],
                   globals.rotated_left_fov_normal[current_sensor_idx][1]);

               F360_Rotate_2D_Vector(
                   right_fov_normal_vector[0],
                   right_fov_normal_vector[1],
                   F360_Cosf(-calibrations.k_fov_normal_rotation_angle),
                   F360_Sinf(-calibrations.k_fov_normal_rotation_angle),
                   globals.rotated_right_fov_normal[current_sensor_idx][0],
                   globals.rotated_right_fov_normal[current_sensor_idx][1]);
           }
       }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Sensor_Configuration()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]
   * F360_Globals_T& globals
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Updates flags inidicating radar sensor configurations
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Check_Sensor_Configuration(
       const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
       F360_Globals_T& globals)
   {
       uint8_t num_active_sensors = 0U;
       F360_Mounting_Location_T sensor_mounting_loc = F360_MOUNTING_LOCATION_UNKNOWN;

       for (const F360_Radar_Sensor_T& current_sensor : sensors)
       {
           if (current_sensor.variable.is_valid)
           {
               sensor_mounting_loc = current_sensor.constant.mounting_location;
               num_active_sensors++;
           }
       }

       // Flag that indicates if only the front center radar configuration is active
       globals.f_single_front_center_radar_only = ((1U == num_active_sensors)
           && (F360_MOUNTING_LOCATION_CENTER_FORWARD == sensor_mounting_loc));
   }

}

