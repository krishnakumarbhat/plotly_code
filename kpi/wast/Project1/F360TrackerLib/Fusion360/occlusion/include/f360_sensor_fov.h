/*===================================================================================*\
* FILE:  f360_sensor_fov.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Sensor_FOV_T class.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef F360_SENSOR_FOV_H
#define F360_SENSOR_FOV_H

#include "f360_reuse.h"
#include "f360_occlusion_types.h"
#include "f360_radar_sensor.h"

namespace f360_variant_A
{
   class Sensor_FOV_T
   {
   public:
      Sensor_FOV_T();

      ~Sensor_FOV_T();

      bool Is_Point_In_FOV(const float32_t azimuth, const float32_t range) const; // Method used to determine whether given point is in sensor FOV

      void Update_Calibrations(
         const F360_Radar_Sensor_T& sensor);

      void Reset();

      float32_t Get_Azimuth_Spread() const;

      float32_t Get_Min_Az() const
      {
         return fov.min_az;
      } // Method used to get minimal azimuth of sensor FOV

      float32_t Get_Max_Range() const
      {
         return fov.max_range;
      } // Method used to get maximal range of sensor FOV

   private:
      Sensor_FOV_Data_T fov; // structure containing information about sensor FOV
   };
}
#endif
