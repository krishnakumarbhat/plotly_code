/** \file
 * This file contains unit tests for content of f360_try_to_merge_two_objects.cpp file
 */

#include "f360_try_to_merge_two_objects.h"
#include "f360_set_variant.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_try_to_merge_two_objects
 *  @{
 */

/** \brief
 * Test group designed for testing Try_To_Merge_Two_Objects function
 */
TEST_GROUP(f360_try_to_merge_two_objects)
{
   F360_Host_T host = {};
   F360_Globals_T globals = {};
   F360_Calibrations_T calib = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   int32_t kill_idx;
   int32_t idx1;
   int32_t idx2;
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   
   /** \setup
    * Set up object list with two active objects, that should be merged
    */
   TEST_SETUP()
   {
      // Set up tracker calibrations
      Initialize_Tracker_Calibrations(calib);
      host.dist_rear_axle_to_vcs_m = 3.0F;

      globals.f_single_front_center_radar_only = false;

      // Set up properties of the first object
      idx1 = 10;
      object_tracks[idx1].vcs_position.x = 20.0F;
      object_tracks[idx1].vcs_position.y = -5.0F;
      object_tracks[idx1].speed = 10.0F;
      object_tracks[idx1].vcs_heading = Angle{ 0.2F };
      object_tracks[idx1].f_moveable = true;
      object_tracks[idx1].f_moving = true;
      object_tracks[idx1].Update_Bbox_Size(3.8F, 2.0F);
      tracker_info.active_obj_ids[tracker_info.num_active_objs] = idx1 + 1;
      tracker_info.num_active_objs++;

      // Set up properties of the second object
      idx2 = 20;
      object_tracks[idx2].vcs_position.x = 20.5F;
      object_tracks[idx2].vcs_position.y = -5.2F;
      object_tracks[idx2].speed = 10.1F;
      object_tracks[idx2].vcs_heading = Angle{ 0.2F };
      object_tracks[idx2].f_moveable = true;
      object_tracks[idx2].f_moving = true;
      object_tracks[idx2].Update_Bbox_Size(3.1F, 1.7F);
      tracker_info.active_obj_ids[tracker_info.num_active_objs] = idx2 + 1;
      tracker_info.num_active_objs++;
      Set_Tracker_Variant(tracker_info.variant);
   }

};

/** \purpose  
 * Check if objects are merged when all merging conditions fulfilled
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects, Should_Merge_Objects_When_All_Conditions_Fulfilled)
{
   /** \precond
    * All object properites set in test setup section
    * Set expected number of objects after merge operation
    */
   const int32_t expected_num_active_objs = 1;

   /** \action
    * Run tested function
    */
   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, sep, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);

   /** \result
    * Check if overall number of objects has decreased which indicates a merge operation
    */
   CHECK_EQUAL(expected_num_active_objs, tracker_info.num_active_objs);
}

/** \purpose
 * Check if objects are not merged when some of conditions fails
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects, Should_Not_Merge_Objects_When_Any_Condition_Failed)
{
   /** \precond
    * Modify second object heading to do not match the first object
    * Set expected number of objects if no merges are done
    */
   object_tracks[idx2].vcs_heading = Angle{ 1.0F };
   const int32_t expected_num_active_objs = 2;

   /** \action
    * Run tested function
    */
   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, sep, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);

   /** \result
    * Check if overall number of objects has remained the same - no merge operations
    */
   CHECK_EQUAL(expected_num_active_objs, tracker_info.num_active_objs);
}

/** \purpose
 * Check if younger object is killed when passed as idx2
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects, Should_Kill_Younger_Object_When_Passed_As_Idx2)
{
   /** \precond
    * Set active objects lifespan, idx1 older than idx2
    * Set expected kill idx
    */
   object_tracks[idx1].time_since_cluster_created = 2.0F;
   object_tracks[idx2].time_since_cluster_created = 1.0F;

   const int32_t expected_kill_idx = 20;

   /** \action
    * Run tested function
    */
   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, sep, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);

   /** \result
    * Check if younger object has been killed
    */
   CHECK_EQUAL(expected_kill_idx, kill_idx);

}

/** \purpose
 * Check if younger object is killed when passed as idx1
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects, Should_Kill_Younger_Object_When_Passed_As_Idx1)
{
   /** \precond
    * Set active objects lifespan, idx2 older than idx1
    * Set expected kill idx
    */
   object_tracks[idx1].time_since_cluster_created = 1.0F;
   object_tracks[idx2].time_since_cluster_created = 2.0F;

   const int32_t expected_kill_idx = 10;

   /** \action
    * Run tested function
    */
   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, sep, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);

   /** \result
    * Check if younger object has been killed
    */
   CHECK_EQUAL(expected_kill_idx, kill_idx);

}

/** \purpose
 * Check if objects are not merged when pre-merge size condition not fulfilled
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects, Should_Not_Merge_Objects_When_Size_Condition_Not_Fulfilled)
{
   /** \precond
    * Set second object width to big value to enforce fail in size check
    * Set expected number of objects - no merge operations
    */
   object_tracks[idx1].Update_Bbox_Size(3.1F, 10.8F);

   const int32_t expected_num_active_objs = 2;

   /** \action
    * Run tested function
    */
   Try_To_Merge_Two_Objects(host, sensors, raw_detection_list, calib, sep, idx1, idx2, globals, object_tracks, detection_props, tracker_info, kill_idx);

   /** \result
    * Check if overall number of objects has remained the same - indicates no merge operations
    */
   CHECK_EQUAL(expected_num_active_objs, tracker_info.num_active_objs);
}

/** @}*/

/** \defgroup  f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept 
 *  @{
 */

/** \brief
 * Test group designed for testing Choose_Obj_Idx_To_Be_Kept function
 */
TEST_GROUP(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept)
{
   F360_Object_Track_T obj_1 = {};
   F360_Object_Track_T obj_2 = {};
   int32_t idx1;
   int32_t idx2;
   int32_t keep_idx;
   int32_t kill_idx;

   /** \setup
    * Set up two objects with
    *   - Filter type CTCA
    *   - Time since cluster created > 2 s
    *   - obj1 closer to host than obj2
    */
   TEST_SETUP()
   {
      idx1 = 1;
      idx2 = 2;
      obj_1.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj_1.time_since_cluster_created = 3.0F;
      obj_1.vcs_position.x = 5.0F;
      obj_1.vcs_position.y = -3.0F;
      obj_1.speed = 3.1F;

      obj_2.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj_2.time_since_cluster_created = 2.5F;
      obj_2.vcs_position.x = 8.0F;
      obj_2.vcs_position.y = -3.0F;
      obj_2.speed = 3.2F;
   }

};

/** \purpose  
 * Check that when both objects are old enough and CTCA, the idx of the closest one is selected as the keep idx.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CTCA)
{
   /** \precond
    * All object properites set in test setup section
    */
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough, CTCA and slow moving, the idx of the closest one is selected as the keep idx.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CTCA_Slow_Moving)
{
   /** \precond
    * All object properites set in test setup section
    * Set both object's speed below slow moving threshold
    */
   obj_1.speed = 2.9F;
   obj_2.speed = 2.85F;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough and CTCA, the idx of the closest one is selected as the keep idx. Object 1 is moved farther away.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CTCA_Obj1_Far)
{
   /** \precond
    * All object properites set in test setup section
    * Move obj1 farther away than obj2
    * Set expected keep idx to idx2 since it's closer.
    */
   obj_1.vcs_position.y = 10.0F;
   const int32_t exp_keep_idx = idx2;
   const int32_t exp_kill_idx = idx1;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough, CTCA and slow moving, the idx of the closest one is selected as the keep idx. Object 1 is moved farther away.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CTCA_Slow_Moving_Obj1_Far)
{
   /** \precond
    * All object properites set in test setup section
    * Move obj1 farther away than obj2
    * Set both object's speed below slow moving threshold
    * Set expected keep idx to idx2 since it's closer.
    */
   obj_1.vcs_position.y = 10.0F;
   obj_1.speed = 2.9F;
   obj_2.speed = 2.85F;
   const int32_t exp_keep_idx = idx2;
   const int32_t exp_kill_idx = idx1;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough and CCA, the idx of the closest one is selected as the keep idx.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CCA)
{
   /** \precond
    * All object properites set in test setup section
    * Set filter type to CCA for both objects
    */
   obj_1.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_2.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough, CCA and slow moving, the idx of the closest one is selected as the keep idx.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Closest_When_Both_CCA_Slow_Moving)
{
   /** \precond
    * All object properites set in test setup section
    * Set both object's speed below slow moving threshold
    * Set filter type to CCA for both objects
    */
   obj_1.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_2.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_1.speed = 2.9F;
   obj_2.speed = 2.85F;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough and slow moving and closest object is CCA while the other is CTCA, the idx of the object farthest away is selected.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Far_Obj_When_Closest_CCA)
{
   /** \precond
    * All object properites set in test setup section
    * Set filter type to CCA for the closest object
    * Set speed of both objects below slow moving threshold
    * Set expected keep id to idx2 since it's CTCA
    */
   obj_1.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_1.speed = 2.9F;
   obj_2.speed = 2.85F;
   const int32_t exp_keep_idx = idx2;
   const int32_t exp_kill_idx = idx1;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are old enough and slow moving and obj1 is farther away and CTCA while obj2 is closer and CCA, the CTCA object is selected.
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Select_Far_Obj_When_Closest_CCA_Flipped_Idx)
{
   /** \precond
    * All object properites set in test setup section
    * Change placement of obj1 and obj2
    * Set filter type to CCA for the closest object
    * Set expected keep id to idx2 since it's CTCA
    */
   obj_1.vcs_position.y = 8.0F;
   obj_2.vcs_position.y = 5.0F;
   obj_1.speed = 2.9F;
   obj_2.speed = 2.85F;
   obj_2.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are newly created, the oldest is selected (Here object 1).
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Both_New_Obj1_Older)
{
   /** \precond
    * All object properites set in test setup section
    * Set time since cluster created below 2 seconds for both objects
    * Set time since cluster created larger for object 1
    */
   obj_1.time_since_cluster_created = 1.5F;
   obj_2.time_since_cluster_created = 1.0F;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when both objects are newly created, the oldest is selected (Here object 2).
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_Both_New_Obj2_Older)
{
   /** \precond
    * All object properites set in test setup section
    * Set time since cluster created below 2 seconds for both objects
    * Set time since cluster created larger for object 1
    */
   obj_1.time_since_cluster_created = 1.5F;
   obj_2.time_since_cluster_created = 1.9F;
   const int32_t exp_keep_idx = idx2;
   const int32_t exp_kill_idx = idx1;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when one objects is newly created and the other not, the oldest is selected (Here object 2).
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_One_New_Obj2_Older)
{
   /** \precond
    * All object properites set in test setup section
    * Set time since cluster created below 2 seconds for both objects
    * Set time since cluster created larger for object 1
    */
   obj_1.time_since_cluster_created = 1.0F;
   obj_2.time_since_cluster_created = 2.9F;
   const int32_t exp_keep_idx = idx2;
   const int32_t exp_kill_idx = idx1;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** \purpose  
 * Check that when one objects is newly created and the other not, the oldest is selected (Here object 1).
 * \req
 * NA
 */
TEST(f360_try_to_merge_two_objects_Choose_Obj_Idx_To_Be_Kept, Choose_Obj_Idx_To_Be_Kept_One_New_Obj1_Older)
{
   /** \precond
    * All object properites set in test setup section
    * Set time since cluster created below 2 seconds for both objects
    * Set time since cluster created larger for object 1
    */
   obj_1.time_since_cluster_created = 2.1F;
   obj_2.time_since_cluster_created = 1.9F;
   const int32_t exp_keep_idx = idx1;
   const int32_t exp_kill_idx = idx2;

   /** \action
    * Run tested function
    */
   Choose_Obj_Idx_To_Be_Kept(idx1, idx2, obj_1, obj_2, keep_idx, kill_idx);

   /** \result
    * Check that the correct index is selected.
    */
   CHECK_EQUAL(exp_keep_idx, keep_idx);
   CHECK_EQUAL(exp_kill_idx, kill_idx);
}

/** @}*/
