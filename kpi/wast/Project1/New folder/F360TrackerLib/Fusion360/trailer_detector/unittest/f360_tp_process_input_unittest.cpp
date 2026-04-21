/** \file
 * This file contains unit tests for content of tp_process_input.cpp file
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

   uint32_t get_window_timer()
   {
      return window_timer;
   }

   void set_window_timer(uint32_t window_timer_set_val)
   {
      window_timer = window_timer_set_val;
   }

   uint32_t get_stationary_timer()
   {
      return stationary_timer;
   }

   void set_stationary_timer(uint32_t stationary_timer_set_val)
   {
      stationary_timer = stationary_timer_set_val;
   }

   TP_Relative_Ratio get_relative_ratio_struct()
   {
      return relative_ratio_struct;
   }

   void set_relative_ratio_struct(TP_Relative_Ratio relative_ratio_struct_set_val)
   {
      relative_ratio_struct = relative_ratio_struct_set_val;
   }

   TP_Cals get_tp_calibs()
   {
      return tp_calibs;
   }

   TP_Mean_RR get_mean_rr_struct()
   {
      return mean_rr_struct;
   }

   void set_mean_rr_struct(TP_Mean_RR mean_rr_struct_set_val)
   {
      mean_rr_struct = mean_rr_struct_set_val;
   }

   void process_input_mock(
      const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_dets,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Process_Input(vehicle_data, raw_dets, det_props, sensors);
   }

   void linspace_mock(
      float32_t first,
      float32_t second,
      float32_t (&value_array)[BIN_SIZE])
   {
      TrailerDetectorTPmock::Linspace(first, second, value_array);
   }
};


/** \defgroup  tp_process_input_Set_Clear_Data
 *  @{
 */

/** \brief
 * This test group checks that the Trailer_Detector_TP class implementation is correctly initializing/setting and clearing data correctly
 */
TEST_GROUP(tp_process_input_Set_Clear_Data)
{	
   // Create an instance of the Trailer_Detector_TP class
   TrailerDetectorTPmock trailer_detector_TP;

   // Get a local copy of the private calibration struct
   const TP_Cals tp_calibs = trailer_detector_TP.get_tp_calibs();
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th_small = 1e-9F;
   const float32_t test_pass_th_large = 1e-4F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance using the Trailer_Detector_TP::Initialize() function.
    */
   TEST_SETUP()
   {
      trailer_detector_TP.Initialize();
   }   
};

/** \purpose  
 * Test that the Trailer_Detector_TP constructor works as intended
 */
TEST(tp_process_input_Set_Clear_Data, Test_Constructor)
{
   /** \precond
    * Create a new instance of the TrailorDetectorTP class.
    */
   TrailerDetectorTPmock new_trailer_detector_TP;

   /** \result
    * Check that the constructor has initialized class members as expected.
    */
   const Trailer_Presence_State exp_trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_presence, new_trailer_detector_TP.get_trailer_presence(), "trailer_presence is not initialized correctly");

   const Trailer_Detector_Conf exp_confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_confidence, new_trailer_detector_TP.get_trailer_presence_conf(), "confidence is not initialized correctly");	

   CHECK_FALSE_TEXT(new_trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is not initialized correctly");	

   const uint32_t exp_window_timer = 0;
   CHECK_EQUAL_TEXT(exp_window_timer, new_trailer_detector_TP.get_window_timer(), "window_timer is not initialized correctly");

   const uint32_t exp_stationary_timer = 0U;
   CHECK_EQUAL_TEXT(exp_stationary_timer, new_trailer_detector_TP.get_stationary_timer(), "stationary_timer is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_02 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_02, new_trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_03 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_03, new_trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_04 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_04, new_trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_05 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_05, new_trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is not initialized correctly");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const int16_t exp_mean_rr_struct_num_array_i = 0;
      CHECK_EQUAL_TEXT(exp_mean_rr_struct_num_array_i, new_trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is not initialized correctly");
   }

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const float32_t rr_step_size = (tp_calibs.k_rr_end - tp_calibs.k_rr_start) / (static_cast<float32_t>(BIN_SIZE));
      const float32_t exp_mean_rr_struct_value_array_i = tp_calibs.k_rr_start + static_cast<float32_t>(i) * rr_step_size + 0.5f * rr_step_size;;
      CHECK_EQUAL_TEXT(exp_mean_rr_struct_value_array_i, new_trailer_detector_TP.get_mean_rr_struct().value_array[i], "mean_rr_struct.value_array is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Initialize() function works as intended
 */
TEST(tp_process_input_Set_Clear_Data, Test_Initialization)
{
   /** \precond
    * Set member variables of Trailer_Detector_TP instance to non default values
    */
   trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_DETECTED);

   trailer_detector_TP.set_trailer_presence_conf(TRAILER_DETECTOR_CONF_MEDIAN);

   trailer_detector_TP.set_f_estimation_done(true);

   trailer_detector_TP.set_window_timer(8U);

   trailer_detector_TP.set_stationary_timer(44U);

   TP_Relative_Ratio relative_ratio_set_val;

   relative_ratio_set_val.dets_n_02 = 12;

   relative_ratio_set_val.dets_n_03 = -2;

   relative_ratio_set_val.dets_n_04 = -55;

   relative_ratio_set_val.dets_n_05 = 99;

   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_set_val);

   TP_Mean_RR mean_rr_struct_set_val;
   for(uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      mean_rr_struct_set_val.num_array[i] = 10;
      mean_rr_struct_set_val.value_array[i] = 1.0F;
   }
   trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);
      
   /** \action
    * Run initialization function
    */
   trailer_detector_TP.Initialize();

   /** \result
    * Check that the class members are initialized (set to default values) as expected.
    */

   const Trailer_Presence_State exp_trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_presence, trailer_detector_TP.get_trailer_presence(), "trailer_presence is not initialized correctly");

   const Trailer_Detector_Conf exp_confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_confidence, trailer_detector_TP.get_trailer_presence_conf(), "confidence is not initialized correctly");	

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is not initialized correctly");	

   const uint32_t exp_window_timer = 0;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TP.get_window_timer(), "window_timer is not initialized correctly");

   const uint32_t exp_stationary_timer = 0U;
   CHECK_EQUAL_TEXT(exp_stationary_timer, trailer_detector_TP.get_stationary_timer(), "stationary_timer is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_02 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_03 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_04 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is not initialized correctly");

   const int16_t exp_relative_ratio_struct_dets_n_05 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is not initialized correctly");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const int16_t exp_mean_rr_struct_num_array_i = 0;
      CHECK_EQUAL_TEXT(exp_mean_rr_struct_num_array_i, trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is not initialized correctly");
   }

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const float32_t rr_step_size = (tp_calibs.k_rr_end - tp_calibs.k_rr_start) / (static_cast<float32_t>(BIN_SIZE));
      const float32_t exp_mean_rr_struct_value_array_i = tp_calibs.k_rr_start + static_cast<float32_t>(i) * rr_step_size + 0.5f * rr_step_size;;
      DOUBLES_EQUAL_TEXT(exp_mean_rr_struct_value_array_i, trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_large, "mean_rr_struct.value_array is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::clear() function works as intended
 */
TEST(tp_process_input_Set_Clear_Data, Test_Clear)
{
   /** \precond
    * Set member variables of Trailer_Detector_TP instance to non default values
    */
   trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_DETECTED);

   trailer_detector_TP.set_trailer_presence_conf(TRAILER_DETECTOR_CONF_MEDIAN);

   trailer_detector_TP.set_f_estimation_done(true);

   trailer_detector_TP.set_window_timer(8U);

   const uint32_t stationary_timer_set_value = 44U;
   trailer_detector_TP.set_stationary_timer(stationary_timer_set_value);

   TP_Relative_Ratio relative_ratio_set_val;

   relative_ratio_set_val.dets_n_02 = 12;

   relative_ratio_set_val.dets_n_03 = -2;

   relative_ratio_set_val.dets_n_04 = -55;

   relative_ratio_set_val.dets_n_05 = 99;

   trailer_detector_TP.set_relative_ratio_struct(relative_ratio_set_val);

   TP_Mean_RR mean_rr_struct_set_val;
   
   for(uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      mean_rr_struct_set_val.num_array[i] = 10;
      mean_rr_struct_set_val.value_array[i] = 1.0F;
   }
   trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);
   
   /** \action
    * Run clear function
    */
   trailer_detector_TP.Clear();

   /** \result
    * Check that the class members are cleared (reset to default values) as expected. All members except stationary_timer are expected to be cleared.
    */

   const Trailer_Presence_State exp_trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_presence, trailer_detector_TP.get_trailer_presence(), "trailer_presence is not cleared correctly");

   const Trailer_Detector_Conf exp_confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_confidence, trailer_detector_TP.get_trailer_presence_conf(), "confidence is not cleared correctly");	

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is not cleared correctly");	

   const uint32_t exp_window_timer = 0;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TP.get_window_timer(), "window_timer is not cleared correctly");

   const uint32_t exp_stationary_timer = stationary_timer_set_value; // It is expected
   CHECK_EQUAL_TEXT(exp_stationary_timer, trailer_detector_TP.get_stationary_timer(), "stationary_timer has been cleared (despite it shouldn't)");

   const int16_t exp_relative_ratio_struct_dets_n_02 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_03 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_04 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_05 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is not cleared correctly");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const int16_t exp_mean_rr_struct_num_array_i = 0;
      CHECK_EQUAL_TEXT(exp_mean_rr_struct_num_array_i, trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is not cleared correctly");
   }

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const float32_t rr_step_size = (tp_calibs.k_rr_end - tp_calibs.k_rr_start) / (static_cast<float32_t>(BIN_SIZE));
      const float32_t exp_mean_rr_struct_value_array_i = tp_calibs.k_rr_start + static_cast<float32_t>(i) * rr_step_size + 0.5f * rr_step_size;
      DOUBLES_EQUAL_TEXT(exp_mean_rr_struct_value_array_i, trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_large, "mean_rr_struct.value_array is not cleared correctly");
   }
}
/** @}*/


/** \defgroup  tp_process_input_Process_Input
 *  @{
 */

/** \brief
 * This test group check so that the Trailer_Detector_TP::Process_Input() function is working correctly.
 */
TEST_GROUP(tp_process_input_Process_Input)
{	
   // Create an instance of the Trailer_Detector_TP class
   TrailerDetectorTPmock trailer_detector_TP;

   // Get a local copy of the private calibration struct
   const TP_Cals tp_calibs = trailer_detector_TP.get_tp_calibs();
   
   // Create variables for default test values of member variables
   Trailer_Presence_State trailer_presence_set_val;
   Trailer_Detector_Conf trailer_presence_conf_set_val;
   bool f_estimation_done_set_val;
   uint32_t window_timer_set_val;
   uint32_t stationary_timer_set_val;
   TP_Relative_Ratio relative_ratio_struct_set_val;
   TP_Mean_RR mean_rr_struct_set_val;
   float32_t min_abs_speed_running_set_val;

   // Create a host data structure
   F360_Host_T vehicle_data;

   // Create detection data structures
   rspp_variant_A::RSPP_Detection_List_T raw_dets;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];

   // Create a sensor data structure
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th_small = 1e-9F;
   const float32_t test_pass_th_large = 1e-4F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with some default values
    * Initilize host data such that Trailer_Detector_TP::Process_Input() is considering host to be approximately stationary
    */
   TEST_SETUP()
   {
      // Initialize Trailer_Detector_TP class instance with default values
      for(uint8_t i = 0U; i < BIN_SIZE; i++)
      {
         mean_rr_struct_set_val.num_array[i] = 10; // Set such that not 0 (which is what the varaible is set to as default in constructor/initialization/clear functions)
         mean_rr_struct_set_val.value_array[i] = 1.0F; // Set such that not equal to linspace(tp_cals_set_val.k_rr_start, tp_cals_set_val.k_rr_end, BIN_SIZE) (which is what the varaible is set to as default in constructor/initialization/clear functions)
      }
      trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);

      trailer_presence_set_val = TRAILER_PRESENCE_STATE_NOT_DETECTED;  // Set such that trailer_precence is not DETECTED (such that we ensure that no data clearing is performed inside of Process_Input() function) and not UNKNOWN (which is what the varaible is set to as default in constructor/initialization/clear functions)
      trailer_detector_TP.set_trailer_presence(trailer_presence_set_val);

      trailer_presence_conf_set_val = TRAILER_DETECTOR_CONF_LOW; // Set such that trailer_presence_conf is not UNKNOWN (which is what the varaible is set to as default in constructor/initialization/clear functions)
      trailer_detector_TP.set_trailer_presence_conf(trailer_presence_conf_set_val); 

      f_estimation_done_set_val = true; // Set such that f_estimation_done is not false (which is what the varaible is set to as default in constructor/initialization/clear functions)
      trailer_detector_TP.set_f_estimation_done(f_estimation_done_set_val); 

      window_timer_set_val = tp_calibs.k_time_threshold; // Set such that window_timer is large (to prevent updataing of data inside of Process_Input()) and not equal to 0 (which is what the varaible is set to as default in constructor/initialization/clear functions).
      trailer_detector_TP.set_window_timer(window_timer_set_val); 

      stationary_timer_set_val = tp_calibs.k_reset_cnt_threshold - 1U; // Set such that is not 0U (which is what the varaible is set to as default in constructor/initialization/clear functions) and close to being equal to tp_cals.k_reset_cnt_threshold (which should enable data to be cleared if host speedd and f_trailer_precense is enabling)
      trailer_detector_TP.set_stationary_timer(stationary_timer_set_val); 

      relative_ratio_struct_set_val.dets_n_02 = 12; // Set such that is not 0 (which is what the varaible is set to as default in constructor/initialization/clear functions)
      relative_ratio_struct_set_val.dets_n_03 = -2; // Set such that is not 0 (which is what the varaible is set to as default in constructor/initialization/clear functions)
      relative_ratio_struct_set_val.dets_n_04 = -55; // Set such that is not 0 (which is what the varaible is set to as default in constructor/initialization/clear functions)
      relative_ratio_struct_set_val.dets_n_05 = 99; // Set such that is not 0 (which is what the varaible is set to as default in constructor/initialization/clear functions)
      trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val); 

      // Initialize host data
      vehicle_data.speed = -(tp_calibs.k_speed_threshold_clear + F360_EPSILON); // Set such that host is reversing faset enough; Negative speed should ensure that nop data collection is performed inside of Process_Input() function and fast enough absolute speed should ensure that no data clearing is performed
  
      // Initialize detection data
      raw_dets.number_of_valid_detections = 7U;

      // Setup first detection such that it is valid to update  relative_ratio_struct.dets_n_02 and mean_rr_struct.num_array[1]
      raw_dets.detections[0].raw.sensor_id = 1;
      raw_dets.detections[0].raw.range_rate = -(0.2F - F360_EPSILON);
      det_props[0].f_double_bounce = false;
      det_props[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[0].f_water_spray = false;
      det_props[0].vcs_position.x = tp_calibs.k_long_max - F360_EPSILON;
      det_props[0].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;

      // Setup second detection such that it is valid to update relative_ratio_struct.dets_n_02 and mean_rr_struct.num_array[1]
      raw_dets.detections[1].raw.sensor_id = 1;
      raw_dets.detections[1].raw.range_rate = 0.2F - F360_EPSILON;
      det_props[1].f_double_bounce = false;
      det_props[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[1].f_water_spray = false;
      det_props[1].vcs_position.x = tp_calibs.k_long_max - F360_EPSILON;
      det_props[1].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;

      // Setup third detection such that its range rate is valid to update relative_ratio_struct.dets_n_03 and mean_rr_struct.num_array[2]
      raw_dets.detections[2].raw.sensor_id = 2;
      raw_dets.detections[2].raw.range_rate = 0.3F - F360_EPSILON;
      det_props[2].f_double_bounce = false;
      det_props[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[2].f_water_spray = false;
      det_props[2].vcs_position.x = tp_calibs.k_long_min + F360_EPSILON;
      det_props[2].vcs_position.y = -(tp_calibs.k_lat_abs - F360_EPSILON);

      // Setup fourth detection such that its range rate is valid to update  relative_ratio_struct.dets_n_04 and mean_rr_struct.num_array[3]
      raw_dets.detections[3].raw.sensor_id = 2;
      raw_dets.detections[3].raw.range_rate = 0.4F - F360_EPSILON;
      det_props[3].f_double_bounce = false;
      det_props[3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[3].f_water_spray = false;
      det_props[3].vcs_position.x = tp_calibs.k_long_min + F360_EPSILON;
      det_props[3].vcs_position.y = -(tp_calibs.k_lat_abs - F360_EPSILON);

      // Setup fifth detection such that its range rate is valid to update relative_ratio_struct.dets_n_05 and mean_rr_struct.num_array[4]
      raw_dets.detections[4].raw.sensor_id = 3;
      raw_dets.detections[4].raw.range_rate = -(0.5F - F360_EPSILON);
      det_props[4].f_double_bounce = false;
      det_props[4].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[4].f_water_spray = false;
      det_props[4].vcs_position.x = tp_calibs.k_long_max - F360_EPSILON;
      det_props[4].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;

      // Setup sixth detection such that its range rate is not valid to update any of relative_ratio_struct.dets_n_0i (i = 2 to 5) but is valid to update and mean_rr_struct.num_array[5]
      raw_dets.detections[5].raw.sensor_id = 3;
      raw_dets.detections[5].raw.range_rate = 0.5F + F360_EPSILON;
      det_props[5].f_double_bounce = false;
      det_props[5].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[5].f_water_spray = false;
      det_props[5].vcs_position.x = tp_calibs.k_long_max - F360_EPSILON;
      det_props[5].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;

      // Setup seventh detection such that its range rate is not valid to update any of relative_ratio_struct.dets_n_0i (i = 2 to 5) and mean_rr_struct.num_array[i] (i = 0 to BIN_SIZE-1)
      raw_dets.detections[6].raw.sensor_id = 3;
      raw_dets.detections[6].raw.range_rate = tp_calibs.k_rr_end - 0.5F + F360_EPSILON;;
      det_props[6].f_double_bounce = false;
      det_props[6].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[6].f_water_spray = false;
      det_props[6].vcs_position.x = tp_calibs.k_long_min + F360_EPSILON;
      det_props[6].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;

      // Setup sensor calibration
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR; // Sensor with id 1 is valid for trailor detector
      sensors[1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR; // Sensor with id 2 is valid for trailor detector
      sensors[2].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR; // Sensor with id 3 is valid for trailor detector
      sensors[3].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD; // Sensor with id 4 is not valid for trailor detector
      sensors[4].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD; /// Sensor with id 5 is not valid for trailor detector
      sensors[5].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD; // Sensor with id 6 is not valid for trailor detector
      sensors[6].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE1; // Sensor with id 7 is not valid for trailor detector
      sensors[7].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1; // Sensor with id 8 is not valid for trailor detector
   }
};

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't clear (and don't update) any member variables when host is reversing fast enough, f_trailer_precence is not equal to 1U but stationary_timer has been saturated at its maximum allowed value.
 */
TEST(tp_process_input_Process_Input, Test_Dont_Clear_blocked_by_host_speed_and_f_trailer_presence_enabled_by_stationary_timer)
{
   /** \precond
    * Use default set up from test group. Host speed, f_trailer_precense and stationary_timer has already been correctly intialized there
    */

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values)
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(window_timer_set_val, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}


/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't clear (and don't update) any member variables when f_trailer_precence is not equal to 1U but host is reversing slow enough to clear and stationary timer has been saturated at is maximum value
 */
TEST(tp_process_input_Process_Input, Test_Dont_Clear_blocked_by_f_trailer_presence_enabled_by_host_speed_and_stationary_timer)
{
   /** \precond
    * f_trailer_presence and stationary timer was was correctly initialized in test group.
    * Set host speed to be small enough
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold_clear - F360_EPSILON); // Use negative speed to disable update


   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values)
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(window_timer_set_val, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't clear (and don't update) any member variables when f_trailer_precence is not equal to 1U but timer is larger than maximum value allowed
 */
TEST(tp_process_input_Process_Input, Test_Dont_Clear_blocked_by_f_trailer_presence_enabled_by_host_speed_and_wrong_timer)
{
   /** \precond
    * f_trailer_presence and stationary timer was was correctly initialized in test group.
    * Set host speed to be small enough
    */
   vehicle_data.speed = tp_calibs.k_speed_threshold + F360_EPSILON; // Use negative speed to disable update
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold + 1);  // Use out-range timer


   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values)
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(window_timer_set_val + 1, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}
/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function clears member variables when host is reversing slow enough, f_trailer_precence is equal to 1U and stationary timer has been saturated at is maximum value
 */
TEST(tp_process_input_Process_Input, Test_Clear)
{
   /** \precond
    * stationary timer was was correctly initialized in test group.
    * Set host speed to be small enough.
    * Set trailer_precence to DETECTED
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold_clear - F360_EPSILON); // Use negative speed to disable update functionality
   trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_DETECTED);


   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that the class members are cleared (reset to default values) as expected.
    * All members except stationary_timer are expected to be cleared.
    * Stationary timer are expected to increase by 1.
    */

   const Trailer_Presence_State exp_trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_presence, trailer_detector_TP.get_trailer_presence(), "trailer_presence is not cleared correctly");

   const Trailer_Detector_Conf exp_confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_confidence, trailer_detector_TP.get_trailer_presence_conf(), "confidence is not cleared correctly");	

   CHECK_FALSE_TEXT(trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is not cleared correctly");	

   const uint32_t exp_window_timer = 0;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TP.get_window_timer(), "window_timer is not cleared correctly");

   const uint32_t exp_stationary_timer = stationary_timer_set_val + 1U; // We are expecting stationary timer to increase by 1
   CHECK_EQUAL_TEXT(exp_stationary_timer, trailer_detector_TP.get_stationary_timer(), "stationary_timer has been cleared (despite it shouldn't)");

   const int16_t exp_relative_ratio_struct_dets_n_02 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_03 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_04 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is not cleared correctly");

   const int16_t exp_relative_ratio_struct_dets_n_05 = 0;
   CHECK_EQUAL_TEXT(exp_relative_ratio_struct_dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is not cleared correctly");

   const float32_t rr_step = (tp_calibs.k_rr_end - tp_calibs.k_rr_start) / static_cast<float32_t>(BIN_SIZE);
   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      const int16_t exp_mean_rr_struct_num_array_i = 0;
      CHECK_EQUAL_TEXT(exp_mean_rr_struct_num_array_i, trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is not cleared correctly");

      const float32_t exp_mean_rr_struct_value_array_i = tp_calibs.k_rr_start + static_cast<float32_t>(i) * rr_step + 0.5f * rr_step;
      DOUBLES_EQUAL_TEXT(exp_mean_rr_struct_value_array_i, trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_large, "mean_rr_struct.value_array is not cleared correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't clear (and don't update) any member variables when stationary timer has not been saturated at is maximum value but when host is reversing slow enough to clear and f_trailer_precence is equal to 1U
 */
TEST(tp_process_input_Process_Input, Test_Dont_Clear_blocked_by_stationary_timer_enabled_by_host_speed_and_f_trailer_presence)
{
   /** \precond
    * Set host speed to be small enough.
    * Set trailer_precence to DETECTED
    * Set stationary timer to not be close to tp_calibs.k_reset_cnt_threshold
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold_clear - F360_EPSILON); // Use negative speed to disable update functionality
   trailer_detector_TP.set_trailer_presence(TRAILER_PRESENCE_STATE_DETECTED);
   trailer_detector_TP.set_stationary_timer(tp_calibs.k_reset_cnt_threshold / 2);


   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members except for stationary timer are updated or cleared (reset to default values).
    * Check that stationary_timer has increased by 1.
    */
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()"); // Note trailer_presence was chaged to 1U during precodition step of this test

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(window_timer_set_val, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   const uint32_t exp_stationary_timer = tp_calibs.k_reset_cnt_threshold / 2 + 1U; // We are expecting stationary timer to increase by 1. Note stationary timer was set to tp_calibs.k_reset_cnt_threshold / 2 during precodition step of this test
   CHECK_EQUAL_TEXT(exp_stationary_timer, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}


/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't update (and don't clear) any member variables when absolute host_speed is small, host speed is negative and window_timer is large
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_blocked_by_abs_host_speed_and_host_speed_sign_and_window_timer)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * detection data is correctly setup in the test group to enable updating of data
    * window_timer is correctly set up in the test group to prevent updating of data
    * Set host speed to be negative but absolute host speed to be small to prevent updating of data
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold - F360_EPSILON);

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values).
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(window_timer_set_val, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't update (and don't clear) any member variables when absolute host_speed is large but host speed is negative and window_timer is large
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_blocked_by_host_speed_sign_and_window_timer_enabled_by_abs_host_speed)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * detection data is correctly setup in the test group to enable updating of data
    * window_timer is correctly set up in the test group to prevent updating of data
    * Set host speed to be negative but absolute host speed to be large to prevent updating of data
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold + F360_EPSILON);

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values).
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(window_timer_set_val, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function don't update (and don't clear) any member variables when absolute host_speed is large, window_timer is small but host speed is negative 
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_blocked_by_host_speed_sign_enabled_by_abs_host_speed_and_window_timer)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * detection data is correctly setup in the test group to enable updating of data
    * Set host speed to be negative but absolute host speed to be large to prevent updating of data
    * Set up window_timer to be small to enable updating of data
    */
   vehicle_data.speed =  -(tp_calibs.k_speed_threshold + F360_EPSILON);
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1);


   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated or cleared (reset to default values).
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(tp_calibs.k_time_threshold - 1, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()"); // Note: window_timer was changed to tp_calibs.k_time_threshold - 1 in the precondition step in this test.

   CHECK_EQUAL_TEXT(stationary_timer_set_val, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()");

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()");
      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function updates member variables when absolute host_speed is large, window_timer is small and host speed is positive 
 */
TEST(tp_process_input_Process_Input, Test_Update)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * detection data is correctly setup in the test group to enable updating of data
    * Set host speed to be positive but absolute host speed to be large to enable updating of data
    * Set up window_timer to be small to enable updating of data
    */
   vehicle_data.speed =  tp_calibs.k_speed_threshold + F360_EPSILON;
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1);

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members are updated as expected. All members except for window_timer, stationary_timer, relative_ratio_struct and mean_rr_struct_set_val.num_array are expected to be unchanged
    * window_timer is expected to be increased by 1
    * stationary_timer is expected to be reset to 0
    * relative_ratio_struct.dets_n_02 are expected to be increased by 1 while dets_03, dets_04, and dets 05 are expected to be increased by 1
    * mean_rr_struct_set_val.num_array[1] are expected to be increased by 2 while num_array[2], num_array[3], num_array[4] and num_array[5] are expected to be increased by 1.
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(tp_calibs.k_time_threshold, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()"); // Note: window_timer was changed to tp_calibs.k_time_threshold - 1 in the precondition step in this test and are expected to increase by 1

   CHECK_EQUAL_TEXT(0U, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()"); // Note: Statinary timer are expected to be reset to 0

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02 + 2, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()"); // Has been updated by 2 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03 + 1, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04 + 1, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05 + 1, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      if(i == 1U)
      {
         CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i] + 2, trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has been updated by 2 detections
      }
      else if ((i > 1) && (i <= 5))
      {
         CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i] + 1, trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detection
      }
      else
      {
         CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has not been updated by any detections
      }

      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function does not update updates member variables despite that absolute host_speed is large, window_timer is small and host speed is positive due to no detections
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_no_dets)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * Set host speed to be positive but absolute host speed to be large to enable updating of data
    * Set up window_timer to be small to enable updating of data
    * detection data is setup in the test group to enable updating of data. Change number of detection to 0 to prevent updating data
    */
   vehicle_data.speed =  tp_calibs.k_speed_threshold + F360_EPSILON;
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold / 4);
   raw_dets.number_of_valid_detections = 0U;

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members except for stationary_timer and window_timer are updated (or cleared).
    * stationary_timer are expected to be reset to 0
    * window_timer are expected to be ioncreased by one
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(tp_calibs.k_time_threshold / 4 + 1, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()"); // Note: window_timer was changed to tp_calibs.k_time_threshold / 4 in the precondition step in this test and is expected to increase by 1

   CHECK_EQUAL_TEXT(0U, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()"); // Note: Statinary timer are expected to be reset to 0

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()"); // Has been updated by 2 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has not been updated by any detections

      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}


/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function does not update updates member variables despite that absolute host_speed is large, window_timer is small and host speed is positive due to detections are not from rear sensors
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_blocked_by_sensor_mounting_pos)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * Set host speed to be positive but absolute host speed to be large to enable updating of data
    * Set up window_timer to be small to enable updating of data
    * detection data is setup in the test group to enable updating of data. Change so that detections origin from sensors that are not mounted on rear of host vehicle to disable update of data
    */
   vehicle_data.speed =  tp_calibs.k_speed_threshold + F360_EPSILON;
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1);
   raw_dets.detections[0].raw.sensor_id = 4;
   raw_dets.detections[1].raw.sensor_id = 5;
   raw_dets.detections[2].raw.sensor_id = 6;
   raw_dets.detections[3].raw.sensor_id = 7;
   raw_dets.detections[4].raw.sensor_id = 8;
   raw_dets.detections[5].raw.sensor_id = 4;
   raw_dets.detections[6].raw.sensor_id = 5;
   
   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members except for stationary_timer and window_timer are updated (or cleared).
    * stationary_timer are expected to be reset to 0
    * window_timer are expected to be increased by one
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(tp_calibs.k_time_threshold, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()"); // Note: window_timer was changed to tp_calibs.k_time_threshold - 1 in the precondition step in this test and are expected to increase by 1

   CHECK_EQUAL_TEXT(0U, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()"); // Note: Statinary timer are expected to be reset to 0

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()"); // Has been updated by 2 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has not been updated by any detections

      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TP::Process_Input() function does not update updates member variables despite that absolute host_speed is large, window_timer is small and host speed is positive due to different detection flags and properties
 */
TEST(tp_process_input_Process_Input, Test_Dont_Update_blocked_by_detection_props)
{
   /** \precond
    * f_trailer_precence and is correctly setup in test group to prevent clearing of data
    * Set host speed to be positive but absolute host speed to be large to enable updating of data
    * Set up window_timer to be small to enable updating of data
    * detection data is setup in the test group to enable updating of data. Change so that different detection flags and properties are preventing update of data
    */
   vehicle_data.speed =  tp_calibs.k_speed_threshold + F360_EPSILON;
   trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1);

   det_props[0].f_double_bounce = true;
   det_props[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   det_props[2].f_water_spray = true;
   det_props[3].vcs_position.x = tp_calibs.k_long_max + 1e-6F;
   det_props[4].vcs_position.x = tp_calibs.k_long_min - 1e-6F;
   det_props[5].vcs_position.y = tp_calibs.k_lat_abs + 1e-6F;
   det_props[6].vcs_position.y = -(tp_calibs.k_lat_abs + 1e-6F);
   raw_dets.detections[6].raw.range_rate = 0.1F; // For detection with index 6 we also need to change range rate otherwise the range rate will also prevent update and we can not tell if the vcs_position.y condition is doing it's job or not
   
   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TP.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no class members except for stationary_timer and window_timer are updated (or cleared).
    * stationary_timer are expected to be reset to 0
    * window_timer are expected to be increased by one
    */
   CHECK_EQUAL_TEXT(trailer_presence_set_val, trailer_detector_TP.get_trailer_presence(), "trailer_presence is unexpectedly changed after call to Process_Input()");

   CHECK_EQUAL_TEXT(trailer_presence_conf_set_val, trailer_detector_TP.get_trailer_presence_conf(), "confidence is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(f_estimation_done_set_val, trailer_detector_TP.get_f_estimation_done(), "f_estimation_done is unexpectedly changed after call to Process_Input()");	

   CHECK_EQUAL_TEXT(tp_calibs.k_time_threshold, trailer_detector_TP.get_window_timer(), "window_timer is unexpectedly changed after call to Process_Input()"); // Note: window_timer was changed to tp_calibs.k_time_threshold - 1 in the precondition step in this test and are expected to increase by 1

   CHECK_EQUAL_TEXT(0U, trailer_detector_TP.get_stationary_timer(), "stationary_timer is unexpectedly changed after call to Process_Input()"); // Note: Statinary timer are expected to be reset to 0

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_02, trailer_detector_TP.get_relative_ratio_struct().dets_n_02, "relative_ratio_struct.dets_n_02 is unexpectedly changed after call to Process_Input()"); // Has been updated by 2 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_03, trailer_detector_TP.get_relative_ratio_struct().dets_n_03, "relative_ratio_struct.dets_n_03 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_04, trailer_detector_TP.get_relative_ratio_struct().dets_n_04, "relative_ratio_struct.dets_n_04 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   CHECK_EQUAL_TEXT(relative_ratio_struct_set_val.dets_n_05, trailer_detector_TP.get_relative_ratio_struct().dets_n_05, "relative_ratio_struct.dets_n_05 is unexpectedly changed after call to Process_Input()"); // Has been updated by 1 detections during function call

   for (uint8_t i = 0U; i < BIN_SIZE; i++)
   {
      CHECK_EQUAL_TEXT(mean_rr_struct_set_val.num_array[i], trailer_detector_TP.get_mean_rr_struct().num_array[i], "mean_rr_struct.num_array is unexpectedly changed after call to Process_Input()"); // Has not been updated by any detections

      DOUBLES_EQUAL_TEXT(mean_rr_struct_set_val.value_array[i], trailer_detector_TP.get_mean_rr_struct().value_array[i], test_pass_th_small, "mean_rr_struct.value_array is unexpectedly changed after call to Process_Input()");
   }
}
/** @}*/


/** \defgroup  tp_process_input_Linspace
 *  @{
 */

/** \brief
 * This test group checks that the Trailer_Detector_TP::linspace function is working as expected
 */
TEST_GROUP(tp_process_input_Linspace)
{	
   // Create an instance of the Trailer_Detector_TP class
   TrailerDetectorTPmock trailer_detector_TP;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-6F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with some default values
    */
   TEST_SETUP()
   {
      // Initialize Trailer_Detector_TP class instance
      trailer_detector_TP.Initialize();
   }
};

/** \purpose  
 * Test that the Trailer_Detector_TP::linspace() function works as expected with BIN_SIZE bins
 */
TEST(tp_process_input_Linspace, Test_100_bins)
{
   /** \precond
    * Setup 10 bins
    */
   const float32_t start_val = 1.1F;
   const float32_t step_size = 1.5F;
   const uint8_t nr_bins = BIN_SIZE;
   const float32_t end_val = start_val + static_cast<float32_t>(nr_bins) * step_size; 
   float32_t output_value_array[BIN_SIZE];
   float32_t expected_value_array[BIN_SIZE];
   expected_value_array[0] = 1.10000002F + step_size*0.5f;
   expected_value_array[1] = 2.5999999F + step_size*0.5f;
   expected_value_array[2] = 4.0999999F + step_size*0.5f;
   expected_value_array[3] = 5.5999999F + step_size*0.5f;
   expected_value_array[4] = 7.0999999F + step_size*0.5f;
   expected_value_array[5] = 8.60000038F + step_size*0.5f;
   expected_value_array[6] = 10.1000004F + step_size*0.5f;
   expected_value_array[7] = 11.6000004F + step_size*0.5f;
   expected_value_array[8] = 13.1000004F + step_size*0.5f;
   expected_value_array[9] = 14.6000004F + step_size*0.5f;
   
   /** \action
    * Run linspace() function
    */
   trailer_detector_TP.linspace_mock(start_val, end_val, output_value_array);

   /** \result
    * Check that output value array contains expected values
    */
   for (uint8_t i = 0U; i < 10; i++)
   {
      DOUBLES_EQUAL(expected_value_array[i], output_value_array[i], test_pass_th);
   }
}
/** @}*/
