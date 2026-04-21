/** \file
 * This file contains unit tests for content of tp_initialize_cals.cpp file
 */

#include "f360_trailer_detector_TP.h"
#include <CppUTest/TestHarness.h>

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock Trailer_Detector_TP in order to be able to access private members to make unit testing easier
class Trailer_Detector_TPmock : public Trailer_Detector_TP
{
   public:

   TP_Cals get_tp_cals()
   {
      return tp_calibs;
   }
};

/** \defgroup  tp_initialize_cals
 *  @{
 */

/** \brief
 * Test group to verify the private struct tp_calibs is initialized to the correct values
 */
TEST_GROUP(tp_initialize_cals)
{	
   // Create an instance of the Trailer_Detector_TP class
   Trailer_Detector_TPmock trailer_detector_TP;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = F360_EPSILON;

   /** \setup
    * None.
    */
};

/** \purpose  
 * Verify the private struct tp_calibs is initialized to the correct values
 */
TEST(tp_initialize_cals, Test_Calibs_Initialized_Correctly)
{
   /** \precond
    * None.
    */
	
   /** \action
    * None.
    */

   /** \result
    * Check so that tp_cals has been reset to default values
    */
   const TP_Cals tp_cals_output = trailer_detector_TP.get_tp_cals();

   const float32_t exp_k_speed_threshold = 0.2F;
   DOUBLES_EQUAL_TEXT(exp_k_speed_threshold, tp_cals_output.k_speed_threshold, test_pass_th, "k_speed_threshold has not been set to the expected value");

   const float32_t exp_k_speed_threshold_clear = 0.833F;
   DOUBLES_EQUAL_TEXT(exp_k_speed_threshold_clear, tp_cals_output.k_speed_threshold_clear, test_pass_th, "k_speed_threshold_clear has not been set to the expected value");

   const int32_t exp_k_reset_cnt_threshold = 1800;
   CHECK_EQUAL_TEXT(exp_k_reset_cnt_threshold, tp_cals_output.k_reset_cnt_threshold, "k_reset_cnt_threshold has not been set to the expected value");

   const int32_t exp_k_time_threshold = 200;
   CHECK_EQUAL_TEXT(exp_k_time_threshold, tp_cals_output.k_time_threshold, "k_time_threshold has not been set to the expected value");

   const float32_t exp_k_long_max = -5.7F;
   DOUBLES_EQUAL_TEXT(exp_k_long_max, tp_cals_output.k_long_max, test_pass_th, "k_long_max has not been set to the expected value");   

   const float32_t exp_k_long_min = -9.7F;
   DOUBLES_EQUAL_TEXT(exp_k_long_min, tp_cals_output.k_long_min, test_pass_th, "k_long_min has not been set to the expected value");   

   const float32_t exp_k_lat_abs = 1.75F;
   DOUBLES_EQUAL_TEXT(exp_k_lat_abs, tp_cals_output.k_lat_abs, test_pass_th, "k_lat_abs has not been set to the expected value");   

   const float32_t exp_k_rr_start = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_k_rr_start, tp_cals_output.k_rr_start, test_pass_th, "k_rr_start has not been set to the expected value");   

   const float32_t exp_k_rr_end = 10.0F;
   DOUBLES_EQUAL_TEXT(exp_k_rr_end, tp_cals_output.k_rr_end, test_pass_th, "k_rr_end has not been set to the expected value");   

   const float32_t exp_k_rr_ratio_threshold = 0.5F;
   DOUBLES_EQUAL_TEXT(exp_k_rr_ratio_threshold, tp_cals_output.k_rr_ratio_threshold, test_pass_th, "k_rr_ratio_threshold has not been set to the expected value");   

   CHECK_TRUE_TEXT((tp_cals_output.k_rr_end - tp_cals_output.k_rr_start) > F360_EPSILON , "k_rr_end not bigger than k_rr_start"); 
}
/** @}*/
