/*===================================================================================*\
* FILE: f360_nees_cfmi_hypotheses_selector.cpp
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
*   Hypotheses used for starting point for minimization of nees cost function
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

#include "f360_nees_cfmi_hypotheses_selector.h"

#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_inliers_check.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_nees_cfmi_ransac.h"
#include "f360_uncertainty_propagation.h"
#include "f360_nees_cfmi_hypothesis_confirm_pos_diff.h"
#include "f360_nees_cfmi_hypothesis_cloud.h"
#include "f360_nees_cfmi_hypothesis_dominant_velocity.h"
namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Velocity_Hypothesis_Selector
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
   * F360_NEES_CFMI_Velocity_T& vel_hyp,
   * const F360_NEES_CFMI_Vel_Hyp_Source_T vel_hyp_index,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Select algorithm to estimate (and estimate) velocity hypothesis based
   * on velocity hypothesis index
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_Velocity_Hypothesis_Selector(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Vel_Hyp_Source_T vel_hyp_index,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& vel_hyp)
   {
      switch (vel_hyp_index)
      {
      case F360_NEES_CFMI_VEL_HYP_SOURCE_CLOUD:
         Get_NEES_CFMI_Information_Cloud_VH(calibrations, nees_cfmi_information);
         Check_CV_Cloud_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);
         break;

      case F360_NEES_CFMI_VEL_HYP_SOURCE_CONF_VH:
         Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);
         Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);
         break;

      case F360_NEES_CFMI_VEL_HYP_SOURCE_DOMINANT:
         Check_CV_Dominant_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);
         break;

      case F360_NEES_CFMI_VEL_HYP_SOURCE_RANSAC:
         Get_CV_NEES_CFMI_RANSAC_Vel_Hyp(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);
         break;

      case F360_NEES_CFMI_VEL_HYP_SOURCE_RANGE_RATE:
         Check_CV_Radial_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);
         break;

      default:
         // Safe state: reset vel hypothesis
         vel_hyp = F360_NEES_CFMI_Velocity_T();
         break;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_CV_Dominant_NEES_CFMI_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   * F360_NEES_CFMI_Velocity_T& dominant_vel_hyp
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check Dominant Velocity
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_CV_Dominant_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& dominant_vel_hyp)
   {
      if (nees_cfmi_information.dominant_velocity.f_valid)
      {

         // Inliers checks
         NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information,
            nees_cfmi_init_info.inliers,
            nees_cfmi_information.dominant_velocity.vel,
            nees_cfmi_information.dominant_velocity.vel_cov,
            calibrations.k_nees_cfmi_vh_check_dom_sl_pd,
            calibrations.k_nees_cfmi_vh_check_dom_sl_cl,
            calibrations.k_nees_cfmi_vh_check_dom_sl_do,
            calibrations.k_nees_cfmi_min_weight_for_inlier);

         // Estimate velocity
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, dominant_vel_hyp);

         nees_cfmi_init_info.f_dominant_valid = dominant_vel_hyp.f_valid;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
   * F360_NEES_CFMI_Velocity_T& conf_pos_diff_vel_hyp
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check Confirmed Position Difference Hypothesis: hypothesis from confirmed position difference VH
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& conf_pos_diff_vel_hyp)
   {
      if (nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid)
      {

         // Inliers checks
         NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information,
            nees_cfmi_init_info.inliers,
            nees_cfmi_information.confirm_pos_diff_hyp_vel.vel,
            nees_cfmi_information.confirm_pos_diff_hyp_vel.vel_cov,
            calibrations.k_nees_cfmi_vh_check_cpd_sl_pd,
            calibrations.k_nees_cfmi_vh_check_cpd_sl_cl,
            calibrations.k_nees_cfmi_vh_check_cpd_sl_do,
            calibrations.k_nees_cfmi_min_weight_for_inlier);

         // Estimate velocity
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, conf_pos_diff_vel_hyp);

         nees_cfmi_init_info.f_confirm_pos_diff_hyp_vel_valid = conf_pos_diff_vel_hyp.f_valid;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_CV_Cloud_NEES_CFMI_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   * F360_NEES_CFMI_Velocity_T* cloud_vel_hyp,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check Cloud velocity hypothesis
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_CV_Cloud_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& cloud_vel_hyp)
   {
      if (nees_cfmi_information.cloud_hyp_vel.f_valid)
      {

         // Inliers checks
         NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information,
            nees_cfmi_init_info.inliers,
            nees_cfmi_information.cloud_hyp_vel.vel,
            nees_cfmi_information.cloud_hyp_vel.vel_cov,
            calibrations.k_nees_cfmi_vh_check_cloud_sl_pd,
            calibrations.k_nees_cfmi_vh_check_cloud_sl_cl,
            calibrations.k_nees_cfmi_vh_check_cloud_sl_do,
            calibrations.k_nees_cfmi_min_weight_for_inlier);

         // Estimate velocity
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, cloud_vel_hyp);

         nees_cfmi_init_info.f_cloud_vh_valid = cloud_vel_hyp.f_valid;
      }
   }


   /*===========================================================================*\
   * FUNCTION: Check_CV_Radial_NEES_CFMI_Hypothesis
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations,
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   * F360_NEES_CFMI_Velocity_T* radial_vel_hyp,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Check Cloud velocity hypothesis
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_CV_Radial_NEES_CFMI_Hypothesis(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Velocity_T& radial_vel_hyp)
   {
      if (nees_cfmi_information.radial_velocity.f_valid)
      {

         // Inliers checks
         NEES_CFMI_CV_Simple_NEES_Inliers_Check(nees_cfmi_information,
            nees_cfmi_init_info.inliers,
            nees_cfmi_information.radial_velocity.vel,
            calibrations.k_nees_cfmi_vh_check_radial_sl_pd,
            calibrations.k_nees_cfmi_vh_check_radial_sl_cl,
            calibrations.k_nees_cfmi_vh_check_radial_sl_do,
            calibrations.k_nees_cfmi_min_weight_for_inlier);

         // Estimate velocity
         Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, nees_cfmi_init_info.inliers, radial_vel_hyp);

         nees_cfmi_init_info.f_radial_vh_valid = radial_vel_hyp.f_valid;
      }
   }
}
