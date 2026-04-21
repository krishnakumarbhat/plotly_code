/** \file
 * This file contains unit tests for content of f360_update_by_tracks_helpers.cpp file
 */

#include "f360_update_by_tracks_helpers.h"
#include <CppUTest/TestHarness.h>

#include "f360_calibrations.h"

using namespace f360_variant_A;

/** \defgroup  should_object_be_considered
 *  @{
 */

 /** \brief
  *
  */
TEST_GROUP(should_object_be_considered)
{
   F360_Object_Track_T object{};
   F360_Calibrations_T calibrations;

   /** \setup
    * Set up zone parameters
    * Set up object status to COASTED
    * Set up object f_moveable to true
    * Set up object f_onguardrail to false
    * Set up object f_track_behind_guardrail to false
    */

   TEST_SETUP()
   {
      object.status = F360_OBJECT_STATUS_COASTED;
      object.f_moveable = true;
      object.on_sep_id = F360_INVALID_UNSIGNED_ID;
      object.behind_sep_id = F360_INVALID_UNSIGNED_ID;
      object.confidenceLevel = 1.0F;
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/** \purpose
 * Purpose of this test is to verify whether when all conditions are met - function returns true
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__All_Conditions_Are_Met)
{
   /** \precond
    * All was set in TEST_SETUP
    */

   /** \action
    * Call tested function
    */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is true
    */
   CHECK_TRUE(f_should_be_considered);
}

/** \purpose
 * Purpose of this test is to verify whether when object status is invalid - function returns false
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__Object_Status_Is_Invalid)
{
   /** \precond
    * All base parameters were set in TEST_SETUP
    * Set object status to invalid
    */
   object.status = F360_OBJECT_STATUS_INVALID;

    /** \action
     * Call tested function
     */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is false
    */
   CHECK_FALSE(f_should_be_considered);
}

/** \purpose
 * Purpose of this test is to verify whether when object status is not moveable - function returns false
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__Object_Is_Not_Moveable)
{
   /** \precond
    * All base parameters were set in TEST_SETUP
    * Set object f_moveable flag to false
    */
   object.f_moveable = false;

   /** \action
    * Call tested function
    */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is false
    */
   CHECK_FALSE(f_should_be_considered);
}

/** \purpose
 * Purpose of this test is to verify whether when object is on guardrail - function returns false
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__Object_Is_On_Guardrail)
{
   /** \precond
    * All base parameters were set in TEST_SETUP
    * Set object on sep id to 1
    */
   object.on_sep_id = 1;

   /** \action
    * Call tested function
    */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is false
    */
   CHECK_FALSE(f_should_be_considered);
}

/** \purpose
 * Purpose of this test is to verify whether when object is behind guardrail - function returns false
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__Object_Is_Behind_Guardrail)
{
   /** \precond
    * All base parameters were set in TEST_SETUP
    * Set object behind sep id to 1
    */
   object.behind_sep_id = 1;

   /** \action
    * Call tested function
    */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is false
    */
   CHECK_FALSE(f_should_be_considered);
}

/** \purpose
 * Purpose of this test is to verify if confidence is lower than threshold
 * \req
 * NA.
 */
TEST(should_object_be_considered, Should_Object_Be_Considered__Below_Confidence_Level_Threshold)
{
   /** \precond
    * All base parameters were set in TEST_SETUP
    * Set object f_track_behind_guardrail flag to true
    */
   object.confidenceLevel = calibrations.k_occlusion_confidence_level - 0.01F;

   /** \action
    * Call tested function
    */
   const bool f_should_be_considered = Is_Object_Relevant_For_Occlusion(calibrations.k_occlusion_confidence_level, object);

   /** \result
    * Check whether f_should_be_considered is false
    */
   CHECK_FALSE(f_should_be_considered);
}
/** @}*/
