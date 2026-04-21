/** \file
   File with unit test for Initialize_Tracks function.
   This file contains unit tests only, qualification tests are in a separate file
*/

#include "f360_initialize_tracks.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>

#include "f360_allocate_id_for_initialized_object.h"
#include "f360_initialization_data_generator.h"
#include "f360_initialization_data_generator_support_functions.h"
#include "f360_occlusion_ut_helpers.h"
#include "f360_set_variant.h"

/** \defgroup  f360_initialize_tracks
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Test group for unique_id assignment in Initialize_Tracks function
**/
TEST_GROUP(f360_initialize_tracks_unique_id)
{
   F360_Host_T host = {};
   F360_Host_Props_T host_props = {};
   F360_Globals_T globals = {};
   F360_Calibrations_T calibrations = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Detection_Hist_T detection_hist = {};
   F360_TRKR_TIMING_INFO_T tracker_time_info = {};
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};

   F360_Object_Status_T expected_initialized_object_status = F360_OBJECT_STATUS_NEW_UPDATED;
   F360_Object_Status_T expected_non_initialized_object_status = F360_OBJECT_STATUS_INVALID;
   uint16_t non_initilized_obj_id = 0U;
   float32_t speed_tolerance = 0.1F;
   bool f_enable_occlusion = true;

   /** \setup
   * Setting up calibrations, tracker info inactive ids and objects inactive ids to default values
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      Set_Tracker_Variant(tracker_info.variant);

      Set_Left_Front_Sensor(sensors[0]);
      Set_Right_Front_Sensor(sensors[1]);
      Set_Left_Rear_Sensor(sensors[2]);
      Set_Right_Rear_Sensor(sensors[3]);

      tracker_info.num_unique_objs = 0U;

      globals.f_single_front_center_radar_only = false;

      for (uint16_t idx = 0U; idx < NUMBER_OF_OBJECT_TRACKS; idx++)
      {
         tracker_info.inactive_obj_ids[idx] = idx + 1U;
         object_tracks[idx].id = idx + 1U;
      }

   }
};

/**
*\purpose  Verify if correct unique_id is assigned to a new track when no other tracks exist
*\req    NA
*/
TEST(f360_initialize_tracks_unique_id, First_Initialized_Object)
{
   /** \precond
    * Setting up successful object initialization
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Initialize_Tracks function
    */
   Initialize_Tracks(
      host,
      host_props,
      raw_detection_list,
      detection_hist,
      globals,
      calibrations,
      sep,
      occlusion,
      sensors,
      clusters,
      object_tracks,
      tracker_info,
      det_props,
      tracker_time_info
   );

   /** \result
    * Checking if object has been assigned with correct unique_id
    */
   CHECK_EQUAL(1U, tracker_info.num_unique_objs);
   CHECK_EQUAL(1U, object_tracks[0].unique_id);
}

/**
*\purpose  Verify if correct unique_id is assigned to a new track when multiple other tracks exist
*\req    NA
*/
TEST(f360_initialize_tracks_unique_id, Next_Initialized_Object)
{
   /** \precond
    * Setting up successful object initialization
    * Setting up all but one tracks to active state, one remains inactive
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);
   
   tracker_info.num_unique_objs = 100000U;
   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS - 1;
   for (uint16_t idx = 0U; idx < NUMBER_OF_OBJECT_TRACKS - 1; idx++)
   {
      tracker_info.active_obj_ids[idx] = idx + 1U;
   }
   std::fill(cmn::begin(tracker_info.inactive_obj_ids), cmn::end(tracker_info.inactive_obj_ids), 0U);
   tracker_info.inactive_obj_ids[0] = NUMBER_OF_OBJECT_TRACKS;

   /** \action
    * Calling Initialize_Tracks function
    */
   Initialize_Tracks(
      host,
      host_props,
      raw_detection_list,
      detection_hist,
      globals,
      calibrations,
      sep,
      occlusion,
      sensors,
      clusters,
      object_tracks,
      tracker_info,
      det_props,
      tracker_time_info
   );

   /** \result
    * Checking if object has been assigned with correct unique_id
    */
   CHECK_EQUAL(100001U, tracker_info.num_unique_objs);
   CHECK_EQUAL(100001U, object_tracks[NUMBER_OF_OBJECT_TRACKS-1U].unique_id);
}

/**
*\purpose  Verify if no unique_id is assigned when initilization unsuccessful
*\req    NA
*/
TEST(f360_initialize_tracks_unique_id, Object_Not_Initialized)
{
   /** \precond
    * Setting up unsuccessful object initialization
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Unsuccesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Initialize_Tracks function
    */
   Initialize_Tracks(
      host,
      host_props,
      raw_detection_list,
      detection_hist,
      globals,
      calibrations,
      sep,
      occlusion,
      sensors,
      clusters,
      object_tracks,
      tracker_info,
      det_props,
      tracker_time_info
   );

   /** \result
    * Check if object has NOT been assigned with unique_id
    */
   CHECK_EQUAL(0U, tracker_info.num_unique_objs);
   CHECK_EQUAL(0U, object_tracks[0U].unique_id);
}

/**
*\purpose  Verify if correct unique_id is assigned when two new objects created
*\req    NA
*/
TEST(f360_initialize_tracks_unique_id, Two_Successfull_And_One_Unsuccessfull_Objects_Initialization)
{
   /** \precond
    * Setting up successful object initialization for two objects and one unsuccessful
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Unsuccesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Initialize_Tracks function
    */
   Initialize_Tracks(
      host,
      host_props,
      raw_detection_list,
      detection_hist,
      globals,
      calibrations,
      sep,
      occlusion,
      sensors,
      clusters,
      object_tracks,
      tracker_info,
      det_props,
      tracker_time_info
   );
   /** \result
    * Check if correct unique_id has been assigned to two new objects, but not the third one
    * First and third cluster from test precond are initlized on first available indexes in 
    * object array which is 0 and 1 - they should have unique_id assigned. Second cluster 
    * from test precond doesn't create an object, so the there is no object with unique_id
    * in index 2 of object_array
    */
   CHECK_EQUAL(2U, tracker_info.num_unique_objs);
   CHECK_EQUAL(1U, object_tracks[0U].unique_id);
   CHECK_EQUAL(2U, object_tracks[1U].unique_id);
   CHECK_EQUAL(0U, object_tracks[2U].unique_id);
}

/** @}*/