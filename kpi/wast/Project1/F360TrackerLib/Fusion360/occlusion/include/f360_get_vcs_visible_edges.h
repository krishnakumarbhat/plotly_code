/*===================================================================================*\
* FILE:  f360_get_vcs_visible_edges.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Get_VCS_Visible_Edges function.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_GET_VCS_VISIBLE_EDGES
#define F360_GET_VCS_VISIBLE_EDGES

#include "f360_object_track.h"
#include "f360_occlusion_types.h"
#include "f360_sensor_mounting_position.h"

namespace f360_variant_A
{
   VCS_Track_Visible_Edges_T Get_VCS_Visible_Edges(
      const F360_Object_Track_T& object);

}
#endif
