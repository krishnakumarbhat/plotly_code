/*===================================================================================*\
* FILE: f360_update_exist_prob_helpers.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains functions definitons used in Update_Existence_Probability()
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#include "f360_math_func.h"
#include "f360_update_exist_prob_helpers.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_Track_EP_With_New_Information()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t & p_real_obj_detected - probability that real object track is detected
   * const float32_t & p_clutter_detected - probability that clutter is detected
   * const float32_t & min_allowed_ep - minimum allowed existence probability (bottom saturation)
   * float32_t & ep - reference to existence probability of the object
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
   * This function updates existence probability value of the updated track with new information.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Track_EP_With_New_Information(
      const float32_t & p_real_obj_detected,
      const float32_t & p_clutter_detected,
      const float32_t & min_allowed_ep,
      float32_t & ep)
   {
      float32_t norm_coef = p_real_obj_detected * ep + p_clutter_detected * (F360_MAX_PROBABILITY - ep);
      norm_coef = std::max(norm_coef, F360_EPSILON);

      ep = (p_real_obj_detected / norm_coef) * ep;

      ep = F360_Saturate(ep, min_allowed_ep, F360_MAX_PROBABILITY);
   }

   /*===========================================================================*\
   * FUNCTION: Update_Track_EP_With_No_Information()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t & p_real_obj_detected - probability that real object track is detected
   * const float32_t & p_clutter_detected - probability that clutter is detected
   * const float32_t & min_allowed_ep - minimum allowed existence probability (bottom saturation)
   * float32_t & ep - reference to existence probability of the object
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
   * This function updates existence probability value of the track with no new information (no dets associated).
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Track_EP_With_No_Information(
      const float32_t & p_real_obj_detected,
      const float32_t & p_clutter_detected,
      const float32_t & min_allowed_ep,
      float32_t & ep)
   {
      float32_t norm_coef = (1.0F - p_real_obj_detected) * ep + (1.0F - p_clutter_detected) * (F360_MAX_PROBABILITY - ep);
      norm_coef = std::max(norm_coef, F360_EPSILON); // saturate from the bottom

      ep = ((1.0F - p_real_obj_detected) / norm_coef) * ep;

      ep = F360_Saturate(ep, min_allowed_ep, F360_MAX_PROBABILITY);
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_P_Track_State()
   *===========================================================================
   * RETURN VALUE:
   * float32_t p_det
   *
   * PARAMETERS:
   * const F360_Object_Track_T & obj - reference to an object track
   * const F360_Calibrations_T & calib - reference to calibrations struct
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates p_det describing the the quality of state estimate of the track.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Calculate_P_Track_State(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calib)
   {
      float32_t p_track_state;

      if (object_track.status == F360_OBJECT_STATUS_NEW_UPDATED)
      {
         p_track_state = calib.k_ep_prob_track_state_init_value;
      }
      else
      {
         const uint32_t actual_state_dimension = static_cast<uint32_t>(STATE_DIMENSION);
         float32_t s2[STATE_DIMENSION] = {}; // vector of state variances
         Normalize_State_Variances(object_track, calib, actual_state_dimension, s2);

         // calculate information, approximation of trace(inv(Covariance)) as sum(inv(Var_i))
         float32_t information_normalized = 0.0F;
         Normalize_Information(s2, calib, actual_state_dimension, information_normalized);

         information_normalized = information_normalized / (static_cast<float32_t>(actual_state_dimension));
         p_track_state = 1.0F - F360_Expf(-calib.k_ep_prob_track_state_exp_scale * (information_normalized + calib.k_ep_prob_track_state_exp_offset));
      }
      return p_track_state;
   }

   /*===========================================================================*\
   * FUNCTION: Normalize_State_Variances()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & obj - reference to an object track
   * const F360_Calibrations_T & calib - reference to calibrations struct
   * const uint32_t actual_state_dimension - actual length of state vector, model depending
   * float32_t (&s2)[STATE_DIMENSION] - reference to state variances array (vector)
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function normalizes state variance depend on object motion model (CTCA or CCA).
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Normalize_State_Variances(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calib,
      const uint32_t & actual_state_dimension,
      float32_t(&s2)[STATE_DIMENSION])
   {
      float32_t s2_th[STATE_DIMENSION] = { 1.0F,1.0F,1.0F,1.0F,1.0F,1.0F }; // initial state variance thresholds

      if (object_track.trk_fltr_type == F360_TRACKER_TRKFLTR_CTCA)
      {
         // update state variances
         for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
         {
            s2[i] = object_track.errcov[i][i];
         }

         s2_th[0] = calib.k_ep_variance_th_pos_xy;
         s2_th[1] = calib.k_ep_variance_th_pos_xy;
         s2_th[2] = calib.k_ep_variance_th_heading;
         s2_th[3] = calib.k_ep_variance_th_curvature;
         s2_th[4] = calib.k_ep_variance_th_velocity;
         s2_th[5] = calib.k_ep_variance_th_tan_accel;
      }
      else // CCA
      {
         for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
         {
            s2[i] = object_track.errcov[i][i];
         }

         s2_th[0] = calib.k_ep_variance_th_pos_xy;
         s2_th[1] = calib.k_ep_variance_th_velocity;
         s2_th[2] = calib.k_ep_variance_th_accel;
         s2_th[3] = calib.k_ep_variance_th_pos_xy;
         s2_th[4] = calib.k_ep_variance_th_velocity;
         s2_th[5] = calib.k_ep_variance_th_accel;
      }

      // normalize state variances
      Normalize_State(s2, s2_th, actual_state_dimension);
   }

   /*===========================================================================*\
   * FUNCTION: Normalize_State()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * float32_t(&s2)[STATE_DIMENSION],
   * const float32_t (&s2_th)[STATE_DIMENSION],
   * const uint32_t &actual_state_dimension)
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
   * This function normalizes state variance matrix.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Normalize_State(
      float32_t(&s2)[STATE_DIMENSION],
      const float32_t(&s2_th)[STATE_DIMENSION],
      const uint32_t & actual_state_dimension)
   {
      for (uint32_t i = 0U; i < actual_state_dimension; i++)
      {
         if (s2_th[i] > F360_MIN_DENOMINATOR)
         {
            s2[i] = s2[i] / s2_th[i];
         }
         else
         {
            s2[i] = INFTY;
         }
      }
   }

   /*===========================================================================*\
    * FUNCTION: Normalize_Information()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const float32_t(&s2)[STATE_DIMENSION],
    * const F360_Calibrations_T & calib,
    * const uint32_t &actual_state_dimension,
    * float32_t &information_normalized)
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
    * This function normalizes information.
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   void Normalize_Information(
      const float32_t(&s2)[STATE_DIMENSION],
      const F360_Calibrations_T & calib,
      const uint32_t & actual_state_dimension,
      float32_t & information_normalized)
   {
      for (uint32_t i = 0U; i < actual_state_dimension; i++)
      {
         const float32_t denominator = std::max(s2[i], calib.k_ep_bottom_saturation_of_normalized_variance);
         if (denominator > F360_MIN_DENOMINATOR)
         {
            information_normalized += 1.0F / denominator; // add inverse normalized state variance
         }
         else
         {
            information_normalized = INFTY;
            break;
         }
      }
   }
}
