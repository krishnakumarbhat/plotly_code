/** \file
   Give a detailed description of what  this unit-test file contain.
*/
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include "f360_reuse.h"
#include "f360_nees_cfmi_structs.h"

//#include "headerfile_needed.h"

/** \defgroup  f360_nees_cfmi_reset
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  Add brief description of test group
**/
TEST_GROUP(f360_nees_cfmi_reset)
{
   /** \setup
   * Nothing to setup in this test group
   **/
   TEST_SETUP()
   {
   }

   /** \teardown
   * Nothing to teardown in this test group
   **/
   TEST_TEARDOWN()
   {
      //mock.clear();
   }

};

/**
*\purpose  Describe purpose of test
*\req    put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_reset, TestCall2Function)
{
   /** \step{1}
    *describe test step
    **/

   /** \precond
   describe preconditions
   **/

   /** \action
   *describe actions
   **/

   /** \result
   *describe test result
   **/
}

/** @}*/
