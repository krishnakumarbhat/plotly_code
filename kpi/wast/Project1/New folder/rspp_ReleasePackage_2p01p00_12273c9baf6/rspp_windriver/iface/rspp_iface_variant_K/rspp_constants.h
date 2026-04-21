#ifndef RSPP_CONSTANTS_VARIANT_K_H
#define RSPP_CONSTANTS_VARIANT_K_H
/*===========================================================================*\
* FILE: rspp_constants.h
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
#include "rspp_variant_definition.h"
namespace rspp_variant_K
{
   static constexpr uint16_t MAX_NUMBER_OF_DETECTIONS = ((MAX_NUMBER_OF_SRR_SENSORS * NUMBER_OF_SRR_DETECTIONS) + (MAX_NUMBER_OF_MRR_SENSORS * NUMBER_OF_MRR_DETECTIONS));
   static constexpr uint8_t MAX_NUMBER_OF_SENSORS = (MAX_NUMBER_OF_SRR_SENSORS + MAX_NUMBER_OF_MRR_SENSORS);
   static constexpr uint16_t MAX_DETS_FOR_SINGLE_SENSOR = (NUMBER_OF_SRR_DETECTIONS > NUMBER_OF_MRR_DETECTIONS)? NUMBER_OF_SRR_DETECTIONS:NUMBER_OF_MRR_DETECTIONS;

   static constexpr uint8_t MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS = 31U; // Number of reference points for detections in vcs long sorted order
   static constexpr uint8_t MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS_ELEMENTS = (MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS + 2U); // Array of reference detection indexes is appended with smallest VCS-long position at first element and largst VCS-long position at last invalid element
   static constexpr int32_t RSPP_INVALID_ID = -1;
}
#endif
