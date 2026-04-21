/*===================================================================================*\
* FILE: f360_host_props.h
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
*   This file contains Host  structure  declaration
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s): defineFusion360Types.m
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_HOST_PROPS_H
#define F360_HOST_PROPS_H

#include "f360_reuse.h"
#include "f360_point.h"
#include "f360_position.h"
#include "f360_host_raw.h"
#include "f360_matrix_dimension.h"
#include "f360_accel.h"
namespace f360_variant_A
{
   typedef struct F360_Host_Props_Tag
   {
      Point position;
      Point delta_position; // Delta for host position between previous and current tracker iteration [m]
      float32_t vel_cov_scm[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Covariance of host front center velocity vector (in WCS) [(m/s)^2]
      float32_t vel_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Covariance of velocity
      float32_t position_inc_cov_scm[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Covariance of position incrementation vector (in WCS) [m^2]
      float32_t position_inc_cov[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]; // Covariance of position incrementation.
      float32_t std_speed_scm; // Standard deviation of host speed at center of rear axle [m/s]
      float32_t std_yaw_rate_scm; // Standard deviation of host yaw rate [rad/s]
      float32_t heading_angle;
      float32_t cos_heading;
      float32_t sin_heading;
      float32_t delta_pointing; // Delta for host pointing angle between previous and current tracker iteration
      float32_t cos_delta_pointing; // Cosine of delta for host pointing angle between previous and current tracker iteration
      float32_t sin_delta_pointing; // Sine of delta for host pointing angle between previous and current tracker iteration
   } F360_Host_Props_T;

   static_assert(112 == sizeof(F360_Host_Props_T), "sizeof(F360_Host_Props_T) not as expected. Remember to align padding if needed");
}
#endif


