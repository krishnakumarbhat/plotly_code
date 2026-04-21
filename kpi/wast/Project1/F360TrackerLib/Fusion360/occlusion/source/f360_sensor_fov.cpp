/*===================================================================================*\
* FILE:  f360_sensor_fov.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of Sensor_FOV_T class member functions
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


#include "f360_sensor_fov.h"

namespace f360_variant_A
{
   /*=========================================================================
   * Method         Sensor_FOV_T::Sensor_FOV_T
   *
   * Description    Constructor of Sensor_FOV_T.
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Sensor_FOV_T::Sensor_FOV_T() :
      fov()
   {

   }

   /*=========================================================================
   * Method         Sensor_FOV_T::~Sensor_FOV_T
   *
   * Description    Destructor of Sensor_FOV_T.
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Sensor_FOV_T::~Sensor_FOV_T()
   {

   }

   /*=========================================================================
   * Method         Sensor_FOV_T::Is_Point_In_FOV
   *
   * Description    Method used to verify whether point is in sensor FOV
   *
   * Parameters     
   * const float32_t azimuth - point azimuth
   * const float32_t range - point range
   *
   * Returns        bool - flag indicating whether point is in sensor FOV
   *
   * Externals:     None.
   *
   * Precondition   Given point coordinates must be in Sensor Coordinates System.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Sensor_FOV_T::Is_Point_In_FOV(
      const float32_t azimuth, 
      const float32_t range) const
   {
      const bool f_is_in_fov =
         (fov.min_az < azimuth) &&
         (azimuth < fov.max_az) &&
         (range < fov.max_range);

      return f_is_in_fov;
   }

   /*=========================================================================
   * Method         Sensor_FOV_T::Update_Calibrations
   *
   * Description    Method used to update field of view azimuth spread and maximal range.
   *
   * Parameters
   * const F360_Radar_Sensor_T& sensor - sensor FOV calibrations dependent on sensor look id
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Sensor_FOV_T::Update_Calibrations(
      const F360_Radar_Sensor_T& sensor)
   {
      fov.min_az = sensor.constant.fov_min_az_rad[sensor.variable.look_id];
      fov.max_az = sensor.constant.fov_max_az_rad[sensor.variable.look_id];
      fov.max_range = sensor.constant.range_limits[sensor.variable.look_id];
   }

   /*=========================================================================
   * Method         Sensor_FOV_T::Reset
   *
   * Description    Method used to reset sensor FOV calibratios.
   *
   * Parameters:    None.
   *
   * Returns:       None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Sensor_FOV_T::Reset()
   {
      fov.min_az = 0.0F;
      fov.max_az = 0.0F;
      fov.max_range = 0.0F;
   }

   /*=========================================================================
   * Method         Sensor_FOV_T::Get_Azimuth_Spread
   *
   * Description    Method used to acess azimuth spread of sensor.
   *
   * Parameters:    None.
   *
   * Returns:       float32_t azimuth_spread - spread on sensor azimuth FOV
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   float32_t Sensor_FOV_T::Get_Azimuth_Spread() const
   {
      const float32_t az_spread = fov.max_az - fov.min_az;
      return az_spread;
   }
}
