/** \file
 * This file contains unit tests for content of f360_object_based_radar_phenomena_internals.cpp file
 */

#include "f360_object_based_radar_phenomena_internals.h"
#include <CppUTest/TestHarness.h>

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference
 *  @{
 */
 /** \brief
  * Test group for testing Can_Object_Be_A_Reference()
  */
TEST_GROUP(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference)
{
   const float32_t floating_threshold = 0.0001F;
   F360_Object_Track_T object_track;

   F360_Calibrations_T calibs;

   /** \setup
    * Set object status to F360_OBJECT_STATUS_UPDATED
    * Set object to be moving
    * Set object's pointing below threshold
    * Set object's lateral postion below threshold
    * Set calibs
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      object_track.status = F360_OBJECT_STATUS_UPDATED;
      object_track.f_moving = true;
      object_track.confidenceLevel = calibs.rp_min_confidence_level + floating_threshold;
      Point center = {0.0F, calibs.rp_max_object_lateral_distance - floating_threshold};
      object_track.bbox.Set_Center(center);
      object_track.bbox.Set_Orientation(Angle{ calibs.rp_max_abs_pointing_disagreement - floating_threshold });
      object_track.bbox.Set_Length(5.0F);
      object_track.bbox.Set_Width(2.0F);
      object_track.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      object_track.vcs_position = object_track.bbox.Get_Corners().Rear_Left();
   }
};

/** \purpose
 * Can_Object_Be_A_Reference should return true if object meets all condtions, case 1
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_meets_all_conditions_to_be_valid_case_1)
{
   /** \precond
    * Same as setup
    */

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if true
    */
   CHECK_TRUE(result);
}

/** \purpose
 * Can_Object_Be_A_Reference should return true if object meets all condtions, case 2
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_meets_all_conditions_to_be_valid_case_2)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's lateral postion within threshold (just above negative limit)
    */
   Point center = {object_track.bbox.Get_Center().x, -calibs.rp_max_object_lateral_distance + floating_threshold};
   object_track.bbox.Set_Center(center);
   object_track.reference_point = F360_REFERENCE_POINT_RIGHT;
   object_track.vcs_position = {-0.17355F, -7.0151F};

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if true
    */
   CHECK_TRUE(result);
}

/** \purpose
 * Can_Object_Be_A_Reference should return true if object meets all condtions, case 3
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_meets_all_conditions_to_be_valid_case_3)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's pointing within threshold (just above negative limit)
    */
   object_track.Set_Bbox_Orientation(Angle{ -calibs.rp_max_abs_pointing_disagreement + floating_threshold });

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if true
    */
   CHECK_TRUE(result);
}

/** \purpose
 * Can_Object_Be_A_Reference should return true if object meets all condtions, case 4
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_meets_all_conditions_to_be_valid_case_4)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's laterla position within threshold (just above negative limit)
   * Set object's pointing within threshold (just above negative limit)
    */
   object_track.Set_Bbox_Orientation(Angle{ -calibs.rp_max_abs_pointing_disagreement + floating_threshold });
   Point center = {object_track.bbox.Get_Center().x, -calibs.rp_max_object_lateral_distance + floating_threshold};
   object_track.bbox.Set_Center(center);
   object_track.reference_point = F360_REFERENCE_POINT_RIGHT;
   object_track.vcs_position = {0.17355F, -7.0151F};

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if true
    */
   CHECK_TRUE(result);
}

/** \purpose
 * Can_Object_Be_A_Reference should return false if object is invalid
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_to_being_invalid)
{
   /** \precond
    * Same as setup + below changes:
    * Set object status to INVALID
    */
   object_track.status = F360_OBJECT_STATUS_INVALID;

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if false
    */
   CHECK_FALSE(result);
}

/** \purpose
 * Can_Object_Be_A_Reference should return false if object not moving
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_to_not_being_moving)
{
   /** \precond
   * Same as setup + below changes:
    * Set object to be stationary
    */
   object_track.f_moving = false;

   /** \action
    * Call Can_Object_Be_A_Reference()
    */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
    * Check if false
    */
   CHECK_FALSE(result);
}

/** \purpose
   * Can_Object_Be_A_Reference should return false if object's pointing is too high (positive)
   * \req
   *  NA.
   */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_too_high_pointing_positive)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's pointing above threshold
   */
   object_track.Set_Bbox_Orientation(Angle{ calibs.rp_max_abs_pointing_disagreement + floating_threshold });

   /** \action
   * Call Can_Object_Be_A_Reference()
   */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
   * Check if false
   */
   CHECK_FALSE(result);
}


/** \purpose
   * Can_Object_Be_A_Reference should return false if object's pointing is too high (negative)
   * \req
   *  NA.
   */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_too_high_pointing_negative)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's pointing above threshold
   */
   object_track.Set_Bbox_Orientation(Angle{ -calibs.rp_max_abs_pointing_disagreement - floating_threshold });

   /** \action
   * Call Can_Object_Be_A_Reference()
   */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
   * Check if false
   */
   CHECK_FALSE(result);
}

/** \purpose
   * Can_Object_Be_A_Reference should return false if object's lateral postion is too high (positive)
   * \req
   *  NA.
   */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_too_high_lateral_position_positive)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's lateral postion above threshold
   */
   Point center = {object_track.bbox.Get_Center().x, calibs.rp_max_object_lateral_distance + floating_threshold};
   object_track.bbox.Set_Center(center);
   object_track.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   object_track.vcs_position = object_track.bbox.Get_Corners().Rear_Left();

   /** \action
   * Call Can_Object_Be_A_Reference()
   */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
   * Check if false
   */
   CHECK_FALSE(result);
}

/** \purpose
   * Can_Object_Be_A_Reference should return false if object's lateral postion is too high (negative)
   * \req
   *  NA.
   */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_too_high_lateral_position_negative)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's lateral postion above threshold
   */
   Point center = {object_track.bbox.Get_Center().x, -(calibs.rp_max_object_lateral_distance + floating_threshold)};
   object_track.bbox.Set_Center(center);
   object_track.reference_point = F360_REFERENCE_POINT_RIGHT;
   object_track.vcs_position = {-0.17355F, -7.0153F};

   /** \action
   * Call Can_Object_Be_A_Reference()
   */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
   * Check if false
   */
   CHECK_FALSE(result);
}

/** \purpose
   * Can_Object_Be_A_Reference should return false if object's confidence level is too low
   * \req
   *  NA.
   */
TEST(f360_object_based_radar_phenomena_internals__Can_Object_Be_A_Reference, object_is_not_valid_due_too_low_confidence_level)
{
   /** \precond
   * Same as setup + below changes:
   * Set object's confidence level equal to threshold
   */
   object_track.confidenceLevel = calibs.rp_min_confidence_level;

   /** \action
   * Call Can_Object_Be_A_Reference()
   */
   bool result = Can_Object_Be_A_Reference(object_track, calibs.rp_max_object_lateral_distance, calibs.rp_max_abs_pointing_disagreement, calibs.rp_min_confidence_level);

   /** \result
   * Check if false
   */
   CHECK_FALSE(result);
}
/** @}*/


/** \defgroup  f360_object_based_radar_phenomena_internals__Determine_Side_Sensors
 *  @{
 */
 /** \brief
  * Test group for testing Determine_Side_Sensors()
  */
TEST_GROUP(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors)
{
   const float32_t floating_threshold = 0.0001F;

   F360_Object_Track_T object_track{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   float32_t longitudinal_gap{};

   /** \setup
   * 
   **/
   TEST_SETUP()
   {
      sensors[0].variable.is_valid = true;
      sensors[0].constant.mounting_position.vcs_position = { 0.0F, 1.0F, 0.0F };
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      longitudinal_gap = 0.0F;
      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      object_track.vcs_position = { -5.0F, 3.0F };
      object_track.bbox.Set_Center(object_track.vcs_position);
      object_track.bbox.Set_Orientation(Angle(0.0F));
      object_track.bbox.Set_Length(10.0F + floating_threshold);
      object_track.bbox.Set_Width(2.0F);
   }
};

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met)
{
   /** \precond
    * Same as setup
    */

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS;i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_left_side1_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_LEFT_SIDE1
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE1;

    /** \action
     * Call Determine_Side_Sensors()
     */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_rear_left_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_LEFT_REAR 
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_left_side2_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_LEFT_SIDE2
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_SIDE2;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_right_forwar_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_RIGHT_FORWARD 
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_right_side1_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_RIGHT_SIDE1 
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE1;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_right_side2_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_RIGHT_SIDE2 
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_SIDE2;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should mark one of sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, all_conditions_Met_and_rear_right_sensor)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_RIGHT_REAR 
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if only first sensor is relevant
    */
   for (int i = 1; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
   CHECK_TRUE(relevant_sensors[0]);
}

/** \purpose
 * Determine_Side_Sensors should not mark any sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, not_marked_due_to_sensor_being_forward)
{
   /** \precond
    * Set location enum to F360_MOUNTING_LOCATION_CENTER_FORWARD
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if there is no revelant sensor
    */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
}

/** \purpose
 * Determine_Side_Sensors should not mark any sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, not_marked_due_to_object_is_to_far_away__front_bumper)
{
   /** \precond
    * Set object longitudinal position to -5.0F
    */
   Point center = {-5.0F - floating_threshold, object_track.bbox.Get_Center().y};
   object_track.bbox.Set_Center(center);


   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if there is no revelant sensor
    */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
}

/** \purpose
 * Determine_Side_Sensors should not mark any sensors as relevant
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, not_marked_due_to_object_is_to_far_away__rear_bumper)
{
   /** \precond
    * Set object longitudinal position to 5.0F
    */
   Point center = {5.0F + floating_threshold, object_track.bbox.Get_Center().y};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if there is no revelant sensor
    */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
}


/** \purpose
 * Determine_Side_Sensors should not mark any sensors as relevant
 * because object is on opposite side wrt. sensor
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors, not_marked_due_to_object_is_on_different_side_than_sensor)
{
   /** \precond
    * Set object lateral position to negative value
    */
   Point center = {object_track.bbox.Get_Center().x, -object_track.bbox.Get_Center().y};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool relevant_sensors[MAX_NUMBER_OF_SENSORS]{};
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, relevant_sensors);

   /** \result
    * Check if there is no revelant sensor
    */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      CHECK_FALSE(relevant_sensors[i]);
   }
}
/** @}*/

/** \defgroup  f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD
 *  @{
 */
 /** \brief
  * Test group for testing Determine_Side_Sensors()
  */
TEST_GROUP(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD)
{
   const float32_t floating_threshold = 0.0001F;

   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   float32_t longitudinal_gap;
   F360_Object_Track_T object_track;

   /** \setup
   * Set sensors calibrations. Only first 4 are valid.
   * Set longitudinal_gap to 2.0F
   * Set object's length
   **/
   TEST_SETUP()
   {
      for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
      {
         sensors[idx].variable.is_valid = false;
         sensors[idx].constant.mounting_location = F360_MOUNTING_LOCATION_UNKNOWN;
      }

      // Front right
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[0].constant.mounting_position.vcs_position.longitudinal = -0.25F;
      sensors[0].constant.mounting_position.vcs_position.lateral = 0.9F;
      sensors[0].variable.is_valid = true;

      // Front left
      sensors[1].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;
      sensors[1].constant.mounting_position.vcs_position.longitudinal = -0.25F;
      sensors[1].constant.mounting_position.vcs_position.lateral = -0.9F;
      sensors[1].variable.is_valid = true;

      // Rear right
      sensors[2].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;
      sensors[2].constant.mounting_position.vcs_position.longitudinal = -3.9F;
      sensors[2].constant.mounting_position.vcs_position.lateral = 0.9F;
      sensors[2].variable.is_valid = true;

      // Rear left
      sensors[3].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;
      sensors[3].constant.mounting_position.vcs_position.longitudinal = -3.9F;
      sensors[3].constant.mounting_position.vcs_position.lateral = -0.9F;
      sensors[3].variable.is_valid = true;

      longitudinal_gap = 2.0F;

      object_track.bbox.Set_Length(6.0F);
   }
};

/** \purpose
 * Determine_Side_Sensors should not determine any sensor if all sensors are invalid
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, all_sensors_are_invalid)
{
   /** \precond
    * Set sensors to be invalid
    */
   for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      sensors[idx].variable.is_valid = false;
   }

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS] = { true }; // set initally only first sensor as valid
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * All sensors are invalid
    */
   for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}

/** \purpose
 * Determine_Side_Sensors should determine only right sensors if an object is on the right
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, determine_only_right_sensors)
{
   /** \precond
    * Object is on the right side and meets longitudinal condtions
    */
   Point center = {0.0F, 4.0F};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS] = { true }; // set initally only first sensor as valid
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * Only sensor on the right side are valid
    */
   CHECK_TRUE(valid_sensors[0]);
   CHECK_FALSE(valid_sensors[1]);
   CHECK_TRUE(valid_sensors[2]);

   for (int idx = 3; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}

/** \purpose
 * Determine_Side_Sensors should determine only right sensors if an object is on the left
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, determine_only_left_sensors)
{
   /** \precond
    * Object is on the left side and meets longitudinal condtions
    */
   Point center = {0.0F, -4.0F};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS] = { true }; // set initally only first sensor as valid
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * Only sensor on the left side are valid
    */
   CHECK_FALSE(valid_sensors[0]);
   CHECK_TRUE(valid_sensors[1]);
   CHECK_FALSE(valid_sensors[2]);
   CHECK_TRUE(valid_sensors[3]);

   for (int idx = 4; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}

/** \purpose
 * Determine_Side_Sensors should not determine any sensor if obejct is too far away (in longitudinal positive direction)
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, no_valid_sensor_due_to_object_too_far_away_positive)
{
   /** \precond
    * Object obejct is too far away (in longitudinal direction)
    * Object on the right
    */
   Point center = {0.5F * object_track.bbox.Get_Length() + sensors[0].constant.mounting_position.vcs_position.longitudinal + longitudinal_gap + floating_threshold, 4.0F};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS];
   std::fill_n(valid_sensors, MAX_NUMBER_OF_SENSORS, true);
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * No valid sensors
    */
   for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}

/** \purpose
 * Determine_Side_Sensors should not determine any sensor if obejct is too far away (in longitudinal negative direction)
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, no_valid_sensor_due_to_object_too_far_away_negative)
{
   /** \precond
    * Object obejct is too far away (in longitudinal direction)
    * Object on the right
    */
   Point center = {-0.5F * object_track.bbox.Get_Length() + sensors[2].constant.mounting_position.vcs_position.longitudinal - longitudinal_gap - floating_threshold, 4.0F};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS];
   std::fill_n(valid_sensors, MAX_NUMBER_OF_SENSORS, true);
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * No valid sensors
    */
   for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}

/** \purpose
 * Determine_Side_Sensors should determine only rear sensor if obejct is close enough (in longitudinal direction)
 * \req
 *  NA.
 */
TEST(f360_object_based_radar_phenomena_internals__Determine_Side_Sensors_moved_from_AJD, front_sensor_invalid_and_rear_valid)
{
   /** \precond
    * Object obejct is too far away (in longitudinal direction)
    * Object on the right
    */
   Point center = {-0.5F * object_track.bbox.Get_Length() + sensors[0].constant.mounting_position.vcs_position.longitudinal - longitudinal_gap - floating_threshold, 4.0F};
   object_track.bbox.Set_Center(center);

   /** \action
    * Call Determine_Side_Sensors()
    */
   bool valid_sensors[MAX_NUMBER_OF_SENSORS];
   std::fill_n(valid_sensors, MAX_NUMBER_OF_SENSORS, true);
   Determine_Side_Sensors(object_track, sensors, longitudinal_gap, valid_sensors);

   /** \result
    * Only sensor on the right side are valid
    */
   CHECK_FALSE(valid_sensors[0]);
   CHECK_FALSE(valid_sensors[1]);
   CHECK_TRUE(valid_sensors[2]);

   for (int idx = 3; idx < MAX_NUMBER_OF_SENSORS; idx++)
   {
      CHECK_FALSE(valid_sensors[idx]);
   }
}
/** @}*/