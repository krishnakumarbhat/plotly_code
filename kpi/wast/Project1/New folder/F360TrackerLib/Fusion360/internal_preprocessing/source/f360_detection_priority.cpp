/*===================================================================================\
 * FILE: f360_detection_priority.cpp
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains function definition for detection_priority.
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

#include "f360_math.h"
#include "f360_detection_priority.h"

namespace f360_variant_A
{
   static float32_t Polar_Position_Priority(const float32_t range, const float32_t vcs_azimuth);
   static float32_t Motion_Priority(const float32_t range_rate, const float32_t vcs_azimuth, 
      const float32_t host_speed, const float32_t min_range_rate, const float32_t vun);

   float32_t Detection_Priority(const float32_t range, const float32_t vcs_azimuth, const float32_t range_rate,
      const float32_t host_speed, const float32_t min_range_rate, const float32_t vun)
   {
      // Most important detections: 2
      // Least important detections: 0
      const float32_t position_prio = Polar_Position_Priority(range, vcs_azimuth);
      const float32_t motion_prio = Motion_Priority(range_rate, vcs_azimuth, host_speed, min_range_rate, vun);

      return position_prio + motion_prio;
   }

   static float32_t Polar_Position_Priority(const float32_t range, const float32_t vcs_azimuth)
   {
      const float32_t k_az_scale = 1.2F;
      const float32_t k_range_scale = 0.0001F;

      const float32_t az_component = 1.0F - (k_az_scale * vcs_azimuth * vcs_azimuth); // may be negative
      const float32_t range_component = fmaxf(0.0F, 1.0F - (k_range_scale * range * range)); // must be positive

      const float32_t priority = fmaxf(az_component, range_component); // limited to range [0, 1]
      return priority;
   }

   static float32_t Motion_Priority(const float32_t range_rate, const  float32_t vcs_azimuth, 
      const float32_t host_speed, const float32_t min_range_rate, const float32_t vun)
   {
      const float32_t k_motion_threshold_inv = 0.2F;

      const float32_t predicted_stationary_range_rate = -host_speed * cosf(vcs_azimuth);
      const float32_t temp = fmodf(predicted_stationary_range_rate - min_range_rate, vun);
      float32_t predicted_stationary_range_rate_aliased;
      if (temp < 0.0F)
      {
         // fmodf() does not behave like MATLAB mod, so need to add vun if result is negative
         predicted_stationary_range_rate_aliased = temp + vun + min_range_rate;
      }
      else
      {
         predicted_stationary_range_rate_aliased = temp + min_range_rate;
      }
      
      const float32_t priority = fminf(1.0F, fabsf(predicted_stationary_range_rate_aliased - range_rate) * k_motion_threshold_inv); // limited to range [0, 1]
      return priority;
   }
}
