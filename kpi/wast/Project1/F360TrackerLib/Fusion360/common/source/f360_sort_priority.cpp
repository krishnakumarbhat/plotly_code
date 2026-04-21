/*===================================================================================*\
* FILE: f360_sort_priority.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains function definition of Sort_Priority_With_New_Track, Quick_Sort_Track_Priority and helper functions.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#include "f360_sort_priority.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   static void Get_Last_Sorted_Tracks_Idx_And_Priority(
      const F360_Tracker_Info_T & tracker_info,
      uint32_t (&last_time_sorted_idx)[NUMBER_OF_OBJECT_TRACKS],
      float32_t (&last_time_sorted_priorities)[NUMBER_OF_OBJECT_TRACKS]);

   static void Update_Track_Priority_Pointers(
      const uint32_t (&sorted_inner_idx)[NUMBER_OF_OBJECT_TRACKS],
      const uint32_t (&last_time_sorted_idx)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T & tracker_info,
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]);

   /*===========================================================================*\
   * FUNCTION: Sort_Priority_With_New_Track()
   *===========================================================================
   * RETURN VALUE:
   * Number of neighbor points
   *
   * PARAMETERS:
   *    F360_Tracker_Info_T & tracker_info,
   *    F360_Object_Track_T* const p_newtrack
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function adds new track to the sorted track linked list
   *
   \*===========================================================================*/
   void Sort_Priority_With_New_Track(
      F360_Tracker_Info_T & tracker_info,
      F360_Object_Track_T * const p_new_track)
   {
      const uint32_t num_active_objs = static_cast<uint32_t>(tracker_info.num_active_objs);
      F360_Object_Track_T * p_current_track = tracker_info.p_highest_priority_track;

      if (p_current_track == NULL)
      {
         // tracker_info.p_highest_priority_track was not initialized yet
         tracker_info.p_highest_priority_track = p_new_track;
         tracker_info.p_lowest_priority_track = p_new_track;
      }
      else if (tracker_info.p_highest_priority_track->priority < p_new_track->priority)
      {
         // new track priority is greater then highest priority track 
         tracker_info.p_highest_priority_track->p_higher_priority_track = p_new_track;
         p_new_track->p_lower_priority_track = tracker_info.p_highest_priority_track;
         tracker_info.p_highest_priority_track = p_new_track;
      }
      else
      {
         bool f_done = false;
         for (uint32_t track_counter = 0U; (track_counter < num_active_objs) && (!f_done); track_counter++)
         {
            F360_Object_Track_T* const p_next_track = p_current_track->p_lower_priority_track;
            if (p_next_track != NULL)
            {
               if (p_next_track->priority < p_new_track->priority)
               {
                  // insert track between p_current_track and p_next_track
                  p_new_track->p_lower_priority_track = p_next_track;
                  p_new_track->p_higher_priority_track = p_current_track;
                  p_current_track->p_lower_priority_track = p_new_track;
                  p_next_track->p_higher_priority_track = p_new_track;
                  f_done = true;
               }
            }
            else
            {
               // there is no next track in prioritized list: add the track at the end 
               p_current_track->p_lower_priority_track = p_new_track;
               p_new_track->p_higher_priority_track = p_current_track;
               tracker_info.p_lowest_priority_track = p_new_track;
               f_done = true;
            }
            
            p_current_track = p_next_track;
         }
      }
   }

   // TODO - DFU-805
   void Quick_Sort_Track_Priority(
      F360_Tracker_Info_T & tracker_info,
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      uint32_t last_time_sorted_idx[NUMBER_OF_OBJECT_TRACKS];
      float32_t last_time_sorted_priorities[NUMBER_OF_OBJECT_TRACKS];
      uint32_t sorted_inner_idx[NUMBER_OF_OBJECT_TRACKS];

      Get_Last_Sorted_Tracks_Idx_And_Priority(tracker_info, last_time_sorted_idx, last_time_sorted_priorities);
      (void)F360_Sort(last_time_sorted_priorities, static_cast<uint32_t>(tracker_info.num_active_objs), false, sorted_inner_idx);

      Update_Track_Priority_Pointers(sorted_inner_idx, last_time_sorted_idx, tracker_info, object_tracks);
   }

   static void Get_Last_Sorted_Tracks_Idx_And_Priority(
      const F360_Tracker_Info_T & tracker_info,
      uint32_t (&last_time_sorted_idx)[NUMBER_OF_OBJECT_TRACKS],
      float32_t (&last_time_sorted_priorities)[NUMBER_OF_OBJECT_TRACKS])
   {
      const uint32_t num_active_objs = static_cast<uint32_t>(tracker_info.num_active_objs);
      F360_Object_Track_T* p_current_track = tracker_info.p_highest_priority_track;
      if (p_current_track == NULL)
      {
         // Do nothing. there is no track to sort.
         // Extra check to verify if there is no active track in tracker info.
         assert(num_active_objs == 0U);
      }
      else
      {
         for (uint32_t index = 0U; index < num_active_objs; index++)
         {
            // Sanity check for pointer.
            assert(p_current_track != NULL);

            last_time_sorted_idx[index] = static_cast<uint32_t>(p_current_track->id) - 1U;
            last_time_sorted_priorities[index] = p_current_track->priority;

            p_current_track = p_current_track->p_lower_priority_track;
         }
      }

      // Sanity check -> the last track should be NULL.
      assert(p_current_track == NULL);
   }

   static void Update_Track_Priority_Pointers(
      const uint32_t (&sorted_inner_idx)[NUMBER_OF_OBJECT_TRACKS],
      const uint32_t (&last_time_sorted_idx)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T & tracker_info,
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      const uint32_t n_track = static_cast<uint32_t>(tracker_info.num_active_objs);

      if (n_track > 0U)
      {
         // Highest priority.
         F360_Object_Track_T* p_current_track = &object_tracks[last_time_sorted_idx[sorted_inner_idx[0]]];
         F360_Object_Track_T* p_lower_priority_track = &object_tracks[last_time_sorted_idx[sorted_inner_idx[1]]];
         F360_Object_Track_T* p_higher_priority_track = NULL;

         tracker_info.p_highest_priority_track = p_current_track;

         p_current_track->p_higher_priority_track = p_higher_priority_track;
         p_current_track->p_lower_priority_track = p_lower_priority_track;
         const uint32_t n_max_iter = n_track - 1U;
         for (uint32_t index = 1U; index < n_max_iter; index++)
         {
            p_higher_priority_track = p_current_track;
            p_current_track = p_lower_priority_track;
            p_lower_priority_track = &object_tracks[last_time_sorted_idx[sorted_inner_idx[index + 1U]]];

            p_current_track->p_higher_priority_track = p_higher_priority_track;
            p_current_track->p_lower_priority_track = p_lower_priority_track;
         }

         // Lowest priority.
         p_higher_priority_track = p_current_track;
         p_current_track = p_lower_priority_track;

         tracker_info.p_lowest_priority_track = p_current_track;

         p_current_track->p_higher_priority_track = p_higher_priority_track;
         p_current_track->p_lower_priority_track = NULL;
      }
   }
}

