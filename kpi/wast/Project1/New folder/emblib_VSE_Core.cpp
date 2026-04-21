/*=========================================================================
 *  FILE: VSE_Core.cpp
 *=========================================================================
 * Copyright © 2020 Aptiv. All rights reserved.
 * Confidential  Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------
 *
 *  DESCRIPTION:
 *    This file contains State Manager class methods definitions.
 *
 *
 *=========================================================================
 *------------------------------------------------------------------------------
 *
 * class:        VSE_CORE
 *
 * Description:  Wrapper for the Core VSE
 *
 *========================================================================*/
#include "emblib_VSE_Core.h"
#include "emblib_f360_math.h"
#include "reuse.h"
/*===========================================================================*\
 * Public Function
\*===========================================================================*/

namespace vse_core
{

#ifndef DEG2RADF
   #define DEG2RADF ((float)0.0174532925199433F)
   #define RAD2DEGF ((float)1.0F / DEG2RADF)
#endif

enum_quality_factor_T Map_QF(const F360_QUALITY_FACTOR qf)
{
   enum_quality_factor_T out_qf = BMW_VSE_QF_UNDEFINED;
   switch (qf)
   {
      case (F360_QF_UNDEFINED):
      {
         out_qf = BMW_VSE_QF_UNDEFINED;
         break;
      }
      case (F360_QF_TEMP_UNDEFINED):
      {
         out_qf = BMW_VSE_QF_TEMP_UNDEFINED;
         break;
      }
      case (F360_QF_INACCURATE):
      {
         out_qf = BMW_VSE_QF_ACCURATED;
         break;
      }
      case (F360_QF_ACCURATE):
      {
         out_qf = BMW_VSE_QF_ACCURATED;
         break;
      }
      default:
      {
         out_qf = BMW_VSE_QF_UNDEFINED;
         break;
      }
   }

   return out_qf;
}

VSE_CORE::VSE_CORE()
{
}

/*===========================================================================*/
VSE_CORE::~VSE_CORE()
{
}
/*===========================================================================*/
void VSE_CORE::Initialize(const f360_variant_C::F360_Host_Calib_T &r_host_calib)
{
   // Map Vehicle_Parameters input
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_wheel_base                   = r_host_calib.wheelbase_m;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_dist_rear_axle_to_vcs        = r_host_calib.dist_rear_axle_to_vcs_m;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_long_dist_radar_to_rear_axle = 0.0F; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_lat_dist_radar_to_rear_axle  = 0.0F; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_steering_gear_ratio          = r_host_calib.steer_gear_ratio;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_understeer_coefficient       = r_host_calib.understeer_coefficient;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_rear_cornering_compliance = r_host_calib.rear_cornering_compliance;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_vehicle_width             = r_host_calib.vehicle_width_m;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_cog_x = r_host_calib.cog_x; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_cog_y = r_host_calib.cog_y; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_front_wheel_radius_static_loaded =
      r_host_calib.front_wheel_radius_m; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_front_track_width =
      r_host_calib.front_track_width_m; // Currently not used

   this->vehicle_index = 0;

   this->BMW_VSE.initialize();
}

/*===========================================================================*/
void VSE_CORE::Step(const uint64_T timestamp_us, const float speed_correction_factor,
                    const f360_variant_C::F360_Host_Raw_T &r_host_raw)
{
   // Map VCAN_VSE input
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_speed_mps          = (F360_Fabsf)(r_host_raw.raw_speed);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_speed_qf           = Map_QF((F360_QUALITY_FACTOR)r_host_raw.speed_qf);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_yaw_rate_rps       = r_host_raw.raw_yaw_rate_rad;
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_yaw_rate_qf        = Map_QF((F360_QUALITY_FACTOR)r_host_raw.yaw_rate_qf);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.f_stationary           = (0.0F == r_host_raw.raw_speed);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.f_reverse              = (0.0F > r_host_raw.raw_speed);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_steering_angle_deg = RAD2DEGF * r_host_raw.steering_wheel_angle_rad;
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_steering_angle_qf =
      Map_QF((F360_QUALITY_FACTOR)r_host_raw.steering_wheel_angle_qf);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_lat_accel         = r_host_raw.lat_accel;
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_lat_accel_qf      = Map_QF((F360_QUALITY_FACTOR)r_host_raw.lat_accel_qf);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_long_accel        = r_host_raw.long_accel;
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.raw_long_accel_qf     = Map_QF((F360_QUALITY_FACTOR)r_host_raw.long_accel_qf);
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.wheel_rpm_front_left  = 0.0F;           // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.wheel_rpm_front_right = 0.0F;           // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.wheel_lin_speed_mps_front_left  = 0.0F; // Currently not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p.wheel_lin_speed_mps_front_right = 0.0F; // Currently not used

   // Map Tracker_VSE input
   this->BMW_VSE.BMW_VSE_Master_Model_U.Tracker_VSE_j.VehSpeedCompFac_KA = speed_correction_factor;
   this->BMW_VSE.BMW_VSE_Master_Model_U.Tracker_VSE_j.VehSpeedCompFac_KA_QF =
      BMW_VSE_QF_ACCURATED; // Not available. Setting to accurate

   // Map System_Time_Micro_Sec input
   this->BMW_VSE.BMW_VSE_Master_Model_U.System_Time_Micro_Sec = static_cast<real_T>(timestamp_us);

   // Map LAST_KEY_CYCLE input
   this->BMW_VSE.BMW_VSE_Master_Model_U.LAST_KEY_CYCLE_e.nvm_last_rem_swa_bias_deg = 0.0F;  // Not used
   this->BMW_VSE.BMW_VSE_Master_Model_U.LAST_KEY_CYCLE_e.nvm_f_last_rem_swa_bias   = false; // Not used

   this->BMW_VSE.step();

   this->vehicle_index++;
}

/*===========================================================================*/
void VSE_CORE::Get_Output(f360_variant_C::F360_Host_T &r_host)
{
   // Map output from
   r_host.vehicle_index = this->vehicle_index;

   const float abs_speed = F360_Hypotf(this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_long_velocity,
                                       this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_lat_velocity);
   r_host.vcs_speed = (0.0F > this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_long_velocity) ? -1.0F * abs_speed : abs_speed;
   r_host.yaw_rate_rad =
      this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.comp_yaw_rate_filtered; // There also wxist unfiltered yawrate
   r_host.vcs_sideslip            = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_sideslip;
   r_host.curvature_rear          = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.curvature_rear_axle;
   r_host.dist_rear_axle_to_vcs_m = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.k_dist_rear_axle_to_vcs;
   r_host.vcs_lat_acceleration    = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_lat_accel;
   r_host.vcs_long_acceleration   = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.vcs_long_accel;

   // World coordinates
   // this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.xpos;
   // this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.ypos;
   // this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.heading;

   r_host.speed        = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.filt_veh_speed_over_ground;
   r_host.acceleration = this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output.accel_rear_axle;

   r_host.rear_cornering_compliance = this->BMW_VSE.BMW_VSE_Master_Model_U.Vehicle_Parameters_f.k_rear_cornering_compliance;
   r_host.speed_correction_factor   = this->BMW_VSE.BMW_VSE_Master_Model_U.Tracker_VSE_j.VehSpeedCompFac_KA;
}

VSE_OUT VSE_CORE::Get_VSE_Output()
{
   return this->BMW_VSE.BMW_VSE_Master_Model_Y.VSE_Output;
}

VCAN_VSE VSE_CORE::Get_VCAN_VSE_Inputs()
{
   return this->BMW_VSE.BMW_VSE_Master_Model_U.VCAN_VSE_p;
}
} // namespace vse_core
