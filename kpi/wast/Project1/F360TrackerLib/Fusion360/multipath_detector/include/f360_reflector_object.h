/*===================================================================================*\
* FILE: f360_reflector_object.h
*====================================================================================
* Copyright (C) 2021-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains declaration of Reflector interface class.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/


#ifndef REFLECTOR_OBJECT_H
#define REFLECTOR_OBJECT_H

#include "f360_object_track.h"
#include "f360_math_func.h"


namespace f360_variant_A
{
   struct Range_Rate_Interval_T
   {
      float32_t rr_min;
      float32_t rr_max;
   };

   class Object_Reflector
   {
   public:
      Object_Reflector();

      Object_Reflector(
         const F360_Object_Track_T &object
      );

      Object_Reflector(
         const F360_Object_Track_T &object,
         const float32_t size_extension);

      bool Is_Valid() const
      {
         return (nullptr != m_object);
      };

      std::pair<bool, Point> Find_Intersection(
         const Point& end_point_A1,
         const Point& end_point_A2
      ) const;

      Range_Rate_Interval_T Compute_Range_Rate_Interval(
         const Point& view_point, 
         const float32_t radius) const;

      bool Is_On_Radius(
         const Point& view_point, 
         const float32_t radius) const;

      bool Is_Inside(
         const Point& analyzed_point
      ) const;

   private:
      const F360_Object_Track_T *m_object;
      Point m_extended_corners[NUM_CORNERS_IN_RECTANGLE] = {};
      float32_t m_size_extension_for_intersection;

      void Calc_Extended_Corners();
   };
}

#endif
