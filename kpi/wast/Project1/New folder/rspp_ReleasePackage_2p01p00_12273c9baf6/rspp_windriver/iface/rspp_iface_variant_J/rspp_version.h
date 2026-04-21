#ifndef RSPP_VERSION_VARIANT_J_H
#define RSPP_VERSION_VARIANT_J_H
/*===================================================================================*\
* FILE: rspp_version.h
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

namespace rspp_variant_J {
   static const int8_t RSPP_Version_Major = 2;
   static const int8_t RSPP_Version_Minor = 1;
   static const int8_t RSPP_Version_Patch = 0;
}

#ifndef RSPP_VERSION_BUILD_ID
#define RSPP_VERSION_BUILD_ID 0x0000000
#endif

#endif
