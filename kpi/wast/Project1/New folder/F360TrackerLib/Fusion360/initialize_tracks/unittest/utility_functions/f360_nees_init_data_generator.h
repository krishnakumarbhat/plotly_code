/*===================================================================================*\
* FILE: f360_nees_init_data_generator.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains nees initialization internal functions data generation
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#ifndef F360_NEES_INIT_DATA_GENERATOR_H
#define F360_NEES_INIT_DATA_GENERATOR_H

#include "f360_initialize_tracks.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void Fill_Valid_NEES_CFMI_Information(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const uint16_t num_of_detections,
      const uint16_t num_of_velocities);
}
#endif
