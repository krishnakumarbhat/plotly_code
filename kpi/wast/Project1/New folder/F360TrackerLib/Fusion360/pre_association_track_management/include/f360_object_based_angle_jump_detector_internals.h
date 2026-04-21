/*===================================================================================*\
* FILE: f360_object_based_angle_jump_detector_internals.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
*    This file contains declarations of functions related to object based 
*    angle jump detector.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#ifndef F360_OBJECT_BASED_ANGLE_JUMP_DETECTOR_INTERNALS_H
#define F360_OBJECT_BASED_ANGLE_JUMP_DETECTOR_INTERNALS_H

#include "f360_object_based_angle_jump_detector.h"
#include "f360_point.h"
#include "f360_reuse.h"
#include "f360_angle.h"

namespace f360_variant_A
{
   //namespace aj_detector - blocked for now. It will be resotred once problem with embedded build is solved - DFU-511
   //{
      struct Det_Restrictions_T
      {
         float32_t min_lat;
         float32_t max_lat;
         float32_t min_long;
         float32_t max_long;
         float32_t max_range;
      };

      bool Is_Det_Suspected(
         const float32_t det_range,
         const int8_t det_confid_az,
         const Point &det_pos_vcs,
         const Det_Restrictions_T &det_restrictions
      );

      bool Is_Det_Object_Based_Angle_Jump(
         const F360_VCS_Velocity_T &sensor_velocity,
         const Point &sensor_mount_pos_vcs,
         const F360_Detection_Props_T &det_prop,
         const rspp_variant_A::RSPP_Detection_T &det_raw,
         const F360_VCS_Velocity_T &obj_velocity_vcs,
         const Det_Restrictions_T &det_restrictions,
         const F360_Calibrations_T& calibs
      );

      float32_t Calc_Max_Range(
         const float32_t object_lateral_pos,
         const float32_t sensor_lateral_pos,
         const float32_t range_gap);

      Det_Restrictions_T Calc_Det_Restrictions_Without_Max_Range(
         const F360_Object_Track_T &obj_track,
         const F360_Calibrations_T &calibs
      );

      float32_t Calc_New_VCS_Aligned_Sensor_Azim(
         const float32_t prev_az_vcs,
         const float32_t jump_value
      );

      bool Is_Det_On_New_Azimuth_Valid(
         const float32_t det_range,
         const Angle &jumped_az_vcs,
         const Point &sensor_mount_pos_vcs,
         const Det_Restrictions_T &det_restrictions,
         const float32_t max_doube_range_hypothesis
      );

      bool Is_Det_Inside_Restricted_AJ_Bbox(
         const Point &det_pos_vcs,
         const Det_Restrictions_T &det_restrictions
      );
   //} DFU-511
}
#endif
