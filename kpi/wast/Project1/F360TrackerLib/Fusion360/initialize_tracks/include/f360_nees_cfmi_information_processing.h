/*===================================================================================*\
* FILE: f360_nees_cfmi_information_processing.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains functions declarations:
*     - Get_NEES_CFMI_Information
*     - Calculate_Detections_Mean_Center
*     - Calculate_Moving_Detections_Ratio
*     - Calculate_Detections_And_Velocities_Inliers_Ratio
*     - Calculate_Mean_Range_Rate_Comp_Variance
*     - Calculate_Mean_Velocity_Covariance
*     - Calculate_Moving_Detections_Ratio
*     - Calculate_Detections_And_Velocities_Inliers_Ratio

* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#ifndef F360_NEES_CFMI_INFORMATION_PROCESSING_H
#define F360_NEES_CFMI_INFORMATION_PROCESSING_H

#include "f360_detection_props.h"
#include "f360_detection_hist.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_cluster.h"
#include "f360_calibrations.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_host_props.h"

namespace f360_variant_A
{
   void Get_NEES_CFMI_Information(const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& detection_hist,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Calculate_Detections_Mean_Center(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_NEES_CFMI_Information_T& nees_cfmi_information);


   void Calculate_Mean_Range_Rate_Comp_Variance(F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Calculate_Mean_Velocity_Covariance(F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Calculate_Moving_Detections_Ratio(F360_NEES_CFMI_Information_T& nees_cfmi_information);

   void Calculate_Detections_And_Velocities_Inliers_Ratio(const F360_Calibrations_T& calibrations, F360_NEES_CFMI_Information_T& nees_cfmi_information);

}
#endif
