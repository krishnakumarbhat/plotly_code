/*===================================================================================*\
* FILE: f360_track_validity.cpp
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This is the main function for the vehicle processing module.
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
*
* DEVIATIONS FROM STANDARDS:
*
*
\*==========================================================================================*/


/******************************
* Includes
*******************************/

#include "f360_track_validity.h"
#include "f360_update_exist_prob.h"
#include "f360_update_object_confidence_levels.h"
#include "f360_get_wall_time.h"
#include "f360_determine_reflected_obj.h"
#include "f360_overall_confidence.h"
#include "f360_detect_multipath.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Track_Validity
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T & host,
   * const F360_Tracker_Info_T & tracker_info - reference to tracker_info struct
   * const F360_Calibrations_T & calibrations - reference to calibrations struct
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS] - reference to array of sensors
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list - reference to detection list
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS], reference to array of static environment polynomials
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS] - reference to array of object tracks
   * F360_TRKR_TIMING_INFO_T & timing_info - reference to timing_info struct
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
   * Main function of Track Validity module
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Track_Validity(
      const F360_Host_T& host,
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibrations,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      const float32_t start_time = get_wall_time();

      Detect_And_Mark_Multipath_Objects(host, tracker_info, calibrations, sensors, sensor_props, static_env_polys, object_tracks);

      //  Accuracy / Uncertainty  estimation func
      for (int32_t iobj = 0; iobj < tracker_info.num_active_objs; iobj++)
      {
         const int32_t trk_idx = tracker_info.active_obj_ids[iobj] - 1;

         if (F360_OBJECT_STATUS_INVALID != object_tracks[trk_idx].status)
         {
            Update_Object_Confidence_Levels(tracker_info, calibrations, object_tracks[trk_idx]);
         }
         Determine_Reflected_Obj(object_tracks, tracker_info, trk_idx, host, static_env_polys, calibrations);
      }

      Update_Existence_Probability(tracker_info, calibrations, object_tracks, timing_info);

      Overall_Confidence(object_tracks, raw_detect_list, tracker_info, calibrations);

      timing_info.track_validity = get_wall_time() - start_time;
   }
}
