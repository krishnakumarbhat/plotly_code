/*===================================================================================*\
* FILE:  f360_sensor_occlusion_info_helpers.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of helpers functions used by Sensor_Occlusion_Info_T class.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_sensor_occlusion_info_helpers.h"
#include "f360_occlusion_types.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Calc_Starting_Sector_IDx()
   *===========================================================================
   * RETURN VALUE:
   * int32_t sector_idx - index of closest left sector to given point
   *
   * PARAMETERS:
   * const float32_t azimuth - given point azimuth
   * const float32_t min_az - reference azimuth w.r.t. starting_sector_idx is calculated
   * const float32_t sectors_width - width of sectors
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
   * This function calculates index of closest index of sector to the left side of point.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   int32_t Calc_Starting_Sector_IDx(
      const float32_t azimuth, 
      const float32_t min_az, 
      const float32_t sectors_width)
   {
      int32_t sector_idx = static_cast<int32_t>(F360_Floorf((azimuth - min_az) / sectors_width));
      sector_idx = Clamp(sector_idx, 0, static_cast<int32_t>(occlusion_num_sectors - 1U));
      return sector_idx;
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Ending_Sector_IDx()
   *===========================================================================
   * RETURN VALUE:
   * int32_t sector_idx - index of closest right sector to given point
   *
   * PARAMETERS:
   * const float32_t azimuth - given point azimuth
   * const float32_t min_az - reference azimuth w.r.t. starting_sector_idx is calculated
   * const float32_t sectors_width - width of sectors
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
   * This function calculates index of closest index of sector to the right side of point.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   int32_t Calc_Ending_Sector_IDx(
      const float32_t azimuth, 
      const float32_t min_az, 
      const float32_t sectors_width)
   {
      int32_t sector_idx = static_cast<int32_t>(F360_Ceilf((azimuth - min_az) / sectors_width));
      sector_idx = Clamp(sector_idx, 0, static_cast<int32_t>(occlusion_num_sectors - 1U));
      return sector_idx;
   }
}

