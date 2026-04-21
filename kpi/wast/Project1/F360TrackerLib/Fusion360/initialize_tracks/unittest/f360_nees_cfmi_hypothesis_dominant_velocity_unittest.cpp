/** \file
 * This file contains unit tests for content of f360_nees_cfmi_hypothesis_dominant_velocity.cpp file
 */

#include "f360_nees_cfmi_hypothesis_dominant_velocity.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  Check_Dominant_Velocity_Preconditions_tests
*  @{
*/

/** \brief
*  Test Check_Dominant_Velocity_Preconditions function.
**/
TEST_GROUP(Check_Dominant_Velocity_Preconditions_tests)
{
   F360_NEES_CFMI_Information_T nees_cfmi_information;
   F360_Detection_Time_Slots_T det_hist_time_slots;

   /** \setup
   * Setup nees_cfmi_information and det_hist_time_slots.
   **/
   TEST_SETUP()
   {
      nees_cfmi_information = {};
      det_hist_time_slots = {};
   }
};

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when number of active slots is greater than 
*          max number of available slots. 
*\req   NA
*/
TEST(Check_Dominant_Velocity_Preconditions_tests, Check_Dominant_Velocity_Preconditions_Number_Of_Active_Slots_Above_Max_Value)
{
   /** \precond
    * Set number of active slots above max value.
    **/
   det_hist_time_slots.number_of_active_slots = F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS + 1U;

   /** \action
   * Call Check_Dominant_Velocity_Preconditions().
   **/
   const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when number of active slots is equal to 0.
*\req   NA
*/
TEST(Check_Dominant_Velocity_Preconditions_tests, last_slot_index_is_not_valid)
{
   /** \precond
    * Set number of active slots to 0.
    **/
   det_hist_time_slots.number_of_active_slots = 0U;

   /** \action
   * Call Check_Dominant_Velocity_Preconditions().
   **/
   const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when number of active detections in last historical detections slot is equal to 0.
*\req   NA
*/
TEST(Check_Dominant_Velocity_Preconditions_tests, number_of_active_dets_is_0)
{
   /** \precond
   * Set number of active detections for slot with index 0 to be 0.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = 0U;

   /** \action
   * Call Check_Dominant_Velocity_Preconditions().
   **/
   const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when number of active detections in last historical detections exceeds maximum value.
*\req   NA
*/
TEST(Check_Dominant_Velocity_Preconditions_tests, number_of_active_dets_is_over_max)
{
   /** \precond
   * Set number of active detections for slot with index 0 to be over max value.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT + 1U;

   /** \precond
   * Call Check_Dominant_Velocity_Preconditions().
   **/
   const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when all conditions are met.
*\req   NA
*/
TEST(Check_Dominant_Velocity_Preconditions_tests, precondtions_valid)
{
   /** \precond
   * Set all properties to met preconditions.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[det_hist_time_slots.number_of_active_slots - 1U].number_of_active_detections = F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT - 1U;

   /** \action
   * Call Check_Dominant_Velocity_Preconditions().
   **/
   const bool preconditions_valid = Check_Dominant_Velocity_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should be met.
   **/
   CHECK_TRUE(preconditions_valid)
}

/** @}*/

/** \defgroup  get_nees_cfmi_information_dominant_velocity
*  @{
*/

/** \brief
*  Test group for testing Get_NEES_CFMI_Information_Dominant_Velocity function.
**/
TEST_GROUP(get_nees_cfmi_information_dominant_velocity)
{
   F360_Calibrations_T calibrations;
   F360_Detection_Hist_T detection_hist = {};
   F360_Detection_Time_Slots_T det_hist_time_slots = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};

   void Add_Detection(float32_t time_since_meas, uint32_t det_index, bool f_historical, Point det_position)
   {
      uint32_t& nees_det_index = nees_cfmi_information.dets_num;
      F360_NEES_CFMI_Detection_T nees_det = {};
      nees_det.pos = det_position;
      nees_det.time_since_meas = time_since_meas;
      nees_det.idx = { det_index, nees_det_index, f_historical };

      nees_cfmi_information.detections[nees_det_index] = nees_det;
      nees_det_index++;
      if (f_historical)
      {
         nees_cfmi_information.old_dets_count++;
      }
      else
      {
         nees_cfmi_information.current_dets_count++;
      }
   }

   TEST_SETUP()
   {
      //Initialize calibrations.
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/**
*\purpose  Check behavior of Get_NEES_CFMI_Information_Dominant_Velocity function when historical detection in slot has 
*          index value greater than max index value.
*\req   NA
*/
TEST(get_nees_cfmi_information_dominant_velocity, Get_NEES_CFMI_Information_Dominant_Velocity_check)
{
   /** \precond
   * Set all properties.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT - 1U;
   det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[0] = MAX_NUMBER_OF_HISTORIC_DETECTIONS;

   /** \action
   * Call Get_NEES_CFMI_Information_Dominant_Velocity().
   **/
   Get_NEES_CFMI_Information_Dominant_Velocity(calibrations, det_hist_time_slots, nees_cfmi_information);

   /** \result
   * Preconditions should fail.
   **/
   CHECK_FALSE(nees_cfmi_information.dominant_velocity.f_valid);
}

/**
*\purpose  Check behavior of Get_NEES_CFMI_Information_Dominant_Velocity function when no time difference is found.
*\req   NA
*/
TEST(get_nees_cfmi_information_dominant_velocity, Get_NEES_CFMI_Information_Dominant_Velocity_Vel_Estimation_Fails)
{
   /** \precond
   * Set all properties.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT - 1U;
   nees_cfmi_information.center_time_since_meas = 0.0F;


   /** \action
   * Call Get_NEES_CFMI_Information_Dominant_Velocity().
   **/
   Get_NEES_CFMI_Information_Dominant_Velocity(calibrations, det_hist_time_slots, nees_cfmi_information);

   /** \result
   * Preconditions should fail.
   **/
   CHECK_FALSE(nees_cfmi_information.dominant_velocity.f_valid);
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when covariance determinant is invalid.
*\req   NA
*/
TEST(get_nees_cfmi_information_dominant_velocity, Get_NEES_CFMI_Information_Dominant_Velocity_Invalid_Vel_Determinant)
{
   /** \precond
   * Set all properties.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = 2U;
   nees_cfmi_information.center_time_since_meas = 0.0F;

   Point det_1_pos = { 1.0F, 1.0F };
   Point det_2_pos = { 1.0F, 1.0F };

   float32_t dets_time_since_meas = 1.0F;
   bool is_historical = true;

   Add_Detection(dets_time_since_meas, 0, is_historical, det_1_pos);
   det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[0] = 0;

   Add_Detection(dets_time_since_meas, 1, is_historical, det_2_pos);
   det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[0] = 1;

   calibrations.k_nees_cfmi_extended_object_pos_std = 0.01F;

   /** \action
   * Call Get_NEES_CFMI_Information_Dominant_Velocity().
   **/
   Get_NEES_CFMI_Information_Dominant_Velocity(calibrations, det_hist_time_slots, nees_cfmi_information);

   /** \result
   * Preconditions should fail.
   **/
   CHECK_FALSE(nees_cfmi_information.dominant_velocity.f_valid);
}

/**
*\purpose  Check behavior of Check_Dominant_Velocity_Preconditions function when covariance estimate is invalid.
*\req   NA
*/
TEST(get_nees_cfmi_information_dominant_velocity, Get_NEES_CFMI_Information_Dominant_Velocity_Invalid_Cov_Estimate)
{
   /** \precond
   * Set all properties.
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = 2U;
   nees_cfmi_information.center_time_since_meas = 0.0F;

   Point det_1_pos = { 1.0F, 1.0F };
   Point det_2_pos = { 1.0F, 1.0F };

   float32_t dets_time_since_meas = 0.0F;
   bool is_historical = true;

   Add_Detection(dets_time_since_meas, 0, is_historical, det_1_pos);
   det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[0] = 0;

   Add_Detection(dets_time_since_meas, 1, is_historical, det_2_pos);
   det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[0] = 1;

   /** \action
   * Call Get_NEES_CFMI_Information_Dominant_Velocity().
   **/
   Get_NEES_CFMI_Information_Dominant_Velocity(calibrations, det_hist_time_slots, nees_cfmi_information);

   /** \result
   * Preconditions should fail.
   **/
   CHECK_FALSE(nees_cfmi_information.dominant_velocity.f_valid);
}

/** @}*/
