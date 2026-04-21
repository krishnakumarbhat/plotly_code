/** \file
 * This file contains unit tests for content of f360_visibility_info.cpp file
 */

#include "f360_visibility_info.h"
#include "f360_internal_preprocessing.h"
#include "rspp_calibrations.h"
#include "f360_math_func.h"
#include "f360_math.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_visibility_info
 *  @{
 */

 /** \brief
  * Test group of Can_Object_Be_Detected_By_Sensors() function. Tests verify whether
  * function properly determines whether object can be seen by any of available sensors.
  */
TEST_GROUP(can_object_be_detected_by_sensors)
{
   /** \setup
    * Set up common variables to use in tests
    * Set selected sensor parameters to have:
    * - azimuth view range: -pi/+pi
    * - view range: 50 [m]
    */
   F360_Object_Track_T obj{};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};

   TEST_SETUP()
   {
      sensors[0].variable.look_id = F360_DET_LOOK_ID_0;
      sensors[0].constant.range_limits[F360_DET_LOOK_ID_0] = 100.0F;
      sensor_props[0].left_fov_normal[F360_DET_LOOK_ID_0] = F360_PI;
      sensor_props[0].right_fov_normal[F360_DET_LOOK_ID_0] = F360_PI;
   }

};

/** \purpose
 * Purpose of this test is to verify whether when there are no valid sensors object is not marked as visible by sensor.
 * \req
 * NA.
 */
TEST(can_object_be_detected_by_sensors, can_object_be_detected_by_sensors__no_valid_sensors)
{
   /** \precond
    * Set all sensors status to invalid
    */
   for (int32_t i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      sensors[i].variable.is_valid = false;
   }

   /** \action
    * Call tested function
    */
   const bool f_visible = Can_Object_Be_Detected_By_Sensors(obj, sensor_props, sensors);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_visible);
}

/** \purpose
* Purpose of this test is to verify whether when object is visible by at least one sensor
* function returns true
* \req
* NA.
*/
TEST(can_object_be_detected_by_sensors, can_object_be_detected_by_sensors__visible_by_one_sensor)
{
   /** \precond
   * Set selected sensor status to valid
   * Place object inside sensor FOV
   */
   sensors[0].variable.is_valid = true;

   obj.vcs_position.x = 20.0F;
   obj.vcs_position.y = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_visible = Can_Object_Be_Detected_By_Sensors(obj, sensor_props, sensors);

   /** \result
   * Check whether returned value is false
   */
   CHECK_TRUE(f_visible);
}
/** @}*/



/** \defgroup  is_rel_point_below_given_range_limit
 *  @{
 */

 /** \brief
  * Test group of Is_Rel_Point_Below_Given_Range_Limit() function. Tests verify whether
  * function properly determines whether object is within object view range.
  */
TEST_GROUP(is_rel_point_below_given_range_limit)
{
   /** \setup
   * Set up common vaiables
   */
   float32_t rel_posn_lon{};
   float32_t rel_posn_lat{};
   float32_t range_limit{};
};

/** \purpose
* Purpose of this test is to verify whether when object is above sensor range limit function returns false
* \req
* NA.
*/
TEST(is_rel_point_below_given_range_limit, is_rel_point_below_given_range_limit__object_out_of_sensor_view_range)
{
   /** \precond
   * Set up sensor range limit to 100.0F
   * Set up object vcs position at 120.0F [m] longitudinal
   */
   range_limit = 100.0F;
   rel_posn_lon = 120.0F;
   rel_posn_lat = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_below_range_limit = Is_Rel_Point_Below_Given_Range_Limit(rel_posn_lon, rel_posn_lat, range_limit);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_below_range_limit)
}

/** \purpose
* Purpose of this test is to verify whether when object is below sensor range limit function returns true
* \req
* NA.
*/
TEST(is_rel_point_below_given_range_limit, is_rel_point_below_given_range_limit__object_in_sensor_view_range)
{
   /** \precond
   * Set up sensor range limit to 100.0F
   * Set up object vcs position at 80.0F [m] longitudinal
   */
   range_limit = 100.0F;
   rel_posn_lon = 80.0F;
   rel_posn_lat = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_below_range_limit = Is_Rel_Point_Below_Given_Range_Limit(rel_posn_lon, rel_posn_lat, range_limit);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_below_range_limit)
}
/** @}*/



/** \defgroup  is_rel_point_within_fov_azim
 *  @{
 */

 /** \brief
  * Test group of Is_Rel_Point_Within_FOV_Azim() function. Tests verify whether
  * function properly determines whether object is within object azimuth view range.
  */
TEST_GROUP(is_rel_point_within_fov_azim)
{
   /** \setup
   * Set up common vaiables
   */
   float32_t rel_posn_lon{};
   float32_t rel_posn_lat{};
   F360_Det_Range_Type_T range_type{};
   RSPP_Calibrations_T rspp_calib{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_Props_T& sens_props = sensor_props[0];
   F360_TRKR_TIMING_INFO_T timing_info{};
   const float32_t azim_th = F360_PI / 4;

   TEST_SETUP()
   {
      Initialize_RSPP_Calibrations(rspp_calib);
      F360_Radar_Sensor_T &sensor_0 =  sensors[0];
      for (int i = 0; i < 4; i++)
      {
         sensors[0].constant.fov_min_az_rad[i] = -azim_th;
         sensors[0].constant.fov_max_az_rad[i] = azim_th;
      }

      const float min_fov_az_angle_lr = std::min(sensor_0.constant.fov_min_az_rad[F360_DET_LOOK_ID_0], sensor_0.constant.fov_min_az_rad[F360_DET_LOOK_ID_1]);
      const float min_fov_az_interior_angle_lr = std::max(min_fov_az_angle_lr, -rspp_calib.fov_interior_limit);
      const float max_fov_az_angle_lr = std::max(sensor_0.constant.fov_max_az_rad[F360_DET_LOOK_ID_0], sensor_0.constant.fov_max_az_rad[F360_DET_LOOK_ID_1]);
      const float max_fov_az_interior_angle_lr = std::min(max_fov_az_angle_lr, rspp_calib.fov_interior_limit);
      const float min_fov_az_angle_mr = std::min(sensor_0.constant.fov_min_az_rad[F360_DET_LOOK_ID_2], sensor_0.constant.fov_min_az_rad[F360_DET_LOOK_ID_3]);
      const float min_fov_az_interior_angle_mr = std::max(min_fov_az_angle_mr, -rspp_calib.fov_interior_limit);
      const float max_fov_az_angle_mr = std::max(sensor_0.constant.fov_max_az_rad[F360_DET_LOOK_ID_2], sensor_0.constant.fov_max_az_rad[F360_DET_LOOK_ID_3]);
      const float max_fov_az_interior_angle_mr = std::min(max_fov_az_angle_mr, rspp_calib.fov_interior_limit);

      sensor_0.constant.interior_fov[F360_DET_LOOK_ID_0] = min_fov_az_interior_angle_lr;
      sensor_0.constant.interior_fov[F360_DET_LOOK_ID_1] = max_fov_az_interior_angle_lr;
      sensor_0.constant.interior_fov[F360_DET_LOOK_ID_2] = min_fov_az_interior_angle_mr;
      sensor_0.constant.interior_fov[F360_DET_LOOK_ID_3] = max_fov_az_interior_angle_mr;

      const float min_fov_vcs_az_angle_lr = sensor_0.constant.mounting_position.vcs_boresight_azimuth_angle + min_fov_az_interior_angle_lr;
      const float max_fov_vcs_az_angle_lr = sensor_0.constant.mounting_position.vcs_boresight_azimuth_angle + max_fov_az_interior_angle_lr;
      const float min_fov_vcs_az_angle_mr = sensor_0.constant.mounting_position.vcs_boresight_azimuth_angle + min_fov_az_interior_angle_mr;
      const float max_fov_vcs_az_angle_mr = sensor_0.constant.mounting_position.vcs_boresight_azimuth_angle + max_fov_az_interior_angle_mr;

      sensor_0.constant.left_fov_normal[F360_DET_LOOK_ID_0] = -F360_Sinf(min_fov_vcs_az_angle_lr);
      sensor_0.constant.left_fov_normal[F360_DET_LOOK_ID_1] = F360_Cosf(min_fov_vcs_az_angle_lr);
      sensor_0.constant.right_fov_normal[F360_DET_LOOK_ID_0] = F360_Sinf(max_fov_vcs_az_angle_lr);
      sensor_0.constant.right_fov_normal[F360_DET_LOOK_ID_1] = -F360_Cosf(max_fov_vcs_az_angle_lr);
      sensor_0.constant.left_fov_normal[F360_DET_LOOK_ID_2] = -F360_Sinf(min_fov_vcs_az_angle_mr);
      sensor_0.constant.left_fov_normal[F360_DET_LOOK_ID_3] = F360_Cosf(min_fov_vcs_az_angle_mr);
      sensor_0.constant.right_fov_normal[F360_DET_LOOK_ID_2] = F360_Sinf(max_fov_vcs_az_angle_mr);
      sensor_0.constant.right_fov_normal[F360_DET_LOOK_ID_3] = -F360_Cosf(max_fov_vcs_az_angle_mr);

      Copy_Signals_From_Sensors_to_Sensor_Props(sensor_props, sensors);

      rel_posn_lon = 10.0F;
      rel_posn_lat = 0.0F;
   }
};

/** \purpose
* Purpose of this test is to verify whether when sensor is in long look and object is in sensor field of view
* function returns true
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__long_look_in_fov_returns_true)
{
   /** \precond
   * Set up object position to be placed in front of sensor.
   * Set range type as long
   */
   range_type = F360_DET_RANGE_TYPE_LONG;
   rel_posn_lat = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_is_in_fov_azim);
}

/** \purpose
* Purpose of this test is to verify whether when sensor is in long look and object is not in sensor
* right azimuth fov, function returns false.
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__long_look_out_of_right_fov_returns_false)
{
   /** \precond
   * Set up object longitudinal position to be placed in front of sensor.
   * Set up object lateral position to be placed above sensor right fov edge
   * Set range type as long
   */
   range_type = F360_DET_RANGE_TYPE_LONG;
   rel_posn_lat = rel_posn_lon * F360_Tanf(azim_th) + 0.1F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_is_in_fov_azim);
}

/** \purpose
* Purpose of this test is to verify whether when sensor is in long look and object is not in sensor
* left azimuth fov, function returns false.
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__long_look_out_of_left_fov_returns_false)
{
   /** \precond
   * Set up object longitudinal position to be placed in front of sensor.
   * Set up object lateral position to be placed below sensor left fov edge
   * Set range type as long
   */
   range_type = F360_DET_RANGE_TYPE_LONG;
   rel_posn_lat = rel_posn_lon * F360_Tanf(-azim_th) - 0.1F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_is_in_fov_azim);
}

/** \purpose
* Purpose of this test is to verify whether when sensor is in medium look and object is in sensor field of view
* function returns true
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__medium_look_in_fov_returns_true)
{
   /** \precond
   * Set up object position to be placed in front of sensor.
   * Set range type as medium
   */
   range_type = F360_DET_RANGE_TYPE_MEDIUM;
   rel_posn_lon = 10.0F;
   rel_posn_lat = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_is_in_fov_azim);
}

/** \purpose
* Purpose of this test is to verify whether when sensor is in long look and object is not in sensor
* right azimuth fov, function returns false.
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__medium_look_out_of_right_fov_returns_false)
{
   /** \precond
   * Set up object longitudinal position to be placed in front of sensor.
   * Set up object lateral position to be placed above sensor right fov edge
   * Set range type as medium
   */
   range_type = F360_DET_RANGE_TYPE_MEDIUM;
   rel_posn_lat = rel_posn_lon * F360_Tanf(azim_th) + 0.1F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_is_in_fov_azim);
}

/** \purpose
* Purpose of this test is to verify whether when sensor is in long look and object is not in sensor
* left azimuth fov, function returns false.
* \req
* NA.
*/
TEST(is_rel_point_within_fov_azim, is_rel_point_within_fov_azim__medium_look_out_of_left_fov_returns_false)
{
   /** \precond
   * Set up object longitudinal position to be placed in front of sensor.
   * Set up object lateral position to be placed below sensor left fov edge
   * Set range type as medium
   */
   range_type = F360_DET_RANGE_TYPE_MEDIUM;
   rel_posn_lat = rel_posn_lon * F360_Tanf(-azim_th) - 0.1F;

   /** \action
   * Call tested function
   */
   const bool f_is_in_fov_azim = Is_Rel_Point_Within_FOV_Azim(rel_posn_lon, rel_posn_lat, sens_props, range_type);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_is_in_fov_azim);
}
/** @}*/



/** \defgroup  is_vcs_point_within_current_fov_of_sensor
 *  @{
 */

 /** \brief
  * Test group of Is_VCS_Point_Within_Current_FOV_Of_Sensor() function. Tests verify whether
  * function properly determines whether object is within object fov.
  */
TEST_GROUP(is_vcs_point_within_current_fov_of_sensor)
{
   /** \setup
   * Set up common vaiables
   */
   Point point_pos;
   F360_Radar_Sensor_Props_T sens_props;
   F360_Radar_Sensor_T sensor;

   TEST_SETUP()
   {
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.range_limits[sensor.variable.look_id] = 50.0F;
      sens_props.left_fov_normal[F360_DET_LOOK_ID_0] = 0.5F;
      sens_props.right_fov_normal[F360_DET_LOOK_ID_0] = 0.5F;
   }
};

/** \purpose
* Purpose of this test is to verify whether when sensor is sensor field of fiew
* function returns true.
* \req
* NA.
*/
TEST(is_vcs_point_within_current_fov_of_sensor, is_vcs_point_within_current_fov_of_sensor__is_in_fov_returns_true)
{
   /** \precond
   * Set up object position to be placed in sensor field of view
   */
   point_pos.x = 10.0F;
   point_pos.y = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_visible = Is_VCS_Point_Within_Current_FOV_Of_Sensor(point_pos, sensor, sens_props, sensor.variable.look_id);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_visible);

}

/** \purpose
* Purpose of this test is to verify whether when object has too high range
* function returns false.
* \req
* NA.
*/
TEST(is_vcs_point_within_current_fov_of_sensor, is_vcs_point_within_current_fov_of_sensor__too_high_range)
{
   /** \precond
   * Set up object position to be above sensor view range
   */
   point_pos.x = sensor.constant.range_limits[sensor.variable.look_id] + 10.0F;
   point_pos.y = 0.0F;

   /** \action
   * Call tested function
   */
   const bool f_visible = Is_VCS_Point_Within_Current_FOV_Of_Sensor(point_pos, sensor, sens_props, sensor.variable.look_id);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_visible);

}

/** \purpose
* Purpose of this test is to verify whether when object is not in sensor 
* azimuth view function returns false.
* \req
* NA.
*/
TEST(is_vcs_point_within_current_fov_of_sensor, is_vcs_point_within_current_fov_of_sensor__out_of_sensor_azimuth_view_range)
{
   /** \precond
   * Set up object position to be above sensor azimuth view range
   */
   point_pos.x = 10.0F;
   point_pos.y = 100.0F;

   /** \action
   * Call tested function
   */
   const bool f_visible = Is_VCS_Point_Within_Current_FOV_Of_Sensor(point_pos, sensor, sens_props, sensor.variable.look_id);

   /** \result
   * Check whether returned value is false
   */
   CHECK_FALSE(f_visible);

}
/** @}*/
