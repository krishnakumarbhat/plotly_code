/** \file
 * This file contains unit tests for content of f360_merge_bbox_overlap_test.cpp file
 */

#include "f360_merge_bbox_overlap_test.h"
#include "f360_calibrations.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_merge_bbox_overlap_test
 *  @{
 */

/** \brief
 * Test group defined for testing Merge_Bbox_Overlap_Test function.
 */
TEST_GROUP(f360_merge_bbox_overlap_test)
{
   // Declare common variables used within all tests in f360_merge_bbox_overlap_test test group.
   F360_Object_Track_T first_object = {};
   F360_Object_Track_T second_object = {};
   F360_Calibrations_T calib;

   TEST_SETUP()
   {
      // Initialize calibrations.
      Initialize_Tracker_Calibrations(calib);

      first_object.vcs_position.x = 0.0F;
      first_object.vcs_position.y = 0.0F;
      first_object.Set_Bbox_Orientation(Angle{ 0.0F });

      second_object.vcs_position.x = 0.0F;
      second_object.vcs_position.y = 0.0F;
      second_object.Set_Bbox_Orientation(Angle{ 0.0F });
   }
};

/** \purpose
 *  Checks whether Merge_Bbox_Overlap_Test pass when second object bbox is inside first object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_Second_Obj_Bbox_In_First_Obj_Bbox)
{
   /** \precond
   *  Put second object bbox inside first one.
   */
   first_object.Update_Bbox_Size(5.0F, 3.0F);
   second_object.Update_Bbox_Size(2.0F, 1.0F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 * Checks whether Merge_Bbox_Overlap_Test pass when bbox corners aren't overlaps, but first object centroid is inside second object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_First_Obj_Centroid_In_Second_Obj_Bbox)
{
   /** \precond
    * Put first object centroid inside second object bbox.
    */
   first_object.Update_Bbox_Size(2.0F, 1.0F);
   second_object.Update_Bbox_Size(10.0F, 0.1F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 * Checks whether Merge_Bbox_Overlap_Test pass when bbox corners aren't overlaps, but second object centroid is inside first object bbox.
 * \req NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_Second_Obj_Centroid_In_First_Obj_Bbox)
{
   /** \precond
    * Put second object centroid inside first object bbox.
    */
   first_object.vcs_position.x = 1.0F;
   first_object.vcs_position.y = 0.5F;

   first_object.Update_Bbox_Size(2.0F, 1.0F);
   second_object.Update_Bbox_Size(10.0F, 0.1F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 *  Checks whether Merge_Bbox_Overlap_Test pass when first object front right corner is inside second object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_First_Obj_FR_Corner_In_Second_Bbox)
{
   /** \precond
   *  Put first object front right corner inside second object bbox.
   */
   first_object.Update_Bbox_Size(5.0F, 3.0F);

   second_object.vcs_position.x = 1.0F;
   second_object.vcs_position.y = 1.0F;

   second_object.Update_Bbox_Size(2.0F, 1.0F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 *  Checks whether Merge_Bbox_Overlap_Test pass when first object rear left corner is inside second object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_First_Obj_RL_Corner_In_Second_Bbox)
{
   /** \precond
   *  Put first object rear left corner inside second object bbox.
   */
   first_object.Update_Bbox_Size(5.0F, 3.0F);

   second_object.vcs_position.x = -1.0F;
   second_object.vcs_position.y = -1.0F;

   second_object.Update_Bbox_Size(4.0F, 1.0F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 *  Checks whether Merge_Bbox_Overlap_Test pass when first object rear right corner is inside second object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_First_Obj_RR_Corner_In_Second_Bbox)
{
   /** \precond
   *  Put first object rear right corner inside second object bbox.
   */
   first_object.Update_Bbox_Size(5.0F, 3.0F);

   second_object.vcs_position.x = -1.0F;
   second_object.vcs_position.y = 1.0F;

   second_object.Update_Bbox_Size(4.0F, 1.0F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}

/** \purpose
 *  Checks whether Merge_Bbox_Overlap_Test pass when first object front left corner is inside second object bbox.
 * \req  NA.
 */
TEST(f360_merge_bbox_overlap_test, Merge_Bbox_Overlap_Test_First_Obj_FL_Corner_In_Second_Bbox)
{
   /** \precond
   *  Put first object front left corner inside second object bbox.
   */
   first_object.Update_Bbox_Size(5.0F, 3.0F);

   second_object.vcs_position.x = 1.0F;
   second_object.vcs_position.y = -1.0F;
   
   second_object.Update_Bbox_Size(4.0F, 1.0F);

   /** \action
    * call Merge_Bbox_Overlap_Test().
    */
   const bool f_test_pass = Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension);

   /** \result
    * Check if function returns true.
    */
   CHECK_TRUE(f_test_pass);
}
/** @}*/
