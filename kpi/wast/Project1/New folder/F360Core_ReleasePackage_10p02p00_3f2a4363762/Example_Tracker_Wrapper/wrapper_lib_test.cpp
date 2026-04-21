/*===================================================================================*\
Disclaimer:
This file is intended as an example showing how to integrate the tracker, it is not to be used as is for production.
All values used/assigned below are example values from previous programs, they are not to be reused.
\*===================================================================================*/


/*===================================================================================*\
 * FILE: Example_Wrapper_F360Tracker.cpp
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains definitions of F360 ISO Object List Output Adaptation functionalities
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

// Inbuilt header inclusion

#include <cmath>
#include <cstring>

namespace f360_variant_A {}
using namespace f360_variant_A;
#include "Example_Wrapper_F360Tracker.h"
#include "tracker_IAL.h"
#include "Tracker_OAL.h"
#include "f360_reuse.h"
#include "f360_look_ID.h"


// Tracker Input and Output Data
static Tracker_IAL_T g_tracker_IAL = {};
extern Tracker_OAL_T g_tracker_OAL;

int main()
{
   //set up simplified CAF parameter
   CAF_Param_T caf_param = { };
   caf_param.host.dist_rear_axle_to_vcs_m = 3.9F;
   caf_param.sensor[0].Sensor_polarity = 1;
   caf_param.sensor[0].Sensor_offset_long = -4.5F;
   caf_param.sensor[0].Sensor_offset_az_angle = 1.57F;

   Initialize_Tracker(&caf_param);
   const uint64_t step_time_us = 50000U;
  
   uint32_t cycle_of_execution = 0;
   while (cycle_of_execution < 10U)
   {
      //setup simple scenario input data
      g_tracker_IAL.tracker_timestamp_us += step_time_us;

      g_tracker_IAL.vehicle_info.speed = 10.0F;
      g_tracker_IAL.vehicle_info.vcs_speed = 10.0F;
      g_tracker_IAL.vehicle_info.speed_correction_factor = 1.0F;

      g_tracker_IAL.srr_sensor_info[0].det_data[0].azimuth = 0.0F;
      g_tracker_IAL.srr_sensor_info[0].det_data[0].det_id = 1;
      g_tracker_IAL.srr_sensor_info[0].det_data[0].range_rate = 5.0F;
      g_tracker_IAL.srr_sensor_info[0].det_data[0].range = 0.0F;

      g_tracker_IAL.srr_sensor_info[0].sens_data.number_of_valid_detections = 1U;
      g_tracker_IAL.srr_sensor_info[0].sens_data.look_index += 1U;
      g_tracker_IAL.srr_sensor_info[0].sens_data.vacs_boresight_az_estimated = 1.58F;
      uint8_t& look_id = g_tracker_IAL.srr_sensor_info[0].sens_data.look_id;
      look_id = (look_id == F360_DET_LOOK_ID_0) ? F360_DET_LOOK_ID_1 : F360_DET_LOOK_ID_0;
      g_tracker_IAL.srr_sensor_info[0].sens_data.timestamp_us += step_time_us;

      Run_Tracker_Wrapper(&g_tracker_IAL);
      cycle_of_execution++;
   }

   int test_result = 0;
   if (g_tracker_OAL.tracker_info.reduced_num_active_objs > 0)
   {
      test_result = 0;
   }
   else
   {
      test_result = -1;
   }
   return test_result;

}
