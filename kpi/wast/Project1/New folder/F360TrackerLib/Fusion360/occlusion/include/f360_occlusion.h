/*===================================================================================*\
* FILE:  f360_occlusion.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Occlusion_T class.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


#ifndef OCCLUSION_DETECTOR_H
#define OCCLUSION_DETECTOR_H

#include "f360_reuse.h"
#include "f360_object_track.h"
#include "f360_timing_info.h"
#include "f360_occlusion_types.h"
#include "f360_tracker_info.h"
#include "f360_radar_sensor.h"
#include "f360_sensor_occlusion_info.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{

   class Occlusion_T
   {
   public:
      Occlusion_T(
         const bool f_enable_occlusion,
         const F360_Calibrations_T& calib,
         const F360_Tracker_Info_T& tracker_info,
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         F360_TRKR_TIMING_INFO_T &timing_info,
         F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]);

      ~Occlusion_T();
      
      Occlusion_Status_T Determine_Occlusion_Status(
         const float32_t vcs_long_posn,
         const float32_t vcs_lat_posn) const; // Method used to determine point occlusion.

      Occlusion_Status_T Determine_Occlusion_Status_Single_Sensor(
         const float32_t vcs_long_posn,
         const float32_t vcs_lat_posn,
         const int32_t sensor_idx) const; // Method used to determine point occlusion for single sensor.

   private:

      bool f_enabled; // Flag indicating whether occlusion algorithm is enabled

      Sensor_Occlusion_Info_T sensors_occlusion[MAX_NUMBER_OF_SENSORS]; // class members holding sensors occlusion information.

      void Update_Sensors_Calibrations(
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         const float32_t range_uncertainty_th); // Method used to update internal sensors calibrations.

      void Update_Sensors_Occlusion(
         const F360_Calibrations_T& calib,
         const F360_Tracker_Info_T& tracker_info,
         F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]); // Method used to update occlusion information with object tracks data.
      
      Occlusion_Status_T Determine_Point_Occlusion_Status(
         const float32_t vcs_long_posn,
         const float32_t vcs_lat_posn) const; // Detailed implemetation of method used to determine point occlusion.

      void Update_Sensors_Occlusion(
         const F360_Object_Track_T& object); // Method used to update sensors occlusion data with single object.

      Occlusion_Status_T Pick_Status_With_Higher_Visibility(
         const Occlusion_Status_T first,
         const Occlusion_Status_T second) const; // Method used to update occlusion status that has higher visibility status.
   };
}

#endif

