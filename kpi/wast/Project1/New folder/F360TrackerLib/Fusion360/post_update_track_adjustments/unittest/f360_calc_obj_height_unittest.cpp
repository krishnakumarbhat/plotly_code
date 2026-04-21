/** \file
 * This file contains unit tests for content of f360_calc_obj_height.cpp file
 */

#include "f360_calc_obj_height.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_calc_obj_height
 *  @{
 */

/** \brief
 * Test group of Calc_Obj_Height() function. Tests cover conditions for valid detections 
 * filtering and proper extraction of height value from RSPP_Detection_T struct.
 */
TEST_GROUP(f360_calc_obj_height)
{
   // Declare common variables used within all tests in this test group.
   F360_Object_Track_T current_track;
   rspp_variant_A::RSPP_Detection_T dets_raw_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Calibrations_T calib;

   /** \setup
    * initialize tracker calibrations
    * set current track assigned detections counter to 1
    * set assigned detection id
    * set an arbitary detection vcs_position_z value to 101.0F
    * detection azimuth confidence level is not low
    * set 0.0F in the first index of the heights array
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      current_track.ndets = 1;
      current_track.detids[0] = 1;
      current_track.f_moving = 1;
      dets_raw_props[0].processed.vcs_position_z = 101.0F;
      dets_raw_props[0].raw.confid_azimuth = rspp_variant_A::RSPP_CONF_AZIMUTH_MIDHIGH;
      dets_raw_props[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   }
};

/** \purpose  
 * This test checks if the otg_height is not calculated for the 
 * stationary objects.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, check_if_stationary)
{
   /** \precond
    * objects f_moving status is set to false.
    */

   current_track.f_moving = 0;
   
   /** \action
    * Call Calc_Obj_Height() function
    */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height did not increase
    */
   DOUBLES_EQUAL(0.0F , current_track.otg_height, F360_EPSILON);
}


/** \purpose  
 * This test checks if the otg_height is not increased when the 
 * azimuth confidence is too low.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, check_if_det_is_filtered_motion_status_moving_low_conf)
{
   /** \precond
    * detection azimuth confidence level is low.
    */

   dets_raw_props[0].raw.confid_azimuth = rspp_variant_A::RSPP_CONF_AZIMUTH_LOW;
   
   /** \action
    * Call Calc_Obj_Height() function
    */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height did not increase
    */
   DOUBLES_EQUAL(0.0F , current_track.otg_height, F360_EPSILON);
}

/** \purpose  
 * This test checks if the otg_height is not increased when the 
 * detection motion status is not classified as moving.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, check_if_det_is_filtered_motion_status_ambiguous_midhigh_conf)
{
   /** \precond
    * detection motion status is not classified as moving.
    */

   dets_raw_props[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   
   /** \action
    * Call Calc_Obj_Height() function
    */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height did not increase
    */
   DOUBLES_EQUAL(0.0F , current_track.otg_height, F360_EPSILON);
}

/** \purpose  
 * This test checks if the otg_height is not increased when the 
 * detection motion status is not classified as moving.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, check_if_det_is_filtered_motion_status_ambiguous_low_conf)
{
   /** \precond
    * detection motion status is not classified as moving.
    * detection azimuth confidence is low.
    */

   dets_raw_props[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   dets_raw_props[0].raw.confid_azimuth = rspp_variant_A::RSPP_CONF_AZIMUTH_LOW;
   
   /** \action
    * Call Calc_Obj_Height() function
    */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height did not increase
    */
   DOUBLES_EQUAL(0.0F , current_track.otg_height, F360_EPSILON);
}

/** \purpose  
 * This test checks if the otg_height properly decays if current track has no 
 * detections at given scan index.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, check_if_otg_height_decreases_with_zero_dets)
{
   /** \precond
    * current track otg_height > 0.0F
    * current_track ndets set to 0,
    * current_track historic ndets counter set to 10.0F.
   */
  
   current_track.otg_height = 101.0F;
   current_track.ndets = 0;
   current_track.ud_mov_historic_ndets = 10.0F;
  
   /** \action
    * Call Calc_Obj_Height() function
   */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height did decrease
    */
   DOUBLES_EQUAL(97.9700012, current_track.otg_height, F360_EPSILON);
   CHECK_TRUE(current_track.otg_height < 101.0F);
}

/** \purpose
 * This test checks if the otg_height properly increases if current track has 
 * assigned detections at given scan index with non-zero vcs_position_z value.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, otg_height_increases_with_non_zero_dets)
{
   /** \precond
    * current_track otg_height equal to 0.0F
    * current_track ndets set to 3
    * current_track historic ndets counter set to 10.0F.
    * current detections heights above the ground
    * current detection confidence level is not low
   */
   current_track.otg_height = 0.0F;
   current_track.ndets = 3;
   current_track.ud_mov_historic_ndets = 10.0F;

   for (uint8_t i=0; i<3; i++)
   {
      current_track.detids[i] = i+1;
      dets_raw_props[i].processed.vcs_position_z = -1.01F * pow(10,i);
      dets_raw_props[i].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      dets_raw_props[i].raw.confid_azimuth = rspp_variant_A::RSPP_CONF_AZIMUTH_MIDHIGH;
   } 
   /** \action
    * Call Calc_Obj_Height() function
   */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height increased and the value is correct.
    */
   DOUBLES_EQUAL(8.82755852, current_track.otg_height, F360_EPSILON);
   CHECK_TRUE(current_track.otg_height > 0.0F);
}

/** \purpose
 * This test checks if the otg_height properly decreases if current track has 
 * assigned detections at given scan index with non-zero vcs_position_z value which will
 * bring otg_height to lower value than at the previous scan index.
 * \req
 * NA.
 */
TEST(f360_calc_obj_height, otg_height_decreases_with_non_zero_dets)
{
   /** \precond
    * current_track otg_height above the ground
    * current_track ndets set to 3
    * current detection heights above the ground
    * current detection confidence level is not low
   */
   current_track.otg_height = 101.0F;
   current_track.ndets = 3;
   current_track.ud_mov_historic_ndets = 10.0F;
   for (uint8_t i=0; i<3; i++)
   {
      current_track.detids[i] = i+1;
      dets_raw_props[i].processed.vcs_position_z = -1.01F * pow(10,i);
      dets_raw_props[i].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      dets_raw_props[i].raw.confid_azimuth = rspp_variant_A::RSPP_CONF_AZIMUTH_MIDHIGH;
   } 

   /** \action
    * call Calc_Obj_Height() function
   */
   Calc_Obj_Height(dets_raw_props, calib, current_track);

   /** \result
    * Check if otg_height decreased and the value is properly calculated.
    */
   DOUBLES_EQUAL(85.9692917, current_track.otg_height, F360_EPSILON);
   CHECK_TRUE(current_track.otg_height < 101.0F);
}
/** @}*/
