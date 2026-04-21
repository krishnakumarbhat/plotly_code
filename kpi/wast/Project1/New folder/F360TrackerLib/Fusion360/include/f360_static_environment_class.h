/*===========================================================================*\
* FILE: f360_static_environment_class.h
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declaration of Stationary_Environment()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_STATIC_ENVIRONMENT_CLASS_H
#define F360_STATIC_ENVIRONMENT_CLASS_H

#include "f360_constants.h"
#include "f360_static_env_poly_types.h"
#include "f360_static_env_helpers.h"

#include "f360_concrete_wall_detector.h"

#include "f360_tracker_info.h"
#include "f360_calibrations.h"
#include "f360_object_track.h"
#include "f360_radar_sensor_props.h"
#include "f360_longi_stat_curve.h"
#include "f360_timing_info.h"
#include "f360_host.h"
#include "rspp_detection_list.h"

namespace f360_variant_A
{
   class Static_Env_T
   {
   public:

      using Static_Env_Polys_Array = Static_Env_Poly_T[F360_NUM_OF_STATIC_ENV_POLYS];

      Static_Env_T();

      ~Static_Env_T();

      void Initialize_Static_Env();

      const Static_Env_Polys_Array& Get_Polynomials() const;

      void Set_Polynomials(Static_Env_Poly_T(&static_polys_in)[F360_NUM_OF_STATIC_ENV_POLYS]);

      void Run_Concrete_Wall_Detector(const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
         const F360_Calibrations_T &calib,
         const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
         const F360_Host_T& host,
         F360_TRKR_TIMING_INFO_T& timing_info);

      void Run_Longi_Stat_Curves(
         const F360_Tracker_Info_T& tracker_info,
         const F360_Calibrations_T& calibs,
         const F360_Host_T& host,
         F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
         F360_TRKR_TIMING_INFO_T& timing_info);

      const F360_Concrete_Wall_Detector_T& Get_CWD() const { return this->concrete_wall_detector; };
      void Set_CWD(const F360_Concrete_Wall_Detector_T& concrete_wall_detector_input) { this->concrete_wall_detector = concrete_wall_detector_input; };

   private:
      Static_Env_Poly_T static_env_polys[F360_NUM_OF_STATIC_ENV_POLYS];
      F360_Longi_Stat_Curve_T longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES];
      F360_Concrete_Wall_Detector_T concrete_wall_detector;

      void Map_LSC_To_Static_Env_Poly();

   };
}


#endif
