/** \file
 * This file contains unit tests for content of f360_concrete_wall_sensor.cpp file
 */

#include "f360_concrete_wall_sensor.h"
#include <CppUTest/TestHarness.h>

 // Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  Init
 *  @{
 */

 /** \brief
  * It checks method Init() via public interface
  */
TEST_GROUP(Init)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   /** \setup
    * Init calibrations
    * Add one sensor the the right side
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;
   }
};

/** \purpose
 * Check if default values are set by initialization (sensor on the right)
 */
TEST(Init, Default_values_right_sensor)
{
   /** \precond
    * Same as test group setup
    */
   Concrete_Wall_Sensor_T cw_sensor;

   /** \action
    * Call Init()
    */
   cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);

   /** \result
    * Concrete wall should return:
    * Sensor longitudinal position: 1.54
    * Sensor side: 1.0 (right side)
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   DOUBLES_EQUAL(-0.15F, cw_sensor.Get_Sensor_Long_Posn(), floating_th);
   DOUBLES_EQUAL(1.0F, cw_sensor.Get_Sensor_Side_Sign(), floating_th);
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if defualt values are set by initialization (sensor on the left)
 */
TEST(Init, Default_values_left_sensor)
{
   /** \precond
    * Same as test group setup
    */
   sensors[sensor_idx].constant.mounting_position.vcs_position = { 1.54F, -2.0F, 3.0F };
   Concrete_Wall_Sensor_T cw_sensor;

   /** \action
    * Call Init()
    */
   cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);

   /** \result
    * Concrete wall should return:
    * Sensor longitudinal position: 1.54
    * Sensor side: -1.0 (left side)
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   DOUBLES_EQUAL(1.54F, cw_sensor.Get_Sensor_Long_Posn(), floating_th);
   DOUBLES_EQUAL(-1.0F, cw_sensor.Get_Sensor_Side_Sign(), floating_th);
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/


/** \defgroup  Execute_Measurements
 *  @{
 */

 /** \brief
  * It checks method Execute_Measurements() via public interface
  */
TEST_GROUP(Execute_Measurements)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calib;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   Concrete_Wall_Sensor_T cw_sensor;

   /** \setup
    * Init calibrations
    * Add one sensor the the right side
    * Add one valid detection
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;
      det_props[0].vcs_position.y = 1.1F;
      raw_detect_list.detections[0].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Checks if detection is laterally further than object (next to sensor) is not used by concrete wall (right side)
 */
TEST(Execute_Measurements, object_next_to_sensor_and_detection_is_laterally_further_right_side)
{
   /** \precond
    * Setup sensor - object next to sensor laterally closer than detection
    * Add one valid detection to concrete wall class object
    */
   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = det_props[0].vcs_position.y - position_offset;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Checks if detection is laterally closer than object (next to sensor) is used by concrete wall (right side)
 */
TEST(Execute_Measurements, object_next_to_sensor_and_detection_is_laterally_closer_right_side)
{
   /** \precond
    * Setup sensor - object next to sensor laterally further than detection
    * Add one valid detection to concrete wall class object
    */
   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = det_props[0].vcs_position.y + position_offset;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.1
    * Confidence 0.066666666666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
* Checks if detection is laterally further than object (next to sensor) is not used by concrete wall (left side)
 */
TEST(Execute_Measurements, object_next_to_sensor_and_detection_is_laterally_further_left_side)
{
   /** \precond
    * Sensor and detection is on the left side
    * Setup sensor - object next to sensor laterally closer than detection
    * Add one valid detection to concrete wall class object
    */
   det_props[0].vcs_position.y *= -1.0F;
   sensors[sensor_idx].constant.mounting_position.vcs_position.lateral *= -1.0F;

   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = det_props[0].vcs_position.y + position_offset;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;


   cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Checks if detection is laterally closer than object (next to sensor) is used by concrete wall (left side)
 */
TEST(Execute_Measurements, object_next_to_sensor_and_detection_is_laterally_closer_left_side)
{
   /** \precond
    * Sensor and detection is on the left side
    * Setup sensor - object next to sensor laterally further than detection
    * Add one valid detection to concrete wall class object
    */
   det_props[0].vcs_position.y *= -1.0F;
   sensors[sensor_idx].constant.mounting_position.vcs_position.lateral *= -1.0F;

   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = det_props[0].vcs_position.y - position_offset;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;

   sensors[sensor_idx].constant.mounting_position.vcs_position.lateral = -1.0F;
   cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: -1.1
    * Confidence 0.066666666666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(-1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Checks if detection is correctly handled when there is no object next to sensor (due to object next to sensor flag is false)
 */
TEST(Execute_Measurements, no_object_next_to_sensor_due_to_flag)
{
   /** \precond
    * Setup sensor - no object next to sensor
    * Add one valid detection to concrete wall class object
    */
   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = false;
   sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.1
    * Confidence 0.06666666666666666666F
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.06666666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Checks if detection is correctly handled when there is no object next to sensor (due to object next to sensor breaks min limit)
 */
TEST(Execute_Measurements, no_object_next_to_sensor_due_to_break_min_limit)
{
   /** \precond
    * Setup sensor - object next to sensor little longitudinally forward
    * Add one valid detection to concrete wall class object
    */
   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.1
    * Confidence 0.066666666666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Checks if detection is correctly handled when there is no object next to sensor (due to object next to sensor breaks max limit)
 */
TEST(Execute_Measurements, no_object_next_to_sensor_due_to_break_max_limit)
{
   /** \precond
    * Setup sensor - object next to sensor little longitudinally behind
    * Add one valid detection to concrete wall class object
    */
   F360_Radar_Sensor_Props_T sensor_props = {};
   sensor_props.f_object_track_next_to_sensor = true;
   sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_max_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length - position_offset;
   sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
   sensor_props.next_to_sensor_object_track_min_long_pos = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;

   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.1
    * Confidence 0.066666666666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/


/** \defgroup  Calc_Lateral_Position_Confidence
 *  @{
 */

 /** \brief
  * It checks method Calc_Lateral_Position_Confidence() via public interface
  */
TEST_GROUP(Calc_Lateral_Position_Confidence)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   F360_Radar_Sensor_Props_T sensor_props = {};

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add 5 valid detections
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      const float relevant_det_long_position = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = relevant_det_long_position;
      det_props[0].vcs_position.y = 1.1F;
      raw_detect_list.detections[0].processed.next_sorted_idx = 1;
      raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[1].f_ok_to_use = true;
      det_props[1].vcs_position.x = relevant_det_long_position;
      det_props[1].vcs_position.y = 1.1F;
      raw_detect_list.detections[1].processed.next_sorted_idx = 2;
      raw_detect_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[2].f_ok_to_use = true;
      det_props[2].vcs_position.x = relevant_det_long_position;
      det_props[2].vcs_position.y = 1.1F;
      raw_detect_list.detections[2].processed.next_sorted_idx = 3;
      raw_detect_list.detections[2].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[3].f_ok_to_use = true;
      det_props[3].vcs_position.x = relevant_det_long_position;
      det_props[3].vcs_position.y = 1.1F;
      raw_detect_list.detections[3].processed.next_sorted_idx = 4;
      raw_detect_list.detections[3].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[4].f_ok_to_use = true;
      det_props[4].vcs_position.x = relevant_det_long_position;
      det_props[4].vcs_position.y = 1.1F;
      raw_detect_list.detections[4].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[4].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence when there is no detection
 */
TEST(Calc_Lateral_Position_Confidence, Zero_confidence_if_no_det)
{
   /** \precond
    * Same as test group setup
    */

    /** \action
     * Call Execute_Measurements()
     */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence 0.0
    * Valid estimate flag: False
    */
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for one detection
 */
TEST(Calc_Lateral_Position_Confidence, One_detection)
{
   /** \precond
    * Same as test group setup
   * Set detection in concrete wall sensor class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for two detections
 */
TEST(Calc_Lateral_Position_Confidence, Two_detections)
{
   /** \precond
    * Same as test group setup
    * Set two detections in concrete wall sensor class object (in different scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 0.2
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(0.2F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for three detections
 */
TEST(Calc_Lateral_Position_Confidence, Three_detections)
{
   /** \precond
    * Same as test group setup
    * Set three detections in concrete wall sensor class object (in different scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[2].vcs_position.x, 2);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 0.4
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(0.4F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for four detections
 */
TEST(Calc_Lateral_Position_Confidence, Four_detections)
{
   /** \precond
    * Same as test group setup
    * Set four detections in concrete wall sensor class object (in different scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[2].vcs_position.x, 2);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[3].vcs_position.x, 3);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 0.6666666666666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(0.6666666666666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for five detections
 */
TEST(Calc_Lateral_Position_Confidence, Five_detections)
{
   /** \precond
    * Same as test group setup
    * Set five detections in concrete wall sensor class object (in different scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[2].vcs_position.x, 2);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[3].vcs_position.x, 3);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[4].vcs_position.x, 4);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 1.0
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}


/** \purpose
 * Check if confidence is correctly calcualted as composition of age and spread confidence for one detection
 * with additional spread in detections lateral position
 */
TEST(Calc_Lateral_Position_Confidence, Five_detections_and_some_spread)
{
   /** \precond
    * Same as test group setup
    * Set five detections in concrete wall sensor class object (in different scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[2].vcs_position.x, 2);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[3].vcs_position.x, 3);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   det_props[4].vcs_position.y = 1.5F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[4].vcs_position.x, 4);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Confidence: 0.8333334F
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(0.8333334F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/

/** \defgroup  Set_Dets_Inside_Sensor_Zone_Edges
 *  @{
 */

 /** \brief
  * It checks method Set_Dets_Inside_Sensor_Zone_Edges() via public interface
  */
TEST_GROUP(Set_Dets_Inside_Sensor_Zone_Edges)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_Props_T sensor_props = {};

   float det_long_posn_below_limits;
   float det_long_posn_above_limits;

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add two valid detections
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length - position_offset;
      det_props[0].vcs_position.y = 1.2F;
      raw_detect_list.detections[0].processed.next_sorted_idx = 1;
      raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[1].f_ok_to_use = true;
      det_props[1].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length - position_offset;
      det_props[1].vcs_position.y = 1.3F;
      raw_detect_list.detections[1].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;


      det_long_posn_below_limits = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length - position_offset;
      det_long_posn_above_limits = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal + calib.k_cwd_sensor_zone_half_length + position_offset;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Check if one detection, which is within concrete wall sensor limits, is correctly added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, First_Detection_Within_Limits)
{
   /** \precond
    * Same as test group setup
    * Add one detection to concrete wall class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 *  Check if one detection, which is below concrete wall sensor longitudinal limits, is NOT added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, First_Detection_Below_Limits)
{
   /** \precond
    * Add one detection (which longitudinal position below limits) to concrete wall class object
    */
   det_props[0].vcs_position.x = det_long_posn_below_limits;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Init()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Valid estimate flag: False
    */
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 *  Check if one detection, which is above concrete wall sensor limits, is NOT added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, First_Detection_Above_Limits)
{
   /** \precond
    * Add one detection (which longitudinal position above limits) to concrete wall class object
    */
   det_props[0].vcs_position.x = det_long_posn_above_limits;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Valid estimate flag: False
    */
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 *  Check if next detection (first already added), which is within concrete wall sensor limits, is correctly added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, Next_Detection_Within_Limits)
{
   /** \precond
    * Add two detections (both longitudinal position are within limits) to concrete wall class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2 (first detection lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 *  Check if next detection (first already added), which is below concrete wall sensor limits, is NOT added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, Next_Detection_Below_Limits)
{
   /** \precond
    * Add first detection (which longitudinal position is within limits) to concrete wall class object
    * Add next detection (which longitudinal position is below limits) to concrete wall class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   det_props[1].vcs_position.x = det_long_posn_below_limits;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2 (first detection lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
*  Check if next detection (first already added), which is above concrete wall sensor limits, is NOT added
 */
TEST(Set_Dets_Inside_Sensor_Zone_Edges, Next_Detection_Above_Limits)
{
   /** \precond
    * Add first detection (which longitudinal position is within limits) to concrete wall class object
    * Add next detection (which longitudinal position is above limits) to concrete wall class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   det_props[1].vcs_position.x = det_long_posn_above_limits;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2 (first detection lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/


/** \defgroup  Update_Aggregated_Lat_Posn
 *  @{
 */

 /** \brief
  * If checks method Update_Aggregated_Lat_Posn() via public interface
  */
TEST_GROUP(Update_Aggregated_Lat_Posn)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_Props_T sensor_props = {};

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add two valid detections
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;
      det_props[0].vcs_position.y = 1.2F;
      raw_detect_list.detections[0].processed.next_sorted_idx = 1;
      raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[1].f_ok_to_use = true;
      det_props[1].vcs_position.x = det_props[0].vcs_position.x + position_offset;
      det_props[1].vcs_position.y = 1.8F;
      raw_detect_list.detections[1].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[1].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Check if method returns aggregated lateral posn as mean value
 */
TEST(Update_Aggregated_Lat_Posn, Calculate_Mean)
{
   /** \precond
    * Add two valid detections to concrete wall class object (in two next scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.5
    */
   DOUBLES_EQUAL(1.5F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
}

/** \purpose
 * Check if method returns aggregated lateral posn as NaN if there is no any detection
 */
TEST(Update_Aggregated_Lat_Posn, Return_NaN_due_to_no_det)
{
   /** \precond
    * Same as test group setup
    */

    /** \action
     * Call Execute_Measurements()
     */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
}

/** \purpose
 * Check if method returns aggregated lateral posn as NaN if detection lateral position spread is high
 */
TEST(Update_Aggregated_Lat_Posn, Return_NaN_due_to_high_lateral_spread)
{
   /** \precond
    * Add two valid detections (with high lateral position difference) to concrete wall class object (in two next scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   det_props[1].vcs_position.y = det_props[0].vcs_position.y + 1.0F + position_offset;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
}

/** @}*/

/** \defgroup  Update_CWD_Sensor
 *  @{
 */

 /** \brief
  * It tests method Update_CWD_Sensor() via public interface
  */
TEST_GROUP(Update_CWD_Sensor)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_Props_T sensor_props = {};

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add two valid detections
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;
      det_props[0].vcs_position.y = 1.2F;
      raw_detect_list.detections[0].processed.next_sorted_idx = 1;
      raw_detect_list.detections[0].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[1].f_ok_to_use = true;
      det_props[1].vcs_position.x = det_props[0].vcs_position.x + position_offset;
      det_props[1].vcs_position.y = 1.4F;
      raw_detect_list.detections[1].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[1].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Check if concrete wall returns default/invalid output if aggregated lateral position is not valid
 */
TEST(Update_CWD_Sensor, Default_output_due_to_not_valid_aggregated_lat_pos)
{
   /** \precond
    * Same as test group setup
    */

    /** \action
     * Call Execute_Measurements()
     */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if concrete wall returns its position as position of the latest used detection
 */
TEST(Update_CWD_Sensor, Return_Closest_Lat_Posn)
{
   /** \precond
    * Add two valid detections to concrete wall class object (in two next scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.4 (second detection lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.4F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
* Check if concrete wall returns its position as aggregated lateral position
* when detections lateral position spread is high
 */
TEST(Update_CWD_Sensor, Return_Aggregated_Lat_Posn_due_to_high_spread)
{
   /** \precond
    * Add two valid detections (laterally close enough) to concrete wall class object (in two next scans)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   det_props[1].vcs_position.y = 1.6F + position_offset;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.405 (aggregated lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.405F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
* Check if concrete wall returns its position as aggregated lateral position
* when there is no valid detection in current scan
 */
TEST(Update_CWD_Sensor, Return_Aggregated_Lat_Posn_due_to_invalid_last_sample)
{
   /** \precond
    * Add two valid detections (laterally close enough) to concrete wall class object (in two next scans)
    * Reset sensor in order to imitate lack of detection in next (third) scan
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   cw_sensor.Reset(calib.k_cwd_max_lateral_range);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.3 (aggregated lateral position)
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.3F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/

/** \defgroup  Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor
 *  @{
 */

 /** \brief
  * It tests private method Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor() via public interface
  */
TEST_GROUP(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_Props_T sensor_props = {};

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add two valid detections
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;
      det_props[0].vcs_position.y = 1.2F;
      raw_detect_list.detections[0].processed.next_sorted_idx = 1;
      raw_detect_list.detections[0].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[1].f_ok_to_use = true;
      det_props[1].vcs_position.x = det_props[0].vcs_position.x + position_offset;
      det_props[1].vcs_position.y = 1.4F;
      raw_detect_list.detections[1].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[1].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * No internal data is filled when there is no single det so far
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, No_Det)
{
   /** \precond
    * Same as test group setup
    */

    /** \action
     * Call Execute_Measurements()
     */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Correctly filled internal data for one detection
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, One_det)
{
   /** \precond
    * Add one valid detection to concrete wall class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * No internal data is filled when detection is on the wrong side of the specific sensor
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, One_det_on_wrong_side)
{
   /** \precond
    * Add one valid detection (on the left side, sensor is on the right) to concrete wall class object
    */
   det_props[0].vcs_position.y = -1.2F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check chosing the closest lateral detection if first detection is laterally closer
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, Two_dets_and_second_det_laterally_further)
{
   /** \precond
    * Add two detections to concrete wall class object (first detection laterally closer than second)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2 (first detection lateral position)
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check chosing the closest lateral detection if second detection is laterally closer
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, Two_dets_and_second_det_laterally_closer)
{
   /** \precond
    * Add two detections to concrete wall class object (second detection laterally closer than first)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   det_props[1].vcs_position.y = 1.1F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.1 (second detection lateral position)
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.1F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 *  No internal data is filled when detection is not valid
 */
TEST(Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor, Two_dets_and_second_det_is_invalid)
{
   /** \precond
    * Add two detections to concrete wall class object (second detection is not ok to use)
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   det_props[1].f_ok_to_use = false;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[1].vcs_position.x, 1);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2 (first detection lateral position)
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/

/** \defgroup  Is_Detection_Valid
 *  @{
 */

 /** \brief
  * It tests private method Is_Detection_Valid() via public interface
  */
TEST_GROUP(Is_Detection_Valid)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Calibrations_T calib;
   const float floating_th = F360_EPSILON;
   const float position_offset = 0.01F;
   const unsigned sensor_idx = 0U;

   Concrete_Wall_Sensor_T cw_sensor;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list = {};
   F360_Radar_Sensor_Props_T sensor_props = {};

   /** \setup
    * Init calibrations
    * Init sensor props (no object next to sensor)
    * Add one sensor to the right side
    * Add one valid detection
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensor_props.f_object_track_next_to_sensor = false;
      sensor_props.next_to_sensor_object_track_max_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_max_long_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_lat_pos = 0.0F;
      sensor_props.next_to_sensor_object_track_min_long_pos = 0.0F;

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal - calib.k_cwd_sensor_zone_half_length + position_offset;
      det_props[0].vcs_position.y = 1.2F;
      raw_detect_list.detections[0].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[0].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
   }
};

/** \purpose
 * Check if method returns true (valid detection)
 */
TEST(Is_Detection_Valid, Valid_det)
{
   /** \precond
    * Set detection in concrete wall sensor class object
    */
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns true when detection is associated to a non-moveable track
 */
TEST(Is_Detection_Valid, Valid_det_associated_to_non_moveable_track)
{
   /** \precond
    * Set detection as associated to track
    * Set detection in concrete wall sensor class object
    * Set the associated object as non-moveable
    */
   det_props[0].object_track_id = 1;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   objects[det_props[0].object_track_id - 1].f_moveable = false;

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2
    * Confidence 0.066666666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.066666666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is associated to a moveable track
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_associated_to_track)
{
   /** \precond
    * Set detection as associated to track
    * Set detection in concrete wall sensor class object
    * Set the associated object as moveable
    */
   det_props[0].object_track_id = 1;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   objects[det_props[0].object_track_id - 1].f_moveable = true;

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is not ok to use
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_not_ok_to_use)
{
   /** \precond
    * Set detection as not ok to use
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_ok_to_use = false;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection's motion status is moving
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_wrong_motion_status)
{
   /** \precond
    * Set detection's motion status to moving
    * Set detection in concrete wall sensor class object
    */
   raw_detect_list.detections[0].processed.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is double bounce
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_double_bounce)
{
   /** \precond
    * Set detection as double bounce
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_double_bounce = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is water spray
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_water_spray)
{
   /** \precond
    * Set detection as water spray
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_water_spray = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is stationary bounce
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_stationary_bounce)
{
   /** \precond
    * Set detection as stationary bounce
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_stationary_bounce = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is object angle jump
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_object_angle_jump)
{
   /** \precond
    * Set detection as object angle jump
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_object_based_angle_jump = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if method returns false when detection is marked as f_azimuth_rdot_outlier
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_marked_as_f_azimuth_rdot_outlier)
{
   /** \precond
    * Set detection as f_azimuth_rdot_outlier
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_azimuth_rdot_outlier = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: NaN
    * Confidence 0.0
    * Valid estimate flag: False
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}


/** \purpose
 * Check if CWD use detections marked as f_close_target
 */
TEST(Is_Detection_Valid, Invalid_det_due_to_being_marked_as_f_close_target)
{
   /** \precond
    * Set detection as f_close_target
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_close_target = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2
    * Confidence 0.0666666
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(0.0666666F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}


/** \purpose
 * Check if CW estimation is valid when detection is marked as f_bistatic
 */
TEST(Is_Detection_Valid, Valid_det_due_to_being_marked_as_f_bistatic)
{
   /** \precond
    * Set detection as f_bistatic
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_ok_to_use = false;
   raw_detect_list.detections[0].raw.f_bistatic = true;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2m
    * Valid estimate flag: true
    */
   CHECK_FALSE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
 * Check if CW estimation is valid when detection is on SEP
 */
TEST(Is_Detection_Valid, Valid_det_due_to_being_on_SEP)
{
   /** \precond
    * Set detection to being on sep
    * Set detection in concrete wall sensor class object
    */
   det_props[0].f_ok_to_use = false;
   det_props[0].on_sep_id = 1U;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);

   /** \action
    * Call Execute_Measurements()
    */
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * Concrete wall should return:
    * Lateral position: 1.2m
    * Valid estimate flag: true
    */
   CHECK_FALSE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(1.2F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** @}*/

/** \defgroup  Check_Lat_Pos_Buffer_For_Outlier
 *  @{
 */

 /** \brief
  * It checks method Check_Lat_Pos_Buffer_For_Outlier() via public interface
  */
TEST_GROUP(Check_Lat_Pos_Buffer_For_Outlier)
{
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calib;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Radar_Sensor_Props_T sensor_props = {};
   const float floating_th = F360_EPSILON;
   const unsigned sensor_idx = 0U;

   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   Concrete_Wall_Sensor_T cw_sensor;

   /** \setup
    * Init calibrations
    * Add one sensor the the right side
    * Init concrete wall sensor class object
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      sensors[sensor_idx].constant.id = 1;
      sensors[sensor_idx].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
      sensors[sensor_idx].constant.mounting_position.vcs_position = { -0.15F, 1.0F, 0.0F };
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_azimuth_angle = F360_DEG2RAD(60.0F);
      sensors[sensor_idx].constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

      cw_sensor.Init(sensors, sensor_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);

      det_props[0].f_ok_to_use = true;
      det_props[0].vcs_position.x = sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal;
      det_props[0].vcs_position.y = 0.0F;
      raw_detect_list.detections[0].processed.next_sorted_idx = F360_INVALID_ID;
      raw_detect_list.detections[0].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;

   }
};

/** \purpose
* Checks if cw lateral position estimate is calculated properly when one outliered sample is present inside samples buffer.
* Minimal number of samples inside buffer to outlier finding works is: 3
*/
TEST(Check_Lat_Pos_Buffer_For_Outlier, if_cw_sensor_lat_posn_est_valid_when_one_outlier_is_present)
{
   /** \precond
    * Push two sample with similar lateral position into buffer.
    */
   
   det_props[0].vcs_position.y = 2.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
   cw_sensor.Reset(999.0F);

   det_props[0].vcs_position.y = 2.5F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
   cw_sensor.Reset(999.0F);

   /** \action
    * Push outlier sample and run Execute_Measurements().
   */
   det_props[0].vcs_position.y = 4.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * cw_sensor should should has:
    * Lateral position estimate: 2.25
    * Laterally closest measurement: equal to outlier lat posn (4.0)
    * Confidence 0.133333340
    * Valid estimate flag: True
    */
   DOUBLES_EQUAL(2.25F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(det_props[0].vcs_position.y, cw_sensor.Get_Laterally_Closest_Measurement().lateral_position, floating_th);
   DOUBLES_EQUAL(0.133333340F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
* Checks if cw lateral position estimate is not calculated when there is not only one outliered sample inside samples buffer.
* Minimal number of samples inside buffer to outlier finding works is: 3
*/
TEST(Check_Lat_Pos_Buffer_For_Outlier, if_cw_sensor_lat_posn_est_is_not_valid_when_more_than_one_outlier_sample)
{
   /** \precond
    * Set detection params to fill sample buffer and run Execute_Measurements() three times.
    * Third sample is outlier.
    */
   
   det_props[0].vcs_position.y = 2.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
   cw_sensor.Reset(999.0F);

   det_props[0].vcs_position.y = 2.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
   cw_sensor.Reset(999.0F);

   det_props[0].vcs_position.y = 4.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
   cw_sensor.Reset(999.0F);

   /** \action
   * Push fourth sample equal to previous which is also outlier and run Execute_Measurements().
   */
   det_props[0].vcs_position.y = 4.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * cw_sensor should should has:
    * Lateral position estimate: nan
    * Laterally closest measurement: equal to outlier lat posn (4.0)
    * Confidence 0.0
    * Valid estimate flag: FALSE
    */
   CHECK_TRUE(std::isnan(cw_sensor.Get_CW_Lat_Posn_Estimate()));
   DOUBLES_EQUAL(det_props[0].vcs_position.y, cw_sensor.Get_Laterally_Closest_Measurement().lateral_position, floating_th);
   DOUBLES_EQUAL(0.0F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_FALSE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}

/** \purpose
* Checks if cw lateral position estimate is calculated properly when more than 5 sampes pushed to buffer(buffer starts overwrite).
* Last sample pushed to buffer is outlier.
* Minimal number of samples inside buffer to outlier finding works is: 3
*/
TEST(Check_Lat_Pos_Buffer_For_Outlier, if_cw_sensor_lat_posn_est_properly_when_sensor_buffer_overwrites)
{
   /** \precond
    * Push five samples into samples buffer to fill it fully.
    */
   for (unsigned i = 0; i < 5; ++i)
   {
      det_props[0].vcs_position.y = 2.0F;
      cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
      cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);
      cw_sensor.Reset(999.0F);
   }

   /** \action
    * Push next sample to overwrite samples buffer and then run Execute_Measurements().
    */
   det_props[0].vcs_position.y = 4.0F;
   cw_sensor.Set_Dets_Inside_Sensor_Zone_Edges(det_props[0].vcs_position.x, 0);
   cw_sensor.Execute_Measurements(det_props, raw_detect_list, sensor_props, calib, objects);

   /** \result
    * cw_sensor should should has:
    * Lateral position estimate: 2.0
    * Laterally closest measurement: equal to outlier lat posn (4.0)
    * Confidence 0.666666687
    * Valid estimate flag: TRUE
    */
   DOUBLES_EQUAL(2.0F, cw_sensor.Get_CW_Lat_Posn_Estimate(), floating_th);
   DOUBLES_EQUAL(det_props[0].vcs_position.y, cw_sensor.Get_Laterally_Closest_Measurement().lateral_position, floating_th);
   DOUBLES_EQUAL(0.666666687F, cw_sensor.Get_CW_Lat_Posn_Confidence(), floating_th);
   CHECK_TRUE(cw_sensor.Is_CW_Lat_Posn_Estimate_Valid());
}


/** @}*/