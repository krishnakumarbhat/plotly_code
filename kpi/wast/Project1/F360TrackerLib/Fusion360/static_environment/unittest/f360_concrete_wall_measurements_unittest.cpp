/** \file
 * This file contains unit tests for content of concrete_wall_measurements.h file
 */
 // Unit testing guidelines: http://confluenceprod1.delphiauto.net:8090/display/F360Core/Unit+testing+guidelines

#include <CppUTest/TestHarness.h>
#include <cmath>
#include "f360_concrete_wall_measurements.h"
#include "f360_concrete_wall_sensor.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   /** \defgroup  concrete_wall_measurements
   *  @{
   */

   /** \brief
   * This test group is for testing Concrete_Wall_Measurements_T class
   */
   TEST_GROUP(concrete_wall_measurements)
   {
      F360_Calibrations_T calib;
      const unsigned cwd_buffer_capacity = Concrete_Wall_Measurements_T::Get_Buffer_Capacity();

      TEST_SETUP()
      {
         Initialize_Tracker_Calibrations(calib);
      }
   };

   /** \purpose
    * Test mean value when there was no samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_mean_on_empty_measurements_invalid_value_should_be_returned)
   {
      /** \precond
       * concrete wall measurements contain no samples
       */
      Concrete_Wall_Measurements_T concrete_wall_measurements;

      /** \action
      * call Get_Mean_Lateral_Distance() function
      */
      const float mean = concrete_wall_measurements.Get_Mean_Lateral_Position();

      /** \result
      * mean is NaN
      */
      CHECK_TRUE(std::isnan(mean));
   }

   /** \purpose
    * Test number of valid samples when there was no samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_number_of_valid_samples_on_empty_measurements_zero_should_be_returned)
   {
      /** \precond
       * concrete wall measurements contain no samples
       */
      Concrete_Wall_Measurements_T concrete_wall_measurements;

      /** \action
      * call Get_Number_Of_Valid_Samples() function
      */
      const unsigned number_of_valid_samples = concrete_wall_measurements.Get_Number_Of_Valid_Samples();

      /** \result
      * return value is equal to 0
      */
      const unsigned expected_number_of_valid_samples = 0U;
      CHECK_EQUAL(expected_number_of_valid_samples, number_of_valid_samples);
   }

   /** \purpose
    * Test number of valid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_number_of_valid_samples_number_of_valid_samples_from_latest_five_mesurements_should_be_returned)
   {
      /** \precond
       * concrete wall measurements contains both valid and invalid samples
       */
      Concrete_Wall_Sample_T sample_1, sample_2, sample_3, sample_4, sample_5, sample_6, sample_7;
      sample_2.is_valid = sample_3.is_valid = sample_6.is_valid = false;
      sample_1.is_valid = sample_4.is_valid = sample_5.is_valid = sample_7.is_valid = true;
      sample_1.lateral_position = 20.1F; //valid but will be overwritten
      sample_2.lateral_position = 127.42F;
      sample_3.lateral_position = 79.07F;
      sample_4.lateral_position = 11.9F; // valid
      sample_5.lateral_position = 7.3F;  // valid
      sample_6.lateral_position = 1.1F;
      sample_7.lateral_position = 3.3F;  // valid

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample_1);
      concrete_wall_measurements.Push(sample_2);
      concrete_wall_measurements.Push(sample_3);
      concrete_wall_measurements.Push(sample_4);
      concrete_wall_measurements.Push(sample_5);
      concrete_wall_measurements.Push(sample_6);
      concrete_wall_measurements.Push(sample_7);

      /** \action
      * call Get_Number_Of_Valid_Samples() function
      */
      const unsigned number_of_valid_samples = concrete_wall_measurements.Get_Number_Of_Valid_Samples();

      /** \result
      * return value is equal to 3
      */
      const unsigned expected_number_of_valid_samples = 3U;
      CHECK_EQUAL(expected_number_of_valid_samples, number_of_valid_samples);
   }

   /** \purpose
    * Test number of consecutive invalid samples when there was no samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_number_of_consecutive_invalid_samples_on_empty_measurements_zero_should_be_returned)
   {
      /** \precond
       * concrete wall measurements contain no samples
       */
      Concrete_Wall_Measurements_T concrete_wall_measurements;

      /** \action
      * call Get_Number_Of_Consecutive_Invalid_Samples() function
      */
      const unsigned number_of_consecutive_invalid_samples = concrete_wall_measurements.Get_Number_Of_Consecutive_Invalid_Samples();

      /** \result
      * return value is equal to 0
      */
      const unsigned expected_number_of_consecutive_invalid_samples = 0U;
      CHECK_EQUAL(expected_number_of_consecutive_invalid_samples, number_of_consecutive_invalid_samples);
   }

   /** \purpose
    * Test number of consecutive invalid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_number_of_consecutive_invalid_samples_number_of_latest_invalid_samples_should_be_returned)
   {
      /** \precond
       * concrete wall measurements contains 1 invalid + 1 valid + 3 invalid samples
       */
      Concrete_Wall_Sample_T sample_1, sample_2, sample_3, sample_4, sample_5;
      sample_1.is_valid = false;
      sample_2.is_valid = true;
      sample_3.is_valid = false;
      sample_4.is_valid = false;
      sample_5.is_valid = false;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample_1);
      concrete_wall_measurements.Push(sample_2);
      concrete_wall_measurements.Push(sample_3);
      concrete_wall_measurements.Push(sample_4);
      concrete_wall_measurements.Push(sample_5);

      /** \action
      * call Get_Number_Of_Valid_Samples() function
      */
      const unsigned number_of_consecutive_invalid_samples = concrete_wall_measurements.Get_Number_Of_Consecutive_Invalid_Samples();

      /** \result
      * return value is equal to 3
      */
      const unsigned expected_number_of_consecutive_invalid_samples = 3U;
      CHECK_EQUAL(expected_number_of_consecutive_invalid_samples, number_of_consecutive_invalid_samples);
   }

   /** \purpose
    * Test mean value when there is no valid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_mean_and_measurements_contain_only_invalid_samples_invalid_value_should_be_returned)
   {
      /** \precond
       * concrete wall measurements with 3 ivalid samples
       */
      Concrete_Wall_Sample_T sample_1, sample_2, sample_3;
      sample_1.is_valid = sample_2.is_valid = sample_3.is_valid = false;
      sample_1.lateral_position = 3.0F;
      sample_2.lateral_position = 4.0F;
      sample_3.lateral_position = 7.0F;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample_1);
      concrete_wall_measurements.Push(sample_2);
      concrete_wall_measurements.Push(sample_3);


      /** \action
      * call Get_Mean_Lateral_Distance() function
      */
      const float mean = concrete_wall_measurements.Get_Mean_Lateral_Position();

      /** \result
      * mean is NaN
      */

      CHECK_TRUE(std::isnan(mean));
   }

   /** \purpose
    * Test mean value when there is five valid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_mean_and_measurements_contain_five_valid_samples_mean_should_be_returned)
   {
      /** \precond
       * concrete wall measurements with 5 valid samples
       */
      Concrete_Wall_Sample_T sample_1, sample_2, sample_3, sample_4, sample_5;
      sample_1.is_valid = sample_2.is_valid = sample_3.is_valid = sample_4.is_valid = sample_5.is_valid = true;
      sample_1.lateral_position = 3.0F;
      sample_2.lateral_position = 4.4F;
      sample_3.lateral_position = 7.0F;
      sample_4.lateral_position = 1.1F;
      sample_5.lateral_position = 5.0F;


      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample_1);
      concrete_wall_measurements.Push(sample_2);
      concrete_wall_measurements.Push(sample_3);
      concrete_wall_measurements.Push(sample_4);
      concrete_wall_measurements.Push(sample_5);

      /** \action
      * call Get_Mean_Lateral_Position() function
      */
      const float mean = concrete_wall_measurements.Get_Mean_Lateral_Position();

      /** \result
      * mean is equal to 4.1
      */

      const float expected_mean = 4.1F;
      DOUBLES_EQUAL(expected_mean, mean, F360_EPSILON);
   }

   /** \purpose
    * Test mean value when there is three valid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, mean_should_be_calculated_when_we_have_only_part_of_valid_samples)
   {
      /** \precond
       * concrete wall measurements with 3 invalid samples and 4 valid samples. Only last 5 should be considered
       */
      Concrete_Wall_Sample_T sample_1, sample_2, sample_3, sample_4, sample_5, sample_6, sample_7;
      sample_2.is_valid = sample_3.is_valid = sample_6.is_valid = false;
      sample_1.is_valid = sample_4.is_valid = sample_5.is_valid = sample_7.is_valid = true;
      sample_1.lateral_position = 20.1F; //valid but will be overwritten
      sample_2.lateral_position = 127.42F;
      sample_3.lateral_position = 79.07F;
      sample_4.lateral_position = 11.9F; // valid
      sample_5.lateral_position = 7.3F;  // valid
      sample_6.lateral_position = 1.1F;
      sample_7.lateral_position = 3.3F;  // valid


      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample_1);
      concrete_wall_measurements.Push(sample_2);
      concrete_wall_measurements.Push(sample_3);
      concrete_wall_measurements.Push(sample_4);
      concrete_wall_measurements.Push(sample_5);
      concrete_wall_measurements.Push(sample_6);
      concrete_wall_measurements.Push(sample_7);

      /** \action
      * call Get_Mean_Lateral_Distance() function
      */
      const float mean = concrete_wall_measurements.Get_Mean_Lateral_Position();

      /** \result
      * mean is equal to 7.5
      */

      const float expected_mean = 7.5F;
      DOUBLES_EQUAL(expected_mean, mean, F360_EPSILON);
   }

   /** \purpose
    * Test mean value when there are only valid samples
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, when_calling_get_mean_and_measurements_contain_only_valid_samples_mean_of_all_samples_should_be_returned)
   {
      /** \precond
      * concrete wall measurements with 1 invalid samples
      */
      Concrete_Wall_Sample_T sample;
      sample.is_valid = true;
      sample.lateral_position = 3.73F;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      concrete_wall_measurements.Push(sample);

      /** \action
      * call Get_Mean_Lateral_Distance() function
      */
      const float mean = concrete_wall_measurements.Get_Mean_Lateral_Position();

      /** \result
      * mean is equal to 3.73
      */
      const float expected_mean = 3.73F;
      DOUBLES_EQUAL(expected_mean, mean, F360_EPSILON);
   }

   /** \purpose
    * Test if age based confidence is 1.0 when all samples in buffer are valid
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_age_based_confidence_is_equal_to_one_when_all_samples_in_buffer_are_valid)
   {
      /** \precond
      * concrete wall measurements with 5 valid samples
      */
      Concrete_Wall_Sample_T sample;
      sample.is_valid = true;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Age_Based_Confidence() function
      */
      const float confidence = concrete_wall_measurements.Calc_Age_Based_Confidence();

      /** \result
      * confidence should be equal to 1.0
      */
      const float expected_confidence = 1.0F;
      DOUBLES_EQUAL(expected_confidence, confidence, F360_EPSILON);
   }

   /** \purpose
    * Test if age based confidence is 0.0 when no valid samples in buffer.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_age_based_confidence_is_equal_to_zero_when_no_valid_samples_in_buffer)
   {
      /** \precond
      * concrete wall measurements with 3 invalid samples
      */
      Concrete_Wall_Sample_T sample;
      sample.is_valid = false;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Age_Based_Confidence() function
      */
      const float confidence = concrete_wall_measurements.Calc_Age_Based_Confidence();

      /** \result
      * confidence should be equal to 0.0
      */
      const float expected_confidence = 0.0F;
      DOUBLES_EQUAL(expected_confidence, confidence, F360_EPSILON);
   }

   /** \purpose
    * Test if spread is equal to zero when all samples have the same lateral position.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_is_equal_to_zero_when_all_samples_have_the_same_lateral_position)
   {
      /** \precond
      * concrete wall measurements with 5 valid samples
      */
      Concrete_Wall_Sample_T sample;
      sample.is_valid = true;
      sample.lateral_position = 1.0F;

      Concrete_Wall_Measurements_T concrete_wall_measurements;
      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Lateral_Pos_Spread() function
      */
      const float lat_posn_spread = concrete_wall_measurements.Calc_Lateral_Pos_Spread();

      /** \result
      * lat_posn_spread should be equal to 0.0
      */
      const float expected_lat_posn_spread = 0.0F;
      DOUBLES_EQUAL(expected_lat_posn_spread, lat_posn_spread, F360_EPSILON);
   }

   /** \purpose
    * Test if spread is equal to assumed value.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_is_equal_to_assumed_value)
   {
      /** \precond
      * concrete wall measurements with 5 valid samples with spread between them equal to 2.0
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample1, sample2, sample3, sample4, sample5;

      sample1.is_valid = true;
      sample1.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample1);

      sample2.is_valid = true;
      sample2.lateral_position = 3.0F;
      concrete_wall_measurements.Push(sample2);

      sample3.is_valid = true;
      sample3.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample3);

      sample4.is_valid = true;
      sample4.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample4);

      sample5.is_valid = true;
      sample5.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample5);

      /** \action
      * call Calc_Lateral_Pos_Spread() function
      */
      const float lat_posn_spread = concrete_wall_measurements.Calc_Lateral_Pos_Spread();

      /** \result
      * lat_posn_spread should be equal to 2.0
      */
      const float expected_lat_posn_spread = 2.0F;
      DOUBLES_EQUAL(expected_lat_posn_spread, lat_posn_spread, F360_EPSILON);
   }

   /** \purpose
    * Test if spread is equal to zero when only one valid sample in buffer.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_is_equal_to_zero_when_only_one_valid_sample_in_buffer)
   {
      /** \precond
      * concrete wall measurements with 1 valid sample.
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample1, sample2, sample3, sample4, sample5;

      sample1.is_valid = false;
      sample1.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample1);

      sample2.is_valid = false;
      sample2.lateral_position = 3.0F;
      concrete_wall_measurements.Push(sample2);

      sample3.is_valid = true;
      sample3.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample3);

      sample4.is_valid = false;
      sample4.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample4);

      sample5.is_valid = false;
      sample5.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample5);

      /** \action
      * call Calc_Lateral_Pos_Spread() function
      */
      const float lat_posn_spread = concrete_wall_measurements.Calc_Lateral_Pos_Spread();

      /** \result
      * lat_posn_spread should be equal to 0.0
      */
      const float expected_lat_posn_spread = 0.0F;
      DOUBLES_EQUAL(expected_lat_posn_spread, lat_posn_spread, F360_EPSILON);
   }

   /** \purpose
    * Test if spread value is set to NaN when no valid samples in buffer.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_value_is_set_to_NaN_when_no_valid_samples_in_buffer)
   {
      /** \precond
      * concrete wall measurements with 5 invalid samples
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample;

      sample.is_valid = false;
      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Lateral_Pos_Spread() function
      */
      const float lat_posn_spread = concrete_wall_measurements.Calc_Lateral_Pos_Spread();

      /** \result
      * lat_posn_spread should be NaN
      */
      CHECK_TRUE(std::isnan(lat_posn_spread));
   }

   /** \purpose
    * Test if spread confidence is equal to 1.0 when spread is equal to 0.0.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_confidence_is_equal_to_one_when_spread_is_equal_to_zero)
   {
      /** \precond
      * concrete wall measurements with 5 valid samples with spread between them equal to 2.0
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample;

      sample.is_valid = true;
      sample.lateral_position = 1.0F;

      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Spread_Based_Confidence() function
      */
      const float confidence = concrete_wall_measurements.Calc_Spread_Based_Confidence(calib.k_cwd_spread_weight);

      /** \result
      * confidence should be 1.0
      */
      const float expected_confidence = 1.0F;
      DOUBLES_EQUAL(expected_confidence, confidence, F360_EPSILON);
   }

   /** \purpose
    * Test if spread confidence is equal to 0.0 when no valid samples in buffer.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_spread_confidence_is_equal_to_zero_when_no_valid_samples_in_buffer)
   {
      /** \precond
      * concrete wall measurements with 5 invalid samples.
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample;

      sample.is_valid = false;
      sample.lateral_position = 1.0F;

      for (unsigned i = 0; i < cwd_buffer_capacity; ++i)
      {
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Calc_Spread_Based_Confidence() function
      */
      const float confidence = concrete_wall_measurements.Calc_Spread_Based_Confidence(calib.k_cwd_spread_weight);

      /** \result
      * confidence should be 0.0
      */
      const float expected_confidence = 0.0F;
      DOUBLES_EQUAL(expected_confidence, confidence, F360_EPSILON);
   }

   /** \purpose
    * Test if function returns last(newest) pushed sample, when buffer is not full.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_function_returns_last_newest_pushed_sample_when_buffer_is_not_full)
   {
      /** \precond
      * concrete wall measurements with 3 valid samples.
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample1, sample2, sample3;
      Concrete_Wall_Sample_T last_sample;

      sample1.is_valid = true;
      sample1.lateral_position = 1.0F;
      concrete_wall_measurements.Push(sample1);

      sample2.is_valid = true;
      sample2.lateral_position = 2.0F;
      concrete_wall_measurements.Push(sample2);

      sample3.is_valid = true;
      sample3.lateral_position = 3.0F;
      concrete_wall_measurements.Push(sample3);

      /** \action
      * call Get_Last_Sample() function
      */
      last_sample = concrete_wall_measurements.Get_Last_Sample();

      /** \result
      * sample nb. 3 should be returned
      */
      CHECK_TRUE(last_sample.is_valid)
      DOUBLES_EQUAL(3.0F, last_sample.lateral_position, F360_EPSILON);
   }

   /** \purpose
    * Test if function returns last(newest) pushed sample, when buffer is overwritten.
    * \req
    * NA.
    */
   TEST(concrete_wall_measurements, if_function_returns_last_newest_pushed_sample_when_buffer_is_overwritten)
   {
      /** \precond
      * pushed 6 valid samples to buffer.
      */
      Concrete_Wall_Measurements_T concrete_wall_measurements;
      Concrete_Wall_Sample_T sample;
      Concrete_Wall_Sample_T last_sample;

      for (unsigned i = 0; i < (cwd_buffer_capacity + 2U); ++i)
      {
         sample.is_valid = true;
         sample.lateral_position = static_cast<float>(i);
         concrete_wall_measurements.Push(sample);
      }

      /** \action
      * call Get_Last_Sample() function
      */
      last_sample = concrete_wall_measurements.Get_Last_Sample();

      /** \result
      * sample with lateral_position = 6.0 should be returned
      */
      CHECK_TRUE(last_sample.is_valid)
      DOUBLES_EQUAL(6.0F, last_sample.lateral_position, F360_EPSILON);
   }

   /** @}*/
}
