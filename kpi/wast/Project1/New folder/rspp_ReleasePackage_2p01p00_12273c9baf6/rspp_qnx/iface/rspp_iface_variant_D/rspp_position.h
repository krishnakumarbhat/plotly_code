#ifndef RSPP_POSITION_H
#define RSPP_POSITION_H
/*===================================================================================*\
* FILE: rspp_position.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

typedef struct RSPP_VCS_Position_Tag
{
   float32_t longitudinal;
   float32_t lateral;
   float32_t height;
} RSPP_VCS_Position_T;

#endif
