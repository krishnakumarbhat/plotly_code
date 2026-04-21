/*===================================================================================*\
* FILE:  f360_sensor_occlusion_info_helpers.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of helpers functions used by Sensor_Occlusion_Info_T class.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_SENSOR_OCCLUSION_INFO_HELPERS_H
#define F360_SENSOR_OCCLUSION_INFO_HELPERS_H

#include "f360_reuse.h"

namespace f360_variant_A
{

   int32_t Calc_Starting_Sector_IDx(
      const float32_t azimuth,
      const float32_t min_az,
      const float32_t sectors_width);

   int32_t Calc_Ending_Sector_IDx(
      const float32_t azimuth,
      const float32_t min_az,
      const float32_t sectors_width);
}
#endif
