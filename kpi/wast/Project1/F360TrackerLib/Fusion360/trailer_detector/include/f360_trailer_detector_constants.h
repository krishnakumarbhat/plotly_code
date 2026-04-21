/*===========================================================================*\
 * FILE: f360_trailer_detector_constants.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer detector constants
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_CONSTANTS_H
#define TRAILER_DETECTOR_CONSTANTS_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   constexpr uint32_t ROW_NUMBER = 60U; // Trailer detection row array size
   constexpr int32_t SVM_CONF_LEVEL = 10; // SVM confidence level
   constexpr uint32_t COL_NUMBER = 30U; // Trailer detection column array size
   constexpr uint32_t CLASS_NUMBER = 20U; // Number of trailer types 
   constexpr int32_t PEAK_GROUP_SIZE = 20; // Peak group size
   constexpr uint8_t BIN_SIZE = 100U; // Trailer presence value array bin size

   enum Trailer_Presence_State : uint8_t
   {
      TRAILER_PRESENCE_STATE_NOT_DETECTED = 0U,
      TRAILER_PRESENCE_STATE_DETECTED = 1U,
      TRAILER_PRESENCE_STATE_UNKNOWN = 2U // Default State
   };

   enum Trailer_Detector_Conf : uint8_t
   {
      TRAILER_DETECTOR_CONF_UNKNOWN = 0U, // Default State
      TRAILER_DETECTOR_CONF_LOW = 1U,
      TRAILER_DETECTOR_CONF_MEDIAN = 2U,
      TRAILER_DETECTOR_CONF_HIGH = 3U,
   };

   enum Trailer_Detector_Status : uint8_t
   {
      TRAILER_DETECTOR_STATUS_NOT_RUNNING = 0U,
      TRAILER_DETECTOR_STATUS_RUNNING = 1U,
      TRAILER_DETECTOR_STATUS_UNKNOWN = 2U, // Default State
   };
}
#endif
