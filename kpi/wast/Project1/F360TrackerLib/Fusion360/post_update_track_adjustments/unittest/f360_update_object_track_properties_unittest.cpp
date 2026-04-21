/** \file
 * This file contains unit tests for content of f360_update_object_track_properties.cpp file
 */

#include "f360_update_object_track_properties.h"
#include <CppUTest/TestHarness.h>
#include "f360_sorted_tracks_mgmt.h"

using namespace f360_variant_A;

/** \defgroup  f360_update_object_track_properties
 *  @{
 */

/** \brief
 * Group of tests covers Update_Object_Track_Properties function responsible for object size
 */
TEST_GROUP(f360_update_object_track_properties)
{
   /** \setup
   * Setting up one object with 4 associated detections, set position and various
   * properties of the 4 detections, calibrations, host props and tracker info
   */
   F360_Calibrations_T calibs;
   F360_Globals_T globals = {};
   F360_Tracker_Info_T tracker_info = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Host_T host = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   const float32_t test_eps = 1e-5F;

   /** \setup
    * Set up a general scenario with calibrations, one object with one detection on each side of
    * the object and relevant tracker info parameters.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      tracker_info.num_active_objs = 1;
      tracker_info.active_obj_ids[0] = 1;
      tracker_info.elapsed_time_s = 0.05F;

      globals.obj_mov_stat_spd_thresh = 1.0F;
      globals.f_single_front_center_radar_only = false;

      object_tracks[0].id = 1;
      object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
      object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
      object_tracks[0].vcs_heading = Angle{ 0.5F };
      object_tracks[0].speed = 10.0F;
      object_tracks[0].curvature = 0.0F;
      object_tracks[0].total_reduced_dets = 3;
      object_tracks[0].num_rr_inlier_dets = 1;
      object_tracks[0].ndets = 4;
      object_tracks[0].f_vehicular_trk = true;
      object_tracks[0].vcs_position.x = 10.0F;
      object_tracks[0].vcs_position.y = 0.0F;
      object_tracks[0].Set_Bbox_Orientation(Angle{ 0.0F });
      object_tracks[0].time_since_track_updated = 0.0F;
      object_tracks[0].f_moveable = true;
      object_tracks[0].f_moving = true;
      object_tracks[0].innovation_length = 2.5F;
      object_tracks[0].innovation_width = 1.5F;
      object_tracks[0].detids[0] = 1;
      object_tracks[0].detids[1] = 2;
      object_tracks[0].detids[2] = 3;
      object_tracks[0].detids[3] = 4;
      object_tracks[0].Update_Bbox_Size(4.0F, 2.0F);

      // Reset vcslong sorted list
      tracker_info.vcslong_sorted_start = NULL;
      for(uint32_t i = 0U; i < NUMBER_OF_OBJECT_TRACKS; i++)
      {
         tracker_info.vcslong_sorted_next_track[i] = NULL;
         tracker_info.vcslong_sorted_prev_track[i] = NULL;
      }
      // Add the single active object to the vcslong sorted list
      Sorted_Tracks_Insert(tracker_info, &(object_tracks[0]));

      // Detection in front of object
      det_props[0].f_rr_inlier = true;
      raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      det_props[0].vcs_position.x = 12.1F;
      det_props[0].vcs_position.y = 0.0F;
      // Detection behind object
      det_props[1].f_rr_inlier = true;
      raw_detect_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      det_props[1].vcs_position.x = 7.9F;
      det_props[1].vcs_position.y = 0.0F;
      // Detection to the right of object
      det_props[2].f_rr_inlier = true;
      raw_detect_list.detections[2].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      det_props[2].vcs_position.x = 10.0F;
      det_props[2].vcs_position.y = 1.1F;
      // Detection to the left of object
      det_props[3].f_rr_inlier = true;
      raw_detect_list.detections[3].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      det_props[3].vcs_position.x = 10.0F;
      det_props[3].vcs_position.y = -1.1F;

      // Assigning dummy sensor id to all presented detections
      raw_detect_list.detections[0].raw.sensor_id = 1;
      raw_detect_list.detections[1].raw.sensor_id = 1;
      raw_detect_list.detections[2].raw.sensor_id = 1;
      raw_detect_list.detections[3].raw.sensor_id = 1;
   }
};

/** \purpose  
 * Test that Update_Object_Track_Properties() sets object total_reduced_dets correctly
 * \req
 * NA
 */
TEST(f360_update_object_track_properties, Update_Object_Track_Properties_sets_total_reduced_dets)
{
   /** \precond
    * Set object total_reduce_dets to 3 and set object num_reduce_dets to 1, to check that they are
    * added together in total_reduced_dets after function call
    */
   object_tracks[0].total_reduced_dets = 3;
   object_tracks[0].num_rr_inlier_dets = 1;
   uint32_t exp_tot_dets = object_tracks[0].total_reduced_dets + object_tracks[0].num_rr_inlier_dets;

   /** \action
    * Call Update_Object_Track_Properties()
    */
   Update_Object_Track_Properties(calibs, globals, raw_detect_list, host, sensors, tracker_info, det_props, object_tracks, timing_info);

   /** \result
    * Check that total_reduced_dets are matching expected data
    */
   CHECK_EQUAL(exp_tot_dets, object_tracks[0].total_reduced_dets)
}

/** \purpose
 * Test that Mark_Det_To_Use_For_Dim_Update() is called
 * \req
 * NA
 */
TEST(f360_update_object_track_properties, Mark_Det_To_Use_For_Dim_Update_is_called)
{
   /** \precond
    * Set object f_moveable to true
    * Set object f_moving to false
    * Set detection parameters so one of them are used for dim update
    */
   object_tracks[0].f_moveable = true;
   object_tracks[0].f_moving = false;

   det_props[0].f_use_in_dimension_update = false;
   raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_props[0].f_rr_inlier = true;
   det_props[1].f_use_in_dimension_update = false;
   raw_detect_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   det_props[1].f_rr_inlier = false;

   /** \action
    * Call Update_Object_Track_Properties()
    */
   Update_Object_Track_Properties(calibs, globals, raw_detect_list, host, sensors, tracker_info, det_props, object_tracks, timing_info);

   /** \result
    * Check that f_use_in_dimension_update are true for first detection and not second detection
    */
   CHECK_TRUE(det_props[0].f_use_in_dimension_update)
   CHECK_FALSE(det_props[1].f_use_in_dimension_update)
}

/** \purpose
 * Test that Calc_Obj_Size_Accuracy() is not called when object has less than 2 associated detections
 * \req
 * NA
 */
TEST(f360_update_object_track_properties, Calc_Obj_Size_Accuracy_is_not_called)
{
   /** \precond
    * Set track filter type to CTCA
    * Set predicted speed and speed to something smaller than 1 m/s
    * Set time since cluster created to something bigger than 0.5 s
    * Set vcs position to close to host (within 10 m in both directions)
    * Set object innovation/accuracy length and width to some values
    */
   object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object_tracks[0].predicted_speed = 0.5F;
   object_tracks[0].speed = 0.5F;
   object_tracks[0].time_since_cluster_created = 0.6F;
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 5.0F;
   object_tracks[0].accuracy_length = 1.0F;
   object_tracks[0].accuracy_width = 0.0F;
   object_tracks[0].innovation_length = 2.0F;
   object_tracks[0].innovation_width = 1.0F;
   object_tracks[0].ndets = 1;

   float32_t exp_acc_len = object_tracks[0].accuracy_length;
   float32_t exp_acc_wid = object_tracks[0].accuracy_width;

   /** \action
    * Call Update_Object_Track_Properties()
    */
   Update_Object_Track_Properties(calibs, globals, raw_detect_list, host, sensors, tracker_info, det_props, object_tracks, timing_info);

   /** \result
    * Check that object accuracy length and width have not been updated
    */
   DOUBLES_EQUAL(exp_acc_len, object_tracks[0].accuracy_length, test_eps)
   DOUBLES_EQUAL(exp_acc_wid, object_tracks[0].accuracy_width, test_eps)
}

/** \purpose
 * Test that Calc_Obj_Height() is called
 * \req
 * NA
 */
TEST(f360_update_object_track_properties, Calc_Obj_Height_is_called)
{
   /** \precond
    * Set object otg_height to 0.0F
    * Set object ud_mov_historic_ndets to 10.0F
    * Set object current detection count to 3
    * Set detection parameters so they will increase otg_height of the object
    */
   object_tracks[0].otg_height = 0.0F;
   object_tracks[0].ud_mov_historic_ndets = 10.0F;
   object_tracks[0].ndets = 3;
   for (uint8_t i=0U; i<3U; i++) 
   {
      raw_detect_list.detections[i].processed.vcs_position_z = -1.01F * pow(10,i);
   }

   /** \action
    * Call Update_Object_Track_Properties()
    */
   Update_Object_Track_Properties(calibs, globals, raw_detect_list, host, sensors, tracker_info, det_props, object_tracks, timing_info);

   /** \result
    * Check that Calc_Obj_Height was called and otg_height property increased
    */
   DOUBLES_EQUAL(8.82755852, object_tracks[0].otg_height, F360_EPSILON);
   CHECK_TRUE(object_tracks[0].otg_height > 0.0F);
}

/** \purpose
 * Test that Sorted_Tracks_Re_Sort() is called
 * \req
 * NA
 */
TEST(f360_update_object_track_properties, Sorted_Tracks_Re_Sort_is_called)
{
   /** \precond
    * From test group setup we have one active object at vcs_position [10, 0]
    * Create a second active object with a smaller vcs_position [-10, 0]
    * Set the f_moveable flag to fasle for both objects and the reference point to be center (such that the vcs_position are not changed by call to Update_Object_Reference_Point())
    * Set up the vcslong sorted list such that is in incorrect order. 
    * I.e. such that object with id 1 is first in the list (already set up from test group setup) and object with id 2 is second in te list
    */
   object_tracks[0].f_moveable = false;
   object_tracks[0].reference_point = F360_REFERENCE_POINT_CENTER;

   object_tracks[1].id = 2;
   object_tracks[1].vcs_position.x = -10.0F;
   object_tracks[1].vcs_position.y = 0.0F;
   object_tracks[1].f_moveable = false;
   object_tracks[1].reference_point = F360_REFERENCE_POINT_CENTER;

   tracker_info.active_obj_ids[1] = 2;
   tracker_info.num_active_objs = 2;

   tracker_info.vcslong_sorted_next_track[0] = &(object_tracks[1]);
   tracker_info.vcslong_sorted_prev_track[1] = &(object_tracks[0]);


   /** \action
    * Call Update_Object_Track_Properties()
    */
   Update_Object_Track_Properties(calibs, globals, raw_detect_list, host, sensors, tracker_info, det_props, object_tracks, timing_info);

   /** \result
    * Check that the sorted list is now in correct order. I.e. that object with id 2 is first and object with id 1 is second
    */
   POINTERS_EQUAL_TEXT(&(object_tracks[1]), tracker_info.vcslong_sorted_start, "Start of vcslong sorted list is wrong");
   POINTERS_EQUAL_TEXT(&(object_tracks[0]), tracker_info.vcslong_sorted_next_track[1], "The track coming after object with id 2 is wrong");
   POINTERS_EQUAL_TEXT(NULL, tracker_info.vcslong_sorted_next_track[0], "The track coming after object with id 1 is wrong");
   POINTERS_EQUAL_TEXT(NULL, tracker_info.vcslong_sorted_prev_track[1], "The track coming before object with id 2 is wrong");
   POINTERS_EQUAL_TEXT(&(object_tracks[1]), tracker_info.vcslong_sorted_prev_track[0], "The track coming before object with id 1 is wrong");
}

/** @}*/
