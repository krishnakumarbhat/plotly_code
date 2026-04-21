/*===========================================================================*\
* FILE: f360_static_environment_class.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Static_Environment_Class()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include <algorithm>
#include "f360_static_environment_class.h"
#include "f360_static_env_helpers.h"
#include "f360_iterator.h"
#include "f360_update_longi_stat_curves.h"
#include "f360_longi_stat_curve_init.h"

namespace f360_variant_A
{
   /*=========================================================================
   * Method         Static_Env_T::Static_Env_T
   *
   * Description    Constructor of Static_Env_T.
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Static_Env_T::Static_Env_T()
   {
      Static_Env_T::Initialize_Static_Env();
   }

   /*=========================================================================
   * Method         Static_Env_T::~Static_Env_T
   *
   * Description    Destructor of Static_Env_T.
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Static_Env_T::~Static_Env_T()
   {

   }

   /*=========================================================================
   * Method         Static_Env_T::Initialize_Stat_Env()
   *
   * Description    Public method used to reset all polynomials
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Static_Env_T::Initialize_Static_Env()
   {
      for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
      {
         Reset_Single_Static_Env_Poly(static_env_polys[i]);
      }

      F360_Longi_Stat_Curve_Init(longi_stat_curves);
   }

   /*=========================================================================
   * Method         Static_Env_T::Get_Polynomials()
   *
   * Description    Public method used to get a const reference to all static polynomials
   *
   * Parameters     None.
   *
   * Returns        const Static_Env_T::Static_Env_Polys_Array& - Constant reference to array with static environment polynomials
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   const Static_Env_T::Static_Env_Polys_Array& Static_Env_T::Get_Polynomials() const {
      return static_env_polys;
   }

   /*=========================================================================
   * Method         Static_Env_T::Set_Polynomials()
   *
   * Description    Public method used to set the private member static_env_polys
   *
   * Parameters     Static_Env_Poly_T(&static_polys_in)[F360_NUM_OF_STATIC_ENV_POLYS]
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Static_Env_T::Set_Polynomials(Static_Env_Poly_T(&static_polys_in)[F360_NUM_OF_STATIC_ENV_POLYS])
   {
      (void)std::copy(cmn::begin(static_polys_in), cmn::end(static_polys_in), cmn::begin(static_env_polys));
   }

   /*=========================================================================
   * Method         Static_Env_T::Run_CWD()
   *
   * Description    Public method used to run Concrete Wall Detector
   *
   * Parameters      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
                     const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list
                     const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
                     const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
                     const F360_Calibrations_T & calib
                     const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
                     const F360_Host_T& host
                     F360_TRKR_TIMING_INFO_T& timing_info

   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Static_Env_T::Run_Concrete_Wall_Detector(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T & calib,
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Host_T& host,
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      if (!concrete_wall_detector.Is_Initialized())
      {
         concrete_wall_detector.Init(sensors, calib);
         concrete_wall_detector.Init_Long_Zones(calib.k_cwd_sensor_zone_half_length);
      }

      concrete_wall_detector.Run_Measurements(det_props, raw_detect_list, sensor_props, calib, object_tracks, std::abs(host.speed), timing_info);
      concrete_wall_detector.Map_Data_To_Static_Env_Poly(static_env_polys, host.curvature_rear);
   }

   /*=========================================================================
   * Method         Static_Env_T::Map_LSC_To_Static_Env_Poly()
   *
   * Description    Maps data from LSC to stationary environment
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Static_Env_T::Map_LSC_To_Static_Env_Poly()
   {
      for (uint8_t i = 0U; i < MAX_NR_OF_LONGI_STAT_CURVES; i++)
      {
         Map_Single_LSC_To_Static_Env_Poly(longi_stat_curves[i], static_env_polys[i]);
      }
   }

   /*=========================================================================
   * Method         Static_Env_T::Run_Longi_Stat_Curves()
   *
   * Description    Updates all LSC polynomials and maps them to stationary environment
   *
   * Parameters     const F360_Tracker_Info_T& tracker_info,
   *                const F360_Calibrations_T& calibs,
   *                const F360_Host_T& host,
   *                F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
   *                F360_TRKR_TIMING_INFO_T& timing_info
   *                
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Static_Env_T::Run_Longi_Stat_Curves(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibs,
      const F360_Host_T& host,
      F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      Update_Longi_Stat_Curves(
         tracker_info,
         calibs,
         host,
         objects,
         longi_stat_curves,
         timing_info
         );

      Map_LSC_To_Static_Env_Poly();
   }
}
