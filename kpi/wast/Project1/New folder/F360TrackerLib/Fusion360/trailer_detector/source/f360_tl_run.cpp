/*===========================================================================*\
 * FILE: tl_run.cpp
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer length run function and Get_Output function
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_trailer_detector_TL.h"
#include "f360_trailer_detector_TL_SVM.h"
#include "f360_trailer_detector_constants.h"
#include "f360_reuse.h"

namespace f360_variant_A
{
   const TL_Cals Trailer_Detector_TL::tl_calibs{};
   /*=========================================================================
    * Method         Trailer_Detector_TL::Run_Trailer_Detector
    *
    * Description    Public method to run trailer length estimation
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
   void Trailer_Detector_TL::Run_Trailer_Detector(
      const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);

      Estimate();
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Get_Output
    *
    * Description    Public method to get trailer length estimation output
    *
    * Parameters     TL_Output& output
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
   void Trailer_Detector_TL::Get_Output(TL_Output& output) const
   {
      output.axle_trailer_length = axle_trailer_length;
      output.trailer_length = trailer_length;
      output.f_estimation_done = f_estimation_done;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Initialize
    *
    * Description    Public function to initialize trailer length class
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
   void Trailer_Detector_TL::Initialize()
   {
      trailer_length = 0.0F;

      trailer_length_peaks = 0.0F;

      svm_outputs.trailer_length_SVM = 0.0F;

      svm_outputs.trailer_length_SVM_conf = 0.0F;

      trailer_length_conf = 0U;

      axle_trailer_length = 0.0F;

      trailer_HV_gap = 0.0F;

      reset_timer = 0U;

      window_timer = 0U;

      first_peak_cnt = 0;

      second_peak_cnt = 0;

      third_peak_cnt = 0;

      first_peak_extension = false;

      f_estimation_done = false;

      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         trailer_detection_row_struct.detection_row[i] = 0;
      }

      for (int32_t i = 0; i < PEAK_GROUP_SIZE; i++)
      {
         first_peak_group[i].peak_pos = 0;
         first_peak_group[i].peak_val = 0;
         first_peak_group[i].peak_left_radius = 0;
         first_peak_group[i].peak_right_radius = 0;

         second_peak_group[i].peak_pos = 0;
         second_peak_group[i].peak_val = 0;
         second_peak_group[i].peak_left_radius = 0;
         second_peak_group[i].peak_right_radius = 0;

         third_peak_group[i].peak_pos = 0;
         third_peak_group[i].peak_val = 0;
         third_peak_group[i].peak_left_radius = 0;
         third_peak_group[i].peak_right_radius = 0;
      }

      front_area.starting_pos = 0;
      front_area.ending_pos = 0;
      front_area.ref_val = 0;
      front_area.mean_val = 0.0F;
      front_area.max_val = 0.0F;

      middle_area.starting_pos = 0;
      middle_area.ending_pos = 0;
      middle_area.ref_val = 0;
      middle_area.mean_val = 0.0F;
      middle_area.max_val = 0.0F;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Clear
    *
    * Description    Public function to clear member variables of trailer length class
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
   void Trailer_Detector_TL::Clear()
   {
      trailer_length = 0.0F;

      trailer_length_peaks = 0.0F;

      svm_outputs.trailer_length_SVM = 0.0F;

      svm_outputs.trailer_length_SVM_conf = 0.0F;

      trailer_length_conf = 0U;

      axle_trailer_length = 0.0F;

      trailer_HV_gap = 0.0F;

      reset_timer = 0U;

      window_timer = 0U;

      first_peak_cnt = 0;

      second_peak_cnt = 0;

      third_peak_cnt = 0;

      first_peak_extension = false;

      f_estimation_done = false;

      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         trailer_detection_row_struct.detection_row[i] = 0;
      }

      for (int32_t i = 0; i < PEAK_GROUP_SIZE; i++)
      {
         first_peak_group[i].peak_pos = 0;
         first_peak_group[i].peak_val = 0;
         first_peak_group[i].peak_left_radius = 0;
         first_peak_group[i].peak_right_radius = 0;

         second_peak_group[i].peak_pos = 0;
         second_peak_group[i].peak_val = 0;
         second_peak_group[i].peak_left_radius = 0;
         second_peak_group[i].peak_right_radius = 0;

         third_peak_group[i].peak_pos = 0;
         third_peak_group[i].peak_val = 0;
         third_peak_group[i].peak_left_radius = 0;
         third_peak_group[i].peak_right_radius = 0;
      }

      front_area.starting_pos = 0;
      front_area.ending_pos = 0;
      front_area.ref_val = 0;
      front_area.mean_val = 0.0F;
      front_area.max_val = 0.0F;

      middle_area.starting_pos = 0;
      middle_area.ending_pos = 0;
      middle_area.ref_val = 0;
      middle_area.mean_val = 0.0F;
      middle_area.max_val = 0.0F;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Increase_Detection_Row
    *
    * Description    Private function to increaes detection rows
    *
    * Parameters     int32_t (&detection_row_new)[ROW_NUMBER], 
    *                int32_t (&detection_row)[ROW_NUMBER]
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
   void Trailer_Detector_TL::Increase_Detection_Row(
      int32_t (&detection_row_new)[ROW_NUMBER], 
      int32_t (&detection_row)[ROW_NUMBER])
   {
      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         detection_row_new[i] += detection_row[i];
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Process_Input
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
   void Trailer_Detector_TL::Process_Input(
      const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      if (vehicle_data.speed <= tl_calibs.k_speed_threshold)
      {
         reset_timer = std::min(reset_timer + 1U, tl_calibs.k_reset_time_threshold);
         if (reset_timer == tl_calibs.k_reset_time_threshold)
         {
            Clear();
         }
         else
         {
            //do nothing
         }
      }
      else if (window_timer < tl_calibs.k_window_timer_threshold)
      {
         reset_timer = 0U;
         window_timer = std::min(window_timer + 1U, tl_calibs.k_window_timer_threshold);

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
                (std::abs(current_detection.raw.range_rate) < tl_calibs.k_ZRRateGate) &&
                (current_detection_prop.vcs_position.x <= -tl_calibs.k_host_length) &&
                (current_detection_prop.vcs_position.x >= -(tl_calibs.k_host_length + tl_calibs.k_max_trailer_length)) &&
                (std::abs(current_detection_prop.vcs_position.y) <= tl_calibs.k_max_trailer_width * 0.5F) &&
                (!current_detection_prop.f_double_bounce) &&
                (F360_DETECTION_WHEELSPIN_TYPE_INVALID == current_detection_prop.wheel_spin_type) &&
                (!current_detection_prop.f_water_spray));

            if (f_trailer_detection)
            {
               int32_t row_index = static_cast<int32_t>(F360_Floorf(-(current_detection_prop.vcs_position.x + tl_calibs.k_host_length) / tl_calibs.k_row_interval + 0.5F));
               row_index = Clamp(row_index, 1, static_cast<int32_t>(ROW_NUMBER));
               // accumulate detections
               trailer_detection_row_struct.detection_row[row_index - 1] += 1;
            }
            else
            {
               //do nothing
            }
         }
      }
      else
      {
         //do nothing
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Estimate
    *
    * Description    Private method used to estimate trailer length
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
   void Trailer_Detector_TL::Estimate()
   {
      if(!f_estimation_done)
      {
         // prediction
         if (window_timer == tl_calibs.k_window_timer_threshold)
         {
            Adjust_Sample();

            Locate_Peaks();

            Estimate_Trailer_Length_Peaks();

            Estimate_Trailer_Length_SVM();

            Post_Processing();

            f_estimation_done = true;
         }
         else
         {
            // do nothing
         }
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Adjust_Sample
    *
    * Description    Private method used to adjust detection row struct.
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
   void Trailer_Detector_TL::Adjust_Sample()
   {
      int32_t max_val = 0;
      max_val = F360_Max_Element(trailer_detection_row_struct.detection_row, ROW_NUMBER - 1U);
      const float32_t temp = 0.05F * static_cast<float32_t>(max_val);
      const int32_t threshold = static_cast <int32_t>(temp);

      for (uint8_t i = 1U; i < ROW_NUMBER; i++)
      {
         if ((trailer_detection_row_struct.detection_row[i - 1U] == trailer_detection_row_struct.detection_row[i]) && (trailer_detection_row_struct.detection_row[i - 1U] >= threshold))
         {
            trailer_detection_row_struct.detection_row[i - 1U] -= 1;
         }
         else
         {
            // do nothing
         }
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Locate_Peaks
    *
    * Description    Peak finding function used to locate first, second and third peaks.
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
   void Trailer_Detector_TL::Locate_Peaks()
   {
      int32_t temp_detection_row[ROW_NUMBER];
      for (uint8_t i = 0U; i < ROW_NUMBER; i++)
      {
         temp_detection_row[i] = trailer_detection_row_struct.detection_row[i];
      }

      int32_t max_val = 0;
      for (uint8_t i = 0U; i < ROW_NUMBER; i++)
      {
         max_val = std::max(max_val, temp_detection_row[i]);
      }

      // first peak groups
      float32_t threshold_forward = 0.8F;
      float32_t threshold_backward = 0.6F;
      uint32_t peak_gap_max = 3U;
      uint32_t peak_r_gap_max = 10U;

      Find_Peak_Group_Info(temp_detection_row, threshold_forward, threshold_backward, peak_gap_max, peak_r_gap_max, max_val, first_peak_group, first_peak_cnt);

      // to avoid first peak group split
      const int32_t tmp_val = trailer_detection_row_struct.detection_row[first_peak_group[0].peak_pos + first_peak_group[0].peak_right_radius];
      if ((first_peak_cnt == 1)
         && (static_cast<float32_t>(tmp_val) >= 0.08F * static_cast<float32_t>(first_peak_group[first_peak_cnt - 1].peak_val))
         && (first_peak_group[first_peak_cnt - 1].peak_right_radius <= 3)
         && (first_peak_group[first_peak_cnt - 1].peak_pos <= 15))
      {
         first_peak_group[0].peak_right_radius += 4;
         first_peak_extension = true;
      }
      else
      {
         // do nothing
      }

      // second peak group
      if (first_peak_cnt >= 1)
      {
         int32_t first_peak_group_end_pos = first_peak_group[first_peak_cnt - 1].peak_pos + first_peak_group[first_peak_cnt - 1].peak_right_radius;
         for (int8_t i = 0; i < first_peak_group_end_pos; i++)
         {
            temp_detection_row[i] = 0;
         }

         max_val = 0;
         for (uint8_t i = 0U; i < 40U; i++)
         {
            max_val = std::max(max_val, temp_detection_row[i]);
         }

         threshold_forward = 0.8F;
         threshold_backward = 0.5F;
         peak_gap_max = 8U;
         peak_r_gap_max = 3U;

         Find_Peak_Group_Info(temp_detection_row, threshold_forward, threshold_backward, peak_gap_max, peak_r_gap_max, max_val, second_peak_group, second_peak_cnt);

         // for some samples, the second peak group is part of first peak group. Here to merge the first one and the second one. Then, 
         // calculate the new second peak group.
         if (((first_peak_cnt == 1) && (second_peak_cnt == 1))
            && (second_peak_group[0].peak_pos - first_peak_group[0].peak_pos <= 6)
            && (second_peak_group[0].peak_pos <= 12)
            && (first_peak_group[0].peak_pos + first_peak_group[0].peak_right_radius == second_peak_group[0].peak_pos - second_peak_group[0].peak_left_radius))
         {
            first_peak_group[0].peak_right_radius += second_peak_group[0].peak_left_radius + second_peak_group[0].peak_right_radius + 1;

            second_peak_cnt = 0;
            first_peak_group_end_pos = first_peak_group[first_peak_cnt - 1].peak_pos + first_peak_group[first_peak_cnt - 1].peak_right_radius;
            for (int8_t i = 0; i < first_peak_group_end_pos; i++)
            {
               temp_detection_row[i] = 0; 
            }
            max_val = 0;
            for (uint8_t i = 0U; i < 40U; i++)
            {
               max_val = std::max(max_val, temp_detection_row[i]);
            }
            Find_Peak_Group_Info(temp_detection_row, threshold_forward, threshold_backward, peak_gap_max, peak_r_gap_max, max_val, second_peak_group, second_peak_cnt);
         }
         else
         {
            // do nothing
         }
      }
      else
      {
         // do nothing
      }

      // third peak
      if (second_peak_cnt >= 1)
      {
         const int32_t second_peak_group_end_pos = second_peak_group[second_peak_cnt - 1].peak_pos + second_peak_group[second_peak_cnt - 1].peak_right_radius;
         for (int8_t i = 0; i < second_peak_group_end_pos; i++)
         {
            temp_detection_row[i] = 0;
         }

         max_val = 0;
         for (uint8_t i = 0U; i < ROW_NUMBER; i++)
         {
            max_val = std::max(max_val, temp_detection_row[i]);
         }

         threshold_forward = 0.8F;
         threshold_backward = 0.5F;
         peak_gap_max = 8U;
         peak_r_gap_max = 10U;

         Find_Peak_Group_Info(temp_detection_row, threshold_forward, threshold_backward, peak_gap_max, peak_r_gap_max, max_val, third_peak_group, third_peak_cnt);
      }
      else
      {
         // do nothing
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Find_Peak_Group_Info
    *
    * Description    Function confirms that found peaks are valid and finds peak radius.
    *
    * Parameters
    * const int32_t(&detection_row)[ROW_NUMBER]
    * const float32_t threshold_forward
    * const float32_t threshold_backward
    * const uint32_t peak_gap_max
    * const uint32_t peak_r_gap_max
    * const int32_t max_val
    * TL_Peak (&peak_list_point)[PEAK_GROUP_SIZE]
    * int32_t& peak_cnt 
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
   void Trailer_Detector_TL::Find_Peak_Group_Info(
      const int32_t(&detection_row)[ROW_NUMBER], 
      const float32_t threshold_forward, 
      const float32_t threshold_backward, 
      const uint32_t peak_gap_max, 
      const uint32_t peak_r_gap_max,
      const int32_t max_val,
      TL_Peak (&peak_list_point)[PEAK_GROUP_SIZE], 
      int32_t& peak_cnt)
   {
      const float32_t temp = threshold_forward * static_cast<float32_t>(max_val);
      int32_t threshold = static_cast<int32_t>(temp);
      int32_t cur_second_peak_index = -1;

      for (uint32_t k = 0U; k < ROW_NUMBER; k++)
      {
         if ((cur_second_peak_index == -1)
            && (detection_row[k] >= threshold)
            && (Is_Peak(detection_row, k)))
         {
            uint32_t r_left = 0U;
            uint32_t r_right = 0U;
            Cal_Radius(trailer_detection_row_struct.detection_row, k, r_left, r_right);

            TL_Peak temp_peak;
            temp_peak.peak_pos = static_cast<int32_t>(k);
            temp_peak.peak_val = detection_row[k];
            temp_peak.peak_left_radius = static_cast<int32_t>(r_left);
            temp_peak.peak_right_radius = static_cast<int32_t>(r_right);
            peak_cnt = std::min(peak_cnt, PEAK_GROUP_SIZE - 1);
            peak_cnt += 1;
            peak_list_point[peak_cnt] = temp_peak;

            cur_second_peak_index = static_cast<int32_t>(k);
            const float32_t thres_float = threshold_backward * static_cast<float32_t>(detection_row[k]);
            threshold = static_cast<int32_t>(thres_float);
         }
         else if ((cur_second_peak_index != -1)
            && (detection_row[k] >= threshold)
            && (Is_Peak(detection_row, k))
            && (k - static_cast<uint32_t>(cur_second_peak_index) <= peak_gap_max))
         {
            uint32_t r_left_cur = 0U;
            uint32_t r_right_cur = 0U;
            Cal_Radius(trailer_detection_row_struct.detection_row, static_cast<uint32_t>(cur_second_peak_index), r_left_cur, r_right_cur);

            uint32_t r_left = 0U;
            uint32_t r_right = 0U;
            Cal_Radius(trailer_detection_row_struct.detection_row, k, r_left, r_right);

            if ((k - r_left) - (static_cast<uint32_t>(cur_second_peak_index) + r_right_cur) >= peak_r_gap_max)
            {
               continue;
            }

            TL_Peak temp_peak;
            temp_peak.peak_pos = static_cast<int32_t>(k);
            temp_peak.peak_val = detection_row[k];
            temp_peak.peak_left_radius = static_cast<int32_t>(r_left);
            temp_peak.peak_right_radius = static_cast<int32_t>(r_right);
            peak_cnt = std::min(peak_cnt, PEAK_GROUP_SIZE - 1);
            peak_cnt += 1;
            peak_list_point[peak_cnt] = temp_peak;

            const float32_t thres_float = threshold_backward * static_cast<float32_t>(detection_row[k]);
            threshold = static_cast<int32_t>(thres_float);
            cur_second_peak_index = static_cast<int32_t>(k);
         }
         else
         {
            // do nothing
         }
      }
   }

  /*=========================================================================
   * Method         Trailer_Detector_TL::Is_Peak
   *
   * Description    Function determines if found peak is valid.
   *
   * Parameters
   * const int32_t (&sample)[ROW_NUMBER]
   * const uint32_t index
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
   bool Trailer_Detector_TL::Is_Peak(
      const int32_t (&sample)[ROW_NUMBER], 
      const uint32_t index)
   {
      bool peak_sts = false;
      if ((0U < index) && (index < ROW_NUMBER - 1U))
      {
         peak_sts = (sample[index] > sample[index - 1U]) && (sample[index] > sample[index + 1U]);
      }

      return peak_sts;
   }

  /*=========================================================================
   * Method         Trailer_Detector_TL::Cal_Radius
   *
   * Description    Function determines peak radius.
   *
   * Parameters
   * const int32_t (&sample)[ROW_NUMBER],
   * const uint32_t index,
   * uint32_t& r_left,
   * uint32_t& r_right
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
   void Trailer_Detector_TL::Cal_Radius(
      const int32_t (&sample)[ROW_NUMBER], 
      const uint32_t index, 
      uint32_t& r_left, 
      uint32_t& r_right)
   {
      constexpr float32_t drop_ratio = 0.02F;

      uint32_t index_left = index;
      while (index_left >= 2U)
      {
         const float32_t temp = drop_ratio * static_cast<float32_t>(sample[index]);
         if (((sample[index_left - 1U] < sample[index_left])
            && (sample[index_left] - sample[index_left - 1U] >= static_cast<int32_t>(temp)))
            || (sample[index_left - 1U] - sample[index_left - 2U] >= static_cast<int32_t>(temp)))
         {
            index_left -= 1U;
         }
         else if ((sample[index_left - 1U] < sample[index_left])
            && (sample[index_left] - sample[index_left - 1U] < static_cast<int32_t>(temp))
            && (sample[index_left - 1U] - sample[index_left - 2U] < static_cast<int32_t>(temp))
            && (static_cast<float32_t>(sample[index_left - 1U]) / static_cast<float32_t>(sample[index_left]) > 0.5F))
         {
            index_left -= 1U;
         }
         else
         {
            break;
         }
      }

      uint32_t index_right = index;
      while (index_right < ROW_NUMBER-2U)
      {
         const float32_t temp = drop_ratio * static_cast<float32_t>(sample[index]);
         if (((sample[index_right + 1U] < sample[index_right])
            && (sample[index_right] - sample[index_right + 1U] >= static_cast<int32_t>(temp)))
            || (sample[index_right + 1U] - sample[index_right + 2U] >= static_cast<int32_t>(temp)))
         {
            index_right += 1U;
         }
         else if ((sample[index_right + 1U] < sample[index_right])
            && (sample[index_right] - sample[index_right + 1U] < static_cast<int32_t>(temp))
            && (sample[index_right + 1U] - sample[index_right + 2U] < static_cast<int32_t>(temp))
            && (static_cast<float32_t>(sample[index_right + 1U]) / static_cast<float32_t>(sample[index_right]) > 0.5F))
         {
            index_right += 1U;
         }
         else
         {
            break;
         }
      }

      r_left = std::max(index - index_left, 1U);
      r_right = std::max(index_right - index, 1U);
   }

  /*=========================================================================
   * Method         Trailer_Detector_TL::Estimate_Trailer_Length_Peaks
   *
   * Description    Function trailer length based on previously determined peaks.
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
   void Trailer_Detector_TL::Estimate_Trailer_Length_Peaks()
   {
      // init i_array_max, j_array_max and k_array_max
      int32_t i_array_max = 0;
      int32_t j_array_max = 0;
      int32_t k_array_max = 0;

      for (int8_t p = 0; p < PEAK_GROUP_SIZE; p++)
      {
         if (p < first_peak_cnt)
         {
            i_array_max = std::max(i_array_max, first_peak_group[p].peak_val);
         }
         if (p < second_peak_cnt)
         {
            j_array_max = std::max(j_array_max, second_peak_group[p].peak_val);
         }
         if (p < third_peak_cnt)
         {
            k_array_max = std::max(k_array_max, third_peak_group[p].peak_val);
         }
      }

      // default trailer length output
      float32_t last_peak = 15.0F;
      trailer_length_peaks = static_cast<float32_t>(last_peak) * tl_calibs.k_row_interval;

      // situation 1: only first peak group is valid
      const int peak_pos_shift = 20;
      front_area.starting_pos = Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
      front_area.ending_pos = std::min(front_area.starting_pos + peak_pos_shift, static_cast<int32_t>(ROW_NUMBER - 1U));
      front_area.ref_val = first_peak_group[first_peak_cnt - 1].peak_val;
      front_area.mean_val = 0.0F;
      front_area.max_val = 0.0F;
      Get_Area_Info(front_area);

      bool flag1 = (second_peak_cnt == 0);
      bool flag2 = (first_peak_extension == false)
         && (first_peak_group[first_peak_cnt - 1].peak_pos >= 17)
         && (front_area.starting_pos >= 20)
         && (front_area.max_val <= 0.35F)
         && (front_area.mean_val <= 0.2F);
      const bool flag3 = (front_area.starting_pos >= 35)
         && (front_area.max_val <= 0.6F)
         && (front_area.mean_val <= 0.2F); 
      const int32_t temp_max_val = F360_Max_Element_Bounded(trailer_detection_row_struct.detection_row, static_cast<uint32_t>(front_area.starting_pos), ROW_NUMBER);
      const bool flag4 = (static_cast<float32_t>(temp_max_val) <= 0.1F * static_cast<float32_t>(i_array_max)) && (front_area.starting_pos <= 20); // short box trailer pattern.

      if (flag1 || flag2 || flag3 || flag4)
      {
         if (flag4)
         {
            last_peak = 25.0F; // it may be a very small box trailer that radar can't reach the end of trailer, so we set a default value which is a little longer than first peak group.
         }
         else
         {
            // limit the max last peak to 50 in case there are some wired reflection.
            last_peak = static_cast<float32_t>(std::min(Peak_Right_Edge(first_peak_group, first_peak_cnt - 1) + 1, 50));
         }

         trailer_length_peaks = (last_peak + 1.0F) * tl_calibs.k_row_interval;
         return;
      }
      else
      {
         // do nothing
      }

      // situation 2: at least second peak group is valid
      // front reflection info, these info will be used in the following steps in this script.
      front_area.starting_pos = Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
      front_area.ending_pos = Peak_Left_Edge(second_peak_group, 0);
      front_area.ref_val = second_peak_group[0].peak_val;
      front_area.mean_val = 0.0F;
      front_area.max_val = 0.0F;
      Get_Area_Info(front_area);

      // middle reflection info, these info will be used in the following steps in this script.
      bool f_noise = true; // third_peak_group, assume the third peak group is noise by default.

      if (third_peak_cnt >= 1)
      {
         // long trailer
         const int32_t dist_first_second = Peak_Left_Edge(second_peak_group, 0) - Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
         const int32_t dist_second_third = Peak_Left_Edge(third_peak_group, 0) - Peak_Right_Edge(second_peak_group, second_peak_cnt - 1);
         flag1 = ((third_peak_group[0].peak_pos > 30) && (second_peak_group[0].peak_pos < 30)
            && ((dist_first_second >= 10) || (dist_second_third >= 10)));

         // short trailer
         flag2 = (second_peak_cnt <= 2) && (third_peak_cnt <= 2) && (third_peak_group[third_peak_cnt - 1].peak_pos <= 30);

         // check if third peak group is valid or not
         // if third peak group is valid, there must be some strong reflection between second peak group and third peak group.
         if (flag1 || flag2)
         {
            bool f_front_block = false;
            middle_area.starting_pos = Peak_Right_Edge(second_peak_group, second_peak_cnt - 1);
            middle_area.ending_pos = Peak_Left_Edge(third_peak_group, 0);
            middle_area.ref_val = std::max(second_peak_group[second_peak_cnt - 1].peak_val, third_peak_group[0].peak_val);
            middle_area.mean_val = 0.0F;
            middle_area.max_val = 0.0F;
            Get_Area_Info(middle_area);

            // for some short open trailers, the first peak group and second peak group are very close to each other.
            f_front_block = ((static_cast<float32_t>(j_array_max) >= 0.7F * static_cast<float32_t>(i_array_max))
               && ((front_area.mean_val >= 0.2F) || (front_area.max_val >= 0.3F) || (middle_area.mean_val * 20.0F <= front_area.mean_val))
               && (middle_area.mean_val <= 0.1F)
               && (third_peak_group[third_peak_cnt - 1].peak_pos <= 30))
               || ((middle_area.ending_pos - middle_area.starting_pos <= 2) && (static_cast<float32_t>(j_array_max) >= 0.7F * static_cast<float32_t>(i_array_max)) && (third_peak_group[third_peak_cnt].peak_pos <= 30));

            if ((!f_front_block)
               && ((middle_area.ending_pos - middle_area.starting_pos <= 4) ||
                  (third_peak_group[0].peak_pos - second_peak_group[second_peak_cnt - 1].peak_pos < 8) ||
                  ((middle_area.ending_pos - middle_area.starting_pos >= 10) && (middle_area.mean_val >= 0.15F) && (middle_area.max_val >= 0.4F)))
               && ((static_cast <float32_t>(k_array_max) >= 0.32F * static_cast <float32_t>(j_array_max))
                  || ((front_area.mean_val <= 0.1F) 
                     && (front_area.max_val <= 0.2F) 
                     && flag1 
                     && (static_cast<float32_t>(k_array_max) >= 0.25F * static_cast<float32_t>(j_array_max)))))
            {
               f_noise = false;
            }
            else if ((!f_front_block) && (second_peak_cnt <= 2)
               && (third_peak_cnt <= 2)
               && (third_peak_group[third_peak_cnt - 1].peak_pos <= 30)
               && (static_cast<float32_t>(k_array_max) >= 0.4F * static_cast<float32_t>(j_array_max))
               && (middle_area.mean_val >= 0.1F)
               && (middle_area.max_val >= 0.1F))
            {
               f_noise = false;
            }
            else
            {
               // do nothing
            }
         }
         else
         {
            // do nothing
         }
      }
      else
      {
         // do nothing
      }

      // initial estimation
      if ((third_peak_cnt == 0) || ((third_peak_cnt >= 1) && f_noise))
      {
         if (Peak_Right_Edge(first_peak_group, first_peak_cnt - 1) >= 30)
         {
            last_peak = static_cast<float32_t>(second_peak_group[second_peak_cnt - 1].peak_pos);
         }
         else
         {
            const int32_t temp = second_peak_group[second_peak_cnt - 1].peak_pos + std::min(second_peak_group[second_peak_cnt - 1].peak_right_radius, 3);
            last_peak = static_cast<float32_t>(temp);
         }
      }
      else
      {
         const int32_t temp = third_peak_group[third_peak_cnt - 1].peak_pos + std::min(third_peak_group[third_peak_cnt - 1].peak_right_radius, 3);
         last_peak = static_cast<float32_t>(temp);
      }

      // shrink estimation
      bool f_shrink = false;
      Shrink_Trailer_Length(f_noise, i_array_max, j_array_max, last_peak, f_shrink);

      // extend estimation
      if (!f_shrink)
      {
         Extend_Trailer_Length(f_noise, i_array_max, j_array_max, k_array_max, last_peak);
      }

      // final estimation
      trailer_length_peaks = (last_peak + 1.0F) * tl_calibs.k_row_interval;
      trailer_length_peaks = fminf(trailer_length_peaks, 11.0F);
      trailer_length_peaks = fmaxf(trailer_length_peaks, 4.0F);
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Peak_Left_Edge
    *
    * Description
    * Function finds left edge of the peak.
    * 
    * Parameters
    * const TL_Peak(&peak_group)[PEAK_GROUP_SIZE] 
    * const int32_t pos
    * 
    * Returns        
    * int32_t peak_group[pos].peak_pos - peak_group[pos].peak_left_radius
    * 
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   int32_t Trailer_Detector_TL::Peak_Left_Edge(
      const TL_Peak(&peak_group)[PEAK_GROUP_SIZE], 
      const int32_t pos)
   {
      return peak_group[pos].peak_pos - peak_group[pos].peak_left_radius;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Peak_Right_Edge
    *
    * Description
    * Function finds right edge of the peak.
    * 
    * Parameters
    * const TL_Peak (&peak_group)[PEAK_GROUP_SIZE]
    * const int32_t pos
    * 
    * Returns
    * int32_t peak_group[pos].peak_pos + peak_group[pos].peak_right_radius
    * 
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   int32_t Trailer_Detector_TL::Peak_Right_Edge(
      const TL_Peak (&peak_group)[PEAK_GROUP_SIZE], 
      const int32_t pos)
   {
      return peak_group[pos].peak_pos + peak_group[pos].peak_right_radius;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::Get_Area_Info
    *
    * Description
    * Function fills TL_Area struct with area parameters.
    * 
    * Parameters
    * TL_Area& area_info
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
   void Trailer_Detector_TL::Get_Area_Info(TL_Area& area_info) const
   {
      for (int32_t i = area_info.starting_pos; i <= area_info.ending_pos; i++)
      {
         area_info.mean_val += static_cast<float32_t>(trailer_detection_row_struct.detection_row[i]);
         area_info.max_val = fmaxf(area_info.max_val, static_cast<float32_t>(trailer_detection_row_struct.detection_row[i]));
      }

      const int32_t temp = (area_info.ending_pos - area_info.starting_pos + 1) * area_info.ref_val;
      area_info.mean_val = area_info.mean_val / static_cast<float32_t>(temp);
      area_info.max_val = area_info.max_val / static_cast<float32_t>(area_info.ref_val);
   }
   
   /*=========================================================================
    * Method         Trailer_Detector_TL::Shrink_Trailer_Length
    *
    * Description
    * Function shrinks estimated trailer length.
    * 
    * Parameters
    * const bool f_noise
    * const int32_t i_array_max
    * const int32_t j_array_max
    * cloat32_t& last_peak
    * bool& f_shrink
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
   void Trailer_Detector_TL::Shrink_Trailer_Length(
      const bool f_noise, 
      const int32_t i_array_max, 
      const int32_t j_array_max, 
      float32_t& last_peak,
      bool& f_shrink)
   {
      // some valid peaks are not located by second peak group (short open trailer)
      int32_t temp_max_val = 0;
      for (int32_t i = Peak_Right_Edge(second_peak_group, second_peak_cnt - 1); i < static_cast<int32_t>(ROW_NUMBER); i++)
      {
         temp_max_val = std::max(temp_max_val, trailer_detection_row_struct.detection_row[i]);
      }

      // front reflection is a little strong means some peaks are there which are not located.
      if ((f_noise) && (second_peak_cnt == 1)
         && (second_peak_group[second_peak_cnt - 1].peak_pos >= 30)
         && (second_peak_group[second_peak_cnt - 1].peak_pos <= 40)
         && (static_cast<float32_t>(temp_max_val) <= 0.25F * static_cast<float32_t>(i_array_max))
         && (((front_area.mean_val >= 0.07F) && (front_area.mean_val <= 0.3F))
            || ((front_area.mean_val < 0.07F) && (front_area.max_val >= 0.4F))))
      {
         temp_max_val = 0;
         int32_t temp_cnt = 0;
         int32_t temp_cnt_pos[ROW_NUMBER] = {};
         const int32_t i_start = Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
         const int32_t i_end = Peak_Left_Edge(second_peak_group, second_peak_cnt - 1);
         for (int32_t i = i_start; i <= i_end; i++)
         {
            if (trailer_detection_row_struct.detection_row[i] > temp_max_val)
            {
               temp_max_val = trailer_detection_row_struct.detection_row[i];
            }
            else
            {
               // do nothing
            }
         }

         for (int32_t i = i_start; i <= i_end; i++)
         {
            if (static_cast<float32_t>(trailer_detection_row_struct.detection_row[i]) >= 0.8F * static_cast<float32_t>(temp_max_val))
            {
               temp_cnt += 1;
               temp_cnt_pos[temp_cnt] = i;
            }
         }

         TL_Area rear_area;
         rear_area.starting_pos = Peak_Right_Edge(second_peak_group, second_peak_cnt - 1);
         rear_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         rear_area.ref_val = second_peak_group[second_peak_cnt - 1].peak_val;
         rear_area.mean_val = 0.0F;
         rear_area.max_val = 0.0F;
         Get_Area_Info(rear_area);

         // if no strong reflection behind second_peak_cnt_ - 1, it means second_peak_cnt_ - 1 is multipath reflection.
         if ((rear_area.mean_val < front_area.mean_val)
            && (rear_area.mean_val <= 0.1F)
            && (((temp_cnt == 1) || (temp_cnt == 2))
               && ((temp_cnt_pos[1] - temp_cnt_pos[0] == 1) || (temp_cnt == 2))
               && (temp_cnt_pos[1] - temp_cnt_pos[0] <= 6)
               && (((second_peak_group[0].peak_pos - i_start + temp_cnt_pos[1]) >= 10))
               && ((front_area.max_val >= 0.5F)
                  || (front_area.max_val < 0.5F)
                  || (static_cast<float32_t>(j_array_max) <= 0.25F * static_cast<float32_t>(i_array_max)))
               ))
         {
            const int32_t temp = first_peak_group[first_peak_cnt - 1].peak_pos + second_peak_group[0].peak_pos;
            last_peak = 0.6F  * static_cast<float32_t>(temp);
            f_shrink = true;
         }
         else
         {
            // do nothing
         }
      }

      // some loaded boat trailer has wired reflection
      if ((!f_shrink) && (f_noise)
         && (second_peak_cnt >= 2)
         && (last_peak >= 40.0F)
         && (first_peak_group[first_peak_cnt - 1].peak_pos >= 17))
      {
         int32_t tmp_last_index = -1;
         for (int32_t i = 0; i < second_peak_cnt - 1; i++)
         {
            const float32_t temp = 0.8F * static_cast<float32_t>(second_peak_group[i].peak_val);
            if (second_peak_group[i + 1].peak_val <= static_cast<int32_t>(temp))
            {
               tmp_last_index = i;
               break;
            }
            else
            {
               tmp_last_index = i;
            }
         }

         last_peak = static_cast<float32_t>(second_peak_group[tmp_last_index].peak_pos);
         f_shrink = true;
      }

      // some utility trailer
      if ((!f_shrink) && (!f_noise)
         && (first_peak_cnt == 1)
         && (second_peak_cnt == 1)
         && (third_peak_cnt >= 1)
         && (third_peak_group[0].peak_pos > 30)
         && (third_peak_group[third_peak_cnt - 1].peak_pos <= 47)
         && (static_cast<float32_t>(j_array_max) >= 0.2F * static_cast<float32_t>(i_array_max))
         && (middle_area.ending_pos - middle_area.starting_pos >= 10)
         && (middle_area.mean_val >= 0.15F)
         && (middle_area.max_val >= 0.4F))
      {
         TL_Area rear_area;

         rear_area.starting_pos = Peak_Right_Edge(third_peak_group, third_peak_cnt - 1) + 4;
         rear_area.starting_pos = std::min(rear_area.starting_pos, 50);

         rear_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         rear_area.ref_val = std::max(second_peak_group[second_peak_cnt - 1].peak_val, third_peak_group[0].peak_val);
         rear_area.mean_val = 0.0F;
         rear_area.max_val = 0.0F;
         Get_Area_Info(rear_area);

         if ((rear_area.mean_val <= 0.05F) && (rear_area.max_val <= 0.1F))
         {
            last_peak = static_cast<float32_t>(Peak_Right_Edge(second_peak_group, second_peak_cnt - 1));
            f_shrink = true;
         }
         else
         {
            // do nothing
         }
      }

      // BMZ-18705-short-flatbed
      if ((!f_shrink) && f_noise
         && (third_peak_cnt >= 1) && (third_peak_group[0].peak_pos > 35)
         && (second_peak_cnt >= 2) && (second_peak_group[second_peak_cnt - 1].peak_pos >= 30)
         && ((static_cast<float32_t>(j_array_max) >= 0.2F * static_cast<float32_t>(i_array_max)) 
            && (static_cast<float32_t>(j_array_max) <= 0.4F * static_cast<float32_t>(i_array_max))))
      {
         TL_Area mid_area;

         mid_area.starting_pos = Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
         mid_area.ending_pos = Peak_Right_Edge(second_peak_group, second_peak_cnt - 1);
         mid_area.ref_val = i_array_max;
         mid_area.mean_val = 0.0F;
         mid_area.max_val = 0.0F;
         Get_Area_Info(mid_area);

         TL_Area rear_area;

         rear_area.starting_pos = mid_area.ending_pos + 1;
         rear_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         rear_area.ref_val = i_array_max;
         rear_area.mean_val = 0.0F;
         rear_area.max_val = 0.0F;
         Get_Area_Info(rear_area);

         if ((mid_area.mean_val >= 0.05F) && (mid_area.max_val >= 0.2F)
            && (rear_area.mean_val <= 0.02F) && (rear_area.max_val <= 0.1F))
         {
            last_peak = 0.5F * static_cast<float32_t>(second_peak_group[0].peak_pos) + 0.5F * static_cast<float32_t>(second_peak_group[second_peak_cnt - 1].peak_pos);
            f_shrink = true;
         }
         else
         {
            // do nothing
         }
      }
   }
   
   void Trailer_Detector_TL::Extend_Trailer_Length(
      const bool f_noise, 
      const int32_t i_array_max, 
      const int32_t j_array_max, 
      const int32_t k_array_max, 
      float32_t& last_peak)
   {
      bool f_extend = false;
      // extend some estimation
      // some peaks are not located for 6m open trailer
      if (f_noise && (third_peak_cnt == 1) && (third_peak_group[0].peak_pos <= 30)
         && (static_cast<float32_t>(k_array_max) >= 0.25F * static_cast<float32_t>(j_array_max)))
      {
         TL_Area rear_area;
         rear_area.starting_pos = Peak_Right_Edge(third_peak_group, third_peak_cnt - 1);
         rear_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         rear_area.ref_val = third_peak_group[third_peak_cnt - 1].peak_val;
         rear_area.mean_val = 0.0F;
         rear_area.max_val = 0.0F;
         Get_Area_Info(rear_area);

         if ((rear_area.mean_val <= 0.1F) && (rear_area.max_val <= 0.5F))
         {
            last_peak = static_cast<float32_t>(third_peak_group[0].peak_pos);
            f_extend = true;
         }
      }

      // some long box trailer, it is difficult to reach the end of trailer
      if ((!f_extend) && f_noise && (second_peak_cnt == 1) && (first_peak_cnt == 1)
         && (first_peak_group[0].peak_pos <= 10) && (second_peak_group[0].peak_pos >= 20)
         && (static_cast<float32_t>(j_array_max) <= 0.4F * static_cast<float32_t>(i_array_max)) && (front_area.mean_val <= 0.05F))
      {
         TL_Area rear_area;
         rear_area.starting_pos = Peak_Right_Edge(second_peak_group, second_peak_cnt - 1);
         rear_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         rear_area.ref_val = second_peak_group[second_peak_cnt - 1].peak_val;
         rear_area.mean_val = 0.0F;
         rear_area.max_val = 0.0F;
         Get_Area_Info(rear_area);

         if (rear_area.mean_val <= 0.05F)
         {
            const int32_t temp = second_peak_group[second_peak_cnt - 1].peak_pos - first_peak_group[second_peak_cnt - 1].peak_pos;
            last_peak = last_peak + 0.7F * static_cast<float32_t>(temp);
            f_extend = true;
         }
      }

      // some long open trailer
      // if the reflection is strong after 8m, it may be a long trailer.
      if ((!f_extend) && f_noise && (third_peak_cnt >= 1) && (first_peak_group[0].peak_pos <= 15))
      {
         TL_Area part1_area;
         part1_area.starting_pos = Peak_Right_Edge(first_peak_group, first_peak_cnt - 1);
         part1_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         part1_area.ref_val = i_array_max;
         part1_area.mean_val = 0.0F;
         part1_area.max_val = 0.0F;
         Get_Area_Info(part1_area);

         TL_Area part2_area;
         part2_area.starting_pos = 40;
         part2_area.ending_pos = static_cast<int32_t>(ROW_NUMBER - 1U);
         part2_area.ref_val = i_array_max;
         part2_area.mean_val = 0.0F;
         part2_area.max_val = 0.0F;
         Get_Area_Info(part2_area);

         TL_Area part3_area;
         part3_area.starting_pos = 15;
         part3_area.ending_pos = 25;
         part3_area.ref_val = i_array_max;
         part3_area.mean_val = 0.0F;
         part3_area.max_val = 0.0F;
         Get_Area_Info(part3_area);

         if (((((part1_area.max_val <= 0.2F) && (part1_area.mean_val >= 0.03F))
            || ((part1_area.max_val <= 0.35F) && (part1_area.mean_val >= 0.05F)))
            && (part2_area.max_val >= 0.03F) && (part2_area.mean_val >= 0.01F)
            && (part1_area.max_val <= 2.0F * part2_area.max_val))
            || ((part1_area.max_val <= 0.25F) && (part1_area.mean_val >= 0.025F)
               && (part2_area.max_val >= 0.1F) && (part2_area.mean_val >= 0.02F))
            || ((part1_area.max_val <= 0.8F) && (part1_area.mean_val >= 0.05F)
               && (part1_area.mean_val <= 0.1F) && (part2_area.max_val >= 0.5F)
               && (part2_area.mean_val >= 0.07F) && (part3_area.mean_val >= 0.05F)
               && (third_peak_group[0].peak_pos >= 40)))
         {
            last_peak = 1.5F * static_cast<float32_t>(third_peak_group[third_peak_cnt - 1].peak_pos) - 0.5F * static_cast<float32_t>(second_peak_group[second_peak_cnt - 1].peak_pos);
         }
         else if ((part1_area.max_val >= 0.7F) && (part1_area.mean_val >= 0.2F)
            && (third_peak_group[third_peak_cnt - 1].peak_pos <= 45))
         {
            last_peak = 1.5F * static_cast<float32_t>(third_peak_group[third_peak_cnt - 1].peak_pos) - 0.5F * static_cast<float32_t>(second_peak_group[second_peak_cnt - 1].peak_pos);
         }
         else if ((part1_area.max_val >= 0.7F) && (part1_area.mean_val >= 0.2F)
            && (third_peak_group[third_peak_cnt - 1].peak_pos > 45))
         {
            last_peak = 0.5F * (static_cast<float32_t>(third_peak_group[third_peak_cnt - 1].peak_pos) + static_cast<float32_t>(second_peak_group[second_peak_cnt - 1].peak_pos));
         }
         else
         {
            // do nothing
         }
      }
   }

   void Trailer_Detector_TL::Estimate_Trailer_Length_SVM()
   {
      float32_t norm_sample[ROW_NUMBER];
      int32_t predict_class = -1;
      float32_t max_score = 0.0F;

      Norm_Detection_Row(trailer_detection_row_struct.detection_row, norm_sample);
      SVM_Classification(norm_sample, predict_class, max_score);
      SVM_confidence(predict_class, max_score);
   }

   void Trailer_Detector_TL::Norm_Detection_Row(const int32_t (&detection_row)[ROW_NUMBER], float32_t (&norm_sample)[ROW_NUMBER])
   {
      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         norm_sample[i] = 0.0F;
      }

      int32_t detection_row_norm_int = 0;
      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         const int32_t temp = detection_row[i] * detection_row[i];
         detection_row_norm_int += temp;
      }
      float32_t detection_row_norm_float = F360_Sqrtf(static_cast<float32_t>(detection_row_norm_int));
      detection_row_norm_float = std::max(detection_row_norm_float, 1.0F);

      const float32_t detection_row_norm_inv = 1.0F / detection_row_norm_float;
      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         norm_sample[i] = static_cast<float32_t>(detection_row[i]) * detection_row_norm_inv;
      }
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::SVM_classification
    *
    * Description    Private method used to do estimation of trailer length
    *                by using SVM classificaiton of 10 possible trailer length classes
    *
    * Parameters     
    * const float32_t (&norm_sample)[ROW_NUMBER]
    * int32_t& predict_class
    * float32_t& max_score
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
   void Trailer_Detector_TL::SVM_Classification(
      const float32_t (&norm_sample)[ROW_NUMBER], 
      int32_t& predict_class, 
      float32_t& max_score)
   {
      float32_t scores[CLASS_NUMBER]{};

      for (uint32_t i = 0U; i < CLASS_NUMBER; i++)
      {
         scores[i] = 0.0F;
      }

      for (uint32_t i = 0U; i < CLASS_NUMBER; i++)
      {
         for (uint32_t k = 0U; k < ROW_NUMBER; k++)
         {
            scores[i] += (norm_sample[k] / scale[i])*beta[i][k];
         }

         scores[i] += bias[i];
      }

      // class
      predict_class = static_cast<int32_t>(F360_Max_Index(scores, CLASS_NUMBER));
      max_score = scores[predict_class];
      svm_outputs.trailer_length_SVM = length_array[predict_class];
   }

   /*=========================================================================
    * Method         Trailer_Detector_TL::SVM_confidence
    *
    * Description    Private method used to do estimate confidance of trailer length
    *                classification.
    *
    * Parameters
    * const int32_t predict_class
    * const float32_t max_score
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
   void Trailer_Detector_TL::SVM_confidence(
      const int32_t predict_class, 
      const float32_t max_score)
   {
      for (int8_t i = 0; i < SVM_CONF_LEVEL; i++)
      {
         if (max_score >= svm_score[predict_class][i])
         {
            svm_outputs.trailer_length_SVM_conf = svm_conf[i];
            break;
         }
         else
         {
            svm_outputs.trailer_length_SVM_conf = 0.0F;
         }
      }
   }

  /*=========================================================================
  * Method         Trailer_Detector_TL::Post_Processing
  *
  * Description    Private method used to finalize estimation and
  *                post process the output from SVM classification
  *                and SVM confidence functions.
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
   void Trailer_Detector_TL::Post_Processing()
   {
      if (trailer_length_peaks >= 1.0F)
      {
         const float32_t SVM_conf_threshold = 0.05F;
         if (svm_outputs.trailer_length_SVM_conf >= SVM_conf_threshold)
         {
            trailer_length = svm_outputs.trailer_length_SVM;
         }
         else
         {
            trailer_length = trailer_length_peaks;
         }

         axle_trailer_length = trailer_length * 0.7F;
         trailer_HV_gap = fminf(static_cast<float32_t>(first_peak_group[0].peak_pos) * tl_calibs.k_row_interval, 2.0F);
         trailer_length_conf = 3U;
      }
      else
      {
         trailer_length = 0.0F;
         axle_trailer_length = 0.0F;
         trailer_HV_gap = 0.0F;
         trailer_length_conf = 0U;
      }
   }
   }
