/** \file
File contains test cases for Calc_Nearest_Assoc_Det_Distance_Sq() function
*/

#include "f360_calc_nearest_assoc_det_distance_sq.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>


//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

/** \defgroup  calc_nearest_assoc_det_distance_sq
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  This tests group covers Calc_Nearest_Assoc_Det_Distance_Sq which works on 
*  detections associated to track. For each associated detection it calculates
*  the smallest squared distance to any of the other associated detections.
*/

TEST_GROUP(calc_nearest_assoc_det_distance_sq)
{
   /** \setup
   * Set single object and detection_props for tests purpose.
   */
   const float32_t tolerance = 1e-5F;
   F360_Object_Track_T object_track;
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS];

   TEST_SETUP()
   {
      // Reset objects and detections states after each test.
      memset(&object_track, 0U, sizeof(object_track));
      for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      {
         F360_Detection_Props_T * curr_det_props = &detection_props[det_idx];
         memset(curr_det_props, 0U, sizeof(&curr_det_props));
         curr_det_props->dist_to_closest_assoc_det_sq = INFTY;
      }
   }
};

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as 
           intended when there is no detection associated to processed track.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_no_dets_associated)
{
   /** \precond
   * ndets for processed track is equal to 0.
   */
   object_track.ndets = 0;

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq for object with no associated detections.
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * All detections should have dist_to_closest_assoc_det_sq equal to the initial value INFTY.
   */
   const float32_t exp_dist_to_closest_assoc_det_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_dist_to_closest_assoc_det_sq)
      {
         f_any_det_changed = true;
      }
   }
   CHECK_TEXT(f_any_det_changed == false, "dist_to_closest_assoc_det_sq has unexpectedly been modified for on or more detections.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
           intended if there is single detection associated to processed track.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_single_det_associated)
{
   /** \precond
   * ndets for processed track is equal to 1.
   */
   // Set object state
   object_track.ndets = 1;
   object_track.detids[0U] = 1U;

   // Set associated detection state: (0, 0) VCS
   detection_props[0U].vcs_position.x = 0.0F;
   detection_props[0U].vcs_position.y = 0.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with single detection associated.
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * All detections, the associated and the non-associated, should have dist_to_closest_assoc_det_sq
   * equal to init value INFTY because function computes nothing for single associated
   * det and for the non-associated detections function does nothing either.
   */
   const float32_t exp_dist_to_closest_assoc_det_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_dist_to_closest_assoc_det_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "dist_to_closest_assoc_det_sq has unexpectedly been modified for on or more detections.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
           intended when there are two detections associated to processed track.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_two_dets_associated)
{
   /** \precond
   * ndets for processed track is equal to 2.
   */
   // Set object state
   object_track.ndets = 2;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;

   // Set associated detection state: first detection point (1, 1) VCS
   detection_props[0U].vcs_position.x = 1.0F;
   detection_props[0U].vcs_position.y = 1.0F;

   // Second detection point(2, 2) VCS
   detection_props[1U].vcs_position.x = 2.0F;
   detection_props[1U].vcs_position.y = 2.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with two
   * detections associated in positions: (1,1), (2,2).
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * Both associated detection should have dist_to_closest_assoc_det_sq equal to 2.0.
   * Unassociated detections should be untouched and keep their initial value.
   */
   // Check associated detections
   const float32_t expected_distance_sq = 2.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 2.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 2U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as intended
*          when there are two detections with same position associated to processed track.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_two_dets_associated_same_position)
{
   /** \precond
   * ndets for processed track is equal to 2.
   */
   // Set object state
   object_track.ndets = 2;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;

   // Set associated detection state: first detection point (1.0, 1.0) VCS
   detection_props[0U].vcs_position.x = 1.0F;
   detection_props[0U].vcs_position.y = 1.0F;

   // Second detection point (1.0, 1.0) VCS
   detection_props[1U].vcs_position.x = 1.0F;
   detection_props[1U].vcs_position.y = 1.0F;

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq() for object with two
   * detections associated which are in same position.
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * Associated detections should have dist_to_closest_assoc_det_sq equal to value 0.0 because detections are in same position.
   * Unassociated detections should be untouched and keep their initial value.
   */
   // Check associated detections
   const float32_t expected_distance_sq = 0.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 2.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 2U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended when there are three detections where two of them have 
*          the same position and the third don't.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_three_dets_associated_two_have_same_position)
{
   /** \precond
   * ndets for processed track is equal to 3.
   */
   // Set object state
   object_track.ndets = 3;
   object_track.detids[0U] = 1U;
   object_track.detids[1U] = 2U;
   object_track.detids[2U] = 3U;

   // Set associated detection state: first detection point (1.0, 1.0) VCS
   detection_props[0U].vcs_position.x = 1.0F;
   detection_props[0U].vcs_position.y = 1.0F;

   // Second detection point(1.0, 1.0) VCS
   detection_props[1U].vcs_position.x = 1.0F;
   detection_props[1U].vcs_position.y = 1.0F;

   // Third detection point(2.0, 2.0) VCS
   detection_props[2U].vcs_position.x = 2.0F;
   detection_props[2U].vcs_position.y = 2.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with two detections 
   * associated which are in same position.
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * First and second associated detection should have dist_to_closest_assoc_det_sq equal
   * to value 0.0 because they are in same position but last detections should have 
   * dist_to_closest_assoc_det_sq equal to value 2.0.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq_1_2 = 0.0F;
   const float32_t expected_distance_sq_3 = 2.0F;

   DOUBLES_EQUAL_TEXT(expected_distance_sq_1_2, detection_props[0U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 1.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_1_2, detection_props[1U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 2.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_3, detection_props[2U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 3.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 3U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended when there are three detections associated to processed
*          track. Detection 1 and 2 are close to each other and detection
*          3 is further away but closest to detection 2.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_three_dets_associated_two_first_close_third_further_away_but_closest_to_det2)
{
   /** \precond
   * ndets for processed track is equal to 3.
   */
   // Set object state
   object_track.ndets = 3;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;

   // Set associated detection state: first detection point (1.0, 1.0) VCS
   detection_props[10U].vcs_position.x = 1.0F;
   detection_props[10U].vcs_position.y = 1.0F;

   // Second detection point (1.5, 1.2) VCS
   detection_props[20U].vcs_position.x = 1.5F;
   detection_props[20U].vcs_position.y = 1.2F;

   // Third detection point(3.0, 3.0) VCS
   detection_props[30U].vcs_position.x = 3.0F;
   detection_props[30U].vcs_position.y = 3.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with three detection associated.
   * First and second detections are close to each other and third is further away.
   Detections positions (1.0, 1.0), (1.5, 1.2), (3.0, 3.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * First two detections should have dist_to_closest_assoc_det_sq equal to 0.29 and third detections
   * should have 5.49.
   * Unassociated detections should be untouched and keep their initial value.
   */
   // Check associated detections
   const float32_t expected_distance_sq_det_idx_10_20 = 0.29F;
   const float32_t expected_distance_sq_det_idx_30 = 5.49F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_10_20, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_10_20, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_30, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as intened
*          when there is three detections associated to processed track.
*          Detection 1 and 2 are close to each other and detection 3 is further away 
*          but closest to detection 1.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_three_dets_associated_two_first_close_third_further_away_but_closest_to_det1)
{
   /** \precond
   * ndets for processed track is equal to 3.
   */
   // Set object state
   object_track.ndets = 3;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;

   // Set associated detection state: first detection point (1.0, 1.0) VCS
   detection_props[10U].vcs_position.x = 1.0F;
   detection_props[10U].vcs_position.y = 1.0F;

   // Second detection point (1.5, 1.2) VCS
   detection_props[20U].vcs_position.x = 1.5F;
   detection_props[20U].vcs_position.y = 1.2F;

   // Third detection point(-3.0, -3.0) VCS
   detection_props[30U].vcs_position.x = -3.0F;
   detection_props[30U].vcs_position.y = -3.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with three detections
   * associated. First and second are close to each other and third is further away.
   Detections positions (1.0, 1.0), (1.5, 1.2), (3.0, 3.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * First two dets should have dist_to_closest_assoc_det_sq equal to 0.29 and third 32.0.
   * Unassociated detections should be untouched and keep their initial value.
   */
   // Check associated detections
   const float32_t expected_distance_sq_det_idx_10_20 = 0.29F;
   const float32_t expected_distance_sq_det_idx_30 = 32.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_10_20, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_10_20, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_30, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if ((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}


/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended if there is three detections associated to processed track.
*          Detection 2 and 3 are close to each other and detection 1 is further 
*          away.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_three_dets_associated_two_last_close)
{
   /** \precond
   * ndets for processed track is equal to 3.
   */
   // Set object state
   object_track.ndets = 3;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;

   // Set associated detection state: first detection point (1.0, 1.0) VCS
   detection_props[10U].vcs_position.x = 1.0F;
   detection_props[10U].vcs_position.y = 1.0F;

   // Second detection point (2.5, 2.5) VCS
   detection_props[20U].vcs_position.x = 2.5F;
   detection_props[20U].vcs_position.y = 2.5F;

   // Third detection point(3.0, 3.0) VCS
   detection_props[30U].vcs_position.x = 3.0F;
   detection_props[30U].vcs_position.y = 3.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with three detection associated.
   * Third and second are close to each other and first is further.
   * Detections positions (1.0, 1.0), (2.5, 2.5), (3.0, 3.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * First detection should have dist_to_closest_assoc_det_sq equal to 4.5 and the two last 
   * detections should have 0.5.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq_det_idx_10 = 4.5F;
   const float32_t expected_distance_sq_det_idx_20_30 = 0.5F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_10, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_20_30, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_det_idx_20_30, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended if there is three detections associated to processed track.
*          Detection 1 and 2 are close to each other and detection 3 is further
*          away.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_three_dets_associated_equal_distances)
{
   /** \precond
   * ndets for processed track is equal to 3.
   */
   // Set object state
   object_track.ndets = 3;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;

   // Set associated detection state: first detection point (1.0, 0.0) VCS
   detection_props[10U].vcs_position.x = 1.0F;
   detection_props[10U].vcs_position.y = 0.0F;

   // Second detection point (3.0, 0.0) VCS
   detection_props[20U].vcs_position.x = 3.0F;
   detection_props[20U].vcs_position.y = 0.0F;

   // Third detection point (2, sqrt(3)) VCS
   detection_props[30U].vcs_position.x = 2.0F;
   detection_props[30U].vcs_position.y = 1.7320508F;  // sqrt(3)

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq() for object with
   * three detections associated. All distances should be equal
   * Detections positions (1.0, 0.0), (3.0, 0.0), (2, sqrt(3)) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * All associated dets should have distances equal to 4.0.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq = 4.0F;

   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended if there is four detections associated to processed track.
*          Detections are placed in a single line.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_four_associated_dets_in_same_line)
{
   /** \precond
   * ndets for processed track is equal to 4.
   */
   // Set object state
   object_track.ndets = 4;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;
   object_track.detids[3U] = 41U;

   // Set associated detection state: first detection point (1.0, 0.0) VCS
   detection_props[10U].vcs_position.x = 1.0F;
   detection_props[10U].vcs_position.y = 0.0F;

   // Second detection point (2.0, 0.0) VCS
   detection_props[20U].vcs_position.x = 2.0F;
   detection_props[20U].vcs_position.y = 0.0F;

   // Third detection point (2.5, 0.0) VCS
   detection_props[30U].vcs_position.x = 2.5F;
   detection_props[30U].vcs_position.y = 0.0F;

   // Fourth detection point (2.6, 0.0) VCS
   detection_props[40U].vcs_position.x = 2.6F;
   detection_props[40U].vcs_position.y = 0.0F;

   /** \action
   * Call a function Calc_Nearest_Assoc_Det_Distance_Sq for object with four detections associated.
   * Detections are placed in straight line, and distances between them are getting smaller.
   * Detections positions (1.0, 0.0), (2.0, 0.0), (2.5, 0.0) (2.6, 0.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * First det should have dist_to_closest_assoc_det_sq equal to 1.0, second 0.25, third and fourth 0.01.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq_10 = 1.0F;
   const float32_t expected_distance_sq_20 = 0.25F;
   const float32_t expected_distance_sq_30_40 = 0.01F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_10, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_20, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_30_40, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_30_40, detection_props[40U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 41.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U) || (det_idx == 40U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as intended
*          if there is four detections associated to processed track. Detections 
*          are placed in a square-shape.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_four_associated_dets_in_square_shape)
{
   /** \precond
   * ndets for processed track is equal to 4.
   */
   // Set object state
   object_track.ndets = 4;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 31U;
   object_track.detids[3U] = 41U;

   // Set associated detection state: first detection point (0.0, 0.0) VCS
   detection_props[10U].vcs_position.x = 0.0F;
   detection_props[10U].vcs_position.y = 0.0F;

   // Second detection point (0.0, 1.0) VCS
   detection_props[20U].vcs_position.x = 0.0F;
   detection_props[20U].vcs_position.y = 1.0F;

   // Third detection point (1.0, 0.0) VCS
   detection_props[30U].vcs_position.x = 1.0F;
   detection_props[30U].vcs_position.y = 0.0F;

   // Fourth detection point (1.0, 1.0) VCS
   detection_props[40U].vcs_position.x = 1.0F;
   detection_props[40U].vcs_position.y = 1.0F;

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq() for object with four
   * detections placed in square-shape.
   * Detections positions (0.0, 0.0), (0.0, 1.0), (1.0, 0.0) (1.0, 1.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * For all detections dist_to_closest_assoc_det_sq should be equal to 1.0.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq = 1.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[40U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 41.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U) || (det_idx == 40U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended if there is six detections associated to processed track.
*          Detections are placed in hexagon and idx are mixed.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_associated_dets_in_heaxagon_shape)
{
   /** \precond
   * ndets for processed track is equal to 6.
   */
   // Set object state
   object_track.ndets = 6;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 61U;
   object_track.detids[3U] = 51U;
   object_track.detids[4U] = 31U;
   object_track.detids[5U] = 41U;

   const float32_t sqrt_3 = 1.7320508F;

   // Set associated detection state: first detection point (-2.0, 0.0) VCS
   detection_props[10U].vcs_position.x = -2.0F;
   detection_props[10U].vcs_position.y = 0.0F;

   // Second detection point (-1.0, sqrt_3) VCS
   detection_props[20U].vcs_position.x = -1.0F;
   detection_props[20U].vcs_position.y = -sqrt_3;

   // Third detection point (1.0, -sqrt_3) VCS
   detection_props[30U].vcs_position.x = 1.0F;
   detection_props[30U].vcs_position.y = -sqrt_3;

   // Fourth detection point (2.0, 0.0) VCS
   detection_props[40U].vcs_position.x = 2.0F;
   detection_props[40U].vcs_position.y = 0.0F;

   // Fifth detection point (1.0, sqrt_3) VCS
   detection_props[50U].vcs_position.x = 1.0F;
   detection_props[50U].vcs_position.y = sqrt_3;

   // Sixth detection point (-1.0, sqrt_3) VCS
   detection_props[60U].vcs_position.x = -1.0F;
   detection_props[60U].vcs_position.y = sqrt_3;

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq() for object with six
   * detections placed in hexagon-shape.
   * Detections positions (0.0, 0.0), (0.0, 1.0), (1.0, 0.0) (1.0, 1.0) VCS
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * For all detections dist_to_closest_assoc_det_sq should be equal to 2.0.
   * Unassociated detections should be untouched and keep their initial value.
   */

   // Check associated detections
   const float32_t expected_distance_sq = 4.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[40U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 41.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[50U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 51.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq, detection_props[60U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 61.");

   // Check unassociated detections
   const float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U) || (det_idx == 40U) || (det_idx == 50U) || (det_idx == 60U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/**
*\purpose  Check if function Calc_Nearest_Assoc_Det_Distance_Sq() works as
*          intended if there is seven detections associated to processed track.
*          Detections are placed in hexagon plus additional detection. Within
*          track detections indexes are mixed, so check robustness for indexes
*          order.
*\req    NA
*/

TEST(calc_nearest_assoc_det_distance_sq, test_Calc_Nearest_Assoc_Det_Distance_Sq_associated_dets_in_heaxagon_shape_and_additional_det)
{
   /** \precond
   * ndets for processed track is equal to 7.
   */
   // Set object state
   object_track.ndets = 7;
   object_track.detids[0U] = 11U;
   object_track.detids[1U] = 21U;
   object_track.detids[2U] = 61U;
   object_track.detids[3U] = 51U;
   object_track.detids[4U] = 31U;
   object_track.detids[5U] = 41U;
   object_track.detids[6U] = 71U;

   const float32_t sqrt_3 = 1.7320508F;

   // Set associated detection state: first detection point (-2.0, 0.0) VCS
   detection_props[10U].vcs_position.x = -2.0F;
   detection_props[10U].vcs_position.y = 0.0F;

   // Second detection point (-1.0, sqrt_3) VCS
   detection_props[20U].vcs_position.x = -1.0F;
   detection_props[20U].vcs_position.y = -sqrt_3;

   // Third detection point (1.0, -sqrt_3) VCS
   detection_props[30U].vcs_position.x = 1.0F;
   detection_props[30U].vcs_position.y = -sqrt_3;

   // Fourth detection point (2.0, 0.0) VCS
   detection_props[40U].vcs_position.x = 2.0F;
   detection_props[40U].vcs_position.y = 0.0F;

   // Fifth detection point (1.0, sqrt_3) VCS
   detection_props[50U].vcs_position.x = 1.0F;
   detection_props[50U].vcs_position.y = sqrt_3;

   // Sixth detection point (-1.0, sqrt_3) VCS
   detection_props[60U].vcs_position.x = -1.0F;
   detection_props[60U].vcs_position.y = sqrt_3;

   // Seventh detection point (-1.0, -1.0) VCS
   detection_props[70U].vcs_position.x = -1.0F;
   detection_props[70U].vcs_position.y = -1.0F;

   /** \action
   * Call function Calc_Nearest_Assoc_Det_Distance_Sq() for object with six detections placed in hexagon-shape.
   */
   Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

   /** \result
   * For all detections dist_to_closest_assoc_det_sq should be equal to 4.0 except for
   * detection 10 where it should be 2.0, and detection 70 and detatcion 20 where it 
   * should be 0.535898.
   * Unassociated detections should be untouched and keep their initial value.
   */
   // Check associated detections
   const float32_t main_expected_distance_sq = 4.0F;
   const float32_t expected_distance_sq_20_70 = 0.535898F;
   const float32_t expected_distance_sq_10_70 = 2.0F;
   DOUBLES_EQUAL_TEXT(expected_distance_sq_10_70, detection_props[10U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 11.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_20_70, detection_props[20U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 21.");
   DOUBLES_EQUAL_TEXT(main_expected_distance_sq, detection_props[30U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 31.");
   DOUBLES_EQUAL_TEXT(main_expected_distance_sq, detection_props[40U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 41.");
   DOUBLES_EQUAL_TEXT(main_expected_distance_sq, detection_props[50U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 51.");
   DOUBLES_EQUAL_TEXT(main_expected_distance_sq, detection_props[60U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 61.");
   DOUBLES_EQUAL_TEXT(expected_distance_sq_20_70, detection_props[70U].dist_to_closest_assoc_det_sq, tolerance, "Unexpected dist_to_closest_assoc_det_sq for detection with ID 71.");

   // Check unassociated detections
   float32_t exp_unassoc_expected_distance_sq = INFTY;
   bool f_any_det_changed = false;
   for (uint32_t det_idx = 0U; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
   {
      if((det_idx == 10U) || (det_idx == 20U) || (det_idx == 30U) || (det_idx == 40U) || (det_idx == 50U) || (det_idx == 60U) || (det_idx == 70U))
      {
         // Skip the associated detections
      }
      else if (detection_props[det_idx].dist_to_closest_assoc_det_sq != exp_unassoc_expected_distance_sq)
      {
         f_any_det_changed = true;
      }
   }

   CHECK_TEXT(f_any_det_changed == false, "One or more of the unassociated detections has unexpectedly been modified.")
}

/** @}*/
