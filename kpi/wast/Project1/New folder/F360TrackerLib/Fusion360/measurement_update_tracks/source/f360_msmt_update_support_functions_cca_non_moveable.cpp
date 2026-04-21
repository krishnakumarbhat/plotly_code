/*===========================================================================*\
* FILE: f360_msmt_update_support_functions_cca_non_moveable.cpp
*============================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function related to Kalman measurement update step for non-moveable CCA objects
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_msmt_update_support_functions_cca_non_moveable.h" 
#include "f360_msmt_update_support_functions_common.h" 
#include "f360_trk_fltr_cca_states.h"
#include "f360_pseudo_msmt.h"
#include "f360_det_cross_covariances.h"
#include "f360_math_func.h"
#include "f360_LinearSolvers.h"

namespace f360_variant_A
{

   // Struct to support when looping over a set of detections to find out if any of them are classified as moving
   struct Is_Det_Moving_CCA_Non_Moveable
   {
      Is_Det_Moving_CCA_Non_Moveable(const rspp_variant_A::RSPP_Detection_List_T& in_raw_detect_list) :
         raw_detect_list(in_raw_detect_list)
      {}

      bool operator()(const uint32_t idx) const
      {
         return (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == raw_detect_list.detections[idx].processed.motion_status);
      }
   private:
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list;
   };

  /*===========================================================================*\
   * FUNCTION: Kalman_Gain_Update_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION]                             - observation model matrix
   * const float32_t (&p_mat)[STATE_DIMENSION][STATE_DIMENSION]                                                      - state covariance
   * const float32_t (&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]    - pseudo-measurement covariance of measurement uncertainty
   * const F360_Calibrations_T& calibs,                                                                              - calibration parameters data structure
   * const uint8_t num_updates_since_init,                                                                           - number of KF measurement updates since object birth
   * float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]                                   - kalman gain
   * float32_t (&s_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]          - innovation covaraince
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
   * r_mat (3x3) must have the following structure:
   *    r_mat = [* * 0
   *             * * 0
   *             0 0 *]
   * 
   * h_mat (3x6) must have the following structure
   *    h_mat = [1 0 0 0 0 0
   *             0 0 0 1 0 0
   *             0 * 0 0 * 0]
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Kalman_Gain_Update_CCA_Non_Moveable(
      const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION],
      const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const F360_Calibrations_T& calibs,
      const uint8_t num_updates_since_init,
      float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      float32_t(&s_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]
   )
   {
      // KF equation: H * P
      float32_t temp_hp_mat[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION];
      Hmat_Times_Pmat_CCA_Non_Moveable(h_mat, p_mat, temp_hp_mat);

      // KF equation: H * P * H' + R
      Smat_CCA_Non_Moveable(h_mat, p_mat, r_mat, s_mat);

      // Use LDL solver for solving linear equation
      // KF equation: P*H'*inv(S) = (H*P)'*inv(S)
      Matrix_Division(s_mat, temp_hp_mat, MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE, STATE_DIMENSION, k_mat);

      if (num_updates_since_init < calibs.k_max_num_cca_updates_since_init_to_limit_acc)
      {
         for (uint32_t i = 0U; i < MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE; i++)
         {
            // Initially don't let acceleration be driven by measurements to avoid overshooting velocity estimate when initialization is very bad
            k_mat[F360_TRK_FLTR_CCA_STATE_AX][i] = 0.0F;
            k_mat[F360_TRK_FLTR_CCA_STATE_AY][i] = 0.0F;
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Error_Cov_Update_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]                             - kalman gain
   * const float32_t (&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]    - covariance of measurement uncertainty
   * const float32_t (&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION]                             - observation model matrix
   * float32_t (&p_mat)[STATE_DIMENSION][STATE_DIMENSION]                                                            - state error covariance matrix, updated in this fuction
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
   * This Measurement update step is performed through the Joseph form of covariance update to avoid numerical instability
   * P_update = (I - KH) * P * (I - KH)' + KRK'
   * 
   * r_mat must have the following structure:
   *    r_mat = [* * 0
   *             * * 0
   *             0 0 *]
   * 
   * h_mat must have the following structure
   *    h_mat = [1 0 0 0 0 0
   *             0 0 0 1 0 0
   *             0 * 0 0 * 0]
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Error_Cov_Update_CCA_Non_Moveable(
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION],
      float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      // KF equation: ikh_mat = I - (K * H)
      float32_t ikh_mat[STATE_DIMENSION][STATE_DIMENSION];
      I_Minus_Kmat_Times_Hmat_CCA_Non_Moveable(h_mat, k_mat, ikh_mat);

      // KF equation: ikhp_mat = (I - (K * H)) * P
      float32_t ikhp_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
      IKHmat_Times_Pmat_CCA_Non_Moveable(ikh_mat, p_mat, ikhp_mat);

      // KF equation: ikhp_ikh_mat = ((I - (K * H)) * P) * (I - K * H)'
      float32_t ikhp_ikh_mat[STATE_DIMENSION][STATE_DIMENSION];
      IKHPmat_Times_IKHmat_Transpose_CCA_Non_Moveable(ikhp_mat, ikh_mat, ikhp_ikh_mat);

      // KF equation: krk_mat = K * R * K'
      float32_t krk_mat[STATE_DIMENSION][STATE_DIMENSION];
      Kmat_Times_Rmat_Times_Kmat_CCA_Non_Moveable(k_mat, r_mat, krk_mat);

      // KF equation: p_mat =  ((I - (K * H)) * P) * (I - K * H)'  +  K * R * K'
      F360_matadd_6x6_symmetric_matrix(ikhp_ikh_mat, krk_mat, p_mat);
   }


   /*===========================================================================*\
   * FUNCTION: State_Update_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&z_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]                      - pseudo measurement vector
   * const float32_t (&zhat_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]                   - predicted measurement (previous state)
   * const float32_t (&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]     - kalman gain
   * float32_t (&state)[STATE_DIMENSION]                                                     - object's current state vector
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

   void State_Update_CCA_Non_Moveable(
      const float32_t(&z_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const float32_t(&zhat_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      float32_t(&state)[STATE_DIMENSION])
   {

      // The Kalman filter equation implemented below is x_updated = x_predicted + K*(z - z_pred)
      for (uint32_t row = 0U; row < STATE_DIMENSION; row++)
      {
         float32_t state_increment = 0.0F;
         for (uint32_t col = 0U; col < MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE; col++)
         {
            state_increment += k_mat[row][col] * (z_mat[col] - zhat_mat[col]);
         }
         state[row] += state_increment;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Hmat_Times_Pmat_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&Hmat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CCA Kalman filter update.
   * const float32_t(&Pmat)[STATE_DIMENSION][STATE_DIMENSION] - the P matrix (state error covariance) in the CCA Kalman filter update
   * float32_t(&HPmat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION]) - the resulting matrix multiplcation of Hmat * Pmat
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
   * This function computes the matrix multiplication of Hmat * Pmat in the CCA non_moveable KF measurement update step
   * 
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Hmat_Times_Pmat_CCA_Non_Moveable(
       const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION],
       const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION],
       float32_t(&hp_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION])
   {
      hp_mat[0][0] = p_mat[0][0];
      hp_mat[0][1] = p_mat[0][1];
      hp_mat[0][2] = p_mat[0][2];
      hp_mat[0][3] = p_mat[0][3];
      hp_mat[0][4] = p_mat[0][4];
      hp_mat[0][5] = p_mat[0][5];

      hp_mat[1][0] = p_mat[3][0];
      hp_mat[1][1] = p_mat[3][1];
      hp_mat[1][2] = p_mat[3][2];
      hp_mat[1][3] = p_mat[3][3];
      hp_mat[1][4] = p_mat[3][4];
      hp_mat[1][5] = p_mat[3][5];

      hp_mat[2][0] = h_mat[2][1] * p_mat[1][0] + h_mat[2][4] * p_mat[4][0];
      hp_mat[2][1] = h_mat[2][1] * p_mat[1][1] + h_mat[2][4] * p_mat[4][1];
      hp_mat[2][2] = h_mat[2][1] * p_mat[1][2] + h_mat[2][4] * p_mat[4][2];
      hp_mat[2][3] = h_mat[2][1] * p_mat[1][3] + h_mat[2][4] * p_mat[4][3];
      hp_mat[2][4] = h_mat[2][1] * p_mat[1][4] + h_mat[2][4] * p_mat[4][4];
      hp_mat[2][5] = h_mat[2][1] * p_mat[1][5] + h_mat[2][4] * p_mat[4][5];
   }

   /*===========================================================================*\
   * FUNCTION: Smat_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CCA Kalman filter update.
   * const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION] - the P matrix (state error covariance) in the CCA Kalman filter update
   * const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE] - the R matrix, i.e measurement noise matrix
   * float32_t(&s_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]) - the resulting innovation matrix
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
   * This function computes the innovation matrix in the CCA non_moveable KF measurement update step
   * The following matrix multiplication is done here:  H * P * H' + R
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Smat_CCA_Non_Moveable(
      const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION],
      const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      float32_t(&s_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE])
   {
      s_mat[0][0] = p_mat[0][0] + r_mat[0][0];
      s_mat[0][1] = p_mat[0][3] + r_mat[0][1];
      s_mat[0][2] = h_mat[2][1] * p_mat[0][1] + h_mat[2][4] * p_mat[0][4];

      s_mat[1][0] = p_mat[3][0] + r_mat[1][0];
      s_mat[1][1] = p_mat[3][3] + r_mat[1][1];
      s_mat[1][2] = h_mat[2][1] * p_mat[3][1] + h_mat[2][4] * p_mat[3][4];

      s_mat[2][0] = h_mat[2][1] * p_mat[1][0] + h_mat[2][4] * p_mat[4][0];
      s_mat[2][1] = h_mat[2][1] * p_mat[1][3] + h_mat[2][4] * p_mat[4][3];
      s_mat[2][2] = r_mat[2][2] + h_mat[2][1] * (h_mat[2][1] * p_mat[1][1] + h_mat[2][4] * p_mat[4][1]) + h_mat[2][4] * (h_mat[2][1] * p_mat[1][4] + h_mat[2][4] * p_mat[4][4]);
   }


   /*===========================================================================*\
   * FUNCTION: I_Minus_Kmat_Times_Hmat_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CCA Kalman filter update.
   * const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE] - the K matrix, i.e measurement noise matrix
   * float32_t(&ikh_mat)[STATE_DIMENSION][STATE_DIMENSION]) - the resulting matrix
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
   * This function computes the matrix multiplication for the CCA non_moveable KF measurement update step
   * The following matrix multiplication is done here:  (I - K*H)
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void I_Minus_Kmat_Times_Hmat_CCA_Non_Moveable(
      const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION],
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      float32_t(&ikh_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      ikh_mat[0][0] = 1.0F - k_mat[0][0];
      ikh_mat[0][1] = -h_mat[2][1] * k_mat[0][2];
      ikh_mat[0][2] = 0.0F;
      ikh_mat[0][3] = -k_mat[0][1];
      ikh_mat[0][4] = -h_mat[2][4] * k_mat[0][2];
      ikh_mat[0][5] = 0.0F;

      ikh_mat[1][0] = -k_mat[1][0];
      ikh_mat[1][1] = 1.0F - h_mat[2][1] * k_mat[1][2];
      ikh_mat[1][2] = 0.0F;
      ikh_mat[1][3] = -k_mat[1][1];
      ikh_mat[1][4] = -h_mat[2][4] * k_mat[1][2];
      ikh_mat[1][5] = 0.0F;

      ikh_mat[2][0] = -k_mat[2][0];
      ikh_mat[2][1] = -h_mat[2][1] * k_mat[2][2];
      ikh_mat[2][2] = 1.0F;
      ikh_mat[2][3] = -k_mat[2][1];
      ikh_mat[2][4] = -h_mat[2][4] * k_mat[2][2];
      ikh_mat[2][5] = 0.0F;

      ikh_mat[3][0] = -k_mat[3][0];
      ikh_mat[3][1] = -h_mat[2][1] * k_mat[3][2];
      ikh_mat[3][2] = 0.0F;
      ikh_mat[3][3] = 1.0F - k_mat[3][1];
      ikh_mat[3][4] = -h_mat[2][4] * k_mat[3][2];
      ikh_mat[3][5] = 0.0F;

      ikh_mat[4][0] = -k_mat[4][0];
      ikh_mat[4][1] = -h_mat[2][1] * k_mat[4][2];
      ikh_mat[4][2] = 0.0F;
      ikh_mat[4][3] = -k_mat[4][1];
      ikh_mat[4][4] = 1.0F - h_mat[2][4] * k_mat[4][2];
      ikh_mat[4][5] = 0.0F;

      ikh_mat[5][0] = -k_mat[5][0];
      ikh_mat[5][1] = -h_mat[2][1] * k_mat[5][2];
      ikh_mat[5][2] = 0.0F;
      ikh_mat[5][3] = -k_mat[5][1];
      ikh_mat[5][4] = -h_mat[2][4] * k_mat[5][2];
      ikh_mat[5][5] = 1.0F;
   }


   /*===========================================================================*\
   * FUNCTION: IKHmat_Times_Pmat_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&h_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][STATE_DIMENSION] - the H matrix (linearized measurement model) used in the CCA Kalman filter update.
   * const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE] - the K matrix, i.e measurement noise matrix
   * float32_t(&ikhp_mat)[STATE_DIMENSION][STATE_DIMENSION]) - the resulting matrix
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
   * This function computes the matrix multiplication for the CCA non_moveable KF measurement update step
   * The following matrix multiplication is done here:  (I - K*H)
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void IKHmat_Times_Pmat_CCA_Non_Moveable(
      const float32_t(&ikh_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t(&p_mat)[STATE_DIMENSION][STATE_DIMENSION],
      float32_t(&ikhp_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {

      ikhp_mat[0][0] = ikh_mat[0][0] * p_mat[0][0] + ikh_mat[0][1] * p_mat[1][0] + ikh_mat[0][3] * p_mat[3][0] + ikh_mat[0][4] * p_mat[4][0];
      ikhp_mat[0][1] = ikh_mat[0][0] * p_mat[0][1] + ikh_mat[0][1] * p_mat[1][1] + ikh_mat[0][3] * p_mat[3][1] + ikh_mat[0][4] * p_mat[4][1];
      ikhp_mat[0][2] = ikh_mat[0][0] * p_mat[0][2] + ikh_mat[0][1] * p_mat[1][2] + ikh_mat[0][3] * p_mat[3][2] + ikh_mat[0][4] * p_mat[4][2];
      ikhp_mat[0][3] = ikh_mat[0][0] * p_mat[0][3] + ikh_mat[0][1] * p_mat[1][3] + ikh_mat[0][3] * p_mat[3][3] + ikh_mat[0][4] * p_mat[4][3];
      ikhp_mat[0][4] = ikh_mat[0][0] * p_mat[0][4] + ikh_mat[0][1] * p_mat[1][4] + ikh_mat[0][3] * p_mat[3][4] + ikh_mat[0][4] * p_mat[4][4];
      ikhp_mat[0][5] = ikh_mat[0][0] * p_mat[0][5] + ikh_mat[0][1] * p_mat[1][5] + ikh_mat[0][3] * p_mat[3][5] + ikh_mat[0][4] * p_mat[4][5];

      ikhp_mat[1][0] = ikh_mat[1][0] * p_mat[0][0] + ikh_mat[1][1] * p_mat[1][0] + ikh_mat[1][3] * p_mat[3][0] + ikh_mat[1][4] * p_mat[4][0];
      ikhp_mat[1][1] = ikh_mat[1][0] * p_mat[0][1] + ikh_mat[1][1] * p_mat[1][1] + ikh_mat[1][3] * p_mat[3][1] + ikh_mat[1][4] * p_mat[4][1];
      ikhp_mat[1][2] = ikh_mat[1][0] * p_mat[0][2] + ikh_mat[1][1] * p_mat[1][2] + ikh_mat[1][3] * p_mat[3][2] + ikh_mat[1][4] * p_mat[4][2];
      ikhp_mat[1][3] = ikh_mat[1][0] * p_mat[0][3] + ikh_mat[1][1] * p_mat[1][3] + ikh_mat[1][3] * p_mat[3][3] + ikh_mat[1][4] * p_mat[4][3];
      ikhp_mat[1][4] = ikh_mat[1][0] * p_mat[0][4] + ikh_mat[1][1] * p_mat[1][4] + ikh_mat[1][3] * p_mat[3][4] + ikh_mat[1][4] * p_mat[4][4];
      ikhp_mat[1][5] = ikh_mat[1][0] * p_mat[0][5] + ikh_mat[1][1] * p_mat[1][5] + ikh_mat[1][3] * p_mat[3][5] + ikh_mat[1][4] * p_mat[4][5];

      ikhp_mat[2][0] = p_mat[2][0] + ikh_mat[2][0] * p_mat[0][0] + ikh_mat[2][1] * p_mat[1][0] + ikh_mat[2][3] * p_mat[3][0] + ikh_mat[2][4] * p_mat[4][0];
      ikhp_mat[2][1] = p_mat[2][1] + ikh_mat[2][0] * p_mat[0][1] + ikh_mat[2][1] * p_mat[1][1] + ikh_mat[2][3] * p_mat[3][1] + ikh_mat[2][4] * p_mat[4][1];
      ikhp_mat[2][2] = p_mat[2][2] + ikh_mat[2][0] * p_mat[0][2] + ikh_mat[2][1] * p_mat[1][2] + ikh_mat[2][3] * p_mat[3][2] + ikh_mat[2][4] * p_mat[4][2];
      ikhp_mat[2][3] = p_mat[2][3] + ikh_mat[2][0] * p_mat[0][3] + ikh_mat[2][1] * p_mat[1][3] + ikh_mat[2][3] * p_mat[3][3] + ikh_mat[2][4] * p_mat[4][3];
      ikhp_mat[2][4] = p_mat[2][4] + ikh_mat[2][0] * p_mat[0][4] + ikh_mat[2][1] * p_mat[1][4] + ikh_mat[2][3] * p_mat[3][4] + ikh_mat[2][4] * p_mat[4][4];
      ikhp_mat[2][5] = p_mat[2][5] + ikh_mat[2][0] * p_mat[0][5] + ikh_mat[2][1] * p_mat[1][5] + ikh_mat[2][3] * p_mat[3][5] + ikh_mat[2][4] * p_mat[4][5];

      ikhp_mat[3][0] = ikh_mat[3][0] * p_mat[0][0] + ikh_mat[3][1] * p_mat[1][0] + ikh_mat[3][3] * p_mat[3][0] + ikh_mat[3][4] * p_mat[4][0];
      ikhp_mat[3][1] = ikh_mat[3][0] * p_mat[0][1] + ikh_mat[3][1] * p_mat[1][1] + ikh_mat[3][3] * p_mat[3][1] + ikh_mat[3][4] * p_mat[4][1];
      ikhp_mat[3][2] = ikh_mat[3][0] * p_mat[0][2] + ikh_mat[3][1] * p_mat[1][2] + ikh_mat[3][3] * p_mat[3][2] + ikh_mat[3][4] * p_mat[4][2];
      ikhp_mat[3][3] = ikh_mat[3][0] * p_mat[0][3] + ikh_mat[3][1] * p_mat[1][3] + ikh_mat[3][3] * p_mat[3][3] + ikh_mat[3][4] * p_mat[4][3];
      ikhp_mat[3][4] = ikh_mat[3][0] * p_mat[0][4] + ikh_mat[3][1] * p_mat[1][4] + ikh_mat[3][3] * p_mat[3][4] + ikh_mat[3][4] * p_mat[4][4];
      ikhp_mat[3][5] = ikh_mat[3][0] * p_mat[0][5] + ikh_mat[3][1] * p_mat[1][5] + ikh_mat[3][3] * p_mat[3][5] + ikh_mat[3][4] * p_mat[4][5];

      ikhp_mat[4][0] = ikh_mat[4][0] * p_mat[0][0] + ikh_mat[4][1] * p_mat[1][0] + ikh_mat[4][3] * p_mat[3][0] + ikh_mat[4][4] * p_mat[4][0];
      ikhp_mat[4][1] = ikh_mat[4][0] * p_mat[0][1] + ikh_mat[4][1] * p_mat[1][1] + ikh_mat[4][3] * p_mat[3][1] + ikh_mat[4][4] * p_mat[4][1];
      ikhp_mat[4][2] = ikh_mat[4][0] * p_mat[0][2] + ikh_mat[4][1] * p_mat[1][2] + ikh_mat[4][3] * p_mat[3][2] + ikh_mat[4][4] * p_mat[4][2];
      ikhp_mat[4][3] = ikh_mat[4][0] * p_mat[0][3] + ikh_mat[4][1] * p_mat[1][3] + ikh_mat[4][3] * p_mat[3][3] + ikh_mat[4][4] * p_mat[4][3];
      ikhp_mat[4][4] = ikh_mat[4][0] * p_mat[0][4] + ikh_mat[4][1] * p_mat[1][4] + ikh_mat[4][3] * p_mat[3][4] + ikh_mat[4][4] * p_mat[4][4];
      ikhp_mat[4][5] = ikh_mat[4][0] * p_mat[0][5] + ikh_mat[4][1] * p_mat[1][5] + ikh_mat[4][3] * p_mat[3][5] + ikh_mat[4][4] * p_mat[4][5];

      ikhp_mat[5][0] = p_mat[5][0] + ikh_mat[5][0] * p_mat[0][0] + ikh_mat[5][1] * p_mat[1][0] + ikh_mat[5][3] * p_mat[3][0] + ikh_mat[5][4] * p_mat[4][0];
      ikhp_mat[5][1] = p_mat[5][1] + ikh_mat[5][0] * p_mat[0][1] + ikh_mat[5][1] * p_mat[1][1] + ikh_mat[5][3] * p_mat[3][1] + ikh_mat[5][4] * p_mat[4][1];
      ikhp_mat[5][2] = p_mat[5][2] + ikh_mat[5][0] * p_mat[0][2] + ikh_mat[5][1] * p_mat[1][2] + ikh_mat[5][3] * p_mat[3][2] + ikh_mat[5][4] * p_mat[4][2];
      ikhp_mat[5][3] = p_mat[5][3] + ikh_mat[5][0] * p_mat[0][3] + ikh_mat[5][1] * p_mat[1][3] + ikh_mat[5][3] * p_mat[3][3] + ikh_mat[5][4] * p_mat[4][3];
      ikhp_mat[5][4] = p_mat[5][4] + ikh_mat[5][0] * p_mat[0][4] + ikh_mat[5][1] * p_mat[1][4] + ikh_mat[5][3] * p_mat[3][4] + ikh_mat[5][4] * p_mat[4][4];
      ikhp_mat[5][5] = p_mat[5][5] + ikh_mat[5][0] * p_mat[0][5] + ikh_mat[5][1] * p_mat[1][5] + ikh_mat[5][3] * p_mat[3][5] + ikh_mat[5][4] * p_mat[4][5];
   }

   /*===========================================================================*\
   * FUNCTION: IKHPmat_Times_IKHmat_Transpose_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&ikhp_mat)[STATE_DIMENSION][STATE_DIMENSION]
   * const float32_t(&ikh_mat)[STATE_DIMENSION][STATE_DIMENSION]
   * float32_t(&ikhp_ikh_mat)[STATE_DIMENSION][STATE_DIMENSION]
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
   * This function computes the matrix multiplication for the CCA non_moveable KF measurement update step
   * The following matrix multiplication is done here:  ikhp_ikh_mat = ((I - (K * H)) * P) * (I - K * H)'
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void IKHPmat_Times_IKHmat_Transpose_CCA_Non_Moveable(
      const float32_t(&ikhp_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t(&ikh_mat)[STATE_DIMENSION][STATE_DIMENSION],
      float32_t(&ikhp_ikh_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      ikhp_ikh_mat[0][0] = ikh_mat[0][0] * ikhp_mat[0][0] + ikh_mat[0][1] * ikhp_mat[0][1] + ikh_mat[0][3] * ikhp_mat[0][3] + ikh_mat[0][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[0][1] = ikh_mat[1][0] * ikhp_mat[0][0] + ikh_mat[1][1] * ikhp_mat[0][1] + ikh_mat[1][3] * ikhp_mat[0][3] + ikh_mat[1][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[0][2] = ikhp_mat[0][2] + ikh_mat[2][0] * ikhp_mat[0][0] + ikh_mat[2][1] * ikhp_mat[0][1] + ikh_mat[2][3] * ikhp_mat[0][3] + ikh_mat[2][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[0][3] = ikh_mat[3][0] * ikhp_mat[0][0] + ikh_mat[3][1] * ikhp_mat[0][1] + ikh_mat[3][3] * ikhp_mat[0][3] + ikh_mat[3][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[0][4] = ikh_mat[4][0] * ikhp_mat[0][0] + ikh_mat[4][1] * ikhp_mat[0][1] + ikh_mat[4][3] * ikhp_mat[0][3] + ikh_mat[4][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[0][5] = ikhp_mat[0][5] + ikh_mat[5][0] * ikhp_mat[0][0] + ikh_mat[5][1] * ikhp_mat[0][1] + ikh_mat[5][3] * ikhp_mat[0][3] + ikh_mat[5][4] * ikhp_mat[0][4];
      ikhp_ikh_mat[1][0] = ikh_mat[0][0] * ikhp_mat[1][0] + ikh_mat[0][1] * ikhp_mat[1][1] + ikh_mat[0][3] * ikhp_mat[1][3] + ikh_mat[0][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[1][1] = ikh_mat[1][0] * ikhp_mat[1][0] + ikh_mat[1][1] * ikhp_mat[1][1] + ikh_mat[1][3] * ikhp_mat[1][3] + ikh_mat[1][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[1][2] = ikhp_mat[1][2] + ikh_mat[2][0] * ikhp_mat[1][0] + ikh_mat[2][1] * ikhp_mat[1][1] + ikh_mat[2][3] * ikhp_mat[1][3] + ikh_mat[2][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[1][3] = ikh_mat[3][0] * ikhp_mat[1][0] + ikh_mat[3][1] * ikhp_mat[1][1] + ikh_mat[3][3] * ikhp_mat[1][3] + ikh_mat[3][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[1][4] = ikh_mat[4][0] * ikhp_mat[1][0] + ikh_mat[4][1] * ikhp_mat[1][1] + ikh_mat[4][3] * ikhp_mat[1][3] + ikh_mat[4][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[1][5] = ikhp_mat[1][5] + ikh_mat[5][0] * ikhp_mat[1][0] + ikh_mat[5][1] * ikhp_mat[1][1] + ikh_mat[5][3] * ikhp_mat[1][3] + ikh_mat[5][4] * ikhp_mat[1][4];
      ikhp_ikh_mat[2][0] = ikh_mat[0][0] * ikhp_mat[2][0] + ikh_mat[0][1] * ikhp_mat[2][1] + ikh_mat[0][3] * ikhp_mat[2][3] + ikh_mat[0][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[2][1] = ikh_mat[1][0] * ikhp_mat[2][0] + ikh_mat[1][1] * ikhp_mat[2][1] + ikh_mat[1][3] * ikhp_mat[2][3] + ikh_mat[1][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[2][2] = ikhp_mat[2][2] + ikh_mat[2][0] * ikhp_mat[2][0] + ikh_mat[2][1] * ikhp_mat[2][1] + ikh_mat[2][3] * ikhp_mat[2][3] + ikh_mat[2][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[2][3] = ikh_mat[3][0] * ikhp_mat[2][0] + ikh_mat[3][1] * ikhp_mat[2][1] + ikh_mat[3][3] * ikhp_mat[2][3] + ikh_mat[3][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[2][4] = ikh_mat[4][0] * ikhp_mat[2][0] + ikh_mat[4][1] * ikhp_mat[2][1] + ikh_mat[4][3] * ikhp_mat[2][3] + ikh_mat[4][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[2][5] = ikhp_mat[2][5] + ikh_mat[5][0] * ikhp_mat[2][0] + ikh_mat[5][1] * ikhp_mat[2][1] + ikh_mat[5][3] * ikhp_mat[2][3] + ikh_mat[5][4] * ikhp_mat[2][4];
      ikhp_ikh_mat[3][0] = ikh_mat[0][0] * ikhp_mat[3][0] + ikh_mat[0][1] * ikhp_mat[3][1] + ikh_mat[0][3] * ikhp_mat[3][3] + ikh_mat[0][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[3][1] = ikh_mat[1][0] * ikhp_mat[3][0] + ikh_mat[1][1] * ikhp_mat[3][1] + ikh_mat[1][3] * ikhp_mat[3][3] + ikh_mat[1][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[3][2] = ikhp_mat[3][2] + ikh_mat[2][0] * ikhp_mat[3][0] + ikh_mat[2][1] * ikhp_mat[3][1] + ikh_mat[2][3] * ikhp_mat[3][3] + ikh_mat[2][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[3][3] = ikh_mat[3][0] * ikhp_mat[3][0] + ikh_mat[3][1] * ikhp_mat[3][1] + ikh_mat[3][3] * ikhp_mat[3][3] + ikh_mat[3][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[3][4] = ikh_mat[4][0] * ikhp_mat[3][0] + ikh_mat[4][1] * ikhp_mat[3][1] + ikh_mat[4][3] * ikhp_mat[3][3] + ikh_mat[4][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[3][5] = ikhp_mat[3][5] + ikh_mat[5][0] * ikhp_mat[3][0] + ikh_mat[5][1] * ikhp_mat[3][1] + ikh_mat[5][3] * ikhp_mat[3][3] + ikh_mat[5][4] * ikhp_mat[3][4];
      ikhp_ikh_mat[4][0] = ikh_mat[0][0] * ikhp_mat[4][0] + ikh_mat[0][1] * ikhp_mat[4][1] + ikh_mat[0][3] * ikhp_mat[4][3] + ikh_mat[0][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[4][1] = ikh_mat[1][0] * ikhp_mat[4][0] + ikh_mat[1][1] * ikhp_mat[4][1] + ikh_mat[1][3] * ikhp_mat[4][3] + ikh_mat[1][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[4][2] = ikhp_mat[4][2] + ikh_mat[2][0] * ikhp_mat[4][0] + ikh_mat[2][1] * ikhp_mat[4][1] + ikh_mat[2][3] * ikhp_mat[4][3] + ikh_mat[2][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[4][3] = ikh_mat[3][0] * ikhp_mat[4][0] + ikh_mat[3][1] * ikhp_mat[4][1] + ikh_mat[3][3] * ikhp_mat[4][3] + ikh_mat[3][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[4][4] = ikh_mat[4][0] * ikhp_mat[4][0] + ikh_mat[4][1] * ikhp_mat[4][1] + ikh_mat[4][3] * ikhp_mat[4][3] + ikh_mat[4][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[4][5] = ikhp_mat[4][5] + ikh_mat[5][0] * ikhp_mat[4][0] + ikh_mat[5][1] * ikhp_mat[4][1] + ikh_mat[5][3] * ikhp_mat[4][3] + ikh_mat[5][4] * ikhp_mat[4][4];
      ikhp_ikh_mat[5][0] = ikh_mat[0][0] * ikhp_mat[5][0] + ikh_mat[0][1] * ikhp_mat[5][1] + ikh_mat[0][3] * ikhp_mat[5][3] + ikh_mat[0][4] * ikhp_mat[5][4];
      ikhp_ikh_mat[5][1] = ikh_mat[1][0] * ikhp_mat[5][0] + ikh_mat[1][1] * ikhp_mat[5][1] + ikh_mat[1][3] * ikhp_mat[5][3] + ikh_mat[1][4] * ikhp_mat[5][4];
      ikhp_ikh_mat[5][2] = ikhp_mat[5][2] + ikh_mat[2][0] * ikhp_mat[5][0] + ikh_mat[2][1] * ikhp_mat[5][1] + ikh_mat[2][3] * ikhp_mat[5][3] + ikh_mat[2][4] * ikhp_mat[5][4];
      ikhp_ikh_mat[5][3] = ikh_mat[3][0] * ikhp_mat[5][0] + ikh_mat[3][1] * ikhp_mat[5][1] + ikh_mat[3][3] * ikhp_mat[5][3] + ikh_mat[3][4] * ikhp_mat[5][4];
      ikhp_ikh_mat[5][4] = ikh_mat[4][0] * ikhp_mat[5][0] + ikh_mat[4][1] * ikhp_mat[5][1] + ikh_mat[4][3] * ikhp_mat[5][3] + ikh_mat[4][4] * ikhp_mat[5][4];
      ikhp_ikh_mat[5][5] = ikhp_mat[5][5] + ikh_mat[5][0] * ikhp_mat[5][0] + ikh_mat[5][1] * ikhp_mat[5][1] + ikh_mat[5][3] * ikhp_mat[5][3] + ikh_mat[5][4] * ikhp_mat[5][4];
   }


   /*===========================================================================*\
   * FUNCTION: Kmat_Times_Rmat_Times_Kmat_CCA_Non_Moveable()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]
   * const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE]
   * float32_t(&krk_mat)[STATE_DIMENSION][STATE_DIMENSION])
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
   * This function computes the matrix multiplication for the CCA non_moveable KF measurement update step
   * The following matrix multiplication is done here:  krk_mat = K * R * K'
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Kmat_Times_Rmat_Times_Kmat_CCA_Non_Moveable(
      const float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      const float32_t(&r_mat)[MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE],
      float32_t(&krk_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      krk_mat[0][0] = k_mat[0][2] * k_mat[0][2] * r_mat[2][2] + k_mat[0][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]);
      krk_mat[0][1] = k_mat[1][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[1][2] * r_mat[2][2];
      krk_mat[0][2] = k_mat[2][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[2][2] * r_mat[2][2];
      krk_mat[0][3] = k_mat[3][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[0][4] = k_mat[4][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[0][5] = k_mat[5][0] * (k_mat[0][0] * r_mat[0][0] + k_mat[0][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[0][0] * r_mat[0][1] + k_mat[0][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[5][2] * r_mat[2][2];

      krk_mat[1][0] = k_mat[0][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[1][2] * r_mat[2][2];
      krk_mat[1][1] = k_mat[1][2] * k_mat[1][2] * r_mat[2][2] + k_mat[1][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]);
      krk_mat[1][2] = k_mat[2][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[2][2] * r_mat[2][2];
      krk_mat[1][3] = k_mat[3][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[1][4] = k_mat[4][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[1][5] = k_mat[5][0] * (k_mat[1][0] * r_mat[0][0] + k_mat[1][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[1][0] * r_mat[0][1] + k_mat[1][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[5][2] * r_mat[2][2];

      krk_mat[2][0] = k_mat[0][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[2][2] * r_mat[2][2];
      krk_mat[2][1] = k_mat[1][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[2][2] * r_mat[2][2];
      krk_mat[2][2] = k_mat[2][2] * k_mat[2][2] * r_mat[2][2] + k_mat[2][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]);
      krk_mat[2][3] = k_mat[3][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[2][4] = k_mat[4][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[2][5] = k_mat[5][0] * (k_mat[2][0] * r_mat[0][0] + k_mat[2][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[2][0] * r_mat[0][1] + k_mat[2][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[5][2] * r_mat[2][2];

      krk_mat[3][0] = k_mat[0][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[3][1] = k_mat[1][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[3][2] = k_mat[2][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[3][2] * r_mat[2][2];
      krk_mat[3][3] = k_mat[3][2] * k_mat[3][2] * r_mat[2][2] + k_mat[3][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]);
      krk_mat[3][4] = k_mat[4][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]) + k_mat[3][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[3][5] = k_mat[5][0] * (k_mat[3][0] * r_mat[0][0] + k_mat[3][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[3][0] * r_mat[0][1] + k_mat[3][1] * r_mat[1][1]) + k_mat[3][2] * k_mat[5][2] * r_mat[2][2];

      krk_mat[4][0] = k_mat[0][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[4][1] = k_mat[1][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[4][2] = k_mat[2][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[4][3] = k_mat[3][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]) + k_mat[3][2] * k_mat[4][2] * r_mat[2][2];
      krk_mat[4][4] = k_mat[4][2] * k_mat[4][2] * r_mat[2][2] + k_mat[4][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]);
      krk_mat[4][5] = k_mat[5][0] * (k_mat[4][0] * r_mat[0][0] + k_mat[4][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[4][0] * r_mat[0][1] + k_mat[4][1] * r_mat[1][1]) + k_mat[4][2] * k_mat[5][2] * r_mat[2][2];

      krk_mat[5][0] = k_mat[0][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[0][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]) + k_mat[0][2] * k_mat[5][2] * r_mat[2][2];
      krk_mat[5][1] = k_mat[1][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[1][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]) + k_mat[1][2] * k_mat[5][2] * r_mat[2][2];
      krk_mat[5][2] = k_mat[2][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[2][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]) + k_mat[2][2] * k_mat[5][2] * r_mat[2][2];
      krk_mat[5][3] = k_mat[3][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[3][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]) + k_mat[3][2] * k_mat[5][2] * r_mat[2][2];
      krk_mat[5][4] = k_mat[4][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[4][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]) + k_mat[4][2] * k_mat[5][2] * r_mat[2][2];
      krk_mat[5][5] = k_mat[5][2] * k_mat[5][2] * r_mat[2][2] + k_mat[5][0] * (k_mat[5][0] * r_mat[0][0] + k_mat[5][1] * r_mat[1][0]) + k_mat[5][1] * (k_mat[5][0] * r_mat[0][1] + k_mat[5][1] * r_mat[1][1]);
   }

   /*===========================================================================*\
   * FUNCTION: Generate_Single_Pseudo_Range_Rate_Compensated_Measurement_CCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const uint32_t(&dets_idx)[MAX_DETS_IN_OBJ_TRK]
   * const uint32_t num_dets
   * float32_t& mean_comp_rdot
   * float32_t& mean_cos_vcs_az
   * float32_t& mean_sin_vcs_az
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
   * This function takes an array of detection indexes and computes the mean
   * compensated range rate, the mean cos(vcs_azimuth) and the mean sin(vcs_azimuth).
   *
   * PRECONDITIONS:
   * Number of detections must be larger than 0.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Generate_Single_Pseudo_Range_Rate_Compensated_Measurement_CCA(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const uint32_t(&dets_idx)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t num_dets,
      float32_t& mean_comp_rdot,
      float32_t& mean_cos_vcs_az,
      float32_t& mean_sin_vcs_az)
   {
      // Use mean of compensated range rate detections as measurement
      float32_t sum_comp_rdot = 0.0F;
      float32_t sum_cos_vcs_az = 0.0F;
      float32_t sum_sin_vcs_az = 0.0F;

      for (uint32_t i = 0U; i < num_dets; i++)
      {
         const uint32_t det_idx = dets_idx[i];

         sum_comp_rdot += det_props[det_idx].range_rate_compensated;
         sum_cos_vcs_az += raw_detection_list.detections[det_idx].processed.cos_vcs_az;
         sum_sin_vcs_az += raw_detection_list.detections[det_idx].processed.sin_vcs_az;
      }

      const float32_t inv_selected_dets_num = 1.0F / static_cast<float32_t>(num_dets);
      mean_comp_rdot = sum_comp_rdot * inv_selected_dets_num;
      mean_cos_vcs_az = sum_cos_vcs_az * inv_selected_dets_num;
      mean_sin_vcs_az = sum_sin_vcs_az * inv_selected_dets_num;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Obj_Suspected_Moveable_CCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * const uint32_t(&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK] - reference to an array containing the indexes of the detections that
   *                                                               are used to perform KF measurement update on the object
   * const uint32_t selected_dets_num - number of detections that are used to perform KF measurement update on the object
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
   * This function checks if there are indications that an object can be moveable.
   * The object f_moveable flag is false as the precondition.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   bool Is_Obj_Suspected_Moveable_CCA(
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const uint32_t(&selected_dets_idx)[MAX_DETS_IN_OBJ_TRK],
      const uint32_t selected_dets_num)
   {
      // Check if any of the detections are classified as moving
      const uint32_t* const begin = &selected_dets_idx[0];
      const uint32_t* const end = &selected_dets_idx[selected_dets_num];
      const uint32_t* const result = std::find_if(begin, end, Is_Det_Moving_CCA_Non_Moveable{ raw_detect_list });
      const bool is_obj_susp_mov = (result != end);

      return is_obj_susp_mov;
   }


   /*===========================================================================*\
   * FUNCTION: Limit_Pseudo_Pos_Meas_Impact_On_Obj_Vel_Estimate_CCA()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object_track - referense to a single object
   * const float32_t host_yaw_rate - Host yaw rate in radians
   * const F360_Calibrations_T& calib - reference to data structure containing calibration parameters
   * float32_t(&k_mat)[STATE_DIMENSION_CCA][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE] - reference to the KF gain matrix
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
   * This function modifies some of the elements in the Kalman Filter gain matrix.
   * The matrix elements that corresponds to how much the pseudo position
   * measurement will impact the object velocity estimate are decreased by a factor.
   * The decrease factor is in the range of [0, 1] and is dependent on the position
   * innovation (i.e. the difference between the time predicted object position and
   * the position pseudo measurement). If the position innovation is small, i.e. the
   * position seems to be changing smoothly, then the decreas factor will be close to
   * 1 meaning that the Kalman gain elements will be modified very little. When the
   * position innovation is larger, i.e. there seems to be a jump in the position, then
   * the decrease factor is closer to 0 meaning that the position measurement will not
   * be allowed to influence the object velocity estimate to as large extent.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Limit_Pseudo_Pos_Meas_Impact_On_Obj_Vel_Estim_CCA(
      const F360_Object_Track_T& object_track,
      const float32_t host_yaw_rate,
      const F360_Calibrations_T& calib,
      float32_t(&k_mat)[STATE_DIMENSION][MSMT_UPDATE_NUM_OF_MSMT_CCA_NON_MOVEABLE])
   {
      const float32_t decrease_factor = Compute_KF_Gain_Decrease_Factor_CCA(object_track, host_yaw_rate, calib);

      k_mat[F360_TRK_FLTR_CCA_STATE_VX][0] *= decrease_factor; // Matrix column 0 corresponds to the longitudinal pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_VX][1] *= decrease_factor; // Matrix column 1 corresponds to the lateral pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_VY][0] *= decrease_factor; // Matrix column 0 corresponds to the longitudinal pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_VY][1] *= decrease_factor; // Matrix column 1 corresponds to the lateral pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_AX][0] *= decrease_factor; // Matrix column 0 corresponds to the longitudinal pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_AX][1] *= decrease_factor; // Matrix column 1 corresponds to the lateral pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_AY][0] *= decrease_factor; // Matrix column 0 corresponds to the longitudinal pseudo position measurement
      k_mat[F360_TRK_FLTR_CCA_STATE_AY][1] *= decrease_factor; // Matrix column 1 corresponds to the lateral pseudo position measurement
   }



   /*===========================================================================*\
   * FUNCTION: Compute_KF_Gain_Decrease_Factor_CCA()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - a decrease factor in the range of [0,1]
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object_track - referense to a single object
   * const float32_t host_yaw_rate - Host yaw rate in radians
   * const F360_Calibrations_T& calib - reference to data structure containing calibration parameters
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
   * This function computes a decrease factor in the range of [0, 1]. This decrease
   * factor can be used to multiply sertain elements of the KF gain matrix with in
   * order to limit how much the position measurement is allowed to impact the object
   * velocity estimate.
   *
   * The decrease factor is dependent on the position innovation (i.e. the difference
   * between the time predicted object position and the position pseudo measurement).
   * If the innovation is smaller than the variable k_max_innov_dist_thres
   * then the decrease factor will be set to 1. If the innovation is between the
   * variables 0.0F and k_max_innov_dist_thres then
   * the decrease factor will decay linearly. The max innovation thresholds consist
   * of their respective constant calibrations and decays with host yaw rate.
   * When the innovation is larger than the calibration value k_max_innov_dist_thres
   * then the decreas factor will take the smallest value given by the calibration
   * parameter k_min_decreas_factor.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   float32_t Compute_KF_Gain_Decrease_Factor_CCA(
      const F360_Object_Track_T& object_track,
      const float32_t host_yaw_rate,
      const F360_Calibrations_T& calib)
   {
      // Limit how much position measurements are allowed to influence velocity estimates
      // for non-moveable objects with only stationary associated detections
      const float32_t innov_long_pos = object_track.vcs_position.x - object_track.pseudo_vcs_position.x;
      const float32_t innov_lat_pos = object_track.vcs_position.y - object_track.pseudo_vcs_position.y;
      const float32_t innov_dist = F360_Get_Hypotenuse(innov_long_pos, innov_lat_pos);

      const float32_t k_max_innov_dist_thres = std::max(0.0F, calib.k_max_innov_dist_yaw_rate_gain * std::abs(host_yaw_rate) + calib.k_max_innov_dist_thres);

      const float32_t decrease_factor = F360_Linear_Equation_With_Saturation(innov_dist, 0.0F, k_max_innov_dist_thres, 1.0F, calib.k_min_decrease_factor);

      return decrease_factor;
   }
}
