/*===================================================================================*\
* FILE: f360_nees_init_data_generator_support_functions.h
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

#ifndef F360_NEES_INIT_DATA_GENERATOR_SUPPORT_FUNCTIONS_H
#define F360_NEES_INIT_DATA_GENERATOR_SUPPORT_FUNCTIONS_H

#include "f360_initialize_tracks.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_point.h"

namespace f360_variant_A
{
   F360_NEES_CFMI_Detection_T Init_NEES_CFMI_Detection(
      Point pos,
      F360_VCS_Velocity_T vel,
      float32_t time_since_meas,
      float32_t timestamp_diff,
      float32_t range_rate_comp_var,
      float32_t f_historical);

   F360_NEES_CFMI_Pos_Diff_Velocity_T Init_NEES_CFMI_Pos_Diff_Velocity(
      F360_VCS_Velocity_T vel,
      float32_t vel_lat_var,
      float32_t vel_long_var,
      float32_t vel_long_lat_cov,
      F360_NEES_CFMI_Vel_RR_Conf_T rr_confidence_level,
      float32_t rr_plausbility,
      float32_t time_since_meas,
      float32_t dt);

   void Add_Detection_To_Nees_Information(F360_NEES_CFMI_Information_T& nees_infromation,
      F360_NEES_CFMI_Detection_T detection);

   void Add_Pos_Diff_Velocity_To_Nees_Information(F360_NEES_CFMI_Information_T& nees_infromation,
      F360_NEES_CFMI_Pos_Diff_Velocity_T velocity,
      bool expected_as_inliers);

   void Add_Center_Info_NEES_CFMI_Information(F360_NEES_CFMI_Information_T& nees_infromation,
      Point posn,
      float32_t posn_lat_var,
      float32_t posn_long_var,
      float32_t posn_long_lat_cov,
      float32_t center_time_since_meas);

   void Calc_NEES_CFMI_Determinants_Cloud_UT(F360_NEES_CFMI_Detection_T& single_det);

   void Calc_NEES_CFMI_Determinants_Pos_Diff_UT(F360_NEES_CFMI_Pos_Diff_Velocity_T& single_vel);
}
#endif
