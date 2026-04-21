/*===================================================================================*\
* FILE:  f360_detect_multipath.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definitions of functions declared in f360_detect_multipath.h
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_constants.h"
#include "f360_host.h"
#include "f360_tracker_info.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
#include "f360_timing_info.h"
#include "f360_multipath_detector.h"
#include "f360_visibility_info.h"
#include "f360_detect_multipath.h"
#include "f360_range_rates.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Check_If_Object_Is_MP_For_Any_Sensor()
   * ===========================================================================
   * RETURN VALUE:
   * bool f_marked_as_multipath - True if object is multipath
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS]
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Object_Track_T &object
   * const F360_Tracker_Info_T & tracker_info
   * Multipath_Detector &mp_detector
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
   * Check for all sensors if object that is approaching host and is in field of view
   * of a sensor is multipath, returns flag indicating if object is multipath.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Check_If_Object_Is_MP_For_Any_Sensor(
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS], 
      const F360_Object_Track_T &object,
      const F360_Tracker_Info_T& tracker_info,
      Multipath_Detector &mp_detector)
   {
      const Point object_position = object.bbox.Get_Center();
      bool f_marked_as_multipath = false;

      for (uint32_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
      {
         if (sensors[sensor_idx].variable.is_valid && Is_VCS_Point_Within_Current_FOV_Of_Sensor(object_position, sensors[sensor_idx], sensor_props[sensor_idx], sensors[sensor_idx].variable.look_id))
         {
            const F360_Sensor_Mounting_Position_T & sensor_position = sensors[sensor_idx].constant.mounting_position;
            const Point sensor_position_2d = { sensor_position.vcs_position.longitudinal, sensor_position.vcs_position.lateral };
            const float32_t object_range_rate = Calculate_Projected_Range_Rate(sensor_position_2d, object_position, object.vcs_velocity);

            // Negative object_range_rate implies that object gets closer to the host
            if ((object_range_rate < 0.0F) && mp_detector.Is_Multipath(sensor_position_2d, object_position, object_range_rate, tracker_info))
            {
               f_marked_as_multipath = true;
               break;
            }
         }
      }
      return f_marked_as_multipath;
   }


   /*===========================================================================*\
   * FUNCTION: Detect_And_Mark_Multipath_Objects()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Tracker_Info_T & tracker_info
   * const F360_Calibrations_T & calibrations
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS]
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS]
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
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
   * Checks if any of active objects is multipath candidate, then checks if candidate  
   * is confirmed as multipath by any sensor. If so - marks object as multipath and  
   * sets default mirror probability for that object.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Detect_And_Mark_Multipath_Objects(const F360_Host_T & host,
      const F360_Tracker_Info_T & tracker_info,
      const F360_Calibrations_T & calibrations,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      if (std::abs(host.speed) < calibrations.k_mp_max_allowed_host_speed_to_use_MP)
      {
         Multipath_Detector mp_detector(static_env_polys, object_tracks, tracker_info, calibrations);
         for (int32_t iobj = 0; iobj < tracker_info.num_active_objs; iobj++)
         {
            const int32_t trk_idx = tracker_info.active_obj_ids[iobj] - 1;
            F360_Object_Track_T& object = object_tracks[trk_idx];
            const bool f_is_multipath_candidate = object.f_moving && 
               ((F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type) || ( std::abs(object.speed) > calibrations.fast_moving_thresh));

            if (f_is_multipath_candidate)
            {
               const bool f_marked_as_multipath = Check_If_Object_Is_MP_For_Any_Sensor(sensor_props, sensors, object, tracker_info, mp_detector);
               if (f_marked_as_multipath)
               {
                  object.mirror_prob = calibrations.k_mp_default_mirror_probability;
               }
            }
         }
      }
   }
}


