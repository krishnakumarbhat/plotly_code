/** \file
   This file contains unit tests for content of f360_pseudo_position_estimation.cpp file
*/

#include "f360_pseudo_position_estimation.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_get_reference_point_para_side.h"
#include "f360_get_reference_point_orth_side.h"
#include "f360_math_func.h"
#include <CppUTest/TestHarness.h>

#include "f360_clear_object_track.h"

using namespace f360_variant_A;

/** \defgroup  f360_pseudo_position_estimation
 *  @{
 */

/** \brief
 * Testing of a function that estimates the position of an object based
 * on position of associated detections and corresponding weights.
 */

TEST_GROUP(f360_pseudo_position_estimation)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibrations = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Host_T host = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Globals_T globals = {};

   // Expected pseudo x and y position and corresponding variances
   float32_t exp_pseudo_pos_x = 0.0F;
   float32_t exp_pseudo_pos_x_var = 0.0F;
   float32_t exp_pseudo_pos_y = 0.0F;
   float32_t exp_pseudo_pos_y_var = 0.0F;

   float32_t exp_pseudo_pos_cov_xy = 0.0F; // Expected pseudo covariance between x- and y-position

   // Estimated value and variance of edge in para and orth direction
   float32_t para_est_mean = 0.0F;
   float32_t para_est_var = 0.0F;
   float32_t orth_est_mean = 0.0F;
   float32_t orth_est_var = 0.0F;

   // variances from previous iteration
   float32_t obj_var_para = 0.0F;
   float32_t obj_var_orth = 0.0F;

   Point assoc_dets_pos_tcs[MAX_DETS_IN_OBJ_TRK] = {};

   /** \setup
    * Initialize object with an arbitrary size, centroid in object center and VCS position to 0
    * Set x pos and y pos part of meascov to zero
    * Set gains to same values as in source file
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);

      globals.f_single_front_center_radar_only = false;
      
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);

      obj.vcs_position.x = 0.0F;
      obj.vcs_position.y = 0.0F;
      Point center = {0.0F,0.0F};
      obj.bbox.Set_Center(center);
      obj.reference_point = F360_REFERENCE_POINT_CENTER;

      obj.Set_Bbox_Orientation(Angle{ 0.0F });

      obj.meascov[0][0] = 0.0F;
      obj.meascov[0][1] = 0.0F;
      obj.meascov[1][0] = 0.0F;
      obj.meascov[1][1] = 0.0F;
   }
};

/**
*\purpose  Verify that correct pseudo position is set when no edge is ok (i.e. reference point is CENTER)
*\req    NA
*/
TEST(f360_pseudo_position_estimation, Pseudo_Position_Estimation_No_Edge_Is_Ok)
{
   /** \precond
    * Set obj.ndets to 2
    * Set position variance for para and orth of first detection to 0, which will result in edge NOK.
    * Set detection tcs position to (x,y) = (3,2)
    * Set expected pseudo x and y pos to mean of detections
    * Set expected meascov matrix to what the test output (at this level of the function, we only test that it is changed - not that the values are necessarily correct)
    */
   obj.ndets = 2U; // 1
   obj.detids[0U] = 1U;
   obj.detids[1U] = 2U;
   assoc_dets_pos_tcs[0].x = 3.0F;
   assoc_dets_pos_tcs[0].y = 2.0F;
   assoc_dets_pos_tcs[1].x = 2.0F;
   assoc_dets_pos_tcs[1].y = 1.0F;

   for (uint32_t i = 0U; i < obj.ndets; i++)
   {
      Convert_TCS_Posn_To_VCS_Posn(
         assoc_dets_pos_tcs[i].x,
         assoc_dets_pos_tcs[i].y,
         obj.bbox.Get_Center().x,
         obj.bbox.Get_Center().y,
         obj.bbox.Get_Orientation(),
         det_props[i].vcs_position.x,
         det_props[i].vcs_position.y);
   }
   exp_pseudo_pos_x = 2.5F;
   exp_pseudo_pos_y = 1.5F;
   exp_pseudo_pos_x_var = 1.18656504F;
   exp_pseudo_pos_y_var = 1.07379186F;
   exp_pseudo_pos_cov_xy = 0.105724864F;

   /** \action
    * Call function
    */
   Pseudo_Position_Estimation(
      calibrations,
      host,
      det_props,
      sensors,
      globals,
      obj);

   /** \result
    * Expect pseudo position to be the same as before function call
    * Expect the variance from previous iteration to have been multiplied by k_gain_no_estimate
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, obj.pseudo_vcs_position.x, F360_EPSILON,
                      "The pseudo position in x direction did not match the expected value when mean calculation failed.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, obj.pseudo_vcs_position.y, F360_EPSILON,
                      "The pseudo position in y direction did not match the expected value when mean calculation failed.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x_var, obj.meascov[0][0], F360_EPSILON,
                      "The variance of the pseudo position in x direction did not match the expected value when mean calculation failed.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y_var, obj.meascov[1][1], F360_EPSILON,
                      "The variance of the pseudo position in y direction did not match the expected value when mean calculation failed.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, obj.meascov[0][1], F360_EPSILON,
                      "The covariance between x component and y component of the pseudo position is not correct.");
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, obj.meascov[1][0], F360_EPSILON,
                      "The covariance between x component and y component of the pseudo position is not correct.");

}

/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Compute_Pseudo_Pos_Mid_Point_Of_Detections
 *  @{
 */

/** \brief
 * Testing of a function that calculates the mid point of a set of detections associated to an object.
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Mid_Point_Of_Detections)
{
   // Common variables used in tests
   F360_Object_Track_T obj;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];

   Point exp_mid_point;

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up 3 detections. 
    */
   TEST_SETUP()
   {
      obj.ndets = 3;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;
      obj.detids[2U] = 3U;

      det_props[0U].vcs_position.x = 10.0F;
      det_props[0U].vcs_position.y = -3.0F;
      det_props[1U].vcs_position.x = 13.0F;
      det_props[1U].vcs_position.y = 1.0F;
      det_props[2U].vcs_position.x = 9.0F;
      det_props[2U].vcs_position.y = 2.0F;
   }
};

/** \purpose
 * Test that correct mid points of a set of associated detections is returned. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Mid_Point_Of_Detections, Compute_Pseudo_Pos_Mid_Point_Of_Detections)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    */
   exp_mid_point.x = 0.5F * (9.0F + 13.0F);
   exp_mid_point.y = 0.5F * ((-3.0F) + 2.0F);

   /** \action
    * Call Compute_Pseudo_Pos_Mid_Point_Of_Detections
    */
   Point mid_point = Compute_Pseudo_Pos_Mid_Point_Of_Detections(obj, det_props);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_mid_point.x, mid_point.x, test_threshold, "Incorrect mid x returned.")
   DOUBLES_EQUAL_TEXT(exp_mid_point.y, mid_point.y, test_threshold, "Incorrect mid y returned.")
}


/** \defgroup  f360_min_max_pseudo_pos_estimation
 *  @{
 */

/** \brief
 * Testing of a function that estimates the position of an object based
 * on position of associated detections using the min/max or weighted average approach depending on object's visible side.
 */

TEST_GROUP(f360_min_max_pseudo_pos_estimation)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   float32_t assoc_dets_para_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float32_t assoc_dets_orth_pos_tcs[MAX_DETS_IN_OBJ_TRK];

   // Expected pseudo x and y position and corresponding variances
   float32_t exp_pseudo_pos_x = 0.0F;
   float32_t exp_pseudo_pos_y = 0.0F;

   Point pseudo_pos_tcs = {};

   /** \setup
    * Initialize object with an arbitrary size, centroid in object center and VCS position to 0
    * Set x pos and y pos part of meascov to zero
    * Set gains to same values as in source file
    */
   TEST_SETUP()
   {      
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);

      obj.vcs_position.x = 0.0F;
      obj.vcs_position.y = 0.0F;
      Point center = {0.0F,0.0F};
      obj.bbox.Set_Center(center);
      obj.reference_point = F360_REFERENCE_POINT_CENTER;

      obj.Set_Bbox_Orientation(Angle{ 0.0F });
   }

};

/**
*\purpose  Verify that correct pseudo position is set when front and right sides are visible.
*\req    NA
*/
TEST(f360_min_max_pseudo_pos_estimation, Pseudo_Position_Estimation_Pos_From_Front_Right_Sides)
{
   /** \precond
    * Set obj.ndets to 2
    * Set obj.reference_point to FRONT_RIGHT
    * Place a detection 0.3m away from edge in para and orth direction (in TCS)
    * Place a detection on object's reference position (in TCS)
    * Set expected pseudo x position to maximum x value of associated dets (in this case first detection)
    * Set expected pseudo y position to maximum y value of associated dets (in this case first detection)
    */
   obj.ndets = 2;
   obj.detids[0U] = 1U;
   obj.detids[1U] = 2U;
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   obj.vcs_position = obj.bbox.Get_Corners().Front_Right();

   assoc_dets_para_pos_tcs[0] = 0.5F * obj.bbox.Get_Length() + 0.3F;
   assoc_dets_orth_pos_tcs[0] = 0.5F * obj.bbox.Get_Width() + 0.3F;
   assoc_dets_para_pos_tcs[1] = 2.0F;
   assoc_dets_orth_pos_tcs[1] = 1.0F;
   
   exp_pseudo_pos_x = assoc_dets_para_pos_tcs[0];
   exp_pseudo_pos_y = assoc_dets_orth_pos_tcs[0];

   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   /** \result
    * Expect pseudo position to have moved the centroid 0.3m in para and in orth direction respectively
    * Expect the pseudo position variance to be gain * 0.5 and gain * 0.2 in para and orth direction respectively
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_tcs.x, F360_EPSILON,
                      "The pseudo position in x direction did not match the expected value when front right side were visible.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_tcs.y, F360_EPSILON,
                      "The pseudo position in y direction did not match the expected value when front right side were visible.")
}

/**
*\purpose  Verify that correct pseudo position is set when rear and left sides are visible.
*\req    NA
*/
TEST(f360_min_max_pseudo_pos_estimation, Pseudo_Position_Estimation_Pos_From_Rear_Left_Sides)
{
   /** \precond
    * Set obj.ndets to 2
    * Set obj.reference_point to REAR_LEFT
    * Set first detection position to 0.1m away from edge in each direction.
    * Set second detection position to (-3, -1) m
    * Set weights in para and orth direction to 1.0F (this will also be normalized weight as ndets = 1)
    * Set expected pseudo x position to maximum x value of associated dets (in this case first detection)
    * Set expected pseudo y position to maximum y value of associated dets (in this case first detection)
    */
   obj.ndets = 2;
   obj.detids[0U] = 1U;
   obj.detids[1U] = 2U;
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   obj.vcs_position = obj.bbox.Get_Corners().Rear_Left();

   assoc_dets_para_pos_tcs[0] = -0.5F * obj.bbox.Get_Length() - 0.1F;
   assoc_dets_orth_pos_tcs[0] = -0.5F * obj.bbox.Get_Width() - 0.1F;
   assoc_dets_para_pos_tcs[1] = -3.0F;
   assoc_dets_orth_pos_tcs[1] = -1.0F;
   exp_pseudo_pos_x = -3.0F;
   exp_pseudo_pos_y = -1.1F;

   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   /** \result
    * Expect pseudo position to have moved the centroid 0.1m in para and in orth direction respectively
    * Expect the pseudo position variance to be gain * 0.1 and gain * 0.7 in para and orth direction respectively
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_tcs.x, F360_EPSILON,
                      "The pseudo position in x direction did not match the expected value when front right side were visible.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_tcs.y, F360_EPSILON,
                      "The pseudo position in y direction did not match the expected value when front right side were visible.")
}

/** \defgroup  f360_pseudo_position_estimation_Only_One_Side_Visible
 *  @{
 */

 /** \brief
  * Test setup to test that the correct pseudo position is calculated when only one edge of the object is seen.
  */
TEST_GROUP(f360_pseudo_position_estimation_Min_Max_Only_One_Side_Visible)
{
   F360_Object_Track_T obj = {};
   float32_t assoc_dets_para_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float32_t assoc_dets_orth_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   Point pseudo_pos_tcs = {};

   float32_t exp_pseudo_pos_x;
   float32_t exp_pseudo_pos_y;

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with LEFT as reference point, positioned at (-2,5).
    * Set up 3 associated detections positioned close to the object's left edge (close enough to each other to have Huber weights == 1)
    */
   TEST_SETUP()
   {
      obj.vcs_position.x = -2.0F;
      obj.vcs_position.y = 5.0F;
      obj.reference_point = F360_REFERENCE_POINT_LEFT;
      obj.bbox.Set_Length(6.0F);
      obj.bbox.Set_Width(2.0F);
      obj.bbox.Set_Orientation(0.0F);
      obj.Update_Bbox_Center();
      obj.ndets = 3;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;
      obj.detids[2U] = 3U;

      assoc_dets_para_pos_tcs[0] = 0.0F;
      assoc_dets_orth_pos_tcs[0] = -1.0F;
      assoc_dets_para_pos_tcs[1] = 2.0F;
      assoc_dets_orth_pos_tcs[1] = -1.1F;
      assoc_dets_para_pos_tcs[2] = -3.0F;
      assoc_dets_orth_pos_tcs[2] = -0.8F;
   }
};

/**
*\purpose  Verify that the correct pseudo position is calculated when object's reference point is LEFT
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Min_Max_Only_One_Side_Visible, Estimate_Para_Only_Left_Side_Visible)
{
   /** \precond
    * An object with ref point LEFT at (-2,5) is set up in the test group
    * Expected pseudo pos x is average of the detection xpos in TCS (in this example they're close enough to all have weight = 1)
    * Expected pseudo pos y is extreme value of detections ypos in TCS since right side of object is visible
   */
   exp_pseudo_pos_x = -7.0F / 3.0F;
   exp_pseudo_pos_y = 4.9F;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   Point pseudo_pos_vcs = {};
   Convert_TCS_Posn_To_VCS_Posn(
      pseudo_pos_tcs.x,
      pseudo_pos_tcs.y,
      obj.bbox.Get_Center().x,
      obj.bbox.Get_Center().y,
      obj.bbox.Get_Orientation(),
      pseudo_pos_vcs.x,
      pseudo_pos_vcs.y);

   /** \result
    * Check that calculated pseudo pos corresponds to expected one.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_vcs.x, test_threshold,"Pseudo pos x is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_vcs.y, test_threshold,"Pseudo pos y is incorrect.");
}

/**
*\purpose  Verify that the correct pseudo position is calculated when object's reference point is RIGHT
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Min_Max_Only_One_Side_Visible, Estimate_Para_Only_Right_Side_Visible)
{
   /** \precond
    * An object with ref point RIGHT at (0,-10) is set up
    * Place 3 detections along the right edge of the object
    * Expected pseudo pos x is average of the detection xpos in TCS (in this example they're close enough to all have weight = 1)
    * Expected pseudo pos y is extreme value of detections ypos in TCS since right side of object is visible
   */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   obj.vcs_position = {0.0F, -10.0F};
   obj.Update_Bbox_Center();

   assoc_dets_para_pos_tcs[0] = 1.0F;
   assoc_dets_orth_pos_tcs[0] = 1.1F;
   assoc_dets_para_pos_tcs[1] = -4.0F;
   assoc_dets_orth_pos_tcs[1] = 0.9F;
   assoc_dets_para_pos_tcs[2] = 0.0F;
   assoc_dets_orth_pos_tcs[2] = 1.0F;

   exp_pseudo_pos_x = -1.0F;
   exp_pseudo_pos_y = -9.9F;
   
   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   Point pseudo_pos_vcs = {};
   Convert_TCS_Posn_To_VCS_Posn(
      pseudo_pos_tcs.x,
      pseudo_pos_tcs.y,
      obj.bbox.Get_Center().x,
      obj.bbox.Get_Center().y,
      obj.bbox.Get_Orientation(),
      pseudo_pos_vcs.x,
      pseudo_pos_vcs.y);

   /** \result
    * Check that calculated pseudo pos corresponds to expected one.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_vcs.x, test_threshold,"Pseudo pos x is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_vcs.y, test_threshold,"Pseudo pos y is incorrect.");
}

/**
*\purpose  Verify that the correct pseudo position is calculated when object's reference point is FRONT
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Min_Max_Only_One_Side_Visible, Estimate_Para_Only_Front_Side_Visible)
{
   /** \precond
    * Set object ref point to FRONT at position (-10,0)
    * Set up 3 associated detections around the objec't front edge
    * Expected pseudo pos y is average of the detection ypos in TCS (in this example they're close enough to all have weight = 1)
    * Expected pseudo pos x is extreme value of detections xpos in TCS since right side of object is visible
   */
   obj.vcs_position = {-10.0F, 0.0F};
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   obj.Update_Bbox_Center();

   assoc_dets_para_pos_tcs[0] = 2.0F;
   assoc_dets_orth_pos_tcs[0] = -1.0F;
   assoc_dets_para_pos_tcs[1] = 2.0F;
   assoc_dets_orth_pos_tcs[1] = 0.0F;
   assoc_dets_para_pos_tcs[2] = 2.0F;
   assoc_dets_orth_pos_tcs[2] = 2.0F;
   exp_pseudo_pos_x = -11.0F;
   exp_pseudo_pos_y = 0.33333F;
   
   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   Point pseudo_pos_vcs = {};
   Convert_TCS_Posn_To_VCS_Posn(
      pseudo_pos_tcs.x,
      pseudo_pos_tcs.y,
      obj.bbox.Get_Center().x,
      obj.bbox.Get_Center().y,
      obj.bbox.Get_Orientation(),
      pseudo_pos_vcs.x,
      pseudo_pos_vcs.y);

   /** \result
    * Check that calculated pseudo pos corresponds to expected one.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_vcs.x, test_threshold,"Pseudo pos x is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_vcs.y, test_threshold,"Pseudo pos y is incorrect.");
}

/**
*\purpose  Verify that the correct pseudo position is calculated when object's reference point is REAR
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Min_Max_Only_One_Side_Visible, Estimate_Para_Only_Rear_Side_Visible)
{
   /** \precond
    * Set object ref point to REAR at position (10,0)
    * Set up 3 associated detections along the objec't rear edge
    * Expected pseudo pos y is average of the detection ypos in TCS (in this example they're close enough to all have weight = 1)
    * Expected pseudo pos x is extreme value of detections xpos in TCS since right side of object is visible
   */
   obj.vcs_position = {10.0F, 0.0F};
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   obj.Update_Bbox_Center();

   assoc_dets_para_pos_tcs[0] = -3.0F;
   assoc_dets_orth_pos_tcs[0] = -3.0F;
   assoc_dets_para_pos_tcs[1] = -3.0F;
   assoc_dets_orth_pos_tcs[1] = -2.0F;
   assoc_dets_para_pos_tcs[2] = -3.0F;
   assoc_dets_orth_pos_tcs[2] = -1.0F;
   exp_pseudo_pos_x = 10.0F;
   exp_pseudo_pos_y = -2.0F;
   
   /** \action
    * Call function
    */
   pseudo_pos_tcs = Compute_Pseudo_Pos_TCS_For_Extended_Object_Case_Min_Max_Weighted_Average(
      rear_front_side, 
      right_left_side, 
      assoc_dets_para_pos_tcs, 
      assoc_dets_orth_pos_tcs, 
      obj.ndets);

   Point pseudo_pos_vcs = {};
   Convert_TCS_Posn_To_VCS_Posn(
      pseudo_pos_tcs.x,
      pseudo_pos_tcs.y,
      obj.bbox.Get_Center().x,
      obj.bbox.Get_Center().y,
      obj.bbox.Get_Orientation(),
      pseudo_pos_vcs.x,
      pseudo_pos_vcs.y);

   /** \result
    * Check that calculated pseudo pos corresponds to expected one.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_x, pseudo_pos_vcs.x, test_threshold,"Pseudo pos x is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_y, pseudo_pos_vcs.y, test_threshold,"Pseudo pos y is incorrect.");
}
/** @}*/


/** \defgroup  f360_pseudo_position_estimation_check_Adjust_Pseudo_Cov_TCS_Wrt_Visibility_is_called
 *  @{
 */

/** \brief
 * Verify that Adjust_Pseudo_Cov_TCS_Wrt_Visibility() is called as expected in Pseudo_Position_Estimation()
 */

TEST_GROUP(f360_pseudo_position_estimation_check_Adjust_Pseudo_Cov_TCS_Wrt_Visibility_is_called)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Radar_Sensor_T sensor[MAX_NUMBER_OF_SENSORS] = {};
   F360_Globals_T globals = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};

   /** \setup
    * Set calibrations.k_pseudo_pos_high_uncertainity to 1337
    * Set obj.ndets to 1
    * Set obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT
    * Set obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT
    * Set all entrie in obj.meascov to 0
    */
   TEST_SETUP()
   {
      calibrations.k_pseudo_pos_high_uncertainity = 1337.0F;

      calibrations.k_pseudo_pos_cov_matrix_bias = 1.0F;
      calibrations.k_pseudo_pos_max_variance_threshold = 1.0F;
      
      obj.ndets = 1;
      obj.detids[0U] = 1U;
      obj.bbox.Set_Length(2.0F);
      obj.bbox.Set_Width(2.0F);
      
      obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
      obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT;

      obj.meascov[0][0] = 0.0F;
      obj.meascov[0][1] = 0.0F;
      obj.meascov[1][0] = 0.0F;
      obj.meascov[1][1] = 0.0F;
   }
};

/**
*\purpose  Verify that Adjust_Pseudo_Cov_TCS_Wrt_Visibility is called for moveable objects
*\req    NA
*/
TEST(f360_pseudo_position_estimation_check_Adjust_Pseudo_Cov_TCS_Wrt_Visibility_is_called, Moveable_Object)
{
   /** \precond
    * Set obj.f_moveable to true
    */
   obj.f_moveable = true;

   /** \action
    * Call function
    */
   Pseudo_Position_Estimation(
      calibrations,
      host,
      det_props,
      sensor,
      globals,
      obj);

   /** \result
    * Verify obj.meascov[0][0] and obj.meascov[1][1] is set to 1337
    * Verify obj.meascov[0][1] and obj.meascov[1][0] is set to 0
    */
   DOUBLES_EQUAL(1337.0F, obj.meascov[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, obj.meascov[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, obj.meascov[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, obj.meascov[1][1], F360_EPSILON)
}

/**
*\purpose  Verify that Adjust_Pseudo_Cov_TCS_Wrt_Visibility is not called for non-moveable objects
*\req    NA
*/
TEST(f360_pseudo_position_estimation_check_Adjust_Pseudo_Cov_TCS_Wrt_Visibility_is_called, Nonmoveable_Object)
{
   /** \precond
    * Set obj.f_moveable to false
    */
   obj.f_moveable = false;

   /** \action
    * Call function
    */
   Pseudo_Position_Estimation(
      calibrations,
      host,
      det_props,
      sensor,
      globals,
      obj);

   /** \result
    * Verify obj.meascov[0][0] is set to 1
    * Verify obj.meascov[1][1] is set to 1
    * Verify obj.meascov[0][1] and obj.meascov[1][0] is set to 0
    */
   DOUBLES_EQUAL(1.0F, obj.meascov[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, obj.meascov[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, obj.meascov[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1.0F, obj.meascov[1][1], F360_EPSILON)
}


/** \defgroup  f360_pseudo_position_estimation_Raw_Pseudo_Pos_Cov
 *  @{
 */

 /** \brief
  * Test setup to test that the raw pseudo covariance for position in TCS is computed as expected.
  */
TEST_GROUP(f360_pseudo_position_estimation_Raw_Pseudo_Pos_Cov)
{
   F360_Object_Track_T obj = {};
   F360_Host_T host = {};
   F360_Calibrations_T calibs;
   float32_t pseudo_cov_tcs[2][2] = {};
   float32_t exp_pseudo_pos_cov_x;
   float32_t exp_pseudo_pos_cov_y;
   float32_t exp_pseudo_pos_cov_xy;

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with REAR LEFT as reference point, positioned at (10,5).
    * Pseudo position is placed slightly behind rear left corner at (9.8, 4.9)
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      host.dist_rear_axle_to_vcs_m = 3.0F;
      obj.vcs_position.x = 10.0F;
      obj.vcs_position.y = 5.0F;
      obj.bbox.Set_Length(6.0F);
      obj.bbox.Set_Width(2.0F);
      obj.bbox.Set_Orientation(0.0F);
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;

      obj.pseudo_vcs_position.x = 9.8F;
      obj.pseudo_vcs_position.y = 4.9F;
   }
};

/**
*\purpose  Verify that the correct raw pseudo position cov is calculated when object's range is within max range
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Raw_Pseudo_Pos_Cov, Raw_Pseudo_Pos_Cov_Range_Below_Max)
{
   /** \precond
    * An object with ref point REAR LEFT at (10,5) is set up in the test group
    * Since range is below max range, no range saturation is needed
    * Exp data is set up to reflect that.
   */
   exp_pseudo_pos_cov_x = 0.2397F;
   exp_pseudo_pos_cov_y = 0.1952F;
   exp_pseudo_pos_cov_xy = 0.0238F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Compute_Raw_Pseudo_Pos_Cov_In_TCS(
      obj,
      host,
      pseudo_cov_tcs,
      calibs);

   /** \result
    * Check that calculated raw pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
*\purpose  Verify that the correct raw pseudo position cov is calculated when object's range is above max range
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Raw_Pseudo_Pos_Cov, Raw_Pseudo_Pos_Cov_Range_Above_Max)
{
   /** \precond
    * Set up an object with ref point REAR LEFT
    * Set position to (100, 60)
    * Set pseudo pos close to ref point at (99, 59)
    * Since range is above max range, range saturation is needed
    * Exp data is set up to reflect that.
   */
   obj.vcs_position.x = 100.0F;
   obj.vcs_position.y = 60.0F;
   obj.pseudo_vcs_position.x = 99.0F;
   obj.pseudo_vcs_position.y = 59.0F;
   exp_pseudo_pos_cov_x = 3.3090F;
   exp_pseudo_pos_cov_y = 9.1257F;
   exp_pseudo_pos_cov_xy = -5.2106F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Compute_Raw_Pseudo_Pos_Cov_In_TCS(
      obj,
      host,
      pseudo_cov_tcs,
      calibs);

   /** \result
    * Check that calculated raw pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/** \defgroup  f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov
 *  @{
 */

 /** \brief
  * Test that the pseudo covariance for position in TCS is adjusted as expected.
  */
TEST_GROUP(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov)
{
   F360_Calibrations_T calibs = {};
   F360_Object_Track_T obj = {};
   F360_Object_Sides_T rear_front_side = {};
   F360_Object_Sides_T right_left_side = {};
   Point pseudo_tcs_position = {};
   float32_t pseudo_cov_tcs[2][2] = {};

   float32_t exp_pseudo_pos_cov_x;
   float32_t exp_pseudo_pos_cov_y;
   float32_t exp_pseudo_pos_cov_xy;

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with REAR LEFT as reference point, positioned at (10,5)
    * Pseudo position is placed behind rear left corner at (9, 4), i.e. (-3, -2) in TCS
    * Set object filter type to CCV.
    * Initialize elements of pseudo_cov_tcs to arbitrary non-zero values. 
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      obj.vcs_position.x = 10.0F;
      obj.vcs_position.y = 5.0F;
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.bbox.Set_Orientation(0.0F);
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      rear_front_side = F360_OBJECT_SIDES_REAR;
      right_left_side = F360_OBJECT_SIDES_LEFT;
      pseudo_tcs_position = {-3.0F, -2.0F};
      obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCV;

      pseudo_cov_tcs[0][0] = 2.0F;
      pseudo_cov_tcs[1][0] = 3.0F;
      pseudo_cov_tcs[0][1] = 3.0F;
      pseudo_cov_tcs[1][1] = 4.0F;
   }
};

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group
    *    - with pseudo pos close to ref point at (9, 4)
    *    - with object sides REAR and LEFT visible
   */
   exp_pseudo_pos_cov_x = 3.0F;
   exp_pseudo_pos_cov_y = 5.0F;
   exp_pseudo_pos_cov_xy = 3.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear)
{
   /** \precond
    * Set object ref point to REAR
    * Set object position at (10, 0)
    * Set pseudo pos close to ref point at (9, 0), i.e (-3, 0) in TCS
    * Object front/rear side is set to REAR visible in test group
    * Set object right/left side invalid
   */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   right_left_side = F360_OBJECT_SIDES_INVALID;
   obj.vcs_position = {10.0F, 0.0F};
   pseudo_tcs_position = {-1.0F, 0.0F};
   exp_pseudo_pos_cov_x = 3.0F;
   exp_pseudo_pos_cov_y = 8.0F;
   exp_pseudo_pos_cov_xy = 3.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated raw pseudo pos cov values correspond to exped ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Left)
{
   /** \precond
    * Set object ref point to LEFT
    * Set object position at (-2, 6)
    * Set pseudo pos close to ref point at (-2, 5), i.e (0, -2) in TCS
    * Set object front/rear side to invalid
    * Set object right/left side LEFT visible
   */
   obj.reference_point = F360_REFERENCE_POINT_LEFT;
   rear_front_side = F360_OBJECT_SIDES_INVALID;
   right_left_side = F360_OBJECT_SIDES_LEFT;
   obj.vcs_position = {-1.0F, 6.0F};
   pseudo_tcs_position = {0.0F, -2.0F};
   exp_pseudo_pos_cov_x = 18.0F;
   exp_pseudo_pos_cov_y = 5.0F;
   exp_pseudo_pos_cov_xy = 3.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is CENTER
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Center)
{
   /** \precond
    * Set object ref point to CENTER
    * Set object position at (-2, 6)
    * Set pseudo pos close to ref point at (-1, 6), i.e (0, -2) in TCS
    * Set object front/rear side to invalid
    * Set object right/left side to invalid
   */
   obj.reference_point = F360_REFERENCE_POINT_CENTER;
   rear_front_side = F360_OBJECT_SIDES_INVALID;
   right_left_side = F360_OBJECT_SIDES_INVALID;
   obj.vcs_position = {-1.0F, 6.0F};
   pseudo_tcs_position = {0.0F, -1.0F};
   exp_pseudo_pos_cov_x = 3.0F;
   exp_pseudo_pos_cov_y = 5.0F;
   exp_pseudo_pos_cov_xy = 3.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT and pseudo pos cov
*          values are above maximum, such that they get saturated.
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left_Saturation)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group
    * Set pseudo pos close to ref point at (9, 4)
    * Set object sides REAR and LEFT visible
    * Set input pseudo pos cov elements close to max threshold such that they exceed it when updated and get saturated.
   */
   pseudo_cov_tcs[0][0] = calibs.k_pseudo_pos_max_variance_threshold - 0.1F;
   pseudo_cov_tcs[1][1] = calibs.k_pseudo_pos_max_variance_threshold - 0.2F;
   pseudo_cov_tcs[1][0] = calibs.k_pseudo_pos_max_variance_threshold - 0.3F;
   pseudo_cov_tcs[0][1] = pseudo_cov_tcs[1][0];

   exp_pseudo_pos_cov_x = calibs.k_pseudo_pos_max_variance_threshold;
   exp_pseudo_pos_cov_y = calibs.k_pseudo_pos_max_variance_threshold;
   exp_pseudo_pos_cov_xy = 28.8817F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_x, pseudo_cov_tcs[0][0], test_threshold, "The pseudo position cov in para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[1][0], test_threshold, "The pseudo position cov in para-orth direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_xy, pseudo_cov_tcs[0][1], test_threshold, "The pseudo position cov in orth_para direction did not match the expected value.")
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_cov_y, pseudo_cov_tcs[1][1], test_threshold, "The pseudo position cov in orth direction did not match the expected value.")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT and object is CTCA
*          such that extra covariance is added when object position and pseudo position differ (outlier handling).
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left_CTCA_Outlier_Handling)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group with
    *    - Pseudo pos close to ref point at (9, 4), i.e. (-3, -2) in TCS
    *    - Object sides REAR and LEFT visible
    * Set object filter type to CTCA
   */
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   float32_t pseudo_pos_cov_x_before_outlier_handling = 3.0F;
   float32_t pseudo_pos_cov_y_before_outlier_handling = 5.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   CHECK_FALSE_TEXT(pseudo_cov_tcs[0][0] == pseudo_pos_cov_x_before_outlier_handling, "No extra cov was addded to para pseudo pos cov")
   CHECK_FALSE_TEXT(pseudo_cov_tcs[1][1] == pseudo_pos_cov_y_before_outlier_handling, "No extra cov was addded to orth pseudo pos cov")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT and object is CCA with
*          large time_since_init such that extra covariance is added when object position and pseudo position differ (outlier handling).
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left_CCA_Outlier_Handling_When_Large_Time_Since_Init)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group with
    *    - Pseudo pos close to ref point at (9, 4), i.e. (-3, -2) in TCS
    *    - Object sides REAR and LEFT visible
    * Set object filter type to CCA
    * Set time_since_initialization larger than calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca (calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca + 1e-3 is used in the test)
    * Set speed larger than calibs.fast_moving_thresh
   */
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj.time_since_initialization = calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca + 1e-6F;
   float32_t pseudo_pos_cov_x_before_outlier_handling = 3.0F;
   float32_t pseudo_pos_cov_y_before_outlier_handling = 5.0F;
   obj.speed = calibs.fast_moving_thresh + 0.1F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   CHECK_TRUE_TEXT(std::abs(pseudo_cov_tcs[0][0] - pseudo_pos_cov_x_before_outlier_handling) > F360_EPSILON, "No extra cov was addded to para pseudo pos cov")
   CHECK_TRUE_TEXT(std::abs(pseudo_cov_tcs[1][1] - pseudo_pos_cov_y_before_outlier_handling) > F360_EPSILON, "No extra cov was addded to orth pseudo pos cov")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT and object is CCA with
*          small time_since_init such that no extra covariance is added when object position and pseudo position differ (outlier handling).
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left_CCA_Outlier_Handling_When_Small_Time_Since_Init)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group with
    *    - Pseudo pos close to ref point at (9, 4), i.e. (-3, -2) in TCS
    *    - Object sides REAR and LEFT visible
    * Set object filter type to CCA
    * Set time_since_initialization smaller than calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca (calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca + 1e-3 is used in the test)
    * Set speed larger than calibs.fast_moving_thresh
   */
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj.time_since_initialization = calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca - 1e-6F;
   float32_t pseudo_pos_cov_x_before_outlier_handling = 3.0F;
   float32_t pseudo_pos_cov_y_before_outlier_handling = 5.0F;
   obj.speed = calibs.fast_moving_thresh + 0.1F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   CHECK_FALSE_TEXT(std::abs(pseudo_cov_tcs[0][0] - pseudo_pos_cov_x_before_outlier_handling) > F360_EPSILON, "Extra cov was unexpectedly addded to para pseudo pos cov")
   CHECK_FALSE_TEXT(std::abs(pseudo_cov_tcs[1][1] - pseudo_pos_cov_y_before_outlier_handling) > F360_EPSILON, "Extra cov was unexpectedly addded to para pseudo pos cov")
}

/**
*\purpose  Verify that the correct adjusted pseudo position cov is calculated when object's ref point is REAR LEFT and object is CCA with
*          small time_since_init such that extra covariance is added when object position and pseudo position differ (outlier handling).
*\req    NA
*/
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Pos_Cov, Adjust_Pseudo_Cov_TCS_Rear_Left_CCA_No_Outlier_Handling_When_Small_Time_Since_Init)
{
   /** \precond
    * An object with ref point REAR LEFT at (10, 5) is set up in the test group with
    *    - Pseudo pos close to ref point at (9, 4), i.e. (-3, -2) in TCS
    *    - Object sides REAR and LEFT visible
    * Set object filter type to CCA
    * Set time_since_initialization smaller than calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca (calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca - 1e-3 is used in the test)
   */
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj.time_since_initialization = calibs.k_time_since_init_th_to_enable_outlier_mitigation_cca - 1e-6F;
   float32_t pseudo_pos_cov_x_before_outlier_handling = 3.0F;
   float32_t pseudo_pos_cov_y_before_outlier_handling = 5.0F;
   
   /** \action
    * Call function Compute_Raw_Pseudo_Pos_Cov_In_TCS()
    */
   Adjust_Pseudo_Cov_TCS(
      obj,
      calibs,
      pseudo_tcs_position,
      pseudo_cov_tcs);

   /** \result
    * Check that calculated adjusted pseudo pos cov values correspond to expected ones
    */
   CHECK_FALSE_TEXT(std::abs(pseudo_cov_tcs[0][0] - pseudo_pos_cov_x_before_outlier_handling) > F360_EPSILON, "Extra cov was unexpectedly addded to para pseudo pos cov")
   CHECK_FALSE_TEXT(std::abs(pseudo_cov_tcs[1][1] - pseudo_pos_cov_y_before_outlier_handling) > F360_EPSILON, "Extra cov was unexpectedly addded to orth pseudo pos cov")
}
/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS
 *  @{
 */

 /** \brief
  * Test that the pseudo covariance increase for position in TCS is computed as expected.
  */
TEST_GROUP(f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS)
{
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibrations = {};
   Point centroid_pseudo_pos_tcs = {};
   F360_Object_Sides_T rear_front_side = {};
   F360_Object_Sides_T right_left_side = {};
   Point obj_var_tcs = {};
   float32_t pseudo_cov_tcs[2][2] = {};

   float32_t diff_pos = 0.0F;
   float32_t cov_pos_inc = 0.0F;

   float32_t exp_pseudo_cov_tcs[2][2] = {};
   float32_t exp_cov_pos_inc = 0.0F;

   float32_t k_gain_weighted_mean;
   float32_t k_gain_simple_estimate;
   float32_t k_bias_para;
   float32_t k_bias_orth;
   float32_t k_max_variance_threshold;
   /** \setup
    * Set up input parameters to relevant functions Adjust_Pseudo_Cov_TCS() and Compute_Pos_Cov_Inc_In_TCS().
    * Values are chosen in such a way that no edge has been correctly estimated (rear_front_side = right_left_side = F360_OBJECT_SIDES_INVALID).
    */
   TEST_SETUP()
   {
      // Set up a default scenario for your tests. E.g. assign values to common variables declared above.
      Initialize_Tracker_Calibrations(calibrations);

      obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj.bbox.Set_Length(5.0F);
      obj.bbox.Set_Width(2.0F);
      obj.reference_point = F360_REFERENCE_POINT_CENTER;
      obj.vcs_position = obj.bbox.Get_Center();

      centroid_pseudo_pos_tcs.x = -0.89F;
      centroid_pseudo_pos_tcs.y = -0.89F;

      rear_front_side = F360_OBJECT_SIDES_INVALID;
      right_left_side = F360_OBJECT_SIDES_INVALID;
      // Let pseudo_cov_tcs be initialized to zero when both rear_front_side and right_left_side are F360_OBJECT_SIDES_INVALID

      obj_var_tcs.x = 2.0F;
      obj_var_tcs.y = 1.0F;

      k_gain_weighted_mean = calibrations.k_pseudo_pos_gain_weighted_mean;
      k_gain_simple_estimate = calibrations.k_pseudo_pos_gain_simple_estimate;
      k_bias_para = calibrations.k_pseudo_pos_var_bias_para;
      k_bias_orth = calibrations.k_pseudo_pos_var_bias_orth;
      k_max_variance_threshold = calibrations.k_pseudo_pos_max_variance_threshold;
   }
};

/** \purpose
 * Check that Compute_Pos_Cov_Inc_In_TCS() returns the expected value when the distance between pseudo position and time predicted position is smaller
 * than the threshold (positive value).
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS, Compute_Pos_Cov_Inc_In_TCS__Short_Dist_Positive)
{
   /** \precond
    * Set position difference to something smaller than threshold.
    * Specify expected output from function.
    */
   diff_pos = 0.89F;
   exp_cov_pos_inc = 0.0F;

   /** \action
    * Call Compute_Pos_Cov_Inc_In_TCS()
    */
   cov_pos_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos);

   /** \result
    * Check that covariance increase is computed as expected.
    */
   DOUBLES_EQUAL_TEXT(exp_cov_pos_inc, cov_pos_inc, F360_EPSILON, "Covariance increase was not computed as expected")
}

/** \purpose
 * Check that Compute_Pos_Cov_Inc_In_TCS() returns the expected value when the distance between pseudo position and time predicted position is smaller
 * than the threshold (negative value).
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS, Compute_Pos_Cov_Inc_In_TCS__Short_Dist_Negative)
{
   /** \precond
    * Set position difference to something negative and, in absolute value, smaller than threshold.
    * Specify expected output from function.
    */
   diff_pos = -0.89F;
   exp_cov_pos_inc = 0.0F;

   /** \action
    * Call Compute_Pos_Cov_Inc_In_TCS()
    */
   cov_pos_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos);

   /** \result
    * Check that covariance increase is computed as expected.
    */
   DOUBLES_EQUAL_TEXT(exp_cov_pos_inc, cov_pos_inc, F360_EPSILON, "Covariance increase was not computed as expected")
}

/** \purpose
 * Check that Compute_Pos_Cov_Inc_In_TCS() returns the expected value when the distance between pseudo position and time predicted position is larger
 * than the threshold.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS, Compute_Pos_Cov_Inc_In_TCS__Large_Dist_Positive)
{
   /** \precond
    * Set position difference to something larger than threshold.
    * Specify expected output from function.
    */
   diff_pos = 0.91F;
   exp_cov_pos_inc = 0.05025089F;

   /** \action
    * Call Compute_Pos_Cov_Inc_In_TCS()
    */
   cov_pos_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos);

   /** \result
    * Check that covariance increase is computed as expected.
    */
   DOUBLES_EQUAL_TEXT(exp_cov_pos_inc, cov_pos_inc, F360_EPSILON, "Covariance increase was not computed as expected")
}

/** \purpose
 * Check that Compute_Pos_Cov_Inc_In_TCS() returns the expected value when the distance between pseudo position and time predicted position is larger
 * than the threshold (negative value).
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Pseudo_Cov_Inc_TCS, Compute_Pos_Cov_Inc_In_TCS__Large_Dist_Negative)
{
   /** \precond
    * Set position difference to something negative and, in absolute value, larger than threshold.
    * Specify expected output from function.
    */
   diff_pos = -0.91F;
   exp_cov_pos_inc = 0.05025089F;

   /** \action
    * Call Compute_Pos_Cov_Inc_In_TCS()
    */
   cov_pos_inc = Compute_Pos_Cov_Inc_In_TCS(calibrations, diff_pos);

   /** \result
    * Check that covariance increase is computed as expected.
    */
   DOUBLES_EQUAL_TEXT(exp_cov_pos_inc, cov_pos_inc, F360_EPSILON, "Covariance increase was not computed as expected")
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Para
 *  @{
 */

 /** \brief
  * Test that the simplified calculation of reference para pos is done correctly.
  */
TEST_GROUP(f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Para)
{
   uint32_t ndets;
   Point assoc_dets_pos_tcs[MAX_DETS_IN_OBJ_TRK] = {};
   float assoc_dets_para_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float assoc_dets_orth_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float32_t ref_orth_pos;

   float32_t simplified_para_pos;
   float32_t exp_para_pos;

   const float32_t k_huber_threshold = 0.4F;
   const float32_t test_thresh = 0.001F;
   /** \setup
    * Set up a default scenario with 3 detections
    * ref_orth_pos is the para position of the detection with the smallest para position
    */
   TEST_SETUP()
   {
      ndets = 3U;
      assoc_dets_pos_tcs[0] = {1.0F, 3.0F};
      assoc_dets_pos_tcs[1] = {2.0F, 3.3F};
      assoc_dets_pos_tcs[2] = {3.0F, 7.0F};

      for (uint32_t i = 0; i < ndets; i++)
      {
         assoc_dets_para_pos_tcs[i] = assoc_dets_pos_tcs[i].x;
         assoc_dets_orth_pos_tcs[i] = assoc_dets_pos_tcs[i].y;
      }

      ref_orth_pos = assoc_dets_pos_tcs[0].y;
   }
};

/** \purpose
 * Check that Calc_Simplified_Pseudo_Pos_Para() returns the expected value.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Para, Calc_Simplified_Pseudo_Pos_Para_3_Dets)
{
   /** \precond
    * 3 detections have been set up in TEST_GROUP
    * Set expected para pos to weighted mean of detections para positions
    */
   
   exp_para_pos = 1.5714F;

   /** \action
    * Call Calc_Simplified_Pseudo_Pos_Para()
    */
   simplified_para_pos = Calc_Weighted_Average_Pseudo_Pos(ndets, assoc_dets_para_pos_tcs, assoc_dets_orth_pos_tcs, ref_orth_pos, k_huber_threshold);

   /** \result
    * Check that reference point para estimation is computed as expected
    */
   DOUBLES_EQUAL_TEXT(exp_para_pos, simplified_para_pos, test_thresh, "Simplified para pos is incorrect")
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Orth
 *  @{
 */

 /** \brief
  * Test that the simplified calculation of reference orth pos is done correctly.
  */
TEST_GROUP(f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Orth)
{
   uint32_t ndets;
   Point assoc_dets_pos_tcs[MAX_DETS_IN_OBJ_TRK] = {};
   float assoc_dets_para_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float assoc_dets_orth_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float32_t ref_para_pos;

   float32_t simplified_orth_pos;
   float32_t exp_orth_pos;

   const float32_t k_huber_threshold = 0.4F;
   const float32_t test_thresh = 0.001F;
   /** \setup
    * Set up a default scenario with 3 detections
    * ref_orth_pos is the para position of the detection with the smallest para position
    */
   TEST_SETUP()
   {
      ndets = 3U;
      assoc_dets_pos_tcs[0] = {5.0F, -1.0F};
      assoc_dets_pos_tcs[1] = {8.0F, 2.0F};
      assoc_dets_pos_tcs[2] = {5.1F, 3.0F};
      for (uint32_t i = 0; i < ndets; i++)
      {
         assoc_dets_para_pos_tcs[i] = assoc_dets_pos_tcs[i].x;
         assoc_dets_orth_pos_tcs[i] = assoc_dets_pos_tcs[i].y;
      }
      ref_para_pos = assoc_dets_pos_tcs[0].x;
   }
};

/** \purpose
 * Check that Calc_Simplified_Pseudo_Pos_Para() returns the expected value.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calc_Simplified_Pseudo_Pos_Orth, Calc_Simplified_Pseudo_Pos_Orth_3_Dets)
{
   /** \precond
    * 3 detections have been set up in TEST_GROUP
    * Set expected orth pos to weighted mean of detections orth positions
    */
   
   exp_orth_pos = 1.0624F;

   /** \action
    * Call Calc_Simplified_Pseudo_Pos_Para()
    */
   simplified_orth_pos = Calc_Weighted_Average_Pseudo_Pos(ndets, assoc_dets_orth_pos_tcs, assoc_dets_para_pos_tcs, ref_para_pos, k_huber_threshold);

   /** \result
    * Check that reference point orth estimation is computed as expected
    */
   DOUBLES_EQUAL_TEXT(exp_orth_pos, simplified_orth_pos, test_thresh, "Simplified orth pos is incorrect")
}
/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility
 *  @{
 */

 /** \brief
  * Test that the pseudo pos covariance is increased as expected depending on the
  * visibility of the object.
  */
TEST_GROUP(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility)
{
   F360_Object_Track_T obj = {};
   F360_Radar_Sensor_T sensor[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calibrations = {};
   F360_Globals_T globals = {};
   float32_t pseudo_pos_cov_tcs[2][2] = {};

   /** \setup
    * Reset the object by calling Clear_Object_Track(obj)
    * 
    */
   TEST_SETUP()
   {
      Clear_Object_Track(obj);

      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(4.0F);

      globals.f_single_front_center_radar_only = true;

      calibrations.k_pseudo_pos_high_uncertainity = 1337.0F;

      sensor[0].variable.is_valid = true;
      
      globals.rotated_left_fov_normal[0][0] = 1.0F; // unit vector pointing 0 deg vcs
      globals.rotated_left_fov_normal[0][1] = 0.0F;
      
      globals.rotated_right_fov_normal[0][0] = 1.0F; // unit vector pointing 0 deg vcs
      globals.rotated_right_fov_normal[0][1] = 0.0F;

      pseudo_pos_cov_tcs[0][0] = 0.0F;
      pseudo_pos_cov_tcs[0][1] = 0.0F;
      pseudo_pos_cov_tcs[1][0] = 0.0F;
      pseudo_pos_cov_tcs[1][1] = 0.0F;
   }
};

/** \purpose
 * Verify pseudo pos cov is not increased when reference point is left and both left front and left rear are inside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Left__Both_Left_Corners_Visible)
{
   /** \precond
    * Set reference point to left
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 0 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_LEFT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(0.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is left and rear left corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Left__Rear_Left_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to left
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to -45 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_LEFT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(-45.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is left and front left corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Left__Front_Left_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to left
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to -135 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_LEFT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(-135.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when reference point is right and both front right and rear right corners are inside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Right__Both_Right_Corners_Visible)
{
   /** \precond
    * Set reference point to rear
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 0 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(0.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is right and right rear corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Right__Rear_Right_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to right
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 45 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(45.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is right and front right corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Right__Front_Right_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to right
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 135 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(135.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when reference point is rear and both rear right and rear left corners are inside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear__Both_Rear_Corners_Visible)
{
   /** \precond
    * Set reference point to rear
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 0 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(0.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is rear and rear right corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear__Rear_Right_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to rear
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 45 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(45.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1s] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is rear and rear left corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear__Rear_Left_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to rear
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to -45 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(-45.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1s] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when reference point is front and both front right and front left corners are inside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front__Both_Front_Corners_Visible)
{
   /** \precond
    * Set reference point to front
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 0 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(0.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is front and front right corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front__Front_Right_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to front
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to 135 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(135.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when reference point is front and front left corner is outside sensor fov
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front__Front_Left_Corner_Not_Visible)
{
   /** \precond
    * Set reference point to front
    * Set object center vcs position to x = 2.1, y = 0
    * Set object orientation to -135 deg vcs
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.bbox.Set_Center(Point{2.1F,0.0F});
   obj.bbox.Set_Orientation(Angle{F360_DEG2RAD(-135.0F)});

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when "reference_point" and "min_projection_reference_point" are the same
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Left__No_Delta)
{
   /** \precond
    * Set reference point to front left
    * Set min projection reference point to front left
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs was changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Left__Delta_In_X_Dir)
{
   /** \precond
    * Set reference point to front left
    * Set min projection reference point to rear left
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Left__Delta_In_Y_Dir)
{
   /** \precond
    * Set reference point to front left
    * Set min projection reference point to front right
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x and y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Left__Delta_In_X_Y_Dir)
{
   /** \precond
    * Set reference point to front left
    * Set min projection reference point to rear right
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when "reference_point" and "min_projection_reference_point" are the same
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Left__No_Delta)
{
   /** \precond
    * Set reference point to rear left
    * Set min projection reference point to rear left
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs was changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Left__Delta_In_X_Dir)
{
   /** \precond
    * Set reference point to rear left
    * Set min projection reference point to front left
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Left__Delta_In_Y_Dir)
{
   /** \precond
    * Set reference point to rear left
    * Set min projection reference point to rear right
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x and y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Left__Delta_In_X_Y_Dir)
{
   /** \precond
    * Set reference point to rear left
    * Set min projection reference point to front right
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when "reference_point" and "min_projection_reference_point" are the same
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Right__No_Delta)
{
   /** \precond
    * Set reference point to front right
    * Set min projection reference point to front right
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs was changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Right__Delta_In_X_Dir)
{
   /** \precond
    * Set reference point to front right
    * Set min projection reference point to rear right
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Right__Delta_In_Y_Dir)
{
   /** \precond
    * Set reference point to front right
    * Set min projection reference point to front left
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x and y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Front_Right__Delta_In_X_Y_Dir)
{
   /** \precond
    * Set reference point to front right
    * Set min projection reference point to rear left
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is not increased when "reference_point" and "min_projection_reference_point" are the same
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Right__No_Delta)
{
   /** \precond
    * Set reference point to rear right
    * Set min projection reference point to rear left
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify no entry in pseudo_pos_cov_tcs was changed from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Right__Delta_In_X_Dir)
{
   /** \precond
    * Set reference point to rear right
    * Set min projection reference point to front right
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Right__Delta_In_Y_Dir)
{
   /** \precond
    * Set reference point to rear right
    * Set min projection reference point to rear left
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_REAR_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** \purpose
 * Verify pseudo pos cov is increased when "reference_point" and "min_projection_reference_point" differ in tcs x and y direction
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Adjust_Pseudo_Cov_TCS_Wrt_Visibility, Ref_Pnt_Rear_Right__Delta_In_X_Y_Dir)
{
   /** \precond
    * Set reference point to rear right
    * Set min projection reference point to front right
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_LEFT;

   /** \action
    * Call Adjust_Pseudo_Cov_TCS_Wrt_Visibility()
    */
   Adjust_Pseudo_Cov_TCS_Wrt_Visibility(obj, sensor, calibrations, globals, pseudo_pos_cov_tcs);
   
   /** \result
    * Verify pseudo_pos_cov_tcs[0][0] was set to 1337
    * Verify pseudo_pos_cov_tcs[1][1] was set to 1337
    * Verify the remaining entries in pseudo_pos_cov_tcs are unchanged from 0
    */
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[0][0], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[0][1], F360_EPSILON)
   DOUBLES_EQUAL(0.0F, pseudo_pos_cov_tcs[1][0], F360_EPSILON)
   DOUBLES_EQUAL(1337.0F, pseudo_pos_cov_tcs[1][1], F360_EPSILON)
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Create_Position_Grid
 *  @{
 */

/** \brief
 * Testing of a function that estimates the position of an object based
 * on position of associated detections using the grid search approach.
 */

TEST_GROUP(f360_pseudo_position_estimation_Create_Position_Grid)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibrations = {};
   float32_t assoc_dets_para_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   float32_t assoc_dets_orth_pos_tcs[MAX_DETS_IN_OBJ_TRK];

   float32_t pos_grid[MAX_DETS_IN_OBJ_TRK] = {};
   uint32_t num_pos_points;

   float32_t exp_pos_grid[MAX_DETS_IN_OBJ_TRK] = {};
   uint32_t exp_num_pos_points;

   float32_t test_threshold = 0.0001F;
   /** \setup
    * Set up an object with
    *    - Position (20,5)
    *    - Reference point REAR_LEFT
    *    - Orientation 0
    *    - [length, width] = [4, 2]
    * Set up 4 detections placed close to the object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.vcs_position.x = 20.0F;
      obj.vcs_position.y = 5.0F;
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      obj.Set_Bbox_Orientation(Angle{ 0.0F });
      obj.ndets = 4;

      assoc_dets_para_pos_tcs[3] = 21.0F;
      assoc_dets_orth_pos_tcs[3] = 4.5F;
      assoc_dets_para_pos_tcs[1] = 22.0F;
      assoc_dets_orth_pos_tcs[1] = 5.5F;
      assoc_dets_para_pos_tcs[2] = 23.0F;
      assoc_dets_orth_pos_tcs[2] = 5.7F;
      assoc_dets_para_pos_tcs[0] = 22.0F;
      assoc_dets_orth_pos_tcs[0] = 6.0F;
   }

};

/** \purpose
 * Test that the correct grid points for the orthogonal grid are generated for an object with 4 assigned detections and reference point in REAR LEFT
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Create_Position_Grid, Orthogonal_Grid_Ref_Pnt_Rear_Left)
{
   /** \precond
    * An object and 4 detections have been set up in the TEST_GROUP
    * Object position = [20,5]
    * Reference point = REAR_LEFT
    * Expected grid points are the detection positions, shifted by the objects half width
    */
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);

   for (uint32_t i = 0U; i < obj.ndets; i++)
   {
      exp_pos_grid[i] = assoc_dets_orth_pos_tcs[i] + obj.bbox.Get_Width() * 0.5F;
   }
   exp_num_pos_points = obj.ndets;


   /** \action
    * Call Create_Position_Grid
    */
   Create_Position_Grid(assoc_dets_orth_pos_tcs, obj.ndets, right_left_side, obj.bbox.Get_Width(), pos_grid, num_pos_points);
   
   /** \result
    * Check that the generated grid points correspond to the expected values
    */
   CHECK_EQUAL_TEXT(exp_num_pos_points, num_pos_points, "Number of grid points is incorrect.")
   for (uint32_t i = 0U; i < num_pos_points; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_pos_grid[i], pos_grid[i], test_threshold, "Position grid point is incorrect.")
   }
}

/** \purpose
 * Test that the correct grid points for the orthogonal grid are generated for an object with 4 assigned detections and reference point in REAR
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Create_Position_Grid, Orthogonal_Grid_Ref_Pnt_Rear)
{
   /** \precond
    * An object and 4 detections have been set up in the TEST_GROUP
    * Object position = [20,5]
    * Reference point = REAR
    * Expected grid points are the detection positions, shifted by the objects half width
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);

   // Generate extected grid point: 11 points on the left of the median and 5 on the right. Increase by 0.1 from left-most detection for each step.
   exp_num_pos_points = 17U;
   for (uint32_t i = 0; i < exp_num_pos_points; i++)
   {
      exp_pos_grid[i] = assoc_dets_orth_pos_tcs[3] + static_cast<float32_t>(i) * 0.1F;
   }

   /** \action
    * Call Create_Position_Grid
    */
   Create_Position_Grid(assoc_dets_orth_pos_tcs, obj.ndets, right_left_side, obj.bbox.Get_Width(), pos_grid, num_pos_points);
   
   /** \result
    * Check that the generated grid points correspond to the expected values
    */
   CHECK_EQUAL_TEXT(exp_num_pos_points, num_pos_points, "Number of grid points is incorrect.")
   for (uint32_t i = 0U; i < num_pos_points; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_pos_grid[i], pos_grid[i], test_threshold, "Position grid point is incorrect.")
   }
}

/** \purpose
 * Test that the number of grid points for the orthogonal grid is correctly saturated when the detection spread is very big and reference point in REAR
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Create_Position_Grid, Orthogonal_Grid_Ref_Pnt_Rear_Num_Points_Saturated)
{
   /** \precond
    * An object and 4 detections have been set up in the TEST_GROUP
    * Make distance between smallest and largest orth pos of detections big.
    * Object position = [20,5]
    * Reference point = REAR
    * Expected grid points are the detection positions, shifted by the objects half width
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   const F360_Object_Sides_T right_left_side = Get_Reference_Point_Orth_Side(obj.reference_point);
   assoc_dets_orth_pos_tcs[3] = -1.0F;
   assoc_dets_orth_pos_tcs[0] = 16.0F;

   // Generate extected grid point: saturated to 79 points with the median in the middle and 39 steps in each direction with interval 0.1.
   exp_num_pos_points = 79U;
   float32_t median_pos = 5.6F;
   for (uint32_t i = 0; i < 40U; i++)
   {
      exp_pos_grid[39U - i] = median_pos - static_cast<float32_t>(i) * 0.1F;
   }
   for (uint32_t i = 0; i < 40U; i++)
   {
      exp_pos_grid[39U + i] = median_pos + static_cast<float32_t>(i) * 0.1F;
   }

   /** \action
    * Call Create_Position_Grid
    */
   Create_Position_Grid(assoc_dets_orth_pos_tcs, obj.ndets, right_left_side, obj.bbox.Get_Width(), pos_grid, num_pos_points);
   
   /** \result
    * Check that the generated grid points correspond to the expected values after saturation
    */
   CHECK_EQUAL_TEXT(exp_num_pos_points, num_pos_points, "Number of grid points is incorrect.")
   for (uint32_t i = 0U; i < num_pos_points; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_pos_grid[i], pos_grid[i], test_threshold, "Position grid point is incorrect.")
   }
}


/** \purpose
 * Test that the correct grid points for the paralell grid are generated for an object with 4 assigned detections and reference point in REAR LEFT
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Create_Position_Grid, Paralell_Grid_Ref_Pnt_Rear_Left)
{
   /** \precond
    * An object and 4 detections have been set up in the TEST_GROUP
    * Object position = [20,5]
    * Reference point = REAR_LEFT
    * Expected grid points are the detection positions, shifted by the objects half width
    */
   const F360_Object_Sides_T rear_front_side = Get_Reference_Point_Para_Side(obj.reference_point);

   for (uint32_t i = 0U; i < obj.ndets; i++)
   {
      exp_pos_grid[i] = assoc_dets_orth_pos_tcs[i] + obj.bbox.Get_Length() * 0.5F;
   }
   exp_num_pos_points = obj.ndets;


   /** \action
    * Call Create_Position_Grid
    */
   Create_Position_Grid(assoc_dets_orth_pos_tcs, obj.ndets, rear_front_side, obj.bbox.Get_Length(), pos_grid, num_pos_points);
   
   /** \result
    * Check that the generated grid points correspond to the expected values
    */
   CHECK_EQUAL_TEXT(exp_num_pos_points, num_pos_points, "Number of grid points is incorrect.")
   for (uint32_t i = 0U; i < num_pos_points; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_pos_grid[i], pos_grid[i], test_threshold, "Position grid point is incorrect.")
   }
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Compute_Detection_Score
 *  @{
 */

/** \brief
 * Testing of a function that computes the score of detections based on their positions in given object's TCS the object's extension.
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Detection_Score)
{
   // Common variables used in tests
   float32_t det_coordinate_tcs;
   F360_Object_Sides_T visible_side;
   float32_t object_dimension_size;

   float32_t detection_score;
   float32_t exp_det_score;
   const float32_t test_threshold = 0.0001F;
   /** \setup
    * A detection coordinate in (modified) TCS is set to 0.5 
    *    - this can be either the para or orth coordinate, depending on the context. Here it doesn't matter.
    * Object visible side is set to INVALID, i.e. the side that gives the best information about the edge position is not visible.
    * Object dimension is set to 2
    *    - this can be either the para or orth coordinate, depending on the context. Here it doesn't matter.
    */
   TEST_SETUP()
   {      
      det_coordinate_tcs = 0.5F;
      visible_side = F360_OBJECT_SIDES_INVALID;
      object_dimension_size = 2.0F;
   }

};

/** \purpose
 * Test that the detection gets the maximum score when visible side is invalid and detection is inside the bounding box
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Invalid_Det_Inside)
{
   /** \precond
    * A detection coordinate, visible side and object dimension have been set up in TEST_GROUP such that
    * the detection is inside the object bounding box
    * Expected detection score is the maximum, i.e 1 since it's inside the bounding box.
    */
   exp_det_score = 1.0F;

   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that the correct detection score is calculated when visible side is invalid and detection is outside the bounding box
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Invalid_Det_Outside)
{
   /** \precond
    * A detection coordinate, visible side and object dimension have been set up in TEST_GROUP
    * Set detection position to be outside the the bounding box
    * Expected detection score 0.9216, below 1 since it's outside the bounding box.
    */
   det_coordinate_tcs = object_dimension_size * 0.5F + 1.0F;
   exp_det_score = 0.9216F;

   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that the score is zero when visible side is invalid and detection is far outside the bounding box
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Invalid_Det_Far_Outside)
{
   /** \precond
    * A detection coordinate, visible side and object dimension have been set up in TEST_GROUP
    * Set detection position to be far outside the the bounding box
    * Expected detection score 0.
    */
   det_coordinate_tcs = 6.0F;
   exp_det_score = 0.0F;

   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that the correct score is computed when the visible side is left and the detection is close to the edge, inside the box.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Inside_Closte_To_Edge)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection inside the bounding box, close to the edge
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   det_coordinate_tcs = 0.1F;
   exp_det_score = 0.992249F;

   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that the correct score is computed when the visible side is left and the detection is close to the edge,
 * inside the box and the bisquare is not saturated.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Inside_Closte_To_Edge_Bisquare_Not_Saturated)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection inside the bounding box, close to the edge
    * Set object dimension to something small, such that bisquare needs to be saturated.
    */
   object_dimension_size = 0.15F;
   visible_side = F360_OBJECT_SIDES_LEFT;
   det_coordinate_tcs = 0.1F;
   exp_det_score = 0.827160478F;

   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that for two detections inside the bounding box, the score is lower for the one farther from the edge.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Inside_Far_From_Edge)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection inside the bounding box, close to the edge
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   const float32_t det_coordinate_tcs_close = 0.1F;
   const float32_t det_coordinate_tcs_not_close = 1.9F;

   /** \action
    * Call Compute_Detection_Score
    */
   const float32_t detection_score_close_to_edge = Compute_Detection_Score(det_coordinate_tcs_close, visible_side, object_dimension_size);
   const float32_t detection_score_far_from_edge = Compute_Detection_Score(det_coordinate_tcs_not_close, visible_side, object_dimension_size);
   
   /** \result
    * Check that score for the detection farther from the edge is smaller than that of the closer detection.
    * Check that the score is not below k_min_score_inside_bbox (0.75)
    */
   CHECK_TRUE_TEXT(detection_score_far_from_edge < detection_score_close_to_edge, "Close detection score is smaller.")
   CHECK_TRUE_TEXT(detection_score_far_from_edge >= 0.75F, "Detection score for a detection inside the box is too small.");
}

/** \purpose
 * Test that the correct score is computed when the visible side is left and the detection is outisde the box, on the side between
 * the object and host (i.e. not ouside the far edge).
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Outside_Close_Edge)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection outside the box, between host and object
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   det_coordinate_tcs = -0.05F;
   exp_det_score = 0.5625F;
   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that detection score is higher outside the non-visible side than outside the visible side.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Compare_Outside_Obj_Different_Sides)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place one detection outside the box, between host and object
    * Place another detection outisde the box on the far side, with same distance from that edge as the first detection
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   const float32_t dist_from_edge = 0.05F;
   const float32_t det_coordinate_tcs_vis_side = -dist_from_edge;
   const float32_t det_coordinate_tcs_non_vis_side = object_dimension_size + dist_from_edge;

   /** \action
    * Call Compute_Detection_Score
    */
   const float32_t detection_score_vis_side = Compute_Detection_Score(det_coordinate_tcs_vis_side, visible_side, object_dimension_size);
   const float32_t detection_score_non_vis_side = Compute_Detection_Score(det_coordinate_tcs_non_vis_side, visible_side, object_dimension_size);
   
   /** \result
    * Check that the detection score outside the non-visible edge is higher than that of the detection outside the visible edge
    */
   CHECK_TRUE_TEXT(detection_score_non_vis_side > detection_score_vis_side, "Detection score incorrectly higher outisde non-visible edge.");
}

/** \purpose
 * Test that a detection that is outside the visible edge with enough margin get a score of zero.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Outside_Close_Edge_With_Margin)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection outside the box, between host and object far enough from the edge for 0 score
    * Expected score is 0.
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   det_coordinate_tcs = -0.5F;
   exp_det_score = 0.0F;
   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is 0.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that a detection that is outside the non-visible edge with enough margin get a score of zero when visible side is left.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Left_Outside_Far_Edge_With_Margin)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to LEFT
    * Place detection outside the box, between host and object far enough from the edge for 0 score
    * Expected score is 0.
    */
   visible_side = F360_OBJECT_SIDES_LEFT;
   det_coordinate_tcs = object_dimension_size  + 1.1F;
   exp_det_score = 0.0F;
   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is 0.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that the correct detection score is calculated when visible side is rear and the detection is outside the far edge (front) with a small margin.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Rear_Outside_Far_Edge)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to REAR
    * Place detection outside the box, between host and object far enough from the edge for 0 score
    * Expected score is 0.
    */
   visible_side = F360_OBJECT_SIDES_REAR;
   det_coordinate_tcs = object_dimension_size  + 0.5F;
   exp_det_score = 0.735074997F;
   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** \purpose
 * Test that a detection that is outside the non-visible edge with enough margin get a score of zero when visible side is rear.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Detection_Score, Compute_Detection_Score_Vis_Side_Rear_Outside_Far_Edge_With_Margin)
{
   /** \precond
    * Object dimension has been set up in TEST_GROUP
    * Set visible side to REAR
    * Place detection outside the box, between host and object far enough from the edge for 0 score
    * Expected score is 0.
    */
   visible_side = F360_OBJECT_SIDES_REAR;
   det_coordinate_tcs = object_dimension_size  + 5.1F;
   exp_det_score = 0.0F;
   /** \action
    * Call Compute_Detection_Score
    */
   detection_score = Compute_Detection_Score(det_coordinate_tcs, visible_side, object_dimension_size);
   
   /** \result
    * Check that the computed detection score is 0.
    */
   DOUBLES_EQUAL_TEXT(exp_det_score, detection_score, test_threshold, "Detection score is incorrect.");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Compute_Bisquare_Score
 *  @{
 */

/** \brief
 * Testing of a specific scoring function - The Bisquare Score
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Bisquare_Score)
{
   // Common variables used in tests
   float32_t input;
   float32_t bisquare;

   float32_t score;
   float32_t exp_score;
   float32_t test_threshold = 0.0001F;
   /** \setup
    * Set up a bisquare parameter and an input for the bisquare function that's smaller than the bisquare.
    */
   TEST_SETUP()
   {      
      input = 0.5F;
      bisquare = 0.8F;
   }
};

/** \purpose
 * Test that the bisquare scoring function returns the correct value when the absolute value of the input is smaller than the bisquare.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Bisquare_Score, Compute_Bisquare_Score_Input_Below_Bisqaure)
{
   /** \precond
    * Using the default test setup from TEST_GROUP.
    */
   exp_score = 0.371338F;

   /** \action
    * Call Compute_Bisquare_Score
    */
   score = Compute_Bisquare_Score(input, bisquare);
   
   /** \result
    * Check that the computed detection score is 0.
    */
   DOUBLES_EQUAL_TEXT(exp_score, score, test_threshold, "Computed score doesn't match the expected value.");
}

/** \purpose
 * Test that the bisquare scoring function returns 0 when the absolute of the input is larger than the bisquare.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Bisquare_Score, Compute_Bisquare_Score_Input_Above_Bisqaure)
{
   /** \precond
    * Set input value above bisquare parameter.
    * Expected output is 0.
    */
   input = bisquare + F360_EPSILON;
   exp_score = 0.0F;
   
   /** \action
    * Call Compute_Bisquare_Score
    */
   score = Compute_Bisquare_Score(input, bisquare);
   
   /** \result
    * Check that the computed detection score is 0.
    */
   DOUBLES_EQUAL_TEXT(exp_score, score, test_threshold, "Computed score doesn't match the expected value.");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Compute_Detection_Score
 *  @{
 */

/** \brief
 * Testing of a function that computes the score of detections based on their positions in given object's TCS the object's extension.
 */

TEST_GROUP(f360_pseudo_position_estimation_Iterate_Over_Grid)
{
   // Common variables used in tests
   float32_t pos_grid[MAX_DETS_IN_OBJ_TRK];
   uint32_t num_grid_points;
   float32_t assoc_dets_pos_tcs[MAX_DETS_IN_OBJ_TRK];
   uint32_t ndets;
   F360_Object_Sides_T obj_visible_side;
   float32_t obj_dimension;

   float32_t pseudo_pos_coordinate_tcs;
   float32_t exp_pseudo_pos_coordinate_tcs;
   float32_t test_threshold = 0.0001F;
   /** \setup
    * Assume an object with reference point rear right at (20, -5) and the orth pos is being estimated, i.e.
    * - obj_visible_side = RIGHT
    * - obj_dimension = object width = 2m
    * Set up 3 detections with
    * - two inside the objects bounding box, close to the edge
    * - and one outisde the box, between host and the object
    * Set up a simple grid with 2 positions only:
    * - One point that is aligned with the actual object position
    * - One point such that a hypothetical object would be aligned with the right-most detection
    */
   TEST_SETUP()
   {      
      ndets = 3U;
      assoc_dets_pos_tcs[0U] = 0.9F;
      assoc_dets_pos_tcs[1U] = 0.95F;
      assoc_dets_pos_tcs[2U] = 1.5F;

      num_grid_points = 2U;
      pos_grid[0U] = 0.0F;
      pos_grid[1U] = 1.0F;

      obj_visible_side = F360_OBJECT_SIDES_RIGHT;
      obj_dimension = 2.0F;
   }
};

/** \purpose
 * Test that the outlier detection determines the pseudo position when there are only 3 detections and 2 grid points. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Iterate_Over_Grid, Iterate_Over_Orth_Grid_Vis_Side_Right_3_Dets_1_Outlier)
{
   /** \precond
    * An orth pos grid, 3 detections and object properties have been set up in TEST_GROUP
    * Since the right-most grid point is aligned with the outlier detection (at tcs y 1.5) and the other two detection are inside the
    * bounding box, there's not enough support to reject the outlier. Thus expected orth pseudo pos is 1.
    */
   exp_pseudo_pos_coordinate_tcs = pos_grid[1U];

   /** \action
    * Call Iterate_Over_Grid
    */
   pseudo_pos_coordinate_tcs = Iterate_Over_Grid(
      pos_grid,
      num_grid_points,
      assoc_dets_pos_tcs,
      ndets,
      obj_visible_side,
      obj_dimension);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_coordinate_tcs, pseudo_pos_coordinate_tcs, test_threshold, "Computed pseudo position coordinate doesn't match the expected value.")
}

/** \purpose
 * Test that the outlier detection's impact on the pseudo position is limited when there are enough detections (5 in this case) to suspect there's an actual edge. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Iterate_Over_Grid, Iterate_Over_Orth_Grid_Vis_Side_Right_4_Dets_1_Outlier)
{
   /** \precond
    * An orth pos grid, 3 detections and object properties have been set up in TEST_GROUP
    * Two extra detection are added close to the initial two close to the object edge.
    * Expected pseudo pos is now 0, i.e. the grid point that matches the actual object and the majority of detections.
    */
   ndets = 5U;
   assoc_dets_pos_tcs[3U] = 0.93F;
   assoc_dets_pos_tcs[4U] = 0.99F;
   exp_pseudo_pos_coordinate_tcs = pos_grid[0U];

   /** \action
    * Call Iterate_Over_Grid
    */
   pseudo_pos_coordinate_tcs = Iterate_Over_Grid(
      pos_grid,
      num_grid_points,
      assoc_dets_pos_tcs,
      ndets,
      obj_visible_side,
      obj_dimension);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_coordinate_tcs, pseudo_pos_coordinate_tcs, test_threshold, "Computed pseudo position coordinate doesn't match the expected value.")
}

/** \purpose
 * Test that the computed pseudo pos is between the two extreme default grid points (0 and 1) when more grid points are added between.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Iterate_Over_Grid, Iterate_Over_Orth_Grid_Vis_Side_Right_3_Dets_1_Outlier_More_Grid_Points)
{
   /** \precond
    * An orth pos grid, 3 detections and object properties have been set up in TEST_GROUP
    * 3 extra grid points are added between 0 and 1.
    * Expected output is 0.5.
    */
   num_grid_points = 5U;
   pos_grid[2U] = 0.25F;
   pos_grid[3U] = 0.5F;
   pos_grid[4U] = 0.75F;
   exp_pseudo_pos_coordinate_tcs = pos_grid[3U];
   

   /** \action
    * Call Iterate_Over_Grid
    */
   pseudo_pos_coordinate_tcs = Iterate_Over_Grid(
      pos_grid,
      num_grid_points,
      assoc_dets_pos_tcs,
      ndets,
      obj_visible_side,
      obj_dimension);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_coordinate_tcs, pseudo_pos_coordinate_tcs, test_threshold, "Computed pseudo position coordinate doesn't match the expected value.")
}

/** \purpose
 * Test that the outlier detection determines the pseudo position when there are only 3 detections and 2 grid points. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Iterate_Over_Grid, Iterate_Over_Orth_Grid_Vis_Side_Right_Similar_Score)
{
   /** \precond
    * An orth pos grid, 3 detections and object properties have been set up in TEST_GROUP
    * Since the right-most grid point is aligned with the outlier detection (at tcs y 1.5) and the other two detection are inside the
    * bounding box, there's not enough support to reject the outlier. Thus expected orth pseudo pos is 1.
    */
   pos_grid[1U] = 0.0000000001F;
   exp_pseudo_pos_coordinate_tcs = pos_grid[1U];

   /** \action
    * Call Iterate_Over_Grid
    */
   pseudo_pos_coordinate_tcs = Iterate_Over_Grid(
      pos_grid,
      num_grid_points,
      assoc_dets_pos_tcs,
      ndets,
      obj_visible_side,
      obj_dimension);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_coordinate_tcs, pseudo_pos_coordinate_tcs, test_threshold, "Computed pseudo position coordinate doesn't match the expected value.")
}

/** \purpose
 * Test that the outlier detection determines the pseudo position when there are only 3 detections and 2 grid points and visible side is left. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Iterate_Over_Grid, Iterate_Over_Orth_Grid_Vis_Side_Left_3_Dets_1_Outlier)
{
   /** \precond
    * An orth pos grid, 3 detections and object properties have been set up in TEST_GROUP
    * Set object visible side to LEFT
    * Mirror detection coordinates and grid points
    * Since the right-most grid point is aligned with the outlier detection (at tcs y 1.5) and the other two detection are inside the
    * bounding box, there's not enough support to reject the outlier. Thus expected orth pseudo pos is -1.
    */
   obj_visible_side = F360_OBJECT_SIDES_LEFT;
   pos_grid[0U] = - pos_grid[0U];
   pos_grid[1U] = - pos_grid[1U];
   assoc_dets_pos_tcs[0U] = - assoc_dets_pos_tcs[0U];
   assoc_dets_pos_tcs[1U] = - assoc_dets_pos_tcs[1U];
   assoc_dets_pos_tcs[2U] = - assoc_dets_pos_tcs[2U];
   exp_pseudo_pos_coordinate_tcs = pos_grid[1U];

   /** \action
    * Call Iterate_Over_Grid
    */
   pseudo_pos_coordinate_tcs = Iterate_Over_Grid(
      pos_grid,
      num_grid_points,
      assoc_dets_pos_tcs,
      ndets,
      obj_visible_side,
      obj_dimension);
      
   /** \result
    * Check that the computed pseudo pos coordinate matches the expected output.
    */
   DOUBLES_EQUAL_TEXT(exp_pseudo_pos_coordinate_tcs, pseudo_pos_coordinate_tcs, test_threshold, "Computed pseudo position coordinate doesn't match the expected value.")
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Calculate_Pseudo_Pos_Weights
 *  @{
 */

/** \brief
 * Testing of a function that calculates weights for a pseudo position measurements based on its distances to the object's reference point.
 */

TEST_GROUP(f360_pseudo_position_estimation_Calculate_Pseudo_Pos_Weights)
{
   // Common variables used in tests
   Point pseudo_pos_tcs;
   Point ref_point_tcs;
   float32_t weights[2];

   float32_t exp_weights[2];

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set an object reference point in TCS at (-2, 1).
    * Set up a pseudo position in TCS at (-2.01, 1.005) 
    */
   TEST_SETUP()
   {
      pseudo_pos_tcs.x = -2.0F;
      pseudo_pos_tcs.y = 1.0F;

      ref_point_tcs.x = -2.01F;
      ref_point_tcs.y = 1.005F;
   }
};

/** \purpose
 * Test that when the estimated pseudo position is very close to the reference point the calculated weights are maximized, i.e. 1. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Pseudo_Pos_Weights, Calculate_Pseudo_Pos_Weights_Pseudo_Pos_Very_Close_To_Ref_Point)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * Expected weights are [1,1] since the distance betwen pseudo point and reference point is below distance for maximum weights.
    */
   exp_weights[0U] = 1.0F;
   exp_weights[1U] = 1.0F;

   /** \action
    * Call Calculate_Pseudo_Pos_Weights
    */
   Calculate_Pseudo_Pos_Weights(pseudo_pos_tcs, ref_point_tcs, weights);
      
   /** \result
    * Check that the computed weights are correct.
    */
   DOUBLES_EQUAL_TEXT(exp_weights[0U], weights[0U], test_threshold, "Incorrect weight for para pos returned.")
   DOUBLES_EQUAL_TEXT(exp_weights[1U], weights[1U], test_threshold, "Incorrect weight for orth pos returned.")
}

/** \purpose
 * Test that when the estimated pseudo position is close to the reference point but outside range for maximum weight, the calculated weights computed correctly. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Pseudo_Pos_Weights, Calculate_Pseudo_Pos_Weights_Pseudo_Pos_Close_To_Ref_Point)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * Slightly change the position of the pseudo position away from the reference point.
    * Expected weights are [0.9801986, 0.9750052]
    */
   pseudo_pos_tcs.x = pseudo_pos_tcs.x - 0.2F;
   pseudo_pos_tcs.y = pseudo_pos_tcs.y + 0.2F;
   exp_weights[0U] = 0.9801986F;
   exp_weights[1U] = 0.9750052F;

   /** \action
    * Call Calculate_Pseudo_Pos_Weights
    */
   Calculate_Pseudo_Pos_Weights(pseudo_pos_tcs, ref_point_tcs, weights);
      
   /** \result
    * Check that the computed weights are correct.
    */
   DOUBLES_EQUAL_TEXT(exp_weights[0U], weights[0U], test_threshold, "Incorrect weight for para pos returned.")
   DOUBLES_EQUAL_TEXT(exp_weights[1U], weights[1U], test_threshold, "Incorrect weight for orth pos returned.")
}

/** \purpose
 * Test that when the estimated pseudo position is far from the reference point, the calculated weight is non-zero. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Pseudo_Pos_Weights, Calculate_Pseudo_Pos_Weights_Pseudo_Pos_Far_From_Ref_Point)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * Change the position of the pseudo measurement to be far away enough from reference point such that the weight would be 0 without saturation.
    * Expectecation is that the output weights will be saturated to non-zero values.
    */
   pseudo_pos_tcs.x = pseudo_pos_tcs.x - 12.0F;
   pseudo_pos_tcs.y = pseudo_pos_tcs.y + 15.0F;

   /** \action
    * Call Calculate_Pseudo_Pos_Weights
    */
   Calculate_Pseudo_Pos_Weights(pseudo_pos_tcs, ref_point_tcs, weights);
      
   /** \result
    * Check that the computed weights are correct.
    */
   CHECK_TRUE_TEXT(weights[0U] > 0.0F, "Weight for para pos not saturated correctly.")
   CHECK_TRUE_TEXT(weights[1U] > 0.0F, "Weight for orth pos not saturated correctly.")
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Compute_Combined_Weighted_Pseudo_Position
 *  @{
 */

/** \brief
 * Testing of a function that calculates weights for a pseudo position measurements based on its distances to the object's reference point.
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Combined_Weighted_Pseudo_Position)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   Point pseudo_pos_tcs_min_max;
   Point pseudo_pos_tcs_grid_search;
   Point combined_pseudo_pos;
   
   Point exp_combined_pseudo_pos;

   float32_t test_threshold = 0.0001F;

   /** \setup
    * Place object with rear reference point at (10,0) and dimensions length, width = [4, 2]
    * Place the two pseudopositions on opposite sides of reference point on equal distance
    */
   TEST_SETUP()
   {
      obj.vcs_position.x = 10.0F;
      obj.vcs_position.y = 0.0F;
      obj.reference_point = F360_REFERENCE_POINT_REAR;
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);

      const float32_t orth_pos_offset = 0.3F;
      pseudo_pos_tcs_min_max.x = obj.vcs_position.x - obj.bbox.Get_Length() * 0.5F;
      pseudo_pos_tcs_min_max.y = obj.vcs_position.y + orth_pos_offset;

      pseudo_pos_tcs_grid_search.x = obj.vcs_position.x - obj.bbox.Get_Length() * 0.5F;
      pseudo_pos_tcs_grid_search.y = obj.vcs_position.y - orth_pos_offset;
   }
};

/** \purpose
 * Test that when two pseudo positions are on euqal distance from the reference point, the combined positon is in the middle. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Combined_Weighted_Pseudo_Position, Compute_Combined_Weighted_Pseudo_Position_Same_Distance)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * Expected combined position is betweeen the two pseudo positions.
    */
   exp_combined_pseudo_pos.x = (pseudo_pos_tcs_grid_search.x + pseudo_pos_tcs_min_max.x) * 0.5F;
   exp_combined_pseudo_pos.y = (pseudo_pos_tcs_grid_search.y + pseudo_pos_tcs_min_max.y) * 0.5F;

   /** \action
    * Call Compute_Combined_Weighted_Pseudo_Position
    */
   Compute_Combined_Weighted_Pseudo_Position(obj, pseudo_pos_tcs_min_max, pseudo_pos_tcs_grid_search, combined_pseudo_pos);
      
   /** \result
    * Check that the computed weights are correct.
    */
   DOUBLES_EQUAL_TEXT(exp_combined_pseudo_pos.x, combined_pseudo_pos.x, test_threshold, "Incorrect pseudo para pos returned.")
   DOUBLES_EQUAL_TEXT(exp_combined_pseudo_pos.y, combined_pseudo_pos.y, test_threshold, "Incorrect pseudo orth pos returned.")
}

/** \purpose
 * Test that when one pseudo position is much farther away than the other, it has almost zero impact on the combined pseudo position. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Combined_Weighted_Pseudo_Position, Compute_Combined_Weighted_Pseudo_Position_One_Far_Away)
{
   /** \precond
    * Move one pseudo position far away from the reference point.
    * The expected output orth position is very close the other pseudo position.
    */
   pseudo_pos_tcs_min_max.y = 15.0F;
   exp_combined_pseudo_pos.x = (pseudo_pos_tcs_grid_search.x + pseudo_pos_tcs_min_max.x) * 0.5F;
   exp_combined_pseudo_pos.y = pseudo_pos_tcs_grid_search.y;

   /** \action
    * Call Compute_Combined_Weighted_Pseudo_Position
    */
   Compute_Combined_Weighted_Pseudo_Position(obj, pseudo_pos_tcs_min_max, pseudo_pos_tcs_grid_search, combined_pseudo_pos);
      
   /** \result
    * Check that the computed weights are correct.
    */
   DOUBLES_EQUAL_TEXT(exp_combined_pseudo_pos.x, combined_pseudo_pos.x, test_threshold, "Incorrect pseudo para pos returned.")
   DOUBLES_EQUAL_TEXT(exp_combined_pseudo_pos.y, combined_pseudo_pos.y, test_threshold, "Incorrect pseudo orth pos returned.")
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Get_Vectors_And_Distances_To_Obj_Corners
 *  @{
 */

/** \brief
 * Testing of a function that computes vectors (and vector lengths) from host center to an object's corners.
 */

TEST_GROUP(f360_pseudo_position_estimation_Get_Vectors_And_Distances_To_Obj_Corners)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Host_T host = {};
   
   float32_t vec_from_host_center_to_obj_corners[4][2];
   float32_t dist_from_host_center_to_obj_corners[4];
   float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with
    * - reference point rear left at [6, 4] m
    * - orientation 0 degrees
    * - [length, width] = [4, 2] m
    * Set host distance to rear axle to 3.33, such that host center is [-2, 0]
    */
   TEST_SETUP()
   {      
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      obj.vcs_position.x = 6.0F;
      obj.vcs_position.y = 4.0F;
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.Update_Bbox_Center();

      host.dist_rear_axle_to_vcs_m = 3.333333333F;
   }
};

/** \purpose
 * Test that the vectors and their lengths are correctly calculated.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Get_Vectors_And_Distances_To_Obj_Corners, Get_Vectors_And_Distances_To_Obj_Corners)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    */
   float32_t exp_vec_from_host_center_to_obj_corners[4][2];
   exp_vec_from_host_center_to_obj_corners[3][0] = 8.0F;
   exp_vec_from_host_center_to_obj_corners[3][1] = 4.0F;
   exp_vec_from_host_center_to_obj_corners[2][0] = 8.0F;
   exp_vec_from_host_center_to_obj_corners[2][1] = 6.0F;
   exp_vec_from_host_center_to_obj_corners[0][0] = 12.0F;
   exp_vec_from_host_center_to_obj_corners[0][1] = 4.0F;
   exp_vec_from_host_center_to_obj_corners[1][0] = 12.0F;
   exp_vec_from_host_center_to_obj_corners[1][1] = 6.0F;

   float32_t exp_dist_from_host_center_to_obj_corners[4];
   exp_dist_from_host_center_to_obj_corners[3] = std::sqrt(80.0F);
   exp_dist_from_host_center_to_obj_corners[2] = std::sqrt(100.0F);
   exp_dist_from_host_center_to_obj_corners[0] = std::sqrt(160.0F);
   exp_dist_from_host_center_to_obj_corners[1] = std::sqrt(180.0F);
   
   /** \action
    * Call Is_Point_Object_Assumption_Valid
    */
   Get_Vectors_And_Distances_To_Obj_Corners(host, obj, vec_from_host_center_to_obj_corners, dist_from_host_center_to_obj_corners);

   /** \result
    * Check that the calculated vectors and their lengths are correct.
    */
   
   for (uint32_t i = 0U; i < 4U; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_dist_from_host_center_to_obj_corners[i], dist_from_host_center_to_obj_corners[i], test_threshold, "Incorrect distance to object corner.");
      for (uint32_t j = 0U; j < 2U; j++)
      {
         DOUBLES_EQUAL_TEXT(exp_vec_from_host_center_to_obj_corners[i][j], vec_from_host_center_to_obj_corners[i][j], test_threshold, "Incorrect element of vector to object corner.");
      } 
   }
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Is_Point_Object_Assumption_Valid
 *  @{
 */

/** \brief
 * Testing of a function that computes the cos of the maximum azimuth spread of an object to deremine if it should be considered a point target or not.
 */

TEST_GROUP(f360_pseudo_position_estimation_Is_Point_Object_Assumption_Valid)
{
   // Common variables used in tests
   float32_t dist_from_host_center_to_obj_corners[4];
   float32_t vec_obj_corners[4][2];

   /** \setup
    * Set up vectors to object corner for an object assumed with
    * - reference point rear at [10, 4] m
    * - orientation 20 degrees
    * - [length, width] = [4, 2] m
    * - and host center in [-2, 0] m
    */
   TEST_SETUP()
   {
      vec_obj_corners[0][0] = 13.41675F;
      vec_obj_corners[0][1] = 6.30777264F; // FL
      vec_obj_corners[1][0] = 14.10079F;
      vec_obj_corners[1][1] = 4.42838764F; // FR
      vec_obj_corners[2][0] = 10.34202F;
      vec_obj_corners[2][1] = 3.06030726F; // RR
      vec_obj_corners[3][0] = 9.65797997F;
      vec_obj_corners[3][1] = 4.9396925F; // RL

      for (uint32_t i = 0U; i < 4; i++)
      {
         dist_from_host_center_to_obj_corners[i] = std::sqrt(vec_obj_corners[i][0]*vec_obj_corners[i][0] + vec_obj_corners[i][1]*vec_obj_corners[i][1]);
      }
      
   }

};

/** \purpose
 * Test that a nearby object is correclty considered as an extended target.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Is_Point_Object_Assumption_Valid, Is_Point_Object_Assumption_Valid_Obj_Close)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    */
   
   /** \action
    * Call Is_Point_Object_Assumption_Valid
    */
   const bool f_result = Is_Point_Object_Assumption_Valid(vec_obj_corners, dist_from_host_center_to_obj_corners);

   /** \result
    * Check that the point target assumption is not valid.
    */
   
   CHECK_FALSE_TEXT(f_result, "Point assumption not valid");
}

/** \purpose
 * Test that a far-away object is correclty considered as a point target.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Is_Point_Object_Assumption_Valid, Is_Point_Object_Assumption_Valid_Obj_Far_Away)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Shift all corner points 65m longitudinally such that maximum azimuth spread is small enough for the point target assumption 
    */
   for (uint32_t i = 0U; i < 4; i++)
   {
      vec_obj_corners[i][0] += 65.0F;
   }

   for (uint32_t i = 0U; i < 4; i++)
   {
      dist_from_host_center_to_obj_corners[i] = std::sqrt(vec_obj_corners[i][0]*vec_obj_corners[i][0] + vec_obj_corners[i][1]*vec_obj_corners[i][1]);
   }
   
   /** \action
    * Call Is_Point_Object_Assumption_Valid
    */
   const bool f_result = Is_Point_Object_Assumption_Valid(vec_obj_corners, dist_from_host_center_to_obj_corners);

   /** \result
    * Check that the point target assumption is not valid.
    */
   
   CHECK_TRUE_TEXT(f_result, "Point assumption is valid");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Calculate_SCS_Min_Range
 *  @{
 */

/** \brief
 * Testing of a function that computes the min range of detections in a pseudo sensor coordinate system (SCS).
 */

TEST_GROUP(f360_pseudo_position_estimation_Calculate_SCS_Min_Range)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   Point host_center_vcs = {};

   const float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with
    * - reference point rear at [10, 4] m
    * - orientation 20 degrees
    * - [length, width] = [4, 2] m
    * Get corners of the object in VCS
    * Set host center to [-2, 0] m
    */
   TEST_SETUP()
   {      
      obj.reference_point = F360_REFERENCE_POINT_REAR;
      obj.vcs_position.x = 100.0F;
      obj.vcs_position.y = 0.0F;
      obj.bbox.Set_Orientation(F360_DEG2RAD(0.0F));
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.Update_Bbox_Center();
      obj.ndets = 3;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;
      obj.detids[2U] = 3U;

      det_props[0U].vcs_position.x = 99.8F;
      det_props[0U].vcs_position.y = -1.5F;

      det_props[1U].vcs_position.x = 102.0F;
      det_props[1U].vcs_position.y = -1.7F;
      
      det_props[2U].vcs_position.x = 101.5F;
      det_props[2U].vcs_position.y = 1.9F;

      host_center_vcs.x = -2.0F;
      host_center_vcs.y = 0.0F;
   }

};

/** \purpose
 * Test that the min detection range in SCS is calculated correctly for the case where the object is right in front of host, far away. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_SCS_Min_Range, Calculate_SCS_Min_Range_Obj_In_Front)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Set expected min range to be the distance to the detection closest to host
    */
   const float32_t exp_min_range_pseudo_SCS = F360_Sqrtf((det_props[0U].vcs_position.x - host_center_vcs.x)*(det_props[0U].vcs_position.x - host_center_vcs.x)
      + (det_props[0U].vcs_position.y - host_center_vcs.y) * (det_props[0U].vcs_position.y - host_center_vcs.y));
   
   /** \action
    * Call Calculate_SCS_Min_Range
    */
   const float32_t min_range_pseudo_SCS = Calculate_SCS_Min_Range(obj, det_props, host_center_vcs);

   /** \result
    * Check that the calculated distance to the closest detection is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_min_range_pseudo_SCS, min_range_pseudo_SCS, test_threshold, "Incorrect min pseudo SCS detection range.");
}

/** \purpose
 * Test that the min detection range in SCS is calculated correctly for the case where the object is to the left and rear of host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_SCS_Min_Range, Calculate_SCS_Min_Range_Obj_In_Rear_Left)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Move the object ot the rear left of host
    * Update detection positions to be close to the object's new position.
    * Set expected min range to be the distance to the detection closest to host
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.vcs_position.x = - 20.0F;
   obj.vcs_position.y = - 5.0F;
   obj.Update_Bbox_Center();

   det_props[0U].vcs_position.x = -19.0F;
   det_props[0U].vcs_position.y = -6.0F;

   det_props[1U].vcs_position.x = -18.0F;
   det_props[1U].vcs_position.y = -3.0F;
   
   det_props[2U].vcs_position.x = -22.0F;
   det_props[2U].vcs_position.y = -3.0F;

   const float32_t exp_min_range_pseudo_SCS = 16.2788F;

   /** \action
    * Call Calculate_SCS_Min_Range
    */
   const float32_t min_range_pseudo_SCS = Calculate_SCS_Min_Range(obj, det_props, host_center_vcs);

   /** \result
    * Check that the calculated distance to the closest detection is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_min_range_pseudo_SCS, min_range_pseudo_SCS, test_threshold, "Incorrect min pseudo SCS detection range.");
}

/** \purpose
 * Test that the min detection range in SCS is calculated correctly for the case where the object is straight behind host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_SCS_Min_Range, Calculate_SCS_Min_Range_Obj_In_Left)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Change object position to be straight behind host
    * Change detection positions to be close to the object's new position
    * Set expected min range to be the distance to the detection closest to host
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.vcs_position.x = - 20.0F;
   obj.vcs_position.y = 0.0F;
   obj.Update_Bbox_Center();

   det_props[0U].vcs_position.x = -20.0F;
   det_props[0U].vcs_position.y = 1.0F;

   det_props[1U].vcs_position.x = -22.0F;
   det_props[1U].vcs_position.y = 0.0F;
   
   det_props[2U].vcs_position.x = -20.0F;
   det_props[2U].vcs_position.y = -0.5F;

   const float32_t exp_min_range_pseudo_SCS = 18.0069F;
   
   /** \action
    * Call Calculate_SCS_Min_Range
    */
   const float32_t min_range_pseudo_SCS = Calculate_SCS_Min_Range(obj, det_props, host_center_vcs);

   /** \result
    * Check that the calculated distance to the closest detection is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_min_range_pseudo_SCS, min_range_pseudo_SCS, test_threshold, "Incorrect min pseudo SCS detection range.");
}

/** \purpose
 * Test that the min detection range in SCS is calculated correctly for the case where the object is in front to the right of host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_SCS_Min_Range, Calculate_SCS_Min_Range_Obj_In_Front_Right)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Change object position to be (3,4) and have reference point REAR RIGHT
    * Change orientation of the object by 90 degrees
    * Change detection positions to be close to the object's new position
    * Set expected min range to be the distance to the detection closest to host
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.bbox.Set_Orientation(F360_DEG2RAD(90.0F));
   obj.vcs_position.x = 3.0F;
   obj.vcs_position.y = 4.0F;
   obj.Update_Bbox_Center();
   obj.ndets = 2;

   det_props[0U].vcs_position.x = 4.0F;
   det_props[0U].vcs_position.y = 4.0F;

   det_props[1U].vcs_position.x = 5.0F;
   det_props[1U].vcs_position.y = 7.0F;

   const float32_t exp_min_range_pseudo_SCS = 7.2111F;
   
   /** \action
    * Call Calculate_SCS_Min_Range
    */
   const float32_t min_range_pseudo_SCS = Calculate_SCS_Min_Range(obj, det_props, host_center_vcs);

   /** \result
    * Check that the calculated distance to the closest detection is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_min_range_pseudo_SCS, min_range_pseudo_SCS, test_threshold, "Incorrect min pseudo SCS detection range.");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth
 *  @{
 */

/** \brief
 * Testing of a function that computes the average azimuth of detections in a pseudo sensor coordinate system (SCS).
 */

TEST_GROUP(f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   Point host_center_vcs = {};
   Angle angle_host_center_to_obj_center_vcs = {};

   const float32_t test_threshold = 0.0001F;

   /** \setup
    * Set up an object with
    * - reference point rear at [100, 0] m
    * - orientation 20 degrees
    * - [length, width] = [4, 2] m
    * Get corners of the object in VCS
    * Set host center to [-2, 0] m
    */
   TEST_SETUP()
   {      
      obj.reference_point = F360_REFERENCE_POINT_REAR;
      obj.vcs_position.x = 100.0F;
      obj.vcs_position.y = 0.0F;
      obj.bbox.Set_Orientation(F360_DEG2RAD(0.0F));
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.Update_Bbox_Center();
      obj.ndets = 3;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;
      obj.detids[2U] = 3U;

      det_props[0U].vcs_position.x = 99.8F;
      det_props[0U].vcs_position.y = -1.5F;

      det_props[1U].vcs_position.x = 102.0F;
      det_props[1U].vcs_position.y = -1.7F;
      
      det_props[2U].vcs_position.x = 101.5F;
      det_props[2U].vcs_position.y = 1.9F;

      host_center_vcs.x = -2.0F;
      host_center_vcs.y = 0.0F;

      angle_host_center_to_obj_center_vcs.Value(F360_Atan2f(obj.bbox.Get_Center().y -  host_center_vcs.y, obj.bbox.Get_Center().x - host_center_vcs.x));
   }

};

/** \purpose
 * Test that the average detection azimuth in SCS is calculated correctly for the case where the object is right in front of host, far away. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth, Calculate_Mean_SCS_Azimuth_Obj_In_Front)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Set expected average azimuth
    */
   const float32_t exp_avg_az_pseudo_SCS = -0.0042F;
   
   /** \action
    * Call Calculate_Mean_SCS_Azimuth
    */
   const Angle avg_az_pseudo_SCS = Calculate_Mean_SCS_Azimuth(obj, det_props, host_center_vcs, angle_host_center_to_obj_center_vcs);

   /** \result
    * Check that the calculated mean detection azimuth is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_avg_az_pseudo_SCS, avg_az_pseudo_SCS.Value(), test_threshold, "Average det SCS azimuth is incorrect.");
}

/** \purpose
 * Test that the average detection azimuth in SCS is calculated correctly for the case where the object is to the left and rear of host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth, Calculate_Mean_SCS_Azimuth_Obj_In_Rear_Left)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Move the object ot the rear left of host
    * Update detection positions to be close to the object's new position.
    * Set expected average azimuth
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.vcs_position.x = - 20.0F;
   obj.vcs_position.y = - 5.0F;
   obj.Update_Bbox_Center();
   angle_host_center_to_obj_center_vcs.Value(F360_Atan2f(obj.bbox.Get_Center().y -  host_center_vcs.y, obj.bbox.Get_Center().x - host_center_vcs.x));

   det_props[0U].vcs_position.x = -19.0F;
   det_props[0U].vcs_position.y = -6.0F;

   det_props[1U].vcs_position.x = -18.0F;
   det_props[1U].vcs_position.y = -3.0F;
   
   det_props[2U].vcs_position.x = -22.0F;
   det_props[2U].vcs_position.y = -3.0F;

   const float32_t exp_avg_az_pseudo_SCS = -0.0669F;
   
   /** \action
    * Call Calculate_Mean_SCS_Azimuth
    */
   const Angle avg_az_pseudo_SCS = Calculate_Mean_SCS_Azimuth(obj, det_props, host_center_vcs, angle_host_center_to_obj_center_vcs);

   /** \result
    * Check that the calculated mean detection azimuth is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_avg_az_pseudo_SCS, avg_az_pseudo_SCS.Value(), test_threshold, "Average det SCS azimuth is incorrect.");
}

/** \purpose
 * Test that the average detection azimuth in SCS is calculated correctly for the case where the object is straight behind host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth, Calculate_Mean_SCS_Azimuth_Obj_In_Left)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Change object position to be straight behind host
    * Change detection positions to be close to the object's new position
    * Set expected min range to be the distance to the detection closest to host
    * Set expected average azimuth
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.vcs_position.x = - 20.0F;
   obj.vcs_position.y = 0.0F;
   obj.Update_Bbox_Center();
   angle_host_center_to_obj_center_vcs.Value(F360_Atan2f(obj.bbox.Get_Center().y -  host_center_vcs.y, obj.bbox.Get_Center().x - host_center_vcs.x));

   det_props[0U].vcs_position.x = -20.0F;
   det_props[0U].vcs_position.y = 1.0F;

   det_props[1U].vcs_position.x = -22.0F;
   det_props[1U].vcs_position.y = 0.0F;
   
   det_props[2U].vcs_position.x = -20.0F;
   det_props[2U].vcs_position.y = -0.5F;

   const float32_t exp_avg_az_pseudo_SCS = -0.0092F;
   
   /** \action
    * Call Calculate_Mean_SCS_Azimuth
    */
   const Angle avg_az_pseudo_SCS = Calculate_Mean_SCS_Azimuth(obj, det_props, host_center_vcs, angle_host_center_to_obj_center_vcs);

   /** \result
    * Check that the calculated mean detection azimuth is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_avg_az_pseudo_SCS, avg_az_pseudo_SCS.Value(), test_threshold, "Average det SCS azimuth is incorrect.");
}

/** \purpose
 * Test that the min detection range and average detection azimuth in SCS is calculated correctly for the case where the object is in front to the right of host. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Calculate_Mean_SCS_Azimuth, Calculate_Mean_SCS_Azimuth_Obj_In_Front_Right)
{
   /** \precond
    * A default test has been set up in the TEST_GROUP.
    * Change object position to be (3,4) and have reference point REAR RIGHT
    * Change orientation of the object by 90 degrees
    * Change detection positions to be close to the object's new position
    * Set expected min range to be the distance to the detection closest to host
    * Set expected average azimuth
    */
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.bbox.Set_Orientation(F360_DEG2RAD(90.0F));
   obj.vcs_position.x = 3.0F;
   obj.vcs_position.y = 4.0F;
   obj.Update_Bbox_Center();
   obj.ndets = 2;
   angle_host_center_to_obj_center_vcs.Value(F360_Atan2f(obj.bbox.Get_Center().y -  host_center_vcs.y, obj.bbox.Get_Center().x - host_center_vcs.x));

   det_props[0U].vcs_position.x = 4.0F;
   det_props[0U].vcs_position.y = 4.0F;

   det_props[1U].vcs_position.x = 5.0F;
   det_props[1U].vcs_position.y = 7.0F;

   const float32_t exp_avg_az_pseudo_SCS = -0.0987F;
   
   /** \action
    * Call Calculate_Mean_SCS_Azimuth
    */
   const Angle avg_az_pseudo_SCS = Calculate_Mean_SCS_Azimuth(obj, det_props, host_center_vcs, angle_host_center_to_obj_center_vcs);

   /** \result
    * Check that the calculated mean detection azimuth is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_avg_az_pseudo_SCS, avg_az_pseudo_SCS.Value(), test_threshold, "Average det SCS azimuth is incorrect.");
}

/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS
 *  @{
 */

/** \brief
 * Testing of a function that computes a vector from host center to the closest corner of an object.
 */

TEST_GROUP(f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   float32_t dist_from_host_center_to_obj_corners[4] = {};
   BboxCorners obj_corners_vcs = {};

   float32_t vec_obj_clostes_corner_to_center_vcs[2] = {};
   float32_t exp_vec_obj_clostes_corner_to_center_vcs[2] = {};
   float32_t test_threshold = 0.0001F;

   const float32_t host_center_vcs[2] = {-2.0F, 0.0F};

   /** \setup
    * Set up an object with
    *    - Reference point REAR LEFT at position (5,5)
    *    - [Length, width] = [4,2]m
    *    - Assign distances to object corners
    *    - Retrieve object corners
    */
   TEST_SETUP()
   {      
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.vcs_position.x = 5.0F;
      obj.vcs_position.y = 5.0F;
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      obj.bbox.Set_Orientation(0.0F);
      obj.Update_Bbox_Center();

      obj_corners_vcs = obj.bbox.Get_Corners();
      const float32_t host_center_vcs[2] = {-2.0F, 0.0F};
      const float32_t vec_from_host_center_to_obj_corners[4][2] = { {obj_corners_vcs.Front_Left().x - host_center_vcs[0], obj_corners_vcs.Front_Left().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Front_Right().x - host_center_vcs[0], obj_corners_vcs.Front_Right().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Rear_Right().x - host_center_vcs[0], obj_corners_vcs.Rear_Right().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Rear_Left().x - host_center_vcs[0], obj_corners_vcs.Rear_Left().y - host_center_vcs[1]} }; // Pseudo sensor/radar in host center

      dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
      dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
      dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
      dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);
   }

};

/** \purpose
 * Test that the correct vector is calculated when object's rear left corner is the closest corner. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS, Get_Vector_From_Obj_Center_To_Closest_Corner_VCS_Rear_Left)
{
   /** \precond
    * A defualt case has been set up in the TEST_GROUP.
    * Expected output vector is (2,1)
    */
   exp_vec_obj_clostes_corner_to_center_vcs[0U] = 2.0F;
   exp_vec_obj_clostes_corner_to_center_vcs[1U] = 1.0F;


   /** \action
    * Call Get_Vector_From_Obj_Center_To_Closest_Corner_VCS
    */
   Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(obj, dist_from_host_center_to_obj_corners, obj_corners_vcs, vec_obj_clostes_corner_to_center_vcs);

   /** \result
    * Check that the computed vector is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[0U], vec_obj_clostes_corner_to_center_vcs[0U], test_threshold, "x component of vector is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[1U], vec_obj_clostes_corner_to_center_vcs[1U], test_threshold, "y component of vector is incorrect.")
}

/** \purpose
 * Test that the correct vector is calculated when object's front left corner is the closest corner. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS, Get_Vector_From_Obj_Center_To_Closest_Corner_VCS_Front_Left)
{
   /** \precond
    * A defualt case has been set up in the TEST_GROUP.
    * Rotate object -90 degrees and change reference point to FRONT LEFT
    * Expected output vector is (1,2)
    */
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.bbox.Set_Orientation(F360_DEG2RAD(-90.0F));
   obj.Update_Bbox_Center();

   obj_corners_vcs = obj.bbox.Get_Corners();
   const float32_t vec_from_host_center_to_obj_corners[4][2] = { {obj_corners_vcs.Front_Left().x - host_center_vcs[0], obj_corners_vcs.Front_Left().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Front_Right().x - host_center_vcs[0], obj_corners_vcs.Front_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Right().x - host_center_vcs[0], obj_corners_vcs.Rear_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Left().x - host_center_vcs[0], obj_corners_vcs.Rear_Left().y - host_center_vcs[1]} }; // Pseudo sensor/radar in host center

   dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
   dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
   dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
   dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);

   exp_vec_obj_clostes_corner_to_center_vcs[0U] = 1.0F;
   exp_vec_obj_clostes_corner_to_center_vcs[1U] = 2.0F;
   /** \action
    * Call Get_Vector_From_Obj_Center_To_Closest_Corner_VCS
    */
   Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(obj, dist_from_host_center_to_obj_corners, obj_corners_vcs, vec_obj_clostes_corner_to_center_vcs);

   /** \result
    * Check that the computed vector is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[0U], vec_obj_clostes_corner_to_center_vcs[0U], test_threshold, "x component of vector is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[1U], vec_obj_clostes_corner_to_center_vcs[1U], test_threshold, "y component of vector is incorrect.")
}

/** \purpose
 * Test that the correct vector is calculated when object's rear right corner is the closest corner. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS, Get_Vector_From_Obj_Center_To_Closest_Corner_VCS_Rear_Right)
{
   /** \precond
    * A defualt case has been set up in the TEST_GROUP.
    * Move object to (-5,5) and rotate 180 degrees such that REAR RIGHT is the reference point
    * Expected output vector is (1,2)
    */
   obj.vcs_position.x = -5.0F;
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.bbox.Set_Orientation(F360_DEG2RAD(180.0F));
   obj.Update_Bbox_Center();

   obj_corners_vcs = obj.bbox.Get_Corners();
   const float32_t vec_from_host_center_to_obj_corners[4][2] = { {obj_corners_vcs.Front_Left().x - host_center_vcs[0], obj_corners_vcs.Front_Left().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Front_Right().x - host_center_vcs[0], obj_corners_vcs.Front_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Right().x - host_center_vcs[0], obj_corners_vcs.Rear_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Left().x - host_center_vcs[0], obj_corners_vcs.Rear_Left().y - host_center_vcs[1]} }; // Pseudo sensor/radar in host center

   dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
   dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
   dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
   dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);

   exp_vec_obj_clostes_corner_to_center_vcs[0U] = -2.0F;
   exp_vec_obj_clostes_corner_to_center_vcs[1U] = 1.0F;
   /** \action
    * Call Get_Vector_From_Obj_Center_To_Closest_Corner_VCS
    */
   Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(obj, dist_from_host_center_to_obj_corners, obj_corners_vcs, vec_obj_clostes_corner_to_center_vcs);

   /** \result
    * Check that the computed vector is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[0U], vec_obj_clostes_corner_to_center_vcs[0U], test_threshold, "x component of vector is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[1U], vec_obj_clostes_corner_to_center_vcs[1U], test_threshold, "y component of vector is incorrect.")
}

/** \purpose
 * Test that the correct vector is calculated when object's front right corner is the closest corner. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Get_Vector_From_Obj_Center_To_Closest_Corner_VCS, Get_Vector_From_Obj_Center_To_Closest_Corner_VCS_Front_Right)
{
   /** \precond
    * A defualt case has been set up in the TEST_GROUP.
    * Move object to (-5,-5) FRONT RIGHT is the reference point
    * Expected output vector is (1,2)
    */
   obj.vcs_position.x = -5.0F;
   obj.vcs_position.y = -5.0F;
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.Update_Bbox_Center();

   obj_corners_vcs = obj.bbox.Get_Corners();
   const float32_t vec_from_host_center_to_obj_corners[4][2] = { {obj_corners_vcs.Front_Left().x - host_center_vcs[0], obj_corners_vcs.Front_Left().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Front_Right().x - host_center_vcs[0], obj_corners_vcs.Front_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Right().x - host_center_vcs[0], obj_corners_vcs.Rear_Right().y - host_center_vcs[1]},
                                                                  {obj_corners_vcs.Rear_Left().x - host_center_vcs[0], obj_corners_vcs.Rear_Left().y - host_center_vcs[1]} }; // Pseudo sensor/radar in host center

   dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
   dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
   dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
   dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);

   exp_vec_obj_clostes_corner_to_center_vcs[0U] = - 2.0F;
   exp_vec_obj_clostes_corner_to_center_vcs[1U] = - 1.0F;
   /** \action
    * Call Get_Vector_From_Obj_Center_To_Closest_Corner_VCS
    */
   Get_Vector_From_Obj_Center_To_Closest_Corner_VCS(obj, dist_from_host_center_to_obj_corners, obj_corners_vcs, vec_obj_clostes_corner_to_center_vcs);

   /** \result
    * Check that the computed vector is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[0U], vec_obj_clostes_corner_to_center_vcs[0U], test_threshold, "x component of vector is incorrect.")
   DOUBLES_EQUAL_TEXT(exp_vec_obj_clostes_corner_to_center_vcs[1U], vec_obj_clostes_corner_to_center_vcs[1U], test_threshold, "y component of vector is incorrect.")
}

/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Compute_Pseudo_Pos_Extended_Obj_Assumption
 *  @{
 */

/** \brief
 * Testing of a function that computes the score of detections based on their positions in given object's TCS the object's extension.
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Extended_Obj_Assumption)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   float32_t exp_det_score;
   float32_t test_threshold = 0.0001F;

   Point pseudo_pos_vcs;

   /** \setup
    * Set up an object with
    *    - Reference point REAR at [20,0]
    *    - [Length, width] = [4,2]m
    * Place some detections close to the object and one obvious outlier
    */
   TEST_SETUP()
   {  
      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.vcs_position.x = 20.0F;
      obj.vcs_position.y = 4.0F;
      obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      obj.bbox.Set_Orientation(0.0F);
      obj.Update_Bbox_Center();

      obj.ndets = 6;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;
      obj.detids[2U] = 3U;
      obj.detids[3U] = 4U;
      obj.detids[4U] = 5U;
      obj.detids[5U] = 6U;
      
      // First detection is 1m from left edge
      det_props[0U].vcs_position.x = 22.0F;
      det_props[0U].vcs_position.y = 3.0F;

      // The rest of the detections are close to the left and rear edges
      det_props[1U].vcs_position.x = 20.0F;
      det_props[1U].vcs_position.y = 4.5F;

      det_props[2U].vcs_position.x = 21.0F;
      det_props[2U].vcs_position.y = 4.0F;

      det_props[3U].vcs_position.x = 20.0F;
      det_props[3U].vcs_position.y = 6.0F;

      det_props[4U].vcs_position.x = 23.0F;
      det_props[4U].vcs_position.y = 4.0F;

      det_props[5U].vcs_position.x = 22.0F;
      det_props[5U].vcs_position.y = 4.0F;
   }

};

/** \purpose
 * Test that when there are many detections close to the object edges and one outlier, the computed pseudo pos is close to the median of the detections.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Extended_Obj_Assumption, Compute_Pseudo_Pos_Extended_Obj_Assumption_Many_Dets_One_Outlier)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * The pseudo position is expected to have
    * - longitudinal position close to the detection with the minimal longitudinal position
    * - lateral position close to the detections on the, in this case close to the median lateral position
    * I.e. in the lateral position, the grid search scheme will have a greater impact on the final pseudo position.
    */
   constexpr int32_t ndets = 6;
   float32_t dets_x[ndets] = {};
   float32_t dets_y[ndets] = {};
   for (uint32_t i = 0U; i < obj.ndets; i++)
   {
      dets_x[i] = det_props[i].vcs_position.x;
      dets_y[i] = det_props[i].vcs_position.y;
   }

   uint32_t sort_idx[ndets] = {};
   (void)F360_Sort(dets_y, static_cast<uint32_t>(obj.ndets), true, sort_idx);
   const float32_t median_y = 0.5F * (dets_y[2] + dets_y[3]);

   const float32_t exp_x_pos = F360_Min_Element(dets_x);


   /** \action
    * Call Compute_Pseudo_Pos_Extended_Obj_Assumption
    */
   const Point pseudo_pos_vcs = Compute_Pseudo_Pos_Extended_Obj_Assumption(det_props, obj);

   /** \result
    * Check that the generated grid points correspond to the expected values
    */
   const float32_t y_dist = std::abs(pseudo_pos_vcs.y - median_y);
   const float32_t x_dist = std::abs(pseudo_pos_vcs.x - exp_x_pos);
   const float32_t test_threshold = 0.01F;

   CHECK_TRUE_TEXT(y_dist < test_threshold, "Pseudo y pos too far from expected value.");
   CHECK_TRUE_TEXT(x_dist < test_threshold, "Pseudo x pos too far from expected value.");
}

/** \purpose
 * Test that when there are only two detections, the min max scheme to calculate the pseudo position is used. As a result, the minimum x and y positions of detections will be selected.
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Extended_Obj_Assumption, Compute_Pseudo_Pos_Extended_Obj_Assumption_Two_Dets_One_Outlier)
{
   /** \precond
    * A default case has been set up in the TEST_GROUP.
    * Change to 2 associated detections
    * - One close to the reference point at (20, 4.5)
    * - One lateral outlier at (22, 3)
    * The pseudo position is expected to have
    * - longitudinal position at the detection with the smallest longitudinal position
    * - lateral position at the detection with the smallest lateral position (i.e. the outlier).
    * I.e. in the lateral position, the min max scheme will have a greater impact on the final pseudo position.
    */
   obj.ndets = 2U;
   constexpr uint32_t ndets = 2U;
   float32_t dets_x[ndets] = {};
   float32_t dets_y[ndets] = {};
   for (uint32_t i = 0U; i < obj.ndets; i++)
   {
      dets_x[i] = det_props[i].vcs_position.x;
      dets_y[i] = det_props[i].vcs_position.y;
   }

   const float32_t exp_y_pos = F360_Min_Element(dets_y);
   const float32_t exp_x_pos = F360_Min_Element(dets_x);


   /** \action
    * Call Compute_Pseudo_Pos_Extended_Obj_Assumption
    */
   const Point pseudo_pos_vcs = Compute_Pseudo_Pos_Extended_Obj_Assumption(det_props, obj);

   /** \result
    * Check that the generated grid points correspond to the expected values
    */
   const float32_t y_dist = std::abs(pseudo_pos_vcs.y - exp_y_pos);
   const float32_t x_dist = std::abs(pseudo_pos_vcs.x - exp_x_pos);
   const float32_t test_threshold = 0.0001F;

   CHECK_TRUE_TEXT(y_dist < test_threshold, "Pseudo y pos too far from expected value.");
   CHECK_TRUE_TEXT(x_dist < test_threshold, "Pseudo x pos too far from expected value.");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Compute_Pseudo_Pos_Object_Point_Assumption
 *  @{
 */

/** \brief
 * Testing of a function that computes the pseudo position of an object given that the object is assumed to be a point target.
 */

TEST_GROUP(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Object_Point_Assumption)
{
   // Common variables used in tests
   F360_Object_Track_T obj = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Host_T host;
   BboxCorners obj_corners_vcs;
   float32_t dist_from_host_center_to_obj_corners[4];

   /** \setup
    * Set up an object with
    * - reference point REAR at [-2, 7]
    * - Orientation 90 deg
    * - [len, wid] = [4,2]
    * - 2 associated detection
    * Set detection positions such that object's pseudo position is clearly updated.
    */
   TEST_SETUP()
   {  
      /*host.dist_rear_axle_to_vcs_m = 3.333333333F;

      obj.bbox.Set_Length(4.0F);
      obj.bbox.Set_Width(2.0F);
      obj.vcs_position.x = -2.0F;
      obj.vcs_position.y = 7.0F;
      obj.reference_point = F360_REFERENCE_POINT_REAR;
      obj.bbox.Set_Orientation(F360_DEG2RAD(90.0F));
      obj.Update_Bbox_Center();

      obj.ndets = 2;
      obj.detids[0U] = 1U;
      obj.detids[1U] = 2U;

      det_props[0U].vcs_position.x = 0.0F;
      det_props[0U].vcs_position.y = 8.0F;
      det_props[1U].vcs_position.x = -2.0F;
      det_props[1U].vcs_position.y = 9.0F;

      // Get object corners in VCS and calculate distances to corners
      obj_corners_vcs = obj.bbox.Get_Corners();
      const float32_t vec_from_host_center_to_obj_corners[4][2] = { {obj_corners_vcs.Front_Left().x - host_center_vcs[0], obj_corners_vcs.Front_Left().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Front_Right().x - host_center_vcs[0], obj_corners_vcs.Front_Right().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Rear_Right().x - host_center_vcs[0], obj_corners_vcs.Rear_Right().y - host_center_vcs[1]},
                                                                     {obj_corners_vcs.Rear_Left().x - host_center_vcs[0], obj_corners_vcs.Rear_Left().y - host_center_vcs[1]} }; // Pseudo sensor/radar in host center

      dist_from_host_center_to_obj_corners[0] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[0][0], vec_from_host_center_to_obj_corners[0][1]);
      dist_from_host_center_to_obj_corners[1] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[1][0], vec_from_host_center_to_obj_corners[1][1]);
      dist_from_host_center_to_obj_corners[2] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[2][0], vec_from_host_center_to_obj_corners[2][1]);
      dist_from_host_center_to_obj_corners[3] = F360_Get_Hypotenuse(vec_from_host_center_to_obj_corners[3][0], vec_from_host_center_to_obj_corners[3][1]);*/
   }

};

/** \purpose
 * Test that the calculated pseudo position is within an expected range longitudinally and laterally. 
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Compute_Pseudo_Pos_Object_Point_Assumption, Compute_Pseudo_Pos_Object_Point_Assumption)
{
   /** \precond
    * A default case has been set up in the TEST GROUP.
    * Since the range to the detection closest to host is 8.245m, that should be the minimum lateral position
    * And since the mean SCS azimuth of detections is ~-7 degrees, that should give a longitudinal position above -1m.
    * In this case, the pseudo position should also be limited by the detections.
    */
   /*const float32_t min_long_pos = - 1.0F;
   const float32_t max_long_pos = det_props[0U].vcs_position.x;
   const float32_t min_lat_pos = 8.245F;
   const float32_t max_lat_pos = det_props[1U].vcs_position.y;*/

   /** \action
    * Call Compute_Pseudo_Pos_Object_Point_Assumption
    */
   //const Point pseudo_pos_vcs = Compute_Pseudo_Pos_Object_Point_Assumption(det_props, host_center_vcs, obj_corners_vcs, dist_from_host_center_to_obj_corners, obj);

   /** \result
    * Check that the calculated pseudo position is within position bounds.
    */
   /*CHECK_TRUE_TEXT(pseudo_pos_vcs.x > min_long_pos, "Pseudo pos x coordinate is too small.");
   CHECK_TRUE_TEXT(pseudo_pos_vcs.x < max_long_pos, "Pseudo pos x coordinate is too large.");
   CHECK_TRUE_TEXT(pseudo_pos_vcs.y > min_lat_pos, "Pseudo pos y coordinate is too small.");
   CHECK_TRUE_TEXT(pseudo_pos_vcs.y < max_lat_pos, "Pseudo pos x coordinate is too large.");*/
}

/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS
 *  @{
 */

/** \brief
 * This test group tests the functionality of Transform_Pseudo_SCS_Pos_To_VCS
 */
TEST_GROUP(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS)
{
   // Common variables used in tests
   const float32_t test_threshold = 0.0001F;

   Point host_center_vcs;

   /** \setup
    * Set host_center_vcs to x=-5, y=0
    */
   TEST_SETUP()
   {  
      host_center_vcs = Point(-5, 0);
   }
};

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is in front right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_front_right_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=0,y=5
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to 45 deg
    */
   const Point exp_pos_vcs = Point(0.0F, 5.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(45.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is in front of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_front_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=sqrt(50)-5,y=0
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to 0 deg
    */
   const Point exp_pos_vcs = Point(2.0710678F, 0.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(0.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is in front left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_front_left_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=0,y=-5
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to 0 deg
    */
   const Point exp_pos_vcs = Point(0.0F, -5.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(-45.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is to the left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_left_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=-5,y=-sqrt(50)
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to -90 deg
    */
   const Point exp_pos_vcs = Point(-5.0F, -7.0710678F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(-90.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is to the down left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_down_left_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=-10,y=-5
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to -135 deg
    */
   const Point exp_pos_vcs = Point(-10.0F, -5.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(-135.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is behind of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_behind_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=-5-sqrt(50),y=0
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to -180 deg
    */
   const Point exp_pos_vcs = Point(-12.0710678F, 0.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(-180.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is to the down right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_down_right_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=-10,y=5
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to 135 deg
    */
   const Point exp_pos_vcs = Point(-10.0F, 5.0F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(135.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that Transform_Pseudo_SCS_Pos_To_VCS transforms a point defined in the pseudo sensor coordinate system (SCS) 
 * converts correctly to a point in the vehicle coordinate system (VCS) when the point is to the right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Pseudo_SCS_Pos_To_VCS, Transform_Pseudo_SCS_Pos_To_VCS_right_of_host)
{
   /** \precond
    * Set exp_pos_vcs to x=-5,y=sqrt(50)
    * Set pos_pseudo_SCS to x=sqrt(50),y=0
    * Set pseudo_scs_rotation_in_vcs to 90 deg
    */
   const Point exp_pos_vcs = Point(-5.0F, 7.0710678F);

   const Point pos_pseudo_SCS = Point(7.0710678F, 0.0F); //sqrt(50)
   const Angle pseudo_scs_rotation_in_vcs = Angle(90.0F * 0.0174532925F);

   /** \action
    * Call Transform_Pseudo_SCS_Pos_To_VCS
    */
   const Point pos_vcs = Transform_Pseudo_SCS_Pos_To_VCS(pos_pseudo_SCS, pseudo_scs_rotation_in_vcs, host_center_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, pos_vcs.x, test_threshold, "Pseudo position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, pos_vcs.y, test_threshold, "Pseudo position y coordinate in vcs is incorrect.");
}

/** @}*/


/** \defgroup  f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point
 *  @{
 */

/** \brief
 * This test group tests the functionality of Convert_Object_Center_Point_To_Reference_Point
 */
TEST_GROUP(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point)
{
   // Common variables used in tests
   const float32_t test_threshold = 0.0001F;

   F360_Object_Track_T obj = {};
   Point obj_center_pseudo_pos_vcs = {};

   /** \setup
    * Set up an object with length 6, width 3, orientation 0 deg vcs
    */
   TEST_SETUP()
   {  
      obj.bbox.Set_Length(6.0F);
      obj.bbox.Set_Width(3.0F);
      obj.bbox.Set_Orientation(0.0F);
   }
};

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_right_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=0,y=10
    * Set obj.reference point to left
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(0.0F,10.0F);
   obj.reference_point = F360_REFERENCE_POINT_LEFT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the front right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_front_right_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=10,y=10
    * Set obj.reference point to rear left
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(10.0F,10.0F);
   obj.reference_point = F360_REFERENCE_POINT_REAR_LEFT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is in front of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_front_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=10,y=0
    * Set obj.reference point to rear
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(10.0F,0.0F);
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is front left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_front_left_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=10,y=-10
    * Set obj.reference point to rear right
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(10.0F,-10.0F);
   obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_left_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=0,y=-10
    * Set obj.reference point to right
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(0.0F,-10.0F);
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the rear left of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_rear_left_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=-10,y=-10
    * Set obj.reference point to front right
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(-10.0F,-10.0F);
   obj.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the rear of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_rear_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=-10,y=0
    * Set obj.reference point to front
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(-10.0F,0.0F);
   obj.reference_point = F360_REFERENCE_POINT_FRONT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object is to the rear right of host
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_rear_right_of_host)
{
   /** \precond
    * Set obj.vcs_position to x=-10,y=10
    * Set obj.reference point to front left
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(-10.0F,10.0F);
   obj.reference_point = F360_REFERENCE_POINT_FRONT_LEFT;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** \purpose
 * Test that, given a point corresponding to the object center point, this function offsets the given
 * position to instead correspond to the objects current reference point when the object reference point is center
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Convert_Object_Center_Point_To_Reference_Point, Convert_Object_Center_Point_To_Reference_Point_center_ref_pnt)
{
   /** \precond
    * Set obj.vcs_position to x=12,y=21
    * Set obj.reference point to front left
    * Call obj.Update_Bbox_Center()
    * Set obj_center_pseudo_pos_vcs to be equal to object center
    * Set expected position to be equal to obj.vcs_position
    */
   obj.vcs_position = Point(12.0F,21.0F);
   obj.reference_point = F360_REFERENCE_POINT_CENTER;
   obj.Update_Bbox_Center();

   obj_center_pseudo_pos_vcs = obj.bbox.Get_Center();

   const Point exp_pos_vcs = obj.vcs_position;

   /** \action
    * Call Convert_Object_Center_Point_To_Reference_Point
    */
   const Point obj_pseudo_reference_point_pos_vcs = Convert_Object_Center_Point_To_Reference_Point(obj, obj_center_pseudo_pos_vcs);

   /** \result
    * Check that the calculated pseudo position is correct.
    */
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.x, obj_pseudo_reference_point_pos_vcs.x, test_threshold, "Reference point position x coordinate in vcs is incorrect.");
   DOUBLES_EQUAL_TEXT(exp_pos_vcs.y, obj_pseudo_reference_point_pos_vcs.y, test_threshold, "Reference point position y coordinate in vcs is incorrect.");
}

/** @}*/

/** \defgroup  f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
 *  @{
 */

/** \brief
 * This test group tests the functionality of Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
 */
TEST_GROUP(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension)
{
   // Common variables used in tests
   const float32_t test_threshold = 0.0001F;
   const float32_t obj_length = 10.0F;
   const float32_t obj_width = 4.0F;

   const float32_t center_point_tcs_dimension = 2.5F;
};

/** \purpose
 * Test that this function translates a position corresponding to an object's center
 * point to instead correspond to the reference point in tcs x direction when the reference point is front
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension, Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension_front)
{
   /** \precond
    * Set seen_side to front
    */
   const F360_Object_Sides_T seen_side = F360_OBJECT_SIDES_FRONT; 

   const float32_t expected_reference_point_x = 7.5F;

   /** \action
    * Call Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
    */
   const float32_t reference_point_x = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(seen_side, obj_length, center_point_tcs_dimension);
  
   /** \result
    * Check that the calculated reference point position is correct.
    */
   DOUBLES_EQUAL_TEXT(expected_reference_point_x, reference_point_x, test_threshold, "Reference point position x coordinate is incorrect.");
}

/** \purpose
 * Test that this function translates a position corresponding to an object's center
 * point to instead correspond to the reference point in tcs x direction when the reference point is rear
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension, Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension_rear)
{
   /** \precond
    * Set seen_side to front
    */
   const F360_Object_Sides_T seen_side = F360_OBJECT_SIDES_REAR; 

   const float32_t expected_reference_point_x = -2.5F;

   /** \action
    * Call Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
    */
   const float32_t reference_point_x = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(seen_side, obj_length, center_point_tcs_dimension);
  
   /** \result
    * Check that the calculated reference point position is correct.
    */
   DOUBLES_EQUAL_TEXT(expected_reference_point_x, reference_point_x, test_threshold, "Reference point position x coordinate is incorrect.");
}

/** \purpose
 * Test that this function translates a position corresponding to an object's center
 * point to instead correspond to the reference point in tcs y direction when the reference point is right
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension, Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension_right)
{
   /** \precond
    * Set seen_side to right
    */
   const F360_Object_Sides_T seen_side = F360_OBJECT_SIDES_RIGHT; 

   const float32_t expected_reference_point_y = 4.5F;

   /** \action
    * Call Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
    */
   const float32_t reference_point_y = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(seen_side, obj_width, center_point_tcs_dimension);
  
   /** \result
    * Check that the calculated reference point position is correct.
    */
   DOUBLES_EQUAL_TEXT(expected_reference_point_y, reference_point_y, test_threshold, "Reference point position y coordinate is incorrect.");
}

/** \purpose
 * Test that this function translates a position corresponding to an object's center
 * point to instead correspond to the reference point in tcs y direction when the reference point is left
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension, Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension_left)
{
   /** \precond
    * Set seen_side to left
    */
   const F360_Object_Sides_T seen_side = F360_OBJECT_SIDES_LEFT; 

   const float32_t expected_reference_point_y = 0.5F;

   /** \action
    * Call Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
    */
   const float32_t reference_point_y = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(seen_side, obj_width, center_point_tcs_dimension);
  
   /** \result
    * Check that the calculated reference point position is correct.
    */
   DOUBLES_EQUAL_TEXT(expected_reference_point_y, reference_point_y, test_threshold, "Reference point position y coordinate is incorrect.");
}

/** \purpose
 * Test that this function translates a position corresponding to an object's center
 * point to instead correspond to the reference point when the reference point is center (-> seen side = invalid)
 * \req
 * NA
 */
TEST(f360_pseudo_position_estimation_Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension, Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension_invalid)
{
   /** \precond
    * Set seen_side to invalid
    */
   const F360_Object_Sides_T seen_side = F360_OBJECT_SIDES_INVALID; 

   const float32_t expected_reference_point = 2.5F;

   /** \action
    * Call Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension
    */
   const float32_t reference_point = Transform_Object_Center_TCS_To_Reference_Point_TCS_In_Single_Dimension(seen_side, obj_width, center_point_tcs_dimension);
  
   /** \result
    * Check that the calculated reference point position is correct.
    */
   DOUBLES_EQUAL_TEXT(expected_reference_point, reference_point, test_threshold, "Reference point position is incorrect.");
}

/** @}*/

