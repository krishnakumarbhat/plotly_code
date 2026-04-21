/*=========================================================================
*  FILE: f360_concrete_wall_sensor.h
*=========================================================================
* Copyright (C) 2021 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------
*
*  DESCRIPTION:
*    This file contains Concrete_Wall_Sensor_T class declaration
*
*========================================================================*/
#ifndef F360_CONCRETE_WALL_SENSOR_H
#define F360_CONCRETE_WALL_SENSOR_H

#include "f360_reuse.h"
#include "f360_mounting_location.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor_props.h"
#include "f360_radar_sensor.h"
#include "f360_concrete_wall_measurements.h"
#include "f360_circular_buffer.h"
#include "f360_calibrations.h"
#include "f360_object_track.h"

namespace f360_variant_A
{
   class Concrete_Wall_Sensor_T
   {
   public:
      Concrete_Wall_Sensor_T();
      ~Concrete_Wall_Sensor_T();
      void Init(const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         const uint32_t sens_idx,
         const float32_t sensor_zone_half_width,
         const float32_t max_lateral_range);

      void Reset(const float32_t max_lateral_range);
      void Execute_Measurements(
         const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list, const F360_Radar_Sensor_Props_T& sensor_props,
         const F360_Calibrations_T &calib,
         const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]);

      void Set_Dets_Inside_Sensor_Zone_Edges(const float32_t det_xpos, const int32_t det_idx);
      float32_t Get_Sensor_Side_Sign() const { return side_sign; };
      float32_t Get_Sensor_Long_Posn() const { return sensor_x_posn; };
      float32_t Get_CW_Lat_Posn_Estimate() const { return lateral_posn_estimate; };
      float32_t Get_CW_Lat_Posn_Confidence() const { return lateral_posn_confid; };
      bool Is_CW_Lat_Posn_Estimate_Valid() const { return f_estimation_valid; };

      const Concrete_Wall_Measurements_T& Get_CW_Measurements() const { return this->det_vcs_lat_pos_buffer; }
      void Set_CW_Measurements(const Concrete_Wall_Measurements_T& det_vcs_lat_pos_buffer_input) { this->det_vcs_lat_pos_buffer = det_vcs_lat_pos_buffer_input; }

   private:
      Concrete_Wall_Measurements_T det_vcs_lat_pos_buffer;  // Circular buffer for lat distances
      Concrete_Wall_Sample_T laterally_closest_measurement; // laterally closest position of relevant detection. When no detection - equal to init value or lateral position of nearest edge of tracker object if this object occlude sensor zone
      float32_t side_sign;                     // -1:left, 1:right
      float32_t sensor_x_posn;                 // sensor vcs longitudinal position.
      float32_t sensor_y_posn;                 // sensor vcs lateral position.
      float32_t sensor_zone_rear_limit;        // Rear limit of longitudinalwise zone with center in sensor VCS longitudinal position.
      float32_t sensor_zone_front_limit;       // Front limit of longitudinalwise zone with center in sensor VCS longitudinal position.
      float32_t lateral_posn_estimate;         // Estimate of concrete wall lateral position.
      float32_t lateral_posn_confid;           // Confidence of lateral position estimation. Calculated based on "age_based_confid" and "age_based_confid"
      float32_t age_based_confid;              // Confidence of lateral position estimation calculated based on samples age.
      float32_t spread_based_confid;           // Confidence of lateral position estimation calculated based on samples lateral position distribution.
      float32_t lateral_posn_spread;           // Span of lateral positions in det_vcs_lat_pos_buffer.
      float32_t aggregated_lateral_posn;       // Lateral position of obstacle estimated based on all valid samples in buffer.
      int32_t first_valid_det_idx;         // Index of first in zone detection.
      int32_t last_valid_det_idx;          // Index of last in zone detection.
      uint32_t sensor_id;                   // Sensor ID (given by the tracker wrapper).
      uint32_t num_dets_between_edges;      // Number of detections with their longitudinal position inside sensor zone.
      F360_Mounting_Location_T sensor_mounting_location;    // Sensor lateral/longitudinal mounting position in VCS.
      bool f_first_valid_det_set;       // Indicates that first detection in zone was found.
      bool f_last_valid_det_set;        // Indicates that last detection in zone was found.
      bool f_estimation_valid;          // Indicates that CW lateral distance estimate is valid.

      void Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor(
         const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
         const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]);

      void Update_Buffer_With_Sample();
      void Update_CWD_Sensor(const F360_Calibrations_T &calib);
      void Update_Aggregated_Lat_Posn(const float32_t max_lateral_spread);
      void Calc_Lateral_Position_Confidence(const float32_t spread_weight);
      bool Is_Aggregated_Lat_Posn_Valid() const;
      bool Is_Detection_Valid(
         const F360_Detection_Props_T &det,
         const rspp_variant_A::RSPP_Detection_T &raw_det,
         const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]) const;
      bool Is_Lateral_Posn_Spread_Valid(const float32_t max_lateral_spread) const;
      void Check_Lat_Pos_Buffer_For_Outlier(const float32_t max_lateral_spread, const uint32_t min_smpls_num_to_outlier_finding);

   public:  // Below functions are for data logging purpose
      float32_t Get_Sensor_Lat_Posn() const { return sensor_y_posn; }
      void Set_Sensor_Lat_Posn(const float32_t sensor_y_posn_input) { this->sensor_y_posn = sensor_y_posn_input; }
      void Set_Sensor_Long_Posn(const float32_t sensor_x_posn_input) { this->sensor_x_posn = sensor_x_posn_input; }
      void Set_Sensor_Side_Sign(const float32_t side_sign_input) { this->side_sign = side_sign_input; }
      float32_t Get_Sensor_Zone_Rear_Limit() const { return sensor_zone_rear_limit; }
      void Set_Sensor_Zone_Rear_Limit(const float32_t sensor_zone_rear_limit_input) { this->sensor_zone_rear_limit = sensor_zone_rear_limit_input; }
      float32_t Get_Sensor_Zone_Front_Limit() const { return sensor_zone_front_limit; }
      void Set_Sensor_Zone_Front_Limit(const float32_t sensor_zone_front_limit_input) { this->sensor_zone_front_limit = sensor_zone_front_limit_input; }
      uint32_t Get_Sensor_Id() const { return sensor_id; }
      void Set_Sensor_Id(const uint32_t sensor_id_input) { this->sensor_id = sensor_id_input; }
      const Concrete_Wall_Sample_T& Get_Laterally_Closest_Measurement() const { return laterally_closest_measurement; }
      void Set_Laterally_Closest_Measurement(const float32_t lateral_position, const bool is_valid)
      {
         this->laterally_closest_measurement.lateral_position = lateral_position;
         this->laterally_closest_measurement.is_valid = is_valid;
      }

      F360_Mounting_Location_T Get_Sensor_Mounting_Location() const { return sensor_mounting_location; }
      void Set_Sensor_Mounting_Location(const int32_t sensor_mounting_location_input) { this->sensor_mounting_location = static_cast<F360_Mounting_Location_T>(sensor_mounting_location_input); }

   };
}
#endif
