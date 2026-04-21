/*===================================================================================*\
* FILE: f360_nees_cfmi_info_cloud.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declarations:
*     - Get_NEES_CFMI_Information_Cloud_Current_Dets()
*     - Get_NEES_CFMI_Information_Cloud_Old_Dets()
*     - Update_NEES_Detection_Slot()
*     - Calc_NEES_CFMI_Determinants_Cloud()
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
\*==========================================================================================*/

#ifndef F360_NEES_CFMI_INFO_CLOUD_H
#define F360_NEES_CFMI_INFO_CLOUD_H

#include "f360_reuse.h"
#include "f360_calibrations.h"
#include "f360_cluster.h"
#include "f360_radar_sensor.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_detection_hist.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void Get_NEES_CFMI_Information_Cloud_Current_Dets(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Get_NEES_CFMI_Information_Cloud_Old_Dets(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Hist_T& detection_hist,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Update_NEES_Detection_Slot(
      const F360_Calibrations_T& calibrations,
      const float32_t det_time_since_measurement,
      const Point detection_position,
      const float32_t(&detection_position_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const uint32_t source_array_index,
      const uint32_t nees_array_index,
      const float32_t det_rrate_comp,
      const float32_t det_azimuth,
      const F360_Detection_Wheelspin_Type_T det_wheelspin_type,
      const int8_t det_motion_status,
      const bool det_is_historical,
      F360_NEES_CFMI_Detection_T& nees_detection_slot,
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Calc_NEES_CFMI_Determinants_Cloud(F360_NEES_CFMI_Detection_T& detection);

}
#endif
