#ifndef RSPP_SENSOR_MOUNTING_POSITION_H
#define RSPP_SENSOR_MOUNTING_POSITION_H
/*===================================================================================*\
* FILE: rspp_sensor_mounting_position.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_position.h"

typedef struct RSPP_Sensor_Mounting_Position_Tag
{
   RSPP_VCS_Position_T vcs_position;
   float32_t vcs_boresight_azimuth_angle;
   float32_t vcs_boresight_elevation_angle;
} RSPP_Sensor_Mounting_Position_T;

#endif
