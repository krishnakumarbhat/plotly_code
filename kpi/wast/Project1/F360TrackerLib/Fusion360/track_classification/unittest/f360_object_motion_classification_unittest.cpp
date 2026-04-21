/** \file
 * This file contains unit tests for content of f360_object_motion_classification.cpp file
 */

#include "f360_object_motion_classification.h"
#include "f360_trk_fltr_ctca_states.h"
#include <CppUTest/TestHarness.h>

#include "f360_math_func.h"

using namespace f360_variant_A;

/** \defgroup  f360_object_motion_classification
 *  @{
 */

 /** \brief
  * Test Group of Object_Motion_Classification() function. Tests verify whether objects
  * are properly selected and their motion status is properly analysed.
  */
TEST_GROUP(f360_object_motion_classification)
{
   F360_Host_T host;
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS];
   F360_Globals_T globals;
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Calibrations_T calib;
   F360_Object_Track_T& object = object_tracks[0];
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};

   /** \setup
    * Initialize tracker calibrations
    * Set active objects as one
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
      tracker_info.num_active_objs = 1;
      tracker_info.active_obj_ids[0] = 1;
      object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
      object.errcov[F360_TRK_FLTR_CTCA_STATE_S][F360_TRK_FLTR_CTCA_STATE_S] = 0.01F;
   }
};

/** \purpose
 * Purpose of this test is to verify whether when object should not be updated its motion status is not modified.
 * \req
 * NA.
 */
TEST(f360_object_motion_classification, object_motion_classification__flag_not_changed_if_should_not_be_updated)
{
   /** \precond
    * Set object status as invalid
    * Set object f_moving flag to true
    */
   object.status = F360_OBJECT_STATUS_INVALID;
   object.f_moving = true;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
    * Call tested function
    */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
    * Check if object motion f_moving flag was not modified
    */
   CHECK_TRUE(object.f_moving);
}

/** \purpose
* Purpose of this test is to verify whether when object is suspected stationary its moving counter is reset and ambiguous counter incremented.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__counters_changed_if_suspected_stationary)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to false
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = false;
   object.cntConsecutiveAmbiguous = 0;
   object.cntConsecutiveMoving = 1;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);
   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if cntConsecutiveMoving was reset
   * Check if cntConsecutiveAmbiguous was incremented
   */
   CHECK_EQUAL(0, object.cntConsecutiveMoving);
   CHECK_EQUAL(1, object.cntConsecutiveAmbiguous);
}

/** \purpose
* Purpose of this test is to verify whether when object is not suspected stationary its moving counter is incremented and ambiguous counter reset.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__counters_changed_if_not_suspected_stationary)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to false
   * Set object speed to high value
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = false;
   object.cntConsecutiveAmbiguous = 1;
   object.cntConsecutiveMoving = 0;
   object.speed = 10.0F;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if cntConsecutiveMoving was incremented
   * Check if cntConsecutiveAmbiguous was reset
   */
   CHECK_EQUAL(1, object.cntConsecutiveMoving);
   CHECK_EQUAL(0, object.cntConsecutiveAmbiguous);
}

/** \purpose
* Purpose of this test is to verify whether when moving counter exceeds threshold, motion status is updated.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__marked_as_moving)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to false
   * Set object cntConsecutiveMoving to value above threshold
   * Set object speed to high value
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = false;
   object.cntConsecutiveAmbiguous = 0;
   object.cntConsecutiveMoving = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.speed = 10.0F;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if object was marked as moving
   */
   CHECK_TRUE(object.f_moving);
   CHECK_FALSE(object.f_stopped);
   CHECK_EQUAL(object.cntConsecutiveStopped, 0);
}

/** \purpose
* Purpose of this test is to verify whether when object is marked as moving and ambigious counter is below threshold
* object f_moving flag is not reset. With f_moving flag is set and f_moveable is true, cntConsecutiveStopped should not increment.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__moving_flag_not_reset)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to true
   * Set object speed to 0
   * Set object f_moveable flag to true
   * Set object cntConsecutiveStopped to 0
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = 0;
   object.cntConsecutiveMoving = 0;
   object.cntConsecutiveStopped = 0;
   object.speed = 0.0F;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moving flag was not reset
   * Check if cntConsecutiveStopped doesn't increment
   */
   CHECK_TRUE(object.f_moving);
   CHECK_EQUAL(object.cntConsecutiveStopped, 0);
}

/** \purpose
* Purpose of this test is to verify whether when object is marked as moving and ambigious counter is above threshold
* object f_moving flag is reset. With f_moving flag is reset and f_moveable is true, it is expected to see 
* cntConsecutiveStopped increment.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__moving_flat_reset)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to true
   * Set object speed to 0
   * Set object f_moveable flag to true
   * Set object cntConsecutiveStopped to 0
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.cntConsecutiveStopped = 0;
   object.speed = 0.0F;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moving flag is reset
   * Check if cntConsecutiveStopped increments
   */
   CHECK_FALSE(object.f_moving);
   CHECK_EQUAL(object.cntConsecutiveStopped, 1);
}

/** \purpose
* Purpose of this test is to verify cntConsecutiveStopped doesn't increment if f_moveable is false
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__stop_counter_not_increment_f_moveable)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to true
   * Set object speed to 0
   * Set object f_moveable flag to false
   * Set object cntConsecutiveStopped to 0
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.0F;
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if cntConsecutiveStopped doesn't increment
   */
   CHECK_EQUAL(object.cntConsecutiveStopped, 0);
}

/** \purpose
* Purpose of this test is to verify that object motion status is updated when moving counter exceeds threshold
* and to verify that object heading and pointing angles are NOT updated for CTCA objects that were previously stationary.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__ctca_object_switch_to_moving)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to false
   * Set object cntConsecutiveMoving to value above threshold
   * Set object filter type to CTCA
   * Set objects velocity vector to a high value
   * Reset objects heading and pointing
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = false;
   object.cntConsecutiveAmbiguous = 0;
   object.cntConsecutiveMoving = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object.vcs_velocity.longitudinal = 5.0F;
   object.vcs_velocity.lateral = 5.0F;
   object.speed = F360_Get_Hypotenuse(object.vcs_velocity.longitudinal, object.vcs_velocity.lateral);
   object.vcs_heading = Angle{ 0.0F };
   object.Set_Bbox_Orientation(Angle{ 0.0F });
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if object was marked as moving and that heading and pointing was not updated
   */
   const float32_t exp_angle = 0.0F;
   CHECK_TRUE(object.f_moving);
   DOUBLES_EQUAL(exp_angle, object.vcs_heading.Value(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Cosf(exp_angle), object.vcs_heading.Cos(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Sinf(exp_angle), object.vcs_heading.Sin(), F360_EPSILON);
   DOUBLES_EQUAL(exp_angle, object.bbox.Get_Orientation().Value(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Cosf(exp_angle), object.bbox.Get_Orientation().Cos(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Sinf(exp_angle), object.bbox.Get_Orientation().Sin(), F360_EPSILON);
}


/** \purpose
* Purpose of this test is to verify that object motion status is updated when moving counter exceeds threshold
* and to verify that object heading and pointing angles are NOT updated for CTCA objects that were previously moving.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__ctca_object_is_already_moving)
{
   /** \precond
   * Set object status as updated
   * Set object f_moving flag to true
   * Set object cntConsecutiveMoving to value above threshold
   * Set object filter type to CTCA
   * Set objects velocity vector to a high value
   * Reset objects heading and pointing
   */
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.cntConsecutiveAmbiguous = 0;
   object.cntConsecutiveMoving = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   object.vcs_velocity.longitudinal = 5.0F;
   object.vcs_velocity.lateral = 5.0F;
   object.speed = F360_Get_Hypotenuse(object.vcs_velocity.longitudinal, object.vcs_velocity.lateral);
   object.vcs_heading = Angle{ 0.0F };
   object.Set_Bbox_Orientation(Angle{ 0.0F });
   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);
   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if object was marked as moving and that heading and pointing was not updated
   */
   const float32_t exp_angle = 0.0F;
   CHECK_TRUE(object.f_moving);
   DOUBLES_EQUAL(exp_angle, object.vcs_heading.Value(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Cosf(exp_angle), object.vcs_heading.Cos(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Sinf(exp_angle), object.vcs_heading.Sin(), F360_EPSILON);
   DOUBLES_EQUAL(exp_angle, object.bbox.Get_Orientation().Value(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Cosf(exp_angle), object.bbox.Get_Orientation().Cos(), F360_EPSILON);
   DOUBLES_EQUAL(F360_Sinf(exp_angle), object.bbox.Get_Orientation().Sin(), F360_EPSILON);
}

/** \purpose
* Purpose of this test is to verify that object is hard switched to CCA from CTCA, if the interval of two consecutive stops 
* (i.e. from mvoing to non-moving) is less than 3 seconds and the object is outside the close-to-host zone.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__hard_switch_consecutive_stops_out_zone)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s)
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.0 second (True condition)
   * Set object position to be outside the close-to-host zone (True condition)
   * Set cntConsecutiveStopped to be less than calib.k_object_motion_min_consec_stopped (False condition)
   * Set object orientation to be 60 deg (False condition)
   */
   
   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 40.0F;
   object.vcs_position.y = 0.0F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.0F;
   object.cntConsecutiveStopped = 10;
   object.bbox.Set_Orientation(F360_DEG2RAD(60.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moving flag is reset
   * Check if the filter type has been changed to CCA
   * Check if the object bounding box size matches the expectation
   */
   CHECK_FALSE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 0.5F);
   CHECK_EQUAL(object.bbox.Get_Width(), 0.5F);
}

/** \purpose
* Purpose of this test is to verify that object is hard switched to CCA from CTCA, if the interval of two consecutive stops 
* (i.e. from mvoing to non-moving) is less than 3 seconds and the object has a very large orientation diff from the host
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__hard_switch_consecutive_stops_diff_orient)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s)
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.0 second (True condition)
   * Set cntConsecutiveStopped to be less than calib.k_object_motion_min_consec_stopped (False condition)
   * Set object position to be within the close-to-host zone (False condition)
   * Set object orientation to be 70 deg (True condition)
   */
   
   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;
   
   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 29.0F;
   object.vcs_position.y = 0.0F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.0F;
   object.cntConsecutiveStopped = 10;
   object.bbox.Set_Orientation(F360_DEG2RAD(70.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag is reset
   * Check if the filter type has been changed to CCA
   * Check if the object bounding box size matches the expectation
   */
   CHECK_FALSE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 0.5F);
   CHECK_EQUAL(object.bbox.Get_Width(), 0.5F);
}

/** \purpose
* Purpose of this test is to verify that object is hard switched to CCA from CTCA, if the object has been moveable but non-moveable
* consistently for sufficiently long and the object has a very large orientation diff from the host
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__hard_switch_long_nonmoving_diff_orient)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s)
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 3.2 second (False condition)
   * Set cntConsecutiveStopped to be more than calib.k_object_motion_min_consec_stopped (True condition)
   * Set object position to be within the close-to-host zone (False condition)
   * Set object orientation to be -110 deg (True condition)
   */
   
   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 29.0F;
   object.vcs_position.y = 0.0F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 3.2F;
   object.cntConsecutiveStopped = 150;
   object.bbox.Set_Orientation(F360_DEG2RAD(-110.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag is reset
   * Check if the filter type has been changed to CCA
   * Check if the object bounding box size matches the expectation
   */
   CHECK_FALSE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 0.5F);
   CHECK_EQUAL(object.bbox.Get_Width(), 0.5F);
}

/** \purpose
* Purpose of this test is to verify that object is hard switched to CCA from CTCA, if the object has been moveable but non-moveable
* consistently for sufficiently long and the object is outside the close-to-host zone.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__hard_switch_long_nonmoving_stops_out_zone)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s)
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.9 second and Set object f_moving flag to false (False condition)
   * Set cntConsecutiveStopped to be more than calib.k_object_motion_min_consec_stopped (True condition)
   * Set object position to be outside the close-to-host zone (True condition)
   * Set object orientation to be -120 deg (False condition)
   */
   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = false;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 29.0F;
   object.vcs_position.y = -4.5F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.9F;
   object.cntConsecutiveStopped = 150;
   object.bbox.Set_Orientation(F360_DEG2RAD(-120.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag is reset
   * Check if the filter type has been changed to CCA
   * Check if the object bounding box size matches the expectation
   */
   CHECK_FALSE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 0.5F);
   CHECK_EQUAL(object.bbox.Get_Width(), 0.5F);
}

/** \purpose
* Purpose of this test is to verify that object is NOT hard switched to CCA from CTCA, if neither the object has been moveable but non-moveable
* consistently for sufficiently long nor the interval of two consecutive stops (i.e. from mvoing to non-moving) is less than 3 seconds.
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__no_hard_switch_former_cond_unfulfilled)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s)
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 3.2 second (False condition)
   * Set cntConsecutiveStopped to be less than calib.k_object_motion_min_consec_stopped (False condition)
   * Set object position to be outside the close-to-host zone (True condition)
   * Set object orientation to be -70 deg (True condition)
   */
   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 29.0F;
   object.vcs_position.y = 4.5F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 3.2F;
   object.cntConsecutiveStopped = 10;
   object.bbox.Set_Orientation(F360_DEG2RAD(-70.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag, bounding box size and filter type are all unchanged
   */
   CHECK_TRUE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 4.0F);
   CHECK_EQUAL(object.bbox.Get_Width(), 2.0F);
}

/** \purpose
* Purpose of this test is to verify that object is NOT hard switched to CCA from CTCA, if the object position is inside the close-to-host zone and object orientation is normal relative to host
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__no_hard_switch_latter_cond_unfulfilled)
{
   /** \precond
   * Set host speed to be above stationary threshold (0.5 m/s) 
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.9 second (True condition)
   * Set cntConsecutiveStopped to be more than calib.k_object_motion_min_consec_stopped (True condition)
   * Set object position to be outside the close-to-host zone (False condition)
   * Set object orientation to be -118 deg (False condition)
   */

   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold + 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 29.0F;
   object.vcs_position.y = 3.5F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.9F;
   object.cntConsecutiveStopped = 160;
   object.bbox.Set_Orientation(F360_DEG2RAD(-118.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag, bounding box size and filter type are all unchanged
   */
   CHECK_TRUE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 4.0F);
   CHECK_EQUAL(object.bbox.Get_Width(), 2.0F);
}

/** \purpose
* Purpose of this test is to verify that object is hard switched to CCA from CTCA, if the object position is outside the close-to-host zone, for stationary host case and object orientation is normal relative to host
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__hard_switch_for_enlarged_close_to_host_zone_for_stationary_host)
{
   /** \precond
   * Set host speed to be below stationary threshold (0.5 m/s) 
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.9 second (True condition)
   * Set cntConsecutiveStopped to be more than calib.k_object_motion_min_consec_stopped (True condition)
   * Set object position to be outside the close-to-host zone for stationary host case
   * Set object orientation to be -118 deg (False condition)
   */

   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold - 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 16.0F;
   object.vcs_position.y = 3.5F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.9F;
   object.cntConsecutiveStopped = 160;
   object.bbox.Set_Orientation(F360_DEG2RAD(-118.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag is reset
   * Check if the filter type has been changed to CCA
   * Check if the object bounding box size matches the expectation
   */
   CHECK_FALSE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 0.5F);
   CHECK_EQUAL(object.bbox.Get_Width(), 0.5F);
}

/** \purpose
* Purpose of this test is to verify that object is not hard switched to CCA from CTCA, if the object position is inside the close-to-host zone, for stationary host case and object orientation is normal relative to host
* \req
* NA.
*/
TEST(f360_object_motion_classification, object_motion_classification__no_hard_switch_for_enlarged_close_to_host_zone_for_stationary_host)
{
   /** \precond
   * Set host speed to be below stationary threshold (0.5 m/s) 
   * Set object status as updated
   * Set object filter type CTCA
   * Set object f_moving flag to true
   * Set object f_moveable flag to true
   * Set object speed to 0.3
   * Set object length & width to a random value
   * Set time_since_last_stop to 2.9 second (True condition)
   * Set cntConsecutiveStopped to be more than calib.k_object_motion_min_consec_stopped (True condition)
   * Set object position to be inside the close-to-host zone for stationary host case
   * Set object orientation to be -118 deg (False condition)
   */

   host.speed = calib.k_object_motion_queue_zone_host_stationary_speed_threshold - 0.01F;

   object.status = F360_OBJECT_STATUS_UPDATED;
   object.f_moving = true;
   object.f_moveable = true;
   object.cntConsecutiveAmbiguous = calib.k_object_motion_min_consec_moving_cnt_th + 5;
   object.cntConsecutiveMoving = 0;
   object.speed = 0.3F;

   object.vcs_position.x = 14.0F;
   object.vcs_position.y = 3.5F;
   object.bbox.Set_Length(4.0F);
   object.bbox.Set_Width(2.0F);

   object.trk_fltr_type = f360_variant_A::F360_TRACKER_TRKFLTR_CTCA;
   object.time_since_last_stop = 2.9F;
   object.cntConsecutiveStopped = 160;
   object.bbox.Set_Orientation(F360_DEG2RAD(-118.0F));

   const Occlusion_T occlusion(calib.k_occlusion_enabled, calib, tracker_info, sensors, timing_info, object_tracks);

   /** \action
   * Call tested function
   */
   Object_Motion_Classification(object_tracks, tracker_info, host, globals, raw_detect_list, calib, sensors, occlusion);

   /** \result
   * Check if moveable flag, bounding box size and filter type are all unchanged
   */
   CHECK_TRUE(object.f_moveable);
   CHECK_TRUE(f360_variant_A::F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type);
   CHECK_EQUAL(object.bbox.Get_Length(), 4.0F);
   CHECK_EQUAL(object.bbox.Get_Width(), 2.0F);
}
/** @}*/
