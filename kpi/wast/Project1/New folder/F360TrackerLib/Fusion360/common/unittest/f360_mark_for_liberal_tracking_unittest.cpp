/** \file
 * This file contains unit tests for content of f360_mark_for_liberal_tracking.cpp file
 */

#include "f360_mark_for_liberal_tracking.h"
#include "f360_clear_detections_props.h"
#include "f360_clear_object_track.h"
#include "f360_set_variant.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  Check_If_Point_Inside_Trapezoid_Zone_rear_case
 *  @{
 */

/** \brief
 * This test group tests Check_If_Point_Inside_Trapezoid_Zone function. Only rear case is tested - bottom edge of zone is longer than top edge.
 */
TEST_GROUP(Check_If_Point_Inside_Trapezoid_Zone_rear_case)
{
   // Declare common variables used within all tests in this test group.
   const float32_t trapezoid_top_lon_pos = 0.0f;
   const float32_t trapezoid_top_len = 2.0f;
   const float32_t trapezoid_bot_lon_pos = -5.0f;
   const float32_t trapezoid_bot_len = 12.0f;
};

/** \purpose  
 * Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is above trapezoid zone.
 * \req NA
 */
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_above_zone)
{
   /** \precond
    * Set point position to be above trapezoid zone.
    */
   const float32_t point_x = 10.0f;
   const float32_t point_y = 5.0f;

   /** \action
    * Call Check_If_Point_Inside_Trapezoid_Zone().
    */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
    * Point should be outside of defined zone.
    */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is below trapezoid zone.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_below_zone)
{
   /** \precond
   * Set point position to be below trapezoid zone.
   */
   const float32_t point_x = -10.0f;
   const float32_t point_y = 5.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is far outside on the left of trapezoid zone.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_on_the_left_far)
{
   /** \precond
   * Set point position to be on the left of trapezoid zone.
   */
   const float32_t point_x = -2.0f;
   const float32_t point_y = -10.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is far outside on the right of trapezoid zone.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_on_the_right_far)
{
   /** \precond
   * Set point position to be on the right of trapezoid zone.
   */
   const float32_t point_x = -2.0f;
   const float32_t point_y = 10.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is outside on the left of trapezoid zone, but relativly close to left edge.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_on_the_left_close)
{
   /** \precond
   * Set point position to be on the left of trapezoid zone.
   */
   const float32_t point_x = -2.0f;
   const float32_t point_y = -4.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is outside on the right of trapezoid zone, but relativly close to right edge.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_on_the_right_close)
{
   /** \precond
   * Set point position to be on the right of trapezoid zone.
   */
   const float32_t point_x = -2.0f;
   const float32_t point_y = 3.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when point is inside trapezoid zone, on left half.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_point_inside_on_left_half)
{
   /** \precond
   * Set point position to be inside zone.
   */
   const float32_t point_x = -3.0f;
   const float32_t point_y = -2.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be inside of defined zone.
   */
   CHECK_TRUE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when top edge length is longer than bottom one and point is located inside zone.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_reversed_zone_point_inside)
{
   /** \precond
   * Set point position to be inside zone. Set top/bottom edges.
   */
   const float32_t point_x = -3.0f;
   const float32_t point_y = 2.0f;

   const float32_t trapezoid_top_len = 12.0f;
   const float32_t trapezoid_bot_len = 2.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be inside of defined zone.
   */
   CHECK_TRUE(is_inside_zone);
}

/** \purpose
* Test Check_If_Point_Inside_Trapezoid_Zone behavior for case when top edge length is longer than bottom one and point is located outside zone.
* \req NA
*/
TEST(Check_If_Point_Inside_Trapezoid_Zone_rear_case, Check_If_Point_Inside_Trapezoid_Zone_reversed_zone_point_ouside)
{
   /** \precond
   * Set point position to be inside zone. Set top/bottom edges.
   */
   const float32_t point_x = -4.0f;
   const float32_t point_y = -4.0f;

   const float32_t trapezoid_top_len = 12.0f;
   const float32_t trapezoid_bot_len = 2.0f;

   /** \action
   * Call Check_If_Point_Inside_Trapezoid_Zone().
   */
   bool is_inside_zone = Check_If_Point_Inside_Trapezoid_Zone(
      trapezoid_top_lon_pos,
      trapezoid_top_len,
      trapezoid_bot_lon_pos,
      trapezoid_bot_len,
      point_x,
      point_y);

   /** \result
   * Point should be outside of defined zone.
   */
   CHECK_FALSE(is_inside_zone);
}

TEST_GROUP(Mark_Detections_For_Liberal_Tracking_tests)
{
   // Declare common variables used within all tests in this test group.
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   
   F360_Detection_Props_T detections[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detections_list{};

   bool Check_If_Any_Det_Mark_For_Lib_Tracking(F360_Detection_Props_T(&detections)[MAX_NUMBER_OF_DETECTIONS])
   {
      bool f_any_det_marked_for_liberal_tracking = false;
      for (uint16_t det_index = 0U; det_index < MAX_NUMBER_OF_DETECTIONS; det_index++)
      {
         F360_Detection_Props_T &detection = detections[det_index];
         f_any_det_marked_for_liberal_tracking |= detection.f_valid_for_liberal_tracking;
      }
      return f_any_det_marked_for_liberal_tracking;
   }

   TEST_SETUP()
   {
      calibrations.liberal_tracking_min_host_speed = F360_KPH2MPS(50.0F);
      calibrations.liberal_tracking_dets_rrate_thr = -5.0F;
      calibrations.liberal_tracking_trapezoid_zone_top_lon_pos = -40.0F;
      calibrations.liberal_tracking_trapezoid_zone_top_len = 20.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_lon_pos = -120.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_len = 40.0F;
      Clear_Detections_Props(detections);
   }
};

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is below calibration threshold.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, host_speed_below_thr)
{
   /** \precond
   * Set host speed to be below threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed - 1.0F;

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of detections met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_not_moving_below_rrate_thr_in_host_front_f_stationary_bounce)
{
   /** \precond
   * Set host speed to be above threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection.f_stationary_bounce = true;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr + 0.1F;
   detection.vcs_position = {1.0F, 0.0F};

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of detections met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_moving_below_rrate_thr_in_host_front_f_stationary_bounce)
{
   /** \precond
   * Set host speed to be above threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = true;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr + 0.1F;
   detection.vcs_position = { 1.0F, 0.0F};

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of detections met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_moving_above_rrate_thr_in_host_front_f_stationary_bounce)
{
   /** \precond
   * Set host speed to be above threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = true;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 0.1F;
   detection.vcs_position = { 1.0F, 0.0F};

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of detections met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_moving_above_rrate_thr_in_host_front)
{
   /** \precond
   * Set host speed to be above threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = false;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 0.1F;
   detection.vcs_position = { 1.0F, 0.0F};

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of detections met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_moving_above_rrate_thr_in_host_back_f_stationary_bounce)
{
   /** \precond
   * Set host speed to be above threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = true;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 0.1F;
   detection.vcs_position = { -1.0F, 0.0F};

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when single detection mets pre-conditions for liberal tracking, but it's not in the zone.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_outside_of_zone_ok_to_use)
{
   /** \precond
   * Set host speed to be above threshold. Set detections properties to met pre-condintions. Set detection position to be outside of trapezoid zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = false;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 1.0F;
   detection.vcs_position = { -20.0F, 0.0F };
   detection.f_ok_to_use = true;

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when single detection mets pre-conditions for liberal tracking and it's inside the zone.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_inside_of_zone_ok_to_use)
{
   /** \precond
   * Set host speed to be above threshold. Set detections properties to met pre-condintions. Set detection position to inside of trapezoid zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = false;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 1.0F;
   detection.vcs_position = { -50.0F, 0.0F };
   detection.f_ok_to_use = true;

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * First detections should be marked as valid for liberal tracking.
   */
   CHECK_TRUE(detection.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when single detection mets pre-conditions for liberal tracking, but it's not in the zone.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_outside_of_zone_not_ok_to_use)
{
   /** \precond
   * Set host speed to be above threshold. Set detections properties to met pre-condintions. Set detection position to be outside of trapezoid zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = false;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 1.0F;
   detection.vcs_position = { -20.0F, 0.0F };
   detection.f_ok_to_use = false;

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Det_Mark_For_Lib_Tracking(detections));
}

/** \purpose
* Test Mark_Detections_For_Liberal_Tracking behavior for case when single detection mets pre-conditions for liberal tracking and it's inside the zone.
* \req NA
*/
TEST(Mark_Detections_For_Liberal_Tracking_tests, det_inside_of_zone_not_ok_to_use)
{
   /** \precond
   * Set host speed to be above threshold. Set detections properties to met pre-condintions. Set detection position to inside of trapezoid zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Detection_Props_T &detection = detections[0];
   raw_detections_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection.f_stationary_bounce = false;
   detection.range_rate_dealiased = calibrations.liberal_tracking_dets_rrate_thr - 1.0F;
   detection.vcs_position = { -50.0F, 0.0F };
   detection.f_ok_to_use = false;

   /** \action
   * Call Mark_Detections_For_Liberal_Tracking().
   */

   Mark_Detections_For_Liberal_Tracking(calibrations, host.speed, MAX_NUMBER_OF_DETECTIONS, raw_detections_list, detections);

   /** \result
   * First detections should be marked as valid for liberal tracking.
   */
   CHECK_TRUE(detection.f_valid_for_liberal_tracking);
}

TEST_GROUP(Mark_Objects_For_Liberal_Tracking_tests)
{
   // Declare common variables used within all tests in this test group.
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info = {};

   bool Check_If_Any_Object_Mark_For_Lib_Tracking(F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS])
   {
      bool any_obj_marked_for_liberal_tracking = false;
      for (uint16_t obj_index = 0U; obj_index < NUMBER_OF_OBJECT_TRACKS; obj_index++)
      {
         F360_Object_Track_T &object = objects[obj_index];
         any_obj_marked_for_liberal_tracking |= object.f_valid_for_liberal_tracking;
      }
      return any_obj_marked_for_liberal_tracking;
   }

   void Reset_Objects(F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS])
   {
      for (uint16_t obj_index = 0U; obj_index < NUMBER_OF_OBJECT_TRACKS; obj_index++)
      {
         F360_Object_Track_T &object = objects[obj_index];
         Clear_Object_Track(object);
      }
   }

   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
      calibrations.liberal_tracking_min_host_speed = 50.0F / 3.6F; // 50 km/h
      calibrations.liberal_tracking_dets_rrate_thr = -5.0;
      calibrations.liberal_tracking_trapezoid_zone_top_lon_pos = -40.0F;
      calibrations.liberal_tracking_trapezoid_zone_top_len = 20.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_lon_pos = -120.0F;
      calibrations.liberal_tracking_trapezoid_zone_bot_len = 40.0F;
      calibrations.liberal_tracking_obj_relative_velocity_lon_thr = 5.0F;
      calibrations.liberal_tracking_obj_heading_thr = F360_DEG2RAD(30.0F);
      calibrations.liberal_tracking_obj_relative_velocity_lon_thr = 5.0F;

      Reset_Objects(objects);
   }
};

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when host speed is below calibration threshold.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, host_speed_below_thr)
{
   /** \precond
   * Set host speed to be below threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed - 1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * None of detections should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Object_Mark_For_Lib_Tracking(objects));
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when host speed is above calibration threshold, but none of objects met conditions
* for liberal tracking.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, host_speed_above_thr)
{
   /** \precond
   * Set host speed to be below threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * None of objects should be marked as valid for liberal tracking.
   */
   CHECK_FALSE(Check_If_Any_Object_Mark_For_Lib_Tracking(objects));
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, but further conditions are not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, object_not_moving_and_in_ego_front)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = false;
   object.vcs_position.x = 1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, but further conditions are not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, object_moving_and_in_ego_front)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position.x = 1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, but further conditions are not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, object_not_moving_and_not_in_ego_front)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = false;
   object.vcs_position.x = -1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, 1st level of conditions are met, but 2nd level of conditions not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, rel_vel_not_ok_heading_not_ok)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position.x = -1.0F;

   object.vcs_velocity.longitudinal = host.vcs_speed + calibrations.liberal_tracking_obj_relative_velocity_lon_thr - 0.01F;
   object.vcs_heading = Angle{ calibrations.liberal_tracking_obj_heading_thr + 0.01F };

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, 1st level of conditions are met, but 2nd level of conditions not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, rel_vel_ok_heading_not_ok)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position.x = -1.0F;

   object.vcs_velocity.longitudinal = host.vcs_speed + calibrations.liberal_tracking_obj_relative_velocity_lon_thr + 0.01F;
   object.vcs_heading = Angle{ calibrations.liberal_tracking_obj_heading_thr + 0.01F };

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, 1st level of conditions are met, but 2nd level of conditions not met.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, rel_vel_not_ok_heading_ok)
{
   /** \precond
   * Set host speed to be above threshold. Set status of first object to be not invalid.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position.x = -1.0F;

   object.vcs_velocity.longitudinal = host.vcs_speed + calibrations.liberal_tracking_obj_relative_velocity_lon_thr - 0.01F;
   object.vcs_heading = Angle{ calibrations.liberal_tracking_obj_heading_thr - 0.01F };

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when single object status is not invalid, 1st and 2nd level of conditions are met, but object not inside zone.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, all_conditions_met_outside_of_zone)
{
   /** \precond
   * Set host speed to be above threshold. Set object properties to met 2 levels of preconditions. Set object position to be outside of the zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position = { -1.0F, 0.0F };
   object.vcs_velocity.longitudinal = host.vcs_speed + calibrations.liberal_tracking_obj_relative_velocity_lon_thr + 0.01F;
   object.vcs_heading = Angle{ calibrations.liberal_tracking_obj_heading_thr - 0.01F };

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(object.f_valid_for_liberal_tracking);
}

/** \purpose
* Test Mark_Objects_For_Liberal_Tracking behavior for case when all pre-conditions for single object are met and it's inside zone.
* \req NA
*/
TEST(Mark_Objects_For_Liberal_Tracking_tests, all_conditions_met_inside_of_zone)
{
   /** \precond
   * Set host speed to be above threshold. Set object properties to met 2 levels of preconditions. Set object position to be inside of the zone.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   F360_Object_Track_T &object = objects[0];
   object.status = F360_OBJECT_STATUS_NEW;
   object.f_moving = true;
   object.vcs_position = { -50.0F, 0.0F };
   object.vcs_velocity.longitudinal = host.vcs_speed + calibrations.liberal_tracking_obj_relative_velocity_lon_thr + 0.01F;
   object.vcs_heading = Angle{ calibrations.liberal_tracking_obj_heading_thr - 0.01F };

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   Mark_Objects_For_Liberal_Tracking(calibrations, host, tracker_info, objects);

   /** \result
   * Choosen object should be marked as valid for liberal tracking.
   */
   CHECK_TRUE(object.f_valid_for_liberal_tracking);
}

TEST_GROUP(Check_Cluster_For_Liberal_Tracking_tests)
{
   // Declare common variables used within all tests in this test group.
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Detection_Props_T detections[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Cluster_T cluster = {};

   TEST_SETUP()
   {
      Clear_Detections_Props(detections);
      cluster.ndets = 3;
      cluster.detids[0] = 1;
      cluster.detids[1] = 2;
      cluster.detids[2] = 3;
   }
};

/** \purpose
* Test Check_Cluster_For_Liberal_Tracking behavior for case when host speed is below threshold.
* \req NA
*/
TEST(Check_Cluster_For_Liberal_Tracking_tests, host_speed_below_threshold)
{
   /** \precond
   * Set host speed to be below threshold.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed - 1.0F;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   bool valid_for_liberal_tracking = Check_Cluster_For_Liberal_Tracking(calibrations, host, detections, cluster);

   /** \result
   * Cluster should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(valid_for_liberal_tracking);
}

/** \purpose
* Test Check_Cluster_For_Liberal_Tracking behavior for case when none of cluster's detections are marked as valid for liberal tracking.
* \req NA
*/
TEST(Check_Cluster_For_Liberal_Tracking_tests, none_of_detections_valid)
{
   /** \precond
   * Set host speed to be above threshold. Set cluster's detections to be not valid for liberal tracking.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   detections[0].f_valid_for_liberal_tracking = false;
   detections[1].f_valid_for_liberal_tracking = false;
   detections[2].f_valid_for_liberal_tracking = false;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   bool valid_for_liberal_tracking = Check_Cluster_For_Liberal_Tracking(calibrations, host, detections, cluster);

   /** \result
   * Cluster should not be marked as valid for liberal tracking.
   */
   CHECK_FALSE(valid_for_liberal_tracking);
}

/** \purpose
* Test Check_Cluster_For_Liberal_Tracking behavior for case when one of cluster's detections is marked as valid for liberal tracking.
* \req NA
*/
TEST(Check_Cluster_For_Liberal_Tracking_tests, one_of_detections_valid)
{
   /** \precond
   * Set host speed to be above threshold. Set one of cluster's detections to be valid for liberal tracking.
   */
   host.speed = calibrations.liberal_tracking_min_host_speed + 1.0F;
   detections[0].f_valid_for_liberal_tracking = false;
   detections[1].f_valid_for_liberal_tracking = false;
   detections[2].f_valid_for_liberal_tracking = true;

   /** \action
   * Call Mark_Objects_For_Liberal_Tracking().
   */
   bool valid_for_liberal_tracking = Check_Cluster_For_Liberal_Tracking(calibrations, host, detections, cluster);

   /** \result
   * Cluster should be marked as valid for liberal tracking.
   */
   CHECK_TRUE(valid_for_liberal_tracking);
}

/** @}*/
