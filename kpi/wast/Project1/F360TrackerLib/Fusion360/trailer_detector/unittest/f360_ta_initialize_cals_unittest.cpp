/** \file
 * This file contains unit tests for content of ta_initialize_cals.cpp file
 */

#include "f360_trailer_detector_TA.h"
#include <CppUTest/TestHarness.h>

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock TrailerDetectorTA in order to be able to access private members to make unit testing easier
class Trailer_Detector_TAmock : public Trailer_Detector_TA
{
   public:

   TA_Cals get_ta_calibs()
   {
      return ta_calibs;
   }

};

/** \defgroup  ta_initialize_cals
 *  @{
 */

/** \brief
 * Test group to verify the private struct ta_calibs is initialized to the correct values
 */
TEST_GROUP(ta_initialize_cals)
{	
   // Create an instance of the TrailerDetectorTA class
   Trailer_Detector_TAmock trailer_detector_TA;

   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = F360_EPSILON;
   
   /** \setup
    * None.
    */

};


/** \purpose  
 *  Verify the private struct ta_calibs is initialized to the correct values
 */
TEST(ta_initialize_cals, Test_Calib_Correctly_Initialized)
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
   const TA_Cals ta_calibs_output = trailer_detector_TA.get_ta_calibs();

   const float32_t exp_true_HV_length = 5.8F;
   DOUBLES_EQUAL_TEXT(exp_true_HV_length, ta_calibs_output.true_HV_length, test_pass_th, "true_HV_length has not been set to the expected value");

   const float32_t exp_speed_threshold = 0.2F;
   DOUBLES_EQUAL_TEXT(exp_speed_threshold, ta_calibs_output.speed_threshold, test_pass_th, "speed_threshold has not been set to the expected value");
   
   const int16_t exp_HV_cnt_threshold = 20;
   CHECK_EQUAL_TEXT(exp_HV_cnt_threshold, ta_calibs_output.HV_cnt_threshold, "HV_cnt_threshold has not been set to the expected value");

   const float32_t exp_delta_t = 0.05F;
   DOUBLES_EQUAL_TEXT(exp_delta_t, ta_calibs_output.delta_t, test_pass_th, "delta_t has not been set to the expected value");   

   const float32_t exp_b2_default = 4.0F;
   DOUBLES_EQUAL_TEXT(exp_b2_default, ta_calibs_output.b2_default, test_pass_th, "b2_default has not been set to the expected value");
}   
/** @}*/
