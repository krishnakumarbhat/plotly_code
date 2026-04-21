/** \file
   File with unit tests for Mark_Det_To_Use_For_Dim_Update function
*/

#include "f360_mark_det_to_use_for_dim_update.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

/** \defgroup  f360_mark_det_to_use_for_dim_update
 *  @{
 */
using namespace f360_variant_A;
/** \brief
 *  Group of tests covers Mark_Det_To_Use_For_Dim_Update function responsible to decide if detection should be used for object's dimensions extension
 */

TEST_GROUP(f360_mark_det_to_use_for_dim_update)
{
   /** \setup
   * Setting up one object and set of detections
   */
   F360_Object_Track_T object_track;
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Calibrations_T calib;

   TEST_SETUP()
   {
      // Initialize calibrations
      Initialize_Tracker_Calibrations(calib);

      // Reset object and detections states after each test.
      object_track = {};
      for (int32_t det_idx = 0; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      {
         detection_props[det_idx] = {};
      }
   }
};

/**
*\purpose  Check if associated detection to not moving and not movable object, has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_stationary)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to false,
    * object's f_moveable set to false
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = false;
   object_track.f_moveable = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}


/**
*\purpose  Deafault scenario for correct detection: associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, 
moving status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_close_to_other_moving_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to true,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}


/**
*\purpose  Possible wheelspin scenario: Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
moving status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_close_to_other_moving_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
stationary status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_close_to_other_stationary_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to true,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
stationary status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_close_to_other_stationary_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
moving status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_no_close_to_other_moving_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to true,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
moving status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_no_close_to_other_moving_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to false,
    * add another detection, farther away than 3m, in order to keep any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO.
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = false;
   detection_props[0].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[1].vcs_position.x = 5.0F;
   detection_props[1].vcs_position.y = 0.0F;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[1].f_rr_inlier = false;
   detection_props[1].dist_to_closest_assoc_det_sq = 100.0F;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
stationary status, selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_no_close_to_other_stationary_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to true,
    * add another detection, farther away than 3m, in order to keep any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO.
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = true;
   detection_props[0].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[1].vcs_position.x = 5.0F;
   detection_props[1].vcs_position.y = 0.0F;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[1].f_rr_inlier = false;
   detection_props[1].dist_to_closest_assoc_det_sq = 100.0F;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to moving object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
stationary status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_moving_detection_no_close_to_other_stationary_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moving set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
moving status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_close_to_other_moving_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to true,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}


/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
moving status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_close_to_other_moving_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
stationary status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_close_to_other_stationary_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to true,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
stationary status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_close_to_other_stationary_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO due to large distance between associated detections,
moving status, selected by trk. Checking if this detection has f_use_in_dimension_update set to true
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_no_close_to_other_moving_selected_by_xtrk)
{
   /** \precond
    * object_track associated with two detections which are far apart,
    * object's f_moveable set to true,
    * primary detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * primary detection's f_rr_inlier set to true,
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moveable = true;

   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;
   detection_props[0].dist_to_closest_assoc_det_sq = INFTY;

   detection_props[1].vcs_position.x = 100.0F;
   detection_props[1].vcs_position.y = 100.0F;
   detection_props[1].dist_to_closest_assoc_det_sq = INFTY;
   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */
   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to true
    */
   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
moving status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_no_close_to_other_moving_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * detection's f_rr_inlier set to false,
    * Add a second detection to ensure that any_other_assoc_det_close is kept as initialized after
    * Mark_Detections_With_Neighbors is called.
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = false;
   detection_props[0].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[1].vcs_position.x = 5.0F;
   detection_props[1].vcs_position.y = 0.0F;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[1].f_rr_inlier = false;
   detection_props[1].dist_to_closest_assoc_det_sq = 100.0F;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
stationary status, selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_no_close_to_other_stationary_selected_by_xtrk)
{
   /** \precond
    * object_track associated with two detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to true,
    * add another detection to ensure that any_other_assoc_det_close is kept as initialized after
    * Mark_Detections_With_Neighbors is called.
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = true;
   detection_props[0].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[1].vcs_position.x = 5.0F;
   detection_props[1].vcs_position.y = 0.0F;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[1].f_rr_inlier = false;
   detection_props[1].dist_to_closest_assoc_det_sq = 100.0F;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Associated to movable object, with F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
stationary status, NOT selected by trk. Checking if this detection has f_use_in_dimension_update set to false
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_object_movable_detection_no_close_to_other_stationary_not_selected_by_xtrk)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * detection's f_rr_inlier set to false,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and single associated detection that met certain conditions
    */


   /** \result
    * Associated detection's flag f_use_in_dimension_update should be set to false
    */
   object_track.ndets = 1;
   object_track.detids[0] = 1;

   object_track.f_moveable = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
}

/**
*\purpose  Deafault scenario for multiple correct detection, with parameters that should make f_use_in_dimension_update flag set to true.
Checking if this detections have proper f_use_in_dimension_update flag value
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_multiple_correct_detections)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * all detections' f_rr_inlier set to true,
    * first detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * second detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * third detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * first detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * second detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS,
    * third detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and multiple associated detection that met certain conditions
    */


   /** \result
    * Associated detections' flag f_use_in_dimension_update should be set to true in all cases
    */
   object_track.ndets = 3;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;
   object_track.detids[2] = 3;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;

   detection_props[1].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   detection_props[1].f_rr_inlier = true;

   detection_props[2].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[2].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[2].f_rr_inlier = true;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
   CHECK_TRUE(detection_props[1].f_use_in_dimension_update);
   CHECK_TRUE(detection_props[2].f_use_in_dimension_update);
}

/**
*\purpose  Deafault scenario for multiple incorrect detection, with parameters that should make f_use_in_dimension_update flag set to false.
Checking if this detections have proper f_use_in_dimension_update flag value
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_multiple_incorrect_detections)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * first detection's f_rr_inlier set to true,
    * first detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * first detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY,
    * second detection's f_rr_inlier set to false,
    * second detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * second detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS,
    * third detection is added to ensure that any_other_assoc_det_close for the first two detections is kept at its initial value
    * after Mark_Detections_With_Neighbors is called.
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and multiple associated detection that met certain conditions
    */


   /** \result
    * Associated detections' flag f_use_in_dimension_update should be set to true in all cases
    */
   object_track.ndets = 3;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;
   object_track.detids[2] = 3;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_STATIONARY;
   detection_props[0].f_rr_inlier = true;
   detection_props[0].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[1].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   detection_props[1].f_rr_inlier = false;
   detection_props[1].vcs_position.x = 3.0F;
   detection_props[1].vcs_position.y = 3.0F;
   detection_props[1].dist_to_closest_assoc_det_sq = 100.0F;

   detection_props[2].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[2].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   detection_props[2].f_rr_inlier = false;
   detection_props[2].vcs_position.x = 3.5F;
   detection_props[2].vcs_position.y = 3.5F;
   detection_props[2].dist_to_closest_assoc_det_sq = 100.0F;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_FALSE(detection_props[0].f_use_in_dimension_update);
   CHECK_FALSE(detection_props[1].f_use_in_dimension_update);
}

/**
*\purpose  Deafault scenario with one correct and two incorrect detections, 
with parameters that should make f_use_in_dimension_update flag set to true for first detections and false for the others.
Checking if this detections have proper f_use_in_dimension_update flag value
*\req    NA
*/
TEST(f360_mark_det_to_use_for_dim_update, test_one_correct_two_incorrect_detections)
{
   /** \precond
    * object_track associated with one detection,
    * object's f_moveable set to true,
    * first detection's f_rr_inlier set to true,
    * first detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * first detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    * second detection's f_rr_inlier set to false,
    * second detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_YES,
    * second detection's motion_status set torspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS,
    * third detection's f_rr_inlier set to true,
    * third detection's any_other_assoc_det_close set to F360_ANY_OTHER_ASSOC_DET_CLOSE_NO,
    * third detection's motion_status set to rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING,
    */

   /** \action
    * Call function Mark_Det_To_Use_For_Dim_Update for single object and multiple associated detection that met certain conditions
    */


   /** \result
    * Associated detections' flag f_use_in_dimension_update should be set to true in all cases
    */
   object_track.ndets = 2;
   object_track.detids[0] = 1;
   object_track.detids[1] = 2;

   object_track.f_moving = true;

   detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   detection_props[0].f_rr_inlier = true;

   detection_props[1].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   raw_detection_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   detection_props[1].f_rr_inlier = false;

   Mark_Det_To_Use_For_Dim_Update(object_track, calib, raw_detection_list, detection_props);

   CHECK_TRUE(detection_props[0].f_use_in_dimension_update);
   CHECK_FALSE(detection_props[1].f_use_in_dimension_update);
}
/** @}*/
