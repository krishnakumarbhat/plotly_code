/*===================================================================================*\
* FILE: f360_nees_cfmi_info_cloud.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains functions definitions:
*     - Get_NEES_CFMI_Information_Cloud_Current_Dets()
*     - Get_NEES_CFMI_Information_Cloud_Old_Dets()
*     - Update_NEES_Detection_Slot()
*     - Calc_NEES_CFMI_Determinants_Cloud()
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#include "f360_nees_cfmi_info_cloud.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information_Cloud_Current_Dets
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Cluster_T& cluster_to_init,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Detection_Props_T& det_props,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Extract valuable information for nees cost function initialization
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_NEES_CFMI_Information_Cloud_Current_Dets(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t num_dets = static_cast<uint32_t>(cluster_to_init.ndets);
      const bool can_iterate = (nees_cfmi_information.dets_num < F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET);

      if (can_iterate)
      {
         for (uint32_t index = 0U; index < num_dets; index++)
         {
            const uint32_t det_index = static_cast<uint32_t>(cluster_to_init.detids[index]) - 1U;
            const F360_Detection_Props_T &detection = det_props[det_index];
            const rspp_variant_A::RSPP_Detection_T &detection_raw = raw_detection_list.detections[det_index];
            F360_NEES_CFMI_Detection_T& nees_detection_slot = nees_cfmi_information.detections[nees_cfmi_information.dets_num];
            const bool det_is_historical = false;
            const int32_t sensor_idx = detection_raw.raw.sensor_id - 1;
            const float32_t time_since_measurement = sensors[sensor_idx].variable.time_since_measurement_s;
            
            Update_NEES_Detection_Slot(calibrations,
               time_since_measurement,
               detection.vcs_position,
               detection.position_cov_nees,
               det_index,
               nees_cfmi_information.dets_num,
               detection.range_rate_compensated,
               detection_raw.processed.vcs_az,
               detection.wheel_spin_type,
               detection_raw.processed.motion_status,
               det_is_historical,
               nees_detection_slot,
               nees_cfmi_information);

            nees_cfmi_information.dets_num++;
            nees_cfmi_information.current_dets_count++;
            if (nees_cfmi_information.dets_num >= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET)
            {
               break;
            }
         }
      }
   }

/*===========================================================================*\
* FUNCTION: Get_NEES_CFMI_Information_Cloud_Old_Dets
*===========================================================================
* RETURN VALUE:
* None
*
* PARAMETERS:
*
* const F360_Calibrations_T& calibrations,
* const F360_Cluster_T& cluster_to_init,
* const F360_Detection_Hist_T& detection_hist,
* F360_NEES_CFMI_Information_T& nees_cfmi_information
*
* DEVIATIONS FROM STANDARDS:
* None.
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* Extract valuable information for nees cost function initialization
*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
*
\*===========================================================================*/
   void Get_NEES_CFMI_Information_Cloud_Old_Dets(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Hist_T& detection_hist,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const uint32_t num_old_dets = static_cast<uint32_t>(cluster_to_init.num_old_dets);
      const bool can_iterate = (nees_cfmi_information.dets_num < F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET);

      if (can_iterate)
      {
         for (uint32_t index = 0U; index < num_old_dets; index++)
         {
            const uint32_t det_index = static_cast<uint32_t>(cluster_to_init.old_det_idx[index]);
            const F360_Detection_Hist_Data_T& detection = detection_hist.det_data[det_index];
            F360_NEES_CFMI_Detection_T& nees_detection_slot = nees_cfmi_information.detections[nees_cfmi_information.dets_num];
            const bool det_is_historical = true;

            Update_NEES_Detection_Slot(calibrations,
               detection.time_since_meas,
               detection.vcs_position,
               detection.position_cov_nees,
               det_index,
               nees_cfmi_information.dets_num,
               detection.rdot_comp,
               detection.vcs_az,
               detection.wheel_spin_type,
               detection.motion_status,
               det_is_historical,
               nees_detection_slot,
               nees_cfmi_information);

            nees_cfmi_information.dets_num++;
            nees_cfmi_information.old_dets_count++;

            if (nees_cfmi_information.dets_num >= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET)
            {
               break;
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Update_NEES_Detection_Slot()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations,
   * const float32_t det_time_since_measurement,
   * const Point detection_position,
   * const float32_t (&detection_position_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   * const uint32_t source_array_index,
   * const float32_t det_rrate_comp,
   * const float32_t det_azimuth,
   * const F360_Detection_Wheelspin_Type_T det_wheelspin_type,
   * const RSPP_Detection_Motion_Status_T det_motion_status
   * const bool det_is_historical
   * F360_NEES_CFMI_Detection_T& nees_detection_slot,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Fill internal NEES structure which represents detection with given data.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_NEES_Detection_Slot(
      const F360_Calibrations_T& calibrations,
      const float32_t det_time_since_measurement,
      const Point detection_position,
      const float32_t (&detection_position_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const uint32_t source_array_index,
      const uint32_t nees_array_index,
      const float32_t det_rrate_comp,
      const float32_t det_azimuth,
      const F360_Detection_Wheelspin_Type_T det_wheelspin_type,
      const int8_t det_motion_status,
      const bool det_is_historical,
      F360_NEES_CFMI_Detection_T& nees_detection_slot,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      nees_detection_slot.timestamp_diff = det_time_since_measurement - nees_cfmi_information.min_time_since_meas;
      const float32_t acceleration_var = calibrations.k_nees_cfmi_process_noise_acc_std * calibrations.k_nees_cfmi_process_noise_acc_std;
      const float32_t acceleration_influence_var = (nees_detection_slot.timestamp_diff * nees_detection_slot.timestamp_diff) * (acceleration_var);

      const Point pos_diff = {
         detection_position.x - nees_cfmi_information.pos_center.x,
         detection_position.y - nees_cfmi_information.pos_center.y,
      };

      const float32_t yaw_rate_std = F360_DEG2RAD(calibrations.k_nees_cfmi_process_noise_yaw_rate_std_deg);
      const float32_t yaw_rate_var = yaw_rate_std * yaw_rate_std;
      const float32_t pos_diff_square = F360_Get_Hypotenuse_Squared(pos_diff.x, pos_diff.y);
      const float32_t yaw_rate_influence_var = yaw_rate_var * pos_diff_square;
      const float32_t range_rate_comp_var = calibrations.k_nees_cfmi_rrate_comp_std * calibrations.k_nees_cfmi_rrate_comp_std;

      nees_detection_slot.f_valid = true;
      nees_detection_slot.idx.f_historical = det_is_historical;
      nees_detection_slot.idx.idx = source_array_index;
      nees_detection_slot.idx.nees_idx = nees_array_index;
      nees_detection_slot.time_since_meas = det_time_since_measurement;
      nees_detection_slot.pos = detection_position;
      (void)std::copy(cmn::begin(detection_position_cov), cmn::end(detection_position_cov), cmn::begin(nees_detection_slot.pos_cov));

      nees_detection_slot.range_rate_comp = det_rrate_comp;
      nees_detection_slot.range_rate_comp_var = range_rate_comp_var + acceleration_influence_var + yaw_rate_influence_var;
      nees_detection_slot.sin_vcs_az = F360_Sinf(det_azimuth);
      nees_detection_slot.cos_vcs_az = F360_Cosf(det_azimuth);

      Calc_NEES_CFMI_Determinants_Cloud(nees_detection_slot);

      if (det_wheelspin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID)
      {
         nees_detection_slot.f_inlier = true;
         nees_cfmi_information.init_dets_inliers_num++;
      }

      if (det_motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING)
      {
         nees_cfmi_information.moving_dets_num++;
      }
   }

/*===========================================================================*\
* FUNCTION: Calc_NEES_CFMI_Determinants_Cloud
*===========================================================================
* RETURN VALUE:
* None
*
* PARAMETERS:
*
* F360_NEES_CFMI_Detection_T& detection
*
* DEVIATIONS FROM STANDARDS:
* None.
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* Calculate NEES cost determinants in case of cloud
*
* PRECONDITIONS:
* None
*
* POSTCONDITIONS:
* None
*
\*===========================================================================*/
   void Calc_NEES_CFMI_Determinants_Cloud(F360_NEES_CFMI_Detection_T& detection)
   {
      if (detection.range_rate_comp_var > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_range_rate_comp_var = 1.0F / detection.range_rate_comp_var;

         detection.cloud_determinants.n_dets = 1U;
         detection.cloud_determinants.Sxx = detection.cos_vcs_az * detection.cos_vcs_az * one_over_range_rate_comp_var;
         detection.cloud_determinants.Sxy = detection.cos_vcs_az * detection.sin_vcs_az * one_over_range_rate_comp_var;
         detection.cloud_determinants.Sx = detection.cos_vcs_az * detection.range_rate_comp * one_over_range_rate_comp_var;
         detection.cloud_determinants.Syy = detection.sin_vcs_az * detection.sin_vcs_az * one_over_range_rate_comp_var;
         detection.cloud_determinants.Sy = detection.sin_vcs_az * detection.range_rate_comp * one_over_range_rate_comp_var;

         detection.cloud_determinants.Syx = detection.cloud_determinants.Sxy;

         detection.cloud_determinants.information = 1.0F / detection.range_rate_comp_var;
      }
   }

}
