/*===========================================================================*\
* FILE: f360_calc_probabilities_of_detections.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains definitions of functions that calculate probability of detection.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_calc_probabilities_of_detections.h"
#include "f360_find_interval_in_array.h"
#include "f360_constants.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Compute_Detection_Probability()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &detection
   * const F360_Radar_Sensor_T &sensor
   * const F360_Calibrations_T &calib
   * F360_Detection_Props_T &detection_prop
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
   * This function computes probability detection based on antenna sensitivity maps.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_Detection_Probability(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib,
      F360_Detection_Props_T &detection_prop)
   {
      const bool f_is_sensor_id_valid = (detection.raw.sensor_id > 0) && (detection.raw.sensor_id < static_cast<int32_t>(MAX_NUMBER_OF_SENSORS));

      if (f_is_sensor_id_valid)
      {
         float32_t probability_of_detection = Determine_Probability_Of_Detection(detection, sensor, calib);
         Lower_Detection_Probability(sensor, calib, detection_prop, probability_of_detection);
         detection_prop.probability_of_detection = probability_of_detection;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Lower_Detection_Probability()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T &sensor
   * const F360_Calibrations_T &calib
   * const F360_Detection_Props_T &detection_prop
   * float32_t &probability_of_detection
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function lowers detection probability based on various failures and degaraded antenna sensitivity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Lower_Detection_Probability(
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib,
      const F360_Detection_Props_T &detection_prop,
      float32_t &probability_of_detection)
   {
      Lower_Detection_Probability_Various_Failures(calib, detection_prop, probability_of_detection);
      Lower_Detection_Probability_Degraded_Antenna_Sensitivity(sensor, calib, probability_of_detection);
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Probability_Of_Detection()
   *===========================================================================
   * RETURN VALUE:
   * float32_t probability_of_detection 
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &detection
   * const F360_Radar_Sensor_T &sensor
   * const F360_Calibrations_T &calib
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function determines probability of a detection based on condition if antenna sensitivity map is available or not.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Determine_Probability_Of_Detection(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib)
   {
      float32_t probability_of_detection = 0.0F;
      const F360_Det_Look_ID_T look_id = sensor.variable.look_id;
      const float32_t det_sq_range = detection.raw.range * detection.raw.range;
      const float32_t fov_max_sq_range = sensor.constant.range_limits[look_id] * sensor.constant.range_limits[look_id];

      if (sensor.variable.f_ant_sens_available)
      {
         probability_of_detection = Determine_Detection_Probability_Antenna_Sensitivity_Map_Available(det_sq_range, fov_max_sq_range, detection, sensor, calib);
      }
      else
      {
         probability_of_detection = Determine_Detection_Probability_Antenna_Sensitivity_Map_Unavailable(detection, sensor, det_sq_range, fov_max_sq_range, look_id, calib);
      }

      return probability_of_detection;
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Detection_Probability_Antenna_Sensitivity_Map_Unavailable()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - probability of detection
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &detection
   * const F360_Radar_Sensor_T &sensor
   * const float32_t det_sq_range
   * const float32_t fov_max_sq_range
   * const F360_Det_Look_ID_T look_id
   * const F360_Calibrations_T &calib
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function determines probability of a detection when antenna sensitivity map is not available.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Determine_Detection_Probability_Antenna_Sensitivity_Map_Unavailable(
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const float32_t det_sq_range,
      const float32_t fov_max_sq_range,
      const F360_Det_Look_ID_T look_id,
      const F360_Calibrations_T &calib)
   {
      const bool f_beyond_sensitivity_map_area = (det_sq_range < fov_max_sq_range)
         && (sensor.constant.fov_min_az_rad[look_id] < detection.raw.azimuth)
         && (detection.raw.azimuth < sensor.constant.fov_max_az_rad[look_id]);

      return (f_beyond_sensitivity_map_area ? calib.k_probability_of_detection_default: calib.k_probability_of_detection_init);

   }

   /*===========================================================================*\
   * FUNCTION: Determine_Detection_Probability_Antenna_Sensitivity_Map_Available()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - probability of detection
   *
   * PARAMETERS:
   * const float32_t det_sq_range
   * const float32_t fov_max_sq_range
   * const rspp_variant_A::RSPP_Detection_T &detection
   * const F360_Radar_Sensor_T &sensor
   * const F360_Calibrations_T &calib
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function determines probability of a detection when antenna sensitivity map is available.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Determine_Detection_Probability_Antenna_Sensitivity_Map_Available(
      const float32_t det_sq_range,
      const float32_t fov_max_sq_range,
      const rspp_variant_A::RSPP_Detection_T &detection,
      const F360_Radar_Sensor_T &sensor,
      const F360_Calibrations_T &calib)
   {
      const bool f_detection_range_within_FOV_max_range = det_sq_range < fov_max_sq_range;
      float32_t probability_of_detection = 0.0F;

      if (f_detection_range_within_FOV_max_range)
      { 
         const uint32_t az_interval = Find_Interval_In_Array(detection.raw.azimuth, sensor.constant.ant_sens_SCS_azim);
         const bool f_valid_azimuth_interval_found = (0U < az_interval) && (az_interval < static_cast<uint32_t>(NUM_VTX_ANT_SENS_POLY));

         if (f_valid_azimuth_interval_found)
         {
            probability_of_detection = Assign_Probability_Based_On_Antena_Sensivity_Map(sensor, az_interval, det_sq_range, calib);
         }
         else // detection azimuth beyond sensitivity map azimuth interval
         {
            probability_of_detection = calib.k_probability_of_detection_init;
         }
      }
      else // detection range beyond sensor max range
      {
         probability_of_detection = calib.k_probability_of_detection_init;
      }

      return probability_of_detection;
   }

   /*===========================================================================*\
   * FUNCTION: Assign_Probability_Based_On_Antena_Sensivity_Map()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - probability of detection
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T &sensor
   * const uint32_t az_interval
   * const float32_t det_sq_range
   * const F360_Calibrations_T &calib
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function determines probability of a detection based on detection range and antenna sensitivity map regions.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Assign_Probability_Based_On_Antena_Sensivity_Map(
      const F360_Radar_Sensor_T &sensor,
      const uint32_t az_interval,
      const float32_t det_sq_range,
      const F360_Calibrations_T &calib)
   {
      float32_t probability_of_detection = 0.0F;
      const float32_t sq_rng_A = sensor.constant.ant_sens_SCS_sq_rng_90[az_interval - 1U];
      const float32_t sq_rng_B = sensor.constant.ant_sens_SCS_sq_rng_90[az_interval];

      const bool f_detection_range_inside_high_probability_region = (det_sq_range < sq_rng_A) && (det_sq_range < sq_rng_B);
      const bool f_detection_range_inside_medium_probability_region = (det_sq_range < sq_rng_A) || (det_sq_range < sq_rng_B);

      if (f_detection_range_inside_high_probability_region)
      {
         probability_of_detection = calib.k_probability_of_detection_high;
      }
      else if (f_detection_range_inside_medium_probability_region)
      {
         probability_of_detection = calib.k_probability_of_detection_medium;
      }
      else
      {
         probability_of_detection = calib.k_probability_of_detection_low;
      }
      
      return probability_of_detection;
   }

}

