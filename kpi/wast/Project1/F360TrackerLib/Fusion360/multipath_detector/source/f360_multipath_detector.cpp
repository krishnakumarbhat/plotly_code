/*===================================================================================*\
* FILE:  f360_multipath_detector.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Multipath_Detector class methods.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


#include "f360_multipath_detector.h"
#include "f360_reflector_object.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   static float32_t Calc_Searching_Range(
      const Point &radar_pos,
      const Point &reflector_pos,
      const Point &mp_candidate_pos
   );

   /*=========================================================================
   * Method         Is_Multipath
   *
   * Description    Check if in provided mp_candidate can be a multipath.
   *
   * Parameters
   *  const Point &radar_pos - radar (wave source) position
   *  const Point &mp_candidate_pos - mp_candidate position
   *  const float32_t mp_candidate_range_rate - mp_candidate range rate
   *  const F360_Tracker_Info_T& tracker_info
   *
   * Returns        bool - true if mp_candidate is potential multipath.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Multipath_Detector::Is_Multipath(
      const Point &radar_pos,
      const Point &mp_candidate_pos,
      const float32_t mp_candidate_range_rate,
      const F360_Tracker_Info_T& tracker_info)
   {
      bool f_multipath;
         
      const std::pair<bool, Point> reflection_point = refl_selector.Get_Reflection_Point(radar_pos, mp_candidate_pos, m_objects);
      const bool &f_reflection_found = reflection_point.first;
      if (f_reflection_found)
      {
         const float32_t searching_range = Calc_Searching_Range(radar_pos, reflection_point.second, mp_candidate_pos);
         f_multipath = Find_Corresponding_Source(reflection_point.second, searching_range, mp_candidate_pos, mp_candidate_range_rate, tracker_info);
      }
      else
      {
         f_multipath = false;
      }


      return f_multipath;
   }

   /*=========================================================================
   * Method         Get_Source_Reflector
   *
   * Description    Returns found source reflector.
   *
   * Parameters     None.
   *
   * Returns        Object_Reflector* - pointer to found reflector..
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   const Object_Reflector* Multipath_Detector::Get_Source_Reflector()
   {
      Object_Reflector*reflector;

      if (obj_source.Is_Valid())
      {
         reflector = &obj_source;
      }
      else
      {
         reflector = nullptr;
      }

      return reflector;
   }

   /*=========================================================================
   * Method         Find_Next_Potential_Source
   *
   * Description    Substep of Find_Corresponding_Source method. It evaluets if 
   *                reflector can be the source reflector.
   *
   * Parameters
   * const Point &reflection_point
   * const float32_t remaining_dist
   * const F360_Tracker_Info_T& tracker_info
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Multipath_Detector::Find_Next_Potential_Source(
      const Point &reflection_point,
      const float32_t remaining_dist,
      const F360_Tracker_Info_T& tracker_info)
   {
      obj_source = {};

      for (int32_t k = last_obj_src_idx + 1; k < static_cast<int32_t>(tracker_info.variant.num_tracks); k++)
      {
         const F360_Object_Track_T &source_obj = m_objects[k];
         
         if ((F360_OBJECT_STATUS_INVALID != source_obj.status) &&
            (source_obj.f_moving))
         {
            const Object_Reflector reflector { source_obj };
            if (Verify_Distance_Hypothesis(reflector, reflection_point, remaining_dist))
            {
               last_obj_src_idx = k;
               obj_source = { m_objects[k] };
               break;
            }
         }
      }
   }

   /*=========================================================================
   * Method         Find_Corresponding_Source
   *
   * Description    For given reflection point, it looks for source reflector.
   *
   * Parameters
   *  const Point &reflection_point
   *  const float32_t searching_range
   *  const Point &mp_candidate_pos,
   *  const float32_t mp_candidate_range_rate
   *  const F360_Tracker_Info_T& tracker_info
   *
   * Returns        bool - true if source is found.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Multipath_Detector::Find_Corresponding_Source(
      const Point &reflection_point,
      const float32_t searching_range,
      const Point &mp_candidate_pos,
      const float32_t mp_candidate_range_rate,
      const F360_Tracker_Info_T& tracker_info)
   {
      bool f_source_found = false;
      last_obj_src_idx = -1;

      while (!f_source_found)
      {
         Find_Next_Potential_Source(reflection_point, searching_range, tracker_info);
         const Object_Reflector* const potential_src_reflector = Get_Source_Reflector();

         if (nullptr != potential_src_reflector)
         {
            if (!potential_src_reflector->Is_Inside(mp_candidate_pos))
            {
               f_source_found = Verify_Range_Rate_Hypothesis(*potential_src_reflector, reflection_point, searching_range, mp_candidate_range_rate);
            }
         }
         else
         {
            break;
         }
      }

      return f_source_found;
   }

   /*=========================================================================
   * Method         Verify_Range_Rate_Hypothesis
   *
   * Description    Verifies range rate hypothesis of true mutipath model.
   *
   * Parameters
   *  const Object_Reflector& reflector,
   *  const Point &reflection_point
   *  const float32_t searching_range
   *  const float32_t mp_candidate_range_rate
   *
   * Returns        bool - true if hypothesis is confirmed.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Multipath_Detector::Verify_Range_Rate_Hypothesis(
      const Object_Reflector& reflector,
      const Point &reflection_point,
      const float32_t searching_range,
      const float32_t mp_candidate_range_rate
   )
   {
      const Range_Rate_Interval_T rr_scope = reflector.Compute_Range_Rate_Interval(reflection_point, searching_range);

      bool f_passed_verification;
      if ((rr_scope.rr_min < mp_candidate_range_rate) && (mp_candidate_range_rate < rr_scope.rr_max))
      {
         f_passed_verification = true;
      }
      else
      {
         f_passed_verification = false;
      }

      return f_passed_verification;
   }

   /*=========================================================================
   * Method         Verify_Distance_Hypothesis
   *
   * Description    Verifes range hypothesis of true mutipath model.
   *
   * Parameters
   *  Object_Reflector &source_reflector
   *  const Point &reflection_point
   *  const float32_t remaining_dist
   *
   * Returns        bool - true if hypothesis is confirmed.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Multipath_Detector::Verify_Distance_Hypothesis(
      const Object_Reflector&source_reflector,
      const Point &reflection_point,
      const float32_t remaining_dist
   )
   {
      bool f_passed_verification;

      if (source_reflector.Is_On_Radius(reflection_point, remaining_dist))
      {
         f_passed_verification = true;
      }
      else
      {
         f_passed_verification = false;
      }

      return f_passed_verification;
   }

   /*===========================================================================*\
    * FUNCTION: Calc_Searching_Range()
    *===========================================================================
    * RETURN VALUE:
    * float32_t - remaining distance.
    *
    * PARAMETERS:
    *  const Point &radar_pos,
    *  const Point &reflector_pos,
    *  const Point &mp_candidate_pos
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
    * Calculate remaining distance from reflector to multipath candidate.
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
 \*===========================================================================*/
   static float32_t Calc_Searching_Range(
      const Point &radar_pos,
      const Point &reflector_pos,
      const Point &mp_candidate_pos
   )
   {
      const float32_t mp_candidate_to_radar_dist = F360_Get_Hypotenuse(radar_pos.y - mp_candidate_pos.y, radar_pos.x - mp_candidate_pos.x);
      const float32_t radar_to_refl_dist = F360_Get_Hypotenuse(radar_pos.y - reflector_pos.y, radar_pos.x - reflector_pos.x);
      return mp_candidate_to_radar_dist - radar_to_refl_dist;
   }

}

