/*===================================================================================*\
* FILE: f360_nees_cfmi_hypothesis_dominant_velocity.cpp
*====================================================================================
* Copyright - 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains definitions of functions: Get_NEES_CFMI_Information_Dominant_Velocity() and Check_Dominant_Velocity_Preconditions()..
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_nees_cfmi_hypothesis_dominant_velocity.h"
#include "f360_uncertainty_propagation.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Get_NEES_CFMI_Information_Dominant_Velocity
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_Detection_Hist_Time_Slots_T& det_hist_time_slots,
   * F360_NEES_CFMI_Information_T& nees_cfmi_information
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate Dominant Velocity: position difference between the newest and
   * the oldest detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_NEES_CFMI_Information_Dominant_Velocity(
      const F360_Calibrations_T& calibrations,
      const F360_Detection_Time_Slots_T& det_hist_time_slots,
      F360_NEES_CFMI_Information_T& nees_cfmi_information)
   {
      const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);
      if (preconditions_valid)
      {
         // Position of oldest dets
         Point position_old_dets = {};
         position_old_dets.x = 0.0F;
         position_old_dets.y = 0.0F;
         float32_t timestamp_old_dets = 0.0F;
         float32_t position_cov_old_dets[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};

         const uint32_t last_valid_slot_idx = det_hist_time_slots.number_of_active_slots - 1U;
         const F360_Detection_Time_Slot_T &time_slot_oldest_dets = det_hist_time_slots.time_since_meas_slots[last_valid_slot_idx];
         const uint32_t num_dets_slot_old_dets = time_slot_oldest_dets.number_of_active_detections;

         for (uint32_t index_2 = 0U; index_2 < num_dets_slot_old_dets; index_2++)
         {
            const uint32_t det_B_index = time_slot_oldest_dets.hist_dets_idx[index_2];
            if (det_B_index < F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT)
            {
               const F360_NEES_CFMI_Detection_T &single_old_det = nees_cfmi_information.detections[det_B_index];

               position_old_dets.x += single_old_det.pos.x;
               position_old_dets.y += single_old_det.pos.y;
               timestamp_old_dets += single_old_det.time_since_meas;
               Add_Into_Uncertainty_2d(single_old_det.pos_cov, position_cov_old_dets);
            }
         }
         const float32_t num_dets_slot_old_dets_invers = 1.0F / static_cast<float32_t>(num_dets_slot_old_dets);
         // Calculate mean values
         position_old_dets.x *= num_dets_slot_old_dets_invers;
         position_old_dets.y *= num_dets_slot_old_dets_invers;
         timestamp_old_dets *= num_dets_slot_old_dets_invers;
         Constant_Uncertainty_Propagation_2d(num_dets_slot_old_dets_invers, position_cov_old_dets);

         // Extend covariance by extended object model uncertainty
         float32_t extended_object_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
         Init_2d_Covariance_By_Std(calibrations.k_nees_cfmi_extended_object_pos_std, extended_object_cov);
         Add_Into_Uncertainty_2d(extended_object_cov, position_cov_old_dets);

         // Calculate relevant information
         const float32_t dt = timestamp_old_dets - nees_cfmi_information.center_time_since_meas;
         F360_VCS_Velocity_T vel_hyp = {};
         bool f_successful_estimation = Pos_Diff_Vel_Estimate(nees_cfmi_information.pos_center, position_old_dets, dt, vel_hyp);

         float32_t vel_hyp_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
         if (f_successful_estimation)
         {
            f_successful_estimation = Pos_Diff_Cov_Estimate(nees_cfmi_information.pos_center_cov, position_cov_old_dets, dt, vel_hyp_cov);
         }
         const float32_t determinant = F360_2d_Matrix_Determinant(vel_hyp_cov);

         // Populate data
         const bool estimate_is_ok = (f_successful_estimation && (determinant > F360_MIN_DENOMINATOR));
         if (estimate_is_ok)
         {
            const float32_t velocity_timestamp = (nees_cfmi_information.center_time_since_meas + timestamp_old_dets) * 0.5F; // Velocity calculated by position difference is estimated in the middle of time-stamp
            const float32_t timestamp_diff = velocity_timestamp - nees_cfmi_information.min_time_since_meas;
            const float32_t process_noise_std = timestamp_diff * calibrations.k_nees_cfmi_process_noise_acc_std;

            float32_t process_noise_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION] = {};
            Init_2d_Covariance_By_Std(process_noise_std, process_noise_cov);

            F360_NEES_CFMI_Pos_Diff_Velocity_T &dominant_velocity = nees_cfmi_information.dominant_velocity;
            // Detection indexes not populated
            dominant_velocity.f_valid = true;
            dominant_velocity.f_inlier = true;
            dominant_velocity.det_A_idx.f_historical = false;
            dominant_velocity.det_B_idx.f_historical = true;
            dominant_velocity.time_since_meas = velocity_timestamp;
            dominant_velocity.vel = vel_hyp;
            Add_Uncertainty_2d(vel_hyp_cov, process_noise_cov, dominant_velocity.vel_cov);
            dominant_velocity.dt = dt;
            dominant_velocity.cov_trace = Trace_Of_2d_Covariance(dominant_velocity.vel_cov);
            dominant_velocity.determinant = determinant;
            Calc_NEES_CFMI_Determinants_Pos_Diff(dominant_velocity);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_Dominant_Velocity_Preconditions
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T &nees_cfmi_information,
   * const F360_Detection_Time_Slots_T &det_hist_time_slots
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check preconditions for Get_NEES_CFMI_Information_Dominant_Velocity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   bool Check_Dominant_Velocity_Preconditions(const F360_Detection_Time_Slots_T &det_hist_time_slots)
   {
      const bool number_of_active_slots_is_valid = (1U < det_hist_time_slots.number_of_active_slots) &&
         (det_hist_time_slots.number_of_active_slots <= F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS);

      bool preconditions_valid;
      if (number_of_active_slots_is_valid)
      {
         const uint32_t last_valid_slot_idx = det_hist_time_slots.number_of_active_slots - 1U;
         const F360_Detection_Time_Slot_T &time_slot_oldest_dets = det_hist_time_slots.time_since_meas_slots[last_valid_slot_idx];
         const bool number_of_oldest_dets_is_ok = (0U < time_slot_oldest_dets.number_of_active_detections) &&
            (time_slot_oldest_dets.number_of_active_detections <= F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT);

         preconditions_valid = number_of_oldest_dets_is_ok;
      }
      else
      {
         preconditions_valid = false;
      }

      return preconditions_valid;
   }
}
