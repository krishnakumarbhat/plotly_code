/** \file
 * This file contains unit tests for content of tw_initialize_cals.cpp file
 */

#include "f360_trailer_detector_TW.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

// Mock class for Trailer_Detector_TW in order to access the protected members to make unit testing easier
class Trailer_Detector_TWmock : public Trailer_Detector_TW
{
   public:

   TW_Cals get_tw_calibs()
   {
      return tw_calibs;
   }
};

/** \defgroup  tw_initialize_cals
 *  @{
 */

/** \brief
 *  Verify the private struct tw_calibs is initialized to the correct values
 */
TEST_GROUP(tw_initialize_cals)
{	
   // Create an instance of the Trailer_Detector_TW class
   Trailer_Detector_TWmock trailer_detector_TW;

   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   /** \setup
    * None.
    */

};

/** \purpose  
 *  Verify the private struct tw_calibs is initialized to the correct values
 */
TEST(tw_initialize_cals, Test_Calibs_Correctly_Initialized)
{
   /** \precond
    * None.
    */

   /** \action
    * None.
    */
   
   /** \result
    * Verify that all calibration values have been set to expected default values
    */
   const float32_t exp_k_speed_threshold = 0.2F;
   const float32_t exp_k_ZRRateGate = 0.3F;
   const float32_t exp_k_host_length = 5.5F;
   const float32_t exp_k_max_trailer_length = 12.0F;
   const float32_t exp_k_max_trailer_width = 3.0F;
   const float32_t exp_k_col_interval = 0.1F;
   const uint32_t exp_k_window_timer_threshold = 800U;
   const uint32_t exp_k_reset_time_threshold = 1800U;

   DOUBLES_EQUAL_TEXT(exp_k_speed_threshold, trailer_detector_TW.get_tw_calibs().k_speed_threshold, test_pass_th, "k_speed_threshold has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_ZRRateGate, trailer_detector_TW.get_tw_calibs().k_ZRRateGate, test_pass_th, "k_ZRRateGate has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_host_length, trailer_detector_TW.get_tw_calibs().k_host_length, test_pass_th, "k_host_length has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_max_trailer_length, trailer_detector_TW.get_tw_calibs().k_max_trailer_length, test_pass_th, "k_max_trailer_length has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_max_trailer_width, trailer_detector_TW.get_tw_calibs().k_max_trailer_width, test_pass_th, "k_max_trailer_width has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_col_interval, trailer_detector_TW.get_tw_calibs().k_col_interval, test_pass_th, "k_col_interval has not been set to the expected value");
   CHECK_EQUAL_TEXT(exp_k_window_timer_threshold, trailer_detector_TW.get_tw_calibs().k_window_timer_threshold, "k_window_timer_threshold has not been set to the expected value");
   CHECK_EQUAL_TEXT(exp_k_reset_time_threshold, trailer_detector_TW.get_tw_calibs().k_reset_time_threshold, "k_reset_time_threshold has not been set to the expected value");   
   CHECK_TRUE_TEXT((trailer_detector_TW.get_tw_calibs().k_col_interval > F360_EPSILON), "k_col_interval is smaller than expected value");
}
/** @}*/
