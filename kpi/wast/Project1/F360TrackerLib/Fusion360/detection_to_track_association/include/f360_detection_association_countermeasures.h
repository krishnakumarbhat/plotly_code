/*===================================================================================*\
* FILE: f360_detection_association_countermeasures.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declarations of Detection_Association_Countermeasures() and related
* support functions.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#ifndef F360_DETECTION_ASSOCIATION_COUNTERMEASURES_H
#define F360_DETECTION_ASSOCIATION_COUNTERMEASURES_H

#include "f360_tracker_info.h"
#include "rspp_detection_list.h"
#include "f360_calibrations.h"
#include "f360_host.h"
#include "f360_radar_sensor.h"
#include "f360_object_track.h"
#include "f360_detection_props.h"

namespace f360_variant_A
{
   void Detection_Association_Countermeasures(
      const F360_Tracker_Info_T & tracker_info,
      const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list,
      const F360_Calibrations_T & calibrations,
      const F360_Host_T & host,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   );
}
#endif
