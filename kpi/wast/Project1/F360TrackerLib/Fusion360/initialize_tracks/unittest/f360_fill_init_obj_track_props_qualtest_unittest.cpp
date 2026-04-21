/** \file
   File with set of qualification tests (which are also unit test) for Fill_Init_Obj_Track_Props function.
*/

#include "f360_fill_init_obj_track_props.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_current_msmt_type.h"
#include "f360_math_func.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

using namespace f360_variant_A;

/** \defgroup  f360_fill_init_obj_track_props_qualtest
 *  @{
 */

/** \brief
 *  Test group for Fill_Init_Obj_Track_Props function
 */

TEST_GROUP(f360_fill_init_obj_track_props_qualtest)
{
   /** \setup
   * Setting variables for fill init object track
   */
   F360_Globals_T globals = {};
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Object_Track_T obj_trk_to_init = {};
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   float32_t initial_state_error_cov[6][6];

   float32_t test_tolerance = 1.0e-6F;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      tracker_info.num_unique_objs = 1U;
      globals.obj_mov_stat_spd_thresh = 1.0F;
      globals.obj_vehicular_spd_thresh = 3.0F;
      globals.f_single_front_center_radar_only = false;
      host.dist_rear_axle_to_vcs_m = 4.0F;
   }
};

/**
*\purpose  Checking object's f_moving flag assigment
*\req     FTCP-12405
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjTrackMovingFlagMoving)
{
   /** \precond
    * Setting variables for object's speed check
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 0.01;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_TRUE(obj_trk_to_init.f_moving);
}

/**
*\purpose  Checking object's f_moving flag assigment
*\req     FTCP-12405
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjTrackMovingFlagMovingValidForLiberalTracking)
{
   /** \precond
   * Setting variables for object's speed check
   */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 0.01;
   init_info.f_valid_for_liberal_tracking = true;

   /** \action
   * Call Fill_Init_Obj_Track_Props
   */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
   * Checking if results match expectations
   */
   CHECK_TRUE(obj_trk_to_init.f_moving);
}

/**
*\purpose  Checking object's f_moving flag assigment
*\req     FTCP-12405
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjTrackMovingFlagNotMovingHighFracAz)
{
   /** \precond
    * Setting variables for object's speed check
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 10.0;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_moving);
}

/**
*\purpose  Checking object's f_moving flag assigment
*\req     FTCP-12405
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjTrackMovingFlagNotMovingLowSpeed)
{
   /** \precond
    * Setting variables for object's speed check
    */
   init_info.VCS_velocity.longitudinal = 0.1;
   init_info.VCS_velocity.lateral = 0.1;
   init_info.frac_az_error_stat_mov = 0.1;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_moving);
}

/**
*\purpose  Checking object's f_moving flag assigment
*\req     FTCP-12405
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjTrackMovingFlagNotMovingLowSpeedAndHighFracAz)
{
   /** \precond
    * Setting variables for object's speed check
    */
   init_info.VCS_velocity.longitudinal = 0.1;
   init_info.VCS_velocity.lateral = 0.1;
   init_info.frac_az_error_stat_mov = 10.0;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_FALSE(obj_trk_to_init.f_moving);
}

/**
*\purpose  Checking object's VCS heading calculation
*\req     FTCP-12402
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjVCSHeadingMovingObj)
{
   /** \precond
    * Setting variables for object's heading check
    */
   init_info.VCS_velocity.longitudinal = 0.0;
   init_info.VCS_velocity.lateral = 10.0;
   init_info.frac_az_error_stat_mov = 0.05;

   float32_t expected_heading = F360_PI_2;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   DOUBLES_EQUAL(expected_heading, obj_trk_to_init.vcs_heading.Value(), test_tolerance);
}

/**
*\purpose  Checking object's VCS heading calculation
*\req     FTCP-12402
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestCalcInitObjVCSHeadingNotMovingObj)
{
   /** \precond
    * Setting variables for object's heading check
    */
   init_info.VCS_velocity.longitudinal = 10.0;
   init_info.VCS_velocity.lateral = 0.0;
   init_info.frac_az_error_stat_mov = 10.0;

   float32_t expected_heading = 0.0;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   DOUBLES_EQUAL(expected_heading, obj_trk_to_init.vcs_heading.Value(), test_tolerance);
}

/**
*\purpose  Checking object's vehicular info update
*\req     FTCP-12404, FTCP-12402 // TOD0 during industrialization: Need to check if these reqs must be updated
*/
TEST(f360_fill_init_obj_track_props_qualtest, TestFillInitObjTrackPropsNominal)
{
   /** \precond
    * Setting variables for object's vehicular info update
    */
   cluster_to_init.exist_prob = 0.9F;
   cluster_to_init.priority = 0.9F;
   cluster_to_init.time_since_created = 0.3F;
   cluster_to_init.time_since_measurement = 0.6F;

   init_info.init_scheme = F360_TRACK_NEES_CV_M_ESTIMATOR;
   init_info.init_vel_source = F360_NEES_CFMI_VEL_HYP_SOURCE_CLOUD;
   init_info.det_bbox.Set_Center(20.0F, 0.0F);
   init_info.det_bbox.Set_Length(1.0F);
   init_info.det_bbox.Set_Width(1.0F);
   init_info.VCS_velocity.longitudinal = 10.0F;
   init_info.VCS_velocity.lateral = 0.0F;
   init_info.frac_az_error_stat_mov = 0.01F;

   obj_trk_to_init.time_since_vehicle_init = -1.0F;

   float32_t expected_time_since_vehicle_init = 0.0F;
   float32_t expected_vcs_position_x = 19.5;

   /** \action
    * Call Fill_Init_Obj_Track_Props
    */
   Fill_Init_Obj_Track_Props(globals, calibrations, host, init_info, cluster_to_init, sep, tracker_info.num_unique_objs, sensors, det_props, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   CHECK_EQUAL(1U, obj_trk_to_init.unique_id);
   CHECK_EQUAL(F360_MSMT_TYPE_DETS_ONLY, obj_trk_to_init.current_msmt_type);
   CHECK_EQUAL(F360_OBJECT_STATUS_NEW_UPDATED, obj_trk_to_init.status);
   DOUBLES_EQUAL(calibrations.k_init_trk_fast_moving_length, obj_trk_to_init.bbox.Get_Length(), test_tolerance);
   DOUBLES_EQUAL(calibrations.k_init_trk_fast_moving_width, obj_trk_to_init.bbox.Get_Width(), test_tolerance);
   CHECK_EQUAL(F360_REFERENCE_POINT_REAR, obj_trk_to_init.reference_point);
   DOUBLES_EQUAL(calibrations.k_init_trk_init_confidence, obj_trk_to_init.raw_confidence_level, test_tolerance);
   DOUBLES_EQUAL(calibrations.k_init_trk_init_confidence, obj_trk_to_init.confidenceLevel, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.hdg_ptng_disagmt, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.time_since_stage_start, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.time_since_initialization, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.time_since_track_updated, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.vcs_accel.longitudinal, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.vcs_accel.lateral, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.tang_accel, test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.curvature, test_tolerance);
   DOUBLES_EQUAL(cluster_to_init.priority, obj_trk_to_init.priority, test_tolerance);
   DOUBLES_EQUAL(cluster_to_init.time_since_created, obj_trk_to_init.time_since_cluster_created, test_tolerance);
   DOUBLES_EQUAL(cluster_to_init.time_since_measurement, obj_trk_to_init.time_since_measurement, test_tolerance);
   DOUBLES_EQUAL(init_info.init_scheme, obj_trk_to_init.init_scheme, test_tolerance);
   DOUBLES_EQUAL(init_info.init_vel_source, obj_trk_to_init.init_vel_source, test_tolerance);
   DOUBLES_EQUAL(expected_vcs_position_x, obj_trk_to_init.vcs_position.x, test_tolerance);
   DOUBLES_EQUAL(init_info.det_bbox.Get_Center().y, obj_trk_to_init.vcs_position.y, test_tolerance);
   DOUBLES_EQUAL(init_info.VCS_velocity.longitudinal, obj_trk_to_init.vcs_velocity.longitudinal, test_tolerance);
   DOUBLES_EQUAL(init_info.VCS_velocity.lateral, obj_trk_to_init.vcs_velocity.lateral, test_tolerance);
   DOUBLES_EQUAL(10.0F, obj_trk_to_init.speed, test_tolerance);
   CHECK_TRUE(obj_trk_to_init.f_moving);
   CHECK_EQUAL(F360_TRACKER_TRKFLTR_CCA, obj_trk_to_init.trk_fltr_type);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.vcs_heading.Value(), test_tolerance);
   DOUBLES_EQUAL(1.0F, obj_trk_to_init.vcs_heading.Cos(), test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.vcs_heading.Sin(), test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.bbox.Get_Orientation().Value(), test_tolerance);
   DOUBLES_EQUAL(1.0F, obj_trk_to_init.bbox.Get_Orientation().Cos(), test_tolerance);
   DOUBLES_EQUAL(0.0F, obj_trk_to_init.bbox.Get_Orientation().Sin(), test_tolerance);
   CHECK_TRUE(obj_trk_to_init.f_vehicular_trk);
   DOUBLES_EQUAL(expected_time_since_vehicle_init, obj_trk_to_init.time_since_vehicle_init, test_tolerance);
}
/** @}*/


/** \defgroup  f360_init_obj_track_covariances
 *  @{
 */
 /** \brief
  *  Test group for Init_Obj_Track_Covariances function
  */

TEST_GROUP(f360_init_obj_track_covariances)
{
   /** \setup
   * Setting up calibrations, init info, obj track to init and test tolerance
   */
   F360_Calibrations_T calibrations = {};
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Object_Track_T obj_trk_to_init = {};
   float32_t test_tolerance = 1.0e-6F;
   F360_Host_T host = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      obj_trk_to_init.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   }


   void check_copied_covariance(float32_t expected_cov[STATE_DIMENSION][STATE_DIMENSION],
      float32_t observed_cov[STATE_DIMENSION][STATE_DIMENSION],
      float32_t tolerance)
   {
      for (uint32_t i = 0; i < STATE_DIMENSION; i++)
      {
         for (uint32_t j = 0; j < STATE_DIMENSION; j++)
         {
            DOUBLES_EQUAL_TEXT(expected_cov[i][j], observed_cov[i][j], tolerance, "Covariances values which supposed to be copied from calibrations don't match");

         }
      }
   }
};

/**
*\purpose  Checking initialization of track's coraviances for nominal cases
*\req   FTCP-12406
*/
TEST(f360_init_obj_track_covariances, TestCCANominalCase)
{
   /** \precond
    * Setting proper filter type
    */
   init_info.VCS_position_cov[F360_2D_IDX_X][F360_2D_IDX_X] = 4.0F;
   init_info.VCS_position_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = -1.0F;
   init_info.VCS_position_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = -1.0F;
   init_info.VCS_position_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = 9.0F;
   init_info.VCS_velocity = { 10.0, 5.0 };
   init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_X] = 10.0F;
   init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.0F;
   init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = 0.0F;
   init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = 25.0F;
   float32_t obj_mov_stat_spd_thresh = 1.0F;
   obj_trk_to_init.vcs_position.Set_Position(2.0F, 1.0F);
   host.dist_rear_axle_to_vcs_m = 6.0F;
   float32_t expected_cov[6][6] = {
      {1.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 30.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.1000000F, 0.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.0000000F, 1.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 75.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.1000000F}
   };
   
   /** \action
    * Call Init_Obj_Track_Covariances
    */
   Init_Obj_Track_Covariances(host, calibrations, init_info, obj_mov_stat_spd_thresh, obj_trk_to_init);

   /** \result
    * Checking if results match expectations
    */
   check_copied_covariance(expected_cov, obj_trk_to_init.errcov, test_tolerance);

}

/** \defgroup  f360_fill_init_obj_track_props_determine_initial_size_qualtest
 *  @{
 */

/** \brief
 *  Test group for Determine_Initial_Size function
 */
TEST_GROUP(f360_fill_init_obj_track_props_determine_initial_size_qualtest)
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
*\purpose  Test that initial object size is correctly determined for a stationary object
*\req   FTCP-12402
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size_qualtest, Test_Initial_Size_Stationary_Object)
{
   /** \precond
    * Set speed below threshold for moving status.
    * Expected length and width are both 1 m.
    */
   // Set up a slow moving object
   float32_t longitudinal_velocity = 0.5F;
   float32_t lateral_velocity = 0.3F;
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = F360_Get_Hypotenuse(longitudinal_velocity, lateral_velocity);
   obj_trk_to_init.f_valid_for_liberal_tracking = false;

   float32_t expected_length = 1.0F;
   float32_t expected_width = 1.0F;

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
*\purpose  Test that initial object size is correctly determined for a slow moving object
*\req   FTCP-12402
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size_qualtest, Test_Initial_Size_Slow_Moving_Object)
{
   /** \precond
    * A slow moving target with speed above threshold for moving status but below threshold for fast moving is set up.
    * Expected length is 3 m and width 1 m.
    */
   float32_t longitudinal_velocity = 2.0F;
   float32_t lateral_velocity = 1.0F;
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = F360_Get_Hypotenuse(longitudinal_velocity, lateral_velocity);
   obj_trk_to_init.f_valid_for_liberal_tracking = false;

   float32_t expected_length = 3.0F;
   float32_t expected_width = 1.0F;

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
*\purpose  Test that initial object size is correctly determined for a fast moving object
*\req   FTCP-12402
*/
TEST(f360_fill_init_obj_track_props_determine_initial_size_qualtest, Test_Initial_Size_Fast_Moving_Object)
{
   /** \precond
    * Set speed above fast moving threshold.
    * Expected length is 6 m and width 2 m.
    */
   float32_t longitudinal_velocity = 5.0F;
   float32_t lateral_velocity = 3.0F;
   F360_Object_Track_T obj_trk_to_init = {};
   obj_trk_to_init.speed = F360_Get_Hypotenuse(longitudinal_velocity, lateral_velocity);
   obj_trk_to_init.f_valid_for_liberal_tracking = false;

   float32_t expected_length = 6.0F;
   float32_t expected_width = 2.0F;
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
