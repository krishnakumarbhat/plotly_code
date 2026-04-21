/*===================================================================================*\
* FILE:  f360_get_scs_visible_edges.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of function used to convert VCS visible edges to SCS visible edges.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_get_scs_visible_edges.h"
#include "f360_calc_point_scs_position.h"
#include <algorithm>

namespace f360_variant_A
{

   static SCS_Edge_T Rotate_VCS_Edge_To_SCS(
      const VCS_Edge_T& vcs_edge,
      const Sensor_Mounting_Position& mounting_pos);

   /*===========================================================================*\
   * FUNCTION: Get_SCS_Visible_Edges()
   *===========================================================================
   * RETURN VALUE:
   * SCS_Track_Visible_Edges_T track_edges - coordinates of track visible edges.
   *
   * PARAMETERS:
   * const VCS_Track_Visible_Edges_T& vcs_visible_edges - visible edges coordinates in VCS
   * const Sensor_Mounting_Position& mounting_pos - sensor mounting position in VCS
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
   * This function converts visible edges coordinates given in VCS to SCS.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   SCS_Track_Visible_Edges_T Get_SCS_Visible_Edges(
      const VCS_Track_Visible_Edges_T& vcs_visible_edges,
      const Sensor_Mounting_Position& mounting_pos
   )
   {
      SCS_Track_Visible_Edges_T scs_visible_edges{};

      scs_visible_edges.first_edge = Rotate_VCS_Edge_To_SCS(vcs_visible_edges.first_edge, mounting_pos);

      scs_visible_edges.second_edge = Rotate_VCS_Edge_To_SCS(vcs_visible_edges.second_edge, mounting_pos);

      return scs_visible_edges;
   }

   /*===========================================================================*\
   * FUNCTION: Rotate_VCS_Edge_To_SCS()
   *===========================================================================
   * RETURN VALUE:
   * SCS_Edge_T& edge - edge rotated to SCS
   *
   * PARAMETERS:
   * const VCS_Edge_T& vcs_edge - single edge coordinates, given in VCS
   * const Sensor_Mounting_Position& mounting_pos - sensor mounting position, given in VCS
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
   * This function converts single edge VCS position to SCS.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static SCS_Edge_T Rotate_VCS_Edge_To_SCS(
      const VCS_Edge_T& vcs_edge,
      const Sensor_Mounting_Position& mounting_pos)
   {
      SCS_Edge_T scs_edge{};
      scs_edge.f_valid = vcs_edge.f_valid;
      if (scs_edge.f_valid)
      {
         scs_edge.point_begin = Calc_Point_SCS_Position(vcs_edge.point_begin.x, vcs_edge.point_begin.y, mounting_pos);
         scs_edge.point_end = Calc_Point_SCS_Position(vcs_edge.point_end.x, vcs_edge.point_end.y, mounting_pos);

         if (scs_edge.point_end.polar.azimuth < scs_edge.point_begin.polar.azimuth)
         {
            std::swap(scs_edge.point_begin, scs_edge.point_end);
         }
      }

      return scs_edge;
   }
}

