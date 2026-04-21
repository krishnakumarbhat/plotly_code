#ifndef F360_OBJECT_OCCLUSION_STATUS_H
#define F360_OBJECT_OCCLUSION_STATUS_H
/*===================================================================================*\
* FILE: f360_object_occlusion_status.h
*====================================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains the declaration of Object_Occlusion_Status
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_occlusion_types.h"

namespace f360_variant_A
{
   struct Object_Occlusion_Status
   {
      Occlusion_Status_T at_vcs_position; // Occlusion status at vcs_position/reference point
   };
}

#endif
