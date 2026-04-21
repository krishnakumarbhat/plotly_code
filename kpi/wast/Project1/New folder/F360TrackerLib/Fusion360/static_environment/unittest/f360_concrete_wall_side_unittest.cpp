/** \file
 * This file contains unit tests for content of f360_concrete_wall_side.cpp file
 */

#include "f360_tracker_info.h"
#include "f360_concrete_wall_side.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  Concrete_Wall_Side_T
 *  @{
 */

 /** \brief
  * Test group of Concrete_Wall_Side_T class.
  */
TEST_GROUP(Concrete_Wall_Side_T)
{
   // Declare common variables used within all tests in this test group.
   float32_t host_curvature = 0.0F;
   float32_t host_speed = 0.0F;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]{};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Tracker_Info_T tracker_info{};
   F360_Calibrations_T calib{};
   Static_Env_Poly_T stat_env_poly{};
   Concrete_Wall_Sensor_T cwd_sensors[MAX_NUMBER_OF_SENSORS]{};
   Concrete_Wall_Side_T cw_left;
   Concrete_Wall_Side_T cw_side;
   uint32_t cwd_sensors_count = 0U;
   const uint32_t front_sensor_idx = 0U;
   const uint32_t rear_sensor_idx = 1U;
   const uint32_t front_det_idx = 0U;
   const uint32_t rear_det_idx = 1U;
   const float32_t float_tol = F360_EPSILON * 100.0F;

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

   void Init_Sensor(const uint32_t sens_idx)
   {
      if (0U == sens_idx) // front sensor
      {
         sensors[0U].constant.mounting_position.vcs_position.longitudinal = 0.0F;
      }
      else if (1U == sens_idx) // rear sensor
      {
         sensors[0U].constant.mounting_position.vcs_position.longitudinal = -4.0F;
      }
      else
      {
         // undefined sensor, do nothing
      }
      sensors[sens_idx].constant.mounting_position.vcs_position.lateral = 1.0F;
      sensors[sens_idx].variable.is_valid = true;
      cwd_sensors[sens_idx].Init(sensors, sens_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
      cwd_sensors_count++;
   }

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};


/** \purpose
 * This test verifies whether Concrete_Wall_Side_T creates empty object.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_constuctor_creates_empty_object)
{
   /** \action
    * Call tested function Concrete_Wall_Side_T().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect invalid status.
    */
   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "Output status should be invalid (0)");
}


/** \purpose
 * This test verifies whether the Reset_Side_CWD really clears all CWD side signals to initial values.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_side_data_is_reset)
{
   /** \precond
    * Init only front sensor and define single relevant detection seen by it.
    * Then add detection to sensor zone and execute measurement by this sensor.
    * Finally fuse side sensors data.
    */
   Init_Sensor(front_sensor_idx);
   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), 5.0F);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side. Fuse sensors data, which produces nonzero output values.
    * Call tested function Reset_Side_CWD() which should change all output values to initial values.
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Reset_Side_CWD();
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "Output status has not been reset");
   CHECK_EQUAL_TEXT(0U, stat_env_poly.age, "Age has not been reset");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.p0, float_tol, "Lateral position has not been reset");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.lower_limit, float_tol, "Lower limit has not been reset");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.upper_limit, float_tol, "Upper limit has not been reset");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.confidence, float_tol, "Confidence has not been reset");
}


/** \purpose
 * This test verifies whether if there is not valid measurement in any of side sensors then the
 * output status should be invalid.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_none_of_side_sensors_has_valid_measurement_then_output_status_is_invalid)
{
   /** \precond
    * Prepare array of CWD sensors with proper content.
    * Init two sensors side, but do not set any valid detection for them.
    * Then execute measurement by each sensor with no detections.
    */

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect invalid status (0) in mapped stat_env_poly.
    */
   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "CW Side status should be invalid, but is not.");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.p0, float_tol, "Lateral position has not been reset.");
}

/** \purpose
 * This test verifies whether when only front sensor has valid measurement CWD status is invalid.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_only_front_sensor_valid_measurement_is_enough_to_get_valid_lat_posn_estimate)
{
   /** \precond
    * Prepare array of CWD sensors with proper content.
    * Init two sensors on right side and define valid detections for each of them.
    * Then set detections belonging only to front sensor. Leave rear sensor with no detections.
    * Finally execute measurements by each of sensors and read the lat posn estimate of the front sensor.
    */

   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + 0.1F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);

   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "Output status should be invalid (0), but is not.");
}

/** \purpose
 * This test verifies whether when only one sensor has valid measurement and this measurement has suitable confidence level - CWD status is valid.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_only_one_sensor_valid_measurement_is_enough_to_get_valid_estimate_when_conf_is_higher_than_threshold)
{
   /** \precond
    * Prepare one CWD sensor with proper content.
    * Init this sensor and define valid detections for it.
    * Finally execute measurements four times to get expected confidence level.
    */

   const float32_t front_cw_lat_posn = 5.0F;

   Init_Sensor(front_sensor_idx);
   Append_Detection(0U, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);

   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);

   // Set confidence threshold for validate based on one sensor estimation
   calib.k_cwd_min_conf_to_confirm_cw_by_single_sensor = 0.66F;

   const float32_t expected_cw_lat_est = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Estimate();

   /** \action
    * Append defined sensor to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   CHECK_EQUAL_TEXT(F360_STATIC_ENV_POLY_STATUS_UPDATED, stat_env_poly.status, "Output status should be valid (1), but is not.");
   DOUBLES_EQUAL(expected_cw_lat_est, stat_env_poly.p0, 0.0001F);
}

/** \purpose
 * This test verifies whether when only rear sensor has valid measurement CWD status is invalid.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_only_rear_sensor_valid_measurement_is_enough_to_get_valid_lat_posn_estimate)
{
   /** \precond
    * Prepare array of CWD sensors with proper content.
    * Init two sensors on right side and define valid detections for each of them.
    * Then set detections belonging only to rear sensor. Leave front sensor with no detections.
    * Finally execute measurements by each of two sensors.
    */

   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + 0.1F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[rear_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[rear_det_idx].vcs_position.x, rear_det_idx);

   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "Output status should be invalid (0), but is not.");
}


/** \purpose
 * This test verifies whether if the spread of later position estimates from all sensors is low enough, then the fused CW lat posn 
 * confidence is the arithmetic mean of individual sensor lat posn confidences. 
 * At the same time fused CW lat posn should be weighted mean of sensor lat posn estimates.
 * Host speed and curvature is set to zero in test group so we expect a straight line that extends
 * the minimum length in longitudinal direction
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_for_valid_measurements_lat_spread_below_lower_limit_lat_posn_confidence_is_arithmetic_mean_of_both)
{
   /** \precond
    * Prepare array cwd_sensors with proper content.
    * Init two sensors on right side and define 2 valid detections for them
    * to have the lat posn spread of detections slightly below lower limit (k_cwd_side_dist_span_low_limit).
    * Execute measurements on by front sensor 5 times with the same detection to increase lat posn confidence to 1.
    * Execute measurements on by front sensor 2 times with the same detection to increase lat posn confidence to 0.2.
    */

   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + calib.k_cwd_side_dist_span_low_limit - 0.01F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   const float32_t front_cw_lat_conf = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Confidence();
   const float32_t front_cw_lat_est = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Estimate();

   cwd_sensors[rear_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[rear_det_idx].vcs_position.x, rear_det_idx);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);
   const float32_t rear_cw_lat_conf = cwd_sensors[rear_sensor_idx].Get_CW_Lat_Posn_Confidence();
   const float32_t rear_cw_lat_est = cwd_sensors[rear_sensor_idx].Get_CW_Lat_Posn_Estimate();

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data(). 
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   float32_t expected_cw_lat_conf = (front_cw_lat_conf + rear_cw_lat_conf) / (1.0F + 1.0F); // arithmetic mean of confidences
   float32_t expected_cw_lat_posn = (front_cw_lat_est * front_cw_lat_conf + rear_cw_lat_est * rear_cw_lat_conf) / (front_cw_lat_conf + rear_cw_lat_conf); // weighted mean
   float32_t expected_upper_limit = cwd_sensors[0U].Get_Sensor_Long_Posn() + calib.k_cwd_min_longitudinal_validity_limit;
   float32_t expected_lower_limit = cwd_sensors[1U].Get_Sensor_Long_Posn() - calib.k_cwd_min_longitudinal_validity_limit;

   CHECK_EQUAL_TEXT(1, stat_env_poly.status, "Output status should be valid (1), but is not.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_conf, stat_env_poly.confidence, float_tol, "Lateral position confidence should be arithmetic mean of all side sensors confidences.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_posn, stat_env_poly.p0, float_tol, "Lateral position estimate should be weighted mean of all side sensors position estimates.");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.p1, float_tol, "SEP p1 coefficient should be zero.");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.p2, float_tol, "SEP p2 coefficient should be zero");
   DOUBLES_EQUAL_TEXT(expected_upper_limit, stat_env_poly.upper_limit, float_tol, "Upper validity limit should be equal to half of sensor zone width.");
   DOUBLES_EQUAL_TEXT(expected_lower_limit, stat_env_poly.lower_limit, float_tol, "Lower validity limit should be equal to half of sensor zone width.");
}


/** \purpose
 * This test verifies whether if the spread of later position estimates from all sensors is in transition area (between lower and upper limits),
 * then the fused CW lat posn confidence is the minimum of individual sensor lat posn confidences. 
 * At the same time fused CW lat posn should be weighted mean of sensor lat posn estimates.
 * In this test we also modify host speed and curvature so we expect the maximum allowed extension in longitudinal direction and
 * that the p2 coefficient of the SEP is equal to 0.5*host_curvature.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_for_valid_measurements_lat_spread_below_upper_limit_lat_posn_confidence_is_minimum_of_both)
{
   /** \precond
    * Prepare array cwd_sensors with proper content.
    * Init two sensors on right side and define 2 valid detections for them
    * to have the lat posn spread of detections slightly below upper limit (k_cwd_side_dist_span_high_limit).
    * Execute measurements on by front sensor 5 times with the same detection to increase lat posn confidence to 1.
    * Execute measurements on by front sensor 2 times with the same detection to increase lat posn confidence to 0.2.
    * Set host curvature to something arbitrary different from zero
    * Set host speed greater than threshold for maximum longitudinal extension
    */

   host_curvature = 0.1F;
   host_speed = calib.k_host_speed_for_max_long_validity_limit + 1.0F;
   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + calib.k_cwd_side_dist_span_high_limit - 0.01F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   const float32_t front_cw_lat_conf = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Confidence();
   const float32_t front_cw_lat_est = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Estimate();

   cwd_sensors[rear_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[rear_det_idx].vcs_position.x, rear_det_idx);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);
   const float32_t rear_cw_lat_conf = cwd_sensors[rear_sensor_idx].Get_CW_Lat_Posn_Confidence();
   const float32_t rear_cw_lat_est = cwd_sensors[rear_sensor_idx].Get_CW_Lat_Posn_Estimate();

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect following values in mapped output.
    */
   float32_t expected_cw_lat_conf = fminf(front_cw_lat_conf, rear_cw_lat_conf); // minimum of confidences
   float32_t expected_cw_lat_posn = (front_cw_lat_est * front_cw_lat_conf + rear_cw_lat_est * rear_cw_lat_conf) / (front_cw_lat_conf + rear_cw_lat_conf); // weighted mean
   float32_t expected_upper_limit = cwd_sensors[0U].Get_Sensor_Long_Posn() + calib.k_cwd_max_longitudinal_validity_limit;
   float32_t expected_lower_limit = cwd_sensors[1U].Get_Sensor_Long_Posn() - calib.k_cwd_max_longitudinal_validity_limit;

   CHECK_EQUAL_TEXT(1, stat_env_poly.status, "Output status should be valid (1), but is not.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_conf, stat_env_poly.confidence, float_tol, "Lateral position confidence should be arithmetic mean of all side sensors confidences.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_posn, stat_env_poly.p0, float_tol, "Lateral position estimate should be weighted mean of all side sensors position estimates.");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.p1, float_tol, "SEP p1 coefficient should be zero.");
   DOUBLES_EQUAL_TEXT(0.5F * host_curvature, stat_env_poly.p2, float_tol, "SEP p2 coefficient should be half of host curvature");
   DOUBLES_EQUAL_TEXT(expected_upper_limit, stat_env_poly.upper_limit, float_tol, "Upper validity limit should be equal to half of sensor zone width.");
   DOUBLES_EQUAL_TEXT(expected_lower_limit, stat_env_poly.lower_limit, float_tol, "Lower validity limit should be equal to half of sensor zone width.");
}

/** \purpose
 * This test verifies whether when the two or more sensors cwd lateral position estimates are valid, spread of them is above upper limit, but confidence is high enough,
 * then the fused CW lat posn confidence is calculated based on closest to host estimate.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_for_two_or_more_sensor_valid_estimates_cw_side_is_calculated_based_on_closer_to_host)
{
   /** \precond
    * Prepare array cwd_sensors with proper content.
    * Init two sensors on right side and define 2 valid detections for them
    * to have the lat posn spread of detections slightly above upper limit (k_cwd_side_dist_span_high_limit).
    * Execute measurements for front sensor 5 times with the same detection to increase lat posn confidence to 1.
    * Execute measurements for rear sensor 2 times with the same detection to increase lat posn confidence to 0.2.
    */

   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + calib.k_cwd_side_dist_span_high_limit + 0.01F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);

   cwd_sensors[rear_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[rear_det_idx].vcs_position.x, rear_det_idx);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect valid CW status and following values in mapped output.
    */
   float32_t expected_cw_lat_conf = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Confidence() * calib.k_cwd_one_sensor_at_side_conf_factor;
   float32_t expected_cw_lat_posn = cwd_sensors[front_sensor_idx].Get_CW_Lat_Posn_Estimate();  // lateral position mean weighted by confidences

   CHECK_EQUAL_TEXT(1, stat_env_poly.status, "SEP status should be set to UPDATED (1), but is not.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_conf, stat_env_poly.confidence, float_tol, "SEP confidence should be equal to nearest to host cw sensor confidence multiplyed by calibration coeficient.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_posn, stat_env_poly.p0, float_tol, "SEP lateral position estimate should be equal to nearest to host cw sensor position estimate.");
}

/** \purpose
 * This test verifies whether when the two or more sensors cwd lateral position estimations are valid but spread of them is above upper limit,
 * the status of cw side estimate is not valid when CW lat posn confidence is below calibration parameter k_cwd_min_conf_to_confirm_cw_by_single_sensor.
 * \req NA
 */
TEST(Concrete_Wall_Side_T, check_if_for_two_or_more_sensor_valid_estimations_cw_side_is_invalid_when_low_confidence)
{
   /** \precond
    * Prepare array cwd_sensors with proper content.
    * Init two sensors on right side and define 2 valid detections for them
    * to have the lat posn spread of detections slightly above upper limit (k_cwd_side_dist_span_high_limit).
    * Execute measurements on by front sensor 3 times with the same detection to increase lat posn confidence to 0.4.
    * Execute measurements on by front sensor 2 times with the same detection to increase lat posn confidence to 0.2.
    */

   const float32_t front_cw_lat_posn = 5.0F;
   const float32_t rear_cw_lat_posn = front_cw_lat_posn + calib.k_cwd_side_dist_span_high_limit + 0.01F;

   Init_Sensor(front_sensor_idx);
   Init_Sensor(rear_sensor_idx);

   Append_Detection(front_det_idx, cwd_sensors[front_sensor_idx].Get_Sensor_Long_Posn(), front_cw_lat_posn);
   Append_Detection(rear_det_idx, cwd_sensors[rear_sensor_idx].Get_Sensor_Long_Posn(), rear_cw_lat_posn);

   cwd_sensors[front_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[front_det_idx].vcs_position.x, front_det_idx);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);
   cwd_sensors[front_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[front_sensor_idx], calib, objects);

   cwd_sensors[rear_sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[rear_det_idx].vcs_position.x, rear_det_idx);
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects); // Measurement is repeated several times to increase measurement confidence
   cwd_sensors[rear_sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[rear_sensor_idx], calib, objects);

   /** \action
    * Append defined sensors to the side.
    * Call tested function Fuse_CWD_Side_Data().
    * Then call mapping function to be able to see the result.
    */
   cw_side.Append_Sensor_Idx(front_sensor_idx);
   cw_side.Append_Sensor_Idx(rear_sensor_idx);
   cw_side.Fuse_CWD_Side_Data(cwd_sensors, host_speed, calib);
   cw_side.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_poly, host_curvature);

   /** \result
    * We expect invalid status and following values in mapped output.
    */
   const float32_t expected_cw_lat_posn = 0.0F;

   CHECK_EQUAL_TEXT(0, stat_env_poly.status, "Output status should be set to INVALID (0), but is not.");
   DOUBLES_EQUAL_TEXT(0.0F, stat_env_poly.confidence, float_tol, "Expected CW lateral position confidence should be equal to 0.");
   DOUBLES_EQUAL_TEXT(expected_cw_lat_posn, stat_env_poly.p0, float_tol, "Lateral position estimate should be set to 0.");
}

/** @}*/
