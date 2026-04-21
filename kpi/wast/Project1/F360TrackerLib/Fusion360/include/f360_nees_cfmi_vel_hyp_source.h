/*===================================================================================*\
* FILE: f360_nees_cfmi_vel_hyp_source.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* Definition of NEES CFMI Velocity Hypothesis sources.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
* None.
*
\*===================================================================================*/

#ifndef F360_NEES_CFMI_VEL_HYP_SOURCE_H
#define F360_NEES_CFMI_VEL_HYP_SOURCE_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   enum F360_NEES_CFMI_Vel_Hyp_Source_T : uint8_t
   {
      // The most important hypothesis should be first.
      F360_NEES_CFMI_VEL_HYP_SOURCE_CLOUD = 0,
      F360_NEES_CFMI_VEL_HYP_SOURCE_CONF_VH = 1,
      F360_NEES_CFMI_VEL_HYP_SOURCE_DOMINANT = 2,
      F360_NEES_CFMI_VEL_HYP_SOURCE_RANSAC = 3,
      F360_NEES_CFMI_VEL_HYP_SOURCE_RANGE_RATE = 4,
      F360_NEES_CFMI_VEL_HYP_SOURCE_STATIONARY = 5,
      F360_NEES_CFMI_VEL_HYP_SOURCE_NONE = 6
   };
}
#endif
