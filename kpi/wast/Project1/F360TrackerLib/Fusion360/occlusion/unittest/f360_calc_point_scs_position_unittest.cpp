/** \file
 * This file contains unit tests for content of f360_calc_point_scs_position.cpp file
 */

#include "f360_calc_point_scs_position.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_calc_point_scs_position
 *  @{
 */

/** \brief
 * Test gropu of Calc_Point_SCS_Position() function. Tests verify whether points are properly converted from VCS to SCS.
 */
TEST_GROUP(f360_calc_point_scs_position)
{
   float32_t vcs_long_posn{};
   float32_t vcs_lat_posn{};
   Sensor_Mounting_Position sensor_mounting_position{};
   
   /** \setup
    * Set up sensor mounting position
    */
   TEST_SETUP()
   {
      const float32_t vcs_boresight_azimuth_angle = -F360_PI / 4;
      sensor_mounting_position.cos_vcs_borseight_azimuth_angle = F360_Cosf(vcs_boresight_azimuth_angle);
      sensor_mounting_position.sin_vcs_borseight_azimuth_angle = F360_Sinf(vcs_boresight_azimuth_angle);
      sensor_mounting_position.vcs_position.y = -1.5F;
      sensor_mounting_position.vcs_position.x = -0.2F;
   }
};

/** \purpose  
 * Purpose of this test is to verify whether given point is properly converted from VCS to sensor SCS
 * \req
 * NA.
 */
TEST(f360_calc_point_scs_position, f360_calc_point_scs_position__Point_Is_Properly_Rotated)
{
   /** \precond
    * Set up position of point
    */
   vcs_lat_posn = -10.0F;
   vcs_long_posn = 25.0F;
	
   /** \action
    * Call tested function
    */
   const SCS_Position_T scs_pos = Calc_Point_SCS_Position(vcs_long_posn, vcs_lat_posn, sensor_mounting_position);

   /** \result
    * Check whether scs azimuth is equal to 0.46F
    * Check whether scs range is equal to 26.5949F
    * Check whether scs lateral position is equal to 23.8294F
    * Check whether scs longitudinal position is equal to 11.8086F
    */	
   DOUBLES_EQUAL(0.46F, scs_pos.polar.azimuth, 1e-4F);
   DOUBLES_EQUAL(26.5949F, scs_pos.polar.range, 1e-4F);
   DOUBLES_EQUAL(23.8294F, scs_pos.cart.x, 1e-4F);
   DOUBLES_EQUAL(11.8086F, scs_pos.cart.y, 1e-4F);
}

/** \purpose
 * Purpose of this test is to verify whether point in center of SCS is properly translated (has coordinates 0.0F, 0.0F)
 * \req
 * NA.
 */
TEST(f360_calc_point_scs_position, f360_calc_point_scs_position__Point_In_Center_Of_SCS_Is_Properly_Rotated)
{
   /** \precond
    * Set up position of point
    */
   vcs_lat_posn = -1.5F;
   vcs_long_posn = -0.2F;

   /** \action
    * Call tested function
    */
   const SCS_Position_T scs_pos = Calc_Point_SCS_Position(vcs_long_posn, vcs_lat_posn, sensor_mounting_position);

   /** \result
    * Check whether all coordinates are equal to 0.0F
    */
   DOUBLES_EQUAL(0.0F, scs_pos.polar.azimuth, 1e-4F);
   DOUBLES_EQUAL(0.0F, scs_pos.polar.range, 1e-4F);
   DOUBLES_EQUAL(0.0F, scs_pos.cart.x, 1e-4F);
   DOUBLES_EQUAL(0.0F, scs_pos.cart.y, 1e-4F);
}

/** \purpose
 * Purpose of this test is to verify whether point in opposite side of sensor is properly rotated
 * \req
 * NA.
 */
TEST(f360_calc_point_scs_position, f360_calc_point_scs_position__Point_In_Opposite_Side_Of_Vector_Is_Properly_Rotated)
{
   /** \precond
    * Set up position of point
    */
   vcs_lat_posn = 0.9F;
   vcs_long_posn = -2.6F;

   /** \action
    * Call tested function
    */
   const SCS_Position_T scs_pos = Calc_Point_SCS_Position(vcs_long_posn, vcs_lat_posn, sensor_mounting_position);

   /** \result
    * Check whether all coordinates are equal to 0.0F
    */
   DOUBLES_EQUAL(F360_PI, scs_pos.polar.azimuth, 1e-4F);
   DOUBLES_EQUAL(3.39411259F, scs_pos.polar.range, 1e-4F);
   DOUBLES_EQUAL(-3.39411259F, scs_pos.cart.x, 1e-4F);
   DOUBLES_EQUAL(0.0F, scs_pos.cart.y, 1e-4F);
}
/** @}*/
