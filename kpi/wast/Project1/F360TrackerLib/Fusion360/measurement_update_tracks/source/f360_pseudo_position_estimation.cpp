/*===========================================================================*\
* FILE: f360_pseudo_position_estimation.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Pseudo_Position_Estimation().
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_pseudo_position_estimation.h"
#include "f360_math_func.h"
#include "f360_get_reference_point_para_side.h"
#include "f360_get_reference_point_orth_side.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_reference_point_support_functions.h"
#include <algorithm>
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_norm_heading_angle.h"
#include "f360_iterator.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Pseudo_Position_Estimation()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calibrations                               - Calibrations structure
   * const F360_Host_T& host                                                - Host structure
   * const Point(&assoc_dets_pos_tcs)[MAX_DETS_IN_OBJ_TRK]                  - Position of associated detections in TCS
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]             - Sensors
   * const F360_Globals_T& globals                                          - Globals
   * F360_Object_Track_T & obj                                              - Object to perform pseudo position estimation for
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
   * Estimates the position and corresponding covariance matrix of an object from the
   * position of a set of detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Pseudo_Position_Estimation(
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Object_Track_T& obj)
   {
         if (obj.ndets == 1U)
         {
            // Only one detection: use this detection as an estimate of the reference point.
            obj.pseudo_vcs_position.x = det_props[obj.detids[0] - 1U].vcs_position.x;
            obj.pseudo_vcs_position.y = det_props[obj.detids[0] - 1U].vcs_position.y;
         }
         else if (F360_REFERENCE_POINT_CENTER == obj.reference_point)
         {
            // Multiple detections and reference point in object center (i.e a non-moveable object): estimate pseudo position as the mid point of all associated detections.
            obj.pseudo_vcs_position = Compute_Pseudo_Pos_Mid_Point_Of_Detections(obj, det_props);
         }
         else
         {
            // Multiple detections and abject has non-center reference point i.e moveable object):
            // Calculate cos of azimuth spread of object to decided if point object or extended object assumption should be used to calculate the pseudo position.
            float32_t dists_from_host_center_to_obj_corners[4];
            float32_t vectors_from_host_center_to_obj_corners[4][2];
            Get_Vectors_And_Distances_To_Obj_Corners(host, obj, vectors_from_host_center_to_obj_corners, dists_from_host_center_to_obj_corners);

            if (Is_Point_Object_Assumption_Valid(vectors_from_host_center_to_obj_corners, dists_from_host_center_to_obj_corners))
            {
               // Object is small or far away and therefore has a small azimuthal spread. Therefore it is approximated as a point target. 
               obj.pseudo_vcs_position = Compute_Pseudo_Pos_Object_Point_Assumption(det_props, host, dists_from_host_center_to_obj_corners, obj);
            }
            else
            {
               // Object is large or close and therefore has a large azimuthal spread. Therefore it is approximated as an extended target.
               obj.pseudo_vcs_position = Compute_Pseudo_Pos_Extended_Obj_Assumption(det_props, obj);
            }
         }

         /*
         Compute covariance of estimated reference point position. First compute a raw value based on sensor
         range and azimuth accuarcy caracteristics. Then adjust this raw value (make larger) to account for the
         scenario and data.
         */
         float32_t pseudo_pos_cov_tcs[2][2];
         Compute_Raw_Pseudo_Pos_Cov_In_TCS(obj, host, pseudo_pos_cov_tcs, calibrations);

         Adjust_Pseudo_Cov_TCS(obj, calibrations, obj.pseudo_vcs_position, pseudo_pos_cov_tcs);

         // Adjust meascov based on the visibility for moveable objects
         if (obj.f_moveable)
         {
            Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensors, calibrations, globals, pseudo_pos_cov_tcs);
         }

         // Rotate pseudo covariance in TCS to VCS.
         float32_t pseudo_pos_cov_vcs[2][2] = { };
         Rotate_2D_Covariance_Matrix(obj.bbox.Get_Orientation().Cos(),
            obj.bbox.Get_Orientation().Sin(),
            pseudo_pos_cov_tcs,
            pseudo_pos_cov_vcs);

         obj.meascov[0][0] = pseudo_pos_cov_vcs[0][0];
         obj.meascov[0][1] = pseudo_pos_cov_vcs[0][1];
         obj.meascov[1][0] = pseudo_pos_cov_vcs[1][0];
         obj.meascov[1][1] = pseudo_pos_cov_vcs[1][1];
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Weighted_Average_Pseudo_Pos()
   *===========================================================================
   * RETURN VALUE:
   * float32_t weighted_avg_pos_est - Weigted average over the positions in input vector assoc_dets_pos_average_dimension
   *
   * PARAMETERS:
   * const uint32_t ndets                                                      - Number of associated detections
   * const float32_t(&assoc_dets_pos_average_dimension)[MAX_DETS_IN_OBJ_TRK]   - Detection positions for which to compute the weighted average
   * const float32_t(&assoc_dets_pos_reference_dimension)[MAX_DETS_IN_OBJ_TRK] - Detection positions which are to be compared against ref_pos in order to determine weights 
   * const float32_t ref_pos                                                   - Referent position for highest weight
   * const float32_t k_huber_threshold                                        - Huber Weight threshold
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
   * This function calculates pseudo position in orth direction based an a weighted average algorithm.
   * The weights are related to the distance to a reference para position corresponding to where we think
   * that the most visible rear/front side of the object is located.
   * This function is called when we don't see left or right side of object but only the rear or the front side.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Weighted_Average_Pseudo_Pos(
      const uint32_t ndets,
      const float32_t(&assoc_dets_pos_average_dimension)[MAX_DETS_IN_OBJ_TRK],
      const float32_t(&assoc_dets_pos_reference_dimension)[MAX_DETS_IN_OBJ_TRK],
      const float32_t ref_pos,
      const float32_t k_huber_threshold)
   {
      float32_t sum_weighted_pos = 0.0F;
      float32_t sum_weights = 0.0F;

      for (uint32_t det_idx = 0U; det_idx < ndets; det_idx++)
      {
         const float32_t dist_to_ref_pos = std::abs(ref_pos - assoc_dets_pos_reference_dimension[det_idx]);
         const float32_t weight = F360_Huber_Weight(dist_to_ref_pos, k_huber_threshold);
         sum_weighted_pos += weight * assoc_dets_pos_average_dimension[det_idx];
         sum_weights += weight;
      }

      float32_t weighted_avg_pos_est = 0.0F;
      if (std::abs(sum_weights) > F360_EPSILON)
      {
         // Compute weighted mean of detections
         weighted_avg_pos_est = sum_weighted_pos / sum_weights;
      }
      else // Protection agaist zero division, compute non-weighted mean instead
      {
         weighted_avg_pos_est = F360_Mean(assoc_dets_pos_average_dimension, ndets);
      }

      return weighted_avg_pos_est;
   }

   /*===========================================================================*\
   * FUNCTION: Adjust_Pseudo_Cov_TCS()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T obj             - Object structure
   * const F360_Calibrations_T & calibrations  - Calibration structure
   * const Point & pseudo_pos_vcs              - Reference point pseudo position in VCS
   * float32_t (&pseudo_cov_tcs)[2][2]         - Pseudo position matrix in TCS
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
   * This function adjusts the covariance matrix dependent on method used to calculate
   * reference points pseudo position.
   * The covariance matrix may also be increased for CTCA tracks due to call to
   * Compute_Pos_Cov_Inc_In_TCS().
   *
   * PRECONDITIONS:
   * pseudo_cov_tcs is expected to have been initialized before call to function
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Adjust_Pseudo_Cov_TCS(
      const F360_Object_Track_T& obj,
      const F360_Calibrations_T& calibrations,
      const Point& pseudo_pos_vcs,
      float32_t(&pseudo_cov_tcs)[2][2])
   {
      // Check which object sides are visible
      const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
      const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);

      if (F360_OBJECT_SIDES_INVALID != rear_front_side)
      {
         // Add uncertainty bias in para direction to account for uncertainties due to object extension
         pseudo_cov_tcs[0][0] += calibrations.k_pseudo_pos_cov_matrix_bias;
      }
      else
      {
         // Add even larger uncertainty depending on object size
         if (F360_OBJECT_SIDES_INVALID != right_left_side)
         {
            // Add bias that is dependent on the length of the object
            pseudo_cov_tcs[0][0] += obj.bbox.Get_Length() * obj.bbox.Get_Length();
         }
         else // Object with center as reference point, i.e. non-movable objects
         {
            pseudo_cov_tcs[0][0] += calibrations.k_pseudo_pos_cov_matrix_bias_non_movable;
         }
      }

      if (F360_OBJECT_SIDES_INVALID != right_left_side)
      {
         // Add uncertainty bias in orth direction to account for uncertainties due to object extension
         pseudo_cov_tcs[1][1] += calibrations.k_pseudo_pos_cov_matrix_bias;
      }
      else
      {
         if (F360_OBJECT_SIDES_INVALID != rear_front_side)
         {
            // Add bias that is dependent on the width of the object
            pseudo_cov_tcs[1][1] += obj.bbox.Get_Width() * obj.bbox.Get_Width();
         }
         else // Object with center as reference point, i.e. non-movable objects
         {
            pseudo_cov_tcs[1][1] += calibrations.k_pseudo_pos_cov_matrix_bias_non_movable;
         }
      }

      // Add extra covariance if time predicted position and pseudo position differ too much for CTCA tracks and fast moving, old CCA tracks (outlier handling)
      const bool f_is_fast_moving_and_old_cca = (((F360_TRACKER_TRKFLTR_CCA == obj.trk_fltr_type) &&
         (std::abs(obj.speed) > calibrations.fast_moving_thresh)) &&
         (obj.time_since_initialization > calibrations.k_time_since_init_th_to_enable_outlier_mitigation_cca));
      if ((F360_TRACKER_TRKFLTR_CTCA == obj.trk_fltr_type) || f_is_fast_moving_and_old_cca)
      {
         const Point ref_pos_tcs = Get_Reference_Point_Pos_In_TCS(obj.reference_point, obj.bbox.Get_Length(), obj.bbox.Get_Width());

         Point pseudo_pos_tcs;
         Convert_VCS_Posn_To_TCS_Posn(pseudo_pos_vcs.x, pseudo_pos_vcs.y, obj.bbox.Get_Center().x, obj.bbox.Get_Center().y, obj.bbox.Get_Orientation(), pseudo_pos_tcs.x, pseudo_pos_tcs.y);

         // Para direction
         const float32_t diff_pos_para = ref_pos_tcs.x - pseudo_pos_tcs.x;
         const float32_t cov_pos_para_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos_para);
         pseudo_cov_tcs[0][0] += cov_pos_para_inc;

         // Orth direction
         const float32_t diff_pos_orth = ref_pos_tcs.y - pseudo_pos_tcs.y;
         const float32_t cov_pos_orth_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos_orth);
         pseudo_cov_tcs[1][1] += cov_pos_orth_inc;
      }

      // Saturate diaginal elements of covariance matrix and modify cross covariance elements for matrix to still be a covariance matrix
      const float32_t saturation_fraction_var_para = std::min(pseudo_cov_tcs[0][0], calibrations.k_pseudo_pos_max_variance_threshold) / pseudo_cov_tcs[0][0];
      const float32_t saturation_fraction_var_orth = std::min(pseudo_cov_tcs[1][1], calibrations.k_pseudo_pos_max_variance_threshold) / pseudo_cov_tcs[1][1];
      pseudo_cov_tcs[0][0] *= saturation_fraction_var_para;
      pseudo_cov_tcs[0][1] *= F360_Sqrtf(saturation_fraction_var_para * saturation_fraction_var_orth);
      pseudo_cov_tcs[1][0] = pseudo_cov_tcs[0][1];
      pseudo_cov_tcs[1][1] *= saturation_fraction_var_orth;
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Pos_Cov_Inc_In_TCS()
   *===========================================================================
   * RETURN VALUE:
   * float32_t cov_pos_inc - Increase in position covariance
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calibrations - Reference to calibration structure
   * float32_t diff_pos                      - Difference in position (in either para or orth position) in TCS
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
   * This function computes the increase in position covariance in TCS based on the
   * difference in position between time predicted position and pseudo position for
   * either the para or orth direction.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Compute_Pos_Cov_Inc_In_TCS(
      const F360_Calibrations_T& calibrations,
      float32_t diff_pos)
   {
      // Make sure diff_pos is the absolute difference in position
      diff_pos = std::abs(diff_pos);

      float32_t cov_pos_inc;
      if (diff_pos < calibrations.k_pseudo_pos_dist_diff_thr)
      {
         // Difference is small enough to not be considered an outlier. Do not add any extra covariance.
         cov_pos_inc = 0.0F;
      }
      else
      {
         // Difference is large enough to be considered an outlier. Add extra covariance.
         cov_pos_inc = calibrations.k_pseudo_pos_dist_diff_gain * (F360_Expf(diff_pos - calibrations.k_pseudo_pos_dist_diff_thr) - 1.0F);
      }
      return cov_pos_inc;
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Raw_Pseudo_Pos_Cov_In_TCS()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& obj             - Object structure
   * const F360_Host_T& host,                   - Host structure
   * float32_t (&pseudo_pos_cov_tcs)[2][2]     - Pseudo position covariance matrix in TCS
   * const F360_Calibrations_T & calibrations   - Calibration structure
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
   * Computes raw covariance values based on sensor range and azimuth accuracy
   * characteristics in polar coordinates and transforms to TCS
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Compute_Raw_Pseudo_Pos_Cov_In_TCS(
      const F360_Object_Track_T& obj,
      const F360_Host_T& host,
      float32_t(&pseudo_pos_cov_tcs)[2][2],
      const F360_Calibrations_T& calibrations)
   {
      // Transform from polar to TCS uncertainty
      const float32_t vec_host_center_to_pseudo_pos[2] = { obj.pseudo_vcs_position.x + 0.5F * host.dist_rear_axle_to_vcs_m, obj.pseudo_vcs_position.y };
      const float32_t range_sq = std::min(vec_host_center_to_pseudo_pos[0] * vec_host_center_to_pseudo_pos[0] + vec_host_center_to_pseudo_pos[1] * vec_host_center_to_pseudo_pos[1], calibrations.k_raw_pseudo_pos_cov_max_saturation_distance);
      const float32_t aspect_angle = F360_Atan2f(vec_host_center_to_pseudo_pos[1], vec_host_center_to_pseudo_pos[0]) - obj.bbox.Get_Orientation().Value(); // Rotation angle from polar to TCS
      const float32_t cos_angle = F360_Cosf(aspect_angle);
      const float32_t cos_angle_sq = cos_angle * cos_angle;
      const float32_t sin_angle = F360_Sinf(aspect_angle);
      const float32_t sin_angle_sq = sin_angle * sin_angle;

      // Calculate covariance matrix values using polar uncertainty
      const float32_t para_var = cos_angle_sq * calibrations.k_range_var + range_sq * sin_angle_sq * calibrations.k_az_var;
      const float32_t orth_var = sin_angle_sq * calibrations.k_range_var + range_sq * cos_angle_sq * calibrations.k_az_var;
      const float32_t para_orth_cov = cos_angle * sin_angle * calibrations.k_range_var - range_sq * cos_angle * sin_angle * calibrations.k_az_var;

      // Assign pseudo position covariance matrix.
      // Covariane matrix symmetric on opposite diagonal
      pseudo_pos_cov_tcs[0][0] = para_var;
      pseudo_pos_cov_tcs[0][1] = para_orth_cov;
      pseudo_pos_cov_tcs[1][0] = para_orth_cov;
      pseudo_pos_cov_tcs[1][1] = orth_var;
   }

   /*===========================================================================*\
    * FUNCTION: Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    *===========================================================================
    * RETURN VALUE:
    * float32_t simplified_para_pos_est - Simple reference point estimation in para direction
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj                                            - Object
    * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]                - Sensors
    * const F360_Calibrations_T& calibrations                                   - Calibrations structure
    * const F360_Globals_T& globals                                             - Globals structure
    * const float32_t(&pseudo_cov_tcs)[2][2]                                   - Psuedo position covariance in TCS
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
    * This function increases the meascov for pseudo position in the appropriate TCS direction,
    * this is done if an object is not fully visible, i.e. some part of the object is out of FOV.
    * The increase is of a magnitude that esentially makes pseudo pos inactive in the given TCS direction.
    *
    * PRECONDITIONS:
    * All the Pointers should Point to valid structures.
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Adjust_Pseudo_Cov_TCS_Wrt_Visibility(
      const F360_Object_Track_T& obj,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T& calibrations,
      const F360_Globals_T& globals,
      float32_t(&pseudo_cov_tcs)[2][2])
   {
      // If the chosen reference point is on a side, check if any of the other reference points on that side are not visible. If so, add to meascov in direction parallel to the side.
      switch (obj.reference_point)
      {
      case (F360_REFERENCE_POINT_LEFT):
      {
         const bool both_points_on_side_visible = (Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_FRONT_LEFT, obj, sensors, globals) && Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_REAR_LEFT, obj, sensors, globals));
         if (!both_points_on_side_visible)
         {
            pseudo_cov_tcs[0][0] = calibrations.k_pseudo_pos_high_uncertainity;
         }
         break;
      }
      case (F360_REFERENCE_POINT_RIGHT):
      {
         const bool both_points_on_side_visible = (Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_FRONT_RIGHT, obj, sensors, globals) && Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_REAR_RIGHT, obj, sensors, globals));
         if (!both_points_on_side_visible)
         {
            pseudo_cov_tcs[0][0] = calibrations.k_pseudo_pos_high_uncertainity;
         }
         break;
      }
      case (F360_REFERENCE_POINT_REAR):
      {
         const bool both_points_on_side_visible = (Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_REAR_LEFT, obj, sensors, globals) && Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_REAR_RIGHT, obj, sensors, globals));
         if (!both_points_on_side_visible)
         {
            pseudo_cov_tcs[1][1] = calibrations.k_pseudo_pos_high_uncertainity;
         }
         break;
      }
      case (F360_REFERENCE_POINT_FRONT):
      {
         const bool both_points_on_side_visible = (Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_FRONT_LEFT, obj, sensors, globals) && Is_Ref_Point_In_Sensors_FOV(F360_REFERENCE_POINT_FRONT_RIGHT, obj, sensors, globals));
         if (!both_points_on_side_visible)
         {
            pseudo_cov_tcs[1][1] = calibrations.k_pseudo_pos_high_uncertainity;
         }
         break;
      }
      case (F360_REFERENCE_POINT_FRONT_LEFT):
      case (F360_REFERENCE_POINT_FRONT_RIGHT):
      case (F360_REFERENCE_POINT_REAR_LEFT):
      case (F360_REFERENCE_POINT_REAR_RIGHT):
      {
         // If reference point is any of the corners, check the delta to the min_projection_reference point (the one that would be chosen disregarding visibility).
         // Increase meascov in the direction where the delta is non-zero.

         if (obj.reference_point != obj.min_projection_reference_point)
         {
            const Point actual_ref_point_tcs = Get_Reference_Point_Pos_In_TCS(obj.reference_point, obj.bbox.Get_Length(), obj.bbox.Get_Width());
            const Point preferred_ref_point_tcs = Get_Reference_Point_Pos_In_TCS(obj.min_projection_reference_point, obj.bbox.Get_Length(), obj.bbox.Get_Width());

            const float32_t delta_x = std::abs(actual_ref_point_tcs.x - preferred_ref_point_tcs.x);
            const float32_t delta_y = std::abs(actual_ref_point_tcs.y - preferred_ref_point_tcs.y);

            const float32_t delta_x_thresh = obj.bbox.Get_Length() * 0.25F;
            const float32_t delta_y_thresh = obj.bbox.Get_Width() * 0.25F;
            if (delta_x > delta_x_thresh)
            {
               pseudo_cov_tcs[0][0] = calibrations.k_pseudo_pos_high_uncertainity;
            }
            if (delta_y > delta_y_thresh)
            {
               pseudo_cov_tcs[1][1] = calibrations.k_pseudo_pos_high_uncertainity;
            }
         }
         break;
      }
      default:
      {
         // Should never happen, MISRA
         break;
      }
      }
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average()
    *===========================================================================
    * RETURN VALUE:
    * Point pseudo_pos_tcs
    *
    * PARAMETERS:
    * const F360_Object_Sides_T rear_front_side
    * const F360_Object_Sides_T right_left_side
    * const float32_t(&assoc_dets_tcs_x_pos)[MAX_DETS_IN_OBJ_TRK]
    * const float32_t(&assoc_dets_tcs_y_pos)[MAX_DETS_IN_OBJ_TRK]
    * const uint32_t ndets
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
    * This function calculates a pseudo position in TCS from an object's associated detections.
    * The scheme is different depending on the visible sides of the object.
    * If the rear or front is visible, the pseudo TCS para position is the most extreme detection (extreme
    * here means that if rear is visible, the detection with the smallest TCS para position is chosen).
    * If the rear or front is not visible, the pseudo TCS para position is calculated as a weighted mean of detections.
    * The same logic is applies for the left/right side visible.
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      const F360_Object_Sides_T rear_front_side,
      const F360_Object_Sides_T right_left_side,
      const float32_t(&assoc_dets_tcs_x_pos)[MAX_DETS_IN_OBJ_TRK],
      const float32_t(&assoc_dets_tcs_y_pos)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t ndets)
   {
      // Huber Weight threshold
      constexpr float32_t k_huber_threshold = 0.4F;

      // Find min and max para and orth positions of associated detections
      const float32_t det_max_para = F360_Max_Element(assoc_dets_tcs_x_pos, ndets);
      const float32_t det_min_para = F360_Min_Element(assoc_dets_tcs_x_pos, ndets);
      const float32_t det_max_orth = F360_Max_Element(assoc_dets_tcs_y_pos, ndets);
      const float32_t det_min_orth = F360_Min_Element(assoc_dets_tcs_y_pos, ndets);

      // Estimate reference point position in para direction
      Point pseudo_pos_tcs = {};
      if (F360_OBJECT_SIDES_REAR == rear_front_side)
      {
         pseudo_pos_tcs.x = det_min_para;
      }
      else if (F360_OBJECT_SIDES_FRONT == rear_front_side)
      {
         pseudo_pos_tcs.x = det_max_para;
      }
      else
      {
         // We don't see the rear or front edge, it's difficult to estimate in para direction. 
         // Use the weighted average pseudo position estimate in para direction instead
         const float32_t reference_orth_pos = ((F360_OBJECT_SIDES_RIGHT == right_left_side) ? det_max_orth : det_min_orth);
         pseudo_pos_tcs.x = Calc_Weighted_Average_Pseudo_Pos(static_cast<uint32_t>(ndets),
            assoc_dets_tcs_x_pos,
            assoc_dets_tcs_y_pos,
            reference_orth_pos,
            k_huber_threshold);
      }

      // Estimate reference point position in orth direction
      if (F360_OBJECT_SIDES_LEFT == right_left_side)
      {
         pseudo_pos_tcs.y = det_min_orth;
      }
      else if (F360_OBJECT_SIDES_RIGHT == right_left_side)
      {
         pseudo_pos_tcs.y = det_max_orth;
      }
      else
      {
         // We don't see the rear or front edge, it's difficult to estimate in para direction. 
         // Use the weighted average pseudo position estimate in para direction instead
         const float32_t reference_para_pos = ((F360_OBJECT_SIDES_FRONT == rear_front_side) ? det_max_para : det_min_para);
         pseudo_pos_tcs.y = Calc_Weighted_Average_Pseudo_Pos(static_cast<uint32_t>(ndets),
            assoc_dets_tcs_y_pos,
            assoc_dets_tcs_x_pos,
            reference_para_pos,
            k_huber_threshold);
      }
      
      return pseudo_pos_tcs;
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Grid_Search()
    *===========================================================================
    * RETURN VALUE:
    * Point pseudo_pos_tcs
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const F360_Object_Sides_T rear_front_side
    * const F360_Object_Sides_T right_left_side
    * const float32_t(&assoc_dets_tcs_x_pos)[MAX_DETS_IN_OBJ_TRK]
    * const float32_t(&assoc_dets_tcs_y_pos)[MAX_DETS_IN_OBJ_TRK]
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
    * This function calculates a pseudo position in TCS based on a grid search scheme. The TCS para and orth
    * of the pseudo position are calculated independently of each other, i.e. there are two 1-dimensional grids.
    * As a first step, a grid is created based on the associated detections and the object's visible side.
    * Then we iterate over the grid to find a hypothetical positioning of the object that gives the best score.
    * The best para and orth positions are combined to give the estimated TCS pseudo position of the object.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Grid_Search(
      const F360_Object_Track_T& obj,
      const F360_Object_Sides_T rear_front_side,
      const F360_Object_Sides_T right_left_side,
      const float32_t(&assoc_dets_tcs_x_pos)[MAX_DETS_IN_OBJ_TRK],
      const float32_t(&assoc_dets_tcs_y_pos)[MAX_DETS_IN_OBJ_TRK])
   {
      // Para direction estimation of object center
      float32_t pos_tcs_x_grid[MAX_DETS_IN_OBJ_TRK];
      uint32_t num_pos_tcs_x_points = 0U;
      Create_Position_Grid(assoc_dets_tcs_x_pos, obj.ndets, rear_front_side, obj.bbox.Get_Length(), pos_tcs_x_grid, num_pos_tcs_x_points);
      const float32_t best_obj_center_tcs_x = Iterate_Over_Grid(pos_tcs_x_grid, num_pos_tcs_x_points, assoc_dets_tcs_x_pos, obj.ndets, rear_front_side, obj.bbox.Get_Length());

      // Orth direction estimation of object center
      float32_t pos_tcs_y_grid[MAX_DETS_IN_OBJ_TRK];
      uint32_t num_pos_tcs_y_points = 0U;
      Create_Position_Grid(assoc_dets_tcs_y_pos, obj.ndets, right_left_side, obj.bbox.Get_Width(), pos_tcs_y_grid, num_pos_tcs_y_points);
      const float32_t best_obj_center_tcs_y = Iterate_Over_Grid(pos_tcs_y_grid, num_pos_tcs_y_points, assoc_dets_tcs_y_pos, obj.ndets, right_left_side, obj.bbox.Get_Width());

      // Transform pseduo pos estimate for object center into pseudo pos estimate of object reference point
      Point pseudo_pos_tcs; 
      pseudo_pos_tcs.x = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(rear_front_side, obj.bbox.Get_Length(), best_obj_center_tcs_x);
      pseudo_pos_tcs.y = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(right_left_side, obj.bbox.Get_Width(), best_obj_center_tcs_y);

      return pseudo_pos_tcs;
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Detection_Score()
    *===========================================================================
    * RETURN VALUE:
    * float32_t det_score
    *
    * PARAMETERS:
    * const float32_t det_mod_tcs_pos
    * const F360_Object_Sides_T visible_side
    * const float32_t obj_dimension
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
    * This function calculates a detection's score, given it's position in relation to an object's visible edge.
    * A higher score is regarded as better. The score is based on the bisquare function, with parameters set to
    * give a detection the highest score on the visible edge. For detections that are outside the visible egde,
    * between the object and host, the score declines rapidly with the distance from the edge. For detections
    * inside the bounding box the score remains high and decreases as the object moves away on the far side of the object.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   float32_t Compute_Detection_Score(
      const float32_t det_mod_tcs_pos,
      const F360_Object_Sides_T visible_side,
      const float32_t obj_dimension)
   {
      float32_t det_score = 0.0F;

      if (F360_OBJECT_SIDES_INVALID == visible_side) // We expect to see the center of the side
      {
         const float32_t dist_to_edges = 0.5F * obj_dimension;
         const bool f_inside_bbox = std::abs(det_mod_tcs_pos) <= dist_to_edges;

         if (f_inside_bbox)
         {
            det_score = 1.0F;
         }
         else
         {
            constexpr float32_t k_bisquare_outside_edge = 5.0F;
            const float32_t dist_from_edge = std::abs(det_mod_tcs_pos) - dist_to_edges;

            // Score will be between 1 and 0
            det_score = Compute_Bisquare_Score(dist_from_edge, k_bisquare_outside_edge);
         }
      }
      else // We expect to see a corner of a side
      {
         // Must be smaller than 1 and larger than 0
         constexpr float32_t k_min_score_inside_bbox = 0.75F;

         const float32_t dist_to_far_edge = obj_dimension;
         const bool f_left_or_rear_side_visible = ((F360_OBJECT_SIDES_LEFT == visible_side) || (F360_OBJECT_SIDES_REAR == visible_side));
         const bool f_inside_bbox = f_left_or_rear_side_visible ? ((det_mod_tcs_pos >= 0.0F) && (det_mod_tcs_pos <= dist_to_far_edge)) : ((det_mod_tcs_pos <= 0.0F) && (det_mod_tcs_pos >= -dist_to_far_edge));

         if (f_inside_bbox)
         {
            constexpr float32_t k_bisquare_inside_bbox = 0.8F;
            const float32_t safe_k_bisquare = std::min(k_bisquare_inside_bbox, dist_to_far_edge);

            // Score will be between 1 and k_min_score_inside_bbox
            det_score = (1.0F - k_min_score_inside_bbox) * Compute_Bisquare_Score(det_mod_tcs_pos, safe_k_bisquare) + k_min_score_inside_bbox;
         }
         else
         {
            constexpr float32_t k_bisquare_outside_visible_edge = 0.1F;
            constexpr float32_t k_bisquare_outside_nonvisible_edge_para = 5.0F;
            constexpr float32_t k_bisquare_outside_nonvisible_edge_orth = 1.0F;

            const bool f_farther_away_than_nonvisible_edge = f_left_or_rear_side_visible ? (det_mod_tcs_pos > dist_to_far_edge) : (det_mod_tcs_pos < -dist_to_far_edge);
            const bool f_rear_or_front_side_visible = ((F360_OBJECT_SIDES_REAR == visible_side) || (F360_OBJECT_SIDES_FRONT == visible_side));

            const float32_t k_bisquare_outside_nonvisible_edge = f_rear_or_front_side_visible ? k_bisquare_outside_nonvisible_edge_para : k_bisquare_outside_nonvisible_edge_orth;

            const float32_t abs_mod_det_para = std::abs(det_mod_tcs_pos);
            const float32_t score_scaling = f_farther_away_than_nonvisible_edge ? k_min_score_inside_bbox : 1.0F;
            const float32_t k_bisquare = f_farther_away_than_nonvisible_edge ? k_bisquare_outside_nonvisible_edge : k_bisquare_outside_visible_edge;
            const float32_t dist_from_edge = f_farther_away_than_nonvisible_edge ? (abs_mod_det_para - dist_to_far_edge) : abs_mod_det_para;

            det_score = score_scaling * Compute_Bisquare_Score(dist_from_edge, k_bisquare);
         }
      }
      return det_score;
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Bisquare_Score()
    *===========================================================================
    * RETURN VALUE:
    * float32_t score
    *
    * PARAMETERS:
    * const float32_t res
    * const float32_t k_bistatic
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
    * This function implements the bisquare scoring function. If the input is larger than the parameter
    * k_bistatic, the score is 0. Otherwise the score is determined by
    * score = (1 - (abs(input) / k_bistatic)^2)^2.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   float32_t Compute_Bisquare_Score(
      const float32_t res,
      const float32_t k_bistatic)
   {
      const float32_t abs_res = std::abs(res);
      float32_t score = 0.0F; // Default score for large residuals

      if (abs_res < k_bistatic) // Compute score for small residuals
      {
         float32_t tmp_var = abs_res / k_bistatic;
         tmp_var = tmp_var * tmp_var;
         tmp_var = 1.0F - tmp_var;
         score = tmp_var * tmp_var;
      }

      return score;
   }

   /*===========================================================================*\
    * FUNCTION: Create_Position_Grid()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const float32_t (&dets_pos)[MAX_DETS_IN_OBJ_TRK]
    * const uint32_t ndets
    * const F360_Object_Sides_T visible_side
    * const float32_t obj_dimension
    * float32_t (&pos_grid)[MAX_DETS_IN_OBJ_TRK]
    * uint32_t & num_pos_points
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
    * This function creates a 1-dimensional grid of possible placements of a hypothetical object
    * for the grid search pseudo position scheme for either para or orth in TCS.
    * When creating a grid for the TCS x (para) of the pseudo position and the rear or front side
    * is visible, the grid points are the TCS x positions of the associated detections.
    * If the rear or front is not visible, the median TCS x of detections is calculated. Then enough grid points
    * are placed on either side of the median with a pre-determined delta such that they cover the entire
    * spread of detections.
    * The procedure is the same for the TCS y (orth).
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Create_Position_Grid(
      const float32_t (&dets_pos)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t ndets,
      const F360_Object_Sides_T visible_side,
      const float32_t obj_dimension,
      float32_t (&pos_grid)[MAX_DETS_IN_OBJ_TRK],
      uint32_t & num_pos_points)
   {
      if (F360_OBJECT_SIDES_INVALID != visible_side)
      {
         // Add grid points such that the visible edge ends up where the detections are. I.e. what the grid search will do is to find the detection which is most likely to correspond to the edge
         const float32_t dist_from_center_to_ref_pnt = ((F360_OBJECT_SIDES_REAR == visible_side) || (F360_OBJECT_SIDES_LEFT == visible_side)) ? (-0.5F * obj_dimension) : (0.5F * obj_dimension);
         for (uint32_t grid_idx = 0U; grid_idx < ndets; grid_idx++)
         {
            pos_grid[grid_idx] = dets_pos[grid_idx] - dist_from_center_to_ref_pnt;
         }
         num_pos_points = static_cast<uint32_t>(ndets);
      }
      else
      {
         float32_t sorted_dets_pos[MAX_DETS_IN_OBJ_TRK];
         (void)std::copy(cmn::begin(dets_pos), cmn::end(dets_pos), cmn::begin(sorted_dets_pos));
         uint32_t sort_idx[MAX_DETS_IN_OBJ_TRK];
         (void)F360_Sort(sorted_dets_pos, static_cast<uint32_t>(ndets), true, sort_idx);

         // When the middle of the side is seen it's are likely to find the optimal solution close to the median of the detections - therefore create grid points around the median
         const uint32_t median_idx1 = (ndets - 1U) / 2U; // Note: It is intentional to discard any fractional part when ndets is even
         const uint32_t median_idx2 = ndets / 2U; // Note: It is intentional to discard any fractional part when ndets is odd
         const float32_t median_pos = 0.5F * (sorted_dets_pos[median_idx1] + sorted_dets_pos[median_idx2]);

         constexpr uint32_t max_grid_points_on_each_side_of_median = (static_cast<uint32_t>(MAX_DETS_IN_OBJ_TRK) - 1U) / 2U; // Note: it is intentional to discard any fractional part such that we are sure not to exceed the number of allowed elements in the grid when MAX_DETS_IN_OBJ_TRK is even 

         // Desired delta between grid points when a side is not seen
         constexpr float32_t k_desired_delta_pos = 0.1F;

         const uint32_t num_grid_points_left_of_median_to_include_min_det = static_cast<uint32_t>(std::abs(((median_pos - sorted_dets_pos[0]) / k_desired_delta_pos) + 1.0F)); // Note: The +1 is added such that the static cast to uint will always round the float upwards
         const uint32_t num_grid_points_right_of_median_to_include_max_det = static_cast<uint32_t>(std::abs(((sorted_dets_pos[ndets - 1U] - median_pos) / k_desired_delta_pos) + 1.0F)); // Note: The +1 is added such that the cast to uint will always round the float upwards
         const uint32_t num_grid_points_left_of_median = std::min(max_grid_points_on_each_side_of_median, num_grid_points_left_of_median_to_include_min_det);
         const uint32_t num_grid_points_right_of_median = std::min(max_grid_points_on_each_side_of_median, num_grid_points_right_of_median_to_include_max_det);

         const uint32_t median_grid_idx = num_grid_points_left_of_median;
         pos_grid[median_grid_idx] = median_pos;
         for (uint32_t idx = 1U; idx <= num_grid_points_left_of_median; idx++)
         {
            const float32_t value = median_pos - static_cast<float32_t>(idx) * k_desired_delta_pos;
            const uint32_t grid_idx = median_grid_idx - idx;
            pos_grid[grid_idx] = value;
         }
         for (uint32_t idx = 1U; idx <= num_grid_points_right_of_median; idx++)
         {
            const float32_t value = median_pos + static_cast<float32_t>(idx) * k_desired_delta_pos;
            const uint32_t grid_idx = num_grid_points_left_of_median + idx;
            pos_grid[grid_idx] = value;
         }
         num_pos_points = num_grid_points_left_of_median + num_grid_points_right_of_median + 1U;
      }
   }

   /*===========================================================================*\
    * FUNCTION: Iterate_Over_Grid()
    *===========================================================================
    * RETURN VALUE:
    * float32_t best_obj_center_pos
    *
    * PARAMETERS:
    * const float32_t (&pos_grid)[MAX_DETS_IN_OBJ_TRK]
    * const uint32_t & num_pos_points
    * const float32_t(&assoc_dets_pos_tcs)[MAX_DETS_IN_OBJ_TRK]
    * const uint32_t ndets
    * const F360_Object_Sides_T visible_side
    * const float32_t obj_dimension
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
    * This function iterates over a grid of potential object positions. For each grid point,
    * a score is calculated for each associated detection. The total score is the sum of all detection
    * scores. The grid point corresponding to the best total score is chosen as the best object center positoion
    * and returned.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
  
   float32_t Iterate_Over_Grid(
      const float32_t (&pos_grid)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t num_grid_points,
      const float32_t(&assoc_dets_pos_tcs)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t ndets,
      const F360_Object_Sides_T obj_visible_side,
      const float32_t obj_dimension)
   {

      const bool f_min_side_visible = (F360_OBJECT_SIDES_REAR == obj_visible_side) || (F360_OBJECT_SIDES_LEFT == obj_visible_side);

      float32_t best_score = -1.0F; // Initialize to something negative since score computed score below is >= 0
      float32_t best_obj_center_pos = 0.0F;
      float32_t best_interval_start = 1e7F; // Intitialize to something very large
      float32_t best_interval_end = -1e7F; // Intitialize to something very small
      for (uint32_t pos_idx = 0U; pos_idx < num_grid_points; pos_idx++)
      {
         const float32_t curr_obj_center_pos = pos_grid[pos_idx];
         const float32_t dist_from_center_to_ref = ((F360_OBJECT_SIDES_INVALID == obj_visible_side) ? 0.0F : (f_min_side_visible ? -0.5F * obj_dimension : 0.5F * obj_dimension));
         float32_t score = 0.0F; // Initialize to 0 (lowest/worst possible score)
         for (uint32_t det_i = 0U; det_i < ndets; det_i++)
         {
            // Compute detection para orth coordinates in a modified TCS that has the reference point position for given grid point as origin
            const float32_t translation = curr_obj_center_pos + dist_from_center_to_ref;
            const float32_t mod_det_pos = assoc_dets_pos_tcs[det_i] - translation;

            const float32_t det_score = Compute_Detection_Score(mod_det_pos, obj_visible_side, obj_dimension);
            score += det_score;
         }

         // Store best pseudo pos hypothesis
         if (score > (best_score + F360_EPSILON))
         {
            // New best score found
            best_score = score;
            best_obj_center_pos = curr_obj_center_pos;
            best_interval_start = curr_obj_center_pos;
            best_interval_end = curr_obj_center_pos;
         }
         else if (score < (best_score - F360_EPSILON))
         {
            // No new best score is found. Do nothing
         }
         else
         {
            // New score is very similar to best score. Do further investigations to determine which option is best.
            // For example if several grid points result in all detections being inside bounding box.
            if (F360_OBJECT_SIDES_INVALID == obj_visible_side)
            {
               if (curr_obj_center_pos < best_interval_start)
               {
                  best_interval_start = curr_obj_center_pos;
               }
               if (curr_obj_center_pos > best_interval_end)
               {
                  best_interval_end = curr_obj_center_pos;
               }
               best_obj_center_pos = 0.5F * (best_interval_start + best_interval_end);
            }
            else
            {
               const bool f_new_option_is_more_extreme = (f_min_side_visible ? (curr_obj_center_pos < best_obj_center_pos) : (curr_obj_center_pos > best_obj_center_pos));
               if (f_new_option_is_more_extreme)
               {
                  // New best score found
                  best_score = score;
                  best_obj_center_pos = curr_obj_center_pos;
               }
            }
         }
      }

      return best_obj_center_pos;
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Pseudo_Pos_Mid_Point_Of_Detections()
    *===========================================================================
    * RETURN VALUE:
    * Point pseudo_position_vcs
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
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
    * This function calculates a pseudo position in VCS from the associated detections.
    * The min and max x position of detections are found and the x-coordinate of the pseudo
    * position is calculated as the mean of those. The same is done for the y-coordinate, independently
    * of the x-coordinate.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Compute_Pseudo_Pos_Mid_Point_Of_Detections(
      const F360_Object_Track_T& obj,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      // Find min and max positions of associated detections
      const uint32_t first_det_idx = obj.detids[0] - 1U;
      float32_t max_vcs_x = det_props[first_det_idx].vcs_position.x;
      float32_t min_vcs_x = det_props[first_det_idx].vcs_position.x;
      float32_t max_vcs_y = det_props[first_det_idx].vcs_position.y;
      float32_t min_vcs_y = det_props[first_det_idx].vcs_position.y;
      for (uint32_t i = 1U; i < obj.ndets; i++)
      {
         const uint32_t det_idx = obj.detids[i] - 1U;
         const float32_t curr_det_x_pos = det_props[det_idx].vcs_position.x;
         const float32_t curr_det_y_pos = det_props[det_idx].vcs_position.y;

         // Loop over all detections to find the most extreme
         if (curr_det_x_pos > max_vcs_x)
         {
            max_vcs_x = curr_det_x_pos;
         }
         else if (curr_det_x_pos < min_vcs_x)
         {
            min_vcs_x = curr_det_x_pos;
         }
         else
         {
            // Do nothing (MISRA compliance).
         }

         if (curr_det_y_pos > max_vcs_y)
         {
            max_vcs_y = curr_det_y_pos;
         }
         else if (curr_det_y_pos < min_vcs_y)
         {
            min_vcs_y = curr_det_y_pos;
         }
         else
         {
            // Do nothing (MISRA compliance).
         }
      }

      // Compute pseudo pos as mid point of min and max positions
      return Point(0.5F * (max_vcs_x + min_vcs_x), 0.5F * (max_vcs_y + min_vcs_y));
   }

   /*===========================================================================*\
    * FUNCTION: Get_Vectors_And_Distances_To_Obj_Corners()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const F360_Host_T& host
    * const F360_Object_Track_T& obj
    * float32_t (&vec_from_host_center_to_obj_corners)[4][2]
    * float32_t (&dist_from_host_center_to_obj_corners)[4]
    *
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
    * This function calculates vectors from the host center to each of the object corners
    * and the corresponding distances (lengths of the vectors).
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Get_Vectors_And_Distances_To_Obj_Corners(
      const F360_Host_T& host,
      const F360_Object_Track_T& obj,
      float32_t (&vec_from_host_center_to_obj_corners)[4][2],
      float32_t (&dist_from_host_center_to_obj_corners)[4])
   {
      const float32_t host_center_vcs[2] = { -0.6F * host.dist_rear_axle_to_vcs_m, 0.0F };
      const BboxCorners obj_corners_vcs = obj.bbox.Get_Corners();
 
      vec_from_host_center_to_obj_corners[0][0] = obj_corners_vcs.Front_Left().x - host_center_vcs[0];
      vec_from_host_center_to_obj_corners[0][1] = obj_corners_vcs.Front_Left().y - host_center_vcs[1];

      vec_from_host_center_to_obj_corners[1][0] = obj_corners_vcs.Front_Right().x - host_center_vcs[0];
      vec_from_host_center_to_obj_corners[1][1] = obj_corners_vcs.Front_Right().y - host_center_vcs[1];

      vec_from_host_center_to_obj_corners[2][0] = obj_corners_vcs.Rear_Right().x - host_center_vcs[0];
      vec_from_host_center_to_obj_corners[2][1] = obj_corners_vcs.Rear_Right().y - host_center_vcs[1];

      vec_from_host_center_to_obj_corners[3][0] = obj_corners_vcs.Rear_Left().x - host_center_vcs[0];
      vec_from_host_center_to_obj_corners[3][1] = obj_corners_vcs.Rear_Left().y - host_center_vcs[1];

      dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
      dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
      dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
      dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);
   }

   /*===========================================================================*\
    * FUNCTION: Is_Point_Object_Assumption_Valid()
    *===========================================================================
    * RETURN VALUE:
    * bool
    *
    * PARAMETERS:
    * const float32_t (&vec_from_host_center_to_obj_corners)[4][2]
    * const float32_t (&dist_from_host_center_to_obj_corners)[4]
    *
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
    * This function determines if an object should be considered as a point target (as opposed to an extended target) based on its azimuth spread.
    * To do this, it considers two paris of vectors to object corners, rear left/front right and rear right/front left.
    * For each pair, the cosine of the angle between them is calculated and the maximum angle is chosen - corresponding
    * to the minimum cosine. If the cosine is large enough (i.e. the angle small enough), the object is considered a point target.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   bool Is_Point_Object_Assumption_Valid(
      const float32_t(&vec_from_host_center_to_obj_corners)[4][2],
      const float32_t(&dist_from_host_center_to_obj_corners)[4])
   {
      const float32_t inv_dist_from_host_center_to_obj_corners[4] = { 1.0F / dist_from_host_center_to_obj_corners[0],
                                                                    1.0F / dist_from_host_center_to_obj_corners[1],
                                                                    1.0F / dist_from_host_center_to_obj_corners[2],
                                                                    1.0F / dist_from_host_center_to_obj_corners[3] };

      const float32_t normalized_vec_from_host_center_to_obj_corners[4][2] = { {vec_from_host_center_to_obj_corners[0][0] * inv_dist_from_host_center_to_obj_corners[0], vec_from_host_center_to_obj_corners[0][1] * inv_dist_from_host_center_to_obj_corners[0]},
                                                                              {vec_from_host_center_to_obj_corners[1][0] * inv_dist_from_host_center_to_obj_corners[1], vec_from_host_center_to_obj_corners[1][1] * inv_dist_from_host_center_to_obj_corners[1]},
                                                                              {vec_from_host_center_to_obj_corners[2][0] * inv_dist_from_host_center_to_obj_corners[2], vec_from_host_center_to_obj_corners[2][1] * inv_dist_from_host_center_to_obj_corners[2]},
                                                                              {vec_from_host_center_to_obj_corners[3][0] * inv_dist_from_host_center_to_obj_corners[3], vec_from_host_center_to_obj_corners[3][1] * inv_dist_from_host_center_to_obj_corners[3]} };

      // Formula used below: cosine ( angle_between_vectors ) = dot product ( A_unitvec, B_unitvec )
      const float32_t cos_angle_FL_and_RR = normalized_vec_from_host_center_to_obj_corners[0][0] * normalized_vec_from_host_center_to_obj_corners[2][0] + normalized_vec_from_host_center_to_obj_corners[0][1] * normalized_vec_from_host_center_to_obj_corners[2][1];
      const float32_t cos_angle_FR_and_RL = normalized_vec_from_host_center_to_obj_corners[1][0] * normalized_vec_from_host_center_to_obj_corners[3][0] + normalized_vec_from_host_center_to_obj_corners[1][1] * normalized_vec_from_host_center_to_obj_corners[3][1];

      // Minimum cosine gives maximum angle
      const float32_t min_cos_angle = std::min(cos_angle_FL_and_RR, cos_angle_FR_and_RL);

      /* Approx cos(2.3deg) meaning that if we see short side only of objectand this short side is
       2m wide we will use extended object based estimation if the object is closer than ~50m.*/
      constexpr float32_t k_cos_min_obj_az_spread_for_extended_object = 0.999194395114446F;

      return (min_cos_angle > k_cos_min_obj_az_spread_for_extended_object);
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Pseudo_Pos_Object_Point_Assumption()
   *===========================================================================
   * RETURN VALUE:
   * Point pseudo position in VCS
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const F360_Host_T& host
   * const float32_t(&dist_from_host_center_to_obj_corners)[4]
   * const F360_Object_Track_T& obj
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
   * Compute the pseudo pos according to:
   * 1) Pretend there is a pseudo sensor in host center. Compute the range and azimuth of all associated detections in a
   *    pseudo SCS coordinate system with x-axis along the line from host center to object center and y-axis orthogonal to this line
   * 2) Find the minimum detection range and the average detection azimuth in the pseudo SCS
   * 3) Compute the pseudo pos for object center in pseudo SCS according to obj_center_pseudo_pos = (min_range + range_from_closest_corner_to_obj_center) * [cos(avg_az, sin(avg_az))].
   *    I.e. determine the pseudo pos for object by placing the object such that the line from host center to average detection azimuth passes through host center while the range to the closest corner equals min detectaion range
   * 4) Transform pseudo pos for object center back into VCS and compute corresponding pseudo pos for reference point
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   Point Compute_Pseudo_Pos_Object_Point_Assumption(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Host_T& host,
      const float32_t(&dist_from_host_center_to_obj_corners)[4],
      const F360_Object_Track_T& obj)
   {
      const Point host_center_vcs = Point(-0.6F * host.dist_rear_axle_to_vcs_m, 0.0F );
      // 1) and 2) Compute detection range in pseudo SCS and find minimum detection range and average detection azimuth
      const float32_t vec_from_host_center_to_obj_center_vcs[2] = { obj.bbox.Get_Center().x - host_center_vcs.x, obj.bbox.Get_Center().y - host_center_vcs.y };
      const Angle pseudo_scs_rotation_in_vcs = Angle(F360_Atan2f(vec_from_host_center_to_obj_center_vcs[1], vec_from_host_center_to_obj_center_vcs[0]));
      const float32_t min_range_pseudo_SCS = Calculate_SCS_Min_Range(obj, det_props, host_center_vcs);
      const Angle avg_az_pseudo_SCS = Calculate_Mean_SCS_Azimuth(obj, det_props, host_center_vcs, pseudo_scs_rotation_in_vcs);


      // 3) Compute pseudo position of object center in our pseudo SCS coordinate system
      const BboxCorners obj_corners_vcs = obj.bbox.Get_Corners();
      float32_t vec_obj_closest_corner_to_center_vcs[2] = {};
      Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(obj, dist_from_host_center_to_obj_corners, obj_corners_vcs, vec_obj_closest_corner_to_center_vcs);
      const float32_t range_obj_closest_corner_to_center_pseudo_scs = vec_obj_closest_corner_to_center_vcs[0] * (pseudo_scs_rotation_in_vcs.Cos() * avg_az_pseudo_SCS.Cos() - pseudo_scs_rotation_in_vcs.Sin() * avg_az_pseudo_SCS.Sin())
         + vec_obj_closest_corner_to_center_vcs[1] * (pseudo_scs_rotation_in_vcs.Sin() * avg_az_pseudo_SCS.Cos() + pseudo_scs_rotation_in_vcs.Cos() * avg_az_pseudo_SCS.Sin()); // Note: Run time optimization has been done where trigonometric formulas for cos(angle_host_center_to_obj_center_vcs + avg_az_pseudo_SCS) and sin(angle_host_center_to_obj_center_vcs + avg_az_pseudo_SCS) have been utilized to avoid having to do two new trigonometric function calls
      const float32_t obj_center_pseudo_pos_range = min_range_pseudo_SCS + range_obj_closest_corner_to_center_pseudo_scs;
      const Point obj_center_pseudo_pos_in_pseudo_SCS = Point(obj_center_pseudo_pos_range * avg_az_pseudo_SCS.Cos(), obj_center_pseudo_pos_range * avg_az_pseudo_SCS.Sin() );

      // 4) Transform pseudo pos for object center back into VCS and then compute final pseudo pos (corresponding to object reference point)
      const Point pseudo_pos_center_vcs = Transform_Pseudo_SCS_Pos_To_VCS(obj_center_pseudo_pos_in_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);
      return Convert_Object_Center_Point_To_Reference_Point(obj, pseudo_pos_center_vcs);
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Pseudo_Pos_Extended_Obj_Assumption()
    *===========================================================================
    * RETURN VALUE:
    * Point pseudo position in VCS
    *
    * PARAMETERS:
    * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
    * const F360_Object_Track_T& obj
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
    * Compute the pseudo position in two different ways and then weight them together into one single pseudo pos estimate.
    * The weighting is based on how close the two different estimates are to the predicted reference point.
    *    Pseudo estimate 1: In case of seeing the relevant object edge we pick the most extreme detection in the direction of the edge.
    *                    And in case of not seeing the relevantobject edge we compute a weighted average in the direction of the edge (where weights are
    *                    based on distance to the edge that we actually can see).
    *    Pseudo estimate 2: Create a position grid in TCS. Then do a grid search to find the most optimal solution (done separately for x and y, independent of each other).
    *                    For each grid point, pretend to place the object in the grid point and compute a score for all detections associated to the object compute.
    *                    Sum up the score for all detections.
    *                    The total score is the score for the grid point. The grid point with highest score "wins" and is choosen as pseudo pos. Detection score
    *                    is computed based on a score function where a far distance distance to the visible edge (or center of object for the case where we don't
    *                    see the relevant side) is punished with a lower score.
    *
    * PRECONDITIONS:
    * All the Pointers should Point to valid structures.
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Compute_Pseudo_Pos_Extended_Obj_Assumption(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Object_Track_T& obj)
   {
      // Reference point position in TCS
      const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
      const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);

      // Convert position of associated detections from VCS to TCS
      float32_t assoc_dets_tcs_x_pos[MAX_DETS_IN_OBJ_TRK];
      float32_t assoc_dets_tcs_y_pos[MAX_DETS_IN_OBJ_TRK];
      for (uint32_t i = 0U; i < obj.ndets; i++)
      {
         const uint32_t det_idx = obj.detids[i] - 1U;

         Convert_VCS_Posn_To_TCS_Posn(
            det_props[det_idx].vcs_position.x,
            det_props[det_idx].vcs_position.y,
            obj.bbox.Get_Center().x,
            obj.bbox.Get_Center().y,
            obj.bbox.Get_Orientation(),
            assoc_dets_tcs_x_pos[i],
            assoc_dets_tcs_y_pos[i]);
      }

      // Way 1 of computing pseudo pos
      const Point pseudo_pos_tcs_min_max_avg = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(rear_front_side, right_left_side, assoc_dets_tcs_x_pos, assoc_dets_tcs_y_pos, obj.ndets);

      /* For two detections the grid search approach should always result in similar result as Call Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average.
         So for run time improvements only call it when we have more than 2 detections. */
      Point pseudo_pos_tcs = {};
      if (obj.ndets > 2U)
      {
         // Way 2 of computing pseudo pos
         const Point pseudo_pos_tcs_grid_search = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Grid_Search(obj, rear_front_side, right_left_side, assoc_dets_tcs_x_pos, assoc_dets_tcs_y_pos);

         // Combine the two pseudo position measurements into one, weighted based on their respective distance to the reference point
         Compute_Combined_Weighted_Pseudo_Position(obj, pseudo_pos_tcs_min_max_avg, pseudo_pos_tcs_grid_search, pseudo_pos_tcs);
      }
      else
      {
         pseudo_pos_tcs.x = pseudo_pos_tcs_min_max_avg.x;
         pseudo_pos_tcs.y = pseudo_pos_tcs_min_max_avg.y;
      }

      // Convert pseudo position estimate from TCS to to VCS
      Point vcs_pseudo_pos;
      Convert_TCS_Posn_To_VCS_Posn(pseudo_pos_tcs.x, pseudo_pos_tcs.y, obj.bbox.Get_Center().x, obj.bbox.Get_Center().y, obj.bbox.Get_Orientation(), vcs_pseudo_pos.x, vcs_pseudo_pos.y);
      return vcs_pseudo_pos;
   }

   /*===========================================================================*\
    * FUNCTION: Get_Vector_From_Obj_Center_To_Closest_Corner_VCS()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const float32_t(&dist_from_host_center_to_obj_corners)[4]
    * const BboxCorners& obj_corners_vcs
    * float32_t (&vec_obj_closest_corner_to_center_vcs)[2])
    *
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
    * This function finds the object corner closest to host center and calculates a vector from
    * the object center to that corner.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(
      const F360_Object_Track_T& obj,
      const float32_t(&dist_from_host_center_to_obj_corners)[4],
      const BboxCorners& obj_corners_vcs,
      float32_t (&vec_obj_closest_corner_to_center_vcs)[2])
   {
      const uint32_t closest_corner_idx = F360_Min_Index(dist_from_host_center_to_obj_corners, 4U);

      switch (closest_corner_idx)
      {
         case 0U:
         {
            vec_obj_closest_corner_to_center_vcs[0] = obj.bbox.Get_Center().x - obj_corners_vcs.Front_Left().x;
            vec_obj_closest_corner_to_center_vcs[1] = obj.bbox.Get_Center().y - obj_corners_vcs.Front_Left().y;
            break;
         }
         case 1U:
         {
            vec_obj_closest_corner_to_center_vcs[0] = obj.bbox.Get_Center().x - obj_corners_vcs.Front_Right().x;
            vec_obj_closest_corner_to_center_vcs[1] = obj.bbox.Get_Center().y - obj_corners_vcs.Front_Right().y;
            break;
         }
         case 2U:
         {
            vec_obj_closest_corner_to_center_vcs[0] = obj.bbox.Get_Center().x - obj_corners_vcs.Rear_Right().x;
            vec_obj_closest_corner_to_center_vcs[1] = obj.bbox.Get_Center().y - obj_corners_vcs.Rear_Right().y;
            break;
         }
         case 3U:
         default:
         {
            vec_obj_closest_corner_to_center_vcs[0] = obj.bbox.Get_Center().x - obj_corners_vcs.Rear_Left().x;
            vec_obj_closest_corner_to_center_vcs[1] = obj.bbox.Get_Center().y - obj_corners_vcs.Rear_Left().y;
            break;
         }
      }
   }

   /*===========================================================================*\
    * FUNCTION: Transform_Pseudo_SCS_Pos_To_VCS()
    *===========================================================================
    * RETURN VALUE:
    * Point
    *
    * PARAMETERS:
    * const Point& point_pseudo_SCS,
    * const Angle& angle_host_center_to_obj_center_vcs,
    * const Point& host_center_vcs
    *
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
    * This function transforms a point defined in the pseudo sensor coordinate system (SCS)
    * to a point in the vehicle coordinate system (VCS).
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Transform_Pseudo_SCS_Pos_To_VCS(
      const Point& obj_center_pseudo_pos_in_pseudo_SCS,
      const Angle& angle_host_center_to_obj_center_vcs,
      const Point& host_center_vcs)
   {
      Point point_vcs;

      F360_Rotate_2D_Vector(obj_center_pseudo_pos_in_pseudo_SCS.x,
         obj_center_pseudo_pos_in_pseudo_SCS.y,
         angle_host_center_to_obj_center_vcs.Cos(),
         angle_host_center_to_obj_center_vcs.Sin(),
         point_vcs.x,
         point_vcs.y);

      point_vcs.x += host_center_vcs.x;
      point_vcs.y += host_center_vcs.y;

      return point_vcs;
   }

   /*===========================================================================*\
    * FUNCTION: Convert_Object_Center_Point_To_Reference_Point()
    *===========================================================================
    * RETURN VALUE:
    * Point pseudo_pos_vcs
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const Point & obj_center_pseudo_pos_vcs
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
    * Given a pseudo point corresponding to the object center, this function outputs
    * a pseudo position corresponding to the object's current reference point.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Point Convert_Object_Center_Point_To_Reference_Point(
      const F360_Object_Track_T& obj,
      const Point & obj_center_pseudo_pos_vcs)
   {
      const Point vec_from_obj_center_to_ref_tcs = Get_Reference_Point_Pos_In_TCS(obj.reference_point, obj.bbox.Get_Length(), obj.bbox.Get_Width());

      float32_t vec_from_obj_center_to_ref_vcs[2];
      F360_Rotate_2D_Vector(vec_from_obj_center_to_ref_tcs.x, vec_from_obj_center_to_ref_tcs.y, obj.bbox.Get_Orientation().Cos(), obj.bbox.Get_Orientation().Sin(), vec_from_obj_center_to_ref_vcs[0], vec_from_obj_center_to_ref_vcs[1]);
      
      const Point pseudo_pos_vcs = Point(obj_center_pseudo_pos_vcs.x + vec_from_obj_center_to_ref_vcs[0], obj_center_pseudo_pos_vcs.y + vec_from_obj_center_to_ref_vcs[1]);

      return pseudo_pos_vcs;
   }

   /*===========================================================================*\
    * FUNCTION: Calculate_SCS_Min_Range()
    *===========================================================================
    * RETURN VALUE:
    * float32_t min range
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
    * const Point& host_center_vcs
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
    * This function finds the detection with the minimum range in the pseudo sensor coordinate system (SCS).
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   float32_t Calculate_SCS_Min_Range(
      const F360_Object_Track_T& obj,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const Point& host_center_vcs)
   {
      float32_t min_range_pseudo_SCS_squared = 1e7F;

      for (uint32_t i = 0U; i < obj.ndets; i++)
      {
         const uint32_t det_idx = obj.detids[i] - 1U;
         const float32_t range_pseudo_SCS_squared = (det_props[det_idx].vcs_position.x - host_center_vcs.x) * (det_props[det_idx].vcs_position.x - host_center_vcs.x)
            + (det_props[det_idx].vcs_position.y - host_center_vcs.y) * (det_props[det_idx].vcs_position.y - host_center_vcs.y);

         if (range_pseudo_SCS_squared < min_range_pseudo_SCS_squared)
         {
            min_range_pseudo_SCS_squared = range_pseudo_SCS_squared;
         }
      }
      return F360_Sqrtf(min_range_pseudo_SCS_squared);
   }

   /*===========================================================================*\
    * FUNCTION: Calculate_Mean_SCS_Azimuth()
    *===========================================================================
    * RETURN VALUE:
    * Angle mean SCS azimuth
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
    * const Point& host_center_vcs
    * const Angle& angle_host_center_to_obj_center_vcs
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
    * This function rotates detection azimuths to SCS and calculates the average SCS azimuth.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   Angle Calculate_Mean_SCS_Azimuth(
      const F360_Object_Track_T& obj,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const Point& host_center_vcs,
      const Angle& angle_host_center_to_obj_center_vcs)
   {
      float32_t sum_az_pseudo_SCS = 0.0F;
      for (uint32_t i = 0U; i < obj.ndets; i++)
      {
         const uint32_t det_idx = obj.detids[i] - 1U;
         const float32_t vec_from_host_center_to_det[2] = { det_props[det_idx].vcs_position.x - host_center_vcs.x, det_props[det_idx].vcs_position.y - host_center_vcs.y };
         const float32_t det_angle_relative_host_center_vcs_orientation = F360_Atan2f(vec_from_host_center_to_det[1], vec_from_host_center_to_det[0]);
         const float32_t det_angle_pseudo_SCS = Normalize_Heading_Angle(det_angle_relative_host_center_vcs_orientation, angle_host_center_to_obj_center_vcs.Value()) - angle_host_center_to_obj_center_vcs.Value();
         sum_az_pseudo_SCS += det_angle_pseudo_SCS;
      }
      return Angle(sum_az_pseudo_SCS / static_cast<float32_t>(obj.ndets)); // No risk of division with 0 since it's checked outside Pseudo_Position_Estimation().
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Combined_Weighted_Pseudo_Position()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const F360_Object_Track_T& obj
    * const Point & pseudo_pos_tcs_min_max_avg,
    * const Point & pseudo_pos_tcs_grid_search,
    * Point & pseudo_pos_tcs
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
    * This function calculates weights for two different pseudo positions in TCS, based on their distance
    * to the reference point and calculates a weighted average of the two.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Compute_Combined_Weighted_Pseudo_Position(
      const F360_Object_Track_T& obj,
      const Point & pseudo_pos_tcs_min_max_avg,
      const Point & pseudo_pos_tcs_grid_search,
      Point & pseudo_pos_tcs)
   {
      const Point ref_pnt_tcs = Get_Reference_Point_Pos_In_TCS(obj.reference_point, obj.bbox.Get_Length(), obj.bbox.Get_Width());
      // Weights for pseudo pos estimate 1
      float32_t weight_min_max_avg[2];
      Calculate_Pseudo_Pos_Weights(pseudo_pos_tcs_min_max_avg, ref_pnt_tcs, weight_min_max_avg);

      // Weights for pseudo pos estimate 2
      float32_t weight_grid_search[2];
      Calculate_Pseudo_Pos_Weights(pseudo_pos_tcs_grid_search, ref_pnt_tcs, weight_grid_search);

      // Weight the two measurements types together based on their proximity to the reference point
      const float32_t inv_weight_sum[2] = { 1.0F / (weight_min_max_avg[0] + weight_grid_search[0]), 1.0F / (weight_min_max_avg[1] + weight_grid_search[1]) }; // No risk of division by zero due to saturation of weight_min_max_weighted_avg and weight_grid_search in code above.
      pseudo_pos_tcs.x = (weight_min_max_avg[0] * pseudo_pos_tcs_min_max_avg.x + weight_grid_search[0] * pseudo_pos_tcs_grid_search.x) * inv_weight_sum[0];
      pseudo_pos_tcs.y = (weight_min_max_avg[1] * pseudo_pos_tcs_min_max_avg.y + weight_grid_search[1] * pseudo_pos_tcs_grid_search.y) * inv_weight_sum[1];
   }

   /*===========================================================================*\
    * FUNCTION: Compute_Combined_Weighted_Pseudo_Position()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const Point& pseudo_pos_tcs,
    * const Point & ref_pnt_tcs,
    * float32_t(&weights)[2]
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
    * This function calculates the weights for a given pseudo position's x and y TCS coordinates based
    * on the distance to the object's reference point. The smaller the distance, the higher the weight.
    * Within a fixed distance, the weights are miximized and then outside they decay exponentially.
    *
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/

   void Calculate_Pseudo_Pos_Weights(
      const Point& pseudo_pos_tcs,
      const Point& ref_pnt_tcs,
      float32_t(&weights)[2])
   {
      constexpr float32_t max_dist_for_highest_weight = 0.15F; // [m] Threshold for distance between pseudo position and reference point below which maximum weight is assigned.
      constexpr float32_t k_weight_decrease_scale_param = 25.0F; // [m] Scaling factor

      const float32_t dist_to_predicted_pos[2] = { std::abs(pseudo_pos_tcs.x - ref_pnt_tcs.x), std::abs(pseudo_pos_tcs.y - ref_pnt_tcs.y) };
      
      weights[0] = (dist_to_predicted_pos[0] < max_dist_for_highest_weight) ? 1.0F : F360_Expf(-0.5F * (dist_to_predicted_pos[0] - max_dist_for_highest_weight) * (dist_to_predicted_pos[0] - max_dist_for_highest_weight) * k_weight_decrease_scale_param);
      weights[1] = (dist_to_predicted_pos[1] < max_dist_for_highest_weight) ? 1.0F : F360_Expf(-0.5F * (dist_to_predicted_pos[1] - max_dist_for_highest_weight) * (dist_to_predicted_pos[1] - max_dist_for_highest_weight) * k_weight_decrease_scale_param);
      weights[0] = std::max(weights[0], F360_EPSILON);
      weights[1] = std::max(weights[1], F360_EPSILON);
   }


   /*===========================================================================*\
    * FUNCTION: Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension()
    *===========================================================================
    * RETURN VALUE:
    * float32_t reference_point_tcs_dimension
    *
    * PARAMETERS:
    * const F360_Object_Sides_T& visible_side_in_dimension,
    * const float32_t& obj_size_in_dimension
    * const float32_t& center_point_tcs_dimension
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
    * This function translates a position corresponding to an object's center
    * point to instead correspond to the reference point in tcs in a single dimension.
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   float32_t Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(
      const F360_Object_Sides_T& visible_side_in_dimension,
      const float32_t& obj_size_in_dimension,
      const float32_t& center_point_tcs_dimension)
   {
      float32_t reference_point_tcs_dimension;

      switch (visible_side_in_dimension)
      {
         case (F360_OBJECT_SIDES_INVALID):
         {
            reference_point_tcs_dimension = center_point_tcs_dimension;
            break;
         }
         case (F360_OBJECT_SIDES_REAR):
         case (F360_OBJECT_SIDES_LEFT):
         {
            reference_point_tcs_dimension = center_point_tcs_dimension - 0.5F * obj_size_in_dimension;
            break;
         }
         case (F360_OBJECT_SIDES_FRONT):
         case (F360_OBJECT_SIDES_RIGHT):
         default:
         {
            reference_point_tcs_dimension = center_point_tcs_dimension + 0.5F * obj_size_in_dimension;
            break;
         }
      }

      return reference_point_tcs_dimension;
   }

}
