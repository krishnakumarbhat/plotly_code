/** \file
 * This file contains unit tests for content of f360_classification_underdrivability_moving.cpp file
 */

#include "f360_classification_underdrivability_moving.h"
#include "f360_object_underdrivability_classification.h"
#include "f360_clear_object_track.h"
#include "f360_get_wall_time.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  Test_Assign_Underdrivability_Status_To_Moving_Objects
 *  @{
 */

/** \brief
 * Test group of Assign_underdrivability_Status_To_Moving_Objects() function. Test checks if 
 * Assign_underdrivability_Status_To_Moving_Objects() and all of
 * the subfunctions are called properly.
 */

TEST_GROUP(Test_Assign_Underdrivability_Status_To_Moving_Objects)
{
   // Declare common variables used within all tests in this test group.
   F360_Calibrations_T calib;
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   F360_TRKR_TIMING_INFO_T timing_info;
   
   /** \setup
    * Init tracker calibration
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      Clear_Object_Track(object_tracks[0]);
   }
};

/** \purpose  
 * This test checks if the object underdrivability status is set to
 * UNDERDRIVABLE_STATUS_CAN_PASS_UNDER if all of the conditions are met when objects
 * position x changes to be below the threshold.
 * This test also checks if the underdrivable probability is set correctly 
 * \req
 * NA.
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_status_is_assigned_if_conditions_met_obj_enters_area)
{
   /** \precond
    * ud status UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER
    * otg_height above threshold
    * vcs position x below the threshold
    * ud_mov_cnt_underdrivable above minimal value for assignemnt of the status
   */
   object_tracks[0].underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
   object_tracks[0].vcs_position.x = calib.ud_mov_posx_max_limit - 0.01F;
   object_tracks[0].otg_height = calib.ud_mov_height_threshold + 0.01F;
   object_tracks[0].ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U;
   object_tracks[0].ndets = 1;
   object_tracks[0].detids[0] = 1U;

   /** \action
    * call Assign_Underdrivability_Status_To_Moving_Object() function
   */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * Check if underdrivable_status is properly set to underdrivable.
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_CAN_PASS_UNDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_can_pass_under, object_tracks[0].probability_underdrivable, F360_EPSILON);
}

/** \purpose  
 * This test checks if the object underdrivability status is set to
 * UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER if all of the conditions are met when objects
 * position x changes to be above the threshold.
 * This test also checks if the underdrivable probability is set correctly
 * \req
 * NA.
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_status_is_assigned_if_conditions_met_obj_exits_area)
{
   /** \precond
    * ud status UNDERDRIVABLE_STATUS_CAN_PASS_UNDER
    * otg_height above threshold
    * vcs position x above the threshold
    * ud_mov_cnt_underdrivable above minimal value for assignemnt of the status
   */
   object_tracks[0].underdrivable_status = ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER;
   object_tracks[0].vcs_position.x = calib.ud_mov_posx_max_limit + 0.01F;
   object_tracks[0].otg_height = calib.ud_mov_height_threshold + 0.01F;
   object_tracks[0].ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U;
   object_tracks[0].ndets = 1;
   object_tracks[0].detids[0] = 1U;

   /** \action
    * call Assign_Underdrivability_Status_To_Moving_Object() function
   */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * Check if underdrivable_status is properly set to not to consider.
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_not_to_consider, object_tracks[0].probability_underdrivable, F360_EPSILON);
}

/** \purpose  
 * This test checks if the counter is incremented when the track is underdrivable
 * 
 * \req
 * NA
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_cnt_underdrivable_is_incremented)
{
   /** \precond
    * make sure that the object historic_height_mean is above underdrivable threshold
    * set the cnt_ud_underdrivable counter to zero
    */

   object_tracks[0].otg_height = calib.ud_mov_height_threshold + 0.1F;
   object_tracks[0].ud_mov_cnt_underdrivable = 0U;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * Verify that cnt_ud_underdrivable is incremented
    */
   CHECK_TRUE(object_tracks[0].ud_mov_cnt_underdrivable  > 0U);
}

/** \purpose  
 * This test checks if the counter is set to 0 since the track is not underdrivable
 * \req
 * NA
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_cnt_underdrivable_is_reset)
{
   /** \precond
    * Make sure that the object is not underdrivable (historic_height_mean below the calibration threshold)
    * The counter is not zero to check if the value is set to zero after 
    */

   object_tracks[0].otg_height = calib.ud_mov_height_threshold - 0.1F;
   object_tracks[0].ud_mov_cnt_underdrivable = 1U;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * ud_mov_cnt_underdrivable counter did reset
    */
   CHECK_TRUE(object_tracks[0].ud_mov_cnt_underdrivable == 0U);
}

/** \purpose  
 * This test checks if the track is not in front of the host and is thus not considered for underdrivability
 * vcs position x < 0
 * This test also checks if the underdrivable probability is set correctly
 * \req
 * NA.
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_obj_is_not_considered_when_behind)
{
   /** \precond
    * The track is not in front of the host and is thus not evaluated for underdrivability
    */

   object_tracks[0].vcs_position.x = calib.ud_mov_posx_min_limit - 0.1F;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * underdrivable_status is properlty set as not to be considered
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_not_to_consider, object_tracks[0].probability_underdrivable, F360_EPSILON);
}

/** \purpose  
 * This test checks if the track is too far ahead in front of the host and is thus not considered for underdrivability
 * and its status is set to 
 * (100 < vcs position x)
 * This test also checks if the underdrivable probability is set correctly
 * \req
 * NA
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_obj_is_not_considered_when_too_far)
{
   /** \precond
    * The track is too far ahead in front of the host and is thus not evaluated for underdrivability
    */

   object_tracks[0].vcs_position.x = calib.ud_mov_posx_max_limit + 0.1F;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * underdrivable_status is properlty set as not to be considered
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_not_to_consider, object_tracks[0].probability_underdrivable, F360_EPSILON);
}

/** \purpose  
 * This test checks if the track will have its underdrivability status set to UNDERDRIVABLE_STATUS_CAN_PASS_UNDER
 * since it satisfies all conditions - i.e is within longitudinal gates of underdrivability classification, its
 * historic height mean is above the set threshold at current and sufficient number of previous scan indexes.
 * This test also checks if the underdrivable probability is set correctly
 * \req
 * NA
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_obj_is_classified_ud)
{
   /** \precond
    * Make sure that the track is considered for underdrivability
    * Make sure that the object is underdrivable
    * Make sure that there has been enough consecutive scans in which the object was underdrivable to set a new status
    */

   object_tracks[0].vcs_position.x = calib.ud_mov_posx_max_limit - 0.1F;
   object_tracks[0].otg_height = calib.ud_mov_height_threshold + 0.1F;
   object_tracks[0].ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * objects underdrivable_status is properlty set as underdrivable
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_CAN_PASS_UNDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_can_pass_under, object_tracks[0].probability_underdrivable, F360_EPSILON);
}

/** \purpose  
 * This test checks if the track will have its underdrivability status set to UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER 
 * when all conditions are met -  it is within longitudinal gates of underdrivability classification, its
 * historic height mean is above the set threshold at current scan index, but was not above it for
 * required number of scan indexes.
 * set threshold.
 * This test also checks if the underdrivable probability is set correctly
 * \req
 * NA
 */
TEST(Test_Assign_Underdrivability_Status_To_Moving_Objects, check_if_obj_is_classified_not_ud)
{
   /** \precond
    * Make sure that the track is considered for underdrivability
    * Make sure that the object historic_height_mean is above threshold for underdrivable
    * Make sure that there has not been enough consecutive scans in which the object was underdrivable to set a new status
    */

   object_tracks[0].vcs_position.x = calib.ud_mov_posx_max_limit - 0.1F;
   object_tracks[0].otg_height = calib.ud_mov_height_threshold + 0.1F;
   object_tracks[0].otg_height = calib.ud_mov_cnt_consecutive_scans - 1U;

   /** \action
    * Call Assign_Underdrivability_Status_To_Moving_Object().
    */
   Assign_Underdrivability_Status_To_Moving_Object(calib, object_tracks[0], timing_info);

   /** \result
    * objects underdrivable_status is properlty set as UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER
    * Check that the assigned underdrivability probability corresponds to "UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER"
    */
   CHECK_TRUE(object_tracks[0].underdrivable_status == ocg::UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER);
   DOUBLES_EQUAL(calib.ud_mov_prob_can_not_pass_under, object_tracks[0].probability_underdrivable, F360_EPSILON);
}
/** @}*/

