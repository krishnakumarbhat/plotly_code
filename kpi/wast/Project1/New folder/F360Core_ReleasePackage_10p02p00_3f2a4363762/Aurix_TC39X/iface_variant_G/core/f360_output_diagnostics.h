/*=========================================================================
*  FILE: f360_output_diagnostics.h
*=========================================================================
* Copyright (C) 2020 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------
*
*  DESCRIPTION:
*    This file contains Output Diagnostics class declaration.
*
*========================================================================*/
#ifndef F360_OUTPUT_DIAGNOSTICS_VARIANT_G_H
#define F360_OUTPUT_DIAGNOSTICS_VARIANT_G_H

/******************************
* Includes
*******************************/
#include "../Types/f360_reuse.h"
#include "../Logging/f360_log_types.h"

/******************************
* Class definition
*******************************/
namespace f360_variant_G
{
   struct Output_Faults_T
   {
      bool f_track_positions_faulty;     // flag indicating that at least one track VCS position is incorrect
      bool f_track_velocities_faulty;    // flag indicating that at least one track OTG-velocity is incorrect
      bool f_track_accelerations_faulty; // flag indicating that at least one track tangential acceleration is incorrect
      uint8_t padding;                       // tasking compiler padding
   };

   class Output_Diagnostics
   {
   public:
      struct Output_Diagnostics_Calibrations_T
      {
         float32_t max_allowed_lateral_position;      // maximal allowed VCS lateral position of track, [m]
         float32_t max_allowed_longitudinal_position; // maximal allowed VCS longitudinal position of track, [m]
         float32_t max_allowed_speed;                 // maximal allowed over-the-groud speed of track, [m/s]
         float32_t min_allowed_speed;                 // minimal allowed over-the-groud speed of track, [m/s]
         float32_t max_allowed_acceleration;          // maximal allowed tangential acceleration of track, [m/s^2]
         float32_t min_allowed_acceleration;          // minimal allowed tangential acceleration of track, [m/s^2]
      };

      Output_Diagnostics();
      virtual ~Output_Diagnostics();

      Output_Diagnostics_Calibrations_T Get_Calib() const
      {
         return calib;
      }

      virtual Output_Faults_T Execute(const F360_Object_Log_Output_T& obj_log) const; // main function of Output_Diagnostics to be called by SafetyLogic
   private:
      const Output_Diagnostics_Calibrations_T calib;
      bool Is_Track_Position_Incorrect(const float32_t vcs_pos_long, const float32_t vcs_pos_lat) const; // function performs sanity check of single track VCS position
      bool Is_Track_Speed_Incorrect(const float32_t speed) const; // function performs sanity check of single track OTG speed
      bool Is_Track_Acceleration_Incorrect(const float32_t accel) const; // function performs sanity check of single track tangential acceleration
   };

   static_assert(4 == sizeof(Output_Faults_T), "sizeof(Output_Faults_T) not as expected. Remember to align padding if needed");
   static_assert(24 == sizeof(Output_Diagnostics::Output_Diagnostics_Calibrations_T), "sizeof(Output_Diagnostics_Calibrations_T) not as expected. Remember to align padding if needed");
}

#endif

