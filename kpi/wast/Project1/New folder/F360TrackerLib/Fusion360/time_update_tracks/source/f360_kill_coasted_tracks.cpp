/*===================================================================================*\
* FILE:  f360_kill_coasted_tracks.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of function which is used to kill objects
*
* 
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_kill_coasted_tracks.h"
#include "f360_kill_obj_trk.h"
#include "f360_get_wall_time.h"
#include "f360_point.h"
#include "f360_reference_point_support_functions.h"

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: Kill_Coasted_Tracks()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
   * F360_Tracker_Info_T &tracker_info,
   * F360_TRKR_TIMING_INFO_T &timing_info,
   * const F360_Calibrations_T & calibrations
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
   * Terminates objects that have coasted for a long time
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Kill_Coasted_Tracks(
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T &tracker_info,
      F360_TRKR_TIMING_INFO_T &timing_info,
      const F360_Calibrations_T & calibrations,
      const F360_Globals_T& globals,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      const float32_t start_time = get_wall_time();

      int32_t kill_idx[NUMBER_OF_OBJECT_TRACKS] = {};
      int32_t n_tracks_to_kill = 0;
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[i] - 1;

         const bool f_normal_coasting = ((F360_OBJECT_STATUS_COASTED == object_tracks[obj_idx].status) &&
                                         (object_tracks[obj_idx].time_since_stage_start > calibrations.k_max_conf_objtrk_coast_time));

         const bool f_suspected_mirror_coasting = ((F360_OBJECT_STATUS_COASTED == object_tracks[obj_idx].status) &&
            (object_tracks[obj_idx].mirror_prob > calibrations.k_mirror_prob_threshold) &&
            (object_tracks[obj_idx].time_since_stage_start > calibrations.k_max_coast_time_mirror));

         const bool f_nonmoveable_obj_coasting_outside_fov = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_tracks[obj_idx], globals, sensors, calibrations);

         if (f_normal_coasting
            || f_suspected_mirror_coasting
            || f_nonmoveable_obj_coasting_outside_fov)
         {
            kill_idx[n_tracks_to_kill] = obj_idx;
            n_tracks_to_kill++;
         }
      }
      for (int32_t i = 0; i < n_tracks_to_kill; i++)
      {
         // Not necessary to clear associated detection's properties since they are cleared during Sensor_Preprocessing.
         Kill_Obj_Trk(kill_idx[i] + 1, object_tracks, tracker_info);
      }

      timing_info.kill_coasted_tracks = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object_track,
   * const F360_Globals_T& globals,
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Calibrations_T& calibrations
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
   * Determines if a stationary object has been coasting outside FLR FOV for longer than
   * a set limit of time.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   bool Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(
      const F360_Object_Track_T& object_track,
      const F360_Globals_T& globals,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations
   )
   {
      const bool f_non_movable = !object_track.f_moveable;
      const bool f_enough_time = object_track.time_since_stage_start > calibrations.k_max_coast_time_outside_fov;
      const bool f_coasted = F360_OBJECT_STATUS_COASTED == object_track.status;
      bool f_is_coasting_outside_fov = false;

      if(globals.f_single_front_center_radar_only && f_non_movable && f_enough_time && f_coasted)
      {
         for(const F360_Radar_Sensor_T& sensor: sensors)
         {
            if (sensor.constant.mounting_location == F360_MOUNTING_LOCATION_CENTER_FORWARD)
            {
               const float32_t left_fov_normals[2] = {sensor.constant.left_fov_normal[F360_DET_LOOK_ID_0], sensor.constant.left_fov_normal[F360_DET_LOOK_ID_1]};
               const float32_t right_fov_normals[2] = {sensor.constant.right_fov_normal[F360_DET_LOOK_ID_0], sensor.constant.right_fov_normal[F360_DET_LOOK_ID_1]};

               f_is_coasting_outside_fov = (!Is_Point_Inside_FOV(object_track.vcs_position, sensor, left_fov_normals, right_fov_normals));

               break;
            }
         }
      }
      
      return f_is_coasting_outside_fov;
   }
}
