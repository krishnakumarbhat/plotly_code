/** \file
 * This file contains unit tests for content of tw_process_input.cpp file
 */

#include "f360_trailer_detector_TW.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

class TrailerDetectorTWmock : public Trailer_Detector_TW
{
   public:
   TW_Cals get_tw_calibs()
   {
      return tw_calibs;
   }

   uint32_t get_reset_timer()
   {
      return reset_timer;
   }

   void set_reset_timer(uint32_t reset_timer_set_val)
   {
      reset_timer = reset_timer_set_val;
   }

   uint32_t get_window_timer()
   {
      return window_timer;
   }

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

   void get_detection_col(uint32_t (&output_detection_col)[COL_NUMBER])
   {
      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         output_detection_col[i] = detection_col[i];
      }
   }

   void set_detection_col(const uint32_t (&detection_col_set_val)[COL_NUMBER])
   {
      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         detection_col[i] = detection_col_set_val[i];
      }
   }

   void process_input_mock(
      const F360_Host_T&vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
      {
         Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);
      }

};

/** \defgroup  tw_process_input_Set_Clear_Data
 *  @{
 */

/** \brief
 * Test checks that the Trailer_Detector_TW class implementation is correctly initializing and clear data correctly
 */
TEST_GROUP(tw_process_input_Set_Clear_Data)
{	
   // Create an instance of the Trailer_Detector_TW class
   TrailerDetectorTWmock trailer_detector_TW;

   // Get a local copy of the private calibration struct
   const TW_Cals tw_calibs = trailer_detector_TW.get_tw_calibs();

   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th_small = 1e-9F;
   const float32_t test_pass_th_large = 1e-4F;   

   /** \setup
    * Initlialize the TrailerDetetorTW class instance with some default values
    */
   TEST_SETUP()
   {
      trailer_detector_TW.Initialize();
   }
};

/** \purpose  
 * Test that the Trailer_Detector_TW constructor works as intended
 */
TEST(tw_process_input_Set_Clear_Data, Test_Constructor)
{
   /** \precond
   * Create a new instance of the TrailerDetectionTW class
   * Create fill output with non default values
   */
	TrailerDetectorTWmock new_trailer_detector_TW;

   /** \result
   * Check that the constructor hos initlialized class members as expected
   */	
   CHECK_FALSE_TEXT(new_trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is not initialized correctly");	

   const float32_t exp_trailer_width = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_width, new_trailer_detector_TW.get_trailer_width(), test_pass_th_small, "trailer_width is not initialized correctly");

   const Trailer_Detector_Conf exp_trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_width_conf, new_trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is not initialized correctly");
   
   const uint32_t exp_reset_timer = 0U;
   CHECK_EQUAL_TEXT(exp_reset_timer, new_trailer_detector_TW.get_reset_timer(), "reset_timer is not initialized correctly");
   
   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, new_trailer_detector_TW.get_window_timer(), "window_timer is not initialized correctly");
   
   uint32_t output_detection_col[COL_NUMBER];
   new_trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint8_t i = 0U; i < COL_NUMBER; i++)
   {
      const uint32_t exp_detection_col = 0U;
      CHECK_EQUAL_TEXT(exp_detection_col, output_detection_col[i], "detection_col is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TW::initialize() function works as intended
 */
TEST(tw_process_input_Set_Clear_Data, Test_Initialization)
{
   /** \precond
   * Set member variables of Trailer_Detector_TW instance to non default values
   */
   trailer_detector_TW.set_f_estimation_done(true);
   trailer_detector_TW.set_trailer_width(11.9F);
   trailer_detector_TW.set_trailer_width_conf(TRAILER_DETECTOR_CONF_MEDIAN);
   trailer_detector_TW.set_reset_timer(17U);
   trailer_detector_TW.set_window_timer(500U);

   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {
      output_detection_col[i] = 1U;
   }

   /** \action
   * Run initialization function
   */
   trailer_detector_TW.Initialize();

   /** \result
   * Check that the class members have been initialized (set to default) correctly.
   */	
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is not initialized correctly");	

   const float32_t exp_trailer_width = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th_small, "trailer_width is not initialized correctly");

   const Trailer_Detector_Conf exp_trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_width_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is not initialized correctly");
   
   const uint32_t exp_reset_timer = 0U;
   CHECK_EQUAL_TEXT(exp_reset_timer, trailer_detector_TW.get_reset_timer(), "reset_timer is not initialized correctly");

   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TW.get_window_timer(), "window_timer is not initialized correctly");
   
   trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {
      const uint32_t exp_detection_col = 0U;
      CHECK_EQUAL_TEXT(exp_detection_col, output_detection_col[i], "detection_col is not initialized correctly");
   }
}

/** \purpose  
 * Test that the Trailer_Detector_TW::clear() function works as intended
 */
TEST(tw_process_input_Set_Clear_Data, Test_Clear)
{
   /** \precond
   * Set member variables of Trailer_Detector_TW instance to non default values
   */
   trailer_detector_TW.set_f_estimation_done(true);
   trailer_detector_TW.set_trailer_width(11.9F);
   trailer_detector_TW.set_trailer_width_conf(TRAILER_DETECTOR_CONF_MEDIAN);
   trailer_detector_TW.set_reset_timer(17U);
   trailer_detector_TW.set_window_timer(500U);

   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {
      output_detection_col[i] = 1U;
   }

   /** \action
    * Run clear function.
    */
   trailer_detector_TW.Clear();

   /** \result
    * Check that the class members (expect for reset_timer) have been cleared (set to default values) correctly.
    */
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is not cleared correctly");	

   const float32_t exp_trailer_width = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th_small, "trailer_width is not cleared correctly");

   const Trailer_Detector_Conf exp_trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_width_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is not cleared correctly");
   
   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TW.get_window_timer(), "window_timer is not cleared correctly");
   
   trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {
      const uint32_t exp_detection_col = 0U;
      CHECK_EQUAL_TEXT(exp_detection_col, output_detection_col[i], "detection_col is not cleared correctly");
   }
}

/** @}*/


/** \defgroup tw_process_input_Process_Input
 *  @{
 */

/** \brief
 * Test checks that the Trailer_Detector_TW class implementation is correctly initializing and clear data correctly
 */
TEST_GROUP(tw_process_input_Process_Input)
{	
   // Create an instance of the Trailer_Detector_TW class
   TrailerDetectorTWmock trailer_detector_TW;

   // Get a local copy of the private calibration struct
   TW_Cals tw_calibs = trailer_detector_TW.get_tw_calibs();
   
   // Create variables for default test values of member variables
   bool f_estimation_done_set_val;
   float32_t trailer_width_set_val;
   Trailer_Detector_Conf trailer_width_conf_set_val;
   uint32_t reset_timer_set_val;
   uint32_t window_timer_set_val;
   uint32_t detection_col_set_val[COL_NUMBER];

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
    * Initlialize the TrailerDetetorTW class instance with some default values
    */
   TEST_SETUP()
   {
      trailer_detector_TW.Initialize();

      f_estimation_done_set_val = true;                        // Set such that variable is not false (which is the default value in constructor/initlialization/clear functions)
      trailer_detector_TW.set_f_estimation_done(f_estimation_done_set_val);

      trailer_width_set_val = 3.0F;                            // Set such that variable is not 0.0F (which is the default value in constructor/initlialization/clear functions)
      trailer_detector_TW.set_trailer_width(trailer_width_set_val);

      reset_timer_set_val = 10U;                                // Set such that variable is not 0 (which is the default value in constructor/initlialization/clear functions)
      trailer_detector_TW.set_reset_timer(reset_timer_set_val);

      window_timer_set_val = 10U;                               // Set such that variable is not 0 (which is the default value in constructor/initlialization/clear functions)
      trailer_detector_TW.set_window_timer(window_timer_set_val);

      trailer_width_conf_set_val = TRAILER_DETECTOR_CONF_LOW;  // Set such that variable is not UNKNOWN (which is the default value in constructor/initlialization/clear functions)
      trailer_detector_TW.set_trailer_width_conf(trailer_width_conf_set_val);

      // Set that there are at least one detection at each bin (which is default value in constructor/initlialization/clear functions)
      for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {
         detection_col_set_val[i] = 1U;
      }
      trailer_detector_TW.set_detection_col(detection_col_set_val);

      // Initialize host data
      vehicle_data.speed = tw_calibs.k_speed_threshold + F360_EPSILON; // Set such that host is drving fast enough to ensure we register detection when moving

      // Initialize detection data
      raw_dets.number_of_valid_detections = 9U;

      // Setup first detection such that it can be assigned a column/bin number
      raw_dets.detections[0].raw.sensor_id = 3;                                   // Left rear sensor
      raw_dets.detections[0].raw.range_rate = -(0.2F + F360_EPSILON);             // Any value such that |range_rate| < 0.3 
      det_props[0].vcs_position.x = -13.0F;                                   // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[0].vcs_position.y = -1.5F;                                    // Left trailer edge
      det_props[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[0].f_water_spray = false;
      
      // Setup second detection such that it can be assigned a column/bin number
      raw_dets.detections[1].raw.sensor_id = 2;                                   // Center rear sensor
      raw_dets.detections[1].raw.range_rate = (0.2F + F360_EPSILON);              // Any value such that |range_rate| < 0.3         
      det_props[1].vcs_position.x = -6.0F;                                    // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[1].vcs_position.y = 0.0F;                                     // Lateral position anywhere between -1.5 and 1.5 since k_max_trailer_width = 3.0
      det_props[1].f_double_bounce = false;
      det_props[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[1].f_water_spray = false;

      // Setup third detection such that it can be assigned a column/bin number
      raw_dets.detections[2].raw.sensor_id = 1;                                   // Right rear sensor
      raw_dets.detections[2].raw.range_rate = 0.1F;                               // Any value such that |range_rate| < 0.3          
      det_props[2].vcs_position.x = -10.3F;                                   // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[2].vcs_position.y = 1.5F;                                     // Right trailer edge 
      det_props[2].f_double_bounce = false;
      det_props[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[2].f_water_spray = false;
      
      // Setup fourth detection such that it can be assigned a column/bin number
      raw_dets.detections[3].raw.sensor_id = 1;                                   // Right rear sensor
      raw_dets.detections[3].raw.range_rate = 0.3F - F360_EPSILON;                // Any value such that |range_rate| < 0.3 
      det_props[3].vcs_position.x = -9.0F;                                    // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[3].vcs_position.y = 0.7F;                                     // Lateral position anywhere between -1.5 and 1.5 since k_max_trailer_width = 3.0
      det_props[3].f_double_bounce = false;
      det_props[3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[3].f_water_spray = false;

      // Setup fifth detection such that it can be assigned a column/bin number
      raw_dets.detections[4].raw.sensor_id = 2;                                   // Center rear sensor
      raw_dets.detections[4].raw.range_rate = 0.01F;                              // Any value such that |range_rate| < 0.3 
      det_props[4].vcs_position.x = -17.0F;                                   // Trailer rear 
      det_props[4].vcs_position.y = 0.0;                                      // Lateral position anywhere between -1.5 and 1.5 since k_max_trailer_width = 3.0
      det_props[4].f_double_bounce = false;
      det_props[4].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[4].f_water_spray = false;
      
      // Setup sixth detection such that it can be assigned a column/bin number
      raw_dets.detections[5].raw.sensor_id = 1;                                   // Right rear sensor
      raw_dets.detections[5].raw.range_rate = 0.3F - F360_EPSILON;                //  Any value such that |range_rate| < 0.3 
      det_props[5].vcs_position.x = -17.5F;                                   // Trailer rear 
      det_props[5].vcs_position.y = 1.5F;                                     // Right trailer edge 
      det_props[5].f_double_bounce = false;
      det_props[5].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[5].f_water_spray = false;

      // Setup seventh detection such that it can be assigned a column/bin number
      raw_dets.detections[6].raw.sensor_id = 3;                                   // Left rear sensor
      raw_dets.detections[6].raw.range_rate = 0.3F - F360_EPSILON;                //  Any value such that |range_rate| < 0.3 
      det_props[6].vcs_position.x = -5.5F;                                    // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[6].vcs_position.y = -1.5F;                                    // Left trailer edge 
      det_props[6].f_double_bounce = false;
      det_props[6].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[6].f_water_spray = false;

      // Setup eight detection such that it can be assigned a column/bin number
      raw_dets.detections[7].raw.sensor_id = 2;                                   // Center rear sensor
      raw_dets.detections[7].raw.range_rate = 0.3F - F360_EPSILON;                //  Any value such that |range_rate| < 0.3 
      det_props[7].vcs_position.x = -5.9F;                                    // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[7].vcs_position.y = 0.92F;                                    // Lateral position anywhere between -1.5 and 1.5 since k_max_trailer_width = 3.0
      det_props[7].f_double_bounce = false;
      det_props[7].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[7].f_water_spray = false;

      // Setup ninth detection such that it can be assigned a column/bin number
      raw_dets.detections[8].raw.sensor_id = 1;                                   // Left rear sensor
      raw_dets.detections[8].raw.range_rate = 0.3F - F360_EPSILON;                //  Any value such that |range_rate| < 0.3 
      det_props[8].vcs_position.x = -6.2F;                                    // Random number between -17.5 and -5.5 since the limit are -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[8].vcs_position.y = -1.4F;                                    // Lateral position anywhere between -1.5 and 1.5 since k_max_trailer_width = 3.0
      det_props[8].f_double_bounce = false;
      det_props[8].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[8].f_water_spray = false;

      // Setup sensor calibration
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;       // Sensor with id 1 is valid for trailor detector
      sensors[1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;        // Sensor with id 2 is valid for trailor detector
      sensors[2].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR;      // Sensor with id 3 is valid for trailor detector
      sensors[3].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;     // Sensor with id 4 is not valid for trailor detector
      sensors[4].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;   // Sensor with id 5 is not valid for trailor detector
      sensors[5].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;    // Sensor with id 6 is not valid for trailor detector
      sensors[6].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE1;       // Sensor with id 7 is not valid for trailor detector
      sensors[7].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1;      // Sensor with id 8 is not valid for trailor detector

   }
};

/** \purpose  
 * Verify that function Process_Input() works as intended if all conditions are meet
 */
TEST(tw_process_input_Process_Input, Test_Assign_Dets_to_correct_bin)
{
   /** \precond
    * Use the first four detections with default set up. Host and detections has been initialized such that f_trailer_detection is true
    */
   raw_dets.number_of_valid_detections = 4U;

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that 
    * - Four detections have updated their respective bins correctly
    * - No other bins have been updated (default test set up for detection_col[i] is one)
    */
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);

   const uint8_t exp_first_dets_col_index = 0U;
   CHECK_EQUAL_TEXT(output_detection_col[exp_first_dets_col_index], 2U, "detection_col[0] is not update correctly");
   
   const uint8_t exp_second_dets_col_index = 15U;
   CHECK_EQUAL_TEXT(output_detection_col[exp_second_dets_col_index], 2U, "detection_col[15] is not update correctly");

   // The safe guard/ out of bounce scenario
   const uint8_t exp_third_dets_col_index = 30U;
   CHECK_EQUAL_TEXT(output_detection_col[exp_third_dets_col_index - 1], 2U, "detection_col[29] is not update correctly");
   
   const uint8_t exp_fourth_dets_col_index = 22U;
   CHECK_EQUAL_TEXT(output_detection_col[exp_fourth_dets_col_index], 2U, "detection_col[22] is not updated correctly");

   /** \Observe 
   * The code line 
   * col_index = col_index <= 0U ? 0U : col_index;
   * is not reachable with the currecnt code layout. 
   */

   // No other bin should be updated
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {  
         if (i != static_cast<uint8_t>(exp_first_dets_col_index) &&
             i != static_cast<uint8_t>(exp_second_dets_col_index) &&
             i != static_cast<uint8_t>(exp_third_dets_col_index - 1U) &&
             i != static_cast<uint8_t>(exp_fourth_dets_col_index))
         {  
            CHECK_EQUAL_TEXT(output_detection_col[i], 1U, "At least one bin has incorrectly been updated");
         }
      }

}

/** \purpose  
 * Check that detections with invalid properties do not update bins
 */
TEST(tw_process_input_Process_Input, Test_Dont_Update_blocked_by_detection_props)
{
   /** \precond
   * The default detection settings passes the variable f_trailer_detection as true. To verify if the Process_Input() 
   * setup is correct, assign each of the nine detections with one property such that f_trailer_detection becomes false. 
   */
   raw_dets.detections[0].raw.range_rate = 0.3F + F360_EPSILON;             // Choose any positive value such that |range_rate| > tw_calibs_set_val.k_ZRRateGate 
   raw_dets.detections[1].raw.range_rate = -(0.3F + F360_EPSILON);          // Choose any negative value such that |range_rate| > tw_calibs_set_val.k_ZRRateGate
   det_props[2].vcs_position.x = -5.0;                                  // Choose any value larger than -tw_calibs.k_host_length
   det_props[3].vcs_position.x = -18.5F;                                // Choose any value smaller than -(tw_calibs.k_host_length + tw_calibs.k_max_trailer_length)
   det_props[4].vcs_position.y = 1.5F + F360_EPSILON;                   // Choose any positive value failing |1.5F| <= half_width
   det_props[5].vcs_position.y = -(1.5F + F360_EPSILON);                // Choose any negative value failing |1.5F| <= half_width
   det_props[6].f_double_bounce = true;                                 // Default false
   det_props[7].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_NEARBY; // Choose any enum except F360_DETECTION_WHEELSPIN_TYPE_INVALID
   det_props[8].f_water_spray = true;                                   // Default false

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no bin has been updated
    */
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);

   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {  
      CHECK_EQUAL_TEXT(output_detection_col[i], 1U, "At least one bin has incorrectly been updated");
   }
}

/** \purpose  
 * Verify that nothing happens if detections comes from sensors invalid for trailer detection
 */
TEST(tw_process_input_Process_Input, Test_Incorrect_sensor_mounting)
{
   /** \precond
   * To verify if the Process_Input() setup is correct, assign three detections with a sensor mounting property such 
   * the if-check for mounting location never enters. 
   */
   raw_dets.number_of_valid_detections = 3U;

   raw_dets.detections[0].raw.sensor_id = 5;        // Right forward
   raw_dets.detections[1].raw.sensor_id = 6;        // Left side
   raw_dets.detections[2].raw.sensor_id = 7;        // RIght side

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no bin has been updated
    */
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);

   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {  
      CHECK_EQUAL_TEXT(output_detection_col[i], 1U, "At least one bin has incorrectly been updated");
   }
}

/** \purpose  
 *  Check that nothing happens if no detections are valid to use
 */
TEST(tw_process_input_Process_Input, Test_No_detections_ok_to_use)
{
   /** \precond
   * Assign the number of valid detections to use to zero. 
   * Check that no bins are updated if the number of detetions okey to use are zero.
   */
   raw_dets.number_of_valid_detections = 0U;

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no bin has been updated
    */
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);

   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {  
      CHECK_EQUAL_TEXT(output_detection_col[i], 1U, "At least one bin has incorrectly been updated");
   }
}

/** \purpose  
 *  In an event when host is driving fast enough and the window timer is too high nothing should happen
 */
TEST(tw_process_input_Process_Input, Test_Window_timer_too_high)
{
   /** \precond
   * Default setting, except the window timer. 
   */
   window_timer_set_val = 850U;
   trailer_detector_TW.set_window_timer(window_timer_set_val);    // Choose any value larger than k_window_timer_threshold = 800

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that no bin has been updated
    */
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);

   for (uint32_t i = 0U; i < COL_NUMBER; i++)
      {  
         CHECK_EQUAL_TEXT(output_detection_col[i], 1U, "At least one bin has incorrectly been updated");
      }
}

/** \purpose  
 * Verify reset timer logic
 */
TEST(tw_process_input_Process_Input, Test_Clear_data_when_host_drives_too_slow)
{
   /** \precond
   * Default setting, except
   * - Assign host a new speed < host speed threshold
   * - Assign reset timer = reset timer threshold
   */
   vehicle_data.speed = 0.001F;                              // Choose any speed lower than k_speed_threshold = 0.2 
   reset_timer_set_val = tw_calibs.k_reset_time_threshold;   // Choose this value to reset the detection calculations in width direction
   trailer_detector_TW.set_reset_timer(reset_timer_set_val);

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that the class members (except for reset_timer) have been cleared (set to default values) correctly.
    */
   CHECK_FALSE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is not cleared correctly");	

   const float32_t exp_trailer_width = 0.0F;
   DOUBLES_EQUAL_TEXT(exp_trailer_width, trailer_detector_TW.get_trailer_width(), test_pass_th_small, "trailer_width is not cleared correctly");

   const Trailer_Detector_Conf exp_trailer_width_conf = TRAILER_DETECTOR_CONF_UNKNOWN;
   CHECK_EQUAL_TEXT(exp_trailer_width_conf, trailer_detector_TW.get_trailer_width_conf(), "trailer_width_conf is not cleared correctly");
   
   const uint32_t exp_window_timer = 0U;
   CHECK_EQUAL_TEXT(exp_window_timer, trailer_detector_TW.get_window_timer(), "window_timer is not cleared correctly");
   
   uint32_t output_detection_col[COL_NUMBER];
   trailer_detector_TW.get_detection_col(output_detection_col);
   for (uint32_t i = 0U; i < COL_NUMBER; i++)
   {
      const uint32_t exp_detection_col = 0U;
      CHECK_EQUAL_TEXT(exp_detection_col, output_detection_col[i], "detection_col is not cleared correctly");
   }
}

/** \purpose  
 * Verify reset timer logic
 */
TEST(tw_process_input_Process_Input, Test_Not_Clear_data_when_host_drives_too_slow_but_reset_time_small)
{
   /** \precond
   * Default setting, except
   * - Assign host a new speed < host speed threshold
   * - Assign reset timer = reset timer threshold
   */
   vehicle_data.speed = 0.001F;                              // Choose any speed lower than k_speed_threshold = 0.2 
   reset_timer_set_val = 17U;   // Choose this value to reset the detection calculations in width direction
   trailer_detector_TW.set_reset_timer(reset_timer_set_val);

   /** \action
    * Run Process_Input() function
    */
   trailer_detector_TW.process_input_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that the class members (except for reset_timer) have been cleared (set to default values) correctly.
    */
   CHECK_TRUE_TEXT(trailer_detector_TW.get_f_estimation_done(), "f_estimation_done is not cleared correctly");	
}
/** @}*/
