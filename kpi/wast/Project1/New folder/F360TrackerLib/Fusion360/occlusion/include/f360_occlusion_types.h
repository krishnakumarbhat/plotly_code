/*===================================================================================*\
* FILE:  f360_occlusion_types.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of types used by Occlusion Module.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef OCCLUSION_TYPES_H
#define OCCLUSION_TYPES_H

#include "f360_reuse.h"
#include "f360_point.h"

namespace f360_variant_A
{

   const uint32_t occlusion_num_sectors = 75U;

   /* Preconditions:
   * Current enum order HAS to be kept. If changes are to be made here - verify whether Determine_Occlusion_Status of
   * Occlusion_T class works as expected - it was optimized to work with this way.
   */
   enum Occlusion_Status_T : uint8_t
   {
      OCCLUSION_STATUS_UNDEFINED = 0,
      OCCLUSION_STATUS_OCCLUDED = 1,
      OCCLUSION_STATUS_ON_EDGE = 2,
      OCCLUSION_STATUS_VISIBLE = 3
   };

   // TODO: DFU-912
   struct SCS_Polar_Position_T
   {
      float32_t range;
      float32_t azimuth;
   };

   struct SCS_Cart_Position_T
   {
      float32_t x;
      float32_t y;
   };

   struct SCS_Position_T
   {
      SCS_Polar_Position_T polar;
      SCS_Cart_Position_T cart;
   };

   struct SCS_Edge_T
   {
      SCS_Position_T point_begin;
      SCS_Position_T point_end;
      bool f_valid;
   };

   struct SCS_Track_Visible_Edges_T
   {
      SCS_Edge_T first_edge;
      SCS_Edge_T second_edge;
   };

   struct VCS_Edge_T
   {
      Point point_begin;
      Point point_end;
      bool f_valid;
   };

   struct VCS_Track_Visible_Edges_T
   {
      VCS_Edge_T first_edge;
      VCS_Edge_T second_edge;
   };

   struct Sensor_FOV_Data_T
   {
      float32_t min_az;
      float32_t max_az;
      float32_t max_range;
   };

   struct Occlusion_Sectors_T
   {
      float32_t width;
      float32_t range[occlusion_num_sectors];
   };

   struct Sensor_Mounting_Position
   {
      Point vcs_position;
      float32_t cos_vcs_borseight_azimuth_angle;
      float32_t sin_vcs_borseight_azimuth_angle;
   };
}
#endif
