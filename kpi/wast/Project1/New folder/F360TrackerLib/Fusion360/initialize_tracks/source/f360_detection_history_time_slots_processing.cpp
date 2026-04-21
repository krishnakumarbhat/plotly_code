/*===================================================================================*\
* FILE: f360_detection_history_time_slots_processing.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*
* DESCRIPTION:
*   This file contains functions definitions used for splitting current and historical detections into
*   time-boxed slots.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_math_func.h"
#include "f360_detection_time_slots.h"
#include "f360_detection_history_time_slots_processing.h"
#include "f360_iterator.h"

namespace f360_variant_A
{
   struct Sort_By_Time_Since_Meas
   {
      bool operator()(const F360_NEES_CFMI_Detection_T& first, const F360_NEES_CFMI_Detection_T& second) const
      {
         return first.time_since_meas < second.time_since_meas;
      }
   };

   struct Detection_Is_Historical
   {
      bool operator()(const F360_NEES_CFMI_Detection_T& detection) const
      {
         return detection.idx.f_historical;
      }
   };

   struct Detection_Is_Current
   {
      bool operator()(const F360_NEES_CFMI_Detection_T& detection) const
      {
         return !detection.idx.f_historical;
      }
   };

   static void Add_Detection_To_Timeslot(F360_Detection_Time_Slots_T & det_hist_time_slots, 
      F360_Detection_Time_Slot_T& current_timeslot, 
      const uint32_t hist_dets_idx);

   static F360_Detection_Time_Slot_T* Add_Timeslot(F360_Detection_Time_Slots_T& det_hist_time_slots);

   static void Fill_Min_Max_Values_For_Each_Time_Slot(const F360_NEES_CFMI_Detection_T(&nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
      F360_Detection_Time_Slots_T& det_hist_time_slots);

   static bool Is_Slot_Limit_Reached(const F360_Detection_Time_Slots_T& det_hist_time_slots);

   static bool Can_Detection_Be_Inserted_To_Current_Slot(const F360_Detection_Time_Slot_T& slot);

   template<class T>
   static void Filter_Nees_Detections(
      const F360_NEES_CFMI_Detection_T(&input_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
      const uint32_t input_dets_count,
      const T filter,
      F360_NEES_CFMI_Detection_T(&output_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET]);

   /*===========================================================================*\
   * FUNCTION: Split_Dets_Into_Time_Slots
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_Detection_Time_Slots_T& det_hist_time_slots,
   * const float32_t dt_for_split
   *
   * ABSTRACT:
   * Split detections into time-boxed slots. First slot is used to store current time instance detections. Others are
   * are used for historical detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Split_Dets_Into_Time_Slots(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots,
      const float32_t dt_for_split)
   {
      Create_Time_Slot_For_Current_Detections(nees_cfmi_information, det_hist_time_slots);
      Split_Hist_Dets_Into_Time_Slots(nees_cfmi_information, det_hist_time_slots, dt_for_split);
      Fill_Min_Max_Values_For_Each_Time_Slot(nees_cfmi_information.detections, det_hist_time_slots);
   }

   /*===========================================================================*\
   * FUNCTION: Create_Time_Slot_For_Current_Detections
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * ABSTRACT:
   * Create time-boxed slot and fill it with current time instance detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Create_Time_Slot_For_Current_Detections(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots)
   {
      const uint32_t number_of_current_dets = nees_cfmi_information.current_dets_count;
      if (number_of_current_dets > 0U)
      {
         F360_NEES_CFMI_Detection_T(&all_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET] = nees_cfmi_information.detections;

         F360_NEES_CFMI_Detection_T current_nees_dets[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET] = {};
         Filter_Nees_Detections(all_nees_dets, nees_cfmi_information.dets_num, Detection_Is_Current{}, current_nees_dets);

         F360_Detection_Time_Slot_T* const current_timeslot = Add_Timeslot(det_hist_time_slots);

         for (uint32_t nees_curr_det_index = 0U; nees_curr_det_index < number_of_current_dets; ++nees_curr_det_index)
         {
            if (Can_Detection_Be_Inserted_To_Current_Slot(*current_timeslot))
            {
               const F360_NEES_CFMI_Detection_T& nees_det = current_nees_dets[nees_curr_det_index];
               Add_Detection_To_Timeslot(det_hist_time_slots, *current_timeslot, nees_det.idx.nees_idx);
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Split_Hist_Dets_Into_Time_Slots
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_NEES_CFMI_Information_T& nees_cfmi_information,
   * F360_Detection_Time_Slots_T& det_hist_time_slots,
   * const float32_t dt_for_split
   *
   * ABSTRACT:
   * Create time-boxed slots and fill it with prevous time instances detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Split_Hist_Dets_Into_Time_Slots(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots,
      const float32_t dt_for_split)
   {
      const uint32_t number_of_hist_dets = nees_cfmi_information.old_dets_count;
      if (number_of_hist_dets > 0U)
      {
         F360_NEES_CFMI_Detection_T(&all_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET] = nees_cfmi_information.detections;

         F360_NEES_CFMI_Detection_T hist_nees_dets[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET] = {};
         Filter_Nees_Detections(all_nees_dets, nees_cfmi_information.dets_num, Detection_Is_Historical{}, hist_nees_dets);

         uint32_t perm[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET];
         float32_t time_since_measurement[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET];
         for (uint32_t i = 0U; i < number_of_hist_dets; i++)
         {
            time_since_measurement[i] = hist_nees_dets[i].time_since_meas;
         }
         (void)F360_Sort(time_since_measurement, number_of_hist_dets, true, perm);

         // Create first time slot and add to it historical detection with lowest time since measurement
         F360_Detection_Time_Slot_T* current_timeslot = Add_Timeslot(det_hist_time_slots);
         Add_Detection_To_Timeslot(det_hist_time_slots, *current_timeslot, hist_nees_dets[perm[0]].idx.nees_idx);

         for (uint32_t i = 1U; i < number_of_hist_dets; ++i)
         {
            const uint32_t nees_hist_det_index = perm[i];
            const uint32_t prev_nees_hist_det_index = perm[i - 1U];
            const F360_NEES_CFMI_Detection_T& nees_det = hist_nees_dets[nees_hist_det_index];
            const float32_t max_possible_time_since_meas = hist_nees_dets[prev_nees_hist_det_index].time_since_meas + dt_for_split;

            if (nees_det.time_since_meas > max_possible_time_since_meas)
            {
               if (Is_Slot_Limit_Reached(det_hist_time_slots))
               {
                  break;
               }
               current_timeslot = Add_Timeslot(det_hist_time_slots);
            }
            if (Can_Detection_Be_Inserted_To_Current_Slot(*current_timeslot))
            {
               Add_Detection_To_Timeslot(det_hist_time_slots, *current_timeslot, nees_det.idx.nees_idx);
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Add_Detection_To_Timeslot
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Detection_Time_Slots_T& det_hist_time_slots
   * F360_Detection_Time_Slot_T& current_timeslot
   * const uint32_t hist_dets_idx
   *
   * ABSTRACT:
   * Add given index of detection to particular time-boxed slot.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Add_Detection_To_Timeslot(F360_Detection_Time_Slots_T& det_hist_time_slots, F360_Detection_Time_Slot_T& current_timeslot, const uint32_t hist_dets_idx)
   {
      det_hist_time_slots.number_of_dets++;
      current_timeslot.hist_dets_idx[current_timeslot.number_of_active_detections] = hist_dets_idx;
      current_timeslot.number_of_active_detections++;
   }

   /*===========================================================================*\
   * FUNCTION: Add_Timeslot
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * ABSTRACT:
   * Add new timeslot to timeslots container and return it.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static F360_Detection_Time_Slot_T* Add_Timeslot(F360_Detection_Time_Slots_T& det_hist_time_slots)
   {
      det_hist_time_slots.number_of_active_slots++;
      const uint32_t slot_index = det_hist_time_slots.number_of_active_slots - 1U;
      return &det_hist_time_slots.time_since_meas_slots[slot_index];
   }

   /*===========================================================================*\
   * FUNCTION: Fill_Min_Max_Values_For_Each_Time_Slot
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_NEES_CFMI_Detection_T(&nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
   * F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * ABSTRACT:
   * Loop over ale time slots and update minmal and maximum time since measurement values for stored detections.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Fill_Min_Max_Values_For_Each_Time_Slot(
      const F360_NEES_CFMI_Detection_T(&nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
      F360_Detection_Time_Slots_T& det_hist_time_slots)
   {
      const uint32_t number_of_slots = det_hist_time_slots.number_of_active_slots;
      for (uint32_t slot_idx = 0U; slot_idx < number_of_slots; ++slot_idx)
      {
         F360_Detection_Time_Slot_T& slot = det_hist_time_slots.time_since_meas_slots[slot_idx];
         const uint32_t first_hist_index = slot.hist_dets_idx[0];
         const uint32_t last_hist_index = slot.hist_dets_idx[slot.number_of_active_detections - 1U];

         const float32_t min_time = nees_dets[first_hist_index].time_since_meas;
         const float32_t max_time = nees_dets[last_hist_index].time_since_meas;
         slot.min_time_since_meas = min_time;
         slot.max_time_since_meas = max_time;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Slot_Limit_Reached
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * ABSTRACT:
   * Return true if maximum number of slots has been reached.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Is_Slot_Limit_Reached(const F360_Detection_Time_Slots_T& det_hist_time_slots)
   {
      return det_hist_time_slots.number_of_active_slots >= F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS;
   }

   /*===========================================================================*\
   * FUNCTION: Can_Detection_Be_Inserted_To_Current_Slot
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Time_Slots_T& det_hist_time_slots
   *
   * ABSTRACT:
   * Return true if maximum number of stored detection is given slot has been reached.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Can_Detection_Be_Inserted_To_Current_Slot(const F360_Detection_Time_Slot_T& slot)
   {
      return slot.number_of_active_detections < F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT;
   }

   /*===========================================================================*\
   * FUNCTION: Filter_Nees_Detections
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_NEES_CFMI_Detection_T(&input_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
   * const uint32_t input_dets_count,
   * const T filter,
   * F360_NEES_CFMI_Detection_T(&output_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET])
   *
   * ABSTRACT:
   * Copy detections from input_nees_dets to output_nees_dets using filter object. Filter is template
   * object which should have () operator overloaded and take as an argument F360_NEES_CFMI_Detection_T. 
   * It should return true if given detection meets conditions for filtering process.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   template<class T>
   static void Filter_Nees_Detections(
      const F360_NEES_CFMI_Detection_T(&input_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET],
      const uint32_t input_dets_count,
      const T filter,
      F360_NEES_CFMI_Detection_T(&output_nees_dets)[F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET])
   {
      uint32_t output_det_index = 0U;

      for (uint32_t input_det_index = 0U; input_det_index < input_dets_count; ++input_det_index)
      {
         const F360_NEES_CFMI_Detection_T nees_det = input_nees_dets[input_det_index];
         if (filter(nees_det))
         {
            output_nees_dets[output_det_index] = nees_det;
            output_det_index++;
         }
      }
   }

}
