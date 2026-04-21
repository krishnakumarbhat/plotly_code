#ifndef RSPP_DETECTION_MOTION_STATUS_VARIANT_F_H
#define RSPP_DETECTION_MOTION_STATUS_VARIANT_F_H
/*===================================================================================*\
* FILE: rspp_detection_motion_status.h
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

namespace rspp_variant_F
{
   enum RSPP_Detection_Motion_Status_T : int8_t
   {
      RSPP_DETECTION_MOTION_STATUS_INVALID = -1,
      RSPP_DETECTION_MOTION_STATUS_STATIONARY = 0,
      RSPP_DETECTION_MOTION_STATUS_MOVING = 1,
      RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS = 2
   };
}
#endif
