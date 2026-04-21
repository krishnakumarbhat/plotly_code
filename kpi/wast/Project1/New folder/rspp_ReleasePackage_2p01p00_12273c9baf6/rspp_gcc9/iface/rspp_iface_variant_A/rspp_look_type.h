#ifndef RSPP_LOOK_TYPE_H
#define RSPP_LOOK_TYPE_H
/*===================================================================================*\
* FILE: rspp_look_type.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_reuse.h"

typedef enum RSPP_Det_Look_Type_Tag : int8_t
{
   RSPP_DET_LOOK_TYPE_INVALID = -1,
   RSPP_DET_LOOK_TYPE_MEDIUM = 0,
   RSPP_DET_LOOK_TYPE_LONG = 1
} RSPP_Det_Look_Type_T;

#endif
