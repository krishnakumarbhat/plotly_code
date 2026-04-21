/** \file
 * This file contains unit tests for content of f360_object_underdrivability_classification_qt.cpp file
 */

#include "f360_object_underdrivability_classification.h"
#include "f360_track_classification.h"
#include <CppUTest/TestHarness.h>
#include "ocg_underdrivability_enum.h"
#include "ocg_constants.h"
#include <iostream>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_object_underdrivability_classification_qt
 *  @{
 */

/** \brief
 * This test group will test the intended functionality of Object_Underdrivability_Classification().
 * Tests are designed to check
 *    - That loop over objects are functioning correctly such that all active objects get their status updated but not inactive objects
 *    - That correct classification method is used for moving and stationary objects respectively
 *    - That stationary objects are not classified when occupancy grid is invalid
 */
TEST_GROUP(f360_object_underdrivability_classification_qt)
{
   // Declare common variables used within all tests in this test group.
   F360_Tracker_Info_T  tracker_info = {};
   ocg::OCG_Outputs_T occupancy_grid = {};
   F360_Host_T host = {};
   F360_Calibrations_T calib = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   
   /** \setup
    * Setup 10 objects that are valid and moving with UD status NOT_TO_CONSIDER and that has a very large estimated height such that they would be classified CAN_PASS_UNDER by Assign_Underdrivability_Status_To_Moving_Object()
    * Setup 10 objects that are invalid and moving with UD status NOT_TO_CONSIDER. Since they are invalid their underdrivable status should be unchanged
    * Remaing objects should be invalid wth UD status NOT_TO_CONSIDER. Since they are invalid their underdrivable status should be unchanged
	 * Setup tracker timestamp to an arbitrary positive value
    * Setup a valid occupancy grid with all zones to CAN_NOT_PASS_UNDER.
	*/
   TEST_SETUP()
   {
      // Use default tracker calibrations
      Initialize_Tracker_Calibrations(calib);

      // Setup tracker timestamp to an arbitrary value
      tracker_info.time_us = 1000.0F;

      // Set up a valid OCG which have all its cells classifed as CAN_NOT_PASS_UNDER
      occupancy_grid.underdrivability.grid_curvature = 0.0F; // No curvature on OCG grid
      occupancy_grid.grid_definition.num_cells_x_close = ocg::NUM_CELLS_X_CLOSE;
      occupancy_grid.grid_definition.num_cells_x_mid = ocg::NUM_CELLS_X_MID;
      occupancy_grid.grid_definition.num_cells_x_far = ocg::NUM_CELLS_X_FAR;
      occupancy_grid.grid_definition.cell_length =  2.0F;
      occupancy_grid.grid_definition.num_cells_y =  ocg::NUM_CELLS_Y;
      occupancy_grid.grid_definition.cell_width = 6.0F;
      occupancy_grid.grid_definition.cell_width_extension_factor = 1.0F; // No extansion of OCG cell width at far distances
      // Set up all cells to have status CAN_NOT_PASS_UNDER
      for (uint8_t row_idx = 0u; row_idx < ocg::NUM_CELLS_X; row_idx ++)
      {
         for (uint8_t col_idx = 0u; col_idx < ocg::NUM_CELLS_Y; col_idx ++)
         {
             occupancy_grid.underdrivability.underdrivability_classification[row_idx][col_idx].underdrivability_status = ocg::UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER;
         }
      }
      occupancy_grid.underdrivability.ogcs_host_rear_axle_position.x = 0.0F; // Set up to 0 such that OCG and VCS has the same definition of origin
      occupancy_grid.underdrivability.ogcs_host_rear_axle_position.y = 0.0F; // Set up to 0 such that OCG and VCS has the same definition of origin
      occupancy_grid.underdrivability.ogcs_host_rear_axle_position.yaw = 0.0F; // No curvature on host predicted path
      occupancy_grid.timestamp = tracker_info.time_us; // Same time stamp as tracker (such that tracker don't need to do prediction of OCG)
      occupancy_grid.f_valid = true; // // Set occupancy grid to valid


      // Create 10 moving valid objects which should be classified as CAN_PASS_UNDER
      F360_Object_Track_T obj = {};
      obj.f_moving = true;
      obj.status = F360_OBJECT_STATUS_UPDATED;
      obj.ndets = 0; // No new detections such that the height estimate is unchanged (except for small reduction due to forgetting factor)
      obj.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
      obj.otg_height = calib.ud_mov_height_threshold + 1.0F; // Estimated height is significantly larger than the threshold for classifying moving objects as CAN_PASS_UNDER
      obj.ud_mov_historic_ndets = 10.0F; // Significantkly larger than 0 such that the estimated mean height is clearly defined
      obj.ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U; // Larger than threshold for classifying moving objects as CAN_PASS_UNDER
      obj.vcs_position.x = 51.0F; // Within the region where UD classification is done for moving objects and in the OCG zone which has status LIKELY_TO_PASS_UNDER
      obj.vcs_position.y = 0.0F; // Within the region where UD classification is done for moving objects 
      for(int32_t obj_idx = 0; obj_idx < 10; obj_idx++)
      {
         object_tracks[obj_idx] = obj;
         object_tracks[obj_idx].id = obj_idx + 1;
         tracker_info.active_obj_ids[tracker_info.num_active_objs] = obj_idx + 1;
         tracker_info.num_active_objs++;
      }

      // Create 10 moving valid objects which should be classified as CAN_NOT_PASS_UNDER
      obj = {};
      obj.f_moving = true;
      obj.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
      obj.status = F360_OBJECT_STATUS_UPDATED;
      obj.otg_height = calib.ud_mov_height_threshold - 1.0F; // Estimated height is significantly lower than the threshold for classifying moving objects as CAN_PASS_UNDER
      obj.ud_mov_historic_ndets = 10.0F; // Significantkly larger than 0 such that the estimated mean height is clearly defined
      obj.ud_mov_cnt_underdrivable = 0U; // Zero the counter for scans at which the object was above the height threshold
      obj.vcs_position.x = calib.ud_mov_posx_max_limit - 10.0F;  // Inside the region where UD classification is done for moving objects
      obj.vcs_position.y = 0.0F; // Within the region where UD classification is done for moving objects 
      for(int32_t obj_idx = 10; obj_idx < 20; obj_idx++)
      {
         object_tracks[obj_idx] = obj;
         object_tracks[obj_idx].id = obj_idx + 1;
         tracker_info.active_obj_ids[tracker_info.num_active_objs] = obj_idx + 1;
         tracker_info.num_active_objs++;
      }

      // Create 10 moving valid objects located in the area behind the host that should be classified as NOT_TO_CONSIDER but would be classified as CAN_PASS_UNDER if they were valid
      obj = {};
      obj.f_moving = true;
      obj.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
      obj.status = F360_OBJECT_STATUS_UPDATED;
      obj.otg_height = calib.ud_mov_height_threshold + 1.0F; // Estimated height is significantly larger than the threshold for classifying moving objects as CAN_PASS_UNDER
      obj.ud_mov_historic_ndets = 10.0F; // Significantkly larger than 0 such that the estimated mean height is clearly defined
      obj.ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U; // Larger than threshold for classifying moving objects as CAN_PASS_UNDER
      obj.vcs_position.x = calib.ud_mov_posx_min_limit - 10.0F;  // Outside the region where UD classification is done for moving objects
      obj.vcs_position.y = 0.0F; // Within the region where UD classification is done for moving objects 
      for(int32_t obj_idx = 20; obj_idx < 30; obj_idx++)
      {
         object_tracks[obj_idx] = obj;
         object_tracks[obj_idx].id = obj_idx + 1;
         tracker_info.active_obj_ids[tracker_info.num_active_objs] = obj_idx + 1;
         tracker_info.num_active_objs++;
      }

      // Create 10 objects which should not be considered for underdrivability classification by neither Assign_Underdrivability_Status_To_Moving_Object or Assign_Underdrivability_Status_To_Moving_Object functions
      obj = {};
      obj.f_moving = false; // Not considered by Assign_Underdrivability_Status_To_Moving_Object
      obj.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER;
      obj.status = F360_OBJECT_STATUS_UPDATED;
      obj.otg_height = calib.ud_mov_height_threshold + 1.0F; // Estimated height is significantly larger than the threshold for classifying moving objects as CAN_PASS_UNDER
      obj.ud_mov_historic_ndets = 10.0F; // Significantkly larger than 0 such that the estimated mean height is clearly defined
      obj.ud_mov_cnt_underdrivable = calib.ud_mov_cnt_consecutive_scans + 1U; // Larger than threshold for classifying moving objects as CAN_PASS_UNDER
      obj.vcs_position.x = calib.ud_mov_posx_min_limit + 10.0F;  // Inside the region where UD classification is done for moving objects
      obj.vcs_position.y = 0.0F; // Within the region where UD classification is done for moving objects 
      for(int32_t obj_idx = 30; obj_idx < static_cast<int32_t>(NUMBER_OF_OBJECT_TRACKS); obj_idx++)
      {
         object_tracks[obj_idx] = obj;
         object_tracks[obj_idx].id = obj_idx + 1;
         tracker_info.active_obj_ids[tracker_info.num_active_objs] = obj_idx + 1;
         tracker_info.num_active_objs++;
      }
   }
};


/** \purpose  
 * The purpose of this test is to check such that valid moving objects with height above the threshhold
 * get their underdrivability status updated by Assign_Underdrivability_Status_To_Moving_Object()
 * \req
 * FTCP-13969
 */
TEST(f360_object_underdrivability_classification_qt, test_valid_moving_objs_height_above_threshold)
{
   /** \precond
    * Default test setup from TEST GROUP is used.
    */
	
   /** \action
    * Call Object_Underdrivability_Classification()
    */
   Object_Underdrivability_Classification(tracker_info, &occupancy_grid, host, calib, object_tracks, timing_info);

   /** \result
    * Check that the underdrivability status for valid moving objects were updated by Assign_Underdrivability_Status_To_Moving_Object(),
    * i.e. that their underdrivability status is CAN_PASS_UNDER
    */
   for(int32_t obj_idx = 0; obj_idx < 10; obj_idx++)
   {
      CHECK_EQUAL(ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER, object_tracks[obj_idx].underdrivable_status);
   }
}

/** \purpose  
 * The purpose of this test is to check such that valid moving objects with height below the threshhold
 * get their underdrivability status updated by Assign_Underdrivability_Status_To_Moving_Object()
 * \req
 * FTCP-13969
 */
TEST(f360_object_underdrivability_classification_qt, test_valid_moving_objs_height_below_threshold)
{
   /** \precond
    * Default test setup from TEST GROUP is used.
    */
	
   /** \action
    * Call Object_Underdrivability_Classification()
    */
   Object_Underdrivability_Classification(tracker_info, &occupancy_grid, host, calib, object_tracks, timing_info);

   /** \result
    * Check that the underdrivability status for valid moving objects were updated by Assign_Underdrivability_Status_To_Moving_Object(),
    * i.e. that their underdrivability status is CAN_NOT_PASS_UNDER
    */
   for(int32_t obj_idx = 10; obj_idx < 20; obj_idx++)
   {
      CHECK_EQUAL(ocg::UNDERDRIVABLE_STATUS_CAN_NOT_PASS_UNDER, object_tracks[obj_idx].underdrivable_status);
   }
}

/** \purpose  
 * The purpose of this test is to check such that valid (historical height mean is above the threshold for required number of consecutive scans)
 * moving objects do not get their underdrivability status updated by Assign_Underdrivability_Status_To_Moving_Object()
 * when they are behind the host vehicle.
 * \req
 * FTCP-13969
 * FTCP-13970
 */
TEST(f360_object_underdrivability_classification_qt, test_valid_moving_objs_behind_host)
{
   /** \precond
    * Default test setup from TEST GROUP is used.
    */
	
   /** \action
    * Call Object_Underdrivability_Classification()
    */
   Object_Underdrivability_Classification(tracker_info, &occupancy_grid, host, calib, object_tracks, timing_info);

   /** \result
    * Check that the underdrivability status for moving objects outside of classification area were properly classified by Assign_Underdrivability_Status_To_Stationary_Object(),
    * i.e. that their underdrivability status is NOT_TO_CONSIDER
    */
   for(int32_t obj_idx = 20; obj_idx < 30; obj_idx++)
   {
      CHECK_EQUAL(ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER, object_tracks[obj_idx].underdrivable_status);
   }
}

/** \purpose  
 * The purpose of this test is to check if objects are set as UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER when their underdrivability status is not
 * updated by either Assign_Underdrivability_Status_To_Moving_Object or Assign_Underdrivability_Status_To_Moving_Object functions.
 * \req
 * FTCP-13970
 */
TEST(f360_object_underdrivability_classification_qt, test_objs_invalid)
{
   /** \precond
    * Occupancy grid not valid, otherwise default test setup from TEST GROUP is used.
    */
   occupancy_grid.f_valid = false; // // Set occupancy grid to invalid

   /** \action
    * Call Object_Underdrivability_Classification()
    */
   Object_Underdrivability_Classification(tracker_info, &occupancy_grid, host, calib, object_tracks, timing_info);

   /** \result
    * Check that the underdrivability status for invalid objects were not updated, i.e. that their undersdrivability status is NOT_TO_CONSIDER
    */
   for(int32_t obj_idx = 30; obj_idx < static_cast<int32_t>(NUMBER_OF_OBJECT_TRACKS); obj_idx++)
   {
      CHECK_EQUAL(ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER, object_tracks[obj_idx].underdrivable_status);
   }
}

/** @}*/
