/*===========================================================================*\
* FILE: f360_mark_for_liberal_tracking.h
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains functions declarations for marking objects/detections for liberal tracking.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#ifndef F360_MARK_FOR_LIBERAL_TRACKING_H
#define F360_MARK_FOR_LIBERAL_TRACKING_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_calibrations.h"
#include "rspp_detection_list.h"
#include "f360_detection_props.h"
#include "f360_object_track.h"
#include "f360_host.h"
#include "f360_cluster.h"
#include "f360_detection_hist.h"
#include "f360_tracker_info.h"

namespace f360_variant_A
{
   void Mark_Objects_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      const F360_Tracker_Info_T& tracker_info,
      F360_Object_Track_T (&objects)[NUMBER_OF_OBJECT_TRACKS]
   );

   void Mark_Detection_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const float32_t host_speed,
      const rspp_variant_A::RSPP_Detection_T& detection,
      F360_Detection_Props_T &detection_prop
   );

   void Mark_Detections_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const float32_t host_speed,
      const uint32_t number_of_valid_detections,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&detections_props)[MAX_NUMBER_OF_DETECTIONS]
   );

   bool Check_Cluster_For_Liberal_Tracking(
      const F360_Calibrations_T &calibrations,
      const F360_Host_T &host,
      const F360_Detection_Props_T (&detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Cluster_T &cluster
   );

   bool Check_If_Point_Inside_Trapezoid_Zone(
      const float32_t trapezoid_top_lon_pos,
      const float32_t trapezoid_top_len,
      const float32_t trapezoid_bot_lon_pos,
      const float32_t trapezoid_bot_len,
      const float32_t point_lon_pos,
      const float32_t point_lat_pos
   );
}

#endif
