/*===================================================================================*\
* FILE: f360_nees_cfmi_structs.cpp
* ====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
* -----------------------------------------------------------------------------------------
* DESCRIPTION:
* NEES CFMI structs management member functions
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: F360_NEES_CFMI_Velocity_T::Reset_Plausibility
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Reset plausibility related part of parent struct
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_NEES_CFMI_Velocity_T::Reset_Plausibility()
   {
      f_plausible = false;

      num_all = 0U;
      num_cloud = 0U;
      num_pos_diff = 0U;

      weight_sum = 0.0F;
      weight_dets_sum = 0.0F;
      weight_vels_sum = 0.0F;

      plausibility = 0.0F;
      plausibility_cloud = 0.0F;
      plausibility_pos_diff = 0.0F;

      information_cloud = 0.0F;
      information_pos_diff = 0.0F;
   }
}

