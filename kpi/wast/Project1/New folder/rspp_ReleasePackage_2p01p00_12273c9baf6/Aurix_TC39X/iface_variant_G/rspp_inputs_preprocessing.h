#ifndef RSPP_INPUTS_PREPROCESSING_VARIANT_G_H
#define RSPP_INPUTS_PREPROCESSING_VARIANT_G_H
/*===========================================================================*\
* FILE: inputs_preprocessing.h
*============================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_core_info.h"
#include "rspp_host.h"
#include "rspp_detection_list.h"
#include "rspp_radar_sensor.h"
#include "rspp_calibrations.h"

namespace rspp_variant_G
{
   void Inputs_Preprocessing(
       const RSPP_Core_Info_T& core_info,
       const RSPP_Host_T& host,
       F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
       const RSPP_Calibrations_T& rspp_calibrations,
       RSPP_Detection_List_T& raw_detect_list);
}
#endif
