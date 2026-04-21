/** \file
 * This file contains unit tests for content of tl_initialize_cals.cpp file
 */

#include "f360_trailer_detector_TL.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

// Mock Trailer_Detector_TL in order to be able to access private members to make unit testing easier
class TrailerDetectorTLmock : public Trailer_Detector_TL
{
   public:
   TL_Cals get_TL_cals()
   {
      return tl_calibs;
   }
};

/** \defgroup  tl_initialize_cals
 *  @{
 */

/** \brief
 * This test group is intended to verify that the private struct tl_calibs is initialized as expected.
 */
TEST_GROUP(tl_initialize_cals)
{	
   // Create an instance of the Trailer_Detector_TP class
   TrailerDetectorTLmock trailer_detector_TL;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   /** \setup
    * None.
    */
};

/** \purpose  
 * Verify that the private struct tl_calibs is initialized as expected.
 */
TEST(tl_initialize_cals, Test_Initialize_Cals)
{
   /** \precond
    * None.
    */
   
   /** \action
    * None.
    */

   /** \result
    * Check that all calibration values have been set to expected default values.
    */
   const float32_t exp_k_speed_threshold = 0.2F;
   const float32_t exp_k_ZRRateGate = 0.3F;
   const float32_t exp_k_host_length = 5.5F;
   const float32_t exp_k_max_trailer_length = 12.0F;
   const float32_t exp_k_max_trailer_width = 3.0F;
   const float32_t k_row_interval = 0.2F;
   const int32_t exp_k_window_timer_threshold = 1200;
   const int32_t exp_k_reset_time_threshold = 1800;

   const TL_Cals tl_cals_output = trailer_detector_TL.get_TL_cals();

   DOUBLES_EQUAL_TEXT(exp_k_speed_threshold, tl_cals_output.k_speed_threshold, test_pass_th, "k_speed_threshold has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_ZRRateGate, tl_cals_output.k_ZRRateGate, test_pass_th, "k_ZRRateGate has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_host_length, tl_cals_output.k_host_length, test_pass_th, "k_host_length has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_max_trailer_length, tl_cals_output.k_max_trailer_length, test_pass_th, "k_max_trailer_length has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(exp_k_max_trailer_width, tl_cals_output.k_max_trailer_width, test_pass_th, "k_max_trailer_width has not been set to the expected value");
   DOUBLES_EQUAL_TEXT(k_row_interval, tl_cals_output.k_row_interval, test_pass_th, "k_row_interval has not been set to the expected value");
   CHECK_EQUAL_TEXT(exp_k_window_timer_threshold, tl_cals_output.k_window_timer_threshold, "k_window_timer_threshold has not been set to the expected value");
   CHECK_EQUAL_TEXT(exp_k_reset_time_threshold, tl_cals_output.k_reset_time_threshold, "k_reset_time_threshold has not been set to the expected value");
}
/** @}*/
