/** \file
   File with set of unit tests for NEES_CFMI_Init_Info_Post_Processing function
*/

#include "f360_nees_cfmi_post_processing_internals.h"
#include "f360_nees_cfmi_try_to_init.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

/** \defgroup  f360_nees_cfmi_post_processing_internals
*  @{
*/

using namespace f360_variant_A;

/** \brief
*  Test group for NEES_CFMI_Init_Info_Post_Processing function
**/
TEST_GROUP(f360_nees_cfmi_post_processing_internals)
{
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   float32_t tolerance = 1e-5F;
   F360_NEES_CFMI_Velocity_T nees_cfmi_velocity = {};
   float32_t inf_expected_value = 1.0F / F360_MIN_DENOMINATOR;

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
*\purpose  Checking init scheme for ransac and m estimator conditions
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Update_NEES_CFMI_Init_Scheme_Ransac_M_Estimator)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.f_valid = true;
   nees_cfmi_init_info.f_ransac_valid = true;
   nees_cfmi_init_info.f_m_estimator_valid = true;

   /** \action
   * Call Update_NEES_CFMI_Init_Scheme
   **/
   Update_NEES_CFMI_Init_Scheme(nees_cfmi_init_info);

   /** \result
   * checking init scheme
   **/
   CHECK_EQUAL(F360_TRACK_NEES_CV_RANSAC_M_ESTIMATOR, nees_cfmi_init_info.init_scheme)
}

/**
*\purpose  Checking init scheme for ransac only
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Update_NEES_CFMI_Init_Scheme_Ransac)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.f_valid = true;
   nees_cfmi_init_info.f_ransac_valid = true;

   /** \action
   * Call Update_NEES_CFMI_Init_Scheme
   **/
   Update_NEES_CFMI_Init_Scheme(nees_cfmi_init_info);

   /** \result
   * checking init scheme
   **/
   CHECK_EQUAL(F360_TRACK_NEES_CV_RANSAC, nees_cfmi_init_info.init_scheme)
}

/**
*\purpose  Checking init scheme for ransac invalid and m estimator invalid
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Update_NEES_CFMI_Init_Scheme_Ransac_Invalid_M_Estimator_Invalid)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.f_valid = true;

   /** \action
   * Call Update_NEES_CFMI_Init_Scheme
   **/
   Update_NEES_CFMI_Init_Scheme(nees_cfmi_init_info);

   /** \result
   * checking init scheme
   **/
   CHECK_EQUAL(F360_TRACK_NEES_CV_ANALYTIC, nees_cfmi_init_info.init_scheme)
}

/**
*\purpose  Checking plausibility scale invalid velocity flag
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Get_Plausibility_Scale_Invalid_Vel_Flag)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = false;

   /** \action
   * Call Get_Plausibility_Scale
   **/
   float32_t plausibility_scale = Get_Plausibility_Scale(nees_cfmi_init_info);

   /** \result
   *checking plausibility scale
   **/
   DOUBLES_EQUAL(1.0F, plausibility_scale, tolerance);
}

/**
*\purpose  Checking plausibility scale for stationary hypothesis and high cloud plausibility
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Get_Plausibility_Scale_Stationary_Hyp_Cloud_True)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.f_stationary_hyp_valid = true;
   nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud = 1.0F;

   /** \action
   * Call Get_Plausibility_Scale
   **/
   float32_t plausibility_scale = Get_Plausibility_Scale(nees_cfmi_init_info);

   /** \result
   *checking plausibility scale
   **/
   float32_t expected_plausibility = 1.0F / nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud;
   DOUBLES_EQUAL(expected_plausibility, plausibility_scale, tolerance);
}

/**
*\purpose  Checking plausibility scale for stationary hypothesis and low cloud plausibility
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Get_Plausibility_Scale_Stationary_Hyp_Cloud_False)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.f_stationary_hyp_valid = true;
   nees_cfmi_init_info.VCS_vel_hyp.plausibility_cloud = 0.0F;

   /** \action
   * Call Get_Plausibility_Scale
   **/
   float32_t plausibility_scale = Get_Plausibility_Scale(nees_cfmi_init_info);

   /** \result
   *checking plausibility scale
   **/
   DOUBLES_EQUAL(inf_expected_value, plausibility_scale, tolerance);
}

/**
*\purpose  Checking plausibility scale for moving hypothesis and low plausibility
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Get_Plausibility_Scale_Moving_Cloud_Low_Plausibility)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.f_stationary_hyp_valid = false;
   nees_cfmi_init_info.VCS_vel_hyp.plausibility = 0.0F;

   /** \action
   * Call Get_Plausibility_Scale
   **/
   float32_t plausibility_scale = Get_Plausibility_Scale(nees_cfmi_init_info);

   /** \result
   *checking plausibility scale
   **/
   DOUBLES_EQUAL(inf_expected_value, plausibility_scale, tolerance);
}

/**
*\purpose  Checking velocity plausibility with velocity plausibility below threshold, lower than expected
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Velocity_Plausibility_Checks_For_Low_Vel_Plausibility_Lower_Than_Expected)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_velocity.f_valid = true;
   nees_cfmi_velocity.f_plausible = true;
   nees_cfmi_velocity.plausibility = 0.0F;

   /** \action
   * Call Velocity_Plausibility_Checks
   **/
   Velocity_Plausibility_Checks(calibrations, nees_cfmi_velocity);

   /** \result
   *checking plausibility scale
   **/
   CHECK_FALSE(nees_cfmi_velocity.f_plausible);
}

/**
*\purpose  Checking velocity plausibility with low velocity plausibility, higher than expected
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Velocity_Plausibility_Checks_For_Low_Vel_Plausibility_Higher_Than_Expected)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_velocity.f_valid = true;
   nees_cfmi_velocity.f_plausible = false;
   nees_cfmi_velocity.plausibility = 0.6F;

   /** \action
   * Call Velocity_Plausibility_Checks
   **/
   Velocity_Plausibility_Checks(calibrations, nees_cfmi_velocity);

   /** \result
   *checking plausibility scale
   **/
   CHECK_TRUE(nees_cfmi_velocity.f_plausible);
}

/**
*\purpose  Checking velocity plausibility with low velocity plausibility, higher than expected
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Velocity_Plausibility_Checks_For_Low_Vel_Plausibility__Non_zer_vel_cov_trace)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_velocity.f_valid = true;
   nees_cfmi_velocity.f_plausible = false;
   nees_cfmi_velocity.vel_cov_trace = 30.0F;
   nees_cfmi_velocity.plausibility = 0.619088894233292F + tolerance;
   /** \action
   * Call Velocity_Plausibility_Checks
   **/
   Velocity_Plausibility_Checks(calibrations, nees_cfmi_velocity);

   /** \result
   *checking plausibility scale
   **/
   CHECK_TRUE(nees_cfmi_velocity.f_plausible);
}

/**
*\purpose  Checking velocity plausibility invalid velocity flag
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Velocity_Plausibility_Checks_For_Invalid_Velocity_Flag)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_velocity.f_valid = false;
   nees_cfmi_velocity.f_plausible = true;

   /** \action
   * Call Velocity_Plausibility_Checks
   **/
   Velocity_Plausibility_Checks(calibrations, nees_cfmi_velocity);

   /** \result
   * checking plausibility scale
   **/
   CHECK_FALSE(nees_cfmi_velocity.f_plausible);
}

/**
*\purpose  Checking initialization sanity check for vel speed above max threshold
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Initialization_Sanity_Checks_For_Speed_Above_Threshold)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid          = true;
   nees_cfmi_init_info.f_fault_detected         = false;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 100.0F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral      = 0.0F;

   /** \action
   * Call Initialization_Sanity_Checks
   **/
   Initialization_Sanity_Checks(calibrations, nees_cfmi_init_info);

   /** \result
   *checking fault detected
   **/
   CHECK_TRUE(nees_cfmi_init_info.f_fault_detected);
}

/**
*\purpose  Checking initialization sanity check for vel cov trace above max threshold
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Initialization_Sanity_Checks_For_vel_cov_trace_Above_Threshold)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid          = true;
   nees_cfmi_init_info.f_fault_detected         = false;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 44.3218384F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral      = 37.3333321F;

   nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff     = 0U;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace    = 5407.78809F;

   /** \action
   * Call Initialization_Sanity_Checks
   **/
   Initialization_Sanity_Checks(calibrations, nees_cfmi_init_info);

   /** \result
   *checking fault detected
   **/
   CHECK_TRUE(nees_cfmi_init_info.f_fault_detected);
}

/**
*\purpose  Checking initialization sanity check for vel speed below max threshold
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Initialization_Sanity_Checks_For_Speed_Below_Threshold)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid          = true;
   nees_cfmi_init_info.f_fault_detected         = false;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 20.0F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral      = 20.0F;

   /** \action
   * Call Initialization_Sanity_Checks
   **/
   Initialization_Sanity_Checks(calibrations, nees_cfmi_init_info);

   /** \result
   *checking fault not detected
   **/
   CHECK_FALSE(nees_cfmi_init_info.f_fault_detected);
}

/**
*\purpose  Checking initialization sanity check for vel cov trace below max threshold
*\req    Na
*/
TEST(f360_nees_cfmi_post_processing_internals, Test_Initialization_Sanity_Checks_For_vel_cov_trace_Below_Threshold)
{
   /** \precond
    * Setting up flags
   **/
   nees_cfmi_init_info.VCS_vel_hyp.f_valid          = true;
   nees_cfmi_init_info.f_fault_detected         = false;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 44.3218384F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral      = 37.3333321F;

   nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff     = 0U;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace    = 500.0F;

   /** \action
   * Call Initialization_Sanity_Checks
   **/
   Initialization_Sanity_Checks(calibrations, nees_cfmi_init_info);

   /** \result
   *checking fault not detected
   **/
   CHECK_FALSE(nees_cfmi_init_info.f_fault_detected);
}

/** @}*/
