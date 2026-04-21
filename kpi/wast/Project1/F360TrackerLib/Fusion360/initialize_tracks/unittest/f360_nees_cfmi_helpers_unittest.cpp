/** \file
   This file contains unittest for f360_nees_cfmi_helpers functions.
   Each function should have defined its own tests group.
*/

#include "f360_nees_cfmi_helpers.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <f360_math.h>
#include <f360_calibrations.h>

/** \defgroup  f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff is a test group for all
*  unittests for Calc_Single_NEES_Value_For_Pos_Diff() function.
**/

TEST_GROUP(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff)
{
   float32_t tolerance = 10e-6F;
   F360_VCS_Velocity_T ref_vel = {};
   F360_VCS_Velocity_T current_nees_vel = {};
   float32_t vel_hyp_cov[2][2] = {};
};

/**
*\purpose  Check if function return maximum nees_value if all elements
*          of pos_diff_cov are equal to zero.
*\req      put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff, testReturningMaxNeesValue)
{
   /** \step{step number e.g. 1}
    */

   /** \precond
    *   Expected result is max_nees_value.
    **/

   /** \action
    *    Call Calc_Single_NEES_Value_For_Pos_Diff()
    **/

   /** \result
    *  After this test function should return
    *  maximum possible value of nees_value.
    **/

   float32_t expected_nees_value = 1.0F / F360_MIN_DENOMINATOR;

   float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel, vel_hyp_cov);

   DOUBLES_EQUAL_TEXT(expected_nees_value, nees_value, tolerance, "Calc_Single_NEES_Value_For_Pos_Diff return unexpected results for nees_value in testReturningMaxNeesValue");
}

/**
*\purpose  Check if function return maximum nees_value if determinat
*          of pos_diff_cov is zero.
*\req      put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff, testReturningNeesValueWhenDeterminantIsZero)
{
   /** \step{step number e.g. 2}
    */

   /** \precond
    *   Expected result is max_nees_value.
    **/

   /** \action
    *    Call Calc_Single_NEES_Value_For_Pos_Diff()
    **/

   /** \result
    *  After this test function should return
    *  maximum possible value of nees_value.
    **/

   vel_hyp_cov[0][0] = 4;
   vel_hyp_cov[1][1] = 1;
   vel_hyp_cov[0][1] = 2;
   vel_hyp_cov[1][0] = vel_hyp_cov[0][1];
   
   float32_t expected_nees_value = 1.0F / F360_MIN_DENOMINATOR;

   float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel, vel_hyp_cov);

   DOUBLES_EQUAL_TEXT(expected_nees_value, nees_value, tolerance, "Calc_Single_NEES_Value_For_Pos_Diff return unexpected results for nees_value in testReturningNeesValueWhenDeterminantIsZero");
}

/**
*\purpose  Check if function return proper nees_value
*\req      put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff, testReturningNeesValue)
{
   /** \step{step number e.g. 3}
    */

   /** \precond
    **/

   /** \action
    *    Call Calc_Single_NEES_Value_For_Pos_Diff()
    **/

   /** \result
    *  After this test we expeced Calc_Single_NEES_Value_For_Pos_Diff
    *  should return expected_nees_value.
    **/

   ref_vel.longitudinal = 3.0F;
   ref_vel.lateral = 4.0F;
   current_nees_vel.longitudinal = 4.0F;
   current_nees_vel.lateral = 3.0F;
   vel_hyp_cov[0][0] = 1;
   vel_hyp_cov[1][1] = 1;
   vel_hyp_cov[0][1] = 0.5;
   vel_hyp_cov[1][0] = vel_hyp_cov[0][1];

   float32_t expected_nees_value = 4.0F;

   float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel, vel_hyp_cov);

   DOUBLES_EQUAL_TEXT(expected_nees_value, nees_value, tolerance, "Calc_Single_NEES_Value_For_Pos_Diff return unexpected results for nees_value in testReturningNeesValue");
}

/**
*\purpose  Check if function return proper nees_value if determinat
*          of pos_diff_cov is unit matrix
*\req      put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff, testReturningNeesValueWithUnitCovMatrix)
{
   /** \step{step number e.g. 4}
    */

   /** \precond
    *   vel_hyp_cov is unit matrix
        difference between ref_vel and vel_hyp component is 1.
    **/

   /** \action
    *    Call Calc_Single_NEES_Value_For_Pos_Diff()
    **/

   /** \result
    *  After this test we expeced Calc_Single_NEES_Value_For_Pos_Diff
    *  should return expected_nees_value.
    **/

   ref_vel.longitudinal = 3.0F;
   ref_vel.lateral = 4.0F;
   current_nees_vel.longitudinal = 4.0F;
   current_nees_vel.lateral = 3.0F;
   vel_hyp_cov[0][0] = 1;
   vel_hyp_cov[1][1] = 1;

   float32_t expected_nees_value = 2.0F;

   float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel, vel_hyp_cov);

   DOUBLES_EQUAL_TEXT(expected_nees_value, nees_value, tolerance, "Calc_Single_NEES_Value_For_Pos_Diff return unexpected results for nees_value in testReturningNeesValueWithUnitCovMatrix");
}

/**
*\purpose  Check if function return proper nees_value when vel_hyp_cov has negative determinant.
*\req    put in requirement tag if any otherwise set to NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_pos_diff, testReturningNeesValueWithNegatveDeterminant)
{
   /** \step{step number e.g. 5}
    */

   /** \precond
    *   In pos_diff_cov matrix diagonal elements are 0, determinant is equal to 1.
    **/

   /** \action
    *    Call Calc_Single_NEES_Value_For_Pos_Diff()
    **/

   /** \result
    *  After this test we expeced Calc_Single_NEES_Value_For_Pos_Diff
    *  should return expected_nees_value maximum nees_value (determinant is negative).
    **/

   vel_hyp_cov[0][1] = 1;
   vel_hyp_cov[1][0] = vel_hyp_cov[0][1];

   float32_t expected_nees_value = 1.0F / F360_MIN_DENOMINATOR;

   float32_t nees_value = Calc_Single_NEES_Value_For_Pos_Diff(ref_vel, current_nees_vel, vel_hyp_cov);

   DOUBLES_EQUAL_TEXT(expected_nees_value, nees_value, tolerance, "Calc_Single_NEES_Value_For_Pos_Diff return unexpected results for nees_value in testReturningNeesValueWithNegatveDeterminant");
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud
*  @{
*/

/** \brief
*  f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud is a test group for all
*  unittests for Calc_Single_NEES_Value_For_Cloud() function.
**/

TEST_GROUP(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud)
{
   const float32_t tolerance = 10e-5F;

   float32_t Calc_Azimuth_To_Match_Velocity_Vector_Direction(const F360_VCS_Velocity_T v)
   {
      return atanf(v.lateral / v.longitudinal);
   }

   float32_t Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(const F360_VCS_Velocity_T v, const float32_t azimuth)
   {
      return v.longitudinal * cos(azimuth) + v.lateral * sin(azimuth);
   }
};

/**
*\purpose  Check if function returns maximum nees_value if all values equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenAllInputValuesEqualToZero)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   F360_VCS_Velocity_T VCS_velocity = {};
   float32_t azimuth = {};
   float32_t range_rate = {};
   float32_t range_rate_var = {};

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth), sinf(azimuth), range_rate, range_rate_var);

   /** \result
   *  Function should return maximum possible value of nees_value.
   **/
   float32_t expected_nees_value = 1.0F / F360_MIN_DENOMINATOR;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}

/**
*\purpose  Check if function return maximum nees_value if range rate variance equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenRangeRateVarEqualToZero)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   F360_VCS_Velocity_T VCS_velocity = { 10.0F, 10.0F };
   float32_t range_rate_var = 0.0F;
   float32_t azimuth = 0.1F;
   float32_t range_rate = 10.0F;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth), sinf(azimuth), range_rate, range_rate_var);

   /** \result
   *  Function should return maximum possible value of nees_value.
   **/
   float32_t expected_nees_value = 1.0F / F360_MIN_DENOMINATOR;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}


/**
*\purpose  Check if function returns nees value equal to zero when range rate error is equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenRangeRateErrorEqualToZero)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   F360_VCS_Velocity_T VCS_velocity = { 10.0F, 0.0F };
   float32_t range_rate_var = 0.5F;
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t range_rate_exact = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact);

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_exact), sinf(azimuth_exact), range_rate_exact, range_rate_var);

   /** \result
   *  Nees value should be equal to zero
   **/
   float32_t expected_nees_value = 0.0F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}


/**
*\purpose  Check if function returns valid nees value when azimuth angle equal to velocity vector direction, 
* but vector magnitude different than range rate
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenVelocityVectorMagnitudeDifferentThanRangeRate)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t range_rate_error = 2.0F;
   F360_VCS_Velocity_T VCS_velocity = { 10.0F, 10.0F };
   float32_t range_rate_var = 0.1F;
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t range_rate_with_error = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact) + range_rate_error;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_exact), sinf(azimuth_exact), range_rate_with_error, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 40.0F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}

/**
*\purpose  Check if function returns valid nees value when azimuth angle and velocity vector direction are orthogonal
* magnitude and range rate the same
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenVelocityVectorOrthogonalToAzimuthAngle)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t azimuth_error = F360_DEG2RAD(90.0F);
   F360_VCS_Velocity_T VCS_velocity = { 0.0F, -10.0F };
   float32_t range_rate_var = 0.1F;
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t azimuth_with_error = azimuth_exact + azimuth_error;
   float32_t range_rate_exact = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact);

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_with_error), sinf(azimuth_with_error), range_rate_exact, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 1000.0F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}

/**
*\purpose  Check if function returns valid nees value when azimuth angle and velocity have opposite directions
* magnitude and range rate the same
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenVelocityVectorOppositeDirectionToAzimuthAngle)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t azimuth_error = F360_DEG2RAD(180.0F);
   F360_VCS_Velocity_T VCS_velocity = { -10.0F, 0.0F };
   float32_t range_rate_var = 0.1F;
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t azimuth_with_error = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity) + azimuth_error;
   float32_t range_rate_exact = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact);

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_with_error), sinf(azimuth_with_error), range_rate_exact, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 4000.0F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}


/**
*\purpose  Check if function returns valid nees value when velocity vector magnitude equal to range rate
* but azimuth angle different than velocity vector direction by small value
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenAzimuthDifferentThanVelocityVectorDirectionBySmallValue)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t azimuth_error = F360_DEG2RAD(5.0F);
   float32_t range_rate_var = 0.1F;
   F360_VCS_Velocity_T VCS_velocity = { 10.0F, -20.0F };
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t azimuth_with_error = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity) + azimuth_error;
   float32_t range_rate_exact = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact);

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_with_error), sinf(azimuth_with_error), range_rate_exact, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 0.0724041611F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}

/**
*\purpose  Check if function returns valid nees value when both vector magnitude and direction
* different than range rate and azimuth
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueWhenMagnitudeAndDirectionDifferentThanRangeRateAndAzimuth)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t azimuth_error = F360_DEG2RAD(3.0F);
   float32_t range_rate_error = 0.5F;
   float32_t range_rate_var = 0.1F;
   F360_VCS_Velocity_T VCS_velocity = { 20.0F, -5.0F };
   float32_t azimuth_exact = Calc_Azimuth_To_Match_Velocity_Vector_Direction(VCS_velocity);
   float32_t azimuth_with_error = azimuth_exact + azimuth_error;
   float32_t range_rate_with_error = Calc_Range_Rate_To_Match_Velocity_Vector_Magnitude(VCS_velocity, azimuth_exact) + range_rate_error;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth_with_error), sinf(azimuth_with_error), range_rate_with_error, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 2.79051805F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}

/**
*\purpose  Check if function returns nees value equal to zero even though azimuth angle not equal to velocity vector direction
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_single_nees_value_for_cloud, testReturningNeesValueEqualToZeroWhenAzimuthAngleDifferentThanVelocityVectorDirection)
{
   /** \step{1}
   * Set input values accoring to test purpose
   */
   float32_t range_rate_var = 0.1F;
   F360_VCS_Velocity_T VCS_velocity = { 10.0F, 0.0F };
   float32_t azimuth = F360_DEG2RAD(45.0F);
   float32_t range_rate = 5.0F * sqrt(2.0F);

   /** \precond
   *   NA
   **/

   /** \action
   *    Call Calc_Single_NEES_Value_For_Cloud()
   **/
   float32_t nees_value = Calc_Single_NEES_Value_For_Cloud(VCS_velocity, cosf(azimuth), sinf(azimuth), range_rate, range_rate_var);

   /** \result
   *  Nees value should be correct
   **/
   float32_t expected_nees_value = 0.0F;
   DOUBLES_EQUAL(expected_nees_value, nees_value, tolerance);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants
*  @{
*/

/** \brief
*  Test group for all unittests for Calc_Vel_From_NEES_CFMI_Determinants() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants)
{
   F360_NEES_CFMI_Determinants_T determinants = {};
   F360_VCS_Velocity_T VCS_velocity = {};
   const float32_t tolerance = 10e-5F;

   /** \setup
   * Fill determinants with example valid data
   **/
   TEST_SETUP()
   {
      determinants.n_dets = 5U;
      determinants.Sxx = 20.0F;
      determinants.Syy = 10.0F;
      determinants.Sx = 10.0F;
      determinants.Sy = -40.0F;
      determinants.Sxy = 10.0F;
      determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same
      determinants.information = 150.0F;
   }

};

/**
*\purpose  Check if function returns false when input variables equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenAllInputValuesEqualToZero)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   F360_NEES_CFMI_Determinants_T determinants = {};
   F360_VCS_Velocity_T VCS_velocity = {};

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/**
*\purpose  Check if function returns false number of detections equal to one
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenNumberOfInputDetectionsEqualToOne)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 1;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/**
*\purpose  Check if function returns false when number of detections above one, but velocity determinant equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenVelocityDeterminantEqualToZero)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.Sxx = 50.0F;
   determinants.Syy = 2.0F;
   determinants.Sxy = 10.0F;
   determinants.Syx = 10.0F;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/**
*\purpose  Check if function returns false when number of detections above one, but velocity determinant negative
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenVelocityDeterminantSmallerThanZero)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 2U;
   determinants.Sxx = 10.0F;
   determinants.Syy = -30.0F;
   determinants.Sx = 20.0F;
   determinants.Sy = -40.0F;
   determinants.Sxy = 15.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/**
*\purpose  Check if function returns a value even when Sxy!=Syx
* In valid data structure this situation should not occur, but we at least need to make
* sure that the function doesn't crash. Sxy = Syx check is outside it's competency
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueSxyNotEqualToSyx)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.Sxy = -10.0F;
   determinants.Syx = 20.0F;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated, function should execute normally
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = { -0.75F, -2.5F };
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}

/**
*\purpose  Check if function returns correct velocity vector for valid input parameters
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenInputParamsValid1)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 2U;
   determinants.Sxx = 10.0F;
   determinants.Syy = 30.0F;
   determinants.Sx = 20.0F;
   determinants.Sy = -40.0F;
   determinants.Sxy = 10.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated correctly
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = {5.0F, -3.0F};
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}

/**
*\purpose  Check if function returns a valid velocity for example input parameters
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenInputParamsValid2)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 8U;
   determinants.Sxx = 10.0F;
   determinants.Syy = 40.0F;
   determinants.Sx = 50.0F;
   determinants.Sy = -20.0F;
   determinants.Sxy = -10.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated correctly
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = { 6.0F, 1.0F };
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}

/**
*\purpose  Check if function returns a velocity vector equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenEstimatedVelocityVectorShouldBeEqualToZero)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.Sx = 0.0F;
   determinants.Sy = 0.0F;

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated correctly
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = { 0.0F, 0.0F };
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}

/**
*\purpose  Check if function returns a velocity vector x component equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenEstimatedVelocityVectorXShouldBeEqualToZeroForNonZeroSxAndSy)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 5U;
   determinants.Sxx = 20.0F;
   determinants.Syy = 10.0F;
   determinants.Sx = 10.0F;
   determinants.Sy = -10.0F;
   determinants.Sxy = -10.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated correctly
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = { 0.0F, -1.0F };
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}

/**
*\purpose  Check if function returns a velocity vector y component equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_from_nees_cfmi_determinants, testReturnedValueWhenEstimatedVelocityVectorYShouldBeEqualToZeroForNonZeroSxAndSy)
{
   /** \step{1}
   *  Set input values accoring to test purpose
   */
   determinants.n_dets = 5U;
   determinants.Sxx = 5.0F;
   determinants.Syy = 2.0F;
   determinants.Sx = -100.0F/3.0F;
   determinants.Sy = -20.0F;
   determinants.Sxy = 3.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \precond
   *   NA
   **/

   /** \action
   *    Call tested function
   **/
   bool f_velocity_valid = Calc_Vel_From_NEES_CFMI_Determinants(determinants, VCS_velocity);

   /** \result
   *  Velocity should be estimated correctly
   **/
   F360_VCS_Velocity_T expected_VCS_velocity = {-6.66666F, 0.0F };
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_VCS_velocity.longitudinal, VCS_velocity.longitudinal, tolerance);
   DOUBLES_EQUAL(expected_VCS_velocity.lateral, VCS_velocity.lateral, tolerance);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity
*  @{
*/

/** \brief
* Test group gathering all unittests for Estimate_Plausibility_For_NEES_CFMI_Velocity() function
*/
TEST_GROUP(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity)
{
   F360_Calibrations_T calibrations = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_NEES_CFMI_Velocity_T velocity = {};
   F360_NEES_CFMI_Velocity_T expected_velocity = {};
   const float32_t tolerance = 10e-5F;

   /** \setup
   * Fill input variables with minimum valid data and corresponding expected outputs
   */
   TEST_SETUP()
   {
      // Inputs
      Initialize_Tracker_Calibrations(calibrations);
      nees_cfmi_information.dets_num = 5U;
      nees_cfmi_information.init_dets_inliers_num = 5U;
      nees_cfmi_information.vels_num = 3U;
      nees_cfmi_information.expected_vels_inliers_num = 2U;
      velocity.weight_dets_sum = 3.5F;
      velocity.weight_vels_sum = 0.8F;
      // Outputs
      expected_velocity.plausibility_cloud = 0.85995F;
      expected_velocity.plausibility_pos_diff = 0.54945F;
      expected_velocity.plausibility = 0.47250F;
   }
};

/** \purpose
* Basic check to verify if the functions returns a valid result for example input parameters.
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_A_Correct_Result_For_Valid_Input_Data)
{
   /** \precond
   * NA
   */

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to plausability from position difference if number of valid detections equal to zero
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_From_Position_Difference_For_Zero_Valid_Detections)
{
   /** \precond
   * Set valid detection number to zero
   */
   nees_cfmi_information.dets_num = 0U;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MAX_PROBABILITY;
   expected_velocity.plausibility = expected_velocity.plausibility_pos_diff;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to plausability from cloud if number of valid velocities equal to zero
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_From_Cloud_For_Zero_Valid_Velocities)
{
   /** \precond
   * Set valid velocities number to zero
   */
   nees_cfmi_information.vels_num = 0U;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_pos_diff = F360_MAX_PROBABILITY; 
   expected_velocity.plausibility = expected_velocity.plausibility_cloud;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to one when max_dets_inliers_ratio and max_vels_inliers_ratio are both reached
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_Equal_To_One_When_Max_Ratios_Reached)
{
   /** \precond
   * Set input values to match max_dets_inliers_ratio and max_vels_inliers_ratio
   */
   velocity.weight_dets_sum = 4.07F;
   nees_cfmi_information.dets_num = 3U;
   velocity.weight_vels_sum = 1.46F;
   nees_cfmi_information.vels_num = 5U;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MAX_PROBABILITY;
   expected_velocity.plausibility_pos_diff = F360_MAX_PROBABILITY;
   expected_velocity.plausibility = F360_MAX_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to one when max_dets_inliers_ratio and max_vels_inliers_ratio are both exceeded
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_Equal_To_One_When_Max_Ratios_Exceeded)
{
   /** \precond
   * Set input values to exceed max_dets_inliers_ratio and max_vels_inliers_ratio
   */
   velocity.weight_dets_sum = 5.07F;
   nees_cfmi_information.dets_num = 3U;
   velocity.weight_vels_sum = 2.46F;
   nees_cfmi_information.vels_num = 5U;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MAX_PROBABILITY;
   expected_velocity.plausibility_pos_diff = F360_MAX_PROBABILITY;
   expected_velocity.plausibility = F360_MAX_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to zero when sum of cloud weights equal to zero
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_Equal_To_Zero_When_Sum_Of_Cloud_Weights_Equal_To_Zero)
{
   /** \precond
   * Set cloud weight equal to zero
   */
   velocity.weight_dets_sum = 0.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MIN_PROBABILITY;
   expected_velocity.plausibility = F360_MIN_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns overall plausability equal to zero when sum of weights of position difference equal to zero
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_Equal_To_Zero_When_Sum_Of_Position_Difference_Weights_Equal_To_Zero)
{
   /** \precond
   * Set weight of position difference equal to zero
   */
   velocity.weight_vels_sum = 0.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);


   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_pos_diff = F360_MIN_PROBABILITY;
   expected_velocity.plausibility = F360_MIN_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns valid plausability equal to one when inlier ratios equal to one and all calibration parameters set to one
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Plausability_Equal_To_One_For_Inlier_Ratios_Equal_To_One_And_Calibration_Params_Set_To_One)
{
   /** \precond
   * Set input parameters according to test specification
   */
   velocity.weight_dets_sum = 5.0F;
   nees_cfmi_information.dets_num = 5.0F;
   nees_cfmi_information.init_dets_inliers_num = 5.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 1.0F;


   velocity.weight_vels_sum = 5.0F;
   nees_cfmi_information.vels_num = 5.0F;
   nees_cfmi_information.expected_vels_inliers_num = 5.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 1.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);

   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MAX_PROBABILITY;
   expected_velocity.plausibility_pos_diff = F360_MAX_PROBABILITY;
   expected_velocity.plausibility = F360_MAX_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns valid plausability when inlier ratios equal to one and all calibration parameters set to 2.0
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Valid_Plausability_For_Inlier_Ratios_Equal_To_One_And_Calibration_Params_Modified)
{
   /** \precond
   * Set input parameters according to test specification
   */
   velocity.weight_dets_sum = 4.0F;
   nees_cfmi_information.dets_num = 4.0F;
   nees_cfmi_information.init_dets_inliers_num = 4.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 2.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 2.0F;

   velocity.weight_vels_sum = 4.0F;
   nees_cfmi_information.vels_num = 4.0F;
   nees_cfmi_information.expected_vels_inliers_num = 4.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 2.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 2.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);

   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = 0.25F;
   expected_velocity.plausibility_pos_diff = 0.25F;
   expected_velocity.plausibility = 0.0625F;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns valid plausability when inlier ratios equal to 0.25, 
* and all calibration parameters set one
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Valid_Plausability_For_Inlier_Ratios_Equal_To_One_And_Calibration_Params_Modified_2)
{
   /** \precond
   * Set input parameters according to test specification
   */
   velocity.weight_dets_sum = 1.0F;
   nees_cfmi_information.dets_num = 4.0F;
   nees_cfmi_information.init_dets_inliers_num = 10.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 0.5F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 0.5F;

   velocity.weight_vels_sum = 1.0F;
   nees_cfmi_information.vels_num = 4.0F;
   nees_cfmi_information.expected_vels_inliers_num = 10.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 0.5F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 0.5F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);

   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = 0.4F;
   expected_velocity.plausibility_pos_diff = 0.4F;
   expected_velocity.plausibility = 0.16F;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns valid plausability when inlier ratios equal to one, init_dets_inliers_num and expected_vels_inliers_num double the value of inlier ratio
* and all calibration parameters set one
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Valid_Plausability_For_Expected_Inliers_And_Velocities_Double_The_Value_Of_Inlier_Ratio)
{
   /** \precond
   * Set input parameters according to test specification
   */
   velocity.weight_dets_sum = 4.0F;
   nees_cfmi_information.dets_num = 4.0F;
   nees_cfmi_information.init_dets_inliers_num = 8.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 1.0F;

   velocity.weight_vels_sum = 4.0F;
   nees_cfmi_information.vels_num = 4.0F;
   nees_cfmi_information.expected_vels_inliers_num = 8.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 1.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);

   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = 0.5F;
   expected_velocity.plausibility_pos_diff = 0.5F;
   expected_velocity.plausibility = 0.25F;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}

/** \purpose
* Check if function returns valid plausability when inlier ratios equal to one, init_dets_inliers_num and expected_vels_inliers_num half the value of inlier ratio
* and all calibration parameters set one
* \req
* NA
*/
TEST(f360_nees_cfmi_helpers_estimate_plausability_for_nees_cfmi_velocity, Should_Return_Valid_Plausability_For_Expected_Inliers_And_Velocities_Half_The_Value_Of_Inlier_Ratio)
{
   /** \precond
   * Set input parameters according to test specification
   */
   velocity.weight_dets_sum = 4.0F;
   nees_cfmi_information.dets_num = 4.0F;
   nees_cfmi_information.init_dets_inliers_num = 0.5F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_cl_in_ratio_scale = 1.0F;

   velocity.weight_vels_sum = 4.0F;
   nees_cfmi_information.vels_num = 4.0F;
   nees_cfmi_information.expected_vels_inliers_num = 0.5F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio = 1.0F;
   calibrations.k_nees_cfmi_plaus_expected_pd_in_ratio_scale = 1.0F;

   /** \action
   * Call the function being tested
   */
   Estimate_Plausibility_For_NEES_CFMI_Velocity(calibrations, nees_cfmi_information, velocity);

   /** \result
   * Compare estimated velocity probability with the expected one
   */
   expected_velocity.plausibility_cloud = F360_MAX_PROBABILITY;
   expected_velocity.plausibility_pos_diff = F360_MAX_PROBABILITY;
   expected_velocity.plausibility = F360_MAX_PROBABILITY;
   DOUBLES_EQUAL(expected_velocity.plausibility_cloud, velocity.plausibility_cloud, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility_pos_diff, velocity.plausibility_pos_diff, tolerance);
   DOUBLES_EQUAL(expected_velocity.plausibility, velocity.plausibility, tolerance);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity
*  @{
*/

/** \brief
*  Test group for all unittests for Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity)
{
   F360_NEES_CFMI_Velocity_T velocity = {};
   F360_NEES_CFMI_Velocity_T expected_output = {};
   const float32_t tolerance = 10e-5F;

   /** \setup
   * Set example valid input values
   */
   TEST_SETUP()
   {
      // Set determinants to values allowing to calculate a correct covariance
      velocity.determinants.Sxx = 20.0F;
      velocity.determinants.Syy = 10.0F;
      velocity.determinants.Sxy = -10.0F;
      velocity.determinants.Syx = velocity.determinants.Sxy; // In valid structure those fields suppose to be the same

      // Set expected output values corresponding to selected input determinants
      expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] = 0.1;
      expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = 0.2F;
      expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.1F;
      expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
   }
};

/** \purpose
* Check if function returns valid output when velocity is marked with valid flag and
* velocity determinants allow to calculate a valid covariance
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity, Should_Return_Correct_Output_When_Velocity_Valid_And_Valid_Covariance_Possible_To_Calculate)
{
   /** \precond
   * Set velocity flag to valid
   */
   velocity.f_valid = true;

   /** \action
   * Call tested function
   **/
   Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(velocity);

   /** \result
   * Function should return a correct result
   **/
   CHECK(velocity.f_valid);
   CHECK(velocity.f_cov_valid);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
* Check if function returns valid output when velocity is marked with invalid flag but
* velocity determinants allow to calculate a valid covariance
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity, Should_Return_Correct_Output_When_Velocity_Invalid_But_Valid_Covariance_Possible_To_Calculate)
{
   /** \precond
   * Set velocity flag to invalid
   */
   velocity.f_valid = false;

   /** \action
   * Call tested function
   **/
   Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(velocity);

   /** \result
   * Function should return a correct result
   **/
   CHECK_FALSE(velocity.f_valid);
   CHECK(velocity.f_cov_valid);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], velocity.vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_output.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], velocity.vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
* Check if function returns correct output when velocity is marked with valid flag but
* velocity determinants do NOT allow to calculate a valid covariance
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity, Should_Return_Correct_Output_When_Velocity_Valid_But_Valid_Covariance_Not_Possible_To_Calculate)
{
   /** \precond
   * Set velocity flag to valid and determinants to values do NOT allowing to calculate a correct covariance
   */
   velocity.f_valid = true;
   velocity.determinants.Sxx = 2.0F;
   velocity.determinants.Syy = 1.0F;
   velocity.determinants.Sxy = 5.0F;
   velocity.determinants.Syx = velocity.determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(velocity);

   /** \result
   * Function should return a correct result
   **/
   CHECK_FALSE(velocity.f_valid);
   CHECK_FALSE(velocity.f_cov_valid);
}

/** \purpose
* Check if function returns correct output when velocity is marked with invalid flag and
* velocity determinants do NOT allow to calculate a valid covariance
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_velocity, Should_Return_Correct_Output_When_Velocity_Invalid_And_Valid_Covariance_Not_Possible_To_Calculate)
{
   /** \precond
   * Set velocity flag to invalid and determinants to values do NOT allowing to calculate a correct covariance
   */
   velocity.f_valid = false;
   velocity.determinants.Sxx = 2.0F;
   velocity.determinants.Syy = 1.0F;
   velocity.determinants.Sxy = 5.0F;
   velocity.determinants.Syx = velocity.determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(velocity);

   /** \result
   * Function should return a correct result
   **/
   CHECK_FALSE(velocity.f_valid);
   CHECK_FALSE(velocity.f_cov_valid);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants
*  @{
*/

/** \brief
*  Test group for all unittests for Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants)
{
   F360_NEES_CFMI_Determinants_T determinants = {};
   float32_t cov[2][2] = {};
   float32_t expected_cov[2][2] = {};
   const float32_t tolerance = 10e-5F;
};

/** \purpose
* Check if function returns valid output for correct input values
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_Valid_Output_For_Correct_Input_Values_1)
{
   /** \precond
   * Set input values according to test purpose
   */
   determinants.Sxx = 20.0F;
   determinants.Syy = 10.0F;
   determinants.Sxy = 10.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Function should return a valid result
   **/
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_X] = 0.1F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = 0.2F;
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = -0.1F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
   
   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_X], cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y], cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X], cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
* Check if function returns valid output for correct input values
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_Valid_Output_For_Correct_Input_Values_2)
{
   /** \precond
   * Set input values according to test purpose
   */
   determinants.Sxx = 10.0F;
   determinants.Syy = 30.0F;
   determinants.Sxy = 15.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Function should return a valid result
   **/
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_X] = 0.4F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = 0.133333F;
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = -0.2F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y];

   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_X], cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y], cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X], cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
* Check if function returns valid output for correct input values
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_Valid_Output_For_Correct_Input_Values_3)
{
   /** \precond
   * Set input values according to test purpose
   */
   determinants.Sxx = -20.0F;
   determinants.Syy = -1.0F;
   determinants.Sxy = -1.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Function should return a valid result
   **/
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_X] = -0.0526315793F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = -1.05263162F;
   expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.0526315793F;
   expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y];

   CHECK(f_velocity_valid);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_X], cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_X][F360_2D_IDX_Y], cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_X], cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
* Check if function returns false when input variables equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_False_When_All_Determinants_Equal_To_Zero)
{
   /** \precond
   * Set input values according to test purpose
   */
   F360_NEES_CFMI_Determinants_T determinants = {};

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Velocity covariance estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/** \purpose
* Check if function returns false when velocity determinant equal to zero
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_False_When_Velocity_Determinant_Equal_To_Zero)
{
   /** \precond
   * Set input values according to test purpose
   */
   determinants.Sxx = 50.0F;
   determinants.Syy = 2.0F;
   determinants.Sxy = 10.0F;
   determinants.Syx = 10.0F;

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Velocity covariance estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}

/** \purpose
* Check if function returns false when velocity determinant negative
*\req      NA
*/
TEST(f360_nees_cfmi_helpers_calc_raw_vel_cov_for_nees_cfmi_determinants, Should_Return_False_When_Velocity_Determinant_Negative)
{
   /** \precond
   * Set input values according to test purpose
   */
   determinants.Sxx = 10.0F;
   determinants.Syy = -30.0F;
   determinants.Sxy = 15.0F;
   determinants.Syx = determinants.Sxy; // In valid structure those fields suppose to be the same

   /** \action
   * Call tested function
   **/
   bool f_velocity_valid = Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(determinants, cov);

   /** \result
   * Velocity covariance estimation should not be possible
   **/
   CHECK_FALSE(f_velocity_valid);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants
*  @{
*/

/** \brief
* Test group for all unittests for Add_Into_NEES_CFMI_Determinant() and Add_Into_NEES_CFMI_Determinant_Weighted() functions
*/
TEST_GROUP(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants)
{
   F360_NEES_CFMI_Determinants_T source_determinants = {};
   F360_NEES_CFMI_Determinants_T destination_determinants = {};
   F360_NEES_CFMI_Determinants_T expected_determinants = {};
   const float32_t tolerance = 10e-5F;

   /** \setup
   * Set example values for input parameters
   */
   TEST_SETUP()
   {
      destination_determinants.n_dets = 3U;
      destination_determinants.information = 1.0F;
      destination_determinants.Sxx = 0.2F;
      destination_determinants.Sxy = 1.0F;
      destination_determinants.Sx = -0.8F;
      destination_determinants.Syy = -0.1F;
      destination_determinants.Syx = destination_determinants.Sxy;
      destination_determinants.Sy = -667.0F;

      source_determinants.n_dets = 5U;
      source_determinants.information = 0.2F;
      source_determinants.Sxx = -0.2F;
      source_determinants.Sxy = 10.0F;
      source_determinants.Sx = -2.0F;
      source_determinants.Syy = 1.0F;
      source_determinants.Syx = source_determinants.Sxy;
      source_determinants.Sy = 666.0F;

      expected_determinants.n_dets = 8U;
      expected_determinants.information = 1.2F;
      expected_determinants.Sxx = 0.0F;
      expected_determinants.Sxy = 11.0F;
      expected_determinants.Sx = -2.8F;
      expected_determinants.Syy = 0.9F;
      expected_determinants.Syx = expected_determinants.Sxy;
      expected_determinants.Sy = -1.0F;
   }
};

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant() returns correct result for valid input parameters
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Should_Return_Correct_Output_For_Valid_Input_Information)
{
   /** \precond
   * NA
   */

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant(destination_determinants, source_determinants);

   /** \result
   * Compare resulting structue with the expected one
   */
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant() does not modify the source structure
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Should_Not_Modify_The_Source_Structure)
{
   /** \precond
   * Copy source structure as expected values before function execution
   */
   expected_determinants.n_dets = source_determinants.n_dets;
   expected_determinants.information = source_determinants.information;
   expected_determinants.Sxx = source_determinants.Sxx;
   expected_determinants.Sxy = source_determinants.Sxy;
   expected_determinants.Sx = source_determinants.Sx;
   expected_determinants.Syy = source_determinants.Syy;
   expected_determinants.Syx = source_determinants.Syx;
   expected_determinants.Sy = source_determinants.Sy;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant(destination_determinants, source_determinants);

   /** \result
   * Compare resulting structue with the expected one
   */
   DOUBLES_EQUAL(expected_determinants.n_dets, source_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, source_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, source_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, source_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, source_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, source_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, source_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, source_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant() returns expected ouput for all input values equal to zero
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Should_Return_Correct_Result_For_Zero_Input_Structures)
{
   /** \precond
   * Fill input structures with zeros
   */
   F360_NEES_CFMI_Determinants_T source_determinants = {};
   F360_NEES_CFMI_Determinants_T destination_determinants = {};

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant(destination_determinants, source_determinants);

   /** \result
   * Compare resulting structure with the expected one
   */
   F360_NEES_CFMI_Determinants_T expected_determinants = {};
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() returns correct result for weight equal to one
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Return_Correct_Output_For_Weight_Equal_To_One)
{
   /** \precond
   * Set weight to one
   */
   float32_t weight = 1.0F;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() returns correct result for weight equal to zero
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Return_Correct_Output_For_Weight_Equal_To_Zero)
{
   /** \precond
   * Set weight to zero and copy all destination determinants to expected determinants before function execution
   * apart from n_dets which should be updated without regard to weight value
   */
   float32_t weight = 0.0F;
   expected_determinants.information = destination_determinants.information;
   expected_determinants.Sxx = destination_determinants.Sxx;
   expected_determinants.Sxy = destination_determinants.Sxy;
   expected_determinants.Sx = destination_determinants.Sx;
   expected_determinants.Syy = destination_determinants.Syy;
   expected_determinants.Syx = destination_determinants.Syx;
   expected_determinants.Sy = destination_determinants.Sy;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() returns correct result for example positive weight
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Return_Correct_Output_For_Example_Positive_Weight)
{
   /** \precond
   * Set weight value
   */
   float32_t weight = 0.5F;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   expected_determinants.information = 1.1F;
   expected_determinants.Sxx = 0.1F;
   expected_determinants.Sxy = 6.0F;
   expected_determinants.Sx = -1.8F;
   expected_determinants.Syy = 0.4F;
   expected_determinants.Syx = expected_determinants.Sxy;
   expected_determinants.Sy = -334.0F;
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() returns correct result for example negative weight
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Return_Correct_Output_For_Example_Negative_Weight)
{
   /** \precond
   * Set weight value
   */
   float32_t weight = -2.0F;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   expected_determinants.n_dets = 8U;
   expected_determinants.information = 0.6F;
   expected_determinants.Sxx = 0.6F;
   expected_determinants.Sxy = -19.0F;
   expected_determinants.Sx = 3.2F;
   expected_determinants.Syy = -2.1F;
   expected_determinants.Syx = expected_determinants.Sxy;
   expected_determinants.Sy = -1999.0F;
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() does not modify the source structure
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Not_Modify_The_Source_Structure)
{
   /** \precond
   * Set weight value and copy source structure as expected values before function execution
   */
   float32_t weight = 0.8F;
   expected_determinants.n_dets = source_determinants.n_dets;
   expected_determinants.information = source_determinants.information;
   expected_determinants.Sxx = source_determinants.Sxx;
   expected_determinants.Sxy = source_determinants.Sxy;
   expected_determinants.Sx = source_determinants.Sx;
   expected_determinants.Syy = source_determinants.Syy;
   expected_determinants.Syx = source_determinants.Syx;
   expected_determinants.Sy = source_determinants.Sy;

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   DOUBLES_EQUAL(expected_determinants.n_dets, source_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, source_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, source_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, source_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, source_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, source_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, source_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, source_determinants.Sy, tolerance);
}

/** \purpose Check if function Add_Into_NEES_CFMI_Determinant_Weighted() returns expected ouput for all input values equal to zero
* \req NA
*/
TEST(f360_nees_cfmi_helpers_add_into_nees_cfmi_determinants, Add_Into_NEES_CFMI_Determinant_Weighted_Should_Return_Correct_Result_For_Zero_Input_Structures)
{
   /** \precond
   * Fill input structures with zeros
   */
   float32_t weight = 0.4F;
   F360_NEES_CFMI_Determinants_T source_determinants = {};
   F360_NEES_CFMI_Determinants_T destination_determinants = {};

   /** \action
   * Call tested function
   */
   Add_Into_NEES_CFMI_Determinant_Weighted(destination_determinants, source_determinants, weight);

   /** \result
   * Compare resulting structue with the expected one
   */
   F360_NEES_CFMI_Determinants_T expected_determinants = {};
   DOUBLES_EQUAL(expected_determinants.n_dets, destination_determinants.n_dets, tolerance);
   DOUBLES_EQUAL(expected_determinants.information, destination_determinants.information, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxx, destination_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sxy, destination_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sx, destination_determinants.Sx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syy, destination_determinants.Syy, tolerance);
   DOUBLES_EQUAL(expected_determinants.Syx, destination_determinants.Syx, tolerance);
   DOUBLES_EQUAL(expected_determinants.Sy, destination_determinants.Sy, tolerance);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants
*  @{
*/

/** \brief
* Test group for all unittests for Calc_Vel_Determinant_From_NEES_CFMI_Determinants() function
*/
TEST_GROUP(f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants)
{
   F360_NEES_CFMI_Determinants_T determinants = {};
   const float32_t tolerance = 10e-5F;
};

/** \purpose Check if function returns positive value for given input parameters
* \req NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants, Should_Return_Positive_Value_For_Given_Input_Parameters)
{
   /** \precond
   * Set example input values
   */
   determinants.Sxx = 2.0F;
   determinants.Sxy = 1.0F;
   determinants.Syy = 3.0F;
   determinants.Syx = determinants.Sxy;

   /** \action
   * Call tested function
   */
   float32_t velocity_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);

   /** \result
   * Compare returned value with the expected one
   */
   float32_t expected_velocity_determinant = 5.0F;
   DOUBLES_EQUAL(expected_velocity_determinant, velocity_determinant, tolerance);
}

/** \purpose Check if function returns negative value for given input parameters
* \req NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants, Should_Return_Negative_Value_For_Given_Input_Parameters)
{
   /** \precond
   * Set example input values
   */
   determinants.Sxx = -5.0F;
   determinants.Sxy = -4.0F;
   determinants.Syy = 3.0F;
   determinants.Syx = determinants.Sxy;

   /** \action
   * Call tested function
   */
   float32_t velocity_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);

   /** \result
   * Compare returned value with the expected one
   */
   float32_t expected_velocity_determinant = -31.0F;
   DOUBLES_EQUAL(expected_velocity_determinant, velocity_determinant, tolerance);
}

/** \purpose Check if function returns zero value for given input parameters
* \req NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants, Should_Return_Zero_Value_For_Given_Input_Parameters)
{
   /** \precond
   * Set example input values
   */
   determinants.Sxx = 12.5F;
   determinants.Sxy = -5.0F;
   determinants.Syy = 2.0F;
   determinants.Syx = determinants.Sxy;

   /** \action
   * Call tested function
   */
   float32_t velocity_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);

   /** \result
   * Compare returned value with the expected one
   */
   float32_t expected_velocity_determinant = 0.0F;
   DOUBLES_EQUAL(expected_velocity_determinant, velocity_determinant, tolerance);
}

/** \purpose Check if function returns zero value for zero input structure
* \req NA
*/
TEST(f360_nees_cfmi_helpers_calc_vel_determinant_from_cfmi_determinants, Should_Return_Zero_Value_For_Zero_Input_Structure)
{
   /** \precond
   * Fill input structure with zeros
   */
   F360_NEES_CFMI_Determinants_T determinants = {};

   /** \action
   * Call tested function
   */
   float32_t velocity_determinant = Calc_Vel_Determinant_From_NEES_CFMI_Determinants(determinants);

   /** \result
   * Compare returned value with the expected one
   */
   float32_t expected_velocity_determinant = 0.0F;
   DOUBLES_EQUAL(expected_velocity_determinant, velocity_determinant, tolerance);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_pos_diff_vel_estimate
*  @{
*/

/** \brief
*  Test group for all unittests for Pos_Diff_Vel_Estimate() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_pos_diff_vel_estimate)
{
   F360_VCS_Velocity_T velocity = {};
   Point point_A = {};
   Point point_B = {};
   float32_t dt = 0.0F;
};

/** \purpose Check if function returns false flag for invalid df value
* \req NA
*/
TEST(f360_nees_cfmi_helpers_pos_diff_vel_estimate, Test_Pos_Diff_Vel_Estimate_Check_False_Value_For_Invalid_Dt)
{
   /** \precond
   * Dt should get invalid value
   */
   dt = 0.0F;

   /** \action
   * Call tested Pos_Diff_Vel_Estimate unction
   */
   bool f_valid = Pos_Diff_Vel_Estimate(point_A, point_B, dt, velocity);

   /** \result
   * Checking if flag is invalid
   */
   CHECK_FALSE(f_valid);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_pos_diff_cov_estimate
*  @{
*/

/** \brief
*  Test group for all unittests for Pos_Diff_Cov_Estimate() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_pos_diff_cov_estimate)
{
   float32_t output_cov[2][2] = {};
   float32_t point_A_cov[2][2] = {};
   float32_t point_B_cov[2][2] = {};
   float32_t dt = 0.0F;
};

/** \purpose Check if function returns false flag for invalid df value
* \req NA
*/
TEST(f360_nees_cfmi_helpers_pos_diff_cov_estimate, Test_Pos_Diff_Cov_Estimate_Check_False_Value_For_Invalid_Dt)
{
   /** \precond
   * Dt should get invalid value
   */
   dt = 0.0F;

   /** \action
   * Call tested Pos_Diff_Cov_Estimate function
   */
   bool f_valid = Pos_Diff_Cov_Estimate(point_A_cov, point_B_cov, dt, output_cov);

   /** \result
   * Checking if flag is invalid
   */
   CHECK_FALSE(f_valid);
}
/** @}*/

/** \defgroup  f360_nees_cfmi_helpers_pos_diff_cov_estimate
*  @{
*/

/** \brief
*  Test group for all unittests for Check_Is_Cluster_In_All_Looks() function
**/
TEST_GROUP(f360_nees_cfmi_helpers_Check_Is_Cluster_In_All_Looks)
{
   F360_Cluster_T cluster_to_init = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Detection_Hist_T detection_hist = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
};

/** \purpose Check if function if cluster is in all looks if current det is in all looks
* \req NA
*/
TEST(f360_nees_cfmi_helpers_Check_Is_Cluster_In_All_Looks, Test_If_Cluster_In_All_Looks_If_Det_In_All_Looks)
{
   /** \precond
   * Adding detection to cluster that is in all looks
   */
   cluster_to_init.ndets = 1;
   cluster_to_init.detids[0] = 1;

   sensors[0].constant.range_limits[0] = 60.0F;
   sensors[0].constant.range_limits[1] = 70.0F;
   sensors[0].constant.range_limits[2] = 60.0F;
   sensors[0].constant.range_limits[3] = 70.0F;

   raw_detection_list.detections[0].raw.range = 10;
   raw_detection_list.detections[0].raw.sensor_id = 1;

   /** \action
   * Call tested Check_Is_Cluster_In_All_Looks function
   */
   bool is_cluster_in_all_looks = Check_If_Cluster_Has_Any_Det_In_All_Looks(cluster_to_init, raw_detection_list, sensors, detection_hist);

   /** \result
   * Checking if cluster is in all looks
   */
   CHECK_TRUE(is_cluster_in_all_looks);
}

/** \purpose Check if Check_Is_Cluster_In_All_Looks function return true if current det is in all looks
* \req NA
*/
TEST(f360_nees_cfmi_helpers_Check_Is_Cluster_In_All_Looks, Test_if_cluster_in_all_looks_if_hist_det_in_all_looks)
{
   /** \precond
   * Adding hist detection to cluster that is in all looks
   */
   cluster_to_init.num_old_dets = 1;
   cluster_to_init.old_det_idx[0] = 0;
   detection_hist.det_data[0].f_is_range_in_all_looks = true;

   /** \action
   * Call tested Check_Is_Cluster_In_All_Looks function
   */
   bool is_cluster_in_all_looks = Check_If_Cluster_Has_Any_Det_In_All_Looks(cluster_to_init, raw_detection_list, sensors, detection_hist);

   /** \result
   * Checking if cluster is in all looks
   */
   CHECK_TRUE(is_cluster_in_all_looks);
}

/** \purpose Check if Check_Is_Cluster_In_All_Looks function return true if current det is not in all looks
* \req NA
*/
TEST(f360_nees_cfmi_helpers_Check_Is_Cluster_In_All_Looks, Test_if_cluster_in_all_looks_if_hist_det_is_not_in_all_looks)
{
   /** \precond
   * Adding hist detection to cluster that is not in all looks
   */
   cluster_to_init.num_old_dets = 1;
   cluster_to_init.old_det_idx[0] = 0;
   detection_hist.det_data[0].f_is_range_in_all_looks = false;

   /** \action
   * Call tested Check_Is_Cluster_In_All_Looks function
   */
   bool is_cluster_in_all_looks = Check_If_Cluster_Has_Any_Det_In_All_Looks(cluster_to_init, raw_detection_list, sensors, detection_hist);

   /** \result
   * Checking if cluster is not in all looks
   */
   CHECK_FALSE(is_cluster_in_all_looks);
}
/** @}*/
