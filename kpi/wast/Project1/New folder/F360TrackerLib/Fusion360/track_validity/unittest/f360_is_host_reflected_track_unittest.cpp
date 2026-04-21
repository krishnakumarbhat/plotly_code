/** \file
* This file contains unit tests for content of f360_is_host_reflected_track.cpp
*/

#include "f360_is_host_reflected_track.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

#include "f360_math.h"

using namespace f360_variant_A;
/** \defgroup  f360_is_host_reflected_track
 *  @{
 */

/** \brief
* Test groups contains common data for test cases related to testing of function Is_Host_Reflected_Track()
* Data is set up in such a way that expectation is that object should be flagged as a host mirror.
* Data is tweaked in individual test cases.
**/

TEST_GROUP(f360_is_host_reflected_track)
{
   /** \setup
   * Initialize calibrations
   * Set up data so that object matches a host mirror object with SEP id 1. 
   * Host speed is set to low to generate fixed thresholds equal to calibration parameters
   * Set SEP with id 1 to valid on right hand side of host
   **/

   F360_Object_Track_T object = {};
   F360_Host_T host = {};
   Static_Env_Poly_T sep[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Calibrations_T calibs = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      host.speed = calibs.k_host_refl_lowspeed_host_speed_th - 0.1F;

      object.speed = 0.0F;
      object.reference_point = F360_REFERENCE_POINT_CENTER;
      object.vcs_position.x = -2.0F;
      object.vcs_position.y = 6.0F;
      object.bbox.Set_Center(object.vcs_position);
      object.vcs_heading = Angle{ 0.0F };
      object.hdg_ptng_disagmt = 0.0F;
      object.bbox.Set_Orientation(Angle{object.vcs_heading + object.hdg_ptng_disagmt});
      object.bbox.Set_Length(5.0F);
      object.bbox.Set_Width(2.0F);
      
      object.speed = host.speed;
      object.behind_sep_id = 1U;

      sep[0].lower_limit = -25.0F;
      sep[0].upper_limit = 25.0F;
      sep[0].p0 = 0.5F * object.vcs_position.y;
      sep[0].p1 = 0.0F;
      sep[0].p2 = 0.0F;
      sep[0].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep[0].poly_type = F360_STATIC_ENV_POLY_TYPE_LSC;
   }
};

/**
*\purpose  Purpose of this test is to verify that object is not flagged as a host mirror
*          since object speed doesn't match host speed.
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Object_Speed_Not_Matching_Host)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    * Set object speed to zero
    */
   object.speed = 0.0F;

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should not be marked as mirror
   **/
   CHECK_FALSE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is not flagged as a host mirror
*          since object isn't flagged as behind any SEP
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Object_Not_Flagged_As_Behind_SEP)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    * Set SEP as invalid
    */
   sep[0].status = F360_STATIC_ENV_POLY_STATUS_INVALID;

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should not be marked as mirror
   **/
   CHECK_FALSE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is not flagged as a host mirror
*          since object lateral position is too far away
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Object_Too_Far_In_Lateral_Direction)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    * Set lateral position far away from host
    */
   object.vcs_position.y = 20.0F;
   object.bbox.Set_Center(object.vcs_position);

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should not be marked as mirror
   **/
   CHECK_FALSE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is flagged as a host mirror
*          since all conditions are fulfilled
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Object_Is_Host_Mirror)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    */

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should be marked as mirror
   **/
   CHECK_TRUE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is not flagged as a host mirror
*          because of suspected SEP is crossing host's path.
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__False_reflection)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    */

   sep[0].lower_limit = -7.2f;
   sep[0].upper_limit = -4.28f;
   sep[0].p0 = -11.86f;
   sep[0].p1 = -5.69f;
   sep[0].p2 = -0.52f;

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should be marked as mirror
   **/
   CHECK_FALSE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is not flagged as a host mirror
*          because of suspected SEP is crossing host's path at a great distance.
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Reflection_Far_Ahead_SEP_Crossing_non_ghost)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    */

   sep[0].lower_limit = -17.77f;
   sep[0].upper_limit = 40.47f;
   sep[0].p0 = -5.90f;
   sep[0].p1 = -0.11f;
   sep[0].p2 = 0.001142f;

   object.speed = 4.75F;
   object.reference_point = F360_REFERENCE_POINT_LEFT;
   object.vcs_position.x = 3.05F;
   object.vcs_position.y = -11.52F;
   object.bbox.Set_Center(object.vcs_position);
   object.vcs_heading = Angle{ -2.98F };
   object.hdg_ptng_disagmt = 0.0F;
   object.bbox.Set_Orientation(Angle{object.vcs_heading + object.hdg_ptng_disagmt});
   object.bbox.Set_Length(3.0F);
   object.bbox.Set_Width(1.0F);

   host.speed = 6.08F;

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should be marked as mirror
   **/
   CHECK_FALSE(f_host_mirror);
}

/**
*\purpose  Purpose of this test is to verify that object is flagged as a host mirror
*          because of suspected SEP has a local extreme value (minimum or maximum)
*          far away from host.
*\req    NA
*/
TEST(f360_is_host_reflected_track, Is_Host_Reflected_Track__Reflection_Far_Ahead_SEP_Crossing_ghost)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading and pointing set to 0 along with cos/sin
    * - Object position is adjacent to host behind SEP id 1
    * - Object is flagged as behind SEP id 1
    * - SEP with id 1 is valid adjacent to host on right side
    */

   sep[0].lower_limit = -24.49F;
   sep[0].upper_limit = 43.89F;
   sep[0].p0 = 7.60F;
   sep[0].p1 = -0.0484F;
   sep[0].p2 = 0.0006845F;

   object.speed = 5.4F;
   object.reference_point = F360_REFERENCE_POINT_LEFT;
   object.vcs_position.x = -2.12F;
   object.vcs_position.y = 14.53F;
   object.bbox.Set_Center(object.vcs_position);
   object.vcs_heading = Angle{ -0.072F };
   object.hdg_ptng_disagmt = -0.006F;
   object.bbox.Set_Orientation(Angle{object.vcs_heading + object.hdg_ptng_disagmt});
   object.bbox.Set_Length(5.62F);
   object.bbox.Set_Width(1.89F);

   host.speed = 6.08F;

   /** \action
   * Call Is_Host_Reflected_Track
   **/
   bool f_host_mirror = Is_Host_Reflected_Track(
      object,
      host,
      sep,
      calibs);

   /** \result
   * Track should be marked as mirror
   **/
   CHECK_TRUE(f_host_mirror);
}
/** @}*/
