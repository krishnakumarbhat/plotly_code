/*===========================================================================*\
* FILE: f360_update_detection_property.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains funcions which updates detection properties.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/
#include "f360_math.h"
#include "f360_update_detection_property.h"
#include "f360_static_env_polys_support_functions.h"
#include "f360_math_func.h"
#include "f360_determine_dets_on_edge_of_fov.h"
#include "f360_detect_wheelspin_pairs.h"
#include "f360_mark_out_det_pairs.h"
#include "f360_calc_probabilities_of_detections.h"
#include "f360_get_wall_time.h"
#include "f360_uncertainty_propagation.h"
#include "f360_mark_host_water_spray_detections.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"
#include "f360_handle_host_vehicle_clutter.h"
#include "f360_identify_and_flag_internal_reflections.h" 
#include "f360_mark_for_liberal_tracking.h"
#include "f360_detect_stationary_bounce.h"
#include "f360_handle_bistatic.h"
#include "f360_filter_out_low_quality_detections_on_or_behind_guardrail.h"
#include "f360_select_prioritized_detections.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_Detection_Property()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *   const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]
   *   const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list
   *   const F360_Host_T &host
   *   const F360_Calibrations_T &calib
   *   const Static_Env_Poly_T (&sep)[F360_NUM_OF_STATIC_ENV_POLYS]
   *   const F360_Tracker_Info_T &tracker_info
   *   const Trailer_Detector_Flt_Fus_Output &trailer_detector_output,
   *   F360_Radar_Sensor_Props_T (&sensor_props)[MAX_NUMBER_OF_SENSORS]
   *   F360_Detection_Props_T (&det_p)[MAX_NUMBER_OF_DETECTIONS]
   *   F360_TRKR_TIMING_INFO_T &timing_info
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
   * This function updates detection properties.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
    void Update_Detection_Property(
        const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
        const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
        const F360_Host_T& host,
        const F360_Calibrations_T& calib,
        const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
        const F360_Tracker_Info_T& tracker_info,
        const Trailer_Detector_Flt_Fus_Output &trailer_detector_output,
        F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
        F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
        F360_TRKR_TIMING_INFO_T& timing_info)
    {
      switch (tracker_info.variant.type)
      {
         // Only relevant for Variant I
         case F360_VARIANT_TYPE_I:
         {
            Select_Prioritized_Detections(raw_detection_list, sensors, host.speed, detection_props);
            break;
         }
         default:
         {
            break;
         }
      }

      Basic_Detections_Processing(raw_detection_list, sensors, calib, sep, detection_props, sensor_props);

      Complex_Detections_Processing(calib, raw_detection_list, host, tracker_info, trailer_detector_output, detection_props, timing_info);
   }

   /*===========================================================================*\
   * FUNCTION: Complex_Detections_Processing()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *   const F360_Calibrations_T &calib,
   *   const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   *   const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
   *   const F360_Host_T &host,
   *   const F360_Tracker_Info_T &tracker_info,
   *   F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   *   F360_TRKR_TIMING_INFO_T &timing_info
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
   * 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Complex_Detections_Processing(
      const F360_Calibrations_T &calib,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Host_T &host,
      const F360_Tracker_Info_T &tracker_info,
      const Trailer_Detector_Flt_Fus_Output &trailer_detector_output,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_TRKR_TIMING_INFO_T &timing_info)
   {
      Detect_Det_On_Trailer(host, raw_detection_list, trailer_detector_output, detection_props);

      Detect_Wheel_Spin_Pairs(calib, raw_detection_list, detection_props, timing_info);

      Mark_Out_Det_Pairs(raw_detection_list.detections, static_cast<int32_t>(raw_detection_list.number_of_valid_detections), detection_props, timing_info);

      Mark_Host_Water_Spray_Detections(raw_detection_list, host, calib, detection_props);

      Detect_Stationary_Bounce_Detections(calib, host, tracker_info, raw_detection_list, detection_props);

      Filter_Out_Low_Quality_Detections_On_Or_Behind_Guardrail(raw_detection_list, calib, detection_props);

      Mark_Detections_For_Liberal_Tracking(calib, host.speed, raw_detection_list.number_of_valid_detections, raw_detection_list, detection_props);
   }


   /*===========================================================================*\
   * FUNCTION: Basic_Detections_Processing()
   *===========================================================================
   * RETURN VALUE:
   * float32_t
   *
   * PARAMETERS:
   *   const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   *   const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
   *   const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
   *   const F360_Calibrations_T &calib,
   *   const Static_Env_Poly_T (&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
   *   F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   *   F360_Radar_Sensor_Props_T (&sensor_props)[MAX_NUMBER_OF_SENSORS]
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
   * For each valid detection it runs a set of functions that determine various detection flags or update detection properties.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Basic_Detections_Processing(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calib,
      const Static_Env_Poly_T (&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Radar_Sensor_Props_T (&sensor_props)[MAX_NUMBER_OF_SENSORS])
   {
      const uint32_t number_of_valid_detections = raw_detection_list.number_of_valid_detections;

      const bool f_mark_internal_reflections_enabled = Update_Buffers_Age_And_Reset_Implausible_Slots(sensors, sensor_props);

      for (uint32_t det_idx = 0U; det_idx < number_of_valid_detections; det_idx++)
      {
         const rspp_variant_A::RSPP_Detection_T &current_detection = raw_detection_list.detections[det_idx];
         F360_Detection_Props_T &current_detection_prop = detection_props[det_idx];
         const int32_t current_sensor_id = current_detection.raw.sensor_id;
         const F360_Radar_Sensor_T &current_sensor = sensors[current_sensor_id - 1];
         F360_Radar_Sensor_Props_T &current_sensor_props = sensor_props[current_sensor_id - 1];

         Mark_Detection_On_SEP(sep, calib, current_detection_prop);

         Mark_Detection_Behind_SEP(current_sensor, sep, calib, current_detection_prop);

         Handle_Host_Vehicle_Clutter(current_detection, sensors, calib, current_detection_prop);

         Identify_And_Mark_Internal_Reflections(current_detection, current_sensor, f_mark_internal_reflections_enabled, current_sensor_props, current_detection_prop);

         Handle_Bistatic_Dets(current_detection, calib, current_sensor_props, current_detection_prop);

         Compute_Detection_Probability(current_detection, current_sensor, calib, current_detection_prop);

         Mark_Detection_On_Edge_Of_FOV(current_detection, sensors, sensor_props, current_detection_prop);
      }
   }
}
