/** \file
 * This file contains unit tests for content of f360_sensor_fov.cpp file
 */

#include "f360_sensor_fov.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_sensor_fov
 *  @{
 */

/** \brief
 * Test group of Sensor_FOV_T class. Tests verify whether field of view parameters are properly used to determine
 * whether point is in sensor field of view.
 */
TEST_GROUP(f360_sensor_fov)
{
   Sensor_FOV_T sensor_fov{};
   float32_t azimuth{};
   float32_t range{};

   F360_Radar_Sensor_T sensors{};
   F360_Det_Look_ID_T sensor_look_id{};
   TEST_SETUP()
   {
      sensor_look_id = F360_DET_LOOK_ID_0;
      sensors.constant.fov_min_az_rad[sensor_look_id] = -1.5F;
      sensors.constant.fov_max_az_rad[sensor_look_id] = 1.5F;
      sensors.constant.range_limits[sensor_look_id] = 50.0F;
   }
};

/** \purpose  
 * Purpose of this test is to verify whether Update_Calibrations functions properly fills sensor field of view
 * \req
 * NA.
 */
TEST(f360_sensor_fov, Get_Min_Az__Returns_Correct_Value_After_Init)
{
   /** \precond
    * All is done in TEST_SETUP
    */

   /** \action
    * Call Update_Calibrations
    */
   sensor_fov.Update_Calibrations(sensors);

   /** \result
    * Check whether minimal azimuth is equal to -1.5F
    * Check whether maximal azimuth is equal to 1.5F
    * Check whether maximal range is equal to 50.0F
    */
   DOUBLES_EQUAL(-1.5F, sensor_fov.Get_Min_Az(), F360_EPSILON);
   DOUBLES_EQUAL(3.0F, sensor_fov.Get_Azimuth_Spread(), F360_EPSILON);
   DOUBLES_EQUAL(50.0F, sensor_fov.Get_Max_Range(), F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether Is_Point_In_FOV returns true when point is in sensor FOV
 * \req
 * NA.
 */
TEST(f360_sensor_fov, Is_Point_In_FOV__Returns_True_When_Seen)
{
   /** \precond
    * Update sensor calibrations
    * Set azimuth to -0.1F
    * Set range to 10.0F
    */
   sensor_fov.Update_Calibrations(sensors);
   azimuth = -0.1F;
   range = 10.0F;

   /** \action
    * Call Is_Point_In_FOV
    */
   const bool f_is_in_fov = sensor_fov.Is_Point_In_FOV(azimuth, range);

   /** \result
    * Check whether returned value is true
    */
   CHECK_TRUE(f_is_in_fov);
}

/** \purpose
 * Purpose of this test is to verify whether Is_Point_In_FOV returns false when point has too low azimuth
 * \req
 * NA.
 */
TEST(f360_sensor_fov, Is_Point_In_FOV__Returns_False_When_Too_Low_Azimuth)
{
   /** \precond
    * Update sensor calibrations
    * Set azimuth to -2.1F
    * Set range to 10.0F
    */
   sensor_fov.Update_Calibrations(sensors);
   azimuth = -2.1F;
   range = 10.0F;

   /** \action
    * Call Is_Point_In_FOV
    */
   const bool f_is_in_fov = sensor_fov.Is_Point_In_FOV(azimuth, range);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_is_in_fov);
}

/** \purpose
 * Purpose of this test is to verify whether Is_Point_In_FOV returns false when point has too high azimuth
 * \req
 * NA.
 */
TEST(f360_sensor_fov, Is_Point_In_FOV__Returns_False_When_Too_High_Azimuth)
{
   /** \precond
    * Update sensor calibrations
    * Set azimuth to 2.1F
    * Set range to 10.0F
    */
   sensor_fov.Update_Calibrations(sensors);
   azimuth = 2.1F;
   range = 10.0F;

   /** \action
    * Call Is_Point_In_FOV
    */
   const bool f_is_in_fov = sensor_fov.Is_Point_In_FOV(azimuth, range);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_is_in_fov);
}

/** \purpose
 * Purpose of this test is to verify whether Is_Point_In_FOV returns false when point has too high range
 * \req
 * NA.
 */
TEST(f360_sensor_fov, Is_Point_In_FOV__Returns_False_When_Too_High_Range)
{
   /** \precond
    * Update sensor calibrations
    * Set azimuth to -0.1F
    * Set range to 50.0F
    */
   sensor_fov.Update_Calibrations(sensors);
   azimuth = -0.1F;
   range = 50.0F;

   /** \action
    * Call Is_Point_In_FOV
    */
   const bool f_is_in_fov = sensor_fov.Is_Point_In_FOV(azimuth, range);

   /** \result
    * Check whether returned value is false
    */
   CHECK_FALSE(f_is_in_fov);
}
/** @}*/
