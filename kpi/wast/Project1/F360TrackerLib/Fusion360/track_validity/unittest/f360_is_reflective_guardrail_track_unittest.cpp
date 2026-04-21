/** \file
 * This file contains unit tests for content of f360_is_reflective_guardrail_track.cpp file
 */

#include "f360_is_reflective_guardrail_track.h"
#include "f360_static_env_polys_support_functions.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_is_reflective_guardrail_track
 *  @{
 */

 /** \brief
  * Test group of Is_Reflective_Guardrail_Track() function. Tests verify whether
  * objects are properly analysed whether they are reflected from guardrail
  */
TEST_GROUP(f360_is_reflective_guardrail_track)
{

   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info = {};
   int32_t ghost_candidate_idx = 0;
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Calibrations_T calib = {};

   F360_Object_Track_T& ghost_obj = object_tracks[0];
   F360_Object_Track_T& source_obj = object_tracks[1];

   /** \setup
    * Initialize tracker calibrations
    * Setp up two SEP's
    * Set up source candidate object so that it is a valid source object 
    * that can yield a reflection in SEP id 1.
    * Set up ghost candidate so that it is a valid reflection of the
    * source object in SEP id 1.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      float32_t host_curvature_rear = 0.000001F;

      tracker_info.num_active_objs = 2;
      tracker_info.active_obj_ids[0] = 1;
      tracker_info.active_obj_ids[1] = 2;

      sep[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep[1].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep[0].p0 = -5.0F;
      sep[1].p0 = 5.0F;
      sep[0].p2 = 0.5F * host_curvature_rear;
      sep[1].p2 = 0.5F * host_curvature_rear;
      sep[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
      sep[1].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
      sep[0].lower_limit = -100.0F;
      sep[1].lower_limit = -100.0F;
      sep[0].upper_limit = 100.0F;
      sep[1].upper_limit = 100.0F;

      ghost_obj.vcs_position.y = -7.5F;
      ghost_obj.vcs_position.x = 10.0F;
      ghost_obj.Update_Bbox_Size(1.0F, 1.0F);
      ghost_obj.Set_Bbox_Orientation(Angle{ 0.0F });
      ghost_obj.f_moving = true;
      ghost_obj.f_moveable = true;
      ghost_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      ghost_obj.status = F360_OBJECT_STATUS_UPDATED;
      ghost_obj.reduced_id = 1;
      ghost_obj.speed = 1.0F;

      source_obj.vcs_position.y = -2.5F;
      source_obj.vcs_position.x = 10.0F;
      source_obj.Update_Bbox_Size(1.0F, 1.0F);
      source_obj.Set_Bbox_Orientation(Angle{ 0.0F });
      source_obj.f_moving = true;
      source_obj.f_moveable = true;
      source_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      source_obj.status = F360_OBJECT_STATUS_UPDATED;
      source_obj.reduced_id = 2;
      source_obj.speed = 1.0F;

      Flag_Single_Object_Behind_SEP(sep, calib, ghost_obj);
      Flag_Single_Object_Behind_SEP(sep, calib, source_obj);
   }
};

/** \purpose
 * Purpose of this test is to verify that object is flagged as a
 * SEP mirror when all conditions are fulfilled
 * \req
 * NA.
 */
TEST(f360_is_reflective_guardrail_track, Is_Reflective_Guardrail_Track__Object_Is_Mirror)
{
   /** \precond
    * In test group data have been set up so that all conditions are fulfilled
    */

    /** \action
     * Call tested function
     */
   bool f_reflection = Is_Reflective_Guardrail_Track(object_tracks, tracker_info, ghost_candidate_idx, sep, calib);

   /** \result
    * Check whether returned value is true
    */
   CHECK_TRUE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that object is not flagged as a
 * SEP mirror when all conditions are fulfilled except that the ghost
 * object is not moving
 * \req
 * NA.
 */
TEST(f360_is_reflective_guardrail_track, Is_Reflective_Guardrail_Track__Object_Is_Not_Candidate_For_Check)
{
   /** \precond
    * In test group data have been set up so that all conditions are fulfilled
    * Set object moving flag to false
    */
   ghost_obj.f_moving = false;

    /** \action
     * Call tested function
     */
   bool f_reflection = Is_Reflective_Guardrail_Track(object_tracks, tracker_info, ghost_candidate_idx, sep, calib);

   /** \result
    * Check whether returned value is true
    */
   CHECK_FALSE(f_reflection);
}
/** @}*/

/** \defgroup  f360_Is_Hypot_Source_Pos_In_Source_Candidate_BBox
 *  @{
 */

 /** \brief
  * Test group related to tests of function f360_Is_Hypot_Source_Pos_In_Source_Candidate_BBox() function.
  * Data is set up in such a way that function should return true.
  */
TEST_GROUP(f360_Is_Hypot_Source_Pos_In_Source_Candidate_BBox)
{
   Point hypothetic_source_pos = {};
   F360_Object_Track_T source_candidate = { };
   F360_Object_Track_T ghost_candidate = { };
   F360_Calibrations_T calibs = { };

   /** \setup
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      hypothetic_source_pos.x = 0.0F;
      hypothetic_source_pos.y = 0.0F;

      ghost_candidate.vcs_position.y = 5.0;
      ghost_candidate.vcs_position.x = 0.0F;
      ghost_candidate.Set_Bbox_Orientation(Angle{ 0.0F });
      ghost_candidate.Update_Bbox_Size(1.0F, 1.0F);

      source_candidate.vcs_position.y = 0.0;
      source_candidate.vcs_position.x = 0.0F;
      source_candidate.Set_Bbox_Orientation(Angle{ 0.0F });
      source_candidate.Update_Bbox_Size(1.0F, 1.0F);
   }
};

/** \purpose
 * Purpose of this test is to verify that function returns true when hypothetic source position
 * matches source candidate position.
 * \req
 * NA.
 */
TEST(f360_Is_Hypot_Source_Pos_In_Source_Candidate_BBox, Is_Hypot_Source_Pos_In_Source_Candidate_BBox__Hypothetic_Source_Pos_Matches_Source_Candidate)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    */

    /** \action
     * Call function Is_Hypot_Source_Pos_In_Source_Candidate_BBox()
     */
   const bool f_matching = Is_Hypot_Source_Pos_In_Source_Candidate_BBox(
      hypothetic_source_pos,
      source_candidate,
      ghost_candidate,
      calibs);

   /** \result
    * Check whether returned value is true
    */
   CHECK_TRUE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns true when hypothetic source position
 * not matches source candidate position.
 * \req
 * NA.
 */
TEST(f360_Is_Hypot_Source_Pos_In_Source_Candidate_BBox, Is_Hypot_Source_Pos_In_Source_Candidate_BBox__Hypothetic_Source_Pos_Not_Matches_Source_Candidate)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set hypothetic source position far away from source candidate position
    */
   hypothetic_source_pos.x = 10.0F;
   hypothetic_source_pos.y = 10.0F;

    /** \action
     * Call function Is_Hypot_Source_Pos_In_Source_Candidate_BBox()
     */
   const bool f_matching = Is_Hypot_Source_Pos_In_Source_Candidate_BBox(
      hypothetic_source_pos,
      source_candidate,
      ghost_candidate,
      calibs);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_matching);
}
/** @}*/

/** \defgroup  f360_Is_Source_Candidate_Similar_To_Ghost_Candidate
 *  @{
 */

 /** \brief
  * Test group related to tests of function Is_Source_Candidate_Similar_To_Ghost_Candidate() function.
  * Data is set up in such a way that function should return true.
  */
TEST_GROUP(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate)
{
   Point hypothetic_source_pos = {};
   F360_Object_Track_T source_candidate = { };
   F360_Object_Track_T ghost_candidate = { };
   F360_Calibrations_T calibs = { };

   /** \setup
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      hypothetic_source_pos.x = 0.0F;
      hypothetic_source_pos.y = 0.0F;

      ghost_candidate.vcs_position.y = 5.0F;
      ghost_candidate.vcs_position.x = 0.0F;
      ghost_candidate.Set_Bbox_Orientation(Angle{ 0.0F });
      ghost_candidate.speed = 10.0F;
      ghost_candidate.vcs_heading = Angle{ 0.0F };
      ghost_candidate.Update_Bbox_Size(1.0F, 1.0F);

      source_candidate.vcs_position.y = 0.0;
      source_candidate.vcs_position.x = 0.0F;
      source_candidate.Set_Bbox_Orientation(Angle{ 0.0F });
      source_candidate.speed = 10.0F;
      source_candidate.vcs_heading = Angle{ 0.0F };
      source_candidate.Update_Bbox_Size(1.0F, 1.0F);
   }
};

/** \purpose
 * Purpose of this test is to verify that function returns true when hypothetic source position
 * matches source candidate position.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Matching)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    */

    /** \action
     * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
     */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is true
    */
   CHECK_TRUE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when hypothetic source position
 * matches source candidate position but source object speed is too high.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Speed_Too_High)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    * 
    * Set source object speed different
    */
   source_candidate.speed = 100.0F;

    /** \action
     * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
     */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is true
    */
   CHECK_FALSE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when hypothetic source position
 * matches source candidate position but source object speed is too low
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Speed_Too_Low)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    *
    * Set source object speed low
    */
   source_candidate.speed = 0.0F;

   /** \action
    * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
    */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is true
    */
   CHECK_FALSE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns true when hypothetic source position
 * matches source candidate position. Heading of source object is large so a different threshold for
 * position is used.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Heading_Large)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    *
    * Set source object heading large
    * Set ghost object heading large
    */
   source_candidate.vcs_heading = Angle{ F360_DEG2RAD(90.0F) };
   ghost_candidate.vcs_heading = Angle{ F360_DEG2RAD(90.0F) };

   /** \action
    * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
    */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is true
    */
   CHECK_TRUE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when hypothetic source position
 * not matches source candidate position in lateral direction.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Lat_Pos_Not_Matching)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    *
    * Set source object lateral position different
    */
   source_candidate.vcs_position.y = -10.0F;
   source_candidate.Update_Bbox_Center();

   /** \action
    * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
    */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when hypothetic source position
 * not matches source candidate in heading.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Heading_Not_Matching)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    *
    * Set source object heading different
    */
   source_candidate.vcs_heading = Angle{ F360_DEG2RAD(90.0F) };

   /** \action
    * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
    */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_matching);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when hypothetic source position
 * not matches source candidate in longitudinal position.
 * \req
 * NA.
 */
TEST(f360_Is_Source_Candidate_Similar_To_Ghost_Candidate, Is_Source_Candidate_Similar_To_Ghost_Candidate__Source_And_Ghost_Not_Matching_Due_To_Source_Long_Pos_Not_Matching)
{
   /** \precond
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    * Set up valid speed and vcs heading for both source and ghost object
    *
    * Set source object long position different
    */
   source_candidate.vcs_position.x = 50.0F;
   source_candidate.Update_Bbox_Center();

   /** \action
    * Call function Is_Hypot_Source_Pos_In_Sourc_Candidate_BBox()
    */
   const bool f_matching = Is_Source_Candidate_Similar_To_Ghost_Candidate(
      source_candidate,
      ghost_candidate,
      hypothetic_source_pos,
      calibs);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_matching);
}
/** @}*/

/** \defgroup  f360_Calculate_Center_Of_Symmetry_LSC
 *  @{
 */

 /** \brief
  * Test group related to tests of function Calculate_Center_Of_Symmetry_LSC() function.
  * Data is set up in such a way that function should return true.
  */
TEST_GROUP(f360_Calculate_Center_Of_Symmetry_LSC)
{
   Point ghost_cand_pos = {};
   Point point_of_reflection = {};
   Static_Env_Poly_T sep = {};

   float32_t test_pass_thres = 0.0001F;

   /** \setup
    * Initialize tracker calibrations
    * Set up hypothetic source position equal to source candidate object position
    * Set up arbitrary valid dimension and orientation of both source and ghost object
    */
   TEST_SETUP()
   {
      ghost_cand_pos.x = 0.0F;
      ghost_cand_pos.y = 4.0F;

      point_of_reflection.x = 0.0F;
      point_of_reflection.y = 2.0F;

      sep.status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep.p0 = 2.0F;
      sep.p1 = 0.0F;
      sep.p2 = 0.0F;
      sep.lower_limit = -20.0F;
      sep.upper_limit = 20.0F;
   }
};

/** \purpose
 * Purpose of this test is to verify that function returns correct center of symmetry
 * when SEP slope is zero
 * \req
 * NA.
 */
TEST(f360_Calculate_Center_Of_Symmetry_LSC, Calculate_Center_Of_Symmetry_LSC__Tangent_Slope_Zero)
{
   /** \precond
    * Ghost candidate VCS position is (0,4)
    * Point of reflection VCS position is (0,2)
    * SEP is a straight line at lateral VCS position 2
    */

   /** \action
   * Call function Calculate_Center_Of_Symmetry_LSC()
   */
   Point vcs_pos = Calculate_Center_Of_Symmetry_SEP(
      ghost_cand_pos,
      point_of_reflection,
      sep);

   /** \result
    * Check that center of symmetry matches expectation
    */
   DOUBLES_EQUAL(0.0F, vcs_pos.x, test_pass_thres);
   DOUBLES_EQUAL(2.0F, vcs_pos.y, test_pass_thres);
}

/** \purpose
 * Purpose of this test is to verify that function returns correct center of symmetry
 * when SEP slope is not zero
 * \req
 * NA.
 */
TEST(f360_Calculate_Center_Of_Symmetry_LSC, Calculate_Center_Of_Symmetry_LSC__Tangent_Slope_Not_Zero)
{
   /** \precond
    * Ghost candidate VCS position is (10,4)
    * Point of reflection VCS position is (8,2)
    * SEP is a curved line
    */
   ghost_cand_pos.x = 10.0F;
   ghost_cand_pos.y = 4.0F;

   point_of_reflection.x = 8.0F;
   point_of_reflection.y = 2.0F;

   sep.status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   sep.p0 = 3.0F;
   sep.p1 = 0.0F;
   sep.p2 = 0.5F;
   sep.lower_limit = -20.0F;
   sep.upper_limit = 20.0F;

    /** \action
    * Call function Calculate_Center_Of_Symmetry_LSC()
    */
   Point vcs_pos = Calculate_Center_Of_Symmetry_SEP(
      ghost_cand_pos,
      point_of_reflection,
      sep);

   /** \result
    * Check that center of symmetry matches expectation
    */
   DOUBLES_EQUAL(4.21538448F, vcs_pos.x, test_pass_thres);
   DOUBLES_EQUAL(4.72307682F, vcs_pos.y, test_pass_thres);
}
/** @}*/

/** \defgroup  f360_Is_Ghost_Reflected_By_SEP
 *  @{
 */

 /** \brief
  * Test group of Is_Ghost_Reflected_By_SEP() function. 
  * Data is set up in such a way that the ghost candidate should be
  * flagged a reflection in the SEP
  */
TEST_GROUP(f360_Is_Ghost_Reflected_By_SEP)
{

   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info = {};
   int32_t ghost_candidate_idx = 0;
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Calibrations_T calib = {};

   F360_Object_Track_T& ghost_obj = object_tracks[0];
   F360_Object_Track_T& source_obj = object_tracks[1];

   Point ghost_cand_pos = {};


   /** \setup
    * Initialize tracker calibrations
    * Setp up two SEP's
    * Set up base object properties
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      float32_t host_curvature_rear = 0.000001F;

      tracker_info.num_active_objs = 2;
      tracker_info.active_obj_ids[0] = 1;
      tracker_info.active_obj_ids[1] = 2;

      sep[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep[1].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep[0].p0 = -5.0F;
      sep[1].p0 = 5.0F;
      sep[0].p2 = 0.5F * host_curvature_rear;
      sep[1].p2 = 0.5F * host_curvature_rear;
      sep[0].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
      sep[1].poly_type = F360_STATIC_ENV_POLY_TYPE_CURVG;
      sep[0].lower_limit = -100.0F;
      sep[1].lower_limit = -100.0F;
      sep[0].upper_limit = 100.0F;
      sep[1].upper_limit = 100.0F;

      ghost_obj.vcs_position.y = -7.5F;
      ghost_obj.vcs_position.x = 10.0F;
      ghost_obj.Set_Bbox_Orientation(Angle{ 0.0F });
      ghost_obj.f_moving = true;
      ghost_obj.f_moveable = true;
      ghost_obj.Update_Bbox_Size(1.0F, 1.0F);
      ghost_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      ghost_obj.status = F360_OBJECT_STATUS_UPDATED;
      ghost_obj.reduced_id = 1;
      ghost_obj.speed = 1.0F;

      ghost_cand_pos = ghost_obj.bbox.Get_Center();

      source_obj.vcs_position.y = -2.5F;
      source_obj.vcs_position.x = 10.0F;
      source_obj.Set_Bbox_Orientation(Angle{ 0.0F });
      source_obj.f_moving = true;
      source_obj.f_moveable = true;
      source_obj.Update_Bbox_Size(1.0F, 1.0F);
      source_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      source_obj.status = F360_OBJECT_STATUS_UPDATED;
      source_obj.reduced_id = 2;
      source_obj.speed = 1.0F;

      Flag_Single_Object_Behind_SEP(sep, calib, ghost_obj);
      Flag_Single_Object_Behind_SEP(sep, calib, source_obj);
   }
};

/** \purpose
 * Purpose of this test is to verify that ghost object is flagged as a
 * SEP mirror given that all conditions are fulfilled as is set up in the test group
 * (i.e. source cantidate is a CTCA object)
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_SEP_Mirror__CTCA_Souce_Canditiate)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   */

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is flagged as a
 * SEP mirror given that all conditions are fulfilled as is set up in the test group
 * but when the filter type and object speed of source canditiate are set to be fast moving CCA.
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_SEP_Mirror__Fast_Moving_CCA_Souce_Canditiate)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror.
   * Change filter type of source object to CCA
   * Change speed of source canditiate to be just above calib.fast_moving_thres.
   * Change speed of ghost candiotate to match that of source candidate
   */
  source_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
  source_obj.speed = calib.fast_moving_thresh + 1e-3F;
  ghost_obj.speed = source_obj.speed;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_TRUE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is of slow moving CCA type
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_Slow_Moving_CCA)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * * Change speed of ghost candiotate to match that of source candidate
   */
  source_obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
  source_obj.speed = calib.fast_moving_thresh - 1e-3F;
  ghost_obj.speed = source_obj.speed;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is flagged as on SEP
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_On_SEP)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object to be flagged as on SEP
   */
   source_obj.on_sep_id = 1U;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is not moving (f_moving flag is false)
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_Not_Moving)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object to be flagged as not moving
   */
   source_obj.f_moving = false;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is newly created
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_New)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object status to new
   */
   source_obj.status = F360_OBJECT_STATUS_NEW_UPDATED;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is not on reduced object list
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_Not_Reduced)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object status to new
   */
   source_obj.reduced_id = 0U;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source candidate is behind an SEP
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_Is_Behind_SEP)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object behind SEP id 1
   */
   source_obj.behind_sep_id = 1U;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since source and ghost candidate does not match
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Source_And_Ghost_Not_Match)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set source object lateral position for away
   */
   source_obj.vcs_position.y = 100.0F;
   source_obj.Update_Bbox_Center();

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since ghost candidate intersection with SEP is below valid
 * calibration interval to activate countermeasure
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Ghost_SEP_Intersection_Too_Low)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set ghost object longitudinal SEP intersection below valid calibration
   */
   ghost_obj.sep_intersection_point.x = calib.k_tv_refl_gr_trk_min_sep_lon_pos - 1.0F;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}

/** \purpose
 * Purpose of this test is to verify that ghost object is not flagged as a
 * SEP mirror since ghost candidate intersection with SEP is above valid
 * calibration interval to activate countermeasure
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Reflected_By_SEP, Is_Ghost_Reflected_By_SEP__Candidate_Is_Not_SEP_Mirror_Due_To_Ghost_SEP_Intersection_Too_High)
{
   /** \precond
   * Data have been set up in test group so
   * that ghost object is expected to be flagged
   * as a SEP mirror
   * Set ghost object longitudinal SEP intersection above valid calibration
   */
   ghost_obj.sep_intersection_point.x = calib.k_tv_refl_gr_trk_max_sep_lon_pos + 1.0F;

   /** \action
   * Call tested function
   */
   bool f_reflection = Is_Ghost_Reflected_By_SEP(
      sep[0],
      ghost_obj,
      ghost_cand_pos,
      tracker_info,
      object_tracks,
      calib);

   /** \result
   * Check whether returned value is true
   */
   CHECK_FALSE(f_reflection);
}
/** @}*/

/** \defgroup  f360_Is_Ghost_Obj_Valid_For_Check
 *  @{
 */

 /** \brief
  * Test group of Is_Ghost_Obj_Valid_For_Check() function. 
  * Data is set up in such a way that the object is valid
  * for a SEP mirror check
  */
TEST_GROUP(f360_Is_Ghost_Obj_Valid_For_Check)
{
   F360_Calibrations_T calib;
   F360_Object_Track_T ghost_candidate = {};

   /** \setup
    * Set up object so that it is a valid candidate for SEP 
    * reflection check
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      ghost_candidate.behind_sep_id = 1U;
      ghost_candidate.f_moving = true;
   }
};

/** \purpose
 * Purpose of this test is to verify that function returns true when object is valid for a CTCA object
 * for a SEP mirror check
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Obj_Valid_For_Check, Is_Ghost_Obj_Valid_For_Check__Object_Is_Valid_For_Check_CTCA)
{
   /** \precond
    * In test group the following have been set up
    *  ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
    *  ghost_candidate.behind_sep_id = 1U;
    *  ghost_candidate.f_moving = true;
    */

    /** \action
     * Call tested function
     */
     bool f_valid_for_check = Is_Ghost_Obj_Valid_For_Check(ghost_candidate, calib);

     /** \result
      * Check whether returned value is true
      */
      CHECK_TRUE(f_valid_for_check);
}

/** \purpose
 * Purpose of this test is to verify that function returns true when object is valid for a fast moving CCA object
 * for a SEP mirror check
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Obj_Valid_For_Check, Is_Ghost_Obj_Valid_For_Check__Object_Is_Valid_For_Check_Fast_Moving_CCA)
{
   /** \precond
    * In test group the following have been set up
    *  ghost_candidate.behind_sep_id = 1U;
    *  ghost_candidate.f_moving = true;
    * Change object filter type to CCA and set speed to slightly larger than calib.fast_moving_thres
    */
   ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   ghost_candidate.speed = calib.fast_moving_thresh + 1e-3F;

    /** \action
     * Call tested function
     */
     bool f_valid_for_check = Is_Ghost_Obj_Valid_For_Check(ghost_candidate, calib);

     /** \result
      * Check whether returned value is true
      */
      CHECK_TRUE(f_valid_for_check);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when object is not valid
 * for a SEP mirror check due to filter type is CCA and object is slow moving
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Obj_Valid_For_Check, Is_Ghost_Obj_Valid_For_Check__Object_Is_Not_Valid_For_Check_Due_To_Slow_Moving_CCA)
{
   /** \precond
    * In test group the following have been set up
    *  ghost_candidate.behind_sep_id = 1U;
    *  ghost_candidate.f_moving = true;
    * Change object filter type to CCA and set speed to slightly blow than calib.fast_moving_thres
    */
   ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   ghost_candidate.speed = calib.fast_moving_thresh - 1e-3F;

    /** \action
     * Call tested function
     */
   bool f_valid_for_check = Is_Ghost_Obj_Valid_For_Check(ghost_candidate, calib);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_valid_for_check);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when object is not valid
 * for a SEP mirror check due to object is not behind any SEP
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Obj_Valid_For_Check, Is_Ghost_Obj_Valid_For_Check__Object_Is_Not_Valid_For_Check_Due_Not_Behind_SEP)
{
   /** \precond
    * In test group the following have been set up
    *  ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
    *  ghost_candidate.behind_sep_id = 1U;
    *  ghost_candidate.f_moving = true;
    *
    * Set object not behind SEP
    */
   ghost_candidate.behind_sep_id = F360_INVALID_UNSIGNED_ID;

   /** \action
    * Call tested function
    */
   bool f_valid_for_check = Is_Ghost_Obj_Valid_For_Check(ghost_candidate, calib);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_valid_for_check);
}

/** \purpose
 * Purpose of this test is to verify that function returns false when object is not valid
 * for a SEP mirror check due to object is not moving
 * \req
 * NA.
 */
TEST(f360_Is_Ghost_Obj_Valid_For_Check, Is_Ghost_Obj_Valid_For_Check__Object_Is_Not_Valid_For_Check_Due_Not_Moving)
{
   /** \precond
    * In test group the following have been set up
    *  ghost_candidate.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
    *  ghost_candidate.behind_sep_id = 1U;
    *  ghost_candidate.f_moving = true;
    *
    * Set object not moving
    */
   ghost_candidate.f_moving = false;

   /** \action
    * Call tested function
    */
   bool f_valid_for_check = Is_Ghost_Obj_Valid_For_Check(ghost_candidate, calib);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_valid_for_check);
}
/** @}*/
