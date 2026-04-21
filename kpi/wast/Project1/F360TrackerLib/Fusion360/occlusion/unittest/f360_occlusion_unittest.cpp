/** \file
 * This file contains unit tests for content of f360_occlusion.cpp file
 */

#include "f360_occlusion.h"
#include "utilities/f360_occlusion_ut_helpers.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_occlusion
 *  @{
 */

/** \brief
 * Test group of Occlusion_T class. Tests verify whether sensor and object information is properly propagated to determine
 * occluded sectors.
 */
TEST_GROUP(f360_occlusion)
{
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS]{};
   F360_Tracker_Info_T tracker_info{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_TRKR_TIMING_INFO_T timing_info{};
   F360_Calibrations_T calib{};

   /** \setup
    * Set up 4 corner sensors parameters.
    * Set up two valid objects
    * Initialize occlusion
    * Initialize tracker calibrations
    */
   TEST_SETUP()
   {
      Set_Left_Rear_Sensor(sensors[0]);
      Set_Left_Front_Sensor(sensors[1]);
      Set_Right_Rear_Sensor(sensors[2]);
      Set_Right_Front_Sensor(sensors[3]);

      Set_Base_Object_Parameters(object_tracks[0]);
      Point center = {5.0F,-5.0F};
      object_tracks[0].bbox.Set_Center(center);

      object_tracks[0].reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
      object_tracks[0].confidenceLevel = 1.0F;
      object_tracks[0].vcs_position = object_tracks[0].bbox.Get_Corners().Rear_Right();


      Set_Base_Object_Parameters(object_tracks[1]);
      object_tracks[1].reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      object_tracks[1].confidenceLevel = 1.0F;
      center = {5.0F,5.0F};
      object_tracks[1].bbox.Set_Center(center);
      object_tracks[1].vcs_position = object_tracks[1].bbox.Get_Corners().Rear_Left();

      tracker_info.num_active_objs = 2;
      tracker_info.active_obj_ids[0] = 1;
      tracker_info.active_obj_ids[1] = 2;

      Initialize_Tracker_Calibrations(calib);
      calib.k_occlusion_range_uncertainty_th = 0.0F;
   }
};


/** \purpose  
 * Purpose of this test is to verify whether object that is outside of sensors FOV will have its occlusion status marked as undefined
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Point_Outside_FOV_Is_Marked_As_Undefined)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Create occlusion object
    * Set up point position to be outside of FOV
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t lat_pos = 0.9F;
   const float32_t long_pos = -2.5F;
	
   /** \action
    * Determine occlusion status of point outside of FOV
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(long_pos, lat_pos);

   /** \result
    * Check whether occlusion status is UNDEFINED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether object that is inside of sensors FOV will have its occlusion status determined
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Point_Inside_FOV_Is_Marked_As_Visible_If_Not_Occluded)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Create occlusion object
    * Set up point position to be inside of FOV
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t lat_pos = 0.9F;
   const float32_t long_pos = 10.0F;

   /** \action
    * Determine occlusion status of point
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(long_pos, lat_pos);

   /** \result
    * Check whether occlusion status is VISIBLE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_VISIBLE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion information is properly propagated
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Information_Is_Properly_Propagated)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Reset FR corner is_valid flag to false
    * Set position of first point to be visible
    * Set position of second point to be occluded
    * Create occlusion object
    */

   sensors[0].variable.is_valid = false;

   const float32_t first_lat_pos = -20.0F;
   const float32_t first_long_pos = 2.5F;

   const float32_t second_lat_pos = -6.0F;
   const float32_t second_long_pos = 6.0F;

   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
	
   /** \action
    * Determine first point occlusion status
    * Determine second point occlusion status
    */
   const auto first_point_occlusion_status = occlusion.Determine_Occlusion_Status(first_long_pos, first_lat_pos);
   const auto second_point_occlusion_status = occlusion.Determine_Occlusion_Status(second_long_pos, second_lat_pos);

   /** \result
    * Check whether first point is visible
    * Check whether second point is occluded
    */
   CHECK_EQUAL(Occlusion_Status_T::OCCLUSION_STATUS_VISIBLE, first_point_occlusion_status);
   CHECK_EQUAL(Occlusion_Status_T::OCCLUSION_STATUS_OCCLUDED, second_point_occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly determined when point is visible
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Point_Is_Visible)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it visible
    * Create occlusion object
    */
   const float32_t vcs_long_posn = 2.5F;
   const float32_t vcs_lat_posn = -2.5F;
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);

    /** \action
     * Determine occlusion status
     */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_VISIBLE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_VISIBLE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly determined when point is on edge of visibility
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Point_Is_On_Edge_Of_Visibility)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be on edge of visibility
    * Create occlusion object
    */
   const float32_t vcs_long_posn = 4.0F;
   const float32_t vcs_lat_posn = -4.0F;
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
    * Determine occlusion status
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_ON_EDGE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_ON_EDGE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly determined when point is on occluded
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Point_Is_Occluded)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be occluded
    * Create occlusion object
    */
   const float32_t vcs_long_posn = 10.0F;
   const float32_t vcs_lat_posn = -10.0F;
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
    * Determine occlusion status
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_OCCLUDED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_OCCLUDED, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is set as visible when point is on edge of
 * object but threshold passed to occlusion object makes it be visible
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Point_On_Edge_Of_Visibility_Marked_As_Visible_By_Adding_Threshold)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be occluded
    * Set range uncertainty threshold to 0.3F
    * Create occlusion object
    */
   const float32_t vcs_long_posn = 4.0F;
   const float32_t vcs_lat_posn = -4.0F;
   calib.k_occlusion_range_uncertainty_th = 0.3F;
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
    * Determine occlusion status
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_VISIBLE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_VISIBLE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly marked as undefined when occlusion is turned off
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Occlusion_Is_Turned_Off)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be visible
    * Turn off occlusion
    * Create occlusion object
    */
   const float32_t vcs_long_posn = 2.5F;
   const float32_t vcs_lat_posn = -2.5F;
   const Occlusion_T occlusion(false, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
    * Determine occlusion status
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_UNDEFINED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether object that was used to update occlusion data is marked as f_used_by_occlusion
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Object_Marked_As_Used_By_Occlusion)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set second object f_moveable flag to false
    */
   object_tracks[1].f_moveable = false;

   /** \action
    * Create occlusion object
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \result
    * Check whether first track was marked as f_used_by_occlusion
    * Check whether second track was not marked as f_used_by_occlusion
    */
   CHECK_TRUE(object_tracks[0].f_used_by_occlusion);
   CHECK_FALSE(object_tracks[1].f_used_by_occlusion);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status are properly sorted
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Sorted)
{
   /** \precond
    * Nothing to set up
    */

   /** \action
    * No action
    */

   /** \result
    * check whether UNDEFINED status is lower than OCCLUDED
    * check whether OCCLUDED status is lower than ON_EDGE
    * check whether ON_EDGE status is lower than VISBILE
    */
   CHECK_TRUE(Occlusion_Status_T::OCCLUSION_STATUS_UNDEFINED < Occlusion_Status_T::OCCLUSION_STATUS_OCCLUDED);
   CHECK_TRUE(Occlusion_Status_T::OCCLUSION_STATUS_OCCLUDED < Occlusion_Status_T::OCCLUSION_STATUS_ON_EDGE);
   CHECK_TRUE(Occlusion_Status_T::OCCLUSION_STATUS_ON_EDGE < Occlusion_Status_T::OCCLUSION_STATUS_VISIBLE);
}

/** \purpose
 * Purpose of this test is to verify whether object that is outside FOV of single sensor will have its occlusion status marked as undefined
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Point_Outside_FOV_Of_Single_Sensor_Is_Marked_As_Undefined)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Create occlusion object
    * Set up point position to be outside of FOV - in the upper right quarter of VCS
    * Set up teseted sensor idx to rear left
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t lat_pos = 5.9F;
   const float32_t long_pos = 20.5F;
   const int32_t tested_sensor_idx = 0U;

   /** \action
    * Determine occlusion status of point outside of FOV of single given sensor
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(long_pos, lat_pos, tested_sensor_idx);

   /** \result
    * Check whether occlusion status is UNDEFINED
    * Tested sensor is left rear, meanwhile tested point is in upper right quarter of VSC
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether object that is inside of single sensors FOV will have its occlusion status determined
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Point_Inside_Single_Sensors_FOV_Is_Marked_As_Visible_If_Not_Occluded)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Create occlusion object
    * Set up point position to be inside of FOV of sinle given sensor
    * Set up teseted sensor idx to front left
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t lat_pos = -5.9F;
   const float32_t long_pos = 15.0F;
   const int32_t tested_sensor_idx = 1U;

   /** \action
    * Determine occlusion status of point inside of FOV of single given sensor
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(long_pos, lat_pos, tested_sensor_idx);

   /** \result
    * Check whether occlusion status is VISIBLE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_VISIBLE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly determined when point is on edge of visibility for single sensor
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Point_Is_On_Edge_Of_Visibility_For_Single_Sensor)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be on edge of visibility
    * Create occlusion object
    * Set up teseted sensor idx to front left
    */
   const float32_t vcs_long_posn = 4.0F;
   const float32_t vcs_lat_posn = -4.0F;
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const int32_t tested_sensor_idx = 1U;

   /** \action
    * Determine occlusion status for single sensor
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(vcs_long_posn, vcs_lat_posn, tested_sensor_idx);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_ON_EDGE
    */
   CHECK_EQUAL(OCCLUSION_STATUS_ON_EDGE, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly determined when point is occluded for single sensor
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_Is_Properly_Determined_When_Point_Is_Occluded_For_Single_Sensor)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Set up tested point position to make it be occluded
    * Create occlusion object
    * Set up tested sensor idx to front left
    */
   const Occlusion_T occlusion(true, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t vcs_long_posn = 10.0F;
   const float32_t vcs_lat_posn = -10.0F;
   const int32_t tested_sensor_idx = 1U;

   /** \action
    * Determine occlusion status for single sensor
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(vcs_long_posn, vcs_lat_posn, tested_sensor_idx);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_OCCLUDED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_OCCLUDED, occlusion_status);
}

/** \purpose
 * Purpose of this test is to verify whether occlusion status is properly marked as undefined when occlusion is turned off
 * \req
 * NA.
 */
TEST(f360_occlusion, Update__Occlusion_Status_For_Single_Sensor_Is_Properly_Determined_When_Occlusion_Is_Turned_Off)
{
   /** \precond
    * All basic input data was set in TEST_SETUP
    * Turn off occlusion
    * Create occlusion object
    * Set up tested point position to make it be visible
    */
   const bool f_enable_occlusion = false;
   const Occlusion_T occlusion(f_enable_occlusion, calib, tracker_info, sensors, timing_info, object_tracks);
   const float32_t vcs_long_posn = 2.5F;
   const float32_t vcs_lat_posn = -2.5F;
   const int32_t tested_sensor_idx = 3;


   /** \action
    * Determine occlusion status
    */
   const auto occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(vcs_long_posn, vcs_lat_posn, tested_sensor_idx);

   /** \result
    * Check whether returned value is equal to OCCLUSION_STATUS_UNDEFINED
    */
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, occlusion_status);
}

/** @}*/
