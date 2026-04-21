/** \file
 * This file contains unit tests for content of f360_get_scs_visible_edges.cpp file
 */

#include "f360_get_scs_visible_edges.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_get_scs_visible_edges
 *  @{
 */

/** \brief
 * Test group of Get_SCS_Visible_Edges() function. Tests verify whether visible edges given in VCS coordinates are properly rotated to SCS.
 */
TEST_GROUP(f360_get_scs_visible_edges)
{
   VCS_Track_Visible_Edges_T vcs_visible_edges{};
   Sensor_Mounting_Position mounting_pos{};
   
   /** \setup
    * Set up visible edges positions
    * Set up sensor mounting position
    */
   TEST_SETUP()
   {
      vcs_visible_edges.first_edge.point_begin.y = -10.0F;
      vcs_visible_edges.first_edge.point_begin.x = -10.0F;

      vcs_visible_edges.first_edge.point_end.y = -10.0F;
      vcs_visible_edges.first_edge.point_end.x = -5.0F;

      vcs_visible_edges.second_edge.point_begin.y = 10.0F;
      vcs_visible_edges.second_edge.point_begin.x = 15.0F;

      vcs_visible_edges.second_edge.point_end.y = 10.0F;
      vcs_visible_edges.second_edge.point_end.x = 10.0F;

      const float32_t vcs_boresight_azimuth_angle = -F360_PI / 4;

      mounting_pos.vcs_position.y = -1.0F;
      mounting_pos.vcs_position.x = 0.0F;

      mounting_pos.cos_vcs_borseight_azimuth_angle = F360_Cosf(vcs_boresight_azimuth_angle);
      mounting_pos.sin_vcs_borseight_azimuth_angle = F360_Sinf(vcs_boresight_azimuth_angle);
   }
};

/** \purpose  
 * Purpose of this test is to verify whether edges are marked as valid if vcs edges are valid
 * \req
 * NA.
 */
TEST(f360_get_scs_visible_edges, Get_SCS_Visible_Edges__Edges_Are_Marked_As_Valid)
{
   /** \precond
    * Set vcs edges f_valid flag to true
    */
   vcs_visible_edges.first_edge.f_valid = true;
   vcs_visible_edges.second_edge.f_valid = true;
	
   /** \action
    * Call tested function
    */
   const SCS_Track_Visible_Edges_T scs_visible_edges = Get_SCS_Visible_Edges(vcs_visible_edges, mounting_pos);

   /** \result
    * Check whether both scs_visible_edges are marked as valid
    */	
   CHECK_TRUE(scs_visible_edges.first_edge.f_valid);
   CHECK_TRUE(scs_visible_edges.second_edge.f_valid);
}

/** \purpose
 * Purpose of this test is to verify whether edges are marked as invalid if vcs edges are invalid
 * \req
 * NA.
 */
TEST(f360_get_scs_visible_edges, Get_SCS_Visible_Edges__Edges_Are_Marked_As_Invalid)
{
   /** \precond
    * Set vcs edges f_valid flag to false
    */
   vcs_visible_edges.first_edge.f_valid = false;
   vcs_visible_edges.second_edge.f_valid = false;

   /** \action
    * Call tested function
    */
   const SCS_Track_Visible_Edges_T scs_visible_edges = Get_SCS_Visible_Edges(vcs_visible_edges, mounting_pos);

   /** \result
    * Check whether both scs_visible_edges are marked as invalid
    */
   CHECK_FALSE(scs_visible_edges.first_edge.f_valid);
   CHECK_FALSE(scs_visible_edges.second_edge.f_valid);
}

/** \purpose
 * Purpose of this test is to verify whether edges are properly translated to SCS
 * \req
 * NA.
 */
TEST(f360_get_scs_visible_edges, Get_SCS_Visible_Edges__Edges_Are_Properly_Rotated)
{
   /** \precond
    * Set vcs edges f_valid flag to true
    */
   vcs_visible_edges.first_edge.f_valid = true;
   vcs_visible_edges.second_edge.f_valid = true;

   /** \action
    * Call tested function
    */
   const SCS_Track_Visible_Edges_T scs_visible_edges = Get_SCS_Visible_Edges(vcs_visible_edges, mounting_pos);

   /** \result
    * Check whether scs_visible_edges.first_edge.point_begin.cart.x is equal to -0.707107067F
    * Check whether scs_visible_edges.first_edge.point_begin.cart.y is equal to -13.4350281F

    * Check whether scs_visible_edges.first_edge.point_begin.polar.azimuth is equal to -1.62337947F
    * Check whether scs_visible_edges.first_edge.point_begin.polar.range is equal to 13.4536238F

    * Check whether scs_visible_edges.first_edge.point_end.cart.x is equal to 2.82842684F
    * Check whether scs_visible_edges.first_edge.point_end.cart.y is equal to -9.89949417F

    * Check whether scs_visible_edges.first_edge.point_end.polar.azimuth is equal to -1.29249668F
    * Check whether scs_visible_edges.first_edge.point_end.polar.range is equal to  10.2956295F


    * Check whether scs_visible_edges.second_edge.point_begin.cart.x is equal to 2.82842731F
    * Check whether scs_visible_edges.second_edge.point_begin.cart.y is equal to 18.3847771F

    * Check whether scs_visible_edges.second_edge.point_begin.polar.azimuth is equal to 1.41814697F
    * Check whether scs_visible_edges.second_edge.point_begin.polar.range is equal to 18.6010761F

    * Check whether scs_visible_edges.second_edge.point_end.cart.x is equal to -0.707106590F
    * Check whether scs_visible_edges.second_edge.point_end.cart.y is equal to 14.8492422F

    * Check whether scs_visible_edges.second_edge.point_end.polar.azimuth is equal to 1.61837947F
    * Check whether scs_visible_edges.second_edge.point_end.polar.range is equal to 14.8660688F
    */

   DOUBLES_EQUAL(-0.707107067F, scs_visible_edges.first_edge.point_begin.cart.x, 1e-4);
   DOUBLES_EQUAL(-13.4350281F, scs_visible_edges.first_edge.point_begin.cart.y, 1e-4);

   DOUBLES_EQUAL(-1.62337947F, scs_visible_edges.first_edge.point_begin.polar.azimuth, 1e-4);
   DOUBLES_EQUAL(13.4536238F, scs_visible_edges.first_edge.point_begin.polar.range, 1e-4);

   DOUBLES_EQUAL(2.82842684F, scs_visible_edges.first_edge.point_end.cart.x, 1e-4);
   DOUBLES_EQUAL(-9.89949417F, scs_visible_edges.first_edge.point_end.cart.y, 1e-4);

   DOUBLES_EQUAL(-1.29249668F, scs_visible_edges.first_edge.point_end.polar.azimuth, 1e-4);
   DOUBLES_EQUAL(10.2956295F, scs_visible_edges.first_edge.point_end.polar.range, 1e-4);


   DOUBLES_EQUAL(2.82842731F, scs_visible_edges.second_edge.point_begin.cart.x, 1e-4);
   DOUBLES_EQUAL(18.3847771F, scs_visible_edges.second_edge.point_begin.cart.y, 1e-4);

   DOUBLES_EQUAL(1.41814697F, scs_visible_edges.second_edge.point_begin.polar.azimuth, 1e-4);
   DOUBLES_EQUAL(18.6010761F, scs_visible_edges.second_edge.point_begin.polar.range, 1e-4);

   DOUBLES_EQUAL(-0.707106590F, scs_visible_edges.second_edge.point_end.cart.x, 1e-4);
   DOUBLES_EQUAL(14.8492422F, scs_visible_edges.second_edge.point_end.cart.y, 1e-4);

   DOUBLES_EQUAL(1.61837947F, scs_visible_edges.second_edge.point_end.polar.azimuth, 1e-4);
   DOUBLES_EQUAL(14.8660688F, scs_visible_edges.second_edge.point_end.polar.range, 1e-4);
}

/** \purpose
 * Purpose of this test is to verify whether edges are points are sorted w.r.t. azimuth after rotation
 * \req
 * NA.
 */
TEST(f360_get_scs_visible_edges, Get_SCS_Visible_Edges__Points_Are_Sorted_By_Azimuth)
{
   /** \precond
    * Set up first edge f_valid flag to true
    * Set up positions of first edge points so they are not sorted by azimuth
    */
   vcs_visible_edges.first_edge.f_valid = true;
   vcs_visible_edges.first_edge.point_begin.y = -5.0F;
   vcs_visible_edges.first_edge.point_begin.x = 15.0F;

   vcs_visible_edges.first_edge.point_end.y = -5.0F;
   vcs_visible_edges.first_edge.point_end.x = -15.0F;

   /** \action
    * Call tested function
    */
   const SCS_Track_Visible_Edges_T scs_visible_edges = Get_SCS_Visible_Edges(vcs_visible_edges, mounting_pos);

   /** \result
    * Check whether first edge begin point azimuth is smaller than end point azimuth.
    */
   CHECK_TRUE(scs_visible_edges.first_edge.point_begin.polar.azimuth < scs_visible_edges.first_edge.point_end.polar.azimuth);
}
/** @}*/
