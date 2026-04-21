/*===================================================================================*\
* FILE: f360_calc_object_init_ep.cpp
*============================================================================
* Copyright ? 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential ? Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition for Calc_Object_Init_Existence_Prob.
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [30-Mar-2018]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/
/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_calc_object_init_ep.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Calc_Object_Init_Existence_Prob()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - inital existence probability
   *
   * PARAMETERS:
   * const float32_t cluster_ep - cluster (from which object is created) existence probability
   * const float32_t cluster_VCS_velocity_plausability - cluster velocity estimation plausability
   * const F360_Calibrations_T &calib - tracker calibrations
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
   * This function calculates inital value of existence probability.
   *
   * PRECONDITIONS:
   * cluster_ep in range <0, 1>
   * cluster_VCS_velocity_plausability in range <0, 1>
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calc_Object_Init_Existence_Prob(
      const float32_t cluster_ep,
      const float32_t cluster_VCS_velocity_plausability,
      const F360_Calibrations_T &calib)
   {
      const float32_t penalty_factor = F360_Power_Scale_Prob(cluster_VCS_velocity_plausability, calib.k_ep_init_factor_exponent);
      const float32_t saturated_penalty_factor = F360_Saturate(penalty_factor, calib.k_ep_init_factor_min_val, calib.k_ep_init_factor_max_val);

      return F360_Saturate(cluster_ep * saturated_penalty_factor, F360_MIN_PROBABILITY, F360_MAX_PROBABILITY);
   }
}
