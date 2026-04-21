/*===================================================================================*\
* FILE: f360_nees_cfmi_info_pos_diff.cpp
*====================================================================================
* Copyright - 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains definitions of functions: Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities().
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_nees_cfmi_info_pos_diff.h"
#include "f360_uncertainty_propagation.h"
#include "f360_math.h"
#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_iterator.h"
#include <algorithm>

namespace f360_variant_A
{
   static F360_NEES_CFMI_Vel_RR_Conf_T Get_Velocity_Hypothesis_Confidence_Level(
      const F360_Calibrations_T& calibrations,
      const float32_t plausibility);

   struct Slot_Is_Invalid
   {
      bool operator()(const F360_Detection_Time_Slot_T& current_slot) const
      {
         return current_slot.number_of_active_detections > F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT;
      }
   };


   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information_Poss_Diff_Old_Dets
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Cluster_T& cluster_to_init,
   * const F360_Detection_Hist_T& detection_hist,
   * const F360_Detection_Hist_Time_Slots_T& det_hist_time_slots,
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
   void Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Time_Slots_T& det_hist_time_slots,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const bool preconditions_valid = Check_Dets_Pos_Diff_Preconditions(det_hist_time_slots);
      if (preconditions_valid)
      {
         const uint32_t nth_time_slot = (nees_cfmi_information.f_is_any_det_in_all_looks) ? static_cast<uint32_t>(2U) : static_cast <uint32_t>(1U);

         // Historical detections for loop.
         const uint32_t num_slots = det_hist_time_slots.number_of_active_slots;

         for (uint32_t slot_index = nth_time_slot; slot_index < num_slots; slot_index++)
         {
            const F360_Detection_Time_Slot_T& time_since_meas_slot_A = det_hist_time_slots.time_since_meas_slots[slot_index - nth_time_slot];
            const F360_Detection_Time_Slot_T& time_since_meas_slot_B = det_hist_time_slots.time_since_meas_slots[slot_index];
            const uint32_t num_dets_slot_A = time_since_meas_slot_A.number_of_active_detections;
            const uint32_t num_dets_slot_B = time_since_meas_slot_B.number_of_active_detections;

            const uint32_t expected_vels_inliers_num_in_slot = std::min(num_dets_slot_A, num_dets_slot_B);
            nees_cfmi_information.expected_vels_inliers_num_in_slot[slot_index] = expected_vels_inliers_num_in_slot;
            nees_cfmi_information.expected_vels_inliers_num += expected_vels_inliers_num_in_slot;

            // Slot A vs Slot B
            for (uint32_t index_1 = 0U; index_1 < num_dets_slot_A; index_1++)
            {
               for (uint32_t index_2 = 0U; index_2 < num_dets_slot_B; index_2++)
               {
                  const uint32_t det_A_index = time_since_meas_slot_A.hist_dets_idx[index_1];
                  const uint32_t det_B_index = time_since_meas_slot_B.hist_dets_idx[index_2];
                  const bool indexes_are_ok = (det_A_index < F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET) && (det_B_index < F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET);

                  if (indexes_are_ok)
                  {
                     const F360_NEES_CFMI_Detection_T& nees_det_A = nees_cfmi_information.detections[det_A_index];
                     const F360_NEES_CFMI_Detection_T& nees_det_B = nees_cfmi_information.detections[det_B_index];

                     Update_NEES_Pos_Diff_Slot(calibrations, nees_det_A, nees_det_B, nees_cfmi_information);

                     if (nees_cfmi_information.vels_num >= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL)
                     {
                        break;
                     }
                  }
               }
               if (nees_cfmi_information.vels_num >= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL)
               {
                  break;
               }
            }
            if (nees_cfmi_information.vels_num >= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL)
            {
               break;
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Dets_Pos_Diff_Preconditions
   *===========================================================================
   * RETURN VALUE:
   * bool preconditions_are_valid
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check preconditions for Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   bool Check_Dets_Pos_Diff_Preconditions(const F360_Detection_Time_Slots_T& det_hist_time_slots)
   {
      static constexpr uint32_t MIN_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS = 2U;
      const uint32_t num_slots = det_hist_time_slots.number_of_active_slots;
      const bool num_slots_is_ok = (MIN_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS <= num_slots) && (num_slots <= F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS);

      bool preconditions_valid;
      if (num_slots_is_ok)
      {
         const F360_Detection_Time_Slot_T* const begin = cmn::begin(det_hist_time_slots.time_since_meas_slots);
         const F360_Detection_Time_Slot_T* const end = &det_hist_time_slots.time_since_meas_slots[num_slots];
         const F360_Detection_Time_Slot_T* const results = std::find_if(begin, end, Slot_Is_Invalid());

         preconditions_valid = (results == end);
      }
      else
      {
         preconditions_valid = false;
      }
      return preconditions_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Update_NEES_Pos_Diff_Slot
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Detection_T& detection_A,
   * const F360_NEES_CFMI_Detection_T& detection_B,
   * const uint32_t det_A_index,
   * const uint32_t det_B_index,
   * const uint32_t slot_index,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Updating slot for detections' pos diff information
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_NEES_Pos_Diff_Slot(
      const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Detection_T& detection_A,
      const F360_NEES_CFMI_Detection_T& detection_B,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const float32_t dt = detection_B.time_since_meas - detection_A.time_since_meas;

      F360_VCS_Velocity_T vel_hyp = {};
      const bool f_successfull_vel_estimation = Pos_Diff_Vel_Estimate(detection_A.pos, detection_B.pos, dt, vel_hyp);


      if (f_successfull_vel_estimation)
      {
         bool f_successfull_cov_estimation;
         float32_t vel_hyp_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
         f_successfull_cov_estimation = Pos_Diff_Cov_Estimate(detection_A.pos_cov, detection_B.pos_cov, dt, vel_hyp_cov);

         const float32_t determinant = F360_2d_Matrix_Determinant(vel_hyp_cov);
         const bool estimate_is_ok = (f_successfull_cov_estimation) && (determinant > F360_MIN_DENOMINATOR);
         if (estimate_is_ok)
         {
            const uint32_t vel_slot_idx = nees_cfmi_information.vels_num;
            F360_NEES_CFMI_Pos_Diff_Velocity_T &single_vel_slot = nees_cfmi_information.velocities[vel_slot_idx];

            const float32_t velocity_timestamp = (detection_A.time_since_meas + detection_B.time_since_meas) * 0.5F; // Velocity calculated by position difference is estimated in the middle of time-stamps
            const float32_t timestamp_diff = velocity_timestamp - nees_cfmi_information.min_time_since_meas;
            const float32_t process_noise_std = timestamp_diff * calibrations.k_nees_cfmi_process_noise_acc_std;

            float32_t process_noise_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
            Init_2d_Covariance_By_Std(process_noise_std, process_noise_cov);

            const float32_t sigma_level_squared = calibrations.k_nees_cfmi_sigma_level_for_vh_confirm * calibrations.k_nees_cfmi_sigma_level_for_vh_confirm;
            const float32_t rr_var = calibrations.k_nees_cfmi_rr_std_for_vh_confirm * calibrations.k_nees_cfmi_rr_std_for_vh_confirm;
            const float32_t det_A_plausibility = Calculate_Single_Det_Plausibility_For_VH(
               vel_hyp,
               vel_hyp_cov,
               detection_A.range_rate_comp,
               rr_var,
               detection_A.cos_vcs_az,
               detection_A.sin_vcs_az,
               sigma_level_squared);

            const float32_t det_B_plausibility = Calculate_Single_Det_Plausibility_For_VH(
               vel_hyp,
               vel_hyp_cov,
               detection_B.range_rate_comp,
               rr_var,
               detection_B.cos_vcs_az,
               detection_B.sin_vcs_az,
               sigma_level_squared);

            single_vel_slot.f_valid = true;
            single_vel_slot.f_inlier = true;
            single_vel_slot.det_A_idx = detection_A.idx;

            single_vel_slot.det_B_idx = detection_B.idx;
            single_vel_slot.time_since_meas = velocity_timestamp;
            single_vel_slot.vel = vel_hyp;
            Add_Uncertainty_2d(vel_hyp_cov, process_noise_cov, single_vel_slot.vel_cov);
            single_vel_slot.dt = dt;
            single_vel_slot.cov_trace = Trace_Of_2d_Covariance(single_vel_slot.vel_cov);
            single_vel_slot.determinant = determinant;
            single_vel_slot.rr_plausbility = (det_A_plausibility + det_B_plausibility) * 0.5F;
            single_vel_slot.rr_confidence_level = Get_Velocity_Hypothesis_Confidence_Level(calibrations, single_vel_slot.rr_plausbility);
            single_vel_slot.f_rr_confirmed = (single_vel_slot.rr_confidence_level != F360_NEES_CFMI_VEL_RR_CONF_INVALID);

            Calc_NEES_CFMI_Determinants_Pos_Diff(single_vel_slot);

            nees_cfmi_information.vels_num++;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Single_Det_Plausibility_For_VH
   *===========================================================================
   * RETURN VALUE:
   * float32_t plausibility value
   *
   * PARAMETERS:
   * const F360_VCS_Velocity_T& vel_hyp
   * const float32_t (&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   * const float32_t range_rate_comp
   * const float32_t range_rate_comp_var
   * const float32_t cos_azimuth
   * const float32_t sin_azimuth
   * const float32_t sigma_squered_thr
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Estimate plausibility of single detections associated to velocity hypothesis
   * based on position difference. This plausibility gives information how probable
   * is that velocity hypothesis is ok with assumption that range rate is not corrupted
   * (wheel spin or clutter)
   * Overall plausibility is mean of plausibility based on two components (as mean):
   * - Direct NEES - NEES value based on constant range rate covariance
   * - Consistent NEES - NEES value with Velocity Hypothesis uncertainty propagation on
   *   range rate.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calculate_Single_Det_Plausibility_For_VH(
      const F360_VCS_Velocity_T& vel_hyp,
      const float32_t(&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t range_rate_comp,
      const float32_t range_rate_comp_var,
      const float32_t cos_azimuth,
      const float32_t sin_azimuth,
      const float32_t sigma_squered_thr)
   {
      float32_t plausibility;
      if (range_rate_comp_var > F360_MIN_DENOMINATOR)
      {
         // Propagation of VH uncertainty on RR.
         const float32_t range_rate_vh_var = Vel_Cov_2_Range_Rate_Var(vel_hyp_cov, cos_azimuth, sin_azimuth);

         const float32_t  estimated_range_rate_error = vel_hyp.longitudinal * cos_azimuth
            + vel_hyp.lateral * sin_azimuth
            - range_rate_comp;

         const float32_t estimated_range_rate_error_squared = (estimated_range_rate_error * estimated_range_rate_error);

         const float32_t nees_direct = estimated_range_rate_error_squared / range_rate_comp_var;
         const float32_t nees_consistent = estimated_range_rate_error_squared / (range_rate_comp_var + range_rate_vh_var);

         const float32_t plausibility_direct = F360_Bisquare_Weight(nees_direct, sigma_squered_thr);
         const float32_t plausibility_consistent = F360_Bisquare_Weight(nees_consistent, sigma_squered_thr);
         plausibility = (plausibility_direct + plausibility_consistent) * 0.5F;
      }
      else
      {
         plausibility = F360_MIN_PROBABILITY;
      }
      return plausibility;
   }

   /*===========================================================================*\
   * FUNCTION: Get_Velocity_Hypothesis_Confidence_Level
   *===========================================================================
   * RETURN VALUE:
   * F360_NEES_CFMI_Vel_RR_Conf_T confidence level
   *
   * PARAMETERS:
   *
   *   const F360_Calibrations_T& calibrations,
   *   const float32_t plausibility,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Get enumerated confidence level based on plausibility measure
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static F360_NEES_CFMI_Vel_RR_Conf_T Get_Velocity_Hypothesis_Confidence_Level(
      const F360_Calibrations_T& calibrations,
      const float32_t plausibility)
   {
      F360_NEES_CFMI_Vel_RR_Conf_T confidence;

      if (plausibility < calibrations.k_nees_cfmi_min_rr_plaus_thr_low_conf)
      {
         confidence = F360_NEES_CFMI_VEL_RR_CONF_INVALID;
      }
      else if (plausibility < calibrations.k_nees_cfmi_min_rr_plaus_thr_med_conf)
      {
         confidence = F360_NEES_CFMI_VEL_RR_CONF_LOW;
      }
      else if (plausibility < calibrations.k_nees_cfmi_min_rr_plaus_thr_high_conf)
      {
         confidence = F360_NEES_CFMI_VEL_RR_CONF_MEDIUM;
      }
      else
      {
         confidence = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
      }
      return confidence;
   }

}
