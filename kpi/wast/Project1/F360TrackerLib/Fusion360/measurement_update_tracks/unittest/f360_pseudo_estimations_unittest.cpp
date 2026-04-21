/** \file
 * This file contains unit tests for content of f360_pseudo_estimations.cpp file
 */

#include "f360_pseudo_estimations.h"
#include <CppUTest/TestHarness.h>
#include "f360_math.h"
#include "f360_constants.h"

using namespace f360_variant_A;

/** \defgroup  f360_pseudo_estimations
 *  @{
 */

/** \brief
 * Testing of Pseudo_Estimations
 */
TEST_GROUP(f360_pseudo_estimations)
{
   // Declare common variables used within all tests in this test group.
   F360_Object_Track_T obj = {};
   F360_Host_T host = {};
   F360_Calibrations_T calibs;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Globals_T globals = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   float32_t test_pass_thr = 1e-6F;
   Point initial_pseudo_pos;

   /** \setup
    * Set up an object with 2 associated detections
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      globals.f_single_front_center_radar_only = false;

      host.dist_rear_axle_to_vcs_m = 3.0F;

      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      obj.vcs_position = {5.0F, 4.0F};
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.bbox.Set_Center(7.0F, 5.0F);
      obj.bbox.Set_Orientation(0.0F);
      obj.pseudo_vcs_position = {0.0F, 0.0F};
      obj.ndets = 2;
      obj.detids[0] = 1;
      obj.detids[1] = 2;

      det_props[0].vcs_position = {8.0F, 4.0F};
      det_props[1].vcs_position = {5.0F, 6.0F};

      initial_pseudo_pos.x = obj.pseudo_vcs_position.x;
      initial_pseudo_pos.y = obj.pseudo_vcs_position.y;
   }
};

/*\purpose
 * Check that Pseudo_Estimations outputs a new pseudo position.
*/
TEST(f360_pseudo_estimations, Test_Pseudo_Pos_Is_Updated)
{
   /**
   * An object with two associated detections and pseudo pos at (0,0) has been set up.
   **/

   /** \action
   * Call the function Pseudo_Estimations
   **/
   Pseudo_Estimations(calibs, host, det_props, sensors, globals, obj, timing_info);

   /** \result
   * Check that pseudo position is updated.
   **/
   CHECK_FALSE_TEXT(initial_pseudo_pos.x ==  obj.pseudo_vcs_position.x, "Pseudo x pos was not updated.");
   CHECK_FALSE_TEXT(initial_pseudo_pos.y ==  obj.pseudo_vcs_position.y, "Pseudo y pos was not updated.");
}
/** @}*/
