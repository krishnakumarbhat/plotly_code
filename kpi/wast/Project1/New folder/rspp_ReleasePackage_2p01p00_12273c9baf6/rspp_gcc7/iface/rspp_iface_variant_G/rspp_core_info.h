#ifndef RSPP_CORE_INFO_H
#define RSPP_CORE_INFO_H
/*===================================================================================*\
* FILE: rspp_core_info.h
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

struct RSPP_Core_Info_T
{
   uint64_t time_us;
   uint64_t prev_time_us;
   uint32_t cnt_loops;
   float32_t elapsed_time_s;     
};

#endif
