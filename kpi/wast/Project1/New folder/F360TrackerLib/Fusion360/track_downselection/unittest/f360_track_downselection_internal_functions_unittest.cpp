/** \file
   Give a detailed description of what  this unit-test file contain.
*/

#include "f360_track_downselection_internal_functions.h"
#include "f360_bounding_box.h"
#include "f360_math.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>

using namespace f360_variant_A;

/** \defgroup  Decrease_Priority_and_Confidence_for_Implausible_Tracks
 *  @{
 */

/** \brief
 *  Test group for Decrease_Priority_and_Confidence_for_Implausible_Tracks()
 **/
TEST_GROUP(Decrease_Priority_and_Confidence_for_Implausible_Tracks)
{
   F360_Host_T host;
   F360_Calibrations_T calib = {};
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   float32_t initial_priority = 0.0F;
   float32_t initial_confidenceLevel = 1.0F;
   const float32_t test_pass_th = 1e-8F;

   TEST_SETUP()
   {
      object_tracks[0].filtered_dets = 1.25F; // required for an object to NOT be ghost type 1 and not decrease its confidence
      object_tracks[0].total_reduced_dets = 1; // required for an object to NOT be ghost type 1 and not decrease its confidence
      object_tracks[0].time_since_initialization = 20.0F; // required for an object to NOT be ghost type 1 and not decrease its confidence
      object_tracks[0].conf_overall = CONF3_HIGH;
      Initialize_Tracker_Calibrations(calib);
      Set_Tracker_Variant(tracker_info.variant);
   }
};

/** \brief
 *  Test checks if not ghost (any type 1 or 2) and not mirror object track has not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_not_fast_yawing_ghost)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 2

   object_tracks[0].speed = -0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed too low for ghost type 2
   object_tracks[0].curvature = 0.99F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature too low for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if not ghost candidate with curvature (and yawrate) insufficient to be fast yawinghas not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_candidate_with_insuffcient_curvature)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 0.99F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature too low for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if not ghost candidate with speed insufficient to be fast yawinghas not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_candidate_with_insuffcient_speed)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 2

   object_tracks[0].speed = -0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed too low for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if type 2 ghost object track has modified priority and confidence when speed and curvature are both sufficient for fast yawing.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_change_priority_and_confidence_for_fast_yawing_ghost)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should be modifie
   */
   CHECK_TRUE(object_tracks[0].confidenceLevel < initial_confidenceLevel)
   CHECK_TRUE(priority == calib.k_track_downselect_max_priority);
}

/** \brief
 *  Test verifies that neither confidence nor priority was not modified for fast yawing track that is neither on nor behind sep.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_ghost_not_on_nor_behind_sep)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = F360_INVALID_UNSIGNED_ID; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if fast yawing track but not movable has not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_but_not_movable_ghost)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel; 

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = false; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if fast yawing track but not movable has not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_ghost_behind_guardrail_but_not_movable)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = 1U;
   object_tracks[0].on_sep_id = F360_INVALID_UNSIGNED_ID; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = false; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if fast yawing track but not movable has not modified priority and confidence.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_not_change_priority_and_confidence_for_fast_yawing_ghost_on_guardrail_but_not_movable)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;

   object_tracks[0].behind_sep_id = F360_INVALID_UNSIGNED_ID;
   object_tracks[0].on_sep_id = 1U; // required true for an object to be ghost type 2
   object_tracks[0].f_moveable = false; // required true for an object to be ghost type 2

   object_tracks[0].speed = 0.1F + calib.k_track_downselect_max_allowed_speed_for_non_fast_yawing_object; // speed sufficient for ghost type 2
   object_tracks[0].curvature = 1.01F * (calib.k_track_downselect_max_allowed_yawrate_for_non_fast_yawing_object / object_tracks[0].speed); // curvature sufficient for ghost type 2

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if priority and confidence is changed for an object that is behind guardrail, has low confidence and low number of filtered detections.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_does_change_priority_and_confidence_for_objects_behind_guardrail_low_num_filtered_dets_low_conf)
{
   /** \precond
   */

   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].behind_sep_id = 1.0F; // required for an object to be ghost type 1
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 1
   object_tracks[0].filtered_dets = 0.3F;

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should be modified
   */
   DOUBLES_EQUAL(calib.k_track_downselect_confidence_level_lowering_factor * calib.low_confidence_level_thresh, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(calib.k_track_downselect_max_priority, priority, test_pass_th);
}

/** \brief
 *  Test checks if priority and confidence have not changed for an object that is behind guardrail, has high confidence and low number of filtered detections.
 **/
TEST(Decrease_Priority_and_Confidence_for_Implausible_Tracks, check_if_function_does_not_change_priority_and_confidence_for_objects_behind_guardrail_low_num_filtered_dets_high_conf)
{
   /** \precond
   */
   object_tracks[0].confidenceLevel = initial_confidenceLevel;
   object_tracks[0].behind_sep_id = 1.0F; // required for an object to not be ghost type 1
   object_tracks[0].f_moveable = true; // required true for an object to be ghost type 1
   object_tracks[0].filtered_dets = 0.3F;

   /** \action
      Call Decrease_Priority_and_Confidence_for_Implausible_Tracks
   */
   const float32_t priority = Decrease_Priority_and_Confidence_for_Implausible_Tracks(object_tracks[0], tracker_info, calib, initial_priority);

   /** \result
      After calling the function the priority and confidence should  NOT be modified
   */
   DOUBLES_EQUAL(initial_confidenceLevel, object_tracks[0].confidenceLevel, test_pass_th);
   DOUBLES_EQUAL(initial_priority, priority, test_pass_th);
}

/** @}*/


/** \defgroup  Pop_Reduced_Id
 *  @{
 */

/** \brief
 *  Test group for Pop_Reduced_Id()
 **/
TEST_GROUP(Pop_Reduced_Id)
{
   F360_Tracker_Info_T tracker_info = {};
   
   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
   }
};

/** \brief
 *  Test checks if first track idx from tracker_info.reduced_inactive_obj_ids is issued.
 **/
TEST(Pop_Reduced_Id, check_if_function_issue_track_idx)
{

   /** \precond
   */
   int32_t reduced_id = 0;
   tracker_info.reduced_num_active_objs = 4;
   int32_t num_obj = tracker_info.reduced_num_active_objs;
   
   for (unsigned int i = 0; i < NUMBER_OF_REDUCED_OBJECT_TRACKS - tracker_info.reduced_num_active_objs; i++)
   {
      tracker_info.reduced_inactive_obj_ids[i] = i + 1 + tracker_info.reduced_num_active_objs;
   }

   int32_t expexted_track_id = tracker_info.reduced_inactive_obj_ids[0];

   /** \action
      Call Pop_Reduced_Id
   */
   reduced_id = Pop_Reduced_Id(tracker_info);

   /** \result
      After pop first element array is shifted to the front and last element should be equal 0
   */
   CHECK_EQUAL(0, tracker_info.reduced_inactive_obj_ids[NUMBER_OF_REDUCED_OBJECT_TRACKS - tracker_info.reduced_num_active_objs]);
   CHECK_EQUAL(expexted_track_id, reduced_id);
   CHECK_EQUAL(reduced_id, tracker_info.reduced_active_obj_ids[num_obj]);
   CHECK_EQUAL(num_obj + 1, tracker_info.reduced_num_active_objs);
}

/** \defgroup  Calc_Track_Priority
 *  @{
 */

/** \brief
 *  Test group for Calc_Track_Priority()
 **/
TEST_GROUP(Calc_Track_Priority)
{
   F360_Host_T host;
   F360_Calibrations_T calib = {};
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   Static_Env_Poly_T stat_env_poly[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   BoundingBox overall_confidence_exclusion_box{};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};

/**
*\purpose  Test checks if function sets max priority when object status lower than F360_OBJECT_STATUS_NEW_UPDATED ( lower than 3).
           Higher priority value means that object is less important, lower means that object is more important.
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_set_max_priority_when_object_has_status_lower_than_F360_OBJECT_STATUS_NEW_UPDATED)
{
   /** \precond
   */
   object_tracks[0].status = F360_OBJECT_STATUS_NEW_COASTED;
   tracker_info.elapsed_time_s = 0.001F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
   CHECK_EQUAL(calib.k_track_downselect_max_priority, priority);
}

/**
*\purpose  Test checks if function sets max priority when object is valid for liberal tracking
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_minimum_priotiry_is_set_for_object_valid_for_liberal_tracking)
{
   /** \precond
   */
   object_tracks[0].f_valid_for_liberal_tracking = true;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
  DOUBLES_EQUAL(-2.0F, priority, F360_EPSILON);
}

/**
*\purpose  Test checks if function sets max priority when object status is equal F360_OBJECT_STATUS_UPDATED and 
           reduced_status == F360_OBJECT_STATUS_INVALID and confidenceLevel < calib->re_down_select_confidence_thresh
           Higher priority value means that object is less important, lower means that object is more important.
*\req    NA
**/
TEST(Calc_Track_Priority, if_set_max_priority)
{  
   /** \precond
   */
   tracker_info.elapsed_time_s = 0.001F;
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_INVALID;
   object_tracks[0].confidenceLevel = 0.3F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
   CHECK_EQUAL(calib.k_track_downselect_max_priority, priority);
}

/**
*\purpose  Test checks if function sets lowest possible priority value (-1.0) when object is near host.
           Higher priority value means that object is less important, lower means that object is more important.
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_target_near_host_set_high_priority)
{
   /** \precond
   */
   tracker_info.elapsed_time_s = 0.001F;
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].vcs_position.x = 1.0F;
   object_tracks[0].vcs_position.y = 1.0F;
   object_tracks[0].conf_overall = CONF3_HIGH;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
    priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

    /** \result
    */
   CHECK_EQUAL(-1.0F, priority);
}

/**
*\purpose  Test checks if function sets priority below 10 when objects ttc (time to collision) is higher than -1
           Higher priority value means that object is less important, lower means that object is more important.
           Negative ttc means that target has projected velocity towards host.
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_ttc_higher_than_minus_1_then_priority_below_10)
{
   /** \precond
   */
   tracker_info.elapsed_time_s = 0.001F;
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].conf_overall = CONF3_HIGH;
   object_tracks[0].vcs_position.x = 15.0F;
   object_tracks[0].vcs_position.y = 9.0F;
   object_tracks[0].vcs_velocity.longitudinal = 2.0F;
   object_tracks[0].vcs_velocity.lateral = 2.0F;
   object_tracks[0].f_moveable = true;
   object_tracks[0].conf_overall = CONF3_HIGH;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   host.vcs_speed = 10.0F;
   host.vcs_sideslip = 0.01F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
   CHECK_TRUE(priority <= 10.0F);
}

/**
*\purpose  Test checks if function set priority above 7 when objects ttc (time to collision) is lower than -1.
           Higher priority value means that object is less important, lower means that object is more important.
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_ttc_is_lower_than_minus_1_then_priority_higher_tan_7)
{
   /** \precond
   */
   tracker_info.elapsed_time_s = 0.001F;
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].vcs_position.x = 15.0F;
   object_tracks[0].vcs_position.y = 9.0F;
   object_tracks[0].vcs_velocity.longitudinal = 20.0F;
   object_tracks[0].vcs_velocity.lateral = 2.0F;
   object_tracks[0].f_moveable = true;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   host.vcs_speed = 10.0F;
   host.vcs_sideslip = 0.01F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
   CHECK_TRUE(priority > 7.0F);
}

/**
*\purpose  Test checks if function sets priority higher than 250 when object has marked as nonmovable.
           Higher priority value means that object is less important, lower means that object is more important.
*\req    NA
**/
TEST(Calc_Track_Priority, Check_if_ttc_for_nonmovable_objects_is_higher_than_250)
{
   /** \precond
   */
   tracker_info.elapsed_time_s = 0.001F;
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].vcs_position.x = 15.0F;
   object_tracks[0].vcs_position.y = 9.0F;
   object_tracks[0].vcs_velocity.longitudinal = 20.0F;
   object_tracks[0].vcs_velocity.lateral = 2.0F;
   object_tracks[0].f_moveable = false;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   host.vcs_speed = 10.0F;
   host.vcs_sideslip = 0.01F;
   float32_t priority = 0.0F;

   /** \action
      Call Calc_Track_Priority
   */
   priority = Calc_Track_Priority(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), calib, tracker_info, overall_confidence_exclusion_box, object_tracks[0]);

   /** \result
   */
   CHECK_TRUE(priority > 250.0F)
}
/** @}*/


/** \defgroup  Assign_Reduced_Idxs_To_Prioritized_Tracks
 *  @{
 */

/** \brief
 *  Test group for Assign_Reduced_Idxs_To_Prioritized_Tracks()
 **/
TEST_GROUP(Assign_Reduced_Idxs_To_Prioritized_Tracks)
{
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   F360_Tracker_Info_T tracker_info = {};

   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
   }
};

/**
*\purpose  Test checks if reduced ids are assigned to tracks
*\req    NA
**/
TEST(Assign_Reduced_Idxs_To_Prioritized_Tracks, Check_if_reduced_idxs_are_assigned_to_tracks)
{
   /** \precond
   */
   int32_t candidates_ids[NUMBER_OF_OBJECT_TRACKS]{1,2,3,4,5,6,7,8,9,10};
   uint32_t ids_of_objs_sorted_by_priority[NUMBER_OF_OBJECT_TRACKS] = {3,5,7};
   uint32_t candidates_cnt = 3;
   object_tracks[4].reduced_id = 1;
   object_tracks[4].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[4].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   object_tracks[6].reduced_id = 2;
   object_tracks[6].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[6].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   object_tracks[8].reduced_id = 3;
   object_tracks[8].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[8].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   
   /** \action
      Call Assign_Reduced_Idxs_To_Prioritized_Tracks
   */
   Assign_Reduced_Idxs_To_Prioritized_Tracks( object_tracks, tracker_info, candidates_ids, ids_of_objs_sorted_by_priority, candidates_cnt);

   /** \result
   */
   CHECK_EQUAL(object_tracks[4].reduced_status, F360_OBJECT_STATUS_UPDATED)
   CHECK_EQUAL(tracker_info.reduced_obj_ids[object_tracks[4].reduced_id - 1], 4+1)
   CHECK_EQUAL(object_tracks[6].reduced_status, F360_OBJECT_STATUS_UPDATED)
   CHECK_EQUAL(tracker_info.reduced_obj_ids[object_tracks[6].reduced_id - 1], 6 + 1)
   CHECK_EQUAL(object_tracks[8].reduced_status, F360_OBJECT_STATUS_UPDATED)
   CHECK_EQUAL(tracker_info.reduced_obj_ids[object_tracks[8].reduced_id - 1], 8 + 1)
}

/**
*\purpose  Test checks if function gets obj id from tracker_info.reduced_inactive_obj_ids[]
*\req    NA
**/
TEST(Assign_Reduced_Idxs_To_Prioritized_Tracks, Check_if_get_obj_idx_from_tracker_info)
{
   /** \precond
   */
   int32_t candidates_ids[NUMBER_OF_OBJECT_TRACKS]{1,2,3,4,5,6,7,8,9,10};
   uint32_t ids_of_objs_sorted_by_priority[NUMBER_OF_OBJECT_TRACKS] = { 3,5,7 };
   uint32_t candidates_cnt = 3;

   for (unsigned int i = 0; i < NUMBER_OF_REDUCED_OBJECT_TRACKS; i++)
   {
      tracker_info.reduced_inactive_obj_ids[i] = i + 1;
   }

   object_tracks[4].reduced_id = 0;
   object_tracks[4].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[4].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   object_tracks[6].reduced_id = 0;
   object_tracks[6].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[6].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   object_tracks[8].reduced_id = 0;
   object_tracks[8].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[8].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;

   /** \action
      Call Assign_Reduced_Idxs_To_Prioritized_Tracks
   */
   Assign_Reduced_Idxs_To_Prioritized_Tracks(object_tracks, tracker_info, candidates_ids, ids_of_objs_sorted_by_priority, candidates_cnt);
  
   /** \result
   */
   CHECK_EQUAL(object_tracks[4].reduced_status, F360_OBJECT_STATUS_NEW);
   CHECK_EQUAL(object_tracks[4].reduced_id, 1);
   CHECK_EQUAL(tracker_info.reduced_obj_ids[object_tracks[4].reduced_id - 1], 4 + 1);
   CHECK_EQUAL(tracker_info.reduced_inactive_obj_ids[0], 4 );
}

/** \defgroup  Deselect_Existing_Reduced_Tracks
 *  @{
 */

/** \brief
 *  Test group for Deselect_Existing_Reduced_Tracks()
 **/
TEST_GROUP(Deselect_Existing_Reduced_Tracks)
{
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   F360_Tracker_Info_T tracker_info;

   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
   }
};

/**
*\purpose  Test checks if function sets reduced status to F360_OBJECT_STATUS_INVALID and reduced id to zero.
*\req    NA
**/
TEST(Deselect_Existing_Reduced_Tracks, Check_if_set_reduced_status_and_reduced_id_on_invalid)
{
   /** \precond
   */
   int32_t candidates_ids[NUMBER_OF_OBJECT_TRACKS]{ 1,2,3,4,5,6,7,8,9,10 };
   uint32_t ids_of_objs_sorted_by_priority[NUMBER_OF_OBJECT_TRACKS]{};
   ids_of_objs_sorted_by_priority[500] = 4;
   uint32_t candidates_cnt = 501;
   tracker_info.reduced_num_active_objs = 1;

   for (unsigned int i = 0; i < NUMBER_OF_REDUCED_OBJECT_TRACKS; i++)
   {
      tracker_info.reduced_inactive_obj_ids[i] = i + 1;
   }

   object_tracks[5].reduced_id = 5;
   object_tracks[5].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[5].reduced_status = F360_OBJECT_STATUS_NEW_COASTED;
   tracker_info.reduced_active_obj_ids[5] = 5;

   /** \action
      Call Deselect_Existing_Reduced_Tracks
   */
   Deselect_Existing_Reduced_Tracks(object_tracks, tracker_info, candidates_ids, ids_of_objs_sorted_by_priority, candidates_cnt);
   
   /** \result
   */
   CHECK_EQUAL(object_tracks[5].reduced_status, F360_OBJECT_STATUS_INVALID);
   CHECK_EQUAL(object_tracks[5].reduced_id, 0);
   CHECK_EQUAL(candidates_cnt, NUMBER_OF_REDUCED_OBJECT_TRACKS);
}

/** \defgroup  Select_Obj_Tracks_to_Downselect
 *  @{
 */

/** \brief
 *  Test group for Select_Obj_Tracks_to_Downselect()
 **/
TEST_GROUP(Select_Obj_Tracks_to_Downselect)
{
   F360_Host_T host;
   F360_Calibrations_T calib = {};
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   Static_Env_Poly_T stat_env_poly[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      Set_Tracker_Variant(tracker_info.variant);
   }
};

/**
*\purpose  Test checks if function issues candidates_ids and priorities.
*\req    NA
**/
TEST(Select_Obj_Tracks_to_Downselect, Check_if_issue_priorities_and_candidates)
{
   /** \precond
   */
   float32_t priorities[NUMBER_OF_OBJECT_TRACKS]{};
   int32_t candidates_ids[NUMBER_OF_OBJECT_TRACKS]{};
   uint32_t candidates_cnt = 0;
   tracker_info.elapsed_time_s = 0.001F;
   tracker_info.num_active_objs = 1;
   
   for (unsigned int i = 0; i < NUMBER_OF_OBJECT_TRACKS; i++)
   {
      tracker_info.active_obj_ids[i] = i + 1;
   }

   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.65F;
   object_tracks[0].conf_overall = CONF3_HIGH;
   object_tracks[0].vcs_position.x = 15.0F;
   object_tracks[0].vcs_position.y = 9.0F;
   object_tracks[0].vcs_velocity.longitudinal = 20.0F;
   object_tracks[0].vcs_velocity.lateral = 2.0F;
   object_tracks[0].f_moveable = true;
   object_tracks[0].conf_overall = CONF3_HIGH;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   host.vcs_speed = 10.0F;
   host.vcs_sideslip = 0.01F;

   /** \action
      Call Select_Obj_Tracks_to_Downselect
   */
   Select_Obj_Tracks_to_Downselect(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), stat_env_poly, tracker_info, calib, object_tracks, priorities, candidates_ids, candidates_cnt);

   /** \result
   */
   CHECK_TRUE(priorities[0] > 0);
   CHECK_TRUE(candidates_cnt > 0 );
}

/**
*\purpose   Test checks if function doesn't select invalid tracks.
*\req    NA
**/
TEST(Select_Obj_Tracks_to_Downselect, Check_if_function_doesnt_select_invalid_tracks)
{
   /** \precond
   */
   float32_t priorities[NUMBER_OF_OBJECT_TRACKS]{};
   int32_t candidates_ids[NUMBER_OF_OBJECT_TRACKS]{};
   uint32_t candidates_cnt = 0;
   tracker_info.elapsed_time_s = 0.001F;
   tracker_info.num_active_objs = 1;
   tracker_info.reduced_num_active_objs = 1;

   for (unsigned int i = 0; i < NUMBER_OF_OBJECT_TRACKS; i++)
   {
      tracker_info.active_obj_ids[i] = i + 1;
   }

   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].reduced_status = F360_OBJECT_STATUS_UPDATED;
   object_tracks[0].confidenceLevel = 0.5F;
   object_tracks[0].reduced_id = 10;
   object_tracks[0].vcs_position.x = 1000000.0F;
   object_tracks[0].vcs_position.y = 1000000.0F;
   object_tracks[0].vcs_velocity.longitudinal = 20.0F;
   object_tracks[0].vcs_velocity.lateral = 2.0F;
   object_tracks[0].f_moveable = false;
   host.dist_rear_axle_to_vcs_m = 3.0F;
   host.vcs_speed = 10.0F;
   host.vcs_sideslip = 0.01F;
   tracker_info.reduced_active_obj_ids[0] = 10;

   /** \action
      Call Select_Obj_Tracks_to_Downselect
   */
   Select_Obj_Tracks_to_Downselect(host, F360_Cosf(host.vcs_sideslip), F360_Sinf(host.vcs_sideslip), stat_env_poly, tracker_info, calib, object_tracks, priorities, candidates_ids, candidates_cnt);

   /** \result
   */
   CHECK_TRUE(object_tracks[0].reduced_status == F360_OBJECT_STATUS_INVALID);
   CHECK_TRUE(object_tracks[0].reduced_id == 0);
}

/** \defgroup  Calc_Range_to_Track
 *  @{
 */

/** \brief
 *  Test group for Calc_Range_to_Track()
 **/
TEST_GROUP(Calc_Range_to_Track)
{
   F360_Tracker_Info_T tracker_info;
};

/**
*\purpose  Test checks if function returns min range in case that coordinates are small.
*\req    NA
**/
TEST(Calc_Range_to_Track, Check_if_min_range_is_issued)
{
   /** \precond
   */
   float32_t vcs_pos_longitudinal = 1.0F;
   float32_t vcs_pos_lateral = 1.0F;
   float32_t min_track_range = 2.0F;
   float32_t range;

   /** \action
      Call Calc_Range_to_Track
   */
   range = Calc_Range_to_Track(vcs_pos_longitudinal, vcs_pos_lateral, min_track_range);

   /** \result
   */
   DOUBLES_EQUAL(min_track_range, range, 0.01);
}

/**
*\purpose  Test checks if range is correctly calculated based on declared inputs.
*\req    NA
**/
TEST(Calc_Range_to_Track, Check_if_propper_range_is_calculate)
{
   /** \precond
   */
   float32_t vcs_pos_longitudinal = 1.0F;
   float32_t vcs_pos_lateral = 1.0F;
   float32_t min_track_range = 1.0F;
   float32_t exp_range = 1.41F;
   float32_t range;

   /** \action
      Call Calc_Range_to_Track
   */
   range = Calc_Range_to_Track(vcs_pos_longitudinal, vcs_pos_lateral, min_track_range);

   /** \result
   */
   DOUBLES_EQUAL(exp_range, range, 0.01);
}
/** @}*/


/** \defgroup  Is_Valid_Trk_Near_Host
 *  @{
 */

/** \brief
 *  Test group for Is_Valid_Trk_Near_Host()
 **/
TEST_GROUP(Is_Valid_Trk_Near_Host)
{
   F360_Host_T host;
   F360_Calibrations_T calib = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];

   TEST_SETUP()
   {
      calib.k_track_downselect_max_vcs_x_range_to_preserve = 15.0F;
      calib.k_track_downselect_max_vcs_y_range_to_preserve = 9.0F;
   }
};

/**
*\purpose  Test checks if object track is valid when it has border parameters equal to calibrations.
*\req    NA
**/
TEST(Is_Valid_Trk_Near_Host, Check_if_propper_range_is_calculate)
{
   /** \precond
   */
   object_tracks[0].vcs_position.x = 13.499F;
   object_tracks[0].vcs_position.y = 8.999F;
   host.dist_rear_axle_to_vcs_m = 3.0F;

   bool validation_result = false;

   /** \action
      Call Is_Valid_Trk_Near_Host
   */
   validation_result = Is_Valid_Trk_Near_Host(object_tracks[0], host, calib);

   /** \result
   */
   CHECK_TRUE(validation_result);
}
/** @}*/


/** \defgroup  Is_Unreliable_Low_Conf_Moveable_Track
 *  @{
 */

/** \brief
 *  Test group for Is_Unreliable_Low_Conf_Moveable_Track()
 **/
TEST_GROUP(Is_Unreliable_Low_Conf_Moveable_Track)
{
   F360_Host_T host;
   F360_Calibrations_T calib = {};
   F360_Object_Track_T object_tracks[5];
   Static_Env_Poly_T stat_env_poly[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   BoundingBox overall_confidence_exclusion_box{};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      host.dist_rear_axle_to_vcs_m = 4.0F;
      overall_confidence_exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(stat_env_poly, calib, host.dist_rear_axle_to_vcs_m * 0.5F);

      // Initialize Objects as moveable
      for (int32_t i = 0; i < 5; i++)
      {
         object_tracks[i].f_moveable = true;
      }

   }
};

/**
*\purpose  Check if a low confidence track in the zone of interest is flagged unreliable
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable)
{
   /** \precond
   
   */

   host.vcs_speed = 10.0F;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_LOW;
   

   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_LOW;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_LOW;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_LOW;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_LOW;

   host.speed = 10.0F;

   for (int32_t i = 0; i < 5; i++)
   {
      /** \action
         Call Is_Unreliable_Low_Conf_Moveable_Track for each track
      */
      const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

      /** \result
      */
      CHECK_TRUE(result);
   }
}

/**
*\purpose  Check if a track is not flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence,
   - has expected negative heading and negative heading difference for CTA scenario
   AND
   - host speed is below CTA speed threshold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_low_host_speed_and_expected_neg_heading_neg_diff_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading = Angle{F360_DEG2RAD(-89.0F)};

   host.vcs_speed = 0.19F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;


   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
      /** \action
         Call Is_Unreliable_Low_Conf_Moveable_Track for each track
      */
      const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

      /** \result
      */
      CHECK_FALSE(result);
   }
}

/**
*\purpose  Check if a track is not flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence,
   - has expected negative heading and positive heading difference for CTA scenario
   AND
   - host speed is below CTA speed threshold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_low_host_speed_and_expected_neg_heading_pos_diff_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(-91.0F) };

   host.vcs_speed = 0.19F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;

   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
       /** \action
          Call Is_Unreliable_Low_Conf_Moveable_Track for each track
       */
       const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

       /** \result
       */
       CHECK_FALSE(result);
   }
}

/**
*\purpose  Check if a track is not flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence,
   - has expected positive heading and negative heading difference for CTA scenario
   AND
   - host speed is below CTA speed threshold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_low_host_speed_and_expected_pos_heading_neg_diff_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(89.0F) };

   host.vcs_speed = 0.19F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;

   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
       /** \action
          Call Is_Unreliable_Low_Conf_Moveable_Track for each track
       */
       const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

       /** \result
       */
       CHECK_FALSE(result);
   }
}

/**
*\purpose  Check if a track is not flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence,
   - has expected positive heading and positive heading difference for CTA scenario
   AND
   - host speed is below CTA speed threshold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_low_host_speed_and_expected_pos_heading_pos_diff_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(91.0F) };

   host.vcs_speed = 0.19F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;

   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
       /** \action
          Call Is_Unreliable_Low_Conf_Moveable_Track for each track
       */
       const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

       /** \result
       */
       CHECK_FALSE(result);
   }
}

/**
*\purpose  Check if a track is flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence
   - has expected negative heading for CTA scenario
   AND
   - host speed is above CTA speed threhsold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_host_is_moving_and_expected_neg_heading_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(-90.0F) };

   host.vcs_speed = 2.0F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;


   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
      /** \action
         Call Is_Unreliable_Low_Conf_Moveable_Track for each track
      */
      const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

      /** \result
      */
      CHECK_TRUE(result);
   }
}

/**
*\purpose  Check if a track is flagged unreliable when:
   A track:
   - is in the zone of interest,
   - has medium confidence
   - has expected positive heading for CTA scenario
   AND
   - host speed is above CTA speed threhsold
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_host_is_moving_and_expected_pos_heading_in_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(90.0F) };

   host.vcs_speed = 2.0F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;


   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
       /** \action
          Call Is_Unreliable_Low_Conf_Moveable_Track for each track
       */
       const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

       /** \result
       */
       CHECK_TRUE(result);
   }
}

/**
*\purpose  Check if a track is not flagged unreliable when:
  A track
  - is in the zone of interest
  - has medium confidence
  - has heading below threshold used in CTA scenarios
   AND 
  - host speed is below CTA speed threshold 
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_unreliable_host_not_moving_and_obj_heading_outside_CTA_scenario)
{
   /** \precond

   */
   const Angle initial_heading{ F360_DEG2RAD(45.0F) };

   host.vcs_speed = 0.0F;
   object_tracks[0].vcs_heading = initial_heading;
   object_tracks[1].vcs_heading = initial_heading;
   object_tracks[2].vcs_heading = initial_heading;
   object_tracks[3].vcs_heading = initial_heading;
   object_tracks[4].vcs_heading = initial_heading;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_MED;


   //Track at front right
   object_tracks[1].vcs_position.x = 10.0F;
   object_tracks[1].vcs_position.y = 10.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_MED;

   // Track at front left
   object_tracks[2].vcs_position.x = 10.0F;
   object_tracks[2].vcs_position.y = -10.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_MED;

   // Track at right side
   object_tracks[3].vcs_position.x = 0.0F;
   object_tracks[3].vcs_position.y = 10.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_MED;

   // Track at left side
   object_tracks[4].vcs_position.x = 0.0F;
   object_tracks[4].vcs_position.y = -10.0F;
   object_tracks[4].vcs_velocity.longitudinal = 0.0F;
   object_tracks[4].conf_overall = CONF3_MED;

   for (int32_t i = 0; i < 5; i++)
   {
      /** \action
         Call Is_Unreliable_Low_Conf_Moveable_Track for each track
      */
      const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

      /** \result
      */
      CHECK_FALSE(result);
   }
}


/**
*\purpose  Check that a track outside the zone of interest or with high confidence is not flagged unreliable
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_track_is_not_unreliable)
{
   /** \precond
   */
   // Track straight in rear
   object_tracks[0].vcs_position.x = -10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_LOW;

   //Track at rear right
   object_tracks[1].vcs_position.x = -10.0F;
   object_tracks[1].vcs_position.y = 5.0F;
   object_tracks[1].vcs_velocity.longitudinal = 0.0F;
   object_tracks[1].conf_overall = CONF3_LOW;

   // Track at rear left
   object_tracks[2].vcs_position.x = -10.0F;
   object_tracks[2].vcs_position.y = -5.0F;
   object_tracks[2].vcs_velocity.longitudinal = 0.0F;
   object_tracks[2].conf_overall = CONF3_LOW;

   // Track with high confidence
   object_tracks[3].vcs_position.x = 10.0F;
   object_tracks[3].vcs_position.y = 0.0F;
   object_tracks[3].vcs_velocity.longitudinal = 0.0F;
   object_tracks[3].conf_overall = CONF3_HIGH;

   host.speed = 10.0F;

   for (int32_t i = 0; i < 4; i++)
   {
      /** \action
         Call Is_Unreliable_Low_Conf_Moveable_Track for each track
      */
      const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[i], host, calib, overall_confidence_exclusion_box);

      /** \result
      */
      CHECK_FALSE(result);
   }
}

/**
*\purpose  Ensure the function does not break down when TTX is infinite
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_infinite_TTX_behavior)
{
   /** \precond
   */
   // Track straight in rear
   object_tracks[0].vcs_position.x = -10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_LOW;

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   */
   CHECK_FALSE(result);
}

/**
*\purpose  Don't hide objects in the exclusion zone
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Exclusion_Zone_Check)
{
   /** \precond
   */
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = -1.0F;
   object_tracks[0].conf_overall = CONF3_LOW;

   object_tracks[1].vcs_position.x = -5.0F;
   object_tracks[1].vcs_position.y = 0.0F;
   object_tracks[1].vcs_velocity.longitudinal = 1.0F;
   object_tracks[1].conf_overall = CONF3_LOW;

   object_tracks[2].vcs_position.x = 1.0F;
   object_tracks[2].vcs_position.y = 4.0F;
   object_tracks[2].vcs_velocity.longitudinal = -1.0F;
   object_tracks[2].conf_overall = CONF3_LOW;

   object_tracks[3].vcs_position.x = 1.0F;
   object_tracks[3].vcs_position.y = -4.0F;
   object_tracks[3].vcs_velocity.longitudinal = -1.0F;
   object_tracks[3].conf_overall = CONF3_LOW;

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result1 = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);
   const bool result2 = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[1], host, calib, overall_confidence_exclusion_box);
   const bool result3 = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[2], host, calib, overall_confidence_exclusion_box);
   const bool result4 = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[3], host, calib, overall_confidence_exclusion_box);

   /** \result
   */
   CHECK_FALSE(result1);
   CHECK_FALSE(result2);
   CHECK_FALSE(result3);
   CHECK_FALSE(result4);
}

/**
*\purpose  Check if object is unreliable due to being CTCA and having high heading
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Exclude_Exclusion_Zone_Check_When_Object_Is_CTCA_And_High_Heading)
{
   /** \precond
   * CTCA object with high (higher than 30 deg) heading, low overall confidence
   */
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = -1.0F;
   object_tracks[0].conf_overall = CONF3_LOW;
   object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object_tracks[0].vcs_heading = Angle{ F360_DEG2RAD(31.0F) };

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   * Object is unreliable
   */
   CHECK_TRUE(result);
}

/**
*\purpose  Check if object is reliable due to being CCA and having high heading
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Exclusion_Zone_Check_When_Object_Is_CCA_And_High_Heading)
{
   /** \precond
   * CCA object with high (higher than 30 deg) heading, low overall confidence
   */
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = -1.0F;
   object_tracks[0].conf_overall = CONF3_LOW;
   object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   object_tracks[0].vcs_heading = Angle{ F360_DEG2RAD(31.0F) };

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   * Object is reliable
   */
   CHECK_FALSE(result);
}

/**
*\purpose  Check if object is reliable due to being CCA and having low heading
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Exclusion_Zone_Check_When_Object_Is_CCA_And_Low_Heading)
{
   /** \precond
   * CCA object with low (less than 30 deg) heading, low overall confidence
   */
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = -1.0F;
   object_tracks[0].conf_overall = CONF3_LOW;
   object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   object_tracks[0].vcs_heading = Angle{ F360_DEG2RAD(29.0F) };

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   * Object is reliable
   */
   CHECK_FALSE(result);
}

/**
*\purpose  Check if object is reliable due to being CTCA and having low heading
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Exclude_Exclusion_Zone_Check_When_Object_Is_CTCA_And_Low_Heading)
{
   /** \precond
   * CTCA object with low (less than 30 deg) heading, low overall confidence
   */
   object_tracks[0].vcs_position.x = 5.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = -1.0F;
   object_tracks[0].conf_overall = CONF3_LOW;
   object_tracks[0].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object_tracks[0].vcs_heading = Angle{ F360_DEG2RAD(29.0F) };

   host.speed = 0.0F;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   * Object is reliable
   */
   CHECK_FALSE(result);
}

/**
*\purpose  Ensure the function checks if the track is unreliable when f_moveable = false 
*\req    NA
**/
TEST(Is_Unreliable_Low_Conf_Moveable_Track, Check_if_unreliable_when_non_moveable)
{
   /** \precond
   */
   // Set to non_moveable
   object_tracks[0].f_moveable = false;

   host.vcs_speed = 10.0F;

   // Track straight in front
   object_tracks[0].vcs_position.x = 10.0F;
   object_tracks[0].vcs_position.y = 0.0F;
   object_tracks[0].vcs_velocity.longitudinal = 0.0F;
   object_tracks[0].conf_overall = CONF3_LOW;

   /** \action
      Call Is_Unreliable_Low_Conf_Moveable_Track for each track
   */
   const bool result = Is_Unreliable_Low_Conf_Moveable_Track(object_tracks[0], host, calib, overall_confidence_exclusion_box);

   /** \result
   */
   CHECK_FALSE(result);
}
/** @}*/

/** \defgroup  check_calibration_values_used_in_downselection
 *  @{
 */

 /** \brief
  * Test group verifies if calibration parameters used in downselection have correct values
  */
TEST_GROUP(check_calibration_values_used_in_downselection)
{
   F360_Calibrations_T calib = {};
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};


/**
*\purpose  Check if k_low_conf_max_allowed_host_speed_in_cta_scenarios has correct value
*\req    NA
**/
TEST(check_calibration_values_used_in_downselection, check_k_low_conf_max_allowed_host_speed_in_cta_scenarios)
{
   DOUBLES_EQUAL(0.2F, calib.k_low_conf_max_allowed_host_speed_in_cta_scenarios, F360_EPSILON);
}

/**
*\purpose  Check if k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios has correct value
*\req    NA
**/
TEST(check_calibration_values_used_in_downselection, check_k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios)
{
   DOUBLES_EQUAL(1.57079633F, calib.k_low_conf_expected_abs_object_heading_vcs_in_cta_scenarios, F360_EPSILON);
}

/**
*\purpose  Check if k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios has correct value
*\req    NA
**/
TEST(check_calibration_values_used_in_downselection, check_k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios)
{
   DOUBLES_EQUAL(0.26179939F, calib.k_low_conf_max_allowed_abs_heading_difference_in_cta_scenarios, F360_EPSILON);
}
/** @}*/

/** \defgroup  define_overall_confidence_lateral_exclusion_box
 *  @{
 */

 /** \brief
  * Test group of Define_Overall_Confidence_Exclusion_Box_Around_Host . Tests verify whether 
  * exclusion box lateral limits are properly determined.
  */
TEST_GROUP(define_overall_confidence_lateral_exclusion_box)
{
   Static_Env_Poly_T static_env_polys[F360_NUM_OF_STATIC_ENV_POLYS];
   const float32_t max_exclusion_box_lat_dist = 8.0F;
   F360_Calibrations_T calib = {};

   /** \setup
    * Create two valid SEPs that spans from -100m to 100m longitudinally. 
    * Set lateral position of the first polynomials to minus half the distance of max exclusion box lateral distance. 
    * Set lateral position of the second polynomials to half the distance of max exclusion box lateral distance. 
    * Call Set function to assign the SEPs to the static environment class.
    * 
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      // Left side
      static_env_polys[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_polys[0].p2 = 0.0F;
      static_env_polys[0].p1 = 0.0F;
      static_env_polys[0].p0 = -0.5F * max_exclusion_box_lat_dist;
      static_env_polys[0].lower_limit = -100.0F;
      static_env_polys[0].upper_limit = 100.0F;

      // Right side
      static_env_polys[1].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_polys[1].p2 = 0.0F;
      static_env_polys[1].p1 = 0.0F;
      static_env_polys[1].p0 = 0.5F * max_exclusion_box_lat_dist;
      static_env_polys[1].lower_limit = -100.0F;
      static_env_polys[1].upper_limit = 100.0F;
   }
};

/** \purpose
 * Purpose of this test is to verify that left limit is properly determined when there is no valid SEP on the left side of host.
 * \req
 * NA.
 */
TEST(define_overall_confidence_lateral_exclusion_box, Define_Overall_Confidence_Lateral_Exclusion_Box__Left_SEP_Not_Present)
{
   /** \precond
    * Set status of left SEP to invalid.
    */
   static_env_polys[0].status = F360_STATIC_ENV_POLY_STATUS_INVALID;

   /** \action
    * Call Define_Overall_Confidence_Exclusion_Box_Around_Host 
    */
   const BoundingBox exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(static_env_polys, calib, max_exclusion_box_lat_dist);

   Point exclusion_box_center = exclusion_box.Get_Center();
   float32_t left_limit = exclusion_box_center.y - 0.5*exclusion_box.Get_Width();

   /** \result
    * Check whether returned left limit is equal to -1.0F * max_exclusion_box_lat_dist
    */
   DOUBLES_EQUAL(-1.0F * max_exclusion_box_lat_dist, left_limit, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify that left limit is properly determined when there is a valid SEP on the left side of host.
 * \req
 * NA.
 */
TEST(define_overall_confidence_lateral_exclusion_box, Define_Overall_Confidence_Lateral_Exclusion_Box__Left_SEP_Present)
{
   /** \precond
    * In test group the following has been set up:
    * - Max lateral exclusion box distance has been set to 8. 
    * - A SEP has been set up on the left side of host with a lateral position that is half the max lateral exclusion box distance.
    * - The lower and upper longitudinal limit of the SEP has been set to -100m and 100m.
    */

   /** \action
    * Call tested function
    */
   const BoundingBox exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(static_env_polys, calib, max_exclusion_box_lat_dist);
   Point exclusion_box_center = exclusion_box.Get_Center();
   float32_t left_limit = exclusion_box_center.y - 0.5*exclusion_box.Get_Width();
   /** \result
    * Check whether returned left limit is equal to left SEP "p0" coefficient
    */
   DOUBLES_EQUAL(static_env_polys[0].p0, left_limit, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify that right limit is properly determined when there is no valid SEP on the right side of host.
 * \req
 * NA.
 */
TEST(define_overall_confidence_lateral_exclusion_box, Define_Overall_Confidence_Lateral_Exclusion_Box__Right_LSC_Not_Present)
{
   /** \precond
    * In test group the following has been set up:
    * - Max lateral exclusion box distance has been set to 8.
    * - A SEP has been set up on the right side of host with a lateral position that is half the max lateral exclusion box distance.
    * - The lower and upper longitudinal limit of the SEP has been set to -100m and 100m.
    * Set the status of right SEP to invalid
    */
   static_env_polys[1].status = F360_STATIC_ENV_POLY_STATUS_INVALID;

   /** \action
    * Call Define_Overall_Confidence_Exclusion_Box_Around_Host 
    */
   const BoundingBox  exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(static_env_polys, calib, max_exclusion_box_lat_dist);

   Point exclusion_box_center = exclusion_box.Get_Center();
   float32_t right_limit = exclusion_box_center.y + 0.5*exclusion_box.Get_Width();
   /** \result
    * Check that returned right limit is equal to max_exclusion_box_lat_dist
    */
   DOUBLES_EQUAL(max_exclusion_box_lat_dist, right_limit, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify that right limit is properly determined when there is a valid SEP on the right side of host.
 * \req
 * NA.
 */
TEST(define_overall_confidence_lateral_exclusion_box, Define_Overall_Confidence_Lateral_Exclusion_Box__Right_LSC_Present)
{
   /** \precond
    * In test group the following has been set up:
    * - Max lateral exclusion box distance has been set to 8.
    * - A SEP has been set up on the right side of host with a lateral position that is half the max lateral exclusion box distance.
    * - The lower and upper longitudinal limit of the SEP has been set to -100m and 100m.
    */

    /** \action
     * Call Define_Overall_Confidence_Exclusion_Box_Around_Host 
     */
   const BoundingBox exclusion_box = Define_Overall_Confidence_Exclusion_Box_Around_Host(static_env_polys, calib, max_exclusion_box_lat_dist);

   Point exclusion_box_center = exclusion_box.Get_Center();
   float32_t right_limit = exclusion_box_center.y + 0.5*exclusion_box.Get_Width();

   /** \result
    * Check that returned right limit is equal to right SEP p0 coefficient
    */
   DOUBLES_EQUAL(static_env_polys[1].p0, right_limit, F360_EPSILON);
}
/** @}*/

/** \defgroup  Is_In_Area_Of_Interest
 *  @{
 */

/** \brief
 *  Test group for Is_In_Area_Of_Interest()
 **/
TEST_GROUP(Is_In_Area_Of_Interest)
{
    const float32_t zone_longpos_shift = 2.0F;
};

/** \brief
 *  Test checks if object is inside the area of interest on the right hand side.
 **/
TEST(Is_In_Area_Of_Interest, check_if_inside_area_right_side)
{
    /** \precond
    */
    const Point track_pos{ 5.0F, 5.0F };
    /** \action
       Call Is_In_Area_Of_Interest
    */
    const bool f_inside_area_of_interest = Is_Outside_Triangular_Zone_Behind_Host(track_pos, zone_longpos_shift);

    /** \result
       check if flag is true
    */
    CHECK_TRUE(f_inside_area_of_interest);
}

/** \brief
 *  Test checks if object is outside the area of interest on the right hand side.
 **/
TEST(Is_In_Area_Of_Interest, check_if_outside_area_right_side)
{
    /** \precond
    */
    const Point track_pos{ -5.0F, 5.0F };

    /** \action
        Call Is_In_Area_Of_Interest
    */
    const bool f_inside_area_of_interest = Is_Outside_Triangular_Zone_Behind_Host(track_pos, zone_longpos_shift);

    /** \result
        check if flag is false
    */
    CHECK_FALSE(f_inside_area_of_interest);
}

/** \brief
 *  Test checks if object is inside the area of interest on the left hand side.
 **/
TEST(Is_In_Area_Of_Interest, check_if_inside_area_left_side)
{
    /** \precond
    */
    const Point track_pos{ 5.0F, -5.0F };

    /** \action
       Call Is_In_Area_Of_Interest
    */
    const bool f_inside_area_of_interest = Is_Outside_Triangular_Zone_Behind_Host(track_pos, zone_longpos_shift);

    /** \result
       check if flag is true
    */
    CHECK_TRUE(f_inside_area_of_interest);
}

/** \brief
 *  Test checks if object is outside the area of interest on the left hand side.
 **/
TEST(Is_In_Area_Of_Interest, check_if_outside_area_left_side)
{
    /** \precond
    */
    const Point track_pos{ -5.0F, -5.0F };

    /** \action
       Call Is_In_Area_Of_Interest
    */
    const bool f_inside_area_of_interest = Is_Outside_Triangular_Zone_Behind_Host(track_pos, zone_longpos_shift);

    /** \result
       check if flag is false
    */
    CHECK_FALSE(f_inside_area_of_interest);
}
/** @}*/

/** \defgroup  Is_Moving_In_Specified_Direction
 *  @{
 */

/** \brief
 *  Test group for Is_Moving_In_Specified_Direction()
 **/
TEST_GROUP(Is_Moving_In_Specified_Direction)
{
    F360_Calibrations_T calib = {};
    F360_Object_Track_T obj_trk;
    float32_t tolerance = 0.1F;
    TEST_SETUP()
    {
       Initialize_Tracker_Calibrations(calib);
       obj_trk.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
       obj_trk.vcs_heading =  Angle{ std::abs(calib.k_low_conf_unreliability_min_heading) + tolerance };
    }
};

/** \brief
 *  Test checks that object is relevant for exclusion zone when it is a CCTA track and its heading is above threshold.
 **/
TEST(Is_Moving_In_Specified_Direction, check_CCTA_above_threshold_relevant)
{
    /** \precond
    */

    /** \action
       Call Is_Moving_In_Specified_Direction
    */
    const bool f_moving_in_specified_direction = Is_Heading_Different_Than_Host(obj_trk, calib);

    /** \result
       Check if flag is true
    */
    CHECK_TRUE(f_moving_in_specified_direction);
}

/** \brief
 *  Test checks that object is irrelevant for exclusion zone when it is a CCTA track and its heading is below threshold.
 **/
TEST(Is_Moving_In_Specified_Direction, check_CCTA_below_threshold_irrelevant)
{
    /** \precond
    */
    obj_trk.vcs_heading = Angle{ std::abs(calib.k_low_conf_unreliability_min_heading) - tolerance };

    /** \action
       Call Is_Moving_In_Specified_Direction
    */
    const bool f_moving_in_specified_direction = Is_Heading_Different_Than_Host(obj_trk, calib);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_moving_in_specified_direction);
}

/** \brief
 *  Test checks that object is irrelevant for exclusion zone when it is a CCA track and its heading is above threshold.
 **/
TEST(Is_Moving_In_Specified_Direction, check_CCA_above_threshold_irrelevant)
{
    /** \precond
    */
    obj_trk.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

    /** \action
       Call Is_Moving_In_Specified_Direction
    */
    const bool f_moving_in_specified_direction = Is_Heading_Different_Than_Host(obj_trk, calib);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_moving_in_specified_direction);
}

/** \brief
 *  Test checks that object is irrelevant for exclusion zone when it is a CCA track and its heading is below threshold.
 **/
TEST(Is_Moving_In_Specified_Direction, check_CCA_below_threshold_irrelevant)
{
    /** \precond
    */
    obj_trk.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
    obj_trk.vcs_heading = Angle{ std::abs(calib.k_low_conf_unreliability_min_heading) - tolerance };
    obj_trk.speed = calib.fast_moving_thresh + tolerance;

    /** \action
       Call Is_Moving_In_Specified_Direction
    */
    const bool f_moving_in_specified_direction = Is_Heading_Different_Than_Host(obj_trk, calib);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_moving_in_specified_direction);
}
/** @}*/

/** \defgroup  Is_Outside_Exclusion_Box
 *  @{
 */

/** \brief
 *  Test group for Is_Outside_Exclusion_Box()
 **/
TEST_GROUP(Is_Outside_Exclusion_Box)
{
    //center(x,    y)     len   wid   ori
    const BoundingBox exclusion_box = BoundingBox(Point(2.0F, 2.0F), 6.0F, 4.0F, Angle{0.0F});
};

/** \brief
 *  Test checks if track is outside of a specified exclusion box.
 **/
TEST(Is_Outside_Exclusion_Box, check_if_outside_exclusion_box)
{
    /** \precond
    */
    Point track_pos = { 3.0F, 5.0F };

    /** \action
       Call Is_Outside_Exclusion_Box
    */
    const bool f_outside_exclusion_box = Is_Outside_Exclusion_Box(track_pos, exclusion_box);

    /** \result
       Check if flag is true
    */
    CHECK_TRUE(f_outside_exclusion_box);
}

/** \brief
 *  Test checks if track is inside of a specified exclusion box.
 **/
TEST(Is_Outside_Exclusion_Box, check_if_inside_exclusion_box)
{
    /** \precond
    */
    Point track_pos = { 3.0F, 3.0F };

    /** \action
       Call Is_Outside_Exclusion_Box
    */
    const bool f_outside_exclusion_box = Is_Outside_Exclusion_Box(track_pos, exclusion_box);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_outside_exclusion_box);
}
/** @}*/

/** \defgroup  Has_Low_TTC
 *  @{
 */

/** \brief
 *  Test group for Has_Low_TTC()
 **/
TEST_GROUP(Has_Low_TTC)
{
    F360_Calibrations_T calib = {};
    const float32_t test_pass_th = 1e-5F;
    float32_t host_speed = 1.0F;
    TEST_SETUP()
    {
        Initialize_Tracker_Calibrations(calib);
    }
};

/** \brief
 *  Test checks if track TTC is below threshold.
 **/
TEST(Has_Low_TTC, check_if_TTC_below_threshold)
{
    /** \precond
    */
    F360_VCS_Velocity_T track_velocity;
    track_velocity.longitudinal = 0.0F;
    Point track_pos = { calib.k_low_conf_unreliability_max_ttc - test_pass_th, 0.0F };

    /** \action
       Call Has_Low_TTC
    */
    const bool f_low_ttc = Has_Low_TTC(host_speed, track_velocity, track_pos, calib.k_low_conf_unreliability_max_ttc);

    /** \result
       Check if flag is true
    */
    CHECK_TRUE(f_low_ttc);
}

/** \brief
 *  Test checks if track TTC is above threshold.
 **/
TEST(Has_Low_TTC, check_if_TTC_above_threshold)
{
    /** \precond
    */
    F360_VCS_Velocity_T track_velocity;
    track_velocity.longitudinal = 0.0F;
    Point track_pos = { calib.k_low_conf_unreliability_max_ttc + test_pass_th, 0.0F };

    /** \action
       Call Has_Low_TTC
    */
    const bool f_low_ttc = Has_Low_TTC(host_speed, track_velocity, track_pos, calib.k_low_conf_unreliability_max_ttc);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_low_ttc);
}

/** \brief
 *  Test checks if track TTC is above threshold if velocity difference is zero.
 **/
TEST(Has_Low_TTC, check_if_TTC_zero)
{
    /** \precond
    */
    F360_VCS_Velocity_T track_velocity;
    track_velocity.longitudinal = 1.0F;
    Point track_pos = { 10.0F, 0.0F };

    /** \action
       Call Has_Low_TTC
    */
    const bool f_low_ttc = Has_Low_TTC(host_speed, track_velocity, track_pos, calib.k_low_conf_unreliability_max_ttc);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_low_ttc);
}

/** \brief
 *  Test checks if track low_TTC flag is false if TTC is at threshold.
 **/
TEST(Has_Low_TTC, check_lowTTC_at_threshold)
{
    /** \precond
    */
    F360_VCS_Velocity_T track_velocity;
    track_velocity.longitudinal = 0.0F;
    Point track_pos = { calib.k_low_conf_unreliability_max_ttc, 0.0F };

    /** \action
       Call Has_Low_TTC
    */
    const bool f_low_ttc = Has_Low_TTC(host_speed, track_velocity, track_pos, calib.k_low_conf_unreliability_max_ttc);

    /** \result
       Check if flag is false
    */
    CHECK_FALSE(f_low_ttc);
}
/** @}*/

/** \defgroup  Cond_LP_Filter_Reduced_Det_Num
 *  @{
 */

 /** \brief
   *  Test group of Cond_LP_Filter_Reduced_Det_Num
   */
TEST_GROUP(Cond_LP_Filter_Reduced_Det_Num)
{
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   F360_Tracker_Info_T tracker_info = {};
   F360_Calibrations_T calib;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};

/** \purpose  
 * Purpose of this test is to verify whether number of filtered dets after LP filter is not changed when number of filtered dets is
 * larger than number of reduced detections and the number of reduced detections is above 0
 *\req    NA
 */
TEST(Cond_LP_Filter_Reduced_Det_Num, Cond_LP_Filter_Reduced_Det_Num__check_whether_number_of_filtered_dets_is_unchanged)
{
   /** \precond
    * Define needed data
    * - Set object status as F360_OBJECT_STATUS_NEW
    * - Set number of filtered detections to 20
    * - Set number of reduced detections to 1
    */
   tracker_info.num_active_objs = 1;
   object_tracks[0].status = F360_OBJECT_STATUS_NEW;
   object_tracks[0].filtered_dets = 20.0F;
   object_tracks[0].num_rr_inlier_dets = 1;

   /** \action
    * Call function Cond_LP_Filter_Reduced_Det_Num
    */
   Cond_LP_Filter_Reduced_Det_Num(tracker_info, calib.k_tv_dets_exp_filter_const, object_tracks);

   /** \result
    * Number of filtered detections should not change
    */
   DOUBLES_EQUAL(object_tracks[0].filtered_dets, 20.0F, F360_EPSILON);
}

/** \purpose  
 * Purpose of this test is to verify whether number of filtered dets after LP filter is decreasing when number of filtered dets is
 * not larger than number of reduced detections or the number of reduced detections equals 0
 *\req    NA
 */
TEST(Cond_LP_Filter_Reduced_Det_Num, Cond_LP_Filter_Reduced_Det_Num__check_whether_number_of_filtered_dets_is_decreasing)
{
   /** \precond
    * Define needed data
    * - Set object status as F360_OBJECT_STATUS_NEW
    * - Set number of filtered detections to 2
    * - Set number of reduced detections to 0
    */
   tracker_info.num_active_objs = 1;
   object_tracks[0].status = F360_OBJECT_STATUS_NEW;
   object_tracks[0].filtered_dets = 2.0F;
   object_tracks[0].num_rr_inlier_dets = 0;

   /** \action
    * Call function Cond_LP_Filter_Reduced_Det_Num
    */
   Cond_LP_Filter_Reduced_Det_Num(tracker_info, calib.k_tv_dets_exp_filter_const, object_tracks);

   /** \result
    * Number of filtered detections should decay
    */
   DOUBLES_EQUAL(object_tracks[0].filtered_dets, 1.9F, F360_EPSILON);
}

/**
 *\purpose  Purpose of this test is to verify whether newly updated tracks have their number of filtered dets initialized.
 *\req    NA
 */
TEST(Cond_LP_Filter_Reduced_Det_Num, Cond_LP_Filter_Reduced_Det_Num__check_whether_new_updated_tracks_have_their_filtered_dets_initialized)
{
   /** \precond
    * Define needed data
    * - Set object status as F360_OBJECT_STATUS_NEW_UPDATED
    * - Set number of filtered detections to 0
    * - Set number of reduced detections to 1
    */
   tracker_info.num_active_objs = 1;
   object_tracks[0].status = F360_OBJECT_STATUS_NEW_UPDATED;
   object_tracks[0].filtered_dets = 0.0F;
   object_tracks[0].num_rr_inlier_dets = 1;

   /** \action
    * Call function Cond_LP_Filter_Reduced_Det_Num
    */
   Cond_LP_Filter_Reduced_Det_Num(tracker_info, calib.k_tv_dets_exp_filter_const, object_tracks);

   /** \result
   * Number of fitlered dets should be greater than 0
   **/
   DOUBLES_EQUAL(object_tracks[0].filtered_dets, 0.525F, F360_EPSILON);
}

/** @}*/
