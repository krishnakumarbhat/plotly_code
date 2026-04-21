/** \file
 * This file contains unit tests for content of f360_msmt_update_obj_trks_ctca.cpp file
 */

#include "f360_msmt_update_obj_trks_ctca.h"
#include <CppUTest/TestHarness.h>

#include "f360_pseudo_msmt.h"
#include "f360_calibrations.h"
#include "f360_math.h"

using namespace f360_variant_A;

/** \defgroup  f360_msmt_update_obj_trks_ctca__no_dets
 *  @{
 */

 /** \brief
  * Setup general values on object states and detection properties that is needed to perfrom
  * a measurement update.
  */
TEST_GROUP(f360_msmt_update_obj_trks_ctca__no_dets)
{
   F360_Host_T host;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Calibrations_T calib;
   F360_Object_Track_T object_track;
   F360_TRKR_TIMING_INFO_T timing_info;
   uint32_t selected_dets_idx[MAX_DETS_IN_OBJ_TRK];
   uint32_t selected_dets_num;

   F360_Object_Track_T copy_obj;

   /** \setup
    * Setup general values for object and detection states.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      // Object
      object_track.vcs_heading = Angle{ F360_DEG2RAD(2.0F) };
      object_track.bbox.Set_Orientation(object_track.vcs_heading);
      object_track.reference_point = F360_REFERENCE_POINT_LEFT;
      object_track.bbox.Set_Length(7.0F);
      object_track.bbox.Set_Width(2.5F);
      const float32_t obj_rear_center_pos[2] = {10.0F, 0.0F};
      object_track.vcs_position.x = obj_rear_center_pos[0] +  object_track.bbox.Get_Orientation().Cos() * 0.5F * object_track.bbox.Get_Length() + object_track.bbox.Get_Orientation().Sin() * 0.5F * object_track.bbox.Get_Width(); // This makes sure center rear ends up in wanted position
      object_track.vcs_position.y = obj_rear_center_pos[1] +  object_track.bbox.Get_Orientation().Sin() * 0.5F * object_track.bbox.Get_Length() - object_track.bbox.Get_Orientation().Cos() * 0.5F * object_track.bbox.Get_Width(); // This makes sure center rear ends up in wanted position
      object_track.Update_Bbox_Center();
      object_track.speed = 5.0F;
      object_track.curvature = 0.01F;
      object_track.tang_accel = 0.1F;

      object_track.pseudo_vcs_position.x = object_track.vcs_position.x + 1.0F;
      object_track.pseudo_vcs_position.y = object_track.vcs_position.y;

      object_track.f_moving = true;
      object_track.f_moveable = true;

      object_track.ndets = 1;
      object_track.detids[0] = 1U;

      // Arbitrary symmetric covariance matrix
      for (int row = 0; row < 6; row++)
      {
         for (int col = 0; col < 6; col++)
         {
            object_track.errcov[row][col] = 1.0F;
         }
      }

      // Arbitrary symmetric pseudo covariance matrix
      for (int row = 0; row < 3; row++)
      {
         object_track.meascov[row][row] = 1.0F;
      }

      // Detections
      raw_detection_list.detections[0].processed.vcs_az = F360_DEG2RAD(5.0F);
      raw_detection_list.detections[0].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[0].processed.vcs_az);
      raw_detection_list.detections[0].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[0].processed.vcs_az);

      det_props[0].vcs_position.x = 10.0F;
      det_props[0].vcs_position.y = 0.5F;

      det_props[0].range_rate_compensated = object_track.speed + 1.0F;

      raw_detection_list.detections[0].processed.std_vcs_az_scm = F360_DEG2RAD(1.0F);
      raw_detection_list.detections[0].processed.std_range_rate_compensated_scm = 0.3F;

      for (int cross_idx = 0; cross_idx < 5; cross_idx++)
      {
         raw_detection_list.detections[0].processed.vcs_cross_covariances_scm[cross_idx] = 0.5F;
      }

      raw_detection_list.detections[0].processed.vcs_position_cov_scm[0][0] = 0.5F;
      raw_detection_list.detections[0].processed.vcs_position_cov_scm[1][1] = 0.5F;
      raw_detection_list.detections[0].processed.vcs_position_cov_scm[0][1] = 0.5F;
      raw_detection_list.detections[0].processed.vcs_position_cov_scm[1][0] = raw_detection_list.detections[0].processed.vcs_position_cov_scm[0][1];
   }

};

/** \purpose
 * Verify that no measurement update is done when there are no associated eligible detections
 * \req
 * NA
 */
TEST(f360_msmt_update_obj_trks_ctca__no_dets, Msmt_Update_Obj_Trks_CTCA_No_Dets)
{
   /** \precond
    * Set number of selected detections for msmnt update to zero.
    * Copy states to expected data since no msnmt update is expected.
    */
   object_track.ndets = 0;
   selected_dets_num = 0U;
   copy_obj = object_track;

   /** \action
    * Call function
    */
   Msmt_Update_ObjTrks_CTCA(
      host,
      det_props,
      raw_detection_list,
      calib,
      selected_dets_idx,
      selected_dets_num,
      object_track,
      timing_info);

   /** \result
    * Verify object states haven't changed
    */
   DOUBLES_EQUAL(copy_obj.vcs_position.x, object_track.vcs_position.x, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_position.y, object_track.vcs_position.y, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_heading.Value(), object_track.vcs_heading.Value(), F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.speed, object_track.speed, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.curvature, object_track.curvature, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.tang_accel, object_track.tang_accel, F360_EPSILON);
}

/** @}*/

/** \defgroup  f360_msmt_update_obj_trks_ctca
 *  @{
 */

 /** \brief
  * Test group for tests related to measurement update of CTCA tracks
  */
TEST_GROUP(f360_msmt_update_obj_trks_ctca)
{
   F360_Host_T host{};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]{};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Calibrations_T calib{};
   F360_Object_Track_T object_track{};
   F360_TRKR_TIMING_INFO_T timing_info{};
   uint32_t selected_dets_idx[MAX_DETS_IN_OBJ_TRK]{};
   uint32_t selected_dets_num{};
   float32_t elapsed_time_s = 1.0F;

   double threshold = 1e-4;
   const float32_t test_pass_th_strict = F360_EPSILON;

   /** \setup
    * Initialize tracker calibrations
    * Set up track parameters to be placed on left side of host, close to y vcs axis.
    * Set up parameters of two detections to make valid measurment
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      object_track.vcs_heading = Angle{ 1.57F };
      object_track.bbox.Set_Orientation(object_track.vcs_heading);
      object_track.reference_point = F360_REFERENCE_POINT_LEFT;
      object_track.bbox.Set_Length(7.0F);
      object_track.bbox.Set_Width(2.5F);
      object_track.curvature = 0.0F;
      const float32_t obj_rear_center_pos[2] = {0.0F, -2.0F};
      object_track.vcs_position.x = obj_rear_center_pos[0] +  object_track.bbox.Get_Orientation().Cos() * 0.5F * object_track.bbox.Get_Length() + object_track.bbox.Get_Orientation().Sin() * 0.5F * object_track.bbox.Get_Width(); // This makes sure center rear ends up in wanted position
      object_track.vcs_position.y = obj_rear_center_pos[1] +  object_track.bbox.Get_Orientation().Sin() * 0.5F * object_track.bbox.Get_Length() - object_track.bbox.Get_Orientation().Cos() * 0.5F * object_track.bbox.Get_Width(); // This makes sure center rear ends up in wanted position
      object_track.Update_Bbox_Center();
      object_track.ndets = 2;

      for (int i = 0; i < STATE_DIMENSION; i++)
      {
         for (int j = 0; j < STATE_DIMENSION; j++)
         {
            object_track.errcov[i][j] = 0.01F;
         }
      }

      object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_X] = 0.5F;
      object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_Y] = 0.5F;
      object_track.meascov[F360_PSEUDO_MSMT_POS_X][F360_PSEUDO_MSMT_POS_Y] = 0.0F;
      object_track.meascov[F360_PSEUDO_MSMT_POS_Y][F360_PSEUDO_MSMT_POS_X] = 0.0F;

      selected_dets_num = 2;
      selected_dets_idx[0] = 0;
      selected_dets_idx[1] = 1;

      raw_detection_list.detections[0].processed.cos_vcs_az = 0.0F;
      raw_detection_list.detections[0].processed.sin_vcs_az = -1.0F;
      det_props[0].vcs_position.x = 0.1F;
      det_props[0].vcs_position.y = -2.0F;
      det_props[0].range_rate_compensated = 5.0F;

      raw_detection_list.detections[1].processed.cos_vcs_az = 0.01F;
      raw_detection_list.detections[1].processed.sin_vcs_az = -0.99F;
      det_props[1].vcs_position.x = -0.1F;
      det_props[1].vcs_position.y = -1.9F;
      det_props[1].range_rate_compensated = 2.5F;
   }
};

/** \purpose
 * Purpose of this test is to verify that when object is moving its curvature is updated.
 * \req
 * NA
 */
TEST(f360_msmt_update_obj_trks_ctca, Msmt_Update_ObjTrks_CTCA__curvature_is_updated_if_moving)
{
   /** \precond
    * Set track as movable
    * Set its speed above threshold
    * Initialize track to make measurement update step return valid numbers
    */
   object_track.f_moving = true;
   object_track.speed = 10.0F;

   /** \action
    * Call tested function.
    */
   Msmt_Update_ObjTrks_CTCA(host, det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
    * Check that curvature of track was changed.
    */
   CHECK_TRUE(object_track.curvature != 0.0F);
}

/** \purpose
* Purpose of this test is to verify that when object is moveable (but not moving) and has high enough speed
* its curvature is updated.
* \req
* NA
*/
TEST(f360_msmt_update_obj_trks_ctca, Msmt_Update_ObjTrks_CTCA__curvature_is_updated_if_movable_and_speed_high_enough)
{
   /** \precond
   * Set track as movable
   * Set its speed below threshold
   * Initialize track to make measurement update step return valid numbers
   */
   object_track.f_moveable = true;
   object_track.speed = 10.0F;

   /** \action
   * Call tested function.
   */
   Msmt_Update_ObjTrks_CTCA(host, det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
   * Check that curvature of track was changed.
   */
   CHECK_TRUE(object_track.curvature != 0.0F);
}

/** \purpose
* Purpose of this test is to verify that when object is not movable and its speed is to high its
* curvature is not updated
* \req
* NA.
*/
TEST(f360_msmt_update_obj_trks_ctca, Msmt_Update_ObjTrks_CTCA__curvature_is_not_updated_if_not_movable_even_if_high_speed)
{
   /** \precond
   * Set track as not movable
   * Set its speed above threshold
   * Initialize track to make measurement update step return valid numbers
   */
   object_track.f_moving = false;
   object_track.f_moveable = false;
   object_track.speed = calib.k_min_speed_for_updating_heading + 0.01F;

   /** \action
   * Call tested function.
   */
   Msmt_Update_ObjTrks_CTCA(host, det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
   * Check that curvature of track was not changed.
   */
   DOUBLES_EQUAL(0.0F, object_track.curvature, threshold);
}

/** \purpose
 * Purpose of this test is to verify that when object is decelerating hard (its tangential acceleration is largely negative)
 * and the sign of object speed is changed from positive to negative by the KF measurement update equations then object speed
 * and tangential acceleration are set to 0.
 * \req
 * NA
 */
TEST(f360_msmt_update_obj_trks_ctca, Msmt_Update_ObjTrks_CTCA__zero_spd_and_acc_if_deacceleration_hard_and_sped_changes_sign_to_negative)
{
   /** \precond
    * Set track as movable
    * Set its speed to slightly above 0m/s (0.001m/s)
    * Set object deacceleration to be largely negative (-8m/s)
    * (Note: Detetion compensated range rate should be set to someting positive so that it appears like object has changed direction is now on its way away from host. 5.0m/s and 2.5m/s respectively from the test group can be used)
    */
   object_track.f_moving = true;
   object_track.speed = 0.001F;
   object_track.tang_accel = -8.0F;

   /** \action
    * Call tested function.
    */
   Msmt_Update_ObjTrks_CTCA(host, det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
    * Check that object speed and tagential acceleration are set to 0
    */
   DOUBLES_EQUAL_TEXT(0.0F, object_track.speed, test_pass_th_strict, "Speed is not set to zero");
   DOUBLES_EQUAL_TEXT(0.0F, object_track.tang_accel, test_pass_th_strict, "Tangential acceleration is not set to zero");
}
/** @}*/
