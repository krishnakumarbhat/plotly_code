/*===================================================================================*\
* FILE: f360_is_reflective_guardrail_track.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function definiton of Is_Reflective_Guardrail_Track() and
* related sub function
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_is_reflective_guardrail_track.h"
#include "f360_math_func.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_check_if_point_is_inside_box.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Reflective_Guardrail_Track()
   *===========================================================================
   * RETURN VALUE:
   * True if guardrail is reflective; false otherwise
   *
   * PARAMETERS:
   * const F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS] - object tracks array
   * const F360_Tracker_Info_T &tracker_info - tracker info struct
   * const int32_t ghost_candidate_idx - index of object track which is considered to be a ghost created by reflection of a real object
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const F360_Calibrations_T &cal - tracker calibrations
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function determines if a ghost candidate is a reflection  guardrail track
   *
   \*===========================================================================*/
   bool Is_Reflective_Guardrail_Track(
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Tracker_Info_T& tracker_info,
      const int32_t ghost_candidate_idx,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T& cal
   )
   {
      /* Check if this is a target behind guardrail can be a ghost created by reflection real source object from the guardrail.
       * We know host position and reflected object track candidate position as well as guardrail parameters
       * we calcualte the position of source of reflection on the opposite side of the guardrail.
       * Then we iterate over object tracks to find the object which position matches roughly the source position
       * and other parameters matches approximately the parameters of reflected object candidate.
       * Is a matchin object track is found than function returns flag f_reflective_guardrail_track = true
       * what means that the reflected object track candidate is actually a ghost being only reflection of matching source object.
      */

      bool f_reflective_guardrail_track = false;

      const F360_Object_Track_T& ghost_candidate = object_tracks[ghost_candidate_idx];

      if (Is_Ghost_Obj_Valid_For_Check(ghost_candidate, cal))
      {
         const uint8_t sep_idx = ghost_candidate.behind_sep_id - 1U;
         const Point ghost_candidate_center_vcs = ghost_candidate.bbox.Get_Center();
         f_reflective_guardrail_track = Is_Ghost_Reflected_By_SEP(sep[sep_idx], ghost_candidate, ghost_candidate_center_vcs, tracker_info, object_tracks, cal);
      }
      return f_reflective_guardrail_track;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Ghost_Obj_Valid_For_Check()
   *===========================================================================
   * RETURN VALUE:
   * bool f_obj_valid
   *
   * PARAMETERS:
   * const F360_Object_Track_T & ghost_candidate - reference to object to be checked
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
   * This function determines if a ghost candidate is valid to be checked as a potential ghost
   * due to reflections in SEP.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Ghost_Obj_Valid_For_Check(
      const F360_Object_Track_T& ghost_candidate,
      const F360_Calibrations_T& calib
   )
   {
      const bool f_filter_type_CTCA_Fast_CCA = (F360_TRACKER_TRKFLTR_CTCA == ghost_candidate.trk_fltr_type) || (std::abs(ghost_candidate.speed) > calib.fast_moving_thresh);
      const bool f_behind_sep = ghost_candidate.behind_sep_id != F360_INVALID_UNSIGNED_ID;
      const bool f_obj_valid = (f_filter_type_CTCA_Fast_CCA && f_behind_sep && ghost_candidate.f_moving);
      return f_obj_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Ghost_Reflected_By_SEP()
   *===========================================================================
   * RETURN VALUE:
   * bool f_reflective_guardrail_track
   *
   * PARAMETERS:
   * const Static_Env_Poly_T& sep,
   * const F360_Object_Track_T &ghost_candidate - object track considered to be candidate of a ghost reflected from source
   * const Point &ghost_cand_pos - ghost candidate position in VCS
   * const F360_Tracker_Info_T &tracker_info - tracker info struct
   * const F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS] - object tracks array
   * const F360_Calibrations_T &cal - tracker calibrations
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
   * This function determines if a ghost candidate is a reflection of SEP
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Ghost_Reflected_By_SEP(
      const Static_Env_Poly_T& sep,
      const F360_Object_Track_T& ghost_candidate,
      const Point& ghost_cand_pos,
      const F360_Tracker_Info_T& tracker_info,
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Calibrations_T& cal)
   {
      bool f_reflective_guardrail_track = false;

      // Check that point of reflection is not too far from host
      if ((ghost_candidate.sep_intersection_point.x >= cal.k_tv_refl_gr_trk_min_sep_lon_pos) && (ghost_candidate.sep_intersection_point.x <= cal.k_tv_refl_gr_trk_max_sep_lon_pos))
      {
         const Point center_of_symmetry = Calculate_Center_Of_Symmetry_SEP(ghost_cand_pos, ghost_candidate.sep_intersection_point, sep);

         // Calculate hypothetical point that can be the source of reflection for ghost candidate
         Point hypothetic_source_pos_vcs;
         hypothetic_source_pos_vcs.x = (2.0F * center_of_symmetry.x) - ghost_cand_pos.x;
         hypothetic_source_pos_vcs.y = (2.0F * center_of_symmetry.y) - ghost_cand_pos.y;

         // Loop over source candidates to find a candidate matching hypothetical source position
         for (int32_t array_idx = 0; array_idx < tracker_info.num_active_objs; array_idx++)
         {
            const int32_t source_candidate_idx = tracker_info.active_obj_ids[array_idx] - 1;
            const F360_Object_Track_T source_candidate = object_tracks[source_candidate_idx];
            if ((&source_candidate != &ghost_candidate) &&
               ((F360_TRACKER_TRKFLTR_CTCA == source_candidate.trk_fltr_type) || (std::abs(source_candidate.speed) > cal.fast_moving_thresh)) &&
               (F360_INVALID_UNSIGNED_ID == source_candidate.on_sep_id) &&
               (source_candidate.f_moving) &&
               (source_candidate.status > F360_OBJECT_STATUS_NEW_UPDATED) &&
               (source_candidate.reduced_id > 0) &&
               (F360_INVALID_UNSIGNED_ID == source_candidate.behind_sep_id) &&
               (Is_Source_Candidate_Similar_To_Ghost_Candidate(source_candidate, ghost_candidate, hypothetic_source_pos_vcs, cal)))
            {
               f_reflective_guardrail_track = true;
               break;
            }
         }
      }
      return f_reflective_guardrail_track;
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Center_Of_Symmetry_SEP()
   *===========================================================================
   * RETURN VALUE:
   * Point center_of_symmetry
   *
   * PARAMETERS:
   * const Point &ghost_cand_pos,
   * const Point &point_of_reflection,
   * const Static_Env_Poly_T& sep
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
   * This function calculates the center of symmetry. The tangent line to the SEP is calculated at the
   * point of reflection. The center of symmetry is found along this tangent, at the point from where the
   * normal of the tangent line crosses the ghost position.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Point Calculate_Center_Of_Symmetry_SEP(
      const Point& ghost_cand_pos,
      const Point& point_of_reflection,
      const Static_Env_Poly_T& sep)
   {
      const float32_t p2 = sep.p2;
      const float32_t p1 = sep.p1;
      const float32_t p0 = sep.p0;

      const float32_t por_x_pow_2 = point_of_reflection.x * point_of_reflection.x;
      const float32_t por_x_pow_3 = por_x_pow_2 * point_of_reflection.x;
      const float32_t tangent_slope = 2.0F * p2 * point_of_reflection.x + p1;
      const float32_t center_of_symmetry_x_numerator = tangent_slope * (ghost_cand_pos.y - p0) + 2.0F * p2 * p2 * por_x_pow_3 + p2 * p1 * por_x_pow_2 + ghost_cand_pos.x;
      const float32_t center_of_symmetry_x_denominator = tangent_slope * tangent_slope + 1.0F;

      Point center_of_symmetry = {};
      center_of_symmetry.x = center_of_symmetry_x_numerator / center_of_symmetry_x_denominator;

      if (std::abs(tangent_slope) < F360_MIN_DENOMINATOR)
      {
         // SEP is very close to straight longitudinally, i.e. tangent slope very close to 0.
         // Thus, center of symmetry can be approximated on the tangent line.
         center_of_symmetry.y = point_of_reflection.y;
      }
      else
      {
         center_of_symmetry.y = ghost_cand_pos.y + (ghost_cand_pos.x - center_of_symmetry.x) / tangent_slope;
      }

      return center_of_symmetry;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Source_Candidate_Similar_To_Ghost_Candidate()
   *===========================================================================
   * RETURN VALUE:
   * bool f_candidates_similar
   *
   * PARAMETERS:
   * const F360_Object_Track_T &source_candidate, object track considered to be candidate of a source of reflection
   * const F360_Object_Track_T &ghost_candidate, object track considered to be candidate of a ghost reflected from source
   * const Point &hypothetic_source_pos, hypothetic position of source candidate center
   * const F360_Calibrations_T &cal, tracker calibrations
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
   * This function evaluates the similarity between ghost and source candidates
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Source_Candidate_Similar_To_Ghost_Candidate(
      const F360_Object_Track_T& source_candidate,
      const F360_Object_Track_T& ghost_candidate,
      const Point& hypothetic_source_pos,
      const F360_Calibrations_T& cal)
   {
      const Point source_candidate_pos_vcs = source_candidate.bbox.Get_Center();

      float32_t min_spd = std::min(std::abs(source_candidate.speed), std::abs(ghost_candidate.speed));
      min_spd = std::abs(min_spd) < F360_EPSILON ? (F360_EPSILON * static_cast<float32_t>(F360_Sign(min_spd))) : min_spd;
      const float32_t diff_vel_rel = std::abs(source_candidate.speed - ghost_candidate.speed) / min_spd;

      bool f_candidates_similar =
         (diff_vel_rel < cal.k_tv_refl_gr_trk_max_diff_rel_vel) &&
         (std::abs(source_candidate_pos_vcs.y - hypothetic_source_pos.y) < cal.k_tv_refl_gr_trk_max_diff_y) &&
         (std::abs(source_candidate.vcs_heading.Value() - ghost_candidate.vcs_heading.Value()) < cal.k_tv_refl_gr_trk_max_diff_heading);

      if (f_candidates_similar) {
         float32_t max_ghost_diff_x_adapt;
         if (std::abs(source_candidate.vcs_heading.Value()) < cal.k_tv_refl_gr_trk_straight_mov_head_th)
         {
            max_ghost_diff_x_adapt = std::max(cal.k_tv_refl_gr_trk_max_diff_x, 0.5F * source_candidate.bbox.Get_Length());
         }
         else
         {
            max_ghost_diff_x_adapt = cal.k_tv_refl_gr_trk_max_diff_x;
         }

         const float32_t diff_x = std::abs(source_candidate_pos_vcs.x - hypothetic_source_pos.x);

         f_candidates_similar = ((diff_x < max_ghost_diff_x_adapt) ||
            Is_Hypot_Source_Pos_In_Source_Candidate_BBox(hypothetic_source_pos, source_candidate, ghost_candidate, cal));
      }

      return f_candidates_similar;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Hypot_Source_Pos_In_Source_Candidate_BBox()
   *===========================================================================
   * RETURN VALUE:
   * bool 
   *
   * PARAMETERS:
   * const Point &hypothetic_source_pos          - Hypothetic position of source candidate center
   * const F360_Object_Track_T &source_candidate - Object track considered to be candidate of a source of reflection
   * const F360_Object_Track_T &ghost_candidate  - Object track considered to be candidate of a ghost reflected from source
   * const F360_Calibrations_T &cal              - Tracker calibrations
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
   * This function checks whether the hypothetic source point is located within
   * source candidate bounding box with some margin.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Hypot_Source_Pos_In_Source_Candidate_BBox(
      const Point& hypothetic_source_pos,
      const F360_Object_Track_T& source_candidate,
      const F360_Object_Track_T& ghost_candidate,
      const F360_Calibrations_T& cal)
   {
      float32_t x_in_source_candidate_tcs;
      float32_t y_in_source_candidate_tcs;
      Convert_VCS_Posn_To_TCS_Posn(
         hypothetic_source_pos.x,
         hypothetic_source_pos.y,
         source_candidate.bbox.Get_Center().x,
         source_candidate.bbox.Get_Center().y,
         source_candidate.bbox.Get_Orientation(),
         x_in_source_candidate_tcs,
         y_in_source_candidate_tcs);

      const float32_t bbox_long_margin = std::max(cal.k_tv_refl_gr_trk_min_bbox_lat_margin, ghost_candidate.bbox.Get_Length());
      const float32_t half_length = 0.5F * source_candidate.bbox.Get_Length();
      const float32_t half_width = 0.5F * source_candidate.bbox.Get_Width();
      float32_t box[2][2];
      box[0][0] = -(half_length + bbox_long_margin);
      box[0][1] = half_length + bbox_long_margin;
      box[1][0] = -(half_width + cal.k_tv_refl_gr_trk_bbox_lat_margin);
      box[1][1] = half_width + cal.k_tv_refl_gr_trk_bbox_lat_margin;

      return Check_If_Point_Is_Inside_Box_In_Same_CS(x_in_source_candidate_tcs, y_in_source_candidate_tcs, box);
   }
}
