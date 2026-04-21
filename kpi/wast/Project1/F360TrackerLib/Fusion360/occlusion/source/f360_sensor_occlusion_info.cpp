/*===================================================================================*\
* FILE:  f360_sensor_occlusion_info.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Sensor_Occlusion_Info_T class methods.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_sensor_occlusion_info.h"
#include "f360_math_func.h"
#include "f360_calc_point_scs_position.h"
#include "f360_get_scs_visible_edges.h"
#include "f360_sensor_occlusion_info_helpers.h"

namespace f360_variant_A
{
   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Sensor_Occlusion_Info_T
   *
   * Description    Constructor of Sensor_Occlusion_Info_T.
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
   Sensor_Occlusion_Info_T::Sensor_Occlusion_Info_T() :
      sectors_width(),
      range_uncertainty(),
      sectors(),
      f_valid(false),
      mounting_position(),
      fov()
   {
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::~Sensor_Occlusion_Info_T
   *
   * Description    Destructor of Sensor_Occlusion_Info_T.
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
   Sensor_Occlusion_Info_T::~Sensor_Occlusion_Info_T()
   {
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Reset
   *
   * Description    Method used to reset occluion information.
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
   void Sensor_Occlusion_Info_T::Reset()
   {
      f_valid = false;
      fov.Reset();

      sectors_width = 0.0F;

      Fill_Sectors_Ranges(0.0F);
   }


   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Update_Calibs
   *
   * Description    Method used to update sensor calibrations
   *
   * Parameters
   * const F360_Radar_Sensor_T& sensors - sensor calibration
   * const F360_Radar_Sensor_T& sensor - sensor properties
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
   void Sensor_Occlusion_Info_T::Update_Calibs(
      const F360_Radar_Sensor_T& sensor,
      const float32_t range_uncertainty_th)
   {
      f_valid = sensor.variable.is_valid;
      if (f_valid)
      {
         fov.Update_Calibrations(sensor);
         if (fov.Get_Azimuth_Spread() < 0.0F)
         {
            f_valid = false;
         }
         else
         {
            Fill_Sectors_Ranges(fov.Get_Max_Range());

            sectors_width = (fov.Get_Azimuth_Spread()) / static_cast<float32_t>(occlusion_num_sectors);

            mounting_position.vcs_position = Point(sensor.constant.mounting_position.vcs_position.longitudinal, sensor.constant.mounting_position.vcs_position.lateral);
            mounting_position.sin_vcs_borseight_azimuth_angle = F360_Sinf(sensor.constant.mounting_position.vcs_boresight_azimuth_angle);
            mounting_position.cos_vcs_borseight_azimuth_angle = F360_Cosf(sensor.constant.mounting_position.vcs_boresight_azimuth_angle);

            range_uncertainty = range_uncertainty_th;
         }
      }
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Update_Occlusion
   *
   * Description    Method used to update occlusion information with edge given in SCS coordinates
   *                Provided edge is sampled on fixed azimuths. At first starting and ending sectors
   *                are calculated - it can be done because of fixed sector width. Later each sector
   *                determines intersection of its bisector and analysed edge. If they intersect,
   *                range with lower value is picked and held within sector.
   *
   * Parameters     const SCS_Edge_T& edge - edge that should be used to update occlusion information, in SCS
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   Edge has to be valid.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void Sensor_Occlusion_Info_T::Update_Occlusion(
      const SCS_Edge_T& edge)
   {
      const int32_t sector_start_idx = Calc_Starting_Sector_IDx(edge.point_begin.polar.azimuth, fov.Get_Min_Az(), sectors_width);
      const int32_t sector_end_idx = Calc_Ending_Sector_IDx(edge.point_end.polar.azimuth, fov.Get_Min_Az(), sectors_width);

      const float32_t first_line_begin[2] = { edge.point_begin.cart.x ,edge.point_begin.cart.y };

      const float32_t first_line_end[2] = { edge.point_end.cart.x ,edge.point_end.cart.y };

      const float32_t second_line_begin[2] = { 0.0F, 0.0F };

      for (int32_t sector_idx = sector_start_idx; sector_idx <= sector_end_idx; sector_idx++)
      {
         const float32_t fsector_idx = static_cast<float32_t>(sector_idx);
         const float32_t sect_az = fov.Get_Min_Az() + (fsector_idx * sectors_width) + (sectors_width / 2.0F);

         float32_t second_line_end[2];
         second_line_end[0] = fov.Get_Max_Range() * F360_Cosf(sect_az); // TODO: DFU-913
         second_line_end[1] = fov.Get_Max_Range() * F360_Sinf(sect_az);

         sectors[sector_idx].Update_Range_With_Intersection_Of_Two_Lines(first_line_begin, first_line_end, second_line_begin, second_line_end);
      }

   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Determine_Occlusion_Status
   *
   * Description    Method used to determine given point occlusion status
   *
   * Parameters     const SCS_Position_T& scs_polar_position - given point position, in SCS
   *
   * Returns        Occlusion_Status_T - enum indicating point occlusion status
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Occlusion_Status_T Sensor_Occlusion_Info_T::Determine_Occlusion_Status(
      const float32_t vcs_long_posn,
      const float32_t vcs_lat_posn) const
   {
      Occlusion_Status_T occl_status = Occlusion_Status_T::OCCLUSION_STATUS_UNDEFINED;
      if (f_valid)
      {
         const SCS_Position_T scs_pos = Calc_Point_SCS_Position(vcs_long_posn, vcs_lat_posn, mounting_position);
         if (fov.Is_Point_In_FOV(scs_pos.polar.azimuth, scs_pos.polar.range))
         {
            occl_status = Determine_Occlusion_Status_Of_Point_In_FOV(scs_pos.polar);
         }
      }
      return occl_status;
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Is_In_FOV
   *
   * Description    Method used to determine whether visible edges of object are in sensor FOV.
   *
   * Parameters     const SCS_Track_Visible_Edges_T& visible_edges - object visible edges, in SCS
   *
   * Returns        bool - flag indicating whether any of object corners is seen by sensor
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   bool Sensor_Occlusion_Info_T::Is_In_FOV(
      const SCS_Edge_T& edge) const
   {
      const bool f_valid_edge_is_seen = 
         (fov.Is_Point_In_FOV(edge.point_begin.polar.azimuth, edge.point_begin.polar.range) ||
            fov.Is_Point_In_FOV(edge.point_end.polar.azimuth, edge.point_end.polar.range));

      return f_valid_edge_is_seen;
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Fill_Sectors_Ranges
   *
   * Description    Method used to fill ranges of sectors with common value
   *
   * Parameters     const float32_t range - range that should fill sectors
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
   void Sensor_Occlusion_Info_T::Fill_Sectors_Ranges(
      const float32_t range)
   {
      for (uint32_t i = 0U; i < occlusion_num_sectors; i++)
      {
         sectors[i].Set_Range(range);
      }
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Update_Occlusion
   *
   * Description    Method used to update occluded sectors information with visible edges given in VCS
   *
   * Parameters     const VCS_Track_Visible_Edges_T& vcs_visible_edges - set of two visible edges in VCS coordinates
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
   void Sensor_Occlusion_Info_T::Update_Occlusion(
      const VCS_Track_Visible_Edges_T& vcs_visible_edges)
   {
      if (f_valid)
      {
         const SCS_Track_Visible_Edges_T scs_visible_edges = Get_SCS_Visible_Edges(vcs_visible_edges, mounting_position);

         if (scs_visible_edges.first_edge.f_valid && 
            Is_In_FOV(scs_visible_edges.first_edge))
         {
            Update_Occlusion(scs_visible_edges.first_edge);
         }

         if (scs_visible_edges.second_edge.f_valid &&
            Is_In_FOV(scs_visible_edges.second_edge))
         {
            Update_Occlusion(scs_visible_edges.second_edge);
         }
      }
   }

   /*=========================================================================
   * Method         Sensor_Occlusion_Info_T::Determine_Occlusion_Status_Of_Point_In_FOV
   *
   * Description    Detailed implementation of method used to determine given point occlusion status
   *
   * Parameters     const SCS_Position_T& scs_polar_position - given point position, in SCS
   *
   * Returns        Occlusion_Status_T - enum indicating point occlusion status
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   Occlusion_Status_T Sensor_Occlusion_Info_T::Determine_Occlusion_Status_Of_Point_In_FOV(
      const SCS_Polar_Position_T scs_polar_position) const
   {
      const int32_t sector_idx_start = Calc_Starting_Sector_IDx(scs_polar_position.azimuth, fov.Get_Min_Az(), sectors_width);
      const int32_t sector_idx_end = Calc_Ending_Sector_IDx(scs_polar_position.azimuth, fov.Get_Min_Az(), sectors_width);

      const float32_t min_rng = std::min(sectors[sector_idx_start].Get_Range(), sectors[sector_idx_end].Get_Range());
      const float32_t max_rng = std::max(sectors[sector_idx_start].Get_Range(), sectors[sector_idx_end].Get_Range());

      const float32_t tested_range = (scs_polar_position.range - range_uncertainty);

      Occlusion_Status_T occl_status;
      if (max_rng <= tested_range)
      {
         occl_status = Occlusion_Status_T::OCCLUSION_STATUS_OCCLUDED;
      }
      else if (min_rng < tested_range)
      {
         occl_status = Occlusion_Status_T::OCCLUSION_STATUS_ON_EDGE;
      }
      else
      {
         occl_status = Occlusion_Status_T::OCCLUSION_STATUS_VISIBLE;
      }

      return occl_status;
   }
}
