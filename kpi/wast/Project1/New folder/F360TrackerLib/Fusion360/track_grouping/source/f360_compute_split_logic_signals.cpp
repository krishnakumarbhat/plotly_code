/*===========================================================================*\
* FILE: f360_compute_split_logic_signals.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Compute_Split_Logic_Signals()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_compute_split_logic_signals.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_math_func.h"
#include "f360_math.h"
#include <algorithm>

namespace f360_variant_A
{
   static void Convert_Dets_To_TCS(
      const F360_Object_Track_T& obj,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      float32_t(&orth_pos)[MAX_DETS_IN_OBJ_TRK]);

   /*===========================================================================*\
   * FUNCTION: Compute_Split_Logic_Signals()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS]
   * const F360_Calibrations_T& calibs
   * const F360_Tracker_Info_T& tracker_info,
   * const float32_t dist_to_rear_axle
   * F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS]
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
   * Main function call to compute signals for split logic. Computes 
   * information that is common for all signals. 
   *
   * Each signal consists of both a raw/instantaneous signal and a low-pass 
   * filtered signal. If there are not enough detections to compute the signal, 
   * the raw signal will be set to -1 whereas the low-passed signal will retain 
   * its value.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_Split_Logic_Signals(
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Calibrations_T& calibs,
      const F360_Tracker_Info_T& tracker_info,
      const float32_t dist_to_rear_axle,
      F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS]) 
   {

      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[i] - 1;

         F360_Object_Track_T& current_object = objects[obj_idx];

         if ((F360_TRACKER_TRKFLTR_CTCA == current_object.trk_fltr_type) || 
            (F360_TRACKER_TRKFLTR_CCA == current_object.trk_fltr_type))
         {
            const F360_Object_Orth_Split_Signals_Status_Type_T split_signals_status = Derive_Object_Orth_Split_Signal_Status(calibs, current_object, dist_to_rear_axle);

            if (F360_RESET_SPLIT_SIGNALS == split_signals_status)
            {
               current_object.orth_delta_filtered = 0.0F;
               current_object.orth_gap_filtered = 0.0F;
            }
            else if (F360_UPDATE_SPLIT_SIGNALS == split_signals_status)
            {
               float32_t orth_pos[MAX_DETS_IN_OBJ_TRK] = {};
               Convert_Dets_To_TCS(current_object, det_p, orth_pos);

               // Sort detections orthogonal position in TCS
               uint32_t orth_perm[MAX_DETS_IN_OBJ_TRK] = {};
               (void)F360_Sort(orth_pos, current_object.ndets, true, orth_perm);

               Compute_And_Filter_Objects_Detections_Max_Delta(calibs.k_orth_split_orth_delta_filter_const, orth_pos, current_object);

               Compute_And_Filter_Objects_Detections_Max_Gap(calibs, orth_pos, current_object);
            }
            else
            {
               // Signal should be frozen, do nothing
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Compute_And_Filter_Objects_Detections_Max_Delta()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const float32_t filter_constant
   * const float32_t(&orth_sorted_pos)[MAX_DETS_IN_OBJ_TRK]
   * F360_Object_Track_T& object
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
   * Computes the maximum orthogonal delta of associated detections, 
   * i.e. the furthest distance between all detections associated to the object.
   *
   * PRECONDITIONS:
   * The positions in orth_sorted_pos are sorted in ascending order
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_And_Filter_Objects_Detections_Max_Delta(
      const float32_t filter_constant,
      const float32_t(&orth_sorted_pos)[MAX_DETS_IN_OBJ_TRK],
      F360_Object_Track_T& object)
   {
      const float32_t max_orth_delta = orth_sorted_pos[object.ndets - 1U] - orth_sorted_pos[0];

      object.orth_delta_filtered = F360_Low_Pass_Filter_First_Order(max_orth_delta, object.orth_delta_filtered, filter_constant);

   }

   /*===========================================================================*\
   * FUNCTION: Derive_Object_Orth_Split_Signal_Status()
   * ===========================================================================
   * RETURN VALUE:
   * F360_Object_Orth_Split_Signals_Status_Type_T orth_delta_status
   *
   * PARAMETERS:
   *  const F360_Calibrations_T& calibs,
   *  const F360_Object_Track_T& object,
   *  const float32_t dist_to_rear_axle
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
   * Derives whether we should reset, freeze or update an objects
   * orthogonal split related signals
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   F360_Object_Orth_Split_Signals_Status_Type_T Derive_Object_Orth_Split_Signal_Status(
      const F360_Calibrations_T& calibs,
      const F360_Object_Track_T& object,
      const float32_t dist_to_rear_axle)
   {
      const float32_t half_host_length = 0.6F * dist_to_rear_axle;
      const float32_t obj_long_pos_host_center = object.vcs_position.x + half_host_length;

      const float32_t dist_sq = F360_Get_Hypotenuse_Squared(obj_long_pos_host_center, object.vcs_position.y);

      F360_Object_Orth_Split_Signals_Status_Type_T orth_delta_status;
      if (dist_sq > calibs.k_orth_split_max_distance_sq)
      {
         orth_delta_status = F360_RESET_SPLIT_SIGNALS;
      }
      else if ((object.ndets < 2U) || (dist_sq < calibs.k_orth_split_min_distance_sq))
      {
         orth_delta_status = F360_FREEZE_SPLIT_SIGNALS;
      }
      else
      {
         orth_delta_status = F360_UPDATE_SPLIT_SIGNALS;
      }

      return orth_delta_status;
   }

   /*===========================================================================*\
   * FUNCTION: Compute_And_Filter_Objects_Detections_Max_Gap()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs,
   * const float32_t(&orth_sorted_pos)[MAX_DETS_IN_OBJ_TRK],
   * F360_Object_Track_T& object)
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
   * Computes the maximum orthogonal gap between all associated
   * detections of an object, i.e. the furthest distance between two
   * detections of an object.
   *
   * PRECONDITIONS:
   * orth_sorted_pos must be sorted sorted before call to this function.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_And_Filter_Objects_Detections_Max_Gap(
      const F360_Calibrations_T& calibs,
      const float32_t(&orth_sorted_pos)[MAX_DETS_IN_OBJ_TRK],
      F360_Object_Track_T& object)
   {
      // Compute all gaps in TCS
      float32_t orth_gaps[MAX_DETS_IN_OBJ_TRK - 1U] = {};
      for (uint32_t j = 0U; j < (object.ndets - 1U); j++)
      {
         orth_gaps[j] = orth_sorted_pos[j + 1U] - orth_sorted_pos[j];
      }
      const uint32_t saturated_ndets = std::min(object.ndets, calibs.k_orth_split_orth_gap_filter_max_dets);
      const float32_t filter_constant = calibs.k_orth_split_orth_gap_filter_prop_const * static_cast<float32_t>(saturated_ndets);
      const float32_t max_orth_gap = F360_Max_Element(orth_gaps, object.ndets - 1U);
      object.orth_gap_filtered = F360_Low_Pass_Filter_First_Order(max_orth_gap, object.orth_gap_filtered, filter_constant);

   }

   /*===========================================================================*\
   * FUNCTION: Convert_Dets_To_TCS()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   *  const F360_Object_Track_T& obj,
   *  const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
   *  float32_t(&orth_pos)[MAX_DETS_IN_OBJ_TRK]
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
   * Transforms an objects associated detections in to TCS system and
   * populates the orthogonal position in orth_pos array
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Convert_Dets_To_TCS(
      const F360_Object_Track_T& obj,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      float32_t(&orth_pos)[MAX_DETS_IN_OBJ_TRK])
   {
      for (uint32_t j = 0U; j < obj.ndets; j++)
      {
         const uint32_t det_idx = obj.detids[j] - 1U;

         Point det_pos_tcs = {};
         Convert_VCS_Posn_To_TCS_Posn(
            det_p[det_idx].vcs_position.x,
            det_p[det_idx].vcs_position.y,
            obj.bbox.Get_Center().x,
            obj.bbox.Get_Center().y,
            obj.bbox.Get_Orientation(),
            det_pos_tcs.x,
            det_pos_tcs.y);

         orth_pos[j] = det_pos_tcs.y;
      }
   }
}

