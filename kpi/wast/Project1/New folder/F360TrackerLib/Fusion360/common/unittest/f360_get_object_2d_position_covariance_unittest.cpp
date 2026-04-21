/** \file
 * This file contains unit tests for content of f360_get_object_2d_position_covariance.cpp file
 */

#include "f360_get_object_2d_position_covariance.h"
#include <CppUTest/TestHarness.h>

#include "f360_trk_fltr_ccv_states.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_matrix_dimension.h"

using namespace f360_variant_A;

/** \defgroup  f360_get_object_2d_position_covariance
 *  @{
 */

/** \brief
 * Simple test group that only contains the test pass/fail threshold definition.
 */
TEST_GROUP(f360_get_object_2d_position_covariance)
{
   // Declare threshold for test evaluation
   float32_t TEST_PASS_TH_SMALL = F360_EPSILON;

};

/**
*\purpose  Check if function returns the correct covariance matrix for CTCA tracks
*\req    NA
*/
TEST(f360_get_object_2d_position_covariance, Test_F360_Get_Object_2D_Position_Covariance_CTCA)
{

   /** \precond
   *Set up object with an arbitrary position error covariance
   **/
   F360_Object_Track_T obj = {};
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   obj.errcov[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_X] = 1.0F;
   obj.errcov[F360_TRK_FLTR_CTCA_STATE_X][F360_TRK_FLTR_CTCA_STATE_Y] = 2.0F;
   obj.errcov[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_X] = 3.0F;
   obj.errcov[F360_TRK_FLTR_CTCA_STATE_Y][F360_TRK_FLTR_CTCA_STATE_Y] = 4.0F;

   // Covariance returned
   float32_t covar[2][2] = {};

   // Expected covariance matrix
   float32_t exp_covar[2][2] = {};
   exp_covar[F360_2D_IDX_X][F360_2D_IDX_X] = 1.0F;
   exp_covar[F360_2D_IDX_X][F360_2D_IDX_Y] = 2.0F;
   exp_covar[F360_2D_IDX_Y][F360_2D_IDX_X] = 3.0F;
   exp_covar[F360_2D_IDX_Y][F360_2D_IDX_Y] = 4.0F;

   /** \action
   *Call function.
   **/
   F360_Get_Object_2D_Position_Covariance(obj, covar);

   /** \result
   *Ensure function output is equal to the expected result.
   **/
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_X][F360_2D_IDX_X], covar[F360_2D_IDX_X][F360_2D_IDX_X], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_X][F360_2D_IDX_Y], covar[F360_2D_IDX_X][F360_2D_IDX_Y], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_Y][F360_2D_IDX_X], covar[F360_2D_IDX_Y][F360_2D_IDX_X], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_Y][F360_2D_IDX_Y], covar[F360_2D_IDX_Y][F360_2D_IDX_Y], TEST_PASS_TH_SMALL);
}

/**
*\purpose  Check if function returns the correct covariance matrix for CCA tracks
*\req    NA
*/
TEST(f360_get_object_2d_position_covariance, Test_F360_Get_Object_2D_Position_Covariance_CCA)
{

   /** \precond
   *Set up object with an arbitrary position error covariance
   **/
   F360_Object_Track_T obj = {};
   obj.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   obj.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = 1.0F;
   obj.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] = 2.0F;
   obj.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] = 3.0F;
   obj.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = 4.0F;

   // Covariance returned
   float32_t covar[2][2] = {};

   // Expected covariance matrix
   float32_t exp_covar[2][2] = {};
   exp_covar[F360_2D_IDX_X][F360_2D_IDX_X] = 1.0F;
   exp_covar[F360_2D_IDX_X][F360_2D_IDX_Y] = 2.0F;
   exp_covar[F360_2D_IDX_Y][F360_2D_IDX_X] = 3.0F;
   exp_covar[F360_2D_IDX_Y][F360_2D_IDX_Y] = 4.0F;

   /** \action
   *Call function.
   **/
   F360_Get_Object_2D_Position_Covariance(obj, covar);

   /** \result
   *Ensure function output is equal to the expected result.
   **/
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_X][F360_2D_IDX_X], covar[F360_2D_IDX_X][F360_2D_IDX_X], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_X][F360_2D_IDX_Y], covar[F360_2D_IDX_X][F360_2D_IDX_Y], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_Y][F360_2D_IDX_X], covar[F360_2D_IDX_Y][F360_2D_IDX_X], TEST_PASS_TH_SMALL);
   DOUBLES_EQUAL(exp_covar[F360_2D_IDX_Y][F360_2D_IDX_Y], covar[F360_2D_IDX_Y][F360_2D_IDX_Y], TEST_PASS_TH_SMALL);
}

/** @}*/
