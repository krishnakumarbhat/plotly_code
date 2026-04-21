/*===================================================================================*\
* FILE: f360_current_msmt_type.h
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains F360_Current_Msmt_Type structure declaration
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#ifndef F360_CURRENT_MSMT_TYPE_H
#define F360_CURRENT_MSMT_TYPE_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   enum F360_Current_Msmt_Type_T : uint8_t
   {
      F360_MSMT_TYPE_INVALID = 0,
      F360_MSMT_TYPE_DETS_ONLY = 1
   };
}
#endif
