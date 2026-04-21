/*===========================================================================*\
* FILE: f360_calc_probabilities_of_detections.h
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains declarations of functions that calculate probability of detection.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/


#ifndef F360_CALC_PROBABILITIES_OF_DETECTIONS_H
#define F360_CALC_PROBABILITIES_OF_DETECTIONS_H

#include "f360_reuse.h"
#include "rspp_detection_list.h"
#include "f360_detection_props.h"
#include "f360_radar_sensor.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   void Compute_Detection_Probability(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib,
      F360_Detection_Props_T &detection_prop
   );

   void Lower_Detection_Probability(
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib,
      const F360_Detection_Props_T &detection_prop,
      float32_t &probability_of_detection
   );

   float32_t Determine_Probability_Of_Detection(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib
   );

   float32_t Determine_Detection_Probability_Antenna_Sensitivity_Map_Unavailable(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const float32_t det_sq_range,
      const float32_t fov_max_sq_range,
      const F360_Det_Look_ID_T look_id,
      const F360_Calibrations_T &calib
   );

   float32_t Determine_Detection_Probability_Antenna_Sensitivity_Map_Available(
      const float32_t det_sq_range,
      const float32_t fov_max_sq_range,
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib
   );

   float32_t Assign_Probability_Based_On_Antena_Sensivity_Map(
      const F360_Radar_Sensor_T &sensor,
      const uint32_t az_interval,
      const float32_t det_sq_range,
      const F360_Calibrations_T &calib
   );

   static inline void Lower_Detection_Probability_Degraded_Antenna_Sensitivity(
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib,
      float32_t &probability_of_detection)
   {
      if (sensor.variable.f_ant_sens_degraded)
      {
         probability_of_detection *= calib.k_detection_probability_degradation_factor;
      }
   }

   static inline void Lower_Detection_Probability_Various_Failures(
      const F360_Calibrations_T &calib,
      const F360_Detection_Props_T &detection_prop,
      float32_t &probability_of_detection)
   {
      if (detection_prop.f_double_bounce)
      {
         probability_of_detection = calib.k_probability_of_detection_init;
      }
   }
}
#endif
