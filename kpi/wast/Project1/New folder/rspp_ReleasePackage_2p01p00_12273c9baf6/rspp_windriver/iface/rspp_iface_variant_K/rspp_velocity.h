#ifndef RSPP_VELOCITY_H
#define RSPP_VELOCITY_H
/*===================================================================================*\
* FILE: rspp_velocity.h
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

typedef struct RSPP_VCS_Velocity_Tag
{
   float32_t longitudinal;
   float32_t lateral;
} RSPP_VCS_Velocity_T;

#endif
