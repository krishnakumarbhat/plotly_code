/*===========================================================================*\
* FILE: f360_initialization_data_generator.h
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
#ifndef F360_INITILIZATION_DATA_GENERATORS_H
#define F360_INITILIZATION_DATA_GENERATORS_H

#include "f360_initialize_tracks.h"

namespace f360_variant_A
{
   void Define_Test_Vector_4_Dealiased_and_2_NotDealiased_Clusters(
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS]
   );

   void Define_Test_Vector_4_Obj_Tracks(
      F360_Object_Track_T(&obj_track)[NUMBER_OF_OBJECT_TRACKS]
   );

   void Define_Test_Vector_N_Obj_Tracks(
      F360_Object_Track_T(&obj_track)[NUMBER_OF_OBJECT_TRACKS],
      uint16_t tracks_to_generate
   );

   void Define_Test_Vector_4_Valid_Moving_Hist_Dets_Different_Positions(
      F360_Detection_Hist_T &det_hist
   );

   void Define_Test_Vector_3_Static_Valid_Hist_Dets(
      F360_Detection_Hist_T &det_hist
   );

   void Define_Test_Vector_4_Valid_Moving_Dets_Different_Positions(
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   );

   void Define_Test_Vector_3_Dets_RangeRate_Diff(
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   );


   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   );

   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Succesfull_Obj_Init_2(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   );

   void Define_Test_Vector_Cluster_With_2_Dets_and_2_Hist_Dets_For_Unsuccesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   );

   void Define_Test_Vector_Cluster_With_3_Dets_and_3_Hist_Dets_1_Det_1_Hist_Det_with_azimuth_error_stat_mov_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   );

   void Define_Test_Vector_Cluster_With_4_Dets_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Host_Props_T &host_props,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list
   );

   void Define_Test_Vector_Cluster_With_3_Moving_Dets_4_Stationary_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Host_Props_T &host_props
   );

   void Define_Test_Vector_Stationary_Cluster_With_3_Stationary_Det_Succesfull_Obj_Init(
      F360_Tracker_Info_T &tracker_info,
      F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      F360_Detection_Hist_T &det_hist,
      F360_Host_Props_T &host_props
   );
}
#endif
