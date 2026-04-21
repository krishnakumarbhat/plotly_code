/*===================================================================================*\
* FILE: f360_detect_veh_trk_close_to_stat_host.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_detect_veh_trk_close_to_stat_host.h"
#include "f360_reuse.h"
#include "f360_math.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Det_VehTrk_Close_To_Stat_Host()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T & host - host property structure
   * const F360_Globals_T & global - global values structure
   * const F360_Calibrations_T & calib - calibration structure
   * F360_Object_Track_T & object_track - single object track
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function detects if track is near stationary host
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Det_VehTrk_Close_To_Stat_Host(
      const F360_Host_T & host,
      const F360_Globals_T & global,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & object_track
   )
   {
      object_track.f_veh_trk_near_stat_host = false;

      if (std::abs(host.vcs_speed) < global.obj_mov_stat_spd_thresh)
      {
         const bool f_valid_trk = ((std::abs(object_track.vcs_position.x + (host.dist_rear_axle_to_vcs_m / (2.0F))) < calib.host_vicinity_vcs_x_range)
               && (std::abs(object_track.vcs_position.y) < calib.host_vicinity_vcs_y_range)
               && (object_track.f_vehicular_trk)
               && (object_track.status > F360_OBJECT_STATUS_NEW_UPDATED)
               && (object_track.time_since_cluster_created > calib.min_object_age_for_object_close_to_stat_host));

         if (f_valid_trk)
         {
            object_track.f_veh_trk_near_stat_host = true;
         }
      }

   }
}
