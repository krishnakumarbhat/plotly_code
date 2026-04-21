/*===================================================================================*\
* FILE:  f360_update_by_tracks_helpers.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of supporting functions used in Update_Sensors_Occlusion() method.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef UPDATE_BY_TRACKS_HELPERS_H
#define UPDATE_BY_TRACKS_HELPERS_H

#include "f360_reuse.h"
#include "f360_object_track.h"

namespace f360_variant_A
{
   bool Is_Object_Relevant_For_Occlusion(
      const float32_t min_confidence_level,
      const F360_Object_Track_T& object);
}
#endif
