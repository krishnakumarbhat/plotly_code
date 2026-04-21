/** \file
   File with set of qualification tests (which are also unit test) for Try_To_Init_Obj_Track function.
*/
#include "f360_nees_cfmi_try_to_init.h"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
//#include "headerfile_needed.h"
#include "f360_initialize_tracks.h"
#include "f360_initialization_data_generator.h"
#include "f360_initialization_data_generator_support_functions.h"
#include "f360_calibrations.h"
/** \defgroup  f360_try_to_init_obj_track_qualtest
*  @{
*/
using namespace f360_variant_A;
/** \brief
*  Test group for Try_To_Init_Obj_Track
**/
TEST_GROUP(f360_try_to_init_obj_track_qualtest)
{
   /** \setup
   * Setting up need parameters for Try_To_Init_Obj_Track
   **/
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Host_T host = {};
   F360_Host_Props_T host_props = {};
   F360_Calibrations_T calibrations = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Cluster_T current_cluster = {};
   F360_Detection_Hist_T detection_hist = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_NEES_CFMI_Pos_Diff_Velocity_T stationary_velocity = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   float32_t tolerance = 1e-6F;
   float32_t zeros_cov[2][2];
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      for (uint8_t cov_row = 0U; cov_row < 2U; cov_row++)
      {
         for (uint8_t cov_col = 0U; cov_col < 2U; cov_col++)
         {
            zeros_cov[cov_row][cov_col] = 0.0F;
         }
      }
      Prepare_Stationary_Info(host_props, stationary_velocity);
   }
};
/**
*\purpose  Test for checking proper objects initilization
*\req    FTCP-12402
*/
TEST(f360_try_to_init_obj_track_qualtest, HostOverlapCheckPosLongOverlappedInRearAxleDistPosLatNOTOverlappedtest)
{
   /** \precond
   Setting up cluster stationary cluster with certain position
   **/
   host.dist_rear_axle_to_vcs_m = 8.0F;
   float32_t expected_obj_pos_long = -5.0F;
   float32_t expected_obj_pos_lat = 10.0F;
   Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, det_props, raw_detection_list, host_props, expected_obj_pos_long, expected_obj_pos_lat);
   current_cluster = clusters[0];
   /** \action
    Run Try_To_Init_Obj_Track function
    **/
   Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detection_list, detection_hist, stationary_velocity, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success);
   DOUBLES_EQUAL(expected_obj_pos_long, init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(expected_obj_pos_lat, init_info.det_bbox.Get_Center().y, tolerance);
}
/**
*\purpose  Test for checking proper objects initilization
*\req    FTCP-12402
*/
TEST(f360_try_to_init_obj_track_qualtest, HostOverlapCheckPosLongOverlappedInRearAxleDistPosLatOverlappedtest)
{
   /** \precond
   Setting up cluster stationary cluster with certain position
   **/
   host.dist_rear_axle_to_vcs_m = 8.0F;
   float32_t expected_obj_pos_long = -5.0F;
   float32_t expected_obj_pos_lat = 0.0;
   Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters,  det_props, raw_detection_list, host_props, expected_obj_pos_long, expected_obj_pos_lat);
   current_cluster = clusters[0];
   /** \action
    Run Try_To_Init_Obj_Track function
    **/
   Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detection_list, detection_hist, stationary_velocity, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success);
   DOUBLES_EQUAL(expected_obj_pos_long, init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(expected_obj_pos_lat, init_info.det_bbox.Get_Center().y, tolerance);
}
/**
*\purpose  Test for checking proper objects initilization
*\req    FTCP-12402
*/
TEST(f360_try_to_init_obj_track_qualtest, HostOverlapCheckPosLongNOTOverlapedPosLatNOTOverlappedtest)
{
   /** \precond
   Setting up cluster stationary cluster with certain position
   **/
   host.dist_rear_axle_to_vcs_m = 8.0F;
   float32_t expected_obj_pos_long = -10.0F;
   float32_t expected_obj_pos_lat = -10.0;
   Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, det_props, raw_detection_list, host_props, expected_obj_pos_long, expected_obj_pos_lat);   
   current_cluster = clusters[0];
   /** \action
    Run Try_To_Init_Obj_Track function
    **/
   Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detection_list, detection_hist, stationary_velocity, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success);
   DOUBLES_EQUAL(expected_obj_pos_long, init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(expected_obj_pos_lat, init_info.det_bbox.Get_Center().y, tolerance);
}
/**
*\purpose  Test for checking proper objects initilization
*\req    FTCP-12402
*/
TEST(f360_try_to_init_obj_track_qualtest, HostOverlapCheckPosLongNOTOverlapedPosLatOverlappedtest)
{
   /** \precond
   Setting up cluster stationary cluster with certain position
   **/
   host.dist_rear_axle_to_vcs_m = 8.0F;
   float32_t expected_obj_pos_long = 10.0F;
   float32_t expected_obj_pos_lat = 1.0;
   Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, det_props, raw_detection_list, host_props, expected_obj_pos_long, expected_obj_pos_lat);
   current_cluster = clusters[0];
   /** \action
    Run Try_To_Init_Obj_Track function
    **/
   Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detection_list, detection_hist, stationary_velocity, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success);
   DOUBLES_EQUAL(expected_obj_pos_long, init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(expected_obj_pos_lat, init_info.det_bbox.Get_Center().y, tolerance);
}
/**
*\purpose  Test for checking proper objects initilization
*\req    FTCP-12402
*/
TEST(f360_try_to_init_obj_track_qualtest, LiberalInitializationTest)
{
   /** \precond
   Setting up host properties and creating single cluster with 4 detections.
   **/
   host.speed = F360_KPH2MPS(55.0F);
   float32_t expected_obj_pos_long = -59.4F;
   float32_t expected_obj_pos_lat = 1.0F;
   float32_t expected_vel_long = F360_KPH2MPS(75.0F);
   float32_t expected_vel_lat = 0.0F;
   Fill_Clusters_With_Moving_Cluster_With_4_Moving_Dets_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props,
      expected_obj_pos_long, expected_obj_pos_lat, expected_vel_long, expected_vel_lat);
   current_cluster = clusters[0];
   int32_t det_id = current_cluster.detids[0];
   det_props[det_id - 1].f_valid_for_liberal_tracking = true;
   /** \action
   Run Try_To_Init_Obj_Track function
   **/
   Try_To_Init_Obj_Track_By_NEES_CFMI(calibrations, host_props, host, current_cluster, sensors, det_props, raw_detection_list, detection_hist, stationary_velocity, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   float32_t lib_init_vel_tolerance = 1.0F;
   CHECK_TRUE(init_info.f_success);
   DOUBLES_EQUAL(expected_obj_pos_long, init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(expected_obj_pos_lat, init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(expected_vel_long, init_info.VCS_velocity.longitudinal, lib_init_vel_tolerance);
   DOUBLES_EQUAL(expected_vel_lat, init_info.VCS_velocity.lateral, lib_init_vel_tolerance);
}
/** @}*/
