/** \file
 * This file contains unit tests for content of ta_estimate.cpp file
 */

#include "f360_trailer_detector_TA.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock TrailerDetectorTA in order to be able to access private members to make unit testing easier
class Trailer_Detector_TAmock : public Trailer_Detector_TA
{
   public:

   int16_t get_HV_cnt()
   {
      return HV_cnt;
   }

   void set_HV_cnt(int16_t HV_cnt_set_val)
   {
      HV_cnt = HV_cnt_set_val;
   }

   bool get_HV_start()
   {
      return HV_start;
   }

   void set_HV_start(bool HV_start_set_val)
   {
      HV_start = HV_start_set_val;
   }

   float32_t get_HV_angle()
   {
      return HV_angle;
   }

   void set_HV_angle(float32_t HV_angle_set_val)
   {
      HV_angle = HV_angle_set_val;
   }

   float32_t get_prev_trailer_angle()
   {
      return prev_trailer_angle;
   }

   void set_prev_trailer_angle(float32_t prev_trailer_angle_set_val)
   {
      prev_trailer_angle = prev_trailer_angle_set_val;
   }

   float32_t get_trailer_angle_rad()
   {
      return trailer_angle_rad;
   }

   void set_trailer_angle_rad(float32_t trailer_angle_deg_set_val)
   {
      trailer_angle_rad = trailer_angle_deg_set_val;
   }

   float32_t get_trailer_angle_rate_rad()
   {
      return trailer_angle_rate_rad;
   }

   void set_trailer_angle_rate_rad(float32_t trailer_angle_rate_deg_set_val)
   {
      trailer_angle_rate_rad = trailer_angle_rate_deg_set_val;
   }

   TA_Cals get_ta_calibs()
   {
      return ta_calibs;
   }

   float32_t get_trailer_axle_length()
   {
      return trailer_axle_length;
   }

   void estimate_mock(const F360_Host_T &vehicle_data, float32_t elapsed_time_s)
   {
      Estimate(vehicle_data, elapsed_time_s);
   }
};

/** \defgroup  ta_estimate_Set_Clear_Data
 *  @{
 */

/** \brief
 * This test group checks that the TrailerDetectorTA class implementation is correctly initializing/setting and clearing data correctly.
 */
TEST_GROUP(ta_estimate_Set_Clear_Data)
{	
   // Create an instance of the TrailerDetectorTA class
   Trailer_Detector_TAmock trailer_detector_TA;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   /** \setup
    * Initialize the TrailerDetectorTA class instance with some non-default values
    */
   TEST_SETUP()
   {
      trailer_detector_TA.set_HV_cnt(23);
      trailer_detector_TA.set_HV_start(true);
      trailer_detector_TA.set_HV_angle(-432.1F);
      trailer_detector_TA.set_prev_trailer_angle(-12.4F);
      trailer_detector_TA.set_trailer_angle_rad(345.6F);
      trailer_detector_TA.set_trailer_angle_rate_rad(111.1F);
      trailer_detector_TA.Set_Trailer_Axle_Length(7.6F);
   }   
};

/** \purpose  
 * Test that the TrailerDetectorTA constructor works as intended
 * \req
 * NA.
 */
TEST(ta_estimate_Set_Clear_Data, Test_Constructor)
{
   /** \precond
    * Create a new instance of the TrailorDetectorTA class.
    */
   Trailer_Detector_TAmock new_trailer_detector_TA;

   /** \result
    * Check that the constructor has initialized class members as expected.
    */
   const int16_t exp_HV_cnt = 0;
   CHECK_EQUAL_TEXT(exp_HV_cnt, new_trailer_detector_TA.get_HV_cnt(), "HV_cnt is not initialized correctly");

   CHECK_FALSE_TEXT(new_trailer_detector_TA.get_HV_start(), "HV_start is not initialized correctly");

   const float32_t exp_HV_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_HV_angle, new_trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle is not initialized correctly");

   const float32_t exp_prev_trailer_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_prev_trailer_angle, new_trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle is not initialized correctly");

   const float32_t exp_trailer_angle_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_deg, new_trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad is not initialized correctly");

   const float32_t exp_trailer_angle_rate_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_deg, new_trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad is not initialized correctly");

   const float32_t exp_trailer_axle_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_axle_length, new_trailer_detector_TA.get_trailer_axle_length(), test_pass_th, "trailer_axle_length is not initialized correctly");
}


/** \purpose  
 * Test that Trailer_Detector_TA::Initialize() works as intended
 * \req
 * NA.
 */
TEST(ta_estimate_Set_Clear_Data, Test_Initalization)
{
   /** \precond
    * Use the default test data from the test setup.
    */

   /** \action
    * Run initialization function
    */
   trailer_detector_TA.Initialize();

   /** \result
    * Check that the function call has initialized class members as expected.
    */
   const int16_t exp_HV_cnt = 0;
   CHECK_EQUAL_TEXT(exp_HV_cnt, trailer_detector_TA.get_HV_cnt(), "HV_cnt is not initialized correctly");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start is not initialized correctly");

   const float32_t exp_HV_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_HV_angle, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle is not initialized correctly");

   const float32_t exp_prev_trailer_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_prev_trailer_angle, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle is not initialized correctly");

   const float32_t exp_trailer_angle_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_deg, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad is not initialized correctly");

   const float32_t exp_trailer_angle_rate_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_deg, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad is not initialized correctly");

   const float32_t exp_trailer_axle_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_axle_length, trailer_detector_TA.get_trailer_axle_length(), test_pass_th, "trailer_axle_length is not initialized correctly");
}

/** \purpose  
 * Test that TrailerDetectorTA::Clear() works as intended
 * \req
 * NA.
 */
TEST(ta_estimate_Set_Clear_Data, Test_Clear)
{
   /** \precond
    * Use the default test data from the test setup.
    * Extract values of f_trailer_angle_deg_conf, f_trailer_angle_rate_deg_conf and trailer_axle_length before test run (to be able to check later that these are unchanged)
    */
   const float32_t trailer_axle_length_before = trailer_detector_TA.get_trailer_axle_length();

   /** \action
    * Run Clear function
    */
   trailer_detector_TA.Clear();

   /** \result
    * Check that the function call has cleared class members as expected.
    */
   const int16_t exp_HV_cnt = 0;
   CHECK_EQUAL_TEXT(exp_HV_cnt, trailer_detector_TA.get_HV_cnt(), "HV_cnt is not cleared correctly");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start is not cleared correctly");

   const float32_t exp_HV_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_HV_angle, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle is not cleared correctly");

   const float32_t exp_prev_trailer_angle = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_prev_trailer_angle, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle is not cleared correctly");

   const float32_t exp_trailer_angle_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_deg, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad is not cleared correctly");

   const float32_t exp_trailer_angle_rate_deg = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_deg, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad is not cleared correctly");

   DOUBLES_EQUAL_TEXT(trailer_axle_length_before, trailer_detector_TA.get_trailer_axle_length(), test_pass_th, "trailer_axle_length is not cleared correctly");
}

/** \purpose  
 * Test that TrailerDetectorTA::Set_Trailer_Axle_Length() works as intended
 * \req
 * NA.
 */
TEST(ta_estimate_Set_Clear_Data, Test_Set_Trailer_Axle_Length)
{
   /** \precond
    * Use the default test data from the test setup.
    */

   /** \action
    * Run Set_Trailer_Axle_Length()
    */
   const float32_t exp_trailer_axle_length = 0.51234F;
   trailer_detector_TA.Set_Trailer_Axle_Length(exp_trailer_axle_length);

   /** \result
    * Check that the trailer_axle_length are correctly set.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_axle_length, trailer_detector_TA.get_trailer_axle_length(), test_pass_th, "trailer_axle_length is not set correctly");
}
/** @}*/


/** \defgroup  ta_estimate_Estimate
 *  @{
 */

/** \brief
 * This test group checks that the TrailerDetectorTA::estimate function is working as expected.
 */
TEST_GROUP(ta_estimate_Estimate)
{	
   // Create an instance of the TrailerDetectorTA class
   Trailer_Detector_TAmock trailer_detector_TA;

   // Get a local copy of the private calibration struct
   const TA_Cals ta_calibs = trailer_detector_TA.get_ta_calibs();

   // Create a structure for host vehicle data
   F360_Host_T vehicle_data;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-4F;

   const float32_t elapsed_time_s = 0.05F;

   /** \setup
    * Initialize the TrailerDetectorTA class instance with some default values such that we update trailer angle estimate if we get valid host properties for one more iteration
    * Initialize host vehicle data such that it enables increase of the HV_cnt counter
    */
   TEST_SETUP()
   {
      // Initialze  TrailerDetectorTA class instance
      trailer_detector_TA.set_HV_cnt(ta_calibs.HV_cnt_threshold - 1); // One less than ta_calibs.HV_cnt_threshold such that we need one more interation with valid host properties in order to start updating angle estimate
      trailer_detector_TA.set_HV_start(false); // Set to false to indicate that ta_calibs.HV_cnt_threshold is too small
      trailer_detector_TA.set_HV_angle(0.087F); // [rad] Approximately 5deg
      trailer_detector_TA.set_trailer_angle_rad(0.087F); // [deg] Same as HV_angle
      trailer_detector_TA.set_prev_trailer_angle(0.087F); // [deg] Set to same as trailer_angle_rad
      trailer_detector_TA.set_trailer_angle_rate_rad(-0.040F); // [deg/s] Set to something non-default
      trailer_detector_TA.Set_Trailer_Axle_Length(1.0F + F360_EPSILON); // Set to slightly larger than 1m

      // Initialze host vehicle data
      vehicle_data.speed = ta_calibs.speed_threshold + F360_EPSILON; // Set absolute value of speed to slightly above the threshold for valid speed and sign of speed to positive to enable increase of HV_cnt
      vehicle_data.yaw_rate_rad = -(0.02F - F360_EPSILON); // Set absolute value to slightly below threshold for valid yaw rate to enable increase of HV_cnt
      vehicle_data.vcs_sideslip = 0.026F; // [rad] Approximately 1.5deg
      vehicle_data.dist_rear_axle_to_vcs_m = 4.2F;
   }   
};

/** \purpose  
 * Test that TrailerDetectorTA::estimates() updates both HV_cnt and angle as expected when the trailer_axle_length is larger than 1m.
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_HV_cnt_And_Angle_Updated_When_trailer_axle_length_Larger_Than_1m)
{
   /** \precond
    * Use the default test data from the test setup.
    * Extract the value of HV_angle before function call (so that we can later check that is has increased/decreased with the correct value)
    * Extract previous trailer angle before function call (so that we can later check that trailer_angle_rate_rad has been correctly updated)
    */
   const float32_t HV_angle_before = trailer_detector_TA.get_HV_angle();
   const float32_t prev_trailer_angle_before = trailer_detector_TA.get_prev_trailer_angle();

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is increased by 1.
    * Check that HV_start has been set to true
    * Check that HV_angle is increased/decreased correctly
    * Check that trailer_angle_rad has been corectly updated and now equals HV_angle but with the difference that  HV_angle is in radians and trailer_angle_rad is in degrees
    * Check that prev_trailer_angle has been corectly updated such that it equals trailer_angle_rad
    * Check that trailer_angle_rate_rad has been correctly updated
    */
   CHECK_EQUAL_TEXT(ta_calibs.HV_cnt_threshold, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_TRUE_TEXT( trailer_detector_TA.get_HV_start(), "HV_start has not ben set to true");

   const float32_t exp_HV_angle_increase = -0.00176994503F;
   const float32_t exp_HV_angle = HV_angle_before + exp_HV_angle_increase;
   DOUBLES_EQUAL_TEXT(exp_HV_angle, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle has not been corectly updated");

   const float32_t exp_trailer_angle_rad = exp_HV_angle;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad has not been corectly updated");

   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle has not been corectly updated");
   
   const float32_t exp_trailer_angle_rate_rad = (exp_trailer_angle_rad - prev_trailer_angle_before) / ta_calibs.delta_t;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_rad, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad has not been corectly updated");
}


/** \purpose  
 * Test that TrailerDetectorTA::estimates() updates both HV_cnt and angle as expected when the trailer_axle_length is smaller than 1m.
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_HV_cnt_And_Angle_Updated_When_trailer_axle_length_Smaller_Than_1m)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change trailer_axle_length to be smaller than 1m
    * Extract the value of HV_angle before function call (so that we can later check that is has increased/decreased with the correct value)
    * Extract previous trailer angle before function call (so that we can later check that trailer_angle_rate_rad has been correctly updated)
    */
   trailer_detector_TA.Set_Trailer_Axle_Length(1.0F - F360_EPSILON);
   const float32_t HV_angle_before = trailer_detector_TA.get_HV_angle();
   const float32_t prev_trailer_angle_before = trailer_detector_TA.get_prev_trailer_angle();

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is increased by 1.
    * Check that HV_start has been set to true
    * Check that HV_angle is increased/decreased correctly
    * Check that trailer_angle_rad has been corectly updated and now equals HV_angle but with the difference that  HV_angle is in radians and trailer_angle_rad is in degrees
    * Check that prev_trailer_angle has been corectly updated such that it equals trailer_angle_rad
    * Check that trailer_angle_rate_rad has been correctly updated
    */
   CHECK_EQUAL_TEXT(ta_calibs.HV_cnt_threshold, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_TRUE_TEXT( trailer_detector_TA.get_HV_start(), "HV_start has not ben set to true");

   const float32_t exp_HV_angle_increase = -0.001197105779690F;
   const float32_t exp_HV_angle = HV_angle_before + exp_HV_angle_increase;
   DOUBLES_EQUAL_TEXT(exp_HV_angle, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle has not been corectly updated");

   const float32_t exp_trailer_angle_rad = exp_HV_angle;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad has not been corectly updated");

   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle has not been corectly updated");
   
   const float32_t exp_trailer_angle_rate_deg = (exp_trailer_angle_rad - prev_trailer_angle_before) / ta_calibs.delta_t;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_deg, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad has not been corectly updated");
}


/** \purpose  
 * Test that TrailerDetectorTA::estimates() increases HV_cnt by 1 but does not update angle estimate when HV_cnt is not yet saturated.
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_HV_cnt_Updated_But_Angle_Is_Not_When_HV_cnt_Not_Reached_Max)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change HV_cnt such that it will not saturate at its maximum value.
    * Extract the value of HV_angle before function call (so that we can later check that is has not been updated)
    */
   trailer_detector_TA.set_HV_cnt(ta_calibs.HV_cnt_threshold - 2);

   const float32_t HV_angle_before = trailer_detector_TA.get_HV_angle();

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is increased by 1.
    * Check that HV_start is not set to true
    * Check that HV_angle is not updated
    * Check that trailer_angle_rad has been corectly set to equal HV_angle but with the difference that HV_angle is in radians and trailer_angle_rad is in degrees
    * Check that prev_trailer_angle has been corectly set such that it equals trailer_angle_rad
    * Check that trailer_angle_rate_rad has been correctly set to 0
    */
   CHECK_EQUAL_TEXT(ta_calibs.HV_cnt_threshold - 1, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start has been wrongly set to true");

   DOUBLES_EQUAL_TEXT(HV_angle_before, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle has not been corectly updated");

   const float32_t exp_trailer_angle_rad = HV_angle_before;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad has the wrong value");

   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle has the wrong value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad has not been corectly updated");
}


/** \purpose  
 * Test that TrailerDetectorTA::estimates() is resetting HV_cnt to 0 and don't update the angles when host vehicle speed is smaller than ta_calibs.speed_threshold and positive.
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_Not_Updated_blocked_by_Small_Speed)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change host speed such that it is slightly below ta_calibs.speed_threshold (but positive)
    * Extract the value of HV_angle before function call (so that we can later check that is has not been updated)
    */
   vehicle_data.speed = ta_calibs.speed_threshold - F360_EPSILON;

   const float32_t HV_angle_before = trailer_detector_TA.get_HV_angle();

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is 0.
    * Check that HV_start is not set to true
    * Check that HV_angle is not updated
    * Check that trailer_angle_rad has been corectly set to equal HV_angle but with the difference that HV_angle is in radians and trailer_angle_rad is in degrees
    * Check that prev_trailer_angle has been corectly set such that it equals trailer_angle_rad
    */
   CHECK_EQUAL_TEXT(0, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start has been wrongly set to true");

   DOUBLES_EQUAL_TEXT(HV_angle_before, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle has been modified even thoug it shouldn' be");

   const float32_t exp_trailer_angle_rad = HV_angle_before;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad has the wrong value");

   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle has the wrong value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad has the wrong value");
}


/** \purpose  
 * Test that TrailerDetectorTA::estimates() is resetting HV_cnt to 0 and don't update the angles when host vehicle yawrate is larger than 0.02
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_Not_Updated_blocked_by_Large_Yaw_Rate)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change host yaw rate such that it is slightly above 0.02
    * Extract the value of HV_angle before function call (so that we can later check that is has not been updated)
    */
   vehicle_data.yaw_rate_rad = -(0.02F + F360_EPSILON);

   const float32_t HV_angle_before = trailer_detector_TA.get_HV_angle();

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is 0.
    * Check that HV_start is not set to true
    * Check that HV_angle is not updated
    * Check that trailer_angle_rad has been corectly set to equal HV_angle but with the difference that HV_angle is in radians and trailer_angle_rad is in degrees
    * Check that prev_trailer_angle has been corectly set such that it equals trailer_angle_rad
    */
   CHECK_EQUAL_TEXT(0, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start has been wrongly set to true");

   DOUBLES_EQUAL_TEXT(HV_angle_before, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle has been modified even thoug it shouldn' be");

   const float32_t exp_trailer_angle_rad = HV_angle_before;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_rad has the wrong value");

   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle has the wrong value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad has the wrong value");
}


/** \purpose  
 * Test that TrailerDetectorTA::estimates() is clearing data when host speed is negative and (absolute value is larger than ta_calibs.speed_threshold)
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_Clear_When_Speed_Is_Negative_And_Abs_Value_is_Large)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change host speed such that it is negative
    * Extract the value of HV_angle before function call (so that we can later check that is has not been updated)
    */
   vehicle_data.speed = -(ta_calibs.speed_threshold + F360_EPSILON);

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is 0.
    * Check that HV_start is false
    * Check that HV_angle is 0
    * Check that trailer_angle_rad is 0
    * Check that prev_trailer_angle is 0
    */
   CHECK_EQUAL_TEXT(0, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start has been wrongly set to true");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle is non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_degis non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle is non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad is non-zero");
}

/** \purpose  
 * Test that TrailerDetectorTA::estimates() is clearing data when host speed is negative and absolute host speed is larger than 
 * \req
 * NA.
 */
TEST(ta_estimate_Estimate, Test_Clear_When_Speed_Is_Negative)
{
   /** \precond
    * Use the default test data from the test setup except for:
    *    Change host speed such that it is negative
    * Extract the value of HV_angle before function call (so that we can later check that is has not been updated)
    */
   vehicle_data.speed = -F360_EPSILON;

   /** \action
    * Run estimate()
    */
   trailer_detector_TA.estimate_mock(vehicle_data, elapsed_time_s);

   /** \result
    * Check that the HV_cnt is 0.
    * Check that HV_start is false
    * Check that HV_angle is 0
    * Check that trailer_angle_rad is 0
    * Check that prev_trailer_angle is 0
    */
   CHECK_EQUAL_TEXT(0, trailer_detector_TA.get_HV_cnt(), "HV_cnt has not increased by 1");

   CHECK_FALSE_TEXT(trailer_detector_TA.get_HV_start(), "HV_start has been wrongly set to true");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_HV_angle(), test_pass_th, "HV_angle is non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rad(), test_pass_th, "trailer_angle_degis non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_prev_trailer_angle(), test_pass_th, "prev_trailer_angle is non-zero");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_TA.get_trailer_angle_rate_rad(), test_pass_th, "trailer_angle_rate_rad is non-zero");
}
/** @}*/
