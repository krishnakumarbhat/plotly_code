#ifndef F360_TRACKER_VARIANT_TYPE_H
#define F360_TRACKER_VARIANT_TYPE_H

/*===================================================================================*\
* FILE: f360_variant_type.h
*====================================================================================
* Copyright (C) 2022 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*  Definition of variant types
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

#include "f360_reuse.h"

typedef enum F360_Tracker_Variant_Type_Tag : uint8_t
{
   F360_VARIANT_TYPE_A = (0),
   F360_VARIANT_TYPE_B = (1),
   F360_VARIANT_TYPE_C = (2),
   F360_VARIANT_TYPE_D = (3),
   F360_VARIANT_TYPE_E = (4),
   F360_VARIANT_TYPE_F = (5),
   F360_VARIANT_TYPE_G = (6),
   F360_VARIANT_TYPE_H = (7),
   F360_VARIANT_TYPE_I = (8),
   F360_VARIANT_TYPE_J = (9),
   F360_VARIANT_TYPE_K = (10),
} F360_Tracker_Variant_T;

#endif
