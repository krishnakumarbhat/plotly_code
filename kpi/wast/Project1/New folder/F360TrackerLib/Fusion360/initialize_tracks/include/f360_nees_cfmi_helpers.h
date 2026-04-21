/*===================================================================================*\
* FILE: f360_nees_cfmi_helpers.h
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
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
*   Helpers function common for different nees cost function initialization modules
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
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
#ifndef F360_TNEES_CFMI_HELPERS_H
#define F360_TNEES_CFMI_HELPERS_H

#include "f360_reuse.h"
#include "f360_velocity.h"
#include "f360_cluster.h"
#include "rspp_detection.h"
#include "rspp_detection_list.h"
#include "f360_detection_hist.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
#include "f360_detection_time_slots.h"
#include "f360_nees_cfmi_structs.h"
#include "f360_radar_sensor.h"

namespace f360_variant_A
{
   float32_t Calc_Single_NEES_Value_For_Cloud(const F360_VCS_Velocity_T & VCS_velocity,
      const float32_t cos_azimuth,
      const float32_t sin_azimuth,
      const float32_t range_rate_compensated,
      const float32_t range_rate_var);

   float32_t Calc_Single_NEES_Value_For_Pos_Diff(const F360_VCS_Velocity_T & ref_vel,
      const F360_VCS_Velocity_T & vel_hyp,
      float32_t const (&vel_hyp_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Add_Into_NEES_CFMI_Determinant(F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
      const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants);

   void Add_Into_NEES_CFMI_Determinant_Weighted(F360_NEES_CFMI_Determinants_T& destination_nees_cost_determinants,
      const F360_NEES_CFMI_Determinants_T& source_nees_cost_determinants,
      const float32_t weight);

   void Estimate_Plausibility_For_NEES_CFMI_Velocity(const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Velocity_T& velocity);

   void Calc_Raw_Vel_Cov_For_NEES_CFMI_Velocity(F360_NEES_CFMI_Velocity_T& velocity);

   bool Calc_Raw_Vel_Cov_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants,
      float32_t(&out_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   bool Calc_Vel_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants,
      F360_VCS_Velocity_T& VCS_velocity);

   float32_t Calc_Vel_Determinant_From_NEES_CFMI_Determinants(const F360_NEES_CFMI_Determinants_T& determinants);

   bool Pos_Diff_Vel_Estimate(
      const Point& point_A,
      const Point& point_B,
      const float32_t dt,
      F360_VCS_Velocity_T & velocity);

   bool Pos_Diff_Cov_Estimate(
      const float32_t(&point_A_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t(&point_B_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION],
      const float32_t dt,
      float32_t(&output_cov)[F360_2D_IDX_DIMENSION][F360_2D_IDX_DIMENSION]);

   void Calc_NEES_CFMI_Determinants_Pos_Diff(F360_NEES_CFMI_Pos_Diff_Velocity_T& single_vel);

   bool Check_If_Cluster_Has_Any_Det_In_All_Looks(
      const F360_Cluster_T& cluster_to_init,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Hist_T& detection_hist);
}
#endif
