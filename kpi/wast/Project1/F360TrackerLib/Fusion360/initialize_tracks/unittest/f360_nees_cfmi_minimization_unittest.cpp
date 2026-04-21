/** \file
   File contains unit tests for f360_nees_cfmi_minimization functions
*/

#include "f360_nees_cfmi_minimization.h"

#include <CppUTest/TestHarness.h>
/** \defgroup  f360_nees_cfmi_minimization
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Test group that contains tests for f360_nees_cfmi_minimization functions
**/
TEST_GROUP(f360_nees_cfmi_minimization)
{
   /** \setup
   * Setting up input parameters for f360_nees_cfmi_inliers_check
   **/
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Inliers_T inliers = {};
   F360_NEES_CFMI_Velocity_T velocity = {};

   float32_t tolerance = 1e-5F;
};

/**
*\purpose  Checking velocity estimation nees function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_minimization, Estimate_Vel_By_CV_NEES_Min_Analytical_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call Estimate_Vel_By_CV_NEES_Min_Analytical
   **/
   Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking velocity estimation nees function for velocities number higher than max value and dets number below than max value
*\req    NA
*/
TEST(f360_nees_cfmi_minimization, Estimate_Vel_By_CV_NEES_Min_Analytical_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call Estimate_Vel_By_CV_NEES_Min_Analytical
   **/
   Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking velocity estimation nees function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_minimization, Estimate_Vel_By_CV_NEES_Min_Analytical_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call Estimate_Vel_By_CV_NEES_Min_Analytical
   **/
   Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking velocity estimation nees function for valid velocities and detections number and invalid V determinant
*\req    NA
*/
TEST(f360_nees_cfmi_minimization, Estimate_Vel_By_CV_NEES_Min_Analytical_Valid_Vels_Valid_Dets_Invalid_V_Det)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/ 
   nees_cfmi_information.detections[0].cloud_determinants.n_dets = 2;
   nees_cfmi_information.vels_num = 1U;
   inliers.f_vels_valid[0] = true;
   inliers.vel_weights[0] = 1.0F;

   nees_cfmi_information.dets_num = 1U;
   inliers.f_dets_valid[0] = true;
   inliers.det_weights[0] = 1.0F;

   /** \action
   *Call Estimate_Vel_By_CV_NEES_Min_Analytical
   **/
   Estimate_Vel_By_CV_NEES_Min_Analytical(nees_cfmi_information, inliers, velocity);

   /** \result
   *Checking velocity valid flag for invalid V determinant
   **/
   CHECK_FALSE(velocity.f_valid);
}

/** @}*/
