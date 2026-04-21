/** \file
 * This file contains unit tests for content of tl_run.cpp file
 */

#include "f360_trailer_detector_TL.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

// Mock Trailer_Detector_TL in order to be able to access private members to make unit testing easier
class TrailerDetectorTLmock : public Trailer_Detector_TL
{
   public:

   void set_reset_timer(uint32_t set_reset_timer_val)
   {
      reset_timer = set_reset_timer_val;
   }

   void set_window_timer(uint32_t windor_timer_set_val)
   {
      window_timer = windor_timer_set_val;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   TL_Trailer_Detection_Row get_trailer_detection_row_struct()
   {
      return trailer_detection_row_struct;
   }

   void call_run_trailer_detector(
      const F360_Host_T &vehicle_data,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS])
   {
      Trailer_Detector_TL::Run_Trailer_Detector(vehicle_data, raw_detect_list, all_detections, sensors);
      TL_Output output;
      Trailer_Detector_TL::Get_Output(output);
   }
};

/** \defgroup  tl_run
 *  @{
 */

/** \brief
 * Test that Trailer_Detector_TL::Run_Trailer_Detector works correctly.
 */
TEST_GROUP(f360_tl_run)
{	
   TrailerDetectorTLmock trailer_detector_TL;

   F360_Host_T vehicle_data = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Detection_Props_T all_detections[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};

   float32_t initial_length = 0.0F;
   float32_t initial_cal_speed_thresh = 0.0F;

   /** \setup
    * Set up an instance of the mocked Trailer_Detector_TL, host speed to 3 m/s and 2 detections, valid for trailer length estimation.
    */
   TEST_SETUP()
   {
      trailer_detector_TL.Initialize();
      trailer_detector_TL.set_window_timer(1199);

      vehicle_data.speed = 3.0F;

      raw_detect_list.number_of_valid_detections = 2U;

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
   }
};

/** \purpose  
 * Test that all subfunctions in Run_Trailer_Detector are called by checking that some values are modified inside them. 
 */
TEST(f360_tl_run, Test_run_trailer_detector)
{
   /** \precond
    * trailer_detector_TL has been created with all calibration values set to 0 (done in constructor).
    * 2 valid trailer detections are set up and host speed is set to 3 m/s
    */
	
   /** \action
    * Call Trailer_Detector_TL::Run_Trailer_Detector().
    */
   trailer_detector_TL.call_run_trailer_detector(vehicle_data, raw_detect_list, all_detections, sensors);
   TL_Output output;
   trailer_detector_TL.Get_Output(output);

   /** \result
    * Check that trailer detector signals are changed from their initialized values after the function call.
    */
   CHECK_FALSE_TEXT(initial_length == output.trailer_length, "Trailer length was not updated.");

   // Check that all elements are not 0.
   int32_t detection_row_element_sum = 0;
   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      detection_row_element_sum += trailer_detector_TL.get_trailer_detection_row_struct().detection_row[i];
   }
   CHECK_FALSE_TEXT(0 == detection_row_element_sum, "detection_row was not updated.")
}

/** @}*/
