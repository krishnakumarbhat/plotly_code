/** \file
 * This file contains unit tests for content of f360_nees_cfmi_hypotheses_selector.cpp file
 */

#include "f360_nees_cfmi_hypotheses_selector.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_nees_cfmi_hypotheses_selector
 *  @{
 */

/** \brief
 * Test group for hypothsis selector functions 
 */
TEST_GROUP(f360_nees_cfmi_hypotheses_selector)
{
   /** \setup
    * Initialize calibrations.
    */
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   F360_NEES_CFMI_Velocity_T vel_hyp = {};
   F360_NEES_CFMI_Vel_Hyp_Source_T vel_hyp_index = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/** \purpose  
 * Testing default value for velocity hypothsis selector
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypotheses_selector, NEES_CFMI_Velocity_Hypothesis_Selector_Def_Vel_Hyp)
{
   /** \precond
    * Setting up velocity hypothesis as none value
    */
   vel_hyp_index = F360_NEES_CFMI_VEL_HYP_SOURCE_NONE;

   /** \action
    * Call NEES_CFMI_Velocity_Hypothesis_Selector
    */
   NEES_CFMI_Velocity_Hypothesis_Selector(calibrations, vel_hyp_index, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);

   /** \result
    * Expecting default state 
    */	
   CHECK_FALSE(vel_hyp.f_valid);
   CHECK_FALSE(vel_hyp.f_plausible);
   CHECK_FALSE(vel_hyp.f_cov_valid);
}

/** \purpose
 * Checking dominant velocity hypothesis for invalid flag
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypotheses_selector, Check_CV_Dominant_NEES_CFMI_Hypothesis_Invalid_Dom_Vel)
{
   /** \precond
    * Setting up invalid flag for velocity hypothesis 
    */
   nees_cfmi_information.dominant_velocity.f_valid = false;

   /** \action
    * Call Check_CV_Dominant_NEES_CFMI_Hypothesis
    */
   Check_CV_Dominant_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);

   /** \result
    * Expecting dominant vel hypothesis will not be valid
    */
   CHECK_FALSE(nees_cfmi_init_info.f_dominant_valid);
}

/** \purpose
 * Checking pos diff hypothesis for invalid flag
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypotheses_selector, Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis_Invalid_Pos_Diff)
{
   /** \precond
    * Setting up invalid flag for pos diff hypothesis
    */
   nees_cfmi_information.confirm_pos_diff_hyp_vel.f_valid = false;

   /** \action
    * Call Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis
    */
   Check_CV_Confirmed_Pos_Diff_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);

   /** \result
    * Expecting pos diff hypothesis will not be valid
    */
   CHECK_FALSE(nees_cfmi_init_info.f_confirm_pos_diff_hyp_vel_valid);
}

/** \purpose
 * Checking cloud hypothesis for invalid flag
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypotheses_selector, Check_CV_Cloud_NEES_CFMI_Hypothesis_Invalid_Cloud)
{
   /** \precond
    * Setting up invalid flag for cloud hypothesis
    */
   nees_cfmi_information.cloud_hyp_vel.f_valid = false;

   /** \action
    * Call Check_CV_Cloud_NEES_CFMI_Hypothesis
    */
   Check_CV_Cloud_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);

   /** \result
    * Expecting cloud hypothesis will not be valid
    */
   CHECK_FALSE(nees_cfmi_init_info.f_cloud_vh_valid);
}

/** \purpose
 * Checking radial hypothesis for invalid flag
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypotheses_selector, Check_CV_Radial_NEES_CFMI_Hypothesis_Invalid_Radial)
{
   /** \precond
    * Setting up invalid flag for radial hypothesis
    */
   nees_cfmi_information.radial_velocity.f_valid = false;

   /** \action
    * Call Check_CV_Radial_NEES_CFMI_Hypothesis
    */
   Check_CV_Radial_NEES_CFMI_Hypothesis(calibrations, nees_cfmi_information, nees_cfmi_init_info, vel_hyp);

   /** \result
    * Expecting radial hypothesis will not be valid
    */
   CHECK_FALSE(nees_cfmi_init_info.f_radial_vh_valid);
}
/** @}*/
