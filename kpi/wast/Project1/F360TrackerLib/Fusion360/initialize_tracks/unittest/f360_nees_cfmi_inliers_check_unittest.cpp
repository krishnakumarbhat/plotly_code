/** \file
   File contains uts for f360_nees_cfmi_inliers_check functions
*/

#include "f360_nees_cfmi_inliers_check.h"
#include <CppUTest/TestHarness.h>

/** \defgroup  f360_nees_cfmi_inliers_check
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Test group for f360_nees_cfmi_inliers_check
**/
TEST_GROUP(f360_nees_cfmi_inliers_check)
{
   /** \setup
   * Setting up input parameters for f360_nees_cfmi_inliers_check
   **/
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Inliers_T inliers = {};
   F360_NEES_CFMI_Velocity_T velocity = {};
   F360_VCS_Velocity_T ref_vel = {};
   float32_t vel_cov[2][2] = {};
   float32_t rr_var = 0.0F;
   float32_t rr_var_extension = 0.0F;
   float32_t sigma_level_pos_diff = 0.0F;
   float32_t sigma_level_cloud = 0.0F;
   float32_t sigma_level_dominant_vel = 0.0F;
   float32_t min_weight = 0.0F;

   float32_t tolerance = 1e-5F;
};

/**
*\purpose  Checking inliers check function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext
   **/
   NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information, inliers, ref_vel, rr_var_extension, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext
   **/
   NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information, inliers, ref_vel, rr_var_extension,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext
   **/
   NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information, inliers, ref_vel, rr_var_extension,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking inliers check function for velocities number lower than max value and dets number lower than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext_Vels_Below_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = 1U;
   min_weight = -1.0;

   /** \action
   *Call NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext
   **/
   NEES_CFMI_CV_NEES_Inliers_Check_With_RR_Var_Ext(nees_cfmi_information, inliers, ref_vel, rr_var_extension,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *One vel inlier and one det inlier shall be found
   **/
   CHECK_TRUE(inliers.f_vels_valid[0]);
   CHECK_TRUE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking simple inliers check function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Simple_NEES_Inliers_Check_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Simple_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Simple_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking simple inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Simple_NEES_Inliers_Check_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Simple_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Simple_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking simple inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Simple_NEES_Inliers_Check_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call NEES_CFMI_CV_Simple_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Simple_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking simple inliers check function for velocities number lower than max value and dets number lower than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Simple_NEES_Inliers_Check_Vels_Below_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = 1U;
   min_weight = -1.0;

   /** \action
   *Call NEES_CFMI_CV_Simple_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Simple_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *One vel inlier and one det inlier shall be found
   **/
   CHECK_TRUE(inliers.f_vels_valid[0]);
   CHECK_TRUE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking constant cov inliers check function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, rr_var,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking constant cov inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, rr_var,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking constant cov inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, rr_var,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking constant cov inliers check function for velocities number lower than max value and dets number lower than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check_Vels_Below_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = 1U;
   min_weight = -1.0;

   /** \action
   *Call NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Constant_Cov_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, rr_var,
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *One vel inlier and one det inlier shall be found
   **/
   CHECK_TRUE(inliers.f_vels_valid[0]);
   CHECK_TRUE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking full inliers check function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Full_NEES_Inliers_Check_Check_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Full_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking full inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Full_NEES_Inliers_Check_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_CV_Full_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking full inliers check function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Full_NEES_Inliers_Check_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call NEES_CFMI_CV_Full_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking full inliers check function for velocities number lower than max value and dets number lower than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_CV_Full_NEES_Inliers_Check_Vels_Below_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = 1U;
   min_weight = -1.0;

   /** \action
   *Call NEES_CFMI_CV_Full_NEES_Inliers_Check
   **/
   NEES_CFMI_CV_Full_NEES_Inliers_Check(nees_cfmi_information, inliers, ref_vel, vel_cov, 
      sigma_level_pos_diff, sigma_level_cloud, sigma_level_dominant_vel, min_weight);

   /** \result
   *One vel inlier and one det inlier shall be found
   **/
   CHECK_TRUE(inliers.f_vels_valid[0]);
   CHECK_TRUE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking accumulate inliers function for velocities number higher than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf_Vels_Above_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf
   **/
   NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking accumulate inliers function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf_Vels_Below_Max_Dets_Above_Max)
{
   /** \precond
   * Setting up vels number less than max and dets number more than max
   **/
   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
   *Call NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf
   **/
   NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking accumulate inliers function for velocities number lower than max value and dets number higher than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf_Vels_Above_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max
   **/
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
   nees_cfmi_information.dets_num = 1U;

   /** \action
   *Call NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf
   **/
   NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, inliers, velocity);

   /** \result
   *Inliers check shall not be executed
   **/
   CHECK_FALSE(inliers.f_vels_valid[0]);
   CHECK_FALSE(inliers.f_dets_valid[0]);
}

/**
*\purpose  Checking accumulate inliers function for velocities number lower than max value and dets number lower than max value
*\req    NA
*/
TEST(f360_nees_cfmi_inliers_check, NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf_Vels_Below_Max_Dets_Below_Max)
{
   /** \precond
   * Setting up vels number more than max and dets number less than max and some values for accumulation
   **/
   nees_cfmi_information.vels_num = 1U;
   inliers.f_vels_valid[0] = true;
   inliers.vel_weights[0] = 1.0F;

   nees_cfmi_information.dets_num = 1U;
   inliers.f_dets_valid[0] = true;
   inliers.det_weights[0] = 1.0F;

   /** \action
   *Call NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf
   **/
   NEES_CFMI_Accumulate_Inliers_Num_Weights_And_Inf(nees_cfmi_information, inliers, velocity);

   /** \result
   *Checking accumulated values
   **/
   float32_t expected_weight_sum = 2.0F;
   DOUBLES_EQUAL(expected_weight_sum, velocity.weight_sum, tolerance);

   float32_t expected_weight_vels_sum = 1.0F;
   DOUBLES_EQUAL(expected_weight_vels_sum, velocity.weight_vels_sum, tolerance);

   float32_t expected_weight_dets_sum = 1.0F;
   DOUBLES_EQUAL(expected_weight_dets_sum, velocity.weight_dets_sum, tolerance);
}
/** @}*/
