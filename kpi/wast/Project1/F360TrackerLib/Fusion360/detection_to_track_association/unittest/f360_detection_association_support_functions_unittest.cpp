/** \file
 * This file contains unit tests for content of f360_detection_association_support_functions.cpp file
 */

#include "f360_detection_association_support_functions.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_set_variant.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;


/** \defgroup  f360_Test_Is_Det_Allowed_To_Associate
*  @{
*/
/** \brief
*  Included tests related to calculation of detection is allowed to associate
*  based flags and properties. Note that this test group also derives coverage
*  of Are_Det_And_Obj_On_Same_Guardrail_Side()
**/
TEST_GROUP(f360_Test_Is_Det_Allowed_To_Associate)
{

   F360_Detection_Props_T det_prop = {};
   rspp_variant_A::RSPP_Detection_T det_raw{};
   F360_Object_Track_T obj_track = {};
   F360_Calibrations_T calib = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   /** \setup
   * Setup test so that the highest complexity branch passes.
   * Then tweak these parameters to reach full branch coverage in
   * each individual test
   **/
   TEST_SETUP()
   {
      // Detection data
      det_prop.f_ok_to_use = true;
      det_prop.on_sep_id = F360_INVALID_UNSIGNED_ID;
      det_raw.processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
      det_prop.behind_sep_id = F360_INVALID_UNSIGNED_ID;
      det_raw.raw.f_bistatic = false;

      // Object data
      obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj_track.f_moving = true;
      obj_track.mirror_prob = 0.0F;
      obj_track.behind_sep_id = F360_INVALID_UNSIGNED_ID;
      obj_track.vcs_heading = Angle{ 0.0F };
      obj_track.vcs_position.x = 0.0F;
      obj_track.vcs_position.y = 8.0F;
      Point center = obj_track.vcs_position;
      obj_track.bbox.Set_Center(center);
      obj_track.reference_point = F360_REFERENCE_POINT_CENTER;
      obj_track.f_behind_sep_ambiguous = false;

      sep_boxes[0] = BoundingBox(Point(-5.0F, 5.0F), Point(5.0F, 11.0F));

      // Init calibrations as defaults
      Initialize_Tracker_Calibrations(calib);
   }
};

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when all conditions
*          fulfilled in Is_Det_Allowed_To_Associate().
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate)
{
   /** \precond
   * Object and detection properties set to fulfill all conditions of Is_Det_Allowed_To_Associate in TEST_SETUP.
   **/

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when all conditions
*          fulfilled in Is_Det_Allowed_To_Associate().
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_CCA)
{
   /** \precond
   * Object and detection properties set to fulfill all conditions of Is_Det_Allowed_To_Associate in TEST_SETUP.
   **/

   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/

   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns false when the object is behind a SEP and detection is not
*          and the object is in the SEP association zone.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Not_Allowed_Due_To_SEP)
{
   /** \precond
   * Object and detection properties set to fulfill all conditions of Is_Det_Allowed_To_Associate in TEST_SETUP.
   * Set object's behind_sep_id to 1.
   **/
   obj_track.behind_sep_id = 1U;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is false
   **/
   CHECK_FALSE_TEXT(f_allowed_to_associate, "Detection should not be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when the object is behind a SEP and detection is not
*          but the object is not in the SEP association zone.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Allowed_Due_To_Not_In_SEP_Box)
{
   /** \precond
   * Object and detection properties set to fulfill all conditions of Is_Det_Allowed_To_Associate in TEST_SETUP.
   * Set object's behind_sep_id to 1.
   * Set object's lateral position to 20, i.e. outside of the SEP association box
   **/
   obj_track.behind_sep_id = 1U;
   obj_track.vcs_position.y = 20.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when detection is flagged as
*          on guardrail and the object is a moving CCV track.
*\req    N/A
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_On_Guardrail_Obj_Moving_CCV)
{
   /** \precond
   * Set object to CCV
   * Set detections to be on guardrail
   **/
   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCV;
   det_prop.on_sep_id = 1;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}


/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns false when detection is flagged as
*          on guardrail and the object is a fast moving CCA track.
*\req    N/A
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_On_Guardrail_Obj_Fast_Moving_CCA)
{
   /** \precond
   * Set object to CCA
   * Set object speed to just above calib.k_fast_moving_thres
   * Set detections to be on guardrail
   **/
   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_track.speed = calib.fast_moving_thresh + 1e-3F;
   det_prop.on_sep_id = 1;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that flag f_allowed_to_associate is false
   **/
   CHECK_FALSE_TEXT(f_allowed_to_associate, "Detection should not be ok for association")
}


/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when detection is flagged as
*          on guardrail and the object is a slow moving CCA track.
*\req    N/A
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_On_Guardrail_Obj_Slow_Moving_CCA)
{
   /** \precond
   * Set object to CCA
   * Set object speed to just below calib.k_fast_moving_thres
   * Set detections to be on guardrail
   **/
   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj_track.speed = calib.fast_moving_thresh - 1e-3F;
   det_prop.on_sep_id = 1;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}


/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when the detection is flagged as
*          on guardrail but the object is not flagged as moving
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_On_Guardrail_Obj_Not_Moving)
{
   /** \precond
   * Set object to moving
   * Set detections to be on guardrail
   **/
   obj_track.f_moving = false;
   det_prop.on_sep_id = 1;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is false
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true due to object having a high mirror probability
*          but the detection motion status is moving which should allow association.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Mirror_Obj_Moving_Detection)
{
   /** \precond
   * Set object mirror probability high
   * Detection motion status set to moving in TEST_SETUP
   *
   **/
   obj_track.mirror_prob = 0.9F;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}


/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when object and detection are both behind guardrail.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Object_And_Det_Behind_Guardrail_Obj_Moving)
{
   /** \precond
   * Set object behind guardrail
   * Set detection behind guardrail
   * Object set to moving in TEST_SETUP
   * Object's position is set within SEP's association zone
   **/
   obj_track.vcs_position.y = 8.0F;
   obj_track.behind_sep_id = 1;
   det_prop.behind_sep_id = 1;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when object is not moving. Detection and object
*          set to not behind guardrail.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Object_Not_Moving)
{
   /** \precond
   * Set object to not moving
   * Detection and object both set to not behind guardrail in TEST_SETUP
   **/
   obj_track.f_moving = false;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should not be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when object and detection are on opposite side of guardrail
*          while object is flagged as not moving. Here the object is behind guardrail while the detection is not.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Obj_Behind_Guardrail_But_Not_Moving)
{
   /** \precond
   * Set object behind guardrail
   * Detection set to not behind guardrail in TEST_SETUP
   * Object flagged as not moving.
   * Object's position is set within SEP's association zone
   **/
   obj_track.vcs_position.y = 8.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);
   obj_track.behind_sep_id = 1;
   obj_track.f_moving = false;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true when object and detection are on opposite side of guardrail
*          while object is flagged as not moving. Here the detection is behind guardrail while the object is not.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_Behind_Guardrail_Obj_Not_Moving)
{
   /** \precond
   * Set detection behind guardrail
   * Object set to not behind guardrail in TEST_SETUP
   * Object flagged as not moving.
   * Object's position is set within SEP's association zone
   **/
   obj_track.vcs_position.y = 6.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);
   det_prop.behind_sep_id = 1;
   obj_track.f_moving = false;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  This test will test that Is_Det_Allowed_To_Associate() returns true due to object and detection both being behind guardrail
*          while object is flagged as not moving.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_And_Obj_Behind_Guardrail_Obj_Not_Moving)
{
   /** \precond
   * Set detection behind guardrail
   * Set object behind guardrail
   * Object flagged as not moving.
   * Object's position is set within SEP's association zone
   **/
   obj_track.vcs_position.y = 8.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);
   det_prop.behind_sep_id = 1;
   obj_track.behind_sep_id = 1;
   obj_track.f_moving = false;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  Purpose of this test is to verify whether bistatic dets are allowed to associate to CTCA objects.
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_Is_Bistatic_CTCA_Object)
{
   /** \precond
   * Set required properties to make detection be allowed to associate
   * Set detection f_bistatic flag as true
   * Set object tracker filter type as CTCA
   **/
   det_prop.behind_sep_id = 1;
   obj_track.behind_sep_id = 1;
   obj_track.f_moving = false;

   det_raw.raw.f_bistatic = true;

   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  Purpose of this test is to verify whether bistatic dets are allowed to associate to CCA objects if they are fast moving (speed above calib.fast_moving_thresh).
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_Is_Bistatic_CCA_Object_Fast_Moving)
{
   /** \precond
   * Set required properties to make detection be allowed to associate
   * Set detection f_bistatic flag as true
   * Set object tracker filter type as CCA
   * Set object speed to just above calib.fast_moving_thresh
   **/
   det_prop.behind_sep_id = 1;
   obj_track.behind_sep_id = 1;
   obj_track.f_moving = false; // To prevent the detection on guardrail countermeasure to kick in so that test can focus only on bistatic countermeasure
   obj_track.speed = calib.fast_moving_thresh + 1e-3F;

   det_raw.raw.f_bistatic = true;

   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is true
   **/
   CHECK_TRUE_TEXT(f_allowed_to_associate, "Detection should be ok for association")
}

/**
*\purpose  Purpose of this test is to verify whether bistatic dets are not allowed to associate to CCA objects if they are slow moving (below above calib.fast_moving_thresh).
*\req    NA
*/
TEST(f360_Test_Is_Det_Allowed_To_Associate, Test_Is_Det_Allowed_To_Associate_Det_Is_Bistatic_CCA_Object_Slow_Moving)
{
   /** \precond
   * Set required properties to make detection be allowed to associate
   * Set detection f_bistatic flag as true
   * Set object tracker filter type as CCA
   * Set object speed to just below calib.fast_moving_thresh
   **/
   det_prop.behind_sep_id = 1;
   obj_track.behind_sep_id = 1;
   obj_track.f_moving = false; // To prevent the detection on guardrail countermeasure to kick in so that test can focus only on bistatic countermeasure
   obj_track.speed = calib.fast_moving_thresh - 1e-3F;

   det_raw.raw.f_bistatic = true;

   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

   /** \action
   *Call Is_Det_Allowed_To_Associate
   **/
   bool f_allowed_to_associate = Is_Det_Allowed_To_Associate(
      det_prop,
      det_raw,
      obj_track,
      calib,
      sep_boxes);

   /** \result
   * Check that detection flag f_allowed_to_associate is false
   **/
   CHECK_FALSE_TEXT(f_allowed_to_associate, "Detection should not be ok for association")
}
/** @}*/

/** \defgroup  f360_Test_Calc_Range_Rate_Threshold
*  @{
*/
/** \brief
*  Included tests related to calculation of range rate threshold for detection association for moveable objects
**/
TEST_GROUP(f360_Test_Calc_Range_Rate_Threshold)
{

   F360_Object_Track_T obj_track;
   rspp_variant_A::RSPP_Detection_T det_raw{};
   F360_Detection_Props_T det_prop;
   F360_Calibrations_T calib = {};
   F360_Radar_Sensor_T sensor = {};
   float32_t range_rate_score_threshold;
   float32_t host_vcs_speed;

   float32_t exp_fov_edge_result;
   float32_t exp_far_coasted_result;
   float32_t exp_general_result;

   /** \setup
   * Setup test so that the highest complexity branch passes.
   * Then tweak these parameters to reach full branch coverage in
   * each individual test
   **/
   TEST_SETUP()
   {
      obj_track.vcs_position.x = 50.0F;
      obj_track.vcs_position.y = 10.0F;
      obj_track.status = F360_OBJECT_STATUS_COASTED;
      obj_track.speed = 10.0F;
      obj_track.f_moveable = true;
      obj_track.time_since_stage_start = 1.0F;
      obj_track.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = obj_track.vcs_position;
      obj_track.bbox.Set_Center(center);

      det_prop.f_FOV_edge = false;

      Initialize_Tracker_Calibrations(calib);

      // This is constant for all tests
      range_rate_score_threshold = 2.0F;
      host_vcs_speed = 5.0F;

      // Three possible return values
      exp_fov_edge_result = range_rate_score_threshold * calib.k_rr_thr_factor_fov_edge;
      exp_far_coasted_result = range_rate_score_threshold * calib.k_rr_thr_factor_far_away_coasted;
      exp_general_result = calib.k_range_rate_score_threshold;
   }

};

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "far away coasting track" range rate threshold is expected
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_A)
{
   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   * The expected value depends on currently hardcoded parameters in function
   **/
   DOUBLES_EQUAL_TEXT(exp_far_coasted_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "general" range rate threshold is expected due to object speed too low
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_B)
{
   /** \precond
   * Set object speed beneath threshold
   **/
   obj_track.speed = 1.0F;

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL_TEXT(exp_general_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "general" range rate threshold is expected due to object not too far away
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_C)
{
   /** \precond
   * Set object position close
   **/
   obj_track.vcs_position.x = 10.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL_TEXT(exp_general_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "general" range rate threshold is expected due to object stage age too young
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_D)
{
   /** \precond
   * Set object position close
   **/
   obj_track.time_since_stage_start = 0.0F;

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL_TEXT(exp_general_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "general" range rate threshold is expected due to object is updated
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_E)
{
   /** \precond
   * Set object position close
   **/
   obj_track.status = F360_OBJECT_STATUS_UPDATED;

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL_TEXT(exp_general_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test Calc_Range_Rate_Threshold() where "FOV edge" range rate threshold is expected due to detection
*          is a "field of view edge" detection
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_F)
{
   /** \precond
   * Set detections flag to true
   **/
   det_prop.f_FOV_edge = true;

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(
      obj_track,
      det_raw,
      det_prop,
      sensor,
      calib,
      host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL_TEXT(exp_fov_edge_result, rdot_thres, F360_EPSILON, "Range rate threshold do not match expected threshold")
}

/**
*\purpose  This test will test if proper value is returned from Calc_Range_Rate_Threshold()
           when detection has f_elevation_unreliable flag is set.
*\req    NA
*/
TEST(f360_Test_Calc_Range_Rate_Threshold, Test_Calc_Range_Rate_Threshold_Moveable_Object_For_Unreliable_Elevation)
{
   /** \precond
   * Set detections flag to true
   **/
   det_raw.raw.elevation = calib.k_mrr360_max_abs_elev_angle_rad + 0.1F;
   sensor.constant.sensor_type = F360_SENSOR_TYPE_MRR360_RADAR;

   /** \action
   *Call Calc_Range_Rate_Threshold
   **/
   float32_t rdot_thres = Calc_Range_Rate_Threshold(obj_track, det_raw, det_prop, sensor, calib, host_vcs_speed);

   /** \result
   * Check that the range rate threshold matches expected value.
   **/
   DOUBLES_EQUAL(exp_fov_edge_result, rdot_thres, F360_EPSILON)
}

/** \defgroup  Assign_Association_Hypothesis
*  @{
*/
/** \brief
* This test group includes test of the function Assign_Association_Hypothesis() defined in
* f360_detection_association_support_functions.cpp.
**/
TEST_GROUP(f360_Test_Assign_Association_Hypothesis)
{
   // Declare common variables used within all tests in this test group.
   uint32_t number_of_valid_detections;
   float32_t det_rdot_comp_array[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Tracker_Info_T tracker_info = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detections_list{};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   /** \setup
   * Set up a default scenario where a detection is associated to an object successfully.
   */
   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);

      raw_detections_list.number_of_valid_detections = 1U;
      det_rdot_comp_array[0] = 1.0F;

      object_tracks[0].ndets = 0;
      object_tracks[0].id = 1;

      tracker_info.num_active_objs = 1;
      tracker_info.active_obj_ids[0] = object_tracks[0].id;


      detection_props[0].object_track_id = object_tracks[0].id;

   }

   void Create_Detection_On_Obj_Edge_And_Assoc_To_Obj(
         F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Object_Track_T & object_track,
         const uint32_t det_idx)
   {
      detection_props[det_idx].vcs_position.x = object_track.bbox.Get_Center().x - object_track.bbox.Get_Length() * 0.5F;
      detection_props[det_idx].vcs_position.y = object_track.bbox.Get_Center().y - object_track.bbox.Get_Width() * 0.5F;
      detection_props[det_idx].object_track_id = object_track.id;
   }
};

/**
*\purpose Check that Assign_Association_Hypothesis() works as intended when a detection has been associated to an object correctly
*\req NA
*/
TEST(f360_Test_Assign_Association_Hypothesis, AssignAssociationHypothesis_AssocDet)
{
   /** \precond
   * Nothing needs to change from the setup in this test.
   */

   /** \action
   * Call Assign_Association_Hypothesis()
   **/
   Assign_Association_Hypothesis(det_rdot_comp_array, tracker_info, raw_detections_list, detection_props, object_tracks);

   /** \result
   * Check that the corresponding detection properties have been modified as expected.
   **/
   DOUBLES_EQUAL_TEXT(det_rdot_comp_array[0], detection_props[0].range_rate_compensated, F360_EPSILON, "Detection property range_rate_compensated was not set as expected.")
   CHECK_EQUAL_TEXT(true, detection_props[0].f_dealiased, "Detection property f_inlier was not set as expected.")
}

/**
*\purpose Check that Assign_Association_Hypothesis() works as intended when the slots for detection association for a track are full.
*\req NA
*/
TEST(f360_Test_Assign_Association_Hypothesis, AssignAssociationHypothesis_MaxNumDetsForTrack)
{
   /** \precond
   * Set number of valid detections to MAX_DETS_IN_OBJ_TRK + 1
   * create MAX_DETS_IN_OBJ_TRK detections on object edge that all want to associate to the obj
   * Create a detection that wants to associate to object but 1m from edge.
   * Fill range rate compensated array for the detections to 1
   * Set expected range rate compensated to 1 (for MAX_DETS_IN_OBJ_TRK best detections)
   */
   raw_detections_list.number_of_valid_detections = MAX_DETS_IN_OBJ_TRK + 1U;
   float32_t expected_rr_comp = 1.0F;

   for (uint32_t det_idx = 0U; det_idx < MAX_DETS_IN_OBJ_TRK + 1U; det_idx++)
   {
      Create_Detection_On_Obj_Edge_And_Assoc_To_Obj(detection_props, object_tracks[0], det_idx);
      det_rdot_comp_array[det_idx] = 1.0F;
   }
   detection_props[MAX_DETS_IN_OBJ_TRK].vcs_position.x += 1.0F;

   /** \action
   * Call Assign_Association_Hypothesis()
   **/
   Assign_Association_Hypothesis(det_rdot_comp_array, tracker_info, raw_detections_list, detection_props, object_tracks);

   /** \result
   * Check that the corresponding detection properties have been modified as expected.
   **/
   for (uint32_t det_idx = 0U; det_idx < MAX_DETS_IN_OBJ_TRK; det_idx++)
   {
      DOUBLES_EQUAL_TEXT(expected_rr_comp, detection_props[det_idx].range_rate_compensated, F360_EPSILON, "Detection property range_rate_compensated was not set as expected.")
      CHECK_TRUE_TEXT(detection_props[det_idx].f_dealiased, "Detection property f_inlier was not set as expected.")
      CHECK_EQUAL_TEXT(1, detection_props[det_idx].object_track_id, "Detection property object_track_id is not correct.")

   }
   DOUBLES_EQUAL_TEXT(0.0F, detection_props[MAX_DETS_IN_OBJ_TRK].range_rate_compensated, F360_EPSILON, "Detection property range_rate_compensated was not set as expected.")
   CHECK_FALSE_TEXT(detection_props[MAX_DETS_IN_OBJ_TRK].f_dealiased, "Detection property f_inlier was not set as expected.")
   CHECK_EQUAL_TEXT(0, detection_props[MAX_DETS_IN_OBJ_TRK].object_track_id, "Detection property object_track_id is not correct.")
}

/**
*\purpose Check that Assign_Association_Hypothesis() works as intended when the detection has not been associated to any object,
* so it's object_track_id is equal to 0.
*\req NA
*/
TEST(f360_Test_Assign_Association_Hypothesis, AssignAssociationHypothesis_NotAssocDet)
{
   /** \precond
   * Set field object_track_id to 0 for the detection.
   */
   detection_props[0].object_track_id = 0;

   /** \action
   * Call Assign_Association_Hypothesis()
   **/
   Assign_Association_Hypothesis(det_rdot_comp_array, tracker_info, raw_detections_list, detection_props, object_tracks);

   /** \result
   * Check that the corresponding detection properties have been modified as expected.
   **/
   DOUBLES_EQUAL_TEXT(0.0F, detection_props[0].range_rate_compensated, F360_EPSILON, "Detection property range_rate_compensated was not set as expected.")
   CHECK_EQUAL_TEXT(false, detection_props[0].f_dealiased, "Detection property f_inlier was not set as expected.")
}
/** @}*/


/** \defgroup  f360_calculate_detection_association_cost_rear_left_corner_moveable_obj
*  @{
*/
/** \brief
* This test group includes test of the function Calculate_Detection_Association_Cost(). The tests will be based on a detection
* on rear left corner of a moveable object.
**/
TEST_GROUP(f360_calculate_detection_association_cost_rear_left_corner_moveable_obj)
{
   // Declare common variables used within all tests in this test group.
   F360_Detection_Props_T det_prop = {};
   F360_Object_Track_T object_track = {};
   float32_t test_pass_threshold = 1e-5F;
   /** \setup
    * Place a moveable object with size 4x2m at (10, 10) in VCS with VCS pointing angle of 0
    * Set up a default scenario where a detection is placed on rear left object corner.
    */
   TEST_SETUP()
   {
      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 10.0F;
      object_track.Set_Bbox_Orientation(Angle{ 0.0F });
      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = object_track.vcs_position;
      object_track.bbox.Set_Center(center);
      object_track.bbox.Set_Length(4.0F);
      object_track.bbox.Set_Width(2.0F);
      object_track.f_moveable = true;


      Create_Detection_On_Obj_Edge(det_prop, object_track);

   }

   void Create_Detection_On_Obj_Edge(
         F360_Detection_Props_T & det_prop,
         const F360_Object_Track_T & object_track)
   {
      det_prop.vcs_position.x = object_track.vcs_position.x - object_track.bbox.Get_Length() * 0.5F;
      det_prop.vcs_position.y = object_track.vcs_position.y - object_track.bbox.Get_Width() * 0.5F;
   }
};

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed on the rear left object edge.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_rear_left_corner_moveable_obj, Calculate_Detection_Association_Cost_Zero_Cost)
{
   /** \precond
   * Nothing needs to change from the setup in this test.
   */
   float32_t expected_det_cost = 0.0F;

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed outside object.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_rear_left_corner_moveable_obj, Calculate_Detection_Association_Cost_Detection_Orthogonal_To_Obj_Rear)
{
   /** \precond
   * A detection has been set up in test group on rear left corner of the object.
   * Move the detection 1m towards host in longitudinal direction
   */
   det_prop.vcs_position.x -= 1.0F;
   float32_t expected_det_cost = 1.0F;

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed outside object in
*         both longitudinal and lateral direction.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_rear_left_corner_moveable_obj, Calculate_Detection_Association_Cost_Detection_Outside_Rear_Left_Corner)
{
   /** \precond
   * A detection has been set up in test group on rear left corner of the object.
   * Move the detection 1m towards host in both longitudinal and lateral direction
   */
   det_prop.vcs_position.x -= 1.0F;
   det_prop.vcs_position.y -= 1.0F;
   float32_t expected_det_cost = 1.41421356F;

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}
/** @}*/

/** \defgroup  f360_calculate_detection_association_cost_front_right_corner_moveable_obj
*  @{
*/
/** \brief
* This test group includes test of the function Calculate_Detection_Association_Cost(). The tests will be based on a detection
* on front right corner of a moveable object.
**/
TEST_GROUP(f360_calculate_detection_association_cost_front_right_corner_moveable_obj)
{
   // Declare common variables used within all tests in this test group.
   F360_Detection_Props_T det_prop = {};
   F360_Object_Track_T object_track = {};
   float32_t test_pass_threshold = 1e-5F;
   /** \setup
    * Place a moveable object with size 4x2m at (10, 10) in VCS with VCS pointing angle of 0
    * Set up a default scenario where a detection is placed on front right object corner.
    */
   TEST_SETUP()
   {
      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 10.0F;
      object_track.Set_Bbox_Orientation(Angle{ 0.0F });
      Point center = object_track.vcs_position;
      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      object_track.bbox.Set_Center(center);
      object_track.bbox.Set_Length(4.0F);
      object_track.bbox.Set_Width(2.0F);
      object_track.f_moveable = true;

      Create_Detection_On_Obj_Edge(det_prop, object_track);

   }

   void Create_Detection_On_Obj_Edge(
         F360_Detection_Props_T & det_prop,
         const F360_Object_Track_T & object_track)
   {
      det_prop.vcs_position.x = object_track.vcs_position.x + object_track.bbox.Get_Length() * 0.5F;
      det_prop.vcs_position.y = object_track.vcs_position.y + object_track.bbox.Get_Width() * 0.5F;
   }
};

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed on rear right corner of object.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_front_right_corner_moveable_obj, Calculate_Detection_Association_Cost_Zero_Cost)
{
   /** \precond
   * A detection has been set up in test group on front right corner of the object.
   */
   float32_t expected_det_cost = 0.0F;

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed inside the object bounding box.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_front_right_corner_moveable_obj, Calculate_Detection_Association_Cost_Detection_On_Front_Edge)
{
   /** \precond
   * A detection has been set up in test group on front right corner of the object.
   * Move the detection 1m inside bounding box in the lateral direction
   */
   det_prop.vcs_position.y -= 1.0F;
   float32_t expected_det_cost = 0.0F; // Detection is on front edge, cost should be 0.

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended when a detection is placed inside the object bounding box.
*\req NA
*/
TEST(f360_calculate_detection_association_cost_front_right_corner_moveable_obj, Calculate_Detection_Association_Cost_Detection_Inside_Object)
{
   /** \precond
   * A detection has been set up in test group on front right corner of the object.
   * Move the detection 1m inside bounding box in both longitudinal and lateral direction
   */
   det_prop.vcs_position.x -= 1.0F;
   det_prop.vcs_position.y -= 0.5F;
   float32_t expected_det_cost = 0.5F; // Distance to closest edge is 0.5m

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")

}
/** @}*/


/** \defgroup  f360_calculate_detection_association_non_moveable_obj
*  @{
*/
/** \brief
* This test group includes test of the function Calculate_Detection_Association_Cost(). The tests are testing the fucntionality
* for a non-moveable object.
**/
TEST_GROUP(f360_calculate_detection_association_non_moveable_obj)
{
   // Declare common variables used within all tests in this test group.
   F360_Detection_Props_T det_prop = {};
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calibs = {};
   const float32_t test_pass_threshold = 1e-5F;

   /** \setup
    * Place a non-moveable object at (10, 10) in VCS.
    * Set up a default scenario where a detection is placed at position (10-1, 10+0.5) in VCS
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 10.0F;
      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      object_track.bbox.Set_Center(object_track.vcs_position);
      object_track.bbox.Set_Length(calibs.k_nonmoveable_target_diameter);
      object_track.bbox.Set_Width(calibs.k_nonmoveable_target_diameter);
      object_track.bbox.Set_Orientation(0.0F);
      object_track.f_moveable = false;

      det_prop.vcs_position.x = object_track.vcs_position.x - 1.0F;
      det_prop.vcs_position.y = object_track.vcs_position.y + 0.5F;

   }
};

/**
*\purpose Check that Calculate_Detection_Association_Cost() works as intended for a non-moveable object.
*\req NA
*/
TEST(f360_calculate_detection_association_non_moveable_obj, Calculate_Detection_Association_Cost)
{
   /** \precond
   * Test setup from test group can be used
   */

   /** \action
   * Call Calculate_Detection_Association_Cost()
   **/
   float32_t det_cost = Calculate_Detection_Association_Cost(det_prop, object_track);

   /** \result
   * Check that association cost for the detection matches the expected data.
   **/
   const  float32_t expected_det_cost = F360_Get_Hypotenuse(1.0F, 0.5F);
   DOUBLES_EQUAL_TEXT(expected_det_cost, det_cost, test_pass_threshold, "The association cost for the detection did not match the expected data.")
}
/** @}*/


/** \defgroup  f360_Test_Calc_Det_Score_Moveable_Object
*  @{
*/
/** \brief
*  Test group for testing if correct algorithm is used for score calculation and correct value is returned
**/
TEST_GROUP(f360_Test_Calc_Det_Score_Moveable_Object)
{
   F360_Detection_Props_T det_p = {};
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calibs;
   float32_t range_rate_diff = 0.0F;

   /** \setup
   * One object setup - base parameters like size and position
   * One detection setup
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 0.0F;
      object_track.speed = 6.0F;
      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center(object_track.vcs_position);
      object_track.bbox.Set_Center(center);
      object_track.bbox.Set_Length(3.0F);
      object_track.bbox.Set_Width(2.0F);
      object_track.Set_Bbox_Orientation(Angle{ 0.0F });
      object_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      object_track.long_buffer_zone_len2 = 1.2F;
      object_track.lat_buffer_zone_wid2 = 0.8F;
      object_track.f_moveable = true;
   }

   // Helper function to add detections on object corners
   void Add_Detection_On_Front_Right_Corner_Of_Object()
   {
      Convert_TCS_Posn_To_VCS_Posn(object_track.bbox.Get_Length() * 0.5F,
         object_track.bbox.Get_Width() * 0.5F,
         object_track.bbox.Get_Center().x,
         object_track.bbox.Get_Center().y,
         object_track.bbox.Get_Orientation(),
         det_p.vcs_position.x,
         det_p.vcs_position.y);
   }

   void Add_Detection_On_Front_Right_Corner_Of_Object_With_Offset(float32_t offset_x, float32_t offset_y)
   {
      Convert_TCS_Posn_To_VCS_Posn(object_track.bbox.Get_Length() * 0.5F + offset_x,
         object_track.bbox.Get_Width() * 0.5F + offset_y,
         object_track.bbox.Get_Center().x,
         object_track.bbox.Get_Center().y,
         object_track.bbox.Get_Orientation(),
         det_p.vcs_position.x,
         det_p.vcs_position.y);
   }
};

/**
*\purpose  This test will test that correct score is returned if detection is inside solid bbox
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Moveable_Object, f360_Test_Calc_Det_Score_Moveable_Object_In_Solid_BBox)
{
   /** \precond
   * Set detection's position to the front right corner of solid bounding box
   */
   Add_Detection_On_Front_Right_Corner_Of_Object();

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det_p.vcs_position, det_p.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(0.0F, score, F360_EPSILON, "Unexpected score returned")
}

/**
*\purpose  This test will test that correct score is returned if detection is only inside extended bbox
*          and is water spray
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Moveable_Object, f360_Test_Calc_Det_Score_Moveable_Object_Only_In_Extended_BBox_And_Watter_Spray)
{
   /** \precond
   * Set detection's position close to the front right corner of extended bounding box
   * Mark detection as water spray
   */
   Add_Detection_On_Front_Right_Corner_Of_Object_With_Offset(
      object_track.long_buffer_zone_len2*calibs.k_ws_bbox_len_extension_factor -0.0001F,
      object_track.lat_buffer_zone_wid2*calibs.k_ws_bbox_wid_extension_factor - 0.0001F);
   det_p.f_water_spray = true;

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det_p.vcs_position, det_p.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(0.209F, score, 0.001F, "Unexpected score returned")
}

/**
*\purpose  This test will test that correct score is returned if detection is only inside extended bbox
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Moveable_Object, f360_Test_Calc_Det_Score_Moveable_Object_Only_In_Extended_BBox)
{
   /** \precond
   * Set detection's position close to the front right corner of extended bounding box
   */
   Add_Detection_On_Front_Right_Corner_Of_Object_With_Offset(
      object_track.long_buffer_zone_len2 - 0.0001F,
      object_track.lat_buffer_zone_wid2 - 0.0001F);
   det_p.f_water_spray = false;

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det_p.vcs_position, det_p.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(0.7F, score, 0.001F, "Unexpected score returned")
}

/**
*\purpose  This test will test that correct score is returned if detection is out of extended bbox
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Moveable_Object, f360_Test_Calc_Det_Score_Moveable_Object_out_of_extended_bbox)
{
   /** \precond
   * Set detection's position to be out the extended bounding box
   */
   Add_Detection_On_Front_Right_Corner_Of_Object_With_Offset(
      object_track.long_buffer_zone_len2 + 0.0001F,
      object_track.lat_buffer_zone_wid2);

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det_p.vcs_position, det_p.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(calibs.k_score_outside_ext_bbox, score, F360_EPSILON, "Unexpected score returned")
}


/** \defgroup  f360_Test_Calc_Det_Score_Non_Moveable_Object
*  @{
*/
/** \brief
*  Test group for testing if correct algorithm is used for score calculation and correct value is returned
**/
TEST_GROUP(f360_Test_Calc_Det_Score_Non_Moveable_Object)
{
   F360_Detection_Props_T det = {};
   F360_Object_Track_T object_track = {};
   F360_Calibrations_T calibs = {};
   const float32_t range_rate_diff = 0.4F;

   const float32_t test_pass_th = 1e-6F;

   /** \setup
   * Object setup:
   *    - Peference point: Center
   *    - Position: [10, 0]
   *    - BBox center: Same as object position
   *    - BBox length and with: Default non-moveable target diameter (calibs.k_nonmoveable_target_diameter)
   *    - BBox orientation: 0deg (value is not improtnt though but we want to add it anyways to fully define the object bbox)
   *    - Speed: 0.1F
   *    - f_moveable: false
   *    - long_buffer_zone_len1, long_buffer_zone_len2, lat_buffer_zone_wid1, lat_buffer_zone_wid2: Default maximum possible asociation gate extension for non-moveable objects (taken from calibs.k_max_assoc_gate_extension_non_moveable)
   * One detection setup:
   * Position: Inside object bbox = Object position + [calibs.k_nonmoveable_target_diameter * 0.25F, 0]. (I.e straight in front of object center half the object radius distance away from the object center)
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      object_track.reference_point = F360_REFERENCE_POINT_CENTER;
      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 0.0F;
      object_track.bbox.Set_Center(object_track.vcs_position);
      object_track.bbox.Set_Length(calibs.k_nonmoveable_target_diameter);
      object_track.bbox.Set_Width(calibs.k_nonmoveable_target_diameter);
      object_track.Set_Bbox_Orientation(Angle{ 0.0F });
      object_track.speed = 0.1F;
      object_track.f_moveable = false;

      object_track.long_buffer_zone_len2 = calibs.k_max_assoc_gate_extension_non_moveable;
      object_track.lat_buffer_zone_wid2 = calibs.k_max_assoc_gate_extension_non_moveable;

      const Point det_pos_offset(calibs.k_nonmoveable_target_diameter * 0.25F, 0.0F);
      det.vcs_position.x = object_track.bbox.Get_Center().x - det_pos_offset.x;
      det.vcs_position.y = object_track.bbox.Get_Center().y - det_pos_offset.y;
   }
};

/**
*\purpose  This test will test that correct score is returned if detection is inside solid bbox
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Non_Moveable_Object, f360_Test_Calc_Det_Score_Non_Moveable_Object_In_Solid_BBox)
{
   /** \precond
   * Test setup from test group can be used (one detection inside the solid bbox of a non-moveable object)
   */

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det.vcs_position, det.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(0.2525F, score, test_pass_th, "Unexpected score returned")
}

/**
*\purpose  This test will test that correct score is returned if detection is only inside extended bbox
*\req    NA
*/
TEST(f360_Test_Calc_Det_Score_Non_Moveable_Object,f360_Test_Calc_Det_Score_Non_Moveable_Object_Only_In_Extended_BBox)
{
   /** \precond
   * Use setup from test group but change the detection position to be outside of the solid bounding box but inside the extended bounding box.
   * Set detection position to object position - [0.0F, 0.8F * calibs.k_max_assoc_gate_extension_non_moveable];
   */
   const Point det_pos_offset(0.0F, 0.8F * (calibs.k_max_assoc_gate_extension_non_moveable + object_track.bbox.Get_Length()*0.5F));
   object_track.long_buffer_zone_len1 = calibs.k_max_assoc_gate_extension_non_moveable;
   det.vcs_position.x = object_track.bbox.Get_Center().x - det_pos_offset.x;
   det.vcs_position.y = object_track.bbox.Get_Center().y - det_pos_offset.y;

   /** \action
   *Call Calc_Det_Score
   **/
   float32_t score = Calc_Det_Score(object_track, calibs, range_rate_diff, det.vcs_position, det.f_water_spray);

   /** \result
   * Check that correct parameters are returned
   **/
   DOUBLES_EQUAL_TEXT(0.6F, score, test_pass_th, "Unexpected score returned")
}


/** \defgroup  f360_Test_Calculate_SEP_Association_Boxes
*  @{
*/
/** \brief
*  Included tests related to calculating the SEP association boxes.
**/
TEST_GROUP(f360_Test_Calculate_SEP_Association_Boxes)
{

   Static_Env_Poly_T seps[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Calibrations_T calib = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   float32_t test_threshold = 0.0001F;

   /** \setup
   * - Create a straight SEP spanning from -5 to 5m longitudinally at lateral position 5m.
   * - Initialize calibrations
   **/
   TEST_SETUP()
   {
      seps[0U].lower_limit = -5.0F;
      seps[0U].upper_limit = 5.0F;
      seps[0U].p0 = 5.0F;
      seps[0U].p1 = 0.0F;
      seps[0U].p2 = 0.0F;
      seps[0U].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;

      // Init calibrations as defaults
      Initialize_Tracker_Calibrations(calib);
   }
};

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a
*          straight SEP (p2 = 0).
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Straight_SEP)
{
   /** \precond
   * A straight SEP has been set up
   * Set up expected values accordingly
   */
   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 5.0F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 12.0F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;
   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a
*          straight SEP (p2 = 0).
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Straight_Tilted_SEP)
{
   /** \precond
   * A straight SEP has been set up
   * Set up expected values accordingly
   */
   seps[0].p1 = -0.8F;

   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 5.0F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 20.0F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;

   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a
*          curved SEP with p2 < 0.
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Curved_SEP_Neg_p2)
{
   /** \precond
   * Set up a SEP with p2 < 0 and p1 != 0
   * Set up expected values accordingly
   */
   seps[0U].p2 = -0.06F;
   seps[0U].p1 = 0.2F;

   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 3.83335F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 14.6667F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;

   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a
*          curved SEP with p2 > 0.
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Curved_SEP_Pos_p2)
{
   /** \precond
   * Set up a SEP with p2 > 0 and p1 != 0
   * Set up expected values accordingly
   */
   seps[0U].p2 = 0.1F;
   seps[0U].p1 = -0.2F;

   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 6.7F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 15.6F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;

   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a SEP
*          with p2 > 0 but the longitudinal extreme value is above the SEPs upper limit.
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Extreme_Point_Above_Upper_Limit)
{
   /** \precond
   * Set up a SEP with properties such that its extreme value is above SEPs upper longitudinal limit
   * Set up expected values accordingly
   */
   seps[0].p1 = 0.8F;
   seps[0].p2 = -0.03F;

   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 4.25F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 20.0F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;

   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/**
*\purpose  Tests that the correct SEP bounding box's longitudinal and lateral min and max values are returned for a SEP
*          with p2 > 0 but the longitudinal extreme value is below the SEPs lower limit.
*\req    NA
*/
TEST(f360_Test_Calculate_SEP_Association_Boxes, Test_Calculate_SEP_Association_Boxes_Extreme_Point_Below_Lower_Limit)
{
   /** \precond
   * Set up a SEP with properties such that its extreme value is below SEPs lower longitudinal limit
   * Set up expected values accordingly
   */
   seps[0].p1 = 0.7F;
   seps[0].p2 = 0.03F;

   float32_t exp_center_x = 0.0F;
   float32_t exp_center_y = 5.75F;
   float32_t exp_length = 22.0F;
   float32_t exp_width = 19.0F;
   float32_t exp_orientation = 0.0F;
   float32_t exp_orientation_cos = 1.0F;
   float32_t exp_orientation_sin = 0.0F;

   /** \action
   *Call Calculate_SEP_Association_Boxes
   **/
   Calculate_SEP_Association_Boxes(seps, calib, sep_boxes);

   /** \result
   * Check that correct SEP bounding box limits are returned.
   **/
   DOUBLES_EQUAL_TEXT(exp_center_x, sep_boxes[0U].Get_Center().x, test_threshold, "Incorrect longitudinal center value.")
   DOUBLES_EQUAL_TEXT(exp_center_y, sep_boxes[0U].Get_Center().y, test_threshold, "Incorrect lateral center value.")
   DOUBLES_EQUAL_TEXT(exp_length, sep_boxes[0U].Get_Length(), test_threshold, "Incorrect bbox length value.")
   DOUBLES_EQUAL_TEXT(exp_width, sep_boxes[0U].Get_Width(), test_threshold, "Incorrect bbox width value.")
   DOUBLES_EQUAL_TEXT(exp_orientation, sep_boxes[0U].Get_Orientation().Value(), test_threshold, "Incorrect bbox orientation value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_cos, sep_boxes[0U].Get_Orientation().Cos(), test_threshold, "Incorrect bbox orientation cosine value.")
   DOUBLES_EQUAL_TEXT(exp_orientation_sin, sep_boxes[0U].Get_Orientation().Sin(), test_threshold, "Incorrect bbox orientation sine value.")
}

/** @}*/

/** \defgroup  f360_Test_Is_Obj_In_SEP_Association_Box
*  @{
*/
/** \brief
*  Included tests for Is_Obj_In_SEP_Association_Box.
**/
TEST_GROUP(f360_Test_Is_Obj_In_SEP_Association_Box)
{
   Point obj_pos = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   /** \setup
   * Set object position to (2, 8)
   * Create a sep association box with longitudinal position limit (-5, 5) and lateral position limit (5, 11)
   **/
   TEST_SETUP()
   {
      // Object data
      obj_pos.x = 2.0F;
      obj_pos.y = 8.0F;

      // SEP association box data
      sep_boxes[0] = BoundingBox(Point(-5.0F, 5.0F), Point(5.0F, 11.0F));
   }
};

/** @}*/

/** \defgroup  f360_Test_Is_Association_Wrt_SEP_Allowed
*  @{
*/
/** \brief
*  Included tests for Is_Association_Wrt_SEP_Allowed.
**/
TEST_GROUP(f360_Test_Is_Association_Wrt_SEP_Allowed)
{

   F360_Detection_Props_T det_prop = {};
   F360_Object_Track_T obj_track = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   /** \setup
   * Set detection not behind a SEP
   * Create a moving CTCA object behind SEP with ID 1
   * Set object behind sep ambiguous flag to false
   * Set object position to (2, 8)
   * Create a sep association box with longitudinal position limit (-5, 5) and lateral position limit (5, 11)
   **/
   TEST_SETUP()
   {
      // Detection data
      det_prop.behind_sep_id = F360_INVALID_UNSIGNED_ID;

      // Object data
      obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      obj_track.f_moving = true;
      obj_track.behind_sep_id = 1U;
      obj_track.f_behind_sep_ambiguous = false;
      obj_track.vcs_position.x = 2.0F;
      obj_track.vcs_position.y = 8.0F;
      obj_track.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = obj_track.vcs_position;
      obj_track.bbox.Set_Center(center);

      // SEP association box data
      sep_boxes[0] = BoundingBox(Point(-5.0F, 5.0F), Point(5.0F, 11.0F));
   }
};

/**
*\purpose  Tests that a moving CTCA object behind a SEP and within its SEP bounding box is not allowed
*          to associate with a detection on opposite side of the SEP
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Not_Allowed_Obj_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is moving and CTCA and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   */

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_FALSE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should not be allowed.")
}

/**
*\purpose  Tests that a moving CTCA object not behind a SEP and within its SEP bounding box is not allowed
*          to associate with a detection behind the SEP
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Not_Allowed_Det_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is moving and CTCA
   * A detection has been set up behind the SEP with ID 1
   * Object is set to not behind any SEP
   */
   obj_track.vcs_position.y = 6.0F;
   obj_track.behind_sep_id = F360_INVALID_UNSIGNED_ID;
   det_prop.behind_sep_id = 1U;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_FALSE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should not be allowed.")
}

/**
*\purpose  Tests that a CTCA object not behind a SEP and within its SEP bounding box is allowed
*          to associate with a detection behind the SEP when the object is not flagged as moving.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Obj_Not_Moving)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is CTCA and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   * Object's f_moving flag is set to false.
   */
   obj_track.f_moving = false;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CTCA object not behind a SEP and within its SEP bounding box is allowed
*          to associate with a detection behind the SEP when the object is flagged as f_behind_sep_ambiguous.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Obj_Ambiguous_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is CTCA and moving and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   * Object's f_behind_sep_ambiguous flag is set to true.
   */
   obj_track.f_behind_sep_ambiguous = true;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CTCA object not behind a SEP and outside its SEP bounding box is allowed
*          to associate with a detection behind the SEP.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Obj_Outside_SEP_Box)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up outside the SEP association box
   * The object is CTCA and moving and behind SEP with ID 1
   * A detection has been set up, not behind the SEP.
   */
   obj_track.vcs_position.y = 13.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);


   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CTCA object behind a SEP and inside its SEP bounding box is allowed
*          to associate with a detection behind the SEP.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed, Is_Association_Wrt_SEP_Allowed_Obj_And_Det_Behind_SEP)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up outside the SEP association box
   * The object is CTCA and moving and behind SEP with ID 1
   * A detection has been set up behind SEP with ID 1.
   */
   det_prop.behind_sep_id = 1U;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/** @}*/



//----------------------------- CCA ------------------------------------


/** \defgroup  f360_Test_Is_Association_Wrt_SEP_Allowed
*  @{
*/
/** \brief
*  Included tests for Is_Association_Wrt_SEP_Allowed.
**/
TEST_GROUP(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA)
{

   F360_Detection_Props_T det_prop = {};
   F360_Object_Track_T obj_track = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};

   /** \setup
   * Set detection not behind a SEP
   * Create a moving CCA object behind SEP with ID 1
   * Set object behind sep ambiguous flag to false
   * Set object position to (2, 8)
   * Create a sep association box with longitudinal position limit (-5, 5) and lateral position limit (5, 11)
   **/
   TEST_SETUP()
   {
      // Detection data
      det_prop.behind_sep_id = F360_INVALID_UNSIGNED_ID;

      // Object data
      obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
      obj_track.f_moving = true;
      obj_track.behind_sep_id = 1U;
      obj_track.f_behind_sep_ambiguous = false;
      obj_track.vcs_position.x = 2.0F;
      obj_track.vcs_position.y = 8.0F;
      obj_track.reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = obj_track.vcs_position;
      obj_track.bbox.Set_Center(center);

      // SEP association box data
      sep_boxes[0] = BoundingBox(Point(-5.0F, 5.0F), Point(5.0F, 11.0F));
   }
};

/**
*\purpose  Tests that a moving CCA object behind a SEP and within its SEP bounding box is not allowed
*          to associate with a detection on opposite side of the SEP
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Not_Allowed_Obj_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is moving and CCA and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   */

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_FALSE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should not be allowed.")
}

/**
*\purpose  Tests that a moving CCA object not behind a SEP and within its SEP bounding box is not allowed
*          to associate with a detection behind the SEP
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Not_Allowed_Det_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is moving and CCA
   * A detection has been set up behind the SEP with ID 1
   * Object is set to not behind any SEP
   */
   obj_track.vcs_position.y = 6.0F;
   obj_track.behind_sep_id = F360_INVALID_UNSIGNED_ID;
   det_prop.behind_sep_id = 1U;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_FALSE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should not be allowed.")
}

/**
*\purpose  Tests that a CCA object not behind a SEP and within its SEP bounding box is allowed
*          to associate with a detection behind the SEP when the object is not flagged as moving.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Obj_Not_Moving)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is CCA and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   * Object's f_moving flag is set to false.
   */
   obj_track.f_moving = false;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CCA object not behind a SEP and within its SEP bounding box is allowed
*          to associate with a detection behind the SEP when the object is flagged as f_behind_sep_ambiguous.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Obj_Ambiguous_Behind)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up inside the SEP association box
   * The object is CCA and moving and behind SEP with ID 1
   * A detection has been set up, not behind the SEP
   * Object's f_behind_sep_ambiguous flag is set to true.
   */
   obj_track.f_behind_sep_ambiguous = true;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CCA object not behind a SEP and outside its SEP bounding box is allowed
*          to associate with a detection behind the SEP.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Obj_Outside_SEP_Box)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up outside the SEP association box
   * The object is CCA and moving and behind SEP with ID 1
   * A detection has been set up, not behind the SEP.
   */
   obj_track.vcs_position.y = 13.0F;
   Point center = obj_track.vcs_position;
   obj_track.bbox.Set_Center(center);


   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/**
*\purpose  Tests that a moving CCA object behind a SEP and inside its SEP bounding box is allowed
*          to associate with a detection behind the SEP.
*\req    NA
*/
TEST(f360_Test_Is_Association_Wrt_SEP_Allowed_CCA, Is_Association_Wrt_SEP_Allowed_Obj_And_Det_Behind_SEP)
{
   /** \precond
   * a SEP association box has been set up
   * An object has been set up and its position is set up outside the SEP association box
   * The object is CCA and moving and behind SEP with ID 1
   * A detection has been set up behind SEP with ID 1.
   */
   det_prop.behind_sep_id = 1U;

   /** \action
   *Call Is_Association_Wrt_SEP_Allowed
   **/
   const bool f_association_allowed_wrt_SEP = Is_Association_Wrt_SEP_Allowed(det_prop, obj_track, sep_boxes);

   /** \result
   * Check that f_association_allowed_wrt_SEP is set to the correct value.
   **/
   CHECK_TRUE_TEXT(f_association_allowed_wrt_SEP, "Association w.r.t. SEP should be allowed.")
}

/** @}*/

/** \defgroup  f360_Compare_Against_Stationary_Hypothesis
*  @{
*/
/** \brief
*  Testing a dealiasing strategy based on comparing a moving object and
*  an imaginary stationary object in its place.
**/
TEST_GROUP(f360_Compare_Against_Stationary_Hypothesis)
{

   F360_Detection_Props_T det_prop = {};
   rspp_variant_A::RSPP_Detection_T detection = {};
   F360_Object_Track_T obj_track = {};
   F360_Calibrations_T calib = {};
   BoundingBox sep_boxes[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   F360_Radar_Sensor_T sensor = {};

   float32_t range_rate_threshold = 2.0F;

   /** \setup
   * The host is traveling straight ahead with rr = 30 m/s
   * The object in front of it travels with rr = 30.5 m/s or
   * is stationary.
   *
   * Dealiasing interval: 30 m/s
   * Minimun of the dealiasing range: -23 m/s
   *
   * Measured rr = 0.4 m/s
   *
   * Imaginary stationary object:
   * Predicted rr = -30 m/s
   * De-aliased rr = -29.6 m/s
   * RR diff = 0.4 m/s
   **/
   TEST_SETUP()
   {
      // Detection data
      detection.processed.cos_vcs_az = 1;
      detection.processed.sin_vcs_az = 0;
      // measured range rate
      detection.raw.range_rate = 0.4F;

      // sensor data
      sensor.variable.look_id = F360_DET_LOOK_ID_0;
      sensor.constant.v_wrapping[sensor.variable.look_id] = 30.0F;
      sensor.constant.min_aliaised_range_rate[sensor.variable.look_id] = -23.0F;
      // host parameters
      sensor.variable.vcs_velocity.longitudinal = 30.0F;
      sensor.variable.vcs_velocity.lateral = 0.0F;

      Initialize_Tracker_Calibrations(calib);

   }
};

/**
*\purpose   Test that the function returns true when the moving object's range rate fits the de-aliased range rate
*           significantly better than that of the hypothetical stationary object.
*\req    NA
*/
TEST(f360_Compare_Against_Stationary_Hypothesis, Moving_Selected)
{
   /** \precond
    *
    * Moving object:
    * Predicted rr = 0.5 m/s
    * De-aliased rr = 0.4 m/s
    * RR diff = 0.1 m/s
    *
    * Imaginary stationary object:
    * Predicted rr = -30 m/s
    * De-aliased rr = -29.6 m/s
    * RR diff = 0.4 m/s
   */

   const float32_t predicted_range_rate = 0.5F;
   const float32_t dealiased_range_rate = 0.4F;

   /** \action
   *
   *  Call Compare_Against_Stationary_Hypothesis
   *
   **/
   const bool f_moving_hypothesis_significantly_better_than_stat = Compare_Against_Stationary_Hypothesis(
      calib,
      sensor,
      detection,
      range_rate_threshold,
      predicted_range_rate,
      dealiased_range_rate
   );

   /** \result
   * Check that f_moving_hypothesis_significantly_better_than_stat is set to the correct value i.e. "True".
   * In other words the moving object has smaller rr error compared to the stationary object and the detection
   * is associated to the former.
   **/
   CHECK_TRUE(f_moving_hypothesis_significantly_better_than_stat);
}

/**
*\purpose   Test the case when the stationary object has a smaller rr error. The function should returns False
*           since moving hypothesis has higher range rate error and the detection should not be assigned to the
*           moving object.
*\req    NA
*/
TEST(f360_Compare_Against_Stationary_Hypothesis, Stationary_Selected)
{
   /** \precond
    *
    * The object in front of it is stationary
    *
    * Moving object:
    * Predicted rr = 1.5 m/s
    * De-aliased rr = 0.4 m/s
    * RR diff = 1.1 m/s
    *
    * Imaginary stationary object:
    * Predicted rr = -30 m/s
    * De-aliased rr = -29.6 m/s
    * RR diff = 0.4 m/s
   */

   const float32_t predicted_range_rate = 1.5F;
   const float32_t dealiased_range_rate = 0.4F;

   /** \action
   *
   *  Call Compare_Against_Stationary_Hypothesis
   *
   **/
   const bool f_moving_hypothesis_significantly_better_than_stat = Compare_Against_Stationary_Hypothesis(
      calib,
      sensor,
      detection,
      range_rate_threshold,
      predicted_range_rate,
      dealiased_range_rate
   );

   /** \result
   * Check that f_moving_hypothesis_significantly_better_than_stat is set to the correct value i.e. "False".
   * In other words the stationary object has smaller rr error compared to the moving object and the detection
   * is associated to the former.
   **/
   CHECK_FALSE(f_moving_hypothesis_significantly_better_than_stat);
}

/**
*\purpose   Test the case when the difference between the stationary and moving objects is smaller than
*           threshold defined in calibrations. The stationary hypothesis needs to be selected as default.
*\req    NA
*/
TEST(f360_Compare_Against_Stationary_Hypothesis, Small_Diff_Select_Default)
{
   /** \precond
    *
    * The host is traveling straight ahead with rr = 30 m/s
    * The object in front of it is stationary
    *
    * Moving object:
    * Predicted rr = 0.7 m/s
    * De-aliased rr = 0.4 m/s
    * RR diff = 0.3 m/s
    *
    * Imaginary stationary object:
    * Predicted rr = -30 m/s
    * De-aliased rr = -29.6 m/s
    * RR diff = 0.4 m/s
   */

   const float32_t predicted_range_rate = 0.7F;
   const float32_t dealiased_range_rate = 0.4F;

   /** \action
   *
   *  Call Compare_Against_Stationary_Hypothesis
   *
   **/
   const bool f_moving_hypothesis_significantly_better_than_stat = Compare_Against_Stationary_Hypothesis(
      calib,
      sensor,
      detection,
      range_rate_threshold,
      predicted_range_rate,
      dealiased_range_rate
   );

   /** \result
   * Check that f_moving_hypothesis_significantly_better_than_stat is set to the correct value i.e. "False".
   **/
   CHECK_FALSE(f_moving_hypothesis_significantly_better_than_stat);
}

/**
*\purpose   This test checks that the function returns false when the moving objects range rate diff is smaller than the stationary,
            but not significantly smaller, i.e. the difference is below the threhsold
*\req    NA
*/
TEST(f360_Compare_Against_Stationary_Hypothesis, Fail_Moving_Better_And_Diff)
{
   /** \precond
    *
    * The host is traveling straight ahead with rr = 30.6 m/s
    * The object in front of it is stationary
    *
    * Moving object:
    * Predicted rr = 1.5 m/s
    * De-aliased rr = 0.4 m/s
    * RR diff = 1.1 m/s
    *
    * Imaginary stationary object:
    * Predicted rr = -30.6 m/s
    * De-aliased rr = -29.6 m/s
    * RR diff = 1.0 m/s
   */

   const float32_t predicted_range_rate = 1.5F;
   const float32_t dealiased_range_rate = 0.4F;

   sensor.variable.vcs_velocity.longitudinal = 30.6F;

   /** \action
   *
   *  Call Compare_Against_Stationary_Hypothesis
   *
   **/
   const bool f_moving_hypothesis_significantly_better_than_stat = Compare_Against_Stationary_Hypothesis(
      calib,
      sensor,
      detection,
      range_rate_threshold,
      predicted_range_rate,
      dealiased_range_rate
   );

   /** \result
   * Check that f_moving_hypothesis_significantly_better_than_stat is set to the correct value i.e. "False".
   * Both
   **/
   CHECK_FALSE(f_moving_hypothesis_significantly_better_than_stat);
}

/**
*\purpose   This tests that the function returns true when the range rate difference
*           for the stationary objects is outside of the range rate threshold, i.e. it doesn't fit well enough to be considered
*\req    NA
*/
TEST(f360_Compare_Against_Stationary_Hypothesis, Fail_Inside_Gate)
{
   /** \precond
    *
    * Moving object:
    * Predicted rr = 0.5 m/s
    * De-aliased rr = 0.4 m/s
    * RR diff = 0.1 m/s
    *
    * Imaginary stationary object:
    * Predicted rr = -30 m/s
    * De-aliased rr = -29.6 m/s
    * RR diff = 0.4 m/s
   */

   const float32_t predicted_range_rate = 0.5F;
   const float32_t dealiased_range_rate = 0.4F;

   //The range rate threshold is small enough that the stationary object is not inside the range rate gate
   range_rate_threshold = 0.2F;

   /** \action
   *
   *  Call Compare_Against_Stationary_Hypothesis
   *
   **/
   const bool f_moving_hypothesis_significantly_better_than_stat = Compare_Against_Stationary_Hypothesis(
      calib,
      sensor,
      detection,
      range_rate_threshold,
      predicted_range_rate,
      dealiased_range_rate
   );

   /** \result
   * Check that f_moving_hypothesis_significantly_better_than_stat is set to the correct value i.e. "True".
   *
   **/
   CHECK_TRUE(f_moving_hypothesis_significantly_better_than_stat);
}

/** @}*/

/** \defgroup  DetectionPositionScoreTests
 *  @{
 */

 /** \brief
  * Test group for testing association score for various positions of detections inside the bbox and outside the bbox
  */

TEST_GROUP(DetectionPositionScoreTests)
{
    F360_Object_Track_T object_track;
    F360_Calibrations_T calib;
    float32_t length;
    float32_t width;
    float long_buffer_zone_len1;
    float long_buffer_zone_len2;
    float lat_buffer_zone_wid1;
    float lat_buffer_zone_wid2;
   // Set up calibration parameters


    TEST_SETUP() {
        // Setup a default object_track and calibration settings
        Point center = Point(0, 0);
        length = 4.0F;
        width = 2.0F;
        long_buffer_zone_len1 = 2;
        long_buffer_zone_len2 = 3;
        lat_buffer_zone_wid1 = 1;
        lat_buffer_zone_wid2 = 2;

        Angle orientation = Angle(0.785398);
        object_track.long_buffer_zone_len1 = long_buffer_zone_len1;
        object_track.long_buffer_zone_len2 = long_buffer_zone_len2;
        object_track.lat_buffer_zone_wid1 = lat_buffer_zone_wid1;
        object_track.lat_buffer_zone_wid2 = lat_buffer_zone_wid2;

        object_track.bbox = BoundingBox(center, length, width, orientation);

        calib.k_base_score_bbox_center = 0.75F;


    }

    void teardown() {
        // Teardown actions if necessary
    }
};

/** \purpose
 * Test the score calculation when detection is inside the bbox, positioned 
 * below the center of the bbox and its score is based on its
 * distance along TCS X to the bottom edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Below_BBox_Center) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection below the center of the bbox in VCS.
     */
     Point detectionPosition_VCS(-1.131, -1.414);
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(detectionPosition_VCS, object_track, calib);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X to the bottom edge of the bbox
     */
    Point detectionPosition_TCS(-1.8, -0.2);
    float32_t half_length = 0.5*length;
    float32_t dist_to_center = std::abs(detectionPosition_TCS.x);
    float32_t expectedScore = calib.k_base_score_bbox_center*(1 - (dist_to_center/(half_length)));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is inside the bbox, positioned 
 * above the center of the bbox and its score is based on its
 * distance along TCS X to the top edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Above_BBox_Center) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection above the center of the bbox in VCS.
     */
    Point detectionPosition_VCS(1.414, 1.131);

    /** \action
     * Call Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(detectionPosition_VCS, object_track, calib);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X to the top edge of the bbox
     */
    Point detectionPosition_TCS(1.8, -0.2);
    float32_t half_length = 0.5*length;
    float32_t dist_to_center = std::abs(detectionPosition_TCS.x);
    float32_t expectedScore = calib.k_base_score_bbox_center*(1 - (dist_to_center/(half_length)));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is inside the bbox, positioned 
 * to the leftt of the center of the bbox and its score is based on its
 * distance along TCS Y to the left edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Left_of_BBox_Center) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection to the left of the center of the bbox in VCS.
     */
    Point detectionPosition_VCS(-0.0707, -1.202);


    /** \action
     * Call Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(detectionPosition_VCS, object_track, calib);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y to the closest edge parallel to TCS X
     */
    Point detectionPosition_TCS(-0.9, -0.8);
    float32_t dist_to_center = std::abs(detectionPosition_TCS.y); // this is 0.8
    float32_t expectedScore = calib.k_base_score_bbox_center*(1 - (dist_to_center/(0.5*width)));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is inside the bbox, positioned 
 * to the right of the center of the bbox and its score is based on its
 * distance along TCS Y to the right edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Right_of_BBox_Center) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection to the right of the center of the bbox in VCS.
     */
    Point detectionPosition_VCS(-1.202, -0.0707);
    Point detectionPosition_TCS(-0.9, 0.8);
    float32_t dist_to_center = std::abs(detectionPosition_TCS.y); // this is 0.8

    /** \action
     * Call Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Inside_Solid_Bbox(detectionPosition_VCS, object_track, calib);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y to the right edge of the bbox
     */
    float32_t expectedScore = calib.k_base_score_bbox_center*(1 - (dist_to_center/(0.5*width)));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned below the bottom edge of the 
 * bbox and its score is based on its distance along TCS X to the
 * bottom edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Below_BBox_Edge) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection below the bottom edge of the bbox.
     */
    Point detectionPosition_VCS(-1.9798, -2.26274);
    Point detectionPosition_TCS(-3, -0.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this is 3-2=1
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the bottom edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}


/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned above the top edge of the 
 * bbox and its score is based on its distance along TCS X to the
 * top edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Above_BBox_Edge) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection above the top edge of the bbox.
     */
    Point detectionPosition_VCS(2.2627, 1.9798);
    Point detectionPosition_TCS(3, -0.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this is 3-2=1

    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the top edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len2));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned to the left of the left edge of the 
 * bbox and its score is based on its distance along TCS Y to the
 * left edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Left_Of_BBox_Edge) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection left of the left edge of the bbox.
     */
    Point detectionPosition_VCS(1.697, 0);
    Point detectionPosition_TCS(1.2, -1.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this is 1.2-1=0.2
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the left edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned to the right of the right edge of the 
 * bbox and its score is based on its distance along TCS Y to the
 * right edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Right_Of_BBox_Edge) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection right of the right edge of the bbox.
     */
    Point detectionPosition_VCS(0, 1.697);
    Point detectionPosition_TCS(1.2, 1.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this is 1.2-1=0.2
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

    /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the right edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid2));
    DOUBLES_EQUAL(expectedScore, score, 0.0001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the bottom left vertex
 * of the bbox and its score is based on its distance along TCS Y to the
 * left edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Bottom_Left_of_BBox_Vertex_normalized_along_width) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the bottom left vertex of bbox.
     */
    Point detectionPosition_VCS(-0.494, -2.616);
    Point detectionPosition_TCS(-2.2, -1.5);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the left edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the bottom left vertex
 * of the bbox and its score is based on its distance along TCS X to the
 * bottom edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Bottom_Left_of_BBox_Vertex_normalized_along_length) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the bottom left vertex of bbox.
     */
    Point detectionPosition_VCS(-1.0606, -2.7577);
    Point detectionPosition_TCS(-2.7, -1.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);
     
     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the bottom edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the top left vertex
 * of the bbox and its score is based on its distance along TCS Y to the
 * left edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Top_Left_of_BBox_Vertex_normalized_along_width) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the top left vertex of bbox.
     */
    Point detectionPosition_VCS(2.616, 0.494);
    Point detectionPosition_TCS(2.2, -1.5);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the left edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the top left vertex
 * of the bbox and its score is based on its distance along TCS X to the
 * top edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Top_Left_of_BBox_Vertex_normalized_along_length) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the top left vertex of bbox.
     */
    Point detectionPosition_VCS(2.7577, 1.0606);
    Point detectionPosition_TCS(2.7, -1.2);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the top edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len2));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the bottom right vertex
 * of the bbox and its score is based on its distance along TCS Y to the
 * right edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Bottom_Right_of_BBox_Vertex_normalized_along_width) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the bottom right vertex of bbox.
     */
    Point detectionPosition_VCS(-3.32340, 0.212);
    Point detectionPosition_TCS(-2.2, 2.5);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the right edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid2));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the bottom right vertex
 * of the bbox and its score is based on its distance along TCS X to the
 * bottom edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Bottom_Right_of_BBox_Vertex_normalized_along_length) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the bottom right vertex of bbox.
     */
    Point detectionPosition_VCS(-3.04, -0.777);
    Point detectionPosition_TCS(-2.7, 1.6);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);

     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the bottom edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len1));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the top right vertex
 * of the bbox and its score is based on its distance along TCS Y to the
 * right edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Top_Right_of_BBox_Vertex_normalized_along_width) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the top right vertex of bbox.
     */
    Point detectionPosition_VCS(-0.2121, 3.323);
    Point detectionPosition_TCS(2.2, 2.5);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.y)-0.5*width; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);
     
     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS Y from the right edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(lat_buffer_zone_wid2));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** \purpose
 * Test the score calculation when detection is outside the bbox and
 * inside the extended bbox, positioned diagonal to the top right vertex
 * of the bbox and its score is based on its distance along TCS X to the
 * bottom edge of the bbox
 * \req
 * NA.
 */
TEST(DetectionPositionScoreTests, DetectionAlong_Top_Right_of_BBox_Vertex_normalized_along_length) {
    /** \precond
     * Bounding box oriented 45 degrees to the VCS X axis and calibrations set in setup.
     * Position detection diaginal to the top right vertex of bbox.
     */
    Point detectionPosition_VCS(0.989,2.828);
    Point detectionPosition_TCS(2.7, 1.3);
    float32_t closest_dist_to_edge = abs(detectionPosition_TCS.x)-0.5*length; // this
    /** \action
     * Call Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox function and save result.
     */
    float32_t score = Get_Score_Based_On_Detection_Position_Between_Solid_Bbox_And_Ext_Bbox(detectionPosition_VCS, object_track);
     
     /** \result
     * Expect a score based on the distance of the detection 
     * measured along TCS X from the bottom edge of the bbox
     */
    float32_t expectedScore = (closest_dist_to_edge/(long_buffer_zone_len2));
    DOUBLES_EQUAL(expectedScore, score, 0.001);
}

/** @}*/
