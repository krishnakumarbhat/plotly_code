/*===================================================================================*\
* FILE: f360_object_based_angle_jump_detector.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
*    This file contains implementations of functions related to object based angle 
*    jump detector.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_object_based_radar_phenomena_internals.h"
#include "f360_object_based_angle_jump_detector.h"
#include "f360_object_based_multibounce_detector.h"
#include "f360_math_func.h"
#include <limits>
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Can_Object_Be_A_Reference()
   *===========================================================================
   * RETURN VALUE:
   * bool - True if evaluated object can be a reference for the algorithms
   *
   * PARAMETERS:
   *  const F360_Object_Track_T &object_track,
   *  const float32_t max_object_lateral_distance,
   *  const float32_t max_pointing_disagreement
   *  const float32_t min_confidence_level
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
   * Check whether object can be a reference for the algorithms. Mostly object parallel
   * and close to the host are used
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Can_Object_Be_A_Reference(
      const F360_Object_Track_T &object_track,
      const float32_t max_object_lateral_distance,
      const float32_t max_pointing_disagreement,
      const float32_t min_confidence_level)
   {
      return ((object_track.status != F360_OBJECT_STATUS_INVALID) 
            && object_track.f_moving 
            && (std::abs(object_track.bbox.Get_Orientation().Value()) < max_pointing_disagreement)
            && (std::abs(object_track.bbox.Get_Center().y) < max_object_lateral_distance)
            && (min_confidence_level < object_track.confidenceLevel));
   }

   /*===========================================================================*\
   * FUNCTION: Check_Dets_Wrt_Reference()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T &reference_track
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Calibrations_T &calibs
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
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
   * Checks detections with respect to reference track. It is done by looking for
   * revelant sensors and calls the algorithms that verifies some of radar phenomena
   *
   * PRECONDITIONS:
   * reference_track passes Can_Object_Be_A_Reference() i.e. moving parallel to the host.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_Dets_Wrt_Reference(
      const F360_Object_Track_T &reference_track,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calibs,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      bool relevant_sensors[MAX_NUMBER_OF_SENSORS] = {};
      Determine_Side_Sensors(reference_track, sensors, calibs.rp_object_max_longitudinal_margin, relevant_sensors);

      if (cmn::end(relevant_sensors) != std::find_if(cmn::begin(relevant_sensors), cmn::end(relevant_sensors), [](const bool i) { return i; }))
      {
         Check_Dets_Against_Angle_Jumps(reference_track, raw_detection_list, relevant_sensors, sensors, calibs, detection_props);
         Check_Dets_Against_Multibounces(reference_track, raw_detection_list, relevant_sensors, sensors, calibs, detection_props);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Side_Sensors()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T &object_track
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const float32_t longitudinal_margin
   * bool(&relevant_sensors)[MAX_NUMBER_OF_SENSORS]
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
   * Determine which sensor are side sensors (also corner sensors)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Determine_Side_Sensors(
      const F360_Object_Track_T &object_track,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const float32_t longitudinal_margin,
      bool(&relevant_sensors)[MAX_NUMBER_OF_SENSORS])
   {
     for (uint32_t sen_idx = 0U; sen_idx < MAX_NUMBER_OF_SENSORS; sen_idx++)
      {
         relevant_sensors[sen_idx] = false; //appriori assume that sensor is invalid

         const  F360_Radar_Sensor_T &sensor = sensors[sen_idx];
         if (sensor.variable.is_valid &&
            ((F360_MOUNTING_LOCATION_LEFT_FORWARD == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_LEFT_SIDE1 == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_LEFT_SIDE2 == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_LEFT_REAR == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_RIGHT_FORWARD == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_RIGHT_SIDE1 == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_RIGHT_SIDE2 == sensor.constant.mounting_location) ||
            (F360_MOUNTING_LOCATION_RIGHT_REAR == sensor.constant.mounting_location)))
         {
            const float32_t front_bumper_pos_vcs = object_track.bbox.Get_Center().x + 0.5F * object_track.bbox.Get_Length();
            const float32_t rear_bumper_pos_vcs = object_track.bbox.Get_Center().x - 0.5F * object_track.bbox.Get_Length();

            const bool f_long_pos_is_ok = ((rear_bumper_pos_vcs - longitudinal_margin) < sensor.constant.mounting_position.vcs_position.longitudinal)
               && (sensor.constant.mounting_position.vcs_position.longitudinal < (front_bumper_pos_vcs + longitudinal_margin));

            if (f_long_pos_is_ok)
            {
               // Object and radar are on the same side
               relevant_sensors[sen_idx] = (F360_Sign(object_track.bbox.Get_Center().y) == F360_Sign(sensor.constant.mounting_position.vcs_position.lateral));
            }
         }
      }
   }
}
