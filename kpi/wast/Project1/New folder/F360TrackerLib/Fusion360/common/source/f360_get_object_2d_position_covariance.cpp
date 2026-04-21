/*===========================================================================*\
* FILE: f360_get_object_2d_position_covariance.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of F360_Get_Object_2D_Position_Covariance()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_get_object_2d_position_covariance.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_matrix_dimension.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: F360_Get_Object_2D_Position_Covariance()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T& obj
   * float32_t covar[2U][2U]
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
   * This function extracts and returns the position covariance from the objects state covariance matrix
   *
   * PRECONDITIONS:
   * All  pointer should point to valid structure
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Get_Object_2D_Position_Covariance(
      const F360_Object_Track_T& obj,
      float32_t(&covar)[2U][2U])
   {
      if (F360_TRACKER_TRKFLTR_CTCA == obj.trk_fltr_type)
      {
         covar[F360_2D_IDX_X][F360_2D_IDX_X] = obj.errcov[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X];
         covar[F360_2D_IDX_X][F360_2D_IDX_Y] = obj.errcov[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
         covar[F360_2D_IDX_Y][F360_2D_IDX_X] = obj.errcov[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X];
         covar[F360_2D_IDX_Y][F360_2D_IDX_Y] = obj.errcov[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y];
      }
      else
      {
         covar[F360_2D_IDX_X][F360_2D_IDX_X] = obj.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X];
         covar[F360_2D_IDX_X][F360_2D_IDX_Y] = obj.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y];
         covar[F360_2D_IDX_Y][F360_2D_IDX_X] = obj.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X];
         covar[F360_2D_IDX_Y][F360_2D_IDX_Y] = obj.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y];
      }
   }
}

