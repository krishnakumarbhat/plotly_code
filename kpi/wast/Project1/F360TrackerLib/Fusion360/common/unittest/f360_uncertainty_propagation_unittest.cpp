/** \file
   This file contains unit tests for all functions contained in f360_uncertainty_propagation
*/

#include "f360_uncertainty_propagation.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <iostream>

//#include "headerfile_needed.h"

using namespace f360_variant_A;
//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

/** \defgroup  f360_uncertainty_propagation
 *  @{
 */

/** \brief
*  Add brief description of test group
**/
TEST_GROUP(f360_uncertainty_propagation)
{
   /** \setup
   * Set up thresholds for comparing if floats are equal
   **/
   const float TEST_PASS_TH = 1e-5F;

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
*\purpose  Purpose of test is to verify that function Test_Constant_Uncertainty_Propagation_2d
*          works as intended. Intended functionality is that function takes a constant and a
*          2D covariance matrix as input and multiplies each element of the matrix with the
*          squared constant.
*\req    NA
*/
TEST(f360_uncertainty_propagation, Test_Constant_Uncertainty_Propagation_2d)
{
   /** \step{1}
    * Check that functions returns a zero matrix when it gets a zero matrix as input
    **/

   /** \precond
    * Set up a non-zero constant and a zero input matrix and an expected output matrix
   **/
   float constant_1 = 5.647F;
   float mat_1[2][2];
   mat_1[0][0] = 0.0F;
   mat_1[0][1] = 0.0F;
   mat_1[1][0] = 0.0F;
   mat_1[1][1] = 0.0F;
   float exp_output_mat_1[2][2];
   exp_output_mat_1[0][0] = 0.0F;
   exp_output_mat_1[0][1] = 0.0F;
   exp_output_mat_1[1][0] = 0.0F;
   exp_output_mat_1[1][1] = 0.0F;

   /** \action
   *Call the function to generate the output matrix
   **/
   Constant_Uncertainty_Propagation_2d(constant_1, mat_1);

   /** \result
   * Check so that all elements of matrix is equal to the expected result
   **/
   CHECK_EQUAL_TEXT(exp_output_mat_1[0][0], mat_1[0][0], "Output matrix is non-zero despite input matrix being zero. Error on element with index 0,0");
   CHECK_EQUAL_TEXT(exp_output_mat_1[0][1], mat_1[0][1], "Output matrix is non-zero despite input matrix being zero. Error on element with index 0,1");
   CHECK_EQUAL_TEXT(exp_output_mat_1[1][0], mat_1[1][0], "Output matrix is non-zero despite input matrix being zero. Error on element with index 1,0");
   CHECK_EQUAL_TEXT(exp_output_mat_1[1][1], mat_1[1][1], "Output matrix is non-zero despite input matrix being zero. Error on element with index 1,1");


   /** \step{2}
    * Check that functions returns a zero matrix when it gets a zero constant as input
    **/

   /** \precond
    * Set up a zero constant and a non-zero input matrix and an expected output matrix
   **/
   float constant_2 = 0.0F;
   float mat_2[2][2];
   mat_2[0][0] = 41.3376F;
   mat_2[0][1] = -18.8279F;
   mat_2[1][0] = -18.8279F;
   mat_2[1][1] = 13.2359F;
   float exp_output_mat_2[2][2];
   exp_output_mat_2[0][0] = 0.0F;
   exp_output_mat_2[0][1] = 0.0F;
   exp_output_mat_2[1][0] = 0.0F;
   exp_output_mat_2[1][1] = 0.0F;

   /** \action
   *Call the function to generate the output matrix
   **/
   Constant_Uncertainty_Propagation_2d(constant_2, mat_2);

   /** \result
   * Check so that all elements of matrix is equal to the expected result
   **/
   CHECK_EQUAL_TEXT(exp_output_mat_2[0][0], mat_2[0][0], "Output matrix is non-zero despite input constant being zero. Error on element with index 0,0");
   CHECK_EQUAL_TEXT(exp_output_mat_2[0][1], mat_2[0][1], "Output matrix is non-zero despite input constant being zero. Error on element with index 0,1");
   CHECK_EQUAL_TEXT(exp_output_mat_2[1][0], mat_2[1][0], "Output matrix is non-zero despite input constant being zero. Error on element with index 1,0");
   CHECK_EQUAL_TEXT(exp_output_mat_2[1][1], mat_2[1][1], "Output matrix is non-zero despite input constant being zero. Error on element with index 1,1");


   /** \step{3}
    * Check that functions returns correct matrix when it gets a positive constant and a non-zero matrix
    **/

   /** \precond
    * Set up a zero constant and a non-zero input matrix and an expected output matrix
   **/
   float constant_3 = 0.7634F;
   float mat_3[2][2];
   mat_3[0][0] = 7.0943F;
   mat_3[0][1] = 10.1700F;
   mat_3[1][0] = 10.1700F;
   mat_3[1][1] = 21.0881F;
   float exp_output_mat_3[2][2];
   exp_output_mat_3[0][0] = 4.134413032F;
   exp_output_mat_3[0][1] = 5.926868125F;
   exp_output_mat_3[1][0] = 5.926868125F;
   exp_output_mat_3[1][1] = 12.289713639F;

   /** \action
   *Call the function to generate the output matrix
   **/
   Constant_Uncertainty_Propagation_2d(constant_3, mat_3);

   /** \result
   * Check so that all elements of matrix is equal to the expected result
   **/
   DOUBLES_EQUAL_TEXT(exp_output_mat_3[0][0], mat_3[0][0], TEST_PASS_TH, "Element 0,0 of output matrix has unexpected value when input constant is positive.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_3[0][1], mat_3[0][1], TEST_PASS_TH, "Element 0,1 of output matrix has unexpected value when input constant is positive.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_3[1][0], mat_3[1][0], TEST_PASS_TH, "Element 1,0 of output matrix has unexpected value when input constant is positive.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_3[1][1], mat_3[1][1], TEST_PASS_TH, "Element 1,1 of output matrix has unexpected value when input constant is positive.");


   /** \step{4}
    * Check that functions returns correct matrix when it gets a negative constant and a non-zero matrix
    **/

   /** \precond
    * Set up a zero constant and a non-zero input matrix and an expected output matrix
   **/
   float constant_4 = -2.1327F;
   float mat_4[2][2];
   mat_4[0][0] = 19.6113F;
   mat_4[0][1] = -16.67236F;
   mat_4[1][0] = -16.67236F;
   mat_4[1][1] = 32.7738F;
   float exp_output_mat_4[2][2];
   exp_output_mat_4[0][0] = 89.200219108F;
   exp_output_mat_4[0][1] = -75.832717110F;
   exp_output_mat_4[1][0] = -75.832717110F;
   exp_output_mat_4[1][1] = 149.068656388F;

   /** \action
   *Call the function to generate the output matrix
   **/
   Constant_Uncertainty_Propagation_2d(constant_4, mat_4);

   /** \result
   * Check so that all elements of matrix is equal to the expected result
   **/
   DOUBLES_EQUAL_TEXT(exp_output_mat_4[0][0], mat_4[0][0], TEST_PASS_TH, "Element 0,0 of output matrix has unexpected value when input constant is negative.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_4[0][1], mat_4[0][1], TEST_PASS_TH, "Element 0,1 of output matrix has unexpected value when input constant is negative.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_4[1][0], mat_4[1][0], TEST_PASS_TH, "Element 1,0 of output matrix has unexpected value when input constant is negative.");
   DOUBLES_EQUAL_TEXT(exp_output_mat_4[1][1], mat_4[1][1], TEST_PASS_TH, "Element 1,1 of output matrix has unexpected value when input constant is negative.");
}

/** @}*/
