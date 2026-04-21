/*===================================================================================*\
 * FILE: f360_nees_cfmi_post_processing.h
 *====================================================================================
 * Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *-----------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   Post-processing function for nees cost function initialization
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

#ifndef F360_NEES_CFMI_POST_PROCESSING_H
#define F360_NEES_CFMI_POST_PROCESSING_H

#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_host_props.h"

namespace f360_variant_A
{
   void NEES_CFMI_Init_Info_Post_Processing(
      const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      const bool valid_for_liberal_tracking);

   void NEES_CFMI_Estimate_Detection_Bounding_Box(
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);
}
#endif
