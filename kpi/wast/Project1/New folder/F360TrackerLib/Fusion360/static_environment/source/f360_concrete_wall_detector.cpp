/*===================================================================================*\
* FILE: f360_concrete_wall_detector.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* Contains function definitions for f360_concrete_wall_detector.h
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#include "f360_concrete_wall_detector.h"
#include "f360_vcs_long_sorted_dets_support_functions.h"
#include "f360_math_func.h"
#include "f360_get_wall_time.h"
#include <limits>

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::F360_Concrete_Wall_Detector_T()
   *===========================================================================
   * RETURN VALUE:
   * None
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
   * Constructor of F360_Concrete_Wall_Detector_T class.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   F360_Concrete_Wall_Detector_T::F360_Concrete_Wall_Detector_T()
   {
      f_initialized = false;
      cwd_long_zones_count = 0U;
      cwd_sensors_count = 0U;
      std::fill(cmn::begin(long_sorted_sensors_idxs), cmn::end(long_sorted_sensors_idxs), 0U);

      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; ++i)
      {
         long_zones[i].num_sensors_in_zone = 0U;
         long_zones[i].x_min = 0.0F;
         long_zones[i].x_max = 0.0F;
         std::fill(cmn::begin(long_zones[i].sensor_idxs), cmn::end(long_zones[i].sensor_idxs), 0U);
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::~F360_Concrete_Wall_Detector_T()
   *===========================================================================
   * RETURN VALUE:
   * None
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
   * Destructor of F360_Concrete_Wall_Detector_T class.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   F360_Concrete_Wall_Detector_T::~F360_Concrete_Wall_Detector_T()
   {
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Init()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Calibrations_T &calib
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
   * This method does initialization of CWD internal structures at the begining of run Static Environment module.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Init(const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS], const F360_Calibrations_T &calib)
   {
      float32_t sensors_long_posn[MAX_NUMBER_OF_SENSORS]; // helper array
      cwd_sensors_count = 0U;

      for (uint32_t sens_idx = 0U; sens_idx < MAX_NUMBER_OF_SENSORS; sens_idx++)
      {
         if ((sensors[sens_idx].variable.is_valid) && (std::abs(sensors[sens_idx].constant.mounting_position.vcs_position.lateral) > calib.k_cwd_min_lat_posn_for_cw_sensor))
         {
            cwd_sensors[sens_idx].Init(sensors, sens_idx, calib.k_cwd_sensor_zone_half_length, calib.k_cwd_max_lateral_range);
            sensors_long_posn[sens_idx] = sensors[sens_idx].constant.mounting_position.vcs_position.longitudinal;
            cwd_sensors_count++;

            if (cwd_sensors[sens_idx].Get_Sensor_Side_Sign() < 0.0F)
            {
               cw_left.Append_Sensor_Idx(sens_idx);
            }
            else
            {
               cw_right.Append_Sensor_Idx(sens_idx);
            }
         }
         else
         {
            sensors_long_posn[sens_idx] = std::numeric_limits<float32_t>::quiet_NaN();
         }
      }

      (void)F360_Sort(sensors_long_posn, MAX_NUMBER_OF_SENSORS, true, long_sorted_sensors_idxs);
      f_initialized = 0U < cwd_sensors_count;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Reset_All_CWD_Sensors()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Calibrations_T &calib
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
   * This method perform reset of Concrete_Wall_Detector_Sensor_T cw_sensors[].
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Reset_All_CWD_Sensors(const F360_Calibrations_T &calib)
   {
      for (uint32_t cw_sensor_idx = 0U; cw_sensor_idx < cwd_sensors_count; cw_sensor_idx++)
      {
         cwd_sensors[cw_sensor_idx].Reset(calib.k_cwd_max_lateral_range);
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Are_Zones_Overlapping()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const uint32_t curr_sens_idx
   * const uint32_t next_sens_idx
   * const float32_t & sensor_zone_half_length
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
   * This method checks if zones of two sensors overlap.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool F360_Concrete_Wall_Detector_T::Are_Sensors_Zones_Overlapping(const uint32_t curr_sens_idx, const uint32_t next_sens_idx, const float32_t sensor_zone_half_width) const
   {
      const float32_t sens_long_posn_diff = cwd_sensors[next_sens_idx].Get_Sensor_Long_Posn() - cwd_sensors[curr_sens_idx].Get_Sensor_Long_Posn();
      return sens_long_posn_diff <= (2.0F * sensor_zone_half_width);
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Set_Zone_Beginning()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const uint32_t sens_idx
   * Long_Zone_T& zone
   * const float32_t & sensor_zone_half_length
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
   * This method is used in defining zones for sensors.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Set_Zone_Beginning(const uint32_t sens_idx, const uint32_t zone_idx, const float32_t sensor_zone_half_length)
   {
      long_zones[zone_idx].x_min = cwd_sensors[sens_idx].Get_Sensor_Long_Posn() - sensor_zone_half_length;
      long_zones[zone_idx].sensor_idxs[long_zones[zone_idx].num_sensors_in_zone] = sens_idx;
      long_zones[zone_idx].num_sensors_in_zone += 1U;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Set_Zone_Ending()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const uint32_t sens_idx
   * Long_Zone_T& zone
   * const float32_t & sensor_zone_half_length
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
   * This method is used in defining zones for sensors.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Set_Zone_Ending(const uint32_t sens_idx, const uint32_t zone_idx, const float32_t sensor_zone_half_length)
   {
      long_zones[zone_idx].x_max = cwd_sensors[sens_idx].Get_Sensor_Long_Posn() + sensor_zone_half_length;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Add_Sensor_Idx_To_Zone_Definition()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const uint32_t sens_idx
   * Long_Zone_T& zone
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
   * This method is used in defining zones for sensors.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Add_Sensor_Idx_To_Zone_Definition(const uint32_t sens_idx, const uint32_t zone_idx)
   {
      long_zones[zone_idx].sensor_idxs[long_zones[zone_idx].num_sensors_in_zone] = sens_idx;
      long_zones[zone_idx].num_sensors_in_zone += 1U;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Init_Long_Zones()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const float32_t & sensor_zone_half_length
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
   * This method defines zone for each sensor. This zone is used for selection detections that could be relevant for CWD.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Init_Long_Zones(const float32_t sensor_zone_half_length)
   {
      if (0U < cwd_sensors_count)
      {
         uint32_t curr_sens_idx = long_sorted_sensors_idxs[0];
         uint32_t next_sens_idx = curr_sens_idx;

         // we start first zone definition (set the x_min, add sensor idx) before the loop
         uint32_t long_zone_idx = 0U;
         Set_Zone_Beginning(curr_sens_idx, long_zone_idx, sensor_zone_half_length);


         for (uint32_t i = 1U; i < cwd_sensors_count; i++)
         {
            next_sens_idx = long_sorted_sensors_idxs[i];

            if (Are_Sensors_Zones_Overlapping(curr_sens_idx, next_sens_idx, sensor_zone_half_length))
            {
               Add_Sensor_Idx_To_Zone_Definition(next_sens_idx, long_zone_idx);
               // do not update x_min nor x_max until there is no longer overlap
            }
            else // no more overalpping zones, we can finish zone definition with current sensor and start new definition with new sensor
            {
               Set_Zone_Ending(curr_sens_idx, long_zone_idx, sensor_zone_half_length);
               long_zone_idx += 1U; // jump to new zone
               Set_Zone_Beginning(next_sens_idx, long_zone_idx, sensor_zone_half_length);
            }
            curr_sens_idx = next_sens_idx;
         }

         // after the loop we need to finish definition (set the x_max) of current zone
         Set_Zone_Ending(next_sens_idx, long_zone_idx, sensor_zone_half_length);
         long_zone_idx += 1U; // count last zone
         cwd_long_zones_count = long_zone_idx; // this is count of valid long zones
      }
   }


   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Is_Initialized()
   *===========================================================================
   * RETURN VALUE:
   * bool
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
   * This method checks if CWD is initialized.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool F360_Concrete_Wall_Detector_T::Is_Initialized() const
   {
      return f_initialized;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Set_Extreme_Dets_Idxs_For_Each_Sensor_Zone()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list
   * const F360_Calibrations_T &calib
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
   * This method finds detections that could be relevant for CWD. This finding is perform for each sensor
   * and idxs of first and last detection in sensor zone is stored respectively in: "first_valid_det_idx" and "last_valid_det_idx"
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Set_Extreme_Dets_Idxs_For_Each_Sensor_Zone(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Calibrations_T &calib)
   {
      Reset_All_CWD_Sensors(calib);

      int32_t det_idx = Get_First_Relevant_Long_Sorted_Det_Idx(long_zones[0].x_min, raw_detect_list);

      for (uint32_t zone_idx = 0U; zone_idx < cwd_long_zones_count; zone_idx++)
      {
         while ((det_idx != F360_INVALID_ID) && (det_props[det_idx].vcs_position.x < long_zones[zone_idx].x_min))
         {
            det_idx = raw_detect_list.detections[det_idx].processed.next_sorted_idx;
         }

         while ((det_idx != F360_INVALID_ID) && (det_idx != raw_detect_list.detections[det_idx].processed.next_sorted_idx))
         {
            for (uint32_t in_zone_sensor_idx = 0U; in_zone_sensor_idx < long_zones[zone_idx].num_sensors_in_zone; in_zone_sensor_idx++)
            {
               const uint32_t sensor_idx = long_zones[zone_idx].sensor_idxs[in_zone_sensor_idx];
               cwd_sensors[sensor_idx].Set_Dets_Inside_Sensor_Zone_Edges(det_props[det_idx].vcs_position.x, det_idx);
            }

            if (det_props[det_idx].vcs_position.x > long_zones[zone_idx].x_max)
            {
               break;
            }

            det_idx = raw_detect_list.detections[det_idx].processed.next_sorted_idx;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Run_Measurements()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list
   * const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS]
   * const F360_Calibrations_T &calib
   * const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS]
   * const float32_t abs_host_speed
   * F360_TRKR_TIMING_INFO_T& timing_info
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
   * This is main method of CWD where all calclations are perform.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Run_Measurements(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const F360_Calibrations_T &calib,
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const float32_t abs_host_speed,
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      const float32_t start_time = get_wall_time();

      // Find detections inside sensors zones regarding to their longitudinal position.
      Set_Extreme_Dets_Idxs_For_Each_Sensor_Zone(det_props, raw_detect_list, calib);

      for (uint32_t sensor_idx = 0U; sensor_idx < cwd_sensors_count; sensor_idx++)
      {
         cwd_sensors[sensor_idx].Execute_Measurements(det_props, raw_detect_list, sensor_props[sensor_idx], calib, object_tracks);
      }

      cw_left.Fuse_CWD_Side_Data(cwd_sensors, abs_host_speed, calib);
      cw_right.Fuse_CWD_Side_Data(cwd_sensors, abs_host_speed, calib);

      if (abs_host_speed <= calib.k_cwd_min_host_speed_for_cwd)
      {
         cw_left.Invalidate_Status();
         cw_right.Invalidate_Status();
      }

      timing_info.concrete_wall_detector = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Concrete_Wall_Detector_T::Map_Data_To_Static_Env_Poly()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * Static_Env_Poly_T(&stat_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const float32_t host_curvature_rear
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
   * This method handles of mapping CWD data to Static_Env_Poly_T srtructure for left/right host side.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Concrete_Wall_Detector_T::Map_Data_To_Static_Env_Poly(Static_Env_Poly_T(&stat_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS], const float32_t host_curvature_rear) const
   {
      cw_left.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_polys[4U], host_curvature_rear);
      cw_right.Map_CWD_Side_Data_To_Static_Env_Poly(stat_env_polys[5U], host_curvature_rear);
   }
}
