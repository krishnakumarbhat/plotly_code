/** \file
   This unit-test file contains UTs for F360 input diagnostics module
*/

#include "f360_input_diagnostics.h"
#include "f360_constants.h"


#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cstring>

using namespace f360_variant_A;

inline uint64_t ms2us(uint64_t in_ms)
{
   return (in_ms * 1000ULL);
}

/** \defgroup  f360_input_diagnostics
 *  @{
 */

 /** \brief
  *  Set up initial values of the signals to be checked by the module.
  *  Keep in mind that class Execute methods needs to be run at least
  *  twice for the module to start detecting errors. In the first cycle
  *  no information about the previous cycle is known, so signals can't be
  *  verified for consistency
  */

TEST_GROUP(f360_input_diagnostics)
{
   /* Define common data for all tests */
   F360_Core_Info_T core_info = {};
   F360_Host_T host = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   Input_Diagnostics input_diagnostics;

   TEST_SETUP()
   {
      /* core_info setup */
      core_info.time_us = ms2us(1100ULL);
      core_info.cnt_loops = 1000U;
      core_info.elapsed_time_s = 0.05F;

      /* host setup */
      host.vehicle_index = 1000U;
      host.speed_qf      = F360_QF_ACCURATE;
      host.yaw_rate_qf   = F360_QF_ACCURATE;
      host.long_accel_qf = F360_QF_ACCURATE;
      host.lat_accel_qf  = F360_QF_ACCURATE;      
      // DFF-1725: Temporary workaround - host_yawrate_invalid changed to check for faulty curvature_rear or vcs_sideslip values
      host.curvature_rear = 0.0F;
      host.vcs_sideslip   = 0.0F;

      /* sensors setup */
      uint64_t sensor_vs_tracker_timestamp_diff_us = ms2us(50ULL);

      for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
      {
         sensors[idx].variable.timestamp_us = core_info.time_us - sensor_vs_tracker_timestamp_diff_us;
         sensors[idx].variable.look_index = 160U;
      }

      /* sensors_calibs setup */
      for (int idx = 0; idx < MAX_NUMBER_OF_SENSORS; idx++)
      {
         sensors[idx].variable.is_valid = true;
         sensors[idx].constant.polarity = 1;
         sensors[idx].constant.mounting_position.vcs_position.lateral = 0.0F;
         sensors[idx].constant.mounting_position.vcs_boresight_azimuth_angle = 0.0F;
      }
   }

   // Checks if a Core_Info_Faults_T structs contain any faults
   bool Contain_Faults(const Core_Info_Faults_T& core_info)
   {
      bool fault = (core_info.cnt_loops_no_increase          ||
                            core_info.time_us_no_increase            ||
                            core_info.elapsed_time_below_lower_limit ||
                            core_info.elapsed_time_above_upper_limit);
      return fault;
   }
   
   // Checks if a Host_Info_Faults_T structs contain any faults
   bool Contain_Faults(const Host_Info_Faults_T& host_info)
   {
      bool fault = (host_info.vehicle_index_no_increase              ||
                            host_info.host_speed_invalid                     ||
                            host_info.host_yawrate_invalid                   ||
                            host_info.host_longitudinal_acceleration_invalid ||
                            host_info.host_lateral_acceleration_invalid);
      return fault;
   }
   
   // Checks if a Radar_Sensor_Faults_T structs contain any faults
   bool Contain_Faults(const Radar_Sensor_Faults_T& sensor)
   {
       bool fault = (sensor.look_index_no_increase ||
                             sensor.sensor_vs_tracker_timestamp_divergence);
      return fault;
   }
   
   // Checks if a Radar_Sensor_Calib_Faults_T structs contain any faults
   bool Contain_Faults(const Radar_Sensor_Calib_Faults_T& sensor_calib)
   {
      bool fault = (sensor_calib.mounting_pos_is_invalid ||
                            sensor_calib.polarity_is_invalid ||
                            sensor_calib.boresight_angle_is_invalid);
      return fault;
   }
   
   // Checks if a Input_Faults_T structs contain any faults
   bool Contain_Faults(const Input_Faults_T& input_faults)
   {
      bool fault = (Contain_Faults(input_faults.core_info) || Contain_Faults(input_faults.host_info));
      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; ++i)
      {
         fault = (fault || Contain_Faults(input_faults.sensors_calibs[i]) || Contain_Faults(input_faults.sensors[i]));
      }
      return fault;
   }
};

/** \purpose
 * First run of Execute function should not return any faults as previous signal states are
 * not known and conditions check can't be executed
 * \req
 * NA
 */
TEST(f360_input_diagnostics, No_Faults_Should_Be_Returned_With_First_Cycle)
{
   /** \precond
    * None
    */

    /** \action
     * Execute InputDiagnostics for the first time
     */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * There should be no reported faults
    */
   CHECK_FALSE(Contain_Faults(actual_faults));
}

/** \purpose
 * Execute function after Reset should not return any faults as previous signal states
 * are not known and conditions check can't be executed
 * \req
 * NA
 */
TEST(f360_input_diagnostics, No_Faults_Should_Be_Returned_With_First_Execution_After_Reset)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    */
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Reset and Execute Input_Diagnostics
    */
   input_diagnostics.Reset();
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * There should be no reported faults
    */
   CHECK_FALSE(Contain_Faults(actual_faults));
}

/** \purpose
 * Execute function should return no faults when all the signals
 * have been incremented correctly
 * \req
 * NA
 */
TEST(f360_input_diagnostics, No_Faults_Should_Be_Returned_When_Signals_Imcremented_Correct)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * New signals values should be incremented compared to the previous ones
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.cnt_loops++;
   core_info.time_us += ms2us(50000ULL);
   host.vehicle_index++;
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; ++i)
   {
      sensors[i].variable.look_index++;
      sensors[i].variable.timestamp_us += ms2us(50000ULL);
   }

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);


   /** \result
    * There should be no reported faults
    */
   CHECK_FALSE(Contain_Faults(actual_faults));
}
