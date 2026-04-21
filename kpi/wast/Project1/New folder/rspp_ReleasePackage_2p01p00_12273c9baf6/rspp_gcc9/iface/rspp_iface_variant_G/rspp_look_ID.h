#ifndef RSPP_LOOK_ID_H
#define RSPP_LOOK_ID_H
/*===================================================================================*\
* FILE: rspp_look_ID.h
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

typedef enum RSPP_Det_Look_ID_Tag : int8_t
{
   RSPP_DET_LOOK_ID_INVALID = (-1),
   RSPP_DET_LOOK_ID_0 = (0), // LR, LL
   RSPP_DET_LOOK_ID_1 = (1), // LR, ML
   RSPP_DET_LOOK_ID_2 = (2), // MR, LL
   RSPP_DET_LOOK_ID_3 = (3), // MR, ML
   RSPP_DET_NUM_LOOK_ID = (4)
} RSPP_Det_Look_ID_T;

#endif
