/*===================================================================================*\
* FILE: f360_nees_cfmi_preconditions.cpp
*====================================================================================
* Copyright 2020 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
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
*   Precondition functions for NEES cost function initialization
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
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/


#include "f360_nees_cfmi_preconditions.h"

#include <algorithm>
#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_uncertainty_propagation.h"
#include "f360_iterator.h"

namespace f360_variant_A
{
   static float32_t Extract_Max_Cov_Trace(
      const F360_Calibrations_T& calibrations,
      const int16_t n_all_dets);

   static bool  Is_Cov_Trace_Precondition_Valid(
      const F360_Calibrations_T& calibrations,
      const float32_t precondition_cov_trace,
      const int16_t n_all_dets);

   static float32_t Calculate_Precondition_Cov_Trace(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T& tracker_info);

   static float32_t Calculate_Precondition_Cov_Trace_For_Cloud(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Tracker_Info_T& tracker_info);

   static void Update_Determinants_By_Det(
      const float32_t range_rate_comp_std,
      const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t det_sin,
      const float32_t det_cos,
      F360_NEES_CFMI_Determinants_T& determinants);

   static void Update_Determinats_By_Det_Range_Rate(
      const float32_t range_rate_comp_std,
      const float32_t det_sin,
      const float32_t det_cos,
      F360_NEES_CFMI_Determinants_T& determinants);

   static void Update_Determinants_By_Det_Pos(
      const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      F360_NEES_CFMI_Determinants_T& determinants);

   static float32_t Calc_Velocity_Cov_Trace(F360_NEES_CFMI_Determinants_T& determinants);

   static bool Is_Det_Index_Valid(const int16_t det_index, const uint16_t max_dets_num);

   static bool Is_Num_Of_Dets_Correct(const int16_t num_dets, const uint8_t num_hist_dets_in_track);

   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Preconditions
   *===========================================================================
   * RETURN VALUE:
   * bool - indication that preconditions are valid.
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations
   * const F360_Object_Track_T& cluster_to_init
   * const F360_Detection_Props_T& det_props
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Detection_Hist_T& detection_hist
   * const F360_Tracker_Info_T &tracker_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Execute fast preconditions to check if initialization should be called.
   * For low number of detections it need to be checked if coarse covariance
   * estimation is low enough to have estimation above noise ratio.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool NEES_CFMI_Preconditions(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T& tracker_info)
   {
      const int16_t n_all_dets = (cluster_to_init.ndets + cluster_to_init.num_old_dets);

      bool f_preconditions_valid;

      // Check for low number of detections
      if (n_all_dets <= calibrations.k_nees_cfmi_extended_prec_max_dets)
      {
         // Preconditions based on raw estimated information
         float32_t precondition_cov_trace;
         if (cluster_to_init.num_old_dets > 0)
         {
            precondition_cov_trace = Calculate_Precondition_Cov_Trace(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);
         }
         else
         {
            precondition_cov_trace = Calculate_Precondition_Cov_Trace_For_Cloud(calibrations, cluster_to_init, raw_detection_list, tracker_info);
         }
         f_preconditions_valid = Is_Cov_Trace_Precondition_Valid(calibrations, precondition_cov_trace, n_all_dets);
      }
      else
      {
         f_preconditions_valid = true;
      }
      return f_preconditions_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Is_Cov_Trace_Precondition_Valid
   *===========================================================================
   * RETURN VALUE:
   * float32_t f_precondition_valid
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations,
   * const float32_t precondition_cov_trace,
   * const int16_t n_all_dets
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Checks if calculated maximum covariance trace is greater than
   * passed covariation trace.
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool  Is_Cov_Trace_Precondition_Valid(
      const F360_Calibrations_T& calibrations,
      const float32_t precondition_cov_trace,
      const int16_t n_all_dets)
   {
      const float32_t max_valid_cov_trace = Extract_Max_Cov_Trace(calibrations, n_all_dets);
      return (precondition_cov_trace < max_valid_cov_trace);
   }

   /*===========================================================================*\
   * FUNCTION: Extract_Max_Cov_Trace
   *===========================================================================
   * RETURN VALUE:
   * float32_t Maximum valid trace of covariance
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations
   * const int16_t n_all_dets,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Extract maximum valid covariance for give number of detections from calibration structure
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static float32_t Extract_Max_Cov_Trace(
      const F360_Calibrations_T& calibrations,
      const int16_t n_all_dets)
   {
      float32_t max_valid_cov_trace;
      switch (n_all_dets)
      {
      case 0:case 1:
         max_valid_cov_trace = F360_EPSILON;
         break;
      case 2:
         max_valid_cov_trace = calibrations.k_nees_cfmi_extended_prec_2_dets_max_std * calibrations.k_nees_cfmi_extended_prec_2_dets_max_std;
         break;
      case 3:
         max_valid_cov_trace = calibrations.k_nees_cfmi_extended_prec_3_dets_max_std * calibrations.k_nees_cfmi_extended_prec_3_dets_max_std;
         break;
      case 4:
         max_valid_cov_trace = calibrations.k_nees_cfmi_extended_prec_4_dets_max_std * calibrations.k_nees_cfmi_extended_prec_4_dets_max_std;
         break;
      case 5:
         max_valid_cov_trace = calibrations.k_nees_cfmi_extended_prec_5_dets_max_std * calibrations.k_nees_cfmi_extended_prec_5_dets_max_std;
         break;
      default:
         max_valid_cov_trace = calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std * calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std;
         break;
      }
      return max_valid_cov_trace;
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Precondition_Cov_Trace
   *===========================================================================
   * RETURN VALUE:
   * float32_t Trace of covariance
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations
   * const F360_Cluster_T& cluster_to_init
   * const const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Detection_Hist_T& detection_hist
   * const F360_Tracker_Info_T &tracker_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate initial raw trace of covariance from detections -> not precise,
   * but fast and gives good indication about quality of detection.
   * Linear equation determinants are calculated for cloud and position difference,
   * For cloud constant range rate variance is assumed,
   * for position difference source velocity is not even calculated,
   * just covariance of detections is used for right side determinants calculation
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static float32_t Calculate_Precondition_Cov_Trace(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T &tracker_info)
   {
      F360_NEES_CFMI_Determinants_T determinants = {};

      // Fast calculation of determinants to calculate raw, preliminary variance
      if (Is_Num_Of_Dets_Correct(cluster_to_init.ndets, tracker_info.variant.num_dets_in_track))
      {
         // For each detection
         for (int16_t index = 0; index < cluster_to_init.ndets; index++)
         {
            const int16_t det_index = cluster_to_init.detids[index] - 1;
            const bool f_detection_index_valid = Is_Det_Index_Valid(det_index, MAX_NUMBER_OF_DETECTIONS);
            if (f_detection_index_valid)
            {
               const F360_Detection_Props_T& curr_det = (det_props[det_index]);
               const rspp_variant_A::RSPP_Detection_T& curr_det_raw = raw_detection_list.detections[det_index];
               Update_Determinants_By_Det(calibrations.k_nees_cfmi_rrate_comp_std, curr_det.position_cov_nees, curr_det_raw.processed.sin_vcs_az, curr_det_raw.processed.cos_vcs_az, determinants);
            }
         }
      }

      // For each historical detection
      if (Is_Num_Of_Dets_Correct(cluster_to_init.num_old_dets, tracker_info.variant.num_dets_in_track))
      {
         for (int16_t index = 0; index < cluster_to_init.num_old_dets; index++)
         {
            const int16_t det_index = cluster_to_init.old_det_idx[index];
            if (Is_Det_Index_Valid(det_index, tracker_info.variant.num_hist_dets))
            {
               const F360_Detection_Hist_Data_T& curr_hist_det = (detection_hist.det_data[det_index]);
               const float32_t sin_az = F360_Sinf(curr_hist_det.vcs_az);
               const float32_t cos_az = F360_Cosf(curr_hist_det.vcs_az);
               Update_Determinants_By_Det(calibrations.k_nees_cfmi_rrate_comp_std, curr_hist_det.position_cov_nees, sin_az, cos_az, determinants);
            }
         }
      }

      return Calc_Velocity_Cov_Trace(determinants);
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Precondition_Cov_Trace_For_Cloud
   *===========================================================================
   * RETURN VALUE:
   * float32_t Trace of covariance
   *
   * PARAMETERS:
   *
   * const F360_Calibrations_T& calibrations
   * const F360_Cluster_T& cluster_to_init
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
   * const F360_Tracker_Info_T& tracker_info
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculate initial raw information from detections -> not precise,
   * but fast and gives good indication about quality of detection.
   * linear equation determinants are calculated for cloud with constant
   * range rate variance assumed.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static float32_t Calculate_Precondition_Cov_Trace_For_Cloud(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Tracker_Info_T& tracker_info)
   {
      F360_NEES_CFMI_Determinants_T determinants = {};

      // Fast calculation of determinants to calculate check raw, preliminary variance

      // For each detections
      if (Is_Num_Of_Dets_Correct(cluster_to_init.ndets, tracker_info.variant.num_dets_in_track))
      {
         for (int16_t index = 0; index < cluster_to_init.ndets; index++)
         {
            const int16_t det_index = cluster_to_init.detids[index] - 1;
            if (Is_Det_Index_Valid(det_index, MAX_NUMBER_OF_DETECTIONS))
            {
               const rspp_variant_A::RSPP_Detection_T& curr_det_raw = raw_detection_list.detections[det_index];
               Update_Determinats_By_Det_Range_Rate(calibrations.k_nees_cfmi_rrate_comp_std,
                  curr_det_raw.processed.sin_vcs_az, curr_det_raw.processed.cos_vcs_az, determinants);
            }
         }
      }
      return  Calc_Velocity_Cov_Trace(determinants);
   }

   /*===========================================================================*\
   * FUNCTION: Update_Determinants_By_Det
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *  float32_t range_rate_comp_std,
   *  const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
   *  const float32_t det_sin,
   *  const float32_t det_cos
   *  F360_NEES_CFMI_Determinants_T& determinants,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function updates determinant values for passed detection signals.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Determinants_By_Det(
      const float32_t range_rate_comp_std,
      const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t det_sin,
      const float32_t det_cos,
      F360_NEES_CFMI_Determinants_T& determinants)
   {
      Update_Determinats_By_Det_Range_Rate(range_rate_comp_std,  det_sin, det_cos, determinants);
      Update_Determinants_By_Det_Pos(position_cov_nees, determinants);
   }

   /*===========================================================================*\
   * FUNCTION: Update_Determinats_By_Det_Range_Rate
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * float32_t range_rate_comp_std
   * const float32_t det_sin,
   * const float32_t det_cos,
   * F360_NEES_CFMI_Determinants_T& determinants,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function updates determinant values by range rate component
   * for passed detection signals.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Determinats_By_Det_Range_Rate(const float32_t range_rate_comp_std,
      const float32_t det_sin,
      const float32_t det_cos,
      F360_NEES_CFMI_Determinants_T& determinants)
   {
      const float32_t range_rate_comp_var = range_rate_comp_std * range_rate_comp_std;
      if (range_rate_comp_var > F360_EPSILON)
      {
         const float32_t range_rate_comp_influence = 1.0F / range_rate_comp_var;

         determinants.n_dets++;
         determinants.Sxx += det_cos * det_cos * range_rate_comp_influence;
         determinants.Sxy += det_cos * det_sin * range_rate_comp_influence;
         determinants.Syy += det_sin * det_sin * range_rate_comp_influence;
      }
      else
      {
         /* Safe state, determinants not updated. */
      }

   }

   /*===========================================================================*\
   * FUNCTION: Update_Determinants_By_Det_Pos
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]
   * F360_NEES_CFMI_Determinants_T& determinants,
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function updates determinant values by position covariance component
   * for passed detection signals.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Determinants_By_Det_Pos(
      const float32_t(&position_cov_nees)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      F360_NEES_CFMI_Determinants_T& determinants)
   {
      float32_t pos_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION];
      const float32_t dt = 0.05F;
      const float32_t vel_cov_const = 1.0F / dt; // Constant for calculating 'velocity variance'

      (void)std::copy(cmn::begin(position_cov_nees), cmn::end(position_cov_nees), cmn::begin(pos_cov));

      Constant_Uncertainty_Propagation_2d(vel_cov_const, pos_cov);
      const float32_t pos_diff_determinant = F360_2d_Matrix_Determinant(pos_cov);

      if (pos_diff_determinant > F360_MIN_DENOMINATOR)
      {
         determinants.n_dets++;
         const float32_t one_over_pos_diff_determinant = 1.0F / pos_diff_determinant;
         determinants.Sxx += pos_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] * one_over_pos_diff_determinant;
         determinants.Sxy -= pos_cov[F360_2D_IDX_X][F360_2D_IDX_Y] * one_over_pos_diff_determinant;
         determinants.Syy += pos_cov[F360_2D_IDX_X][F360_2D_IDX_X] * one_over_pos_diff_determinant;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Det_Index_Valid
   *===========================================================================
   * RETURN VALUE:
   * bool f_vaild_det_index
   *
   * PARAMETERS:
   * const int16_t det_index
   * const uint16_t max_dets_num
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function checks if det_index is smaller than maximum allowed index.
   * is equal or smaller than MAX_DETS_IN_OBJ_TRK.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Is_Det_Index_Valid(const int16_t det_index, const uint16_t max_dets_num)
   {
     return (det_index < static_cast<int32_t>(max_dets_num));
   }

   /*===========================================================================*\
   * FUNCTION: Is_Num_Of_Dets_Correct
   *===========================================================================
   * RETURN VALUE:
   * bool f_correct_number_of_dets
   *
   * PARAMETERS:
   * const int32_t num_dets
   * const uint32_t num_hist_dets_in_track
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function checks if number of detections
   * is equal or smaller than MAX_DETS_IN_OBJ_TRK.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Is_Num_Of_Dets_Correct(const int16_t num_dets, const uint8_t num_hist_dets_in_track)
   {
      return (num_dets <= static_cast<int16_t>(num_hist_dets_in_track));
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Velocity_Cov_Trace
   *===========================================================================
   * RETURN VALUE:
   * float32_t cov_trace
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Determinants_T& determinants
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function calculates covariance trace based on passed determinants.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static float32_t Calc_Velocity_Cov_Trace(F360_NEES_CFMI_Determinants_T& determinants)
   {
      // Update the last determinant
      determinants.Syx = determinants.Sxy;

      // Fast raw covariance calculation
      const float32_t V_determiant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);
      float32_t cov_trace;

      if (V_determiant > F360_MIN_DENOMINATOR)
      {
         const float32_t one_over_V_determiant = 1.0F / V_determiant;
         const float32_t cov_VxVx = determinants.Syy * one_over_V_determiant;
         const float32_t cov_VyVy = determinants.Sxx * one_over_V_determiant;

         cov_trace = cov_VxVx + cov_VyVy;
      }
      else
      {
         cov_trace = 1.0F / F360_MIN_DENOMINATOR;
      }

      return cov_trace;
   }
}
