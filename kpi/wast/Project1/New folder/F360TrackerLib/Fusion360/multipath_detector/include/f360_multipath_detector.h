/*===================================================================================*\
* FILE:  f360_multipath_detector.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Multipath_Detector class.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


#ifndef MULTIPATH_DETECTOR_H
#define MULTIPATH_DETECTOR_H

#include "f360_reuse.h"
#include "f360_object_track.h"
#include "f360_tracker_info.h"
#include "f360_static_environment_class.h"
#include "f360_math_func.h"
#include "f360_reflector_object.h"
#include "f360_reflector_selector.h"

namespace f360_variant_A
{
   class Multipath_Detector
   {
   public:
      Multipath_Detector(
         const Static_Env_T::Static_Env_Polys_Array& polys,
         const F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
         const F360_Tracker_Info_T &tracker_info,
         const F360_Calibrations_T &calib) :
         m_polys(polys), m_objects(objects),
         refl_selector(calib.k_mp_half_long_zone, calib.k_mp_half_lat_zone, calib.k_mp_object_reflector_size_extension, tracker_info)
      { }

      bool Is_Multipath(
         const Point &radar_pos,
         const Point &mp_candidate_pos,
         const float32_t mp_candidate_range_rate,
         const F360_Tracker_Info_T& tracker_info);

   private:
      const Static_Env_T::Static_Env_Polys_Array& m_polys;
      const F360_Object_Track_T(&m_objects)[NUMBER_OF_OBJECT_TRACKS];

      Object_Track_Reflector_Selector refl_selector;
      int32_t last_obj_src_idx = -1;
      Object_Reflector obj_source;
    
      const Object_Reflector* Get_Source_Reflector();
      
      void Find_Next_Potential_Source(
         const Point &reflection_point,
         const float32_t remaining_dist,
         const F360_Tracker_Info_T& tracker_info);

      bool Find_Corresponding_Source(
         const Point &reflection_point,
         const float32_t searching_range,
         const Point &mp_candidate_pos,
         const float32_t mp_candidate_range_rate,
         const F360_Tracker_Info_T& tracker_info);

      static bool Verify_Range_Rate_Hypothesis(
         const Object_Reflector& reflector,
         const Point &reflection_point,
         const float32_t searching_range,
         const float32_t mp_candidate_range_rate);

      static bool Verify_Distance_Hypothesis(
         const Object_Reflector&source_reflector,
         const Point &reflection_point,
         const float32_t remaining_dist);
   };
}

#endif

