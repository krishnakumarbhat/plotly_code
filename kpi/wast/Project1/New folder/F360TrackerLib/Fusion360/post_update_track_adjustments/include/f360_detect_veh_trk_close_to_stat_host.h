/*===================================================================================*\
* FILE: f360_detect_veh_trk_close_to_stat_host.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*  This file contains function declaration of Detect_Veh_Trk_Close_To_Stat_Host()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_DETECT_VEH_TRK_CLOSE_TO_STAT_HOST_H
#define F360_DETECT_VEH_TRK_CLOSE_TO_STAT_HOST_H

#include "f360_globals.h"
#include "f360_host.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
namespace f360_variant_A
{
   void Det_VehTrk_Close_To_Stat_Host(
      const F360_Host_T & host,
      const F360_Globals_T & global,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & object_track
   );
}
#endif
