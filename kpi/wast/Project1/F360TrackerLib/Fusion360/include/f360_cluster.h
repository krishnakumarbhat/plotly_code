/*===================================================================================*\
* FILE: cluster.h
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
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
*   This file contains Cluster_T structure declaration
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): TypesFusion360.h
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
#ifndef F360_CLUSTER_H
#define F360_CLUSTER_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_velocity.h"
#include "f360_point.h"
#include "f360_accel.h"
#include "f360_object_status.h"
#include "f360_look_type.h"
#include "f360_track_init.h"
#include "f360_trk_fltr_type.h"
#include "f360_object_class.h"
#include "f360_cluster_motion_status.h"
namespace f360_variant_A
{
   typedef struct F360_Cluster_Tag
   {
      Point vcs_position;
      float32_t rep_vcs_az;
      float32_t cos_vcs_az;
      float32_t sin_vcs_az;
      float32_t rep_rdotcomp;
      float32_t exist_prob;
      float32_t time_since_created; // currently contains time relative to tracker execution time instead of measurement time 
      float32_t time_since_cluster_updated; // currently contains time relative to tracker execution time instead of measurement time 
      float32_t time_since_measurement; // measurement time relative to current time
      float32_t priority; // cluster priority value [-], range <0, 1> (higher priority means that cluster is more important) 
      int16_t id;
      int16_t ndets;
      int16_t detids[MAX_DETS_IN_OBJ_TRK];
      int16_t old_det_idx[MAX_DETS_IN_OBJ_TRK];
      int16_t num_types_of_dets[2];
      int16_t num_old_dets;
      uint16_t num_of_cluster_merges; // number of cluster merges in current tracker instance [-]
      F360_Cluster_Motion_Status_T motion_status; // Motion status of cluster.
      bool f_dealiased;
      bool f_to_be_killed; // Flag indicating that cluster should be killed by it's not yet done.
      uint8_t low_rcs_dets_cnt;
   } F360_Cluster_T;

   #ifdef __TASKING__
      static_assert(
         (sizeof(F360_Cluster_T::detids) + sizeof(F360_Cluster_T::old_det_idx) + 60) == sizeof(F360_Cluster_T), "sizeof(F360_Cluster_T) not as expected. Remember to align padding if needed");
   #else
      static_assert(
         (sizeof(F360_Cluster_T::detids) + sizeof(F360_Cluster_T::old_det_idx) + 60) == sizeof(F360_Cluster_T), "sizeof(F360_Cluster_T) not as expected. Remember to align padding if needed");
   #endif
}
#endif
