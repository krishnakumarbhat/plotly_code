/** \file
 * This file contains unit tests for content of tw_estimate.cpp file
 */

#include "f360_trailer_detector_TW.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

class TrailerDetectorTWmock : public Trailer_Detector_TW
{
   public:

   void set_window_timer(uint32_t window_timer_set_val)
   {
      window_timer = window_timer_set_val;
   }

   float32_t get_trailer_width()
   {
      return trailer_width;
   }

   void set_trailer_width(float32_t trailer_width_set_val)
   {
      trailer_width = trailer_width_set_val;
   }

   Trailer_Detector_Conf get_trailer_width_conf()
   {
      return trailer_width_conf;
   }

   void set_trailer_width_conf(Trailer_Detector_Conf trailer_width_conf_set_val)
   {
      trailer_width_conf = trailer_width_conf_set_val;
   }
   
   bool get_f_estimation_done()
   {
      return f_estimation_done;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   void set_detection_col(const uint32_t (&detection_col_set_val)[COL_NUMBER])
   {
      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         detection_col[i] = detection_col_set_val[i];
      }
   }

   void estimate_mock()
   {
      Trailer_Detector_TW::Estimate();
   }

};

/** \defgroup  tw_estimate
 *  @{
 */

/** \brief
 * Add brief description of test group, i.e. describe what functionality is tested.
 * When using multiple test groups, make sure to write a brief description for each test group.
 * The description should be unique and describe the specific scenario that is tested in that group.
 */
TEST_GROUP(tw_estimate)
{	
   TrailerDetectorTWmock trailer_detector_TW;

   const float32_t test_pass_th = 1e-6F;
      
   /** \setup
    * Describe what is done in test setup. Remove test setup function and this tag if it is not used.
    */
   TEST_SETUP()
   {
      trailer_detector_TW.Initialize();
      trailer_detector_TW.set_trailer_width(0.1F);

      trailer_detector_TW.set_window_timer(800U);      
   }
};

/** \purpose  
 * Test that when detection col array is has a flat distribution (5 in all bins), the trailer width correctly estimated to the maximum width.
 */
TEST(tw_estimate, FunctionToTest_Descriptive_Tag_2)
{
   /** \precond
    * Set up a detection column with 5 detections in each bin
    * Set expected width to 3m
    */
   const uint32_t detection_col_set_val[COL_NUMBER] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = 3.0F;
   Trailer_Detector_Conf exp_trailer_detector_conf = TRAILER_DETECTOR_CONF_HIGH;
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_TRUE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** \purpose  
 * Test that when detection col array is has a flat distribution (5 in all bins), the trailer width correctly estimated to the maximum width.
 */
TEST(tw_estimate, FunctionToTest_Descriptive_Tag_3)
{
   /** \precond
    * Set up a detection column with 5 detections in each bin
    * Set expected width to 3m
    */
   const uint32_t detection_col_set_val[COL_NUMBER] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 2, 5};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = 3.0F;
   Trailer_Detector_Conf exp_trailer_detector_conf = TRAILER_DETECTOR_CONF_HIGH;
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_TRUE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}
/** \purpose  
 * Test that when detection col array has a normal distribution, the width is correctly estimated.
 */
TEST(tw_estimate, Estimate_Det_Col_Flat)
{
   /** \precond
    * Set up a detection column with a normal distribution.
    * Set expected width to 1m
    */
   const uint32_t detection_col_set_val[COL_NUMBER] = {2, 5, 3, 22, 27, 57, 72, 119, 211, 260, 379, 411, 449, 498, 504, 465, 401, 325, 280, 179, 127, 88, 62, 27, 18, 4, 2, 2, 0, 1};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = 1.0F;
   Trailer_Detector_Conf exp_trailer_detector_conf = TRAILER_DETECTOR_CONF_HIGH;
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_TRUE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** \purpose  
 * Test that when detection col array is sampled from a uniform random distribution, the width is correctly estimated.
 */
TEST(tw_estimate, Estimate_Det_Col_Uniform_Random)
{
   /** \precond
    * A detection col is set up with random numbers in each bin
    * Set expected width to 2m.
    */
   const uint32_t detection_col_set_val[COL_NUMBER] = {2687, 45, 3689, 3766, 555, 904, 2443, 3856, 2744, 267, 3625, 3863, 3505, 2020, 3380, 2486, 1227, 2718, 2669, 1696, 3116, 452, 88, 3695, 2362, 2682, 347, 1506, 3776, 2418};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = 2.0F;
   Trailer_Detector_Conf exp_trailer_detector_conf = TRAILER_DETECTOR_CONF_HIGH;
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_TRUE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** \purpose  
 * Test that trailer width is not updated when it's above 0.5m.
 */
TEST(tw_estimate, Estimate_Trailer_Width_Above_Half_Meter)
{
   /** \precond
    * A detection col is set up with random numbers in each bin
    * Trailer width is set to 1m
    * Set expected width to same value as before estimate().
    */
   trailer_detector_TW.set_trailer_width(1.0F);
   const uint32_t detection_col_set_val[COL_NUMBER] = {2687, 45, 3689, 3766, 555, 904, 2443, 3856, 2744, 267, 3625, 3863, 3505, 2020, 3380, 2486, 1227, 2718, 2669, 1696, 3116, 452, 88, 3695, 2362, 2682, 347, 1506, 3776, 2418};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = trailer_detector_TW.get_trailer_width();
   Trailer_Detector_Conf exp_trailer_detector_conf = trailer_detector_TW.get_trailer_width_conf();
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** \purpose  
 * Test that trailer width is not updated when window_timer is not 800.
 */
TEST(tw_estimate, Estimate_Window_Timer_Not_800)
{
   /** \precond
    * A detection col is set up with random numbers in each bin
    * Trailer width is set to 0.2m
    * window_timer is set to 900.
    * Set expected width to same value as before estimate().
    */
   trailer_detector_TW.set_trailer_width(0.2F);
   trailer_detector_TW.set_window_timer(900U);
   const uint32_t detection_col_set_val[COL_NUMBER] = {2687, 45, 3689, 3766, 555, 904, 2443, 3856, 2744, 267, 3625, 3863, 3505, 2020, 3380, 2486, 1227, 2718, 2669, 1696, 3116, 452, 88, 3695, 2362, 2682, 347, 1506, 3776, 2418};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = trailer_detector_TW.get_trailer_width();
   Trailer_Detector_Conf exp_trailer_detector_conf = trailer_detector_TW.get_trailer_width_conf();
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** \purpose  
 * Test that trailer width is not updated when window_timer is not 800 and width is above 0.5m.
 */
TEST(tw_estimate, Estimate_Window_Timer_Not_800_Width_Above_Half_Meter)
{
   /** \precond
    * A detection col is set up with random numbers in each bin
    * Trailer width is set to 1.0m
    * window_timer is set to 900.
    * Set expected width to same value as before estimate().
    */
   trailer_detector_TW.set_trailer_width(1.0F);
   trailer_detector_TW.set_window_timer(900U);
   const uint32_t detection_col_set_val[COL_NUMBER] = {2687, 45, 3689, 3766, 555, 904, 2443, 3856, 2744, 267, 3625, 3863, 3505, 2020, 3380, 2486, 1227, 2718, 2669, 1696, 3116, 452, 88, 3695, 2362, 2682, 347, 1506, 3776, 2418};
   trailer_detector_TW.set_detection_col(detection_col_set_val);

   float32_t exp_trailer_width = trailer_detector_TW.get_trailer_width();
   Trailer_Detector_Conf exp_trailer_detector_conf = trailer_detector_TW.get_trailer_width_conf();
	
   /** \action
    * Call estimate() through the mock object.
    */
   trailer_detector_TW.estimate_mock();

   /** \result
    * Check that the expected width is estimated and confidence level is correct.
    */
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is incorrect.")   
   CHECK_EQUAL_TEXT(exp_trailer_detector_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th, "trailer_width is incorrect.")
}

/** @}*/
