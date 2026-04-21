/** \file
 * This file contains unit tests for content of f360_nees_cfmi_hypothesis_confirm_pos_diff.cpp file
 */

#include "f360_nees_cfmi_hypothesis_confirm_pos_diff.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_nees_cfmi_hypothesis_confirm_pos_diff
 *  @{
 */

/** \brief
 * Test group for f360_nees_cfmi_hypothesis_confirm_pos_diff functions
 */
TEST_GROUP(f360_nees_cfmi_hypothesis_confirm_pos_diff)
{
   /** \setup
    * Setting up input arguments
    */
   F360_Calibrations_T calibrations;
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/** \purpose
 * Checking pos diff velocity hypothesis for zero velocities number
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Zero_Vels_Num)
{
   /** \precond
    * Setting up zero number of velocities
    */
   nees_cfmi_information.vels_num = 0U;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 * Checking pos diff velocity hypothesis for valid but insufficient number of velocities
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Insufficient_Vels_Num)
{
   /** \precond
    * Setting up one velocities
    */
   nees_cfmi_information.vels_num = 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 * Checking pos diff velocity hypothesis for above than max number of velocities
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Above_Max_Num_Vels)
{
   /** \precond
    * Setting up 1 velocity
    */
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 * Checking pews diff velocity hypothesis with not enough velocities 
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Not_Enough_Vels_Num)
{
   /** \precond
    * Setting up velocities
    */
   nees_cfmi_information.expected_vels_inliers_num = 3U;
   nees_cfmi_information.vels_num = 2U;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 * Checking pos diff velocity hypothesis least low number of velocities
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Least_Low_Vels_Num)
{
   /** \precond
    * Setting up velocities
    */
   nees_cfmi_information.expected_vels_inliers_num = 3U;
   nees_cfmi_information.vels_num = 2U;

   nees_cfmi_information.velocities[0].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;
   nees_cfmi_information.velocities[1].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 * Checking pos diff velocity hypothesis least min possible number of velocities
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Least_Min_Possible_Vels_Num)
{
   /** \precond
    * Setting up velocities
    */
   calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate = 0U;
   nees_cfmi_information.expected_vels_inliers_num = 5U;
   nees_cfmi_information.vels_num = 3U;

   nees_cfmi_information.velocities[0].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;
   nees_cfmi_information.velocities[1].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;
   nees_cfmi_information.velocities[2].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_INVALID;

   /** \action
    * Call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_inlier);
}

/** \purpose
 *  Test that checks whether confirmation of velocity hypothesis is valid when confidence level is high
 *  and number of detections is above threshold.
 * \req  NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_High)
{
   /** \precond
    * Set up 2 vels_num with high confidence.
    */
   nees_cfmi_information.vels_num = 2U;
   nees_cfmi_information.expected_vels_inliers_num = 1U;

   nees_cfmi_information.velocities[0].rr_plausbility = 0.5F;
   nees_cfmi_information.velocities[1].rr_plausbility = 0.1F;

   for (uint32_t i = 0; i < nees_cfmi_information.vels_num; i++)
   {
      nees_cfmi_information.velocities[i].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
      nees_cfmi_information.velocities[i].pos_diff_determinants.n_dets = 1U;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxy = 1.5F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Syy = 4.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sy = 2.0F;
   }

   /** \action
    * call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Check if confirmation of position difference hypothesis pass.
    */
   CHECK_TRUE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid);
}

/** \purpose
 *  Test that checks whether confirmation of velocity hypothesis is valid when confidence level is medium
 *  and number of detections is above threshold.
 * \req  NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Medium)
{
   /** \precond
    * Set up 2 vels_num with medium confidence.
    */
   nees_cfmi_information.vels_num = 3U;
   nees_cfmi_information.expected_vels_inliers_num = 1U;

   nees_cfmi_information.velocities[0].rr_plausbility = 0.5F;
   nees_cfmi_information.velocities[1].rr_plausbility = 0.1F;

   for (uint32_t i = 0; i < nees_cfmi_information.vels_num; i++)
   {
      nees_cfmi_information.velocities[i].pos_diff_determinants.n_dets = 1U;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxy = 1.5F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Syy = 4.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sy = 2.0F;
   }

   nees_cfmi_information.velocities[0].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_MEDIUM;
   nees_cfmi_information.velocities[1].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_MEDIUM;
   nees_cfmi_information.velocities[2].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;

   /** \action
    * call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Check if confirmation of position difference hypothesis pass.
    */
   CHECK_TRUE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid);
}

/** \purpose
 *  Test that checks whether confirmation of velocity hypothesis is valid when confidence level is low
 *  and number of detections is above threshold.
 * \req  NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Low)
{
   /** \precond
    * Set up 2 vels_num with low confidence.
    */
   nees_cfmi_information.vels_num = 2U;
   nees_cfmi_information.expected_vels_inliers_num = 1U;

   nees_cfmi_information.velocities[0].rr_plausbility = 0.5F;
   nees_cfmi_information.velocities[1].rr_plausbility = 0.1F;


   for (uint32_t i = 0; i < nees_cfmi_information.vels_num; i++)
   {
      nees_cfmi_information.velocities[i].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_LOW;
      nees_cfmi_information.velocities[i].pos_diff_determinants.n_dets = 1U;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxy = 1.5F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Syy = 4.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sy = 2.0F;
   }

   /** \action
    * call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Check if confirmation of position difference hypothesis pass.
    */
   CHECK_TRUE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid);
}

/** \purpose
 *  Test that checks whether confirmation of velocity hypothesis is valid when confidence level is high
 *  and more than minimum number of velocities for confirmation are available.
 * \req  NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Many_Dets)
{
   /** \precond
    * Set up 4 vels_num with high confidence.
    */
   nees_cfmi_information.vels_num = 4U;
   nees_cfmi_information.expected_vels_inliers_num = 1U;

   nees_cfmi_information.velocities[0].rr_plausbility = 0.5F;
   nees_cfmi_information.velocities[1].rr_plausbility = 0.1F;
   nees_cfmi_information.velocities[2].rr_plausbility = 0.5F;
   nees_cfmi_information.velocities[3].rr_plausbility = 1.0F;

   for (uint32_t i = 0; i < nees_cfmi_information.vels_num; i++)
   {
      nees_cfmi_information.velocities[i].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
      nees_cfmi_information.velocities[i].pos_diff_determinants.n_dets = 1U;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxx = 1.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sxy = 1.5F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Syy = 4.0F;
      nees_cfmi_information.velocities[i].pos_diff_determinants.Sy = 2.0F;
   }

   /** \action
    * call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Check if confirmation of position difference hypothesis pass.
    */
   CHECK_TRUE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid);
}

/** \purpose
 *  Test that checks whether confirmation of velocity hypothesis is valid when confidence level is high.
 *  However, no determinant for positions difference are calculated.
 * \req  NA.
 */
TEST(f360_nees_cfmi_hypothesis_confirm_pos_diff, Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp_Many_Dets_Without_Other_Parameters)
{
   /** \precond
    * Set up 4 vels_num with high confidence.
    */
   nees_cfmi_information.vels_num = 4U;
   nees_cfmi_information.expected_vels_inliers_num = 1U;

   nees_cfmi_information.velocities[0].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
   nees_cfmi_information.velocities[1].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
   nees_cfmi_information.velocities[2].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;
   nees_cfmi_information.velocities[3].rr_confidence_level = F360_NEES_CFMI_VEL_RR_CONF_HIGH;

   /** \action
    * call Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp().
    */
   Get_NEES_CFMI_Information_Confirm_Pos_Diff_Hyp(calibrations, nees_cfmi_information);

   /** \result
    * Check if confirmation of position difference hypothesis fail.
    */
   CHECK_FALSE(nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid);
}
/** @}*/
