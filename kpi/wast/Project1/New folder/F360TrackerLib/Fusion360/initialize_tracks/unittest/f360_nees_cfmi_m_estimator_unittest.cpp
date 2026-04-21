/** \file
   File contains tests for nees cfmi m estimator functions
*/

#include "f360_nees_cfmi_m_estimator.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

/** \defgroup  f360_nees_cfmi_m_estimator
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Test group for m estimator functions
**/
TEST_GROUP(f360_nees_cfmi_m_estimator)
{
   /** \setup
   * Setting up basic values
   **/
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   F360_NEES_CFMI_Velocity_T nees_cfmi_velocity = {};

   TEST_SETUP()
   {
      calibrations.k_nees_cfmi_m_est_plaus_min_overall_plaus = 1.0F;
      calibrations.k_nees_cfmi_m_est_plaus_min_cl_plaus = 1.0F;
      calibrations.k_nees_cfmi_m_est_plaus_min_pd_plaus = 1.0F;
      nees_cfmi_information.min_dets_inliers_num = 1U;
      nees_cfmi_information.min_vels_inliers_num = 1U;
   }
};

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest0)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest1)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest2)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest3)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest4)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest5)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest6)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest7)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest8)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest9)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest10)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest11)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest12)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest13)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest14)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest15)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest16)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest17)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest18)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest19)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest20)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest21)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest22)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest23)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest24)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest25)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest26)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest27)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest28)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckFalseTest29)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 2U;
   nees_cfmi_velocity.num_pos_diff = 0U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest30)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 2.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest31)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 2.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 0.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test plausibility checks for lines coverage
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, PlausibilityCheckTrueTest32)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_velocity.plausibility = 0.0F;
   nees_cfmi_velocity.plausibility_cloud = 0.0F;
   nees_cfmi_velocity.plausibility_pos_diff = 2.0F;
   nees_cfmi_velocity.num_cloud = 0U;
   nees_cfmi_velocity.num_pos_diff = 2U;

   /** \action
   Run CV_M_Estimator_Plausibility_Check function
   **/
   bool f_success = CV_M_Estimator_Plausibility_Check(calibrations, nees_cfmi_information, nees_cfmi_velocity);

   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Test if sigma-level modification of the m-estimator works for pos diff number less then explected inleiers number
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, SigmaModificationForLessPosDiffThenExpectedImliers)
{
   /** \precond
   Setting up base conditions values
   **/
   nees_cfmi_init_info.f_stationary_hyp = false;
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff = 1U;
   nees_cfmi_init_info.sigma_level_pos_diff = 0.2F;
   nees_cfmi_init_info.VCS_vel_hyp.plausibility_pos_diff = 0.9F;
   nees_cfmi_information.expected_vels_inliers_num = 1U;   
   calibrations.k_nees_cfmi_m_est_msl_min_pd_plaus = 0.5F;
   calibrations.k_nees_cfmi_m_est_msl_pd_sl_for_min_plaus = 0.4F;
   calibrations.k_nees_cfmi_m_est_msl_pd_sl_neutral = 0.8F;
   calibrations.k_nees_cfmi_m_est_max_iter = 1U;
   
   /** \action
   Run Estimation_NEES_CFMI function
   **/
   Estimate_Vel_By_CV_NEES_M_Estimator(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_EQUAL(nees_cfmi_init_info.sigma_level_pos_diff, 0.2F);
}

/**
*\purpose  Test the m-estimator covergence test for valid but incorrect data, no covergence expected
*\req    NA
*/
TEST(f360_nees_cfmi_m_estimator, MEstimatorConvergenceCheckForValidIncorrectData)
{
   /** \precond
   Setting up base conditions values
   **/
   calibrations.k_nees_cfmi_m_est_max_iter = 2U;
   calibrations.k_nees_cfmi_m_est_conv_check_max_vel_diff = 0.0F;
   calibrations.k_sigma_level_dominant_velocity = 1.0F;
   calibrations.k_nees_cfmi_min_weight_for_inlier = 0.01F;

   nees_cfmi_init_info.f_stationary_hyp = false;
   nees_cfmi_init_info.VCS_vel_hyp.f_valid = true;
   nees_cfmi_init_info.VCS_vel_hyp.num_pos_diff = 1U;
   nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal = 1.0F;
   nees_cfmi_init_info.VCS_vel_hyp.vel.lateral = 1.0F;
   nees_cfmi_init_info.inliers.f_dets_valid[0] = true;
   nees_cfmi_init_info.inliers.f_vels_valid[0] = true;

   nees_cfmi_information.vels_num = 1U;
   nees_cfmi_information.dets_num = 1U;
   nees_cfmi_information.dominant_velocity.f_valid = true;
   nees_cfmi_information.expected_vels_inliers_num = 1U; 

   nees_cfmi_information.velocities[0].f_inlier = true; 
   nees_cfmi_information.velocities[0].vel.lateral = 1.0F;
   nees_cfmi_information.velocities[0].vel.longitudinal = 1.0F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.n_dets = 1;
   nees_cfmi_information.velocities[0].pos_diff_determinants.information = 0.5F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Sx = 0.4F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Sxx = 0.45F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Sxy = 0.45F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Syx = 0.45F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Syy = 0.55F;
   nees_cfmi_information.velocities[0].pos_diff_determinants.Sy = 0.6F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[0][0] = 0.1F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[0][1] = 0.05F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[1][0] = 0.04F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[1][1] = 0.1F;

   nees_cfmi_information.detections[0].f_inlier = true;
   nees_cfmi_information.detections[0].pos.x = 1.0F;
   nees_cfmi_information.detections[0].pos.y = 1.0F;
   nees_cfmi_information.detections[0].pos_cov[0][0] = 0.1F;
   nees_cfmi_information.detections[0].pos_cov[0][1] = 0.05F;
   nees_cfmi_information.detections[0].pos_cov[1][0] = 0.04F;
   nees_cfmi_information.detections[0].pos_cov[1][1] = 0.1F;
   nees_cfmi_information.detections[0].cos_vcs_az = 0.7071F;
   nees_cfmi_information.detections[0].sin_vcs_az = 0.7071F;
   nees_cfmi_information.detections[0].range_rate_comp = 1.414F;
   nees_cfmi_information.detections[0].cloud_determinants.n_dets = 1;
   nees_cfmi_information.detections[0].cloud_determinants.information = 0.5F;
   nees_cfmi_information.detections[0].cloud_determinants.Sx = 0.4F;
   nees_cfmi_information.detections[0].cloud_determinants.Sxx = 0.45F;
   nees_cfmi_information.detections[0].cloud_determinants.Sxy = 0.45F;
   nees_cfmi_information.detections[0].cloud_determinants.Syx = 0.45F;
   nees_cfmi_information.detections[0].cloud_determinants.Syy = 0.55F;
   nees_cfmi_information.detections[0].cloud_determinants.Sy = 0.6F;

   nees_cfmi_init_info.sigma_level_pos_diff = 1.5F;
   nees_cfmi_init_info.sigma_level_cloud = 1.5F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[0][0] = 0.1F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[0][1] = 0.05F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[1][0] = 0.04F;
   nees_cfmi_init_info.VCS_vel_hyp.vel_cov[1][1] = 0.1F;
   
   nees_cfmi_init_info.VCS_vel_hyp.determinants.n_dets = 1;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.information = 0.5F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Sx = 0.4F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Sxx = 0.45F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Sxy = 0.45F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Syx = 0.45F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Syy = 0.55F;
   nees_cfmi_init_info.VCS_vel_hyp.determinants.Sy = 0.6F;
   /** \action
   Run Estimation_NEES_CFMI function
   **/
   Estimate_Vel_By_CV_NEES_M_Estimator(calibrations, nees_cfmi_information, nees_cfmi_init_info);
   /** \result
   Checking if results matching the expectations
   **/
   CHECK_FALSE(nees_cfmi_init_info.f_m_estimator_valid);

}
/** @}*/
