/*===================================================================================*\
* FILE:  f360_occlusion_sector.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Occlusion_Sector_T class
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_OCLUSION_SECTOR
#define F360_OCLUSION_SECTOR

#include "f360_reuse.h"
#include "f360_occlusion_types.h"

namespace f360_variant_A
{
   class Occlusion_Sector_T
   {
   public:
      Occlusion_Sector_T();
      ~Occlusion_Sector_T();

      void Update_Range_With_Intersection_Of_Two_Lines(
         const float32_t(&A)[2],
         const float32_t(&B)[2],
         const float32_t(&C)[2],
         const float32_t(&D)[2]); // Method used to determine intersection of two lines and update sector held by range.

      float32_t Get_Range() const // Method used to access current range held by sector
      {
         return range;
      }

      void Set_Range(const float32_t new_range) // Method used to set current range of sector
      {
         range = new_range;
      }

   private:
      float32_t range;
   };

   using Occlusion_Sector_Array = Occlusion_Sector_T[occlusion_num_sectors];
}
#endif
