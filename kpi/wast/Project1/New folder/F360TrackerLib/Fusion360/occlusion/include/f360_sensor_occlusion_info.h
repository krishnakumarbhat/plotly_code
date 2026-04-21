/*===================================================================================*\
* FILE:  f360_sensor_occlusion_info.h
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Sensor_Occlusion_Info_T class.
* Class holds information about sensor field of view and occluded segments.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#ifndef SENSOR_OCCLUSION_INFO_H
#define SENSOR_OCCLUSION_INFO_H

#include "f360_reuse.h"
#include "f360_occlusion_types.h"
#include "f360_radar_sensor.h"
#include "f360_occlusion_sector.h"
#include "f360_sensor_fov.h"


namespace f360_variant_A
{

   class Sensor_Occlusion_Info_T
   {
   public:
      Sensor_Occlusion_Info_T();
      ~Sensor_Occlusion_Info_T();

      void Reset(); // method used to reset sensor information

      void Update_Calibs(
         const F360_Radar_Sensor_T& sensor,
         const float32_t range_uncertainty_th);   // method used to update calibrations
                                                // basing on given sensor properties

      void Update_Occlusion(
         const VCS_Track_Visible_Edges_T& vcs_visible_edges); // method updates occlusion information with visible edges of object

      Occlusion_Status_T Determine_Occlusion_Status(
         const float32_t vcs_long_posn,
         const float32_t vcs_lat_posn) const; // method used to determine given point occlusion status

      inline const Occlusion_Sector_Array& Get_Occlusion_Sectors() const // method used to access occluded sectors
      {
         return sectors;
      }

      inline float32_t Get_Sectors_Width() const // method used to access width of sectors
      {
         return sectors_width;
      }

      inline Sensor_FOV_T Get_Props() const // method used to access sensor properties - FOV
      {
         return fov;
      }

      inline Sensor_Mounting_Position Get_Mounting_Position() const // method used to access sensor mounting position
      {
         return mounting_position;
      }

   private:

      float32_t sectors_width;
      float32_t range_uncertainty;

      Occlusion_Sector_T sectors[occlusion_num_sectors];

      bool f_valid; // flag holding information whether sensor is valid or not
      Sensor_Mounting_Position mounting_position; // sensor mounting position

      Sensor_FOV_T fov; // sensor field of view

      Occlusion_Status_T Determine_Occlusion_Status_Of_Point_In_FOV(
         const SCS_Polar_Position_T scs_polar_position) const;

      void Update_Occlusion(
         const SCS_Edge_T& edge);   // method used to update occluded sectors with given edge.

      bool Is_In_FOV(
         const SCS_Edge_T& edge) const;  // method used to check whether edge is in sensor field of view

      void Fill_Sectors_Ranges(const float32_t range);

   };

   using Sensor_Occlusion_Info_Array = Sensor_Occlusion_Info_T[MAX_NUMBER_OF_SENSORS];
}

#endif
