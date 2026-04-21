#ifndef PROCESS_DETECTION_LOG_T_H
#define PROCESS_DETECTION_LOG_T_H
/*===========================================================================*\
 * FILE: ProcessDetectionLog.h
 *===========================================================================
 * Copyright 2016 Delphi Technologies, Inc., All Rights Reserved.
 * Delphi Confidential
 *---------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains the F360 Tracker Processed Detection Information
 *
 * ABBREVIATIONS:
 *   TODO: List of abbreviations used, or reference(s) to external document(s)
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *   Requirements Document(s): PDD-10024333-012_(CADS4_VFP_Ethernet_Communication).doc
 *   (Design & Requirements)
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     SW REF 264.15D "Delphi C Coding Standards" [12-Mar-2006]
 *
 * CHANGE SYNERGY INFO:
 * %full_filespec:   ProcessDetectionLog.h %
 * %version:         1 %
 * %date_modified:   Jan 6, 2017 %
 *
 * DEVIATIONS FROM STANDARDS:
 *   None
 *
\*===========================================================================*/

#include "f360_reuse.h"
/*===========================================================================*\
* Other Header Files
\*===========================================================================*/
#include "SensorInfoLog.h"
#include "emblib_T360_Types.h"
#include <stdint.h>

/*===========================================================================*\
* Exported local (file scope) Constants
\*===========================================================================*/
#define DETECTION_LOG_STREAM_NUM     3
#define DETECTION_LOG_STREAM_VERSION 17
#define DETECTION_LOG_NUM_CHUNKS     4

// #ifndef __TASKING__
//    #pragma pack(push, save_pack_det, 4)
// #else
// #endif

/*===========================================================================*\
* Exported Type Declarations
\*===========================================================================*/
typedef struct Detection_Log_Tag
{
   float vcs_x[MAX_ALL_RADAR_DETS];             //!< [m] x-position of detection in VCS coordinate.
   float vcs_y[MAX_ALL_RADAR_DETS];             //!< [m] y-position of detection in VCS coordinate.
   float rngrate_dealiased[MAX_ALL_RADAR_DETS]; //!< [m/s] de-aliased range rate.
   float rngrate_comp[MAX_ALL_RADAR_DETS];      //!< [m/s] range rate compensated by sensor velocity along radial direction.
   float snr[MAX_ALL_RADAR_DETS];               //!< [-] signal to noise ratio
   uint32_t padding_32;

   uint16_t num_dets;                       //!< Total number of valid detections used in the current tracker iteration
   uint16_t raw_det_id[MAX_ALL_RADAR_DETS]; //!< ID of detection as reported by sensor.
   uint16_t objTrkID[MAX_ALL_RADAR_DETS];   //!< [0-250] ID of track to which this detection is associated to.
   uint16_t clusterID[MAX_ALL_RADAR_DETS];  //!< [0-250] ID of cluster owning this detection.
   uint16_t padding_16;

   uint8_t sensorID[MAX_ALL_RADAR_DETS]; //!< ID of sensor which reported this detection.
   uint8_t
      motion_status[MAX_ALL_RADAR_DETS];   //!< [(-1)-2], (enum F360_DET_MOTION) (-1)-invalid, 0-stationary, 1-moving, 2-ambigous.
   uint8_t wheel_spin[MAX_ALL_RADAR_DETS]; //!< [0-2] wheel-spin type, (enum F360_DET_WHEELSPIN_TYPE) (0)-not a WS, 0-detection
                                           //!< pair WS, 1-object type WS.
   uint8_t f_azimuth_error[MAX_ALL_RADAR_DETS];   //  OBSOLETE!! Remove when adding a new parameter
   uint8_t f_dealiased[MAX_ALL_RADAR_DETS];       //!< [bool] is the range rate of this detection de-aliased.
   uint8_t f_double_bounce[MAX_ALL_RADAR_DETS];   //!< INTERNAL USE
   uint8_t f_FOV_edge[MAX_ALL_RADAR_DETS];        //!< [bool] is this detection on the edge of sensor FOV.
   uint8_t f_selected_by_trk[MAX_ALL_RADAR_DETS]; //!< [bool] is this detection selected by the associated track.
   uint8_t f_close_target[MAX_ALL_RADAR_DETS];    //!< INTERNAL USE
   uint8_t f_inside_gate[MAX_ALL_RADAR_DETS];     //!< INTERNAL USE
   uint8_t f_ok_to_use[MAX_ALL_RADAR_DETS];       //!< INTERNAL USE
   uint8_t f_on_guardrail[MAX_ALL_RADAR_DETS];    //!< [bool] is this detection on guardrail.
   uint8_t f_bistatic[MAX_ALL_RADAR_DETS];        // 1< [bool] is this detection bistatic
} Detection_Log_T;

typedef struct Process_Detections_Log_Tag
{
   Sensor_Info_Log_T sensorInfoLog[NUM_TOTAL_RADAR_SENSORS];
   uint32_t padding_32;
   Detection_Log_T detsLog;
} Process_Detections_Log_T;

//  **********************************************************************************************************
//  ************************ WARNING!!!!!! *******************************************************************
//  **********************************************************************************************************
//  The following compile-time assertion fails if the size of the log stream type does not equal the expected
//  size.  If it fails, then the size must be corrected AND the Stream LogVersion must changed.
//  If the version in this Stream LogVersion is NOT changed, then DV tool will not be able to decode the stream!
//  **********************************************************************************************************

// LOGSIZE_ASSERT(sizeof(Process_Detections_Log_T) == 91924);
// LOGSIZE_ASSERT((sizeof(Process_Detections_Log_T) / DETECTION_LOG_NUM_CHUNKS) * DETECTION_LOG_NUM_CHUNKS ==
// sizeof(Process_Detections_Log_T));

// #ifndef __TASKING__
//    #pragma pack(pop, save_pack_det)
// #else
// #endif
#endif /* PROCESS_DETECTION_LOG_T_H */
