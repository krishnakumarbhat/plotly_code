/*===================================================================================*\
 * FILE:  f360_populate_internal_trailer_detector_log.cpp
 *====================================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_populate_internal_trailer_detector_log.h"

namespace f360_variant_A
{
   void Populate_Internal_Trailer_Detector_Log_Data(F360_Internal_Trailer_Detector_T& trailer_internal_log,
      const Trailer_Detector_Core& trailer_detector)
   {
      const Trailer_Detector_TP tp_obj = trailer_detector.Get_Trailer_Presence_Instance();
      const Trailer_Detector_TL tl_obj = trailer_detector.Get_Trailer_Length_Instance();
      const Trailer_Detector_TW tw_obj = trailer_detector.Get_Trailer_Width_Instance();
      const Trailer_Detector_TA ta_obj = trailer_detector.Get_Trailer_Angle_Instance();

      trailer_internal_log.tp_window_timer = tp_obj.Get_Window_Timer();
      trailer_internal_log.tp_f_estimation_done = tp_obj.Get_Estimation_Done_Flag();
      trailer_internal_log.TP_Relative_Ratio_dets_n_02 = tp_obj.Get_Relative_Ratio_Struct().dets_n_02;
      trailer_internal_log.TP_Relative_Ratio_dets_n_03 = tp_obj.Get_Relative_Ratio_Struct().dets_n_03;
      trailer_internal_log.TP_Relative_Ratio_dets_n_04 = tp_obj.Get_Relative_Ratio_Struct().dets_n_04;
      trailer_internal_log.TP_Relative_Ratio_dets_n_05 = tp_obj.Get_Relative_Ratio_Struct().dets_n_05;
      (void) std::copy(cmn::begin(tp_obj.Get_Mean_RR_Struct().value_array), 
                       cmn::begin(tp_obj.Get_Mean_RR_Struct().value_array), 
                       trailer_internal_log.TP_Mean_RR_value_array);
      auto& num_array = tp_obj.Get_Mean_RR_Struct().num_array;
      (void) std::copy(cmn::begin(num_array), cmn::end(num_array), trailer_internal_log.TP_Mean_RR_num_array);

      trailer_internal_log.tl_f_estimation_done = tl_obj.Get_Estimation_Done_Flag();
      trailer_internal_log.tl_reset_timer = tl_obj.Get_Reset_Timer();
      trailer_internal_log.tl_window_timer = tl_obj.Get_Window_Timer();
      trailer_internal_log.axel_trailer_length = tl_obj.Get_Axle_Trailer_Length_Struct();
      auto& detection_row = tl_obj.Get_Trailer_Detection_Row().detection_row;
      (void) std::copy(cmn::begin(detection_row), cmn::end(detection_row), trailer_internal_log.detection_row);

      trailer_internal_log.tw_reset_timer = tw_obj.Get_Reset_Timer();
      trailer_internal_log.tw_window_timer = tw_obj.Get_Window_Timer();
      trailer_internal_log.tw_f_estimation_done = tw_obj.Get_Estimation_Done_Flag();
      const uint32_t* const p_detection_col_begin = tw_obj.Get_Detection_Col_Begin();
      const uint32_t* const p_detection_col_end = tw_obj.Get_Detection_Col_End();
      (void) std::copy(p_detection_col_begin, p_detection_col_end, trailer_internal_log.detection_col);

      trailer_internal_log.HV_angle = ta_obj.Get_HV_Angle();
      trailer_internal_log.HV_cnt = ta_obj.Get_HV_Cnt();
      trailer_internal_log.prev_trailer_angle = ta_obj.Get_Prev_Trailer_Angle();
      trailer_internal_log.trailer_axel_length = ta_obj.Get_Trailer_Axle_Length();
   }

   void Populate_Internal_Trailer_Detector_Data(Trailer_Detector_Core& trailer_detector,
      const F360_Internal_Trailer_Detector_T& trailer_internal_log, 
      const Trailer_Detector_Log_T& trailer_detector_log)
   {
      Trailer_Detector_TP tp_obj = trailer_detector.Get_Trailer_Presence_Instance();
      Trailer_Detector_TL tl_obj = trailer_detector.Get_Trailer_Length_Instance();
      Trailer_Detector_TW tw_obj = trailer_detector.Get_Trailer_Width_Instance();
      Trailer_Detector_TA ta_obj = trailer_detector.Get_Trailer_Angle_Instance();
      Trailer_Detector_Outputs output_obj = trailer_detector.Get_Trailer_Output_Instance();

      tp_obj.Set_Trailer_Presence(static_cast<Trailer_Presence_State>(trailer_detector_log.trailer_presence));
      tp_obj.Set_Window_Timer(trailer_internal_log.tp_window_timer);
      tp_obj.Set_Trailer_Presence_Conf(static_cast<Trailer_Detector_Conf>(trailer_detector_log.trailer_presence_conf));
      tp_obj.Set_Estimation_Done_Flag(trailer_internal_log.tp_f_estimation_done);
      tp_obj.Set_Stationary_Timer(trailer_detector_log.stationary_timer);
      TP_Relative_Ratio relative_ratio_struct = tp_obj.Get_Relative_Ratio_Struct();
      relative_ratio_struct.dets_n_02 = trailer_internal_log.TP_Relative_Ratio_dets_n_02;
      relative_ratio_struct.dets_n_03 = trailer_internal_log.TP_Relative_Ratio_dets_n_03;
      relative_ratio_struct.dets_n_04 = trailer_internal_log.TP_Relative_Ratio_dets_n_04;
      relative_ratio_struct.dets_n_05 = trailer_internal_log.TP_Relative_Ratio_dets_n_05;
      tp_obj.Set_Relative_Ratio_Struct(relative_ratio_struct);
      TP_Mean_RR mean_rr_struct = tp_obj.Get_Mean_RR_Struct();
      (void) std::copy(cmn::begin(trailer_internal_log.TP_Mean_RR_value_array), cmn::end(trailer_internal_log.TP_Mean_RR_value_array), mean_rr_struct.value_array);
      (void) std::copy(cmn::begin(trailer_internal_log.TP_Mean_RR_num_array), cmn::end(trailer_internal_log.TP_Mean_RR_num_array), mean_rr_struct.num_array);
      tp_obj.Set_Mean_RR_Struct(mean_rr_struct);

      tl_obj.Set_Estimation_Done_Flag(trailer_internal_log.tl_f_estimation_done);
      tl_obj.Set_Reset_Timer(trailer_internal_log.tl_reset_timer);
      tl_obj.Set_Window_Timer(trailer_internal_log.tl_window_timer);
      tl_obj.Set_Trailer_Length_Struct(trailer_detector_log.trailer_length);
      tl_obj.Set_Axle_Trailer_Length_Struct(trailer_internal_log.axel_trailer_length);
      TL_Trailer_Detection_Row trailer_detection_row_struct = tl_obj.Get_Trailer_Detection_Row();
      (void) std::copy(cmn::begin(trailer_internal_log.detection_row), 
                       cmn::end(trailer_internal_log.detection_row), 
                       trailer_detection_row_struct.detection_row);

      tl_obj.Set_Trailer_Detection_Row(trailer_detection_row_struct);

      tw_obj.Set_Reset_Timer(trailer_internal_log.tw_reset_timer);
      tw_obj.Set_Window_Timer(trailer_internal_log.tw_window_timer);
      tw_obj.Set_Estimation_Done_Flag(trailer_internal_log.tw_f_estimation_done);
      uint32_t detection_col[COL_NUMBER];
      (void) std::copy(cmn::begin(trailer_internal_log.detection_col),
                       cmn::end(trailer_internal_log.detection_col), 
                       cmn::begin(detection_col));
      tw_obj.Set_Detection_Col(detection_col);
      tw_obj.Set_Trailer_Width(trailer_detector_log.trailer_width);
      tw_obj.Set_Trailer_Width_Conf(static_cast<Trailer_Detector_Conf>(trailer_detector_log.trailer_width_conf));

      ta_obj.Set_HV_Cnt(trailer_internal_log.HV_cnt);
      ta_obj.Set_HV_Angle(trailer_internal_log.HV_angle);
      ta_obj.Set_Prev_Trailer_Angle(trailer_internal_log.prev_trailer_angle);
      ta_obj.Set_Trailer_Axle_Length(trailer_internal_log.trailer_axel_length);
      ta_obj.Set_Trailer_Angle_Rad(trailer_detector_log.trailer_angle);
      ta_obj.Set_Trailer_Angle_Rate_Rad(trailer_detector_log.trailer_angle_rate);

      output_obj.Set_Radar_Detection_Timer(trailer_detector_log.radar_detection_timer);

      trailer_detector.Set_Trailer_Presence_Instance(tp_obj);
      trailer_detector.Set_Trailer_Length_Instance(tl_obj);
      trailer_detector.Set_Trailer_Width_Instance(tw_obj);
      trailer_detector.Set_Trailer_Angle_Instance(ta_obj);
      trailer_detector.Set_Trailer_Output_Instance(output_obj);
   }
}
