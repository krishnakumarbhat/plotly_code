#ifndef RSPP_SENSOR_TYPE_H
#define RSPP_SENSOR_TYPE_H
/*===================================================================================*\
* FILE: rspp_sensor_type.h
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

typedef enum RSPP_Sensor_Type_Tag : int8_t
{
   RSPP_SENSOR_TYPE_UNKNOWN         = (-1),
   RSPP_SENSOR_TYPE_SRR2_RADAR      = (0),
   RSPP_SENSOR_TYPE_SRR4_RADAR      = (1),
   RSPP_SENSOR_TYPE_SRR4_MM_RADAR   = (2),
   RSPP_SENSOR_TYPE_SRR5_RADAR      = (3),
   RSPP_SENSOR_TYPE_MRR360_RADAR    = (4),
   RSPP_SENSOR_TYPE_ESR_RADAR       = (5),
   RSPP_SENSOR_TYPE_MRR1_RADAR      = (6),
   RSPP_SENSOR_TYPE_MRR2_RADAR      = (7),
   RSPP_SENSOR_TYPE_MRR3_RADAR      = (8),
   RSPP_SENSOR_TYPE_LIDAR           = (9),
   RSPP_SENSOR_TYPE_VISION          = (10),
   RSPP_SENSOR_TYPE_VEHICLE         = (11),
   RSPP_SENSOR_TYPE_FLR4_RADAR      = (12),
   RSPP_SENSOR_TYPE_FLR4_PLUS_RADAR = (13),
   RSPP_SENSOR_TYPE_SRR6_RADAR      = (14),
   RSPP_SENSOR_TYPE_SRR6_PLUS_RADAR = (15),
   RSPP_SENSOR_TYPE_SRR7_PLUS_RADAR = (16),
   RSPP_SENSOR_TYPE_FLR7_RADAR      = (17),
   RSPP_SENSOR_TYPE_SRR6_PLUS_PLT_RADAR = (18),
   RSPP_SENSOR_TYPE_SRR6_PLUS_PLT_1XCAN_RADAR = (19),
   RSPP_SENSOR_TYPE_SRR6_PLUS_PLT_1XETH_RADAR = (20),
   RSPP_SENSOR_TYPE_SRR6_PLUS_PLT_1GB_EHT_RADAR = (21),
   RSPP_SENSOR_TYPE_SRR7_PLUS_PLT_RADAR = (22),
   RSPP_SENSOR_TYPE_FLR7_PLT_RADAR = (23),
   RSPP_SENSOR_TYPE_FLR4_PLUS_PLT_STANDALONE_RADAR = (24),
   RSPP_SENSOR_TYPE_FLR4_PLT_RADAR = (25),
   RSPP_SENSOR_TYPE_FLR4_PLT_STANDALONE_RADAR = (26)
} RSPP_Sensor_Type_T;

#endif
