/** \file
 * This file contains unit tests for content of f360_nees_cfmi_stationary_checks.cpp file
 */

#include "f360_nees_cfmi_stationary_checks.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_nees_cfmi_stationary_checks
 *  @{
 */

/** \brief
 * Test group for testing Check_Stationary_NEES_CFMI_Hypothesis function.
 */
TEST_GROUP(f360_nees_cfmi_stationary_checks)
{
   // Declare common variables used within all tests in f360_nees_cfmi_stationary_checks test group.
   F360_Calibrations_T calib;
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   F360_NEES_CFMI_Pos_Diff_Velocity_T stationary_velocity = {};

   TEST_SETUP()
   {
      //Initialize calibrations.
      Initialize_Tracker_Calibrations(calib);
   }
};

/** \purpose
 * Check whether stationary dominant velocity hypothesis is valid when nees condition
 * from Check_Stationary_Dominant_Hypothesis is not met, but all conditions from
 * Check_Stationary_Inliers_Check are met.
 * \req  NA.
 */
TEST(f360_nees_cfmi_stationary_checks, Check_Stationary_NEES_CFMI_Hypothesis_Stat_Inliers_Conditions_Met)
{
   /** \precond
    *  Set up needed variables.
    */
   stationary_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.dt = calib.k_nees_cfmi_stat_dom_hyp_min_dt + 0.01F;
   nees_cfmi_information.dominant_velocity.vel.longitudinal = 10.0F;
   calib.k_nees_cfmi_stat_dom_hyp_max_speed = nees_cfmi_information.dominant_velocity.vel.longitudinal + 1.0F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][0] = 0.002F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][1] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][0] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][1] = 0.002F;
   calib.k_nees_cfmi_stat_inliers_check_min_det_plaus = 0.1F;

   /** \action
    * call Check_Stationary_NEES_CFMI_Hypothesis().
    */
   Check_Stationary_NEES_CFMI_Hypothesis(calib, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);

   /** \result
    * Check if stationary and dominant hypotheses have been checked and velocity hypothesis is valid.
    */
   CHECK_TRUE(nees_cfmi_init_info.VCS_vel_hyp.f_valid);
}

/** \purpose
 * Check whether stationary dominant velocity hypothesis is invalid when nees condition
 * from Check_Stationary_Dominant_Hypothesis is not met.
 * \req  NA.
 */
TEST(f360_nees_cfmi_stationary_checks, Check_Stationary_NEES_CFMI_Hypothesis_Dominant_Too_Big_Ness)
{
   /** \precond
    *  Set up needed variables
    */
   stationary_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.dt = calib.k_nees_cfmi_stat_dom_hyp_min_dt + 0.01F;
   nees_cfmi_information.dominant_velocity.vel.longitudinal = 10.0F;
   calib.k_nees_cfmi_stat_dom_hyp_max_speed = nees_cfmi_information.dominant_velocity.vel.longitudinal + 1.0F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][0] = 0.002F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][1] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][0] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][1] = 0.002F;
   nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud = calib.k_nees_cfmi_stat_inliers_check_min_det_plaus - 0.1F;
   calib.k_nees_cfmi_stat_inliers_check_min_det_plaus = 2.0F;

   /** \action
    * call Check_Stationary_NEES_CFMI_Hypothesis().
    */
   Check_Stationary_NEES_CFMI_Hypothesis(calib, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);

   /** \result
    * Check if stationary ans dominant hypotheses have been checked and velocity hypothesis is invalid.
    */
   CHECK_FALSE(nees_cfmi_init_info.VCS_vel_hyp.f_valid);
}

/** \purpose
 * Check whether stationary dominant velocity hypothesis is invalid when nees condition
 * from Check_Stationary_Dominant_Hypothesis is not met.
 * \req  NA.
 */
TEST(f360_nees_cfmi_stationary_checks, Check_Stationary_NEES_CFMI_Hypothesis_Dominant_Too_Big_Ness_2)
{
   /** \precond
    *  Set up needed variables
    */
   stationary_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.dt = calib.k_nees_cfmi_stat_dom_hyp_min_dt + 0.01F;
   nees_cfmi_information.dominant_velocity.vel.longitudinal = 10.0F;
   calib.k_nees_cfmi_stat_dom_hyp_max_speed = nees_cfmi_information.dominant_velocity.vel.longitudinal + 1.0F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][0] = 0.002F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][1] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][0] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][1] = 0.002F;
   calib.k_nees_cfmi_stat_inliers_check_min_det_plaus = 2.0F;

   /** \action
    * call Check_Stationary_NEES_CFMI_Hypothesis().
    */
   Check_Stationary_NEES_CFMI_Hypothesis(calib, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);

   /** \result
    * Check if stationary ans dominant hypotheses have been checked and velocity hypothesis is invalid.
    */
   CHECK_FALSE(nees_cfmi_init_info.VCS_vel_hyp.f_valid);
}

/** \purpose
 * Check whether stationary dominant velocity hypothesis is valid when all conditions
 * from Check_Stationary_Dominant_Hypothesis are met.
 * \req  NA.
 */
TEST(f360_nees_cfmi_stationary_checks, Check_Stationary_NEES_CFMI_Hypothesis_Dominant_Above_Thr_Ness_Ok)
{
   /** \precond
    * Set up needed variables
    */
   stationary_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.f_valid = true;
   nees_cfmi_information.dominant_velocity.dt = calib.k_nees_cfmi_stat_dom_hyp_min_dt + 0.01F;
   nees_cfmi_information.dominant_velocity.vel.longitudinal = 10.0F;
   stationary_velocity.vel.longitudinal = 9.5F;
   calib.k_nees_cfmi_stat_dom_hyp_max_speed = nees_cfmi_information.dominant_velocity.vel.longitudinal + 1.0F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][0] = 0.002F;
   nees_cfmi_information.dominant_velocity.vel_cov[0][1] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][0] = 0.001F;
   nees_cfmi_information.dominant_velocity.vel_cov[1][1] = 0.001F;
   calib.k_nees_cfmi_stat_dom_hyp_sigma_level = 20.0F;

   /** \action
    * call Check_Stationary_NEES_CFMI_Hypothesis().
    */
   Check_Stationary_NEES_CFMI_Hypothesis(calib, nees_cfmi_information, stationary_velocity, nees_cfmi_init_info);

   /** \result
    * Check if stationary ans dominant hypotheses have been checked and velocity hypothesis is valid.
    */
   CHECK_TRUE(nees_cfmi_init_info.VCS_vel_hyp.f_valid);
}

/** @}*/

/** \defgroup  f360_stationary_nees_test
 *  @{
 */

/** \brief
 * Test group for testing Stationary_NEES_Test function.
 */
TEST_GROUP(f360_stationary_nees_test)
{
   // Declare common variables used within all tests in f360_stationary_nees_test test group.
   F360_Calibrations_T calib;
   F360_Host_Props_T host_props = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};

   TEST_SETUP()
   {
      //Initialize calibrations.
      Initialize_Tracker_Calibrations(calib);
   }
};

/** @}*/
