/*===================================================================================*\
* FILE: f360_tracked_object_init_info.h
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains F360_Tracker_Info_T  structure declaration
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): defineFusion360Types.m
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_TRACKED_OBJECT_INIT_INFO_H
#define F360_TRACKED_OBJECT_INIT_INFO_H

#include "f360_reuse.h"
#include "f360_velocity.h"
#include "f360_track_init.h"
#include "f360_nees_cfmi_vel_hyp_source.h"
#include "f360_bounding_box.h"
namespace f360_variant_A
{
   typedef struct F360_Tracked_Object_Init_Info_Tag
   {
      F360_Tracked_Object_Init_Info_Tag()
      {
         det_bbox = {};
         VCS_position_cov[0][0] = 0.0F;
         VCS_position_cov[0][1] = 0.0F;
         VCS_position_cov[1][0] = 0.0F;
         VCS_position_cov[1][1] = 0.0F;
         VCS_velocity= {};
         VCS_velocity_cov[0][0] = 0.0F;
         VCS_velocity_cov[0][1] = 0.0F;
         VCS_velocity_cov[1][0] = 0.0F;
         VCS_velocity_cov[1][1] = 0.0F;
         VCS_velocity_plausability = 0.0F;
         frac_az_error_stat_mov = 0.0F;
         init_scheme = {};
         init_vel_source = {};
         f_success = false;
         f_valid_for_liberal_tracking = false;
      }
      
      BoundingBox det_bbox;
      float32_t VCS_position_cov[2][2];
      F360_VCS_Velocity_T VCS_velocity;
      float32_t VCS_velocity_cov[2][2];
      float32_t VCS_velocity_plausability;

      float32_t frac_az_error_stat_mov;

      F360_Track_Init_T init_scheme;
      F360_NEES_CFMI_Vel_Hyp_Source_T init_vel_source;
      bool f_success;
      bool f_valid_for_liberal_tracking;

   } F360_Tracked_Object_Init_Info_T;
}
#endif
