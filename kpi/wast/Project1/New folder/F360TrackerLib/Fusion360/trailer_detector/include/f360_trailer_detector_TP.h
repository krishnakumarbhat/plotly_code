/*===========================================================================*\
 * FILE: f360_trailer_detector_TP.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer presence class, a unit of trailer detector module
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_TP_H
#define TRAILER_DETECTOR_TP_H

#include <algorithm>
#include "f360_trailer_detector_constants.h"
#include "f360_trailer_detector_outputs.h"
#include "f360_host.h"
#include "f360_constants.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   struct TP_Cals
   {
      float32_t k_speed_threshold = 0.2F; // [m/s] speed threshold (above) to activate trailer presence estimation
      float32_t k_speed_threshold_clear = 0.833F; // [m/s] speed threshold (below) to clear trailer presence estimation
      float32_t k_long_max = -5.7F; // [m] maximum vcs x position of detections to be scoped for trailer presence estimation
      float32_t k_long_min = -9.7F; // [m] minimum vcs x position of detections to be scoped for trailer presence estimation
      float32_t k_lat_abs = 1.75F; // [m] absolute maximum vcs y position to be scoped for trailer presence estimation
      float32_t k_rr_start = 0.0F; // [m/s] start of targetted range rate
      float32_t k_rr_end = 10.0F; // [m/s] end of targetted range rate
      float32_t k_rr_ratio_threshold = 0.5F; // range rate ratio threshold for deciding trailer presence
      float32_t k_rr_mean_threshold = 1.55F; // range rate mean threshold for deciding trailer d´presence
      uint32_t k_reset_cnt_threshold = 1800U; // radar cycles 1800*0.05 = 90s
      uint32_t k_time_threshold = 200U; // radar cycles 200*0.05 = 10s
   };

   struct TP_Relative_Ratio
   {
      int16_t dets_n_02;
      int16_t dets_n_03;
      int16_t dets_n_04;
      int16_t dets_n_05;
   };

   struct TP_Mean_RR
   {
      float32_t value_array[BIN_SIZE];
      int16_t num_array[BIN_SIZE];
   };

   class Trailer_Detector_TP
   {
   public:
      Trailer_Detector_TP()
      {
         Initialize();
      };

      ~Trailer_Detector_TP() {};

      void Initialize();

      void Clear();

      void Get_Output(TP_Output& output) const;

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

      static void Linspace(const float32_t first, const float32_t second, float32_t (&value_array)[BIN_SIZE]);

      static const TP_Cals tp_calibs;

      Trailer_Presence_State trailer_presence;
      Trailer_Detector_Conf trailer_presence_conf;
      uint32_t window_timer;
      bool f_estimation_done;
      uint32_t stationary_timer;
      TP_Relative_Ratio relative_ratio_struct;
      TP_Mean_RR mean_rr_struct;
   
   // debugging purpose only
   public:
      const uint32_t& Get_Window_Timer() const {return this->window_timer;};
      void Set_Window_Timer(const uint32_t& window_timer_input) { this->window_timer = window_timer_input;};
      const TP_Relative_Ratio& Get_Relative_Ratio_Struct() const {return this->relative_ratio_struct;};
      void Set_Relative_Ratio_Struct(const TP_Relative_Ratio& relative_ratio_struct_input) { this->relative_ratio_struct = relative_ratio_struct_input;};
      const TP_Mean_RR& Get_Mean_RR_Struct() const {return this->mean_rr_struct;};
      void Set_Mean_RR_Struct(const TP_Mean_RR& mean_rr_struct_input) { this->mean_rr_struct = mean_rr_struct_input;};
      const bool& Get_Estimation_Done_Flag() const {return this->f_estimation_done;};
      void Set_Estimation_Done_Flag(const bool& f_estimation_done_input) { this->f_estimation_done = f_estimation_done_input;};

      void Set_Trailer_Presence(const Trailer_Presence_State& trailer_presence_input) { this->trailer_presence = trailer_presence_input;};
      void Set_Trailer_Presence_Conf(const Trailer_Detector_Conf& trailer_presence_conf_input) { this->trailer_presence_conf = trailer_presence_conf_input;};
      void Set_Stationary_Timer(const uint32_t& stationary_timer_input) { this->stationary_timer = stationary_timer_input;};
   };
}
#endif
