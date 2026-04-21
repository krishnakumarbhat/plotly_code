/*===================================================================================*\
* FILE: f360_uncertainty_propagation.h
*====================================================================================
* Copyright - 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*  Function declarations for uncertainty propagation functions
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_UNCERTAINTY_PROPAGATION_H
#define F360_UNCERTAINTY_PROPAGATION_H

#include "f360_reuse.h"
#include "f360_matrix_dimension.h"
namespace f360_variant_A
{
   void Uncertainty_Propagation_2d(const float32_t (&jacobian)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t (&input_variance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      float32_t (&output_variance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Add_Uncertainty_2d(const float32_t (&cov_A)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t (&cov_B)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      float32_t (&output_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Add_Into_Uncertainty_2d(const float32_t (&input_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      float32_t (&output_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Constant_Uncertainty_Propagation_2d(const float32_t constant,
      float32_t (&covariance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Reset_2d_Covariance(float32_t (&covariance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Init_2d_Covariance_By_Std(const float32_t constant_std,
      float32_t (&covariance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   float32_t Trace_Of_2d_Covariance(const float32_t(&covariance)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Jacobian_2D_Cart2Pol(const float32_t x,
      const float32_t y,
      float32_t (&jacobian)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   float32_t Vel_Cov_2_Range_Rate_Var(const float32_t (&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t cos_azimuth,
      const float32_t sin_azimuth);
}
#endif


