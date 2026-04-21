/*===========================================================================*\
* FILE: f360_concrete_wall_measurements.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definitions for f360_concrete_wall_measurements.h
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "f360_concrete_wall_measurements.h"
#include <limits>
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Concrete_Wall_Measurements_T()
   *
   * Description    Constructor of Concrete_Wall_Measurements_T class.
   *
   * Parameters     None
   * Returns        None
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Concrete_Wall_Measurements_T::Concrete_Wall_Measurements_T() :samples()
   {
      farthest_outlier_sample = nullptr;

      for (uint32_t i = buffer_capacity; i > 0U; --i)
      {
         samples_weights[i - 1U] = static_cast<float32_t>(i) / static_cast<float32_t>(buffer_capacity);
      }
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Reset()
   *
   * Description    Resets data inside Concrete_Wall_Measurements_T object.
   *
   * Parameters     None
   * Returns        None
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Concrete_Wall_Measurements_T::Reset()
   {
      farthest_outlier_sample = nullptr;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Push
   *
   * Description    Push new sample in to the measurements.
   *
   * Parameters     const Concrete_Wall_Sample_T& sample - sample that will be added to measurements
   * Returns        None
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Concrete_Wall_Measurements_T::Push(const Concrete_Wall_Sample_T& sample)
   {
      samples.push(sample);
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Get_Mean_Lateral_Position
   *
   * Description    Returns mean value of lateral_position from all valid samples. If there are no valid samples available, method will return 'std::numeric_limits<float32_t>::quiet_NaN()'
   *
   * Parameters     None
   * Returns        float32_t - mean value of lateral_position from all valid samples or 'std::numeric_limits<float32_t>::quiet_NaN()' if there are no valid samples available.
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   float32_t Concrete_Wall_Measurements_T::Get_Mean_Lateral_Position()
   {
      float32_t sum = 0.0F;
      uint32_t number_of_valid_samples = 0U;
      for (uint32_t i = 0U; i < samples.size(); ++i)
      {
         if (samples[i].is_valid && (&samples[i] != farthest_outlier_sample))
         {
            ++number_of_valid_samples;
            sum += samples[i].lateral_position;
         }
      }

      return number_of_valid_samples == 0U ? std::numeric_limits<float32_t>::quiet_NaN() :
         sum / static_cast<float32_t>(number_of_valid_samples);
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Get_Last_Sample
   *
   * Description    Returns last(newest) sample added to buffer
   *
   * Parameters     None
   * Returns        Concrete_Wall_Sample_T - last added to buffer sample
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Concrete_Wall_Sample_T Concrete_Wall_Measurements_T::Get_Last_Sample()
   {
      return samples[samples.size() - 1U];
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Get_Number_Of_Valid_Samples
   *
   * Description    Returns number of samples with 'is_valid' flag set to true
   *
   * Parameters     None
   * Returns        std::size_t - number of valid samples
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   std::size_t Concrete_Wall_Measurements_T::Get_Number_Of_Valid_Samples()
   {
      std::size_t number_of_valid_samples = 0U;
      for (std::size_t i = 0U; i < samples.size(); ++i)
      {
         const Concrete_Wall_Sample_T& sample = samples[i];
         if (sample.is_valid)
         {
            ++number_of_valid_samples;
         }
      }
      return number_of_valid_samples;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Get_Number_Of_Consecutive_Invalid_Samples
   *
   * Description    Returns number of consecutive invalid samples at the end of the 'samples' buffer.
   *
   * Parameters     None
   * Returns        std::size_t - Returns number of consecutive invalid samples
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   std::size_t Concrete_Wall_Measurements_T::Get_Number_Of_Consecutive_Invalid_Samples()
   {
      std::size_t number_of_consecutive_invalid_samples = 0U;
      for (std::size_t i = samples.size(); i > 0U; --i)
      {
         const Concrete_Wall_Sample_T& sample = samples[i - 1U];
         if (sample.is_valid)
         {
            break;
         }
         else
         {
            ++number_of_consecutive_invalid_samples;
         }
      }
      return number_of_consecutive_invalid_samples;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Calc_Age_Based_Confidence()
   *
   * Description    Returns confidence calculated based on age of valid samples in buffer.
   *
   * Parameters     None
   * Returns        float32_t - Returns calculated confidence value.
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   float32_t Concrete_Wall_Measurements_T::Calc_Age_Based_Confidence()
   {
      float32_t sum_valid_dets_age_weights = 0.0F;
      float32_t sum_age_weights = 0.0F;

      for (uint32_t i = 0U; i < buffer_capacity; ++i)
      {
         if (samples[i].is_valid && (&samples[i] != farthest_outlier_sample))
         {
            sum_valid_dets_age_weights += samples_weights[i];
         }

         sum_age_weights += samples_weights[i];
      }
      return sum_valid_dets_age_weights / sum_age_weights;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Calc_Lateral_Pos_Spread()
   *
   * Description    Returns lateral spread of valid samples i buffer.
   *
   * Parameters     None
   * Returns        float32_t - Returns lateral spread value.
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   float32_t Concrete_Wall_Measurements_T::Calc_Lateral_Pos_Spread()
   {
      float32_t valid_samples_lat_pos[buffer_capacity] = {};
      uint32_t samples_count = 0U;
      float32_t spread_val = 0.0F;

      for (uint32_t i = 0U; i < samples.size(); ++i)
      {
         if (samples[i].is_valid && (&samples[i] != farthest_outlier_sample))
         {
            valid_samples_lat_pos[samples_count] = samples[i].lateral_position;
            ++samples_count;
         }
      }

      if (samples_count > 0U)
      {
         const float32_t min_lat_pos = F360_Min_Element(valid_samples_lat_pos, samples_count);
         const float32_t max_lat_pos = F360_Max_Element(valid_samples_lat_pos, samples_count);

         spread_val = max_lat_pos - min_lat_pos;
      }
      else
      {
         spread_val = std::numeric_limits<float32_t>::quiet_NaN();
      }

      return spread_val;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Calc_Spread_Based_Confidence()
   *
   * Description    Returns confidence value calculated based on lateral spread.
   *
   * Parameters     const float32_t & spread_weight
   * Returns        float32_t - Returns confidence value.
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   float32_t Concrete_Wall_Measurements_T::Calc_Spread_Based_Confidence(const float32_t spread_weight)
   {
      const float32_t lat_pos_spread = Calc_Lateral_Pos_Spread();
      float32_t confidence = 0.0F;

      if (!std::isnan(lat_pos_spread))
      {
         confidence = 1.0F / std::max(1.0F, spread_weight * lat_pos_spread);
      }
      return confidence;
   }

   /*=========================================================================
   * Method         Concrete_Wall_Measurements_T::Find_Farthest_Outlier_Sample()
   *
   * Description    Finds outlier sample inside samples buffer and store its pointer inside Concrete_Wall_Measurements_T object.
   *
   * Parameters     None.
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Concrete_Wall_Measurements_T::Find_Farthest_Outlier_Sample()
   {
      uint32_t samples_count = 0U;
      Concrete_Wall_Sample_T* samples_ptrs[buffer_capacity]{};

      for (uint32_t i = 0U; i < samples.size(); ++i)
      {
         if (samples[i].is_valid)
         {
            samples_ptrs[samples_count] = &samples[i];
            ++samples_count;
         }
      }

      farthest_outlier_sample = *std::max_element(&samples_ptrs[0], &samples_ptrs[samples_count],
         [](const Concrete_Wall_Sample_T* const left, const Concrete_Wall_Sample_T* const right) { return std::abs(left->lateral_position) < std::abs(right->lateral_position); });
   }
}
