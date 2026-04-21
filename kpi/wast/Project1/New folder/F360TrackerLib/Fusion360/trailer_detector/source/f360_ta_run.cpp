/*===========================================================================*\
 * FILE: ta_run.cpp
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer angle run function and Get_Output function
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_trailer_detector_TA.h"
#include "f360_trailer_detector_constants.h"

namespace f360_variant_A
{
   const TA_Cals Trailer_Detector_TA::ta_calibs{};
   /*=========================================================================
    * Method         Trailer_Detector_TA::Run_Trailer_Detector
    *
    * Description    Public method to run trailer angle estimation
    *
    * Parameters     const F360_Host_T &vehicle_data
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
   void Trailer_Detector_TA::Run_Trailer_Detector(const F360_Host_T &vehicle_data,
      const float32_t elapsed_time_s)
   {
      Estimate(vehicle_data, elapsed_time_s);
   }

   /*=========================================================================
    * Method         Trailer_Detector_TA::Get_Output
    *
    * Description    Public method to get trailer angle estimation output
    *
    * Parameters     TA_Output& output
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
   void Trailer_Detector_TA::Get_Output(TA_Output& output) const
   {
      output.trailer_angle_rad = trailer_angle_rad;
      output.trailer_angle_rate_rad = trailer_angle_rate_rad;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TA::Estimate
    *
    * Description    Private method used to estimate trailer angle
    *
    * Parameters     const F360_Host_T &vehicle_data
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
   void Trailer_Detector_TA::Estimate(const F360_Host_T &vehicle_data, const float32_t elapsed_time_s)
   {
      float32_t m_trailer_axle_length = trailer_axle_length;
      const float32_t alpha = vehicle_data.vcs_sideslip;

      // set default m_trailer_axle_length
      if (m_trailer_axle_length <= 1.0F)
      {
         m_trailer_axle_length = ta_calibs.b2_default;
      }

      // estimation
      if ((vehicle_data.speed >= ta_calibs.speed_threshold) &&
         (std::abs(vehicle_data.yaw_rate_rad) <= 0.02F))
      {
         HV_cnt = std::min(static_cast<int16_t>(HV_cnt + 1), ta_calibs.HV_cnt_threshold);
      }
      else
      {
         HV_cnt = 0;
      }

      if (HV_cnt == ta_calibs.HV_cnt_threshold)
      {
         HV_start = true;
      }

      // reset HV_angle prev_trailer_angle
      if (vehicle_data.speed < 0.0F)
      {
         Clear();
      }

      if(HV_start)
      {
         const float32_t dist_rear_axle_to_rear_bumper = ta_calibs.true_HV_length - vehicle_data.dist_rear_axle_to_vcs_m;
         const float32_t ratio = dist_rear_axle_to_rear_bumper / vehicle_data.dist_rear_axle_to_vcs_m;
         const float32_t trailer_w = 1.0F / m_trailer_axle_length*std::abs(vehicle_data.speed)*(F360_Cosf(alpha)*F360_Sinf(HV_angle) - ratio*F360_Sinf(alpha)*F360_Cosf(HV_angle));
         HV_angle += vehicle_data.yaw_rate_rad*elapsed_time_s - trailer_w*elapsed_time_s;
      }

      trailer_angle_rad = HV_angle;

      trailer_angle_rate_rad = (trailer_angle_rad - prev_trailer_angle) / elapsed_time_s; // [rad/s]
      prev_trailer_angle = trailer_angle_rad; // [radian]

   }

   /*=========================================================================
    * Method         Trailer_Detector_TA::Initialize
    *
    * Description    Public method used to initialize private member variables
    *                of TrailerDetectorTA
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
   void Trailer_Detector_TA::Initialize()
   {
      HV_cnt = 0;
      HV_start = false;
      HV_angle = 0.0F;
      prev_trailer_angle = 0.0F;
      trailer_angle_rad = 0.0F;
      trailer_angle_rate_rad = 0.0F;
      trailer_axle_length = 0.0F;
   }

   /*=========================================================================
    * Method         Trailer_Detector_TA::Clear
    *
    * Description    Public method used to clear private member variables
    *                of TrailerDetectorTA
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
   void Trailer_Detector_TA::Clear()
   {
      HV_cnt = 0;
      HV_start = false;
      HV_angle = 0.0F;
      prev_trailer_angle = 0.0F;
      trailer_angle_rad = 0.0F;
      trailer_angle_rate_rad = 0.0F;
   }
}
