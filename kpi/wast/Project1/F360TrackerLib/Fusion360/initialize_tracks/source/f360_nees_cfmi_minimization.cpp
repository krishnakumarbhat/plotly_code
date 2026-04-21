/*===================================================================================*\
* FILE: f360_nees_cfmi_minimization.cpp
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   Function for minimization of defined NEES cost function
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
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_minimization.h"

#include "f360_nees_cfmi_helpers.h"

namespace f360_variant_A
{
   static void Calc_Vel_For_NEES_CFMI_Velocity(F360_NEES_CFMI_Velocity_T& velocity);

   /*===========================================================================*\
   * FUNCTION: Estimate_Vel_By_CV_NEES_Min_Analytical
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * const F360_NEES_CFMI_Inliers_T& inliers,
   * F360_NEES_CFMI_Velocity_T& velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Analytical solution for Constant Velocity NEES cost function estimation
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Estimate_Vel_By_CV_NEES_Min_Analytical(const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Inliers_T& inliers,
      F360_NEES_CFMI_Velocity_T& velocity)
   {
      const uint32_t num_dets = nees_cfmi_information.dets_num;
      const uint32_t num_vels = nees_cfmi_information.vels_num;

      if ((num_dets <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET) &&
         (num_vels <= F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL))
      {
         // For each detection
         for (uint32_t index = 0U; index < num_dets; index++)
         {
            // Inliers checks
            if (inliers.f_dets_valid[index])
            {
               const F360_NEES_CFMI_Detection_T& single_det = nees_cfmi_information.detections[index];
               Add_Into_NEES_CFMI_Determinant_Weighted(velocity.determinants, single_det.cloud_determinants, inliers.det_weights[index]);
            }
         }

         // For each velocity
         for (uint32_t index = 0U; index < num_vels; index++)
         {
            // Inliers checks
            if (inliers.f_vels_valid[index])
            {
               const F360_NEES_CFMI_Pos_Diff_Velocity_T& vel = nees_cfmi_information.velocities[index];
               Add_Into_NEES_CFMI_Determinant_Weighted(velocity.determinants, vel.pos_diff_determinants, inliers.vel_weights[index]);
            }
         }

         Calc_Vel_For_NEES_CFMI_Velocity(velocity);
      }
      else
      {
         velocity = F360_NEES_CFMI_Velocity_T();
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Vel_For_NEES_CFMI_Velocity
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_NEES_CFMI_Velocity_T& velocity
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate velocity (if possible) based on nees velocity info
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Calc_Vel_For_NEES_CFMI_Velocity(F360_NEES_CFMI_Velocity_T& velocity)
   {
      if (velocity.determinants.n_dets > 1U)
      {
         velocity.V_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(velocity.determinants);
         if (velocity.V_determinant > F360_MIN_DENOMINATOR)
         {
            velocity.f_valid = true;
            velocity.vel.longitudinal = ((velocity.determinants.Sx * velocity.determinants.Syy) - (velocity.determinants.Sxy * velocity.determinants.Sy)) / velocity.V_determinant;
            velocity.vel.lateral = ((velocity.determinants.Sxx * velocity.determinants.Sy) - (velocity.determinants.Sx * velocity.determinants.Syx)) / velocity.V_determinant;
         }
         else
         {
            // Initialization not possible
            velocity.f_valid = false;
         }
      }
      else
      {
         // Initialization not possible
         velocity.f_valid = false;
      }
   }

}

