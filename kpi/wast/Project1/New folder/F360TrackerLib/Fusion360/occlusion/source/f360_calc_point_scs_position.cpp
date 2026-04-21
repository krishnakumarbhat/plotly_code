/*===================================================================================*\
* FILE:  f360_calc_point_scs_position.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Calc_Point_SCS_Position() function.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_calc_point_scs_position.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Calc_Point_SCS_Position()
   *===========================================================================
   * RETURN VALUE:
   * SCS_Position_T scs_pos - point SCS position, calculated coordinates are writen into this structure
   *
   * PARAMETERS:
   * const float32_t vcs_long_posn - given point VCS longitudinal position
   * const float32_t vcs_lat_posn - given point VCS lateral position
   * const Sensor_Mounting_Position& sens_mount_pos - sensor mounting position, given in VCS
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates point position in sector coordinate system.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   SCS_Position_T Calc_Point_SCS_Position(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn,
      const Sensor_Mounting_Position& sens_mount_pos)
   {
      const float32_t lon_scs = vcs_long_posn - sens_mount_pos.vcs_position.x;
      const float32_t lat_scs = vcs_lat_posn - sens_mount_pos.vcs_position.y;

      const float32_t cos_bs_ang = sens_mount_pos.cos_vcs_borseight_azimuth_angle;
      const float32_t sin_bs_ang = sens_mount_pos.sin_vcs_borseight_azimuth_angle;

      const float32_t x = cos_bs_ang * lon_scs + sin_bs_ang * lat_scs;
      const float32_t y = -sin_bs_ang * lon_scs + cos_bs_ang * lat_scs;

      SCS_Position_T scs_pos;
      scs_pos.polar.azimuth = F360_Atan2f(y, x);
      scs_pos.polar.range = F360_Get_Hypotenuse(y, x);
      scs_pos.cart.x = x;
      scs_pos.cart.y = y;

      return scs_pos;
   }
}

