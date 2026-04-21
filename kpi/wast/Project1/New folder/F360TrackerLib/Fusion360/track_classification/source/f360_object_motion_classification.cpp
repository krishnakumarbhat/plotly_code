/*===================================================================================*\
* FILE:  f360_object_motion_classification.h
*====================================================================================

*Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.

* Confidential - Restricted Aptiv information. Do not disclose."

*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains implementation of Is_Object_Suspected_Stationary() function.
*
*

* Applicable Standards (in order of precedence: highest first):

*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]

*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]

***/

#include "f360_object_motion_classification.h"
#include "f360_object_motion_classification_helpers.h"
#include "f360_is_object_suspected_stationary.h"
#include "f360_math_func.h"
#include "f360_adjust_fltr_type_dependent_params.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_trk_fltr_ctca_states.h"

namespace f360_variant_A
{
   static bool Is_Outside_Queue_Zone(const F360_Object_Track_T& obj_track, const F360_Calibrations_T& calib, const F360_Host_T& host);

   /*===========================================================================*\
   * FUNCTION: Object_Motion_Classification
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS] - reference to objects tracks struct
   * F360_Tracker_Info_T& tracker_info - reference to struct containing information about tracker
   * const F360_Host_T& host - reference to struct containing information about host vehicle
   * const F360_Globals_T& globals - reference to struct containing global variables
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list - - reference to stuct containing raw detection list
   * const F360_Calibrations_T& calib - reference to calibrations struct
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS] - sensors array
   * const Occlusion_T& occlusion - reference to occlusion object
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
   * Main function to be called to determine object motion status.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Object_Motion_Classification(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Tracker_Info_T& tracker_info,
      const F360_Host_T& host,
      const F360_Globals_T& globals,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Calibrations_T& calib,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const Occlusion_T& occlusion)
   {
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_trk_idx = tracker_info.active_obj_ids[i] - 1;

         F360_Object_Track_T& object = object_tracks[obj_trk_idx];

         if (Object_Motion_Status_Should_Be_Updated(object, calib.k_hyst_time_for_coasted_objects))
         {
            const bool f_stationary_suspected = Is_Object_Suspected_Stationary(object, raw_detect_list, host, calib, sensors, occlusion);
            const bool prev_f_moving = object.f_moving;

            if (f_stationary_suspected)
            {
               object.cntConsecutiveMoving = 0;
               object.cntConsecutiveAmbiguous++;
            }
            else
            {
               object.cntConsecutiveMoving++;
               object.cntConsecutiveAmbiguous = 0;
            }

            const int32_t min_num_consec_moving = Get_Min_Num_Consec_Moving(object, host.yaw_rate_rad, calib);

            if (min_num_consec_moving <= object.cntConsecutiveMoving)
            {
               object.f_moving = true;
               object.cntConsecutiveStopped = 0;
               object.f_stopped = false;
            }
            else if (min_num_consec_moving <= object.cntConsecutiveAmbiguous)
            {
               object.f_moving = false;
            }
            else
            {
               // do not change object moving status
            }

            // non moveable hard switch
            bool non_moveable_hs = false;
            if (prev_f_moving && (!object.f_moving)) // falling edge
            {
               object.f_stopped = true;
               // if the interval of two consecutive stops (i.e. from mvoing to non-moving) is less than 3 seconds
               // such a frequent flickering is likely a ghost
               
               // For a new born moving object, if it comes to a stop within 3 seconds, it 
               // is also likely a ghost
               if (object.time_since_last_stop < calib.k_object_motion_min_consec_stop_time_th) 
               {
                  non_moveable_hs = true;
               }
               object.time_since_last_stop = 0.0F; //reset the timer
            }

            if ((!object.f_moving) && object.f_moveable)
            {
               ++object.cntConsecutiveStopped;
            }

            const float32_t orientation_abs = std::abs(object.bbox.Get_Orientation().Value());
            // flag to mark object orientation close to perpendicular to the host's
            const bool f_orient_susp = (calib.k_object_motion_max_abs_orient_diff_to_host < orientation_abs)
                                       && (orientation_abs < (F360_PI - calib.k_object_motion_max_abs_orient_diff_to_host));

            if (((calib.k_object_motion_min_consec_stopped <= object.cntConsecutiveStopped) || non_moveable_hs)
                && (Is_Outside_Queue_Zone(object, calib, host) || f_orient_susp))
            {
               object.f_moveable = false;
               Adjust_Fltr_Type_CTCA_To_CCA(object);

               object.bbox.Set_Length(calib.k_nonmoveable_target_diameter);
               object.bbox.Set_Width(calib.k_nonmoveable_target_diameter);
            }
         }

         Update_Object_Properties(object, globals);
      }

   }

   /*===========================================================================*\
   * FUNCTION: Is_Outside_Queue_Zone
   *===========================================================================
   * RETURN VALUE:
   * Bool
   *
   * PARAMETERS:
   * F360_Object_Track_T& obj_track
   * const F360_Calibrations_T& calib
   * const F360_Host_T& host
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
   * Function to check whether the target object is outside of the host's vicinity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Is_Outside_Queue_Zone(const F360_Object_Track_T& obj_track, const F360_Calibrations_T& calib, const F360_Host_T& host)
   {
      const float32_t zone_distance_lateral = calib.k_object_motion_queue_zone_lat_dist;
      float32_t zone_distance_forward = INFTY;
      float32_t zone_distance_backward = INFTY;
      if (std::abs(host.speed) > calib.k_object_motion_queue_zone_host_stationary_speed_threshold)
      {
         zone_distance_forward = calib.k_object_motion_queue_zone_long_dist;
         zone_distance_backward = -calib.k_object_motion_queue_zone_long_dist;
      }
      else
      {
         zone_distance_forward = calib.k_object_motion_queue_zone_long_dist_host_stationary;
         zone_distance_backward = -calib.k_object_motion_queue_zone_long_dist_host_stationary;
      }

      return (std::abs(obj_track.vcs_position.y) > zone_distance_lateral) ||
         ((obj_track.vcs_position.x > zone_distance_forward) || (obj_track.vcs_position.x < zone_distance_backward));
   }
}
