/*===========================================================================*\
* FILE: f360_try_to_dealiase_range_rate.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Try_To_Dealiase_Range_Rate()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_try_to_dealiase_range_rate.h" 
#include "f360_math.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Try_To_Dealiase_Range_Rate()
   * ===========================================================================
   * RETURN VALUE:
   * bool f_dealiased                - True if range rate have been dealiased succesfully
   *
   * PARAMETERS:
   *   const float32_t rdot                 - Measured raw range rate to try for dealiasing (not compensated)
   *   const float32_t rdot_pred            - Predicted dealiased measured raw range rate 
   *   const float32_t threshold            - Threshold for matching prediction and dealiased range rate
   *   const float32_t rdot_interval_width  - Range rate interval width (property of the sensor)
   *   const float32_t min_aliased_rdot     - Min range rate the sensor can measure before aliasing happens (property of the sensor)
   *   float32_t & rdot_dealiased           - Dealiased range rate
   *   float32_t & interval                 - Interval used for dealiasing
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
   * This function tries to dealias a given range rate towards a prediction.
   * The prediction is evaluated against the range rate intervals of the sensor
   * and if the prediction is close to an interval limit it checks both
   * "adjacent" intervals. If the prediction is well within one interval
   * only the predicted interval is checked.
   * 
   * Note that the terms "upper" and "lower" intervals are being used.
   * For positive predictions the "upper" interval is predicted_interval + 1.
   * For negative predictions the "upper" interval is predicted_interval - 1.
   * And vice versa for "lower" interval.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Try_To_Dealiase_Range_Rate(
      const float32_t rdot,
      const float32_t rdot_pred,
      const float32_t threshold,
      const float32_t rdot_interval_width,
      const float32_t min_aliased_rdot,
      float32_t & rdot_dealiased,
      float32_t & rr_interval)
   {
      float32_t constant;
      float32_t sign;
      if (rdot_pred >= 0.0F)
      {
         constant = min_aliased_rdot;
         sign = 1.0F;
      }
      else
      {
         constant = min_aliased_rdot + rdot_interval_width;
         sign = -1.0F;
      }

      const float32_t one_over_width = 1.0F / rdot_interval_width;
      const float32_t frac_thres = threshold * one_over_width;
      const float32_t interval_frac = sign * (rdot_pred - constant) * one_over_width;

      // Get the integer part of interval fraction
      int32_t predicted_interval_int = static_cast<int32_t>(interval_frac);
      const float32_t predicted_interval = static_cast<float32_t>(predicted_interval_int);

      const float32_t hysteresis = interval_frac - predicted_interval;

      // If predicted range rate is close to an aliasing interval we need to make sure
      // to check both "adjacent" intervals
      int32_t start_interval;
      if (hysteresis < frac_thres)
      {
         // Do two checks, lower and predicted interval
         start_interval = predicted_interval_int - 1;
      }
      else if (hysteresis > (1.0F - frac_thres))
      {
         // Do two checks, predicted and upper interval
         start_interval = predicted_interval_int;
         predicted_interval_int++;
      }
      else
      {
         // Predicted range rate is well within one interval, only one check needed
         start_interval = predicted_interval_int;
      }
      
      bool f_dealiased = false;
      for (int32_t i = start_interval; i <= predicted_interval_int; i++)
      {
         const float32_t current_interval = static_cast<float32_t>(i);
         const float32_t rdot_dealias_temp = (rdot + sign * current_interval * rdot_interval_width);

         const float32_t delta = rdot_pred - rdot_dealias_temp;

         if (std::abs(delta) < threshold)
         {
            rdot_dealiased = rdot_dealias_temp;
            rr_interval = sign * current_interval;
            f_dealiased = true;
            break;
         }
      }

      return f_dealiased;

   }
}

