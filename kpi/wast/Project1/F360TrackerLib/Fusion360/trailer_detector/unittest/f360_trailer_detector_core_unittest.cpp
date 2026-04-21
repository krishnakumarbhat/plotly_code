/** \file
 * This file contains unit tests for content of trailer_detector_core.cpp file
 */

#include "f360_trailer_detector_core.h"
#include <CppUTest/TestHarness.h>
#include <cstring>


// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

class TrailerDetectorCoremock : public Trailer_Detector_Core
{
   public:

   void Post_Processing_Of_Output_mock(const F360_Host_T& vehicle_data)
   {
      Post_Processing_Of_Output(vehicle_data);
   }

   void set_trailer_detector_output(const Trailer_Detector_Flt_Fus_Output& trailer_detector_output_set_val)
   {
       trailer_detector_output = trailer_detector_output_set_val;
   }

   void set_output(const Trailer_Detector_Outputs& output_set_val)
   {
       output = output_set_val;
   }
};


/** \defgroup  trailer_detector_core_Post_Processing_Of_Output
 *  @{
 */

/** \brief
 * This group tests that the function Trailer_Detector_Core::Post_Processing_Of_Output() function as intended
 */
TEST_GROUP(trailer_detector_core_Post_Processing_Of_Output)
{	
   // Create an instance of the Trailer_Detector_Core class
   TrailerDetectorCoremock trailer_detector;

   // Setup the different types of output structures that the trailer detector are using
   Trailer_Detector_Flt_Fus_Output trailer_detector_output_set_val;
   Trailer_Detector_Outputs output_set_val;

   // Create structure for host data
   F360_Host_T vehicle_data;

   // Set up threshold for floating number comparision
   float32_t test_pass_th = 1e-9F;

   
   /** \setup
    * Initialize the instance of the Trailer_Detector_Core class with default values
    * Initialize the outout structures with default test data such that all 4 different types of
    * trailer detector estimators (presence, length, width and angle) are iutputting valid estimates
    * that indicate a trailer is present with higest confidence
    */
   TEST_SETUP()
   {
      // Initialize the trailer detector
      trailer_detector.Initialize();

      // Setup the stucture that containe the output from the four estimators
      output_set_val.tp_outputs.trailer_presence = TRAILER_PRESENCE_STATE_DETECTED;
      output_set_val.tp_outputs.confidence = TRAILER_DETECTOR_CONF_HIGH;
      output_set_val.tp_outputs.f_estimation_done = true;
      output_set_val.tp_outputs.stationary_timer = 5U; // Set to a non-default value

      output_set_val.tl_outputs.trailer_length = 4.5F; // Set to a non-default value
      output_set_val.tl_outputs.axle_trailer_length = 2.3F; // Set to a non-default value
      output_set_val.tl_outputs.confidence = TRAILER_DETECTOR_CONF_HIGH;
      output_set_val.tl_outputs.f_estimation_done = true;

      output_set_val.tw_outputs.trailer_width = 2.8F; // Set to a non-default value
      output_set_val.tw_outputs.confidence = TRAILER_DETECTOR_CONF_HIGH;
      output_set_val.tw_outputs.f_estimation_done = true;

      output_set_val.ta_outputs.trailer_angle_rad = -3.2F; // Set to a non-default value
      output_set_val.ta_outputs.trailer_angle_rate_rad = 3.4F; // Set to a non-default value
      output_set_val.ta_outputs.confidence = TRAILER_DETECTOR_CONF_MEDIAN;

      output_set_val.radar_detection_timer = 33U; // Set to a non-default value
      output_set_val.trailer_detection_status = TRAILER_DETECTOR_STATUS_UNKNOWN; // Set to value that we don't expect it to have after call to Post_Processing_Of_Output() (Post_Processing_Of_Output() can set it to either FAULTY or NORMAL but not UNKNOWN)

      trailer_detector.set_output(output_set_val);

      // Setup the stucture that contains the output that is accessible from outside off the trailer detector class
      std::memset(&trailer_detector_output_set_val, 0, sizeof(trailer_detector_output_set_val));
      trailer_detector.set_trailer_detector_output(trailer_detector_output_set_val);

      //setup electronic trailer presence signal
      vehicle_data.f_trailer_presence_hardware = false;
   }
};

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when all four different estimator types have succeded in generating trailer estimates.
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_All_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used
    */
	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(0U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_NOT_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.confidence, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(output_set_val.tl_outputs.trailer_length, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.tw_outputs.trailer_width, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tw_outputs.confidence, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rad, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rate_rad, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when trailer presence estimatot detects NOT_PRESENT and other estimators are all giving valid outputs
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_TP_Say_NOT_DETECTED_Other_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer precence to NOT_DETECTED
    */
   output_set_val.tp_outputs.trailer_presence = TRAILER_PRESENCE_STATE_NOT_DETECTED;
   trailer_detector.set_output(output_set_val);

	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(0U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_NOT_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when trailer presence estimator detects UNKNOWN and other estimators are all giving valid outputs.
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_TP_Say_UNKNOWN_Other_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer precence to UNKNOWN
    */
   output_set_val.tp_outputs.trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   trailer_detector.set_output(output_set_val);

	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(output_set_val.radar_detection_timer + 1U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_UNKNOWN, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(0.0F, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when trailer presence estimator detects UNKNOWN but electronic signal from the host vehicle also shows the trailer is attached.
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_electronically_trailer_detected)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer precence to UNKNOWN
    *    Set electronic trailer presence signal to TRUE 
    */
   output_set_val.tp_outputs.trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   vehicle_data.f_trailer_presence_hardware = true;
   trailer_detector.set_output(output_set_val);

   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(0U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_NOT_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(output_set_val.tl_outputs.trailer_length, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.tw_outputs.trailer_width, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tw_outputs.confidence, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rad, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rate_rad, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when all different estimator types have succeded in generating trailer estimates except for length estimator
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_Length_Estimate_Not_Valid_Other_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer length estimator output to invalid
    */
   output_set_val.tl_outputs.f_estimation_done = false;
   trailer_detector.set_output(output_set_val);
	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(output_set_val.radar_detection_timer + 1U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.confidence, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(output_set_val.tl_outputs.trailer_length, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.tw_outputs.trailer_width, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tw_outputs.confidence, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rad, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rate_rad, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when all different estimator types have succeded in generating trailer estimates except for width estimator
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_Width_Estimate_Not_Valid_Other_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer wi estimator output to invalid
    */
   output_set_val.tw_outputs.f_estimation_done = false;
   trailer_detector.set_output(output_set_val);
	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(output_set_val.radar_detection_timer + 1U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.confidence, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(output_set_val.tl_outputs.trailer_length, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.tw_outputs.trailer_width, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tw_outputs.confidence, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rad, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rate_rad, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}

/** \purpose  
 * The purpose with this test is to check that Trailer_Detector_Core::Post_Processing_Of_Output() works as intended 
 * when all different estimator types have succeded in generating trailer estimates except for width and length estimators
 * \req
 * NA.
 */
TEST(trailer_detector_core_Post_Processing_Of_Output, Test_Width_And_Length_Estimates_Not_Valid_Other_Estimators_Has_Valid_Estimates)
{
   /** \precond
    * Default data from test setup is used except for:
    *    Change trailer length and width estimator outputs to invalid
    */
   output_set_val.tw_outputs.f_estimation_done = false;
   output_set_val.tl_outputs.f_estimation_done = false;
   trailer_detector.set_output(output_set_val);
	
   /** \action
    * Call Post_Processing_Of_Output()
    * Call Trailer_Detector_Core::Get_Trailer_Detector_Output() to extract the output from the trailer detector
    */
   trailer_detector.Post_Processing_Of_Output_mock(vehicle_data);
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector.Get_Trailer_Detector_Output();

   /** \result
    * Test so that the trailer detector output is as expected
    */
   CHECK_EQUAL_TEXT(output_set_val.radar_detection_timer + 1U, trailer_detector_output.radar_detection_timer, "radar_detection_timer has unexpected value");
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_STATUS_RUNNING, trailer_detector_output.trailer_detection_status, "trailer_detection_status has unexpected value");

   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.trailer_presence, trailer_detector_output.trailer_presence, "trailer_presence has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.confidence, trailer_detector_output.trailer_presence_conf, "trailer_presence_conf has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tp_outputs.stationary_timer, trailer_detector_output.stationary_timer, "stationary_timer has unexpected value");
   
   DOUBLES_EQUAL_TEXT(output_set_val.tl_outputs.trailer_length, trailer_detector_output.trailer_length, test_pass_th, "trailer_length has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.tw_outputs.trailer_width, trailer_detector_output.trailer_width, test_pass_th, "trailer_width has unexpected value");
   CHECK_EQUAL_TEXT(output_set_val.tw_outputs.confidence, trailer_detector_output.trailer_width_conf, "trailer_width_conf has unexpected value");

   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rad, trailer_detector_output.trailer_angle, test_pass_th, "trailer_angle has unexpected value");
   DOUBLES_EQUAL_TEXT(output_set_val.ta_outputs.trailer_angle_rate_rad, trailer_detector_output.trailer_angle_rate, test_pass_th, "trailer_angle_rate has unexpected value");
}
/** @}*/