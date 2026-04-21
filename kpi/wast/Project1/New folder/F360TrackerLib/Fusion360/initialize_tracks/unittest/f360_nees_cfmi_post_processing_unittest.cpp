/** \file
   File with set of unit tests for NEES_CFMI_Init_Info_Post_Processing function
*/

#include "f360_nees_cfmi_post_processing.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

/** \defgroup  f360_nees_cfmi_post_processing
*  @{
*/

using namespace f360_variant_A;

/** \brief
*  Test group for NEES_CFMI_Init_Info_Post_Processing function
**/
TEST_GROUP(f360_nees_cfmi_post_processing)
{
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_Host_Props_T host_props = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   float32_t tolerance = 1e-4F;
   bool cluster_valid_for_liberal_tracking = false;

   /** \setup
   * Setting up calibrations
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);

      nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   }
};

/**
*\purpose  Checking if position is calculating correctly between two detections
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_Check_Position_Calc_From_2_Dets)
{
   /** \precond
    * Adding detections
   **/
   nees_cfmi_init_info.f_stationary_hyp_valid = true;

   uint16_t i = 0;
   nees_cfmi_information.detections[i].idx.f_historical = false;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 0.0;
   nees_cfmi_information.detections[i].pos.x = 1.0;

   i = 1;
   nees_cfmi_information.detections[i].idx.f_historical = false;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 0.0;
   nees_cfmi_information.detections[i].pos.x = 0.0;

   /** \action
   * Call NEES_CFMI_Init_Info_Post_Processing
   **/
   NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   /** \result
   *describe test result
   **/
   DOUBLES_EQUAL(0.0, nees_cfmi_init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(0.5, nees_cfmi_init_info.det_bbox.Get_Center().x, tolerance);
}

/**
*\purpose  Checking if position is calculating correctly between two detections, one from current time index and one hist det
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_Check_Position_Calc_From_1_Dets_And_1_Hist_Det)
{
   /** \precond
    * Adding detections
   **/
   nees_cfmi_init_info.f_stationary_hyp_valid = true;

   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral = 0.0;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 1.0;

   uint16_t i = 0;
   nees_cfmi_information.detections[i].idx.f_historical = false;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 0.0;
   nees_cfmi_information.detections[i].pos.x = 1.0;

   i = 1;
   nees_cfmi_information.detections[i].idx.f_historical = true;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 0.0;
   nees_cfmi_information.detections[i].pos.x = 0.0;
   nees_cfmi_information.detections[i].timestamp_diff = 1.0;

   /** \action
   * Call NEES_CFMI_Init_Info_Post_Processing
   **/
   NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   /** \result
   *describe test result
   **/
   DOUBLES_EQUAL(0.0, nees_cfmi_init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(1, nees_cfmi_init_info.det_bbox.Get_Center().x, tolerance);
}


/**
*\purpose  Checking if position is calculating correctly between two detections, one from current time index and one hist det
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_Check_Position_Calc_From_2_Dets_And_2_Hist_Det)
{
   /** \precond
    * Adding detections
   **/
   nees_cfmi_init_info.f_stationary_hyp_valid = true;

   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral = 1.0;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 1.0;

   uint16_t i = 0;
   nees_cfmi_information.detections[i].idx.f_historical = false;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 0.0;
   nees_cfmi_information.detections[i].pos.x = 0.0;

   i = 1;
   nees_cfmi_information.detections[i].idx.f_historical = false;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 1.0;
   nees_cfmi_information.detections[i].pos.x = 1.0;

   i = 2;
   nees_cfmi_information.detections[i].idx.f_historical = true;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 3.0;
   nees_cfmi_information.detections[i].pos.x = 3.0;
   nees_cfmi_information.detections[i].timestamp_diff = 1.0;

   i = 3;
   nees_cfmi_information.detections[i].idx.f_historical = true;
   nees_cfmi_information.dets_num++;
   nees_cfmi_information.detections[i].pos.y = 4.0;
   nees_cfmi_information.detections[i].pos.x = 4.0;
   nees_cfmi_information.detections[i].timestamp_diff = 1.0;

   /** \action
   * Call NEES_CFMI_Init_Info_Post_Processing
   **/
   NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   /** \result
   *describe test result
   **/
   DOUBLES_EQUAL(2.5, nees_cfmi_init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(2.5, nees_cfmi_init_info.det_bbox.Get_Center().x, tolerance);
}

/**
*\purpose  Checking condition with f_stationary_test_rejected and f_stationary_hyp_valid being true, only for lines coverage purpose
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_Stat_Test_Rejected_And_Stat_Hyp_Valid)
{
   /** \precond
    * Setting up flags telling that stationary NEES test failed (hypothesis about velocity belonging to stationary object was rejected)
    * but stationary hypothesis was confirmed (contradicting statements)
   **/
   nees_cfmi_init_info.f_stationary_test_rejected = true;
   nees_cfmi_init_info.f_stationary_hyp_valid = true;

   /** \action
   * Call NEES_CFMI_Init_Info_Post_Processing
   **/
   NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   /** \result
   * Expect that even though NEES test failed but stationary hypothesis was confirmed the entire F360_NEES_CFMI_Velocity_T is
   * still considered valid.
   **/
   CHECK_TRUE(nees_cfmi_init_info.VCS_vel_hyp.f_valid)
}

/**
*\purpose  Checking condition with valid flag false and fault detector true, only for lines coverage purpose
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_Flag_Valid_False_Fault_True)
{
   /** \precond
    * Setting up flags telling that structure used for accumulation information of single velocity hypothesis is valid
    * but during processing a fault (actually fale fault) was detected, sanity checks failed.
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = false;
   nees_cfmi_init_info.f_fault_detected = true;

   /** \action
   * Call NEES_CFMI_Init_Info_Post_Processing
   **/
   NEES_CFMI_Init_Info_Post_Processing(calibrations, nees_cfmi_information, nees_cfmi_init_info, cluster_valid_for_liberal_tracking);

   /** \result
   * Expect that even falsely set fault flag is enoungh to invalidate F360_NEES_CFMI_Velocity_T structure.
   **/
   CHECK_FALSE(nees_cfmi_init_info.VCS_vel_hyp.f_valid)
}

/**
*\purpose  Check if calibs are set correctly (velocity plausibility)
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing, Test_calibs__vel_plaus)
{
   /** \result
   * Calibrations set as expected
   **/
   DOUBLES_EQUAL(0.999F, calibrations.k_nees_cfmi_vel_plaus_thr, tolerance);
   DOUBLES_EQUAL(7.716049382716F, calibrations.k_nees_cfmi_vel_plaus_cov_trace_low_plaus, tolerance);
   DOUBLES_EQUAL(192.9012345679F, calibrations.k_nees_cfmi_vel_plaus_cov_trace_high_plaus, tolerance);
   DOUBLES_EQUAL(0.57F, calibrations.k_nees_cfmi_vel_plaus_cov_low_plaus, tolerance);
   DOUBLES_EQUAL(0.9F, calibrations.k_nees_cfmi_vel_plaus_cov_high_plaus, tolerance);
}

/** \brief
*  Test group for NEES_CFMI_Estimate_Detection_Bounding_Box function
**/
TEST_GROUP(f360_NEES_CFMI_Estimate_Detection_Bounding_Box)
{
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   float32_t tolerance = 1e-3F;

   float32_t exp_len;
   float32_t exp_wid;
   float32_t exp_heading;
   float32_t exp_center_x;
   float32_t exp_center_y;

   /** \setup
   * Setting two NEES CFMI information with 2 detections (one historical)
   * Set up NEES CFMI init info with
   *  - Longitudinal velocity = 5 m/s
   *  - Lateral velocity = 0 m/S
   *  - i.e. initial heading = 0 rad.
   **/
   TEST_SETUP()
   {
      nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
      nees_cfmi_init_info.VCS_vel_hyp.vel.lateral = 0.0F;
      nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 5.0F;

      nees_cfmi_information.dets_num = 2U;
      nees_cfmi_information.detections[0U].pos.x = 5.0F;
      nees_cfmi_information.detections[0U].pos.y = 6.0F;
      nees_cfmi_information.detections[1U].pos.x = 10.0F;
      nees_cfmi_information.detections[1U].pos.y = 10.0F;
   }
};

/**
*\purpose  Check that detection bounding box properties are set properly with two detections (out of which one is historical).
*\req    Na
*/
TEST(f360_NEES_CFMI_Estimate_Detection_Bounding_Box, Test_Two_Dets_One_Historical_Zero_Orientation)
{
   /** \precond
    * Init info has been set up with detections in TEST_SETUP
    * Set second detection position to (5.5, 10) and make it f_historical = true, with timestamp diff = 1
    * Set expected output
    */
   nees_cfmi_information.detections[1U].pos.x = 5.5F;
   nees_cfmi_information.detections[1U].pos.y = 10.0F;
   nees_cfmi_information.detections[1U].timestamp_diff = 1.0F;
   nees_cfmi_information.detections[1U].idx.f_historical = true;
   exp_len = 5.5F;
   exp_wid = 4.0F;
   exp_heading = 0.0F;
   exp_center_x = 7.75F;
   exp_center_y = 8.0F;

   /** \action
    * Call NEES_CFMI_Estimate_Detection_Bounding_Box
    */
   NEES_CFMI_Estimate_Detection_Bounding_Box(nees_cfmi_information, nees_cfmi_init_info);

   /** \result
    * Checking if results match expectations
    */
   DOUBLES_EQUAL(exp_len, nees_cfmi_init_info.det_bbox.Get_Length(), tolerance);
   DOUBLES_EQUAL(exp_wid, nees_cfmi_init_info.det_bbox.Get_Width(), tolerance);
   DOUBLES_EQUAL(exp_center_x, nees_cfmi_init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(exp_center_y, nees_cfmi_init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(exp_heading, nees_cfmi_init_info.det_bbox.Get_Orientation().Value(), tolerance);
}

/**
*\purpose  Check that detection bounding box is set properly with three detections and init velocity at 45 degrees.
*\req    Na
*/
TEST(f360_NEES_CFMI_Estimate_Detection_Bounding_Box, Test_Three_Dets_45_Deg_Orientation)
{
   /** \precond
    * Init info:
    *    - Set xvel and yvel to (3,3) m/s
    * Set up three detections on a triangle with corners (3,3), (3,8) and (8,8)
    * Set expected data accordingly
    */
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral = 3.0F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 3.0F;

   nees_cfmi_information.dets_num = 3U;
   nees_cfmi_information.detections[0U].pos.x = 3.0F;
   nees_cfmi_information.detections[0U].pos.y = 3.0F;
   nees_cfmi_information.detections[1U].pos.x = 3.0F;
   nees_cfmi_information.detections[1U].pos.y = 8.0F;
   nees_cfmi_information.detections[2U].pos.x = 8.0F;
   nees_cfmi_information.detections[2U].pos.y = 8.0F;

   exp_len = 7.0711F;
   exp_wid = 3.5355F;
   exp_heading = F360_PI / 4.0F;
   exp_center_x = 4.25F;
   exp_center_y = 6.7499F;

   /** \action
    * Call NEES_CFMI_Estimate_Detection_Bounding_Box
    */
   NEES_CFMI_Estimate_Detection_Bounding_Box(nees_cfmi_information, nees_cfmi_init_info);

   /** \result
    * Checking if results match expectations
    */
   DOUBLES_EQUAL(exp_len, nees_cfmi_init_info.det_bbox.Get_Length(), tolerance);
   DOUBLES_EQUAL(exp_wid, nees_cfmi_init_info.det_bbox.Get_Width(), tolerance);
   DOUBLES_EQUAL(exp_center_x, nees_cfmi_init_info.det_bbox.Get_Center().x, tolerance);
   DOUBLES_EQUAL(exp_center_y, nees_cfmi_init_info.det_bbox.Get_Center().y, tolerance);
   DOUBLES_EQUAL(exp_heading, nees_cfmi_init_info.det_bbox.Get_Orientation().Value(), tolerance);
}

/** @}*/
