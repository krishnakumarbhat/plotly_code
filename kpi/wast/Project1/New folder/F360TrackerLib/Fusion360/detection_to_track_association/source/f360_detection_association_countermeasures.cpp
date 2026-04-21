/*===================================================================================*\
* FILE:  f360_detection_association_countermeasures.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*--------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains functions for different countermeasures active related to detection
* to track association.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_detection_association_countermeasures.h"
#include "f360_mark_detections_wheel_spin_from_objects.h"
#include "f360_mark_dets_as_close_target_and_farside.h"
#include "f360_find_detection_inliers.h"
#include "f360_nearby_wheel_spins.h"
#include "f360_mark_azimuth_range_rate_outliers.h"
#include "f360_cond_deassoc_low_rr_dets.h" 

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: Detection_Association_Countermeasures()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Tracker_Info_T & tracker_info
   * const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list
   * const F360_Calibrations_T & calibrations
   * const F360_Host_T & host
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]
   * F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
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
   * This function collects and calls all countermeasures active for detection
   * association, both detection-based and object-based.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Detection_Association_Countermeasures(
      const F360_Tracker_Info_T & tracker_info,
      const rspp_variant_A::RSPP_Detection_List_T & raw_detection_list,
      const F360_Calibrations_T & calibrations,
      const F360_Host_T & host,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_Object_Track_T (&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      // Track-based countermeasures
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[i] - 1;
         F360_Object_Track_T& object = object_tracks[obj_idx];

         Mark_Detections_Wheel_Spin_From_Objects(
            tracker_info,
            raw_detection_list.detections,
            raw_detection_list.number_of_valid_detections,
            sensors,
            calibrations,
            object,
            detection_props);

         Mark_Azimuth_Range_Rate_Outliers(
            object,
            calibrations,
            host.dist_rear_axle_to_vcs_m,
            raw_detection_list,
            detection_props);

         Mark_Dets_As_Close_Target_And_Farside(
            raw_detection_list.number_of_valid_detections,
            raw_detection_list,
            object,
            detection_props,
            calibrations);

         Cond_Deassoc_Low_RR_Dets(
            calibrations,
            raw_detection_list,
            detection_props,
            object);
      }

      // Detection-based countermeasure
      Detect_Nearby_Wheel_Spins(raw_detection_list, calibrations, detection_props);
   }

}
