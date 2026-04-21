/** \file
 * This file contains unit tests for content of ta_run.cpp file
 */

#include "f360_trailer_detector_TA.h"
#include <CppUTest/TestHarness.h>


// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock TrailerDetectorTA in order to be able to access private members to make unit testing easier
class Trailer_Detector_TAmock : public Trailer_Detector_TA
{
   public:

   void set_HV_cnt(int16_t HV_cnt_set_val)
   {
      HV_cnt = HV_cnt_set_val;
   }

   void set_HV_start(bool HV_start_set_val)
   {
      HV_start = HV_start_set_val;
   }

   void set_HV_angle(float32_t HV_angle_set_val)
   {
      HV_angle = HV_angle_set_val;
   }

   void set_prev_trailer_angle(float32_t prev_trailer_angle_set_val)
   {
      prev_trailer_angle = prev_trailer_angle_set_val;
   }

   void set_trailer_angle_rad(float32_t trailer_angle_deg_set_val)
   {
      trailer_angle_rad = trailer_angle_deg_set_val;
   }

   void set_trailer_angle_rate_rad(float32_t trailer_angle_rate_deg_set_val)
   {
      trailer_angle_rate_rad = trailer_angle_rate_deg_set_val;
   }

   TA_Cals get_ta_calibs()
   {
      return ta_calibs;
   }
};

/** \defgroup  ta_run
 *  @{
 */

/** \brief
 * This test group contains a collection of test for the Trailer_Detector_TA::Run_Trailer_Detector() function
 */
TEST_GROUP(ta_run)
{	
   // Create an instance of the Trailer_Detector_TA class
   Trailer_Detector_TAmock trailer_detector_TA;

   // Get a local copy of the private calibration struct
   const TA_Cals ta_calibs = trailer_detector_TA.get_ta_calibs();

   // Create a structure for host vehicle data
   F360_Host_T vehicle_data;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-4F;

   float32_t elapsed_time_s = 0.05F;

   /** \setup
    * Initialize the TrailerDetectorTA class instance with some default values such that we update trailer angle estimate if we get valid host properties for one more iteration
    * Initialize host vehicle data such that it enables increase of the HV_cnt counter
    */
   TEST_SETUP()
   {
      trailer_detector_TA.set_HV_cnt(ta_calibs.HV_cnt_threshold - 1); // One less than ta_calibs.HV_cnt_threshold such that we need one more interation with valid host properties in order to start updating angle estimate
      trailer_detector_TA.set_HV_start(false); // Set to false to indicate that ta_calibs.HV_cnt_threshold is too small
      trailer_detector_TA.set_HV_angle(0.087F); // [rad] Approximately 5deg
      trailer_detector_TA.set_trailer_angle_rad(0.087F); // [deg] Same as HV_angle
      trailer_detector_TA.set_prev_trailer_angle(0.087F); // [deg] Set to same as trailer_angle_rad
      trailer_detector_TA.set_trailer_angle_rate_rad(-2.3F); // [deg/s] Set to something non-default
      trailer_detector_TA.Set_Trailer_Axle_Length(1.0F + F360_EPSILON); // Set to slightly larger than 1m

      // Initialze host vehicle data
      vehicle_data.speed = ta_calibs.speed_threshold + F360_EPSILON; // Set absolute value of speed to slightly above the threshold for valid speed and sign of speed to positive to enable increase of HV_cnt
      vehicle_data.yaw_rate_rad = -(0.02F - F360_EPSILON); // Set absolute value to slightly below threshold for valid yaw rate to enable increase of HV_cnt
      vehicle_data.vcs_sideslip = 0.026F; // [rad] Approximately 1.5deg
      vehicle_data.dist_rear_axle_to_vcs_m = 4.2F; // [rad] Approximately 1.5deg
   }
};

/** \purpose  
 * Test that TrailerDetectorTA::estimates() updates both HV_cnt and angle as expected when the trailer_axle_length is larger than 1m.
 * \req
 * NA.
 */
TEST(ta_run, Test_HV_cnt_And_Angle_Updated_Correctly)
{
   /** \precond
    * Use the default test data from the test setup.
    */

   /** \action
    * Run Run_Trailer_Detector() and extract class output after function call
    */
   trailer_detector_TA.Run_Trailer_Detector(vehicle_data, elapsed_time_s);
   TA_Output ta_output_vals;
   trailer_detector_TA.Get_Output(ta_output_vals);

   /** \result
    * Check that trailer_angle_rad has been corectly computed
    */

   const float32_t exp_trailer_angle_rad = 0.08521155912F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rad, ta_output_vals.trailer_angle_rad, test_pass_th, "trailer_angle_rad has not been correctly computed");

   const float32_t exp_trailer_angle_rate_rad = -0.0353989F;
   DOUBLES_EQUAL_TEXT(exp_trailer_angle_rate_rad, ta_output_vals.trailer_angle_rate_rad, test_pass_th, "trailer_angle_rate_rad has not been correctly computed");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, ta_output_vals.confidence, "confidence has not been correctly computed");
}
/** @}*/

