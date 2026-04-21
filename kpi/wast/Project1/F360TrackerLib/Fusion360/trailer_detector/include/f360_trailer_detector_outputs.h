/*===========================================================================*\
 * FILE: f360_trailer_detector_outputs.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer detector internal output structure
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_OUTPUT_H
#define TRAILER_DETECTOR_OUTPUT_H

#include "f360_reuse.h"
#include "f360_trailer_detector_constants.h"

namespace f360_variant_A
{
   class TP_Output
   {
   public:
      TP_Output()
      {
         trailer_presence = TRAILER_PRESENCE_STATE_NOT_DETECTED;
         confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
         f_estimation_done = false;
         stationary_timer = 0U;
      }

      Trailer_Presence_State trailer_presence;
      Trailer_Detector_Conf confidence;
      uint32_t stationary_timer;
      bool f_estimation_done;
   };

   class TL_Output
   {
   public:
      TL_Output()
      {
         trailer_length = 0.0F;
         axle_trailer_length = 0.0F;
         confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
         f_estimation_done = false;
      }

      float32_t trailer_length;
      float32_t axle_trailer_length;
      Trailer_Detector_Conf confidence;
      bool f_estimation_done;
   };

   class TW_Output
   {
   public:
      TW_Output()
      {
         trailer_width = 0.0F;
         confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
         f_estimation_done = false;
      }

      float32_t trailer_width;
      Trailer_Detector_Conf confidence;
      bool f_estimation_done;
   };

   class TA_Output
   {
   public:
      TA_Output()
      {
         trailer_angle_rad = 0.0F;
         trailer_angle_rate_rad = 0.0F;
         confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
      }

      float32_t trailer_angle_rad;
      float32_t trailer_angle_rate_rad;
      Trailer_Detector_Conf confidence;
   };

   class Trailer_Detector_Outputs
   {
   public:
      Trailer_Detector_Outputs() :
         tp_outputs(),
         tl_outputs(),
         tw_outputs(),
         ta_outputs()
      {
         radar_detection_timer = 0U;
         trailer_detection_status = TRAILER_DETECTOR_STATUS_UNKNOWN;
      };

      TP_Output tp_outputs;
      TL_Output tl_outputs;
      TW_Output tw_outputs;
      TA_Output ta_outputs;

      uint32_t radar_detection_timer;
      Trailer_Detector_Status trailer_detection_status;

      // debugging purpose only
      void Set_Radar_Detection_Timer(const uint32_t& radar_detection_timer_input) { this->radar_detection_timer = radar_detection_timer_input;};
   };
}

#endif
