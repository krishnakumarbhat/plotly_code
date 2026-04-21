/** \file
 * This file contains unit tests for content of f360_near_by_wheel_spins.cpp file
 */

#include "f360_nearby_wheel_spins.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"
#include <cstring>
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_near_by_wheel_spins
 *  @{
 */

/** \brief
 * This group assumes only one wheel spin det
 */
TEST_GROUP(f360_near_by_wheel_spins__one_wheel_spin)
{	
   RSPP_Calibrations_T rspp_calibrations;
   F360_Calibrations_T calibrations;

   F360_Detection_Props_T det_properties[MAX_NUMBER_OF_DETECTIONS];
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   int num_of_dets;
   float32_t low_floating_value = 0.00001F;
   
   /** \setup
    * Init calibrations and add one wheel_spin detection
    */
   TEST_SETUP()
   {
      Initialize_RSPP_Calibrations(rspp_calibrations);
      Initialize_Tracker_Calibrations(calibrations);

      num_of_dets = 1;

      det_properties[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;
      det_properties[0].vcs_position.x = -3.0F;
      det_properties[0].vcs_position.y = 2.0F;
   }
};

/** \purpose  
 * Check if detection above wheel spin which is close enough is marked as wheel spin
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__one_det_to_mark_above)
{
   /** \precond
    * Add det above wheel-spin and within limits and sort them
    */  
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_marking_th - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   raw_detect_list.number_of_valid_detections = num_of_dets;
   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */	
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection is not marked as a wheel spin");
}

/** \purpose
 * Check if detection above wheel spin which is close enough is not marked as wheel spin due to being wheel spin already
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_wheelspin_already)
{
   /** \precond
    * Add det above wheel-spin and within limits and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_marking_th - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS, "Detection wheel spin type is changed as a wheel spin");
}

/** \purpose
 * Check if detection above wheel spin whether is not marked as wheel spin due to being to far away longitudinaly
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_too_far_away_longitudinaly)
{
   /** \precond
    * Add det above wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_marking_th + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection above wheel spin whether is not marked as wheel spin due to being to far away laterally (positive)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_too_far_away_lateraly_positive)
{
   /** \precond
    * Add det above wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_marking_th - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th + low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection above wheel spin which is not marked as wheel spin due to being to far away laterally (negative)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_too_far_away_lateraly_negative)
{
   /** \precond
    * Add det above wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_marking_th - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y - calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin, which is close enough, is marked as wheel spin
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__one_det_to_mark_below)
{
   /** \precond
    * Add det below wheel-spin, within limits, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x - calibrations.k_nbws_long_marking_th + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;
   raw_detect_list.number_of_valid_detections = num_of_dets;
   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection is not marked as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin, which is close enough, is not marked as wheel spin due to being wheel spin already
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_wheel_spin_already)
{
   /** \precond
   * Add det below wheel-spin, mark it as wheel spin, and sort them
   */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x - calibrations.k_nbws_long_marking_th + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS, "Detection wheel spin type is changed as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin is not marked as wheel spin due to being too far away longitudinaly
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_longitudinaly_too_far)
{
   /** \precond
    * Add det below wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x - calibrations.k_nbws_long_marking_th - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin is not marked as wheel spin due to being to far away laterally (positive)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_laterally_positive_too_far)
{
   /** \precond
    * Add det below wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x - calibrations.k_nbws_long_marking_th + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th + low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin is not marked as wheel spin due to being to far away laterally (negative)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_laterally_negative_too_far)
{
   /** \precond
    * Add det below wheel-spin, beyond limit, and sort them
    */
   det_properties[1].vcs_position.x = det_properties[0].vcs_position.x - calibrations.k_nbws_long_marking_th + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y - calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection is not marked as wheel spin due to being out of area of interest (positive long)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_area_positive_long)
{
   /** \precond
    * Add det out of are of interest and sort them
    */
   det_properties[1].vcs_position.x = calibrations.k_nbws_max_long_pos + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection is not marked as wheel spin due to being out of area of interest (negative long)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_area_negative_long)
{
   /** \precond
    * Add det out of are of interest and sort them
    */
   det_properties[1].vcs_position.x = calibrations.k_nbws_min_long_pos - low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection is not marked as wheel spin due to being out of area of interest (positive lat)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_area_positive_lat)
{
   /** \precond
    * Add det out of are of interest and sort them
    */
   det_properties[1].vcs_position.x = det_properties[1].vcs_position.y;
   det_properties[1].vcs_position.y = calibrations.k_nbws_max_lat_pos + low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detection below wheel spin is not marked as wheel spin due to being out of area of interest (negative lat)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_area_negative_lat)
{
   /** \precond
    * Add det out of are of interest and sort them
    */
   det_properties[1].vcs_position.x = det_properties[1].vcs_position.y;
   det_properties[1].vcs_position.y = calibrations.k_nbws_min_lat_pos - low_floating_value;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}

/** \purpose
 * Check if detections are not marked as wheel spin due to being out of area of interest
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__dets_not_marked_if_out_of_area)
{
   /** \precond
    * Add 2 dets out of are of interest and sort them
    */
   det_properties[1].vcs_position.x = calibrations.k_nbws_max_long_pos + low_floating_value;
   det_properties[1].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   det_properties[2].vcs_position.x = calibrations.k_nbws_max_long_pos + 2 * low_floating_value;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should not be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[1].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection is marked as wheel spin as a wheel spin");
}


/** \purpose
 * Check if nothing happend if there is no dets to check
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__one_wheel_spin, Detect_Nearby_Wheel_Spins__no_dets_to_check)
{
   /** \precond
    * Sort det 
    */
   F360_Detection_Props_T ref_det_properties[MAX_NUMBER_OF_DETECTIONS];
   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);
   memcpy(&ref_det_properties, &det_properties, sizeof(det_properties));

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * This part of code should be reached
    */
   CHECK_TRUE(0 == memcmp(&ref_det_properties, &det_properties, sizeof(det_properties)));
}
/** @}*/




/** \defgroup  f360_near_by_wheel_spins__wheel_spin_cluster
 *  @{
 */

 /** \brief
  * Group used for checking if clustering is done properly.
  */
TEST_GROUP(f360_near_by_wheel_spins__wheel_spin_cluster)
{
   F360_Calibrations_T calibrations;
   RSPP_Calibrations_T rspp_calibrations;

   F360_Detection_Props_T det_properties[MAX_NUMBER_OF_DETECTIONS];
   int num_of_dets;
   float32_t low_floating_value = 0.00001F;
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};

   /** \setup
    * Init calibrations and add two wheel_spin detections
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      Initialize_RSPP_Calibrations(rspp_calibrations);

      num_of_dets = 2;

      det_properties[0].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;
      det_properties[0].vcs_position.x = -3.0F;
      det_properties[0].vcs_position.y = 2.0F;

      det_properties[1].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;
      det_properties[1].vcs_position.x = det_properties[0].vcs_position.x + calibrations.k_nbws_long_asc_th - low_floating_value;
      det_properties[1].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_asc_th - low_floating_value;;
   }
};

/** \purpose
 * Check if detection, which is within wheel spin cluster, is marked as wheel spin
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_marked_within_cluster)
{
   /** \precond
    * Add det within limits wheel spin cluster (long sequence 0 - 2 - 1)
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x - low_floating_value;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;
   raw_detect_list.number_of_valid_detections = num_of_dets;
   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[2].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection is not marked as a wheel spin");
}

/** \purpose
 * Check if detection, which is within wheel spin cluster, is not marked as wheel spin due to beeing too far laterally (positive)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_cluster_laterally_positive)
{
   /** \precond
    * Add det within limits wheel spin cluster (long sequence 0 - 2 - 1)
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x - low_floating_value;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y + calibrations.k_nbws_lat_marking_th + low_floating_value;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[2].wheel_spin_type != F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection is marked as a wheel spin");
}

/** \purpose
 * Check if detection, which is within wheel spin cluster, is not marked as wheel spin due to beeing too far laterally (negative)
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_not_marked_if_out_of_cluster_laterally_negative)
{
   /** \precond
    * Add det within limits wheel spin cluster (long sequence 0 - 2 - 1)
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x - low_floating_value;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y - calibrations.k_nbws_lat_marking_th - low_floating_value;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[2].wheel_spin_type != F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection is marked as a wheel spin");
}

/** \purpose
 * Check if detection, which is within wheel spin cluster, is not marked as wheel spin due to already wheel spin
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_not_marked_if_already_wheel_spin)
{
   /** \precond
    * Add det within limits wheel spin cluster (long sequence 0 - 2 - 1)
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x - low_floating_value;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   num_of_dets++;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[2].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS, "Detection's wheel spin type is changed");
}

/** \purpose
 * Check if detection detection is not marked due to reached maxiumum number of clusters
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_not_marked_cluster_number_overflow)
{
   /** \precond
    * Add det within limits wheel spin cluster
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x + 2.0F * calibrations.k_nbws_long_asc_th;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   num_of_dets++;

   det_properties[3].vcs_position.x = det_properties[2].vcs_position.x + low_floating_value;
   det_properties[3].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   calibrations.k_nbws_max_num_clusters = 1;

   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[3].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_INVALID, "Detection should be not marked as wheel spin");
}

/** \purpose
 * Check if detection detection is marked (counter test for cluster number overflowin
 * \req
 * NA.
 */
TEST(f360_near_by_wheel_spins__wheel_spin_cluster, Detect_Nearby_Wheel_Spins__det_marked_cluster_number_not_overflow)
{
   /** \precond
    * Add det within limits wheel spin cluster
    */
   det_properties[2].vcs_position.x = det_properties[1].vcs_position.x + 2.0F * calibrations.k_nbws_long_asc_th;
   det_properties[2].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[2].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_DETECTION_PAIRS;
   num_of_dets++;
   
   det_properties[3].vcs_position.x = det_properties[2].vcs_position.x + low_floating_value;
   det_properties[3].vcs_position.y = det_properties[0].vcs_position.y;
   det_properties[3].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   num_of_dets++;

   calibrations.k_nbws_max_num_clusters = 2;
   raw_detect_list.number_of_valid_detections = num_of_dets;
   Sort_Detections_Vcs_Long(rspp_calibrations, raw_detect_list);

   /** \action
    * Call Detect_Nearby_Wheel_Spins().
    */
   Detect_Nearby_Wheel_Spins(raw_detect_list, calibrations, det_properties);

   /** \result
    * Detection should be marked as NEARBY WHEEL SPIN
    */
   CHECK_TRUE_TEXT(det_properties[3].wheel_spin_type == F360_DETECTION_WHEELSPIN_TYPE_NEARBY, "Detection should be marked as wheel spin");
}

/** @}*/