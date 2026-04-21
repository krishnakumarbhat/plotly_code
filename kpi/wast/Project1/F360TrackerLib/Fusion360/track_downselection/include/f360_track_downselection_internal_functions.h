/*===================================================================================*\
* FILE: f360_track_downselection_internal_functions.h
*====================================================================================
* Copyright (C) 2020 Aptiv. All Rights Reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains signatures of functions used in Track_Downselection()
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/
#ifndef F360_TRACK_DOWNSELECTION_INTERNAL_FUNCTIONS_H
#define F360_TRACK_DOWNSELECTION_INTERNAL_FUNCTIONS_H

#include "f360_host.h"
#include "f360_tracker_info.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
#include "f360_bounding_box.h"
#include "f360_static_env_poly_types.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   float32_t Calc_Track_Priority(
      const F360_Host_T& host,
      const float32_t cos_host_vcs_sideslip,
      const float32_t sin_host_vcs_sideslip,
      const F360_Calibrations_T& calib,
      const F360_Tracker_Info_T& tracker_info,
      const BoundingBox& overall_confidence_exclusion_box,
      F360_Object_Track_T& obj_trk);

   int32_t Pop_Reduced_Id(F360_Tracker_Info_T& tracker_info);

   void Select_Obj_Tracks_to_Downselect(
      const F360_Host_T& host,
      const float32_t cos_host_vcs_sideslip,
      const float32_t sin_host_vcs_sideslip,
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calib,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      float32_t(&priorities)[NUMBER_OF_OBJECT_TRACKS],
      int32_t(&candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t& candidates_cnt);

   void Assign_Reduced_Idxs_To_Prioritized_Tracks(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      const int32_t(&candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t(&ids_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
      const uint32_t candidates_cnt);

   void Deselect_Existing_Reduced_Tracks(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      int32_t(&candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t(&ids_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t& candidates_cnt);

   float32_t Calc_Range_to_Track(
      const float32_t vcs_pos_longitudinal,
      const float32_t vcs_pos_lateral,
      const float32_t min_track_range);

   bool Is_Valid_Trk_Near_Host(
      const F360_Object_Track_T& obj_trk,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib);

   float32_t Calc_Filtered_Dets_Thresh(
      const float32_t & time_since_initialization,
      const F360_Calibrations_T & calib);

   float32_t Calc_Average_Num_Dets(
      const F360_Object_Track_T& obj_trk,
      const float32_t delta_time);

   float32_t Decrease_Priority_and_Confidence_for_Implausible_Tracks(
      F360_Object_Track_T & obj_trk,
      const F360_Tracker_Info_T &tracker_info,
      const F360_Calibrations_T &calib,
      const float32_t priority);

   bool Is_Unreliable_Low_Conf_Moveable_Track(
      const F360_Object_Track_T& obj_trk,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib,
      const BoundingBox& exclusion_box);

   BoundingBox Define_Overall_Confidence_Exclusion_Box_Around_Host(
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T& calib,
      const float32_t box_longpos_shift);

   bool Is_Outside_Triangular_Zone_Behind_Host(
      const Point& track_posn,
      const float32_t triangular_zone_long_shift);

   bool Is_Heading_Different_Than_Host(
      const F360_Object_Track_T& obj_trk,
      const F360_Calibrations_T &calib);

   bool Is_Outside_Exclusion_Box(
      const Point& track_posn,
      const BoundingBox& exclusion_box);

   bool Has_Low_TTC(
      const float32_t host_speed,
      const F360_VCS_Velocity_T& track_velocity,
      const Point& track_pos,
      const float32_t k_low_conf_unreliability_max_ttc);

   void Cond_LP_Filter_Reduced_Det_Num(
      const F360_Tracker_Info_T& tracker_info,
      const float32_t &filtration_factor,
      F360_Object_Track_T(&obj_trks)[NUMBER_OF_OBJECT_TRACKS]);
}
#endif
