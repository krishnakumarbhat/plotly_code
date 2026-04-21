/*===================================================================================*\
* FILE: f360_nees_cfmi_try_to_init.h
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
*   Functions for NEES cost function minimization initialization - try to initialize object
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
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_NEES_CFMI_TRY_TO_INIT_H
#define F360_NEES_CFMI_TRY_TO_INIT_H

#include "f360_reuse.h"
#include "f360_velocity.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_detection_hist.h"
#include "f360_cluster.h"
#include "f360_calibrations.h"
#include "f360_tracked_object_init_info.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_host.h"
#include "f360_host_props.h"
#include "f360_radar_sensor.h"

namespace f360_variant_A
{
   void Try_To_Init_Obj_Track_By_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      const F360_Host_T& host,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_Tracked_Object_Init_Info_T& init_info);

   void Liberal_Initialization(const F360_Calibrations_T &calibrations,
      const F360_NEES_CFMI_Information_T &nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T &nees_cfmi_init_info);

   bool Best_NEES_CFMI_VH_Plausbility_Checks(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      const F360_NEES_CFMI_Velocity_T& vel_hypothesis);

   void Init_NEES_CFMI_Init_Info(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);

   void Update_Track_Init_Info_By_NEES_CFMI_Init_Info(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      const F360_Cluster_T& cluster,
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Host_T& host,
      F360_Tracked_Object_Init_Info_T& init_info);

   void Prepare_Info_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Hist_T& detection_hist,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Information_T& nees_cfmi_information
      );

   void Estimation_NEES_CFMI(const F360_Calibrations_T& calibrations,
      const F360_Host_Props_T& host_props,
      const F360_Host_T& host,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_NEES_CFMI_Pos_Diff_Velocity_T& stationary_velocity,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Tracked_Object_Init_Info_T& init_info);

   float32_t Get_Plausibility_Scale(const F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info);
}
#endif
