/*===================================================================================*\
* FILE:  f360_calc_point_scs_position.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Calc_Point_SCS_Position() function.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef CALC_POINT_SCS_POSITION_H
#define CALC_POINT_SCS_POSITION_H

#include "f360_occlusion_types.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   SCS_Position_T Calc_Point_SCS_Position(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn,
      const Sensor_Mounting_Position& sens_mount_pos);
}
#endif
