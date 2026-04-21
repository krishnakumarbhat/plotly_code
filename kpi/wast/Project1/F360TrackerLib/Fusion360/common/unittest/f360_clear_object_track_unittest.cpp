/** \file
   This file contains unit tests for f360_clear_object_track.cpp
*/

#include "f360_clear_object_track.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

using namespace f360_variant_A;

/** \defgroup  f360_clear_object_track
 *  @{
**/


/** \brief
*  Unit test cases for common module.
*  This module includes test cases for f360_clear_object_track.cpp.
**/
TEST_GROUP(f360_clear_object_track)
{
};

/**
*\purpose Checks that all fields of object properties in F360_Object_Track_T
* struct has been cleared after calling Clear_Object_Track(), i.e.
* that floats equal 0.0F, that signed integers equal 0, that unsigned
* integers equal 0U, that booleans equal false and that enums equal
* invalid/unknown/undetermined.
*\req    NA
*/
TEST(f360_clear_object_track, TestAllFieldsBeenCleared)
{
   /** \step{1}
    * Call the Clear_Object_Track() function and verify that all object
    * properties has been cleared.
    **/

   /** \precond
   * Previous to calling the  Clear_Object_Track() function, all object
   * properties should have a value that differs from the default value
   * to be set by the function.
   **/
   F360_Object_Track_T obj_track;
   F360_Object_Track_T obj_track_prior_h;
   F360_Object_Track_T obj_track_prior_l;
   const int32_t orig_obj_id = 2;
   uint32_t expected_size_of_obj_track = 980U; /* should be equal to sizeof(obj_tracks)*/
   if (sizeof(void *) == 8)
   {
      expected_size_of_obj_track += 20;
   }

   obj_track.speed = 101.1F;
   obj_track.predicted_speed = 101.1F;

   obj_track.hdg_ptng_disagmt = 101.1F;

   obj_track.curvature = 101.1F;

   obj_track.tang_accel = 101.1F;

   obj_track.predicted_tang_accel = 101.1F;

   obj_track.vcs_position.x = 101.1F;
   obj_track.vcs_position.y = 101.1F;

   obj_track.predicted_vcs_position.x = 101.1F;
   obj_track.predicted_vcs_position.y = 101.1F;

   obj_track.pseudo_vcs_position.x = 101.1F;
   obj_track.pseudo_vcs_position.y = 101.1F;

   obj_track.vcs_velocity.longitudinal = 101.1F;
   obj_track.vcs_velocity.lateral = 101.1F;

   obj_track.predicted_vcs_velocity.longitudinal = 101.1F;
   obj_track.predicted_vcs_velocity.lateral = 101.1F;

   obj_track.vcs_accel.longitudinal = 101.1F;
   obj_track.vcs_accel.lateral = 101.1F;

   obj_track.vcs_heading = Angle{ 101.1F };

   obj_track.predicted_vcs_heading = 101.1F;

   obj_track.Set_Bbox_Orientation(Angle{ 101.1F });

   obj_track.predicted_vcs_pointing = 101.1F;

   obj_track.bbox.Set_Length(101.1F);

   obj_track.bbox.Set_Width(101.1F);

   obj_track.status = F360_OBJECT_STATUS_UPDATED;

   obj_track.occlusion_status.at_vcs_position = OCCLUSION_STATUS_VISIBLE;

   obj_track.time_since_cluster_created = 101.1F;

   obj_track.time_since_track_updated = 101.1F;

   for(unsigned int row_ind = 0; row_ind < STATE_DIMENSION; row_ind++)
   {
      for(unsigned int col_ind = 0; col_ind < STATE_DIMENSION; col_ind++)
      {
         obj_track.errcov[row_ind][col_ind] = 101.0F;
      }
   }

   obj_track.init_scheme = F360_TRACK_NEES_CV_ANALYTIC;

   obj_track.init_vel_source = F360_NEES_CFMI_VEL_HYP_SOURCE_RANSAC;

   obj_track.ndets = 101;

   for(unsigned int idx = 0; idx < MAX_DETS_IN_OBJ_TRK; idx++)
   {
      obj_track.detids[idx] = 101U;
   }

   obj_track.num_rr_inlier_dets = 101;
   obj_track.num_dets_used_in_rr_msmt_update = 101;

   obj_track.length_uncertainty = 101.0F;
   obj_track.width_uncertainty = 101.0F;

   obj_track.f_crossing = true;

   obj_track.f_moving = true;

   obj_track.f_moveable = true;

   obj_track.f_oncoming = true;

   obj_track.f_low_confidence_level = true;

   obj_track.f_vehicular_trk = true;

   obj_track.f_veh_trk_near_stat_host = true;

   obj_track.f_valid_for_liberal_tracking = true;

   obj_track.mirror_prob = 1.0F;

   obj_track.id = orig_obj_id;

   obj_track.reduced_id = 101;

   obj_track.reduced_status = F360_OBJECT_STATUS_UPDATED;

   obj_track.cntConsecutiveAmbiguous = 101;

   obj_track.cntConsecutiveMoving = 101;

   obj_track.raw_confidence_level = 101.1F;

   obj_track.confidenceLevel = 101.1F;

   obj_track.prev_avrg_conf_level = 101.1F;

   obj_track.time_since_stage_start = 101.1F;

   obj_track.num_types_of_dets[0] = 101;
   obj_track.num_types_of_dets[1] = 101;

   for(unsigned int row_ind = 0; row_ind < 3U; row_ind++)
   {
      for(unsigned int col_ind = 0; col_ind < 3U; col_ind++)
      {
         obj_track.meascov[row_ind][col_ind] = 101.0F;
      }
   }

   obj_track.cnt_error_in_predicted_speed = 101;

   obj_track.long_buffer_zone_len1 = 101.1F;

   obj_track.long_buffer_zone_len2 = 101.1F;

   obj_track.lat_buffer_zone_wid1 = 101.1F;

   obj_track.lat_buffer_zone_wid2 = 101.1F;

   obj_track.f_fast_moving = true;

   obj_track.time_since_initialization = 101.1F;

   obj_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

   obj_track.time_since_vehicle_init = 101.1F;

   obj_track.total_reduced_dets = 101;

   obj_track.filtered_dets = 101.1F;

   obj_track.f_need_to_hide_trk = true;

   obj_track.f_ghost_NU_2_C = true;

   obj_track.f_overlapping_with_object = true;

   obj_track.time_since_measurement = 101.1F;

   obj_track.innovation_length = 101.1F;

   obj_track.innovation_width = 101.1F;

   obj_track.accuracy_length = 101.1F;

   obj_track.accuracy_width = 101.1F;

   obj_track.priority = 101.1F;

   obj_track.p_higher_priority_track = &obj_track_prior_h;
   obj_track.p_lower_priority_track = &obj_track_prior_l;

   obj_track.reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;
   obj_track.min_projection_reference_point = F360_REFERENCE_POINT_FRONT_RIGHT;

   obj_track.object_class = F360_OBJ_CLASS_BICYCLE;

   obj_track.f_used_by_occlusion = true;

   obj_track.dead_zone_status = F360_Dead_Zone_Status_T::INSIDE;

   obj_track.exist_prob = 101.1F;

   obj_track.p_track_state = 101.1F;
   obj_track.p_det_sensor = 101.1F;
   obj_track.p_measurement = 101.1F;
   obj_track.p_birth = 101.1F;
   obj_track.p_persist = 101.1F;

   obj_track.f_track_born = true;

   obj_track.probability_pedestrian = 101.1F;
   obj_track.probability_car = 101.1F;
   obj_track.probability_motorcycle = 101.1F;
   obj_track.probability_bicycle = 101.1F;
   obj_track.probability_truck = 101.1F;
   obj_track.probability_undet = 101.1F;
   obj_track.probability_underdrivable = 101.1F;

   obj_track.underdrivable_status = ocg::UNDERDRIVABLE_STATUS_CAN_PASS_UNDER;

   obj_track.lsc_next_in_cluster = &obj_track_prior_h;
   obj_track.lsc_prev_in_cluster = &obj_track_prior_l;

   obj_track.behind_sep_id = 2U;
   obj_track.on_sep_id = 2U;
   obj_track.f_behind_sep_ambiguous = true;
   obj_track.sep_intersection_point.x = 101.1F;
   obj_track.sep_intersection_point.y = 101.1F;

   obj_track.reserved_value_1 = 101.1F;
   obj_track.reserved_value_2 = 101.1F;
   obj_track.reserved_value_3 = 101.1F;
   obj_track.reserved_value_4 = 101.1F;
   obj_track.reserved_value_5 = 101.1F;

   obj_track.conf_longitudinal_position = CONF9_LOW1;
   obj_track.conf_lateral_position = CONF9_LOW1;
   obj_track.conf_longitudinal_velocity = CONF9_LOW1;
   obj_track.conf_lateral_velocity = CONF9_LOW1;
   obj_track.conf_speed = CONF9_LOW1;
   obj_track.conf_overall = CONF3_LOW;

   obj_track.orth_delta_filtered = 4.0F;
   obj_track.orth_gap_filtered = 3.5F;
   obj_track.prev_vcs_center_pos.y = 20.0F;
   obj_track.prev_vcs_center_pos.x = 20.0F;
   obj_track.filtered_pos_diff_heading = F360_DEG2RAD(15.0F);

   obj_track.filtered_hist_assoc_det_rr_err_mean = 101.1F;
   obj_track.filtered_hist_assoc_det_rr_err_var = 101.1F;
   obj_track.filtered_hist_assoc_n_dets = 101.1F;

   obj_track.average_rcs = 15.0F;

   obj_track.low_rcs_dets_cnt = 1U;
   /** \action
   * Compute size of obj_track
   **/

   uint32_t size_of_obj_track = sizeof(obj_track);

   /** \result
   * Verify that F360_Object_Track_T interface has not been modified.
   * If this test fails then the code of this UT has to be modified
   * in the following ways:
   *    1) When obj_track is filled with data above, the newly
   *    created field in the interface has to been filled as well.
   *    Example: obj_track.new_field = 101.1F;
   *
   *    2) A new check has to be added below to verify that the newly
   *    created field in the interface has been properly cleared.
   *    Example: DOUBLES_EQUAL(0.0F, obj_track.new_field, 0.0F);
   *
   *    3) The assignment of a value to the expected_size_of_obj_trk
   *    variable in this test has to be modified such that
   *    expected_size_of_obj_trk corresponds to sizeof() the new
   *    F360_Object_Track_T interface.
   **/
   UNSIGNED_LONGS_EQUAL_TEXT(expected_size_of_obj_track, size_of_obj_track,
         "Failed check to verify that F360_Object_Track_T interface has not been modified.");

   /** \action
   * Call the function
   **/

   Clear_Object_Track(obj_track);

   /** \result
   * Verify that all fields of obj_track has been cleared except
   * the id field which should be unchanged.
   **/

   DOUBLES_EQUAL(0.0F, obj_track.pseudo_vcs_position.x, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.pseudo_vcs_position.y, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.speed, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_speed, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.hdg_ptng_disagmt, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.curvature, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.tang_accel, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_tang_accel, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.vcs_position.x, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.vcs_position.y, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_position.x, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_position.y, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.vcs_velocity.longitudinal, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.vcs_velocity.lateral, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_velocity.longitudinal, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_velocity.lateral, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.vcs_accel.longitudinal, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.vcs_accel.lateral, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.vcs_heading.Value(), 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_heading, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.length_uncertainty, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.width_uncertainty, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.bbox.Get_Orientation().Value(), 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.predicted_vcs_pointing, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.bbox.Get_Length() , 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.bbox.Get_Length(), 0.0F);

   LONGS_EQUAL(F360_OBJECT_STATUS_INVALID, obj_track.status);
   CHECK_EQUAL(OCCLUSION_STATUS_UNDEFINED, obj_track.occlusion_status.at_vcs_position);

   DOUBLES_EQUAL(-1.0F, obj_track.time_since_cluster_created, 0.0F);
   DOUBLES_EQUAL(-1.0F, obj_track.time_since_track_updated, 0.0F);

   for(unsigned int row_ind = 0; row_ind < STATE_DIMENSION; row_ind++)
   {
      for(unsigned int col_ind = 0; col_ind < STATE_DIMENSION; col_ind++)
      {
         DOUBLES_EQUAL(0.0F, obj_track.errcov[row_ind][col_ind], 0.0F);
      }
   }

   LONGS_EQUAL(F360_TRACK_INVALID, obj_track.init_scheme);
   LONGS_EQUAL(F360_NEES_CFMI_VEL_HYP_SOURCE_NONE, obj_track.init_vel_source);

   LONGS_EQUAL(0, obj_track.ndets);
   for(unsigned int idx = 0; idx < MAX_DETS_IN_OBJ_TRK; idx++)
   {
      UNSIGNED_LONGS_EQUAL(0U, obj_track.detids[idx]);
   }

   LONGS_EQUAL(0, obj_track.num_rr_inlier_dets);
   LONGS_EQUAL(0, obj_track.num_dets_used_in_rr_msmt_update);

   CHECK_FALSE(obj_track.f_crossing);
   CHECK_FALSE(obj_track.f_moving);
   CHECK_FALSE(obj_track.f_moveable);
   CHECK_FALSE(obj_track.f_oncoming);
   CHECK_FALSE(obj_track.f_low_confidence_level);
   CHECK_FALSE(obj_track.f_vehicular_trk);
   CHECK_FALSE(obj_track.f_veh_trk_near_stat_host);
   CHECK_FALSE(obj_track.f_valid_for_liberal_tracking);

   DOUBLES_EQUAL(0.0F, obj_track.mirror_prob, 0.0F);

   UNSIGNED_LONGS_EQUAL(orig_obj_id, obj_track.id);
   LONGS_EQUAL(0, obj_track.reduced_id);

   LONGS_EQUAL(F360_OBJECT_STATUS_INVALID, obj_track.reduced_status);

   LONGS_EQUAL(0, obj_track.cntConsecutiveAmbiguous);
   LONGS_EQUAL(0, obj_track.cntConsecutiveMoving);

   DOUBLES_EQUAL(0.0F, obj_track.raw_confidence_level, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.confidenceLevel, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.prev_avrg_conf_level, 0.0F);

   DOUBLES_EQUAL(-1.0F, obj_track.time_since_stage_start, 0.0F);

   LONGS_EQUAL(0, obj_track.num_types_of_dets[0]);
   LONGS_EQUAL(0, obj_track.num_types_of_dets[1]);

   for(unsigned int row_ind = 0; row_ind < 3U; row_ind++)
   {
      for(unsigned int col_ind = 0; col_ind < 3U; col_ind++)
      {
         DOUBLES_EQUAL(0.0F, obj_track.meascov[row_ind][col_ind], 0.0F);
      }
   }

   LONGS_EQUAL(0, obj_track.cnt_error_in_predicted_speed);

   DOUBLES_EQUAL(0.0F, obj_track.long_buffer_zone_len1, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.long_buffer_zone_len2, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.lat_buffer_zone_wid1, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.lat_buffer_zone_wid2, 0.0F);

   CHECK_FALSE(obj_track.f_fast_moving);

   DOUBLES_EQUAL(-1.0F, obj_track.time_since_initialization, 0.0F);

   LONGS_EQUAL(F360_TRACKER_TRKFLTR_INVALID, obj_track.trk_fltr_type);

   DOUBLES_EQUAL(-1.0F, obj_track.time_since_vehicle_init, 0.0F);

   LONGS_EQUAL(0, obj_track.total_reduced_dets);
   DOUBLES_EQUAL(0.0F, obj_track.filtered_dets, 0.0F);

   CHECK_FALSE(obj_track.f_need_to_hide_trk);
   CHECK_FALSE(obj_track.f_ghost_NU_2_C);
   CHECK_FALSE(obj_track.f_overlapping_with_object);

   DOUBLES_EQUAL(-1.0F, obj_track.time_since_measurement, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.innovation_length, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.innovation_width, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.accuracy_length, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.accuracy_width, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.priority, 0.0F);

   CHECK_TRUE(obj_track.p_higher_priority_track == NULL);
   CHECK_TRUE(obj_track.p_lower_priority_track == NULL);

   LONGS_EQUAL(F360_REFERENCE_POINT_CENTER, obj_track.reference_point);
   LONGS_EQUAL(F360_REFERENCE_POINT_CENTER, obj_track.min_projection_reference_point);

   LONGS_EQUAL(F360_OBJ_CLASS_UNDETERMINED, obj_track.object_class);

   CHECK_FALSE(obj_track.f_used_by_occlusion);

   CHECK_EQUAL(F360_Dead_Zone_Status_T::UNDEFINED, obj_track.dead_zone_status);

   DOUBLES_EQUAL(0.0F, obj_track.exist_prob, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.p_track_state, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.p_det_sensor, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.p_measurement, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.p_birth, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.p_persist, 0.0F);
   CHECK_FALSE(obj_track.f_track_born);

   DOUBLES_EQUAL(0.0F, obj_track.probability_pedestrian, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_car, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_motorcycle, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_bicycle, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_truck, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_undet, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.probability_underdrivable, 0.0F);

   CHECK_EQUAL(obj_track.underdrivable_status, ocg::UNDERDRIVABLE_STATUS_NOT_TO_CONSIDER);

   POINTERS_EQUAL(NULL, obj_track.lsc_next_in_cluster);
   POINTERS_EQUAL(NULL, obj_track.lsc_prev_in_cluster);

   CHECK_EQUAL(F360_INVALID_UNSIGNED_ID, obj_track.on_sep_id);
   CHECK_EQUAL(F360_INVALID_UNSIGNED_ID, obj_track.behind_sep_id);
   CHECK_FALSE(obj_track.f_behind_sep_ambiguous);
   DOUBLES_EQUAL(INFTY, obj_track.sep_intersection_point.x, 0.0F);
   DOUBLES_EQUAL(INFTY, obj_track.sep_intersection_point.y, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.reserved_value_1, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.reserved_value_2, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.reserved_value_3, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.reserved_value_4, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.reserved_value_5, 0.0F);

   CHECK_EQUAL(obj_track.conf_longitudinal_position, CONF9_NONE);
   CHECK_EQUAL(obj_track.conf_lateral_position, CONF9_NONE);
   CHECK_EQUAL(obj_track.conf_longitudinal_velocity, CONF9_NONE);
   CHECK_EQUAL(obj_track.conf_lateral_velocity, CONF9_NONE);
   CHECK_EQUAL(obj_track.conf_speed, CONF9_NONE);
   CHECK_EQUAL(obj_track.conf_overall, CONF3_NONE);

   DOUBLES_EQUAL(0.0F, obj_track.orth_delta_filtered, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.orth_gap_filtered, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.prev_vcs_center_pos.y, 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.prev_vcs_center_pos.x, 0.0F);
   DOUBLES_EQUAL(INFTY, obj_track.filtered_pos_diff_heading, 0.0F);

   DOUBLES_EQUAL(0.0F, obj_track.filtered_hist_assoc_det_rr_err_mean , 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.filtered_hist_assoc_det_rr_err_var  , 0.0F);
   DOUBLES_EQUAL(0.0F, obj_track.filtered_hist_assoc_n_dets  , 0.0F);

   CHECK_EQUAL(0, obj_track.low_rcs_dets_cnt);
}

/** @}*/
