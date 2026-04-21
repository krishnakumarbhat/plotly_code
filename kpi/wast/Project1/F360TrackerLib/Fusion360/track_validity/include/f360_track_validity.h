/*===================================================================================*\
* FILE: f360_track_validity.h
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

#ifndef TRACK_VALIDITY_H
#define TRACK_VALIDITY_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_host.h"
#include "f360_tracker_info.h"
#include "f360_radar_sensor_props.h"
#include "f360_radar_sensor.h"
#include "rspp_detection_list.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
#include "f360_timing_info.h"
#include "f360_static_env_poly_types.h"

namespace f360_variant_A
{
   void Track_Validity(
      const F360_Host_T& host,
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibrations,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_TRKR_TIMING_INFO_T& timing_info);
}

#endif
