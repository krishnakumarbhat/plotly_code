/*===================================================================================*\
* FILE: f360_concrete_wall_side.cpp
*====================================================================================
*Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
*Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* Contains function definitions for f360_concrete_wall_side.h
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#include "f360_concrete_wall_side.h"
#include <numeric>
#include <algorithm>
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION:  Concrete_Wall_Side_T::Concrete_Wall_Side_T()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
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
   * This is constructor for Concrete_Wall_Side_T class.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   Concrete_Wall_Side_T::Concrete_Wall_Side_T()   // ctor
   {
      long_validity_min = 0.0F;
      long_validity_max = 0.0F;
      fused_lat_posn_estimate = 0.0F;
      fused_lat_posn_confid = 0.0F;
      sensors_count = 0U;
      std::fill(cmn::begin(sensor_idxs), cmn::end(sensor_idxs), 0U);
      status = CW_NOT_VALID;
   }

   /*===========================================================================*\
   * FUNCTION:  Concrete_Wall_Side_T::Fuse_CWD_Side_Data()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const Concrete_Wall_Detector_Sensor_T(&concrete_wall_sensors)[MAX_NUMBER_OF_SENSORS]
   * const float32_t abs_host_speed
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
   * This is method for calculating CWD at left/right side of host.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Side_T::Fuse_CWD_Side_Data(const Concrete_Wall_Sensor_T(&concrete_wall_sensors)[MAX_NUMBER_OF_SENSORS], const float32_t abs_host_speed, const F360_Calibrations_T &calib)
   {
      Reset_Side_CWD();

      uint32_t cwd_relevant_sensors_count = 0U;
      float32_t cwd_relevant_sensors_xpos[cwd_max_num_sensors_per_side];
      float32_t cwd_all_side_sensors_xpos[cwd_max_num_sensors_per_side];

      float32_t cwd_relevant_lat_posn_est[cwd_max_num_sensors_per_side];
      float32_t cwd_relevant_lat_posn_confid[cwd_max_num_sensors_per_side];

      for (uint32_t sens_at_side_idx = 0U; sens_at_side_idx < sensors_count; sens_at_side_idx++)
      {
         const uint32_t sensor_idx = sensor_idxs[sens_at_side_idx];

         cwd_all_side_sensors_xpos[sens_at_side_idx] = concrete_wall_sensors[sensor_idx].Get_Sensor_Long_Posn();

         if (concrete_wall_sensors[sensor_idx].Is_CW_Lat_Posn_Estimate_Valid())
         {
            cwd_relevant_sensors_xpos[cwd_relevant_sensors_count] = concrete_wall_sensors[sensor_idx].Get_Sensor_Long_Posn();
            cwd_relevant_lat_posn_est[cwd_relevant_sensors_count] = concrete_wall_sensors[sensor_idx].Get_CW_Lat_Posn_Estimate();
            cwd_relevant_lat_posn_confid[cwd_relevant_sensors_count] = concrete_wall_sensors[sensor_idx].Get_CW_Lat_Posn_Confidence();
            cwd_relevant_sensors_count++;
         }
      }

      if (1U == cwd_relevant_sensors_count)  // Valid data from one sensor at side available
      {
         if (cwd_relevant_lat_posn_confid[0] >= calib.k_cwd_min_conf_to_confirm_cw_by_single_sensor)
         {
            status = CW_VALID;
            long_validity_min = F360_Min_Element(cwd_all_side_sensors_xpos, sensors_count) - calib.k_cwd_one_sensor_long_validity_offset;
            long_validity_max = F360_Max_Element(cwd_all_side_sensors_xpos, sensors_count) + calib.k_cwd_one_sensor_long_validity_offset;
            fused_lat_posn_confid = cwd_relevant_lat_posn_confid[0U] * calib.k_cwd_one_sensor_at_side_conf_factor;
            fused_lat_posn_estimate = cwd_relevant_lat_posn_est[0U];
         }
      }
      else if (2U <= cwd_relevant_sensors_count)   // Valid data available from at least two sensors at side.
      {
         const float32_t min_relevant_lat_posn_est = F360_Min_Element(cwd_relevant_lat_posn_est, cwd_relevant_sensors_count);
         const float32_t max_relevant_lat_posn_est = F360_Max_Element(cwd_relevant_lat_posn_est, cwd_relevant_sensors_count);
         const float32_t lat_dist_span = std::abs(max_relevant_lat_posn_est - min_relevant_lat_posn_est);

         // Depend on sensor cw estimate lateral distant span cw side fusion is perform in slightly differnt way
         if (lat_dist_span <= calib.k_cwd_side_dist_span_high_limit)
         {
            status = CW_VALID;
            // Derive valid long limit based on host speed
            const float32_t long_dist = F360_Linear_Equation_With_Saturation(
               abs_host_speed,
               calib.k_host_speed_for_min_long_validity_limit,
               calib.k_host_speed_for_max_long_validity_limit,
               calib.k_cwd_min_longitudinal_validity_limit,
               calib.k_cwd_max_longitudinal_validity_limit);

            long_validity_min = F360_Min_Element(cwd_relevant_sensors_xpos, cwd_relevant_sensors_count) - long_dist;
            long_validity_max = F360_Max_Element(cwd_relevant_sensors_xpos, cwd_relevant_sensors_count) + long_dist;

            float32_t numerator = 0.0F;
            float32_t denominator = 0.0F;
            for (uint32_t i = 0U; i < cwd_relevant_sensors_count; i++)
            {
               numerator += cwd_relevant_lat_posn_confid[i] * cwd_relevant_lat_posn_est[i];
               denominator += cwd_relevant_lat_posn_confid[i];
            }

            fused_lat_posn_estimate = numerator / denominator;

            if (calib.k_cwd_side_dist_span_low_limit < lat_dist_span)
            {
               fused_lat_posn_confid = F360_Min_Element(cwd_relevant_lat_posn_confid, cwd_relevant_sensors_count);
            }
            else
            {
               const float32_t * const begin = &cwd_relevant_lat_posn_confid[0U];
               const float32_t * const end = &cwd_relevant_lat_posn_confid[cwd_relevant_sensors_count];
               fused_lat_posn_confid = std::accumulate(begin, end, 0.0F) / static_cast<float32_t>(cwd_relevant_sensors_count);
            }

         }
         else
         {
            // Estimates from at least two sensors valid but they are laterally too far from each other.
            // If nearest of them has enough confidence it will be reported as cw_side lateral estimate
            std::pair<float32_t, float32_t> estimates_at_side[cwd_max_num_sensors_per_side];  // pair<confidence, lat_posn_est>

            for (uint32_t i = 0U; i < cwd_relevant_sensors_count; i++)
            {
               estimates_at_side[i].first = concrete_wall_sensors[sensor_idxs[i]].Get_CW_Lat_Posn_Confidence();
               estimates_at_side[i].second = concrete_wall_sensors[sensor_idxs[i]].Get_CW_Lat_Posn_Estimate();
            }

            const std::pair<float32_t, float32_t> nearest_estimate = *std::min_element(&estimates_at_side[0], &estimates_at_side[cwd_relevant_sensors_count],
               [](const std::pair<float32_t, float32_t> left, const std::pair<float32_t, float32_t> right) { return (std::abs(left.second) < std::abs(right.second)); });

            if (nearest_estimate.first > calib.k_cwd_min_conf_to_confirm_cw_by_single_sensor)
            {
               status = CW_VALID;
               long_validity_min = F360_Min_Element(cwd_all_side_sensors_xpos, sensors_count) - calib.k_cwd_one_sensor_long_validity_offset;
               long_validity_max = F360_Max_Element(cwd_all_side_sensors_xpos, sensors_count) + calib.k_cwd_one_sensor_long_validity_offset;
               fused_lat_posn_confid = nearest_estimate.first * calib.k_cwd_one_sensor_at_side_conf_factor;
               fused_lat_posn_estimate = nearest_estimate.second;
            }
            else
            {
               status = CW_NOT_VALID;
               long_validity_min = 0.0F;
               long_validity_max = 0.0F;
               fused_lat_posn_confid = 0.0F;
               fused_lat_posn_estimate = 0.0F;
            }
         }
      }
      else
      {
         // Do nothing. MISRA rule.
      }
   }

   /*===========================================================================*\
   * FUNCTION:  Concrete_Wall_Side_T::Append_Sensor_Idx()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * const uint32_t sensor_idx
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
   * This is method for assigning sensor Id to one of two sides: left/right.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Side_T::Append_Sensor_Idx(const uint32_t sensor_idx)
   {
      sensor_idxs[sensors_count] = sensor_idx;
      sensors_count++;
   }

   /*===========================================================================*\
   * FUNCTION:  Concrete_Wall_Side_T::Reset_Side_CWD()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * None
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
   * This method resets concrete wall estimate.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Concrete_Wall_Side_T::Reset_Side_CWD()
   {
      long_validity_min = 0.0F;
      long_validity_max = 0.0F;
      fused_lat_posn_estimate = 0.0F;
      fused_lat_posn_confid = 0.0F;
      status = CW_NOT_VALID;
   }

   /*===========================================================================*\
   * FUNCTION:  Concrete_Wall_Side_T::Map_CWD_Side_Data_To_Static_Env_Poly()
   *===========================================================================
   * RETURN VALUE:
   * void
   *
   * PARAMETERS:
   * Static_Env_Poly_T &stat_env_poly
   * const float32_t host_curvature_rear
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This method maps CWD data to Static_Env_Poly_T.
   *
   \*===========================================================================*/
   void Concrete_Wall_Side_T::Map_CWD_Side_Data_To_Static_Env_Poly(Static_Env_Poly_T &stat_env_poly, const float32_t host_curvature_rear) const
   {
      stat_env_poly.age = (CW_VALID == status) ? 1U : 0U;
      stat_env_poly.confidence = fused_lat_posn_confid;
      stat_env_poly.lower_limit = long_validity_min;
      stat_env_poly.upper_limit = long_validity_max;
      stat_env_poly.p0 = fused_lat_posn_estimate;
      stat_env_poly.p1 = 0.0F;
      stat_env_poly.p2 = 0.5F * host_curvature_rear;
      stat_env_poly.status = (CW_VALID == status) ? F360_STATIC_ENV_POLY_STATUS_UPDATED : F360_STATIC_ENV_POLY_STATUS_INVALID;
      stat_env_poly.poly_type = F360_STATIC_ENV_POLY_TYPE_CWD;
   }
}
