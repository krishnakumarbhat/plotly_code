/** \file
 * This file contains unit tests for content of f360_sort_data_type.cpp file
 */

#include "f360_sort_data_type.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_sort_data_type
 *  @{
 */

/** \brief
 * Test group of F360_Sort_Data_T struct. Function verifies comparsion operators.
 */
TEST_GROUP(f360_sort_data_type)
{

};

/** \purpose
 * Purpose of this test is to verify whether operator < returns false if first argument is greater than second.
 * \req
 * NA.
 */
TEST(f360_sort_data_type, f360_sort_data_type__less_operator_returns_false)
{
   /** \precond
    * Set input arguments.
    */
   F360_Sort_Data_T a{};
   F360_Sort_Data_T b{};

   a.data = 10.0F;
   b.data = 5.0F;

   /** \action
    * Call tested function.
    */
   const bool result = a < b;

   /** \result
    * Check whether returned value is false.
    */
   CHECK_FALSE(result);
}

/** \purpose
* Purpose of this test is to verify whether operator < returns false if first argument is greater than second.
* \req
* NA.
*/
TEST(f360_sort_data_type, f360_sort_data_type__less_operator_returns_true)
{
   /** \precond
   * Set input arguments.
   */
   F360_Sort_Data_T a{};
   F360_Sort_Data_T b{};

   a.data = 5.0F;
   b.data = 50.0F;

   /** \action
   * Call tested function.
   */
   const bool result = a < b;

   /** \result
   * Check whether returned value is true.
   */
   CHECK_TRUE(result);
}

/** \purpose
* Purpose of this test is to verify whether operator < returns false if first argument is greater than second.
* \req
* NA.
*/
TEST(f360_sort_data_type, f360_sort_data_type__greater_operator_returns_false)
{
   /** \precond
   * Set input arguments.
   */
   F360_Sort_Data_T a{};
   F360_Sort_Data_T b{};

   a.data = 5.0F;
   b.data = 50.0F;

   /** \action
   * Call tested function.
   */
   const bool result = a > b;

   /** \result
   * Check whether returned value is false.
   */
   CHECK_FALSE(result);
}

/** \purpose
* Purpose of this test is to verify whether operator < returns false if first argument is greater than second.
* \req
* NA.
*/
TEST(f360_sort_data_type, f360_sort_data_type__greater_operator_returns_true)
{
   /** \precond
   * Set input arguments.
   */
   F360_Sort_Data_T a{};
   F360_Sort_Data_T b{};

   a.data = 50.0F;
   b.data = 5.0F;

   /** \action
   * Call tested function.
   */
   const bool result = a > b;

   /** \result
   * Check whether returned value is true.
   */
   CHECK_TRUE(result);
}
/** @}*/
