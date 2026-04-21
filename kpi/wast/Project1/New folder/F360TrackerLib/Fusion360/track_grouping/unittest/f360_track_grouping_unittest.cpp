/** \file
 * This file contains unit tests for content of f360_track_grouping.cpp file
 */

#include "f360_track_grouping.h"
#include <CppUTest/TestHarness.h>

#include "f360_static_env_helpers.h"
#include "f360_set_variant.h"

using namespace f360_variant_A;

/** \defgroup  f360_track_grouping
 *  @{
 */

/** \brief
 *  Test group for unit testing Track_Grouping function.
 */
TEST_GROUP(f360_track_grouping_ut)
{
   //Initialize common variables used within all tests in this test group.
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Globals_T globals = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   F360_Calibrations_T calib;
   Static_Env_Poly_T static_env_polys[F360_NUM_OF_STATIC_ENV_POLYS];
   F360_Host_T host = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
      
   /** \setup
    * Initialize calibrations, globals and common tracker_info and objects properties.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      Set_Tracker_Variant(tracker_info.variant);

      for (uint32_t idx = 0U; idx < F360_NUM_OF_STATIC_ENV_POLYS; idx++)
      {
         Reset_Single_Static_Env_Poly(static_env_polys[idx]);
      }

      //globals.obj_mov_stat_spd_thresh = 4.0F;
      globals.f_single_front_center_radar_only = false;

      object_tracks[20].id = 21;
      object_tracks[20].vcs_position.x = 5.0F;
      object_tracks[20].vcs_position.y = 5.0F;
      object_tracks[20].reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      object_tracks[20].Update_Bbox_Size(3.0F, 1.0F);
      object_tracks[20].Set_Bbox_Orientation(Angle{ 0.0F });
      object_tracks[20].Update_Bbox_Center();

      object_tracks[25].id = 26;
      object_tracks[25].vcs_position.x = 5.1F;
      object_tracks[25].vcs_position.y = 5.2F;
      object_tracks[25].reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      object_tracks[25].Update_Bbox_Size(3.0F, 1.0F);
      object_tracks[25].Set_Bbox_Orientation(Angle{ 0.0F });
      object_tracks[25].Update_Bbox_Center();

      const int32_t active_obj_id_1 = 21;
      const int32_t active_obj_id_2 = 26;
      tracker_info.num_active_objs = 0;
      int32_t num_inactive_objs = 0;
      for (int32_t obj_id = 1; obj_id <= static_cast<int32_t>(tracker_info.variant.num_tracks); obj_id++)
      {
         if((obj_id == active_obj_id_1) || (obj_id == active_obj_id_2))
         {
            tracker_info.active_obj_ids[tracker_info.num_active_objs] = obj_id;
            tracker_info.num_active_objs++;
         }
         else
         {
            tracker_info.inactive_obj_ids[num_inactive_objs] = obj_id;
            num_inactive_objs++;
         }

      }
      tracker_info.active_obj_ids[0] = active_obj_id_1;
      tracker_info.active_obj_ids[1] = active_obj_id_2;
      tracker_info.vcslong_sorted_start = &(object_tracks[20]);
      tracker_info.vcslong_sorted_prev_track[active_obj_id_1 - 1] = NULL;
      tracker_info.vcslong_sorted_next_track[active_obj_id_1 - 1] = &(object_tracks[active_obj_id_2 -1]);
      tracker_info.vcslong_sorted_prev_track[active_obj_id_2 - 1] = &(object_tracks[active_obj_id_1 - 1]);
      tracker_info.vcslong_sorted_next_track[active_obj_id_2 - 1] = NULL;
   }
};

/** \purpose  
 *  Test checks whether for loop will be break in first iteration when vcslong sorted list
 *  is not filled properly.
 *
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Invalid_Vcslong_Sorted_List)
{
   /** \precond
    * Vcslong_sorted list starts from NULL pointer.
    * Several objects are active.
    */
   tracker_info.vcslong_sorted_start = NULL;
   tracker_info.num_active_objs = 2;

   int32_t exp_num_active_obj = tracker_info.num_active_objs;

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Check if object vcs_long_soretd list and number of active objects have not been changed.
    */
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_start);
   CHECK_EQUAL(exp_num_active_obj, tracker_info.num_active_objs);
}

/** \purpose
 *  Test checks whether two objects merge fails when first movable object has too small speed for merge.
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Speed_Diff_Is_Below_Thr_For_Movable_Object)
{
   /** \precond
    * Define two active, movable objects and first object speed is below minimum speed threshold for object merge.
    */
   tracker_info.num_active_objs = 2;

   object_tracks[20].f_moveable = true;
   object_tracks[20].speed = globals.obj_mov_stat_spd_thresh - 1.0F;
   
   object_tracks[25].f_moveable = true;
   object_tracks[25].speed = globals.obj_mov_stat_spd_thresh + 1.0F;
   
   int32_t exp_num_active_obj = tracker_info.num_active_objs;

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Check if object vcs_long_soretd list and number of active objects have not been changed.
    */
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_start);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_prev_track[20]);
   POINTERS_EQUAL((&object_tracks[25]), tracker_info.vcslong_sorted_next_track[20]);
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_prev_track[25]);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_next_track[25]);
   CHECK_EQUAL(exp_num_active_obj, tracker_info.num_active_objs);
}

/** \purpose
 *  Test checks whether two non movable objects does not merge when their are to far away from each other.
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Too_Big_Long_Distance_Between_Non_Movable_Objects)
{
   /** \precond
    * Define two active not movable objects, their longitudinal distance is above maximal threshold for merging two objects.
    */
   tracker_info.num_active_objs = 2;

   int32_t exp_num_active_obj = tracker_info.num_active_objs;

   object_tracks[20].f_moveable = false;
   object_tracks[20].speed = 1.0F;

   object_tracks[25].f_moveable = false;
   object_tracks[25].speed = 1.0F;
   object_tracks[25].vcs_position.x = object_tracks[20].vcs_position.x + (4.0F * object_tracks[20].bbox.Get_Length());
   object_tracks[25].Update_Bbox_Center();

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Check if object vcs_long_soretd list and number of active objects have not been changed.
    */
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_start);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_prev_track[20]);
   POINTERS_EQUAL((&object_tracks[25]), tracker_info.vcslong_sorted_next_track[20]);
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_prev_track[25]);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_next_track[25]);
   CHECK_EQUAL(exp_num_active_obj, tracker_info.num_active_objs);
}


/** \purpose
 *  Test checks whether two movable objects does not merge when their are to far away from each other.
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Movable_Objects_Should_Not_Be_Merged_Too_Far_Away)
{
   /** \precond
    * Define two active movable objects, their longitudinal distance is above maximal threshold for merging two objects.
    */
   tracker_info.num_active_objs = 2;

   int32_t exp_num_active_obj = tracker_info.num_active_objs;

   object_tracks[20].f_moveable = true;
   object_tracks[20].f_moving = true;
   object_tracks[20].speed = globals.obj_mov_stat_spd_thresh + 1.0F;

   object_tracks[25].f_moveable = true;
   object_tracks[25].f_moving = true;
   object_tracks[25].speed = globals.obj_mov_stat_spd_thresh + 1.0F;
   object_tracks[25].vcs_position.x = object_tracks[20].vcs_position.x + 2.0F * object_tracks[20].bbox.Get_Length() + 0.01F;
   object_tracks[25].Update_Bbox_Center();

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Check if object vcs_long_soretd list and number of active objects have been properly updated.
    */
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_start);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_prev_track[20]);
   POINTERS_EQUAL((&object_tracks[25]), tracker_info.vcslong_sorted_next_track[20]);
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_prev_track[25]);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_next_track[25]);
   CHECK_EQUAL(exp_num_active_obj, tracker_info.num_active_objs);
}

/** \purpose
 *  Test checks that two objects are not merged when one is not moving.
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Movable_Objects_Should_Not_Be_Merged_One_Not_Moving)
{
   /** \precond
    * Define two active movable objects, their longitudinal distance is below maximal threshold for merging two objects.
    * Set first object to moving and the second to not moving
    */
   tracker_info.num_active_objs = 2;

   int32_t exp_num_active_obj = tracker_info.num_active_objs;

   object_tracks[20].f_moveable = true;
   object_tracks[20].f_moving = true;
   object_tracks[20].speed = globals.obj_mov_stat_spd_thresh + 1.0F;

   object_tracks[25].f_moveable = false;
   object_tracks[25].f_moving = false;
   object_tracks[25].speed = globals.obj_mov_stat_spd_thresh + 1.0F;

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Check if object vcs_long_soretd list and number of active objects have been properly updated.
    */
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_start);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_prev_track[20]);
   POINTERS_EQUAL((&object_tracks[25]), tracker_info.vcslong_sorted_next_track[20]);
   POINTERS_EQUAL((&object_tracks[20]), tracker_info.vcslong_sorted_prev_track[25]);
   POINTERS_EQUAL(NULL, tracker_info.vcslong_sorted_next_track[25]);
   CHECK_EQUAL(exp_num_active_obj, tracker_info.num_active_objs);
}

/** \purpose
 *  Test checks that split logic is called as expected from
 *  top level function Track_Grouping()
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Track_Grouping_Verify_Split_Logic)
{
   /** \precond
    * Set tracker info to have 1 active object 
    * Initialize an object that should trigger a split.
    */
   tracker_info.num_active_objs = 1;
   tracker_info.inactive_obj_ids[tracker_info.variant.num_tracks  - 2] = tracker_info.active_obj_ids[1];
   tracker_info.active_obj_ids[1] = 0;

   object_tracks[20].speed = calib.k_orth_split_min_speed + 1.0F;
   object_tracks[20].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object_tracks[20].orth_gap_filtered = calib.k_orth_split_min_orth_gap_for_split + 1.0F;
   object_tracks[20].orth_delta_filtered = object_tracks[20].orth_gap_filtered + 1.0F;

   /** \action
    *  call Track_Grouping().
    */
   Track_Grouping(calib, static_env_polys, host, sensors, raw_detection_list, globals, object_tracks, detection_props, tracker_info, timing_info);

   /** \result
    *  Verify that split happened by checking that the number of
    *  active objects have increased by 1
    */
   CHECK_EQUAL(2, tracker_info.num_active_objs);
}

/** \purpose
 *  Test checks if calibration values are correctly set up.
 * \req  NA.
 */
TEST(f360_track_grouping_ut, Verify_Track_Grouping_Calibration_Values)
{
   /** \precond
   * Set test tolerance.
   */
   const float32_t test_tolerance = 1e-5F;

   /** \result
    */
   DOUBLES_EQUAL(0.75F, calib.k_track_grouping_half_long_extension, test_tolerance);
   DOUBLES_EQUAL(0.0F, calib.k_track_grouping_half_lat_extension, test_tolerance);
   DOUBLES_EQUAL(0.523599F, calib.k_track_grouping_hdg_gate, test_tolerance);
   DOUBLES_EQUAL(1.0F, calib.k_track_grouping_speed_gate, test_tolerance);
   DOUBLES_EQUAL(2.2F, calib.merging_m2m_distance_threshold, test_tolerance);
   DOUBLES_EQUAL(4.0F, calib.merging_lateral_det_spread_threshold, test_tolerance);
   DOUBLES_EQUAL(6.94444F, calib.merging_m2m_max_obj_speed, test_tolerance);
}

/** @}*/
