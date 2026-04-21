/** \file
 * This file contains unit tests for content of f360_calculate_merged_object_dimensions.cpp file
 */

#include "f360_calculate_merged_object_dimensions.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_calculate_merged_object_dimensions
 *  @{
 */

/** \brief
 *  Test group for testing Calculate_Merged_Object_Dimensions function.
 */
TEST_GROUP(f360_calculate_merged_object_dimensions)
{	
   // Initialize common variables used within all tests in f360_calculate_merged_object_dimensions test group.
   F360_Object_Track_T obj_to_keep = {};
   F360_Object_Track_T obj_to_kill = {};
   
   // Helper function for verification whether calculated dimensions are equal to expected parameters. 
   bool Are_Dimensions_Equal(const F360_Dimensions_T & expected_dimension, const F360_Dimensions_T & dimension) 
   {
      float32_t TOLERANCE = 0.0001F;
      bool f_equal = (std::abs(expected_dimension.length - dimension.length) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.length - dimension.length) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.width - dimension.width) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.len1 - dimension.len1) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.len2 - dimension.len2) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.wid1 - dimension.wid1) < TOLERANCE);
      f_equal &= (std::abs(expected_dimension.wid2 - dimension.wid2) < TOLERANCE);
      return f_equal;
   }
};

/** \purpose  
 * Test that Calculate_Merged_Object_Dimensions() return expected dimensions for two objects
 * with 0 rad orientation and placed in first quarter in VCS.
 * \req  NA.
 */
TEST(f360_calculate_merged_object_dimensions, Calculate_Merged_Object_In_First_Quarter)
{
   /** \precond
    * Define two objects with exact same dimension. Place object to kill above and on right side of object to keep.
    */
   obj_to_keep.vcs_position.x = 10.0F;
   obj_to_keep.vcs_position.y = 5.0F;
   obj_to_keep.bbox.Set_Length(4.0F);
   obj_to_keep.bbox.Set_Width(2.0F);
   obj_to_keep.Set_Bbox_Orientation(Angle{ 0.0F });

   obj_to_kill.vcs_position.x = 11.0F;
   obj_to_kill.vcs_position.y = 6.0F;
   obj_to_kill.bbox.Set_Length(4.0F);
   obj_to_kill.bbox.Set_Width(2.0F);
   obj_to_kill.Set_Bbox_Orientation(Angle{ 0.0F });

   F360_Dimensions_T exp_dim;
   exp_dim.length = 5.0F;
   exp_dim.len1 = 2.0F;
   exp_dim.len2 = 3.0F;
   exp_dim.width = 3.0F;
   exp_dim.wid1 = 1.0F;
   exp_dim.wid2 = 2.0F;

   /** \action
    * call Calculate_Merged_Object_Dimensions().
    */
   const F360_Dimensions_T dim = Calculate_Merged_Object_Dimensions(obj_to_keep, obj_to_kill);
  
   /** \result
    * Check that the dimension match expected data.
    */	
   CHECK_TRUE(Are_Dimensions_Equal(exp_dim,dim));
}

/** \purpose
 * Test that Calculate_Merged_Object_Dimensions() return expected dimensions for two objects
 *  with pi/2 rad orientation and placed in second quarter in VCS.
 * \req  NA.
 */
TEST(f360_calculate_merged_object_dimensions, Calculate_Merged_Object_In_Second_Quarter)
{
   /** \precond
    * Define two objects. Place object to kill below and on right side of object to keep.
    */
   obj_to_keep.vcs_position.x = 3.0F;
   obj_to_keep.vcs_position.y = -4.0F;
   obj_to_keep.bbox.Set_Length(5.0F);
   obj_to_keep.bbox.Set_Width(3.0F);
   obj_to_keep.Set_Bbox_Orientation(Angle{ F360_PI_2 });

   obj_to_kill.vcs_position.x = 2.5F;
   obj_to_kill.vcs_position.y = -6.0F;
   obj_to_kill.bbox.Set_Length(4.0F);
   obj_to_kill.bbox.Set_Width(2.0F);
   obj_to_kill.Set_Bbox_Orientation(Angle{ F360_PI_2 });

   F360_Dimensions_T exp_dim;
   exp_dim.length = 6.5F;
   exp_dim.len1 = 4.0F;
   exp_dim.len2 = 2.5F;
   exp_dim.width = 3.0F;
   exp_dim.wid1 = 1.5F;
   exp_dim.wid2 = 1.5F;

   /** \action
    * call Calculate_Merged_Object_Dimensions().
    */
   const F360_Dimensions_T dim = Calculate_Merged_Object_Dimensions(obj_to_keep, obj_to_kill);
   
   /** \result
    * Check that the dimension match expected data.
    */
   CHECK_TRUE(Are_Dimensions_Equal(exp_dim, dim));
}

/** \purpose
 *  Test that Calculate_Merged_Object_Dimensions() returns expected dimensions for two objects
 *  with -pi rad orientation and placed in Third quarter in VCS.
 * \req  NA.
 */
TEST(f360_calculate_merged_object_dimensions, Calculate_Merged_Object_In_Third_Quarter)
{
   /** \precond
    * Define two objects. Place object to kill below and on left side of object to keep.
    */
   obj_to_keep.vcs_position.x = -3.0F;
   obj_to_keep.vcs_position.y = -5.0F;
   obj_to_keep.bbox.Set_Length(3.0F);
   obj_to_keep.bbox.Set_Width(1.5F);
   obj_to_keep.Set_Bbox_Orientation(Angle{ -F360_PI });

   obj_to_kill.vcs_position.x = -2.0F;
   obj_to_kill.vcs_position.y = -4.0F;
   obj_to_kill.bbox.Set_Length(3.0F);
   obj_to_kill.bbox.Set_Width(2.0F);
   obj_to_kill.Set_Bbox_Orientation(Angle{ -F360_PI });

   F360_Dimensions_T exp_dim;
   exp_dim.length = 4.0F;
   exp_dim.len1 = 2.5F;
   exp_dim.len2 = 1.5F;
   exp_dim.width = 2.75F;
   exp_dim.wid1 = 2.0F;
   exp_dim.wid2 = 0.75F;

   /** \action
    * call Calculate_Merged_Object_Dimensions().
    */
   const F360_Dimensions_T dim = Calculate_Merged_Object_Dimensions(obj_to_keep, obj_to_kill);

   /** \result
    * Check that the dimension match expected data.
    */
   CHECK_TRUE(Are_Dimensions_Equal(exp_dim, dim));
}

/** \purpose
 *  Test that Calculate_Merged_Object_Dimensions() returns expected dimensions for two objects
 *  with pi/4 rad orientation and placed in fourth quarter in VCS.
 * \req  NA.
 */
TEST(f360_calculate_merged_object_dimensions, Calculate_Merged_Object_In_Fourth_Quarter)
{
   /** \precond
    * Define two objects. Place object to kill above and on left side of object to keep.
    */
   obj_to_keep.vcs_position.x = -3.0F;
   obj_to_keep.vcs_position.y = 5.0F;
   obj_to_keep.bbox.Set_Length(3.0F);
   obj_to_keep.bbox.Set_Width(1.0F);
   obj_to_keep.Set_Bbox_Orientation(Angle{ F360_PI / 4.0F });

   obj_to_kill.vcs_position.x = -2.0F;
   obj_to_kill.vcs_position.y = 5.0F;
   obj_to_kill.bbox.Set_Length(3.0F);
   obj_to_kill.bbox.Set_Width(1.5F);
   obj_to_kill.Set_Bbox_Orientation(Angle{ F360_PI / 4.0F });

   F360_Dimensions_T exp_dim;
   exp_dim.length = 3.7071F;
   exp_dim.len1 = 1.5;
   exp_dim.len2 = 2.2071F;
   exp_dim.width = 1.9571F;
   exp_dim.wid1 = 1.4571F;
   exp_dim.wid2 = 0.5F;

   /** \action
    * call Calculate_Merged_Object_Dimensions().
    */
   const F360_Dimensions_T dim = Calculate_Merged_Object_Dimensions(obj_to_keep, obj_to_kill);
   
   /** \result
    * Check that the dimension match expected data.
    */
   CHECK_TRUE(Are_Dimensions_Equal(exp_dim, dim));
}

/** \purpose
 * Test that Calculate_Merged_Object_Dimensions() return expected dimensions for two objects
 *  with different rad orientation.
 * \req  NA.
 */
TEST(f360_calculate_merged_object_dimensions, Calculate_Merged_Object_Diff_In_Orientation)
{
   /** \precond
    * Define two objects. Place object to kill below and on right side of object to keep.
    * Define different orientation for both objects.
    */
   obj_to_keep.vcs_position.x = 3.0F;
   obj_to_keep.vcs_position.y = -4.0F;
   obj_to_keep.bbox.Set_Length(5.0F);
   obj_to_keep.bbox.Set_Width(3.0F);
   obj_to_keep.Set_Bbox_Orientation(Angle{ F360_PI_2 });

   obj_to_kill.vcs_position.x = 2.5F;
   obj_to_kill.vcs_position.y = -6.0F;
   obj_to_kill.bbox.Set_Length(4.0F);
   obj_to_kill.bbox.Set_Width(2.5F);
   obj_to_kill.Set_Bbox_Orientation(Angle{ F360_PI / 4.0F });

   F360_Dimensions_T exp_dim;
   exp_dim.length = 6.7981F;
   exp_dim.len1 = 4.2981F;
   exp_dim.len2 = 2.5F;
   exp_dim.width = 4.5962F;
   exp_dim.wid1 = 1.7981F;
   exp_dim.wid2 = 2.7981F;

   /** \action
    * call Calculate_Merged_Object_Dimensions().
    */
   const F360_Dimensions_T dim = Calculate_Merged_Object_Dimensions(obj_to_keep, obj_to_kill);
   
   /** \result
    * Check that the dimension match expected data.
    */
   CHECK_TRUE(Are_Dimensions_Equal(exp_dim, dim));
}

/** @}*/
