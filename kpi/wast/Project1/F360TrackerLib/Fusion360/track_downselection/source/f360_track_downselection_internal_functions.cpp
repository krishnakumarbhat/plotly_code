/*===================================================================================*\
* FILE: f360_track_downselection_internal_functions.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv. All Rights Reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definitions of functions used in Track_Downselection()
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/
#include "f360_track_downselection_internal_functions.h"
#include "f360_math.h"
#include <cstring>
#include "f360_push_reduced_id.h"
#include "f360_math_func.h"
#include "f360_calc_trk_ttc.h"
#include "f360_find_closest_valid_sep_on_given_side.h"
#include "f360_calculate_curvi_position.h"
#include <algorithm>

namespace f360_variant_A
{
   static float32_t Calculate_Stationary_Object_Priority(
      const F360_Calibrations_T& calib,
      const F360_Host_T& host,
      const F360_Object_Track_T& obj);

   /*===========================================================================*\
   * FUNCTION: Calc_Track_Priority()
   *===========================================================================
   * RETURN VALUE:
   * float32_t priority
   *
   * PARAMETERS:
   * const F360_Host_T& host,
   * const float32_t cos_host_vcs_sideslip,
   * const float32_t sin_host_vcs_sideslip,
   * const F360_Calibrations_T& calib,
   * const F360_Tracker_Info_T& tracker_info,
   * const BoundingBox& overall_confidence_exclusion_box,
   * F360_Object_Track_T& obj_trk,
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
   * This function updates the priority. Higher value of priority means that object is less important.
                                         Lower value means that object is more important.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Track_Priority(
      const F360_Host_T& host,
      const float32_t cos_host_vcs_sideslip,
      const float32_t sin_host_vcs_sideslip,
      const F360_Calibrations_T& calib,
      const F360_Tracker_Info_T& tracker_info,
      const BoundingBox& overall_confidence_exclusion_box,
      F360_Object_Track_T& obj_trk)
   {
      float32_t priority;

      if (obj_trk.f_valid_for_liberal_tracking)
      {
         priority = -2.0F;    // -2 means that object has highest priority for liberal tracking.
      }
      else if (
         (obj_trk.status < F360_OBJECT_STATUS_NEW_UPDATED)
         || obj_trk.f_low_confidence_level
         || (CONF3_NONE == obj_trk.conf_overall)
         || Is_Unreliable_Low_Conf_Moveable_Track(obj_trk, host, calib, overall_confidence_exclusion_box))
      {
         priority = calib.k_track_downselect_max_priority; // low prio, probably will not be downselected
      }
      else if ((
         (obj_trk.status == F360_OBJECT_STATUS_UPDATED) || (obj_trk.status == F360_OBJECT_STATUS_COASTED))
         && (obj_trk.reduced_status == F360_OBJECT_STATUS_INVALID)
         && (obj_trk.confidenceLevel < calib.k_track_downselect_confidence_thresh)) // object not on reduced list with low confidence, but is tracked internally
      {
         priority = calib.k_track_downselect_max_priority; // low prio, probably will not be downselected
      }
      else
      {
         if (Is_Valid_Trk_Near_Host(obj_trk, host, calib))
         {
            priority = -1.0F;    // -1 means that object has high priority.
         }
         else
         {
            const float32_t range = Calc_Range_to_Track(obj_trk.vcs_position.x, obj_trk.vcs_position.y, calib.k_calc_ttc_min_obj_track_range);
            if (obj_trk.f_moveable)
            {
               const float32_t ttc = Calc_Trk_Ttc(obj_trk, host, cos_host_vcs_sideslip, sin_host_vcs_sideslip, calib, range);
               if (ttc >= -1.0F)
               {
                  priority = std::min(calib.k_track_downselect_upper_ttc_saturation_level, ttc);
               }
               else
               {
                  priority = calib.k_track_downselect_lower_ttc_saturation_level - std::max(ttc, calib.k_track_downselect_min_negative_ttc) + range;
               }
            }
            else
            {
               priority = Calculate_Stationary_Object_Priority(calib, host, obj_trk);
            }
         }
      }

      if (priority < calib.k_track_downselect_max_priority)
      {
         priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(obj_trk, tracker_info, calib, priority);
      }

      // Lower confidence if NU_2_C ghost track
      if (obj_trk.f_ghost_NU_2_C && (obj_trk.time_since_track_updated >= calib.k_hyst_time_for_coasted_objects))
      {
         if (obj_trk.confidenceLevel > (calib.k_track_downselect_confidence_level_lowering_factor * calib.low_confidence_level_thresh))
         {
            obj_trk.confidenceLevel = calib.k_track_downselect_confidence_level_lowering_factor * calib.low_confidence_level_thresh;
         }
      }

      return priority;
   }

   /*===========================================================================*\
   * FUNCTION: Pop_Reduced_Id()
   *===========================================================================
   * RETURN VALUE:
   *  int32_t reduced_id
   *
   * PARAMETERS:
   * F360_Tracker_Info_T& tracker_info
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
   * This function pops reduced Id from front of array.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   * There is a reduced IDs to pop
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   int32_t Pop_Reduced_Id(F360_Tracker_Info_T& tracker_info)
   {
      const int32_t num_obj = tracker_info.reduced_num_active_objs;
      const int32_t num_inactive_obj = static_cast<int32_t>(tracker_info.variant.num_reduced_tracks) - num_obj;
      const int32_t reduced_id = tracker_info.reduced_inactive_obj_ids[0];

      // Function get and return first available obj_idx from tracker_info.reduced_inactive_obj_ids[]
      // Then shift the indexes that has been left by one place to "left" and put zero at last element.

      for (int32_t idx = 1; idx < num_inactive_obj; ++idx)
      {
         tracker_info.reduced_inactive_obj_ids[idx - 1] = tracker_info.reduced_inactive_obj_ids[idx];
      }

      tracker_info.reduced_inactive_obj_ids[num_inactive_obj - 1] = 0;
      tracker_info.reduced_active_obj_ids[num_obj] = reduced_id;
      tracker_info.reduced_num_active_objs++;

      return reduced_id;
   }

   /*===========================================================================*\
      * FUNCTION: Select_Obj_Tracks_to_Downselect()
      *===========================================================================
      * RETURN VALUE:
      * none
      *
      * PARAMETERS:
      * const F360_Host_T& host,
      * const float32_t cos_host_vcs_sideslip,
      * const float32_t sin_host_vcs_sideslip,
      * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      * F360_Tracker_Info_T& tracker_info,
      * const F360_Calibrations_T& calib,
      * F360_Object_Track_T (& object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      * float32_t (& priorities)[NUMBER_OF_OBJECT_TRACKS],
      * int32_t (& candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      * uint32_t& candidates_cnt
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
      * This function selects, based on calculated by itself priority, tracks to downselection.
      *
      * PRECONDITIONS:
      * All the Pointers should Point to valid structures.
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
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
      uint32_t& candidates_cnt)
   {
      const BoundingBox overall_confidence_exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(static_env_polys, calib, host.dist_rear_axle_to_vcs_m * 0.5F);
      for (int32_t loop_idx = 0; loop_idx < tracker_info.num_active_objs; loop_idx++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[loop_idx] - 1;
         const float32_t track_priority = Calc_Track_Priority(host, cos_host_vcs_sideslip, sin_host_vcs_sideslip, calib, tracker_info, overall_confidence_exclusion_box, object_tracks[obj_idx]);
         if (track_priority < calib.k_track_downselect_max_priority)    // Lower value of priority means that object is more important
         {
            priorities[candidates_cnt] = track_priority;
            candidates_idxs[candidates_cnt] = obj_idx;
            candidates_cnt++;
         }
         else
         {
            if (object_tracks[obj_idx].reduced_id > F360_INVALID_REDUCED_ID)
            {
               Push_Reduced_Id(object_tracks[obj_idx].reduced_id, tracker_info);

               object_tracks[obj_idx].reduced_status = F360_OBJECT_STATUS_INVALID;
               object_tracks[obj_idx].reduced_id = F360_INVALID_REDUCED_ID;
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Assign_Reduced_Idxs_To_Prioritized_Tracks()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
   * F360_Tracker_Info_T& tracker_info,
   * const int32_t (& candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
   * uint32_t (& idxs_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
   * const uint32_t candidates_cnt
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
   * This function assigns reduced idxs to prioritized tracks .
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Assign_Reduced_Idxs_To_Prioritized_Tracks(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      const int32_t(&candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t(&ids_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
      const uint32_t candidates_cnt)
   {
      for (uint32_t loop_idx = 0U; loop_idx < candidates_cnt; loop_idx++)
      {
         const int32_t obj_idx = candidates_idxs[ids_of_objs_sorted_by_priority[loop_idx]];

         if (object_tracks[obj_idx].reduced_id > F360_INVALID_REDUCED_ID)
         {
            object_tracks[obj_idx].reduced_status = object_tracks[obj_idx].status;
            object_tracks[obj_idx].time_since_downselected += tracker_info.elapsed_time_s;
            tracker_info.reduced_obj_ids[object_tracks[obj_idx].reduced_id - 1] = obj_idx + 1;
         }
         else
         {
            // create new reduced track
            const int32_t reduced_id = Pop_Reduced_Id(tracker_info);
            assert(reduced_id > F360_INVALID_REDUCED_ID);
            object_tracks[obj_idx].reduced_status = F360_OBJECT_STATUS_NEW;
            object_tracks[obj_idx].reduced_id = reduced_id;
            object_tracks[obj_idx].time_since_downselected = 0.0F;
            tracker_info.reduced_obj_ids[reduced_id - 1] = obj_idx + 1;
         }
      }
   }

   /*===========================================================================*\
  * FUNCTION: Deselect_Existing_Reduced_Tracks()
  *===========================================================================
  * RETURN VALUE:
  * none
  *
  * PARAMETERS:
  * F360_Object_Track_T (& object_tracks)[NUMBER_OF_OBJECT_TRACKS],
  * F360_Tracker_Info_T& tracker_info,
  * int32_t (& candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
  * uint32_t (& idxs_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
  * uint32_t& candidates_cnt
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
  * This function deselects any existing reduced tracks now ranked lower than the number of reduced slots.
  *
  * PRECONDITIONS:
  * All the Pointers should Point to valid structures.
  *
  * POSTCONDITIONS:
  * None
  *
  \*===========================================================================*/
   void Deselect_Existing_Reduced_Tracks(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      int32_t(&candidates_idxs)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t(&ids_of_objs_sorted_by_priority)[NUMBER_OF_OBJECT_TRACKS],
      uint32_t& candidates_cnt)
   {
      for (uint32_t loop_idx = tracker_info.variant.num_reduced_tracks; loop_idx < candidates_cnt; loop_idx++)
      {
         const int32_t obj_idx = candidates_idxs[ids_of_objs_sorted_by_priority[loop_idx]];
         if (object_tracks[obj_idx].reduced_id > F360_INVALID_REDUCED_ID)
         {
            Push_Reduced_Id(object_tracks[obj_idx].reduced_id, tracker_info);

            object_tracks[obj_idx].reduced_status = F360_OBJECT_STATUS_INVALID;
            object_tracks[obj_idx].reduced_id = F360_INVALID_REDUCED_ID;
         }
      }
      candidates_cnt = tracker_info.variant.num_reduced_tracks;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Range_to_Track()
   *===========================================================================
   * RETURN VALUE:
   * int32_t range
   *
   * PARAMETERS:
   * const float32_t vcs_pos_longitudinal,
   * const float32_t vcs_pos_lateral,
   * const float32_t min_track_range
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
   * This function calculates range (sqrt(x^2 + y^2)) from host to object track.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Range_to_Track(
      const float32_t vcs_pos_longitudinal,
      const float32_t vcs_pos_lateral,
      const float32_t min_track_range)
   {
      const float32_t range = F360_Get_Hypotenuse(vcs_pos_longitudinal, vcs_pos_lateral);

      return (range < min_track_range) ? min_track_range : range;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Valid_Trk_Near_Host()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * F360_Object_Track_T& obj_trk,
   * const F360_Host_T& host,
   * const F360_Calibrations_T& calib
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
   * This function calculates if track near host is valid.
   *
   * PRECONDITIONS:
   * All the References should point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Valid_Trk_Near_Host(
      const F360_Object_Track_T& obj_trk,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib)
   {
      const float32_t vcs_lon_mid_pos = host.dist_rear_axle_to_vcs_m * 0.5F;

      return (std::abs(obj_trk.vcs_position.x + vcs_lon_mid_pos) < calib.k_track_downselect_max_vcs_x_range_to_preserve)
         && (std::abs(obj_trk.vcs_position.y) < calib.k_track_downselect_max_vcs_y_range_to_preserve);
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Filtered_Dets_Thresh()
   *===========================================================================
   * RETURN VALUE:
   * float32_t filtered_dets_thresh
   *
   * PARAMETERS:
   * const float32_t & time_since_initialization,
   * const F360_Calibrations_T & calib
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
   * This function calculates filtered dets threshold.
   *
   * PRECONDITIONS:
   * All the References should point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Filtered_Dets_Thresh(
      const float32_t & time_since_initialization,
      const F360_Calibrations_T & calib)
   {
      float32_t filtered_dets_thresh;
      if (time_since_initialization < calib.k_track_downselect_min_time_filter_dets_thresh)
      {
         filtered_dets_thresh = calib.k_track_downselect_dets_threshold;
      }
      else
      {
         filtered_dets_thresh = 1.0F;
      }

      return filtered_dets_thresh;
   }

   /*===========================================================================*\
      * FUNCTION: Calc_Average_Num_Dets()
      *===========================================================================
      * RETURN VALUE:
      * float32_t average_dets
      *
      * PARAMETERS:
      * const F360_Object_Track_T& obj_trk,
      * const float32_t delta_time
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
      * This function calculates average number of detections associated to object.
      *
      * PRECONDITIONS:
      * All the References should point to valid structures.
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
   float32_t Calc_Average_Num_Dets(
      const F360_Object_Track_T& obj_trk,
      const float32_t delta_time)
   {
      // Only calculate average detections if the object have been alive for at least one second

      float32_t average_dets;
      if (obj_trk.time_since_initialization < 1.0F)
      {
         average_dets = 0.0F;
      }
      else
      {
         average_dets = (static_cast<float32_t>(obj_trk.total_reduced_dets) * delta_time) / obj_trk.time_since_initialization;
      }

      return average_dets;
   }

   /*===========================================================================*\
      * FUNCTION: Decrease_Priority_and_Confidence_for_Implausible_Tracks()
      *===========================================================================
      * RETURN VALUE:
      * float32_t new_priority
      *
      * PARAMETERS:
      * F360_Object_Track_T & obj_trk,
      * const F360_Tracker_Info_T &tracker_info,
      * const F360_Calibrations_T &calib,
      * const float32_t priority
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
      * This function decreases object confidence level and priority if object is behind guardrail or is mirrored,
      * and returns new value of object priority if calculated.
      *
      * PRECONDITIONS:
      * All the References should point to valid structures.
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
   float32_t Decrease_Priority_and_Confidence_for_Implausible_Tracks(
      F360_Object_Track_T & obj_trk,
      const F360_Tracker_Info_T &tracker_info,
      const F360_Calibrations_T &calib,
      const float32_t priority)
   {
      // Check if track is on the other side of the guardrail
      const bool f_filt_num_dets_below_dynamic_th = obj_trk.filtered_dets <= Calc_Filtered_Dets_Thresh(obj_trk.time_since_initialization, calib);
      const bool f_filt_num_dets_below_low_th = obj_trk.filtered_dets <= calib.k_track_downselect_dets_threshold_low;
      const bool f_ave_num_dets_below_ave_th = Calc_Average_Num_Dets(obj_trk, tracker_info.elapsed_time_s) <= calib.k_track_downselect_average_dets_thresh;
      const bool f_insufficient_num_dets_updating_track = f_filt_num_dets_below_dynamic_th && (f_filt_num_dets_below_low_th || f_ave_num_dets_below_ave_th);

      const bool f_possibly_ghost_1 =
         ((obj_trk.behind_sep_id != F360_INVALID_UNSIGNED_ID)
         && (obj_trk.f_moveable)
         && (f_insufficient_num_dets_updating_track)
         && (obj_trk.confidenceLevel < calib.k_track_downselect_max_confidence_for_possibly_ghost));

      // Check if track is on or behind the guardrail and if fast yawing
      const float32_t yawrate = obj_trk.speed * obj_trk.curvature;
      const bool f_track_has_significant_yawrate_and_speed =
         (obj_trk.speed >= calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object)
         && (std::abs(yawrate) >= calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object);

      const bool f_possibly_ghost_2 =
         (((obj_trk.behind_sep_id != F360_INVALID_UNSIGNED_ID) || (obj_trk.on_sep_id != F360_INVALID_UNSIGNED_ID))
         && (f_track_has_significant_yawrate_and_speed)
         && (obj_trk.f_moveable));

      // If object is supposed to be a ghost or mirror decrease its confidence and priority.
      float32_t new_priority;
      if (f_possibly_ghost_1 || f_possibly_ghost_2 || (obj_trk.mirror_prob > calib.k_mirror_prob_threshold))
      {
         if (obj_trk.confidenceLevel > calib.k_track_downselect_confidence_level_lowering_factor * calib.low_confidence_level_thresh)
         {
            obj_trk.confidenceLevel = calib.k_track_downselect_confidence_level_lowering_factor * calib.low_confidence_level_thresh;
         }

         new_priority = calib.k_track_downselect_max_priority; // Max value of priority means that object is less important.
      }
      else
      {
         new_priority = priority;
      }

      return new_priority;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Unreliable_Low_Conf_Moveable_Track()
   *===========================================================================
   * RETURN VALUE:
   * bool f_unreliable
   *
   * PARAMETERS:
   * F360_Object_Track_T& obj_trk,
   * const F360_Host_T& host,
   * const F360_Calibrations_T& calib,
   * const BoundingBox& exclusion_box
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
   * Determines if low confidence track is reliable or not.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Unreliable_Low_Conf_Moveable_Track(
      const F360_Object_Track_T& obj_trk,
      const F360_Host_T& host,
      const F360_Calibrations_T& calib,
      const BoundingBox& exclusion_box)
   {
      const float32_t obj_heading_difference = std::abs(obj_trk.vcs_heading.Value()) - calib.k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios;
      const bool f_obj_moving_perpendicular_to_host = std::abs(obj_heading_difference) <= calib.k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios;
      const bool host_is_slow = host.vcs_speed < calib.k_low_conf_max_allowed_host_speed_in_cta_scenarios;

      const bool f_is_cta_scenario = host_is_slow && f_obj_moving_perpendicular_to_host;
      const CONF3_T conf_threshold_for_low_conf = f_is_cta_scenario ? CONF3_MED : CONF3_HIGH;
      
      const bool f_unreliable = (
         (obj_trk.conf_overall < conf_threshold_for_low_conf)
         && Is_Outside_Triangular_Zone_Behind_Host(obj_trk.vcs_position, -host.dist_rear_axle_to_vcs_m)
         && (Is_Heading_Different_Than_Host(obj_trk, calib)
            || Is_Outside_Exclusion_Box(obj_trk.vcs_position, exclusion_box))
         && Has_Low_TTC(host.vcs_speed, obj_trk.vcs_velocity, obj_trk.vcs_position, calib.k_low_conf_unreliability_max_ttc)
         && obj_trk.f_moveable);
      
      return f_unreliable;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Outside_Triangular_Zone_Behind_Host()
   *===========================================================================
   * RETURN VALUE:
   * bool f_outside_triangular_zone_behind_host
   *
   * PARAMETERS:
   * const Point& track_posn,
   * const float32_t triangular_zone_long_shift
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
   * Determine if track is outside triangular zone behind host. Zone is defined 
   * by two rays spread apart at 90 degree angle, with y axis beeing the symmetry 
   * line, with starting point at vcs_x = triangular_zone_long_shift, vcs_y = 0
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Outside_Triangular_Zone_Behind_Host(
      const Point& track_posn,
      const float32_t triangular_zone_long_shift)
   {
      const float32_t translated_posn_x = track_posn.x - triangular_zone_long_shift;
      const bool f_outside_triangular_zone_behind_host = (track_posn.y < translated_posn_x) || (track_posn.y > -translated_posn_x);

      return f_outside_triangular_zone_behind_host;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Heading_Different_Than_Host()
   *===========================================================================
   * RETURN VALUE:
   * bool f_moving_in_specified_direction
   *
   * PARAMETERS:
   * const F360_Object_Track_T& obj_trk,
   * const F360_Calibrations_T &calib
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
   * Determines if a CTCA or fast moving CCA object's heading differs significantly from host's.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Heading_Different_Than_Host(
      const F360_Object_Track_T& obj_trk,
      const F360_Calibrations_T& calib)
   {
      const bool f_moving_in_different_direction_than_host = (
         ((F360_TRACKER_TRKFLTR_CTCA == obj_trk.trk_fltr_type) || (std::abs(obj_trk.speed) > calib.fast_moving_thresh))
         && (std::abs(obj_trk.vcs_heading.Value()) > calib.k_low_conf_unreliability_min_heading));

      return f_moving_in_different_direction_than_host;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Outside_Exclusion_Box()
   *===========================================================================
   * RETURN VALUE:
   * bool f_outside_exclusion_box
   *
   * PARAMETERS:
   * const Point& track_posn,
   * const BoundingBox& exclusion_box
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
   * Determines if a track is outside of rectangular exclusion box defined around the host.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Outside_Exclusion_Box(
      const Point& track_posn,
      const BoundingBox& exclusion_box)
   {
      const bool f_outside_exclusion_box = !exclusion_box.Contains(track_posn);

      return f_outside_exclusion_box;
   }

   /*===========================================================================*\
   * FUNCTION: Has_Low_TTC()
   *===========================================================================
   * RETURN VALUE:
   * bool f_low_ttc
   *
   * PARAMETERS:
   * const float32_t host_speed,
   * const F360_VCS_Velocity_T& track_velocity,
   * const Point& track_pos,
   * const float32_t k_low_conf_unreliability_max_ttc
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
   * Determines if a track has low TTC in longitudinal direction.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Has_Low_TTC(
      const float32_t host_speed,
      const F360_VCS_Velocity_T& track_velocity,
      const Point& track_pos,
      const float32_t k_low_conf_unreliability_max_ttc)
   {
      const float32_t vel_diff = (host_speed - track_velocity.longitudinal);
      const bool f_low_ttc = (std::abs(vel_diff) > F360_EPSILON) ? ((track_pos.x / vel_diff) < k_low_conf_unreliability_max_ttc) : false;

      return f_low_ttc;
   }


   /*===========================================================================*\
   * FUNCTION: Define_Overall_Confidence_Exclusion_Box_Around_Host()
   *===========================================================================
   * RETURN VALUE:
   * BoundingBox
   *
   * PARAMETERS:
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS] - array of static environment polynomials
   * const F360_Calibrations_T& calib - tracker calibrations
   * const float32_t box_longpos_shift - shift of rectangular exclusion box in longitudinal direction
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
   * Function defines rectangular axis-aligned exclusion box used to limit the scope of
   * overall confidence filter (to not apply it in exclusion box i.e close to host).
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   BoundingBox Define_Overall_Confidence_Exclusion_Box_Around_Host(
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T& calib, 
      const float32_t box_longpos_shift)
   {
      const float32_t exclusion_box_lat = calib.k_conf_downselection_exclusion_box_lat;
      
      const Closest_SEP_Info closest_left_sep = Find_Closest_SEP_On_Given_Side(static_env_polys, F360_HOST_LEFT_SIDE, 0.0F);
      const float32_t left_limit = (closest_left_sep.id != F360_INVALID_UNSIGNED_ID) ? std::max(closest_left_sep.lat_pos, -exclusion_box_lat) : -exclusion_box_lat;
      const Point rear_left_corner = Point(-calib.k_conf_downselection_exclusion_box_long - box_longpos_shift, left_limit);

      const Closest_SEP_Info closest_right_sep = Find_Closest_SEP_On_Given_Side(static_env_polys, F360_HOST_RIGHT_SIDE, 0.0F);
      const float32_t right_limit = (closest_right_sep.id != F360_INVALID_UNSIGNED_ID) ? std::min(closest_right_sep.lat_pos, exclusion_box_lat) : exclusion_box_lat;
      const Point front_right_corner = Point(calib.k_conf_downselection_exclusion_box_long - box_longpos_shift, right_limit);

      return BoundingBox(rear_left_corner, front_right_corner);
   }

   /*===========================================================================*\
   * FUNCTION: Cond_LP_Filter_Reduced_Det_Num()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *  const F360_Tracker_Info_T& tracker_info,
   *  const float32_t &filtration_factor,
   *  F360_Object_Track_T(&obj_trks)[NUMBER_OF_OBJECT_TRACKS])
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
   * This function applies a low pass filter to the number of reduced detections
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Cond_LP_Filter_Reduced_Det_Num(
      const F360_Tracker_Info_T& tracker_info,
      const float32_t &filtration_factor,
      F360_Object_Track_T(&obj_trks)[NUMBER_OF_OBJECT_TRACKS])
   {
      for (int32_t loop_idx = 0; loop_idx < tracker_info.num_active_objs; loop_idx++)
      {
         const uint32_t ndets = obj_trks[loop_idx].num_rr_inlier_dets; 

         const bool f_make_filtration = !((0U < ndets) && (static_cast<float32_t>(ndets) < obj_trks[loop_idx].filtered_dets));

         if (F360_OBJECT_STATUS_NEW_UPDATED == obj_trks[loop_idx].status)
         {
            obj_trks[loop_idx].filtered_dets = 0.5F; // initial value of low pass filter
         }

         if (f_make_filtration)
         {
            obj_trks[loop_idx].filtered_dets = (filtration_factor * obj_trks[loop_idx].filtered_dets) + ((1.0F - filtration_factor) * static_cast<float32_t>(ndets));
         }

      }
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Stationary_Object_Priority()
   *===========================================================================
   * RETURN VALUE:
   * float32_t priority
   *
   * PARAMETERS:
   *  const F360_Calibrations_T& calib
   *  const F360_Host_T& host
   *  const F360_Object_Track_T& obj
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
   * This calculates the priority for stationary objects. Visualization of
   * scheme can be found in DEX-2982
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   static float32_t Calculate_Stationary_Object_Priority(
      const F360_Calibrations_T& calib,
      const F360_Host_T& host,
      const F360_Object_Track_T& obj)
   {
      const float32_t obj_curvi_lat_pos_abs = std::abs(Calculate_Curvi_Lat_Pos(host, obj.vcs_position.x, obj.vcs_position.y));

      const float32_t calib_curvi_lat_dist_thr = 8.75F;
      const float32_t calib_curvi_lat_gain = 0.25F;

      // Lateral score depends on distance to host curvi trajectory
      float32_t score_lat = 0.0F;
      if (obj_curvi_lat_pos_abs < calib_curvi_lat_dist_thr)
      {
         score_lat = obj_curvi_lat_pos_abs * calib_curvi_lat_gain;
      }
      else
      {
         score_lat = obj_curvi_lat_pos_abs - (1.0F - calib_curvi_lat_gain) * calib_curvi_lat_dist_thr;
      }

      const float32_t obj_long_pos_relative_host_center_pos = host.dist_rear_axle_to_vcs_m * 0.5F + obj.vcs_position.x;

      // Longitudinal score depends on host travel direction and host speed
      const bool f_host_moving_forward = F360_Sign(host.speed) >= 0;
      const bool f_object_is_in_front_of_host = obj_long_pos_relative_host_center_pos >= 0.0F;
      const bool f_object_is_in_direction_host_is_moving = (f_host_moving_forward && f_object_is_in_front_of_host) || ((!f_host_moving_forward) && (!f_object_is_in_front_of_host));

      const float32_t calib_min_abs_host_speed = 3.0F;
      const float32_t speed_factor = std::max(std::abs(host.speed), calib_min_abs_host_speed);

      float32_t score_long = 0.0F;
      if (f_object_is_in_direction_host_is_moving)
      {
         const float32_t calib_direction_of_travel_score_gain = 3.333F;
         score_long = (std::abs(obj_long_pos_relative_host_center_pos) / speed_factor) * calib_direction_of_travel_score_gain; // decrease score the faster host moves (more important)
      }
      else
      {
         const float32_t calib_non_direction_of_travel_score_gain = 0.5F;
         score_long = std::abs(obj_long_pos_relative_host_center_pos) * speed_factor * calib_non_direction_of_travel_score_gain; // increase score the faster host moves (less important)
      }

      // Final score
      float32_t score = score_lat + score_long;

      // To ensure that score does not grow too large to interfere with other downselection prioty schemes
      const float32_t calib_score_saturation_start = 500.0F;
      if (score > calib_score_saturation_start)
      {
         const float32_t calib_score_saturation_gain = 0.1F;
         score = calib_score_saturation_start + score * calib_score_saturation_gain;
      }

      // The first calibration constant makes sure that non-movable object always has higher score (lower priority) relative to moving objects
      const float32_t priority = calib.k_track_downselect_low_priority_level_for_nonmoveable_tracks + score;

      return priority;
   }
}

