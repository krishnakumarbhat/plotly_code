/** \file
 * This file contains unit tests for content of f360_nees_cfmi_ransac.cpp file
 */

#include "f360_nees_cfmi_ransac.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_nees_cfmi_ransac
 *  @{
 */

/** \brief
 * Test group designed for testing f360_nees_cfmi_ransac related functions
 */
TEST_GROUP(f360_nees_cfmi_ransac)
{	
   // Declare needed input structures
   F360_Calibrations_T calibrations;
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info = {};
   F360_NEES_CFMI_Velocity_T best_nees_velocity = {};
   float32_t tolerance = 1e-4F;
   
   /** \setup
    * Initialize calibrations structure
    */
   TEST_SETUP()
   {
      // Fill data structures with needed information
      Initialize_Tracker_Calibrations(calibrations);
   }
   
};

/** \purpose  
 * RANSAC velocity hypothesis should not be determined if number of velocities exceeds maximum value
 * \req
 * NA
 */
TEST(f360_nees_cfmi_ransac, Should_Not_Execute_Ransac_When_Max_Number_Of_Velocities_Exceeded)
{
   /** \precond
    * Set number of velocities to more than allowed for RANSAC velocity determination
    */
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;
	
   /** \action
    * Call Get_CV_NEES_CFMI_RANSAC_Vel_Hyp function
    */
   Get_CV_NEES_CFMI_RANSAC_Vel_Hyp(calibrations, nees_cfmi_information, nees_cfmi_init_info, best_nees_velocity);

   /** \result
    * RANSAC should not be executed and best_velocity should be invalid
    */
   CHECK_FALSE(best_nees_velocity.f_valid);
}

/** \purpose
 * RANSAC velocity hypothesis should not be determined if number of detections exceeds maximum value
 * \req
 * NA
 */
TEST(f360_nees_cfmi_ransac, Should_Not_Execute_Ransac_When_Max_Number_Of_Detections_Exceeded)
{
   /** \precond
    * Set number of detections to more than allowed for RANSAC velocity determination
    */
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
    * Call Get_CV_NEES_CFMI_RANSAC_Vel_Hyp function
    */
   Get_CV_NEES_CFMI_RANSAC_Vel_Hyp(calibrations, nees_cfmi_information, nees_cfmi_init_info, best_nees_velocity);

   /** \result
    * RANSAC should not be executed and best_velocity should be invalid
    */

   CHECK_FALSE(best_nees_velocity.f_valid);
}


/** \purpose
 * RANSAC preconditions should set number of allowed RANSAC iterations to min value
 * when estimated needed value of iterations is smaller than min
 * \req
 * NA
 */
TEST(f360_nees_cfmi_ransac, Should_Set_Number_Of_RANSAC_Iterations_To_Min_When_Estimated_Iterations_Needed_Smaller_Than_Min)
{
   /** \precond
    * Set number of velocities above min_ransac_iterations
    * Set expected velocities to inliers ratio
    */
   nees_cfmi_information.vels_num = calibrations.k_nees_cfmi_ransac_min_iterations + 1U;
   nees_cfmi_information.expected_vels_inliers_ratio = 1.0F;

   /** \action
    * Call CV_RANSAC_Preconditions function
    */
   CV_RANSAC_Preconditions(calibrations, nees_cfmi_information, nees_cfmi_init_info);

   /** \result
    * RANSAC iterations should be set to min
    */
   const uint32_t expected_max_ransac_iterations = calibrations.k_nees_cfmi_ransac_min_iterations;
   CHECK_EQUAL(expected_max_ransac_iterations, nees_cfmi_init_info.max_number_of_ransac_iterations);
}

/** \purpose
 * RANSAC preconditions should set number of allowed RANSAC iterations to estimated number
 * when estimated value of iterations is greater than min but smaller than max
 * \req
 * NA
 */
TEST(f360_nees_cfmi_ransac, Should_Set_Number_Of_RANSAC_Iterations_To_Estimated_Iterations)
{
   /** \precond
    * Set number of velocities above min_ransac_iterations
    * Set expected velocities to inliers ratio
    */
   nees_cfmi_information.vels_num = calibrations.k_nees_cfmi_ransac_min_iterations + 1U;
   nees_cfmi_information.expected_vels_inliers_ratio = 0.5F;

   /** \action
    * Call CV_RANSAC_Preconditions function
    */
   CV_RANSAC_Preconditions(calibrations, nees_cfmi_information, nees_cfmi_init_info);

   /** \result
    * RANSAC iterations should be set to estimated value
    */
   const uint32_t expected_max_ransac_iterations = 10U;
   CHECK_EQUAL(expected_max_ransac_iterations, nees_cfmi_init_info.max_number_of_ransac_iterations);
}

/** \purpose
 * RANSAC preconditions should set number of allowed RANSAC iterations to max value
 * when estimated needed value of iterations is greater than max
 * \req
 * NA
 */
TEST(f360_nees_cfmi_ransac, Should_Set_Number_Of_RANSAC_Iterations_To_Max_When_Estimated_Iterations_Needed_Greater_Than_Max)
{
   /** \precond
    * Set number of velocities above min_ransac_iterations
    * Set expected velocities to inliers ratio
    */
   nees_cfmi_information.vels_num = calibrations.k_nees_cfmi_ransac_min_iterations + 1U;
   nees_cfmi_information.expected_vels_inliers_ratio = 0.05F;

   /** \action
    * Call CV_RANSAC_Preconditions function
    */
   CV_RANSAC_Preconditions(calibrations, nees_cfmi_information, nees_cfmi_init_info);

   /** \result
    * RANSAC iterations should be set to max
    */
   const uint32_t expected_max_ransac_iterations = calibrations.k_nees_cfmi_ransac_max_iterations;
   CHECK_EQUAL(expected_max_ransac_iterations, nees_cfmi_init_info.max_number_of_ransac_iterations);
}
/** @}*/

/** \defgroup Estimate_Velocity_After_Ransac_Runs_Test_Group
 *  @{
 */

 /** \brief
  * Test group designed for testing Estimate_Velocity_After_Ransac_Runs function.
  */
TEST_GROUP(Estimate_Velocity_After_Ransac_Runs_Test_Group)
{
   // Declare needed input structures
   F360_Calibrations_T calibrations;
   F360_NEES_CFMI_Information_T nees_cfmi_information;
   F360_NEES_CFMI_Init_Info_T nees_cfmi_init_info;
   F360_NEES_CFMI_Velocity_T best_nees_velocity;
   const float32_t tolerance = 1e-4F;

   /** \setup
    * Initialize calibrations structure
    */
   TEST_SETUP()
   {
      // Fill data structures with needed information
      Initialize_Tracker_Calibrations(calibrations);
      nees_cfmi_information = {};
      nees_cfmi_init_info = {};
      best_nees_velocity = {};
   }
};

/** \purpose
 * Velocity estimation should not be done because input velocity info after RANSAC is not valid.
 * \req
 * NA
 */
TEST(Estimate_Velocity_After_Ransac_Runs_Test_Group, Input_Velocity_Is_Not_Valid)
{
   /** \precond
    * Set input velocity info to be invalid.
    */
   const float32_t expected_vel_long = 10.0F;
   const float32_t expected_vel_lat = 10.0F;

   best_nees_velocity.f_valid = false;
   best_nees_velocity.vel.longitudinal = expected_vel_long;
   best_nees_velocity.vel.lateral = expected_vel_lat;

   /** \action
    * Call Estimate_Velocity_After_Ransac_Runs function
    */
   Estimate_Velocity_After_Ransac_Runs(calibrations, nees_cfmi_information, nees_cfmi_init_info, best_nees_velocity);

   /** \result
    * Output velocity should not be reseted and be same as input.
    */
   CHECK_EQUAL(expected_vel_long, best_nees_velocity.vel.longitudinal);
   CHECK_EQUAL(expected_vel_lat, best_nees_velocity.vel.lateral);
}
/** @}*/
