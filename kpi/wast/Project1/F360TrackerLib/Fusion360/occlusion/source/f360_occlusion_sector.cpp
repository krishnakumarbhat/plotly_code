/*===================================================================================*\
* FILE:  f360_occlusion_sector.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definiton of Occlusion_Sector_T class member functions.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_occlusion_sector.h"
#include "f360_intersections.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*=========================================================================
   * Method         Occlusion_Sector_T::Occlusion_Sector_T
   *
   * Description    Constructor of Occlusion_Sector_T.
   *
   * Parameters     None.
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
   Occlusion_Sector_T::Occlusion_Sector_T():
      range(0.0F)
   {

   }

   /*=========================================================================
   * Method         Occlusion_Sector_T::~Occlusion_Sector_T
   *
   * Description    Destructor of Occlusion_Sector_T.
   *
   * Parameters     None.
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
   Occlusion_Sector_T::~Occlusion_Sector_T()
   {

   }

   /*=========================================================================
   * Method         Occlusion_Sector_T::Update_Range_With_Intersection_Of_Two_Lines
   *
   * Description    Method used to update current range held by sector with intersection of two lines.
   *
   * Parameters     
   * const float32_t(&A)[2] - first line begining point, {longitudinal, lateral}
   * const float32_t(&B)[2] - first line ending point, {longitudinal, lateral} 
   * const float32_t(&C)[2] - second line begining point, {longitudinal, lateral}
   * const float32_t(&D)[2] - second line ending point, {longitudinal, lateral}
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
   void Occlusion_Sector_T::Update_Range_With_Intersection_Of_Two_Lines(
      const float32_t(&A)[2], 
      const float32_t(&B)[2], 
      const float32_t(&C)[2], 
      const float32_t(&D)[2])
   {
      float32_t x_intersect = 0.0F;
      float32_t y_intersect = 0.0F;
      const bool f_lines_are_intersecting = Determine_Segments_Intersection_Limited(A, B, C, D, x_intersect, y_intersect);
      if (f_lines_are_intersecting)
      {
         const float32_t new_range = F360_Get_Hypotenuse(x_intersect, y_intersect);
         range = std::min(new_range, range);
      }
   }

}
