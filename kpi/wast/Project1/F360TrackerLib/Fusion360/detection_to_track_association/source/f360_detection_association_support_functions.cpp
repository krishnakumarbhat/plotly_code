/*===========================================================================*\
* FILE: f360_detection_association_support_functions.cpp
*============================================================================
* Copyright (C) 2020-2022 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains declaration of supporting functions for detection association.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

#include <cmath>
#include <algorithm>
#include "f360_detection_association_support_functions.h"
#include "f360_norm_heading_angle.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_check_if_point_is_inside_box.h"
#include "f360_math_func.h"
#include "f360_associate_detection_to_object.h"
#include "f360_iterator.h"
#include "f360_calc_dist_to_edge.h"
#include "f360_bounding_box.h"
#include "f360_try_to_dealiase_range_rate.h"

namespace f360_variant_A
{
   static void Collect_Detection_Candidates_For_Association(
      const uint32_t number_of_valid_detections,
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      const int32_t obj_idx,
      uint32_t (&det_idx_want_to_assoc)[MAX_DETS_FOR_SINGLE_SENSOR],
      uint16_t & num_dets_want_to_assoc);

   /*===========================================================================*\
   * FUNCTION: Calc_Det_Score()
   *===========================================================================
   * RETURN VALUE:
   * float32_t score
   *
   * PARAMETERS:
   *  const F360_Object_Track_T
   *  const F360_Calibrations_T
   *  const float32_t range_rat
   *  const Point & vcs_pos,
   *  const bool f_water_spray) 
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
   * Calculates a score on how well a detection matches predicted position and
   * range rate to an object. The lower the score the better the association is.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calc_Det_Score(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calib,
      const float32_t range_rate_diff,
      const Point & vcs_pos,
      const bool f_water_spray)
   {
      float32_t final_score;

      if(object_track.f_moveable)
      {
         if (object_track.bbox.Contains(vcs_pos))
         {
            final_score = Calculate_Final_Det_Score_Inside_Solid_Bbox(vcs_pos, object_track, calib, range_rate_diff);
         }
         else if (Check_If_Point_Is_Inside_Extended_Bounding_Box_Cond(vcs_pos, object_track, f_water_spray, calib))
         {
            // Detection is inside extended bounding box but outside solid bounding box, calculate score accordingly
            final_score = Calculate_Final_Det_Score_Inside_Extended_Bbox(vcs_pos, object_track, calib, range_rate_diff);
         }
         else
         {
            // Highest value for detections outside the extended bounding box
            final_score = calib.k_score_outside_ext_bbox;
         }
      }
      else
      {
         if (object_track.bbox.Circle_Contains(vcs_pos))
         {
            final_score = Calculate_Final_Det_Score_Inside_Solid_Circle(vcs_pos, object_track, calib, range_rate_diff);
         }
         else
         {
            final_score = Calculate_Final_Det_Score_Inside_Extended_Circle(vcs_pos, object_track, calib, range_rate_diff);           
         }
      }


      return final_score;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Range_Rate_Threshold()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track,
   * const rspp_variant_A::RSPP_Detection_T & det_raw,
   * const F360_Detection_Props_T & det_prop,
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
   * Calculate range rate threshold for association on range rate.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calc_Range_Rate_Threshold(
      const F360_Object_Track_T & object_track,
      const rspp_variant_A::RSPP_Detection_T & det_raw,
      const F360_Detection_Props_T & det_prop,
      const F360_Radar_Sensor_T& sens,
      const F360_Calibrations_T & calibrations,
      const float32_t host_vcs_speed)
   {

      float32_t range_rate_thr = calibrations.k_range_rate_score_threshold;
      const bool f_elevation_unreliable = (fabsf(det_raw.raw.elevation) > F360_DEG2RAD(6.0F)) &&
         (sens.constant.sensor_type == F360_SENSOR_TYPE_MRR360_RADAR) &&
         (host_vcs_speed > calibrations.k_mrr360_min_host_speed_el_check);

      if (det_prop.f_FOV_edge || f_elevation_unreliable)
      {
         // Detection is on edge of FOV. Decrease range rate threshold
         range_rate_thr *= calibrations.k_rr_thr_factor_fov_edge;
      }
      else if ((object_track.status == F360_OBJECT_STATUS_COASTED) &&
         (object_track.time_since_stage_start >= calibrations.k_hyst_time_for_coasted_objects) &&
         (F360_Get_Hypotenuse_Squared(object_track.vcs_position.x, object_track.vcs_position.y) > calibrations.k_vcs_distance_sqr_thr) &&
         (object_track.speed > calibrations.k_speed_threshold))
      {
         // Object is coasted and far away. Increase range rate threshold
         range_rate_thr *=  calibrations.k_rr_thr_factor_far_away_coasted;
      }
      else
      {
         // Do nothing
      }

      return range_rate_thr;
   }

   /*===========================================================================*\
   * FUNCTION: Are_Det_And_Obj_Behind_The_Same_SEP()
   *===========================================================================
   * RETURN VALUE:
   * bool f_on_same_side
   *
   * PARAMETERS:
   * const uint8_t det_behind_sep_id
   * const uint8_t obj_behind_sep_id
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
   * Determine if detection and object are behind the same of static environment
   * polynomial (SEP).
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Are_Det_And_Obj_Behind_The_Same_SEP(
      const uint8_t det_behind_sep_id,
      const uint8_t obj_behind_sep_id)
   {
      const bool f_on_same_side = (det_behind_sep_id == obj_behind_sep_id);

      return f_on_same_side;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Det_Allowed_To_Associate()
   *===========================================================================
   * RETURN VALUE:
   * bool f_allowed_to_associate
   *
   * PARAMETERS:
   * const F360_Detection_Props_T & det_p,
   * const rspp_variant_A::RSPP_Detection_T &det_raw,
   * const F360_Object_Track_T & obj,
   * const F360_Calibrations_T & calib,
   * const BoundingBox(&sep_association_boxes)[F360_NUM_OF_STATIC_ENV_POLYS])
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
   * Check various conditions and determine if detection is allowed to associate
   * to an object.
   *
   * Conditions for association
   *   - Detections have to be ok to use
   *   - Detection can't be on a SEP if object is a moving CTCA object.
   *   - If object seems to be a mirror then ambiguous detections are not allowed to associate
   *   - Detection and object must be on the same side of a SEP if the object is a moving CTCA object (unless the
   *     the object is flagged as f_behind_SEP_ambiguous or far away from the SEP).
   *   - Stretchy track countermeasure is not needed
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Is_Det_Allowed_To_Associate(
      const F360_Detection_Props_T & det_p,
      const rspp_variant_A::RSPP_Detection_T &det_raw,
      const F360_Object_Track_T & obj,
      const F360_Calibrations_T & calib,
      const BoundingBox(&sep_association_boxes)[F360_NUM_OF_STATIC_ENV_POLYS])
   {
      const bool f_suspected_mirror_obj = obj.mirror_prob > calib.k_mirror_prob_threshold;
      const bool f_moving_ctca_or_cca_trk_detection_on_guardrail = ((det_p.on_sep_id != F360_INVALID_UNSIGNED_ID) && obj.f_moving && ((F360_TRACKER_TRKFLTR_CTCA == obj.trk_fltr_type) || (std::abs(obj.speed) > calib.fast_moving_thresh)));
      const bool f_suspected_mirror_obj_and_detection_ambiguous_motion = (f_suspected_mirror_obj && (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS == det_raw.processed.motion_status));
      const bool f_SEP_condition_fulfilled = Is_Association_Wrt_SEP_Allowed(det_p, obj, sep_association_boxes);

      bool f_is_bistatic_allowed_to_associate;
      if (det_raw.raw.f_bistatic)
      {
         if ((obj.trk_fltr_type == F360_TRACKER_TRKFLTR_CTCA) || (std::abs(obj.speed) > calib.fast_moving_thresh))
         {
            f_is_bistatic_allowed_to_associate = true;
         }
         else
         {
            f_is_bistatic_allowed_to_associate = false;
         }
      }
      else
      {
         f_is_bistatic_allowed_to_associate = true;
      }

      const bool f_allowed_to_associate = ((det_p.f_ok_to_use) &&
         (f_is_bistatic_allowed_to_associate) &&
         (!f_moving_ctca_or_cca_trk_detection_on_guardrail) &&
         (!f_suspected_mirror_obj_and_detection_ambiguous_motion) &&
         (f_SEP_condition_fulfilled));

      return f_allowed_to_associate;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Association_Wrt_SEP_Allowed()
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_association_allowed
   *
   * PARAMETERS:
   * const F360_Detection_Props_T & det_p,
   * const F360_Object_Track_T & obj,
   * const BoundingBox(&sep_association_boxes)[F360_NUM_OF_STATIC_ENV_POLYS]
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Determines if association between an object and detection is allowed w.r.t. SEPs.
   * - If the object is ambiguously behind the SEP, then association is allowed even if detection and object are on opposite side of the SEP.
   * - If detection and object are on the same side of the SEP, association is always allowed.
   * - Association across SEPs is always allowed for non-moving tracks.
   * - If object is far away from SEP (i.e. outside SEP association box), association is always allowed, regardless of object/detection behind SEP status.
   *
   * - If the object and detection are on opposite sides of a SEP, the object is moving and CTCA, not flagged as ambiguous and inside SEP association box,
   *   association is not allowed.
   *
   \*===========================================================================*/
   bool Is_Association_Wrt_SEP_Allowed(
      const F360_Detection_Props_T & det_p,
      const F360_Object_Track_T & obj,
      const BoundingBox(&sep_association_boxes)[F360_NUM_OF_STATIC_ENV_POLYS])
   {
      bool f_association_allowed;
      if (obj.f_moving)
      {
         if (obj.f_behind_sep_ambiguous)
         {
            f_association_allowed = true;
         }
         else
         {
            const bool f_obj_and_det_on_same_side_of_SEP = Are_Det_And_Obj_Behind_The_Same_SEP(det_p.behind_sep_id, obj.behind_sep_id);
            if (f_obj_and_det_on_same_side_of_SEP)
            {
               f_association_allowed = true;
            }
            else
            {
               const uint8_t sep_id_of_interest = obj.behind_sep_id != F360_INVALID_UNSIGNED_ID ? obj.behind_sep_id : det_p.behind_sep_id;
               const bool f_obj_in_SEP_zone = sep_association_boxes[sep_id_of_interest - 1U].Contains(Point(obj.bbox.Get_Center().x, obj.bbox.Get_Center().y));
               if (f_obj_in_SEP_zone)
               {
                  f_association_allowed = false;
               }
               else
               {
                  f_association_allowed = true;
               }
            }
         }
      }
      else
      {
         f_association_allowed = true;
      }
      return f_association_allowed;
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_SEP_Association_Boxes()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
   * const F360_Calibrations_T & calib,
   * BoundingBox(&sep_boxes)[F360_NUM_OF_STATIC_ENV_POLYS]
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculates an extented bounding box for each valid SEP in VCS. First, a bounding
   * is calculated with:
   * - the SEP's upper and lower limits as max/min longitudinal bounds
   * - min/max of the SEPs lateral position evaluated at either upper limit, lower limit or extreme point.
   * Then k_SEP_association_zone_extension is added to the box in all directions. If the SEPs second order
   * constant (p2) is close to zero, the min/max lat positions are evaluated at the longitudinal end points.
   *
   \*===========================================================================*/
   void Calculate_SEP_Association_Boxes(
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T & calib,
      BoundingBox(&sep_boxes)[F360_NUM_OF_STATIC_ENV_POLYS])
   {
      for (uint8_t sep_idx = 0U; sep_idx < F360_NUM_OF_STATIC_ENV_POLYS; sep_idx++)
      {
         if (F360_STATIC_ENV_POLY_STATUS_INVALID != static_env_polys[sep_idx].status)
         {
            const Static_Env_Poly_T & sep = static_env_polys[sep_idx];

            const float32_t lat_pos_lower = sep.Lateral_Pos_At(sep.lower_limit);
            const float32_t lat_pos_upper = sep.Lateral_Pos_At(sep.upper_limit);

            float32_t SEP_lat_min;
            float32_t SEP_lat_max;
            if (std::abs(sep.p2) < F360_EPSILON)
            {
               SEP_lat_min = std::min(lat_pos_lower, lat_pos_upper);
               SEP_lat_max = std::max(lat_pos_lower, lat_pos_upper);
            }
            else
            {
               const float32_t long_val_extreme_point = -sep.p1 / (2.0F * sep.p2);

               if ((long_val_extreme_point < sep.upper_limit) && (long_val_extreme_point > sep.lower_limit))
               {
                  const float32_t lat_pos_mid = sep.Lateral_Pos_At(long_val_extreme_point);
                  const float32_t possible_extreme_points[3] = { lat_pos_lower, lat_pos_mid, lat_pos_upper };
                  SEP_lat_min = F360_Min_Element(possible_extreme_points);
                  SEP_lat_max = F360_Max_Element(possible_extreme_points);
               }
               else
               {
                  SEP_lat_min = std::min(lat_pos_lower, lat_pos_upper);
                  SEP_lat_max = std::max(lat_pos_lower, lat_pos_upper);
               }
            }
            const float32_t bounding_box_extension = calib.k_SEP_association_zone_extension;

            const float32_t max_long = sep.upper_limit + bounding_box_extension;
            const float32_t min_long = sep.lower_limit - bounding_box_extension;
            const float32_t max_lat = SEP_lat_max + bounding_box_extension;
            const float32_t min_lat = SEP_lat_min - bounding_box_extension;
            const Point left_rear = Point(min_long, min_lat);
            const Point right_front = Point(max_long, max_lat);
            sep_boxes[sep_idx] = BoundingBox(left_rear, right_front);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Compare_Against_Stationary_Hypothesis()
   *===========================================================================
   * RETURN VALUE:
   * bool f_moving_hypothesis_significantly_better_than_stat
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T& sensor
   * const rspp_variant_A::RSPP_Detection_T& detection
   * const float32_t range_rate_threshold
   * const float32_t predicted_range_rate
   * const float32_t dealiased_rngrate
   * bool & f_inside_gate
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * This function calculates the difference between predicted and de-aliased range rate
   * for an assumed stationary object. This difference is compared to that of the real object
   * and if the stationary object fits better, and the difference is significant, association
   * to the real object is blocked.
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Compare_Against_Stationary_Hypothesis(
      const F360_Calibrations_T& calibrations,
      const F360_Radar_Sensor_T& sensor,
      const rspp_variant_A::RSPP_Detection_T& detection,
      const float32_t range_rate_threshold,
      const float32_t predicted_range_rate,
      const float32_t dealiased_rngrate
   )
   {
      // Calculate predicted range rate for an assumed stationary object
      const float32_t relative_velocity_x = -sensor.variable.vcs_velocity.longitudinal;
      const float32_t relative_velocity_y = -sensor.variable.vcs_velocity.lateral;
      const float32_t stat_predicted_range_rate = relative_velocity_x * detection.processed.cos_vcs_az + relative_velocity_y * detection.processed.sin_vcs_az;

      float32_t stat_dealiased_rngrate;
      float32_t stat_rr_interval;
      const bool f_stat_inside_gate = Try_To_Dealiase_Range_Rate(
         detection.raw.range_rate,
         stat_predicted_range_rate,
         range_rate_threshold,
         sensor.constant.v_wrapping[sensor.variable.look_id],
         sensor.constant.min_aliaised_range_rate[sensor.variable.look_id],
         stat_dealiased_rngrate,
         stat_rr_interval);

      bool f_moving_hypothesis_significantly_better_than_stat = true;
      if (f_stat_inside_gate) // Stationary hypothesis is inside rough range rate gate, compare with moving object's range rate diff
      {
         const float32_t moving_obj_rr_hypothesis_error = std::abs(predicted_range_rate - dealiased_rngrate);
         const float32_t stat_rr_hypothesis_error = std::abs(stat_predicted_range_rate - stat_dealiased_rngrate);

         const bool f_moving_hypothesis_better = (stat_rr_hypothesis_error > moving_obj_rr_hypothesis_error);
         const bool f_significant_hypothesis_difference = (std::abs(stat_rr_hypothesis_error - moving_obj_rr_hypothesis_error) > calibrations.k_min_rr_diff_from_stationary_hypothesis);

         f_moving_hypothesis_significantly_better_than_stat = (f_moving_hypothesis_better && f_significant_hypothesis_difference);
      }
      return f_moving_hypothesis_significantly_better_than_stat;
   }

   /*===========================================================================*\
   * FUNCTION: Collect_Detection_Candidates_For_Association()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const uint32_t number_of_valid_detections
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * const int32_t obj_idx
   * uint32_t (&det_idx_want_to_assoc)[MAX_DETS_IN_OBJ_TRK]
   * uint16_t & num_dets_want_to_assoc
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function goes through all the detections and collects the indices of all detections that want
   * to associate to the specified object and counts how many detections that want to associate to the object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static void Collect_Detection_Candidates_For_Association(
      const uint32_t number_of_valid_detections,
      const F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      const int32_t obj_idx,
      uint32_t (&det_idx_want_to_assoc)[MAX_DETS_FOR_SINGLE_SENSOR],
      uint16_t & num_dets_want_to_assoc)
   {
      for (uint32_t det_i = 0U; det_i < number_of_valid_detections; det_i++)
      {
         if (detection_props[det_i].object_track_id == (obj_idx + 1))
         {
            // Detection wants to associate to object, store in temporary array
            if (num_dets_want_to_assoc < MAX_DETS_FOR_SINGLE_SENSOR)
            {
               det_idx_want_to_assoc[num_dets_want_to_assoc] = det_i;
               num_dets_want_to_assoc++;
            }
            else
            {
               break;
            }

         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Assign_Association_Hypothesis()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const uint32_t number_of_valid_detections
   * const float32_t det_rdot_comp_array[MAX_NUMBER_OF_DETECTIONS]
   * const F360_Tracker_Info_T & tracker_info
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function loops over all valid objects and associates detections to objects.
   * For every detection that associates to an object, the detection's compensated range rate property
   * is updated and the flag f_dealiased is set to true.
   * If more than maximum allowed detections in an object wants to associate to an object,
   * a cost is calculated for each detection and the detections with the lowest costs are associated.
   * The leftover detections are set as not ok to use.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Assign_Association_Hypothesis(
      const float32_t(&det_rdot_comp_array)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Tracker_Info_T & tracker_info,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      for (int32_t obj_i = 0; obj_i < tracker_info.num_active_objs; obj_i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[obj_i] - 1;

         uint16_t num_dets_want_to_assoc = 0U;
         uint32_t det_idx_want_to_assoc[MAX_DETS_FOR_SINGLE_SENSOR];

         Collect_Detection_Candidates_For_Association(raw_detect_list.number_of_valid_detections, detection_props, obj_idx, det_idx_want_to_assoc, num_dets_want_to_assoc);

         if (num_dets_want_to_assoc > tracker_info.variant.num_dets_in_track)
         {
            // Calculate a detection cost for all detections and choose the best ones
            float32_t temp_cost_array[MAX_DETS_FOR_SINGLE_SENSOR];

            for (uint16_t det_i = 0U; det_i < num_dets_want_to_assoc; det_i++)
            {
               const uint32_t det_idx = det_idx_want_to_assoc[det_i];
               temp_cost_array[det_i] = Calculate_Detection_Association_Cost(detection_props[det_idx], object_tracks[obj_idx]);
            }

            uint32_t perm[MAX_DETS_FOR_SINGLE_SENSOR];
            (void)F360_Sort(temp_cost_array, static_cast<uint32_t>(num_dets_want_to_assoc), true, perm);

            // Associate the MAX_DETS_IN_OBJ_TRK detections with lowest cost
            for (uint32_t i = 0U; i < tracker_info.variant.num_dets_in_track; i++)
            {
               const uint32_t det_idx = det_idx_want_to_assoc[perm[i]];

               // Return value is redundant, all detections will associate correctly as Associate_Detection_To_Object()
               // is called MAX_DETS_IN_OBJ_TRK times and object is valid.
               (void)Associate_Detection_To_Object(tracker_info, raw_detect_list.detections[det_idx], object_tracks[obj_idx], detection_props[det_idx], (det_idx + 1U));
               detection_props[det_idx].range_rate_compensated = det_rdot_comp_array[det_idx];
               detection_props[det_idx].f_dealiased = true;
            }

            // Discard the leftover detections
            for (uint32_t i = tracker_info.variant.num_dets_in_track; i < num_dets_want_to_assoc; i++)
            {
               const uint32_t det_idx = det_idx_want_to_assoc[perm[i]];
               detection_props[det_idx].f_ok_to_use = false;
               detection_props[det_idx].object_track_id = 0;
            }
         }
         else
         {
            // All detections that want to associate fit in object
            for (uint32_t i = 0U; i < num_dets_want_to_assoc; i++)
            {
               const uint32_t det_idx = det_idx_want_to_assoc[i];

               // Return value is redundant, all detections will associate correctly as Associate_Detection_To_Object()
               // is called less than MAX_DETS_IN_OBJ_TRK times and object is valid.
               (void)Associate_Detection_To_Object(tracker_info, raw_detect_list.detections[det_idx], object_tracks[obj_idx], detection_props[det_idx], (det_idx + 1U));
               detection_props[det_idx].range_rate_compensated = det_rdot_comp_array[det_idx];
               detection_props[det_idx].f_dealiased = true;

            }
         }
      }
   }

/*===========================================================================*\
* FUNCTION: Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox()
*===========================================================================
* RETURN VALUE:
* float32_t position_score
*
* PARAMETERS:
* const Point& det_position_vcs
* const F360_Object_Track_T& object_track,
* const F360_Calibrations_T& calib
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* Calculates the score of detection based on the position of the detection
* in the bounding box. In the bbox shown belowm it is divided into zones numbered
* 1,2,3,4 and the front of the bbox, is the bbox edge that lies in zone 1.
* For Zones numbered 1 and 3, the score is calculated considering the distance
* of the detection to  the shortest edge and in zones numbered 2 and 4 the
* score is calculated considering the distance of the detection to the longest edge.

  FRONT
+--------+
|\  1  / |
| \   /  |
|  \ /   |
| 4 X  2 |
|  / \   |
| /   \  |
|/   3 \ |
+--------+

*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
\*===========================================================================*/

   float32_t Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(
      const Point& det_position_vcs,
      const F360_Object_Track_T& object_track,
      const F360_Calibrations_T& calib)
   {
      float32_t position_score = calib.k_base_score_bbox_center;

      /* Transform detection in to the objects reference frame TCS */
      Point detection_in_tcs = det_position_vcs;
      detection_in_tcs.Transform_To_Relative_Coordinate_System(object_track.bbox.Get_Center(), object_track.bbox.Get_Orientation());
      const BoundingBox bbox_in_tcs = BoundingBox(Point(0.0F,0.0F), object_track.bbox.Get_Length(), object_track.bbox.Get_Width(), Angle(0.0F));

      const float32_t distance_from_det_to_bbox_center_along_tcs_x = std::fabs(detection_in_tcs.x);
      const float32_t distance_from_det_to_bbox_center_along_tcs_y = std::fabs(detection_in_tcs.y);
      const float32_t slope_from_det_to_bbox_center = bbox_in_tcs.Get_Slope_from_bbox_center_to_point(detection_in_tcs);
      const float32_t slope_of_diagonal_of_bbox = bbox_in_tcs.Get_Slope_Of_Diagonal();
      const float32_t abs_slope_from_det_to_bbox_center = std::fabs(slope_from_det_to_bbox_center);
      const float32_t abs_slope_of_diagonal_of_bbox = std::fabs(slope_of_diagonal_of_bbox);

      /*Check the slope of the detction to the center and compare it the slope of the bbox diagonal,
      which indicates the zone the detection belongs to and then normalize its distance with the length or width of the bbox*/
      if (abs_slope_from_det_to_bbox_center >= abs_slope_of_diagonal_of_bbox)
      {
         position_score = calib.k_base_score_bbox_center * (1.0F - (distance_from_det_to_bbox_center_along_tcs_y / (0.5F*bbox_in_tcs.Get_Width())));
      }
      else
      {
         position_score = calib.k_base_score_bbox_center *(1.0F - (distance_from_det_to_bbox_center_along_tcs_x / (0.5F*bbox_in_tcs.Get_Length())));
      }

      return position_score;
   }
   /*===========================================================================*\
   * FUNCTION: Calculate_Final_Det_Score_Inside_Solid_Bbox()
   *===========================================================================
   * RETURN VALUE:
   * float32_t final_score
   *
   * PARAMETERS:
   * const Point & det_position_vcs
   * const F360_Object_Track_T & object_track
   * const F360_Calibrations_T & calib
   * const float32_t range_rate_diff
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate final detection score for detections positioned inside object bounding box
   * based on the position and range rate of the object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calculate_Final_Det_Score_Inside_Solid_Bbox(
      const Point & det_position_vcs,
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calib,
      const float32_t range_rate_diff)
   {
      const float32_t score_based_on_det_position = Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(det_position_vcs, object_track, calib);
      float32_t weights[3];
      weights[0] = calib.k_para_diff_weight_inside_box;
      weights[1] = calib.k_orth_diff_weight_inside_box;
      weights[2] = calib.k_rdot_diff_weight_inside_box;

      const float32_t final_score = (weights[0] + weights[1]) * score_based_on_det_position + weights[2] * (std::fabs(range_rate_diff / calib.k_range_rate_score_threshold));
      return final_score;
   }


/*===========================================================================*\
* FUNCTION: Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox()
*===========================================================================
* RETURN VALUE:
* float32_t position_score
*
* PARAMETERS:
* const Point& det_position_vcs
* const F360_Object_Track_T& object_track,
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* Calculates the score of detection based on the position of the detection
* outside the bounding box but inside the extended bounding box. For the bbox and
* extended bbox shown below, various zones are numbered between 1 and 12. The front
* of the bbox is the edge in zone 2. The front of the extended bbox is the edge which
* is in the zone 9,6,10. For detections in zone5 and zone 7
* the distance from the longest edge of the bbox is considered for scoring.
* For detections in zone 6 and 8 the distance from the shortest edge of the bbox
* is considered for scoring. For detections in zone 9, 10, 11, 12
* where it is not obvious which edge to use to calculate the score
* we draw a diagonal from the vertex of the bbox to the closest corresponding
* vertex of the extended bbox and the detections below the diagonal
* are scored based on the distance from the longest edge
* and the detections above the diagonal are scored based on the distance
* to the shortest edge.
*

       FRONT
%%%%%%%%%%%%%%%%%%%%%%
%     |        |      %
%   9 |   6    | 10   %
%     |        |      %
%-----+--------+------%
%     |\      /|      %
%     | \  2 / |      %
%     |  \  /  |      %
%   5 | 1 \/  3| 7    %
%     |   /\   |      %
%     |  /  \  |      %
%     | /  4 \ |      %
%     |/      \|      %
%-----+--------+------%
%     |        |      %
% 12  |   8    |   11 %
%     |        |      %
%%%%%%%%%%%%%%%%%%%%%%


*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
\*===========================================================================*/

   float32_t Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(
      const Point& det_position_vcs,
      const F360_Object_Track_T& object_track)
   {
      float32_t position_score = 1.0F;
      float32_t length_of_obj_buffer_zone_along_tcs_x = 1.0F;
      float32_t width_of_obj_buffer_zone_along_tcs_y = 1.0F;

      /* Transform detection in to the objects reference frame TCS */
      Point detection_in_tcs = det_position_vcs;
      detection_in_tcs.Transform_To_Relative_Coordinate_System(object_track.bbox.Get_Center(), object_track.bbox.Get_Orientation());

      /*Create a BoundingBox instance in tcs frame*/
      const BoundingBox bbox_in_tcs = BoundingBox(Point(0.0F,0.0F), object_track.bbox.Get_Length(), object_track.bbox.Get_Width(), Angle(0.0F));

      if (detection_in_tcs.x < 0.0F)
      {
         length_of_obj_buffer_zone_along_tcs_x = object_track.long_buffer_zone_len1;
      }
      else
      {
         length_of_obj_buffer_zone_along_tcs_x = object_track.long_buffer_zone_len2;
      }
      if ( detection_in_tcs.y < 0.0F)
      {
         width_of_obj_buffer_zone_along_tcs_y = object_track.lat_buffer_zone_wid1;
      }
      else
      {
         width_of_obj_buffer_zone_along_tcs_y = object_track.lat_buffer_zone_wid2;
      }

      const float32_t bbox_half_width = 0.5F * bbox_in_tcs.Get_Width();
      const float32_t bbox_half_length = 0.5F * bbox_in_tcs.Get_Length();

      const bool check_less_than_half_width = std::fabs( detection_in_tcs.y) <= bbox_half_width;
      const bool check_greater_than_half_length = std::fabs(detection_in_tcs.x) >= bbox_half_length;
      const bool check_greater_than_half_width = std::fabs( detection_in_tcs.y) > bbox_half_width;
      const bool check_less_than_half_length = std::fabs(detection_in_tcs.x) < bbox_half_length;

      const float32_t dist_to_detection_from_bbox_edge_along_tcs_x = std::fabs(detection_in_tcs.x) - bbox_half_length;
      const float32_t dist_to_detection_from_bbox_edge_along_tcs_y = std::fabs( detection_in_tcs.y) - bbox_half_width;

      /*If the detection is behind or in front of the bbox, score based on the distance to the distance to the edge along tcs x*/
      if (check_less_than_half_width && check_greater_than_half_length)
      {
         position_score = dist_to_detection_from_bbox_edge_along_tcs_x / length_of_obj_buffer_zone_along_tcs_x;
      }
      /*If the detection is to the left or right of the bbox, score based on the distance to the edge along tcs y*/
      else if (check_greater_than_half_width && check_less_than_half_length)
      {
         position_score = dist_to_detection_from_bbox_edge_along_tcs_y / width_of_obj_buffer_zone_along_tcs_y;
      }
      /* Refer to the figure in the Abstract of the function, if the detection lies in zone 9,10,11,12
      then we consider scoring based on the diagonals of the vertex of the bbox and the closest corresponding vertex of the extended bbox */
      else
      {
         const Point closest_corner = bbox_in_tcs.Get_Closest_Corner_To_Point(detection_in_tcs);
         const float32_t slope_of_detection_to_closest_bbox_corner = Point::get_slope_between_points(closest_corner, detection_in_tcs);
         const float32_t slope_of_diagonal_of_corner_rectangle = width_of_obj_buffer_zone_along_tcs_y / length_of_obj_buffer_zone_along_tcs_x;

         if (std::fabs(slope_of_detection_to_closest_bbox_corner) >= std::fabs(slope_of_diagonal_of_corner_rectangle))
         {
            position_score = dist_to_detection_from_bbox_edge_along_tcs_y / width_of_obj_buffer_zone_along_tcs_y;
         }
         else
         {
            position_score = dist_to_detection_from_bbox_edge_along_tcs_x / length_of_obj_buffer_zone_along_tcs_x;
         }
      }

      return position_score;
   }
   /*===========================================================================*\
   * FUNCTION: Calculate_Final_Det_Score_Inside_Extended_Bbox()
   *===========================================================================
   * RETURN VALUE:
   * float32_t final_score
   *
   * PARAMETERS:
   * const Point & det_position_vcs
   * const F360_Object_Track_T & object_track
   * const F360_Calibrations_T & calib
   * const float32_t range_rate_diff
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate final detection score for detections positioned inside object extended
   * bounding box, using detection position and range rate.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calculate_Final_Det_Score_Inside_Extended_Bbox(
      const Point & det_position_vcs,
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calib,
      const float32_t range_rate_diff)
   {

      float32_t weights[3];
      weights[0] = calib.k_para_diff_weight_inside_ext_box;
      weights[1] = calib.k_orth_diff_weight_inside_ext_box;
      weights[2] = calib.k_rdot_diff_weight_inside_ext_box;

      const float position_score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(det_position_vcs, object_track);
      const float final_score = (weights[0] + weights[1]) * position_score + weights[2] * (std::fabs(range_rate_diff) / calib.k_range_rate_score_threshold);

      return final_score;
   }


   /*===========================================================================*\
   * FUNCTION: Calculate_Final_Det_Score_Inside_Solid_Circle()
   *===========================================================================
   * RETURN VALUE:
   * float32_t score
   *
   * PARAMETERS:
   * const Point & det_position_vcs
   * const F360_Object_Track_T & object_track
   * const F360_Calibrations_T & calib
   * const float32_t range_rate_diff
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate detection score for detections positioned inside the outline of a
   * non-moveable object, i.e inside the solid object circle.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calculate_Final_Det_Score_Inside_Solid_Circle(
      const Point& det_position_vcs,
      const F360_Object_Track_T& object_track,
      const F360_Calibrations_T& calib,
      const float32_t range_rate_diff)
   {
      const float32_t dist = object_track.bbox.Distance_To_Center(det_position_vcs);
      const float32_t normalized_dist = dist / (calib.k_nonmoveable_target_diameter * 0.5F);
      const float32_t normalized_rr_diff = range_rate_diff / calib.k_range_rate_score_threshold;

      const float32_t position_score = calib.k_base_score_bbox_center*(1.0F - std::fabs(normalized_dist));

      const float32_t score = (calib.k_dist_weight_inside_solid_circle * position_score +
         calib.k_rdot_diff_weight_inside_solid_circel *std::fabs(normalized_rr_diff));

      return score;
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Final_Det_Score_Inside_Extended_Circle()
   *===========================================================================
   * RETURN VALUE:
   * float32_t score
   *
   * PARAMETERS:
   * const Point & det_position_vcs
   * const F360_Object_Track_T & object_track
   * const F360_Calibrations_T & calib
   * const float32_t range_rate_diff
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate detection score for detections positioned inside the extened
   * circle that constitutes the position association gate of a non-moveable object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calculate_Final_Det_Score_Inside_Extended_Circle(
      const Point& det_position_vcs,
      const F360_Object_Track_T& object_track,
      const F360_Calibrations_T& calib,
      const float32_t range_rate_diff)
   {
      const float32_t dist = object_track.bbox.Distance_To_Center(det_position_vcs);
      const float32_t bbox_radius = object_track.bbox.Get_Length() * 0.5F;
      const float32_t normalized_dist = (dist-bbox_radius) / (object_track.long_buffer_zone_len1);
      const float32_t normalized_rr_diff = range_rate_diff / calib.k_range_rate_score_threshold;

      const float32_t score =  calib.k_dist_weight_inside_ext_circle * std::fabs(normalized_dist) +
         calib.k_rdot_diff_weight_inside_ext_circel * std::fabs(normalized_rr_diff);

      return score;
   }


   /*===========================================================================*\
   * FUNCTION: Calculate_Detection_Association_Cost()
   *===========================================================================
   * RETURN VALUE:
   * float32_t det_cost - The detection cost
   *
   * PARAMETERS:
   * const F360_Detection_Props_T & det_prop - Detection property
   * const F360_Object_Track_T & obj_trk     - Object track
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates a cost for a detection in order to be able to prioritize
   * what detections to associate to a track. The function calculates the distance from
   * the detection to each edge and uses the distance to the closest edge as cost.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   float32_t Calculate_Detection_Association_Cost(
      const F360_Detection_Props_T & det_prop,
      const F360_Object_Track_T & obj_trk)
   {
      float32_t det_cost = 0.0F;

      if (obj_trk.f_moveable)
      {
         Point det_tcs;

         Convert_VCS_Posn_To_TCS_Posn(
            det_prop.vcs_position.x,
            det_prop.vcs_position.y,
            obj_trk.bbox.Get_Center().x,
            obj_trk.bbox.Get_Center().y,
            obj_trk.bbox.Get_Orientation(),
            det_tcs.x,
            det_tcs.y);

         const float32_t half_length = 0.5F * obj_trk.bbox.Get_Length();
         const float32_t half_width = 0.5F * obj_trk.bbox.Get_Width();

         const Point tcs_front_left = { half_length, -half_width };
         const Point tcs_front_right = { half_length, half_width };
         const Point tcs_rear_left = { -half_length, -half_width };
         const Point tcs_rear_right = { -half_length, half_width };

         const float32_t dist_front = Calculate_Distance_To_Edge(tcs_front_left, tcs_front_right, det_tcs);
         const float32_t dist_left = Calculate_Distance_To_Edge(tcs_front_left, tcs_rear_left, det_tcs);
         const float32_t dist_rear = Calculate_Distance_To_Edge(tcs_rear_left, tcs_rear_right, det_tcs);
         const float32_t dist_right = Calculate_Distance_To_Edge(tcs_front_right, tcs_rear_right, det_tcs);

         // Find shortest distance to one of the edges and use that as cost for detection
         const float32_t shortest_dist_para = std::min(dist_front, dist_rear);
         const float32_t shortest_dist_orth = std::min(dist_left, dist_right);
         det_cost = std::min(shortest_dist_para, shortest_dist_orth);
      }
      else
      {
         det_cost = obj_trk.bbox.Distance_To_Center(det_prop.vcs_position);
      }

      return det_cost;
   }

}
