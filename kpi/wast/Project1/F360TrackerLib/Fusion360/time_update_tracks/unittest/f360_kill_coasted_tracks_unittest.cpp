/** \file
 * This file contains unit tests for content of f360_kill_coasted_tracks.cpp file
 */

#include "f360_kill_coasted_tracks.h"
#include "rspp_calibrations.h"
#include "f360_radar_sensor_props.h"
#include "f360_internal_preprocessing.h"
#include <CppUTest/TestHarness.h>
#include <iostream>

using namespace f360_variant_A;

/** \defgroup  f360_kill_coasted_tracks
 *  @{
 */

/** \brief
 * 
 * The group populates variables used for testing. Sensor mounting location is set as center forward. Azimuth angle, latitude and longitude
 * are set  
 */
TEST_GROUP(f360_kill_coasted_tracks)
{
   F360_Object_Track_T object_track;
   F360_Globals_T globals;
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Calibrations_T calibrations;
   RSPP_Calibrations_T rspp_calib{};
   const float32_t azim_th = F360_PI / 4;
   
   /** \setup
    * Setting up a single front sensor in the first cell of the array containing all sensors.
    * Setting the global variable that would indicate that there is only one sensor present and it is the FLR sensor.
    * Setting the object to fulfill all conditions to be marked for removal.
    */
   TEST_SETUP()
   {
      Initialize_RSPP_Calibrations(rspp_calib);
      sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
      sensors[0].constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      sensors[0].constant.mounting_position.vcs_position.longitudinal = -5.5F;
      sensors[0].constant.mounting_position.vcs_position.lateral = 0.0F;

      for (int i = 0; i < 4; i++)
      {
         sensors[0].constant.fov_min_az_rad[i] = -azim_th;
         sensors[0].constant.fov_max_az_rad[i] = azim_th;
      }

      const float min_fov_az_angle_lr = std::min(sensors[0].constant.fov_min_az_rad[F360_DET_LOOK_ID_0], sensors[0].constant.fov_min_az_rad[F360_DET_LOOK_ID_1]);
      const float min_fov_az_interior_angle_lr = std::max(min_fov_az_angle_lr, -rspp_calib.fov_interior_limit);
      const float max_fov_az_angle_lr = std::max(sensors[0].constant.fov_max_az_rad[F360_DET_LOOK_ID_0], sensors[0].constant.fov_max_az_rad[F360_DET_LOOK_ID_1]);
      const float max_fov_az_interior_angle_lr = std::min(max_fov_az_angle_lr, rspp_calib.fov_interior_limit);
      const float min_fov_az_angle_mr = std::min(sensors[0].constant.fov_min_az_rad[F360_DET_LOOK_ID_2], sensors[0].constant.fov_min_az_rad[F360_DET_LOOK_ID_3]);
      const float min_fov_az_interior_angle_mr = std::max(min_fov_az_angle_mr, -rspp_calib.fov_interior_limit);
      const float max_fov_az_angle_mr = std::max(sensors[0].constant.fov_max_az_rad[F360_DET_LOOK_ID_2], sensors[0].constant.fov_max_az_rad[F360_DET_LOOK_ID_3]);
      const float max_fov_az_interior_angle_mr = std::min(max_fov_az_angle_mr, rspp_calib.fov_interior_limit);

      sensors[0].constant.interior_fov[F360_DET_LOOK_ID_0] = min_fov_az_interior_angle_lr;
      sensors[0].constant.interior_fov[F360_DET_LOOK_ID_1] = max_fov_az_interior_angle_lr;
      sensors[0].constant.interior_fov[F360_DET_LOOK_ID_2] = min_fov_az_interior_angle_mr;
      sensors[0].constant.interior_fov[F360_DET_LOOK_ID_3] = max_fov_az_interior_angle_mr;

      const float min_fov_vcs_az_angle_lr = sensors[0].constant.mounting_position.vcs_boresight_azimuth_angle + min_fov_az_interior_angle_lr;
      const float max_fov_vcs_az_angle_lr = sensors[0].constant.mounting_position.vcs_boresight_azimuth_angle + max_fov_az_interior_angle_lr;
      const float min_fov_vcs_az_angle_mr = sensors[0].constant.mounting_position.vcs_boresight_azimuth_angle + min_fov_az_interior_angle_mr;
      const float max_fov_vcs_az_angle_mr = sensors[0].constant.mounting_position.vcs_boresight_azimuth_angle + max_fov_az_interior_angle_mr;

      sensors[0].constant.left_fov_normal[F360_DET_LOOK_ID_0] = -F360_Sinf(min_fov_vcs_az_angle_lr);
      sensors[0].constant.left_fov_normal[F360_DET_LOOK_ID_1] = F360_Cosf(min_fov_vcs_az_angle_lr);
      sensors[0].constant.right_fov_normal[F360_DET_LOOK_ID_0] = F360_Sinf(max_fov_vcs_az_angle_lr);
      sensors[0].constant.right_fov_normal[F360_DET_LOOK_ID_1] = -F360_Cosf(max_fov_vcs_az_angle_lr);
      sensors[0].constant.left_fov_normal[F360_DET_LOOK_ID_2] = -F360_Sinf(min_fov_vcs_az_angle_mr);
      sensors[0].constant.left_fov_normal[F360_DET_LOOK_ID_3] = F360_Cosf(min_fov_vcs_az_angle_mr);
      sensors[0].constant.right_fov_normal[F360_DET_LOOK_ID_2] = F360_Sinf(max_fov_vcs_az_angle_mr);
      sensors[0].constant.right_fov_normal[F360_DET_LOOK_ID_3] = -F360_Cosf(max_fov_vcs_az_angle_mr);

      Copy_Signals_From_Sensors_to_Sensor_Props(sensor_props, sensors);

      globals.f_single_front_center_radar_only = true;

      object_track.status = F360_OBJECT_STATUS_COASTED;
      object_track.f_moveable = false;
      object_track.time_since_stage_start = calibrations.k_max_coast_time_outside_fov + 0.005F;
      object_track.vcs_position = Point(0.0F, 10.0F);

   }
};

/** \purpose  
 * The test verifies that movable objects are not considered for removal outside FOV.
 * \req
 * NA
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_moving)
{
   /** \precond
    * Declare the test object as movable.
    * Set the result variable to the answer opposite to the correct one.
    */
   object_track.f_moveable = true;
   bool result = true;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the tested object is movable.
    */
   CHECK_FALSE(result);
}

/** \purpose  
 * This test verified that the non movable object that has been outside FLR FOV not long enough 
 * is not designated for removal.
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_insufficient_time)
{
   /** \precond
    * Set the time since the state change to a values that is less than the one that
    * would indicate that the object has not yet been outside FOV long enough.
    * Set the result variable to the answer opposite to the correct one.
    */
   object_track.time_since_stage_start = calibrations.k_max_coast_time_outside_fov - 0.01F;
   bool result = true;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the tested object has not yet been outside FOV long enough.
    */
   CHECK_FALSE(result);
}

/** \purpose  
 * This test verifies that the tested function returns false object status is not "coasted".
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_not_coasted)
{
   /** \precond
    * Set invalid object status (something other than F360_OBJECT_STATUS_COASTED)
    * Set the result variable to the answer opposite to the correct one.
    */
   object_track.status = F360_OBJECT_STATUS_NEW;
   bool result = true;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the tested object is not coasting.
    */
   CHECK_FALSE(result);
}

/** \purpose  
 * This test verifies that the tested function returns false since the object is inside FOV so
 * should not be evaluated.
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_inside_fov)
{
   /** \precond
    * Set the tested object's position so that the object is inside FOV
    * Set the result variable to the answer opposite to the correct one.
    */
   object_track.vcs_position = Point(10.0F, 0.0F);
   bool result = true;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the tested object is inside FOV.
    */
   CHECK_FALSE(result);
}

/** \purpose  
 * This test verifies that the tested function returns true since the object is outside FOV so
 * should be evaluated.
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_outside_fov)
{
   /** \precond
    * Set the result variable to the answer opposite to the correct one.
    */
   bool result = false;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "true" since the tested object fulfills all conditions to be dropped.
    */
   CHECK_TRUE(result);
}

/** \purpose  
 * This test verifies that the tested function returns false since there is no front radar.
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_not_front_sensor)
{
   /** \precond
    * Set the only sensor's position as an incorrect one. 
    * Set the result variable to the answer opposite to the correct one.
    */
   bool result = true;
   sensors[0].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the tested object has not yet been outside FOV long enough.
    */
   CHECK_FALSE(result);
}

/** \purpose  
 * This test verifies that the tested function returns false since the global variable is false.
 * \req
 * NA.
 */
TEST(f360_kill_coasted_tracks, Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV_false_global_front_only)
{
   /** \precond
    * Set the global variable indicating that there is only one single front center radar and false. 
    * Set the result variable to the answer opposite to the correct one.
    */
   globals.f_single_front_center_radar_only = false;
   bool result = true;
	
   /** \action
    * Call the tested function Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV.
    */
   result = Is_Non_Movable_Coasting_Obj_Outside_Front_Only_FOV(object_track, globals, sensors, calibrations);

   /** \result
    * The tested function is expected to return "false" since the this is not front radar only set up.
    */
   CHECK_FALSE(result);
}

/** @}*/
