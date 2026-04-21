/** \file
     File with set of qualification tests (which are also unit test)  for F360 input diagnostics module
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

/** \defgroup  f360_input_diagnostics_qualtest
 *  @{
 */

 /** \brief
  *  Set up initial values of the signals to be checked by the module.
  *  Keep in mind that class Execute methods needs to be run at least
  *  twice for the module to start detecting errors. In the first cycle
  *  no information about the previous cycle is known, so signals can't be
  *  verified for consistency
  */

TEST_GROUP(f360_input_diagnostics_qualtest)
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
};

/** \purpose
 * Execute function should return appropriate faults status when
 * core_info received is the same as in the last cycle
 * \req
 * FTCP-8603, FTCP-8837,
 * FTCP-11448,
 */
TEST(f360_input_diagnostics_qualtest, No_Increase_Faults_Should_Be_Returned_When_Core_Signal_Received_Twice_Without_Change)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    */
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signals check without incrementing core_info signal with new iteration
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.core_info.cnt_loops_no_increase);
   CHECK(actual_faults.core_info.time_us_no_increase);
   CHECK(actual_faults.host_info.vehicle_index_no_increase);
}

/** \purpose
 * Execute function should return appropriate fault status when the tracker_index
 * (core_info.cnt_loops) is lower than in the last cycle
 * \req
 * FTCP-8837
 */
TEST(f360_input_diagnostics_qualtest, Fault_Should_Be_Returned_When_Core_Info_Cnt_Loops_Lower_Than_Before)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Decrement core_info.cnt_loops signal value
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.cnt_loops--;

   /** \action
    * Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.core_info.cnt_loops_no_increase);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * core_info.time_us is lower than in the last cycle
 * \req
 * FTCP-8603
 */
TEST(f360_input_diagnostics_qualtest, Fault_Should_Be_Returned_When_Core_Info_Time_us_Lower_Than_Before)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Decrease core_info.time_us
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.time_us -= ms2us(30ULL);

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.core_info.time_us_no_increase);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * core_info.elapsed_time is below lower limit
 * \req
 * FTCP-8605
 */
TEST(f360_input_diagnostics_qualtest, Fault_Should_Be_Returned_When_Core_Info_Elapsed_Time_Below_Lower_Limit)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Modify core_info.elapsed_time to below limit
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.elapsed_time_s = input_diagnostics.calib.min_allowed_core_info_elapsed_time_s - 0.01F;

   /** \action
    * Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.core_info.elapsed_time_below_lower_limit);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * core_info.elapsed_time is above upper limit
 * \req
 * FTCP-8604
 */
TEST(f360_input_diagnostics_qualtest, Fault_Should_Be_Returned_When_Core_Info_Elapsed_Above_Upper_Limit)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Modify core_info.elapsed_time to above limit
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.elapsed_time_s = input_diagnostics.calib.max_allowed_core_info_elapsed_time_s + 0.01F;

   /** \action
    * Modify core_info.elapsed_time to above limit and Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.core_info.elapsed_time_above_upper_limit);
}

/** \purpose
 * Execute function should return appropriate fault status when the vehicle_index
 * is lower than in the last cycle
 * \req
 * FTCP-11448
 */
TEST(f360_input_diagnostics_qualtest, Fault_Should_Be_Returned_When_Host_Info_Vehicle_index_Lower_Than_Before)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Decrement host.vehicle_index signal value
    */
   input_diagnostics.Execute(core_info, host, sensors);
   host.vehicle_index--;

   /** \action
    * Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.vehicle_index_no_increase);
}

/** \purpose
 * Execute function should return appropriate fault status when the host qualifiers are QF_ACCURATE
 * \req
 * FTCP-8602, FTCP-10857, FTCP-13077, FTCP-13076
 */
TEST(f360_input_diagnostics_qualtest, True_Should_Be_Returned_When_Host_QF_Is_Accurate)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Set host Quality Factors to F360_QF_ACCURATE
    */
   input_diagnostics.Execute(core_info, host, sensors);
   host.speed_qf      = F360_QF_ACCURATE;
   host.yaw_rate_qf   = F360_QF_ACCURATE;
   host.long_accel_qf = F360_QF_ACCURATE;
   host.lat_accel_qf  = F360_QF_ACCURATE;
   // DFF-1725: Temporary workaround - host_yawrate_invalid changed to check for faulty curvature_rear or vcs_sideslip values
   host.curvature_rear = 0.0F;
   host.vcs_sideslip   = 0.0F;

   /** \action
    * Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.host_info.host_speed_invalid);
   CHECK_FALSE(actual_faults.host_info.host_yawrate_invalid);
   CHECK_FALSE(actual_faults.host_info.host_longitudinal_acceleration_invalid);
   CHECK_FALSE(actual_faults.host_info.host_lateral_acceleration_invalid);
}

/** \purpose
 * Execute function should return appropriate fault status when the host qualifiers are not QF_ACCURATE
 * \req
 * FTCP-8602, FTCP-10857, FTCP-13077, FTCP-13076
 */
TEST(f360_input_diagnostics_qualtest, True_Should_Be_Returned_When_Host_QF_Is_NOT_Accurate)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * i = 0: Set host Quality Factors to F360_QF_UNDEFINED
    * i = 1: Set host Quality Factors to F360_QF_TEMP_UNDEFINED
    * i = 2: Set host Quality Factors to F360_QF_INACCURATE
    */
   input_diagnostics.Execute(core_info, host, sensors);
   for (int i = 0; i < 3; i++)
   {
      host.speed_qf      = static_cast<F360_QUALITY_FACTOR>(i);
      // host.yaw_rate_qf   = static_cast<F360_QUALITY_FACTOR>(i);
      host.long_accel_qf = static_cast<F360_QUALITY_FACTOR>(i);
      host.lat_accel_qf  = static_cast<F360_QUALITY_FACTOR>(i);

      /** \action
       * Execute check
       */
      Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

      /** \result
       * Signal fault should be reported
       */
      CHECK(actual_faults.host_info.host_speed_invalid);
      // CHECK(actual_faults.host_info.host_yawrate_invalid);
      CHECK(actual_faults.host_info.host_longitudinal_acceleration_invalid);
      CHECK(actual_faults.host_info.host_lateral_acceleration_invalid);
   }
}

/** \purpose
 * Execute function should return appropriate fault status for different curvature_rear and vcs_sideslip values
 * \req
 * FTCP-10857
 */
TEST(f360_input_diagnostics_qualtest, True_Should_Be_Returned_When_Big_Curvature_Rear_Or_VCS_Sideslip)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Set curvature_rear to big negative value
    */
   input_diagnostics.Execute(core_info, host, sensors);
   host.curvature_rear = -5.3F;

   /** \action
    * Execute check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Reset curvature_rear to 0
    */
   host.curvature_rear = 0.0F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set vcs_sideslip to big negative value
    */
   host.vcs_sideslip = -5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set vcs_sideslip to big positive value
    */
   host.vcs_sideslip = 5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Reset vcs_sideslip to 0
    */
   host.vcs_sideslip = 0.0F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set curvature_rear to big negative value
    * Set vcs_sideslip to big positive value
    */
   host.curvature_rear = -5.3F;
   host.vcs_sideslip = 5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set curvature_rear to big positive value
    * Set vcs_sideslip to big negative value
    */
   host.curvature_rear = 5.3F;
   host.vcs_sideslip = -5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set curvature_rear to big positive value
    * Set vcs_sideslip to big positive value
    */
   host.curvature_rear = 5.3F;
   host.vcs_sideslip = 5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
   
   /** \precond
    * Set curvature_rear to big negative value
    * Set vcs_sideslip to big negative value
    */
   host.curvature_rear = -5.3F;
   host.vcs_sideslip = -5.3F;

   /** \action
    * Execute check
    */
   actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.host_info.host_yawrate_invalid);
}

/** \purpose
 * Execute function should return appropriate faults status when
 * sensors signal received is the same as in the last cycle
 * \req
 * FTCP-8591
 */
TEST(f360_input_diagnostics_qualtest, No_Increase_Faults_Should_Be_Returned_When_Sensors_Signal_Received_Twice_Without_Change)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Increment only core_info and host signals and not sensors signals
    */
   input_diagnostics.Execute(core_info, host, sensors);
   core_info.cnt_loops++;
   core_info.time_us += ms2us(30ULL);
   host.vehicle_index++;

   /** \action
    * Execute signals check without incrementing sensors signal with new iteration
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal faults should be reported
    */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; ++i)
   {
      CHECK(actual_faults.sensors[i].look_index_no_increase);
   }
}

/** \purpose
 * Execute function should return appropriate fault status when
 * sensors.look_index is the lower than in the last cycle
 * \req
 * FTCP-8591
 */
TEST(f360_input_diagnostics_qualtest, No_Increase_Fault_Should_Be_Returned_When_Look_Index_For_Single_Sensor_Lower_Than_Before)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Increment sensors[i].look_index for all sensors apart from one
    */
   input_diagnostics.Execute(core_info, host, sensors);

   const int FAULTY_SENSOR_IDX = 1;

   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; ++i)
   {
      if (i == FAULTY_SENSOR_IDX)
      {
         sensors[i].variable.look_index--;
      }
      else
      {
         sensors[i].variable.look_index++;
      }
   }

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors[FAULTY_SENSOR_IDX].look_index_no_increase);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * difference between sensors timestamp and tracker timestamp is higher than expected
 * \req
 * FTCP-8592
 */
TEST(f360_input_diagnostics_qualtest, Signal_Fault_Should_Be_Returned_When_Difference_Between_Sensor_And_Tracker_Timestamp_Too_High)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Fill timestamp_us for one faulty sensor with value above specification
    */
   input_diagnostics.Execute(core_info, host, sensors);
   const int FAULTY_SENSOR_IDX = 1;
   sensors[FAULTY_SENSOR_IDX].variable.timestamp_us = core_info.time_us - input_diagnostics.calib.max_allowed_sensor_vs_tracker_timestamp_diff_us - 1ULL;

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors[FAULTY_SENSOR_IDX].sensor_vs_tracker_timestamp_divergence);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * difference between sensors timestamp and tracker timestamp is lower than expected
 * \req
 * FTCP-8592
 */
TEST(f360_input_diagnostics_qualtest, Signal_Fault_Should_Be_Returned_When_Difference_Between_Sensor_And_Tracker_Timestamp_Too_Low)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Fill timestamp_us for one faulty sensor with value below specification
    */
   input_diagnostics.Execute(core_info, host, sensors);
   const int FAULTY_SENSOR_IDX = 1;
   sensors[FAULTY_SENSOR_IDX].variable.timestamp_us = core_info.time_us - input_diagnostics.calib.min_allowed_sensor_vs_tracker_timestamp_diff_us + 1ULL;

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors[FAULTY_SENSOR_IDX].sensor_vs_tracker_timestamp_divergence);
}

/** \purpose
 * Execute function should return appropriate fault status when
 * f_data_on_stack_is_empty is true and sensors data is zero
 * \req
 *.NA
 */
TEST(f360_input_diagnostics_qualtest, Signal_Fault_Should_Not_Be_Set_When_No_Sensor_Data_In_Received)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Then set the sensor 1 data to zeros.
    */
   input_diagnostics.Execute(core_info, host, sensors);

   const int FAULTY_SENSOR_IDX = 1;
   sensors[FAULTY_SENSOR_IDX].variable.timestamp_us = 0;
   sensors[FAULTY_SENSOR_IDX].variable.look_index = 0;

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should not be reported because when no input data is received the timestamp difference
    * check shall not be executed
    */

   CHECK_FALSE(actual_faults.sensors[FAULTY_SENSOR_IDX].sensor_vs_tracker_timestamp_divergence);
}

/** \purpose
 * Execute function must not return timestamp fault status when
 * sensor calibration is not valid
 * \req
 * FTCP-8592
 */
TEST(f360_input_diagnostics_qualtest, No_Fault_Reported_For_Sensor_When_Calibration_Not_Valid)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Fill timestamp_us for one faulty sensor with value below specification
    */
   input_diagnostics.Execute(core_info, host, sensors);
   const int SENSOR_IDX = 1;
   sensors[SENSOR_IDX].variable.is_valid = false;
   sensors[SENSOR_IDX].variable.timestamp_us = core_info.time_us - input_diagnostics.calib.min_allowed_sensor_vs_tracker_timestamp_diff_us + 1ULL;

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors[SENSOR_IDX].sensor_vs_tracker_timestamp_divergence);
}

/** \purpose
 * Execute function must not return sensor look_index_no_increase fault status
 * during look_index rollover (overflow)
 * \req
 * FTCP-8591
 */
TEST(f360_input_diagnostics_qualtest, No_Fault_Reported_For_Sensor_When_look_index_Overflow)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    * Simulate look index rollover (overflow)
    */
   const int SENSOR_IDX = 1;
   sensors[SENSOR_IDX].variable.look_index = 65535U; // Overflow value is 65535
   input_diagnostics.Execute(core_info, host, sensors);

   sensors[SENSOR_IDX].variable.look_index = 0U;

   /** \action
    * Execute signal check
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors[SENSOR_IDX].look_index_no_increase);
}

/** \purpose
 * Execute function should return sensor look_index_no_increase fault status when
 * sensor look_index equal to zero two times in a row
 * \req
 * FTCP-8591
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Look_Index_Equal_To_Zero_Twice_In_A_Row)
{
   /** \precond
    * Execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 1;
   sensors[SENSOR_IDX].variable.look_index = 0U;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with look_index equal to zero
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors[SENSOR_IDX].look_index_no_increase);
}

/** \purpose
 * Execute function should return sensor polarity_is_invalid fault status when
 * sensor polarity absolute value is equal to zero
 * \req
 * FTCP-8594
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Polarity_Is_Equal_To_Zero)
{
   /** \precond
    * Set polarity at zero and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 1;
   sensors[SENSOR_IDX].constant.polarity = 0;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with polarity equal to zero
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].polarity_is_invalid);
}

/** \purpose
 * Execute function should return sensor polarity_is_invalid fault status when
 * sensor polarity absolute value is greater than expected positive
 * \req
 * FTCP-8594
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Polarity_Is_Greater_Than_Expected_Positive)
{
   /** \precond
    * Set polarity value greater than expected and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 4;
   sensors[SENSOR_IDX].constant.polarity = input_diagnostics.calib.valid_polarity_abs + 1;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with polarity greater than expected
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].polarity_is_invalid);
}

/** \purpose
 * Execute function should return sensor polarity_is_invalid fault status when
 * sensor polarity absolute value is greater than expected negative
 * \req
 * FTCP-8594
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Polarity_Is_Greater_Than_Expected_Negative)
{
   /** \precond
    * Set polarity greater than expected negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.polarity = (-1) * input_diagnostics.calib.valid_polarity_abs - 3;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with polarity absolute value greater than exected
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].polarity_is_invalid);
}

/** \purpose
 * Execute function should return sensor polarity_is_invalid fault status when
 * sensor polarity absolute value is valid positive
 * \req
 * FTCP-8594
 */
TEST(f360_input_diagnostics_qualtest, Polarity_Fault_Should_Not_Be_Reported_When_Sensor_Polarity_Is_Valid_Positive)
{
   /** \precond
    * Set polarity to valid value and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 0;
   sensors[SENSOR_IDX].constant.polarity = input_diagnostics.calib.valid_polarity_abs;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with valid polarity value
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].polarity_is_invalid);
}

/** \purpose
 * Execute function should return sensor polarity_is_invalid fault status when
 * sensor polarity absolute value is valid negative
 * \req
 * FTCP-8594
 */
TEST(f360_input_diagnostics_qualtest, Polarity_Fault_Should_Not_Be_Reported_When_Sensor_Polarity_Is_Valid_Negative)
{
   /** \precond
    * Set polarity to valid negative value and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 2;
   sensors[SENSOR_IDX].constant.polarity = (-1) * input_diagnostics.calib.valid_polarity_abs;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with valid negative polarity
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].polarity_is_invalid);
}

/** \purpose
 * Execute function should return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is greater than expected positive
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Mounting_Position_Is_Too_High_Positive)
{
   /** \precond
    * Set mounting position to above the limit positive and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 2;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs + 0.1F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position set above limit positive
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is greater than expected negative
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Mounting_Position_Is_Too_High_Negative)
{
   /** \precond
    * Set mounting position to above the limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = (-1.0F)*input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs - 0.001F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position set above limit negative
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should NOT return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is equal to calibrated limit value positive
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Not_Be_Reported_When_Sensor_Mounting_Position_Equal_To_Limit_Positive)
{
   /** \precond
    * Set mounting position equal to the limit positive and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 4;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position equal to limit positive
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should NOT return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is equal to limit negative
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Not_Be_Reported_When_Sensor_Mounting_Position_Equal_To_Limit_Negative)
{
   /** \precond
    * Set mounting position equal to the limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = (-1.0F)*input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position equal to the limit negative
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should NOT return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is within limit positive
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Not_Be_Reported_When_Sensor_Mounting_Position_Is_Within_Limit_Positive)
{
   /** \precond
    * Set mounting position within limit positive and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs - 0.0001F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position within limit positive
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should NOT return sensor mounting_pos_is_invalid fault status when
 * sensor mounting position is within limit negative
 * \req
 * FTCP-8596
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Not_Be_Reported_When_Sensor_Mounting_Position_Is_Within_Limit_Negative)
{
   /** \precond
    * Set mounting position within limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_position.lateral = (-1.0F)*input_diagnostics.calib.max_allowed_lateral_mounting_position_abs_vcs + 0.01F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with mounting position within limit negative
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].mounting_pos_is_invalid);
}

/** \purpose
 * Execute function should return sensor boresight_angle_is_invalid fault status as true when
 * sensor azimuth boresight angle is greater than PI
 * \req
 * FTCP-10137
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Azimuth_Boresight_Angle_Is_Greater_Than_Upper_Limit)
{
   /** \precond
    * Set mounting position within limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_boresight_azimuth_angle = input_diagnostics.calib.max_allowed_azimuth_boresight_angle_abs_vcs + 0.01F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with azimuth boresight angle greatger than max allowed value.
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].boresight_angle_is_invalid);
}

/** \purpose
 * Execute function should return sensor boresight_angle_is_invalid fault status as true when
 * sensor azimuth boresight angle is less than (-1.0)*PI
 * \req
 * FTCP-10137
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Azimuth_Boresight_Angle_Is_Less_Than_Lower_Limit)
{
   /** \precond
    * Set mounting position within limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_boresight_azimuth_angle = (-1.0F)*input_diagnostics.calib.max_allowed_azimuth_boresight_angle_abs_vcs - 0.01F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with azimuth boresight angle less than minimum allowed value.
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should be reported
    */
   CHECK(actual_faults.sensors_calibs[SENSOR_IDX].boresight_angle_is_invalid);
}

/** \purpose
 * Execute function should return sensor boresight_angle_is_invalid fault status as false when
 * sensor azimuth boresight angle is within the lower limit
 * \req
 * FTCP-10137
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Azimuth_Boresight_Angle_Is_Within_Lower_Limit)
{
   /** \precond
    * Set mounting position within limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_boresight_azimuth_angle = (-1.0F)*input_diagnostics.calib.max_allowed_azimuth_boresight_angle_abs_vcs + 0.01F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with azimuth boresight angle within minimum allowed value.
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].boresight_angle_is_invalid);
}

/** \purpose
 * Execute function should return sensor boresight_angle_is_invalid fault status as false when
 * sensor azimuth boresight angle is within the upper limit
 * \req
 * FTCP-10137
 */
TEST(f360_input_diagnostics_qualtest, Signal_Faults_Should_Be_Reported_When_Sensor_Azimuth_Boresight_Angle_Is_Within_Upper_Limit)
{
   /** \precond
    * Set mounting position within limit negative and execute input diagnostics class for the first time to fill previous signal states
    */
   const int SENSOR_IDX = 3;
   sensors[SENSOR_IDX].constant.mounting_position.vcs_boresight_azimuth_angle = input_diagnostics.calib.max_allowed_azimuth_boresight_angle_abs_vcs - 0.01F;
   input_diagnostics.Execute(core_info, host, sensors);

   /** \action
    * Execute signal check again with azimuth boresight angle within max allowed value.
    */
   Input_Faults_T actual_faults = input_diagnostics.Execute(core_info, host, sensors);

   /** \result
    * Signal fault should NOT be reported
    */
   CHECK_FALSE(actual_faults.sensors_calibs[SENSOR_IDX].boresight_angle_is_invalid);
}

/** @}*/

