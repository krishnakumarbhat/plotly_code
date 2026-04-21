/** \file
 * This file contains unit tests for content of f360_verify_object_size.cpp file
 */

#include "f360_verify_object_size.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_verify_object_size
 *  @{
 */

/** \brief
 *  Test group for testing Verify_Object_Size function.
 */
TEST_GROUP(f360_verify_object_size)
{
   // Initialize common variables used within all tests in f360_verify_object_size test group.
   F360_Object_Track_T object_track_to_keep = {};
   F360_Object_Track_T object_track_to_kill = {};
   F360_Calibrations_T calibs;
   F360_Dimensions_T dimensions = {};
   
   /** \setup
    * Set up two objects with
    *   - f_vehicular_track true
    *   - speeds above 3 m/s
    *   - time since split = -1 (i.e. have not split recently)
    * Set initial merged object dimensions to
    *   - width = 2 m
    *   - length = 15 m
    */
   TEST_SETUP()
   {
      // Initialize calibrations.
      Initialize_Tracker_Calibrations(calibs);

      object_track_to_keep.f_vehicular_trk = true;
      object_track_to_keep.speed = 5.0F;
      object_track_to_keep.time_since_split = -1.0F;

      object_track_to_kill.f_vehicular_trk = true;
      object_track_to_kill.speed = 5.2F;
      object_track_to_kill.time_since_split = -1.0F;

      dimensions.length = 15.0F;
      dimensions.width = 2.0F;
   }
};

/** \purpose
 *  Test that when both objects are vehicular and fast moving (speed above 3 m/s) and have not been split recently, they
 *  are allowed to merge when the merged objects dimensions are below the thresholds for fast moving objects.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Dim_Below_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    */
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and slow moving (speed below 3 m/s) and have not been split recently, they
 *  are not allowed to merge when the merged objects dimensions are below the thresholds for slow moving objects.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Slow_Dim_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set speeds of both objects below slow moving threshold (3 m/s)
    */
   object_track_to_keep.speed = 2.7F;
   object_track_to_kill.speed = 2.9F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and only the keep object is slow moving, they are allowed to merge when
 *  the merged object dimensions are below the thresholds for fast moving objects.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Keep_Obj_Slow_Dim_Below_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set speeds of the keep object below slow moving threshold (3 m/s)
    */
   object_track_to_keep.speed = 2.7F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and only the kill object is slow moving, they are allowed to merge when
 *  the merged object dimensions are below the thresholds for fast moving objects.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Kill_Obj_Slow_Dim_Below_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set speed of the kill object below slow moving threshold (3 m/s)
    */
   object_track_to_kill.speed = 2.7F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and fast moving, they are not allowed to merge if the total merged
 *  object length is above the length threshold for fast moving objects.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Len_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set merged object length above the allowed threshold
    */
   dimensions.length = calibs.k_fast_movable_max_target_length + 3.0F + 0.001F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and fast moving, they are not allowed to merge if the total merged
 *  object width is above the width threshold.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Width_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set merged object width above the allowed threshold
    */
   dimensions.width = 3.91F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and fast moving, they are allowed to merge if the total merged
 *  object width is above the width threshold and both objects have been split recently.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Recently_Split_Width_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set merged object width above the allowed threshold
    * Set both object's time since split to a positive value, indicating that they have both been involved in a split recently.
    */
   object_track_to_keep.time_since_split = 1.0F;
   object_track_to_kill.time_since_split = 0.5F;
   dimensions.width = 3.91F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and fast moving, they are not allowed to merge if the total merged
 *  object width is above the width threshold and only the keep object has been split recently.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Keep_Recently_Split_Width_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set merged object width above the allowed threshold
    * Set both object's time since split to a positive value, indicating that they have both been involved in a split recently.
    */
   object_track_to_keep.time_since_split = 1.0F;
   dimensions.width = 3.91F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test that when both objects are vehicular and fast moving, they are not allowed to merge if the total merged
 *  object width is above the width threshold and only the kill object has been split recently.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Both_Obj_Vehicular_Fast_Kill_Recently_Split_Width_Above_Thresh)
{
   /** \precond
    * Test scenario is set up in the TEST_GROUP.
    * Set merged object width above the allowed threshold
    * Set both object's time since split to a positive value, indicating that they have both been involved in a split recently.
    */
   object_track_to_kill.time_since_split = 1.0F;
   dimensions.width = 3.91F;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test checks whether function returns true when one object has f_vehicular_trk flag set to true
 *  and second one has f_vehicular_trk flag set to false and max width value is below threshold.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Max_Object_Width_Is_Ok_And_Movable_Flag_Mismatch)
{
   /** \precond
    * Setup estimated dimension values and set object to kill f_vehicular_trk flag as true.
    */
   dimensions.width = calibs.k_movable_max_target_width - 0.1F;
   dimensions.length = calibs.k_slow_movable_max_target_length - 0.1F;
   object_track_to_keep.f_vehicular_trk = false;
   object_track_to_kill.f_vehicular_trk = true;
  
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test checks whether function returns true when one object has f_vehicular_trk flag set to true
 *  and second one has f_vehicular_trk flag set to false and max width value is below threshold.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Max_Object_Width_Is_Ok_And_Movable_Flag_Mismatch_2)
{
   /** \precond
    * Setup estimated dimension values and set object to keep movable flag as true.
    */
   dimensions.width = calibs.k_movable_max_target_width - 0.1F;
   dimensions.length = calibs.k_slow_movable_max_target_length - 0.1F;
   object_track_to_keep.f_vehicular_trk = true;
   object_track_to_kill.f_vehicular_trk = false;

   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return true.
    */
   CHECK_TRUE(f_test_passed);
}

/** \purpose
 *  Test checks whether function returns false when both objects have f_vehicular_trk flag set to false
 *  and object width is above threshold for that object type.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Max_Object_Width_Is_Above_Thr_And_Non_Movable_Objects)
{
   /** \precond
    * Setup estimated dimension values and set object to keep f_vehicular_trk flag as false.
    */
   dimensions.width = calibs.k_movable_max_target_width + 0.1F;
   dimensions.length = calibs.k_slow_movable_max_target_length - 0.1F;
   object_track_to_keep.f_vehicular_trk = false;
   object_track_to_kill.f_vehicular_trk = false;

   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);
   
   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}

/** \purpose
 *  Test checks whether function returns false when both objects have f_vehicular_trk flag set to false, 
 *  but estimated length is above threshold.
 */
TEST(f360_verify_object_size, Verify_Object_Size_Max_Object_Length_Above_Threshold_For_Non_Moveable_Objects)
{
   /** \precond
    *  Setup estimated dimension values and set both objects f_vehicular_trk flag to false.
    */
   dimensions.width = calibs.k_movable_max_target_width - 0.1F;
   dimensions.length = calibs.k_slow_movable_max_target_length + 0.1F;
   object_track_to_keep.f_vehicular_trk = false;
   object_track_to_kill.f_vehicular_trk = false;
   
   /** \action
    * call Verify_Object_Size().
    */
   bool f_test_passed = Verify_Object_Size(object_track_to_keep, object_track_to_kill, calibs, dimensions);

   /** \result
    * Function shall return false.
    */
   CHECK_FALSE(f_test_passed);
}
/** @}*/
