/** \file
 * This file contains unit tests for content of tl_process_input.cpp file
 */

#include "f360_trailer_detector_TL.h"
#include <CppUTest/TestHarness.h>
#include "rspp_detection_list.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock Trailer_Detector_TL in order to be able to access private members to make unit testing easier
class TrailerDetectorTLmock : public Trailer_Detector_TL
{
   public:
   void call_process_input(const F360_Host_T&vehicle_data,
         const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
         const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Trailer_Detector_TL::Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);
   }

   void call_increase_detection_row(int32_t (&detection_row_new)[ROW_NUMBER], int32_t (&detection_row)[ROW_NUMBER])
   {
      Trailer_Detector_TL::Increase_Detection_Row(detection_row_new, detection_row);
   }

   uint32_t get_reset_timer()
   {
      return reset_timer;
   }

   void set_reset_timer(uint32_t set_reset_timer_val)
   {
      reset_timer = set_reset_timer_val;
   }

   uint32_t get_window_timer()
   {
      return window_timer;
   }

   void set_window_timer(uint32_t windor_timer_set_val)
   {
      window_timer = windor_timer_set_val;
   }

   bool get_f_estimation_done()
   {
      return f_estimation_done;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   TL_Trailer_Detection_Row get_trailer_detection_row_struct()
   {
      return trailer_detection_row_struct;
   }

   void set_trailer_detection_row_struct(TL_Trailer_Detection_Row trailer_detection_row_struct_set_val)
   {
      trailer_detection_row_struct = trailer_detection_row_struct_set_val;
   }

   float32_t trailer_length;
   float32_t axle_trailer_length;
};

/** \defgroup  f360_TL_process_input
 *  @{
 */

/** \brief
 * Test group to set up a mocked Trailer_Detector_TL object and fill its properties to then test the constructor,
 * initialization and clearing.
 */
TEST_GROUP(f360_TL_process_input)
{	
   // Initialize an object of the mocked trailer detector TL
   TrailerDetectorTLmock trailer_detector_TL;

   // Define a threshold for floating number equality comparision
   const float32_t test_pass_th_small = 1e-9F;
   const float32_t test_pass_th_large = 1e-4F;
   /** \setup
    * Describe what is done in test setup. Remove test setup function and this tag if it is not used.
    */
   TEST_SETUP()
   {
      trailer_detector_TL.Initialize();
      trailer_detector_TL.set_reset_timer(1U);
      trailer_detector_TL.set_window_timer(2U);
      trailer_detector_TL.set_f_estimation_done(true);

      TL_Trailer_Detection_Row trailer_detection_row_struct_set_val;
      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         trailer_detection_row_struct_set_val.detection_row[i] = i;
      }
      trailer_detector_TL.set_trailer_detection_row_struct(trailer_detection_row_struct_set_val);
   }

   /** \teardown
    * Clear the mocked object
    */
   TEST_TEARDOWN()
   {
      trailer_detector_TL.Clear();
   }
};

/** \purpose  
 * Test that the Trailer_Detector_TL constructor works as intended.
 */
TEST(f360_TL_process_input, Test_Constructor)
{	
   /** \action
    * Create a new TrailerDetectorT object 
    */
   TrailerDetectorTLmock new_trailer_detector_TL;

   /** \result
    * Check that the constructor has initialized class members as expected.
    */
   const float32_t exp_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_length, new_trailer_detector_TL.trailer_length, test_pass_th_small, "trailer_length is not initialized correctly");

   const float32_t exp_axle_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_axle_trailer_length, new_trailer_detector_TL.axle_trailer_length, test_pass_th_small, "axle_trailer_length is not initialized correctly");

   const uint32_t exp_reset_timer = 0U;
   CHECK_EQUAL_TEXT(exp_reset_timer, new_trailer_detector_TL.get_reset_timer(), "reset_timer is not initialized correctly");

   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, new_trailer_detector_TL.get_window_timer(), "window_timer is not initialized correctly");

   const bool exp_f_estimation_done = false;
   CHECK_TRUE_TEXT(exp_f_estimation_done == new_trailer_detector_TL.get_f_estimation_done(), "trailer_length is not initialized correctly");

   const int32_t exp_detection_row = 0;

   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      CHECK_EQUAL_TEXT(exp_detection_row, new_trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TL::Initialize works as intended.
 */
TEST(f360_TL_process_input, Test_Initialize)
{	
   /** \precond
    * An instance of Trailer_Detector_TL has been set up in the test group with non-default values
    */

   /** \action
    * Call Trailer_Detector_TL::Initialize
    */
   trailer_detector_TL.Initialize();
   TL_Output output;
   trailer_detector_TL.Get_Output(output);

   /** \result
    * Check that all the signals are set to their predetermined initial values.
    */
   const float32_t exp_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_length, output.trailer_length, test_pass_th_small, "trailer_length is not initialized correctly");

   const float32_t exp_axle_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_axle_trailer_length, output.axle_trailer_length, test_pass_th_small, "axle_trailer_length is not initialized correctly");

   const uint32_t exp_reset_timer = 0U;
   CHECK_EQUAL_TEXT(exp_reset_timer, trailer_detector_TL.get_reset_timer(), "reset_timer is not initialized correctly");

   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TL.get_window_timer(), "window_timer is not initialized correctly");

   const bool exp_f_estimation_done = false;
   CHECK_TRUE_TEXT(exp_f_estimation_done == trailer_detector_TL.get_f_estimation_done(), "trailer_length is not initialized correctly");

   const int32_t exp_detection_row = 0;

   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      CHECK_EQUAL_TEXT(exp_detection_row, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TL::Clear works as intended.
 */
TEST(f360_TL_process_input, Test_Clear)
{	
   /** \precond
    * An instance of Trailer_Detector_TL has been set up in the test group with non-default values
    */

   /** \action
    * Call Trailer_Detector_TL::Initialize
    */
   trailer_detector_TL.Clear();
   TL_Output output;
   trailer_detector_TL.Get_Output(output);

   /** \result
    * Check that all the signals are set to their predetermined initial values.
    */
   const float32_t exp_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_length, output.trailer_length, test_pass_th_small, "trailer_length is not initialized correctly");

   const float32_t exp_axle_trailer_length = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_axle_trailer_length, output.axle_trailer_length, test_pass_th_small, "axle_trailer_length is not initialized correctly");

   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TL.get_window_timer(), "window_timer is not initialized correctly");

   const bool exp_f_estimation_done = false;
   CHECK_TRUE_TEXT(exp_f_estimation_done == trailer_detector_TL.get_f_estimation_done(), "trailer_length is not initialized correctly");

   const int32_t exp_detection_row = 0;
   
   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      CHECK_EQUAL_TEXT(exp_detection_row, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is not initialized correctly");
   }
}

/** \defgroup  f360_process_input_functionality
 *  @{
 */

/** \brief
 * Test group to set up an instance of the mocked Trailer_Detector_TL and test its process input functionality.
 */
TEST_GROUP(f360_process_input_functionality)
{	
      // Initialize an object of the mocked trailer detector TL
   TrailerDetectorTLmock trailer_detector_TL;

   F360_Host_T vehicle_data;
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list;
   F360_Detection_Props_T all_detections[MAX_NUMBER_OF_DETECTIONS];
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];

   int32_t detection_row_new[ROW_NUMBER];
   int32_t detection_row[ROW_NUMBER];
   
   /** \setup
    * Describe what is done in test setup. Remove test setup function and this tag if it is not used.
    */
   TEST_SETUP()
   {
      trailer_detector_TL.Initialize();

      trailer_detector_TL.set_window_timer(1U);
      trailer_detector_TL.set_reset_timer(1800U);

      vehicle_data.speed = 0.1F;

      for (uint8_t i = 0U; i < ROW_NUMBER; i++)
      {
         detection_row_new[i] = i;
         detection_row[i] = - i;
      }
   }

   /** \teardown
    * Describe what is done in test teardown. Remove test teardown function and this tag if it is not used.
    */
   TEST_TEARDOWN()
   {
      // Perform any necessary clean up. E.g. mock().Clear().
      trailer_detector_TL.Clear();
   }
};

/** \purpose  
 * Test that Increase_Detection_Row correctly assigns new values to detection_row_new. The test is set up such that all its elements should be 0.
 */
TEST(f360_process_input_functionality, Test_increase_detection_row)
{
   /** \precond
    * Two arrays, detection_row_new and detection_row have been initialized in TEST_GROUP and filled with i and -i respectively 
    * in the i'th position.
    * Expected outcome is that detection_row_new is filled with zeros after function call.
    */
   int32_t exp_detection_row_new_val = 0;
	
   /** \action
    * Call Increase_Detection_Row()
    */
   trailer_detector_TL.call_increase_detection_row(detection_row_new, detection_row);

   /** \result
    * Check that all elements of detection_row_new are 0.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      CHECK_EQUAL_TEXT(exp_detection_row_new_val, detection_row_new[i], "detection_row_new is not correctly filled.");
   }	
}

/** \purpose  
 * Test that when the object has very low speed and Trailer_Detector_TL has a reset timer above 1800, Trailer_Detector_TL is cleared.
 */
TEST(f360_process_input_functionality, process_input_low_speed_reset_timer_overshoot)
{
   /** \precond
    * Host speed is set to 0.1 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 1
    * Expectation is that Trailer_Detector_TL shall be cleared, i.e.
    *    - window_timer should be set to 0
    *    - reset_timer should not be cleared
    */
   uint32_t exp_window_timer = 0U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that some Trailer_Detector_TL signals are cleared.
    */
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TL.get_window_timer(), "trailer_detector_TL was not cleared correctly")
}

/** \purpose  
 * Test that when the object has very low speed and Trailer_Detector_TL has a reset timer below 1800, Trailer_Detector_TL is not cleared.
 */
TEST(f360_process_input_functionality, process_input_low_speed_reset_timer_no_overshoot)
{
   /** \precond
    * Host speed is set to 0.1 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 100
    *    - window_timer = 1
    * Expectation is that Trailer_Detector_TL shall not be cleared, i.e.
    *    - window_timer should be kept at 1
    *    - reset_timer should not be cleared but incremented by 1
    */
   trailer_detector_TL.set_reset_timer(100U);
   uint32_t exp_reset_timer = trailer_detector_TL.get_reset_timer() + 1U;
   uint32_t exp_window_timer = trailer_detector_TL.get_window_timer();
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that some Trailer_Detector_TL signals are cleared.
    */
   CHECK_EQUAL_TEXT(exp_reset_timer, trailer_detector_TL.get_reset_timer(), "trailer_detector_TL was not cleared correctly")
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TL.get_window_timer(), "trailer_detector_TL was not cleared correctly")
}

/** \brief
 * Test group to set up an instance of the mocked Trailer_Detector_TL and test its process input main functionality.
 */
TEST_GROUP(f360_process_input_main_functionality)
{	
      // Initialize an object of the mocked trailer detector TL
   TrailerDetectorTLmock trailer_detector_TL;

   F360_Host_T vehicle_data;
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list;
   F360_Detection_Props_T all_detections[MAX_NUMBER_OF_DETECTIONS];
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];

   int32_t detection_row_new[ROW_NUMBER];
   int32_t detection_row[ROW_NUMBER];
   
   /** \setup
    * Describe what is done in test setup. Remove test setup function and this tag if it is not used.
    */
   TEST_SETUP()
   {
      //trailer_detector_TL = TrailerDetectorTLmock();
      trailer_detector_TL.Initialize();

      trailer_detector_TL.set_window_timer(600U);
      trailer_detector_TL.set_reset_timer(1800U);

      vehicle_data.speed = 10.0F;
      
      // Set up history in the detection_row_struct with 1 previously binned detection
      TL_Trailer_Detection_Row trailer_det_row_input;
      for (uint8_t i = 0U; i < ROW_NUMBER; i++)
      {
         trailer_det_row_input.detection_row[i] = 0;
      }

      trailer_detector_TL.set_trailer_detection_row_struct(trailer_det_row_input);

      // Set up detections
      raw_detect_list.number_of_valid_detections = 3U;

      uint32_t det_idx_1 = 0U;
      raw_detect_list.detections[det_idx_1].raw.range_rate = 0.1F;
      raw_detect_list.detections[det_idx_1].raw.sensor_id = 1U;
      all_detections[det_idx_1].vcs_position.x = -10.0F;
      all_detections[det_idx_1].vcs_position.y = 1.0F;
      all_detections[det_idx_1].f_double_bounce = false;
      all_detections[det_idx_1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      all_detections[det_idx_1].f_water_spray = false;
      sensors[raw_detect_list.detections[det_idx_1].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;

      uint32_t det_idx_2 = 1U;
      raw_detect_list.detections[det_idx_2].raw.range_rate = 0.1F;
      raw_detect_list.detections[det_idx_2].raw.sensor_id = 2U;
      all_detections[det_idx_2].vcs_position.x = -15.0F;
      all_detections[det_idx_2].vcs_position.y = 1.0F;
      all_detections[det_idx_2].f_double_bounce = false;
      all_detections[det_idx_2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      all_detections[det_idx_2].f_water_spray = false;
      sensors[raw_detect_list.detections[det_idx_2].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;

      uint32_t det_idx_3 = 2U;
      raw_detect_list.detections[det_idx_3].raw.range_rate = 0.1F;
      raw_detect_list.detections[det_idx_3].raw.sensor_id = 3U;
      all_detections[det_idx_3].vcs_position.x = -5.5F;
      all_detections[det_idx_3].vcs_position.y = 1.0F;
      all_detections[det_idx_3].f_double_bounce = false;
      all_detections[det_idx_3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      all_detections[det_idx_3].f_water_spray = false;
      sensors[raw_detect_list.detections[det_idx_3].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR;
   }

   /** \teardown
    * Describe what is done in test teardown. Remove test teardown function and this tag if it is not used.
    */
   TEST_TEARDOWN()
   {
      // Perform any necessary clean up. E.g. mock().Clear().
      trailer_detector_TL.Clear();
   }
};

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations.
 */
TEST(f360_process_input_main_functionality, process_input_3_valid_dets)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 600
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up
    * Expectation is that these three detections are stored in the detection row struct arrays.
    */
   uint8_t exp_row_idx_1 = 22U;
   uint8_t exp_row_idx_2 = 47U;
   uint8_t exp_row_idx_3 = 0U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_2)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_3)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations when window timer is only in  interval.
 */
TEST(f360_process_input_main_functionality, process_input_3_valid_dets_only_shorter_window)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 100
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up
    * Expectation is that these three detections are stored in the detection row struct arrays properly
    *    - Only the historic one is stored in the array
    *    - The new detections are only stored in the  array
    */
   trailer_detector_TL.set_window_timer(100U);
   uint8_t exp_row_idx_1 = 22U;
   uint8_t exp_row_idx_2 = 47U;
   uint8_t exp_row_idx_3 = 0U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_2)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_3)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations when window timer is only in interval.
 */
TEST(f360_process_input_main_functionality, process_input_3_valid_dets_only)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 1100
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up
    * Expectation is that these three detections are stored in the detection row struct arrays.
    */
   trailer_detector_TL.set_window_timer(1100U);
   uint8_t exp_row_idx_1 = 22U;
   uint8_t exp_row_idx_2 = 47U;
   uint8_t exp_row_idx_3 = 0U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_2)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_3)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations when window timer is 0.
 */
TEST(f360_process_input_main_functionality, process_input_3_valid_dets_window_timer_is_0s)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 0
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up
    * Expectation is that these three detections are stored in the detection row struct arrays.
    */
   trailer_detector_TL.set_window_timer(0U);
   const uint8_t exp_row_idx_1 = 0U;
   const uint8_t exp_row_idx_2 = 22U;
   const uint8_t exp_row_idx_3 = 47U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct and that the window timer is incremented by one while reset timer is reset to 0.
    */
   const TL_Trailer_Detection_Row local_row_struct =  trailer_detector_TL.get_trailer_detection_row_struct();

   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1)
      {
         CHECK_EQUAL_TEXT(1, local_row_struct.detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_2)
      {
         CHECK_EQUAL_TEXT(1, local_row_struct.detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_3)
      {
         CHECK_EQUAL_TEXT(1, local_row_struct.detection_row[i], "detection_row is incorrectly filled");
      }
      else
      {
         CHECK_EQUAL_TEXT(0, local_row_struct.detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
   CHECK_EQUAL_TEXT(1U, trailer_detector_TL.get_window_timer(), "window timer is not correctly incremented.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations. Set the first detections
 * position such that it is assigned a max row idx to check that it correctly assigns the maximum allowed row idx.
 */
TEST(f360_process_input_main_functionality, process_input_3_valid_dets_new_cals_to_test_idx_overshoot)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 600
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up
    * - First detection's xpos is set to -17.49 such that its initial idx is = ROW_NUMBER-1
    * Expectation is that these three detections are stored in the detection row struct arrays.
    */

   all_detections[0U].vcs_position.x = -17.49F;
   uint8_t exp_row_idx_1 = 22U; // The idx is extected to be saturated at ROW_NUMBER-1
   uint8_t exp_row_idx_2 = 47U;
   uint8_t exp_row_idx_3 = 0U;
   uint8_t exp_row_idx_4 = ROW_NUMBER-1U; // The idx is extected to be saturated at ROW_NUMBER-1
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1) // Check that the previously stored detection is still there.
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_2)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_3)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else if (i == exp_row_idx_4)
      {
         CHECK_EQUAL_TEXT(1, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }   else
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 invalid detections and 1 from previous iterations. The detections are invalid due to
 * their x_pos and range rates respectively
 */
TEST(f360_process_input_main_functionality, process_input_3_invalid_dets_RR_and_xpos)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 600
    *    - a detection row struct with 1 historic detection added to the array
    * 3 invalid detections are set up
    *    - 1 detection with too much high range rate
    *    - 1 detection with xpos too big
    *    - 1 detection with xpos too small 
    * Expectation is that these three detections are not stored in the detection row struct arrays.
    */
   raw_detect_list.detections[0U].raw.range_rate = 1.0F;
   all_detections[1U].vcs_position.x = -1.0F;
   all_detections[2U].vcs_position.x = -25.0F;

   uint8_t exp_row_idx_1 = 22U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1) // Check that only the detection already stored is still there
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else // all other slots should be empty
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 4 invalid detections and 1 from previous iterations. The detections are invalid due to their
 * y_pos, double bounce status, wheel spins status and water spray status respectively.
 */
TEST(f360_process_input_main_functionality, process_input_4_invalid_dets_ypos_double_bounce_wheel_spin_water_spray)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 600
    *    - a detection row struct with 1 historic detection added to the array
    * 4 invalid detections are set up
    *    - 1 detection with ypos too big
    *    - 1 detection flagged as double bounce
    *    - 1 detection flagged as wheel spin
    *    - 1 detection flagged as water spray 
    * Expectation is that these 4 detections are not stored in the detection row struct arrays.
    */
   all_detections[0U].vcs_position.y = 3.0F;
   all_detections[1U].f_double_bounce = true;
   all_detections[2U].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;

   raw_detect_list.number_of_valid_detections = 4U;
   uint32_t det_idx_4 = 3U;
   raw_detect_list.detections[det_idx_4].raw.range_rate = 0.1F;
   raw_detect_list.detections[det_idx_4].raw.sensor_id = 1U;
   all_detections[det_idx_4].vcs_position.x = -8.0F;
   all_detections[det_idx_4].vcs_position.y = 1.0F;
   all_detections[det_idx_4].f_double_bounce = false;
   all_detections[det_idx_4].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   all_detections[det_idx_4].f_water_spray = true;
   sensors[raw_detect_list.detections[det_idx_4].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;

   uint8_t exp_row_idx_1 = 22U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1) // Check that only the detection already stored (in array) is still there
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else // all other slots should be empty
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** \purpose  
 * Test that Process_Input works as intended when there are 3 detections and 1 from previous iterations. The 3 detections are all from non-rear sensors.
 */
TEST(f360_process_input_main_functionality, process_input_3_dets_not_from_rear_sensors)
{
   /** \precond
    * Host speed is set to 10 m/s
    * A mocked instance of Trailer_Detector_TL has been set up in the test group with
    *    - reset_timer = 1800
    *    - window_timer = 600
    *    - a detection row struct with 1 historic detection added to the array
    * 3 valid detections are set up but all are from non-rear sensors
    * Expectation is that these 3 detections are not stored in the detection row struct arrays.
    */
   sensors[raw_detect_list.detections[0U].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE1;
   sensors[raw_detect_list.detections[1U].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE2;
   sensors[raw_detect_list.detections[2U].raw.sensor_id - 1].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1;

   uint8_t exp_row_idx_1 = 22U;
	
   /** \action
    * Call Process_Input()
    */
   trailer_detector_TL.call_process_input(vehicle_data, raw_detect_list, all_detections, sensors);

   /** \result
    * Check that the detections are correclty binned in the detection row struct.
    */
   for (uint8_t i = 0U; i < ROW_NUMBER; i++)
   {
      if (i == exp_row_idx_1) // Check that only the detection already stored (in array) is still there
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
      else // all other slots should be empty
      {
         CHECK_EQUAL_TEXT(0, trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i], "detection_row is incorrectly filled");
      }
   }
   CHECK_EQUAL_TEXT(0U, trailer_detector_TL.get_reset_timer(), "reset timer is not correctly set.")
}

/** @}*/
