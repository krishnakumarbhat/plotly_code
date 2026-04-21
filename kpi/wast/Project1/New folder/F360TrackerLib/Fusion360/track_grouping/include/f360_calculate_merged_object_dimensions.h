/*===================================================================================*\
* FILE: f360_calculate_merged_object_dimensions.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains declaration of Calculate_Merged_Object_Dimensions function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/
#ifndef F360_CALCULATE_MERGED_OBJECT_DIMENSIONS_H
#define F360_CALCULATE_MERGED_OBJECT_DIMENSIONS_H

#include "f360_object_track.h"
#include "f360_dimensions.h"

namespace f360_variant_A
{
   F360_Dimensions_T Calculate_Merged_Object_Dimensions(
      const F360_Object_Track_T & obj_to_keep,
      const F360_Object_Track_T & obj_to_kill);
}
#endif
