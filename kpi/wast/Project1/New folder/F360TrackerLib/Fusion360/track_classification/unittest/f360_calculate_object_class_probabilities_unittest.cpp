/** \file
   File contains unit tests for all functions used for object classification.
*/

#include "f360_calculate_object_class_probabilities.h"
#include "f360_calibrations.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include<algorithm>

//#include "headerfile_needed.h"

//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

/** \defgroup  f360_calculate_object_class_probabilities
 *  @{
 */

using namespace f360_variant_A;
/** \brief
 *  Test group for all tests for functions used for object classification.
 */
TEST_GROUP(f360_calculate_object_class_probabilities)
{
   /** \setup
   * Setting up thresholds for passing tests.
   * Setting up common variables
   */
   const float TEST_PASS_TH_LARGE = 1e-5f;
   const float TEST_PASS_TH_MID = 1e-6f;
   const float TEST_PASS_TH_SMALL = 1e-7F;

   F360_Object_Track_T object = {};
   F360_Host_T vehicle_data = {};
   F360_Tracker_Info_T tracker_info = {};

   F360_Calibrations_T cals{};

   /** \setup
   * Nothing to setup in this test group
   */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(cals);
   }

   /** \teardown
   * Nothing to teardown in this test group
   */
   TEST_TEARDOWN()
   {
      //mock.clear();
   }

};

/**
*\purpose  Testing calcAprioriProbability function for low absolute values of host speed
*\req    NA
*/
TEST(f360_calculate_object_class_probabilities, Test_calibration_settings)
{
   /** \step{1}
   * Testing that k_ad_oc_boundary_lowspeed is positive
   */

   /** \action
   * Compute if k_ad_oc_boundary_lowspeed is positive or negative
   */
   bool is_positive = cals.k_ad_oc_boundary_lowspeed > 0.0F;

   /** \result
   * Check hat k_ad_oc_boundary_lowspeed is positive
   */
   CHECK_TEXT(is_positive, "Calibration parameter  k_ad_oc_boundary_lowspeed is not positive");

   /** \step{2}
   * Testing that k_ad_oc_boundary_highspeed is positive
   */

   /** \action
   * Compute if k_ad_oc_boundary_highspeed is positive or negative
   */
   is_positive = cals.k_ad_oc_boundary_highspeed > 0.0F;

   /** \result
   * Check hat k_ad_oc_boundary_lowspeed is positive
   */
   CHECK_TEXT(is_positive, "Calibration parameter k_ad_oc_boundary_highspeed is not positive");

   /** \step{3}
   * Testing that k_ad_oc_boundary_highspeed is larger than k_ad_oc_boundary_lowspeed
   */

    /** \action
    * Compute if k_ad_oc_boundary_lowspeed is positive or negative
    */
   bool is_larger = cals.k_ad_oc_boundary_highspeed > cals.k_ad_oc_boundary_lowspeed;

   /** \result
   * Check that k_ad_oc_boundary_lowspeed is positive
   */
   CHECK_TEXT(is_larger, "Calibration parameter  k_ad_oc_boundary_higsspeed is smaller than k_ad_oc_boundary_lowspeed.");

   /** \step{4}
   * Testing that all apriory probabilities are in the range of [0 1]
   */

    /** \action
    * Compute if apriory probability parameters are in the allowed range
    */
   bool is_in_range_lowspeed_pedestrian = (cals.k_ad_oc_apriori_lowspeed_pedestrian >= 0.0F) && (cals.k_ad_oc_apriori_lowspeed_pedestrian <= 1.0F);
   bool is_in_range_lowspeed_2wheel = (cals.k_ad_oc_apriori_lowspeed_2wheel >= 0.0F) && (cals.k_ad_oc_apriori_lowspeed_2wheel <= 1.0F);
   bool is_in_range_lowspeed_car = (cals.k_ad_oc_apriori_lowspeed_car >= 0.0F) && (cals.k_ad_oc_apriori_lowspeed_car <= 1.0F);
   bool is_in_range_lowspeed_truck = (cals.k_ad_oc_apriori_lowspeed_truck >= 0.0F) && (cals.k_ad_oc_apriori_lowspeed_truck <= 1.0F);
   bool is_in_range_highspeed_pedestrian = (cals.k_ad_oc_apriori_highspeed_pedestrian >= 0.0F) && (cals.k_ad_oc_apriori_highspeed_pedestrian <= 1.0F);
   bool is_in_range_highspeed_2wheel = (cals.k_ad_oc_apriori_highspeed_2wheel >= 0.0F) && (cals.k_ad_oc_apriori_highspeed_2wheel <= 1.0F);
   bool is_in_range_highspeed_car = (cals.k_ad_oc_apriori_highspeed_car >= 0.0F) && (cals.k_ad_oc_apriori_highspeed_car <= 1.0F);
   bool is_in_range_highspeed_truck = (cals.k_ad_oc_apriori_highspeed_truck >= 0.0F) && (cals.k_ad_oc_apriori_highspeed_truck <= 1.0F);


   /** \result
   * Check that all apriory probabilities are in the allowed range
   */
   CHECK_TEXT(is_in_range_lowspeed_pedestrian, "Calibration parameter k_ad_oc_apriori_lowspeed_pedestrian is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_lowspeed_2wheel, "Calibration parameter k_ad_oc_apriori_lowspeed_2wheel is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_lowspeed_car, "Calibration parameter k_ad_oc_apriori_lowspeed_car is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_lowspeed_truck, "Calibration parameter k_ad_oc_apriori_lowspeed_truck is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_highspeed_pedestrian, "Calibration parameter k_ad_oc_apriori_highspeed_pedestrian is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_highspeed_2wheel, "Calibration parameter k_ad_oc_apriori_highspeed_2wheel is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_highspeed_car, "Calibration parameter k_ad_oc_apriori_highspeed_car is not in the range [0, 1]");
   CHECK_TEXT(is_in_range_highspeed_truck, "Calibration parameter k_ad_oc_apriori_highspeed_truck is not in the range [0, 1]");

   /** \step{5}
   * Testing that all sum of apriory probabilities for low host speed case sum up to 1
   */

   /** \action
   * Compute sum of all apriory probabilities for low host speed case
   */
   float prob_sum = cals.k_ad_oc_apriori_lowspeed_pedestrian +
      cals.k_ad_oc_apriori_lowspeed_2wheel +
      cals.k_ad_oc_apriori_lowspeed_car +
      cals.k_ad_oc_apriori_lowspeed_truck;

   /** \result
   * Check that sum of all apriory probabilities are equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum, TEST_PASS_TH_SMALL, "Calibration parameters for apriory probabilities for low host speed case do not sum up to 1.");

   /** \step{6}
   * Testing that all sum of apriory probabilities for high host speed case sum up to 1
   */

   /** \action
   * Compute sum of all apriori probabilities for high host speed case
   */
   prob_sum = cals.k_ad_oc_apriori_highspeed_pedestrian +
      cals.k_ad_oc_apriori_highspeed_2wheel +
      cals.k_ad_oc_apriori_highspeed_car +
      cals.k_ad_oc_apriori_highspeed_truck;

   /** \result
   * Check that sum of all apriory probabilities are equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum, TEST_PASS_TH_LARGE, "Calibration parameters for apriory probabilities for high host speed case do not sum up to 1.");

}


/**
*\purpose  Testing calcAprioriProbability function for low absolute values of host speed
*\req    NA
*/
TEST(f360_calculate_object_class_probabilities, Test_calcAprioriProbability_low_speed)
{
   /** \step{1}
   * Testing correct output for the case where host speed is in the range [0, calib.k_ad_oc_boundary_lowspeed]
   */

   /** \precond
   * Setting up input host speed to be small and positive.
   * Setting up the expected apriory output to be equal to calibration parameters.
   *
   */
   vehicle_data.speed = 0.5F*cals.k_ad_oc_boundary_lowspeed;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori1;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori1;
   expected_apriori1.pedestrian = cals.k_ad_oc_apriori_lowspeed_pedestrian;
   expected_apriori1.two_wheel = cals.k_ad_oc_apriori_lowspeed_2wheel;
   expected_apriori1.car = cals.k_ad_oc_apriori_lowspeed_car;
   expected_apriori1.truck = cals.k_ad_oc_apriori_lowspeed_truck;

   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori1, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori1.pedestrian, output_apriori1.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, low and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.two_wheel, output_apriori1.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, low and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.car, output_apriori1.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, low and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.truck, output_apriori1.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, low and positive host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   float prob_sum = output_apriori1.pedestrian +
      output_apriori1.two_wheel +
      output_apriori1.car +
      output_apriori1.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum, TEST_PASS_TH_SMALL, "Apriory probabilities for low and positive host speed case do not sum up to 1.");


   /** \step{2}
   * Testing correct output for the case where host speed is in the range [-calib.k_ad_oc_boundary_lowspeed, 0]
   */

   /** \precond
   * Setting up input host speed to be small and negative.
   * Setting up the expected apriory output to be equal to calibration parameters.
   *
   */
   vehicle_data.speed = -0.5F*cals.k_ad_oc_boundary_lowspeed;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori2;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori2;
   expected_apriori2.pedestrian = cals.k_ad_oc_apriori_lowspeed_pedestrian;
   expected_apriori2.two_wheel = cals.k_ad_oc_apriori_lowspeed_2wheel;
   expected_apriori2.car = cals.k_ad_oc_apriori_lowspeed_car;
   expected_apriori2.truck = cals.k_ad_oc_apriori_lowspeed_truck;
   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori2, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori2.pedestrian, output_apriori2.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, low and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.two_wheel, output_apriori2.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, low and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.car, output_apriori2.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, low and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.truck, output_apriori2.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, low and negative host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   prob_sum = output_apriori2.pedestrian +
      output_apriori2.two_wheel +
      output_apriori2.car +
      output_apriori2.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum, TEST_PASS_TH_SMALL, "Apriory probabilities for low and negative host speed case do not sum up to 1.");
}

/**
*\purpose  Testing calcAprioriProbability function for high absolute values of host speed
*\req    NA
*/
TEST(f360_calculate_object_class_probabilities, Test_calcAprioriProbability_high_speed)
{
   /** \step{1}
   * Testing correct output for the case where host speed is in the range [calib.k_ad_oc_boundary_highspeed, inf]
   */

   /** \precond
   * Setting up input host speed to be large and positive.
   * Setting up the expected apriory output to be equal to calibration parameters.
   *
   */
   vehicle_data.speed = 1.2F*cals.k_ad_oc_boundary_highspeed;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori1;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori1;
   expected_apriori1.pedestrian = cals.k_ad_oc_apriori_highspeed_pedestrian;
   expected_apriori1.two_wheel = cals.k_ad_oc_apriori_highspeed_2wheel;
   expected_apriori1.car = cals.k_ad_oc_apriori_highspeed_car;
   expected_apriori1.truck = cals.k_ad_oc_apriori_highspeed_truck;

   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori1, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori1.pedestrian, output_apriori1.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, high and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.two_wheel, output_apriori1.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, high and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.car, output_apriori1.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, high and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.truck, output_apriori1.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, high and positive host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   float prob_sum1 = output_apriori1.pedestrian +
      output_apriori1.two_wheel +
      output_apriori1.car +
      output_apriori1.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum1, TEST_PASS_TH_LARGE, "Apriory probabilities for high and positive host speed case do not sum up to 1.");

   /** \step{2}
   * Testing correct output for the case where host speed is in the range [-inf, -calib.k_ad_oc_boundary_highspeed]
   */

   /** \precond
   * Setting up input host speed to be large and negative.
   * Setting up the expected apriory output to be equal to calibration parameters.
   *
   */
   vehicle_data.speed = -1.2F*cals.k_ad_oc_boundary_highspeed;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori2;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori2;
   expected_apriori2.pedestrian = cals.k_ad_oc_apriori_highspeed_pedestrian;
   expected_apriori2.two_wheel = cals.k_ad_oc_apriori_highspeed_2wheel;
   expected_apriori2.car = cals.k_ad_oc_apriori_highspeed_car;
   expected_apriori2.truck = cals.k_ad_oc_apriori_highspeed_truck;

   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori2, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori2.pedestrian, output_apriori2.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, high and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.two_wheel, output_apriori2.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, high and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.car, output_apriori2.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, high and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.truck, output_apriori2.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, high and negative host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   float prob_sum2 = output_apriori2.pedestrian +
      output_apriori2.two_wheel +
      output_apriori2.car +
      output_apriori2.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum2, TEST_PASS_TH_LARGE, "Apriory probabilities for high and negative host speed case do not sum up to 1.");
}

/**
*\purpose  Testing calcAprioriProbability function for medium absolute values of host speed
*\req    NA
*/
TEST(f360_calculate_object_class_probabilities, Test_calcAprioriProbability_mid_speed)
{
   /** \step{1}
   * Testing correct output for the case where host speed is in the range [calib.k_ad_oc_boundary_lowspeed, calib.k_ad_oc_boundary_highspeed] (positive host speed)
   */

   /** \precond
   * Setting up input host speed to be moderately large and positive.
   * Setting up the expected apriory output to be equal to a linear mixture of calibration parameters.
   *
   */
   float interval_length = cals.k_ad_oc_boundary_highspeed - cals.k_ad_oc_boundary_lowspeed;
   vehicle_data.speed = cals.k_ad_oc_boundary_highspeed - 0.3F*interval_length;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori1;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori1;
   expected_apriori1.pedestrian = cals.k_ad_oc_apriori_highspeed_pedestrian - 0.3F*(cals.k_ad_oc_apriori_highspeed_pedestrian - cals.k_ad_oc_apriori_lowspeed_pedestrian);
   expected_apriori1.two_wheel = cals.k_ad_oc_apriori_highspeed_2wheel - 0.3F*(cals.k_ad_oc_apriori_highspeed_2wheel - cals.k_ad_oc_apriori_lowspeed_2wheel);
   expected_apriori1.car = cals.k_ad_oc_apriori_highspeed_car - 0.3F*(cals.k_ad_oc_apriori_highspeed_car - cals.k_ad_oc_apriori_lowspeed_car);
   expected_apriori1.truck = cals.k_ad_oc_apriori_highspeed_truck - 0.3F*(cals.k_ad_oc_apriori_highspeed_truck - cals.k_ad_oc_apriori_lowspeed_truck);

   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori1, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori1.pedestrian, output_apriori1.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, moderately and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.two_wheel, output_apriori1.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, moderately and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.car, output_apriori1.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, moderately high and positive host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori1.truck, output_apriori1.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, moderately high and positive host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   float prob_sum1 = output_apriori1.pedestrian +
      output_apriori1.two_wheel +
      output_apriori1.car +
      output_apriori1.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum1, TEST_PASS_TH_SMALL, "Apriory probabilities for moderately high and positive host speed case do not sum up to 1.");

   /** \step{2}
   * Testing correct output for the case where host speed is in the range [-calib.k_ad_oc_boundary_highspeed, -calib.k_ad_oc_boundary_lowspeed] (negative host speed)
   */

   /** \precond
   * Setting up input host speed to be moderately large and negative.
   * Setting up the expected apriory output to be equal to a linear mixture of calibration parameters.
   *
   */
   vehicle_data.speed = -cals.k_ad_oc_boundary_highspeed + 0.3F*interval_length;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T output_apriori2;
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T expected_apriori2;
   expected_apriori2.pedestrian = cals.k_ad_oc_apriori_highspeed_pedestrian - 0.3F*(cals.k_ad_oc_apriori_highspeed_pedestrian - cals.k_ad_oc_apriori_lowspeed_pedestrian);
   expected_apriori2.two_wheel = cals.k_ad_oc_apriori_highspeed_2wheel - 0.3F*(cals.k_ad_oc_apriori_highspeed_2wheel - cals.k_ad_oc_apriori_lowspeed_2wheel);
   expected_apriori2.car = cals.k_ad_oc_apriori_highspeed_car - 0.3F*(cals.k_ad_oc_apriori_highspeed_car - cals.k_ad_oc_apriori_lowspeed_car);
   expected_apriori2.truck = cals.k_ad_oc_apriori_highspeed_truck - 0.3F*(cals.k_ad_oc_apriori_highspeed_truck - cals.k_ad_oc_apriori_lowspeed_truck);

   /** \action
   * Call the function calcAprioriProbability
   */
   calcAprioriProbability(output_apriori2, cals, vehicle_data);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_apriori2.pedestrian, output_apriori2.pedestrian, TEST_PASS_TH_SMALL, "Test for pedestrian apriory probability, moderately and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.two_wheel, output_apriori2.two_wheel, TEST_PASS_TH_SMALL, "Test for two wheeler apriory probability, moderately and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.car, output_apriori2.car, TEST_PASS_TH_SMALL, "Test for car apriory probability, moderately high and negative host speed.");
   DOUBLES_EQUAL_TEXT(expected_apriori2.truck, output_apriori2.truck, TEST_PASS_TH_SMALL, "Test for truck apriory probability, moderately high and negative host speed.");

   /** \action
   * Compute the sum of the outputted apriory probabilities
   */
   float prob_sum2 = output_apriori2.pedestrian +
      output_apriori2.two_wheel +
      output_apriori2.car +
      output_apriori2.truck;

   /** \result
   * Check that the sum of the outputted apriory probabilities is equal to 1
   */
   DOUBLES_EQUAL_TEXT(1.0F, prob_sum2, TEST_PASS_TH_SMALL, "Apriory probabilities for moderately high and negative host speed case do not sum up to 1.");
}

/**
*\purpose  Testing evaluateNormalDistribution function for values above and below both positive and negative mean
*\req    NA
*/
TEST(f360_calculate_object_class_probabilities, Test_evaluateNormalDistribution)
{
   /** \step{1}
   * Testing that function outputs correct value for value that is above a positive mean
   */

   /** \precond
   * Setting up mean and varaince parameters of a normal distributed stochastic variable.
   * Setting up a sampled value from the distribution
   * Setting up the expected output.
   *
   */
   float mean1 = 13.5F;
   float std1 = 2.3F;
   float value1 = 16.4F;
   float output1;
   float expected_output1 = 0.078336338F;
   /** \action
   * Call the function evaluateNormalDistribution
   */
   output1 = evaluateNormalDistribution(value1, mean1, std1);

   /** \result
   * Check that the output from evaluateNormalDistribution corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output1, output1, TEST_PASS_TH_MID, "Error when evaluating a Normal distribution with positive mean and value to evaluate is above the mean.");

   /** \step{2}
   * Testing that function outputs correct value for value that is below a positive mean
   */

   /** \precond
   * Setting up mean and varaince parameters of a normal distributed stochastic variable.
   * Setting up a sampled value from the distribution
   * Setting up the expected output.
   *
   */
   float mean2 = 2.1F;
   float std2 = 10.3F;
   float value2 = -3.3F;
   float output2;
   float expected_output2 = 0.033758841F;
   /** \action
   * Call the function evaluateNormalDistribution
   */
   output2 = evaluateNormalDistribution(value2, mean2, std2);

   /** \result
   * Check that the output from evaluateNormalDistribution corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output2, output2, TEST_PASS_TH_MID, "Error when evaluating a Normal distribution with positive mean and value to evaluate is below the mean.");

   /** \step{3}
   * Testing that function outputs correct value for value that is above a negative mean
   */

   /** \precond
   * Setting up mean and varaince parameters of a normal distributed stochastic variable.
   * Setting up a sampled value from the distribution
   * Setting up the expected output.
   *
   */
   float mean3 = -30.7F;
   float std3 = 1.2F;
   float value3 = -30.0F;
   float output3;
   float expected_output3 = 0.280439018F;
   /** \action
   * Call the function evaluateNormalDistribution
   */
   output3 = evaluateNormalDistribution(value3, mean3, std3);

   /** \result
   * Check that the output from evaluateNormalDistribution corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output3, output3, TEST_PASS_TH_MID, "Error when evaluating a Normal distribution with negative mean and value to evaluate is above the mean.");

   /** \step{4}
   * Testing that function outputs correct value for value that is below a negative mean
   */

   /** \precond
   * Setting up mean and varaince parameters of a normal distributed stochastic variable.
   * Setting up a sampled value from the distribution
   * Setting up the expected output.
   *
   */
   float mean4 = -8.9F;
   float std4 = 3.9F;
   float value4 = -9.9F;
   float output4;
   float expected_output4 = 0.098984877F;

   /** \action
   * Call the function evaluateNormalDistribution
   */
   output4 = evaluateNormalDistribution(value4, mean4, std4);

   /** \result
   * Check that the output from evaluateNormalDistribution corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output4, output4, TEST_PASS_TH_MID, "Error when evaluating a Normal distribution with negative mean and value to evaluate is below the mean.");

   /** \step{5}
   * Testing that function outputs expected value when input standard deviation is negative (i.e. input is invalid)
   */

   /** \precond
   * Setting up mean and varaince parameters of a normal distributed stochastic variable.
   * Setting up a sampled value from the distribution
   * Setting up the expected output. When input standard deviation is negative we expect function to use std = 1.0 instead.
   *
   */
   float mean5 = 23.5F;
   float std5 = -4.3F;
   float value5 = 25.1F;
   float output5;
   float expected_output5 = 0.110920834F;
   /** \action
   * Call the function evaluateNormalDistribution
   */
   output5 = evaluateNormalDistribution(value5, mean5, std5);

   /** \result
   * Check that the output from evaluateNormalDistribution corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output5, output5, TEST_PASS_TH_MID, "Error when evaluating a Normal distribution with a negative standard deviation.");
}

/**
*\purpose  Testing calcProbability_Criteria_Bayes for soome different object speeds, lengths and widths
*\req  NA
*/
TEST(f360_calculate_object_class_probabilities, Test_calcProbability_Criteria_Bayes)
{
   /** \step{1}
   * Testing correct output for object length = 5.0m, object width = 1.8m, object speed = 35.3m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up calibration settings
   * Setting up apriory probabilities
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Calibration settings
   F360_Calibrations_T cals_known;
   Initialize_Tracker_Calibrations(cals_known);
   cals_known.k_ad_oc_mean_length_pedestrian = 0.8F;
   cals_known.k_ad_oc_standard_deviation_length_pedestrian = 0.4F;
   cals_known.k_ad_oc_mean_width_pedestrian = 0.8F;
   cals_known.k_ad_oc_standard_deviation_width_pedestrian = 0.3F;
   cals_known.k_ad_oc_mean_speed_pedestrian = 1.4F;
   cals_known.k_ad_oc_standard_deviation_speed_pedestrian = 1.2F;
   cals_known.k_ad_oc_mean_length_2wheel = 1.0F;
   cals_known.k_ad_oc_standard_deviation_length_2wheel = 0.5F;
   cals_known.k_ad_oc_mean_width_2wheel = 1.15F;
   cals_known.k_ad_oc_standard_deviation_width_2wheel = 0.2F;
   cals_known.k_ad_oc_mean_speed_2wheel = 25.0F;
   cals_known.k_ad_oc_standard_deviation_speed_2wheel = 14.0F;
   cals_known.k_ad_oc_mean_length_car = 5.3F;
   cals_known.k_ad_oc_standard_deviation_length_car = 1.9F;
   cals_known.k_ad_oc_mean_width_car = 2.0F;
   cals_known.k_ad_oc_standard_deviation_width_car = 0.3F;
   cals_known.k_ad_oc_mean_speed_car = 25.0F;
   cals_known.k_ad_oc_standard_deviation_speed_car = 14.0F;
   cals_known.k_ad_oc_mean_length_truck = 18.0F;
   cals_known.k_ad_oc_standard_deviation_length_truck = 4.0F;
   cals_known.k_ad_oc_mean_width_truck = 2.2F;
   cals_known.k_ad_oc_standard_deviation_width_truck = 0.3F;
   cals_known.k_ad_oc_mean_speed_truck = 22.0F;
   cals_known.k_ad_oc_standard_deviation_speed_truck = 11.0F;
   // Apriory probability settings
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T apriory_probs;
   apriory_probs.pedestrian = 0.22F;
   apriory_probs.two_wheel = 0.19F;
   apriory_probs.car = 0.31F;
   apriory_probs.truck = 0.28F;
   // Object properties
   object.speed = 35.3F;
   object.bbox.Set_Length(5.0F);
   object.bbox.Set_Width(1.8F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs1;
   output_probs1.probability_pedestrian = 0.26F;
   output_probs1.probability_2wheel = 0.24F;
   output_probs1.probability_car = 0.27F;
   output_probs1.probability_truck = 0.20F;
   output_probs1.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs1;
   expected_output_probs1.probability_pedestrian = 0.0000000F;
   expected_output_probs1.probability_2wheel = 0.0000000F;
   expected_output_probs1.probability_car = 0.9990897F;
   expected_output_probs1.probability_truck = 0.0009103F;
   expected_output_probs1.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs1, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs1.probability_pedestrian, output_probs1.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 5m, width 1.8m and speed 35.3m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs1.probability_2wheel, output_probs1.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 5m, width 1.8m and speed 35.3m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs1.probability_car, output_probs1.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 5m, width 1.8m and speed 35.3m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs1.probability_truck, output_probs1.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 5m, width 1.8m and speed 35.3m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs1.probability_unknown, output_probs1.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 5m, width 1.8m and speed 35.3m/s.");

   /** \step{2}
   * Testing correct output for object length = 5.0m, object width = 1.8m, object speed = 1.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 1.0F;
   object.bbox.Set_Length(5.0F);
   object.bbox.Set_Width(1.8F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs2;
   output_probs2.probability_pedestrian = 0.26F;
   output_probs2.probability_2wheel = 0.24F;
   output_probs2.probability_car = 0.27F;
   output_probs2.probability_truck = 0.20F;
   output_probs2.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs2;
   expected_output_probs2.probability_pedestrian = 0.0000000F;
   expected_output_probs2.probability_2wheel = 0.0000000F;
   expected_output_probs2.probability_car = 0.9989866F;
   expected_output_probs2.probability_truck = 0.0010134F;
   expected_output_probs2.probability_unknown = 0.0000000F;


   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs2, object, cals_known);

   /** \result
   * Check that the output apriory probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs2.probability_pedestrian, output_probs2.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 5m, width 1.8m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs2.probability_2wheel, output_probs2.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 5m, width 1.8m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs2.probability_car, output_probs2.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 5m, width 1.8m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs2.probability_truck, output_probs2.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 5m, width 1.8m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs2.probability_unknown, output_probs2.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 5m, width 1.8m and speed 1.0m/s.");

   /** \step{3}
   * Testing correct output for object length = 17.0m, object width = 2.1m, object speed = 20.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 20.0F;
   object.bbox.Set_Length(17.0F);
   object.bbox.Set_Width(2.1F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs3;
   output_probs3.probability_pedestrian = 0.26F;
   output_probs3.probability_2wheel = 0.24F;
   output_probs3.probability_car = 0.27F;
   output_probs3.probability_truck = 0.20F;
   output_probs3.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs3;
   expected_output_probs3.probability_pedestrian = 0.0000000F;
   expected_output_probs3.probability_2wheel = 0.0000000F;
   expected_output_probs3.probability_car = 0.0000000F;
   expected_output_probs3.probability_truck = 1.0000000F;
   expected_output_probs3.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs3, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs3.probability_pedestrian, output_probs3.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 17m, width 2.1m and speed 20.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs3.probability_2wheel, output_probs3.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 17m, width 2.1m and speed 20.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs3.probability_car, output_probs3.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 17m, width 2.1m and speed 20.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs3.probability_truck, output_probs3.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 17m, width 2.1m and speed 20.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs3.probability_unknown, output_probs3.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 17m, width 2.1m and speed 20.0m/s.");

   /** \step{4}
   * Testing correct output for object length = 17.0m, object width = 2.1m, object speed = 1.5m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 1.5F;
   object.bbox.Set_Length(17.0F);
   object.bbox.Set_Width(2.1F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs4;
   output_probs4.probability_pedestrian = 0.26F;
   output_probs4.probability_2wheel = 0.24F;
   output_probs4.probability_car = 0.27F;
   output_probs4.probability_truck = 0.20F;
   output_probs4.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs4;
   expected_output_probs4.probability_pedestrian = 0.0000000F;
   expected_output_probs4.probability_2wheel = 0.0000000F;
   expected_output_probs4.probability_car = 0.0000000F;
   expected_output_probs4.probability_truck = 1.0000000F;
   expected_output_probs4.probability_unknown = 0.0000000F;


   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs4, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs4.probability_pedestrian, output_probs4.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 17m, width 2.1m and speed 1.5m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs4.probability_2wheel, output_probs4.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 17m, width 2.1m and speed 1.5m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs4.probability_car, output_probs4.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 17m, width 1.8m and speed 1.5m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs4.probability_truck, output_probs4.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 17m, width 2.1m and speed 1.5m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs4.probability_unknown, output_probs4.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 17m, width 2.1m and speed 1.5m/s.");

   /** \step{5}
   * Testing correct output for object length = 1.0m, object width = 1.0m, object speed = 25.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 25.0F;
   object.bbox.Set_Length(1.0F);
   object.bbox.Set_Width(1.0F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs5;
   output_probs5.probability_pedestrian = 0.26F;
   output_probs5.probability_2wheel = 0.24F;
   output_probs5.probability_car = 0.27F;
   output_probs5.probability_truck = 0.20F;
   output_probs5.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs5;
   expected_output_probs5.probability_pedestrian = 0.0000000F;
   expected_output_probs5.probability_2wheel = 0.9998868F;
   expected_output_probs5.probability_car = 0.0001132F;
   expected_output_probs5.probability_truck = 0.0000000F;
   expected_output_probs5.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs5, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs5.probability_pedestrian, output_probs5.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 1m, width 1m and speed 25.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs5.probability_2wheel, output_probs5.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELER probability for object with length 1m, width 1m and speed 25.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs5.probability_car, output_probs5.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 1m, width 1m and speed 25.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs5.probability_truck, output_probs5.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 1m, width 1m and speed 25.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs5.probability_unknown, output_probs5.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 1m, width 1m and speed 25.0m/s.");

   /** \step{6}
   * Testing correct output for object length = 1.0m, object width = 1.0m, object speed = 1.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 1.0F;
   object.bbox.Set_Length(1.0F);
   object.bbox.Set_Width(1.0F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs6;
   output_probs6.probability_pedestrian = 0.26F;
   output_probs6.probability_2wheel = 0.24F;
   output_probs6.probability_car = 0.27F;
   output_probs6.probability_truck = 0.20F;
   output_probs6.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs6;
   expected_output_probs6.probability_pedestrian = 0.9774401F;
   expected_output_probs6.probability_2wheel = 0.0225573F;
   expected_output_probs6.probability_car = 0.0000000F;
   expected_output_probs6.probability_truck = 0.0000000F;
   expected_output_probs6.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs6, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs6.probability_pedestrian, output_probs6.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 1m, width 1m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs6.probability_2wheel, output_probs6.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 1m, width 1m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs6.probability_car, output_probs6.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 1m, width 1m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs6.probability_truck, output_probs6.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 1m, width 1m and speed 1.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs6.probability_unknown, output_probs6.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 1m, width 1m and speed 1.0m/s.");

   /** \step{7}
   * Testing correct output for object length = 3.0m, object width = 1.2m, object speed = 8.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 8.0F;
   object.bbox.Set_Length(3.0F);
   object.bbox.Set_Width(1.2F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs7;
   output_probs7.probability_pedestrian = 0.26F;
   output_probs7.probability_2wheel = 0.24F;
   output_probs7.probability_car = 0.27F;
   output_probs7.probability_truck = 0.20F;
   output_probs7.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs7;
   expected_output_probs7.probability_pedestrian = 0.0000000F;
   expected_output_probs7.probability_2wheel = 0.0764057F;
   expected_output_probs7.probability_car = 0.9234776F;
   expected_output_probs7.probability_truck = 0.0001167F;
   expected_output_probs7.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs7, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs7.probability_pedestrian, output_probs7.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 3.0m, width 1.2m and speed 8.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs7.probability_2wheel, output_probs7.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 3.0m, width 1.2m and speed 8.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs7.probability_car, output_probs7.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 3.0m, width 1.2m and speed 8.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs7.probability_truck, output_probs7.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 3.0m, width 1.2m and speed 8.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs7.probability_unknown, output_probs7.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 3.5m, width 1.2m and speed 8.0m/s.");

   /** \step{8}
   * Testing correct output for object length = 10.0m, object width = 2.0m, object speed = 15.0m/s and a given known set of calibrations
   */

   /** \precond
   * Setting up object width length and speed
   * Setting up the expected output
   */
   // Object properties
   object.speed = 15.0F;
   object.bbox.Set_Length(10.0F);
   object.bbox.Set_Width(2.0F);
   // Output
   OBJECT_CLASS_PROBABILITY_T output_probs8;
   output_probs8.probability_pedestrian = 0.26F;
   output_probs8.probability_2wheel = 0.24F;
   output_probs8.probability_car = 0.27F;
   output_probs8.probability_truck = 0.20F;
   output_probs8.probability_unknown = 0.03F;
   OBJECT_CLASS_PROBABILITY_T expected_output_probs8;
   expected_output_probs8.probability_pedestrian = 0.0000000F;
   expected_output_probs8.probability_2wheel = 0.0000000F;
   expected_output_probs8.probability_car = 0.4292538F;
   expected_output_probs8.probability_truck = 0.5707462F;
   expected_output_probs8.probability_unknown = 0.0000000F;

   /** \action
   * Call the function calcProbability_Criteria_Bayes
   */
   calcProbability_Criteria_Bayes(apriory_probs, output_probs8, object, cals_known);

   /** \result
   * Check that the output probabilities corresponds to the correct values
   */
   DOUBLES_EQUAL_TEXT(expected_output_probs8.probability_pedestrian, output_probs8.probability_pedestrian, TEST_PASS_TH_LARGE, "Unexpected PEDESTRIAN probability for object with length 10m, width 2m and speed 15.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs8.probability_2wheel, output_probs8.probability_2wheel, TEST_PASS_TH_LARGE, "Unexpected TWO_WHEELR probability for object with length 10m, width 2m and speed 15.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs8.probability_car, output_probs8.probability_car, TEST_PASS_TH_LARGE, "Unexpected CAR probability for object with length 10m, width 2m and speed 15.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs8.probability_truck, output_probs8.probability_truck, TEST_PASS_TH_LARGE, "Unexpected TRUCK probability for object with length 10m, width 2m and speed 15.0m/s.");
   DOUBLES_EQUAL_TEXT(expected_output_probs8.probability_unknown, output_probs8.probability_unknown, TEST_PASS_TH_LARGE, "Unexpected UNKNOWN probability for object with length 10m, width 2m and speed 15.0m/s.");
}


/** \purpose
* Purpose of this test is to verify whether probability that object class is undetermined is set on 1.0
*  in case when initial probability was greater than 1.0.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, calcUndetProb__if_probability_undet_is_decreasing_in_case_of_exceed_1)
{
   /** \precond
   * Set object properties
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.probability_undet = 1.2F;

   /** \action
   * Call the function calcUndetProb()
   */
   calcUndetProb(object, cals);

   /** \result
   * Check whether undetermined probability is equal maximum allowable value (1.0)
   */
   DOUBLES_EQUAL(1.0F, object.probability_undet, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify whether probability is split equaly when object is moving at low speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_probability_is_splited_when_low_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 1.0F;
   object.probability_motorcycle = 0.4F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const float32_t expected_prob_bicycle = 0.5F * object.probability_motorcycle;
   const float32_t expected_prob_motorcycle = 0.5F * object.probability_motorcycle;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expected_prob_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_prob_motorcycle, object.probability_motorcycle, F360_EPSILON);
}


/** \purpose
* Purpose of this test is to verify whether probability is splited when objects speed is in range of defined threshold.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_probability_is_splited_when_specific_speed_range)
{
   /** \precond
   * Set object properties
   */
   object.speed = 10.0F;
   object.probability_motorcycle = 0.3F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   float32_t expected_prob_bicycle = cals.k_ad_oc_weight_frac_2wheel * object.probability_motorcycle;
   float32_t expected_prob_motorcycle = (1.0F - cals.k_ad_oc_weight_frac_2wheel) * object.probability_motorcycle;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expected_prob_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_prob_motorcycle, object.probability_motorcycle, F360_EPSILON);
}


/** \purpose
* Purpose of this test is to verify whether probability_bicycle is set to zero when objects speed exceeds calib threshold.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_probability_bicycle_set_to_0_at_speed_exceeds_threshold)
{
   /** \precond
   * Set object properties
   */
   object.speed = 15.0F;
   object.probability_motorcycle = 0.5F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   float32_t expected_prob_bicycle = 0.0F;
   float32_t expected_prob_motorcycle = object.probability_motorcycle;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expected_prob_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_prob_motorcycle, object.probability_motorcycle, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with low classification probability and low speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_low_probability_and_low_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 1.0F;
   object.probability_motorcycle = 0.2F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_UNDETERMINED;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with high classification probability and low speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_high_probability_and_low_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 1.0F;
   object.probability_motorcycle = 0.95F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_BICYCLE;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with low classification probability and medium speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_low_probability_and_medium_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 10.0F;
   object.probability_motorcycle = 0.4F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_UNDETERMINED;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with high classification probability and medium speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_high_probability_and_medium_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 10.0F;
   object.probability_motorcycle = 0.95F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_MOTORCYCLE;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with low classification probability and high speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_low_probability_and_high_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 20.0F;
   object.probability_motorcycle = 0.5F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_UNDETERMINED;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify if correct class is assigned for object with high classification probability and high speed
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__if_correct_class_assigned_when_high_probability_and_high_speed)
{
   /** \precond
   * Set object properties
   */
   object.speed = 20.0F;
   object.probability_motorcycle = 0.95F;
   object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
   const F360_Object_Class_T expected_class = F360_OBJ_CLASS_MOTORCYCLE;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check if object class is determined as expected
   */
   CHECK_EQUAL(expected_class, object.object_class);
}

/** \purpose
* Purpose of this test is to verify whether 2wheel probability is split equally to motorcycle and bicycle if object is not classified as motorcycle
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Split_Motorcycle_Prob_Into_Two_Classes__prob_split_equally_if_not_motorcycle)
{
   /** \precond
   * Set object properties
   */
   object.probability_motorcycle = 0.5F;
   float32_t expected_prob_bicycle = 0.25F;
   float32_t expected_prob_motorcycle = 0.25F;

   /** \action
   * Call the function Split_Motorcycle_Prob_Into_Two_Classes()
   */
   Split_Motorcycle_Prob_Into_Two_Classes(object, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expected_prob_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_prob_motorcycle, object.probability_motorcycle, F360_EPSILON);
}


/** \purpose
* Purpose of this test is to verify whether function calculates probability values and sets object class.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, run_obj_class__if_probability_values_are_calculated_and_obj_class_is_set)
{
   /** \precond
   * Set object properties
   */
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   object_tracks[0].speed = 2.0F;
   object_tracks[0].bbox.Set_Length(1.0F);
   object_tracks[0].bbox.Set_Width(1.0F);
   object_tracks[0].status = F360_OBJECT_STATUS_UPDATED;

   object_tracks[1].speed = 20.0F;
   object_tracks[1].bbox.Set_Length(5.0F);
   object_tracks[1].bbox.Set_Width(2.0F);
   object_tracks[1].status = F360_OBJECT_STATUS_UPDATED;

   float32_t initial_prob_pedestrian = object_tracks[0].probability_pedestrian;
   float32_t initial_prob_car = object_tracks[1].probability_car;
  
   tracker_info.num_active_objs = 2;
   tracker_info.active_obj_ids[0] = 1;
   tracker_info.active_obj_ids[1] = 2;
   vehicle_data.speed = 5.0;

   /** \action
   * Call the function run_obj_class()
   */
   run_obj_class(object_tracks, vehicle_data, tracker_info, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   CHECK_TRUE(initial_prob_pedestrian < object_tracks[0].probability_pedestrian);
   CHECK_EQUAL(F360_OBJ_CLASS_PEDESTRIAN, object_tracks[0].object_class);
   CHECK_TRUE(initial_prob_car < object_tracks[0].probability_car);
   CHECK_EQUAL(F360_OBJ_CLASS_CAR, object_tracks[1].object_class);
}


/** \purpose
* Purpose of this test is to verify whether prevent division by zero works in function calcProbability_Criteria_Bayes().
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, calcProbability_Criteria_Bayes__prevent_division_by_0)
{
   /** \precond
   * Set object properties
   */
   OBJ_CLASS_A_PRIORI_PROBABILITIES_T apriori;
   apriori.pedestrian = 1e-22F;
   apriori.car = 0.0F;
   apriori.truck = 0.0F;
   apriori.two_wheel = 0.0F;

   object.bbox.Set_Length(1.0F);
   object.bbox.Set_Width(1.0F);
   object.speed = 1.2F;

   OBJECT_CLASS_PROBABILITY_T criteria_Bayes;
   float32_t expexted_prob_pedestrian = 0.307283401F *  apriori.pedestrian; 

   /** \action
   * Call the function calcProbability_Criteria_Bayes()
   */
   calcProbability_Criteria_Bayes(apriori, criteria_Bayes, object, cals);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expexted_prob_pedestrian, criteria_Bayes.probability_pedestrian, F360_EPSILON);
}


/** \purpose
* Purpose of this test is to verify whether prevent division by zero works in function filteringAndNormalizationOfProbabilities().
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, filteringAndNormalizationOfProbabilities__prevent_division_by_0)
{
   /** \precond
   * Set object properties
   */
   OBJECT_CLASS_PROBABILITY_T criteria_Bayes;
   criteria_Bayes.probability_pedestrian = 1e-22F;
   criteria_Bayes.probability_car = 0.0F;
   criteria_Bayes.probability_truck = 0.0F;
   criteria_Bayes.probability_2wheel = 0.0F;

   object.probability_pedestrian = 1e-22F;
   object.probability_car = 0.0F;
   object.probability_truck = 0.0F;
   object.probability_motorcycle = 0.0F;
   
   object.probability_undet = 0.0F;

   float32_t expexted_prob_pedestrian = 1.00000003e-22F;

   /** \action
   * Call the function filteringAndNormalizationOfProbabilities()
   */
   filteringAndNormalizationOfProbabilities(object, criteria_Bayes);

   /** \result
   * Check whether probability is calculated correctly.
   */
   DOUBLES_EQUAL(expexted_prob_pedestrian, object.probability_pedestrian, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify if slow-moving (speed below expected speed range for pedestrians) 
* objects initialy classified as neither Pedestrian nor stationary are not affected.
* \reqs
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Update_Probability_Undetermined__increase_undet_prob_if_slow_slow_moving_and_ofType_car)
{
   /** \precond
    * Setup object as:
    * - type: CAR
    * - movable: no
    * - speed: lower than expexted for pedestrian
    * - probabilities: random
    **/
   object.f_moveable = false;
   object.object_class = F360_OBJ_CLASS_CAR;

   object.speed = 0.2F;
   object.probability_car = 0.1F;
   object.probability_pedestrian = 0.1F;
   object.probability_motorcycle = 0.1F;
   object.probability_bicycle = 0.1F;
   object.probability_truck = 0.1F;
   object.probability_undet = 0.5;

   float32_t expected_probability_car = 0.1F;
   float32_t expected_probability_pedestrian = 0.1F;
   float32_t expected_probability_motorcycle = 0.1F;
   float32_t expected_probability_bicycle = 0.1F;
   float32_t expected_probability_truck = 0.1F;
   float32_t expected_probability_undetermined = 0.5F;

   /** \action
   * Call the function Update_Probability_Undetermined()
   */
   Update_Probability_Undetermined(cals, object);

   /** \result
   * Check whether probability is unchanged.
   */

   DOUBLES_EQUAL(expected_probability_car, object.probability_car, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_pedestrian, object.probability_pedestrian, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_motorcycle, object.probability_motorcycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_truck, object.probability_truck, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_undetermined, object.probability_undet, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify whether slow-moving (speed in expected speed range for pedestrians)
* suspected to be a stationary object has probability_undetermined increased.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Update_Probability_Undetermined__increase_undet_prob_if_non_moving_non_zero_spd)
{
   /** \precond
    * Setup object as:
    * - movable: no
    * - speed: as expected for pedestrian, lower than max for filtering stationary objects with non-zero speed
    * - probabilities: random
    **/
   object.f_moveable = false;

   object.speed = 0.4F;
   object.probability_car = 0.1F;
   object.probability_pedestrian = 0.1F;
   object.probability_motorcycle = 0.1F;
   object.probability_bicycle = 0.1F;
   object.probability_truck = 0.1F;

   float32_t expected_probability_car = 0.03F;
   float32_t expected_probability_pedestrian = 0.03F;
   float32_t expected_probability_motorcycle = 0.03F;
   float32_t expected_probability_bicycle = 0.03F;
   float32_t expected_probability_truck = 0.03F;
   float32_t expected_probability_undetermined = 0.85F;

   /** \action
   * Call the function Update_Probability_Undetermined()
   */
   Update_Probability_Undetermined(cals, object);

   /** \result
   * Check whether probability is set as expected.
   */

   DOUBLES_EQUAL(expected_probability_car, object.probability_car, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_pedestrian, object.probability_pedestrian, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_motorcycle, object.probability_motorcycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_truck, object.probability_truck, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_undetermined, object.probability_undet, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify whether fast moving (speed above expected speed range for
* pedestrians) objects are not affected.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Update_Probability_Undetermined__increase_predestrian_probability_high_speed)
{
   /** \precond
    * Setup object as:
    * - movable: yes
    * - speed: higher than expected for pedestrian
    * - probabilities: probability for pedestrian greater than 0.9F
    **/
   object.f_moveable = true;

   object.speed = 5.5F;
   object.probability_pedestrian = 0.91F;
   object.probability_car = 0.01F;
   object.probability_motorcycle = 0.01F;
   object.probability_bicycle = 0.01F;
   object.probability_truck = 0.01F;
   object.probability_undet = 0.05F;

   float32_t expected_probability_car = 0.01F;
   float32_t expected_probability_motorcycle = 0.01F;
   float32_t expected_probability_bicycle = 0.01F;
   float32_t expected_probability_truck = 0.01F;
   float32_t expected_probability_undetermined = 0.05F;
   float32_t expected_probability_pedestrian = 0.91F;

   /** \action
   * Call the function Update_Probability_Undetermined()
   */
   Update_Probability_Undetermined(cals, object);

   /** \result
   * Check whether probability is unchanged.
   */

   DOUBLES_EQUAL(expected_probability_car, object.probability_car, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_motorcycle, object.probability_motorcycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_truck, object.probability_truck, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_undetermined, object.probability_undet, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_pedestrian, object.probability_pedestrian, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify whether slow-moving (in range of expected speed for pedestrian)
* objects of neither Pedestrian nor Undetermined are not affected.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Update_Probability_Undetermined__increase_predestrian_probability_ofType_car)
{
   /** \precond
    * Setup object as:
    * - movable: yes
    * - speed: as expected for pedestrian
    * - probabilities: probability for pedestrian greater than 0.9F
    **/
   object.f_moveable = true;

   object.speed = 0.7F;
   object.probability_pedestrian = 0.91F;
   object.probability_car = 0.01F;
   object.probability_motorcycle = 0.01F;
   object.probability_bicycle = 0.01F;
   object.probability_truck = 0.01F;
   object.probability_undet = 0.05F;
   object.object_class= F360_OBJ_CLASS_CAR;

   float32_t expected_probability_car = 0.01F;
   float32_t expected_probability_motorcycle = 0.01F;
   float32_t expected_probability_bicycle = 0.01F;
   float32_t expected_probability_truck = 0.01F;
   float32_t expected_probability_undetermined = 0.05F;
   float32_t expected_probability_pedestrian = 0.91F;

   /** \action
   * Call the function Update_Probability_Undetermined()
   */
   Update_Probability_Undetermined(cals, object);

   /** \result
   * Check whether probability is unchanged.
   */

   DOUBLES_EQUAL(expected_probability_car, object.probability_car, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_motorcycle, object.probability_motorcycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_truck, object.probability_truck, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_undetermined, object.probability_undet, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_pedestrian, object.probability_pedestrian, F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify whether slow moving (speed below expected speed range for pedestrians)
* are not affected.
* \req
* NA.
*/
TEST(f360_calculate_object_class_probabilities, Update_Probability_Undetermined__increase_predestrian_probability_low_speed)
{
   /** \precond
    * Setup object as:
    * - movable: yes
    * - speed: lower than expected for pedestrian
    * - probabilities: probability for pedestrian greater than 0.9F
    **/
   object.f_moveable = true;

   object.speed = 0.2F;
   object.probability_pedestrian = 0.91F;
   object.probability_car = 0.01F;
   object.probability_motorcycle = 0.01F;
   object.probability_bicycle = 0.01F;
   object.probability_truck = 0.01F;
   object.probability_undet = 0.15F;

   float32_t expected_probability_car = 0.01F;
   float32_t expected_probability_motorcycle = 0.01F;
   float32_t expected_probability_bicycle = 0.01F;
   float32_t expected_probability_truck = 0.01F;
   float32_t expected_probability_undetermined = 0.15F;
   float32_t expected_probability_pedestrian = 0.91F;

   /** \action
   * Call the function Update_Probability_Undetermined()
   */
   Update_Probability_Undetermined(cals, object);

   /** \result
   * Check whether probability is unchanged.
   */

   DOUBLES_EQUAL(expected_probability_car, object.probability_car, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_motorcycle, object.probability_motorcycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_bicycle, object.probability_bicycle, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_truck, object.probability_truck, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_undetermined, object.probability_undet, F360_EPSILON);
   DOUBLES_EQUAL(expected_probability_pedestrian, object.probability_pedestrian, F360_EPSILON);
}

/** @}*/