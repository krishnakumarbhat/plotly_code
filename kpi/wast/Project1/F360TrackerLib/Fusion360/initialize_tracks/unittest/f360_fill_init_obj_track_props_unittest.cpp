/** \file
   File contains uts for Fill_Init_Obj_Track_Props function
*/

#include "f360_fill_init_obj_track_props.h"
#include "f360_trk_fltr_ccv_states.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_math_func.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

using namespace f360_variant_A;

/** \defgroup  f360_fill_init_obj_track_props
 *  @{
 */

/** \brief
 *  Test group for Fill_Init_Obj_Track_Props function
 */

TEST_GROUP(f360_fill_init_obj_track_props)
{
   /** \setup
   * Setting variables for fill init object track
   */
   F360_Globals_T globals = {};
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Object_Track_T obj_trk_to_init = {};
   F360_Tracker_Info_T tracker_info = {};
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};

   float32_t test_tolerance = 1.0e-6F;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      tracker_info.num_unique_objs = 1U;
      globals.f_single_front_center_radar_only = false;
      globals.obj_mov_stat_spd_thresh = 1.0F;
      globals.obj_vehicular_spd_thresh = 3.0F;
      host.dist_rear_axle_to_vcs_m = 4.0F;
   }
};


/**
*\purpose  Checking object's reference_point property assignment
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, Fill_Init_Obj_Track_Props__Reference_Point)
{
   /** \precond
    * Setting initialized object to be right in front of host with 0deg heading.
    * Expected reference_point is rear and object position should equal init_info position.
    */
   init_info.det_bbox.Set_Center(10.0F, 0.0F);
   init_info.det_bbox.Set_Length(1.0F);
   init_info.det_bbox.Set_Width(1.0F);
   init_info.VCS_velocity.longitudinal = 5.0F;
   init_info.VCS_velocity.lateral = 0.0F;

   float32_t expected_vcs_position_x = 9.5;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    * Object reference_point should be REAR
	* Object position should be same as in init_info
    */
   CHECK_EQUAL_TEXT(F360_REFERENCE_POINT_REAR, obj_trk_to_init.reference_point, "Object reference_point is wrong!");
   DOUBLES_EQUAL_TEXT(expected_vcs_position_x, obj_trk_to_init.vcs_position.x, test_tolerance, "Object longitudinal position is wrong!");
   DOUBLES_EQUAL_TEXT(init_info.det_bbox.Get_Center().y, obj_trk_to_init.vcs_position.y, test_tolerance, "Object lateral position is wrong!");
}


/**
*\purpose  Checking object's on_sep_id property assignment
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, Fill_Init_Obj_Track_Props__Object_On_SEP)
{
   /** \precond
    * Setting SEP id 1 as a valid SEP, straight line with a valid interval
    * Setting initialized object at same lateral position as SEP id 1
    */
   sep[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   sep[0].lower_limit = -50.0F;
   sep[0].upper_limit = 50.0F;
   sep[0].p0 = 10.0F;
   sep[0].p1 = 0.0F;
   sep[0].p2 = 0.0F;

   init_info.det_bbox.Set_Center(10.0F, sep[0].p0);
   init_info.frac_az_error_stat_mov = 0.01;
   init_info.VCS_velocity.longitudinal = 5.0F;
   init_info.VCS_velocity.lateral = 0.0F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    * Object should be flagged as on SEP and not behind SEP
    */
   CHECK_EQUAL(1U, obj_trk_to_init.on_sep_id);
   CHECK_EQUAL(F360_INVALID_UNSIGNED_ID, obj_trk_to_init.behind_sep_id);
}

/**
*\purpose  Checking object's behind_sep_id property assignment
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, Fill_Init_Obj_Track_Props__Object_Behind_SEP)
{
   /** \precond
    * Setting SEP id 1 as a valid SEP, straight line with a valid interval
    * Setting initialized object greater than lateral position as SEP id 1
    * Setting initialized object speed arbitrary large to yield CTCA type
    */
   sep[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   sep[0].lower_limit = -50.0F;
   sep[0].upper_limit = 50.0F;
   sep[0].p0 = 10.0F;
   sep[0].p1 = 0.0F;
   sep[0].p2 = 0.0F;

   init_info.det_bbox.Set_Center(10.0F, sep[0].p0 + 2.0F);
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 0.01;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    * Object should be flagged as behind SEP and not on SEP
    */
   CHECK_EQUAL(1U, obj_trk_to_init.behind_sep_id);
   CHECK_EQUAL(F360_INVALID_UNSIGNED_ID, obj_trk_to_init.on_sep_id);
}

/**
*\purpose  Checking object's filter type assigment
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestCalcInitObjTrackFilterTypeCCA)
{
   /** \precond
    * Setting variables for object's filter type check
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 0.1;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_EQUAL(F360_TRACKER_TRKFLTR_CCA, obj_trk_to_init.trk_fltr_type);
}

/**
*\purpose  Checking object's existence probability calculation
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestCalcInitObjExistProb)
{
   /** \precond
    * Setting variables for object's existence probability check
    */
   init_info.VCS_velocity_plausability = 0.4F;
   cluster_to_init.exist_prob = 0.8F;

   float32_t expected_exist_prob = 0.512F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   DOUBLES_EQUAL(expected_exist_prob, obj_trk_to_init.exist_prob, test_tolerance);
}

/**
*\purpose  Checking object's vehicular info update 
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestUpdateObjVeicularInfoObjMovingHighSpeed)
{
   /** \precond
    * Setting variables for object's vehicular info update 
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 0.0;
   init_info.frac_az_error_stat_mov = 0.01;

   obj_trk_to_init.time_since_vehicle_init = -1.0;

   float32_t expected_time_since_vehicle_init = 0.0F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_TRUE(obj_trk_to_init.f_vehicular_trk);
   DOUBLES_EQUAL(expected_time_since_vehicle_init, obj_trk_to_init.time_since_vehicle_init, test_tolerance);
}

/**
*\purpose  Checking object's vehicular info update
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestUpdateObjVeicularInfoObjMovingLowSpeed)
{
   /** \precond
    * Setting variables for object's vehicular info update
    */
   init_info.VCS_velocity.longitudinal = 2.0;
   init_info.VCS_velocity.lateral = 0.0;
   init_info.frac_az_error_stat_mov = 0.01;

   obj_trk_to_init.time_since_vehicle_init = -1.0;

   float32_t expected_time_since_vehicle_init = -1.0F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_vehicular_trk);
   DOUBLES_EQUAL(expected_time_since_vehicle_init, obj_trk_to_init.time_since_vehicle_init, test_tolerance);
}

/**
*\purpose  Checking object's vehicular info update
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestUpdateObjVeicularInfoObjNotMovingHighSpeed)
{
   /** \precond
    * Setting variables for object's vehicular info update
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 0.0;
   init_info.frac_az_error_stat_mov = 10.0;

   obj_trk_to_init.time_since_vehicle_init = -1.0;

   float32_t expected_time_since_vehicle_init = -1.0F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_vehicular_trk);
   DOUBLES_EQUAL(expected_time_since_vehicle_init, obj_trk_to_init.time_since_vehicle_init, test_tolerance);
}

/**
*\purpose  Checking object's vehicular info update
*\req     NA
*/
TEST(f360_fill_init_obj_track_props, TestUpdateObjVeicularInfoObjNotMovingLowSpeed)
{
   /** \precond
    * Setting variables for object's vehicular info update
    */
   init_info.VCS_velocity.longitudinal = 0.1;
   init_info.VCS_velocity.lateral = 0.0;
   init_info.frac_az_error_stat_mov = 10.0;

   obj_trk_to_init.time_since_vehicle_init = -1.0;

   float32_t expected_time_since_vehicle_init = -1.0F;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_vehicular_trk);
   DOUBLES_EQUAL(expected_time_since_vehicle_init, obj_trk_to_init.time_since_vehicle_init, test_tolerance);
}

/** \defgroup  f360_fill_init_obj_track_props_determine_initial_size
 *  @{
 */

/** \brief
 *  Test group for Determine_Initial_Size function
 */
TEST_GROUP(f360_fill_init_obj_track_props_determine_initial_size)
{
   /** \setup
   * Setting variables for Determine_Initial_Size
   */
   F360_Globals_T globals = {};
   F360_Calibrations_T calibrations = {};

   float32_t TEST_PASS_TH = 1e-6F;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      globals.obj_mov_stat_spd_thresh = 1.0F;
   }
};

/**
*\purpose  Test that initial object size is correctly determined for a fast moving object, which is marked as valid for liberal tracking.
*\req   N/A
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size, Test_Initial_Size_Fast_Moving_Object_Valid_For_Liberal_Tracking)
{
   /** \precond
   * Set speed above fast moving threshold.
   * Expected length is 6 m and width 2 m.
   * Expected len1 and len2 is half length and expected ref_point_long_offset_ratio is therefor 0.5.
   * Expected wid1 and wid 2 is half width and expected ref_point_lat_offset_ratio is therefor 0.5.
   * 
   */
   float32_t longitudinal_velocity = 5.0F;
   float32_t lateral_velocity = 3.0F;
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = F360_Get_Hypotenuse(longitudinal_velocity, lateral_velocity);
   obj_trk_to_init.f_valid_for_liberal_tracking = true;

   const float32_t expected_length = calibrations.liberal_tracking_initial_length;
   const float32_t expected_width = calibrations.k_init_trk_fast_moving_width;

   /** \action
   * Call Determine_Initial_Size
   */
   Determine_Initial_Size(globals, calibrations, obj_trk_to_init);

   /** \result
   * Check that length and width are determined correctly.
   */
   DOUBLES_EQUAL_TEXT(expected_length, obj_trk_to_init.bbox.Get_Length(), TEST_PASS_TH, "Length was incorrect.");
   DOUBLES_EQUAL_TEXT(expected_width, obj_trk_to_init.bbox.Get_Width(), TEST_PASS_TH, "Width was incorrect."); 
}

/**
*\purpose  Test that initial object size is correctly determined for a fast moving object, which is not marked as valid for liberal tracking.
*\req   N/A
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size, Test_Initial_Size_Fast_Moving_Object_Not_Valid_For_Liberal_Tracking)
{
   /** \precond
   * Set speed above fast moving threshold.
   * Expected length is calibrations.k_init_trk_fast_moving_length and expected width is calibrations.k_init_trk_fast_moving_width.
   * Expected len1 and len2 is half length and expected ref_point_long_offset_ratio is therefor 0.5.
   * Expected wid1 and wid2 is half width and expected ref_point_lat_offset_ratio is therefor 0.5.
   * 
   */
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = calibrations.k_init_trk_fast_moving_thresh + 0.01F;
   obj_trk_to_init.f_valid_for_liberal_tracking = false;

   const float32_t expected_length = calibrations.k_init_trk_fast_moving_length;
   const float32_t expected_width = calibrations.k_init_trk_fast_moving_width;

   /** \action
   * Call Determine_Initial_Size
   */
   Determine_Initial_Size(globals, calibrations, obj_trk_to_init);

   /** \result
   * Check that length and width are determined correctly.
   */
   DOUBLES_EQUAL_TEXT(expected_length, obj_trk_to_init.bbox.Get_Length(), TEST_PASS_TH, "Length was incorrect.");
   DOUBLES_EQUAL_TEXT(expected_width, obj_trk_to_init.bbox.Get_Width(), TEST_PASS_TH, "Width was incorrect.");
}

/**
*\purpose  Test that initial object size is correctly determined for a slow moving object.
*\req   N/A
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size, Test_Initial_Size_Slow_Moving_Object)
{
   /** \precond
   * Set speed above stationary threshold.
   * Expected length is calibrations.k_init_trk_slow_moving_length and expected width is calibrations.k_init_trk_slow_moving_width.
   * Expected len1 and len2 is half length and expected ref_point_long_offset_ratio is therefor 0.5.
   * Expected wid1 and wid2 is half width and expected ref_point_lat_offset_ratio is therefor 0.5.
   * 
   */
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = globals.obj_mov_stat_spd_thresh + 0.01F;

   const float32_t expected_length = calibrations.k_init_trk_slow_moving_length;
   const float32_t expected_width = calibrations.k_init_trk_slow_moving_width;

   /** \action
   * Call Determine_Initial_Size
   */
   Determine_Initial_Size(globals, calibrations, obj_trk_to_init);

   /** \result
   * Check that length and width are determined correctly.
   */
   DOUBLES_EQUAL_TEXT(expected_length, obj_trk_to_init.bbox.Get_Length(), TEST_PASS_TH, "Length was incorrect.");  
   DOUBLES_EQUAL_TEXT(expected_width, obj_trk_to_init.bbox.Get_Width(), TEST_PASS_TH, "Width was incorrect."); 
}

/**
*\purpose  Test that initial object size is correctly determined for a stationary object.
*\req   N/A
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size, Test_Initial_Size_Stationary_Moving_Object)
{
   /** \precond
   * Set speed below stationary threshold.
   * Expected length is calibrations.k_init_trk_stationary_length and expected width is calibrations.k_init_trk_stationary_width.
   * Expected len1 and len2 is half length and expected ref_point_long_offset_ratio is therefor 0.5.
   * Expected wid1 and wid2 is half width and expected ref_point_lat_offset_ratio is therefor 0.5.
   * 
   */
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = globals.obj_mov_stat_spd_thresh - 0.01F;

   const float32_t expected_length = calibrations.k_init_trk_stationary_length;
   const float32_t expected_width = calibrations.k_init_trk_stationary_width;

   /** \action
   * Call Determine_Initial_Size
   */
   Determine_Initial_Size(globals, calibrations, obj_trk_to_init);

   /** \result
   * Check that length and width are determined correctly.
   */
   DOUBLES_EQUAL_TEXT(expected_length, obj_trk_to_init.bbox.Get_Length(), TEST_PASS_TH, "Length was incorrect.");
   DOUBLES_EQUAL_TEXT(expected_width, obj_trk_to_init.bbox.Get_Width(), TEST_PASS_TH, "Width was incorrect."); 
}

/** @}*/

/** \brief
*  Test group for Calc_Init_Position_And_Size function
**/
TEST_GROUP(f360_Calc_Init_Position_And_Size)
{
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Host_T host = {};
   F360_Calibrations_T calibrations = {};
   F360_Globals_T globals = {};
   F360_Object_Track_T obj_to_init = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   float32_t tolerance = 1e-4F;

   F360_Reference_Point_T exp_ref_point;
   float32_t exp_len;
   float32_t exp_wid;
   float32_t exp_pos_x;
   float32_t exp_pos_y;

   /** \setup
   * Set up an object with
   *  - f_movable = true,
   *  - speed above k_init_trk_fast_moving_thresh
   * Set up Init_info bounding box with:
   *  - Widht 2m
   *  - Length 5m
   *  - Orientation 0 rad
   *  - Center position at (12, 5) 
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);

      obj_to_init.f_moveable = true;
      globals.f_single_front_center_radar_only = false;
      globals.obj_mov_stat_spd_thresh = 5.0F;
      obj_to_init.speed = globals.obj_mov_stat_spd_thresh + 0.1F;

      init_info.det_bbox.Set_Center(12.0F, 5.0F);
      init_info.det_bbox.Set_Length(5.0F);
      init_info.det_bbox.Set_Width(2.0F);
      init_info.det_bbox.Set_Orientation(0.0F);
   }
};

/**
*\purpose  Check that correct initial position and size is assigned to a movable object
*\req    Na
*/
TEST(f360_Calc_Init_Position_And_Size, Init_Pos_And_Size_Movable_Obj)
{
   /** \precond
    * Init info and a movable object has been set up with detections in TEST_SETUP
    * Set expected output accordingly
    */
   cluster_to_init.vcs_position.x = 9.5F;
   cluster_to_init.vcs_position.y = 4.0F;
   exp_ref_point = F360_REFERENCE_POINT_REAR_LEFT;
   exp_pos_x = cluster_to_init.vcs_position.x;
   exp_pos_y = cluster_to_init.vcs_position.y;
   exp_len = calibrations.k_init_trk_slow_moving_length;
   exp_wid = calibrations.k_init_trk_slow_moving_width;



   /** \action
    * Call Calc_Init_Position_And_Size
    */
   Calc_Init_Position_And_Size(init_info, host, calibrations, globals, sensors, cluster_to_init, det_props, obj_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_TRUE(exp_ref_point == obj_to_init.reference_point);
   DOUBLES_EQUAL(exp_len, obj_to_init.bbox.Get_Length(), tolerance);
   DOUBLES_EQUAL(exp_wid, obj_to_init.bbox.Get_Width(), tolerance);
   DOUBLES_EQUAL(exp_pos_x, obj_to_init.vcs_position.x, tolerance);
   DOUBLES_EQUAL(exp_pos_y, obj_to_init.vcs_position.y, tolerance);
}

/**
*\purpose  Check that correct initial position and size is assigned to a non-movable object
*\req    Na
*/
TEST(f360_Calc_Init_Position_And_Size, Init_Pos_And_Size_Non_Movable_Obj)
{
   /** \precond
    * Init info and a movable object has been set up with detections in TEST_SETUP
    * Set f_movable to false and speed to 0.1 m/s
    * Set expected output accordingly
    */
   obj_to_init.f_moveable = false;
   obj_to_init.speed = 0.1F;

   exp_ref_point = F360_REFERENCE_POINT_CENTER;
   const F360_Reference_Point_T exp_min_proj_ref_point = F360_REFERENCE_POINT_CENTER;
   cluster_to_init.vcs_position.x = 12.0F;
   cluster_to_init.vcs_position.y = 5.0F;
   exp_pos_x = cluster_to_init.vcs_position.x;
   exp_pos_y = cluster_to_init.vcs_position.y;
   exp_len = calibrations.k_nonmoveable_target_diameter;
   exp_wid = calibrations.k_nonmoveable_target_diameter;

   /** \action
    * Call Calc_Init_Position_And_Size
    */
   Calc_Init_Position_And_Size(init_info, host, calibrations, globals, sensors, cluster_to_init, det_props, obj_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_TRUE(exp_ref_point == obj_to_init.reference_point);
   CHECK_TRUE(exp_min_proj_ref_point == obj_to_init.min_projection_reference_point);
   DOUBLES_EQUAL(exp_len, obj_to_init.bbox.Get_Length(), tolerance);
   DOUBLES_EQUAL(exp_wid, obj_to_init.bbox.Get_Width(), tolerance);
   DOUBLES_EQUAL(exp_pos_x, obj_to_init.vcs_position.x, tolerance);
   DOUBLES_EQUAL(exp_pos_y, obj_to_init.vcs_position.y, tolerance);
}


/**
*\purpose  Check that correct initial position and size is assigned to a non-movable object
*\req    Na
*/
TEST(f360_Calc_Init_Position_And_Size, Init_Pos_And_Size_Non_Movable_Obj_Det_Not_In_Gating)
{
   /** \precond
    * Init info and a movable object has been set up with detections in TEST_SETUP
    * Set f_movable to false and speed to 0.1 m/s
    * Set expected output accordingly
    */
   obj_to_init.f_moveable = false;
   obj_to_init.speed = 0.1F;

   exp_ref_point = F360_REFERENCE_POINT_CENTER;
   const F360_Reference_Point_T exp_min_proj_ref_point = F360_REFERENCE_POINT_CENTER;
   cluster_to_init.vcs_position.x = 12.0F;
   cluster_to_init.vcs_position.y = 5.0F;
   exp_pos_x = 0.1275916F;
   exp_pos_y = -0.3965492F;
   exp_len = calibrations.k_nonmoveable_target_diameter;
   exp_wid = calibrations.k_nonmoveable_target_diameter;
   cluster_to_init.ndets = 2;
   cluster_to_init.detids[0] = 1;
   cluster_to_init.detids[1] = 2;
   det_props[0].vcs_position.Set_Position(0.1F, 0.3F);
   det_props[1].vcs_position.Set_Position(-0.1F, -0.5F);

   /** \action
    * Call Calc_Init_Position_And_Size
    */
   Calc_Init_Position_And_Size(init_info, host, calibrations, globals, sensors, cluster_to_init, det_props, obj_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_TRUE(exp_ref_point == obj_to_init.reference_point);
   CHECK_TRUE(exp_min_proj_ref_point == obj_to_init.min_projection_reference_point);
   DOUBLES_EQUAL(exp_len, obj_to_init.bbox.Get_Length(), tolerance);
   DOUBLES_EQUAL(exp_wid, obj_to_init.bbox.Get_Width(), tolerance);
   DOUBLES_EQUAL(exp_pos_x, obj_to_init.vcs_position.x, tolerance);
   DOUBLES_EQUAL(exp_pos_y, obj_to_init.vcs_position.y, tolerance);
}

/** @}*/
