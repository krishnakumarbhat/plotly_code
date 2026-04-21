/** \file
 * This file contains unit tests for content of f360_update_extended_bbox_offsets_for_object_in_dead_zone.cpp file
 */

#include "f360_update_extended_bbox_offsets_for_object_in_dead_zone.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_extend_assoc_gates_of_object_in_dead_zone
 *  @{
 */

 /** \brief
  * Test group of Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone. Tests verify whether object associaiton gates are
  * properly increased when object is in dead zone and meets all conditions.
  */
TEST_GROUP(f360_update_extended_bbox_offsets_for_object_in_dead_zone)
{
   F360_Calibrations_T calib{};
   float32_t host_speed{};
   Dead_Zone_T dead_zone{};
   F360_Object_Track_T object{};
   float32_t long_buffer1{};
   float32_t long_buffer2{};

   /** \setup
    * Initialize tracker calibrations
    * Set up initial host speed
    * Set up dead zone parameters
    * Set up object length
    * Set up object parameters to meet all conditions
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      host_speed = 10.0F;

      dead_zone.basic.lower = -5.0F;
      dead_zone.basic.upper = 0.0F;

      dead_zone.extended.lower = dead_zone.basic.lower - calib.k_dead_zone_long_limit_extension;
      dead_zone.extended.upper = dead_zone.basic.upper + calib.k_dead_zone_long_limit_extension;

      object.bbox.Set_Length(2.0F);

      object.dead_zone_status = F360_Dead_Zone_Status_T::INSIDE;
      object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

      object.on_sep_id = F360_INVALID_UNSIGNED_ID;
      object.behind_sep_id = F360_INVALID_UNSIGNED_ID;

      object.vcs_position.x = -2.5F;
      object.vcs_position.y = -2.0F;
      object.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = {-2.5F,-2.0F};
      object.bbox.Set_Center(center);
      object.speed = host_speed;
   }
};

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is inside dead zone its association gates are properly updated.
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Assoc_Gates_Are_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    */

    /** \action
     * Call tested function
     */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were increased to 2.5F both
    */
   DOUBLES_EQUAL(2.5F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(2.5F, long_buffer2, F360_EPSILON);
}


/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is inside dead zone its association gates are not updated for a slow moving CCA object
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Assoc_Gates_Are_Updated_Slow_Moving_CCA)
{
   /** \precond
    * Set object filter to CCA and speed to just below calib.fast_moving_thres. 
    * Adjust host speed to be equal to object speed
    */
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   object.speed = calib.fast_moving_thresh - 1e-3F;
   host_speed = object.speed; 

    /** \action
     * Call tested function
     */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 are 0
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is inside dead zone its association gates are properly updated for a fast moving CCA object
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Assoc_Gates_Are_Updated_Fast_Moving_CCA)
{
   /** \precond
    * Set object filter to CCA and speed to just above calib.fast_moving_thres. 
    * Adjust host speed to be equal to object speed
    */
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   object.speed = calib.fast_moving_thresh + 1e-3F;
   host_speed = object.speed; 

    /** \action
     * Call tested function
     */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were increased to 2.5F both
    */
   DOUBLES_EQUAL(2.5F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(2.5F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is in front of host its association gates are properly updated.
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Is_In_Front_Of_Host_Assoc_Gates_Are_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set up object vcs longitudinal positon to be in front of host
    * Set up object DEAD_ZONE_STATUS to be qual to F360_Dead_Zone_Status_T::IN_FRONT;
    */
   Point center = {0.8F,0.0F};
   object.bbox.Set_Center(center);
   object.dead_zone_status = F360_Dead_Zone_Status_T::IN_FRONT;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 was incresed to 5.8F
    * Check whether long_buffer2 was increased to 2.2F
    */
   DOUBLES_EQUAL(5.8F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(2.2F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is entering front of host its association gates are properly updated.
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Is_Entering_Front_Of_Host_Assoc_Gates_Are_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set up object vcs longitudinal positon to be in front of host
    * Set up object DEAD_ZONE_STATUS to be qual to F360_Dead_Zone_Status_T::ENTERING_FRONT;
    */
   Point center = {1.8F,0.0F};
   object.bbox.Set_Center(center);
   object.dead_zone_status = F360_Dead_Zone_Status_T::ENTERING_FRONT;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 was incresed to 1.2F
    * Check whether long_buffer2 was increased to 1.8F
    */
   DOUBLES_EQUAL(1.8F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(1.2F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is in rear of host its association gates are properly updated.
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Is_In_Rear_Of_Host_Assoc_Gates_Are_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set up object vcs longitudinal positon to be in rear of host
    * Set up object DEAD_ZONE_STATUS to be qual to F360_Dead_Zone_Status_T::IN_REAR;
    */
   Point center = {-5.8F,0.0F};
   object.bbox.Set_Center(center);
   object.dead_zone_status = F360_Dead_Zone_Status_T::IN_REAR;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 was incresed to 2.2F
    * Check whether long_buffer2 was increased to 5.8F
    */
   DOUBLES_EQUAL(2.2F, long_buffer1, 1e-4);
   DOUBLES_EQUAL(5.8F, long_buffer2, 1e-4);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions, is entering rear of host its association gates are properly updated.
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Is_Entering_Rear_Of_Host_Assoc_Gates_Are_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set up object vcs longitudinal positon to be in rear of host
    * Set up object DEAD_ZONE_STATUS to be qual to F360_Dead_Zone_Status_T::ENTERING_REAR;
    */
   Point center = {-6.8F,0.0F};
   object.bbox.Set_Center(center);
   object.dead_zone_status = F360_Dead_Zone_Status_T::ENTERING_REAR;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 was incresed to 1.2F
    * Check whether long_buffer2 was increased to 1.8F
    */
   DOUBLES_EQUAL(1.2F, long_buffer1, 1e-4);
   DOUBLES_EQUAL(1.8F, long_buffer2, 1e-4);
}

/** \purpose
 * Purpose of this test is to verify whether when object meets all conditions but its dead zone status is obsolete, its association gates are not modified
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__All_Conditions_Met_Dead_Zone_Status_Obsolete_Assoc_Gates_Not_Modified)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set up object vcs longitudinal positon to be in rear of host
    * Set up object DEAD_ZONE_STATUS to be qual to F360_Dead_Zone_Status_T::LEAVING_REAR;
    */
   Point center = {-6.8F,0.0F};
   object.bbox.Set_Center(center);
   object.dead_zone_status = F360_Dead_Zone_Status_T::LEAVING_REAR;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 was not modified (is equal to 0.0F)
    * Check whether long_buffer2 was not modified (is equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, 1e-4);
   DOUBLES_EQUAL(0.0F, long_buffer2, 1e-4);
}

/** \purpose
 * Purpose of this test is to verify whether object association gates are not modified when its relative speed difference is too big
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__Rel_Speed_Diff_Too_Big_Not_Updated)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set object speed to be equal to 2.0F * host_speed
    */
   object.speed = 2.0F * host_speed;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were not modified (are equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether object association gates are not modified when its dead zone status is OUTSIDE
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__Dead_Zone_Status_Outside)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set object DEAD ZONE status as OUTSIDE
    */
   object.dead_zone_status = F360_Dead_Zone_Status_T::OUTSIDE;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were not modified (are equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether object association gates are not modified when its dead zone status is UNDEFINED
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__Dead_Zone_Status_Undefined)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set object DEAD ZONE status as UNDEFINED
    */
   object.dead_zone_status = F360_Dead_Zone_Status_T::UNDEFINED;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were not modified (are equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether object association gates are not modified when its on guardrail
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__Track_On_Guardrail)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set object to be on some guardrail
    */
   object.on_sep_id = 1;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were not modified (are equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether object association gates are not modified when its behind guardrail
 * \req
 * NA.
 */
TEST(f360_update_extended_bbox_offsets_for_object_in_dead_zone, Extend_Assoc_Gates_Of_Object_In_Dead_Zone__Track_Behind_Guardrail)
{
   /** \precond
    * All is set in TEST_SETUP()
    * Set object to be behind some guardrail
    */
   object.behind_sep_id = 2;

   /** \action
    * Call tested function
    */
   Update_Extended_BBox_Offsets_For_Object_In_Dead_Zone(calib, host_speed, dead_zone, object, long_buffer1, long_buffer2);

   /** \result
    * Check whether long_buffer1 and long_buffer2 were not modified (are equal to 0.0F)
    */
   DOUBLES_EQUAL(0.0F, long_buffer1, F360_EPSILON);
   DOUBLES_EQUAL(0.0F, long_buffer2, F360_EPSILON);
}
/** @}*/