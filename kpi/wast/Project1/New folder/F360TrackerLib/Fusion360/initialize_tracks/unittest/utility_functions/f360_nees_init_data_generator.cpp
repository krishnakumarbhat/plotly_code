/*===================================================================================*\
* FILE: f360_nees_init_data_generator.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains nees initialization internal functions data generation
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_init_data_generator.h"

#include "f360_nees_init_data_generator_support_functions.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   void Fill_Valid_NEES_CFMI_Information(
      F360_NEES_CFMI_Information_T& nees_cfmi_information, 
      const uint16_t num_of_detections, 
      const uint16_t num_of_velocities)
   {
      Point object_position = { 50.0F, 5.0F };
      F360_VCS_Velocity_T object_velocity = { 10.0F, 0.5F };

      const float32_t noise_gain = 0.1F;

      // Fill F360_NEES_CFMI_Information_T with detections corrupted with deterministic noise
      for (uint16_t i = 0U; i < num_of_detections; i++)
      {
         const float32_t noise_sign = (i % 2) ? -1.0F : 1.0F;

         const Point det_position = { 
            object_position.x + noise_sign * noise_gain * i,
            object_position.y + noise_sign * noise_gain * i};

         const F360_VCS_Velocity_T det_velocity = { object_velocity.lateral + noise_sign * noise_gain * i,
                                                    object_velocity.longitudinal + noise_sign * noise_gain * i };

         const float32_t time_since_meas = 0.05F * i;
         const float32_t timestamp_diff = 0.05F * i;
         const float32_t range_rate_comp_var = 0.1F;

         const bool f_historical = (i > 3U)? true : false;

         const F360_NEES_CFMI_Detection_T nees_detection = Init_NEES_CFMI_Detection(det_position, det_velocity, time_since_meas, timestamp_diff, range_rate_comp_var, f_historical);

         Add_Detection_To_Nees_Information(nees_cfmi_information, nees_detection);
      }

      // Fill F360_NEES_CFMI_Information_T with velocities corrupted with deterministic noise
      // This is decoupled from actual detection position differences to make testing more adaptable
      for (uint16_t i = 0U; i < num_of_velocities; i++)
      {
         const float32_t noise_sign = (i % 2) ? -1.0F : 1.0F;

         const F360_VCS_Velocity_T pos_diff_velocity = { object_velocity.longitudinal + noise_sign * noise_gain * i,
                                                         object_velocity.lateral + noise_sign * noise_gain * i};
         const float32_t vel_lat_var = 0.2F;
         const float32_t vel_long_var = 0.5F;
         const float32_t vel_long_lat_cov = 0.3F;
         const F360_NEES_CFMI_Vel_RR_Conf_T rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
         const float32_t rr_plausbility = 0.8F;
         const float32_t time_since_meas = 0.05F * i;
         const float32_t dt = 0.05F;

         const F360_NEES_CFMI_Pos_Diff_Velocity_T nees_pos_diff_velocity = Init_NEES_CFMI_Pos_Diff_Velocity(pos_diff_velocity, vel_lat_var, vel_long_var, vel_long_lat_cov,
                                                                                                            rr_confidence_level, rr_plausbility, time_since_meas, dt);

         const bool f_expected_as_inliers = (i < 4U) ? true : false;

         Add_Pos_Diff_Velocity_To_Nees_Information(nees_cfmi_information, nees_pos_diff_velocity, f_expected_as_inliers);
      }

      const float32_t posn_lat_var = 1.5F;
      const float32_t posn_long_var = 1.2F;
      const float32_t posn_long_lat_cov = 1.1F;
      const float32_t center_time_since_meas = 0.1F;

      Add_Center_Info_NEES_CFMI_Information(nees_cfmi_information, object_position, posn_lat_var, posn_long_var, posn_long_lat_cov, center_time_since_meas);
   }
}
