/** \file
 * This file contains unit tests for content of f360_inputs_preprocessing.cpp file
 */

#include "f360_inputs_preprocessing.h"
#include "f360_sensor_capability.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_inputs_preprocessing
 *  @{
 */

TEST_GROUP(f360_inputs_preprocessing)
{	
   // Declare common variables used within all tests in this test group.
   const float32_t test_pass_th = 1e-6F;
   /** \setup
    * Describe what is done in test setup. Remove test setup function and this tag if it is not used.
    */
   TEST_SETUP()
   {
      
      // Set up a default scenario for your tests. E.g. assign values to common variables declared above.
   }

};

/** \purpose
 * Test that standard deviation for host yaw rate from SCM is updated correctly.
 * \req NA.
 */
TEST(f360_inputs_preprocessing, Sensor_Capability_Host__SCM_Yaw_Rate_Std)
{
    /** \precond
     * Deafult values with elasped time equal to 0.05F
     */
    float32_t elapsed_time = 0.05F;
    F360_Host_T host{};
    F360_Globals_T globals{};
    F360_Host_Props_T host_props{};

    const float32_t expected_yaw_rate_std = 0.001F;

    /** \action
     * Call Sensor_Capability_Host
     */
    Sensor_Capability_Host(elapsed_time, host, globals.max_otg_speed, host_props);

    /** \result
     * Verify that standard deviation for host yaw rate has been updated as expected.
     */
    DOUBLES_EQUAL_TEXT(expected_yaw_rate_std, host_props.std_yaw_rate_scm, test_pass_th, "Standard deviation for host yaw rate did not match expected value.");
}
/** @}*/
