/*===================================================================================*\
* FILE: f360_initialize_tracked_objects.h
*====================================================================================
* Copyright 2020 Aptiv Technologies, Inc., All Rights Reserved.* Aptiv Confidential
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
*   This file contains F360_Tracker declaration of Initialize_Tracks() function
*   for tracked objects state initialization.
*   
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/

#ifndef F360_INITIALIZE_TRACKS_H
#define F360_INITIALIZE_TRACKS_H

#include "f360_calibrations.h"
#include "f360_detection_props.h"
#include "f360_detection_hist.h"
#include "f360_globals.h"
#include "f360_host.h"
#include "f360_host_props.h"
#include "f360_cluster.h"
#include "f360_object_track.h"
#include "f360_timing_info.h"
#include "f360_tracker_info.h"
#include "f360_static_env_poly_types.h"
#include "f360_occlusion.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void Initialize_Tracks(
      const F360_Host_T& host,
      const F360_Host_Props_T& host_props,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      const F360_Detection_Hist_T& det_hist,
      const F360_Globals_T& globals,
      const F360_Calibrations_T& calibrations,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const Occlusion_T& occlusion,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_TRKR_TIMING_INFO_T& tracker_time_info);

      void Prepare_Stationary_Info(const F360_Host_Props_T& host_props,
      F360_NEES_CFMI_Pos_Diff_Velocity_T &stationary_velocity);
}
#endif
