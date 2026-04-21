/*===================================================================================*\
* FILE: f360_concrete_wall_sensor.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* Contains function definitions for f360_concrete_wall_sensor.h
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#include "f360_concrete_wall_sensor.h"
#include "f360_math_func.h"
#include <limits>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Concrete_Wall_Sensor_T()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None.
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Constructor of Concrete_Wall_Sensor_T class.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   f360_variant_A::Concrete_Wall_Sensor_T::Concrete_Wall_Sensor_T()
   {
      sensor_mounting_location = F360_MOUNTING_LOCATION_UNKNOWN;
      f_first_valid_det_set = false;
      f_last_valid_det_set = false;
      f_estimation_valid = false;

      sensor_id = 0U;
      num_dets_between_edges = 0U;

      first_valid_det_idx = -1;
      last_valid_det_idx = -1;

      side_sign = 0.0F;
      sensor_x_posn = 0.0F;
      sensor_y_posn = 0.0F;
      sensor_zone_rear_limit = 0.0F;
      sensor_zone_front_limit = 0.0F;

      laterally_closest_measurement.lateral_position = std::numeric_limits<float32_t>::quiet_NaN();
      laterally_closest_measurement.is_valid = false;

      lateral_posn_estimate = std::numeric_limits<float32_t>::quiet_NaN();
      age_based_confid = 0.0F;
      spread_based_confid = 0.0F;
      lateral_posn_confid = 0.0F;
      lateral_posn_spread = 0.0F;
      aggregated_lateral_posn = std::numeric_limits<float32_t>::quiet_NaN();
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::~Concrete_Wall_Sensor_T()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None.
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Destructor of Concrete_Wall_Detector_Sensor_T class.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   f360_variant_A::Concrete_Wall_Sensor_T::~Concrete_Wall_Sensor_T()
   {
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Init()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const uint32_t sens_idx,
   * const float32_t sensor_zone_half_width,
   * const float32_t max_lateral_range
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Initialize function for Concrete_Wall_Sensor.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Init(
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const uint32_t sens_idx,
      const float32_t sensor_zone_half_width,
      const float32_t max_lateral_range)
   {
      sensor_id = sensors[sens_idx].constant.id;
      sensor_mounting_location = sensors[sens_idx].constant.mounting_location;
      sensor_x_posn = sensors[sens_idx].constant.mounting_position.vcs_position.longitudinal;
      sensor_y_posn = sensors[sens_idx].constant.mounting_position.vcs_position.lateral;
      sensor_zone_rear_limit = sensor_x_posn - sensor_zone_half_width;
      sensor_zone_front_limit = sensor_x_posn + sensor_zone_half_width;
      side_sign = (sensor_y_posn < 0.0F) ? -1.0F : 1.0F;    // left : right
      laterally_closest_measurement.lateral_position = side_sign * max_lateral_range;
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Reset()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t & max_lateral_range
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Method that resets some data in Concrete_Wall_Sensor_T.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Reset(const float32_t max_lateral_range)
   {
      first_valid_det_idx = -1;
      last_valid_det_idx = -1;
      f_first_valid_det_set = false;
      f_last_valid_det_set = false;
      f_estimation_valid = false;
      num_dets_between_edges = 0U;
      laterally_closest_measurement.lateral_position = side_sign * max_lateral_range;
      laterally_closest_measurement.is_valid = false;
      lateral_posn_estimate = std::numeric_limits<float32_t>::quiet_NaN();
      lateral_posn_confid = 0.0F;
      age_based_confid = 0.0F;
      spread_based_confid = 0.0F;
      lateral_posn_spread = 0.0F;
      aggregated_lateral_posn = std::numeric_limits<float32_t>::quiet_NaN();

      det_vcs_lat_pos_buffer.Reset();
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Execute_Measurements()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const F360_Radar_Sensor_Props_T& sensor_props
   * const F360_Calibrations_T & calib
   * const F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Method for calculating data for sensor CWD.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Execute_Measurements(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Radar_Sensor_Props_T& sensor_props,
      const F360_Calibrations_T & calib,
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      // Checking if there is tracker object in sensor zone
      if (sensor_props.f_object_track_next_to_sensor &&
         ((sensor_props.next_to_sensor_object_track_min_long_pos <= sensor_zone_rear_limit) &&
         (sensor_zone_front_limit <= sensor_props.next_to_sensor_object_track_max_long_pos)))
      {
         // Setting closer to host object edge as reference position to limit number of detection that must be validate.
         laterally_closest_measurement.lateral_position = (side_sign < 0.0F) ? sensor_props.next_to_sensor_object_track_max_lat_pos : sensor_props.next_to_sensor_object_track_min_lat_pos;
      }

      Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor(det_props, raw_detect_list, object_tracks);
      Update_Buffer_With_Sample();
      Check_Lat_Pos_Buffer_For_Outlier(calib.k_cwd_max_lateral_spread, calib.k_cwd_min_num_samples_for_outlier_finding);
      Update_Aggregated_Lat_Posn(calib.k_cwd_max_lateral_spread);
      Update_CWD_Sensor(calib);

   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list
   * const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method finds laterally closest valid detection for each sensor zone.
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Find_Valid_Detection_VCS_Lat_Pos_Closest_To_Sensor(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      if (0U < num_dets_between_edges)
      {
         int32_t det_idx = first_valid_det_idx;

         for (uint32_t dets_cnt = 1U; dets_cnt <= num_dets_between_edges; dets_cnt++)
         {
            if (0.0F < (det_props[det_idx].vcs_position.y * side_sign))
            {
               const rspp_variant_A::RSPP_Detection_Tag &raw_det = raw_detect_list.detections[det_idx];

               const bool is_det_valid = Is_Detection_Valid(det_props[det_idx], raw_det, object_tracks);

               if (is_det_valid && (std::abs(det_props[det_idx].vcs_position.y) < std::abs(laterally_closest_measurement.lateral_position)))
               {
                  laterally_closest_measurement.lateral_position = det_props[det_idx].vcs_position.y;
                  laterally_closest_measurement.is_valid = true;
               }
            }
            det_idx = raw_detect_list.detections[det_idx].processed.next_sorted_idx;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Is_Detection_Valid()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Detection_Props_T & det
   * const rspp_variant_A::RSPP_Detection_T &raw_det
   * const F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method checks if detection is valid for CWD.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Concrete_Wall_Sensor_T::Is_Detection_Valid(const F360_Detection_Props_T & det, const rspp_variant_A::RSPP_Detection_T &raw_det, const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]) const
   {
      bool f_det_not_assoc_or_assoc_to_stat_obj;
      if (det.object_track_id > 0)
      {
         f_det_not_assoc_or_assoc_to_stat_obj = !(object_tracks[det.object_track_id - 1].f_moveable);
      }
      else
      {
         f_det_not_assoc_or_assoc_to_stat_obj = true;
      }

      return ((f_det_not_assoc_or_assoc_to_stat_obj)
         && (det.f_ok_to_use || raw_det.raw.f_bistatic || (F360_INVALID_UNSIGNED_ID != det.on_sep_id))
         && (raw_det.processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS)
         && (!det.f_double_bounce)
         && (!det.f_water_spray)
         && (!det.f_stationary_bounce)
         && (!det.f_object_based_angle_jump)
         && (!det.f_azimuth_rdot_outlier));
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Is_Lateral_Posn_Spread_Valid()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const float32_t max_lateral_spread
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method checks if lateral_posn_spread is valid.
   * It means that lateral_posn_spread is below some threshold (max_lateral_spread).
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Concrete_Wall_Sensor_T::Is_Lateral_Posn_Spread_Valid(const float32_t max_lateral_spread) const
   {
      return (!std::isnan(lateral_posn_spread)) && (lateral_posn_spread < max_lateral_spread);
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Check_Lat_Pos_Buffer_For_Outlier()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const float32_t max_lateral_spread
   * const uint32_t min_smpls_num_to_outlier_finding
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method checks if outliered sample is present in samples buffer.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Check_Lat_Pos_Buffer_For_Outlier(const float32_t max_lateral_spread, const uint32_t min_smpls_num_to_outlier_finding)
   {
      lateral_posn_spread = det_vcs_lat_pos_buffer.Calc_Lateral_Pos_Spread();

      if ((!Is_Lateral_Posn_Spread_Valid(max_lateral_spread)) && (min_smpls_num_to_outlier_finding <= det_vcs_lat_pos_buffer.Get_Number_Of_Valid_Samples()))
      {
         det_vcs_lat_pos_buffer.Find_Farthest_Outlier_Sample();
      }
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Update_Buffer_with_Sample()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * None
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method pushes sample regarding to valid detection lateral position to buffer.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Update_Buffer_With_Sample()
   {
      det_vcs_lat_pos_buffer.Push(laterally_closest_measurement);
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Update_CWD_Sensor()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method performs and updates calculations for sensors CWD.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Update_CWD_Sensor(const F360_Calibrations_T &calib)
   {
      if (Is_Aggregated_Lat_Posn_Valid())
      {
         if (det_vcs_lat_pos_buffer.Get_Last_Sample().is_valid)
         {
            // Check if new sample lateral position is not greater than aggregated_lateral_posn + k_cwd_max_diff_from_aggregated_lat_pos
            const bool f_diff_below_limit = std::abs(aggregated_lateral_posn - det_vcs_lat_pos_buffer.Get_Last_Sample().lateral_position) < std::abs(calib.k_cwd_max_diff_from_aggregated_lat_pos) ? true : false;
            if (f_diff_below_limit)
            {
               lateral_posn_estimate = det_vcs_lat_pos_buffer.Get_Last_Sample().lateral_position;
               Calc_Lateral_Position_Confidence(calib.k_cwd_spread_weight);
               f_estimation_valid = true;
            }
            else
            {
               lateral_posn_estimate = aggregated_lateral_posn;
               Calc_Lateral_Position_Confidence(calib.k_cwd_spread_weight);
               f_estimation_valid = true;
            }
         }
         else
         {
            lateral_posn_estimate = aggregated_lateral_posn;
            Calc_Lateral_Position_Confidence(calib.k_cwd_spread_weight);
            f_estimation_valid = true;
         }
      }
      else
      {
         lateral_posn_estimate = std::numeric_limits<float32_t>::quiet_NaN();
         f_estimation_valid = false;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Update_Aggregated_Lat_Posn()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * constfloat32_t max_lateral_spread
   * const uint32_t min_smpls_num_to_outlier_finding
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method calculates and updates aggregated lateral position for CWD sensor.
   * It also detects outliers detections in sensor buffer.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Update_Aggregated_Lat_Posn(const float32_t max_lateral_spread)
   {
      lateral_posn_spread = det_vcs_lat_pos_buffer.Calc_Lateral_Pos_Spread();

      if (Is_Lateral_Posn_Spread_Valid(max_lateral_spread))
      {
         aggregated_lateral_posn = det_vcs_lat_pos_buffer.Get_Mean_Lateral_Position();
      }
      else
      {
         aggregated_lateral_posn = std::numeric_limits<float32_t>::quiet_NaN();
      }
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Is_Aggregated_Lat_Posn_Valid()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * None
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method checks if aggregated lateral position is valid.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Concrete_Wall_Sensor_T::Is_Aggregated_Lat_Posn_Valid() const
   {
      return !std::isnan(aggregated_lateral_posn);
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Calc_Lateral_Position_Confidence()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calib
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method claculates lateral position confidence for CWD sensor.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Calc_Lateral_Position_Confidence(const float32_t spread_weight)
   {
      age_based_confid = det_vcs_lat_pos_buffer.Calc_Age_Based_Confidence();
      spread_based_confid = det_vcs_lat_pos_buffer.Calc_Spread_Based_Confidence(spread_weight);
      lateral_posn_confid = age_based_confid * spread_based_confid;
   }

   /*===========================================================================*\
   * FUNCTION: Concrete_Wall_Sensor_T::Set_Dets_Inside_Sensor_Zone_Edges()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const float32_t det_xpos
   * const int32_t det_idx
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method checks if longitudinal position of detection is closest to sensor_zone_rear_limit or sensor_zone_front_limit
   * and sets suitable flags when those detections were find. When detection if found its idx is stored.
   * Also calculates number of detections in sensor zone.
   *
   * PRECONDITIONS:
   * If f_first_valid_det_set is true then each time this method is called then det_xpos should be higher
   * than its value from previous call. This function is used by higher abstraction method that uses
   * sorted detection list (longitudinally).
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Sensor_T::Set_Dets_Inside_Sensor_Zone_Edges(const float32_t det_xpos, const int32_t det_idx)
   {
      if ((sensor_zone_rear_limit < det_xpos) &&
         (det_xpos < sensor_zone_front_limit))
      {
         // First detection inside sensor edges, set as min and max simultaneously
         if (!f_first_valid_det_set)
         {
            f_first_valid_det_set = true;
            first_valid_det_idx = det_idx;
         }

         // Next detection inside edges updating only max. Min was updated by previous det.
         last_valid_det_idx = det_idx;
         num_dets_between_edges += 1U;
      }
      // det_xpos bigger than sensor edges and min was set previously
      // if f_first_valid_det_set is already true then det_xpos is higher than sensor_zone_rear_limit (sorted detections)
      else if ((f_first_valid_det_set) &&
         (sensor_zone_front_limit < det_xpos))
      {
         f_last_valid_det_set = true;
      }
      else
      {
         //Do nothing: MISRA rule
      }
   }
}
