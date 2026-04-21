/*===========================================================================*\
 * FILE: tw_run.cpp
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer wdith run function and Get_Output function
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_trailer_detector_TW.h"
#include "f360_trailer_detector_constants.h"

namespace f360_variant_A
{
   const TW_Cals Trailer_Detector_TW::tw_calibs{};
   /*=========================================================================
    * Method         Trailer_Detector_TW::Run_Trailer_Detector
    *
    * Description    Public method to run trailer width estimation
    *
    * Parameters     const F360_Host_T &vehicle_data,
                     const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
                     const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
                     const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Run_Trailer_Detector(const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);

      Estimate();
   }

   /*=========================================================================
    * Method         Trailer_Detector_TW::Get_Output
    *
    * Description    Public method to get trailer width estimation output
    *
    * Parameters     TW_Output& output
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Get_Output(TW_Output& output) const
   {
      output.trailer_width = trailer_width;
      output.confidence = trailer_width_conf;
      output.f_estimation_done = f_estimation_done;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TW::Initialize
    *
    * Description    Public function to Initialize trailer width class
    *
    * Parameters     None.
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Initialize()
   {
      f_estimation_done = false;

      trailer_width = 0.0F;

      trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;

      reset_timer = 0U;

      window_timer = 0U;

      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         detection_col[i] = 0U;
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TW::clear
    *
    * Description    Public function to clear member variables of trailer width class
    *
    * Parameters     None.
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Clear()
   {
      f_estimation_done = false;

      trailer_width = 0.0F;

      trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;

      window_timer = 0U;

      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         detection_col[i] = 0U;
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TW::Process_Input
    *
    * Description    Private function used to process host and processed detection data
    *
    * Parameters     const F360_Host_T &vehicle_data,
                     const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
                     const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
                     const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Process_Input(const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      if (vehicle_data.speed <= tw_calibs.k_speed_threshold)
      {
         reset_timer = std::min(reset_timer + 1U, tw_calibs.k_reset_time_threshold);
         if (reset_timer == tw_calibs.k_reset_time_threshold)
         {
            Clear();
         }
      }
      else if (window_timer < tw_calibs.k_window_timer_threshold)
      {
         reset_timer = 0U;
         window_timer = std::min(window_timer + 1U, tw_calibs.k_window_timer_threshold);

         for (uint32_t det_idx = 0U; det_idx < raw_detect_list.number_of_valid_detections; det_idx++)
         {
            const rspp_variant_A::RSPP_Detection_T current_detection = raw_detect_list.detections[det_idx];
            const F360_Detection_Props_T& current_detection_prop = all_detections[det_idx];
            const int32_t current_sensor_id = current_detection.raw.sensor_id;
            const F360_Radar_Sensor_T& current_sensor = sensors[current_sensor_id - 1];

            const float32_t half_width = (static_cast<float32_t>(COL_NUMBER) * tw_calibs.k_col_interval) * 0.5F;
            // check detection validation
            const bool f_trailer_detection =
               (((F360_MOUNTING_LOCATION_RIGHT_REAR == current_sensor.constant.mounting_location) ||
                  (F360_MOUNTING_LOCATION_LEFT_REAR == current_sensor.constant.mounting_location) ||
                  (F360_MOUNTING_LOCATION_CENTER_REAR == current_sensor.constant.mounting_location)) &&
                  (std::abs(current_detection.raw.range_rate) < tw_calibs.k_ZRRateGate) &&
                  (current_detection_prop.vcs_position.x <= -tw_calibs.k_host_length) &&
                  (current_detection_prop.vcs_position.x >= -(tw_calibs.k_host_length + tw_calibs.k_max_trailer_length)) &&
                  (std::abs(current_detection_prop.vcs_position.y) <= half_width) &&
                  (!current_detection_prop.f_double_bounce) &&
                  (F360_DETECTION_WHEELSPIN_TYPE_INVALID == current_detection_prop.wheel_spin_type) &&
                  (!current_detection_prop.f_water_spray));

            if (f_trailer_detection)
            {
               uint8_t col_index = 0U;
               
               col_index = static_cast<uint8_t>(std::floor((half_width + current_detection_prop.vcs_position.y) / tw_calibs.k_col_interval));
               col_index = Clamp(col_index, static_cast<uint8_t>(0U), static_cast<uint8_t>(COL_NUMBER-1U));
               // accumulate detections
               detection_col[col_index] += 1U;
            }
         }
      }
      else
      {
         // do nothing
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TW::Estimate
    *
    * Description    Private method used to estimate trailer width
    *
    * Parameters     None.
    *
    * Returns        None.
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void Trailer_Detector_TW::Estimate()
   {
      if ((trailer_width < 0.5F) && (window_timer == tw_calibs.k_window_timer_threshold))
      {
         uint32_t left_index = 0U;
         uint32_t left_max = detection_col[left_index];
         uint32_t right_index = COL_NUMBER - 1U;
         uint32_t right_max = detection_col[right_index];

         const uint32_t col_index_half = COL_NUMBER / 2U;
         for (uint32_t i = 0U; i < col_index_half; i++)
         {
            if (detection_col[i] > left_max)
            {
               left_max = detection_col[i];
               left_index = i;
            }

            if (detection_col[i + col_index_half] > right_max)
            {
               right_max = detection_col[i + col_index_half];
               right_index = i + col_index_half;
            }
         }

         // width extension, static cast to signed as it is non-trivial if these are always positive
         int32_t left_index_update = static_cast<int32_t>(left_index);
         int32_t right_index_update = static_cast<int32_t>(right_index);

         const float32_t left_max_update = static_cast<float32_t>(left_max) * 0.5F;
         const float32_t right_max_update = static_cast<float32_t>(right_max) * 0.5F;

         bool f_left = true;
         bool f_right = true;

         while (f_left || f_right)
         {
            if ((left_index_update - 1 >= 0) && (static_cast<float32_t>(detection_col[left_index_update - 1]) >= left_max_update))
            {
               left_index_update--;
            }
            else
            {
               f_left = false;
            }
            if ((right_index_update + 1 < static_cast<int32_t>(COL_NUMBER)) && (static_cast<float32_t>(detection_col[right_index_update + 1]) >= right_max_update))
            {
               right_index_update++;
            }
            else
            {
               f_right = false;
            }
         }
         const int32_t temp = right_index_update - left_index_update + 1;
         trailer_width = static_cast<float32_t>(temp) * tw_calibs.k_col_interval;
         f_estimation_done = true;
         trailer_width_conf = TRAILER_DETECTOR_CONF_HIGH;
      }
      else
      {
         //do nothing
      }
   }
}
