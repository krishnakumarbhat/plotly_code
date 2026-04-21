/** \file
 * This file contains unit tests for content of f360_rotate_state_covariance_matrix.cpp file
 */

#include "f360_rotate_state_covariance_matrix.h"
#include <CppUTest/TestHarness.h>

#include <math.h>

#include "f360_trk_fltr_ctca_states.h"
#include "f360_trk_fltr_ccv_states.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_math.h"

 //#include "headerfile_needed.h"

 // Unit testing guidelines: http://confluenceprod1.delphiauto.net:8090/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA
 *  @{
 */

/** \brief
 * This is a test group for testing of the function Get_Rotation_Jacobian() when input
 * track filter type is CTCA.
 */
TEST_GROUP(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA)
{
   // Declare common variables
   F360_Trk_Fltr_Type_T trk_fltr_type;
   float32_t cos_rot_angle;
   float32_t sin_rot_angle;
   float32_t jacobian[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t expected_jacobian[STATE_DIMENSION][STATE_DIMENSION] = {};

   /** \setup
    * Setting up track filter type to be CTCA.
    * Setting up the deterministic part of the expected Jacobian
    */
   TEST_SETUP()
   {
      trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

      expected_jacobian[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_H] = 1.0F;
      expected_jacobian[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_C] = 1.0F;
      expected_jacobian[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S] = 1.0F;
      expected_jacobian[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_A] = 1.0F;
   }
};

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CTCA object when rotation angle is in the first
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA, Get_Rotation_Jacobian_CTCA_rot_in_first_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the first quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(0.4F);
   sin_rot_angle = F360_Sinf(0.4F);

   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CTCA object when rotation angle is in the second
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA, Get_Rotation_Jacobian_CTCA_rot_in_second_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the second quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(1.5F);
   sin_rot_angle = F360_Sinf(1.5F);

   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CTCA object when rotation angle is in the third
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA, Get_Rotation_Jacobian_CTCA_rot_in_third_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the third quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(3.6F);
   sin_rot_angle = F360_Sinf(3.6F);

   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CTCA object when rotation angle is in the fourth
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CTCA, Get_Rotation_Jacobian_CTCA_rot_in_fourth_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the fourth quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(4.9F);
   sin_rot_angle = F360_Sinf(4.9F);

   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}
/** @}*/


/** \defgroup  f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA
 *  @{
 */

/** \brief
 * This is a test group for testing of the function Get_Rotation_Jacobian() when the input
 * track filter type is CCA
 */
TEST_GROUP(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA)
{
   // Declare common variables
   F360_Trk_Fltr_Type_T trk_fltr_type;
   float32_t cos_rot_angle;
   float32_t sin_rot_angle;
   float32_t jacobian[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t expected_jacobian[STATE_DIMENSION][STATE_DIMENSION] = {};

   /** \setup
    * Setting up track filter type to be CCA.
    */
   TEST_SETUP()
   {
      trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   }
};

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CCA object when rotation angle is in the first
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA, Get_Rotation_Jacobian_CCA_rot_in_first_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the first quadrant.
    * Defining the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(0.4F);
   sin_rot_angle = F360_Sinf(0.4F);

   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CCA object when rotation angle is in the first
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA, Get_Rotation_Jacobian_CCA_rot_in_second_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the second quadrant.
    * Defining the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(1.5F);
   sin_rot_angle = F360_Sinf(1.5F);

   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = cos_rot_angle;

   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CCA object when rotation angle is in the third
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA, Get_Rotation_Jacobian_CCA_rot_in_third_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the third quadrant.
    * Defining the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(3.6F);
   sin_rot_angle = F360_Sinf(3.6F);

   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Get_Rotation_Jacobian() returns
 *  the expected Jacobian for a CCA object when rotation angle is in the fourth
 *  quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Get_Rotation_Jacobian_CCA, Get_Rotation_Jacobian_CCA_rot_in_fourth_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the fourth quadrant.
    * Defining the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(4.9F);
   sin_rot_angle = F360_Sinf(4.9F);

   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = cos_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = -sin_rot_angle;
   expected_jacobian[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = cos_rot_angle;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Get_Rotation_Jacobian(trk_fltr_type, cos_rot_angle, sin_rot_angle, jacobian);

   /** \result
    * Check that the output Jacobian match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         CHECK_EQUAL_TEXT(expected_jacobian[row][col], jacobian[row][col], "The output Jacobian does not match the expected data.")
      }
   }
}
/** @}*/



/** \defgroup  f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA
 *  @{
 */

/** \brief
 * This is a test group for testing of the function Rotate_State_Covariance_Matrix() when
 * input track filter type is CTCA
 */
TEST_GROUP(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA)
{

   // Declare common variables
   F360_Trk_Fltr_Type_T trk_fltr_type;
   float32_t cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t cos_rot_angle;
   float32_t sin_rot_angle;
   float32_t rotated_cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t expected_rotated_cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};

   float32_t tolerance;

   /** \setup
    * Setting up the covariance matrix and the track filter type.
    * Setting up the deterministic part of the expected output Jacobian
    * Setting up tolerance for checking equality between floats.
    */
   TEST_SETUP()
   {
      trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 6.71F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = 0.76F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H] = 0.68F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_X] = cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C] = 1.10F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_X] = cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S] = -1.10F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_X] = cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A] = -0.04F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_X] = cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 8.83F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H] = 1.55F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C] = 3.28F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S] = -3.56F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A] = -2.68F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_H] = 6.71F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_C] = 1.10F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_C];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_S] = 1.84F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_S];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_A] = 2.84F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_A];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_C] = 8.09F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_S] = -2.87F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_S];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_A] = -0.87F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_A];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S] = 7.27F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_A] = 1.07F;
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_S] = cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_A];
      cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_A] = 5.05F;

      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_H];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_C];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_C];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_S] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_S];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_S];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_A] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_H] = cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_C];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_S] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_S];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_S];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_A] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_C] = cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S] = cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_A] = cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_S] = cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_A];
      expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_A] = cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_A];

      tolerance = 1e-6F;
   }
};

/** \purpose
 * This test checks if the function Rotate_State_Covariance_Matrix() returns
 * the expected covariance for a CTCA object when rotation angle is in the first
 * quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA, Rotate_State_Covariance_Matrix_CTCA_rot_in_first_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the first quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(0.18F);
   sin_rot_angle = F360_Sinf(0.18F);

   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 7.045677784190459F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = 1.084692273266826F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H] = 0.946509549905950F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C] = 1.669445062903637F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S] = -1.719573343462868F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A] = -0.519153004492734F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 8.494322215809543F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H] = 1.403217613892028F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C] = 3.030074781576632F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S] = -3.305551015557306F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A] = -2.629539913735133F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A];

   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Rotate_State_Covariance_Matrix(trk_fltr_type, cov_mat, cos_rot_angle, sin_rot_angle, rotated_cov_mat);

   /** \result
    * Check that the output covariance matrix match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         DOUBLES_EQUAL_TEXT(expected_rotated_cov_mat[row][col], rotated_cov_mat[row][col], tolerance, "The output covariance matrix does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Rotate_State_Covariance_Matrix() returns
 * the expected covariance for a CTCA object when rotation angle is in the second
 * quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA, Rotate_State_Covariance_Matrix_CTCA_rot_in_second_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the second quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(2.9F);
   sin_rot_angle = F360_Sinf(2.9F);

   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 6.478251655687747F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = 0.180516522696820F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H] = -0.289415092020049F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C] = -0.283316181842687F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S] = 0.216326369662772F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A] = -0.602349875687489F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 9.061748344312255F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H] = -1.667674699847373F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C] = -3.447917043826037F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S] = 3.719785330067923F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A] = 2.611737855769462F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A];

   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Rotate_State_Covariance_Matrix(trk_fltr_type, cov_mat, cos_rot_angle, sin_rot_angle, rotated_cov_mat);

   /** \result
    * Check that the output covariance matrix match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         DOUBLES_EQUAL_TEXT(expected_rotated_cov_mat[row][col], rotated_cov_mat[row][col], tolerance, "The output covariance matrix does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Rotate_State_Covariance_Matrix() returns
 * the expected covariance for a CTCA object when rotation angle is in the third
 * quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA, Rotate_State_Covariance_Matrix_CTCA_rot_in_third_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the third quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(3.5F);
   sin_rot_angle = F360_Sinf(3.5F);

   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 7.470173425422376F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = 1.269371507942828F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H] = -1.180504550276652F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C] = -2.180671342841829F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S] = 2.278890646594923F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A] = 0.977557317699813F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 8.069826574577624F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H] = -1.212975270471793F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C] = -2.685716383855230F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S] = 2.947924256296653F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A] = 2.495672592831749F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A];

   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Rotate_State_Covariance_Matrix(trk_fltr_type, cov_mat, cos_rot_angle, sin_rot_angle, rotated_cov_mat);

   /** \result
    * Check that the output covariance matrix match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         DOUBLES_EQUAL_TEXT(expected_rotated_cov_mat[row][col], rotated_cov_mat[row][col], tolerance, "The output covariance matrix does not match the expected data.")
      }
   }
}

/** \purpose
 * This test checks if the function Rotate_State_Covariance_Matrix() returns
 * the expected covariance for a CTCA object when rotation angle is in the fourth
 * quadrant
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CTCA, Rotate_State_Covariance_Matrix_CTCA_rot_in_fourth_quadrant)
{
   /** \precond
    * Setting up the rotation angle to be in the fourth quadrant.
    * Defining the undeterministic part of the expected Jacobian output
    */
   cos_rot_angle = F360_Cosf(5.0F);
   sin_rot_angle = F360_Sinf(5.0F);

   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 8.245959776545119F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = -1.214356739640836F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H] = -1.293442339612871F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C] = -2.833243216885545F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S] = 3.101742013791224F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A] = 2.558570568678682F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_A];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 7.294040223454881F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H] = 1.091744894238935F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_H][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_H];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C] = 1.985228670448834F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_C][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_C];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S] = -2.064654082378538F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_S];
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A] = -0.798571628027972F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_A][F360_TRK_FLTR_CTCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_A];

   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Rotate_State_Covariance_Matrix(trk_fltr_type, cov_mat, cos_rot_angle, sin_rot_angle, rotated_cov_mat);

   /** \result
    * Check that the output covariance matrix match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         DOUBLES_EQUAL_TEXT(expected_rotated_cov_mat[row][col], rotated_cov_mat[row][col], tolerance, "The output covariance matrix does not match the expected data.")
      }
   }
}
/** @}*/


/** \defgroup  f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CCA
 *  @{
 */

/** \brief
 * This is a test group for testing of the function Rotate_State_Covariance_Matrix() when
 * input track filter type is CCA
 */
TEST_GROUP(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CCA)
{

   // Declare common variables
   F360_Trk_Fltr_Type_T trk_fltr_type;
   float32_t cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t cos_rot_angle;
   float32_t sin_rot_angle;
   float32_t rotated_cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t expected_rotated_cov_mat[STATE_DIMENSION][STATE_DIMENSION] = {};

   float32_t tolerance;

   /** \setup
    * Setting up the covariance matrix and the track filter type to CCA
    * Setting up tolerance for checking equality between floats.
    */
   TEST_SETUP()
   {
      trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = 10.1682F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VX] = -5.4370F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AX] = 7.4322F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = -4.1438F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VY] = 0.2407F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AY] = -10.3912F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_X] = cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VX];
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = 9.5900F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AX] = 1.8433F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_Y] = 3.1668F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = -2.5158F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AY] = 9.0877;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_X] = cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AX] ;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VX] = cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AX];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = 15.4084F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_Y] = -0.0900F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VY] = 3.5283F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = -0.3546F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y];
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VX] = cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_Y];
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AX] = cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_Y];
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = 7.4868F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VY] = 2.1847F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AY] = 8.0054F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_X] = cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AX] = cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = 8.6328F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AY] = 0.4575F;
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_X] = cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_VX] = cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_Y] = cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_VY] = cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AY];
      cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = 22.5249F;

      tolerance = 1e-6F;
   }
};

/** \purpose
 * This test checks if the function Rotate_State_Covariance_Matrix() returns
 * the expected covariance
 * \req
 * NA.
 */
TEST(f360_rotate_state_covariance_matrix_Rotate_State_Covariance_Matrix_CCA, Rotate_State_Covariance_Matrix_CCA)
{
   /** \precond
    * Setting up the rotation angle to be 0.18rad.
    * Defining the expected output to be R * P * R' where R corresponds to the rotation matrix
    */
   cos_rot_angle = F360_Cosf(0.18F);
   sin_rot_angle = F360_Sinf(0.18F);

   float32_t expected_rotated_cov_mat[STATE_DIMENSION][STATE_DIMENSION];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = 8.622502903659690F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VX] = -4.592525185570625F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AX] = 5.604443623432468F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = -4.350463322508540F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VY] = 1.473948475217658F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AY] = -9.954299098909944F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VX];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = 8.673068703738789F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AX] = 4.021028772625709F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_Y] = 4.400048475217658F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] = -2.523127677054407F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AY] = 8.439246347524104F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AX] ;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AX];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = 15.511578684028642F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_Y] = 0.346900901090055F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VY] = 2.879846347524104F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY] = 0.921610776874893F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_Y];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = 9.032497096340309F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VY] = 1.340225185570624F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AY] = 9.833156376567533F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_VY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_VY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_VY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = 9.549731296261212F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AY] = -1.720228772625709F;
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_X] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_AY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_VX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_AY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AX] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_Y] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_AY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_VY] = expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_AY];
   expected_rotated_cov_mat[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = 22.421721315971357F;


   /** \action
    * Call Get_Rotation_Jacobian()
    */
   Rotate_State_Covariance_Matrix(trk_fltr_type, cov_mat, cos_rot_angle, sin_rot_angle, rotated_cov_mat);

   /** \result
    * Check that the output covariance matrix match expected data
    */
   for (uint8_t row = 0U; row < STATE_DIMENSION; row++)
   {
      for (uint8_t col = 0U; col < STATE_DIMENSION; col++)
      {
         DOUBLES_EQUAL_TEXT(expected_rotated_cov_mat[row][col], rotated_cov_mat[row][col], tolerance, "The output covariance matrix does not match the expected data.")
      }
   }
}
/** @}*/