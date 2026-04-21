/*===================================================================================*\
* FILE: f360_msmt_update_obj_trks_cca_moveable.cpp
*====================================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains Msmt_Update_ObjTrks_CCA_Moveable() function implementation
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#include "f360_msmt_update_obj_trks_cca_moveable.h"
#include "f360_msmt_update_support_functions_cca_moveable.h"
#include "f360_norm_heading_angle.h"
#include "f360_get_wall_time.h"
#include "f360_pseudo_msmt.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_math.h"
#include "f360_math_func.h"
#include "f360_handle_spd_and_acc_when_stopping.h"

namespace f360_variant_A
{
  /*===========================================================================*\
   * FUNCTION: Msmt_Update_Obj_Trks_CCA_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T & host - host property structure
   * const F360_Globals_T & global - global values structure
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS] - detection properties array
   * const F360_Calibrations_T & calib - calibration structure
   * F360_Object_Track_T & object_track - single object track
   * F360_TRKR_TIMING_INFO_T & timing_info - timing structure
   * uint32_t (&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK] - array of selected detections indexes
   * uint32_t selected_dets_num - number of selected detections
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function does KF measurement update of a CCA object.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Msmt_Update_Obj_Trks_CCA_Moveable(
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Calibrations_T & calib,
      const uint32_t (&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t selected_dets_num,
      F360_Object_Track_T& object_track,
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      const float32_t start_time = get_wall_time();

      if (selected_dets_num > 0U)
      {
         const uint32_t non_rr_pseudo_msmts_num = 2U;
         const uint32_t total_pseudo_msmts_num = selected_dets_num + non_rr_pseudo_msmts_num;

         const float32_t x_posn = object_track.vcs_position.x;
         const float32_t y_posn = object_track.vcs_position.y;
         const float32_t x_vel = object_track.vcs_velocity.longitudinal;
         const float32_t y_vel = object_track.vcs_velocity.lateral;
         const float32_t x_accel = object_track.vcs_accel.longitudinal;
         const float32_t y_accel = object_track.vcs_accel.lateral;

         // Initialize state vector
         float32_t state[STATE_DIMENSION];
         state[F360_TRK_FLTR_CCA_STATE_X] = x_posn;
         state[F360_TRK_FLTR_CCA_STATE_VX] = x_vel;
         state[F360_TRK_FLTR_CCA_STATE_AX] = x_accel;
         state[F360_TRK_FLTR_CCA_STATE_Y] = y_posn;
         state[F360_TRK_FLTR_CCA_STATE_VY] = y_vel;
         state[F360_TRK_FLTR_CCA_STATE_AY] = y_accel;

         float32_t z_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT];
         float32_t zhat_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT];
         float32_t h_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] = {};
         float32_t r_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] = {};

         // Position measurement
         z_mat[0] = object_track.pseudo_vcs_position.x;
         z_mat[1] = object_track.pseudo_vcs_position.y;

         zhat_mat[0] = x_posn;
         zhat_mat[1] = y_posn;

         h_mat[0][F360_TRK_FLTR_CCA_STATE_X] = 1.0F;
         h_mat[1][F360_TRK_FLTR_CCA_STATE_Y] = 1.0F;

         r_mat[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_X] = object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_X];
         r_mat[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_Y] = object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_Y];
         r_mat[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_X] = object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_X];
         r_mat[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_Y] = object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_Y];

         // Range rate measurements
         const float32_t normalized_rr_variance = static_cast<float32_t>(selected_dets_num) * calib.k_ref_msmt_cov_cca;
         for (uint32_t loop_index_j = 0U; loop_index_j < selected_dets_num; loop_index_j++)
         {
            const uint32_t index = non_rr_pseudo_msmts_num + loop_index_j;

            const float32_t cos_vcs_az = raw_detection_list.detections[selected_dets_idx[loop_index_j]].processed.cos_vcs_az;
            const float32_t sin_vcs_az = raw_detection_list.detections[selected_dets_idx[loop_index_j]].processed.sin_vcs_az;

            z_mat[index] = det_props[selected_dets_idx[loop_index_j]].range_rate_compensated;

            zhat_mat[index] = (x_vel * cos_vcs_az) + (y_vel * sin_vcs_az);

            h_mat[index][F360_TRK_FLTR_CCA_STATE_VX] = cos_vcs_az;
            h_mat[index][F360_TRK_FLTR_CCA_STATE_VY] = sin_vcs_az;

            r_mat[index][index] = normalized_rr_variance;
         }
         

         // Kalman gain matrix K, and innovation covariance matrix S
         float32_t k_mat[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT];
         float32_t s_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT];
         Kalman_Gain_Update_CCA_Moveable(h_mat, object_track.errcov, r_mat, total_pseudo_msmts_num, calib, object_track.num_updates_since_init, k_mat, s_mat);

         Error_Cov_Update_CCA_Moveable(k_mat, r_mat, h_mat, total_pseudo_msmts_num, object_track.errcov);

         State_Update_CCA_Moveable(z_mat, zhat_mat, total_pseudo_msmts_num, k_mat, state);

         const float32_t delta_pox_x = state[F360_TRK_FLTR_CCA_STATE_X] - object_track.vcs_position.x;
         const float32_t delta_pos_y = state[F360_TRK_FLTR_CCA_STATE_Y] - object_track.vcs_position.y;
         object_track.vcs_position.x = state[F360_TRK_FLTR_CCA_STATE_X];
         object_track.vcs_position.y = state[F360_TRK_FLTR_CCA_STATE_Y];
         object_track.bbox.Translate(delta_pox_x, delta_pos_y);

         const F360_VCS_Velocity_T prev_vel = object_track.vcs_velocity;
         object_track.vcs_velocity.longitudinal = state[F360_TRK_FLTR_CCA_STATE_VX];
         object_track.vcs_accel.longitudinal = state[F360_TRK_FLTR_CCA_STATE_AX];
         object_track.vcs_velocity.lateral = state[F360_TRK_FLTR_CCA_STATE_VY];
         object_track.vcs_accel.lateral = state[F360_TRK_FLTR_CCA_STATE_AY];

         // Compute object speed
         object_track.speed = F360_Get_Hypotenuse(object_track.vcs_velocity.lateral, object_track.vcs_velocity.longitudinal);

         // Compute tangential acceleration for object
         object_track.tang_accel = object_track.vcs_accel.longitudinal * object_track.vcs_heading.Cos() + object_track.vcs_accel.lateral * object_track.vcs_heading.Sin();

         // Handle when object stops from hard break
         Handle_Spd_And_Acc_When_Stopping_CCA(prev_vel, calib, object_track);

         // Update object heading, bbox pointing, heading rate and curvature
         (void)object_track.vcs_heading.Value(F360_Atan2f(object_track.vcs_velocity.lateral, object_track.vcs_velocity.longitudinal)).Normalize(); // Heading corresponds to the direction of the velocity vector (which is allowed to rotate even when object stands still)

         if (object_track.f_moving && (object_track.speed > calib.k_cca_min_speed_to_update_pnt)) // Note: For CCA speed is always positive so we don't need to take abs
         {
            Measurement_Update_Pointing_Heading_Rate_CCA(calib, object_track);

            // Compute object bbox curvature
            object_track.curvature = object_track.heading_rate / object_track.speed;
         }
         else
         {
            // BBox of stationary objects should not rotate Therefore: 
            //    - Freeze object pointing (i.e. don't update)
            //    - Set heading rate and curvature to 0
            object_track.heading_rate = 0.0F;
            object_track.curvature = 0.0F;
         }
      }

      const float32_t run_time = get_wall_time() - start_time;
      timing_info.msmt_update_obj_trks_cca_moveable += run_time;
      timing_info.msmt_update_obj_trks_cca += run_time;
   }
}
