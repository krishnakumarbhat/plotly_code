/*===========================================================================*\
* FILE: f360_msmt_update_support_functions_ctca.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function related to Kalman measurement update step of CTCA objects
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_msmt_update_support_functions_ctca.h"
#include "f360_msmt_update_support_functions_common.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_pseudo_msmt.h"
#include "f360_det_cross_covariances.h"
#include "f360_math_func.h"
#include "f360_LinearSolvers.h"
#include "f360_norm_heading_angle.h"
#include "f360_reference_point_support_functions.h"
#include "f360_reference_point_support_functions.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_reference_point_support_functions.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Fill_H_Z_Mat_Single_Row_With_RR_Info_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track
   * const F360_Detection_Props_T & det_prop
   * const rspp_variant_A::RSPP_Detection_T & det
   * const uint32_t row_idx
   * float32_t (&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION]
   * float32_t (&z_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT]
   * float32_t (&zhat_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT]
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function fills a row of the H matrix, the Z and Z_hat vectors with range rate information.
   * The H matrix the measurement model linearized at the current state.
   * The Z vector is the measurement vector and the Z_hat vector is the expected measurement vector.
   * Z_hat is derived using the non-linearized measurement model h(x).
   * (Z - Z_hat) is the innovation, later used in the kalman filter.
   *
   \*===========================================================================*/
   void Fill_H_Z_Mat_Single_Row_With_RR_Info_CTCA(
      const F360_Object_Track_T& object_track,
      const F360_Detection_Props_T& det_prop,
      const rspp_variant_A::RSPP_Detection_T& det,
      const uint32_t row_idx,
      float32_t(&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      float32_t(&z_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT],
      float32_t(&zhat_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT]
   )
   {
      const float32_t det_cos_vcs_az = det.processed.cos_vcs_az;
      const float32_t det_sin_vcs_az = det.processed.sin_vcs_az;

      const float32_t trk_cos_vcs_head = object_track.vcs_heading.Cos();
      const float32_t trk_sin_vcs_head = object_track.vcs_heading.Sin();
      const float32_t trk_spd = object_track.speed;
      const float32_t trk_curv = object_track.curvature;

      float32_t tcs_vec_from_ref_pnt_to_center_rear[2];
      Get_TCS_Vec_From_Obj_Center_Rear_to_Ref_Point(object_track, tcs_vec_from_ref_pnt_to_center_rear);
      tcs_vec_from_ref_pnt_to_center_rear[0] = -tcs_vec_from_ref_pnt_to_center_rear[0];
      tcs_vec_from_ref_pnt_to_center_rear[1] = -tcs_vec_from_ref_pnt_to_center_rear[1];
      float32_t vcs_x_center_rear = 0.0F;
      float32_t vcs_y_center_rear = 0.0F;
      Convert_TCS_Posn_To_VCS_Posn(tcs_vec_from_ref_pnt_to_center_rear[0], tcs_vec_from_ref_pnt_to_center_rear[1], object_track.vcs_position.x, object_track.vcs_position.y, object_track.bbox.Get_Orientation(), vcs_x_center_rear, vcs_y_center_rear); // Note: It is intended to use object vcs posiiton rather than object bbox center here since the vector tcs_vec_from_ref_pnt_to_center_rear is defined from object reference point
      const float32_t dx = det_prop.vcs_position.x - vcs_x_center_rear;
      const float32_t dy = det_prop.vcs_position.y - vcs_y_center_rear;

      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_X] = -det_sin_vcs_az * trk_curv * trk_spd;
      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_Y] = det_cos_vcs_az * trk_curv * trk_spd;
      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_H] = trk_spd * (trk_cos_vcs_head * (det_sin_vcs_az + trk_curv * (tcs_vec_from_ref_pnt_to_center_rear[0] * det_cos_vcs_az + tcs_vec_from_ref_pnt_to_center_rear[1] * det_sin_vcs_az)) - trk_sin_vcs_head * (det_cos_vcs_az + trk_curv * (tcs_vec_from_ref_pnt_to_center_rear[1] * det_cos_vcs_az - tcs_vec_from_ref_pnt_to_center_rear[0] * det_sin_vcs_az)));
      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_C] = trk_spd * (dx * det_sin_vcs_az - dy * det_cos_vcs_az);
      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_S] = (trk_cos_vcs_head - trk_curv * dy) * det_cos_vcs_az + (trk_sin_vcs_head + trk_curv * dx) * det_sin_vcs_az;
      h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_A] = 0.0F;

      z_mat[row_idx] = det_prop.range_rate_compensated;

      // Project object velocity vectors in point of detection (compensated for object yaw rate) onto detection radial direction to get expected measurement.
      zhat_mat[row_idx] = trk_spd * h_mat[row_idx][F360_TRK_FLTR_CTCA_STATE_S];
   }

   /*===========================================================================*\
   * FUNCTION: Filling_Msmt_Cov_By_H_RR_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib - calibration structure
   * const uint32_t selected_dets_num - number of detections that are chosen to be used for the measurement update
   * float32_t (&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] - measurement covariance matrix
   *
   * EXTERNAL REFERENCES:
   * [1] SCM_Msmt_Upd_CTCA_Fill_Rmat_comp_rngrate.pptx. Document can be found
   *     as an attachment to the DFD-65 Jira ticket,
   *     http://jiraprod1.delphiauto.net:8080/browse/DFD-65
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function updates comp range rate measurements covariance matrix for CTCA objects based on the derivation in [1] as well as
   * computing a gain. This gain is based on the object's stored gain used in the previous scan as well as the smallest comp range rate variance
   * computed for this iteration and object.
   *
   * PRECONDITIONS:
   * r_mat must be initialized to zero before call to function
   * selected_dets_num must be larger than 0, i.e. there must be available range rate measuremnts to be used in the Kalman filter update
   *
   \*===========================================================================*/
   void Filling_Msmt_Cov_By_H_RR_CTCA(
      const F360_Calibrations_T& calib,
      const uint32_t selected_dets_num,
      float32_t(&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT])
   {
      constexpr uint32_t first_rr_comp_idx_in_meas_cov = static_cast<uint32_t>(MSMT_UPDATE_MAX_NUM_OF_NON_RR_MSMT);

      for (uint32_t i = 0U; i < selected_dets_num; i++)
      {
         const uint32_t index = first_rr_comp_idx_in_meas_cov + i;
         r_mat[index][index] = calib.k_ref_msmt_cov_ctca;
      }
   }


   /*===========================================================================*\
   * FUNCTION: Calculate_RR_Uncertainty_And_Find_Min_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track - single object track
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS] - detection property data structure
   * const uint32_t (&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK], - array with indexes of detections that are chosen to be used for the measurement update
   * const uint32_t selected_dets_num - number of detections that are chosen to be used for the measurement update
   * const uint32_t first_rr_comp_idx_in_meas_cov - Offset to first range rate slot in R matrix
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * float32_t (&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] - measurement covariance matrix
   * float32_t & min_rr_uncertainty - Min calculated range rate uncertainty for detections that are chosen to be used for the measurement update
   *
   * EXTERNAL REFERENCES:
   * [1] SCM_Msmt_Upd_CTCA_Fill_Rmat_comp_rngrate.pptx. Document can be found
   *     as an attachment to the DFD-65 Jira ticket,
   *     http://jiraprod1.delphiauto.net:8080/browse/DFD-65
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function updates comp range rate measurements covariance matrix for CTCA objects based on the derivation in [1].
   * The lowest uncertainty value for the chosen detections is also found.
   *
   * PRECONDITIONS:
   * min range rate uncertainty is set to a large number.
   *
   \*===========================================================================*/
   void Calculate_RR_Uncertainty_And_Find_Min_CTCA(
      const F360_Object_Track_T& object_track,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const uint32_t(&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t selected_dets_num,
      const uint32_t first_rr_comp_idx_in_meas_cov,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      float32_t(&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      float32_t& min_rr_uncertainty)
   {
      // Loop over all detections that has been selected for the measurement update and
      // calculate the variance for compensated range rate measurements
      for (uint32_t loop_index = 0U; loop_index < selected_dets_num; loop_index++)
      {
         // Define current detection index
         const uint32_t curr_det_idx = selected_dets_idx[loop_index];

         // Extract detection properties
         const float32_t det_vcs_posx = det_props[curr_det_idx].vcs_position.x;
         const float32_t det_vcs_posy = det_props[curr_det_idx].vcs_position.y;
         const float32_t det_cos_vcs_az = raw_detection_list.detections[curr_det_idx].processed.cos_vcs_az;
         const float32_t det_sin_vcs_az = raw_detection_list.detections[curr_det_idx].processed.sin_vcs_az;
         const float32_t det_var_vcs_posx = raw_detection_list.detections[curr_det_idx].processed.vcs_position_cov_scm[F360_2D_IDX_X][F360_2D_IDX_X];
         const float32_t det_var_vcs_posy = raw_detection_list.detections[curr_det_idx].processed.vcs_position_cov_scm[F360_2D_IDX_Y][F360_2D_IDX_Y];
         const float32_t det_cov_vcs_posx_vcs_posy = raw_detection_list.detections[curr_det_idx].processed.vcs_position_cov_scm[F360_2D_IDX_X][F360_2D_IDX_Y];
         float32_t det_var_vcs_az = raw_detection_list.detections[curr_det_idx].processed.std_vcs_az_scm;
         det_var_vcs_az *= det_var_vcs_az; // Compute variance from STD
         float32_t det_var_rr_comp = raw_detection_list.detections[curr_det_idx].processed.std_range_rate_compensated_scm;
         det_var_rr_comp *= det_var_rr_comp; // Compute variance from STD
         const float32_t det_cov_vcs_az_vcs_posx = raw_detection_list.detections[curr_det_idx].processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_X_VCS_AZ];
         const float32_t det_cov_vcs_posx_rr_comp = raw_detection_list.detections[curr_det_idx].processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_X_VCS_COMP_RANGE_RATE];
         const float32_t det_cov_vcs_az_vcs_posy = raw_detection_list.detections[curr_det_idx].processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_Y_VCS_AZ];
         const float32_t det_cov_vcs_posy_rr_comp = raw_detection_list.detections[curr_det_idx].processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_Y_VCS_COMP_RANGE_RATE];
         const float32_t det_cov_vcs_az_rr_comp = raw_detection_list.detections[curr_det_idx].processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_AZ_VCS_COMP_RANGE_RATE];

         /* Computing some help variables accoring to reference [1] in function header.
         The measurement equation for compensated range rate used in the measurement update is:
         y_rr_comp =  obj_speed*(det_cos_az*(obj_cos_hdg - obj_curv*(det_posy - obj_posy)) + det_sin_az*(obj_sin_hdg + obj_curv*(det_posx - obj_posx))) + noise_rr_comp
         The below help variables correspond to derivatives of this function w.r.t the
         detection properties that are involved in the expression, i.e. w.r.t det_az,
         det_xpos and det_ypos respectively */
         const float32_t d_rr_comp_d_det_vcs_az = object_track.speed * (-det_sin_vcs_az * (object_track.vcs_heading.Cos() - object_track.curvature * (det_vcs_posy - object_track.vcs_position.y)) + det_cos_vcs_az * (object_track.vcs_heading.Sin() + object_track.curvature * (det_vcs_posx - object_track.vcs_position.x))); // Denoted with f1 in reference [1]
         const float32_t d_rr_comp_d_det_vcs_xpos = object_track.speed * object_track.curvature * det_sin_vcs_az; // Denoted with f2 in reference [1]
         const float32_t d_rr_comp_d_det_vcs_ypos = -object_track.speed * object_track.curvature * det_cos_vcs_az; // Denoted with f3 in reference [1]

         // Computing all the individual terms that the variance is made up from according to reference [1] in function header.
         const float32_t term_az = d_rr_comp_d_det_vcs_az * d_rr_comp_d_det_vcs_az * det_var_vcs_az;
         const float32_t term_posx = d_rr_comp_d_det_vcs_xpos * d_rr_comp_d_det_vcs_xpos * det_var_vcs_posx;
         const float32_t term_posy = d_rr_comp_d_det_vcs_ypos * d_rr_comp_d_det_vcs_ypos * det_var_vcs_posy;
         const float32_t term_az_posx = 2.0F * d_rr_comp_d_det_vcs_az * d_rr_comp_d_det_vcs_xpos * det_cov_vcs_az_vcs_posx;
         const float32_t term_az_posy = 2.0F * d_rr_comp_d_det_vcs_az * d_rr_comp_d_det_vcs_ypos * det_cov_vcs_az_vcs_posy;
         const float32_t term_posx_posy = 2.0F * d_rr_comp_d_det_vcs_xpos * d_rr_comp_d_det_vcs_ypos * det_cov_vcs_posx_vcs_posy;
         const float32_t term_az_rr_comp = 2.0F * d_rr_comp_d_det_vcs_az * det_cov_vcs_az_rr_comp;
         const float32_t term_posx_rr_comp = 2.0F * d_rr_comp_d_det_vcs_xpos * det_cov_vcs_posx_rr_comp;
         const float32_t term_posy_rr_comp = 2.0F * d_rr_comp_d_det_vcs_ypos * det_cov_vcs_posy_rr_comp;
         const float32_t term_rr_comp = det_var_rr_comp;

         // Compute value to fill measurement covariance matrix with
         const float32_t rr_uncertainty = term_az + term_posx + term_posy + term_az_posx + term_az_posy
            + term_posx_posy + term_az_rr_comp + term_posx_rr_comp + term_posy_rr_comp + term_rr_comp;
         min_rr_uncertainty = std::min(rr_uncertainty, min_rr_uncertainty);

         // Fill the measurement covariance matrix
         const uint32_t r_mat_idx = first_rr_comp_idx_in_meas_cov + loop_index;
         r_mat[r_mat_idx][r_mat_idx] = rr_uncertainty;
      }
   }


   /*===========================================================================*\
   * FUNCTION: Filling_Msmt_Cov_By_Pos_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track - single object track
   * float32_t (&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] - measurement covariance matrix
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function updates position measurement covariance matrix for CTCA objects.
   *
   \*===========================================================================*/
   void Filling_Msmt_Cov_By_Pos_CTCA(
      const F360_Object_Track_T& object_track,
      float32_t(&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT]
   )
   {
      r_mat[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_X] = object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_X];
      r_mat[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_Y] = object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_Y];
      r_mat[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_X] = object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_X];
      r_mat[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_Y] = object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_Y];
   }

   /*===========================================================================*\
   * FUNCTION: Kalman_Gain_Update_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION]              - observation model matrix
   * const float32_t (&hT_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT]             - transposed observation model matrix
   * const float32_t (&p_mat)[STATE_DIMENSION][STATE_DIMENSION]                          - state covariance
   * const float32_t (&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT]  - pseudo-measurement covariance of measurement uncertainty
   * const uint32_t nr_total_msnmts                                                      - number of valid measurements
   * float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT]                    - kalman gain
   * float32_t (&s_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT]        - innovation covaraince
   *
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
   * This function updates the Kalman gain for an object.
   *
   * PRECONDITIONS:
   * r_mat must have the following structure:
   *    r_mat = [* * 0 0 0 0 ....
   *             * * 0 0 0 0 ....
   *             0 0 0 * 0 0 ....
   *             . . . . . .
   *             . . . . . .
   *             . . . . . .]
   * h_mat must have the following structure
   *    h_mat = [1 0 0 0 0 0
   *             0 1 0 0 0 0
   *             * * * * * 0 (this row is repeaded for each present range rate measurement)
   *             . . . . . .
   *             . . . . . .
   *             . . . . . .]
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Kalman_Gain_Update_CTCA(
      const float32_t(&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t(&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const uint32_t nr_total_msnmts,
      float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      float32_t(&s_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT]
   )
   {

      // KF equation: H * P
      float32_t temp_hp_mat[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION];
      Hmat_Times_Pmat_CTCA(h_mat, p_mat, nr_total_msnmts, temp_hp_mat);

      // KF equation: H * P * H' + R
      // First fill s_mat with H*P*H' only
      HPmat_Times_Hmat_Transpose_CTCA(temp_hp_mat, h_mat, nr_total_msnmts, s_mat);
      // Now add R into s_mat
      s_mat[0][0] += r_mat[0][0];
      s_mat[0][1] += r_mat[0][1];
      s_mat[1][0] = s_mat[0][1];
      s_mat[1][1] += r_mat[1][1];
      for (uint32_t loop_index_k = 2U; loop_index_k < nr_total_msnmts; loop_index_k++)
      {
         s_mat[loop_index_k][loop_index_k] += r_mat[loop_index_k][loop_index_k];
      }

      //KF equation: P*H'*inv(S) = (H*P)'*inv(S)
      Matrix_Division(s_mat, temp_hp_mat, nr_total_msnmts, STATE_DIMENSION, k_mat);
   }

   /*===========================================================================*\
   * FUNCTION: Error_Cov_Update_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT]              - kalman gain
   * const float32_t (&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT]  - covariance of measurement uncertainty
   * const float32_t (&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION]              - observation model matrix
   * const uint32_t nr_total_msnmts                                                   - number of valid measurements                                           - a boolean indicating if there is a heading measurement present
   * float32_t (&p_mat)[STATE_DIMENSION][STATE_DIMENSION]                                - state error covariance matrix, updated in this fuction
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
   * This function updates an object's error covariance matrix.
   *
   * PRECONDITIONS:
   * r_mat must have the following structure:
   *    r_mat = [* * 0 0 0 0 ....
   *             * * 0 0 0 0 ....
   *             0 0 * 0 0 0 ....
   *             . . . . . .
   *             . . . . . .
   *             . . . . . .]
   * h_mat must have the following structure
   *    h_mat = [1 0 0 0 0 0
   *             0 1 0 0 0 0
   *             * * * * * 0 (this row is repeaded for each present range rate measurement)
   *             . . . . . .
   *             . . . . . .
   *             . . . . . .]
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Error_Cov_Update_CTCA(
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const float32_t(&r_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const float32_t(&h_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const uint32_t nr_total_msnmts,
      float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      /*Measurement update
      Use the Joseph form of covariance update to avoid numerical instability
      P_update = (I - KH) * P * (I - KH)' + KRK'
      */

      // KF equation: I - (K * H)
      float32_t temp2_mat[STATE_DIMENSION][STATE_DIMENSION];
      // First fill with -K*H
      Negative_Kmat_Times_Hmat_CTCA(k_mat, h_mat, nr_total_msnmts, temp2_mat);
      // Then add identity matrix
      for (uint32_t loop_index = 0U; loop_index < STATE_DIMENSION; loop_index ++)
      {
         temp2_mat[loop_index][loop_index] += 1.0F;
      }

      // KF equation: (I - (K * H)) * P
      float32_t temp3_mat[STATE_DIMENSION][STATE_DIMENSION];
      F360_matmul_6x6_6x6(temp2_mat, p_mat, temp3_mat);

      // KF equation: ((I - (K * H)) * P) * (I - K * H)'
      float32_t temp4_mat[STATE_DIMENSION][STATE_DIMENSION];
      F360_matmul_6x6_6x6T_symmetric_matrix(temp3_mat, temp2_mat, temp4_mat);

      // KF equation: K * R
      float32_t temp0_mat[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT];
      Kmat_Times_Rmat_CTCA(k_mat, r_mat, nr_total_msnmts, temp0_mat);

      // KF equation: K * R * K'
      float32_t temp5_mat[STATE_DIMENSION][STATE_DIMENSION];
      F360_Matmul_MxN_PxN_Transpose_symmetric_matrix(temp0_mat, k_mat, temp5_mat, STATE_DIMENSION, nr_total_msnmts, STATE_DIMENSION);

      //  KF equation: ((I - (K * H)) * P) * (I - K * H)'  +  K * R * K'
      F360_matadd_6x6_symmetric_matrix(temp4_mat, temp5_mat, p_mat);
   }

   /*===========================================================================*\
   * FUNCTION: State_Update_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&z_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT]             - pseudo measurement vector
   * const float32_t (&zhat_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT]          - predicted measurement (previous state)
   * const uint32_t nr_total_msnmts                                 - number of valid measurements
   * float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT]  - kalman gain
   * float32_t (&state)[STATE_DIMENSION]                               - object's current state vector
   *
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
   * This function updates the current state of an object.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void State_Update_CTCA(
      const float32_t(&z_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const float32_t(&zhat_mat)[MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const uint32_t nr_total_msnmts,
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      float32_t(&state)[STATE_DIMENSION])
   {

      // The Kalman filter equation implemented below is x_updated = x_predicted + K*(z - z_pred)
      for (uint32_t row = 0U; row < STATE_DIMENSION; row++)
      {
         float32_t state_increment = 0.0F;
         for (uint32_t col = 0U; col < nr_total_msnmts; col++)
         {
            state_increment += k_mat[row][col] * (z_mat[col] - zhat_mat[col]);
         }
         state[row] += state_increment;
      }
   }


   /*===========================================================================*\
   * FUNCTION: Hmat_Times_Pmat_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CTCA Kalman filter update.
   * const float32_t(&Pmat)[STATE_DIMENSION][STATE_DIMENSION] - the P matrix (state error covariance) in the CTCA Kalman filter update
   * const uint32_t num_measurements - total number of measurements
   * float32_t(&HPmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] - the resulting matrix multiplcation of Hmat * Pmat
   *
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
   * This function computes the matrix multiplication of Hmat * Pmat in the CTCA KF measurement update step.
   * It utilizes that Hmat has the following structure in order to optimize the computations with respect
   * to run time:
   * Hmat = [1 0 0 0 0 0;
   *         0 1 0 0 0 0;
   *         * * * * * 0; (this row is repeated for each available range rate measurement)
   *         ....]
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Hmat_Times_Pmat_CTCA(
      const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const float32_t(&Pmat)[STATE_DIMENSION][STATE_DIMENSION],
      const uint32_t num_measurements,
      float32_t(&HPmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION])
   {
      // Compute first two rows of result mat as [HP]_0j = sum_k h_0k*p_kj where we utilize that h_00 = 1 and h_11 = 1 and h_0k = 0 for all k != 0 and h_1k = 0 for all k != 1
      for (uint32_t row_idx = 0U; row_idx < 2U; row_idx++)
      {
         for (uint32_t col_idx = 0U; col_idx < STATE_DIMENSION; col_idx++)
         {
            HPmat[row_idx][col_idx] = Pmat[row_idx][col_idx];
         }
      }

      constexpr uint32_t first_rr_idx = static_cast<uint32_t>(MSMT_UPDATE_MAX_NUM_OF_NON_RR_MSMT);

      // Compute remaining rows of result mat as [HP]_ij = sum_k h_ik*p_kj where we utilize that h_i5 = 0 for all i => first_rr_idx
      for (uint32_t row_idx = first_rr_idx; row_idx < num_measurements; row_idx++)
      {
         for (uint32_t col_idx = 0U; col_idx < STATE_DIMENSION; col_idx++)
         {
            HPmat[row_idx][col_idx] = Hmat[row_idx][0] * Pmat[0][col_idx] + Hmat[row_idx][1] * Pmat[1][col_idx] + Hmat[row_idx][2] * Pmat[2][col_idx] + Hmat[row_idx][3] * Pmat[3][col_idx] + Hmat[row_idx][4] * Pmat[4][col_idx];
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: HPmat_Times_Hmat_Transpose_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&HPmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] - a matrix corresponding to H * P (linearized measurement model * state error covariance) used in the CTCA Kalman filter update
   * const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CTCA Kalman filter update
   * const uint32_t num_measurements - total number of measurements
   * float32_t(&HPHTmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] - the resulting matrix multiplcation of HPmat* Hmat'
   *
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
   * This function computes the matrix multiplication of HPmat * Hmat' in the CTCA KF measurement update step.
   * It utilizes that Hmat has the following structure in order to optimize the computations with respect
   * to run time:
   * Hmat = [1 0 0 0 0 0;
   *         0 1 0 0 0 0;
   *         * * * * * 0; (this row is repeated for each available range rate measurement)
   *         ....]
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void HPmat_Times_Hmat_Transpose_CTCA(
      const float32_t(&HPmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const uint32_t num_measurements,
      float32_t(&HPHTmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT])
   {
      // Compute first column of result mat as [HPHT]_i0 = sum_k hp_ik*hT_k0 = sum_k hp_ik*h_0k where we utilize that h_00 = 1 and h_0k = 0 for all k != 0
      for (uint32_t row_idx = 0U; row_idx < num_measurements; row_idx++)
      {
         HPHTmat[row_idx][0] = HPmat[row_idx][0];
         HPHTmat[0][row_idx] = HPHTmat[row_idx][0]; // HPHT is symmetric
      }

      // Compute second column of result mat as [res_mat]_i1 = sum_k hp_ik*hT_k1 = sum_k hp_ik*h_1k where we utilize that h_11 = 1 and h_1k = 0 for all k != 1

      for (uint32_t row_idx = 1U; row_idx < num_measurements; row_idx++) // We can start on 1 instead of on 0 since HPHT is symmetric
      {
         HPHTmat[row_idx][1] = HPmat[row_idx][1];
         HPHTmat[1][row_idx] = HPHTmat[row_idx][1];
      }

      constexpr uint32_t first_rr_idx = static_cast<uint32_t>(MSMT_UPDATE_MAX_NUM_OF_NON_RR_MSMT);

      // Compute remaining columns of result mat as [HPNT]_ij = sum_k hp_ik*hT_kj = sum_k hp_ik*h_jk where we utilize that h_i5 = 0 for all i => first_rr_idx
      for (uint32_t row_idx = first_rr_idx; row_idx < num_measurements; row_idx++) // We can start on first_rr_idx instead of on 0 since HPHT is symmetric
      {
         for (uint32_t col_idx = first_rr_idx; col_idx < num_measurements; col_idx++)
         {
            if (row_idx <= col_idx)
            {
               // Compute value
               HPHTmat[row_idx][col_idx] = HPmat[row_idx][0] * Hmat[col_idx][0] + HPmat[row_idx][1] * Hmat[col_idx][1] + HPmat[row_idx][2] * Hmat[col_idx][2] + HPmat[row_idx][3] * Hmat[col_idx][3] + HPmat[row_idx][4] * Hmat[col_idx][4];
            }
            else
            {
               // Copy value to utilize that HPHT is symmetric
               HPHTmat[row_idx][col_idx] = HPHTmat[col_idx][row_idx];
            }
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Negative_Kmat_Times_Hmat_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&Kmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT] - the K matrix (Kalman gain) used in the CTCA Kalman filter update
   * const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CTCA Kalman filter update
   * const uint32_t num_measurements - total number of measurements
   * float32_t(&negKHmat)[STATE_DIMENSION][STATE_DIMENSION] - the resulting matrix multiplcation of -Kmat * Hmat
   *
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
   * This function computes the matrix multiplication of -Kmat * Hmat in the CTCA KF measurement update step.
   * It utilizes that Hmat has the following structure in order to optimize the computations with respect
   * to run time:
   * Hmat = [1 0 0 0 0 0;
   *         0 1 0 0 0 0;
   *         * * * * * 0; (this row is repeated for each available range rate measurement)
   *         ....]
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Negative_Kmat_Times_Hmat_CTCA(
      const float32_t(&Kmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const float32_t(&Hmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][STATE_DIMENSION],
      const uint32_t num_measurements,
      float32_t(&negKHmat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      constexpr uint32_t first_rr_meas_idx = static_cast<uint32_t>(MSMT_UPDATE_MAX_NUM_OF_NON_RR_MSMT);

      // Compute first and second column of result mat as [KH]_i0 = sum_k k_ik*h_k0 where we utilize that h_00 = 1 and h_k0 = 0 for all k != 0 or k <= first_rr_meas_idx and h_11 = 1 and h_k1 = 0 for all k != 1 or k <= first_rr_meas_idx
      for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx++)
      {
         for (uint32_t col_idx = 0U; col_idx < 2U; col_idx++)
         {
            negKHmat[row_idx][col_idx] = -Kmat[row_idx][col_idx];
            for (uint32_t k = first_rr_meas_idx; k < num_measurements; k++)
            {
               negKHmat[row_idx][col_idx] -= Kmat[row_idx][k] * Hmat[k][col_idx];
            }
         }
      }

      // Compute remaining columns of result mat as [KH]_ij = sum_k K_ik*h_kj where we utilize that h_kj = 0 for all k > first_rr_idx and that h_k5 = 0 for all j
      for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx++)
      {
         for (uint32_t col_idx = first_rr_meas_idx; col_idx < (STATE_DIMENSION - 1U); col_idx++)
         {
            negKHmat[row_idx][col_idx] = 0.0F;
            for (uint32_t k = first_rr_meas_idx; k < num_measurements; k++)
            {
               negKHmat[row_idx][col_idx] -= Kmat[row_idx][k] * Hmat[k][col_idx];
            }
         }
         negKHmat[row_idx][5] = 0.0F;
      }
   }


   /*===========================================================================*\
   * FUNCTION: Kmat_Times_Rmat_CTCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&Kmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT] - the K matrix (Kalman gain) used in the CTCA Kalman filter update
   * const float32_t(&Rmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT] - The R matrix (measurement error covariance) used in CTCA Kalman filter update
   * const uint32_t num_measurements - total number of measurements
   * float32_t(&KRmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT] - - the resulting matrix multiplcation of Kmat * Rmat
   *
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
   * This function computes the matrix multiplication of Kmat * Rmat in the CTCA KF measurement update step.
   * It utilizes that Rmat has the following structure in order to optimize the computations with respect
   * to run time:
   * Rmat = [* * 0 0 0 0;
   *         * * 0 0 0 0;
   *         0 0 0 * 0 0; (this row, where the * value is located at the matrix diagonal, is repeated for each available range rate measurement)
   *         ....]
   *
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Kmat_Times_Rmat_CTCA(
      const float32_t(&Kmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const float32_t(&Rmat)[MSMT_UPDATE_MAX_NUM_OF_MSMT][MSMT_UPDATE_MAX_NUM_OF_MSMT],
      const uint32_t num_measurements,
      float32_t(&KRmat)[STATE_DIMENSION][MSMT_UPDATE_MAX_NUM_OF_MSMT])
   {
      // Compute first two columns of result mat as [KR]_i0/1 = sum_l k_il*hT_l0/1 where we utilize that h_l0/1 = 0 for all l > 1
      for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx++)
      {
         for (uint32_t col_idx = 0U; col_idx < 2U; col_idx++)
         {
            KRmat[row_idx][col_idx] = Kmat[row_idx][0] * Rmat[0][col_idx] + Kmat[row_idx][1] * Rmat[1][col_idx];
         }
      }

      // Compute remaining columns of result mat as [KR]_ij = sum_l k_il*r_lj where we utilize that r_lj = 0 for all l != j
      for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx++) // We can start on first_rr_idx instead of on 0 since HPHT is symmetric
      {
         for (uint32_t col_idx = 2U; col_idx < num_measurements; col_idx++)
         {
            KRmat[row_idx][col_idx] = Kmat[row_idx][col_idx] * Rmat[col_idx][col_idx];
         }
      }
   }
}
