/** \file
     contains test cases for Mark_Detections_With_Neighbors() function
*/

#include "f360_mark_detections_with_neighbors.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>

/** \defgroup  f360_mark_detections_with_neighbors
 *  @{
 */

using namespace f360_variant_A;
/** \brief
 *  Test group for Mark_Detections_With_Neighbors and its support functions. 
 *  It checks whether, for each detection associated to an object, the 
 *  detection properties any_other_assoc_det_close and 
 *  dist_to_closest_assoc_det_sq signal is set properly.  
 */

TEST_GROUP(f360_mark_detections_with_neighbors)
{
   /** \setup
   * Declaration of single object_track and detection_props table
   */
   const float32_t tolerance = 1e-5F;
   const float32_t small_distance = 0.1F;
   const float32_t huge_distance = 100.0F;
   F360_Object_Track_T object_track;
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Calibrations_T calibrations;

   TEST_SETUP()
   {
      // Initialize objects and detections states after each test.
      memset(&object_track, 0, sizeof(object_track));
      
      for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      { 
         F360_Detection_Props_T * curr_det_props = &detection_props[det_idx];
         memset(curr_det_props, 0, sizeof(&curr_det_props));
         curr_det_props->any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
         curr_det_props->dist_to_closest_assoc_det_sq = INFTY;
      }

      calibrations.max_dist_for_neighbor_detections = 4.0F;
   }

   /** \teardown
    * Nothing to teardown in this test group
    */
   TEST_TEARDOWN()
   {
      //mock.clear();
   }

};


/**
*\purpose  Check if support function function Check_If_Detection_Has_Neighbor()
*          correctly sets any_other_assoc_det_close property for detections 
*          associated  an object
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Check_If_Detection_Has_Neighbor)
{
   /** \precond
   * Object with seven associated detections. All detections have different 
   * dist_to_closest_assoc_det_sq.
   */
   object_track.ndets = 7;

   object_track.detids[0U] = 21U;
   object_track.detids[1U] = 54U;
   object_track.detids[2U] = 4U;
   object_track.detids[3U] = 58U;
   object_track.detids[4U] = 12U;
   object_track.detids[5U] = 13U;
   object_track.detids[6U] = 8U;

   detection_props[20U].dist_to_closest_assoc_det_sq = 2.3F;
   detection_props[53U].dist_to_closest_assoc_det_sq = 20.0F;
   detection_props[3U].dist_to_closest_assoc_det_sq = 16.001F;
   detection_props[57U].dist_to_closest_assoc_det_sq = 15.999F;
   detection_props[11U].dist_to_closest_assoc_det_sq = 0.0F;
   detection_props[12U].dist_to_closest_assoc_det_sq = 100.4F;
   detection_props[7U].dist_to_closest_assoc_det_sq = 9.8;

   /** \action
   * Run Check_If_Detection_Has_Neighbor to set any_other_assoc_det_close for the associated detections.
   */
   Check_If_Detection_Has_Neighbor(object_track, calibrations, detection_props);


   /** \result
   * Associated detections with dist_to_closest_assoc_det_sq less than 16.0F should be marked as 
   * F360_ANY_OTHER_ASSOC_DET_CLOSE_YES and if dist_to_closest_assoc_det_sq is larger than 16 they 
   * should be marked as F360_ANY_OTHER_ASSOC_DET_CLOSE_NO. Unassociated detections should be untouched
   * and therefore still have the initial value F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED.
   */

   // Check associated detections that should be marked as having a neighbour
   const F360_Any_Other_Assoc_Det_Close expected_any_other_assoc_det_close_21_58_12_8 = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
   CHECK_EQUAL_TEXT(expected_any_other_assoc_det_close_21_58_12_8, detection_props[20U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 21.");
   CHECK_EQUAL_TEXT(expected_any_other_assoc_det_close_21_58_12_8, detection_props[57U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 58.");
   CHECK_EQUAL_TEXT(expected_any_other_assoc_det_close_21_58_12_8, detection_props[11U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 12.");
   CHECK_EQUAL_TEXT(expected_any_other_assoc_det_close_21_58_12_8, detection_props[7U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 8.");

   // Check associated detections that should be marked as not having a neighbour
   const F360_Any_Other_Assoc_Det_Close exepected_any_other_assoc_det_close_54_4_13 = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
   CHECK_EQUAL_TEXT(exepected_any_other_assoc_det_close_54_4_13, detection_props[53U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 54.");
   CHECK_EQUAL_TEXT(exepected_any_other_assoc_det_close_54_4_13, detection_props[3U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 4.");
   CHECK_EQUAL_TEXT(exepected_any_other_assoc_det_close_54_4_13, detection_props[12U].any_other_assoc_det_close, "Unexpected value of any_other_assoc_det_close for detection with ID 13.");

   // Check all non-associated detections that should be marked as undetermined 
   const F360_Any_Other_Assoc_Det_Close exepected_any_other_assoc_det_close_unassoc = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if ((20U == det_idx) || (57U == det_idx) || (11U == det_idx) || (7U == det_idx) || (53U == det_idx) || (3U == det_idx) || (12U == det_idx))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].any_other_assoc_det_close != exepected_any_other_assoc_det_close_unassoc)
      {
         f_any_det_changed = true;
      }
   }
   CHECK_TEXT(false == f_any_det_changed, "One or more of the unassociated detections has unexpectedly been modified.")
}


/**
*\purpose  Check that function Mark_Detections_With_Neighbors() doesn't calculate distance
*          when there is no associated detection to the object
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_without_detections)
{
   /** \precond
   * Object without any associated detection.
   */
   object_track.ndets = 0;

   /** \action
   * Run Mark_Detections_With_Neighbors()
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Due to lack of associated detections any_other_assoc_det_close and 
   * dist_to_closest_assoc_det_sq properties of all detections should be 
   * untouched and still have their initial values.
   */
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}

/**
*\purpose  Check that function Mark_Detections_With_Neighbors() doesn't calculate distance 
*          when only one detection is associated to object and verify that 
*          any_other_assoc_det_close is undetermined.
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_with_one_detection)
{
   /** \precond
   * Object with one associated detection.
   */
   object_track.ndets = 1;
   object_track.detids[0U] = 1U;

   /** \action
   * run Mark_Detections_With_Neighbors.
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Because of only one associated detection Calc_Nearest_Assoc_Det_Distance_Sq() 
   * shouldn't be called and the value of any_other_assoc_det_close shouldn't be
   * determined. All unassociated detections should be untouched and still have 
   * the initial value.
   */

   // Check associated detection
   DOUBLES_EQUAL_TEXT(detection_props[0U].dist_to_closest_assoc_det_sq, INFTY, F360_EPSILON, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 1.");
   CHECK_EQUAL_TEXT(detection_props[0U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED, "Unexpected value of any_other_assoc_det_close for detection with ID 1.");

   // Check unassociated detections
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 1U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}

/**
*\purpose  Verify that any_other_assoc_det_close is set to NO for two detections 
*          associated to an object when the detections are far away from each other.
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_with_two_detections_without_neighbors)
{
   /** \precond
   * Object with two associated detections.
   Their distance is huge.
   */
   object_track.ndets = 2;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;

   detection_props[0U].vcs_position.x = 0.0F;
   detection_props[0U].vcs_position.y = 0.0F;

   detection_props[1U].vcs_position.x = detection_props[0U].vcs_position.x + huge_distance;
   detection_props[1U].vcs_position.y = detection_props[0U].vcs_position.y + huge_distance;

   /** \action
   * Run Mark_Detections_With_Neighbors()
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Due to huge distance between the associated detections they should be 
   * recognized as F360_ANY_OTHER_ASSOC_DET_CLOSE_NO.
   * The closest distance for the detections should be 2*huge_distance^2.
   * Unassociated detections should be untouched and keep their initial values.
   */

   // Check the associated detections
   CHECK_EQUAL_TEXT(detection_props[0U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_NO, "Unexpected value of any_other_assoc_det_close for detection with ID 1.");
   CHECK_EQUAL_TEXT(detection_props[1U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_NO, "Unexpected value of any_other_assoc_det_close for detection with ID 2.");

   const float expected_distance_sq = 2.0F*huge_distance*huge_distance;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 2.");

   // Check unassociated detections
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 2U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}

/**
*\purpose  Verify that any_other_assoc_det_close is set to YES for two detections
*          associated to an objectvwhen the detections are close to each other.
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_with_two_detections_close_to_each_other)
{
   /** \precond
   * Object with two associated detections.
   Their distance is small.
   */
   object_track.ndets = 2;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;

   detection_props[0U].vcs_position.x = 0.0F;
   detection_props[0U].vcs_position.y = 0.0F;

   detection_props[1U].vcs_position.x = detection_props[0U].vcs_position.x + small_distance;
   detection_props[1U].vcs_position.y = detection_props[0U].vcs_position.y + small_distance;

   /** \action
   * Run Mark_Detections_With_Neighbors()
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Due to multiple detections associated to object Calc_Nearest_Assoc_Det_Distance_Sq
   * should be called. Due to small distance to other detections these detections should
   * be recognize as F360_ANY_OTHER_ASSOC_DET_CLOSE_YES.
   * The closest distance for the detections should be 2*small_distance^2.
   * Unassociated detections should be untouched and keep their initial values.
   */

   // Check associated detections
   CHECK_EQUAL_TEXT(detection_props[0U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 1.");
   CHECK_EQUAL_TEXT(detection_props[1U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 2.");

   const float expected_distance_sq = 2.0F*small_distance*small_distance;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 2.");

   // Check unassociated detections
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 2U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}

/**
*\purpose  Verify that any_other_assoc_det_close is set to NO for one of three associated
*          detection which is far away from the two other others and that the two 
*          associated detections which are close to each other have 
*          any_other_assoc_det_close set to YES
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_with_three_detections_where_first_two_are_close)
{
   /** \precond
   * Object with three associated detections
   One of them has huge distance from the two others.
   */
   object_track.ndets = 3;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;
   object_track.detids[2U] = 3U;

   detection_props[0U].vcs_position.x = 0.0F;
   detection_props[0U].vcs_position.y = 0.0F;

   detection_props[1U].vcs_position.x = detection_props[0U].vcs_position.x + small_distance;
   detection_props[1U].vcs_position.y = detection_props[0U].vcs_position.y + small_distance;

   detection_props[2U].vcs_position.x = detection_props[0U].vcs_position.x - huge_distance;
   detection_props[2U].vcs_position.y = detection_props[0U].vcs_position.y - huge_distance;

   /** \action
   * Run Mark_Detections_With_Neighbors()
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Due to huge distance to the other associated detections the third detection 
   * should be recognize as F360_ANY_OTHER_ASSOC_DET_CLOSE_NO. The two first 
   * detections are close and should be recoginze as F360_ANY_OTHER_ASSOC_DET_CLOSE_YES.
   * Expected closest distance for third detection is 2*huge_distance^2. Expected 
   * closest distance for first two detections are 2*small_distance^2
   * Unassociated detections should be untouched and keep their initial values.
   */

   // Check associated detections
   CHECK_EQUAL_TEXT(detection_props[0U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 1.");
   CHECK_EQUAL_TEXT(detection_props[1U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 2.");
   CHECK_EQUAL_TEXT(detection_props[2U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_NO, "Unexpected value of any_other_assoc_det_close for detection with ID 3.");

   const float expected_distance_sq_1_2 = 2.0F*small_distance*small_distance;
   const float expected_distance_sq_3 = 2.0F*huge_distance*huge_distance;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_1_2, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_1_2, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 2.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_3, detection_props[2U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 3.");

   // Check unassociated detections
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 3U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}

/**
*\purpose  Verify that any_other_assoc_det_close is set to NO for one of three associated
*          detection which is far away from the two other others and that the two 
*          associated detections which are close to each other have 
*          any_other_assoc_det_close set to YES.
*\req      NA
*/
TEST(f360_mark_detections_with_neighbors, test_Mark_Detections_With_Neighbors_object_with_three_detections_where_first_is_far_away)
{
   /** \precond
   * Object with three associated detections
   One of them has huge distance from the two others.
   */
   object_track.ndets = 3;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;
   object_track.detids[2U] = 3U;

   detection_props[0U].vcs_position.x = 0.0F;
   detection_props[0U].vcs_position.y = 0.0F;

   detection_props[1U].vcs_position.x = detection_props[0U].vcs_position.x + huge_distance;
   detection_props[1U].vcs_position.y = detection_props[0U].vcs_position.y + huge_distance;

   detection_props[2U].vcs_position.x = detection_props[1U].vcs_position.x + small_distance;
   detection_props[2U].vcs_position.y = detection_props[1U].vcs_position.y + small_distance;

   /** \action
   * Run Mark_Detections_With_Neighbors()
   */
   Mark_Detections_With_Neighbors(object_track, calibrations, detection_props);

   /** \result
   * Due to huge distance to the other associated detections the first
   * detection should be recognize as F360_ANY_OTHER_ASSOC_DET_CLOSE_NO.
   * The two other two associated detections are close and should be 
   * recognized as F360_ANY_OTHER_ASSOC_DET_CLOSE_YES.
   * Expected closest distance for first detection is 2*huge_distance^2.
   * Expected closest distance for second and third detections are 2*small_distance^2
   * Unassociated detections should be untouched and keep their initial values.
   */

   // Check associated detections
   CHECK_EQUAL_TEXT(detection_props[0U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_NO, "Unexpected value of any_other_assoc_det_close for detection with ID 1.");
   CHECK_EQUAL_TEXT(detection_props[1U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 2.");
   CHECK_EQUAL_TEXT(detection_props[2U].any_other_assoc_det_close, F360_ANY_OTHER_ASSOC_DET_CLOSE_YES, "Unexpected value of any_other_assoc_det_close for detection with ID 3.");

   const float expected_distance_sq_1 = 2.0F*huge_distance*huge_distance;
   const float expected_distance_sq_2_3 = 2.0F*small_distance*small_distance;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_1, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_2_3, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 2.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_2_3, detection_props[2U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected value of dist_to_closest_assoc_det_sq for detection with ID 3.");

   // Check unassociated detections
   bool f_dets_status_undeterminied = true;
   bool f_dets_distance_not_computed = true;
   for (uint32_t det_idx = 3U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      f_dets_status_undeterminied &= (F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED == detection_props[det_idx].any_other_assoc_det_close);
      f_dets_distance_not_computed &= (INFTY == detection_props[det_idx].dist_to_closest_assoc_det_sq);
   }
   CHECK_TRUE_TEXT(f_dets_status_undeterminied, "any_other_assoc_det_close of one or more detections has unexpectedly been modified.");
   CHECK_TRUE_TEXT(f_dets_distance_not_computed, "dist_to_closest_assoc_det_sq of one or more detections has unexpectedly been modified.");
}


