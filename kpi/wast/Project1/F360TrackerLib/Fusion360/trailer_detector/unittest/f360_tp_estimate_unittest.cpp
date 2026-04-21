/** \file
 * This file contains unit tests for content of tp_estimate.cpp file
 */

#include "f360_trailer_detector_TP.h"
#include <CppUTest/TestHarness.h>


// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock Trailer_Detector_TP in order to be able to access private members to make unit testing easier
class TrailerDetectorTPmock : public Trailer_Detector_TP
{
   public:

   Trailer_Presence_State get_trailer_presence()
   {
      return trailer_presence;
   }

   void set_trailer_presence(Trailer_Presence_State trailer_presence_set_val)
   {
      trailer_presence = trailer_presence_set_val;
   }

   Trailer_Detector_Conf get_trailer_presence_conf()
   {
      return trailer_presence_conf;
   }

   void set_trailer_presence_conf(Trailer_Detector_Conf trailer_presence_conf_set_val)
   {
      trailer_presence_conf = trailer_presence_conf_set_val;
   }

   bool get_f_estimation_done()
   {
      return f_estimation_done;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   void set_window_timer(uint32_t window_timer_set_val)
   {
      window_timer = window_timer_set_val;
   }

   void set_relative_ratio_struct(TP_Relative_Ratio relative_ratio_struct_set_val)
   {
      relative_ratio_struct = relative_ratio_struct_set_val;
   }

   TP_Mean_RR get_mean_rr_struct()
   {
      return mean_rr_struct;
   }

   TP_Cals get_tp_cals()
   {
      return tp_calibs;
   }

   void set_mean_rr_struct(TP_Mean_RR mean_rr_struct_set_val)
   {
      mean_rr_struct = mean_rr_struct_set_val;
   }

   void estimate_mock()
   {
      Estimate();
   }
};

/** \defgroup  tp_estimate
 *  @{
 */

/** \brief
 * This test group checks that the function Trailer_Detector_TP:Estimate() works as intended
 */
TEST_GROUP(tp_estimate)
{	
   // Create an instance of the Trailer_Detector_TP class
   TrailerDetectorTPmock trailer_detector_TP;

      // Get a local copy of the private calibration struct
   const TP_Cals tp_calibs = trailer_detector_TP.get_tp_cals();
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th_small = 1e-9F;
   const float32_t test_pass_th_large = 1e-4F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with some default values
    */
   TEST_SETUP()
   {      
      trailer_detector_TP.Initialize();

      trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_NOT_DETECTED); // Setup trailer_presence to prevent Estimate() function to change the value of any memebr variables

      trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold); // Setup window_timer to enable Estimate() function to change the value of memeber variables

      TP_Relative_Ratio relative_ratio_struct_set_val; // Set up to enable the decision that there is a trailer with highest confidence
      relative_ratio_struct_set_val.dets_n_02 =  5;
      relative_ratio_struct_set_val.dets_n_03 =  5;
      relative_ratio_struct_set_val.dets_n_04 =  0;
      relative_ratio_struct_set_val.dets_n_05 =  1;
      trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);

      TP_Mean_RR mean_rr_struct_set_val = trailer_detector_TP.get_mean_rr_struct(); // To extract the value of mean_rr_struct.value_array after initialzation;
      mean_rr_struct_set_val.num_array[0] = 2; // Set such that range rate mean is below tp_calibs.k_rr_mean_threshold
      mean_rr_struct_set_val.num_array[1] = 3;
      mean_rr_struct_set_val.num_array[2] = 5;
      mean_rr_struct_set_val.num_array[3] = 0;
      mean_rr_struct_set_val.num_array[4] = 1;
      mean_rr_struct_set_val.num_array[29] = 10; // 10 makes the mean just below tp_calibs.k_rr_mean_threshold while 11 makes it just above
      trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);

      trailer_detector_TP.set_trailer_presence_conf(TRAILER_DETECTOR_CONF_UNKNOWN); // Setup to be 0 (which is a value that is not used/set inside of estimte() function, in this way we can easily detect when function has changed the value)

      trailer_detector_TP.set_f_estimation_done(false); // Set to false since Estimate() function can not change flag to false, only to tru so now we can easily detect if function has changed the value.
   }   
};

/** \purpose  
 * Test so that Trailer_Detector_TP::Estimate() does nothing when f_trailer_precence == NOT_DETECTED
 * \req
 * NA
 */
TEST(tp_estimate, Test_Do_Nothing_blocked_by_f_trailer_precence_0)
{
   /** \precond
    * Use data from test setup
    */
	
   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that none of trailer_precence, trailer_precence_conf or f_estimation_done flags has changed its value
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that Trailer_Detector_TP::Estimate() does nothing when trailer_precence == DETECTED
 * \req
 * NA
 */
TEST(tp_estimate, Test_Do_Nothing_blocked_by_f_trailer_precence_1)
{
   /** \precond
    * Use data from test setup except for change trailer_precence to DETECTED
    */
	 trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_DETECTED);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that none of trailer_precence, trailer_precence_conf or f_estimation_done flags has changed its value
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that Trailer_Detector_TP::Estimate() does nothing when window_timer < tp_cals.k_time_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Do_Nothing_blocked_by_window_timer_small)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precece to UNKNOWN
    * Change window_timer to be smaller than tp_cals.k_time_threshold
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that none of trailer_precence, trailer_precence_conf or f_estimation_done flags has changed its value
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_UNKNOWN, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}


/** \purpose  
 * Test so that trailer is detected with highest confidence when rang rate ratio in relative_ratio_struct > 0.9 and range rate mean in mean_rr_struct is < tp_cals.k_rr_mean_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Detected_Highest_Conf)
{
   /** \precond
    * Use data from test setup except for change trailer_precece to UNKNOWN
    */
	 trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is DETECTED, trailer_precence_conf is HIGH and f_estimation_done is true
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_TRUE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that trailer is detected with highest confidence when range rate ratio in 0.7 <= relative_ratio_struct < 0.9 and range rate mean in mean_rr_struct is < tp_cals.k_rr_mean_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Detected_Mid_Conf)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precece to UNKNOWN
    * Change relative_ratio_struct such that the range rate ratio is larger than 0.7 and smaller than 0.9
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Relative_Ratio relative_ratio_struct_set_val; // Set up to enable the decision that there is a trailer with highest confidence
   relative_ratio_struct_set_val.dets_n_02 =  4;
   relative_ratio_struct_set_val.dets_n_03 =  4;
   relative_ratio_struct_set_val.dets_n_04 =  2;
   relative_ratio_struct_set_val.dets_n_05 =  1;
   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is DETECTED, trailer_precence_conf is MEDIAN and f_estimation_done is true
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_MEDIAN, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_TRUE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that trailer is detected with highest confidence when range rate ratio in 0.5 < relative_ratio_struct < 0.7 and range rate mean in mean_rr_struct is < tp_cals.k_rr_mean_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Detected_Lowest_Conf)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precece to UNKNOWN
    * Change relative_ratio_struct such that the range rate ratio is larger than tp_cals.k_rr_mean_threshold and smaller than 0.7
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Relative_Ratio relative_ratio_struct_set_val; // Set up to enable the decision that there is a trailer with highest confidence
   relative_ratio_struct_set_val.dets_n_02 =  3;
   relative_ratio_struct_set_val.dets_n_03 =  3;
   relative_ratio_struct_set_val.dets_n_04 =  3;
   relative_ratio_struct_set_val.dets_n_05 =  2;
   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is DETECTED, trailer_precence_conf is LOW and f_estimation_done is true
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_LOW, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_TRUE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}


/** \purpose  
 * Test so that trailer is not detected with highest confidence when range rate ratio is < tp_cals.k_rr_mean_threshold despite range rate mean in mean_rr_struct is < tp_cals.k_rr_mean_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Not_Detected_Highest_Conf_blocked_by_rr_ratio)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precece to UNKNOWN
    * Change relative_ratio_struct such that the range rate ratio is larger than tp_cals.k_rr_mean_threshold and smaller than 0.7
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Relative_Ratio relative_ratio_struct_set_val; // Set up to enable the decision that there is a trailer with highest confidence
   relative_ratio_struct_set_val.dets_n_02 =  2;
   relative_ratio_struct_set_val.dets_n_03 =  2;
   relative_ratio_struct_set_val.dets_n_04 =  3;
   relative_ratio_struct_set_val.dets_n_05 =  4;
   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is NOT_DETECTED, trailer_precence_conf is HIGH and f_estimation_done is true
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_TRUE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that trailer is not detected with highest confidence when range rate mean in mean_rr_struct is > tp_cals.k_rr_mean_threshold despite range rate ratio in relative_ratio_struct > tp_cals.k_rr_mean_threshold
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Not_Detected_Highest_Conf_blocked_by_rr_mean)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precece to UNKNOWN
    * Change mean_rr_struct such that mean rr > tp_cals.k_rr_mean_threshold
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Mean_RR mean_rr_struct_set_val = trailer_detector_TP.get_mean_rr_struct(); // To extract the value of mean_rr_struct.value_array after initialzation;
   mean_rr_struct_set_val.num_array[0] = 2; // Set such that range rate mean is below tp_calibs.k_rr_mean_threshold
   mean_rr_struct_set_val.num_array[1] = 3;
   mean_rr_struct_set_val.num_array[2] = 5;
   mean_rr_struct_set_val.num_array[3] = 0;
   mean_rr_struct_set_val.num_array[4] = 1;
   mean_rr_struct_set_val.num_array[29] = 11; // 11 makes the mean just above tp_calibs.k_rr_mean_threshold (while 10 makes it just below)
   trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is NOT_DETECTED, trailer_precence_conf is HIGH and f_estimation_done is true
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_TRUE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}

/** \purpose  
 * Test so that trailer is not detected with highest confidence when number of detections in relative_ratio_struct is less than 11
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Not_Detected_Highest_Conf_blocked_few_dets_in_relative_ratio_struct)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precence to UNKNOWN
    * Change relative_ratio_struct such that number of detections is 10
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Relative_Ratio relative_ratio_struct_set_val; // Set up to enable the decision that there is a trailer with highest confidence
   relative_ratio_struct_set_val.dets_n_02 =  5; // Same as test setup
   relative_ratio_struct_set_val.dets_n_03 =  5; // Same as test setup
   relative_ratio_struct_set_val.dets_n_04 =  0; // Same as test setup
   relative_ratio_struct_set_val.dets_n_05 =  0; // One less than in test setup such that total number of detections in relative_ratio_struct is less than 11
   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);
   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is NOT_DETECTED, trailer_precence_conf is HIGH and f_estimation_done has not changed its value
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}


/** \purpose  
 * Test so that trailer is not detected with highest confidence when number of detections in mean_rr_struct is less than 11
 * \req
 * NA
 */
TEST(tp_estimate, Test_Trailer_Not_Detected_Highest_Conf_blocked_few_dets_in_mean_rr_struct)
{
   /** \precond
    * Use data from test setup except for:
    * Change trailer_precence to UNKNOWN
    * Change mean_rr_struct such that number of detections is 10
    */
	trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_UNKNOWN);

   TP_Mean_RR mean_rr_struct_set_val = trailer_detector_TP.get_mean_rr_struct(); // To extract the value of mean_rr_struct.value_array after initialzation;
   mean_rr_struct_set_val.num_array[0] = 2; // Same as test setup
   mean_rr_struct_set_val.num_array[1] = 3; // Same as test setup
   mean_rr_struct_set_val.num_array[2] = 5; // Same as test setup
   mean_rr_struct_set_val.num_array[3] = 0; // Same as test setup
   mean_rr_struct_set_val.num_array[4] = 0; // Changed compared to test setup
   mean_rr_struct_set_val.num_array[29] = 0; // Changed compared to test setup
   trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);

   /** \action
    * Call Trailer_Detector_TP::Estimate() function
    */
   trailer_detector_TP.estimate_mock();

   /** \result
    * Check so that trailer_precence is NOT_DETECTED, trailer_precence_conf is HIGH and f_estimation_done has not changed its value
    */	
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_precence has unexpected value");

   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_TP.get_trailer_presence_conf(), "trailer_precence_conf has unexpected value");

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done has unexpected value");
}
/** @}*/
