/** \file
 * This file contains unit tests for content of f360_calc_obj_size.cpp file
 */

#include "f360_calc_obj_size.h"
#include <CppUTest/TestHarness.h>
#include "f360_reference_point.h"
#include "f360_update_object_reference_point.h"

using namespace f360_variant_A;

/** \defgroup  f360_calc_obj_size
 *  @{
 */

 /** \brief
  * The purpose of this test group is to test the functionality of the function Calc_Obj_Size().
  */
TEST_GROUP(f360_calc_obj_size)
{
   /** \setup
    * Set up variables called by Calc_Obj_Size function.
    */
   F360_Calibrations_T calib;
   F360_Object_Track_T  obj = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T dets_raw = {};
   float32_t measured_len;
   float32_t measured_wid;

   float32_t exp_updated_len;
   float32_t exp_updated_wid;
   float32_t exp_measured_len;
   float32_t exp_measured_wid;
   float32_t exp_len_uncertainty;
   float32_t exp_wid_uncertainty;

   float32_t tolerance = 1e-6F;

   TEST_SETUP()
   {
      // Use default tracker settings for calibrations
      Initialize_Tracker_Calibrations(calib);

      /* Set up object with
         - Reference point REAR RIGHT
            - with position (7,-6)
         - Pointing 0 degrees
         - Width 2m
         - Length 6m
         - vehicular_track set to true
         - f_movable set to true 
         - Speed 10 m/s
         - 3 associated detections
      */
      obj.reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
      obj.id = 15U;
      obj.vcs_position.x = 7.0F;
      obj.vcs_position.y = -6.0F;
      obj.bbox.Set_Length(6.0F);
      obj.bbox.Set_Width(2.0F);
      obj.bbox.Set_Orientation(0.0F);
      obj.length_uncertainty = 3.0F;
      obj.width_uncertainty = 2.0F;
      obj.ndets = 3U;
      obj.detids[0] = 1;
      obj.detids[1] = 2;
      obj.detids[2] = 3;
      obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj.f_moveable = true;
      obj.f_vehicular_trk = true;
      obj.speed = 10.0F;

      /* Set up 3 associated detections
         - with positions (6,-6), (14,-6) and (10, -8.2)
         - Range > min range to update size of object 
      */
     det_props[0U].vcs_position = {6.0F, -6.0F};
     det_props[1U].vcs_position = {14.0F, -6.0F};
     det_props[2U].vcs_position = {10.0F, -8.3F};
     det_props[0U].object_track_id = obj.id;
     det_props[1U].object_track_id = obj.id;
     det_props[2U].object_track_id = obj.id;
     // Choose some arbitrary ranges for the dets
     dets_raw.detections[0U].raw.range = 30.0F;
     dets_raw.detections[1U].raw.range = 40.0F;
     dets_raw.detections[2U].raw.range = 50.0F; 
   }
};

/** \purpose
 * The purpose of this test is to check that a non-movable object is not updated (kept at default dimensions for non-movable objects).
 */
TEST(f360_calc_obj_size, Test_Non_Movable_Object_Not_Updated)
{
   /** \precond
    * Set up a non-movable CCA object with default size
   */
   obj = {};
   obj.bbox.Set_Length(calib.k_nonmoveable_target_diameter);
   obj.bbox.Set_Width(calib.k_nonmoveable_target_diameter);
   obj.length_uncertainty = 1.0F;
   obj.width_uncertainty = 2.0F;
   obj.f_moveable = false;
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   measured_len = 1.0F;
   measured_wid = 0.5F;
   float32_t init_measured_len = measured_len;
   float32_t init_measured_wid = measured_wid;
   float32_t init_width_uncertainty = obj.width_uncertainty;
   float32_t init_length_uncertainty = obj.length_uncertainty;

   exp_updated_len = calib.k_nonmoveable_target_diameter;
   exp_updated_wid = calib.k_nonmoveable_target_diameter;
   
   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(init_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(init_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(init_length_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(init_width_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Detections are set up such that 
 * the measured length/wid is bigger than the object's current size. Hence, size should grow in both directions.
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Rear_Right_Visible_Grow)
{
   /** \precond
    * An object has been set up in TEST_GROUP
   */
   exp_updated_len = 6.750156F;
   exp_updated_wid = 2.085745F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.429082F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Seven detections are associated and set up
 * such that the measured length is bigger than the object's initial size. Thus measurement uncertainty will be decreased and updated
 * length bigger.
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Rear_Right_Visible_Grow_Decreased_Uncertainty)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Add extra detections
   */
   det_props[3U].vcs_position = {9.0F, -8.2F};
   det_props[3U].object_track_id = obj.id;
   det_props[4U].vcs_position = {7.0F, -7.9F};
   det_props[4U].object_track_id = obj.id;
   det_props[5U].vcs_position = {7.5F, -7.5F};
   det_props[5U].object_track_id = obj.id;
   det_props[6U].vcs_position = {7.6F, -7.2F};
   det_props[6U].object_track_id = obj.id;
   obj.ndets = 7;
   obj.detids[3] = 4;
   obj.detids[4] = 5;
   obj.detids[5] = 6;
   obj.detids[6] = 7;

   dets_raw.detections[3U].raw.range = 30.0F;
   dets_raw.detections[4U].raw.range = 40.0F;
   dets_raw.detections[5U].raw.range = 50.0F;
   dets_raw.detections[6U].raw.range = 50.0F;

   exp_updated_len = 7.411903F;
   exp_updated_wid = 2.085745F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 0.882439F;
   exp_wid_uncertainty = 1.429082F;
   
   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Seven detections are associated and set up
 * but the para spread of detections is smaller than current object size, so measurement uncertainty is not decreased.
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Rear_Right_Visible_No_Decreased_Uncertainty)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Add extra detections
    * Decrease longitudinal gap between detections
   */
   det_props[1U].vcs_position = {8.0F, -6.0F};

   det_props[3U].vcs_position = {9.0F, -8.2F};
   det_props[3U].object_track_id = obj.id;
   det_props[4U].vcs_position = {7.0F, -7.9F};
   det_props[4U].object_track_id = obj.id;
   det_props[5U].vcs_position = {7.5F, -7.5F};
   det_props[5U].object_track_id = obj.id;
   det_props[6U].vcs_position = {7.6F, -7.2F};
   det_props[6U].object_track_id = obj.id;
   obj.ndets = 7;
   obj.detids[3] = 4;
   obj.detids[4] = 5;
   obj.detids[5] = 6;
   obj.detids[6] = 7;

   dets_raw.detections[3U].raw.range = 30.0F;
   dets_raw.detections[4U].raw.range = 40.0F;
   dets_raw.detections[5U].raw.range = 50.0F;
   dets_raw.detections[6U].raw.range = 50.0F;

   exp_updated_len = 5.249844F;
   exp_updated_wid = 2.085745F;
   exp_measured_len = 4.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.429082F;
   
   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. 11 detections are associated and set up
 * such that the measured length is bigger than the object's initial size. Thus measurement uncertainty will be decreased and updated
 * length bigger. However, decrease factor is saturated when number of detections is more than 10.
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Rear_Right_Visible_Grow_Decreased_Uncertainty_Saturated)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Add extra detections
   */
   det_props[3U].vcs_position = {9.0F, -8.2F};
   det_props[3U].object_track_id = obj.id;
   det_props[4U].vcs_position = {7.0F, -7.9F};
   det_props[4U].object_track_id = obj.id;
   det_props[5U].vcs_position = {7.5F, -7.5F};
   det_props[5U].object_track_id = obj.id;
   det_props[6U].vcs_position = {7.6F, -7.2F};
   det_props[6U].object_track_id = obj.id;
   det_props[7U].vcs_position = {7.6F, -7.2F};
   det_props[7U].object_track_id = obj.id;
   det_props[8U].vcs_position = {7.6F, -7.2F};
   det_props[8U].object_track_id = obj.id;
   det_props[9U].vcs_position = {7.6F, -7.2F};
   det_props[9U].object_track_id = obj.id;
   det_props[10U].vcs_position = {7.6F, -7.2F};
   det_props[10U].object_track_id = obj.id;
   obj.ndets = 11;
   obj.detids[3] = 4;
   obj.detids[4] = 5;
   obj.detids[5] = 6;
   obj.detids[6] = 7;
   obj.detids[7] = 8;
   obj.detids[8] = 9;
   obj.detids[9] = 10;
   obj.detids[10] = 11;
   obj.detids[11] = 12;

   dets_raw.detections[3U].raw.range = 30.0F;
   dets_raw.detections[4U].raw.range = 40.0F;
   dets_raw.detections[5U].raw.range = 50.0F;
   dets_raw.detections[6U].raw.range = 50.0F;
   dets_raw.detections[7U].raw.range = 30.0F;
   dets_raw.detections[8U].raw.range = 40.0F;
   dets_raw.detections[9U].raw.range = 50.0F;
   dets_raw.detections[10U].raw.range = 50.0F;
   dets_raw.detections[11U].raw.range = 50.0F;

   exp_updated_len = 7.875039F;
   exp_updated_wid = 2.085745F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 0.187504F;
   exp_wid_uncertainty = 1.429082F;
   
   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Detections are set up such that 
 * the measured length/wid is smaller than the object's current size. Hence, size should shrink in both directions.
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Rear_Right_Visible_Shrink)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Move detections to make measured len and wid smaller than current size
    *    - Change x pos of first detection to 9m
    *    - Change y pos of third detection to -7,5m
   */
   det_props[0U].vcs_position.x = 9.0F;
   det_props[2U].vcs_position.y = -7.5F;
   exp_updated_len = 5.624922F;
   exp_updated_wid = 1.857092F;
   exp_measured_len = 5.0F;
   exp_measured_wid = 1.5F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.429082F;

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it when only REAR side of object is visible.
 * This means that process noise is decreased and updated length should be slightly smaller comapred to the default case in TEST_GROUP. 
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Only_Rear_Visible)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set object reference point to REAR
   */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   exp_updated_len = 6.750016F;
   exp_updated_wid = 2.085745F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875039F;
   exp_wid_uncertainty = 1.429082F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it when only RIGHT side of object is visible.
 * This means that process noise is decreased and updated width should be slightly smaller comapred to the default case in TEST_GROUP. 
 */
TEST(f360_calc_obj_size, Test_Fast_Moving_Far_Away_Only_Right_Visible)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set object reference point to RIGHT
   */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   exp_updated_len = 6.750156F;
   exp_updated_wid = 2.085717F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.428622F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Detections are set up such that 
 * the measured length/wid is bigger than the object's current size. Hence, size should grow in both directions. In this test
 * the object is slow moving such that process noise is decreased and the expected updated len/wid is smaller than the default case.
 */
TEST(f360_calc_obj_size, Test_Slow_Moving_Far_Away_Rear_Right_Visible_Grow)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set object speed to 1.5 m/s, i.e. lower than threshold to use low speed proces noise
    * Note that f_vehicular_trk is still set to true, so dimension limits are kept at default
   */
   obj.speed = 1.5F;
   exp_updated_len = 6.750000F;
   exp_updated_wid = 2.085714F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875000F;
   exp_wid_uncertainty = 1.428572F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it when only REAR side of object is visible and the
 * object is slow moving with at least one detection closer than 5 m. The length should not be updated in this situation. 
 */
TEST(f360_calc_obj_size, Test_Slow_Moving_Close_Dets_Only_Rear_Visible)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set object reference point to REAR
    * Set object speed to 4 m/s (below thresh to update dimension if side is not visible)
    * Set range of one detection to 4.5 m (below range criteria)
   */
   obj.reference_point = F360_REFERENCE_POINT_REAR;
   obj.speed = 4.0F;
   dets_raw.detections[0].raw.range = 4.5F;

   // CONTINUE HERE
   exp_updated_len = obj.bbox.Get_Length();
   exp_updated_wid = 2.085745F;
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = obj.length_uncertainty;
   exp_wid_uncertainty = 1.429082F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it when only RIGHT side of object is visible and the
 * object is slow moving with at least one detection closer than 5 m. The width should not be updated in this situation. 
 */
TEST(f360_calc_obj_size, Test_Slow_Moving_Close_Dets_Only_Right_Visible)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set object reference point to RIGHT
    * Set object speed to 4 m/s (below thresh to update dimension if side is not visible)
    * Set range of one detection to 4.5 m (below range criteria)
   */
   obj.reference_point = F360_REFERENCE_POINT_RIGHT;
   obj.speed = 4.0F;
   dets_raw.detections[0].raw.range = 4.5F;
   exp_updated_len = 6.750156F;
   exp_updated_wid = obj.bbox.Get_Width();
   exp_measured_len = 8.0F;
   exp_measured_wid = 2.3F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = obj.width_uncertainty;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Detections are set up such that 
 * the measured length/wid is bigger than the maximum allowed measurements.
 */
TEST(f360_calc_obj_size, Test_Saturate_Dimensions_At_Max)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set associated detections far away enough to exceed maximum measurements in both length and width
   */
   det_props[0].vcs_position.x = -5.0F;
   det_props[1].vcs_position.x = 21.0F;
   det_props[2].vcs_position.y = -10.0F;
   exp_updated_len = 13.126484F;
   exp_updated_wid = 2.142908F;
   exp_measured_len = 25.0F;
   exp_measured_wid = 2.5F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.429082F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** \purpose
 * The purpose of this test is to check that the object size is correct after updating it. Detections are set up such that 
 * the measured length/wid is smaller than the miminum allowed measurements.
 */
TEST(f360_calc_obj_size, Test_Saturate_Dimensions_At_Min)
{
   /** \precond
    * An object has been set up in TEST_GROUP
    * Set associated detections far away enough to exceed maximum measurements in both length and width
   */
   det_props[0].vcs_position.x = 6.0F;
   det_props[1].vcs_position.x = 7.5F;
   det_props[2].vcs_position.y = -6.2F;
   det_props[2].vcs_position.x = 6.0F;
   exp_updated_len = 4.687227F;
   exp_updated_wid = 1.714184F;
   exp_measured_len = 2.5F;
   exp_measured_wid = 1.0F;
   exp_len_uncertainty = 1.875391F;
   exp_wid_uncertainty = 1.429081F;
   

   /** \action
    * Call the function Calc_Obj_Size()
    */
   Calc_Obj_Size(det_props, dets_raw, calib, measured_len, measured_wid, obj);

   /** \result
    * Check that the output object dimensions and related variables are correct after the call to Calc_Obj_Size().
    */
   DOUBLES_EQUAL_TEXT(exp_updated_len, obj.bbox.Get_Length(), tolerance, "Unexpected value for object updated length.");
   DOUBLES_EQUAL_TEXT(exp_updated_wid, obj.bbox.Get_Width(), tolerance, "Unexpected value for object updated width.");
   DOUBLES_EQUAL_TEXT(exp_measured_len, measured_len, tolerance, "Unexpected value for measured length.");
   DOUBLES_EQUAL_TEXT(exp_measured_wid, measured_wid, tolerance, "Unexpected value for measured width.");
   DOUBLES_EQUAL_TEXT(exp_len_uncertainty, obj.length_uncertainty, tolerance, "Unexpected value for object updated length uncertainty.");
   DOUBLES_EQUAL_TEXT(exp_wid_uncertainty, obj.width_uncertainty, tolerance, "Unexpected value for object updated width uncertainty.");
}

/** @}*/

/** \defgroup  f360_calc_obj_size_Determine_Min_Dimensions_For_Object
 *  @{
 */
 
/** \brief
 * The purpose of this test group is to test the functionality of the function Determine_Min_Dimensions_For_Object().
 */
TEST_GROUP(f360_calc_obj_size_Determine_Min_Dimensions_For_Object)
{
   /** \setup
    * Set up an object.
    * Set up tracker calibrations.
    * Set up a threshold for accepting equality when comparing two floats;
    */
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibs = {};
   const float32_t acceptable_equality_th = 1e-8F;

   TEST_SETUP()
   {
      // Use default tracker settings for calibrations
      Initialize_Tracker_Calibrations(calibs);
   }
};

/** \purpose
 * The purpose of this test is to check so that the function Determine_Min_Dimensions_For_Object()
 * returns the expected minimum object length and width for a slow moving CCA object object.
 */
TEST(f360_calc_obj_size_Determine_Min_Dimensions_For_Object, Test_slow_mov_CCA_Obj)
{
   /** \precond
    * Set up the object to be CCA
    * Setup objecy speed to be slow (slightly belov calibs.fast_moving_thres)
    * Set up variables to store the output from the function call in
    * Set up the expected output object dimension limitations.
    */
   // Set up object properties
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj.speed = calibs.fast_moving_thresh - 1e-3F;

   // Set up variables to store function output in
   float32_t output_min_target_length;
   float32_t output_min_target_width;

   // Set up expected data for function output
   const float32_t exp_min_target_length = calibs.k_nonmoveable_target_diameter;
   const float32_t exp_min_target_width = calibs.k_nonmoveable_target_diameter;


   /** \action
    * Call the function Determine_Min_Dimensions_For_Object()
    */
   Determine_Min_Dimensions_For_Object(obj, calibs, output_min_target_length, output_min_target_width);

   /** \result
    * Check that the output object dimensions limitations are correct after the call to Determine_Min_Dimensions_For_Object().
    */
   DOUBLES_EQUAL_TEXT(exp_min_target_length, output_min_target_length, acceptable_equality_th, "Unexpected value for object minimum length.");
   DOUBLES_EQUAL_TEXT(exp_min_target_width, output_min_target_width, acceptable_equality_th, "Unexpected value for object minimum width.");
}

/** \purpose
 * The purpose of this test is to check so that the function Determine_Min_Dimensions_For_Object()
 * returns the expected minimum object length and width for a CTCA object.
 */
TEST(f360_calc_obj_size_Determine_Min_Dimensions_For_Object, Test_CTCA_Obj)
{
   /** \precond
    * Set up the object to be CTCA
    * Set up variables to store the output from the function call in
    * Set up the expected output object dimension limitations.
    */
   // Set up object properties
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

   // Set up variables to store function output in
   float32_t output_min_target_length;
   float32_t output_min_target_width;

   // Set up expected data for function output
   const float32_t exp_min_target_length = calibs.k_min_CTCA_target_length;
   const float32_t exp_min_target_width = calibs.k_min_CTCA_target_width;


   /** \action
    * Call the function Determine_Min_Dimensions_For_Object()
    */
   Determine_Min_Dimensions_For_Object(obj, calibs, output_min_target_length, output_min_target_width);

   /** \result
    * Check that the output object dimensions limitations are correct after the call to Determine_Min_Dimensions_For_Object().
    */
   DOUBLES_EQUAL_TEXT(exp_min_target_length, output_min_target_length, acceptable_equality_th, "Unexpected value for object minimum length.");
   DOUBLES_EQUAL_TEXT(exp_min_target_width, output_min_target_width, acceptable_equality_th, "Unexpected value for object minimum width.");
}

/** @}*/


/** \defgroup  f360_calc_obj_size_Determine_Min_Dimensions_For_Object
 *  @{
 */

/** \brief
 * The purpose of this test group is to test the functionality of the function Determine_Max_Dimensions_For_Object().
 */
TEST_GROUP(f360_calc_obj_size_Determine_Max_Dimensions_For_Object)
{
   /** \setup
    * Set up an object.
    * Set up tracker calibrations.
    * Set up a threshold for accepting equality when comparing two floats;
    */
   F360_Object_Track_T obj = {};
   F360_Calibrations_T calibs = {};
   const float32_t acceptable_equality_th = 1e-8F;

   TEST_SETUP()
   {
      // Use default tracker settings for calibrations
      Initialize_Tracker_Calibrations(calibs);
   }
};

/** \purpose
 * The purpose of this test is to check so that the function Determine_Max_Dimensions_For_Object()
 * returns the expected maximum object length and width for a vehicular moveable object.
 */
TEST(f360_calc_obj_size_Determine_Max_Dimensions_For_Object, Test_Vehicular_Moveable_Obj)
{
   /** \precond
    * Set up the object to be vehicular and moveable
    * Set up variables to store the output from the function call in
    * Set up the expected output object dimension limitations.
    */
   // Set up object properties
   obj.f_moveable = true;
   obj.f_vehicular_trk = true;

   // Set up variables to store function output in
   float32_t output_max_target_length;
   float32_t output_max_target_width;

   // Set up expected data for function output
   const float32_t exp_max_target_length = calibs.k_fast_movable_max_target_length;
   const float32_t exp_max_target_width = calibs.k_movable_max_target_width;


   /** \action
    * Call the function Determine_Max_Dimensions_For_Object()
    */
   Determine_Max_Dimensions_For_Object(obj, calibs, output_max_target_length, output_max_target_width);

   /** \result
    * Check that the output object dimensions limitations are correct after the call to Determine_Max_Dimensions_For_Object().
    */
   DOUBLES_EQUAL_TEXT(exp_max_target_length, output_max_target_length, acceptable_equality_th, "Unexpected value for object maximum length.");
   DOUBLES_EQUAL_TEXT(exp_max_target_width, output_max_target_width, acceptable_equality_th, "Unexpected value for object maximum width.");
}

/** \purpose
 * The purpose of this test is to check so that the function Determine_Max_Dimensions_For_Object()
 * returns the expected maximum object length and width for a slow moveable object.
 */
TEST(f360_calc_obj_size_Determine_Max_Dimensions_For_Object, Test_Slow_Moveable_Obj)
{
   /** \precond
    * Set up the object to be slow moveable
    * Set up variables to store the output from the function call in
    * Set up the expected output object dimension limitations.
    */
   // Set up object properties
   obj.f_moveable = true;
   obj.f_vehicular_trk = false;

   // Set up variables to store function output in
   float32_t output_max_target_length;
   float32_t output_max_target_width;

   // Set up expected data for function output
   const float32_t exp_max_target_length = calibs.k_slow_movable_max_target_length;
   const float32_t exp_max_target_width = calibs.k_movable_max_target_width;


   /** \action
    * Call the function Determine_Max_Dimensions_For_Object()
    */
   Determine_Max_Dimensions_For_Object(obj, calibs, output_max_target_length, output_max_target_width);

   /** \result
    * Check that the output object dimensions limitations are correct after the call to Determine_Max_Dimensions_For_Object().
    */
   DOUBLES_EQUAL_TEXT(exp_max_target_length, output_max_target_length, acceptable_equality_th, "Unexpected value for object maximum length.");
   DOUBLES_EQUAL_TEXT(exp_max_target_width, output_max_target_width, acceptable_equality_th, "Unexpected value for object maximum width.");
}

/** \purpose
 * The purpose of this test is to check so that the function Determine_Max_Dimensions_For_Object()
 * returns the expected maximum object length and width for a non-moveable object.
 */
TEST(f360_calc_obj_size_Determine_Max_Dimensions_For_Object, Test_nonmoveable)
{
   /** \precond
    * Set object moveable flag to false
    * Set expected output object dimension limitations according to calibrations.
    */
   // Set up object properties
   obj.f_moveable = false;

   // Set up variables to store function output in
   float32_t output_max_target_length = 0.0F;
   float32_t output_max_target_width = 0.0F;

   // Set up expected data for function output
   const float32_t exp_max_target_length = 0.0F;
   const float32_t exp_max_target_width = 0.0F;

   /** \action
    * Call the function Determine_Max_Dimensions_For_Object()
    */
   Determine_Max_Dimensions_For_Object(obj, calibs, output_max_target_length, output_max_target_width);

   /** \result
    * Check that the output object dimensions limitations are correct after the call to Determine_Max_Dimensions_For_Object().
    */
   DOUBLES_EQUAL_TEXT(exp_max_target_length, output_max_target_length, acceptable_equality_th, "Unexpected value for object maximum length.");
   DOUBLES_EQUAL_TEXT(exp_max_target_width, output_max_target_width, acceptable_equality_th, "Unexpected value for object maximum width.");
}
/** @}*/