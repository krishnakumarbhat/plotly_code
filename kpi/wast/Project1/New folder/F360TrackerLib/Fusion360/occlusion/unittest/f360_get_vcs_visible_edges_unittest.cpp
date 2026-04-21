/** \file
 * This file contains unit tests for content of f360_get_vcs_visible_edges.cpp file
 */

#include "f360_get_vcs_visible_edges.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_get_vcs_visible_edges
 *  @{
 */

 /** \brief
  * Test group of Get_VCS_Visible_Edges function. Tests verify whether edges are properly picked
  * and rotated to target coordinates system.
  */
TEST_GROUP(f360_get_vcs_visible_edges)
{
   F360_Object_Track_T object{};

   /** \setup
    * Set up object size parameters
    */
   TEST_SETUP()
   {
      object.Update_Bbox_Size(2.0F, 2.0F);

      object.Set_Bbox_Orientation(Angle{ 0.0F });
   }
};

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: REAR, LEFT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Rear_Left)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_REAR_LEFT
    */
   object.reference_point = F360_REFERENCE_POINT_REAR_LEFT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as valid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to -1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether second edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether second edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether second edge ending point lateral coordinate is equal to -1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_TRUE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: REAR - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Rear)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_REAR
    */
   object.reference_point = F360_REFERENCE_POINT_REAR;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as invalid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to -1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: REAR, RIGHT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Rear_Right)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_REAR_RIGHT
    */
   object.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as valid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to -1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether second edge starting point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether second edge ending point lateral coordinate is equal to 1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_TRUE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: LEFT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Left)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_LEFT
    */
   object.reference_point = F360_REFERENCE_POINT_LEFT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as invalid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether first edge ending point lateral coordinate is equal to -1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are INVALID - function returns marks two edges as invalid.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Invalid)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_INVALID
    */
   object.reference_point = F360_REFERENCE_POINT_CENTER;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as invalid
    * Check whether second edge is marked as invalid
    */
   CHECK_FALSE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: RIGHT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Right)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_RIGHT
    */
   object.reference_point = F360_REFERENCE_POINT_RIGHT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as invalid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether first edge starting point lateral coordinate is equal to 1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: FRONT, LEFT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Front_Left)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_FRONT_LEFT
    */
   object.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as valid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to 1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether second edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether second edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether second edge ending point lateral coordinate is equal to -1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_TRUE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: FRONT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__FRONT)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_FRONT
    */
   object.reference_point = F360_REFERENCE_POINT_FRONT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as invalid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to 1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are: FRONT, RIGHT - function returns coordinates of those edges.
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Front_Right)
{
   /** \precond
    * Set up object visible edges to F360_COMBINED_VISIBLE_EDGES_FRONT_RIGHT
    */
   object.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as valid
    * Check whether second edge is marked as valid
    *
    * Check whether first edge starting point longitudinal coordinate is equal to 1.0F
    * Check whether first edge starting point lateral coordinate is equal to -1.0F
    *
    * Check whether first edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether first edge ending point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge starting point longitudinal coordinate is equal to -1.0F
    * Check whether second edge starting point lateral coordinate is equal to 1.0F
    *
    * Check whether second edge ending point longitudinal coordinate is equal to 1.0F
    * Check whether second edge ending point lateral coordinate is equal to 1.0F
    */
   CHECK_TRUE(raw_visible_edges.first_edge.f_valid);
   CHECK_TRUE(raw_visible_edges.second_edge.f_valid);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(-1.0F, raw_visible_edges.first_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.first_edge.point_end.y, F360_EPSILON);

   DOUBLES_EQUAL(-1.0F, raw_visible_edges.second_edge.point_begin.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_begin.y, F360_EPSILON);

   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.x, F360_EPSILON);
   DOUBLES_EQUAL(1.0F, raw_visible_edges.second_edge.point_end.y, F360_EPSILON);
}

/** \purpose
 * Purpsoe of this test is to verify whether when object visible edges are not handled yet - function marks both edges as invalid
 * \req
 * NA.
 */
TEST(f360_get_vcs_visible_edges, Get_VCS_Visible_Edges__Unhandled_Visible_Edges)
{
   /** \precond
    * Set up object visible edges to unhandled value (way beyond enum limit)
    */
   object.reference_point = static_cast<F360_Reference_Point_T>(100);

   /** \action
    * Call tested function
    */
   const VCS_Track_Visible_Edges_T raw_visible_edges = Get_VCS_Visible_Edges(object);

   /** \result
    * Check whether first edge is marked as invalid
    * Check whether second edge is marked as invalid
    */
   CHECK_FALSE(raw_visible_edges.first_edge.f_valid);
   CHECK_FALSE(raw_visible_edges.second_edge.f_valid);
}
/** @}*/
