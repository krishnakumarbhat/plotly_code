/*===========================================================================*\
* FILE: f360_mark_for_liberal_tracking.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains functions definitions for marking objects/detections for liberal tracking.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "f360_mark_for_liberal_tracking.h"
#include "rspp_detection_motion_status.h"
#include "f360_math.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Mark_Detection_For_Liberal_Tracking()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calibrations - F360 tracker calibrations
   * const float32_t host_speed - host speed
   * const rspp_variant_A::RSPP_Detection_T &detection,
   * F360_Detection_Props_T &detection_prop - processed detection props
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * Marks detection with liberal tracking flag.
   *
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Mark_Detection_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const float32_t host_speed,
      const rspp_variant_A::RSPP_Detection_T &detection,
      F360_Detection_Props_T &detection_prop)
   {
      if (calibrations.liberal_tracking_min_host_speed < host_speed)
      {
         const bool det_is_moving = detection.processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
         const bool det_below_rrate_thr = detection_prop.range_rate_dealiased < calibrations.liberal_tracking_dets_rrate_thr;
         const bool det_not_in_front = detection_prop.vcs_position.x < 0.0F;
         const bool det_should_be_checked = ((det_is_moving) &&
            (det_below_rrate_thr) &&
            (det_not_in_front) &&
            (!detection_prop.f_stationary_bounce));

         if (det_should_be_checked)
         {
            const bool valid_for_liberal_tracking = Check_If_Point_Inside_Trapezoid_Zone(
               calibrations.liberal_tracking_trapezoid_zone_top_lon_pos,
               calibrations.liberal_tracking_trapezoid_zone_top_len,
               calibrations.liberal_tracking_trapezoid_zone_bot_lon_pos,
               calibrations.liberal_tracking_trapezoid_zone_bot_len,
               detection_prop.vcs_position.x,
               detection_prop.vcs_position.y);

            detection_prop.f_valid_for_liberal_tracking = valid_for_liberal_tracking;
            detection_prop.f_ok_to_use = (detection_prop.f_ok_to_use) || (valid_for_liberal_tracking);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Mark_Detections_For_Liberal_Tracking()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calibrations - F360 tracker calibrations
   * const float32_t host_speed - host speed
   * const uint32_t number_of_valid_detections -  number of valid detections
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * F360_Detection_Props_T &detection_prop - detections props
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * Marks detections with liberal tracking flag.
   *
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Mark_Detections_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const float32_t host_speed,
      const uint32_t number_of_valid_detections,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&detections_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      for (uint32_t det_idx = 0U; det_idx < number_of_valid_detections; det_idx++)
      {
         F360_Detection_Props_T &current_detection_prop = detections_props[det_idx];
         Mark_Detection_For_Liberal_Tracking(calibrations, host_speed, raw_detect_list.detections[det_idx], current_detection_prop);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Mark_Objects_For_Liberal_Tracking()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calibrations - F360 tracker calibrations
   * const F360_Host_T &host - host data
   * const F360_Tracker_Info_T& tracker_info,
   * F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS] - tracked objects array
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * Marks objects with liberal tracking flag.
   *
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Mark_Objects_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      const F360_Tracker_Info_T& tracker_info,
      F360_Object_Track_T (&objects)[NUMBER_OF_OBJECT_TRACKS])
   {
      if (calibrations.liberal_tracking_min_host_speed < host.speed)
      {
         for (uint16_t object_index = 0U; object_index < tracker_info.variant.num_tracks; object_index++)
         {
            F360_Object_Track_T& object = objects[object_index];
            const bool object_status_not_invalid = object.status != F360_OBJECT_STATUS_INVALID;

            if (object_status_not_invalid)
            {
               const bool obj_is_moving = object.f_moving;
               const bool obj_not_in_front = object.vcs_position.x < 0.0F;
               const bool obj_should_be_checked_level_1 = (obj_is_moving && obj_not_in_front);

               if (obj_should_be_checked_level_1)  // 1st level of discrimination
               {
                  const float32_t obj_rel_velocity_longitudinal = object.vcs_velocity.longitudinal - host.vcs_speed;
                  const bool obj_approaching_host_fast = calibrations.liberal_tracking_obj_relative_velocity_lon_thr < obj_rel_velocity_longitudinal;
                  const float32_t heading_thr = calibrations.liberal_tracking_obj_heading_thr;
                  const bool obj_heading_towards_host = std::abs(object.vcs_heading.Value()) < heading_thr;
                  const bool obj_should_be_checked_level_2 = (obj_approaching_host_fast && obj_heading_towards_host);

                  if (obj_should_be_checked_level_2)  // 2nd level of discrimination
                  {
                     object.f_valid_for_liberal_tracking = Check_If_Point_Inside_Trapezoid_Zone(
                        calibrations.liberal_tracking_trapezoid_zone_top_lon_pos,
                        calibrations.liberal_tracking_trapezoid_zone_top_len,
                        calibrations.liberal_tracking_trapezoid_zone_bot_lon_pos,
                        calibrations.liberal_tracking_trapezoid_zone_bot_len,
                        object.vcs_position.x,
                        object.vcs_position.y);
                  }
               }
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Cluster_For_Liberal_Tracking()
   * ===========================================================================
   * RETURN VALUE:
   * bool cluster_valid_for_liberal_tracking                    - indicate whether cluster should be processed with liberal tracking conditions
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calibrations                              - calibrations structure
   * const F360_Host_T &host                                              - host properties
   * const F360_Detection_Props_T (&detections)[MAX_NUMBER_OF_DETECTIONS] - detections array
   * const F360_Cluster_T &cluster                                        - cluster to check
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * Check if cluster is valid for liberal tracking processing. If any detection
   * is marked as valid for liberal tracking, then function returns true. If not - function returns false. 
   *
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Check_Cluster_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      const F360_Detection_Props_T (&detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Cluster_T &cluster) 
   {
      bool cluster_valid_for_liberal_tracking = false;
      if (calibrations.liberal_tracking_min_host_speed < host.speed)
      {
         const uint16_t dets_count_in_cluster = static_cast<uint16_t>(cluster.ndets);
         for (uint16_t det_in_cluster_index = 0U; det_in_cluster_index < dets_count_in_cluster; det_in_cluster_index++)
         {
            const uint16_t detection_index = static_cast<uint16_t>(cluster.detids[det_in_cluster_index]) - 1U;
            if (detections[detection_index].f_valid_for_liberal_tracking)
            {
               cluster_valid_for_liberal_tracking = true;
               break;
            }
         }
      }
      return cluster_valid_for_liberal_tracking;
   }

   /*===========================================================================*\
   * FUNCTION: Check_If_Point_Inside_Trapezoid_Zone()
   * ===========================================================================
   * RETURN VALUE:
   * bool point_inside_trapezoid_zone   - True if point is inside trapezoid zone
   *
   * PARAMETERS:
   * const float32_t trapezoid_top_lon_pos     - longitudinal position of top edge of trapezoid
   * const float32_t trapezoid_top_len         - length of top edge of trapezoid
   * const float32_t trapezoid_bot_lon_pos     - longitudinal position of bottom edge of trapezoid
   * const float32_t trapezoid_bot_len         - length of bottom edge of trapezoid
   * const float32_t point_lon_pos             - longitudinal position of point to check
   * const float32_t point_lat_pos             - lateral position of point to check
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * Check if given point is inside trapezoid zone. It is assumed that zone is symetrical around VCS X axis and
   * top and bottom edges are parallel to VCS Y axis.
   * --------------------------------------------------------------------------
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Check_If_Point_Inside_Trapezoid_Zone(
      const float32_t trapezoid_top_lon_pos,
      const float32_t trapezoid_top_len,
      const float32_t trapezoid_bot_lon_pos,
      const float32_t trapezoid_bot_len,
      const float32_t point_lon_pos,
      const float32_t point_lat_pos)
   {
      bool point_inside_trapezoid_zone = false; //Initial value
      const bool is_between_top_bottom_edges = (trapezoid_bot_lon_pos < point_lon_pos) && (point_lon_pos < trapezoid_top_lon_pos);

      if (is_between_top_bottom_edges)
      {
         const float32_t top_len_half = trapezoid_top_len * 0.5F;
         const float32_t bot_len_half = trapezoid_bot_len * 0.5F;

         const float32_t lateral_max = trapezoid_top_len < trapezoid_bot_len ? bot_len_half : top_len_half;
         const bool is_between_left_right_borders = (-lateral_max < point_lat_pos) && (point_lat_pos < lateral_max);

         if (is_between_left_right_borders)
         {
            // calculate right edge slope
            float32_t border_slope = (trapezoid_bot_lon_pos - trapezoid_top_lon_pos) / (bot_len_half - top_len_half);
            const float32_t border_offset = trapezoid_top_lon_pos - border_slope*top_len_half;   // b = y - ax
            
            const bool point_on_right_side = 0.0F < point_lat_pos;
            if (!point_on_right_side)
            {
               border_slope = -border_slope;  // left edge slope is negative of right edge slope -> assume that trapezoid zone is symetrical
            }

            const float32_t point_intersect_with_border_lon = border_slope * point_lat_pos + border_offset;   // y = ax + b
            const bool check_below_border = trapezoid_top_len < trapezoid_bot_len;   // check below or over the border
            if (check_below_border)
            {
               point_inside_trapezoid_zone = point_lon_pos < point_intersect_with_border_lon;
            }
            else
            {
               point_inside_trapezoid_zone = point_intersect_with_border_lon < point_lon_pos;
            }
         }
      }
      return point_inside_trapezoid_zone;
   }
}
