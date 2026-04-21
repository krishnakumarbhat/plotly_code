/*===================================================================================*\
* FILE: f360_object_based_angle_jump_detector.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
*    This file contains implementations of functions related to object based angle 
*    jump detector.
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

#include "f360_object_based_angle_jump_detector.h"
#include "f360_object_based_angle_jump_detector_internals.h"
#include "f360_math.h"

namespace f360_variant_A
{
   //namespace aj_detector - blocked for now. It will be resotred once problem with embedded build is solved - DFU-511
   //{
      /*===========================================================================*\
      * FUNCTION: Detect_Angle_Jumps_From_Objects()
      *===========================================================================
      * RETURN VALUE:
      * None
      *
      * PARAMETERS:
      *  const F360_Tracker_Info_T & tracker_info,
      *  const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      *  const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
      *  F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      *  const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      *  const F360_Calibrations_T &calibs
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
      * Main function of object based angle jump detector. Angle jumps are found
      * based on object parameters (i.e. expected range rate). Only object that
      * are close enough to the host are taken into account.
      *
      * PRECONDITIONS:
      * Object should be in "time updated" state
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
      void Check_Dets_Against_Angle_Jumps(
         const F360_Object_Track_T &obj_track,
         const rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
         const bool(&valid_sensors)[MAX_NUMBER_OF_SENSORS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         const F360_Calibrations_T &calibs,
         F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
      )
      {
         Det_Restrictions_T det_restrictions = Calc_Det_Restrictions_Without_Max_Range(obj_track, calibs);

         for (uint32_t det_idx = 0U; det_idx < raw_detection_list.number_of_valid_detections; det_idx++)
         {
            const rspp_variant_A::RSPP_Detection_T &det_raw = raw_detection_list.detections[det_idx];

            const int32_t sensor_idx = det_raw.raw.sensor_id - 1;
            if (valid_sensors[sensor_idx] && (!detection_props[det_idx].f_object_based_angle_jump))
            {
               det_restrictions.max_range = Calc_Max_Range(obj_track.bbox.Get_Center().y, sensors[sensor_idx].constant.mounting_position.vcs_position.lateral, calibs.obj_aj_det_range_gap);
               F360_Detection_Props_T &det_prop = detection_props[det_idx];

               if (Is_Det_Suspected(det_raw.raw.range, det_raw.raw.confid_azimuth, det_prop.vcs_position, det_restrictions))
               {
                  const F360_VCS_Velocity_T sen_velocity_vcs = sensors[sensor_idx].variable.vcs_velocity;
                  const Point sen_pos_vcs = Point(sensors[sensor_idx].constant.mounting_position.vcs_position.longitudinal, sensors[sensor_idx].constant.mounting_position.vcs_position.lateral);
                  det_prop.f_object_based_angle_jump = Is_Det_Object_Based_Angle_Jump(sen_velocity_vcs, sen_pos_vcs, det_prop, det_raw, obj_track.vcs_velocity, det_restrictions, calibs);

                  if (det_prop.f_object_based_angle_jump)
                  {
                     det_prop.f_ok_to_use = false;
                  }
               }
            }
         }
      }
   //} DFU-511
}
