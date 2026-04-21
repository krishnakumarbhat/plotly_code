/** \file
 * This file contains unit tests for content of tp_run.cpp file
 */

#include "f360_trailer_detector_TP.h"
#include <CppUTest/TestHarness.h>

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock Trailer_Detector_TP in order to be able to access private members to make unit testing easier
class Trailer_Detector_TPmock : public Trailer_Detector_TP
{
   public:

   void set_trailer_presence(Trailer_Presence_State trailer_presence_set_val)
   {
      trailer_presence = trailer_presence_set_val;
   }

   void set_trailer_presence_conf(Trailer_Detector_Conf trailer_presence_conf_set_val)
   {
      trailer_presence_conf = trailer_presence_conf_set_val;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   void set_window_timer(int16_t window_timer_set_val)
   {
      window_timer = window_timer_set_val;
   }

   void set_stationary_timer(uint32_t stationary_timer_set_val)
   {
      stationary_timer = stationary_timer_set_val;
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
};

/** \defgroup  tp_run
 *  @{
 */

/** \brief
 * This test group checks that Trailer_Detector_TP::Run_Trailer_Detector() works as intended
 */
TEST_GROUP(tp_run)
{	
    // Create an instance of the Trailer_Detector_TP class
   Trailer_Detector_TPmock trailer_detector_TP;

   // Get a local copy of the private calibration struct
   const TP_Cals tp_calibs = trailer_detector_TP.get_tp_calibs();

   // Create variables for default test values of member variables
   Trailer_Presence_State trailer_presence_set_val;
   Trailer_Detector_Conf trailer_presence_conf_set_val;
   bool f_estimation_done_set_val;
   int16_t window_timer_set_val;
   uint32_t stationary_timer_set_val;
   TP_Relative_Ratio relative_ratio_struct_set_val;
   TP_Mean_RR mean_rr_struct_set_val;

   // Create a host data structure
   F360_Host_T vehicle_data;

   // Create detection data structures
   rspp_variant_A::RSPP_Detection_List_T raw_dets;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];

   // Create a sensor data structure
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with data such that the Run_Trailer_Detector() function estimates
    * that there is trailer present with highest confidence after getting one more valid trailer detection but that it 
    * estimates no trailer is present with highest confidence if there is no more valid trailer detection added.
    */
   TEST_SETUP()
   {
      // Initialize Trailer_Detector_TP class instance with default values
      trailer_detector_TP.Initialize();

      // Initialize member variables such that there is one detection to little before we can detect that there is a trailer present with highest confidence
      trailer_presence_set_val = TRAILER_PRESENCE_STATE_UNKNOWN;
      trailer_detector_TP.set_trailer_presence(trailer_presence_set_val); // Setup trailer_presence to indicate that we don't yet know if there is trailer or not

      trailer_presence_conf_set_val = TRAILER_DETECTOR_CONF_LOW;
      trailer_detector_TP.set_trailer_presence_conf(trailer_presence_conf_set_val); // Setup to not be HIGH

      window_timer_set_val = tp_calibs.k_time_threshold - 1;
      trailer_detector_TP.set_window_timer(tp_calibs.k_time_threshold - 1); // Setup window_timer to enable such that there needs to be one more scan with new detection information before we can do trailer estimation
      
      // Set up relative_ratio_struct_set_val to enable the decision that there is a trailer with highest confidence after geting one more detection (i.e number of detection is less than 11 but rr ratio of dets_n_02 and dets_n_02 > 0.9)
      relative_ratio_struct_set_val.dets_n_02 =  5;
      relative_ratio_struct_set_val.dets_n_03 =  5;
      relative_ratio_struct_set_val.dets_n_04 =  0;
      relative_ratio_struct_set_val.dets_n_05 =  0;
      trailer_detector_TP.set_relative_ratio_struct(relative_ratio_struct_set_val);

     // Set up mean_rr_struct to enable the decision that there is a trailer with highest confidence after geting one more detection (i.e number of detection is less than 11 but mean rr ratio is smaller than tp_calibs.k_rr_mean_threshold)
      mean_rr_struct_set_val = trailer_detector_TP.get_mean_rr_struct(); // To extract the value of mean_rr_struct.value_array after initialzation;
      mean_rr_struct_set_val.num_array[0] = 2;
      mean_rr_struct_set_val.num_array[1] = 3;
      mean_rr_struct_set_val.num_array[2] = 5;
      mean_rr_struct_set_val.num_array[3] = 0;
      mean_rr_struct_set_val.num_array[4] = 0;
      mean_rr_struct_set_val.num_array[29] = 0;
      trailer_detector_TP.set_mean_rr_struct(mean_rr_struct_set_val);

      f_estimation_done_set_val = false;
      trailer_detector_TP.set_f_estimation_done(false); // Set to estimation not done

      
      stationary_timer_set_val = 5U; // Set be larger than 0 but to be small enough to disable clearing of data due to host being stationary for too long
      trailer_detector_TP.set_stationary_timer(stationary_timer_set_val); 

      // Initialize host data
      vehicle_data.speed = 0.5F * (tp_calibs.k_speed_threshold + tp_calibs.k_speed_threshold_clear); // Set to be in between tp_cals.k_speed_threshold and k_speed_threshold_clear such that detections will be used during process input but also such that the stationary_timer is increasing

      // Setup sensor calibration
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR; // Sensor with id 1 is valid for trailor detector

      // Initialize detection data. Setup one detection that is valid to be added to the aggregated data used for trailer presence estimation
      raw_dets.number_of_valid_detections = 1U;

      // Setup detection such that it is valid to update relative_ratio_struct.dets_n_02 and mean_rr_struct.num_array[1]
      raw_dets.detections[0].raw.sensor_id = 1;
      raw_dets.detections[0].raw.range_rate = -(0.2F - F360_EPSILON); // Will update relative_ratio_struct.dets_n_02 and mean_rr_struct.num_array[1]
      det_props[0].f_double_bounce = false;
      det_props[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[0].f_water_spray = false;
      det_props[0].vcs_position.x = tp_calibs.k_long_max - F360_EPSILON;
      det_props[0].vcs_position.y = tp_calibs.k_lat_abs - F360_EPSILON;
   }
};

/** \purpose  
 * Verify that Trailer_Detector_TP::Run_Trailer_Detector() estimates that there is trailer present with highest confidence after getting one more valid trailer detection.
 * \req
 * NA
 */
TEST(tp_run, Test_Trailer_Presence_Estimated_With_Highest_Conf_When_One_New_Detection)
{
   /** \precond
    * Use default data from test setup
    */
	
   /** \action
    * Call Run_Trailer_Detector()
    * Extract the output from the function with help of Get_Output() function
    */
   trailer_detector_TP.Run_Trailer_Detector(vehicle_data, raw_dets, det_props, sensors);
   TP_Output tp_output_values;
   tp_output_values.trailer_presence = TRAILER_PRESENCE_STATE_UNKNOWN; // Set to a value that we are not expecting after function call
   tp_output_values.confidence = TRAILER_DETECTOR_CONF_LOW; // Set to a value that we are not expecting after function call
   tp_output_values.f_estimation_done = true; // Set to a value that we are not expecting
   tp_output_values.stationary_timer = 100U; // Set to a value that we are not expecting
   trailer_detector_TP.Get_Output(tp_output_values);


   /** \result
    * Check that
    * trailer_presence is DETECTED
    * confidence is HIGH
    * f_estimation_done is true
    * stationary_timer is 0 (i.e it has been cleared)
    * min_abs_speed_running is tp_calibs.k_speed_threshold (i.e. is unchanged)
    */
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_DETECTED, tp_output_values.trailer_presence, "trailer_presence has an unexpected value")	
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, tp_output_values.confidence, "confidence has an unexpected value")	
   CHECK_TRUE_TEXT(tp_output_values.f_estimation_done, "f_estimation_done has an unexpected value")	
   CHECK_EQUAL_TEXT(0U, tp_output_values.stationary_timer, "stationary_timer has an unexpected value")
}

/** \purpose  
 * Verify that Trailer_Detector_TP::Run_Trailer_Detector() estimates that there is no trailer present with highest confidence when not getting any additional valid trailer detection.
 * \req
 * NA
 */
TEST(tp_run, Test_Trailer_NOT_Presence_Estimated_With_Highest_Conf_When_No_New_Detection)
{
   /** \precond
    * Use default data from test setup except set number of detection to 0
    */
   raw_dets.number_of_valid_detections = 0U;
	
   /** \action
    * Call Run_Trailer_Detector()
    * Extract the output from the function with help of Get_Output() function
    */
   trailer_detector_TP.Run_Trailer_Detector(vehicle_data, raw_dets, det_props, sensors);
   TP_Output tp_output_values;
   tp_output_values.trailer_presence = TRAILER_PRESENCE_STATE_DETECTED; // Set to a value that we are not expecting efter function call
   tp_output_values.confidence = TRAILER_DETECTOR_CONF_LOW; // Set to a value that we are not expecting after function call
   tp_output_values.f_estimation_done = false; // Set to a value that we are not expecting
   tp_output_values.stationary_timer = 100U; // Set to a value that we are not expecting
   trailer_detector_TP.Get_Output(tp_output_values);


   /** \result
    * Check that
    * f_trailer_presence is NOT_DETECTED
    * confidence is HIGH
    * f_estimation_done is false
    * stationary_timer is 0 (i.e it has been cleared)
    * min_abs_speed_running is tp_calibs.k_speed_threshold (i.e. is unchanged)
    */
   CHECK_EQUAL_TEXT(TRAILER_PRESENCE_STATE_NOT_DETECTED, tp_output_values.trailer_presence, "trailer_presence has an unexpected value")	
   CHECK_EQUAL_TEXT(TRAILER_DETECTOR_CONF_HIGH, tp_output_values.confidence, "confidence has an unexpected value")	
   CHECK_FALSE_TEXT(tp_output_values.f_estimation_done, "f_estimation_done has an unexpected value")	
   CHECK_EQUAL_TEXT(0U, tp_output_values.stationary_timer, "stationary_timer has an unexpected value")
}
/** @}*/