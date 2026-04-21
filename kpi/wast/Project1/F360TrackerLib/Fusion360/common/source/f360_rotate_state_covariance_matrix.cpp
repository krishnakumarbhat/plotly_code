/*===================================================================================*\
* FILE: f360_rotate_state_covariance_matrix.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
* This file contains declarations of funcions for adjusting/rotating state covariance matrix 
* when changing coordinate system for the state vector into a new system with different 
* orientation.
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
\*===================================================================================*/
#include "f360_rotate_state_covariance_matrix.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Rotate_State_Covariance_Matrix()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Trk_Fltr_Type_T trk_fltr_type - The track filter type of the object whos state vector is transformed.
   * const float32_t(&cov_mat)[STATE_DIMENSION][STATE_DIMENSION] - The state covariance matrix of the objects whos state vector is transformed.
   * const float32_t cos_rot_angle - Cosine of the rotation angle. The rotation angle is defined as the angle the base axis of the coordinate system in which the state vector is expressed needs to rotate in order to be aligned with the new coordinate system.
   * const float32_t sin_rot_angle - Sine of the rotation angle. The rotation angle is defined as the angle the base axis of the coordinate system in which the state vector is expressed needs to rotate in order to be aligned with the new coordinate system.
   * float32_t(&rotated_cov_mat)[STATE_DIMENSION][STATE_DIMENSION] - The covariance matrix of the transformed state vector
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * The object state vector is tracked in a well defined coordinate system. 
   * When wanting to express the state vector in some other coordinate system,
   * the covariance matrix of the state vector also has to be expressed in the same
   * system. This function computes the covariance of an object state vector after 
   * a change of basis from one cartesian coordinate system to another with a different
   * rotation (for example a change of basis from VCS to WCS but function also works
   * for other rotations)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Rotate_State_Covariance_Matrix(
      const F360_Trk_Fltr_Type_T trk_fltr_type,
      const float32_t(&cov_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t cos_rot_angle,
      const float32_t sin_rot_angle,
      float32_t(&rotated_cov_mat)[STATE_DIMENSION][STATE_DIMENSION])
   {
      float32_t rotation_jacobian[STATE_DIMENSION][STATE_DIMENSION] = {}; // This array needs to be initialized to zeros otherwise Get_Rotation_Jacobian() does not work
      Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, rotation_jacobian);

      float32_t temp_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
      F360_matmul_6x6_6x6(rotation_jacobian, cov_mat, temp_mat);
      F360_matmul_6x6_6x6T(temp_mat, rotation_jacobian, rotated_cov_mat);
   }

   /*===========================================================================*\
   * FUNCTION: Get_Rotation_Jacobian()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Trk_Fltr_Type_T & trk_fltr_type - the track filter type of the object whos state vector is transformed
   * const float32_t cos_rot_angle - cosine of the rotation angle. The rotation angle is defined as the angle the base axis of the coordinate system in which the state vector is expressed needs to rotate in order to be aligned with the new coordinate system
   * const float32_t sin_rot_angle - sine of the rotation angle. The rotation angle is defined as the angle the base axis of the coordinate system in which the state vector is expressed needs to rotate in order to be aligned with the new coordinate system
   * float32_t (&jacobian)[STATE_DIMENSION][STATE_DIMENSION] - a reference to the jacobian
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function returns the Jacobian of a the transformation of an object 
   * state vector when changing basis to another coordinate system with a different
   * rotation (for example a change of basis from VCS to WCS but function also works
   * for other rotations)
   *
   * PRECONDITIONS:
   * The parameter jacobian must have previously been initialized with zeros for all
   * elements.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Rotation_Jacobian(
      const F360_Trk_Fltr_Type_T trk_fltr_type,
      const float32_t cos_rot_angle,
      const float32_t sin_rot_angle,
      float32_t (&jacobian)[STATE_DIMENSION][STATE_DIMENSION])
   {
      if (F360_TRACKER_TRKFLTR_CTCA == trk_fltr_type)
      {
         // Position WCS to VCS rotation
         jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = cos_rot_angle;
         jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = sin_rot_angle;
         jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = -sin_rot_angle;
         jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = cos_rot_angle;
         // Heading, curvature, speed and tangential acceleration are unaffected by WCS to VCS rotation 
         jacobian[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_H] = 1.0F;
         jacobian[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_C] = 1.0F;
         jacobian[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S] = 1.0F;
         jacobian[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_A] = 1.0F;
      }
      else
      {
         // Position WCS to VCS rotation
         jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = cos_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = -sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = cos_rot_angle;
         // Velocity WCS to VCS rotation
         jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = cos_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = -sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = cos_rot_angle;
         // Acceleration WCS to VCS.
         jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = cos_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = -sin_rot_angle;
         jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = cos_rot_angle;
      }
   }
}
