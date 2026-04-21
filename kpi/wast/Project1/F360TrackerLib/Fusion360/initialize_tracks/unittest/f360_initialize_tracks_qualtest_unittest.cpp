/** \file
   File with set of qualification tests (which are also unit test) for initialize_tracks function
*/

#include "f360_initialize_tracks.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>

//#include "headerfile_needed.h"
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
*  Test group for Initialize_Tracks function
**/
TEST_GROUP(f360_initialize_tracked_objects)
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

      globals.f_single_front_center_radar_only = false;

      Set_Left_Front_Sensor(sensors[0]);
      Set_Right_Front_Sensor(sensors[1]);
      Set_Left_Rear_Sensor(sensors[2]);
      Set_Right_Rear_Sensor(sensors[3]);
      for (uint16_t idx = 0U; idx < NUMBER_OF_OBJECT_TRACKS; idx++)
      {
         tracker_info.inactive_obj_ids[idx] = idx + 1U;
         object_tracks[idx].id = idx + 1U;
      }

   }
};

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12397 
*/
TEST(f360_initialize_tracked_objects, Lowest_Cluster_Priority_Initialization_Try_With_Obj_Saturation)
{
   /** \precond
    * Setting up 251 cluster for objects initalization, from first 250 objects shall be inialized but the last one shall not
    */
   // this test might fail if there will be some logic for initialization that depends on objects overlaping
   // defining 250 clusters with high priority, every object from these clusters shall have speed about 10 [m/s]
   for (uint16_t init_num = 0U; init_num < NUMBER_OF_OBJECT_TRACKS; init_num++)
   {
      Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   }
   // defining one 251th cluster with low priority from which object shall not be iniatialized, 
   // if there will be object initialized from this cluster it shall have speed about 15.3 [m/s]
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   //Initializing empty occlusion object - no occluding objects present
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
    * Checking if 250 object will be initialized from first 250 clusters
    */
   // all clusters from 1-250 shall get initialized and have f_to_be_killed set to true
   // all objects shall have speed about 10 [m/s], it is expected there will be no object initialized with speed about 15.3 [m/s]
   for (uint16_t init_num = 0U; init_num < NUMBER_OF_OBJECT_TRACKS; init_num++)
   {
      CHECK_TRUE(clusters[init_num].f_to_be_killed);
      DOUBLES_EQUAL(10.0F, object_tracks[init_num].speed, 0.1F);
   }
   // checking if there is saturation
   CHECK_EQUAL(NUMBER_OF_OBJECT_TRACKS, tracker_info.num_active_objs);
   // 251th cluster shall not be initialized to object and f_to_be_killed shall remain on false
   CHECK_FALSE(clusters[NUMBER_OF_OBJECT_TRACKS].f_to_be_killed);
}

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12408, FTCP-12399, FTCP-12402
*/
TEST(f360_initialize_tracked_objects, Highest_Cluster_Priority_Initialization_Try_With_Obj_Saturation)
{
   /** \precond
    * Setting up 1001 cluster for objects initalization, from first 1001 objects shall be inialized and the last one shall overwrite object with lowest priority
    */
    // this test might fail if there will be some logic for initialization that depends on objects overlaping
    // defining 1000 clusters with low priority, every object from these clusters shall have speed about 15.3 [m/s]
   for (uint16_t init_num = 0U; init_num < NUMBER_OF_OBJECT_TRACKS; init_num++)
   {
      Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   }
   // defining one 1001th cluster with high priority, if there will be object initialized from this cluster it shall have speed about 10 [m/s]
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   //Initializing empty occlusion object - no occluding objects present
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
    * Checking if 1000 have been initilized where one is overwritten
    */
   // first initialized object in array shall be the one with highest priority
   DOUBLES_EQUAL(10.0F, object_tracks[0U].speed, speed_tolerance);
   // 1001th cluster shall get initialized and has f_to_be_killed set to true
   CHECK_TRUE(clusters[NUMBER_OF_OBJECT_TRACKS].f_to_be_killed);
   // all clusters from 1-1000 shall get initialized and have f_to_be_killed set to true
   // 2nd - 1000th objects shall have speed about 15 [m/s]
   CHECK_TRUE(clusters[0U].f_to_be_killed);
   for (uint16_t init_num = 1U; init_num < NUMBER_OF_OBJECT_TRACKS - 1U; init_num++)
   {
      CHECK_TRUE(clusters[init_num].f_to_be_killed);
      DOUBLES_EQUAL(15.3F, object_tracks[init_num].speed, speed_tolerance);
   }
   DOUBLES_EQUAL(15.3F, object_tracks[NUMBER_OF_OBJECT_TRACKS - 1U].speed, speed_tolerance);
   // 250th cluster shall remain untouched
   CHECK_FALSE(clusters[NUMBER_OF_OBJECT_TRACKS - 1U].f_to_be_killed);
   // checking if there is saturation
   CHECK_EQUAL(NUMBER_OF_OBJECT_TRACKS, tracker_info.num_active_objs);
   CHECK_EQUAL(0, tracker_info.inactive_obj_ids[0]);
}

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12403
*/
TEST(f360_initialize_tracked_objects, Single_Successfull_Object_Initalization)
{
   /** \precond
    * Setting up cluster with 2 dets from current time instance and 2 hist dets for success initialization
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if object had been initialized correctly with proper status
    */
   CHECK_EQUAL(1U, tracker_info.num_active_objs);
   CHECK_EQUAL(1U, tracker_info.active_obj_ids[0U]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[0].status);
}

/**
*\purpose  High level initialization tracks testing. Checking initialiation scheme
*\req    FTCP-12404
*/
TEST(f360_initialize_tracked_objects, Single_Successfull_Object_Initalization_With_Init_Scheme_Check)
{
   /** \precond
    * Setting up cluster with 2 dets from current time instance and 2 hist dets for success initialization
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if object had been initialized correctly with proper initialization scheme
    */
   CHECK_EQUAL(1U, tracker_info.num_active_objs);
   CHECK_EQUAL(1U, tracker_info.active_obj_ids[0U]);
   CHECK_EQUAL(F360_TRACK_NEES_CV_M_ESTIMATOR, object_tracks[0].init_scheme);
}

/**
*\purpose  High level initialization tracks testing. Checking if all detections from cluster that were used for init,
*\will be assigned to new object
*\req    FTCP-12398
*/
TEST(f360_initialize_tracked_objects, Detections_Assignment_For_Single_Successfull_Object_Initalization)
{
   /** \precond
    * Setting cluster with 2 hist dets and 2 dets from current time instance.
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if 2 detections from current time instance were associated to new object
    */
   CHECK_EQUAL(1U, det_props[0].object_track_id);
   CHECK_EQUAL(1U, det_props[1].object_track_id);
}

/**
*\purpose  High level initialization tracks testing. Checking if all detections from cluster that were used for init,
*\will not be assigned any object due lack of initialization
*\req    FTCP-12398
*/
TEST(f360_initialize_tracked_objects, Detections_Assignment_For_Single_Unsuccessfull_Object_Initalization)
{
   /** \precond
    * Setting cluster with 2 hist dets and 2 dets from current time instance for unsuccessfull initialization
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if no detections are assiciated to any object
    */
   CHECK_EQUAL(0U, det_props[0].object_track_id);
   CHECK_EQUAL(0U, det_props[0].object_track_id);
}

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12403
*/
TEST(f360_initialize_tracked_objects, Single_Unsuccessfull_Object_Initalization)
{
   /** \precond
    * Setting cluster with 2 hist dets and 2 dets from current time instance for unsuccessfull initialization
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if no object had been initialized
    */
   CHECK_EQUAL(0U, tracker_info.num_active_objs);
   CHECK_EQUAL(non_initilized_obj_id, tracker_info.active_obj_ids[0U]);
   CHECK_EQUAL(expected_non_initialized_object_status, object_tracks[0U].status);
}

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12403
*/
TEST(f360_initialize_tracked_objects, Two_Successfull_And_One_Unsuccessfull_Objects_Initialization)
{
   /** \precond
    * Setting up three cluster. Two for initialization, all with 2 dets form current time instant and 2 hist dets
    * Initializing empty occlusion object - no occluding objects present
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
    * Checking if two objects have been initialized with proper status
    */

   CHECK_EQUAL(2U, tracker_info.num_active_objs);
   //first object should be initialized
   CHECK_EQUAL(1U, tracker_info.active_obj_ids[0U]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[0].status);
   //second object should be initialized
   CHECK_EQUAL(2U, tracker_info.active_obj_ids[1U]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[1].status);
   //third object should not be initialized
   CHECK_EQUAL(non_initilized_obj_id, tracker_info.active_obj_ids[2U]);
   CHECK_EQUAL(expected_non_initialized_object_status, object_tracks[2U].status);
}

/**
*\purpose  High level initialization tracks testing
*\req    FTCP-12408, FTCP-12402
*/
TEST(f360_initialize_tracked_objects, Highest_Cluster_Priority_Reinitialization_With_Obj_Saturation)
{
   /** \precond
    * Setting up cluster with high enought priority for successfull object initialization
    */
   uint16_t new_obj_idx = 19U;
   // preparing 250 dummy objects
   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS;
   for (uint16_t init_num = 0U; init_num < NUMBER_OF_OBJECT_TRACKS; init_num++)
   {
      tracker_info.inactive_obj_ids[init_num] = 0U;
      tracker_info.active_obj_ids[init_num] = init_num + 1U;

      object_tracks[init_num].speed = 5.0F;
      object_tracks[init_num].priority = 0.4F + init_num * 0.001F;
   }
   tracker_info.p_highest_priority_track = &object_tracks[NUMBER_OF_OBJECT_TRACKS - 1U];
   // making 20th object the one with the lowest priority
   object_tracks[new_obj_idx].priority = 0.2F;
   tracker_info.p_lowest_priority_track = &object_tracks[new_obj_idx];

   // defining one cluster with high priority, if there will be object initialized from this cluster it shall have speed about 10 [m/s]
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   
   // Initializing empty occlusion object - no occluding objects present
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
    * Checking if new object had been initialized
    */
   // 20th object shall be replaced by object initialized from cluster 
   DOUBLES_EQUAL(10.0F, object_tracks[new_obj_idx].speed, speed_tolerance);
   CHECK_TRUE(clusters[0U].f_to_be_killed);
   // all objects despite the 20th shall remain the same
   for (uint16_t init_num = 0U; init_num < NUMBER_OF_OBJECT_TRACKS - 1U; init_num++)
   {
      if (init_num != new_obj_idx)
      {
         DOUBLES_EQUAL(5.0F, object_tracks[init_num].speed, speed_tolerance);
      }
   }
}

/**
*\purpose  High level initialization tracks testing. Checking if object after initalization are correctly sorted by priority
*\req    FTCP-12400
*/
TEST(f360_initialize_tracked_objects, Check_If_Objects_Are_Correctly_Sorted_By_Priority)
{
   /** \precond
    * Setting up four clusters for initialziation with different priority and position
    * Initializing empty occlusion object - no occluding objects present
    */
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 100.0, 10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 0.0,   10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 10.0,  10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 20.0,  10.0);
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
    * Priority in this case shall be based on distance (the higher distance from 0.0, 0.0 the lower priority)
    */

   // check from highest priority to lowest
   CHECK_EQUAL(tracker_info.p_highest_priority_track->id, object_tracks[1].id);
   CHECK_EQUAL(3U, object_tracks[1].p_lower_priority_track->id);
   CHECK_EQUAL(4U, object_tracks[2].p_lower_priority_track->id);
   CHECK_EQUAL(1U, object_tracks[3].p_lower_priority_track->id);
   CHECK_EQUAL(tracker_info.p_lowest_priority_track->id, object_tracks[3].p_lower_priority_track->id);
}

/**
*\purpose  High level initialization tracks testing. Checking if object after initalization are correctly sorted by longitudinal position
*\req    FTCP-12401
*/
TEST(f360_initialize_tracked_objects, Check_If_Objects_Are_Correctly_Sorted_By_Longitudinal_Position)
{
   /** \precond
    * Setting up four clusters for initialziation with different priority and position
    * Initializing empty occlusion object - no occluding objects present
    */
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 100.0, 10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 0.0, 10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 10.0, 10.0);
   Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, host_props, 20.0, 10.0);
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
    * Four objects shall be sorted by longitudinal position
    */
   CHECK_EQUAL(2U,   tracker_info.vcslong_sorted_start->id);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_prev_track[1]); // checking if there are no track 'before' the first one
   CHECK_EQUAL(3U,   tracker_info.vcslong_sorted_next_track[1]->id);
   CHECK_EQUAL(4U,   tracker_info.vcslong_sorted_next_track[2]->id);
   CHECK_EQUAL(1U,   tracker_info.vcslong_sorted_next_track[3]->id);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_next_track[0]);
}

/**
*\purpose  Check if new object will be initilized from cluster even though other moving non-occluding object exists
*\req    FTCP-13056
*/
TEST(f360_initialize_tracked_objects, Single_Non_Occluded_Cluster__Successful_Initilization)
{
   /** \precond
    * Setting up cluster with 2 dets from current time instance and 2 hist dets - valid for initalization
    * Setting up moving high confidence object which does not occlude created cluster and triggering occlusion map creation
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   
   const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, object_tracks, det_props);
   F360_Object_Track_T& object_track_to_create = object_tracks[new_id - 1];
   Set_Base_Object_Parameters(object_track_to_create);
   
   // Shift position of object 
   object_track_to_create.vcs_position.x = -5.0F;
   object_track_to_create.vcs_position.y = -0.1F;
   object_track_to_create.bbox.Set_Center(object_track_to_create.vcs_position);  // Note: Object reference_point was set to CENTER in Set_Base_Object_Parameters()

   // Shift reference point of object
   object_track_to_create.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   object_track_to_create.vcs_position = object_track_to_create.bbox.Get_Corners().Front_Right(); 

   // Set confidence level
   object_track_to_create.confidenceLevel = 1.0F;

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
    * Checking if new object had been initialized correctly with proper status
    * One active object already exists, so creation of second object is expected
    */
   CHECK_EQUAL(2, tracker_info.num_active_objs);
   CHECK_EQUAL(2, tracker_info.active_obj_ids[1]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[1].status);
}

/**
*\purpose  Check if new object initilization will be stopped from cluster which is occluded by existing object
*\req    FTCP-13056
*/
TEST(f360_initialize_tracked_objects, Single_Occluded_Cluster__Unsuccessful_Initialization)
{
   /** \precond
    * Setting up cluster with 2 dets from current time instance and 2 hist dets - valid for initalization
    * Setting up moving high confidence object occluding created cluster and triggering occlusion map creation
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);

   const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, object_tracks, det_props);
   F360_Object_Track_T& object_track_to_create = object_tracks[new_id - 1];
   Set_Base_Object_Parameters(object_track_to_create);
   
   // Shift position of object
   object_track_to_create.vcs_position.x = 5.0F;
   object_track_to_create.vcs_position.y = 0.1F;
   object_track_to_create.bbox.Set_Center(object_track_to_create.vcs_position); // Note: Object reference_point was set to CENTER in Set_Base_Object_Parameters()

   // Shift reference point of object
   object_track_to_create.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   object_track_to_create.vcs_position = object_track_to_create.bbox.Get_Corners().Rear_Left();

   // Set confidence level of object
   object_track_to_create.confidenceLevel = 1.0F;

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
    * Checking if no additional object has been initialized (one already exists as set in test preconditions)
    */
   CHECK_EQUAL(1, tracker_info.num_active_objs);
   CHECK_EQUAL(non_initilized_obj_id, tracker_info.active_obj_ids[1]);
   CHECK_EQUAL(expected_non_initialized_object_status, object_tracks[1].status);
}

/**
*\purpose  Check if only one new object will be initialized when two clusters valid for initialization exist, but one of them is occluded
*\req    FTCP-13056
*/
TEST(f360_initialize_tracked_objects, Occluded_And_Non_Occluded_Cluster__Only_One_Successfull_Initialization)
{

   /** \precond
    * Setting up three cluster. Two valid for initialization, all with 2 dets form current time instant and 2 hist dets
    * Setting up moving high confidence object occluding first cluster and triggering occlusion map creation
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Unsuccesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);

   const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, object_tracks, det_props);
   F360_Object_Track_T& object_track_to_create = object_tracks[new_id - 1];
   Set_Base_Object_Parameters(object_track_to_create);
   object_track_to_create.vcs_position.x = 5.0F;
   object_track_to_create.vcs_position.y = 0.1F;
   object_track_to_create.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   object_track_to_create.confidenceLevel = 1.0F;
   Point center = object_track_to_create.vcs_position;
   object_track_to_create.bbox.Set_Center(center);

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
    * Checking if only single additional object had been initialized
    */
   CHECK_EQUAL(2, tracker_info.num_active_objs);
   CHECK_EQUAL(2, tracker_info.active_obj_ids[1]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[1].status);
   CHECK_EQUAL(non_initilized_obj_id, tracker_info.active_obj_ids[2]);
   CHECK_EQUAL(expected_non_initialized_object_status, object_tracks[2].status);
}

/**
*\purpose  Check if new object will be initialized when cluster is occluded by existing object, but contains one detection valid for liberal initialization
*\req    FTCP-13056
*/
TEST(f360_initialize_tracked_objects, Occluded_Cluster_With_Detections_Valid_For_Liberal_Initilization__Successfull_Initialization)
{
   /** \precond
    * Setting up cluster with 2 dets from current time instance and 2 hist dets - valid for initalization
    * Setting f_valid_for_liberal_initialization flag for one detection in initialized cluster
    * Set host speed to activate liberal tracking
    * Setting up moving high confidence object occluding created cluster and triggering occlusion map creation
    */
   Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(tracker_info, clusters, raw_detection_list, det_props, detection_hist, host_props);
   const int32_t active_cluster_idx = tracker_info.active_cluster_ids[0] - 1;
   const int32_t det_idx = clusters[active_cluster_idx].detids[0] - 1;
   det_props[det_idx].f_valid_for_liberal_tracking = true;
   host.speed = 20.0F;

   const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, object_tracks, det_props);
   F360_Object_Track_T& object_track_to_create = object_tracks[new_id - 1];
   Set_Base_Object_Parameters(object_track_to_create);
   object_track_to_create.vcs_position.x = 5.0F;
   object_track_to_create.vcs_position.y = 0.1F;
   object_track_to_create.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   object_track_to_create.confidenceLevel = 1.0F;

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
    * Checking if only single additional object had been initialized
    */
   CHECK_EQUAL(2, tracker_info.num_active_objs);
   CHECK_EQUAL(2, tracker_info.active_obj_ids[1]);
   CHECK_EQUAL(expected_initialized_object_status, object_tracks[1].status);
   CHECK_EQUAL(non_initilized_obj_id, tracker_info.active_obj_ids[2]);
   CHECK_EQUAL(expected_non_initialized_object_status, object_tracks[2].status);
}

/** @}*/
