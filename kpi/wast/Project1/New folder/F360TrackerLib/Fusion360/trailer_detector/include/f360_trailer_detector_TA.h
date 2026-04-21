/*===========================================================================*\
 * FILE: f360_trailer_detector_TA.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer angle estimation class, a unit of trailer detector module
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_TA_H
#define TRAILER_DETECTOR_TA_H

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include <algorithm>
#include "f360_trailer_detector_constants.h"
#include "f360_trailer_detector_outputs.h"
#include "f360_host.h"
#include "f360_constants.h"
#include "f360_detection_props.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   struct TA_Cals
   {
      float32_t true_HV_length = 5.8F; // [m] host vehicle length
      float32_t speed_threshold = 0.2F; // [m/s] speed threshold to activate trailer detector feature
      int16_t HV_cnt_threshold = 20; // [-] counter threshold for trailer angle estimation 
      float32_t delta_t = 0.05F; // [s/cycle] cyclic execution time
      float32_t b2_default = 4.0F; // [m] trailer axle length
   };

   class Trailer_Detector_TA
   {
   public:
      Trailer_Detector_TA()
      {
         Initialize();
      };

      ~Trailer_Detector_TA() {};

      void Initialize();

      void Clear();

      void Get_Output(TA_Output& output) const;

      void Set_Trailer_Axle_Length(const float32_t trailer_axle_length_in)
      {
         trailer_axle_length = trailer_axle_length_in;
      }

      void Run_Trailer_Detector(const F360_Host_T &vehicle_data, const float32_t elapsed_time_s);

   protected:
      void Estimate(const F360_Host_T &vehicle_data, const float32_t elapsed_time_s);

      static const TA_Cals ta_calibs;

      int16_t HV_cnt;
      bool HV_start;
      float32_t HV_angle;
      float32_t prev_trailer_angle;
      float32_t trailer_angle_rad;
      float32_t trailer_angle_rate_rad;
      float32_t trailer_axle_length;

   // debugging purpose only
   public:
      const int16_t& Get_HV_Cnt() const {return this->HV_cnt;};
      void Set_HV_Cnt(const int16_t& HV_cnt_input) { this->HV_cnt = HV_cnt_input;};
      const float32_t& Get_HV_Angle() const {return this->HV_angle;};
      void Set_HV_Angle(const float32_t& HV_angle_input) { this->HV_angle = HV_angle_input;};
      const float32_t& Get_Prev_Trailer_Angle() const {return this->prev_trailer_angle;};
      void Set_Prev_Trailer_Angle(const float32_t& prev_trailer_angle_input) { this->prev_trailer_angle = prev_trailer_angle_input;};
      const float32_t& Get_Trailer_Axle_Length() const {return this->trailer_axle_length;};

      void Set_Trailer_Angle_Rad(const float32_t& trailer_angle_rad_input) { this->trailer_angle_rad = trailer_angle_rad_input;};
      void Set_Trailer_Angle_Rate_Rad(const float32_t& trailer_angle_rate_rad_input) { this->trailer_angle_rate_rad = trailer_angle_rate_rad_input;};

   };
}
#endif
