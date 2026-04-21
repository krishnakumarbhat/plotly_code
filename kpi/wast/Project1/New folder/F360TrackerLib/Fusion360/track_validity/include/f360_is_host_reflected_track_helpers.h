/*===================================================================================*\
* FILE: f360_is_host_reflected_track_helpers.h
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of supporting functions used in Is_Host_Reflected_Track().
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#ifndef F360_IS_HOST_MIRROR_TRACK_HELPERS_H
#define F360_IS_HOST_MIRROR_TRACK_HELPERS_H

#include "f360_object_track.h"
#include "f360_calibrations.h"
#include "f360_point.h"
#include "f360_reuse.h"
#include "f360_static_env_poly_types.h"

namespace f360_variant_A
{
   bool Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox(
      const F360_Object_Track_T& object,
      const Point& host_mirror_track_tcs_pos,
      const F360_Calibrations_T& calib);

   Point Calc_Predicted_Reflected_Track_TCS_Position(
      const F360_Object_Track_T& object,
      const float32_t sep_lat_pos,
      const float32_t half_of_host_len);

   void Determine_Heading_And_Speed_Threshold(
      const float32_t host_speed,
      const F360_Calibrations_T& calib,
      float32_t& max_heading,
      float32_t& max_speed_diff);

   bool Is_Object_Suspected_Of_Being_Host_Reflection(
      const F360_Object_Track_T& object,
      const float32_t& host_speed,
      const F360_Calibrations_T& calib);

   bool Is_SEP_Valid_For_Host_Mirror_Ghost(
      const Static_Env_Poly_T& sep,
      const float32_t host_half_length);
}
#endif
