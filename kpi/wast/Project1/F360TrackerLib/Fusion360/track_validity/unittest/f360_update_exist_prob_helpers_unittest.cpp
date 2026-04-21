/** \file
 * This file contains unit tests for content of f360_update_exist_prob_helpers.cpp file
 */

#include "f360_update_exist_prob_helpers.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_trk_fltr_ccv_states.h"
#include <CppUTest/TestHarness.h>
#include "f360_tracker_info.h"

using namespace f360_variant_A;

/** \defgroup  f360_update_exist_prob_helpers_functions
 *  @{
 */

 /** \brief
 * Test group of f360_update_exist_prob_helpers_functions. Tests verify sub functions used inside
 * Update_Existence_Probability().
 **/
TEST_GROUP(f360_update_exist_prob_helper_functions)
{
   /** \setup
   * Set up common variables
   **/
   F360_Tracker_Info_T tracker_info = {};
   F360_Calibrations_T calibrations = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Object_Track_T& object = object_tracks[0];

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};


/**
*\purpose  Purpose of this test is to verify whether new updated object track has p_track_state equal to calibration value.
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Calculate_P_Track_State__if_p_track_state_set_to_calib_value)
{
   /** \precond
   * Set object status to F360_OBJECT_STATUS_NEW_UPDATED
   **/
   const float32_t expected_p_track_state = calibrations.k_ep_prob_track_state_init_value;
   object.status = F360_OBJECT_STATUS_NEW_UPDATED;

   /** \action
   * Call Calculate_P_Track_State
   **/
   const float32_t p_track_state = Calculate_P_Track_State(object, calibrations);

   /** \result
   * Check whether object p_track_state is equal to calibration value
   **/
   DOUBLES_EQUAL(expected_p_track_state, p_track_state, F360_EPSILON)
}

/**
*\purpose  Purpose of this test is to verify whether a CTCA track has the correct p_track_state
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Calculate_P_Track_State__CTCA_object)
{
   /** \precond
   * Set object status to F360_OBJECT_STATUS_UPDATED
   * Set object filter typ to CTCA
   * Fill errcov with 1s
   **/
   const float32_t expected_p_track_state = 0.583599091F;
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

   for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
   {
      for (uint32_t j = 0U; j < STATE_DIMENSION; j++)
      {
         object.errcov[i][j] = 1.0F;
      }
   }

   /** \action
   * Call Calculate_P_Track_State
   **/
   const float32_t p_track_state = Calculate_P_Track_State(object, calibrations);

   /** \result
   * Check whether object p_track_state is equal to the expected value
   **/
   DOUBLES_EQUAL(expected_p_track_state, p_track_state, F360_EPSILON)
}

/**
*\purpose  Purpose of this test is to verify whether CCA track has the correct p_track_state
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Calculate_P_Track_State__CCA_object)
{
   /** \precond
   * Set object status to F360_OBJECT_STATUS_UPDATED
   * Set object filter typ to CCA
   * Fill errcov with 1s
   **/
   const float32_t expected_p_track_state = 0.6345649F;
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

   for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
   {
      for (uint32_t j = 0U; j < STATE_DIMENSION; j++)
      {
         object.errcov[i][j] = 1.0F;
      }
   }

   /** \action
   * Call Calculate_P_Track_State
   **/
   const float32_t p_track_state = Calculate_P_Track_State(object, calibrations);

   /** \result
   * Check whether object p_track_state is equal to the correct value
   **/
   DOUBLES_EQUAL(expected_p_track_state, p_track_state, F360_EPSILON)
}

/**
*\purpose  Purpose of this test is to verify whether state variance is normalized for CCA model.
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Normalize_State_Variances__if_state_variance_is_calculated_for_CCA)
{
   /** \precond
   * Set object motion model to CCA and provide other data necessary for calculatins.
   **/
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   float32_t s2[STATE_DIMENSION] = {};

   object.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] = 4.0F;
   object.errcov[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] = 2.25F;
   object.errcov[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = 1.0F;
   object.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] = 4.0F;
   object.errcov[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] = 2.25F;
   object.errcov[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = 1.0F;
   const uint32_t actual_state_dimension = static_cast<uint32_t>(STATE_DIMENSION);

   /** \action
   * Call Normalize_State_Variances
   **/
   Normalize_State_Variances(object, calibrations, actual_state_dimension, s2);

   /** \result
   * Check whether object state variance values are qual to expected.
   **/
   DOUBLES_EQUAL(1.0F, s2[0], F360_EPSILON);
   DOUBLES_EQUAL(1.0F, s2[1], F360_EPSILON);
   DOUBLES_EQUAL(1.0F, s2[2], F360_EPSILON);
   DOUBLES_EQUAL(1.0F, s2[3], F360_EPSILON);
   DOUBLES_EQUAL(1.0F, s2[4], F360_EPSILON);
   DOUBLES_EQUAL(1.0F, s2[5], F360_EPSILON); 
}

/**
*\purpose  Purpose of this test is to verify whether low denominator value in function Normalize_State() is handled.
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Normalize_State_Variances__if_low_denominator_is_handle)
{
   /** \precond
   * Set the data to get the branch wherein denominator values colse to zero are handled.
   **/
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
   float32_t s2[STATE_DIMENSION] = {}; // vector of state variances
   float32_t s2_th[STATE_DIMENSION] = { 1.0F,1.0F,1.0F,1.0F,1.0F,1.0F }; // initial state variance thresholds
   const uint32_t actual_state_dimension = static_cast<uint32_t>(STATE_DIMENSION);

   s2_th[0] = F360_MIN_DENOMINATOR * 0.5F;
   s2_th[1] = F360_MIN_DENOMINATOR * 0.5F;
   s2_th[2] = F360_MIN_DENOMINATOR * 0.5F;
   s2_th[3] = F360_MIN_DENOMINATOR * 0.5F;
   s2_th[4] = F360_MIN_DENOMINATOR * 0.5F;
   s2_th[5] = F360_MIN_DENOMINATOR * 0.5F;

   /** \action
   * Call Normalize_State
   **/
   Normalize_State(s2, s2_th, actual_state_dimension);

   /** \result
   * Check whether object state variance values are set to large  values.
   **/
   DOUBLES_EQUAL(INFTY, s2[0], F360_EPSILON);
   DOUBLES_EQUAL(INFTY, s2[1], F360_EPSILON);
   DOUBLES_EQUAL(INFTY, s2[2], F360_EPSILON);
   DOUBLES_EQUAL(INFTY, s2[3], F360_EPSILON);
   DOUBLES_EQUAL(INFTY, s2[4], F360_EPSILON);
   DOUBLES_EQUAL(INFTY, s2[5], F360_EPSILON);
}


/**
*\purpose  Purpose of this test is to verify whether Normalize_Information() is handled in the normal
* case where denominator is large enough so that we don't have to worry about division with zero
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Normalize_Information_default)
{
   /** \precond
   * Set up an array representing the information. Let it be larger than 
   * calibs.k_ep_bottom_saturation_of_normalized_variance for all elements except the last.
   * the last element  is smaller than calibs.k_ep_bottom_saturation_of_normalized_variance so that
   * we can test the denominator saturation case.
   **/
   const float32_t s2[STATE_DIMENSION] = {0.5F, 0.1F, 1.0F, 2.0F, 5.0F, 0.5F * calibrations.k_ep_bottom_saturation_of_normalized_variance};
   const uint32_t actual_state_dimension = static_cast<uint32_t>(STATE_DIMENSION);

   /** \action
   * Call Normalize_State
   **/
  float32_t output_normalized_information = 0.0F;
   Normalize_Information(s2, calibrations, actual_state_dimension, output_normalized_information);

   /** \result
   * Check whether object state variance values are set to large  values.
   **/
  const float32_t expected_normalized_information = 1.0F / s2[0] + 1.0F / s2[1] + 1.0F / s2[2] + 1.0F / s2[3] + 1.0F / s2[4] + 1.0F / calibrations.k_ep_bottom_saturation_of_normalized_variance;    
  DOUBLES_EQUAL(expected_normalized_information, output_normalized_information, F360_EPSILON);
}

/**
*\purpose  Purpose of this test is to verify whether Normalize_Information() is handled for the case
* case where denominator is very small so that have to protect ourselves aginst division with zero.
*\req    NA.
*/
TEST(f360_update_exist_prob_helper_functions, Normalize_Information_protection_against_zero_division)
{
   /** \precond
   * Change calibs.k_ep_bottom_saturation_of_normalized_variance such that it is smaller than F360_MIN_DENOMINATOR
   * Set up an array representing the information. Let it be larger than 
   * calibs.k_ep_bottom_saturation_of_normalized_variance for all elements except the last.
   * the last element is smaller than calibs.k_ep_bottom_saturation_of_normalized_variance so that
   * we can test the case of a very small denominator.
   **/
  calibrations.k_ep_bottom_saturation_of_normalized_variance = F360_MIN_DENOMINATOR * 0.5F;
  const float32_t s2[STATE_DIMENSION] = {0.5F, 0.1F, 1.0F, 2.0F, 5.0F, 0.5F * calibrations.k_ep_bottom_saturation_of_normalized_variance};
  const uint32_t actual_state_dimension = static_cast<uint32_t>(STATE_DIMENSION);

   /** \action
   * Call Normalize_State
   **/
  float32_t output_normalized_information = 0.0F;
   Normalize_Information(s2, calibrations, actual_state_dimension, output_normalized_information);

   /** \result
   * Check whether object state variance values are set to large  values.
   **/ 
  DOUBLES_EQUAL(INFTY, output_normalized_information, F360_EPSILON);
}

/** @}*/
