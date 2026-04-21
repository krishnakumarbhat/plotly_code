/*===================================================================================*\
* FILE: f360_determine_reflected_obj.cpp
*====================================================================================
* Copyright 2019 APTIV, Inc., All Rights Reserved.
* APTIV Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This is the function to determine if target is a mirror in guard rails.
*
* ABBREVIATIONS:
*   OTG Over-The-ground
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
*
* DEVIATIONS FROM STANDARDS:
*
*
\*==========================================================================================*/


/******************************
* Includes
*******************************/

#include "f360_math_func.h"
#include "f360_determine_reflected_obj.h"
#include "f360_is_host_reflected_track.h"
#include "f360_is_reflective_guardrail_track.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Determine_Reflected_Obj()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *  F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
   *  const F360_Tracker_Info_T & tracker_info,
   *  const int32_t obj_idx,
   *  const F360_Host_T & host,
   *  const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
   *  const F360_Calibrations_T & calibrations
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
   * This function evaluates if an objects appears to be a ghost object
   * that is mirrored from host or another object
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None.
   *
   \*===========================================================================*/
   void Determine_Reflected_Obj(
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Tracker_Info_T & tracker_info,
      const int32_t obj_idx,
      const F360_Host_T & host,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Calibrations_T & calibrations)
   {

      // If flag is on, hide all tracks that is behind static environment. Tracks flagged as on static env are not hidden.
      if (calibrations.hide_tracks_outside_guardrail &&
         (object_tracks[obj_idx].behind_sep_id != F360_INVALID_UNSIGNED_ID) &&
         (F360_INVALID_UNSIGNED_ID == object_tracks[obj_idx].on_sep_id))
      {
         // All objects behind guardrail are handled as guardrail mirror reflections.
         object_tracks[obj_idx].mirror_prob = calibrations.k_reflected_object_max_mirror_probability;
      }
      else
      {

         bool f_mirror = Is_Host_Reflected_Track(object_tracks[obj_idx], host, sep, calibrations);

         if ((!f_mirror) && (host.vcs_speed > calibrations.k_reflective_guardrail_track_min_host_speed))
         {
            f_mirror = Is_Reflective_Guardrail_Track(object_tracks, tracker_info, obj_idx, sep, calibrations);
         }   

         if (f_mirror)
         {
            object_tracks[obj_idx].mirror_prob = calibrations.k_reflected_object_max_mirror_probability; // TODO: DFU-786: Consider second "growing" filter constant
         }
         else
         {
            const float32_t k_decrease_value = 0.10F;

            if (k_decrease_value > object_tracks[obj_idx].mirror_prob)
            {
               // Saturate probability to 0
               object_tracks[obj_idx].mirror_prob = 0.0F;
            }
            else
            {
               object_tracks[obj_idx].mirror_prob -= k_decrease_value;
            }
         }
      }
   }
}
