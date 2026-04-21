/** \file
 * This file contains unit tests for content of f360_nees_cfmi_hypothesis_radial.cpp file
 */

#include "f360_nees_cfmi_hypothesis_radial.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_nees_cfmi_hypothesis_radial
 *  @{
 */

/** \brief
 * Test group for f360_nees_cfmi_hypothesis_radial functions
 */
TEST_GROUP(f360_nees_cfmi_hypothesis_radial)
{	
   /** \setup
    * Setting up input arguments
    */
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/** \purpose
 * Checking radial velocity hypothesis for zero detections
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_radial, Get_NEES_CFMI_Information_Radial_VH_Zero_Dets)
{
   /** \precond
    * Setting up zero number of detections
    */
   nees_cfmi_information.dets_num = 0U;

   /** \action
    * Call Get_NEES_CFMI_Information_Radial_VH
    */
   Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.radial_velocity.f_inlier);
}

/** \purpose
 * Checking radial velocity hypothesis for valid but insufficient number of detections
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_radial, Get_NEES_CFMI_Information_Radial_VH_Insufficient_Dets)
{
   /** \precond
    * Setting up 1 detection
    */
   nees_cfmi_information.dets_num = 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Radial_VH
    */
   Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.radial_velocity.f_inlier);
}

/** \purpose
 * Checking radial velocity hypothesis for above than max number of detections
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_radial, Get_NEES_CFMI_Information_Radial_VH_Above_Max_Num_Dets)
{
   /** \precond
    * Setting up 1 detection
    */
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Radial_VH
    */
   Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.radial_velocity.f_inlier);
}

/** \purpose
 * Checking radial velocity hypothesis for sufficient number of detections but with invalid values
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_radial, Get_NEES_CFMI_Information_Radial_VH_3_Valid_Dets_Num)
{
   /** \precond
    * Setting up 3 valid detection
    */
   nees_cfmi_information.dets_num = 3U;
   nees_cfmi_information.detections[0].f_inlier = true;
   nees_cfmi_information.detections[1].f_inlier = true;
   nees_cfmi_information.detections[2].f_inlier = true;
   nees_cfmi_information.detections[0].range_rate_comp_var = 1.0F;
   nees_cfmi_information.detections[1].range_rate_comp_var = 1.0F;
   nees_cfmi_information.detections[2].range_rate_comp_var = 1.0F;
   nees_cfmi_information.detections[0].cloud_determinants.n_dets = 1U;
   nees_cfmi_information.detections[1].cloud_determinants.n_dets = 1U;
   nees_cfmi_information.detections[2].cloud_determinants.n_dets = 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Radial_VH
    */
   Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.radial_velocity.f_inlier);
}

/** \purpose
 * Checking radial velocity hypothesis for sufficient number of detections but with invalid values
 * \req
 * NA.
 */
TEST(f360_nees_cfmi_hypothesis_radial, Get_NEES_CFMI_Information_Radial_VH_3_Valid_Dets_Num_Zero_Range_Rate)
{
   /** \precond
    * Setting up 3 valid detection with zero rr
    */
   calibrations.k_nees_cfmi_min_estimates_num_for_vel_estimate = 0U;
   nees_cfmi_information.dets_num = 3U;
   nees_cfmi_information.detections[0].f_inlier = true;
   nees_cfmi_information.detections[1].f_inlier = true;
   nees_cfmi_information.detections[2].f_inlier = true;
   nees_cfmi_information.detections[0].range_rate_comp_var = 0.0F;
   nees_cfmi_information.detections[1].range_rate_comp_var = 0.0F;
   nees_cfmi_information.detections[2].range_rate_comp_var = 0.0F;
   nees_cfmi_information.detections[0].cloud_determinants.n_dets = 1U;
   nees_cfmi_information.detections[1].cloud_determinants.n_dets = 1U;
   nees_cfmi_information.detections[2].cloud_determinants.n_dets = 1U;

   /** \action
    * Call Get_NEES_CFMI_Information_Radial_VH
    */
   Get_NEES_CFMI_Information_Radial_VH(calibrations, nees_cfmi_information);

   /** \result
    * Velocity hypothesis shall be invalid
    */
   CHECK_FALSE(nees_cfmi_information.radial_velocity.f_inlier);
}
/** @}*/
