/*===========================================================================*\
* FILE: f360_fill_init_obj_track_props.h
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declaration for Fill_Init_Obj_Track_Props.
*
* 
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_FILL_INIT_OBJ_TRACK_PROPS_H
#define F360_FILL_INIT_OBJ_TRACK_PROPS_H

#include "f360_calibrations.h"
#include "f360_detection_props.h"
#include "f360_object_track.h"
#include "f360_cluster.h"
#include "f360_globals.h"
#include "f360_host_props.h"
#include "f360_tracker_info.h"
#include "f360_tracked_object_init_info.h"
#include "f360_static_env_poly_types.h"
#include "f360_host.h"
#include "f360_radar_sensor.h"

namespace f360_variant_A
{
   void Fill_Init_Obj_Track_Props(
      const F360_Globals_T& globals,
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Tracked_Object_Init_Info_T& init_info,
      const F360_Cluster_T& cluster_to_init,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const uint32_t new_unique_id,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& obj_trk_to_init);

   void Calc_Init_Position_And_Size(
      const F360_Tracked_Object_Init_Info_T& init_info,
      const F360_Host_T& host,
      const F360_Calibrations_T& calibrations,
      const F360_Globals_T& globals,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Cluster_T &cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& obj_trk_to_init);

   void Init_Obj_Track_Covariances(
      const F360_Host_T& host,
      const F360_Calibrations_T& calibrations,
      const F360_Tracked_Object_Init_Info_T& init_info,
      const float32_t & obj_mov_stat_spd_thresh,
      F360_Object_Track_T& obj_trk_to_init);

   void Determine_Initial_Size(
      const F360_Globals_T & globals,
      const F360_Calibrations_T & calibrations,
      F360_Object_Track_T& obj_trk_to_init);
}

#endif
