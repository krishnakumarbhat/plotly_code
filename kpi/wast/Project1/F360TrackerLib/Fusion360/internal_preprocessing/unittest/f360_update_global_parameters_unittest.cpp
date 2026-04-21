/** \file
 * This file contains unit tests for content of f360_update_global_parameters.cpp file
 */

#include "f360_update_global_parameters.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  Update_Global_Parameters
 *  @{
 */

/** \brief
 * Test group dedicated to verifying main function Update_Global_Parameters calls the correct sub functions
 */
TEST_GROUP(Update_Global_Parameters)
{
   F360_Host_T host = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calibrations = {};
   F360_Globals_T globals = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};

   /** \setup
     * Set host.vcs_speed = 9.0F
     */
   TEST_SETUP()
   {
      host.vcs_speed = 9.0F;
   }
};

/** \purpose
 * Verify the function Calc_Obj_Mov_Stat_Thresh is called by verifying that globals.obj_mov_stat_spd_thresh
 * is updated.
 * \req
 * NA
 */
TEST(Update_Global_Parameters, Check_Calc_Obj_Mov_Stat_Thresh_Is_Called)
{
   /** \precond
     * None.
     */

   /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Verify globals.obj_mov_stat_spd_thresh = 1.4
    */
   DOUBLES_EQUAL_TEXT(1.4F, globals.obj_mov_stat_spd_thresh, F360_EPSILON, "obj_mov_stat_spd_thresh was not updated as expected")
}
/** @}*/

/** \defgroup  Update_Global_Parameters__Calculate_Shrinked_FOV_Normals
 *  @{
 */

/** \brief
 * This test group checks the functionality of Calculate_Shrinked_FOV_Normals to ensure that it correctly
 * rotates the FoV normals (right/left_fov_normal_lr) found in sensor properties according to the calibration value
 * k_fov_normal_rotation_angle
 */
TEST_GROUP(Update_Global_Parameters__Calculate_Shrinked_FOV_Normals)
{
   F360_Host_T host = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calibrations = {};
   F360_Globals_T globals = {};
   const uint32_t valid_sensor_idx1 = 0;
   const uint32_t valid_sensor_idx2 = MAX_NUMBER_OF_SENSORS - 1;

   /** \setup
    * Set calibrations.k_fov_normal_rotation_angle to 0
    * For all sensors, set
    * - is_valid is valid to false
    * - left_fov_normal[F360_LOOK_ID_0] = 1.11F
    * - left_fov_normal[F360_LOOK_ID_1] = 1.12F
    * - right_fov_normal[F360_LOOK_ID_0] = 1.13F
    * - right_fov_normal[F360_LOOK_ID_1] = 1.14F
    * - left_fov_normal[F360_LOOK_ID_2] = 1.21F
    * - left_fov_normal[F360_LOOK_ID_3] = 1.22F
    * - right_fov_normal[F360_LOOK_ID_2] = 1.23F
    * - right_fov_normal[F360_LOOK_ID_3] = 1.24F
    * - rotated_left_fov_normal_lr[0] and rotated_left_fov_normal_lr[1] to 0.9
    * - rotated_right_fov_normal_lr[0] and rotated_right_fov_normal_lr[1] to 0.9
    */
   TEST_SETUP()
   {
      calibrations.k_fov_normal_rotation_angle = 0.0F;

      for (uint8_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
      {
         sensors[sensor_idx].variable.is_valid = false;
         sensors[sensor_idx].variable.look_id = F360_DET_LOOK_ID_INVALID;

         // Long range look 
         sensors[sensor_idx].constant.left_fov_normal[F360_DET_LOOK_ID_0] = 1.11F;
         sensors[sensor_idx].constant.left_fov_normal[F360_DET_LOOK_ID_1] = 1.12F;
         sensors[sensor_idx].constant.right_fov_normal[F360_DET_LOOK_ID_0] = 1.13F;
         sensors[sensor_idx].constant.right_fov_normal[F360_DET_LOOK_ID_1] = 1.14F;

         // Medium range look
         sensors[sensor_idx].constant.left_fov_normal[F360_DET_LOOK_ID_2] = 1.21F;
         sensors[sensor_idx].constant.left_fov_normal[F360_DET_LOOK_ID_3] = 1.22F;
         sensors[sensor_idx].constant.right_fov_normal[F360_DET_LOOK_ID_2] = 1.23F;
         sensors[sensor_idx].constant.right_fov_normal[F360_DET_LOOK_ID_3] = 1.24F;

         globals.rotated_left_fov_normal[sensor_idx][0] = 0.9F;
         globals.rotated_left_fov_normal[sensor_idx][1] = 0.9F;

         globals.rotated_right_fov_normal[sensor_idx][0] = 0.9F;
         globals.rotated_right_fov_normal[sensor_idx][1] = 0.9F;
      }
   }
};

/** \purpose
 * Verify that if no sensor is valid, no entry in globals is changed compared to the test setup
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Calculate_Shrinked_FOV_Normals, No_Valid_Sensors)
{
   /** \precond
    * None.
    */

   /** \action
    * Call Update_Global_Parameters
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Verify nothing has changed since the test setup
    */
      for (uint8_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
      {
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][0], F360_EPSILON, "This value was changed when it was expected no to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][1], F360_EPSILON, "This value was changed when it was expected no to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][0], F360_EPSILON, "This value was changed when it was expected no to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][1], F360_EPSILON, "This value was changed when it was expected no to")
      }
}

/** \purpose
 * Verify that only if a sensor is valid, the rotated FOV normals in globals are set to the
 * corresponding FOV normal in sensors during a long range look
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Calculate_Shrinked_FOV_Normals, Valid_Sensors_No_Rotation_Long_Range_Look)
{
   /** \precond
    * Set is_valid for sensor_idx 1 and 4 to true
    */
   const uint8_t valid_sensor_idx_1 = 1U;
   const uint8_t valid_sensor_idx_2 = 4U;
   sensors[valid_sensor_idx_1].variable.is_valid = true;
   sensors[valid_sensor_idx_2].variable.is_valid = true;
   sensors[valid_sensor_idx_1].variable.look_id = F360_DET_LOOK_ID_0; // Long range look
   sensors[valid_sensor_idx_2].variable.look_id = F360_DET_LOOK_ID_1;

   /** \action
    * Call Update_Global_Parameters
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Verify that sensor_idx 1 and 4,
    * - globals.rotated_left_fov_normal[sensor_idx][0]
    * - globals.rotated_left_fov_normal[sensor_idx][1]
    * - globals.rotated_right_fov_normal[sensor_idx][0]
    * - globals.rotated_right_fov_normal[sensor_idx][1]
    * are set to the same value as the corresponding normal in sensors.
    * For all other sensor_idx, the value should be unchanged from the setup.
    */
   for (uint8_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
   {
      if ((valid_sensor_idx_1 == sensor_idx) || ( valid_sensor_idx_2 == sensor_idx))
      {
         DOUBLES_EQUAL_TEXT(1.11F, globals.rotated_left_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.12F, globals.rotated_left_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.13F, globals.rotated_right_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.14F, globals.rotated_right_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
      }
      else
      {
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
      }
   }
}

/** \purpose
 * Verify that only if a sensor is valid, the rotated FOV normals in globals are set to the
 * corresponding FOV normal in sensors during a medium range look
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Calculate_Shrinked_FOV_Normals, Valid_Sensors_No_Rotation_Medium_Range_Look)
{
   /** \precond
    * Set is_valid for sensor_idx 1 and 4 to true
    */
   const uint8_t valid_sensor_idx_1 = 1U;
   const uint8_t valid_sensor_idx_2 = 4U;
   sensors[valid_sensor_idx_1].variable.is_valid = true;
   sensors[valid_sensor_idx_2].variable.is_valid = true;
   sensors[valid_sensor_idx_1].variable.look_id = F360_DET_LOOK_ID_2; // Medium range look
   sensors[valid_sensor_idx_2].variable.look_id = F360_DET_LOOK_ID_3;

   /** \action
    * Call Update_Global_Parameters
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Verify that sensor_idx 1 and 4,
    * - globals.rotated_left_fov_normal[sensor_idx][0]
    * - globals.rotated_left_fov_normal[sensor_idx][1]
    * - globals.rotated_right_fov_normal[sensor_idx][0]
    * - globals.rotated_right_fov_normal[sensor_idx][1]
    * are set to the same value as the corresponding normal in sensors.
    * For all other sensor_idx, the value should be unchanged from the setup.
    */
   for (uint8_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
   {
      if ((valid_sensor_idx_1 == sensor_idx) || ( valid_sensor_idx_2 == sensor_idx))
      {
         DOUBLES_EQUAL_TEXT(1.21F, globals.rotated_left_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.22F, globals.rotated_left_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.23F, globals.rotated_right_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(1.24F, globals.rotated_right_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
      }
      else
      {
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_left_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][0], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
         DOUBLES_EQUAL_TEXT(0.9F, globals.rotated_right_fov_normal[sensor_idx][1], F360_EPSILON, "This value was not set to the the same value as the corresponding value in sensor_props when it was expected to")
      }
   }
}

/** \purpose
 * Verify that if a sensor is valid and rotation is made, the rotated FOV normals in globals are set to the
 * correct values
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Calculate_Shrinked_FOV_Normals, Valid_Sensor_Rotation)
{
   /** \precond
    * calibrations.k_fov_normal_rotation_angle to rotate 30 degrees
    */
   calibrations.k_fov_normal_rotation_angle = F360_DEG2RAD(30.0F);

   uint8_t valid_sensor = 1U;
   sensors[valid_sensor].variable.is_valid = true;
   sensors[valid_sensor].constant.left_fov_normal[0] = 1.0F; // unit vector pointing vcs 0 deg
   sensors[valid_sensor].constant.left_fov_normal[1] = 0.0F;
   sensors[valid_sensor].constant.right_fov_normal[0] = 1.0F;
   sensors[valid_sensor].constant.right_fov_normal[1] = 0.0F;
   sensors[valid_sensor].variable.look_id = F360_DET_LOOK_ID_0; // Long range look

   const float32_t exp_left_fov_normal_lr_x = 0.866025403784439F; // Unit vector pointing vcs 30 deg
   const float32_t exp_left_fov_normal_lr_y = 0.5F;

   const float32_t exp_right_fov_normal_lr_x = 0.866025403784439F; // unit vector pointing vcs -30 deg
   const float32_t exp_right_fov_normal_lr_y = -0.5F;

   /** \action
    * Call Update_Global_Parameters
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Verify that globals.rotated_left_fov_normal[valid_sensor][0] is set to exp_left_fov_normal_lr_x
    * Verify that globals.rotated_left_fov_normal[valid_sensor][1] is set to exp_left_fov_normal_lr_y
    * Verify that globals.rotated_right_fov_normal[valid_sensor][0] is set to exp_right_fov_normal_lr_x
    * Verify that globals.rotated_right_fov_normal[valid_sensor][1] is set to exp_right_fov_normal_lr_y
    */
   DOUBLES_EQUAL_TEXT(exp_left_fov_normal_lr_x, globals.rotated_left_fov_normal[valid_sensor][0], F360_EPSILON, "The vector was not rotated as expected")
   DOUBLES_EQUAL_TEXT(exp_left_fov_normal_lr_y, globals.rotated_left_fov_normal[valid_sensor][1], F360_EPSILON, "The vector was not rotated as expected")
   DOUBLES_EQUAL_TEXT(exp_right_fov_normal_lr_x, globals.rotated_right_fov_normal[valid_sensor][0], F360_EPSILON, "The vector was not rotated as expected")
   DOUBLES_EQUAL_TEXT(exp_right_fov_normal_lr_y, globals.rotated_right_fov_normal[valid_sensor][1], F360_EPSILON, "The vector was not rotated as expected")

}
/** @}*/


/** \defgroup Update_Global_Parameters__Check_Sensor_Configuration
 *  @{
 */

/** \brief
 * This test group verifies the functionality of the function Check_Sensor_Configuration() to ensure that this function
 * only sets f_single_front_center_radar_only to true when sensors are set up such that it matches a
 * single sensor that is mounted at F360_MOUNTING_LOCATION_CENTER_FORWARD.
 */
TEST_GROUP(Update_Global_Parameters__Check_Sensor_Configuration)
{
   F360_Host_T host = {};
   F360_Calibrations_T calibrations = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};

   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Globals_T globals = {};

   /** \setup
    * Set is_valid flag for all sensors to false.
    * Set mounting location for all sensors to F360_MOUNTING_LOCATION_UNKNOWN
    * Set f_single_front_center_radar_only to false.
    */
   TEST_SETUP()
   {
      for (F360_Radar_Sensor_T& sensor : sensors)
      {
         sensor.variable.is_valid = false;
         sensor.constant.mounting_location = F360_MOUNTING_LOCATION_UNKNOWN;
      }
      globals.f_single_front_center_radar_only = false;
   }
};

/** \purpose
 * Verify Check_Sensor_Configuration doesn't set f_single_front_center_radar_only to true when there are no valid sensors
 * and none of the sensors are mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, No_Valid_Sensors_And_Not_Mounted_In_Center)
{
   /** \precond
    * None.
    */

   /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_FALSE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to true when it was expected to be false.")
}

/** \purpose
 * Verify Check_Sensor_Configuration doesn't set f_single_front_center_radar_only to true when there are no valid sensors
 * but all of the sensors are mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, No_Valid_Sensors_And_Mounted_In_Center)
{
   /** \precond
    * Set mounting_location to F360_MOUNTING_LOCATION_CENTER_FORWARD for all sensors in sensors.
    */
   for (F360_Radar_Sensor_T& sensor : sensors)
   {
      sensor.constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
   }

   /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_FALSE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to true when it was expected to be false.")
}

/** \purpose
 * Verify Check_Sensor_Configuration doesn't set f_single_front_center_radar_only to true when all sensors are valid and
 * all of the sensors are mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, All_Sensors_Valid_And_Mounted_In_Center)
{
   /** \precond
    * Set mounting_location to F360_MOUNTING_LOCATION_CENTER_FORWARD for all sensors in sensors.
    * Set is_valid to true for all sensors in sensors.
    */
   for (F360_Radar_Sensor_T& sensor : sensors)
   {
      sensor.constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
      sensor.variable.is_valid = true;
   }

    /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_FALSE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to true when it was expected to be false.")
}

/** \purpose
 * Verify Check_Sensor_Configuration doesn't set f_single_front_center_radar_only to true when all sensors are valid but 
 * none of the sensors are mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, All_Sensors_Valid_But_Not_Mounted_In_Center)
{
   /** \precond
    * Set is_valid to true for all sensors in sensors.
    */
   for (F360_Radar_Sensor_T& sensor : sensors)
   {
      sensor.variable.is_valid = true;
   }

    /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_FALSE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to true when it was expected to be false.")
}

/** \purpose
 * Verify Check_Sensor_Configuration do set f_single_front_center_radar_only to true when a single sensor is valid at index 0 and
 * it is mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, Single_Sensor_Valid_At_Idx0_And_Mounted_In_Center)
{
   /** \precond
    * Set is_valid to true and mounting location to F360_MOUNTING_LOCATION_CENTER_FORWARD for a single sensor in sensors.
    */
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
   sensors[0].variable.is_valid = true;

    /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_TRUE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to false when it was expected to be true.")
}

/** \purpose
 * Verify Check_Sensor_Configuration do set f_single_front_center_radar_only to true when a single sensor is valid at index 3 and
 * it is mounted in F360_MOUNTING_LOCATION_CENTER_FORWARD.
 * \req
 * NA
 */
TEST(Update_Global_Parameters__Check_Sensor_Configuration, Single_Sensor_Valid_At_Idx3_And_Mounted_In_Center)
{
   /** \precond
    * Set is_valid to true and mounting location to F360_MOUNTING_LOCATION_CENTER_FORWARD for a single sensor in sensors.
    */
   sensors[3].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
   sensors[3].variable.is_valid = true;

    /** \action
    * Call Update_Global_Parameters().
    */
   Update_Global_Parameters(host, sensors, calibrations, globals, timing_info);

   /** \result
    * Check that globals.f_single_front_center_radar_only is set to false
    */
   CHECK_TRUE_TEXT(globals.f_single_front_center_radar_only,"f_single_front_center_radar_only was set to false when it was expected to be true.")
}
/** @}*/
