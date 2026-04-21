/*===========================================================================*\
 * FILE: f360_trailer_detector_TL.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer length estimation class, a unit of trailer detector module
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_TL_H
#define TRAILER_DETECTOR_TL_H

/*===========================================================================*\
* Includes
\*===========================================================================*/
#include <algorithm>
#include "f360_host.h"
#include "f360_constants.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_math_func.h"
#include "f360_trailer_detector_constants.h"
#include "f360_trailer_detector_outputs.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   struct TL_Cals
   {
      float32_t k_speed_threshold = 0.2F; // [m/s] speed threshold to activate trailer detector feature
      float32_t k_ZRRateGate = 0.3F; // [m/s] detection range rate gate
      float32_t k_host_length = 5.5F; // [m] host vehicle length
      float32_t k_max_trailer_length = 12.0F; // [m] maximum trailer length
      float32_t k_max_trailer_width = 3.0F; // [m] maximum trailer width
      float32_t k_row_interval = 0.2F; // [m] detection row array interval
      uint32_t k_window_timer_threshold = 1200U; // 1200 execution loops = 60s
      uint32_t k_reset_time_threshold = 1800U; // reset timer to clear TL estimation, 1800 execution loops = 90s
   };

   struct TL_Trailer_Detection_Row
   {
      int32_t detection_row[ROW_NUMBER];
   };

   struct SVM_Output
   {
      float32_t trailer_length_SVM;
      float32_t trailer_length_SVM_conf;
   };

   struct TL_Peak
   {
      int32_t peak_pos;
      int32_t peak_val;
      int32_t peak_left_radius;
      int32_t peak_right_radius;
   };

   struct TL_Area
   {
      int32_t starting_pos;
      int32_t ending_pos;
      int32_t ref_val;
      float32_t mean_val;
      float32_t max_val;
   };

   class Trailer_Detector_TL
   {
   public:
      Trailer_Detector_TL()
      {
         Initialize();
      };

      ~Trailer_Detector_TL() {};

      void Initialize();

      void Clear();

      void Get_Output(TL_Output& output) const;

      void Run_Trailer_Detector(
         const F360_Host_T &vehicle_data,
         const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
         const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]);

   protected:
      void Process_Input(
         const F360_Host_T&vehicle_data,
         const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
         const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]);

      void Estimate();

      void Adjust_Sample();

      void Locate_Peaks();

      void Find_Peak_Group_Info(
         const int32_t(&detection_row)[ROW_NUMBER], 
         const float32_t threshold_forward, 
         const float32_t threshold_backward,
         const uint32_t peak_gap_max, 
         const uint32_t peak_r_gap_max,
         const int32_t max_val,
         TL_Peak (&peak_list_point)[PEAK_GROUP_SIZE], 
         int32_t& peak_cnt);

      static bool Is_Peak(
         const int32_t(&sample)[ROW_NUMBER], 
         const uint32_t index);

      static void Cal_Radius(
         const int32_t(&sample)[ROW_NUMBER], 
         const uint32_t index, 
         uint32_t& r_left, 
         uint32_t& r_right);

      void Estimate_Trailer_Length_Peaks();

      static int32_t Peak_Left_Edge(
         const TL_Peak(&peak_group)[PEAK_GROUP_SIZE], 
         const int32_t pos);

      static int32_t Peak_Right_Edge(
         const TL_Peak(&peak_group)[PEAK_GROUP_SIZE], 
         const int32_t pos);

      void Get_Area_Info(TL_Area& area_info) const;

      void Shrink_Trailer_Length(
         const bool f_noise, 
         const int32_t i_array_max, 
         const int32_t j_array_max,
         float32_t& last_peak,
         bool& f_shrink);
     
      void Extend_Trailer_Length(
         const bool f_noise, 
         const int32_t i_array_max, 
         const int32_t j_array_max, 
         const int32_t k_array_max,
         float32_t& last_peak);

      void Estimate_Trailer_Length_SVM();

      static void Norm_Detection_Row(
         const int32_t(&detection_row)[ROW_NUMBER], 
         float32_t (&norm_sample)[ROW_NUMBER]);

      void SVM_Classification(
         const float32_t(&norm_sample)[ROW_NUMBER], 
         int32_t& predict_class, 
         float32_t& max_score);

      void SVM_confidence(
         const int32_t predict_class, 
         const float32_t max_score);

      void Post_Processing();

      static void Increase_Detection_Row(
         int32_t (&detection_row_new)[ROW_NUMBER], 
         int32_t (&detection_row)[ROW_NUMBER]);

      static const TL_Cals tl_calibs;

      bool f_estimation_done;

      uint32_t reset_timer;

      uint32_t window_timer;

      TL_Trailer_Detection_Row trailer_detection_row_struct;
      
      float32_t trailer_length;

      float32_t trailer_length_peaks;

      SVM_Output svm_outputs;

      float32_t trailer_HV_gap;

      uint8_t trailer_length_conf;

      float32_t axle_trailer_length;

      int32_t first_peak_cnt;

      int32_t second_peak_cnt;

      int32_t third_peak_cnt;

      bool first_peak_extension;

      TL_Peak first_peak_group[PEAK_GROUP_SIZE];

      TL_Peak second_peak_group[PEAK_GROUP_SIZE];

      TL_Peak third_peak_group[PEAK_GROUP_SIZE];

      TL_Area front_area;

      TL_Area middle_area;

   // debugging purpose only
   public:
      const bool& Get_Estimation_Done_Flag() const {return this->f_estimation_done;};
      void Set_Estimation_Done_Flag(const bool& f_estimation_done_input) { this->f_estimation_done = f_estimation_done_input;};
      const uint32_t& Get_Reset_Timer() const {return this->reset_timer;};
      void Set_Reset_Timer(const uint32_t& reset_timer_input) { this->reset_timer = reset_timer_input;};
      const uint32_t& Get_Window_Timer() const {return this->window_timer;};
      void Set_Window_Timer(const uint32_t& window_timer_input) { this->window_timer = window_timer_input;};
      const float32_t& Get_Trailer_Length_Struct() const {return this->trailer_length;};
      void Set_Trailer_Length_Struct(const float32_t& trailer_length_input) { this->trailer_length = trailer_length_input;};
      const float32_t& Get_Axle_Trailer_Length_Struct() const {return this->axle_trailer_length;};
      void Set_Axle_Trailer_Length_Struct(const float32_t& axle_trailer_length_input) { this->axle_trailer_length = axle_trailer_length_input;};
      const TL_Trailer_Detection_Row& Get_Trailer_Detection_Row() const {return this->trailer_detection_row_struct;};
      void Set_Trailer_Detection_Row(const TL_Trailer_Detection_Row& trailer_detection_row_struct_input) { this->trailer_detection_row_struct = trailer_detection_row_struct_input;};
   };
}   
#endif
