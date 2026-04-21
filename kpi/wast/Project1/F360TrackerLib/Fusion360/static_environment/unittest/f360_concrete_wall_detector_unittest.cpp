/** \file
 * This file contains unit tests for content of f360_concrete_wall_detector.cpp file
 */

#include "f360_timing_info.h"
#include "f360_concrete_wall_detector.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  Run_CWD_Measurements
 *  @{
 */

 /** \brief
  * Test group of method Run_CWD_Measurements().
  */
TEST_GROUP(Run_CWD_Measurements)
{
   // Declare common variables used within all tests in this test group.
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]{};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_Calibrations_T calib{};
   Static_Env_Poly_T stat_env_polys[F360_NUM_OF_STATIC_ENV_POLYS]{};
   F360_Concrete_Wall_Detector_T concrete_wall_detector{};
   float32_t host_curvature = 0.0F;
   float32_t host_speed = 0.0F;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   void Append_Detection(const uint32_t det_idx, const float32_t long_posn, const float32_t lat_posn)
   {
      if (det_idx > 0U)
      {
         raw_detect_list.detections[det_idx - 1U].processed.next_sorted_idx = det_idx; // first extend the list of sorted detections by one
      }
      det_props[det_idx].vcs_position.y = lat_posn;
      det_props[det_idx].vcs_position.x = long_posn;
      raw_detect_list.detections[det_idx].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[det_idx].f_ok_to_use = true;
      raw_detect_list.detections[det_idx].processed.prev_sorted_idx = det_idx - 1U;
      raw_detect_list.detections[det_idx].processed.next_sorted_idx = F360_INVALID_ID;
   }

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};

/** \purpose
 * This test verifies CWD status is invalid if only one valid detection is in sensor zone.
 * \req NA
 */
TEST(Run_CWD_Measurements, if_lat_posn_the_only_det_in_zone_become_estimated_lat_posn_of_side_cwd)
{
   /** \precond
    * Set sensor long posn.
    * Set three detection parameters but the only one is in sensor zone.
    * Sort longitudinaly these detections.
    * Set raw_detect_list data
    * Initialize CWD and Long Zones
    */
   sensors[0U].constant.mounting_position.vcs_position.longitudinal = 0.0F;
   sensors[0U].variable.is_valid = true;
   sensors[0U].constant.mounting_position.vcs_position.lateral = 1.0F;

   Append_Detection(0U, -0.2F, 5.0F);
   Append_Detection(1U, 0.0F, 3.0F);
   Append_Detection(2U, 0.2F, 2.0F);

   raw_detect_list.detections[0U].processed.next_sorted_idx = 1;
   raw_detect_list.detections[1U].processed.next_sorted_idx = 2;
   raw_detect_list.detections[2U].processed.next_sorted_idx = -1;

   raw_detect_list.vcslong_sorted_ref_det_idx[0] = 0;
   raw_detect_list.vcslong_sorted_ref_det_idx[1] = 1;
   raw_detect_list.vcslong_sorted_ref_det_idx[2] = 2;

   concrete_wall_detector.Init(sensors, calib);
   concrete_wall_detector.Init_Long_Zones(calib.k_cwd_sensor_zone_half_length);

   /** \action
    * Call Run_CWD_Measurements()
    * Call Map_CWD_Data_To_Static_Env_Poly()
    */
   concrete_wall_detector.Run_Measurements(det_props, raw_detect_list, sensor_props, calib, objects, host_speed, timing_info);
   concrete_wall_detector.Map_Data_To_Static_Env_Poly(stat_env_polys, host_curvature);

   /** \result
    * Check if CWD is invalid.
    */
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, stat_env_polys[5].status);
}
/** @}*/

/** \defgroup  Init_Long_Zones
 *  @{
 */

 /** \brief
  * Test group of method Init_Long_Zones().
  */
TEST_GROUP(Init_Long_Zones)
{
   // Declare common variables used within all tests in this test group.
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]{};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_Calibrations_T calib{};
   Static_Env_Poly_T stat_env_polys[F360_NUM_OF_STATIC_ENV_POLYS]{};
   F360_Concrete_Wall_Detector_T concrete_wall_detector{};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   float32_t host_curvature = 0.0F;
   float32_t host_speed = 0.0F;
   F360_TRKR_TIMING_INFO_T timing_info = {};

   void Append_Detection(const uint32_t det_idx, const float32_t long_posn, const float32_t lat_posn)
   {
      if (det_idx > 0U)
      {
         raw_detect_list.detections[det_idx - 1U].processed.next_sorted_idx = det_idx; // first extend the list of sorted detections by one
      }
      det_props[det_idx].vcs_position.y = lat_posn;
      det_props[det_idx].vcs_position.x = long_posn;
      raw_detect_list.detections[det_idx].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[det_idx].f_ok_to_use = true;
      raw_detect_list.detections[det_idx].processed.prev_sorted_idx = det_idx - 1U;
      raw_detect_list.detections[det_idx].processed.next_sorted_idx = F360_INVALID_ID;
   }

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};


/** \purpose
 * This test verifies whether if only detections from sensor zones are used for cwd calculations.
 * This tests indirectly that sensors zone are initialized.
 * \req NA
 */
TEST(Init_Long_Zones, if_only_detections_from_sensor_zones_are_used_for_cwd_calculations)
{
   /** \precond
    * Set detections laterally close to edges of zones.
    * Sort longitudinaly these detections.
    * Set raw_detect_list data
    * Initialize CWD
    */
   calib.k_cwd_sensor_zone_half_length = 0.51F;
   // Front sensor
   sensors[0U].variable.is_valid = true;
   sensors[0U].constant.mounting_position.vcs_position.longitudinal = 0.0F;
   sensors[0U].constant.mounting_position.vcs_position.lateral = 1.0F;
   sensors[0U].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;

   // Rear sensor
   sensors[1U].variable.is_valid = true;
   sensors[1U].constant.mounting_position.vcs_position.longitudinal = -4.0F;
   sensors[1U].constant.mounting_position.vcs_position.lateral = 1.0F;
   sensors[1U].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;


   Append_Detection(0U, -4.52F, 1.0F);
   Append_Detection(1U, -4.50F, 2.0F);  // first and only detection in rear sensor zone
   Append_Detection(2U, -3.48F, 3.0F);
   Append_Detection(3U, -0.52F, 4.0F);
   Append_Detection(4U, -0.50F, 2.0F);  // first and only detection in front sensor zone
   Append_Detection(5U,  0.52F, 6.0F);

   raw_detect_list.detections[0U].processed.next_sorted_idx = 1;
   raw_detect_list.detections[1U].processed.next_sorted_idx = 2;
   raw_detect_list.detections[2U].processed.next_sorted_idx = 3;
   raw_detect_list.detections[3U].processed.next_sorted_idx = 4;
   raw_detect_list.detections[4U].processed.next_sorted_idx = 5;
   raw_detect_list.detections[5U].processed.next_sorted_idx = -1;

   raw_detect_list.vcslong_sorted_ref_det_idx[0] = 0;
   raw_detect_list.vcslong_sorted_ref_det_idx[1] = 1;
   raw_detect_list.vcslong_sorted_ref_det_idx[2] = 2;
   raw_detect_list.vcslong_sorted_ref_det_idx[3] = 3;
   raw_detect_list.vcslong_sorted_ref_det_idx[4] = 4;
   raw_detect_list.vcslong_sorted_ref_det_idx[5] = 5;

   concrete_wall_detector.Init(sensors, calib);

   float32_t expected_cwd_lat_posn = (det_props[1].vcs_position.y + det_props[4].vcs_position.y) / 2.0F;

   /** \action
    * Call Init_Long_Zones()
    * Call Run_CWD_Measurements()
    * Call Map_CWD_Data_To_Static_Env_Poly()
    */
   concrete_wall_detector.Init_Long_Zones(calib.k_cwd_sensor_zone_half_length);
   concrete_wall_detector.Run_Measurements(det_props, raw_detect_list, sensor_props, calib, objects, host_speed, timing_info);
   concrete_wall_detector.Map_Data_To_Static_Env_Poly(stat_env_polys, host_curvature);

   /** \result
    * Check if estimated lateral position of CWD side is equal to laterel position 
    * calculated based on only detections placed in sensor zones.
    * That means the zones was defined correctly and detections belong to them are used for calculations.
    */
   DOUBLES_EQUAL(expected_cwd_lat_posn, stat_env_polys[5].p0, F360_EPSILON)
}
/** @}*/
