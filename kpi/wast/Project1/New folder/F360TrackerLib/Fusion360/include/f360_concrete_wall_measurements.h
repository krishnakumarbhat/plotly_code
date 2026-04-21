/*===========================================================================*\
* FILE: f360_concrete_wall_measurements.h
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Get_Track_Bbox_In_VCS().
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/
#ifndef F360_CONCRETE_WALL_MEASUREMENTS_H
#define F360_CONCRETE_WALL_MEASUREMENTS_H

#include "f360_circular_buffer.h"
#include "f360_constants.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   struct Concrete_Wall_Sample_T
   {
      float32_t lateral_position;
      bool is_valid;
      uint8_t padding[3];
   };

   class Concrete_Wall_Measurements_T
   {
   private:
      static const uint32_t buffer_capacity = 5U;

   public:
      Concrete_Wall_Measurements_T();
      void Reset();
      void Push(const Concrete_Wall_Sample_T& sample);
      Concrete_Wall_Sample_T Get_Last_Sample();
      std::size_t Get_Number_Of_Valid_Samples();
      std::size_t Get_Number_Of_Consecutive_Invalid_Samples();
      static uint32_t Get_Buffer_Capacity() { return buffer_capacity; }
      float32_t Get_Mean_Lateral_Position();
      float32_t Calc_Age_Based_Confidence();
      float32_t Calc_Lateral_Pos_Spread();
      float32_t Calc_Spread_Based_Confidence(const float32_t spread_weight);
      void Find_Farthest_Outlier_Sample();

      using Concrete_Wall_Samples_T = cmn::Circular_Buffer<Concrete_Wall_Sample_T, buffer_capacity>;
   private:
      const Concrete_Wall_Sample_T * farthest_outlier_sample;
      Concrete_Wall_Samples_T samples;
      float32_t samples_weights[buffer_capacity];

   public:
      const Concrete_Wall_Samples_T & Get_Samples() const { return this->samples; };
      void Set_Samples(const Concrete_Wall_Samples_T& samples_input) { this->samples = samples_input; };
   };
}
#endif
