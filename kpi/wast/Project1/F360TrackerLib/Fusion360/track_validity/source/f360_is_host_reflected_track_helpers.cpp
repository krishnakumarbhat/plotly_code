/*===================================================================================*\
* FILE: f360_is_host_reflected_track_helpers.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of supporting functions used in Is_Host_Reflected_Track().
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_is_host_reflected_track_helpers.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_check_if_point_is_inside_box.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox()
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_in_bbox - Flag indicating whether predicted ghost mirror track is 
   *                             placed in extended bounding box of analysed object.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object       - Analysed object
   * const Point& host_mirror_track_tcs_pos  - Predicted ghost position in TCS of analysed object
   * const F360_Calibrations_T& calib        - Tracker calibrations
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
   * This function check whether predicted ghost position is placed in extended
   * bounding box of analysed object.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   bool Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox(
      const F360_Object_Track_T& object, 
      const Point& host_mirror_track_tcs_pos,
      const F360_Calibrations_T& calib)
   {

      float32_t box[2][2];
      const float32_t half_length = 0.5F * object.bbox.Get_Length();
      const float32_t half_width = 0.5F * object.bbox.Get_Width();
      box[0][0] = -(half_length + calib.k_host_refl_bbox_long_ext);
      box[0][1] = (half_length + calib.k_host_refl_bbox_long_ext);
      box[1][0] = -(half_width + calib.k_host_refl_bbox_lat_ext);
      box[1][1] = (half_width + calib.k_host_refl_bbox_lat_ext);

      const bool f_is_in_bbox = Check_If_Point_Is_Inside_Box_In_Same_CS(host_mirror_track_tcs_pos.x, host_mirror_track_tcs_pos.y, box);

      return f_is_in_bbox;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Predicted_Reflected_Track_TCS_Position()
   *===========================================================================
   * RETURN VALUE:
   * Point ghost_tcs_pos - Predicted host reflected track position in TCS of analysed track
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object - Analysed object
   * const float32_t sep_lat_pos,     - Lateral position of SEP
   * const float32_t half_of_host_len - Assumed half of host length used to determine mirror track position
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
   * This function calculates position of possible object reflection from host in analysed
   * track coordinates system.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   Point Calc_Predicted_Reflected_Track_TCS_Position(
      const F360_Object_Track_T& object,
      const float32_t sep_lat_pos,
      const float32_t half_of_host_len)
   {
      Point ghost_tcs_pos = {};

      const float32_t x_test_vcs = -1.0F * half_of_host_len;
      const float32_t y_test_vcs = 2.0F * sep_lat_pos;

      Convert_VCS_Posn_To_TCS_Posn(
         x_test_vcs,
         y_test_vcs,
         object.bbox.Get_Center().x,
         object.bbox.Get_Center().y,
         object.bbox.Get_Orientation(),
         ghost_tcs_pos.x,
         ghost_tcs_pos.y);

      return ghost_tcs_pos;
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Heading_And_Speed_Threshold()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const float32_t host_speed - speed of host vehicle
   * const F360_Calibrations_T& calib - tracker calibrations
   * float32_t& max_heading - determined maximum heading, output value
   * float32_t& max_speed_diff - determined maximum speed diff, output value
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
   * This function picks maximum heading and calculates maximum speed difference
   * between host and analysed object to classify whether it is suspected of being
   * host mirror track.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Determine_Heading_And_Speed_Threshold(
      const float32_t host_speed,
      const F360_Calibrations_T& calib,
      float32_t& max_heading,
      float32_t& max_speed_diff)
   {
      if (std::abs(host_speed) <= calib.k_host_refl_lowspeed_host_speed_th)
      {
         max_speed_diff = calib.k_host_refl_lowspeed_speed_diff_th;
         max_heading = calib.k_host_refl_lowspeed_heading_th;

      }
      else
      {
         max_heading = calib.k_host_refl_highspeed_heading_th;
         max_speed_diff = calib.k_host_refl_highspeed_min_speed_diff_th + (calib.k_host_refl_highspeed_speed_diff_ramp_coef * std::abs(host_speed));
         max_speed_diff = std::min(calib.k_host_refl_highspeed_max_speed_diff_th, max_speed_diff);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Object_Suspected_Of_Being_Host_Reflection()
   *===========================================================================
   * RETURN VALUE:
   * bool !f_is_not_suspected - Flag indicating whether analysed object is 
   *                                    suspected of being host mirror track
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object - analysed object
   * const float32_t& host_speed - host speed
   * const F360_Calibrations_T& calib - tracker calibrations
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
   * This function analyses whether object is suspected of being mirror track.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   bool Is_Object_Suspected_Of_Being_Host_Reflection(
      const F360_Object_Track_T& object,
      const float32_t& host_speed,
      const F360_Calibrations_T& calib)
   {
      float32_t max_heading;
      float32_t max_speed_diff;
      Determine_Heading_And_Speed_Threshold(host_speed, calib, max_heading, max_speed_diff);

      const bool f_is_not_suspected = ((max_heading < std::abs(object.vcs_heading.Value())) ||
         (object.vcs_position.x < calib.k_host_refl_min_obj_long_pos) ||
         (calib.k_host_refl_max_obj_long_pos < object.vcs_position.x) ||
         (std::abs(host_speed - object.speed) > max_speed_diff));

      return !f_is_not_suspected;
   }

   /*===========================================================================*\
   * FUNCTION: Is_SEP_Valid_For_Host_Mirror_Ghost()
   *===========================================================================
   * RETURN VALUE:
   * bool f_sep_valid
   *
   * PARAMETERS:
   *  const Static_Env_Poly_T& sep,
   *  const float32_t host_half_length
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
   * This function evaluates if an SEP is valid to be used for countermeasure
   * Is_Host_Mirror_Ghost(). The SEP is valid if the SEP itself is valid and that
   * the SEP interval is at least partly valid adjacent to host.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   bool Is_SEP_Valid_For_Host_Mirror_Ghost(
      const Static_Env_Poly_T& sep,
      const float32_t host_half_length)
   {
      const float32_t host_min_vcs_x = -2.0F * host_half_length;

      bool f_sep_valid;
      // Note that SEP is considered valid if it spans whole host length or if it begins and/or ends next to host
      if ((sep.status != F360_STATIC_ENV_POLY_STATUS_INVALID) &&
         (sep.upper_limit > host_min_vcs_x) && (sep.lower_limit < 0.0F))
      {
         f_sep_valid = true;
      }
      else
      {
         f_sep_valid = false;
      }

      return f_sep_valid;
   }
}
