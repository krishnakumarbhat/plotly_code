/** \file
   This file contains test for functions in f360_mark_trailer_detections.
*/

#include "f360_mark_object_tracks_next_to_sensors.h"
#include <CppUTest/TestHarness.h>
#include "f360_constants.h"
#include "f360_sensor_type.h"
#include "f360_set_variant.h"
#include "f360_mark_trailer_detections.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"

using namespace f360_variant_A;

/** \defgroup  f360_mark_trailer_detections
 *  @{
 */
/** \brief
*  This test group contains tests for mark_trailer_detections
*  function.
**/
TEST_GROUP(f360_mark_trailer_detections)
{
   rspp_variant_A::RSPP_Detection_List_T raw_det_list{};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Host_T host = {};
   RSPP_Calibrations_T rspp_calibs = {};
   Trailer_Detector_Flt_Fus_Output trailer_detector_output = {};

   /** \setup
    * Default setup will change f_ok_to_use and f_det_on_trailer after function call
    **/
   TEST_SETUP()
   {

      // Initialize calibrations
      Initialize_RSPP_Calibrations(rspp_calibs);

      // Initialize host
      host.dist_rear_axle_to_vcs_m = 3.7F;

      // Initialize detections
      raw_det_list.number_of_valid_detections = 5U;

      const float32_t distance_rear_axle_to_tow_hitch = 1.2F;    // distance between rear axle to to hitch (m)
      raw_det_list.detections[0].processed.vcs_position_x = -(host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch);
      det_props[0].f_ok_to_use = true;
      det_props[0].f_det_on_trailer = false;
      det_props[0].vcs_position.x = -(host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch);
      det_props[0].vcs_position.y = 0.0F;

      raw_det_list.detections[1].processed.vcs_position_x = -(host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch) - 1.0F;
      det_props[1].f_ok_to_use = true;
      det_props[1].f_det_on_trailer = false;
      det_props[1].vcs_position.x = -(host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch) - 1.0F;
      det_props[1].vcs_position.y = 0.0F;

      raw_det_list.detections[2].processed.vcs_position_x = 10.0F;
      det_props[2].f_ok_to_use = true;
      det_props[2].f_det_on_trailer = false;
      det_props[2].vcs_position.x = 10.0F;
      det_props[2].vcs_position.y = 0.0F;

      raw_det_list.detections[3].processed.vcs_position_x = -20.0F;
      det_props[3].f_ok_to_use = true;
      det_props[3].vcs_position.x = -20.0F;
      det_props[3].vcs_position.y = 0.0F;

      raw_det_list.detections[4].processed.vcs_position_x = -10.0F;
      det_props[4].f_ok_to_use = true;
      det_props[4].vcs_position.x = -10.0F;
      det_props[4].vcs_position.y = 1.6F;


      // Initialize detection sorted info (Done in Clear_Detection() function call in production code)
      for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      {
         raw_det_list.detections[det_idx].processed.prev_sorted_idx = F360_INVALID_ID;
         raw_det_list.detections[det_idx].processed.next_sorted_idx = F360_INVALID_ID;
      }

      Sort_Detections_Vcs_Long(rspp_calibs, raw_det_list);


      trailer_detector_output.trailer_presence = TRAILER_PRESENCE_STATE_DETECTED;
      trailer_detector_output.trailer_width = 2.0F;
      trailer_detector_output.trailer_length = 8.0F;

   }
};

/**
*\purpose
* The purpose of the test is to verify if the detections
* are classified to be detections on trailer when the trailer condition satified.
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer)
{
   /** \precond
   * Use default settings
   **/

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_useis set to expected value.
   **/
   CHECK_TRUE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_FALSE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_TRUE_TEXT(det_props[1].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_FALSE_TEXT(det_props[1].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[2].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[2].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[3].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[3].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[4].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[4].f_ok_to_use , "f_ok_to_use not set right");

}

/**
*\purpose
* The purpose of the test is to verify the flag f_det_on_trailer and f_ok_to_use won't
* be updated when trailer length is 0.
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer_trailer_length_0)
{
   /** \precond
   * Use default settings
   **/
   trailer_detector_output.trailer_length = 0.0F;

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_use is not updated
   **/
   CHECK_FALSE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[1].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[1].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[2].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[2].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[3].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[3].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[4].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[4].f_ok_to_use , "f_ok_to_use not set right");

}

/**
*\purpose
* The purpose of the test is to verify the flag f_det_on_trailer and f_ok_to_use won't
* be updated when trailer not detected.
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer_trailer_not_detected)
{
   /** \precond
   * Use default settings
   **/
   trailer_detector_output.trailer_presence = TRAILER_PRESENCE_STATE_NOT_DETECTED;

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_use is not updated
   **/
   CHECK_FALSE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[1].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[1].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[2].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[2].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[3].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[3].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[4].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[4].f_ok_to_use , "f_ok_to_use not set right");

}

/**
*\purpose
* The purpose of the test is to verify the flag f_det_on_trailer and f_ok_to_use won't
* be updated when trailer width is 0.
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer_trailer_width_0)
{
   /** \precond
   * Use default settings
   **/
   trailer_detector_output.trailer_width = 0.0F;

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_use is not updated
   **/
   CHECK_FALSE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[1].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[1].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[2].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[2].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[3].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[3].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[4].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[4].f_ok_to_use , "f_ok_to_use not set right");

}

/**
*\purpose
* The purpose of the test is to verify the flag f_det_on_trailer and f_ok_to_use will
* be updated to the correct status when all the detection are behind rear axle
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer_all_detections_behind_rear_axle)
{
   /** \precond
   * Use default settings
   **/
   raw_det_list.detections[2].processed.vcs_position_x = -10.0F;
   det_props[2].f_ok_to_use = true;
   det_props[2].f_det_on_trailer = false;
   det_props[2].vcs_position.x = -10.0F;
   det_props[2].vcs_position.y = 0.0F;

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_useis set to expected value.
   **/
   CHECK_TRUE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_FALSE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_TRUE_TEXT(det_props[1].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_FALSE_TEXT(det_props[1].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_TRUE_TEXT(det_props[2].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_FALSE_TEXT(det_props[2].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[3].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[3].f_ok_to_use , "f_ok_to_use not set right");
   CHECK_FALSE_TEXT(det_props[4].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[4].f_ok_to_use , "f_ok_to_use not set right");

}

/**
*\purpose
* The purpose of the test is to verify the flag f_det_on_trailer and f_ok_to_use will
* be updated to the correct status when there is only one detection.
*/
TEST(f360_mark_trailer_detections, detect_det_on_trailer_single_detection)
{
   /** \precond
   * Use default settings
   **/
   raw_det_list.number_of_valid_detections = 1U;

   /** \action
   * Execute the function
   **/
   Detect_Det_On_Trailer(host, raw_det_list, trailer_detector_output, det_props);

   /** \result
   * Check that f_det_on_trailer and f_ok_to_useis set to expected value.
   **/
   CHECK_FALSE_TEXT(det_props[0].f_det_on_trailer , "f_det_on_trailer not set right");
   CHECK_TRUE_TEXT(det_props[0].f_ok_to_use , "f_ok_to_use not set right");

}
