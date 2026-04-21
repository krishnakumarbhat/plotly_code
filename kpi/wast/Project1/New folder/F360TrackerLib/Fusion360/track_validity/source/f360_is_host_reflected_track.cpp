/*===================================================================================*\
* FILE: f360_is_host_reflected_track.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function definiton of Is_Mirror_Track() function
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_is_host_reflected_track.h"
#include "f360_is_host_reflected_track_helpers.h"
#include "f360_calculate_curvi_position.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Host_Reflected_Track()
   *===========================================================================
   * RETURN VALUE:
   * bool f_is_host_mirror_track
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object - reference to analysed object
   * const F360_Host_T& host - host information
   * const Static_Env_Poly_T (&sep)[F360_NUM_OF_STATIC_ENV_POLYS], - estimated barriers
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
   * This function analyses whether object is reflection of host due to adjacent
   * reflective barrier.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   bool Is_Host_Reflected_Track(
      const F360_Object_Track_T& object,
      const F360_Host_T& host,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T& calib)
   {
      bool f_is_host_mirror_track = false;

      if (Is_Object_Suspected_Of_Being_Host_Reflection(object, host.speed, calib))
      {
         for (uint8_t sep_idx = 0U; sep_idx < F360_NUM_OF_STATIC_ENV_POLYS; sep_idx++)
         {
            if (Is_SEP_Valid_For_Host_Mirror_Ghost(sep[sep_idx], calib.k_host_refl_half_host_length))
            {
               const float32_t host_center_vcs_x = -calib.k_host_refl_half_host_length;
               const float32_t sep_lat_pos_host_center = sep[sep_idx].Lateral_Pos_At(host_center_vcs_x);

               const float32_t sep_long_pos_extreme_val = (-sep[sep_idx].p1) / (2.0F * sep[sep_idx].p2);
               const float32_t sep_lat_pos_extreme_val = sep[sep_idx].Lateral_Pos_At(sep_long_pos_extreme_val);

               // flag indicating that lateral position at host center and extreme value of LCS are same sign
               const bool pos_host_center_lat_pos_extreme_same_sign = (sep_lat_pos_host_center * sep_lat_pos_extreme_val) > 0.0F;

               // crossed path of host if different signs
               const bool sep_crossing_host_path_close_to_host = (!pos_host_center_lat_pos_extreme_same_sign) && 
                  (std::abs(sep_long_pos_extreme_val) <= calib.k_host_refl_filtering_distance);

               const Point host_mirror_track_tcs_pos = Calc_Predicted_Reflected_Track_TCS_Position(object, sep_lat_pos_host_center, calib.k_host_refl_half_host_length);

               if ((!sep_crossing_host_path_close_to_host) &&
                  (Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox(object, host_mirror_track_tcs_pos, calib)))
               {
                  f_is_host_mirror_track = true;
                  break;
               }
            }
         }
      }

      return f_is_host_mirror_track;
   }
}
