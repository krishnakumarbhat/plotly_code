/*===================================================================================*\
* FILE: f360_uncertainty_propagation.cpp
*====================================================================================
* Copyright - 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   Function definitions for uncertainty propagation functions
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

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_uncertainty_propagation.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * Internal Function Prototypes
   \*===========================================================================*/


   /*===========================================================================*\
   * Global Function Definitions
   \*===========================================================================*/

   /*===========================================================================*\
   * FUNCTION: Uncertainty_Propagation_2d()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * float32_t (&jacobian)[2][2],
   * const float32_t (&input_variance)[2][2],
   * float32_t (&output_variance)[2][2]
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
   * Propagate uncertainty using 2d matrix uncertainty propagation
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Uncertainty_Propagation_2d(const float32_t (&jacobian)[2][2],
      const float32_t (&input_variance)[2][2],
      float32_t (&output_variance)[2][2])
   {
      float32_t temp_array[2][2];
      float32_t sum_of_products;

      // Calculate J*E*J'
      // First calculate temp_array = J*E.

      // TODO: DFU-806
      for (uint32_t i = 0U; i < 2U; i++)
      {
         for (uint32_t j = 0U; j < 2U; j++)
         {
            sum_of_products = 0.0F;
            for (uint32_t k = 0U; k < 2U; k++)
            {
               sum_of_products += (jacobian[i][k]) * (input_variance[k][j]);
            }
            temp_array[i][j] = sum_of_products;
         }
      }

      // E_out = J*E*J' = temp_array*J'
      for (uint32_t i = 0U; i < 2U; i++)
      {
         for (uint32_t j = 0U; j < 2U; j++)
         {
            sum_of_products = 0.0F;
            for (uint32_t k = 0U; k < 2U; k++)
            {
               // J'[k][j] = J[j][k]
               sum_of_products += temp_array[i][k] * (jacobian[j][k]);
            }
            output_variance[i][j] = sum_of_products;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Add_Uncertainty_2d()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&cov_A)[2][2],
   * const float32_t (&cov_B)[2][2],
   * float32_t (&output_cov)[2][2]
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
   * Add two 2d uncertainties.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Add_Uncertainty_2d(const float32_t (&cov_A)[2][2],
      const float32_t (&cov_B)[2][2],
      float32_t (&output_cov)[2][2])
   {
      for (uint32_t i = 0U; i < 2U; i++)
      {
         for (uint32_t j = 0U; j < 2U; j++)
         {
            output_cov[i][j] = cov_A[i][j] + cov_B[i][j];
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Add_Into_Uncertainty_2d()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&input_cov)[2][2],
   * float32_t (&output_cov)[2][2]
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
   * Add into 2d uncertainty another input uncertainty
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Add_Into_Uncertainty_2d(const float32_t (&input_cov)[2][2],
      float32_t (&output_cov)[2][2])
   {
      for (uint32_t  i = 0U; i < 2U; i++)
      {
         for (uint32_t j = 0U; j < 2U; j++)
         {
            output_cov[i][j] += input_cov[i][j];
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Constant_Uncertainty_Propagation_2d()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t constant
   * float32_t (&covariance)[2][2]
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
   * Multiply uncertainty by constant
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Constant_Uncertainty_Propagation_2d(const float32_t constant,
      float32_t (&covariance)[2][2])
   {
      const float32_t constant_square = constant * constant;

      for (uint8_t i = 0U; i < 2U; i++)
      {
         for (uint8_t j = 0U; j < 2U; j++)
         {
            covariance[i][j] *= constant_square;
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Reset_2d_Covariance()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * float32_t (&covariance)[2][2]
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
   * Reset Covariance values
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Reset_2d_Covariance(float32_t (&covariance)[2][2])
   {
      for (uint32_t i = 0U; i < 2U; i++)
      {
         for (uint32_t j = 0U; j < 2U; j++)
         {
            covariance[i][j] *= 0.0F;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Init_2d_Covariance_By_Std()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t constant_std
   * float32_t (&covariance)[2][2]
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
   * Initialize covariance 2d matrix by standard deviation
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Init_2d_Covariance_By_Std(const float32_t constant_std,
      float32_t (&covariance)[2][2])
   {
      const float32_t constant_square = constant_std * constant_std;

      covariance[F360_2D_IDX_X][F360_2D_IDX_X] = constant_square;
      covariance[F360_2D_IDX_Y][F360_2D_IDX_X] = 0.0F;
      covariance[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.0F;
      covariance[F360_2D_IDX_Y][F360_2D_IDX_Y] = constant_square;
   }

   /*===========================================================================*\
   * FUNCTION: Trace_Of_2d_Covariance()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&covariance)[2][2]
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
   * Calculate trace of 2d covariance matrix
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Trace_Of_2d_Covariance(const float32_t (&covariance)[2][2])
   {
      return covariance[F360_2D_IDX_X][F360_2D_IDX_X] + covariance[F360_2D_IDX_Y][F360_2D_IDX_Y];
   }

   /*===========================================================================*\
   * FUNCTION: Jacobian_2D_Cart2Pol()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t X
   * const float32_t Y
   * float32_t (&jacobian)[2][2]
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
   * Calculate Jacobian for 2D Cartesian to polar transformation
   * j[0][0] - magnitude
   * j[1][1] - angle
   * M = sqrt(x^2+y^2)
   * A = anat2(y,x)
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * All pointers are valid.
   *
   \*===========================================================================*/
   void Jacobian_2D_Cart2Pol(const float32_t x,
      const float32_t y,
      float32_t (&jacobian)[2][2])
   {
      const float32_t magnitude_squared = (x * x) + (y * y);
      if (magnitude_squared > F360_MIN_DENOMINATOR)
      {
         const float32_t magnitude = F360_Sqrtf(magnitude_squared);

         jacobian[F360_2D_IDX_X][F360_2D_IDX_X] = x / magnitude;
         jacobian[F360_2D_IDX_X][F360_2D_IDX_Y] = y / magnitude;
         jacobian[F360_2D_IDX_Y][F360_2D_IDX_X] = -y / magnitude_squared;
         jacobian[F360_2D_IDX_Y][F360_2D_IDX_Y] = x / magnitude_squared;
      }
      else
      {
         // Safe state
         jacobian[F360_2D_IDX_X][F360_2D_IDX_X] = 1.0F;
         jacobian[F360_2D_IDX_X][F360_2D_IDX_Y] = 1.0F;
         jacobian[F360_2D_IDX_Y][F360_2D_IDX_X] = 1.0F;
         jacobian[F360_2D_IDX_Y][F360_2D_IDX_Y] = 1.0F;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Vel_Cov_2_Range_Rate_Var
   *===========================================================================
   * RETURN VALUE:
   * float32_t range_rate_var
   *
   * PARAMETERS:
   *
   *   const float32_t (&vel_hyp_cov)[2][2],
   *   const float32_t cos_azimuth,
   *   const float32_t sin_azimuth
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Propagate velocity covaraince into range rate variance
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Vel_Cov_2_Range_Rate_Var(const float32_t (&vel_hyp_cov)[2][2],
      const float32_t cos_azimuth,
      const float32_t sin_azimuth)
   {
      float32_t range_rate_var;

      range_rate_var = ((cos_azimuth * cos_azimuth) * vel_hyp_cov[F360_2D_IDX_X][F360_2D_IDX_X]) +
         ((sin_azimuth * sin_azimuth) * vel_hyp_cov[F360_2D_IDX_Y][F360_2D_IDX_Y]) +
         ((2.0F) * (sin_azimuth * cos_azimuth) * vel_hyp_cov[F360_2D_IDX_X][F360_2D_IDX_Y]);

      return range_rate_var;
   }

   /*===========================================================================*\
   * Internal Function Definitions
   \*===========================================================================*/
}


