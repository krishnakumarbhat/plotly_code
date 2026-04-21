/*===================================================================================*\
Disclaimer:
This file is intended as an example showing how to integrate the tracker, it is not to be used as is for production.
All values used/assigned below are example values from previous programs, they are not to be reused.
\*===================================================================================*/

/*===================================================================================*\
 * FILE:  tracker_OAL.h
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains declarations of output data from f360_tracker
 *
 * Applicable Standards (in order of precedence: highest first):
 * ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 * ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/
#ifndef TRACKER_OAL_H
#define TRACKER_OAL_H

#include "f360_reuse.h"
#include "f360_log_types.h"
#include "TrackerInfoLog.h"
#include "VehicleInfoLog.h"
#include "F360HostPropsLog.h"
#include "HostCalibsLog.h"
#include "TimingInfoLog.h"
#include "StaticEnvPolysLog.h"
#include "f360_functional_safety_faults_log.h"
#include "SyncInfoLog.h"
#include "f360_sensor_calib_log.h"
#include "f360_object_log.h"
#include "TrailerDetectorLog.h"

typedef struct Tracker_OAL_Tag
{
   F360_Object_Log_Output_T objects;
   F360_Detection_Log_Output_T detections;
   Tracker_Info_Log_T tracker_info;
   Vehicle_Info_Log_T vehicle_info;
   F360_Host_Props_Log_T f360_host_props_info;
   Host_Calibs_Log_T host_calib_log;
   Timing_Info_Log_T timing_log;
   F360_Static_Env_Poly_Log_T Log_Static_Env_Polys;
   Functional_Safety_Faults_Log_T fs_faults_log;
   Sync_Info_Log_T sync_info;
   F360_Sensor_Calib_Log_Output_T sensor_calibs;
   Trailer_Detector_Log_T trailer_detector_log;
} Tracker_OAL_T;

#endif // TRACKER_OAL_H
