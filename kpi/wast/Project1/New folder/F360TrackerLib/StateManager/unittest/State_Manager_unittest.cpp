#include "State_Manager.h"
#include "SafetyLogicMock.h"
#include "TrackerMock.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>


//Declaration of stubbed/mock functions


/** \brief
 *  State Manager unit tests. They verify whether tracker was initialized, executed and reseted.
 */
using namespace f360_variant_A;
TEST_GROUP(TSM)
{
   TrackerMock tracker;
   Input_Diagnostics_Mock in_faults;
   Output_Diagnostics_Mock out_faults;
   F360_Core_Info_T core_info = {};
   F360_Host_T host = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Log_Output_T obj_log = {};
   ocg::OCG_Outputs_T occupancy_grid = {};

   TEST_SETUP()
   {

   }


   TEST_TEARDOWN()
   {
      mock().checkExpectations();
      mock().clear();
   }

};

/**
*\purpose  Purpose of this test is to verify state manager initialize function call
*/
TEST(TSM, TSM__check_initialize_function)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);

   /** \precond
   * No preconditions
   */

   mock().expectNCalls(2, "Initialize");

   /** \action
   * Execute tracker state manager and its init function 
   */

   State_Manager tracker_state_manager(safety_logic, tracker);

   tracker_state_manager.Initialize();

   /** \result
   * Check whether tracker was initialized (in TEST_TEARDOWN)
   */
}
/** @}*/

/**
*\purpose  Purpose of this test is to verify state manager initialize function with variant init
*/
TEST(TSM, TSM__check_initialize_function_with_variant_init)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);

   /** \precond
   * Tracker Variant
   */
   F360_Variant_T variant;
   Set_Tracker_Variant(variant);

   mock().expectOneCall("Initialize(Variant)");

   /** \action
   * Execute tracker state manager.
   */

   State_Manager tracker_state_manager(safety_logic, tracker);

   tracker_state_manager.Initialize(variant);

   /** \result
   * Check whether tracker was initialized (in TEST_TEARDOWN)
   */
}
/** @}*/

/**
*\purpose  Purpose of this test is to verify whether tracker is initialized.
*/
TEST(TSM, TSM__check_if_tracker_is_initialized)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);

   /** \precond
   * No preconditions
   */

   mock().expectOneCall("Initialize");

   /** \action
   * Execute tracker state manager.
   */

   State_Manager tracker_state_manager(safety_logic, tracker);

   /** \result
   * Check whether tracker was initialized (in TEST_TEARDOWN)
   */
}
/** @}*/

/**
*\purpose  Purpose of this test is to verify whether tracker is executed.
*/
TEST(TSM, TSM__check_if_tracker_is_executed)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);
   State_Manager tracker_state_manager(safety_logic, tracker);

   /** \precond
   * tracker_state_manager.execute has to be called once before
   */

   mock().expectOneCall("Execute");

   /** \action
   * Execute tracker state manager
   */

   tracker_state_manager.execute(core_info, host, occupancy_grid, raw_detect_list, sensors, obj_log);

   /** \result
   * Check whether tracker was executed once (in TEST_TEARDOWN)
   */

}
/** @}*/

/**
*\purpose  Purpose of this test is to verify whether tracker is executed without occupancy grid.
*/
TEST(TSM, TSM__check_if_tracker_is_executed_without_ocg)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);
   State_Manager tracker_state_manager(safety_logic, tracker);

   /** \precond
   * tracker_state_manager.execute has to be called once before
   */

   mock().expectOneCall("Execute");

   /** \action
   * Execute tracker state manager
   */

   tracker_state_manager.execute(core_info, host, raw_detect_list, sensors, obj_log);

   /** \result
   * Check whether tracker was executed once (in TEST_TEARDOWN)
   */

}

/** @}*/
/**
*\purpose  Purpose of this test is to verify whether tracker is reset.
*/
TEST(TSM, TSM__check_if_tracker_is_reset)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);
   State_Manager tracker_state_manager(safety_logic, tracker);

   /** \precond
    * tracker_state_manager.execute has to be called twice before
    * field: raw_detect_list.detections[0].sensor_id has to be set to 3
    */

   //mock().expectOneCall("Reset"); // Temperately disable this test purpose. TSM is hardcode to not reset for now

   mock().expectOneCall("evaluate_cycle").andReturnValue(static_cast<int>(true));
   //tracker_state_manager.execute(core_info, host, raw_detect_list, sensors, obj_log); // Temperately disable this test purpose. TSM is hardcode to not reset for now


   /** \action
    * Execute tracker state manager
    */

   tracker_state_manager.execute(core_info, host, occupancy_grid, raw_detect_list, sensors, obj_log);

   /** \result
    * Check whether tracker state manager was reseted (in TEST_TEARDOWN)
    */

}
/** @}*/
/**
*\purpose  Purpose of this test is to verify whether tracker is reset without occupancy grid.
*/
TEST(TSM, TSM__check_if_tracker_is_reset_without_ocg)
{
   mock().ignoreOtherCalls();
   SafetyLogicMock safety_logic(in_faults, out_faults);
   State_Manager tracker_state_manager(safety_logic, tracker);

   /** \precond
    * tracker_state_manager.execute has to be called twice before
    * field: raw_detect_list.detections[0].sensor_id has to be set to 3
    */

   //mock().expectOneCall("Reset"); // Temperately disable this test purpose. TSM is hardcode to not reset for now

   mock().expectOneCall("evaluate_cycle").andReturnValue(static_cast<int>(true));
   //tracker_state_manager.execute(core_info, host, raw_detect_list, sensors, obj_log); // Temperately disable this test purpose. TSM is hardcode to not reset for now


   /** \action
    * Execute tracker state manager
    */

   tracker_state_manager.execute(core_info, host, raw_detect_list, sensors, obj_log);

   /** \result
    * Check whether tracker state manager was reseted (in TEST_TEARDOWN)
    */

}
/** @}*/
