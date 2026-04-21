/*===========================================================================*\
* FILE: f360_calc_object_init_ep.h
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declaration for Sort_Clusters_By_Priority.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/
#ifndef F360_CALC_OBJECT_INIT_EP_H
#define F360_CALC_OBJECT_INIT_EP_H

#include "f360_reuse.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   float32_t Calc_Object_Init_Existence_Prob(
      const float32_t cluster_ep,
      const float32_t cluster_VCS_velocity_plausability,
      const F360_Calibrations_T &calib
   );
}

#endif
