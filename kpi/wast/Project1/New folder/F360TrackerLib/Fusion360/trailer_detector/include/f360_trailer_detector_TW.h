/*===========================================================================*\
 * FILE: f360_trailer_detector_TW.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer width estimation class, a unit of trailer detector module
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_TW_H
#define TRAILER_DETECTOR_TW_H

#include <algorithm>
#include "f360_trailer_detector_constants.h"
#include "f360_trailer_detector_outputs.h"
#include "f360_host.h"
#include "f360_constants.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   struct TW_Cals
   {
      float32_t k_speed_threshold = 0.2F; // [m/s] speed threshold (above) to activate trailer width estimation
      float32_t k_ZRRateGate = 0.3F; // [m/s] detection range rate gate
      float32_t k_host_length = 5.5F; // [m] host vehicle length
      float32_t k_max_trailer_length = 12.0F; // [m] maximum trailer length
      float32_t k_max_trailer_width = 3.0F; // [m] maximum trailer width
      float32_t k_col_interval = 0.1F; // [m] detection column array interval
      uint32_t k_window_timer_threshold = 800U; // radar cycles 800*0.05 = 40s
      uint32_t k_reset_time_threshold = 1800U; // radar cycles 1800*0.05 = 90s
   };

   class Trailer_Detector_TW
   {
   public:

      Trailer_Detector_TW()
      {
         Initialize();
      };

      ~Trailer_Detector_TW() {};

      void Initialize();

      void Clear();

      void Get_Output(TW_Output& output) const;

      void Run_Trailer_Detector(const F360_Host_T& vehicle_data,
         const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
         const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]);

   protected:
      void Process_Input(const F360_Host_T& vehicle_data,
         const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
         const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]);

      void Estimate();

      static const TW_Cals tw_calibs;

      uint32_t reset_timer;
      uint32_t window_timer;
      float32_t trailer_width;
      Trailer_Detector_Conf trailer_width_conf;
      bool f_estimation_done;
      uint32_t detection_col[COL_NUMBER];

   // debugging purpose only
   public:
      const uint32_t& Get_Reset_Timer() const {return this->reset_timer;};
      void Set_Reset_Timer(const uint32_t& reset_timer_input) { this->reset_timer = reset_timer_input;};
      const uint32_t& Get_Window_Timer() const {return this->window_timer;};
      void Set_Window_Timer(const uint32_t& window_timer_input) { this->window_timer = window_timer_input;};
      const bool& Get_Estimation_Done_Flag() const {return this->f_estimation_done;};
      void Set_Estimation_Done_Flag(const bool& f_estimation_done_input) { this->f_estimation_done = f_estimation_done_input;};
      const uint32_t* Get_Detection_Col_Begin() const {return this->detection_col;};
      const uint32_t* Get_Detection_Col_End() const { return &(this->detection_col[COL_NUMBER]); };
      void Set_Detection_Col(const uint32_t(&detection_col_input)[COL_NUMBER]) { (void)std::copy(cmn::begin(detection_col_input), cmn::end(detection_col_input), this->detection_col);};
   
      void Set_Trailer_Width(const float32_t& trailer_width_input) { this->trailer_width = trailer_width_input;};
      void Set_Trailer_Width_Conf(const Trailer_Detector_Conf& trailer_width_conf_input) { this->trailer_width_conf = trailer_width_conf_input;};
   };
}
#endif
