/*===================================================================================*\
* FILE:  f360_update_by_tracks_helpers.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of supporting functions used in Update_Sensors_Occlusion() method.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_update_by_tracks_helpers.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Object_Relevant_For_Occlusion()
   *===========================================================================
   * RETURN VALUE:
   * bool f_valid - flag indicating whether object should be considered
   * in occlusion analysis.
   *
   * PARAMETERS:
   * const float32_t min_confidence_level
   * const F360_Object_Track_T& object
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
   * This function verifies whether object should be used to determine occlusion.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Object_Relevant_For_Occlusion(
      const float32_t min_confidence_level,
      const F360_Object_Track_T& object)
   {
      const bool f_is_relevant = (F360_OBJECT_STATUS_INVALID != object.status) &&
         (object.f_moveable) &&
         (F360_INVALID_UNSIGNED_ID == object.on_sep_id) &&
         (F360_INVALID_UNSIGNED_ID == object.behind_sep_id) &&
         (object.confidenceLevel > min_confidence_level);

      return f_is_relevant;
   }
}

