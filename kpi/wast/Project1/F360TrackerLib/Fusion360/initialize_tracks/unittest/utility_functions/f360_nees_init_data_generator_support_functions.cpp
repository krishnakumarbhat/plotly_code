/*===================================================================================*\
* FILE: f360_nees_init_data_generator_support_functions.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains supporting functions for nees initialization data generation
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_init_data_generator_support_functions.h"

#include "f360_initialization_data_generator_support_functions.h"
#include "f360_data_generator.h"
#include "f360_initialize_tracks.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   F360_NEES_CFMI_Detection_T Init_NEES_CFMI_Detection(
      Point posn,
      F360_VCS_Velocity_T vel,
      float32_t time_since_meas,
      float32_t timestamp_diff,
      float32_t range_rate_comp_var,
      float32_t f_historical)
   {
      F360_NEES_CFMI_Detection_T detection = {};
      detection.f_valid = true;
      detection.f_inlier = true;
      detection.idx.f_historical = f_historical;
      detection.idx.idx = 0;
      detection.time_since_meas = time_since_meas;
      detection.timestamp_diff = timestamp_diff;
      detection.pos.y = posn.y;
      detection.pos.x = posn.x;
      detection.range_rate_comp = Get_RangeRate_From_Cart_Coordinates(posn.y, posn.x, vel.lateral, vel.longitudinal);
      detection.range_rate_comp_var = range_rate_comp_var;

      const float32_t azimuth = Get_Azimuth_From_Cart_Pos(posn.y, posn.x);
      detection.sin_vcs_az = F360_Sinf(azimuth);
      detection.cos_vcs_az = F360_Cosf(azimuth);
      Calc_NEES_CFMI_Determinants_Cloud_UT(detection);

      return detection;
   }

   F360_NEES_CFMI_Pos_Diff_Velocity_T Init_NEES_CFMI_Pos_Diff_Velocity(
      F360_VCS_Velocity_T vel,
      float32_t vel_lat_var,
      float32_t vel_long_var,
      float32_t vel_long_lat_cov,
      F360_NEES_CFMI_Vel_RR_Conf_T rr_confidence_level,
      float32_t rr_plausbility,
      float32_t time_since_meas,
      float32_t dt)
   {
      F360_NEES_CFMI_Pos_Diff_Velocity_T velocity = {};
      velocity.f_valid = true;
      velocity.f_inlier = true;
      velocity.f_rr_confirmed = (rr_confidence_level != F360_NEES_CFMI_VEL_RR_CONF_INVALID);
      // Det index kept zero since not used.
      velocity.time_since_meas = time_since_meas;
      velocity.vel.lateral = vel.lateral;
      velocity.vel.longitudinal = vel.longitudinal;
      velocity.vel_cov[0][0] = vel_long_var;
      velocity.vel_cov[1][1] = vel_lat_var;
      velocity.vel_cov[0][1] = vel_long_lat_cov;
      velocity.vel_cov[1][0] = vel_long_lat_cov;
      velocity.rr_confidence_level = rr_confidence_level;
      velocity.dt = dt;
      velocity.cov_trace = vel_long_var + vel_lat_var;
      velocity.rr_plausbility = rr_plausbility;
      velocity.determinant = (vel_long_var * vel_lat_var) - (vel_long_lat_cov * vel_long_lat_cov);
      Calc_NEES_CFMI_Determinants_Pos_Diff_UT(velocity);

      return velocity;
   }

   void Add_Detection_To_Nees_Information(F360_NEES_CFMI_Information_T& nees_infromation,
      F360_NEES_CFMI_Detection_T detection)
   {
      nees_infromation.detections[nees_infromation.dets_num] = detection;
      nees_infromation.dets_num++;
      nees_infromation.init_dets_inliers_num++;
   }

   void Add_Pos_Diff_Velocity_To_Nees_Information(F360_NEES_CFMI_Information_T& nees_infromation,
      F360_NEES_CFMI_Pos_Diff_Velocity_T velocity,
      bool f_expected_as_inliers)
   {
      nees_infromation.velocities[nees_infromation.vels_num] = velocity;
      nees_infromation.vels_num++;
      if (f_expected_as_inliers)
      {
         nees_infromation.expected_vels_inliers_num++;
      }
      if (velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_LOW)
      {
         nees_infromation.num_of_confirm_pos_diff_hyp_low++;
      }
      else if (velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_MEDIUM)
      {
         nees_infromation.num_of_confirm_pos_diff_hyp_med++;
      }
      else if (velocity.rr_confidence_level == F360_NEES_CFMI_VEL_RR_CONF_HIGH)
      {
         nees_infromation.num_of_confirm_pos_diff_hyp_high++;
      }
   }

   void Add_Center_Info_NEES_CFMI_Information(F360_NEES_CFMI_Information_T& nees_information,
      Point posn,
      float32_t posn_lat_var,
      float32_t posn_long_var,
      float32_t posn_long_lat_cov,
      float32_t center_time_since_meas)
   {
      nees_information.f_center_valid = true;
      nees_information.pos_center.y = posn.y;
      nees_information.pos_center.x = posn.x;
      nees_information.pos_center_cov[0][0] = posn_lat_var;
      nees_information.pos_center_cov[1][1] = posn_long_var;
      nees_information.pos_center_cov[1][0] = posn_long_lat_cov;
      nees_information.pos_center_cov[0][1] = posn_long_lat_cov;
      nees_information.center_time_since_meas = center_time_since_meas;
   }

   void Calc_NEES_CFMI_Determinants_Cloud_UT(F360_NEES_CFMI_Detection_T& single_det)
   {
      if (single_det.range_rate_comp_var > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_range_rate_comp_var = 1.0F / single_det.range_rate_comp_var;

         single_det.cloud_determinants.n_dets = 1U;
         single_det.cloud_determinants.Sxx = single_det.cos_vcs_az * single_det.cos_vcs_az * one_over_range_rate_comp_var;
         single_det.cloud_determinants.Sxy = single_det.cos_vcs_az * single_det.sin_vcs_az * one_over_range_rate_comp_var;
         single_det.cloud_determinants.Sx = single_det.cos_vcs_az * single_det.range_rate_comp * one_over_range_rate_comp_var;
         single_det.cloud_determinants.Syy = single_det.sin_vcs_az * single_det.sin_vcs_az * one_over_range_rate_comp_var;
         single_det.cloud_determinants.Sy = single_det.sin_vcs_az * single_det.range_rate_comp * one_over_range_rate_comp_var;

         single_det.cloud_determinants.Syx = single_det.cloud_determinants.Sxy;

         single_det.cloud_determinants.information = 1.0F / single_det.range_rate_comp_var;
      }
   }

   void Calc_NEES_CFMI_Determinants_Pos_Diff_UT(F360_NEES_CFMI_Pos_Diff_Velocity_T& single_vel)
   {
      if (single_vel.determinant > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_determinant = 1.0F / single_vel.determinant;

         single_vel.pos_diff_determinants.n_dets = 1U;
         single_vel.pos_diff_determinants.Sxx = single_vel.vel_cov[F360_2D_COV_IDX_Y][F360_2D_COV_IDX_Y] * one_over_determinant;
         single_vel.pos_diff_determinants.Sxy = -(single_vel.vel_cov[F360_2D_COV_IDX_X][F360_2D_COV_IDX_Y] * one_over_determinant);
         single_vel.pos_diff_determinants.Sx = ((single_vel.vel.longitudinal * single_vel.vel_cov[F360_2D_COV_IDX_Y][F360_2D_COV_IDX_Y]) -
            (single_vel.vel.lateral * single_vel.vel_cov[F360_2D_COV_IDX_X][F360_2D_COV_IDX_Y])) * one_over_determinant;
         single_vel.pos_diff_determinants.Syy = single_vel.vel_cov[F360_2D_COV_IDX_X][F360_2D_COV_IDX_X] * one_over_determinant;
         single_vel.pos_diff_determinants.Sy = ((single_vel.vel.lateral * single_vel.vel_cov[F360_2D_COV_IDX_X][F360_2D_COV_IDX_X]) -
            (single_vel.vel.longitudinal * single_vel.vel_cov[F360_2D_COV_IDX_X][F360_2D_COV_IDX_Y]))  * one_over_determinant;

         single_vel.pos_diff_determinants.Syx = single_vel.pos_diff_determinants.Sxy;

         single_vel.pos_diff_determinants.information = single_vel.cov_trace * one_over_determinant; // Quick Fisher information calculation
      }
   }
}
