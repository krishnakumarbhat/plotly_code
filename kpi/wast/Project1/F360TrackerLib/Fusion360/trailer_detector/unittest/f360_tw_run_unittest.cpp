/** \file
 * This file contains unit tests for content of tw_run.cpp file
 */

#include "f360_trailer_detector_TW.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

class Trailer_Detector_TWmock : public Trailer_Detector_TW
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

   void process_input_mock(
      const F360_Host_T&vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Process_Input(vehicle_data, raw_detect_list, all_detections, sensors);
   }

   void run_trailer_detector_mock(const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Trailer_Detector_TW::Run_Trailer_Detector(vehicle_data, raw_detect_list, all_detections, sensors);
   }

};

/** \defgroup  tw_run
 *  @{
 */

/** \brief
 * Test group to test that Trailer_Detector_TW::Run_Trailer_Detector works as intended.
 */
TEST_GROUP(tw_run)
{	
   Trailer_Detector_TWmock trailer_detector_TW;
   F360_Host_T vehicle_data;
   rspp_variant_A::RSPP_Detection_List_T raw_dets;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   
   /** \setup
    * Initialize a trailer_detector_tw object
    * Set host speed to above lower limit for estimation
    * Set up 5 detections that are valid for trailer width update
    */
   TEST_SETUP()
   {
      trailer_detector_TW.Initialize();
      trailer_detector_TW.set_window_timer(799U);
      vehicle_data.speed = 5.0F;

      // Initialize detection data
      raw_dets.number_of_valid_detections = 5U;//9U;

      // Setup first detection such that it can be assigned a column/bin number
      raw_dets.detections[0].raw.sensor_id = 2;                                       // Left rear sensor
      raw_dets.detections[0].raw.range_rate = -(0.2F + F360_EPSILON);                 // Set such that it is possible to test the range rates absolut value condition in negative direction
      det_props[0].vcs_position.x = -6.0F;                                       // Random number between -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[0].vcs_position.y = -1.5F; // Lateral position in the trailers left edge 
      det_props[0].f_double_bounce = false;
      det_props[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[0].f_water_spray = false;
      
      // Setup second detection such that it can be assigned a column/bin number
      raw_dets.detections[1].raw.sensor_id = 2;                                      // Center rear sensor
      raw_dets.detections[1].raw.range_rate = (0.2F + F360_EPSILON);                 // Set such that it is possible to test the range rates absolut value condition in positive direction          
      det_props[1].vcs_position.x = -6.0F;                                       // Random number between -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[1].vcs_position.y = -0.4F;                                        // Lateral position anywhere between -1.5 and 1.5 
      det_props[1].f_double_bounce = false;
      det_props[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[1].f_water_spray = false;

      // Setup third detection such that it can be assigned a column/bin number
      raw_dets.detections[2].raw.sensor_id = 2;                                      // Right rear sensor
      raw_dets.detections[2].raw.range_rate = 0.2F;                                  // Set such that it is possible to test the range rates absolut value condition in positive direction          
      det_props[2].vcs_position.x = -6.0F;                                      // Random number between -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[2].vcs_position.y = -0.39F; // Lateral position in the trailers right edge 
      det_props[2].f_double_bounce = false;
      det_props[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[2].f_water_spray = false;
      
      // Setup fourth detection such that it can be assigned a column/bin number
      raw_dets.detections[3].raw.sensor_id = 2;                                      // Center rear sensor
      raw_dets.detections[3].raw.range_rate = 0.2F;                                  // Any range rate approximately close to the other detections range rate
      det_props[3].vcs_position.x = -6.0F;                                       // Random number between -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[3].vcs_position.y = 1.0F;                         // Lateral position anywhere between -1.5 and 1.5 
      det_props[3].f_double_bounce = false;
      det_props[3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[3].f_water_spray = false;


      // Setup fifth detection such that it can be assigned a column/bin number
      raw_dets.detections[4].raw.sensor_id = 2;                                      // Center rear sensor
      raw_dets.detections[4].raw.range_rate = 0.2F;                                  // Any range rate approximately close to the other detections range rate
      det_props[4].vcs_position.x = -6.0F;                                      // Random number between -(k_host_length + k_max_trailer_length) and -k_host_length 
      det_props[4].vcs_position.y = 1.5F;                                       // Lateral position anywhere between -1.5 and 1.5 
      det_props[4].f_double_bounce = false;
      det_props[4].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
      det_props[4].f_water_spray = false;

      // Setup sensor calibration
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;       // Sensor with id 1 is valid for trailor detector
      sensors[1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;        // Sensor with id 2 is valid for trailor detector
      sensors[2].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR;      // Sensor with id 3 is valid for trailor detector
      sensors[3].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;     // Sensor with id 4 is not valid for trailor detector
      sensors[4].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;   // Sensor with id 5 is not valid for trailor detector
      sensors[5].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;    // Sensor with id 6 is not valid for trailor detector
      sensors[6].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE1;       // Sensor with id 7 is not valid for trailor detector
      sensors[7].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1;      // Sensor with id 8 is not valid for trailor detector
      sensors[7].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1;      // Sensor with id 8 is not valid for trailor detector
   }
};

/** \purpose  
 * Test that the trailer width is updated when the window_timer hits 800 and there are valid detections.
 */
TEST(tw_run, FunctionToTest_Descriptive_Tag)
{
   /** \precond
    * A trailer_detector_tw object is set up
    * 5 valid detections are set up
    * Set trailer width to some initial value.
    * Set trailer width conf to low.
    */
   float32_t initial_trailer_width = 0.1F;
   trailer_detector_TW.set_trailer_width(initial_trailer_width);

   Trailer_Detector_Conf initial_trailer_width_conf = TRAILER_DETECTOR_CONF_LOW;
   trailer_detector_TW.set_trailer_width_conf(initial_trailer_width_conf);
	
   /** \action
    * Call Trailer_Detector_TW::Run_Trailer_Detector() from the mocked object.
    */
   trailer_detector_TW.run_trailer_detector_mock(vehicle_data, raw_dets, det_props, sensors);

   /** \result
    * Check that the updated trailer width is not same as the initial value.
    */
   CHECK_FALSE_TEXT(initial_trailer_width_conf == trailer_detector_TW.get_trailer_width_conf(), "Trailer width conf has not been updated.");
   CHECK_FALSE_TEXT(std::fabs(initial_trailer_width - trailer_detector_TW.get_trailer_width()) < F360_EPSILON, "Trailer width has not been updated.");
}

/** @}*/

/** \defgroup  tw_get_output
 *  @{
 */

/** \brief
 * Test group to test that Trailer_Detector_TW::Get_Output works as intended.
 */
TEST_GROUP(tw_get_output)
{	
   Trailer_Detector_TWmock trailer_detector_TW;
   const float32_t test_pass_th = 1e-6F;
   
   /** \setup
    * Initialize a TrailerDetectorMock object and assign some values
    */
   TEST_SETUP()
   {
      trailer_detector_TW.Initialize();
      trailer_detector_TW.set_trailer_width(2.0F);
      trailer_detector_TW.set_trailer_width_conf(TRAILER_DETECTOR_CONF_HIGH);
      trailer_detector_TW.set_f_estimation_done(true);
   }
};

/** \purpose  
 * Test that Get_Output returns the correct signals
 */
TEST(tw_get_output, Test_Get_Output)
{
   /** \precond
    * A trailer_detector_tw object is set up with
    * - width = 2m
    * - width confidence = high
    * - estimation_done = true
    */
	TW_Output output_struct = {};
   const float32_t exp_output_width = trailer_detector_TW.get_trailer_width();
   const Trailer_Detector_Conf exp_width_conf = trailer_detector_TW.get_trailer_width_conf();
   const bool exp_f_estimation_done = trailer_detector_TW.get_f_estimation_done();

   /** \action
    * Call Trailer_Detector_TW::Get_Output() from the mocked object.
    */
   trailer_detector_TW.Get_Output(output_struct);

   /** \result
    * Check that the updated trailer width is not same as the initial value.
    */
   CHECK_EQUAL_TEXT(exp_f_estimation_done, output_struct.f_estimation_done, "Output f_estimation_done not correct.");
   CHECK_EQUAL_TEXT(exp_width_conf, output_struct.confidence, "Output width confidence not correct.");
   DOUBLES_EQUAL_TEXT(exp_output_width, output_struct.trailer_width, test_pass_th, "Output trailer width not correct.");
}

/** @}*/
