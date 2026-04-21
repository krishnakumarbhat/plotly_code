/** \file
 * This file contains unit tests for content of f360_calc_obj_size_accuracy.cpp file
 */

#include "f360_calc_obj_size_accuracy.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_calc_obj_size_accuracy
 *  @{
 */

/** \brief
*  Group of tests covers Calc_Obj_Size_Accuracy function responsible to calculate the accuracy of length and width of objects
**/
TEST_GROUP(f360_calc_obj_size_accuracy)
{
   /** \setup
   * Setting up one object, calibrations and other variables
   */
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calib;
   float32_t measured_len;
   float32_t measured_wid;
   float32_t eclipse_penalty_factor;
   float32_t filt_coef_acy;

   TEST_SETUP()
   {
      // Initialize calibrations
      Initialize_Tracker_Calibrations(calib);

      // Assign object and other variables used in the test
      object_track.status = F360_OBJECT_STATUS_INVALID;
      object_track.innovation_length = 1.0F;
      object_track.innovation_width = 2.0F;
      object_track.accuracy_length = 2.5F;
      object_track.accuracy_width = 1.5F;
      measured_len = 2.0F;
      measured_wid = 2.0F;
      eclipse_penalty_factor = 1.0F;
      filt_coef_acy = 0.3F;
   }
};

/**
*\purpose
*  Test that accuracy for length and width are correctly calculated for moveable objects
*\req   NA
*/
TEST(f360_calc_obj_size_accuracy, Calc_Obj_Size_Accuracy_moveable)
{
   /** \precond
   *  Use base setup and f_moveable flag is set to true
   **/
   object_track.f_moveable = true;
   float32_t raw_accuracy_length = object_track.innovation_length * eclipse_penalty_factor;
   float32_t raw_accuracy_width = object_track.innovation_width * eclipse_penalty_factor;
   float32_t exp_len = filt_coef_acy * object_track.accuracy_length + (1.0F - filt_coef_acy)*raw_accuracy_length;
   float32_t exp_wid = filt_coef_acy * object_track.accuracy_width + (1.0F - filt_coef_acy)*raw_accuracy_width;

   /** \action
    * Call function Calc_Obj_Size_Accuracy
    */

   /** \result
    * Accuracy for object length and width should coincide with expected data
    */
   Calc_Obj_Size_Accuracy(measured_len, measured_wid, calib, object_track);

   float32_t val_len = object_track.accuracy_length;
   float32_t val_wid = object_track.accuracy_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Accuracy length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Accuracy width does not match expected data.")
}

/**
*\purpose
*  Test that accuracy for length and width are correctly calculated for non-moveable objects
*\req   NA
*/
TEST(f360_calc_obj_size_accuracy, Calc_Obj_Size_Accuracy_nonmoveable)
{
   /** \precond
   *  Use base setup and f_moveable flag is set to false
   **/
   object_track.f_moveable = false;
   float32_t exp_acc_len = 2.0F * calib.k_max_assoc_gate_radius_non_moveable - calib.k_nonmoveable_target_diameter;
   float32_t exp_acc_wid = 2.0F * calib.k_max_assoc_gate_radius_non_moveable - calib.k_nonmoveable_target_diameter;

   /** \action
    * Call function Calc_Obj_Size_Accuracy
    */

   /** \result
    * Accuracy for object length and width should coincide with expected data
    */
   Calc_Obj_Size_Accuracy(measured_len, measured_wid, calib, object_track);

   float32_t val_acc_len = object_track.accuracy_length;
   float32_t val_acc_wid = object_track.accuracy_width;
   DOUBLES_EQUAL_TEXT(exp_acc_len, val_acc_len, F360_EPSILON, "Accuracy length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_acc_wid, val_acc_wid, F360_EPSILON, "Accuracy width does not match expected data.")
}

/** @}*/

/** \defgroup  f360_calc_obj_size_innovation
 *  @{
 */

/** \brief
*  Group of tests covers Calc_Obj_Size_Innovation function responsible to calculate the innovation length and width of objects
**/
TEST_GROUP(f360_calc_obj_size_innovation)
{
   /** \setup
   * Setting up one object, calibrations and other variables
   */
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calib;
   float32_t measured_len;
   float32_t measured_wid;
   float32_t innov_no_update_length;
   float32_t innov_no_update_width;
   float32_t time_for_coasted_objects;
   float32_t filt_coef_innov_coasting_obj;

   TEST_SETUP()
   {
      // Initialize calibrations
      Initialize_Tracker_Calibrations(calib);

      // Assign commonly used variables
      object_track.innovation_length = 1.0F;
      object_track.innovation_width = 2.0F;
      innov_no_update_length = 6.0F;
      innov_no_update_width = 2.0F;
      measured_len = 0.0F;
      measured_wid = 0.0F;
      time_for_coasted_objects = 0.055F;
      filt_coef_innov_coasting_obj = 0.9F;
   }
};

/**
*\purpose
*  Test that innovation length and width are correctly set for object with status F360_OBJECT_STATUS_NEW
*\req   NA
*/
TEST(f360_calc_obj_size_innovation, Calc_Obj_Size_Innovation_object_status_new)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_NEW
   **/
   object_track.status = F360_OBJECT_STATUS_NEW;

   float32_t exp_len = innov_no_update_length;
   float32_t exp_wid = innov_no_update_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that innovation length and width are correctly set for object with status F360_OBJECT_STATUS_NEW_UPDATED
*\req   NA
*/
TEST(f360_calc_obj_size_innovation, Calc_Obj_Size_Innovation_object_status_new_updated)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_NEW_UPDATED
   **/
   object_track.status = F360_OBJECT_STATUS_NEW_UPDATED;

   float32_t exp_len = innov_no_update_length;
   float32_t exp_wid = innov_no_update_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that innovation length and width are correctly set for object with status F360_OBJECT_STATUS_NEW_COASTED
*\req  NA
*/
TEST(f360_calc_obj_size_innovation, Calc_Obj_Size_Innovation_object_status_new_coasted)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_NEW_COASTED
   **/
   object_track.status = F360_OBJECT_STATUS_NEW_COASTED;

   float32_t exp_len = innov_no_update_length;
   float32_t exp_wid = innov_no_update_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that innovation length and width are correctly calculated for object with status F360_OBJECT_STATUS_COASTED
*  and with time_since_track_updated greater than calib.k_hyst_time_for_coasted_objects
*\req  NA
*/
TEST(f360_calc_obj_size_innovation, Calc_Obj_Size_Innovation_object_status_coasted_and_time_since_update)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_COASTED
   *  object time_since_track_updated set to greater than k_hyst_time_for_coasted_objects
   **/
   object_track.status = F360_OBJECT_STATUS_COASTED;
   object_track.time_since_track_updated = time_for_coasted_objects + 0.05F;

   float32_t exp_len = filt_coef_innov_coasting_obj * object_track.innovation_length + (1.0F - filt_coef_innov_coasting_obj) * innov_no_update_length;
   float32_t exp_wid = filt_coef_innov_coasting_obj * object_track.innovation_width + (1.0F - filt_coef_innov_coasting_obj) * innov_no_update_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/** @}*/

/** \defgroup  f360_determine_measured_length_and_width
 *  @{
 */

/** \brief
*  Group of tests covers Calc_Obj_Size_Innovation (calling Determine_Measured_Length and Determine_Measured_Width functions)
*  responsible to calculate the innovation length and width of objects
**/
TEST_GROUP(f360_determine_measured_length_and_width)
{
   /** \setup
   * Setting up one object, calibrations and other variables
   */
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calib;
   float32_t time_for_coasted_objects;
   float32_t filt_coef_innov_updated_obj;

   TEST_SETUP()
   {
      // Initialize calibrations
      Initialize_Tracker_Calibrations(calib);

      // Assign commonly used variables
      object_track.innovation_length = 1.0F;
      object_track.innovation_width = 2.0F;
      object_track.Update_Bbox_Size(1.6F, 1.0F);

      time_for_coasted_objects = 0.055F;
      filt_coef_innov_updated_obj = 0.9F;
   }
};

/**
*\purpose
*  Test that object innovation length are calculated correctly in Determine_Measured_Length and Determine_Measured_Width
*  with object status F360_OBJECT_STATUS_UPDATED, time_since_track_updated greater than calib.k_hyst_time_for_coasted_objects
*  and measured lengths and widths all set to zero
*\req  NA
*/
TEST(f360_determine_measured_length_and_width, check_determine_measured_lengths_and_widths_zero)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_UPDATED
   *  object time since updated set to greater than k_hyst_time_for_coasted_objects
   *  measured_len1, measured_len2, measured_wid1 and measured_wid2 set to zero
   *  set up variables and calculate expected values
   **/
   object_track.status = F360_OBJECT_STATUS_UPDATED;
   object_track.time_since_track_updated = time_for_coasted_objects + 0.05F;
   float32_t measured_len = 0.0F;
   float32_t measured_wid = 0.0F;

   float32_t measured_length = 0.0F;
   float32_t measured_width = 0.0F;
   float32_t error_length = std::abs(measured_length - object_track.bbox.Get_Length());
   float32_t error_width = std::abs(measured_width - object_track.bbox.Get_Width());
   float32_t exp_len = filt_coef_innov_updated_obj * object_track.innovation_length + (1.0F - filt_coef_innov_updated_obj) * error_length;
   float32_t exp_wid = filt_coef_innov_updated_obj * object_track.innovation_width + (1.0F - filt_coef_innov_updated_obj) * error_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that object innovation length are calculated correctly in Determine_Measured_Length and Determine_Measured_Width
*  with object status F360_OBJECT_STATUS_COASTED, time_since_track_updated less than calib.k_hyst_time_for_coasted_objects
*  and measured lengths and widths all set to greater than zero
*\req  NA
*/
TEST(f360_determine_measured_length_and_width, check_determine_measured_lengths_and_widths_not_zero)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_COASTED
   *  object time since updated set to less than k_hyst_time_for_coasted_objects
   *  measured_len1 set to greater than zero
   *  measured_len2 set to greater than zero
   *  measured_wid1 set to greater than zero
   *  measured_wid2 set to greater than zero
   *  set up variables and calculate expected values
   **/
   object_track.status = F360_OBJECT_STATUS_COASTED;
   object_track.time_since_track_updated = time_for_coasted_objects - 0.05F;
   float32_t measured_len = 2.0F;
   float32_t measured_wid = 2.0F;

   float32_t error_length = std::abs(measured_len - object_track.bbox.Get_Length());
   float32_t error_width = std::abs(measured_wid - object_track.bbox.Get_Width());
   float32_t exp_len = filt_coef_innov_updated_obj * object_track.innovation_length + (1.0F - filt_coef_innov_updated_obj) * error_length;
   float32_t exp_wid = filt_coef_innov_updated_obj * object_track.innovation_width + (1.0F - filt_coef_innov_updated_obj) * error_width;


   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that object innovation length are calculated correctly in Determine_Measured_Length and Determine_Measured_Width
*  with object status F360_OBJECT_STATUS_UPDATED, time_since_track_updated less than calib.k_hyst_time_for_coasted_objects
*  and measured_len1 and measured_wid1 set to zero and measured_len2 and measured_wid2 set to greater than zero
*\req  NA
*/
TEST(f360_determine_measured_length_and_width, check_determine_measured_len1_and_wid1_zero)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_UPDATED
   *  object time since updated set to less than k_hyst_time_for_coasted_objects
   *  measured_len1 set to zero
   *  measured_len2 set to greater than zero
   *  measured_wid1 set to zero
   *  measured_wid2 set to greater than zero
   *  set up variables and calculate expected values
   **/
   object_track.status = F360_OBJECT_STATUS_UPDATED;
   object_track.time_since_track_updated = time_for_coasted_objects - 0.05F;
   float32_t measured_len = 1.1F;
   float32_t measured_wid = 0.5F;

   float32_t error_length = std::abs(measured_len - object_track.bbox.Get_Length());
   float32_t error_width = std::abs(measured_wid - object_track.bbox.Get_Width());
   float32_t exp_len = filt_coef_innov_updated_obj * object_track.innovation_length + (1.0F - filt_coef_innov_updated_obj) * error_length;
   float32_t exp_wid = filt_coef_innov_updated_obj * object_track.innovation_width + (1.0F - filt_coef_innov_updated_obj) * error_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/**
*\purpose
*  Test that object innovation length are calculated correctly in Determine_Measured_Length and Determine_Measured_Width
*  with object status F360_OBJECT_STATUS_UPDATED, time_since_track_updated greater than calib.k_hyst_time_for_coasted_objects
*  and measured_len1 and measured_wid1 set to greater than zero and measured_len2 and measured_wid2 set to zero
*\req  NA
*/
TEST(f360_determine_measured_length_and_width, check_determine_measured_len2_and_wid2_zero)
{
   /** \precond
   *  object status set to F360_OBJECT_STATUS_UPDATED
   *  object time since updated set to less than k_hyst_time_for_coasted_objects
   *  measured_len1 set to greater than zero
   *  measured_len2 set to zero
   *  measured_wid1 set to greater than zero
   *  measured_wid2 set to zero
   *  set up variables and calculate expected values
   **/
   object_track.status = F360_OBJECT_STATUS_UPDATED;
   object_track.time_since_track_updated = time_for_coasted_objects - 0.05F;
   float32_t measured_len = 1.0F;
   float32_t measured_wid = 1.0F;
   float32_t error_length = std::abs(measured_len - object_track.bbox.Get_Length());
   float32_t error_width = std::abs(measured_wid - object_track.bbox.Get_Width());
   float32_t exp_len = filt_coef_innov_updated_obj * object_track.innovation_length + (1.0F - filt_coef_innov_updated_obj) * error_length;
   float32_t exp_wid = filt_coef_innov_updated_obj * object_track.innovation_width + (1.0F - filt_coef_innov_updated_obj) * error_width;

   /** \action
    * Call function Calc_Obj_Size_Innovation
    */

   /** \result
    * Object innovation length and width should coincide with expected data
    */
   Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, object_track);
   float32_t val_len = object_track.innovation_length;
   float32_t val_wid = object_track.innovation_width;
   DOUBLES_EQUAL_TEXT(exp_len, val_len, F360_EPSILON, "Innovation length does not match expected data.")
   DOUBLES_EQUAL_TEXT(exp_wid, val_wid, F360_EPSILON, "Innovation width does not match expected data.")
}

/** @}*/
