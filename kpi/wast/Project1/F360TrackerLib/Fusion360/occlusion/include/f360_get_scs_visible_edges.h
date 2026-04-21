/*===================================================================================*\
* FILE:  f360_get_scs_visible_edges.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of function used to convert VCS visible edges to SCS visible edges.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_GET_SCS_VISIBLE_EDGES
#define F360_GET_SCS_VISIBLE_EDGES

#include "f360_occlusion_types.h"
#include "f360_sensor_mounting_position.h"

namespace f360_variant_A
{

   SCS_Track_Visible_Edges_T Get_SCS_Visible_Edges(
      const VCS_Track_Visible_Edges_T& vcs_visible_edges,
      const Sensor_Mounting_Position& mounting_pos
   );

}
#endif
