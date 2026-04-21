/*===========================================================================*\
 * FILE: tp_run.cpp
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer presence run function and Get_Output function
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_trailer_detector_TP.h"
#include "f360_trailer_detector_constants.h"
#include "f360_reuse.h"
#include "f360_iterator.h"

namespace f360_variant_A
{
   const TP_Cals Trailer_Detector_TP::tp_calibs{};
   /*=========================================================================
    * Method         Trailer_Detector_TP::Run_Trailer_Detector
    *
    * Description    Public method to run trailer presence estimation
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
   void Trailer_Detector_TP::Run_Trailer_Detector(const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   )
   {
      Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);

      Estimate();
   }

   /*=========================================================================
    * Method         Trailer_Detector_TP::Get_Output
    *
    * Description    Public method to get trailer presence estimation output
    *
    * Parameters     TP_Output& output
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
   void Trailer_Detector_TP::Get_Output(TP_Output& output) const
   {
      output.trailer_presence = trailer_presence;
      output.confidence = trailer_presence_conf;
      output.f_estimation_done = f_estimation_done;
      output.stationary_timer = stationary_timer;
   }
   
   /*=========================================================================
    * Method         Trailer_Detector_TP::initialize
    *
    * Description    Public function to initialize trailer presence class
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
   void Trailer_Detector_TP::Initialize()
   {
      trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
      trailer_presence_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
      f_estimation_done = false;
      window_timer = 0U;
      stationary_timer = 0U;
      relative_ratio_struct.dets_n_02 = 0;
      relative_ratio_struct.dets_n_03 = 0;
      relative_ratio_struct.dets_n_04 = 0;
      relative_ratio_struct.dets_n_05 = 0;

      std::fill(cmn::begin(mean_rr_struct.num_array), cmn::end(mean_rr_struct.num_array), static_cast<int16_t>(0));

      Linspace(tp_calibs.k_rr_start, tp_calibs.k_rr_end, mean_rr_struct.value_array);
   }

   /*=========================================================================
    * Method         Trailer_Detector_TP::Clear
    *
    * Description    Public function to clear member variables of trailer presence class
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
   void Trailer_Detector_TP::Clear()
   {
      trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
      trailer_presence_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
      trailer_presence_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
      f_estimation_done = false;
      window_timer = 0U;
      relative_ratio_struct.dets_n_02 = 0;
      relative_ratio_struct.dets_n_03 = 0;
      relative_ratio_struct.dets_n_04 = 0;
      relative_ratio_struct.dets_n_05 = 0;

      std::fill(cmn::begin(mean_rr_struct.num_array), cmn::end(mean_rr_struct.num_array), static_cast<int16_t>(0));

      Linspace(tp_calibs.k_rr_start, tp_calibs.k_rr_end, mean_rr_struct.value_array);
   }

   /*=========================================================================
    * Method         Trailer_Detector_TP::Process_Input
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
   void Trailer_Detector_TP::Process_Input(const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      // detection collection
      if ((vehicle_data.speed >= tp_calibs.k_speed_threshold) &&
         (window_timer < tp_calibs.k_time_threshold))
      {
         stationary_timer = 0U;
         window_timer++;
         for (uint32_t det_idx = 0U; det_idx < raw_detect_list.number_of_valid_detections; det_idx++)
         {
            const rspp_variant_A::RSPP_Detection_T current_detection = raw_detect_list.detections[det_idx];
            const F360_Detection_Props_T& current_detection_prop = all_detections[det_idx];
            const int32_t current_sensor_id = current_detection.raw.sensor_id;
            const F360_Radar_Sensor_T& current_sensor = sensors[current_sensor_id - 1];

            // check detection validation
            const bool f_trailer_detection = 
               (((F360_MOUNTING_LOCATION_RIGHT_REAR == current_sensor.constant.mounting_location) ||
                 (F360_MOUNTING_LOCATION_LEFT_REAR == current_sensor.constant.mounting_location) ||
                 (F360_MOUNTING_LOCATION_CENTER_REAR == current_sensor.constant.mounting_location)) &&
                (!current_detection_prop.f_double_bounce) &&
                (F360_DETECTION_WHEELSPIN_TYPE_INVALID == current_detection_prop.wheel_spin_type) &&
                (!current_detection_prop.f_water_spray) &&
                (current_detection_prop.vcs_position.x <= tp_calibs.k_long_max) &&
                (current_detection_prop.vcs_position.x >= tp_calibs.k_long_min) &&
                (std::abs(current_detection_prop.vcs_position.y) <= tp_calibs.k_lat_abs));

            if (f_trailer_detection)
            {
               // criteria 1
               const float32_t detection_abs_rr = std::abs(current_detection.raw.range_rate);
               if (detection_abs_rr <= 0.2F)
               {
                  relative_ratio_struct.dets_n_02++;
               }
               else if (detection_abs_rr <= 0.3F)
               {
                  relative_ratio_struct.dets_n_03++;
               }
               else if (detection_abs_rr <= 0.4F)
               {
                  relative_ratio_struct.dets_n_04++;
               }
               else if (detection_abs_rr <= 0.5F)
               {
                  relative_ratio_struct.dets_n_05++;
               }
               else
               {
                  // do nothing
               }

               // criteria 2
               if (detection_abs_rr >= tp_calibs.k_rr_end - 0.5F)
               {
                  // do nothing
               }
               else
               {
                  const float32_t bin_interval_inverse = (static_cast<float32_t>(BIN_SIZE)) / (tp_calibs.k_rr_end - tp_calibs.k_rr_start);
                  const uint16_t bin_index = static_cast<uint16_t>(F360_Floorf((detection_abs_rr - tp_calibs.k_rr_start) * bin_interval_inverse));
                  mean_rr_struct.num_array[bin_index]++;
               }
            } // f_trailer_detection
         }
      }

      // clear data
      if ((std::abs(vehicle_data.speed) < tp_calibs.k_speed_threshold_clear) && (TRAILER_PRESENCE_STATE_DETECTED == trailer_presence))
      {
         stationary_timer += 1U;
         if (stationary_timer >= tp_calibs.k_reset_cnt_threshold)
         {
            Clear();
         }
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TP::Linspace
    *
    * Description    Private function used to create evenly spaced numbers over a specified interval
    *
    * Parameters     const float32_t first, 
    *                const float32_t second,  
    *                float32_t (&value_array)[BIN_SIZE]
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
   void Trailer_Detector_TP::Linspace(const float32_t first, const float32_t second, float32_t (&value_array)[BIN_SIZE])
   {
      const float32_t step = BIN_SIZE > 0U ? (second - first) / (static_cast<float32_t>(BIN_SIZE)) : 0.0F;
      for (uint8_t i = 0U; i < BIN_SIZE; i++)
      {
         value_array[i] = first + static_cast<float32_t>(i)*step + 0.5F * step;
      }
   }
   /*=========================================================================
    * Method         Trailer_Detector_TP::Estimate
    *
    * Description    Private method used to estimate trailer presence
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
   void Trailer_Detector_TP::Estimate()
   {
      const bool f_estimation_already_performed = (TRAILER_PRESENCE_STATE_NOT_DETECTED == trailer_presence) || (TRAILER_PRESENCE_STATE_DETECTED== trailer_presence);
      const bool f_not_enough_time_elapsed_to_begin_estimation = (window_timer != tp_calibs.k_time_threshold);
      if (f_estimation_already_performed || f_not_enough_time_elapsed_to_begin_estimation)
      {
         // Do nothing
      }
      else
      {
         // criteria 1
         const int16_t sum_dets_n = relative_ratio_struct.dets_n_02 + relative_ratio_struct.dets_n_03 + relative_ratio_struct.dets_n_04 + relative_ratio_struct.dets_n_05;

         // criteria 2
         int32_t sum_num_array = 0;
         for (uint8_t i = 0U; i < BIN_SIZE; i++)
         {
            sum_num_array += mean_rr_struct.num_array[i];
         }

         if ((sum_dets_n > 10) && (sum_num_array > 10)) // There are enough detections to perform estimation
         {
            
            float32_t rr_ratio = 0.0F;
            float32_t rr_mean = 0.0F;

            const int16_t temp = relative_ratio_struct.dets_n_02 + relative_ratio_struct.dets_n_03;
            rr_ratio = static_cast<float32_t>(temp) / static_cast<float32_t>(sum_dets_n);

            const float32_t inv_sum_num_arrary = 1.0F / static_cast<float32_t>(sum_num_array);
            for (uint8_t i = 0U; i < BIN_SIZE; i++)
            {
               rr_mean += static_cast<float32_t>(mean_rr_struct.num_array[i]) * mean_rr_struct.value_array[i] * inv_sum_num_arrary;
            }

            // decision
            if ((rr_ratio > tp_calibs.k_rr_ratio_threshold) &&
               (rr_mean < tp_calibs.k_rr_mean_threshold))
            {
               trailer_presence = TRAILER_PRESENCE_STATE_DETECTED;
               f_estimation_done = true;

               if (rr_ratio >= 0.9F)
               {
                  trailer_presence_conf = TRAILER_DETECTOR_CONF_HIGH;
               }
               else if (rr_ratio >= 0.7F)
               {
                  trailer_presence_conf = TRAILER_DETECTOR_CONF_MEDIAN;
               }
               else
               {
                  trailer_presence_conf = TRAILER_DETECTOR_CONF_LOW;
               }
            }
            else
            {
               trailer_presence = TRAILER_PRESENCE_STATE_NOT_DETECTED;
               f_estimation_done = true;
               trailer_presence_conf = TRAILER_DETECTOR_CONF_HIGH;
            }
         }
         else // There are not enough detections to perform estimation
         {
            trailer_presence = TRAILER_PRESENCE_STATE_NOT_DETECTED;
            trailer_presence_conf = TRAILER_DETECTOR_CONF_HIGH;
         }
      }
   }
}
