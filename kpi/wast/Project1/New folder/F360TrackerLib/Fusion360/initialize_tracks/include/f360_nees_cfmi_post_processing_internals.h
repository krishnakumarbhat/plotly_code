/*===================================================================================*\
 * FILE: f360_nees_cfmi_post_processing_internals.h
 *====================================================================================
 * Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *-----------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   Internal post-processing function for nees cost function initialization (needed for UT)
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

#ifndef F360_NEES_CFMI_POST_PROCESSING_INTERNALS_H
#define F360_NEES_CFMI_POST_PROCESSING_INTERNALS_H

#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void Update_NEES_CFMI_Init_Scheme(F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   void Velocity_Plausibility_Checks(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Velocity_T& nees_cfmi_velocity);

   void Initialization_Sanity_Checks(
      const F360_Calibrations_T& calibrations,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);
}
#endif
