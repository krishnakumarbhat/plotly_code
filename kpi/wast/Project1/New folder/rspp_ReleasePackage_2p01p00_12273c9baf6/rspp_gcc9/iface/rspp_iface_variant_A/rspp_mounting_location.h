#ifndef RSPP_MOUNTING_LOCATION_H
#define RSPP_MOUNTING_LOCATION_H
/*===================================================================================*\
* FILE: rspp_Mounting_Location.h
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

typedef enum RSPP_Mounting_Location_Tag : int8_t
{
   RSPP_MOUNTING_LOCATION_UNKNOWN = (-1),
   RSPP_MOUNTING_LOCATION_LEFT_FORWARD = (0),
   RSPP_MOUNTING_LOCATION_LEFT_SIDE1 = (8),
   RSPP_MOUNTING_LOCATION_LEFT_SIDE2 = (16),
   RSPP_MOUNTING_LOCATION_LEFT_REAR = (24),
   RSPP_MOUNTING_LOCATION_CENTER_FORWARD = (1),
   RSPP_MOUNTING_LOCATION_CENTER_REAR = (25),
   RSPP_MOUNTING_LOCATION_RIGHT_FORWARD = (2),
   RSPP_MOUNTING_LOCATION_RIGHT_SIDE1 = (10),
   RSPP_MOUNTING_LOCATION_RIGHT_SIDE2 = (18),
   RSPP_MOUNTING_LOCATION_RIGHT_REAR = (26),
   RSPP_MOUNTING_LOCATION_CENTER2_FORWARD = (3),
   RSPP_MOUNTING_LOCATION_CENTER2_REAR = (27),
   RSPP_MOUNTING_LOCATION_CENTER3_FORWARD = (4),
   RSPP_MOUNTING_LOCATION_CENTER3_REAR = (28)
}
RSPP_Mounting_Location_T;

#endif
