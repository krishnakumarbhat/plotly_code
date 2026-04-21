/** \file
 * This file contains unit tests for content of f360_sensor_occlusion_info_helpers.cpp file
 */

#include "f360_sensor_occlusion_info_helpers.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  calc_starting_sector_idx
 *  @{
 */

/** \brief
 * Test group of Calc_Starting_Sector_IDx. Tests verify whether starting sector is properly calculated and clamped.
 */
TEST_GROUP(calc_starting_sector_idx)
{
   float32_t azimuth{};
   float32_t min_az{};
   float32_t sectors_width{};
   /** \setup
    * Set up min_az to -0.5F
    * Set up sector width to 0.02F;
    */
   TEST_SETUP()
   {
      min_az = -0.5F;
      sectors_width = 0.02F;
   }
};

/** \purpose  
 * Purpose of this test is to verify whether starting sector idx is properly calculated when no clamp is needed
 * \req
 * NA.
 */
TEST(calc_starting_sector_idx, calc_starting_sector_idx__properly_calculated_when_no_clamp_needed)
{
   /** \precond
    * Set up azimuth to -0.25F
    */
   azimuth = -0.25F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Starting_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 12
    */	
   CHECK_EQUAL(12, sector_idx);
}

/** \purpose
 * Purpose of this test is to verify whether starting sector idx is properly clamped when azimuth is below min_az
 * \req
 * NA.
 */
TEST(calc_starting_sector_idx, calc_starting_sector_idx__properly_calculated_when_azimuth_is_below_min_az)
{
   /** \precond
    * Set up azimuth to -1.25F
    */
   azimuth = -1.25F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Starting_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 0
    */
   CHECK_EQUAL(0, sector_idx);
}

/** \purpose
 * Purpose of this test is to verify whether starting sector idx is properly clamped when azimuth is above maximum azimuth
 * \req
 * NA.
 */
TEST(calc_starting_sector_idx, calc_starting_sector_idx__properly_calculated_when_azimuth_is_above_max_az)
{
   /** \precond
    * Set up azimuth to 1.5F;
    */
   azimuth = 1.5F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Starting_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 74
    */
   CHECK_EQUAL(74, sector_idx);
}
/** @}*/



/** \defgroup  calc_ending_sector_idx
 *  @{
 */

/** \brief
 * Test group of Calc_Starting_Sector_IDx. Tests verify whether starting sector is properly calculated and clamped.
 */
TEST_GROUP(calc_ending_sector_idx)
{
   float32_t azimuth{};
   float32_t min_az{};
   float32_t sectors_width{};
   /** \setup
    * Set up min_az to -0.5F
    * Set up sector width to 0.02F;
    */
   TEST_SETUP()
   {
      min_az = -0.5F;
      sectors_width = 0.02F;
   }
};

/** \purpose
 * Purpose of this test is to verify whether starting sector idx is properly calculated when no clamp is needed
 * \req
 * NA.
 */
TEST(calc_ending_sector_idx, calc_ending_sector_idx__properly_calculated_when_no_clamp_needed)
{
   /** \precond
    * Set up azimuth to -0.25F
    */
   azimuth = -0.25F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Ending_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 13
    */
   CHECK_EQUAL(13, sector_idx);
}

/** \purpose
 * Purpose of this test is to verify whether starting sector idx is properly clamped when azimuth is below min_az
 * \req
 * NA.
 */
TEST(calc_ending_sector_idx, calc_ending_sector_idx__properly_calculated_when_azimuth_is_below_min_az)
{
   /** \precond
    * Set up azimuth to -1.25F
    */
   azimuth = -1.25F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Ending_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 0
    */
   CHECK_EQUAL(0, sector_idx);
}

/** \purpose
 * Purpose of this test is to verify whether starting sector idx is properly clamped when azimuth is above maximum azimuth
 * \req
 * NA.
 */
TEST(calc_ending_sector_idx, calc_ending_sector_idx__properly_calculated_when_azimuth_is_above_max_az)
{
   /** \precond
    * Set up azimuth to 1.5F;
    */
   azimuth = 1.5F;

   /** \action
    * Call tested function
    */
   const int32_t sector_idx = Calc_Ending_Sector_IDx(azimuth, min_az, sectors_width);

   /** \result
    * Check whether sector_idx is equal to 74
    */
   CHECK_EQUAL(74, sector_idx);
}
/** @}*/
