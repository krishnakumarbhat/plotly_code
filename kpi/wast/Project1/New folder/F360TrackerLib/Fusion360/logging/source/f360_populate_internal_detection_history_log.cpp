/*===================================================================================*\
* FILE:  f360_populate_internal_detection_history_log.cpp
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*------------------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*/
#include "f360_populate_internal_detection_history_log.h"

namespace f360_variant_A
{
   void Populate_Internal_Detection_History_Data(F360_Detection_Hist_T& det_hist,
      const F360_Internal_Detection_Hist_T(&det_hist_data_log)[MAX_NUMBER_OF_HISTORIC_DETECTIONS])
   {
      det_hist.n_occupied = 0;
      det_hist.max_occupation = 0;
      for (uint16_t i = 0U; i < MAX_NUMBER_OF_HISTORIC_DETECTIONS; i++)
      {
         const int32_t occupied_idx = det_hist_data_log[i].occupied_idx;
         const bool f_valid_occupied = ((0 == occupied_idx) && (0U == i)) ||
            ((0 < occupied_idx) && (static_cast<int32_t>(MAX_NUMBER_OF_HISTORIC_DETECTIONS) > occupied_idx));
         if (f_valid_occupied)
         {
            det_hist.det_data[occupied_idx].position_cov_nees[0][0] = det_hist_data_log[i].position_cov_nees[0][0];
            det_hist.det_data[occupied_idx].position_cov_nees[0][1] = det_hist_data_log[i].position_cov_nees[0][1];
            det_hist.det_data[occupied_idx].position_cov_nees[1][0] = det_hist_data_log[i].position_cov_nees[1][0];
            det_hist.det_data[occupied_idx].position_cov_nees[1][1] = det_hist_data_log[i].position_cov_nees[1][1];
            det_hist.det_data[occupied_idx].vcs_position.y = det_hist_data_log[i].vcs_lat_posn;
            det_hist.det_data[occupied_idx].vcs_position.x = det_hist_data_log[i].vcs_long_posn;
            det_hist.det_data[occupied_idx].rdot = det_hist_data_log[i].rdot;
            det_hist.det_data[occupied_idx].rdot_comp = det_hist_data_log[i].rdot_comp;
            det_hist.det_data[occupied_idx].vcs_az = det_hist_data_log[i].vcs_az;
            det_hist.det_data[occupied_idx].time_since_meas = det_hist_data_log[i].time_since_meas;
            det_hist.det_data[occupied_idx].v_wrapping = det_hist_data_log[i].v_wrapping;
            det_hist.det_data[occupied_idx].r_wrapping = det_hist_data_log[i].r_wrapping;
            det_hist.det_data[occupied_idx].cluster_idx = det_hist_data_log[i].cluster_idx;
            det_hist.det_data[occupied_idx].look_type = static_cast<F360_Det_Look_Type_T>(det_hist_data_log[i].look_type);
            det_hist.det_data[occupied_idx].range_type = static_cast<F360_Det_Range_Type_T>(det_hist_data_log[i].range_type);
            det_hist.det_data[occupied_idx].motion_status = static_cast<rspp_variant_A::RSPP_Detection_Motion_Status_T>(det_hist_data_log[i].motion_status);
            det_hist.det_data[occupied_idx].wheel_spin_type = static_cast<F360_Detection_Wheelspin_Type_T>(det_hist_data_log[i].wheel_spin_type);
            det_hist.det_data[occupied_idx].f_dealiased = (1U == det_hist_data_log[i].f_dealiased);
            det_hist.det_data[occupied_idx].f_FOV_edge = (1U == det_hist_data_log[i].f_FOV_edge);
            det_hist.det_data[occupied_idx].f_selected = (1U == det_hist_data_log[i].f_selected);
            det_hist.det_data[occupied_idx].f_azimuth_error_stat_mov = (1U == det_hist_data_log[i].f_azimuth_error_stat_mov);
            det_hist.det_data[occupied_idx].f_is_range_in_all_looks = (1U == det_hist_data_log[i].f_is_range_in_all_looks);
            det_hist.det_data[occupied_idx].f_potential_angle_jump = (1U == det_hist_data_log[i].f_potential_angle_jump);

            det_hist.f_idx_occupied[occupied_idx] = true;
            det_hist.max_occupation = occupied_idx;
            det_hist.n_occupied++;
         }
      }
   }

   void Populate_Internal_Detection_History_Log_Data(
      F360_Internal_Detection_Hist_T(&det_hist_data_log)[MAX_NUMBER_OF_HISTORIC_DETECTIONS],
      const F360_Detection_Hist_T& det_hist)
   {
      int32_t n = 0;
      for (uint16_t i = 0U; i < MAX_NUMBER_OF_HISTORIC_DETECTIONS; i++)
      {
         if (det_hist.f_idx_occupied[i])
         {
            det_hist_data_log[n].position_cov_nees[0][0] = det_hist.det_data[i].position_cov_nees[0][0];
            det_hist_data_log[n].position_cov_nees[0][1] = det_hist.det_data[i].position_cov_nees[0][1];
            det_hist_data_log[n].position_cov_nees[1][0] = det_hist.det_data[i].position_cov_nees[1][0];
            det_hist_data_log[n].position_cov_nees[1][1] = det_hist.det_data[i].position_cov_nees[1][1];
            det_hist_data_log[n].vcs_lat_posn = det_hist.det_data[i].vcs_position.y;
            det_hist_data_log[n].vcs_long_posn = det_hist.det_data[i].vcs_position.x;
            det_hist_data_log[n].rdot = det_hist.det_data[i].rdot;
            det_hist_data_log[n].rdot_comp = det_hist.det_data[i].rdot_comp;
            det_hist_data_log[n].vcs_az = det_hist.det_data[i].vcs_az;
            det_hist_data_log[n].time_since_meas = det_hist.det_data[i].time_since_meas;
            det_hist_data_log[n].v_wrapping = det_hist.det_data[i].v_wrapping;
            det_hist_data_log[n].r_wrapping = det_hist.det_data[i].r_wrapping;
            det_hist_data_log[n].cluster_idx = det_hist.det_data[i].cluster_idx;
            det_hist_data_log[n].occupied_idx = static_cast<int16_t>(i);
            det_hist_data_log[n].look_type = static_cast<int8_t>(det_hist.det_data[i].look_type);
            det_hist_data_log[n].range_type = static_cast<int8_t>(det_hist.det_data[i].range_type);
            det_hist_data_log[n].motion_status = static_cast<int8_t>(det_hist.det_data[i].motion_status);
            det_hist_data_log[n].wheel_spin_type = static_cast<int8_t>(det_hist.det_data[i].wheel_spin_type);
            det_hist_data_log[n].f_dealiased = det_hist.det_data[i].f_dealiased ? 1U : 0U;
            det_hist_data_log[n].f_FOV_edge = det_hist.det_data[i].f_FOV_edge ? 1U : 0U;
            det_hist_data_log[n].f_selected = det_hist.det_data[i].f_selected ? 1U : 0U;
            det_hist_data_log[n].f_azimuth_error_stat_mov = det_hist.det_data[i].f_azimuth_error_stat_mov ? 1U : 0U;
            det_hist_data_log[n].f_is_range_in_all_looks = det_hist.det_data[i].f_is_range_in_all_looks ? 1U : 0U;
            det_hist_data_log[n].f_potential_angle_jump = det_hist.det_data[i].f_potential_angle_jump ? 1U : 0U;
            n++;
         }
      }
   }
}
