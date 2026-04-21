/** \file
Unit test to evaluates the fault status from Input Diagnostics and Output Diagnostics
*/

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

#include "f360_input_diagnostics_mock.h"
#include "f360_output_diagnostics_mock.h"
#include "f360_safety_control_logic.h"

using namespace f360_variant_A;

inline bool operator==(const SafetyControlLogic::SCL_Output_T& first, const SafetyControlLogic::SCL_Output_T& second)
{
   bool f_is_equal = (first.should_reset == second.should_reset) &&
      (first.core_info_fault_status == second.core_info_fault_status) &&
      (first.host_info_fault_status == second.host_info_fault_status) &&
      (first.object_track_fault_status  == second.object_track_fault_status) &&
      (first.overall_fault_status  == second.overall_fault_status);
   for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      f_is_equal = f_is_equal &&
         (first.sensors_calibs_fault_status[i] == second.sensors_calibs_fault_status[i]) &&
         (first.sensors_fault_status[i] == second.sensors_fault_status[i]);
   }
   return (f_is_equal);
}

inline bool operator!=(const SafetyControlLogic::SCL_Output_T& first, const SafetyControlLogic::SCL_Output_T& second)
{
   return !(first == second);
}

SimpleString StringFrom(const SafetyControlLogic::CYCLE_FAULT_STATUS status)
{
   switch (status)
   {
      case SafetyControlLogic::FAULT_PRESENT_STATUS:
         return{ "FAULT_PRESENT_STATUS" };
      case SafetyControlLogic::FAULT_PARTIAL_PRESENT_STATUS:
         return{ "FAULT_PARTIAL_PRESENT_STATUS" };
      case SafetyControlLogic::FAULT_NOT_PRESENT_STATUS:
         return{ "FAULT_NOT_PRESENT_STATUS" };
      default:
         return{ "fault_undefined_in_StringFrom" };
   }
}

SimpleString StringFrom(const SafetyControlLogic::SCL_Output_T& scl_out)
{
   return SimpleString("[fault_status: ") + StringFrom(scl_out.overall_fault_status) +
      SimpleString(", should_reset: ") + StringFrom(scl_out.should_reset) +
      SimpleString("]");
}

SafetyControlLogic::SCL_Output_T get_empty_SCL_Output()
{
   SafetyControlLogic::SCL_Output_T scl_output = {};
   scl_output.core_info_fault_status    = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
   scl_output.host_info_fault_status    = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
   scl_output.object_track_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      scl_output.sensors_calibs_fault_status[i] = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      scl_output.sensors_fault_status[i]        = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
   }
   scl_output.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
   scl_output.should_reset = false;

   return (scl_output);
}

TEST_GROUP(f360_safety_control_logic)
{
   const F360_Core_Info_T core_info = {};
   const F360_Host_T host = {};
   const F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   const F360_Object_Log_Output_T obj_log = {};

   TEST_SETUP()
   {}

   TEST_TEARDOWN()
   {
      mock().checkExpectations();
      mock().clear();
   }

   Input_Diagnostics_Mock input_diagnostics_mock;
   Output_Diagnostics_Mock output_diagnostics_mock;
   SafetyControlLogic safetyControlLogic { input_diagnostics_mock, output_diagnostics_mock };
};

/**
*\purpose  when there is no Input Faults from Input Diagnostics and no Output Faults from Output
*\         Diagnostics Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and
*\         should_reset as False.
*\req      FTCP-8907
*/
TEST(f360_safety_control_logic, whenThereIsNoFaults)
{
   /** \precond
   * there are no faults from Input_Diagnostics and Output_Diagnostics
   */
   Input_Faults_T input_faults = {};
   Output_Faults_T output_faults = {};
   SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

   /** \action
   * call evaluate_cycle
   */
   SafetyControlLogic::SCL_Output_T result = safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   /** \result
   * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
   */
   CHECK_EQUAL(expected, result);
}

/**
*\purpose  when there is Input Fault from Core_Info_Faults_T, after 1 cycle Evaluate Cycle
*\         should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\req      FTCP-8623, FTCP-8604, FTCP-8605, FTCP-8603, FTCP-8837
*/
TEST(f360_safety_control_logic, whenThereIsCoreInputFaultforOneCycle)
{
   /** \precond
   * there are input faults
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle once
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);


      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is Input Fault from Core_Info_Faults_T, after 2 cycle Evaluate Cycle
*\         should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\req      FTCP-8623, FTCP-8604, FTCP-8605, FTCP-8603, FTCP-8837
*/
TEST(f360_safety_control_logic, whenThereIsCoreInputFaultforTwoCycle)
{
   /** \precond
   * Input fault exists and evaluate cycle should be called once
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //Call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle second time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when Input Fault from Core_Info_Faults_T is disappears after 2 cycle,
*\         Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\req      FTCP-8623, FTCP-8604, FTCP-8605, FTCP-8603, FTCP-8837
*/
TEST(f360_safety_control_logic, whenThereIsNoCoreInputFaultAfterTwoCycles)
{
   /** \precond
   * there are input faults and evaluate cycle called twice and fault disappears
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle second time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //Check No Faults of Core_Info_Faults_T after 2 cycles
      input_faults.core_info.time_us_no_increase            = false;
      input_faults.core_info.cnt_loops_no_increase          = false;
      input_faults.core_info.elapsed_time_below_lower_limit = false;
      input_faults.core_info.elapsed_time_above_upper_limit = false;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is Core Info Input Fault, after 3 cycle Evaluate Cycle should return
*\         core_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8623, FTCP-8604, FTCP-8605, FTCP-8603, FTCP-8837, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsCoreInputFaultEvalueateCycleCalledThrice)
{
   /** \precond
   * there are input faults
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle once
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle twice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle thrice
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS after 3 indices and should_reset as false
      expected.core_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is core info Input Fault, after 3 cycle Evaluate Cycle should return
*\         core_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false. To set back fault_status
*\         as FAULT_NOT_PRESENT_STATUS, it should check for 10 indices for no faults
*\req      FTCP-8623, FTCP-8604, FTCP-8605, FTCP-8603, FTCP-8837, FTCP-8906, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsElapsedTimeFaultEvalueateCycleCalledTenTimesToCheckNoFaults)
{
   /** \precond
   * there are input faults
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle once
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle twice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle thrice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //No faults
      input_faults.core_info.time_us_no_increase            = false;
      input_faults.core_info.cnt_loops_no_increase          = false;
      input_faults.core_info.elapsed_time_below_lower_limit = false;
      input_faults.core_info.elapsed_time_above_upper_limit = false;

      //Call evaluate_cycle 9 times
      for (int j = 0; j<9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         expected.core_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);
      }

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as false
      */
      expected.core_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is Input Fault, after 3 cycle Evaluate Cycle should return
*\         core_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false. To set back fault_status
*\         as FAULT_NOT_PRESENT_STATUS, it should check for 10 indices for no faults. While
*\         Setting back to FAULT_NOT_PRESENT_STATUS, if fault occurs the no fault should be checked
*\         again for 10 more indices
*\req      FTCP-8906, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsCoreFaultEvalueateCycleCalledTenTimesToCheckNoFaults)
{
   /** \precond
   * there are input faults
   * i = 0: time_us_no_increase fault
   * i = 1: cnt_loops_no_increase fault
   * i = 2: elapsed_time_below_lower_limit fault
   * i = 3: elapsed_time_above_upper_limit fault
   */

   // Loop over the different faults
   for (int i = 0; i < 4; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Input faults exist
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle once
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle twice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle thrice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //No faults
      input_faults.core_info.time_us_no_increase            = false;
      input_faults.core_info.cnt_loops_no_increase          = false;
      input_faults.core_info.elapsed_time_below_lower_limit = false;
      input_faults.core_info.elapsed_time_above_upper_limit = false;

      //Call evaluate_cycle 6 times
      for (int j = 0; j<6; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         expected.core_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);
      }

      //Faults present
      input_faults.core_info.time_us_no_increase            = (0 == i);
      input_faults.core_info.cnt_loops_no_increase          = (1 == i);
      input_faults.core_info.elapsed_time_below_lower_limit = (2 == i);
      input_faults.core_info.elapsed_time_above_upper_limit = (3 == i);


      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //Call Evaluate_cycle
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //Evaluate Cycle should return core_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false
      expected.core_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);

      //No faults
      input_faults.core_info.time_us_no_increase            = false;
      input_faults.core_info.cnt_loops_no_increase          = false;
      input_faults.core_info.elapsed_time_below_lower_limit = false;
      input_faults.core_info.elapsed_time_above_upper_limit = false;

      //Call evaluate_cycle 9 times
      for (int j = 0; j<9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         expected.core_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);
      }

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return core_info_fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as false
      */
      expected.core_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  When there is Input Fault from Host_Info_Faults_T for 1 consecutive cycles.
*\         Evaluate Cycle should return host_info_fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\         Test for all the different host info faults
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsHostInputFaultforOneCycle)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * call evaluate_cycle once
      */
      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  When there is Input Fault from Host_Info_Faults_T for 2 consecutive cycles.
*\         Evaluate Cycle should return host_info_fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\         Test for all the different host info faults
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsHostInputFaultforTwoCycle)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * call evaluate_cycle two times time
      */
      for (int j = 0; j < 2; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      }

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  When there is Input Fault from Host_Info_Faults_T for 2 consecutive cycles then the fault dissapears.
*\         Evaluate Cycle should return host_info_fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False.
*\         Test for all the different host info faults
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsNoHostInputFaultAfterTwoCycles)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * 1. call evaluate_cycle 2 times
      * 2. Reset the fault
      * 3. call evaluate cycle a third time
      */
      // 1. call evaluate_cycle 2 times
      for (int j = 0; j < 2; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      }

      // 2. Reset faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = false;
      input_faults.host_info.host_speed_invalid                     = false;
      input_faults.host_info.host_yawrate_invalid                   = false;
      input_faults.host_info.host_longitudinal_acceleration_invalid = false;
      input_faults.host_info.host_lateral_acceleration_invalid      = false;

      // 3. call evaluate cycle a third time 
      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  When there is Input Fault from Host_Info_Faults_T for 3 consecutive cycles.
*\         Evaluate Cycle should return host_info_fault_status as FAULT_PRESENT_STATUS and should_reset as False.
*\         Test for all the different host info faults
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsHostInputFaultforThreeCycles)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * 1. call evaluate_cycle 2 times (Fault should no be present)
      * 2. call evaluate cycle a third time
      */
      // 1. call evaluate_cycle 2 times
      for (int j = 0; j < 2; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         CHECK_EQUAL(expected, result);
      }

      // 3. call evaluate cycle a third time
      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
      */
      expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
      if ((0 != i) && (2 != i))
      {
         expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      }
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is Input Fault, after 3 cycle Evaluate Cycle should return
*\         host_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false. To set back fault_status
*\         as FAULT_NOT_PRESENT_STATUS, it should check for 10 indices for no faults
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083,  FTCP-13209, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsHostInfoFaultEvaluateCycleCalledTenTimesToCheckNoFaults)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * 1. call evaluate_cycle 3 times
      * 2. Check that fault is preset
      * 3. Reset Fault
      * 4. call evaluate_cycle 9 times (Fault should be present)
      * 5. call evaluate_cycle time 10 after reset
      */
      // 1. call evaluate_cycle 3 times
      for (int j = 0; j < 3; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      }

      // 2. Check that fault is preset
      expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
      if ((0 != i) && (2 != i))
      {
         expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      }
      CHECK_EQUAL(expected, result);

      // 3. Reset faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = false;
      input_faults.host_info.host_speed_invalid                     = false;
      input_faults.host_info.host_yawrate_invalid                   = false;
      input_faults.host_info.host_longitudinal_acceleration_invalid = false;
      input_faults.host_info.host_lateral_acceleration_invalid      = false;

      // 4. call evaluate_cycle 9 times
      for (int j = 0; j < 9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
      if ((0 != i) && (2 != i))
         {
            expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
            expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         }
         CHECK_EQUAL(expected, result);
      }

      // 5. call evaluate_cycle time 10 after reset
      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as false
      */
      expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is Input Fault, after 3 cycle Evaluate Cycle should return
*\         host_info_fault_status as FAULT_PRESENT_STATUS and should_reset as false. To set back fault_status
*\         as FAULT_NOT_PRESENT_STATUS, it should check for 10 indices for no faults. While
*\         Setting back to FAULT_NOT_PRESENT_STATUS, if fault occurs the no fault should be checked
*\         again for 10 more indices
*\req      FTCP-8628,  FTCP-8627,  FTCP-13082,  FTCP-13081,  FTCP-13083,  FTCP-13209, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsHostInfoFaultEvaluateCycleCalledTenTimesToCheckNoFaults_2)
{
   /** \precond
   * Set host info fault to true
   * i = 0: vehicle_index_no_increase fault
   * i = 1: host_speed_invalid fault
   * i = 2: host_yawrate_invalid fault
   * i = 3: host_longitudinal_acceleration_invalid fault
   * i = 4: host_lateral_acceleration_invalid fault
   */

   // Loop over the different faults
   for (int i = 0; i < 5; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      // Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      /** \action
      * 1. call evaluate_cycle 3 times
      * 2. Check that fault is preset
      * 3. Reset Fault
      * 4. call evaluate_cycle 6 times (Fault should be present)
      * 5. Set fault
      * 6. call evaluate_cycle (Fault should be present)
      * 7. Reset Fault
      * 8. call evaluate_cycle 9 times (Fault should be present)
      * 9. call evaluate_cycle time 10 after reset
      */
      // 1. call evaluate_cycle 3 times
      for (int j = 0; j < 3; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      }

      // 2. Check that fault is preset
      expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
      if ((0 != i) && (2 != i))
      {
         expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      }
      CHECK_EQUAL(expected, result);

      // 3. Reset faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = false;
      input_faults.host_info.host_speed_invalid                     = false;
      input_faults.host_info.host_yawrate_invalid                   = false;
      input_faults.host_info.host_longitudinal_acceleration_invalid = false;
      input_faults.host_info.host_lateral_acceleration_invalid      = false;

      // 4. call evaluate_cycle 6 times
      for (int j = 0; j < 6; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
         if ((0 != i) && (2 != i))
         {
            expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
            expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         }
         CHECK_EQUAL(expected, result);
      }

      // 5. Set faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = (0 == i);
      input_faults.host_info.host_speed_invalid                     = (1 == i);
      input_faults.host_info.host_yawrate_invalid                   = (2 == i);
      input_faults.host_info.host_longitudinal_acceleration_invalid = (3 == i);
      input_faults.host_info.host_lateral_acceleration_invalid      = (4 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      // 6. call evaluate_cycle 
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
      if ((0 != i) && (2 != i))
      {
         expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      }
      CHECK_EQUAL(expected, result);

      // 7. Reset faults of Core_Info_Faults_T
      input_faults.host_info.vehicle_index_no_increase              = false;
      input_faults.host_info.host_speed_invalid                     = false;
      input_faults.host_info.host_yawrate_invalid                   = false;
      input_faults.host_info.host_longitudinal_acceleration_invalid = false;
      input_faults.host_info.host_lateral_acceleration_invalid      = false;

      // 8. Call evaluate_cycle 9 times
      for (int j = 0; j < 9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         expected.host_info_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         // Temporary workaround - Force the host_info_fault_status and overall_Fault_status to FAULT_NOT_PRESENT_STATUS except for stale check and yawrate invalid check. TODO: remove in DFF-958
         if ((0 != i) && (2 != i))
         {
            expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
            expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         }
         expected.should_reset = false;
         CHECK_EQUAL(expected, result);
      }

      // 9. call evaluate_cycle time 10 after reset
      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as false
      */
      expected.host_info_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.should_reset = false;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is look_index_no_increase Fault from Input Diagnostics,
*\         after 2 cycle Evaluate Cycle should return fault_status as
*\         FAULT_NOT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8834
*/
TEST(f360_safety_control_logic, whenThereIsLookIndexFaultforTwoCycles)
{
   /** \precond
   * there are input faults
   */

   //Faults of look_index_no_increase one sensor per loop
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors[i].look_index_no_increase = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //Call evaluate_cycle
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle second time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as False
      */
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is look_index_no_increase Fault from Input Diagnostics,
*\         after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8834, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsLookIndexFault)
{
   /** \precond
   * there are input faults
   */

   //Faults of look_index_no_increase one sensor per loop
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors[i].look_index_no_increase = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle second time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
      */
      expected.sensors_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is sensor_vs_tracker_timestamp_divergence Fault from Input Diagnostics,
*\         after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8835, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsRadarSensorTimeStrampFault)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle once 
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle twice
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
      */
      expected.sensors_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is one sensor Fault from Input Diagnostics, after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\         To set back fault_status to FAULT_NOT_PRESENT_STATUS, it should check for 10 indices with no faults
*\req      FTCP-8835, FTCP-8906, FTCP-13208, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsSensorTimestampFaultEvalueateCycleCalledTenTimesToCheckNoFaults)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      // Check fault present
      expected.sensors_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);

      //No Fault
      input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = false;

      for (int j = 0; j< 9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         expected.sensors_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);
      }

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as False
      */
      expected.sensors_fault_status[i] = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is sensor Fault from Input Diagnostics,after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\         To set back fault_status to Ok, it should check for 10 indices with no faults.
*\         While setting back to FAULT_NOT_PRESENT_STATUS, if there fault occurs, again
*\         no faults checked for 10 more indices.
*\req      FTCP-8823, FTCP-8596, FTCP-13208, FTCP-8822, FTCP-8594, FTCP-10147, FTCP-10137
*\         FTCP-8835, FTCP-8592, FTCP-13206, FTCP-8834, FTCP-8591, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThreIsSensorFaultEvalueateCycleCalledTenTimesToCheckNoFaults)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++) // Loop all sensors
   {
      for (int j = 0; j < 5; j++)
      {
         Input_Faults_T input_faults = {};
         Output_Faults_T output_faults = {};
         SafetyControlLogic::SCL_Output_T result = {};
         SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
         SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

         //Sensor Faults Present
         input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = (0 == j);
         input_faults.sensors[i].look_index_no_increase                 = (1 == j);
         input_faults.sensors_calibs[i].polarity_is_invalid             = (2 == j);
         input_faults.sensors_calibs[i].mounting_pos_is_invalid         = (3 == j);
         input_faults.sensors_calibs[i].boresight_angle_is_invalid      = (4 == j);

         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         //call evaluate_cycle
         safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         //call evaluate_cycle
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         CHECK_EQUAL(expected, result);

         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         //call evaluate_cycle
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         // Check fault present
         expected.sensors_fault_status[i]        = (2 > j) ? SafetyControlLogic::FAULT_PRESENT_STATUS : SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.sensors_calibs_fault_status[i] = (2 > j) ? SafetyControlLogic::FAULT_NOT_PRESENT_STATUS : SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);

         //No Faults
         input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = false;
         input_faults.sensors[i].look_index_no_increase                 = false;
         input_faults.sensors_calibs[i].polarity_is_invalid             = false;
         input_faults.sensors_calibs[i].mounting_pos_is_invalid         = false;
         input_faults.sensors_calibs[i].boresight_angle_is_invalid      = false;

         for (int k = 0; k< 5; k++)
         {
            mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
            mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

            result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
            expected.sensors_fault_status[i]        = (2 > j) ? SafetyControlLogic::FAULT_PRESENT_STATUS : SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
            expected.sensors_calibs_fault_status[i] = (2 > j) ? SafetyControlLogic::FAULT_NOT_PRESENT_STATUS : SafetyControlLogic::FAULT_PRESENT_STATUS;
            expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
            CHECK_EQUAL(expected, result);
         }

         //Faults Present
         input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = (0 == j);
         input_faults.sensors[i].look_index_no_increase                 = (1 == j);
         input_faults.sensors_calibs[i].polarity_is_invalid             = (2 == j);
         input_faults.sensors_calibs[i].mounting_pos_is_invalid         = (3 == j);
         input_faults.sensors_calibs[i].boresight_angle_is_invalid      = (4 == j);

         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         // check evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
         expected.sensors_fault_status[i]        = (2 > j) ? SafetyControlLogic::FAULT_PRESENT_STATUS : SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
         expected.sensors_calibs_fault_status[i] = (2 > j) ? SafetyControlLogic::FAULT_NOT_PRESENT_STATUS : SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         CHECK_EQUAL(expected, result);

         //No Faults
         input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence = false;
         input_faults.sensors[i].look_index_no_increase                 = false;
         input_faults.sensors_calibs[i].polarity_is_invalid             = false;
         input_faults.sensors_calibs[i].mounting_pos_is_invalid         = false;
         input_faults.sensors_calibs[i].boresight_angle_is_invalid      = false;
         for (int k = 0; k<9; k++)
         {
            mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
            mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

            result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
            expected.sensors_fault_status[i]        = (2 > j) ? SafetyControlLogic::FAULT_PRESENT_STATUS : SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
            expected.sensors_calibs_fault_status[i] = (2 > j) ? SafetyControlLogic::FAULT_NOT_PRESENT_STATUS : SafetyControlLogic::FAULT_PRESENT_STATUS;
            expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
            CHECK_EQUAL(expected, result);
         }

         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         /** \action
         * call evaluate_cycle
         */
         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         /** \result
         * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS after 10 indices and should_reset as False
         */
         expected = get_empty_SCL_Output();
         CHECK_EQUAL(expected, result);
      }
   }
}

/**
*\purpose  when there is mounting_pos_is_invalid Fault from Input Diagnostics,
*\         after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8823, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsMountPosInvalidFault)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors_calibs[i].mounting_pos_is_invalid = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle second time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
      */
      expected.sensors_calibs_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is polarity_is_invalid Fault from Input Diagnostics,
*\         after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-8822, FTCP-13217
*/
TEST(f360_safety_control_logic, whenThereIsMountPolarityInvalidFault)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors_calibs[i].polarity_is_invalid = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle second time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as False
      */
      expected.sensors_calibs_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when boresight_angle_is_invalid is set to true from Input Diagnostics,
*\         after 3 cycle Evaluate Cycle
*\         should return fault_status as FAULT_PRESENT_STATUS and should_reset as false.
*\req      FTCP-10137, FTCP-13217
*/
TEST(f360_safety_control_logic, When_Mount_Boresight_Angle_Is_Invalid)
{
   /** \precond
   * there are input faults
   */
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      Input_Faults_T input_faults = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      input_faults.sensors_calibs[i].boresight_angle_is_invalid = true;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle first time
      safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle second time
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      CHECK_EQUAL(expected, result); // After two cycles of fault present, the fault status should not set yet.
      /** \action
      * call evaluate_cycle third time
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS after three cycles and should_reset as False
      */
      expected.sensors_calibs_fault_status[i] = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      CHECK_EQUAL(expected, result);
   }
}
/**
*\purpose  when there is fault signals from Output Diagnostics, Evaluate Cycle
*\         should always return fault_status as FAULT_PRESENT_STATUS and should_reset as True.
*\req      FTCP-8833, FTCP-8613, FTCP-8610, FTCP-8611, FTCP-8608, FTCP-13217, FTCP-13282
*/
TEST(f360_safety_control_logic, whenThereIsOutputFaultFault)
{
   /** \precond
   * there are output faults
   * i = 0: f_track_positions_faulty
   * i = 1: f_track_velocities_faulty
   * i = 2: f_track_accelerations_faulty
   */

   // Loop over the different faults
   for (int i = 0; i < 3; i++)
   {
      Input_Faults_T input_faults   = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = (0 == i);
      output_faults.f_track_velocities_faulty    = (1 == i);
      output_faults.f_track_accelerations_faulty = (2 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      //Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as true
      expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.should_reset = true;

      CHECK_EQUAL(expected, result);

      //No Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = false;
      output_faults.f_track_velocities_faulty    = false;
      output_faults.f_track_accelerations_faulty = false;

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);


      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_PRESENT_STATUS and should_reset as false
      */
      expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.should_reset = false;

      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is faults from Output Diagnostics for one cycle than then no faults.
*\         object_track_fault_status should be FAULT_PRESENT_STATUS for ten cycles.
*\         should_reset should be true only when the fault is true.
*\req      FTCP-8833, FTCP-8613, FTCP-8610, FTCP-8611, FTCP-8608, FTCP-13207, FTCP-13217, FTCP-13282
*/
TEST(f360_safety_control_logic, whenThereIsOuputFaultsForOneCycleAndThenNoFaults)
{
   /** \precond
   * there are output faults
   * i = 0: f_track_positions_faulty
   * i = 1: f_track_velocities_faulty
   * i = 2: f_track_accelerations_faulty
   */

   // Loop over the different faults
   for (int i = 0; i < 3; i++)
   {
      Input_Faults_T input_faults   = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = (0 == i);
      output_faults.f_track_velocities_faulty    = (1 == i);
      output_faults.f_track_accelerations_faulty = (2 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.should_reset = true;
      CHECK_EQUAL(expected, result);

      //No Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = false;
      output_faults.f_track_velocities_faulty    = false;
      output_faults.f_track_accelerations_faulty = false;

      //Call evaluate_cycle for 9 times
      for (int j = 0; j< 9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.should_reset = false;

         CHECK_EQUAL(expected, result);
      }

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as false
      */
      expected.object_track_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.should_reset = false;

      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is faults from Output Diagnostics for one cycle than then no faults for
*\         five cycles and the fault again for one more cycle, and then no faults.
*\         object_track_fault_status should be FAULT_PRESENT_STATUS for ten cycles after the last fault.
*\         should_reset should be true only when the fault is true.
*\req      FTCP-8833, FTCP-8613, FTCP-8610, FTCP-8611, FTCP-8608, FTCP-13207, FTCP-13217, FTCP-13282
*/
TEST(f360_safety_control_logic, whenThereIsOuputFaultsForOneCycleAndThenAgainAfterFiveCycles)
{
   /** \precond
   * there are output faults
   * i = 0: f_track_positions_faulty
   * i = 1: f_track_velocities_faulty
   * i = 2: f_track_accelerations_faulty
   */

   // Loop over the different faults
   for (int i = 0; i < 3; i++)
   {
      Input_Faults_T input_faults   = {};
      Output_Faults_T output_faults = {};
      SafetyControlLogic::SCL_Output_T result = {};
      SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();
      SafetyControlLogic safetyControlLogic_loop { input_diagnostics_mock, output_diagnostics_mock };

      //Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = (0 == i);
      output_faults.f_track_velocities_faulty    = (1 == i);
      output_faults.f_track_accelerations_faulty = (2 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      //call evaluate_cycle
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.should_reset = true;
      CHECK_EQUAL(expected, result);

      //No Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = false;
      output_faults.f_track_velocities_faulty    = false;
      output_faults.f_track_accelerations_faulty = false;

      //Call evaluate_cycle for 5 times
      for (int j = 0; j< 5; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
         expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.should_reset = false;
         CHECK_EQUAL(expected, result);
      }

      //Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = (0 == i);
      output_faults.f_track_velocities_faulty    = (1 == i);
      output_faults.f_track_accelerations_faulty = (2 == i);

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);
      expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
      expected.should_reset = true;
      CHECK_EQUAL(expected, result);

      //Faults of F360_Object_Track_T
      output_faults.f_track_positions_faulty     = false;
      output_faults.f_track_velocities_faulty    = false;
      output_faults.f_track_accelerations_faulty = false;

      //Call evaluate_cycle for 9 times
      for (int j = 0; j< 9; j++)
      {
         mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
         mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

         result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

         expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
         expected.should_reset = false;
         CHECK_EQUAL(expected, result);
      }

      mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
      mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

      /** \action
      * call evaluate_cycle
      */
      result = safetyControlLogic_loop.evaluate_cycle(core_info, host, sensors, obj_log);

      /** \result
      * Evaluate Cycle should return fault_status as FAULT_NOT_PRESENT_STATUS and should_reset as false
      */
      expected.object_track_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.overall_fault_status = SafetyControlLogic::FAULT_NOT_PRESENT_STATUS;
      expected.should_reset = false;

      CHECK_EQUAL(expected, result);
   }
}

/**
*\purpose  when there is different outputs of SCL_Output_T the Operator should be not equal.
*\req      NA
*/
TEST(f360_safety_control_logic, operatorBoolNotEqualCheck)
{
   /** \precond
   * there are different SCL_Output_T
   */
   SafetyControlLogic::SCL_Output_T scl_out1 = get_empty_SCL_Output();
   SafetyControlLogic::SCL_Output_T scl_out2 = get_empty_SCL_Output();
   scl_out1.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
   scl_out1.should_reset = true;

   /** \action
   * call operator bool
   */
   bool result = operator==(scl_out1, scl_out2);

   /** \result
   * Check operator returns false
   */
   bool expected = false;
   CHECK_EQUAL(expected, result);
}

/**
*\purpose  when there is different outputs of SCL_Output_T the Operator should be not equal.
*\req      NA
*/
TEST(f360_safety_control_logic, operatorBoolOneObjectNotEqualCheck)
{
   /** \precond
   * there are different should_reset
   */
   SafetyControlLogic::SCL_Output_T scl_out1 = get_empty_SCL_Output();
   SafetyControlLogic::SCL_Output_T scl_out2 = get_empty_SCL_Output();
   scl_out1.should_reset = true;

   /** \action
   * call operator bool
   */
   bool result = operator==(scl_out1, scl_out2);

   /** \result
   * Check operator returns false
   */
   bool expected = false;
   CHECK_EQUAL(expected, result);
}

/**
*\purpose  Check input is returned correctly after execution of Input Diagnostics
*\req      NA
*/
TEST(f360_safety_control_logic, checkInputFaultReturned)
{
   /** \precond
   * there are input faults
   */
   Input_Faults_T input_faults = {};
   Output_Faults_T output_faults = {};

   bool expected_input_fault = true;

   input_faults.core_info.time_us_no_increase = true;

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

   //call evaluate_cycle once
   safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

   //call evaluate_cycle twice
   safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);


   //call evaluate_cycle thrice
   safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   /** \action
   * call get_input_status
   */
   const Input_Faults_T& result = safetyControlLogic.get_input_status();

   /** \result
   * Check get_input_status returned value
   */
   CHECK_EQUAL(expected_input_fault, result.core_info.time_us_no_increase);
}

/**
*\purpose  Check Output is returned correctly after execution of Output Diagnostics
*\req      NA
*/
TEST(f360_safety_control_logic, checkOutputFaultReturned)
{
   /** \precond
   * there are output faults
   */
   Input_Faults_T input_faults = {};
   Output_Faults_T output_faults = {};

   bool expected_output_fault = true;

   output_faults.f_track_positions_faulty = true;

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

   //call evaluate_cycle
   safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   /** \action
   * call get_input_status
   */
   const Output_Faults_T& result = safetyControlLogic.get_output_status();

   /** \result
   * Check get_output_status returned value
   */
   CHECK_EQUAL(expected_output_fault, result.f_track_positions_faulty);
}

/**
*\purpose  Check output is returned correctly after execution of Safety Control Logic
*\req      NA
*/
TEST(f360_safety_control_logic, checkSclOutputReturned)
{
   /** \precond
   * there are output faults
   */
   Input_Faults_T input_faults = {};
   Output_Faults_T output_faults = {};
   SafetyControlLogic::SCL_Output_T expected = get_empty_SCL_Output();

   expected.object_track_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
   expected.overall_fault_status = SafetyControlLogic::FAULT_PRESENT_STATUS;
   expected.should_reset = true;

   output_faults.f_track_positions_faulty = true;

   mock().expectOneCall("Input_Diagnostics::Execute").andReturnValue(&input_faults);
   mock().expectOneCall("Output_Diagnostics::Execute").andReturnValue(&output_faults);

   //call evaluate_cycle once
   safetyControlLogic.evaluate_cycle(core_info, host, sensors, obj_log);

   /** \action
   * call get_scl_status
   */
   const SafetyControlLogic::SCL_Output_T& result = safetyControlLogic.get_scl_status();

   /** \result
   * Check get_scl_status returned value
   */
   CHECK_EQUAL(expected, result);
}
/** @}*/

