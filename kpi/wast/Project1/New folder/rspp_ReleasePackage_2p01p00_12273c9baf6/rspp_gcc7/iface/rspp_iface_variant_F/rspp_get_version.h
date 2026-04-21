#ifndef RSPP_GET_VERSION_VARIANT_F_H
#define RSPP_GET_VERSION_VARIANT_F_H
/*===========================================================================*\
* FILE: rspp_get_version.h
*============================================================================
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
   void RSPP_Get_Version(int8_t* const major, int8_t* const minor, int8_t* const patch, uint64_t* const build_id);
}
#endif
