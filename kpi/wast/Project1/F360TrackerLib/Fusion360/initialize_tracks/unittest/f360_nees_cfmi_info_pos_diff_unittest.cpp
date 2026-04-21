/** \file
 * This file contains unit tests for content of f360_nees_cfmi_info_pos_diff.cpp file
 */

#include "f360_nees_cfmi_info_pos_diff.h"
#include <CppUTest/TestHarness.h>
#include "f360_iterator.h"

using namespace f360_variant_A;

/** \defgroup  check_pos_diff_preconditions
*  @{
*/

/** \brief
*  Test Check_Dets_Pos_Diff_Preconditions function.
**/
TEST_GROUP(check_pos_diff_preconditions)
{
   F360_Detection_Time_Slots_T det_hist_time_slots;

   /** \setup
   * Setup input arguments as default.
   **/
   TEST_SETUP()
   {
      det_hist_time_slots = {};
   }

};

/**
*\purpose  Check Check_Dets_Pos_Diff_Preconditions for too low number of active slots.
*\req   NA
*/
TEST(check_pos_diff_preconditions, Check_Dets_Pos_Diff_Preconditions_Low_Num_Active_Slots)
{
   /** \precond
   * Set all properties to met preconditions.
   **/
   det_hist_time_slots.number_of_active_slots = 0U;

   /** \action
   * Call testing function.
   **/
   const bool preconditions_valid = Check_Dets_Pos_Diff_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/**
*\purpose  Check Check_Dets_Pos_Diff_Preconditions for valid number of active slots.
*\req   NA
*/
TEST(check_pos_diff_preconditions, Check_Dets_Pos_Diff_Preconditions_Valid_Num_Active_Slots)
{
   /** \precond
   * Set all properties to met preconditions.
   **/
   det_hist_time_slots.number_of_active_slots = 3U;

   /** \action
   * Call testing function.
   **/
   const bool preconditions_valid = Check_Dets_Pos_Diff_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should be met.
   **/
   CHECK_TRUE(preconditions_valid)
}

/**
*\purpose  Check Check_Dets_Pos_Diff_Preconditions for too high number of active slots.
*\req   NA
*/
TEST(check_pos_diff_preconditions, Check_Dets_Pos_Diff_Preconditions_High_Num_Active_Slots)
{
   /** \precond
   * Set all properties to met preconditions.
   **/
   det_hist_time_slots.number_of_active_slots = F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS + 1U;

   /** \action
   * Call testing function.
   **/
   const bool preconditions_valid = Check_Dets_Pos_Diff_Preconditions(det_hist_time_slots);

   /** \result
   * Preconditions should not be met.
   **/
   CHECK_FALSE(preconditions_valid)
}

/** \defgroup  calculate_single_det_plausibility_for_vh
*  @{
*/

/** \brief
*  Test group for testing Calculate_Single_Det_Plausibility_For_VH function.
**/
TEST_GROUP(calculate_single_det_plausibility_for_vh)
{
   F360_VCS_Velocity_T vel_hyp = {};
   float32_t vel_hyp_cov[2][2] = {};
   float32_t range_rate_comp = 0.0;
   float32_t range_rate_comp_var = 0.0;
   float32_t cos_azimuth = 0.0;
   float32_t sin_azimuth = 0.0;
   float32_t sigma_squered_thr = 0.0;

   float32_t tolerance = 1e-3;
};

/**
*\purpose  Basic test for checking if range_rate_comp_var has invalid value
*\req   NA
*/
TEST(calculate_single_det_plausibility_for_vh, Check_Calculate_Single_Det_Plausibility_For_VH_Invalid_Range_Rate_Comp_Var)
{
   /** \step{1}
   * Using default values 
   **/

   /** \action
   * Call tested function.
   **/
   float32_t plausibility = Calculate_Single_Det_Plausibility_For_VH(vel_hyp, vel_hyp_cov, range_rate_comp, range_rate_comp_var, cos_azimuth,
      sin_azimuth, sigma_squered_thr);

   /** \result
   * Preconditions should not be met.
   **/
   DOUBLES_EQUAL(plausibility, F360_MIN_PROBABILITY, tolerance);
}

/** @}*/


/** \defgroup  get_nees_cfmi_information_pos_dets
*  @{
*/

/** \brief
*  Test group for testing Get_NEES_CFMI_Information_Pos_Diff_Current_Dets and Get_NEES_CFMI_Information_Pos_Diff_Old_Dets functions.
**/
TEST_GROUP(get_nees_cfmi_information_pos_dets)
{
   F360_Calibrations_T calibrations;
   F360_Detection_Time_Slots_T det_hist_time_slots = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};

   float32_t tolerance = 1e-3;

   /** \setup
   * Setup calibrations.
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/**
*\purpose  Checking Get_NEES_CFMI_Information_Pos_Diff_Current_Dets behavior when number of position based velocities saturates to maximum and detections
* are split to two time slots.
*\req   NA
*/
TEST(get_nees_cfmi_information_pos_dets, Number_Of_Caluclated_Velocities_Saturate_Two_Slots)
{
   /** \precond
   * Setting up two detections, for vel_successful_estimation false, cov_successful_estimation false, determinant invalid
   **/
   det_hist_time_slots.number_of_active_slots = 2U;
   det_hist_time_slots.time_since_meas_slots[0].number_of_active_detections = 8U;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = 8U;

   float32_t pos_cov[2][2] = { {1.0F, 0.0F}, {0.0F, 1.0F} };
   for (uint32_t det_index = 0U; det_index < 8U; det_index++)
   {
      // Update first slot
      det_hist_time_slots.time_since_meas_slots[0].hist_dets_idx[det_index] = det_index;
      nees_cfmi_information.detections[det_index].time_since_meas = 1.0F;
      std::copy(cmn::begin(pos_cov), cmn::end(pos_cov), cmn::begin(nees_cfmi_information.detections[det_index].pos_cov));

      // Update second slot
      det_hist_time_slots.time_since_meas_slots[1].hist_dets_idx[det_index] = det_index + 8U;
      nees_cfmi_information.detections[det_index + 8U].time_since_meas = 1.05F;
      std::copy(cmn::begin(pos_cov), cmn::end(pos_cov), cmn::begin(nees_cfmi_information.detections[det_index + 8U].pos_cov));
   }

   /** \action
   * Call tested function.
   **/
   Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities(calibrations, det_hist_time_slots, nees_cfmi_information);

   /** \result
   * Number of position based velocities should not exceed allowed maximum.
   **/
   CHECK_EQUAL(F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL, nees_cfmi_information.vels_num);

}

/**
*\purpose  Checking Get_NEES_CFMI_Information_Pos_Diff_Current_Dets behavior when number of position based velocities saturates to maximum and detections
* are split to three time slots.
*\req   NA
*/
TEST(get_nees_cfmi_information_pos_dets, Number_Of_Caluclated_Velocities_Saturate_Three_Slots)
{
   /** \precond
   * Setting up three detections slots, each is filled with 8 detections.
   **/
   det_hist_time_slots.number_of_active_slots = 3U;
   uint32_t dets_count_in_slot = 8U;
   det_hist_time_slots.time_since_meas_slots[0].number_of_active_detections = dets_count_in_slot;
   det_hist_time_slots.time_since_meas_slots[1].number_of_active_detections = dets_count_in_slot;
   det_hist_time_slots.time_since_meas_slots[2].number_of_active_detections = dets_count_in_slot;
   
   float32_t pos_cov[2][2] = { {1.0F, 0.0F}, {0.0F, 1.0F} };
   for (uint32_t det_index = 0U; det_index < 8U; det_index++)
   {
      // Update slots
      for (uint32_t slot_index = 0U; slot_index < det_hist_time_slots.number_of_active_slots; slot_index++)
      {
         uint32_t det_props_index = det_index + dets_count_in_slot * slot_index;
         det_hist_time_slots.time_since_meas_slots[slot_index].hist_dets_idx[det_index] = det_index + det_props_index;
         nees_cfmi_information.detections[det_props_index].time_since_meas = 1.0F + 0.05 * slot_index;
         std::copy(cmn::begin(pos_cov), cmn::end(pos_cov), cmn::begin(nees_cfmi_information.detections[det_props_index].pos_cov));
      }
   }
   /** \action
   * Call tested function.
   **/
   Get_NEES_CFMI_Information_Dets_Pos_Diff_Velocities(calibrations, det_hist_time_slots, nees_cfmi_information);
   
   /** \result
   * Number of position based velocities should not exceed allowed maximum.
   **/
   CHECK_EQUAL(F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL, nees_cfmi_information.vels_num);

}
/** @}*/
