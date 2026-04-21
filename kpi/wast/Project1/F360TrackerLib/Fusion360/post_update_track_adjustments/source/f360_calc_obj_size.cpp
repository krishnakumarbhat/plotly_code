/*===========================================================================*\
* FILE: f360_calc_obj_size.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Calc_Obj_Size()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/


#include <algorithm>

#include "f360_calc_obj_size.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_math.h"
#include "f360_math_func.h"
#include "f360_get_reference_point_para_side.h"
#include "f360_get_reference_point_orth_side.h"
#include "f360_iterator.h"
#include "f360_functional.h"

namespace f360_variant_A
{
   static void Limit_Size_And_Enforce_CTCA_Obj_Aspect_Ratio(
      const F360_Calibrations_T& calib,
      const Dimension_Limits_T& dim_limits,
      F360_Object_Track_T& obj);

   /*===========================================================================*\
   * FUNCTION: Calc_Obj_Size()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * onst F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T& detection_list
   * const F360_Calibrations_T& calib
   * float32_t& measured_length
   * float32_t& measured_width
   * F360_Object_Track_T& object_track
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
   * Calculates object size.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calc_Obj_Size(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& detection_list,
      const F360_Calibrations_T& calib,
      float32_t& measured_length,
      float32_t& measured_width,
      F360_Object_Track_T& object_track)
   {
      if (!object_track.f_moveable)
      {
         object_track.bbox.Set_Length(calib.k_nonmoveable_target_diameter);
         object_track.bbox.Set_Width(calib.k_nonmoveable_target_diameter);
      }
      else
      {
         // Determine minimum and maximum dimensions of object depending on filter type and movement status
         Dimension_Limits_T obj_dim_limits;
         Determine_Min_Dimensions_For_Object(object_track, calib, obj_dim_limits.length.minimum, obj_dim_limits.width.minimum);
         Determine_Max_Dimensions_For_Object(object_track, calib, obj_dim_limits.length.maximum, obj_dim_limits.width.maximum);

         /*
         Find extents of associated detections to get size measurements. Find min and max para and orth coordinates by iterating 
         through detections and transforming them to TCS coordinates.
         Initialize variables with with help of first detection (we know number of detections is larger than 2 so there are at least 
         one that we can use for initilization) and then iterate through rest of detections. 
         */
         const float32_t cos_pointing = object_track.bbox.Get_Orientation().Cos();
         const float32_t sin_pointing = object_track.bbox.Get_Orientation().Sin();
         const uint32_t first_det_idx = object_track.detids[0] - 1U;
         float32_t min_para = cos_pointing * det_props[first_det_idx].vcs_position.x + sin_pointing * det_props[first_det_idx].vcs_position.y;
         float32_t max_para = min_para;
         float32_t min_orth = -sin_pointing * det_props[first_det_idx].vcs_position.x + cos_pointing * det_props[first_det_idx].vcs_position.y;
         float32_t max_orth = min_orth;
         const float32_t first_det_range = detection_list.detections[first_det_idx].raw.range;
         bool f_range_criterea_met = (first_det_range > calib.k_size_update_min_det_range);
         for (uint32_t i = 1U; i < object_track.ndets; i++)
         {
            const uint32_t det_idx = object_track.detids[i] - 1U;

            // Project the detections on para/orth axes of the object and find minimum and maximum
            const float32_t det_para = cos_pointing * det_props[det_idx].vcs_position.x + sin_pointing * det_props[det_idx].vcs_position.y;
            const float32_t det_orth = -sin_pointing * det_props[det_idx].vcs_position.x + cos_pointing * det_props[det_idx].vcs_position.y;

            min_para = (det_para < min_para) ? det_para : min_para;
            max_para = (det_para > max_para) ? det_para : max_para;
            min_orth = (det_orth < min_orth) ? det_orth : min_orth;
            max_orth = (det_orth > max_orth) ? det_orth : max_orth;

            const float32_t curr_det_range = detection_list.detections[det_idx].raw.range;
            f_range_criterea_met = ((curr_det_range > calib.k_size_update_min_det_range) && f_range_criterea_met);
         }

         // Check if left/right or front/rear are visible
         const F360_Object_Sides_T visible_side = Get_Reference_Point_Orth_Side(object_track.reference_point);
         const bool f_left_or_right_visible = (visible_side != F360_OBJECT_SIDES_INVALID);
         const F360_Object_Sides_T visible_front_rear = Get_Reference_Point_Para_Side(object_track.reference_point);
         const bool f_front_or_rear_visible = (visible_front_rear != F360_OBJECT_SIDES_INVALID);

         const bool f_ok_to_update_non_visible_side = ((object_track.speed > calib.k_size_update_min_speed_to_update_nonvisible_side) && f_range_criterea_met);

         // Update length
         measured_length = F360_Saturate(max_para - min_para, obj_dim_limits.length.minimum, obj_dim_limits.length.maximum);

         if (f_left_or_right_visible || f_ok_to_update_non_visible_side)
         {
            float32_t process_noise = std::abs(object_track.speed) < calib.k_size_update_speed_threshold_low_speed_process_noise ? calib.k_size_update_low_speed_process_noise : calib.k_size_update_base_process_noise;
            float32_t measurement_uncertainty = calib.k_size_update_base_measurement_uncertainty;
            
            // If neither side is visible, make filter slower by reducing process noise
            if (!f_left_or_right_visible)
            {
               process_noise = process_noise * calib.k_size_update_process_noise_pruning;
            }

            if ((object_track.ndets > calib.k_min_num_dets_to_decrease_meas_uncertainty) && (object_track.bbox.Get_Length() < (max_para - min_para)))
            {
               // Make measurement more certain if there are lots of dets, eg. trucks and object needs to grow
               const uint32_t n_dets_above_min_nbr = object_track.ndets - calib.k_min_num_dets_to_decrease_meas_uncertainty;
               const float32_t ndet_scale = 1.0F / std::min(static_cast<float32_t>(n_dets_above_min_nbr), static_cast<float32_t>(calib.k_min_num_dets_to_decrease_meas_uncertainty));
               measurement_uncertainty *= ndet_scale * ndet_scale;
            }
            // Kalman filter for length estimation
            object_track.length_uncertainty += process_noise;
            const float32_t gain = object_track.length_uncertainty / (object_track.length_uncertainty + measurement_uncertainty);
            const float32_t updated_length = object_track.bbox.Get_Length() + gain * (measured_length - object_track.bbox.Get_Length());
            object_track.length_uncertainty = (1.0F - gain) * object_track.length_uncertainty;

            object_track.bbox.Set_Length(updated_length);
         }

         // Update width
         measured_width = F360_Saturate(max_orth - min_orth, obj_dim_limits.width.minimum, obj_dim_limits.width.maximum);

         if (f_front_or_rear_visible || f_ok_to_update_non_visible_side)
         {
            float32_t process_noise = std::abs(object_track.speed) < calib.k_size_update_speed_threshold_low_speed_process_noise ? calib.k_size_update_low_speed_process_noise : calib.k_size_update_base_process_noise;
            const float32_t measurement_uncertainty = calib.k_size_update_base_measurement_uncertainty;
            // If neither front or rear is visible, make filter slower by reducing process noise
            if (!f_front_or_rear_visible)
            {
               process_noise = process_noise * calib.k_size_update_process_noise_pruning;
            }
            // Kalman filter for width estimation
            object_track.width_uncertainty += process_noise;
            const float32_t gain = object_track.width_uncertainty / (object_track.width_uncertainty + measurement_uncertainty);
            const float32_t updated_width = object_track.bbox.Get_Width() + gain * (measured_width - object_track.bbox.Get_Width());
            object_track.width_uncertainty = (1.0F - gain) * object_track.width_uncertainty;

            object_track.bbox.Set_Width(updated_width);
         }

         Limit_Size_And_Enforce_CTCA_Obj_Aspect_Ratio(calib, obj_dim_limits, object_track);

         // Update affected object signals after resizing length and width
         object_track.Update_Bbox_Center();
      }
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Max_Dimensions_For_Object()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & obj
   * const F360_Calibrations_T & calib
   * float32_t & max_target_length
   * float32_t & max_target_width
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
   * This function determines what maximum length and width are allowed for 
   * the object depending on its filter type, f_moveable and f_vehicular_trk properties.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Determine_Max_Dimensions_For_Object(
      const F360_Object_Track_T & obj,
      const F360_Calibrations_T & calib,
      float32_t & max_target_length,
      float32_t & max_target_width)
   {
      if (obj.f_moveable)
      {
         /*
         If object at some point has been seen to move with high speed(f_vehicular_trk is true), then allow for larger object size.
         Otherwise limit maximim object size to approximately fit an object of car size. This prevents moveable radar ghosts in
         guardrail from growing too large.
         */ 
         max_target_length = (obj.f_vehicular_trk ? calib.k_fast_movable_max_target_length : calib.k_slow_movable_max_target_length);
         max_target_width = calib.k_movable_max_target_width;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Min_Dimensions_For_Object()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & obj
   * const F360_Calibrations_T & calib
   * float32_t & min_target_length
   * float32_t & min_target_width
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
   * This function determines what minimum length and width are allowed
   * for the object depending on its track filter type.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Determine_Min_Dimensions_For_Object(
      const F360_Object_Track_T & obj,
      const F360_Calibrations_T & calib,
      float32_t & min_target_length,
      float32_t & min_target_width)
   {
      if ((F360_TRACKER_TRKFLTR_CTCA == obj.trk_fltr_type) || (std::abs(obj.speed) > calib.fast_moving_thresh)) // For CTCA and fast moving CCA
      {
         min_target_length = calib.k_min_CTCA_target_length;
         min_target_width = calib.k_min_CTCA_target_width;
      }
      else
      {
         min_target_length = calib.k_nonmoveable_target_diameter;
         min_target_width = calib.k_nonmoveable_target_diameter;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Limit_Size_And_Enforce_CTCA_Obj_Aspect_Ratio()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib
   * const Dimension_Limits_T & dim_limits
   * F360_Object_Track_T & obj
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
   * Functions checks wheter the object bounding box dimensions are within the allowed minimum and
   * maximum limits. The function also checks so that the length and width ratio of the object are
   * within allowed limits. If the current object dimensions are deemed to be improper then the
   * function computes and returns new proposed object width and length that are within allowed
   * limits. If the current object dimensions are deemed to be okay then the function returns the
   * current dimensions.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Limit_Size_And_Enforce_CTCA_Obj_Aspect_Ratio(
      const F360_Calibrations_T & calib,
      const Dimension_Limits_T & dim_limits,
      F360_Object_Track_T & obj)
   {
      float32_t resized_length = obj.bbox.Get_Length();
      float32_t resized_width = obj.bbox.Get_Width();

      // Limit object size
      resized_length = F360_Saturate(resized_length, dim_limits.length.minimum, dim_limits.length.maximum);
      resized_width = F360_Saturate(resized_width, dim_limits.width.minimum, dim_limits.width.maximum);

      // Enforce object aspect ratio while making sure object dimensions are still limited
      if ((F360_TRACKER_TRKFLTR_CTCA == obj.trk_fltr_type) || ((F360_TRACKER_TRKFLTR_CCA == obj.trk_fltr_type) && (std::abs(obj.speed) > calib.fast_moving_thresh)))
      {
         const float32_t aspect_width = F360_Saturate(calib.k_min_aspect_ratio * resized_length, dim_limits.width.minimum, dim_limits.width.maximum);
         resized_width = std::max(resized_width, aspect_width);
         const float32_t aspect_length = F360_Saturate(resized_width / calib.k_max_aspect_ratio, dim_limits.length.minimum, dim_limits.length.maximum);
         resized_length = std::max(resized_length, aspect_length);
      }
      obj.bbox.Set_Length(resized_length);
      obj.bbox.Set_Width(resized_width);
   }
}

