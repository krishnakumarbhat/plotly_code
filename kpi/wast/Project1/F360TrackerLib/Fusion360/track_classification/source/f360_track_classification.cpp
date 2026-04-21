/*===================================================================================*\
* FILE: f360_object_classification.cpp
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This is the main function for the vehicle processing module.
*
* ABBREVIATIONS:
*   OTG	Over-The-ground
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

#include "f360_track_classification.h"
#include "f360_calculate_object_class_probabilities.h"
#include "f360_get_wall_time.h"
#include "f360_object_occlusion_classification.h"
#include "f360_object_motion_classification.h"
#include "f360_object_underdrivability_classification.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Track_Classification
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T & host
   * F360_Tracker_Info_T & tracker_info
   * const ocg::OCG_Outputs_T* occupancy_grid
   * const F360_Globals_T & globals
   * const rspp_variant_A::RSPP_Detection_List_T & dets_raw
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Calibrations_T & calib
   * const Occlusion_T& occlusion - reference to occlusion object
   * F360_TRKR_TIMING_INFO_T & timing_info
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
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
   * constructor
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Track_Classification(
      const F360_Host_T & host,
      const F360_Tracker_Info_T & tracker_info,
      const ocg::OCG_Outputs_T* const p_occupancy_grid,
      const F360_Globals_T & globals,
      const rspp_variant_A::RSPP_Detection_List_T & dets_raw,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T & calib,
      const Occlusion_T& occlusion,
      F360_TRKR_TIMING_INFO_T& timing_info,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      const float32_t start_time = get_wall_time();

      run_obj_class(object_tracks, host, tracker_info, calib);

      Object_Motion_Classification(object_tracks, tracker_info, host, globals, dets_raw, calib, sensors, occlusion);

      Object_Occlusion_Classification(occlusion, tracker_info, object_tracks);

      Object_Underdrivability_Classification(tracker_info, p_occupancy_grid, host, calib, object_tracks, timing_info);

      timing_info.track_classification = get_wall_time() - start_time;
   }
}
