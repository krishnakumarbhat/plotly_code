/*===========================================================================*\
* FILE: f360_initialization_data_generator_support_functions.h
*============================================================================
* Copyright ? 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential ? Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains tracks, clusters, detections data generation for unit testing
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [30-Mar-2018]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/
#ifndef F360_INITIALIZATION_DATA_GENERATOR_SUPPORT_FUNCTIONS_H
#define F360_INITIALIZATION_DATA_GENERATOR_SUPPORT_FUNCTIONS_H

#include "f360_initialize_tracks.h"

namespace f360_variant_A
{
   float32_t Get_Norm_From_Cart_Pos(
      float32_t posn_lat,
      float32_t posn_long
   );

   float32_t Get_RangeRate_From_Cart_Coordinates(
      float32_t posn_lat,
      float32_t posn_long,
      float32_t vel_lat,
      float32_t vel_long
   );

   float32_t Move_Pos_By_Step(
      float32_t posn,
      float32_t vel,
      float32_t time_step
   );

   F360_Detection_Hist_Data_T Gen_Single_Hist_Det(
      const rspp_variant_A::RSPP_Detection_Motion_Status_T &motion_status,
      float32_t expected_pos_lat,
      float32_t expected_pos_long,
      float32_t vel_lat,
      float32_t vel_long,
      float32_t dt,
      float32_t pos_std
   );

   std::pair<rspp_variant_A::RSPP_Detection_T,F360_Detection_Props_T> Gen_Single_Det(
      const rspp_variant_A::RSPP_Detection_Motion_Status_T &motion_status,
      float32_t expected_pos_lat,
      float32_t expected_pos_long,
      float32_t vel_lat,
      float32_t vel_long,
      float32_t dt,
      float32_t pos_std,
      uint16_t global_id
   );

   int16_t Get_Det_First_Free_Idx(
      const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list
   );

   void Fill_Clusters_With_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat
   );

   void Fill_Clusters_With_Stationary_Cluster_With_4_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat
   );

   void Fill_Clusters_With_Moving_Cluster_With_4_Moving_Dets_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      float32_t cluster_pos_long,
      float32_t cluster_pos_lat,
      float32_t expected_vel_long,
      float32_t expected_vel_lat
   );
}
#endif
