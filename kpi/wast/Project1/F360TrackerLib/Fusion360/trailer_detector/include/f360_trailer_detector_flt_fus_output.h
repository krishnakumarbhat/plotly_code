/*===========================================================================*\
 * FILE: f360_trailer_detector_flt_fus_output.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer detector fused output structure
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_FLT_FUS_OUTPUT_H
#define TRAILER_DETECTOR_FLT_FUS_OUTPUT_H

#include "f360_trailer_detector_constants.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   struct Trailer_Detector_Flt_Fus_Output {

      float32_t trailer_length; // [m] trailer length between tow hitch and trailer rear bumper
      float32_t trailer_width;  // [m] trailer width
      float32_t trailer_angle;  // [rad] angle between trailer orientation and host orientation
      float32_t trailer_angle_rate; // [rad/s] angular rate between trailer orientation and host orientation
      float32_t trailer_HV_gap; // [m] gap between host vehicle and trailer
      uint32_t radar_detection_timer; // [-] timer incrementing per execution during trailer estimation phase
      uint32_t stationary_timer;      // [-] timer to record execution index after convergence and host being stationary
      Trailer_Detector_Status trailer_detection_status; // [-] trailer detector status: Normal/Faulty/Unknown
      Trailer_Presence_State trailer_presence; // [-] trailer presence state: Detected/Not Detected/Unknown
      Trailer_Detector_Conf trailer_presence_conf; // [-] Unknown/Low/Median/High
      Trailer_Detector_Conf trailer_length_conf; // [-] Unknown/Low/Median/High
      Trailer_Detector_Conf trailer_width_conf; // [-] Unknown/Low/Median/High
      Trailer_Detector_Conf trailer_angle_conf; // [-] Unknown/Low/Median/High
      Trailer_Detector_Conf trailer_angle_rate_conf; // [-] Unknown/Low/Median/High

   };

}
#endif
