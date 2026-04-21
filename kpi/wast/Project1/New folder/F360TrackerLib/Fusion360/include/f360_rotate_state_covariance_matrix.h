/*===================================================================================*\
* FILE: f360_rotate_state_covariance_matrix.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
* This file contains definitions of funcions for adjusting/rotating state covariance matrix 
* when changing coordinate system for the state vector into a new system with different 
* orientation.
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
\*===================================================================================*/
#ifndef F360_ROTATE_STATE_COVARIANCE_MATRIX_H
#define F360_ROTATE_STATE_COVARIANCE_MATRIX_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_trk_fltr_type.h"

namespace f360_variant_A
{
   void Get_Rotation_Jacobian(
      const F360_Trk_Fltr_Type_T trk_fltr_type,
      const float32_t cos_rot_angle,
      const float32_t sin_rot_angle,
      float32_t (&jacobian)[STATE_DIMENSION][STATE_DIMENSION]);

   void Rotate_State_Covariance_Matrix(
      const F360_Trk_Fltr_Type_T trk_fltr_type,
      const float32_t(&cov_mat)[STATE_DIMENSION][STATE_DIMENSION],
      const float32_t cos_rot_angle,
      const float32_t sin_rot_angle,
      float32_t(&rotated_cov_mat)[STATE_DIMENSION][STATE_DIMENSION]);
}
#endif
