/** \file
   File contains unit tests for nees cfmi try to init functions
*/

#include "f360_nees_cfmi_try_to_init.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

/** \defgroup  f360_nees_cfmi_try_to_init
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Tests mainly for lines coverage of main functions
**/
TEST_GROUP(f360_nees_cfmi_try_to_init)
{
   /** \setup
   * Setting up basic values without calibrations initialization
   **/
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Velocity_T vel_hypothesis = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   F360_Tracked_Object_Init_Info_T init_info = {};
   F360_Cluster_T cluster = {};
   F360_Detection_Hist_T detection_hist = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Host_T host = {};
   F360_Host_Props_T host_props = {};
   F360_NEES_CFMI_Pos_Diff_Velocity_T stationary_velocity = {};

   TEST_SETUP()
   {
      calibrations.k_nees_cfmi_vh_check_min_overall_plaus = 1.0;
      calibrations.k_nees_cfmi_vh_check_min_cl_plaus = 1.0;
      calibrations.k_nees_cfmi_vh_check_min_pd_plaus = 1.0;
      calibrations.k_nees_cfmi_stat_check_max_mov_dets = 1.0;
      calibrations.k_nees_cfmi_stat_check_max_mov_dets_ratio = 0.5;
      nees_cfmi_information.min_dets_inliers_num = 1;
      nees_cfmi_information.min_vels_inliers_num = 1;

   }
};

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest0)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest1)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest2)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest3)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest4)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest5)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest6)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest7)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest8)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest9)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest10)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest11)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest12)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest13)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest14)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest15)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest16)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest17)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest18)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest19)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest20)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest21)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest22)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest23)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest24)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest25)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest26)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest27)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest28)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckFalseTest29)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 2;
   vel_hypothesis.num_pos_diff = 0;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest30)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 2.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest31)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 2.0;
   vel_hypothesis.plausibility_pos_diff = 0.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, PlausibilityCheckTrueTest32)
{
   /** \precond
   Setting up base conditions values
   **/
   vel_hypothesis.plausibility = 0.0;
   vel_hypothesis.plausibility_cloud = 0.0;
   vel_hypothesis.plausibility_pos_diff = 2.0;
   vel_hypothesis.num_cloud = 0;
   vel_hypothesis.num_pos_diff = 2;

   /** \action
   Run Best_NEES_CFMI_VH_Plausbility_Checks function
   **/
   bool f_success = Best_NEES_CFMI_VH_Plausbility_Checks(calibrations, nees_cfmi_information, vel_hypothesis);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test for InitNEESCFMIInitInfo for line coverage
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, InitNEESCFMIInitInfoOneInvalid)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.vels_num = 2;

   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(nees_cfmi_init_info.inliers.f_dets_valid[0])
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibTrueValidFalsePlausibleFalseTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = false;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = true;
   nees_cfmi_init_info.f_plausible = false;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success)
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibTrueValidTruePlausibleFalseTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = true;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = true;
   nees_cfmi_init_info.f_plausible = false;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success)
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibTrueValidFalsePlausibleTrueTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = false;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = true;
   nees_cfmi_init_info.f_plausible = true;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success)
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibTrueValidTruePlausibleTrueTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = true;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = true;
   nees_cfmi_init_info.f_plausible = true;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success)
}



/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibFalseValidFalsePlausibleFalseTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = false;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = false;
   nees_cfmi_init_info.f_plausible = false;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success)
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibFalseValidTruePlausibleFalseTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = true;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = false;
   nees_cfmi_init_info.f_plausible = false;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success)
}

/**
*\purpose  Test for Update_Init_Info_By_NEES_CFMI_Init_Info for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibFalseValidFalsePlausibleTrueTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = false;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = false;
   nees_cfmi_init_info.f_plausible = true;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(init_info.f_success)
}

/**
*\purpose  Test for line coverage with true nees_cfmi_init_with_not_plausible_vel
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, UpdateInitInfoByNEESCFMIInitInfoCalibFalseValidTruePlausibleTrueTest)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_valid = true;
   calibrations.f_nees_cfmi_init_with_not_plausible_vel = false;
   nees_cfmi_init_info.f_plausible = true;
   nees_cfmi_init_info.det_bbox.Set_Center(Point(5.0,5.0));
   /** \action
   Run Update_Init_Info_By_NEES_CFMI_Init_Info function
   **/
   Update_Track_Init_Info_By_NEES_CFMI_Init_Info(calibrations, nees_cfmi_init_info, cluster, detection_hist, raw_detect_list, host, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(init_info.f_success)
}
/**
*\purpose  Test stationary_hyp value if there are no moving detections
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, StationaryHypothesisFlagValueNoMovingDetections)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.moving_dets_num = 0;
   nees_cfmi_information.moving_dets_ratio = 0.0F;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(nees_cfmi_init_info.f_stationary_hyp);
}


/**
*\purpose  Test stationary_hyp value if there are not enough moving detections
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, StationaryHypothesisFlagValueNotEnoughMovingDetections)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.moving_dets_num = 1;
   nees_cfmi_information.moving_dets_ratio = 0.333F;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(nees_cfmi_init_info.f_stationary_hyp);
}

/**
*\purpose  Test stationary_hyp value if there are enough moving detections
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, StationaryHypothesisFlagValueEnoughMovingDetections)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.moving_dets_num = 3;
   nees_cfmi_information.moving_dets_ratio = 0.75;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(nees_cfmi_init_info.f_stationary_hyp);
}


/**
*\purpose  Test if an inlier detection is considered valid
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, InlierDetectionValid)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.dets_num = 1;
   nees_cfmi_information.detections[0].f_inlier = true;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(nees_cfmi_init_info.inliers.f_dets_valid[0]);
}


/**
*\purpose  Test if an outlier detection is considered invalid
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, OutlierDetectionInvalid)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.dets_num = 1;
   nees_cfmi_information.detections[0].f_inlier = false;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(nees_cfmi_init_info.inliers.f_dets_valid[0]);
}


/**
*\purpose  Test if an inlier velocity detection is considered valid
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, InlierVElocityDetectionValid)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.vels_num = 1;
   nees_cfmi_information.velocities[0].f_inlier = true;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(nees_cfmi_init_info.inliers.f_vels_valid[0]);
}


/**
*\purpose  Test if an outlier velocity detection is considered invalid
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, OutlierVelocityDetectionInvalid)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_information.vels_num = 1;
   nees_cfmi_information.velocities[0].f_inlier = false;
   /** \action
   Run Init_NEES_CFMI_Init_Info function
   **/
   Init_NEES_CFMI_Init_Info(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(nees_cfmi_init_info.inliers.f_vels_valid[0]);
}


/**
*\purpose  Test vsc_velocity value if stationary hypothesis is chosen and valid
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, VCSVelocityForStationaryHypothesis)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_stationary_hyp = true;
   nees_cfmi_init_info.f_stationary_hyp_valid = true;
   /** \action
   Run Estimation_NEES_CFMI function
   **/
   Estimation_NEES_CFMI(calibrations, host_props, host, cluster, det_props, stationary_velocity, nees_cfmi_init_info, nees_cfmi_information, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(nees_cfmi_init_info.f_stationary);
   CHECK_EQUAL(nees_cfmi_init_info.VCS_vel_hyp.vel.lateral, nees_cfmi_information.stationary_velocity.vel.lateral);
   CHECK_EQUAL(nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal, nees_cfmi_information.stationary_velocity.vel.longitudinal);
}

/**
*\purpose  Test if a non-stationary velocity hypothesis is selected correctly for One_Time_Instance_Path
*\req    NA
*/
TEST(f360_nees_cfmi_try_to_init, VCSVelocityNonStationaryHypothesisOneTimeInstancePath)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_stationary_hyp = false;
   nees_cfmi_information.vels_num = 0U;
   /** \action
   Run Estimation_NEES_CFMI function
   **/
   Estimation_NEES_CFMI(calibrations, host_props, host, cluster, det_props, stationary_velocity, nees_cfmi_init_info, nees_cfmi_information, init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_EQUAL(nees_cfmi_init_info.vel_hyp_source, F360_NEES_CFMI_VEL_HYP_SOURCE_CLOUD);
}
/** @}*/
