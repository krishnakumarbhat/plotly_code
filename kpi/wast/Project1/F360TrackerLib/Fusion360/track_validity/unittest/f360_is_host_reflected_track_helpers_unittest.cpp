/** \file
 * This file contains unit tests for content of f360_is_host_reflected_track_helpers.cpp file
 */

#include "f360_is_host_reflected_track_helpers.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  f360_Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
 *  @{
 */

/** \brief
 * Test group containing common data for tests related to function Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox()
 */
TEST_GROUP(f360_Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox)
{
   F360_Object_Track_T object = {};
   Point host_mirror_track_tcs_pos = {};
   F360_Calibrations_T calibs = {};

   /** \setup
    * Initialize tracker calibrations
    * Set up an object with posiiton in VCS origin
    * Set up arbitrary valid dimensions of object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      object.reference_point = F360_REFERENCE_POINT_CENTER;
      object.vcs_position.x = 0.0F;
      object.vcs_position.y = 0.0F;
      object.bbox.Set_Center(object.vcs_position);
      object.vcs_heading = Angle{ 0.0F };
      object.hdg_ptng_disagmt = 0.0F;
      object.bbox.Set_Orientation(object.vcs_heading + object.hdg_ptng_disagmt);
      object.bbox.Set_Length(2.0F);
      object.bbox.Set_Width(2.0F);
   }
};

/** \purpose  
 * Verify that function returns true if host ghost predicted position is inside 
 * zone to trigger countermeasure.
 * \req
 * NA
 */
TEST(f360_Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox, Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox__Ghost_Position_Inside_Zone)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Set object properties, 
    *   - Position in VCS origin
    *   - VCS pointing set to 0, along with updated cos/sin values of pointing
    *   - All size dimensions set to 1
    * Set host_mirror_track_vcs_pos to be inside zone
    */
   host_mirror_track_tcs_pos.x = 0.0F;
   host_mirror_track_tcs_pos.y = 0.0F;

   /** \action
    * Call Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
    */
   bool f_inside_susp_zone = Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox(
      object,
      host_mirror_track_tcs_pos,
      calibs);

   /** \result
    * Verify that function have returned true
    */
   CHECK_TRUE(f_inside_susp_zone);
}

/** \purpose
 * Verify that function returns true if host ghost predicted position is outside 
 * zone to trigger countermeasure. Host ghost predicted position is "above" the object.
 * \req
 * NA
 */
TEST(f360_Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox, Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox__Ghost_Position_Outside_Zone_Above)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Set object properties,
    *   - Position in VCS origin
    *   - VCS pointing set to 0, along with updated cos/sin values of pointing
    *   - All size dimensions set to 1
    * Set host_mirror_track_vcs_pos to be outside zone (greater than len2 + calibration)
    */
   host_mirror_track_tcs_pos.x = 0.5F * object.bbox.Get_Length() + calibs.k_host_refl_bbox_long_ext + 0.1F;
   host_mirror_track_tcs_pos.y = 0.0F;

   /** \action
    * Call Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
    */
   bool f_inside_susp_zone = Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox(
      object,
      host_mirror_track_tcs_pos,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_inside_susp_zone);
}
/** @}*/

/** \defgroup  f360_Determine_Heading_And_Speed_Threshold
 *  @{
 */

 /** \brief
  * Test group containing common data for tests related to function Determine_Heading_And_Speed_Threshold()
  */
TEST_GROUP(f360_Determine_Heading_And_Speed_Threshold)
{
   float32_t host_speed;
   F360_Calibrations_T calibs;
   float32_t max_heading;
   float32_t max_speed_diff;

   float32_t test_pass_thres = 0.0001F;

   /** \setup
    * Initialize tracker calibrations
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
   }
};

/** \purpose
 * Verify that function returns correct thresholds when host speed is low
 * \req
 * NA
 */
TEST(f360_Determine_Heading_And_Speed_Threshold, Determine_Heading_And_Speed_Threshold__Low_Host_Speed)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * Set host speed below calibration
    */
   host_speed = calibs.k_host_refl_lowspeed_host_speed_th - 0.1F;

   /** \action
    * Call Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
    */
   Determine_Heading_And_Speed_Threshold(
      host_speed,
      calibs,
      max_heading,
      max_speed_diff);

   /** \result
    * Verify that function have returned correct thresholds
    */
   DOUBLES_EQUAL(calibs.k_host_refl_lowspeed_speed_diff_th, max_speed_diff, test_pass_thres);
   DOUBLES_EQUAL(calibs.k_host_refl_lowspeed_heading_th, max_heading, test_pass_thres);
   
}

/** \purpose
 * Verify that function returns correct thresholds when host speed is above calibration value
 * \req
 * NA
 */
TEST(f360_Determine_Heading_And_Speed_Threshold, Determine_Heading_And_Speed_Threshold__Host_Speed_Above_Cal)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * Set host speed above calibration
    */
   host_speed = calibs.k_host_refl_lowspeed_host_speed_th + 0.1F;

   /** \action
    * Call Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
    */
   Determine_Heading_And_Speed_Threshold(
      host_speed,
      calibs,
      max_heading,
      max_speed_diff);

   /** \result
    * Verify that function have returned correct thresholds
    */
   DOUBLES_EQUAL(3.44F, max_speed_diff, test_pass_thres);
   DOUBLES_EQUAL(calibs.k_host_refl_highspeed_heading_th, max_heading, test_pass_thres);
}

/** \purpose
 * Verify that function returns correct thresholds when host speed is by far above calibration value
 * and speed max_speed_diff should be saturated
 * \req
 * NA
 */
TEST(f360_Determine_Heading_And_Speed_Threshold, Determine_Heading_And_Speed_Threshold__Host_Speed_Above_Cal_Saturated_Threshold)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * Set host speed very high
    */
   host_speed = 150.0F;

   /** \action
    * Call Is_Predicted_Ghost_Position_In_Suspected_Object_Extended_Bbox
    */
   Determine_Heading_And_Speed_Threshold(
      host_speed,
      calibs,
      max_heading,
      max_speed_diff);

   /** \result
    * Verify that function have returned correct thresholds
    */
   DOUBLES_EQUAL(calibs.k_host_refl_highspeed_max_speed_diff_th, max_speed_diff, test_pass_thres);
   DOUBLES_EQUAL(calibs.k_host_refl_highspeed_heading_th, max_heading, test_pass_thres);
}

/** @}*/

/** \defgroup  f360_Is_Object_Suspected_Of_Being_Host_Reflection
 *  @{
 */

 /** \brief
  * Test group containing common data for tests related to function Is_Object_Suspected_Of_Being_Host_Reflection()
  */
TEST_GROUP(f360_Is_Object_Suspected_Of_Being_Host_Reflection)
{
   F360_Object_Track_T object;
   float32_t host_speed;
   F360_Calibrations_T calibs;

   /** \setup
    * Initialize tracker calibrations
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      host_speed = calibs.k_host_refl_lowspeed_host_speed_th - 0.1F;

      object.vcs_heading = Angle{ 0.0F };
      object.vcs_position.x = calibs.k_host_refl_min_obj_long_pos - 1.0F;
      object.speed = 0.0F;
   }
};

/** \purpose
 * Verify that function returns correct false when object is too far to the rear in longitudinal position
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__Object_Too_Far_To_The_Rear)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading set to 0
    * - Object position is outside suspected zone (too far to the rear in longitudinal position)
    * - Object speed set to 0
    */

   /** \action
    * Call Is_Object_Suspected_Of_Being_Host_Reflection
    */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_suspected);
}

/** \purpose
 * Verify that function returns correct false when object is too far to the front in longitudinal position
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__Object_Too_Far_To_The_Front)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object heading set to 0
    * - Object speed set to 0
    * Set object position above calibration for max longitudinal position
    */
   object.vcs_position.x = calibs.k_host_refl_max_obj_long_pos + 1.0F;

    /** \action
     * Call Is_Object_Suspected_Of_Being_Host_Reflection
     */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_suspected);
}

/** \purpose
 * Verify that function returns correct false when object heading is too large
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__Object_Heading_Too_Large)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object speed set to 0
    * - Object position is outside suspected zone (too far to the rear in longitudinal position)
    * Set object heading large so it's larger than dynamic threshold
    */
   object.vcs_heading = Angle{ F360_DEG2RAD(90.0F) };

   /** \action
   * 
    * Call Is_Object_Suspected_Of_Being_Host_Reflection
    */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_suspected);
}

/** \purpose
 * Verify that function returns correct false when object position is outside valid zone
 * but speed condition is fulfilled
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__Object_Speed_Same_As_Host)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object speed set to 0
    * - Object position is outside suspected zone (too far to the rear in longitudinal position)
    * Set object speed equal to host speed
    */
   object.speed = host_speed;

   /** \action
   *
    * Call Is_Object_Suspected_Of_Being_Host_Reflection
    */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_suspected);
}

/** \purpose
 * Verify that function returns correct false when object position is inside valid zone
 * but object speed is too far off from host speed
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__All_Conditions_Fulfilled_Except_Object_Speed)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object speed set to 0
    * - Object position is outside suspected zone (too far to the rear in longitudinal position)
    * Set object speed equal to host speed
    */
   object.vcs_position.x = 0.0F;

   /** \action
   *
    * Call Is_Object_Suspected_Of_Being_Host_Reflection
    */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_suspected);
}

/** \purpose
 * Verify that function returns correct true when all conditions are met for the object to be a
 * host mirror suspect.
 * \req
 * NA
 */
TEST(f360_Is_Object_Suspected_Of_Being_Host_Reflection, Is_Object_Suspected_Of_Being_Host_Reflection__Object_Is_Suspect)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - Object speed set to 0
    * Set object position adjacent to host
    * Set object speed equal to host speed
    */
   object.vcs_position.x = 0.0F;
   object.speed = host_speed;

   /** \action
   *
    * Call Is_Object_Suspected_Of_Being_Host_Reflection
    */
   bool f_suspected = Is_Object_Suspected_Of_Being_Host_Reflection(
      object,
      host_speed,
      calibs);

   /** \result
    * Verify that function have returned true
    */
   CHECK_TRUE(f_suspected);
}
/** @}*/


/** \defgroup  f360_Calc_Predicted_Reflected_Track_TCS_Position
 *  @{
 */

 /** \brief
  * Test group containing common data for tests related to function Calc_Predicted_Reflected_Track_TCS_Position()
  */
TEST_GROUP(f360_Calc_Predicted_Reflected_Track_TCS_Position)
{
   F360_Object_Track_T object;
   float32_t sep_lat_pos = 3.0F;
   float32_t half_host_length = 2.0F;

   float32_t test_pass_thres = 0.0001F;

   /** \setup
    * Initialize tracker calibrations
    */
   TEST_SETUP()
   {
      object.reference_point = F360_REFERENCE_POINT_CENTER;
      object.vcs_position.x = -2.0F;
      object.vcs_position.y = 4.0F;
      object.bbox.Set_Center(object.vcs_position);
      object.vcs_heading = Angle(0.0F);
      object.hdg_ptng_disagmt = 0.0F;
      object.bbox.Set_Orientation(object.vcs_heading + object.hdg_ptng_disagmt);
      object.bbox.Set_Length(5.0F);
      object.bbox.Set_Width(2.0F);
   }
};

/** \purpose
 * Verify that function returns correct predicted VCS position for an arbitrary object adjacent to host
 * \req
 * NA
 */
TEST(f360_Calc_Predicted_Reflected_Track_TCS_Position, Calc_Predicted_Reflected_Track_TCS_Position__Predicted_Position)
{
   /** \precond
    * In test group the following have been set up
    * - Object pointing set to 0, along with cos/sin
    * - Object position at VCS (-2, 4)
    */

    /** \action
     * Call Calc_Predicted_Reflected_Track_TCS_Position
     */
   Point tcs_pos = Calc_Predicted_Reflected_Track_TCS_Position(
      object,
      sep_lat_pos,
      half_host_length);

   /** \result
    * Verify that function have returned correct position in TCS frame
    */
   DOUBLES_EQUAL(0.0F, tcs_pos.x, test_pass_thres);
   DOUBLES_EQUAL(2.0F, tcs_pos.y, test_pass_thres);
}
/** @}*/

/** \defgroup  f360_Is_SEP_Valid_For_Host_Mirror_Ghost
 *  @{
 */

 /** \brief
  * Test group containing common data for tests related to function Is_SEP_Valid_For_Host_Mirror_Ghost()
  */
TEST_GROUP(f360_Is_SEP_Valid_For_Host_Mirror_Ghost)
{
   Static_Env_Poly_T sep = {};
   F360_Calibrations_T calibs = {};

   /** \setup
    * Initialize tracker calibrations
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      sep.status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      sep.lower_limit = -25.0F;
      sep.upper_limit = 25.0F;

   }
};

/** \purpose
 * Verify that function returns true when SEP is valid and its interval extends both behind and
 * in front of host.
 * \req
 * NA
 */
TEST(f360_Is_SEP_Valid_For_Host_Mirror_Ghost, Is_SEP_Valid_For_Host_Mirror_Ghost__SEP_Spanning_Over_Host_Length)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - SEP lower limit set to -25
    * - SEP upper limit set to 25
    */

    /** \action
     * Call Is_SEP_Valid_For_Host_Mirror_Ghost
     */
   bool f_valid_Sep = Is_SEP_Valid_For_Host_Mirror_Ghost(
      sep,
      calibs.k_host_refl_half_host_length);

   /** \result
    * Verify that function have returned true
    */
   CHECK_TRUE(f_valid_Sep);
}

/** \purpose
 * Verify that function returns false when SEP is valid but its valid interval is strictly behind host
 * \req
 * NA
 */
TEST(f360_Is_SEP_Valid_For_Host_Mirror_Ghost, Is_SEP_Valid_For_Host_Mirror_Ghost__SEP_Behind_Host)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - SEP lower limit set to -25
    * - SEP upper limit set to 25
    * Set SEP upper limit behind host
    */
   sep.upper_limit = -2.1F * calibs.k_host_refl_half_host_length;

    /** \action
     * Call Is_SEP_Valid_For_Host_Mirror_Ghost
     */
   bool f_valid_Sep = Is_SEP_Valid_For_Host_Mirror_Ghost(
      sep,
      calibs.k_host_refl_half_host_length);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_valid_Sep);
}

/** \purpose
 * Verify that function returns false when SEP is valid but its valid interval is strictly in front of host
 * \req
 * NA
 */
TEST(f360_Is_SEP_Valid_For_Host_Mirror_Ghost, Is_SEP_Valid_For_Host_Mirror_Ghost__SEP_In_Front_Of_Host)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - SEP lower limit set to -25
    * - SEP upper limit set to 25
    * Set SEP lower limit in front of host
    */
   sep.lower_limit = 1.0F;

   /** \action
    * Call Is_SEP_Valid_For_Host_Mirror_Ghost
    */
   bool f_valid_Sep = Is_SEP_Valid_For_Host_Mirror_Ghost(
      sep,
      calibs.k_host_refl_half_host_length);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_valid_Sep);
}

/** \purpose
 * Verify that function returns false when SEP is invalid
 * \req
 * NA
 */
TEST(f360_Is_SEP_Valid_For_Host_Mirror_Ghost, Is_SEP_Valid_For_Host_Mirror_Ghost__SEP_Invalid)
{
   /** \precond
    * In test group the following have been set up
    * - Initialize calibrations
    * - SEP lower limit set to -25
    * - SEP upper limit set to 25
    * Set SEP as invalid
    */
   sep.status = F360_STATIC_ENV_POLY_STATUS_INVALID;

   /** \action
    * Call Is_SEP_Valid_For_Host_Mirror_Ghost
    */
   bool f_valid_Sep = Is_SEP_Valid_For_Host_Mirror_Ghost(
      sep,
      calibs.k_host_refl_half_host_length);

   /** \result
    * Verify that function have returned false
    */
   CHECK_FALSE(f_valid_Sep);
}
/** @}*/
