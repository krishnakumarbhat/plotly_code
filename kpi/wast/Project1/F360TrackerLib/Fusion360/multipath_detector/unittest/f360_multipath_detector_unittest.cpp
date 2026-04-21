/** \file
 * This file contains unit tests for content of f360_multipath_detector.cpp file
 */

#include "f360_multipath_detector.h"
#include "f360_sorted_tracks_mgmt.h"
#include <CppUTest/TestHarness.h>
#include "f360_calibrations.h"
#include "f360_set_variant.h"

 // Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  Is_Multipath___SEP_Reflector
 *  @{
 */

 /** \brief
  * This test group contains test cases for Is_Multipath()
  */
TEST_GROUP(Is_Multipath___SEP_Reflector)
{
   F360_Calibrations_T calibs;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS]{};
   Static_Env_T::Static_Env_Polys_Array static_env_polys_array;
   F360_Tracker_Info_T tracker_info = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);
   }

   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
};

/** \purpose
 * Test checks if item is reported as multipath when range to it is equal to range to source.
 * \req
 * NA
 */
IGNORE_TEST(Is_Multipath___SEP_Reflector, if_item_has_range_equal_to_source)
{
   //IGNORE reason: in current implementation SEP is not used as reflector. Reflectors are limited to objects.

   /** \preconditions
   * Set object(reflection source) parameters
   * Set reflector(guardrail) parameters
   * Set lateral and longitudinal position of sensor
   * Set lateral and longitudinal position of examined item to be reported as multipath
   */
   objects[0].vcs_position.x = 16.0F;
   objects[0].vcs_position.y = 0.0F;
   objects[0].vcs_velocity.longitudinal = 6.0F;
   objects[0].vcs_velocity.lateral = 0.0F;

   static_env_polys_array[0].p0 = 6.0F;
   static_env_polys_array[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
   static_env_polys_array[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;

   const Point radar_pos = {};

   const Point item_pos = { 16.0F, 12.0F };

   const float32_t item_range_rate = 4.8F;
   
   /** \action
    * Call Is_Multipath()
    */
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as mutipath
    */
   CHECK_TRUE(expected_result);
}


/** \purpose
 * Test checks if item is not reported as multipath when range to it is lower than range to source.
 * \req
 * NA
 */
IGNORE_TEST(Is_Multipath___SEP_Reflector, if_item_has_range_lower_than_source)
{
   //IGNORE reason: in current implementation SEP is not used as reflector. Reflectors are limited to objects.
   /** \preconditions
   * Set object(reflection source) parameters
   * Set reflector(guardrail) parameters
   * Set lateral and longitudinal position of sensor
   * Set lateral and longitudinal position of examined item to be NOT reported as multipath
   */
   objects[0].vcs_position.x = 16.0F;
   objects[0].vcs_position.y = 0.0F;
   objects[0].vcs_velocity.longitudinal = 6.0F;
   objects[0].vcs_velocity.lateral = 0.0F;

   static_env_polys_array[0].p0 = 6.0F;
   static_env_polys_array[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
   static_env_polys_array[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;

   const Point radar_pos = {};

   const Point item_pos = { 14.0F, 12.0F };

   const float32_t item_range_rate = 4.8F;

   /** \action
    * Call Is_Multipath()
    */
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}


/** \purpose
 * Test checks if item is not reported as multipath when range to it is equal to range to source
 * but source speed is equal to zero.
 * \req
 * NA
 */
IGNORE_TEST(Is_Multipath___SEP_Reflector, if_item_has_range_equal_to_source_range_but_wrong_range_rate)
{
   //IGNORE reason: in current implementation SEP is not used as reflector. Reflectors are limited to objects.
   /** \preconditions
   * Set object(reflection source) parameters
   * Set reflector(guardrail) parameters
   * Set lateral and longitudinal position of sensor
   * Set lateral and longitudinal position of examined item to be NOT reported as multipath
   */
   objects[0].vcs_position.x = 16.0F;
   objects[0].vcs_position.y = 0.0F;
   objects[0].vcs_velocity.longitudinal = 0.0F;
   objects[0].vcs_velocity.lateral = 0.0F;
   
   static_env_polys_array[0].p0 = 6.0F;
   static_env_polys_array[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
   static_env_polys_array[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;

   const Point radar_pos = {};

   const Point item_pos = { 16.0F, 12.0F };

   const float32_t item_range_rate = 0.0F;

   /** \action
    * Call Is_Multipath()
    */
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}


/** \purpose
 * Test checks if item is not reported as multipath when range to it is lower than range to source
 * but both have the same longitudinal position and range rates.
 * \req
 * NA
 */
IGNORE_TEST(Is_Multipath___SEP_Reflector, if_item_and_source_have_the_same_long_pos_and_range_rates)
{
   //IGNORE reason: in current implementation SEP is not used as reflector. Reflectors are limited to objects.
   /** \preconditions 
   * Set object(reflection source) parameters
   * Set reflector(guardrail) parameters
   * Set lateral and longitudinal position of sensor
   * Set lateral and longitudinal position of examined item to be NOT reported as multipath
   */
   objects[0].vcs_position.x = 16.0F;
   objects[0].vcs_position.y = 0.0F;
   objects[0].vcs_velocity.longitudinal = 4.0F;
   objects[0].vcs_velocity.lateral = 0.0F;
   static_env_polys_array[0].p0 = 6.0F;
   static_env_polys_array[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
   static_env_polys_array[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;

   const Point radar_pos = {};

   const Point item_pos = { 16.0F, 8.0F };

   const float32_t item_range_rate = 3.2F;

   /** \action
    * Call Is_Multipath()
    */
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}
/** @}*/

/** \defgroup  Is_Multipath___Stationary_Object_Reflector
 *  @{
 */

 /** \brief
  * This test group contains test cases for Is_Multipath() when reflector is stationary
  */
TEST_GROUP(Is_Multipath___Stationary_Object_Reflector)
{
   F360_Calibrations_T calibs;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS]{};
   Static_Env_T::Static_Env_Polys_Array static_env_polys_array = {};
   F360_Tracker_Info_T tracker_info = {};

   Point radar_pos;
   Point item_pos;
   float32_t item_range_rate;

   F360_Object_Track_T &reflector = objects[0];
   F360_Object_Track_T &refl_source = objects[1];

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);

      //Set (reflector) stationary reflector parameters
      reflector.status = F360_OBJECT_STATUS_UPDATED;
      reflector.Set_Bbox_Orientation(Angle{ 0.0F });
      reflector.id = 1;
      reflector.vcs_position = { 15.0F, 4.0F };
      reflector.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = { 15.0F, 4.0F };
      reflector.bbox.Set_Center(center);
      reflector.bbox.Set_Length(2.0F);
      reflector.bbox.Set_Width(2.0F);

      // Set object (reflection source) parameters
      refl_source.vcs_velocity = { 5.0F, -1.0F };
      refl_source.f_moving = true;
      refl_source.Set_Bbox_Orientation(Angle{ -0.1F });
      refl_source.status = F360_OBJECT_STATUS_UPDATED;
      refl_source.id = 2;
      reflector.vcs_position = { 6.0F, 14.0F };
      refl_source.reference_point = F360_REFERENCE_POINT_CENTER;
      center = { 6.0F, 14.0F };
      refl_source.bbox.Set_Center(center);
      refl_source.bbox.Set_Length(6.0F);
      refl_source.bbox.Set_Width(3.0F);

      tracker_info.active_obj_ids[0] = 1;
      tracker_info.active_obj_ids[1] = 2;

      radar_pos = { 2.0F, 2.0F };
      item_pos = { 26.0F, 6.0F };
      item_range_rate = -4.0F;

      tracker_info.num_active_objs = 2;

      // Reset sorted vcslong list
      tracker_info.vcslong_sorted_start = NULL;
      for (uint32_t i = 0; i < NUMBER_OF_OBJECT_TRACKS; i++)
      {
         tracker_info.vcslong_sorted_next_track[i] = NULL;
         tracker_info.vcslong_sorted_prev_track[i] = NULL;
      }
      Sorted_Tracks_Insert(tracker_info, &(reflector));
      Sorted_Tracks_Insert(tracker_info, &(refl_source));
   }
};

/** \purpose
 * Check if Is_Multipath() returns true when there is true multipath case
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, valid_multipath)
{
   /** \preconditions 
   * Same as group setup - two objects that match true multipath model.
   */

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as multipath
    */
   CHECK_TRUE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns false when there is no any intersection with reflector
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, no_intersections_with_reflector)
{
   /** \preconditions 
   * Same as setup
   * Reflector obejct does not intersect with sensor-item segment
   */
   reflector.vcs_position = { 16.0F, 8.0F };
   Point center = {16.0F, 8.0F};
   reflector.bbox.Set_Center(center);

   /** \action
    * Call Is_Multipath()
    */

   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as no mutipath
    */
   CHECK_FALSE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns true when there is true multipath case (two potential reflectors, real is closer)
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, valid_multipath__use_closer_reflector)
{
   /** \preconditions 
   * Same as group setup - three objects that match true multipath model.
   */
   objects[2].vcs_position = { 20.0F, 5.0F };
   Point center = {20.0F, 5.0F};
   objects[2].bbox.Set_Center(center);
   objects[2].status = F360_OBJECT_STATUS_UPDATED;
   objects[2].Set_Bbox_Orientation(Angle{ 0.0F });
   objects[2].id = 3;
   

   tracker_info.num_active_objs++;
   Sorted_Tracks_Insert(tracker_info, &(objects[2]));

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as mutipath
    */
   CHECK_TRUE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns true when there is true multipath case (two potential reflectors, real is further) - case 1
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, valid_multipath__use_closer_reflector__reverted_order__case_1)
{
   /** \preconditions
   * Same as group setup - three objects that match true multipath model.
   */
   objects[2].vcs_position = { 20.0F, 5.0F };
   Point center = {20.0F, 5.0F};
   objects[2].bbox.Set_Center(center);
   objects[2].status = F360_OBJECT_STATUS_UPDATED;
   objects[2].Set_Bbox_Orientation(Angle{ 0.0F });
   objects[2].id = 3;

   //Revert order
   F360_Object_Track_T temp = reflector;
   reflector = objects[2];
   objects[2] = temp;

   tracker_info = {};
   Set_Tracker_Variant(tracker_info.variant);
   tracker_info.num_active_objs = 3;
   Sorted_Tracks_Insert(tracker_info, &(objects[2]));
   Sorted_Tracks_Insert(tracker_info, &(objects[1]));
   Sorted_Tracks_Insert(tracker_info, &(objects[0]));

   /** \action
    * Call Is_Multipath()
    */

   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as mutipath
    */
   CHECK_TRUE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns true when there is true multipath case (two potential reflectors, real is further)  case 2
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, valid_multipath__use_closer_reflector__reverted_order__case_2)
{
   /** \preconditions
   * Same as group setup - three objects that match true multipath model.
   */
   objects[2].vcs_position = { 14.7F, 4.0F };
   Point center = {14.7F, 4.0F};
   objects[2].bbox.Set_Center(center);
   objects[2].Update_Bbox_Size(1.0F, 1.0F);
   objects[2].status = F360_OBJECT_STATUS_UPDATED;
   objects[2].Set_Bbox_Orientation(Angle{ 0.0F });
   objects[2].id = 3;

   //Revert order
   F360_Object_Track_T temp = reflector;
   reflector = objects[2];
   objects[2] = temp;

   tracker_info = {};
   Set_Tracker_Variant(tracker_info.variant);
   tracker_info.num_active_objs = 3;
   Sorted_Tracks_Insert(tracker_info, &(objects[2]));
   Sorted_Tracks_Insert(tracker_info, &(objects[1]));
   Sorted_Tracks_Insert(tracker_info, &(objects[0]));

   /** \action
    * Call Is_Multipath()
    */

   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as mutipath
    */
   CHECK_TRUE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns false when there is true multipath case (two potential reflectors, real is closer)
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, invalid_multipath_due_to_being_inside)
{
   /** \preconditions
   * Same as group setup - two objects that match true multipath model.
   * Set reflection source object to item position.
   */
   refl_source.vcs_position = item_pos;
   refl_source.reference_point = F360_REFERENCE_POINT_CENTER;
   Point center = item_pos;
   refl_source.bbox.Set_Center(center);

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as NOT multipath
    */
   CHECK_FALSE(expected_result);
}


/** \purpose
 * Check if Is_Multipath() returns false when source reflector is too close (inside searching range)
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, range_hypothesis__source_too_close)
{
   /** \preconditions 
   * Same as group setup but source object is positioned closer reflection point.
   */ 
   refl_source.vcs_position = {9.0F, 9.0F};
   refl_source.reference_point = F360_REFERENCE_POINT_CENTER;
   Point center = {9.0F, 9.0F};
   refl_source.bbox.Set_Center(center);

   //Set lateral and longitudinal position of examined item to be reported as multipath

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns false when source reflector is too far away (outside searching range)
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, range_hypothesis__source_too_far_away)
{
   /** \preconditions 
   * Same as group setup but source object is positioned further reflection point.
   */
   refl_source.vcs_position = {3.0F, 14.0F};
   refl_source.reference_point = F360_REFERENCE_POINT_CENTER;
   Point center = {3.0F, 14.0F};
   refl_source.bbox.Set_Center(center);

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns false when item range rate is too low
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, range_rate_hypothesis__item_too_low_speed)
{
   /** \preconditions 
   * Same as group setup but item range rate is set to low value.
   */
   item_range_rate = -5.11F;

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}

/** \purpose
 * Check if Is_Multipath() returns false when item range rate is too high
 * \req
 * NA
 */
TEST(Is_Multipath___Stationary_Object_Reflector, range_rate_hypothesis__item_too_high_speed)
{
   /** \preconditions
   * Same as group setup but item range rate is set to high value.
   */
   item_range_rate = -2.2F;

   /** \action
    * Call Is_Multipath()
    */
   Multipath_Detector multipath_detector = Multipath_Detector(static_env_polys_array, objects, tracker_info, calibs);
   const bool expected_result = multipath_detector.Is_Multipath(radar_pos, item_pos, item_range_rate, tracker_info);

   /** \result
    * Function should report item as not mutipath
    */
   CHECK_FALSE(expected_result);
}

/** @}*/
