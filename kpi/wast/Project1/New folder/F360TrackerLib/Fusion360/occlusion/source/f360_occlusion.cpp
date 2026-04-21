/*===================================================================================*\
* FILE:  f360_occlusion_detector.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Occlusion_T class methods.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/


#include "f360_occlusion.h"
#include "f360_get_wall_time.h"
#include "f360_timing_info.h"
#include "f360_update_by_tracks_helpers.h"
#include "f360_get_track_bbox_in_vcs.h"
#include "f360_math_func.h"
#include "f360_calc_point_scs_position.h"
#include "f360_get_vcs_visible_edges.h"
#include "f360_rectangle.h"
#include <algorithm>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Occlusion_T
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const bool f_enable_occlusion
   * const F360_Calibrations_T& calib
   * const F360_Tracker_Info_T& tracker_info
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * F360_TRKR_TIMING_INFO_T &timing_info
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
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
   * Constructor of Occlusion_T
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Occlusion_T::Occlusion_T(
      const bool f_enable_occlusion,
      const F360_Calibrations_T& calib,
      const F360_Tracker_Info_T& tracker_info,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      F360_TRKR_TIMING_INFO_T &timing_info,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]):
      f_enabled(f_enable_occlusion),
      sensors_occlusion()
   {
      const float32_t start_time = get_wall_time();
      if (f_enabled)
      {
         Update_Sensors_Calibrations(sensors, calib.k_occlusion_range_uncertainty_th);
         Update_Sensors_Occlusion(calib, tracker_info, object_tracks);
      }
      timing_info.occlusion = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::~Occlusion_T
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * None.
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
   * Destructor of Occlusion_T.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   Occlusion_T::~Occlusion_T()
   {

   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Determine_Occlusion_Status()
   *===========================================================================
   * RETURN VALUE:
   * Occlusion_Status_T occl_status - Enum indicating point occlusion status.
   *
   * PARAMETERS:
   * const float32_t vcs_long_posn
   * const float32_t vcs_lat_posn
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
   * Public method used to determine given point occlusion status combined for 
   * all sensor viewpoints
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Occlusion_Status_T Occlusion_T::Determine_Occlusion_Status(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn) const
   {
      Occlusion_Status_T occl_status;
      if (f_enabled)
      {
         occl_status = Determine_Point_Occlusion_Status(vcs_long_posn, vcs_lat_posn);
      }
      else
      {
         occl_status = Occlusion_Status_T::OCCLUSION_STATUS_UNDEFINED;
      }

      return occl_status;
   }


   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Determine_Occlusion_Status_Single_Sensor()
   *===========================================================================
   * RETURN VALUE:
   * Occlusion_Status_T occl_status - Enum indicating point occlusion status.
   *
   * PARAMETERS:
   * const float32_t vcs_long_posn
   * const float32_t vcs_lat_posn
   * const int32_t sensor_idx
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
   * Public method used to determine given point occlusion status for a single sensor
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Occlusion_Status_T Occlusion_T::Determine_Occlusion_Status_Single_Sensor(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn,
      const int32_t sensor_idx) const
   {
      Occlusion_Status_T occl_status;
      if (f_enabled)
      {
         occl_status = sensors_occlusion[sensor_idx].Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);
      }
      else
      {
         occl_status = Occlusion_Status_T::OCCLUSION_STATUS_UNDEFINED;
      }

      return occl_status;
   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Update_Sensors_Calibrations()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS] - Sensor calibrations array
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]             - Sensor properties array
   * const float32_t range_uncertainty_th                                  - Range uncertainty threashold
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
   * Method used to update sensors properties.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Occlusion_T::Update_Sensors_Calibrations(
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const float32_t range_uncertainty_th)
   {
      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         sensors_occlusion[i].Update_Calibs(sensors[i], range_uncertainty_th);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Update_Sensors_Occlusion()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calib
   * const F360_Tracker_Info_T& tracker_info
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
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
   * Method used to update occlusion info by tracks.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Occlusion_T::Update_Sensors_Occlusion(
      const F360_Calibrations_T& calib,
      const F360_Tracker_Info_T& tracker_info,
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS])
   {
      const Rectangle_T zone(calib.k_occlusion_min_long, calib.k_occlusion_max_long,
         calib.k_occlusion_min_lat, calib.k_occlusion_max_lat);

      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         F360_Object_Track_T& object = object_tracks[tracker_info.active_obj_ids[i] - 1];

         if (Is_Object_Relevant_For_Occlusion(calib.k_occlusion_confidence_level, object) &&
            zone.Contains(object.bbox.Get_Center().x, object.bbox.Get_Center().y))
         {
            object.f_used_by_occlusion = true;
            Update_Sensors_Occlusion(object);
         }
         else
         {
            object.f_used_by_occlusion = false;
         }
      }
   }


   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Determine_Point_Occlusion_Status()
   *===========================================================================
   * RETURN VALUE:
   * Occlusion_Status_T occl_status - Enum indicating point occlusion status.
   *
   * PARAMETERS:
   * const float32_t vcs_long_posn
   * const float32_t vcs_lat_posn
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
   * Detailed implementation of method used to determine occlusion status of given point.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Occlusion_Status_T Occlusion_T::Determine_Point_Occlusion_Status(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn) const
   {
      Occlusion_Status_T occl_status = Occlusion_Status_T::OCCLUSION_STATUS_UNDEFINED;

      for (unsigned i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         const Occlusion_Status_T sens_occl_status = sensors_occlusion[i].Determine_Occlusion_Status(vcs_long_posn, vcs_lat_posn);

         occl_status = Pick_Status_With_Higher_Visibility(occl_status, sens_occl_status);

         if (Occlusion_Status_T::OCCLUSION_STATUS_VISIBLE == occl_status)
         {
            break;
         }
      }
      return occl_status;
   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Update_Sensors_Occlusion()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object
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
   * Method selects visible edges of object and passes them to sensors to update 
   * their occlusion information
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Occlusion_T::Update_Sensors_Occlusion(const F360_Object_Track_T& object)
   {
      const VCS_Track_Visible_Edges_T vcs_visible_edges = Get_VCS_Visible_Edges(object);

      for (uint32_t sensor_idx = 0U; sensor_idx < MAX_NUMBER_OF_SENSORS; sensor_idx++)
      {
         sensors_occlusion[sensor_idx].Update_Occlusion(vcs_visible_edges);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Occlusion_T::Pick_Status_With_Higher_Visibility()
   *===========================================================================
   * RETURN VALUE:
   * Occlusion_Status_T (std::max(first, second))
   *
   * PARAMETERS:
   * const Occlusion_Status_T first
   * const Occlusion_Status_T second
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
   * Method selects occlusion status with higher visibility information.
   * 
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Occlusion_Status_T Occlusion_T::Pick_Status_With_Higher_Visibility(
      const Occlusion_Status_T first, 
      const Occlusion_Status_T second) const
   {
      return (std::max(first, second));
   }
}

