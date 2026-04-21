/** \file
 * This file contains unit tests for content of f360_compute_split_logic_signals.cpp file
 */

#include "f360_compute_split_logic_signals.h"
#include <CppUTest/TestHarness.h>

#include "f360_clear_detections_props.h"
#include "f360_clear_object_track.h"
#include "f360_math_func.h"
#include "f360_math.h"

using namespace f360_variant_A;

/** \defgroup  f360_compute_split_logic_signals
 *  @{
 */

/** \brief
 *  *  Test group for unit testing Compute_Split_Logic_Signals function.
 */
TEST_GROUP(f360_compute_split_logic_signals)
{
   // Initialize common variables used within all tests in this test group.
   const float32_t TOLERANCE = 0.0001F;
   F360_Detection_Props_T det_p[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Calibrations_T calibs = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Host_T host = {};

   /** \setup
    * Reset variables commonly used within all tests in this test group.
    */
   TEST_SETUP()
   {
      Clear_Detections_Props(det_p);
      Initialize_Tracker_Calibrations(calibs);

      for (uint32_t idx = 0U; idx < NUMBER_OF_OBJECT_TRACKS; idx++)
      {
         Clear_Object_Track(object_tracks[idx]);
         object_tracks[idx].Set_Bbox_Orientation(Angle{ 0.0F });
         object_tracks[idx].vcs_heading = Angle{ 0.0F };
      }
   }
};

/** \purpose  
 * Verify that Compute_Objects_Detections_Max_Gap() is computing the intended orth_gap_filtered given an object and an array of sorted orth positions (between radar detections' positions)
 * when there are 5 detections associated.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_And_Filter_Objects_Detections_Max_Gap_Verify_Calculation_Correctness_5dets)
{
   /** \precond
    * Set orth_sorted_pos such it the largest element is max_gap.
    * Compute the expected orth_gap_filtered signal (previous filtered value = 0.0)
    */
   F360_Object_Track_T& object = object_tracks[0];
   object.ndets = 5;
   object.orth_gap_filtered = 0.0F;

   float32_t orth_sorted_pos[MAX_DETS_IN_OBJ_TRK] = {};
   orth_sorted_pos[0] = -1.0F;
   orth_sorted_pos[1] = 1.0F;
   orth_sorted_pos[2] = 2.0F;

   float32_t expected_max_gap_filtered =0.6F;

   /** \action
    * Call Compute_And_Filter_Objects_Detections_Max_Gap().
    */
   Compute_And_Filter_Objects_Detections_Max_Gap(calibs, orth_sorted_pos, object);
   /** \result
    * Check that the output match expected data.
    */
   DOUBLES_EQUAL(expected_max_gap_filtered, object.orth_gap_filtered, TOLERANCE);
}

/** \purpose
 * Verify that Compute_Objects_Detections_Max_Gap() is computing the intended orth_gap_filtered given an object and an array of sorted orth positions (between radar detections' positions)
 * when there are 2 detections associated.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_And_Filter_Objects_Detections_Max_Gap_Verify_Calculation_Correctness_2dets)
{
   /** \precond
    * Set orth_sorted_pos such it the largest element is max_gap.
    * Compute the expected orth_gap_filtered signal (previous filtered value = 0.0)
    */
   F360_Object_Track_T& object = object_tracks[0];
   object.ndets = 2;
   object.orth_gap_filtered = 0.0F;

   float32_t orth_sorted_pos[MAX_DETS_IN_OBJ_TRK] = {};
   orth_sorted_pos[0] = -1.0F;
   orth_sorted_pos[1] = 1.0F;
   orth_sorted_pos[2] = 2.0F;

   float32_t expected_max_gap_filtered =0.24F;

   /** \action
    * Call Compute_And_Filter_Objects_Detections_Max_Gap().
    */
   Compute_And_Filter_Objects_Detections_Max_Gap(calibs, orth_sorted_pos, object);
   /** \result
    * Check that the output match expected data.
    */
   DOUBLES_EQUAL(expected_max_gap_filtered, object.orth_gap_filtered, TOLERANCE);
}

/** \purpose
 * Verify that Compute_Objects_Detections_Max_Gap() is computing the intended orth_gap_filtered given an object and an array of sorted orth positions (between radar detections' positions)
 * when there are more than 5 detections associated.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_And_Filter_Objects_Detections_Max_Gap_Verify_Calculation_Correctness_7dets)
{
   /** \precond
    * Set orth_sorted_pos such it the largest element is max_gap.
    * Compute the expected orth_gap_filtered signal (previous filtered value = 0.0)
    */
   F360_Object_Track_T& object = object_tracks[0];
   object.ndets = 7;
   object.orth_gap_filtered = 0.0F;

   float32_t orth_sorted_pos[MAX_DETS_IN_OBJ_TRK] = {};
   orth_sorted_pos[0] = -1.0F;
   orth_sorted_pos[1] = 1.0F;
   orth_sorted_pos[2] = 2.0F;

   float32_t expected_max_gap_filtered =0.6F;

   /** \action
    * Call Compute_And_Filter_Objects_Detections_Max_Gap().
    */
   Compute_And_Filter_Objects_Detections_Max_Gap(calibs, orth_sorted_pos, object);
   /** \result
    * Check that the output match expected data.
    */
   DOUBLES_EQUAL(expected_max_gap_filtered, object.orth_gap_filtered, TOLERANCE);
}

/** \purpose
 * Verify that Compute_Objects_Detections_Max_Delta() is computing the intended orth_delta_filtered given an object and sorted (radar detections') positions.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_And_Filter_Objects_Detections_Max_Delta_Verify_Calculation_Correctness)
{
   /** \precond
    * Set orth_sorted_pos such it has a total spread of max_delta.
    * Compute the expected delta filtered signal.
    */
   float32_t max_delta = 3;

   F360_Object_Track_T& object = object_tracks[0];
   object.ndets = 3;
   object.orth_delta_filtered = 0.0F;

   float32_t orth_sorted_pos[MAX_DETS_IN_OBJ_TRK] = {};
   orth_sorted_pos[0] = -0.5 * max_delta;
   orth_sorted_pos[1] = 0;
   orth_sorted_pos[2] = 0.5 * max_delta;

   float32_t expected_max_delta_filtered = F360_Low_Pass_Filter_First_Order(max_delta, object.orth_delta_filtered, calibs.k_orth_split_orth_delta_filter_const);

   /** \action
    * Call Compute_And_Filter_Objects_Detections_Max_Delta().
    */
   Compute_And_Filter_Objects_Detections_Max_Delta(calibs.k_orth_split_orth_delta_filter_const, orth_sorted_pos, object);
   /** \result
    * Check that the output match expected data.
    */
   DOUBLES_EQUAL(expected_max_delta_filtered, object.orth_delta_filtered, TOLERANCE);
}

/** \purpose
 * Verify that Derive_Object_Orth_Split_Signal_Status() will set a split_signals_status appropriately given different object positions and number of radar detections.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Derive_Object_Orth_Split_Signal_Status_Check_All_Split_Status_Flags)
{
   /** \precond
    * Set object_to_reset position to be farther away from host than calib.k_orth_split_max_distance_sq.
    * Set object_to_freeze_1 to have 2 detections and position to be closer to host than calib.k_orth_split_min_distance_sq.
    * Set object_to_freeze_2 to have 1 detection and its position to be calib.k_orth_split_min_distance_sq < obj_pos < calib.k_orth_split_max_distance_sq.
    * Set object_to_update to have have 2 detection and its position to be calib.k_orth_split_min_distance_sq < obj_pos < calib.k_orth_split_max_distance_sq.
    */

   F360_Object_Track_T& object_to_reset = object_tracks[0];
   F360_Object_Track_T& object_to_freeze_1 = object_tracks[1];
   F360_Object_Track_T& object_to_freeze_2 = object_tracks[2];
   F360_Object_Track_T& object_to_freeze_3 = object_tracks[3];
   F360_Object_Track_T& object_to_update = object_tracks[4];

   float32_t dist_sq_reset = 1.2 * calibs.k_orth_split_max_distance_sq;
   float32_t dist_sq_freeze = 0.8 * calibs.k_orth_split_min_distance_sq;
   float32_t dist_sq_update = calibs.k_orth_split_min_distance_sq + 0.5*(calibs.k_orth_split_max_distance_sq - calibs.k_orth_split_min_distance_sq);

   // Compute object.vcs_position.y such that they trigger the reset, freeze, and update flag
   host.dist_rear_axle_to_vcs_m = 3 / 0.6F;
   float32_t half_host_length = 0.6F * host.dist_rear_axle_to_vcs_m;

   // dist_sq = obj_long_pos_host_center^2 + obj.vcs_pos.lat^2
   float32_t obj_long_pos_host_center = object_to_reset.vcs_position.x + half_host_length;
   object_to_reset.vcs_position.y = F360_Sqrtf(dist_sq_reset - obj_long_pos_host_center*obj_long_pos_host_center);

   obj_long_pos_host_center = object_to_freeze_1.vcs_position.x + half_host_length;
   object_to_freeze_1.vcs_position.y = F360_Sqrtf(dist_sq_freeze - obj_long_pos_host_center*obj_long_pos_host_center);
   object_to_freeze_1.ndets = 2;

   obj_long_pos_host_center = object_to_freeze_2.vcs_position.x + half_host_length;
   object_to_freeze_2.vcs_position.y = F360_Sqrtf(dist_sq_update - obj_long_pos_host_center*obj_long_pos_host_center);
   object_to_freeze_2.ndets = 1;

   obj_long_pos_host_center = object_to_freeze_2.vcs_position.x + half_host_length;
   object_to_freeze_3.vcs_position.y = F360_Sqrtf(dist_sq_freeze - obj_long_pos_host_center*obj_long_pos_host_center);
   object_to_freeze_3.ndets = 1;

   obj_long_pos_host_center = object_to_update.vcs_position.x + half_host_length;
   object_to_update.vcs_position.y = F360_Sqrtf(dist_sq_update - obj_long_pos_host_center*obj_long_pos_host_center);
   object_to_update.ndets = 2;

   /** \action
    * Call Derive_Object_Orth_Split_Signal_Status().
    */
   F360_Object_Orth_Split_Signals_Status_Type_T orth_split_status_reset = Derive_Object_Orth_Split_Signal_Status(calibs, object_to_reset, host.dist_rear_axle_to_vcs_m);
   F360_Object_Orth_Split_Signals_Status_Type_T orth_split_status_freeze_1 = Derive_Object_Orth_Split_Signal_Status(calibs, object_to_freeze_1, host.dist_rear_axle_to_vcs_m);
   F360_Object_Orth_Split_Signals_Status_Type_T orth_split_status_freeze_2 = Derive_Object_Orth_Split_Signal_Status(calibs, object_to_freeze_2, host.dist_rear_axle_to_vcs_m);
   F360_Object_Orth_Split_Signals_Status_Type_T orth_split_status_freeze_3 = Derive_Object_Orth_Split_Signal_Status(calibs, object_to_freeze_3, host.dist_rear_axle_to_vcs_m);
   F360_Object_Orth_Split_Signals_Status_Type_T orth_split_status_update = Derive_Object_Orth_Split_Signal_Status(calibs, object_to_update, host.dist_rear_axle_to_vcs_m);

   /** \result
    * Check that the output match expected data.
    */
   CHECK_EQUAL(orth_split_status_reset, F360_RESET_SPLIT_SIGNALS);
   CHECK_EQUAL(orth_split_status_freeze_1, F360_FREEZE_SPLIT_SIGNALS);
   CHECK_EQUAL(orth_split_status_freeze_2, F360_FREEZE_SPLIT_SIGNALS);
   CHECK_EQUAL(orth_split_status_freeze_3, F360_FREEZE_SPLIT_SIGNALS);
   CHECK_EQUAL(orth_split_status_update, F360_UPDATE_SPLIT_SIGNALS);
}

/** \purpose
 * Test that function Compute_Split_Logic_Signals() resets/freezes/updates split logic signals as intended given some specific object positions and detections.
 * In this test most objects have filter type CTCA.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_Split_Logic_Signals_Verify_That_Signals_Are_Updated_As_Expected_CTCA)
{
   /** \precond
    * Setup 4 objects such that;
    * obj_idx 0 will trigger a reset.
    * obj_idx 1 will be irrelevant to the function (due to incorrect trk_fltr_type) and thus remain unchanged.
    * obj_idx 2 will trigger a freeze signals.
    * obj_idx 3 will trigger an split signal.
    */
   float32_t detections_max_delta_and_gap = 6.0F;
   float32_t initial_gap_delta_filtered = 1.0F;

   float32_t dist_sq_reset = 1.2 * calibs.k_orth_split_max_distance_sq;
   float32_t dist_sq_freeze = 0.8 * calibs.k_orth_split_min_distance_sq;
   float32_t dist_sq_update = calibs.k_orth_split_min_distance_sq + 0.5*(calibs.k_orth_split_max_distance_sq - calibs.k_orth_split_min_distance_sq);

   tracker_info.num_active_objs = 4;
   for (int32_t i = 0; i < tracker_info.num_active_objs ; i++)
   {
      tracker_info.active_obj_ids[i] = i+1;
      object_tracks[i].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      object_tracks[i].orth_delta_filtered = initial_gap_delta_filtered;
      object_tracks[i].orth_gap_filtered = initial_gap_delta_filtered;
   }
   object_tracks[1].trk_fltr_type = F360_TRACKER_TRKFLTR_CCV;

   // Set positions of objects
   host.dist_rear_axle_to_vcs_m = 0.0F;
   object_tracks[0].vcs_position.x = F360_Sqrtf(dist_sq_reset);
   object_tracks[1].vcs_position.x = F360_Sqrtf(dist_sq_reset);
   object_tracks[2].vcs_position.x = F360_Sqrtf(dist_sq_freeze);
   object_tracks[3].vcs_position.x = F360_Sqrtf(dist_sq_update);

   // Set positions of detections to center around object(s) that need them
   object_tracks[3].detids[0] = 1;
   object_tracks[3].detids[1] = 2;
   object_tracks[3].ndets = 2;
   det_p[object_tracks[3].detids[0]-1].vcs_position.x = object_tracks[3].vcs_position.x;
   det_p[object_tracks[3].detids[0]-1].vcs_position.y      = object_tracks[3].vcs_position.y-0.5*detections_max_delta_and_gap;
   det_p[object_tracks[3].detids[1]-1].vcs_position.x = object_tracks[3].vcs_position.x;
   det_p[object_tracks[3].detids[1]-1].vcs_position.y      = object_tracks[3].vcs_position.y+0.5*detections_max_delta_and_gap;

   // Set expected split signals for updated object(s).
   float32_t expected_max_delta_filtered = 2.0F;
   float32_t expected_max_gap_filtered = 1.6F;

   /** \action
    * Call Compute_Split_Logic_Signals().
    */
   Compute_Split_Logic_Signals(det_p, calibs, tracker_info, host.dist_rear_axle_to_vcs_m, object_tracks);
   /** \result
    * Check that the output match expected data.
    * object_tracks[0] should have its split signals reset (set to 0).
    * object_tracks[1] should have its split signals unchanged.
    * object_tracks[2] should have its split signals unchanged.
    * object_tracks[3] should have its split signals updated.
    */
   DOUBLES_EQUAL(0.0F, object_tracks[0].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(0.0F, object_tracks[0].orth_gap_filtered, TOLERANCE);

   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[1].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[1].orth_gap_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[2].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[2].orth_gap_filtered, TOLERANCE);

   DOUBLES_EQUAL(expected_max_delta_filtered, object_tracks[3].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(expected_max_gap_filtered, object_tracks[3].orth_gap_filtered, TOLERANCE);
}

/** \purpose
 * Test that function Compute_Split_Logic_Signals() resets/freezes/updates split logic signals as intended given some specific object positions and detections.
 * In this test, most objects have filter type CCA.
 * \req
 * NA.
 */
TEST(f360_compute_split_logic_signals, Compute_Split_Logic_Signals_Verify_That_Signals_Are_Updated_As_Expected_CCA)
{
   /** \precond
    * Setup 4 objects such that;
    * obj_idx 0 will trigger a reset.
    * obj_idx 1 will be irrelevant to the function (due to incorrect trk_fltr_type) and thus remain unchanged.
    * obj_idx 2 will trigger a freeze signals.
    * obj_idx 3 will trigger an split signal.
    */
   float32_t detections_max_delta_and_gap = 6.0F;
   float32_t initial_gap_delta_filtered = 1.0F;

   float32_t dist_sq_reset = 1.2 * calibs.k_orth_split_max_distance_sq;
   float32_t dist_sq_freeze = 0.8 * calibs.k_orth_split_min_distance_sq;
   float32_t dist_sq_update = calibs.k_orth_split_min_distance_sq + 0.5*(calibs.k_orth_split_max_distance_sq - calibs.k_orth_split_min_distance_sq);

   tracker_info.num_active_objs = 4;
   for (int32_t i = 0; i < tracker_info.num_active_objs ; i++)
   {
      tracker_info.active_obj_ids[i] = i+1;
      object_tracks[i].trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
      object_tracks[i].orth_delta_filtered = initial_gap_delta_filtered;
      object_tracks[i].orth_gap_filtered = initial_gap_delta_filtered;
   }
   object_tracks[1].trk_fltr_type = F360_TRACKER_TRKFLTR_CCV;

   // Set positions of objects
   host.dist_rear_axle_to_vcs_m = 0.0F;
   object_tracks[0].vcs_position.x = F360_Sqrtf(dist_sq_reset);
   object_tracks[1].vcs_position.x = F360_Sqrtf(dist_sq_reset);
   object_tracks[2].vcs_position.x = F360_Sqrtf(dist_sq_freeze);
   object_tracks[3].vcs_position.x = F360_Sqrtf(dist_sq_update);

   // Set positions of detections to center around object(s) that need them
   object_tracks[3].detids[0] = 1;
   object_tracks[3].detids[1] = 2;
   object_tracks[3].ndets = 2;
   det_p[object_tracks[3].detids[0]-1].vcs_position.x = object_tracks[3].vcs_position.x;
   det_p[object_tracks[3].detids[0]-1].vcs_position.y      = object_tracks[3].vcs_position.y-0.5*detections_max_delta_and_gap;
   det_p[object_tracks[3].detids[1]-1].vcs_position.x = object_tracks[3].vcs_position.x;
   det_p[object_tracks[3].detids[1]-1].vcs_position.y      = object_tracks[3].vcs_position.y+0.5*detections_max_delta_and_gap;

   // Set expected split signals for updated object(s).
   float32_t expected_max_delta_filtered = 2.0F;
   float32_t expected_max_gap_filtered = 1.6F;

   /** \action
    * Call Compute_Split_Logic_Signals().
    */
   Compute_Split_Logic_Signals(det_p, calibs, tracker_info, host.dist_rear_axle_to_vcs_m, object_tracks);
   /** \result
    * Check that the output match expected data.
    * object_tracks[0] should have its split signals reset (set to 0).
    * object_tracks[1] should have its split signals unchanged.
    * object_tracks[2] should have its split signals unchanged.
    * object_tracks[3] should have its split signals updated.
    */
   DOUBLES_EQUAL(0.0F, object_tracks[0].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(0.0F, object_tracks[0].orth_gap_filtered, TOLERANCE);

   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[1].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[1].orth_gap_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[2].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(initial_gap_delta_filtered, object_tracks[2].orth_gap_filtered, TOLERANCE);

   DOUBLES_EQUAL(expected_max_delta_filtered, object_tracks[3].orth_delta_filtered, TOLERANCE);
   DOUBLES_EQUAL(expected_max_gap_filtered, object_tracks[3].orth_gap_filtered, TOLERANCE);
}

/** @}*/

