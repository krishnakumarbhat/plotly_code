/** \file
This file contains unit tests that verifies the content of f360_clustering_detections.cpp
*/

#include "f360_clustering_detections.h"
#include "f360_clustering_data_generator.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//Declaration of stubbed/mock functions

using namespace f360_variant_A;

/** \defgroup  f360_clustering_detections
*  @{
*/

/** \brief
*  Includes tests that will test the behavior of functions implemented in f360_clustering_detections.cpp.
**/
TEST_GROUP(f360_clustering_detections)
{
   /** \setup
   * Nothing to setup in this test group
   **/
   float32_t epsilon_float = {};

   F360_Detection_Props_T det_p_1 = {};
   F360_Detection_Props_T det_p_2 = {};
   rspp_variant_A::RSPP_Detection_T det_1 = {};
   float32_t distance = {};
   bool result = {};
   F360_Calibrations_T calib = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Clustering_Configuration_T clustering_config = {};
   F360_Detection_Props_T det_p[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   F360_Host_Props_T host_props = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Radar_Sensor_T& sensor = sensors[0];
   F360_Host_T host= {};
   TEST_SETUP()
   {
      epsilon_float = 1.00e-5;
      host.vcs_speed = 5.0F;
   }

   /** \teardown
   * Nothing to teardown in this test group
   **/
   TEST_TEARDOWN()
   {
      //mock.clear();
   }
};


TEST(f360_clustering_detections, Clustering_Distance_Position)
{
   /**
   *\purpose  Test that the distance squared is computed correctly between two detections
   *\req    NA
   */

   /** \precond
   Set reasonable values to long and lat positions of two detections
   **/
   det_p_1.vcs_position.x = 5.00F;
   det_p_1.vcs_position.y = -9.30F;
   det_p_2.vcs_position.x = 5.30F;
   det_p_2.vcs_position.y = -8.50F;
   /** \action
   * Call Clustering_Distance_Position()
   **/
   distance = Clustering_Distance_Position(det_p_1, det_p_2);

   /** \result
   * Check if computed distance is the same as the expected
   **/
   DOUBLES_EQUAL_TEXT(0.73F, distance, epsilon_float, "Computed distance is not equal to expected");
}

TEST(f360_clustering_detections, Clustering_Distance_Position_Rangerate)
{
   /**
   *\purpose  Test that the distance squared is computed correctly between two detections when difference in rangerate is small
   *\req    NA
   */

   /** \precond
   Set reasonable values to long and lat positions and range rate dealiased of two detections
   **/
   det_p_1.vcs_position.x = 5.00F;
   det_p_1.vcs_position.y = -9.30F;
   det_p_1.range_rate_dealiased = 1.0F;
   det_p_2.vcs_position.x = 5.30F;
   det_p_2.vcs_position.y = -8.50F;
   det_p_2.range_rate_dealiased = 1.2F;

   /** \action
   * Call Clustering_Distance_Position_Rangerate()
   **/
   distance = Clustering_Distance_Position_Rangerate(det_p_1, det_p_2);

   /** \result
   * Check if computed distance is the same as the expected
   **/
   DOUBLES_EQUAL_TEXT(0.73F, distance, epsilon_float, "Computed distance is not equal to expected");

   /**
   *\purpose  Test that the distance squared is computed correctly between two detections when difference in rangerate is big
   *\req    NA
   */

   /** \precond
   Set reasonable values to long and lat positions and range rate dealiased of two detections,
   i.e. the difference in range rate is larger than range_rate_gate = 2.0F
   **/
   det_p_1.vcs_position.x = 5.00F;
   det_p_1.vcs_position.y = -9.30F;
   det_p_1.range_rate_dealiased = 1.0F;

   det_p_2.vcs_position.x = 5.30F;
   det_p_2.vcs_position.y = -8.50F;
   det_p_2.range_rate_dealiased = 3.2F;

   /** \action
   * Call Clustering_Distance_Position_Rangerate()
   **/
   distance = Clustering_Distance_Position_Rangerate(det_p_1, det_p_2);

   /** \result
   * Check if computed distance is the same as the expected
   **/
   CHECK_TRUE_TEXT(distance>1.0E35, "Computed distance is not set to predefined infinity (1E36)");
}

TEST(f360_clustering_detections, Cluster_Leftovers_Check)
{
   /**
   *\purpose  Test that a detection is marked as ok for leftovers clustering when all conditions are met
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_TRUE_TEXT(result, "Real leftover detection was marked as not suitable for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when object track id is not 0
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 1U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when cluster id is not 0
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 1U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when wheel spin type is not of type INVALID
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when f_double_bounce is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = true;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;


   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when f_close_target is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = true;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;


   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when f_det_pair is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = true;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;


   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when f_FOV_edge is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = true;
   det_p_1.f_ok_to_use = true;


   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when f_ok_to_use is false
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = false;


   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for leftovers clustering");

   /**
   *\purpose  Test that a detection is marked as not ok for leftovers clustering when hvc flag is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_1.raw.f_host_veh_clutter = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Real leftover detection was marked as not suitable for leftovers clustering");
}

/**
*\purpose  Test that bistatic is not allowed to cluster
*\req    NA
*/
TEST(f360_clustering_detections, Cluster_Leftovers_Check__Bisatic_Not_Allowed_To_Cluster)
{

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_1.raw.f_bistatic = true;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if function returned false;
   **/
   CHECK_FALSE(result);
}

/**
*\purpose  Test that waterspray is not allowed to cluster
*\req    NA
*/
TEST(f360_clustering_detections, Cluster_Leftovers_Check__Waterspray_Not_Allowed_To_Cluster)
{

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.object_track_id = 0U;
   det_p_1.cluster_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.f_water_spray = true;
   det_1.raw.f_bistatic = false;

   /** \action
   * Call Cluster_Leftovers_Check()
   **/
   result = Cluster_Leftovers_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if function returned false;
   **/
   CHECK_FALSE(result);
}

/**
*\purpose  Test that bistatic is not allowed to cluster
*\req    NA
*/
TEST(f360_clustering_detections, Cluster_Moving_Check__Bisatic_Not_Allowed_To_Cluster)
{

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_1.raw.f_host_veh_clutter = false;
   det_1.raw.f_bistatic = true;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if function returned false;
   **/
   CHECK_FALSE(result);
}

/**
*\purpose  Test that water spray is not allowed to cluster
*\req    NA
*/
TEST(f360_clustering_detections, Cluster_Moving_Check__Waterspray_Not_Allowed_To_Cluster)
{

   /** \precond
   Set attributes of a detection
   **/
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_1.raw.f_host_veh_clutter = false;
   det_p_1.f_water_spray = true;
   det_1.raw.f_bistatic = false;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if function returned false;
   **/
   CHECK_FALSE(result);
}

TEST(f360_clustering_detections, Cluster_Moving_Check_With_Liberal_Tracking_Valid_Preconditions_Valid_Combined_Flag)
{
   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when its marks for liberal tracking, preconditions are valid and combined flag is valid
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_p_1.f_valid_for_liberal_tracking = true;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_TRUE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");
}

TEST(f360_clustering_detections, Cluster_Moving_Check_With_Liberal_Tracking_Valid_Preconditions_Invalid_Combined_Flag)
{
   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when its marks for liberal tracking, preconditions are valid and combined flag is invalid
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = true;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_p_1.f_valid_for_liberal_tracking = true;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_TRUE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");
}

/**
*\purpose  Test that a detection is not marked as valid for clustering of moving detections when its elevation is unreliable.
*\req    NA
*/
TEST(f360_clustering_detections, Cluster_Moving_Check_With_Elevation_Unreliable)
{
   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = false;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 1;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = true;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;
   det_p_1.f_valid_for_liberal_tracking = false;
   sensor.constant.sensor_type = F360_SENSOR_TYPE_MRR360_RADAR;
   det_1.raw.elevation = calib.k_mrr360_max_abs_elev_angle_rad + 0.1F;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE(result);
}

TEST(f360_clustering_detections, Cluster_Moving_Check_With_Liberal_Tracking_Invalid_Preconditions_Valid_Combined_Flag)
{
   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when its marks for liberal tracking, preconditions are invalid and combined flag is valid
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = false;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 1;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_p_1.f_valid_for_liberal_tracking = true;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");
}

TEST(f360_clustering_detections, Cluster_Moving_Check_With_Liberal_Tracking_Invalid_Preconditions_Invalid_Combined_Flag)
{
   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when its marks for liberal tracking, preconditions are invalid and combined flag is invalid
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = false;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 1;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = true;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;

   det_p_1.f_valid_for_liberal_tracking = true;

   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");
}

TEST(f360_clustering_detections, Cluster_Moving_Check)
{
   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when all checks are true (incl. motion status moving and is_separate_clustering=true)
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_TRUE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as ok for clustering of moving detections when all checks are true (incl. motion status stationary and is_separate_clustering=false)
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = false;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_TRUE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when motion status is stationary and is_separate_clustering=true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when object track id is not 0
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   det_p_1.object_track_id = 1U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when wheel spin type is not INVALID
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when f_double_bounce=true.
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = true;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections f_close_target=true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = true;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when f_det_pair=true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = true;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when f_FOV_edge=true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = true;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when f_ok_to_use=false
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = false;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when on_sep_id > F360_INVALID_UNSIGNED_ID
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = 1;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");

   /**
   *\purpose  Test that a detection is marked as not ok for clustering of moving detections when hvc flag is true
   *\req    NA
   */

   /** \precond
   Set attributes of a detection and calib
   **/
   calib.is_separate_clustering = true;
   det_1.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_p_1.object_track_id = 0U;
   det_p_1.wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   det_p_1.f_double_bounce = false;
   det_p_1.f_close_target = false;
   det_p_1.f_det_pair = false;
   det_p_1.f_FOV_edge = false;
   det_p_1.f_ok_to_use = true;
   det_p_1.on_sep_id = F360_INVALID_UNSIGNED_ID;
   det_1.raw.f_host_veh_clutter = true;


   /** \action
   * Call Cluster_Moving_Check()
   **/
   result = Cluster_Moving_Check(calib, sensor, det_p_1, det_1, host.vcs_speed);

   /** \result
   * Check if computed result boolean is the same as the expected
   **/
   CHECK_FALSE_TEXT(result, "Detection was marked wrongly for clustering of moving detections");
}

TEST(f360_clustering_detections, Cluster_Detections)
{
   /**
   *\purpose  Test that Cluster_Detections works as intended by checking that DBscan and
   * Initialize_Clusters are not called when no valid detections are provided
   *\req    NA
   */

   /** \precond
   Set number_of_valid_detections = 0 to ensure that det_point_index can never be bigger than 0 and DBscan and Initialize_Clusters are never called.
   Also set mock expectations correspondingly.
   **/
   raw_detection_list.number_of_valid_detections = 0U;
   mock().expectNoCall("DBscan");
   mock().expectNoCall("Initialize_Clusters");

   /** \action
   * Call Cluster_Detections()
   **/

   Cluster_Detections(raw_detection_list, sensors, calib, host, clustering_config, tracker_info, det_p, clusters);

   /** \result
   * Check that mock expectations were met.
   **/
   mock().checkExpectations();
   mock().clear();
}

/** \defgroup  f360_find_and_prioritize_detections
*  @{
*/

/** \brief
*  Includes tests that will test the behavior of function find_and_prioritize_detections.
**/
TEST_GROUP(f360_find_and_prioritize_detections)
{
   /** \setup
   * Setting up arguments for find_and_prioritize_detections function and assigning them with basic values
   **/
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   RSPP_Calibrations_T rspp_calib;
   F360_Calibrations_T calib = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   F360_Clustering_Configuration_T clustering_config;
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Host_T host = {};
   float32_t det_vcs_pos_long = 0.0;
   float32_t det_vcs_pos_lat = 0.0;
   float32_t det_range_rate = 0.0;
   float32_t det_probability_of_detection = 1.0;
   int32_t object_track_id = 0;
   int16_t valid_det_count = 0;
   int16_t valid_det_sorted_idxs[MAX_NUMBER_OF_DETECTIONS];
   int16_t local_det_idx = 0;
   int8_t dets_in_zone = 10;
   uint8_t on_sep_id = F360_INVALID_UNSIGNED_ID;
   bool valid_dets[MAX_NUMBER_OF_DETECTIONS] = {};
   bool det_f_dealiased = true;
   bool f_double_bounce = false;
   bool f_close_target = false;
   bool f_det_pair = false;
   bool f_FOV_edge = false;
   bool f_ok_to_use = true;
   bool f_host_veh_clutter = false;
   F360_Detection_Wheelspin_Type_T wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      clustering_config.clustering_radius = 2.0F;
      clustering_config.min_pts_in_cluster = 1U;
      clustering_config.clustering_radius_sq = clustering_config.clustering_radius * clustering_config.clustering_radius;
      clustering_config.Cluster_Distance_Sq_Function = Clustering_Distance_Position_Rangerate;
      clustering_config.Detection_Cluster_Check = Cluster_Moving_Check;
      host.vcs_speed = 5.0F;
   }
};

TEST(f360_find_and_prioritize_detections, check_priority_zones_sorting)
{
   /** \precond
   Preparing valid detection data for each priority zone
   **/

   // assigning moving detections third zone
   det_vcs_pos_lat = 20.0F;
   det_vcs_pos_long = 0.0F;
   for (int16_t i = 0; i < 10; i++)
   {
      local_det_idx = i;
      det_vcs_pos_long += 0.1F;
      det_vcs_pos_lat += 0.1F;

      Add_Simple_Det_Data(raw_detection_list, raw_detection_list.detections[local_det_idx], local_det_idx,
         det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, detection_props);

      Add_Det_Data_For_Validity(detection_props[local_det_idx], raw_detection_list.detections[local_det_idx], f_double_bounce, f_close_target,
         f_det_pair, f_FOV_edge, f_ok_to_use, on_sep_id, f_host_veh_clutter, object_track_id, wheel_spin_type);
   }
   // assigning moving detections first zone
   det_vcs_pos_lat = 0.0F;
   det_vcs_pos_long = 0.0F;
   for (int16_t i = 10; i < 20; i++)
   {
      local_det_idx = i;
      det_vcs_pos_long += 0.1F;

      Add_Simple_Det_Data(raw_detection_list, raw_detection_list.detections[local_det_idx], local_det_idx,
         det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, detection_props);

      Add_Det_Data_For_Validity(detection_props[local_det_idx], raw_detection_list.detections[local_det_idx], f_double_bounce, f_close_target,
         f_det_pair, f_FOV_edge, f_ok_to_use, on_sep_id, f_host_veh_clutter, object_track_id, wheel_spin_type);
   }

   // assigning moving detections fourth zone
   det_vcs_pos_lat = 100.0F;
   det_vcs_pos_long = 100.0F;
   for (int16_t i = 20; i < 30; i++)
   {
      local_det_idx = i;
      det_vcs_pos_long += 0.1F;
      det_vcs_pos_lat += 0.1F;

      Add_Simple_Det_Data(raw_detection_list, raw_detection_list.detections[local_det_idx], local_det_idx,
         det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, detection_props);

      Add_Det_Data_For_Validity(detection_props[local_det_idx], raw_detection_list.detections[local_det_idx], f_double_bounce, f_close_target,
         f_det_pair, f_FOV_edge, f_ok_to_use, on_sep_id, f_host_veh_clutter, object_track_id, wheel_spin_type);
   }

   // assigning moving detections second zone
   det_vcs_pos_lat = 0.0F;
   det_vcs_pos_long = 0.0F;
   for (int16_t i = 30; i < 40; i++)
   {
      local_det_idx = i;
      det_vcs_pos_long -= 0.1F;

      Add_Simple_Det_Data(raw_detection_list, raw_detection_list.detections[local_det_idx], local_det_idx,
         det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, detection_props);

      Add_Det_Data_For_Validity(detection_props[local_det_idx], raw_detection_list.detections[local_det_idx], f_double_bounce, f_close_target,
         f_det_pair, f_FOV_edge, f_ok_to_use, on_sep_id, f_host_veh_clutter, object_track_id, wheel_spin_type);
   }

   // assigning moving detections fifth zone
   det_vcs_pos_lat = -100.0F;
   det_vcs_pos_long = -100.0F;
   for (int16_t i = 40; i < 50; i++)
   {
      local_det_idx = i;
      det_vcs_pos_long -= 0.1F;
      det_vcs_pos_lat -= 0.1F;

      Add_Simple_Det_Data(raw_detection_list, raw_detection_list.detections[local_det_idx], local_det_idx,
         det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, detection_props);

      Add_Det_Data_For_Validity(detection_props[local_det_idx], raw_detection_list.detections[local_det_idx], f_double_bounce, f_close_target,
         f_det_pair, f_FOV_edge, f_ok_to_use, on_sep_id, f_host_veh_clutter, object_track_id, wheel_spin_type);
   }
   
   Sort_Detections_Vcs_Long(rspp_calib,raw_detection_list);
   
   /** \action
   * Calling Find_And_Prioritize_Detections
   **/
   Find_And_Prioritize_Detections(raw_detection_list, sensors, calib, host, clustering_config, detection_props, valid_det_count, valid_det_sorted_idxs, valid_dets);
   
   /** \result
   * Checking if detections are properly sorted by priority zones
   **/

   // first zone
   for (int8_t i = 0; i < 10; i++)
   {
      CHECK_EQUAL(10+i, valid_det_sorted_idxs[i]);
   }

   // second zone
   for (int8_t i = 0; i < 10; i++)
   {
      CHECK_EQUAL(30+i, valid_det_sorted_idxs[dets_in_zone + i]);
   }

   // third zone
   for (int8_t i = 0; i < 10; i++)
   {
      CHECK_EQUAL(i, valid_det_sorted_idxs[2 * dets_in_zone + i]);
   }

   // fourth zone
   for (int8_t i = 0; i < 10; i++)
   {
      CHECK_EQUAL(20+i, valid_det_sorted_idxs[3 * dets_in_zone + i]);
   }

   // fifth zone
   for (int8_t i = 0; i < 10; i++)
   {
      CHECK_EQUAL(40+i, valid_det_sorted_idxs[4 * dets_in_zone + i]);
   }
}

/** @}*/
