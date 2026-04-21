/** \file
   This file tests if we mark out super resolution detection pairs
    whose range and range rates are exactly equal and come from the same sensor
*/

#include "f360_mark_out_det_pairs.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>


/** \defgroup  f360_mark_out_det_pairs
 *  @{
 */
using namespace f360_variant_A;
/** \brief
*  Mark high resoltion detection pairs when their range and range rate are equal from a given sensor
**/
TEST_GROUP(f360_mark_out_det_pairs)
{
   /** \setup
   * Nothing to setup in this test group
   **/
   TEST_SETUP()
   {
   }

   /** \teardown
   * Nothing to teardown in this test group
   **/
   TEST_TEARDOWN()
   {
      //mock.clear();
   }

};

/**
*\purpose
* The purpose of the test is to verify if the detection pairs
* are marked as pairs for equal range and range rates from the same sensor
*/

TEST(f360_mark_out_det_pairs, mark_dets_equal_range_and_range_rate)
{
   /** \precond
   Initialize detections with equal range and range rate from the same sensor and set their status as moving
   **/

   rspp_variant_A::RSPP_Detection_T dets[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   int32_t num_valid_dets = 3;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   dets[0].raw.range = 10.01;
   dets[1].raw.range = 10.01;
   dets[0].raw.range_rate = 2.01;
   dets[1].raw.range_rate = 2.01;
   dets[0].raw.sensor_id = 2;
   dets[1].raw.sensor_id = 2;
   dets[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   dets[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;



   /** \action
   *Call Mark_Out_Det_Pairs() to mark the detections as dectection_pair
   **/
   Mark_Out_Det_Pairs(dets, num_valid_dets, det_props, timing_info);

   /** \result
   *check f_det_pair is set to true
   **/
   bool expected_flag = true;
   CHECK_EQUAL(det_props[0].f_det_pair, expected_flag);
   CHECK_EQUAL(det_props[1].f_det_pair, expected_flag);


}

/**
*\purpose
* The purpose of the test is to verify if the detection pairs
* are not marked as pairs for equal range and unequal range rates from the same sensor
*/

TEST(f360_mark_out_det_pairs, mark_dets_equal_range_and_unequal_range_rate)
{
   /** \precond
   Initialize detections with equal range and  unequal range rate from the same sensor and set their status as moving
   **/

   rspp_variant_A::RSPP_Detection_T dets[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   int32_t num_valid_dets = 3;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   dets[0].raw.range = 10.01;
   dets[1].raw.range = 10.01;
   dets[0].raw.range_rate = 2.01;
   dets[1].raw.range_rate = 1.89;
   dets[0].raw.sensor_id = 2;
   dets[1].raw.sensor_id = 2;
   dets[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   dets[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;



   /** \action
   *Call Mark_Out_Det_Pairs() to mark the detections as dectection_pair
   **/
   Mark_Out_Det_Pairs(dets, num_valid_dets, det_props, timing_info);

   /** \result
   *check f_det_pair is set to false
   **/
   bool expected_flag = false;
   CHECK_EQUAL(det_props[0].f_det_pair, expected_flag);
   CHECK_EQUAL(det_props[1].f_det_pair, expected_flag);


}

/**
*\purpose
* The purpose of the test is to verify if the detection pairs
* are marked as pairs for equal range and range rates from the same sensor.
* However only one detection is moving and the other is not
*/

TEST(f360_mark_out_det_pairs, mark_dets_with_only_one_moving_detection)
{
   /** \precond
   Initialize detections with equal range and range rate from the same sensor and set their status as moving
   **/

   rspp_variant_A::RSPP_Detection_T dets[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   int32_t num_valid_dets = 3;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   dets[0].raw.range = 10.01;
   dets[1].raw.range = 10.01;
   dets[0].raw.range_rate = 2.01;
   dets[1].raw.range_rate = 2.01;
   dets[0].raw.sensor_id = 2;
   dets[1].raw.sensor_id = 2;
   dets[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   dets[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS ;



   /** \action
   *Call Mark_Out_Det_Pairs() to mark the detections as dectection_pair
   **/
   Mark_Out_Det_Pairs(dets, num_valid_dets, det_props, timing_info);

   /** \result
   *check f_det_pair is set to true for moving detection only
   **/
   bool expected_flag_for_moving = true;
   bool expected_flag_for_ambigous = false;
   CHECK_EQUAL(det_props[0].f_det_pair, expected_flag_for_moving);
   CHECK_EQUAL(det_props[1].f_det_pair, expected_flag_for_ambigous);


}

/**
*\purpose
* The purpose of the test is to verify if the detection pairs
* are not marked as pairs for equal range and range rates
* for detections from different sensors
*/

TEST(f360_mark_out_det_pairs, mark_dets_from_different_sensors)
{
   /** \precond
   Initialize detections with equal range and range rate from the same sensor and set their status as moving
   **/

   rspp_variant_A::RSPP_Detection_T dets[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   int32_t num_valid_dets = 3;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   dets[0].raw.range = 10.01;
   dets[1].raw.range = 10.01;
   dets[0].raw.range_rate = 2.01;
   dets[1].raw.range_rate = 2.01;
   dets[0].raw.sensor_id = 2;
   dets[1].raw.sensor_id = 1;
   dets[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   dets[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;



   /** \action
   *Call Mark_Out_Det_Pairs() to mark the detections as dectection_pair
   **/
   Mark_Out_Det_Pairs(dets, num_valid_dets, det_props, timing_info);

   /** \result
   *check f_det_pair is set to false
   **/
   bool expected_flag = false;
   CHECK_EQUAL(det_props[0].f_det_pair, expected_flag);
   CHECK_EQUAL(det_props[1].f_det_pair, expected_flag);


}

/** @}*/
