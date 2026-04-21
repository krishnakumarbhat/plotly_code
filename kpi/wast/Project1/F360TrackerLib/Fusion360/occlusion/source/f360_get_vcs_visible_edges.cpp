/*===================================================================================*\
* FILE:  f360_get_vcs_visible_edges.cpp
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains definition of Get_VCS_Visible_Edges() function.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include "f360_get_vcs_visible_edges.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_math_func.h"
#include "f360_calc_point_scs_position.h"

namespace f360_variant_A
{

   /*===========================================================================*\
   * FUNCTION: Prepare_Edge()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const Point& pt_begin             - Begining point of edge
   * const Point& pt_end               - Ending point of edge
   * const F360_Object_Track_T& object - Object that is analysed
   * VCS_Edge_T& edge                  - Edge VCS position, calculated coordinates are writen into this structure
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
   * Function calculates coordinates of given track visible edges based on 
   * object bounding box center.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Prepare_Edge(
      const Point& pt_begin,
      const Point& pt_end,
      const F360_Object_Track_T& object,
      VCS_Edge_T& edge
   )
   {
      edge.f_valid = true;
      Convert_TCS_Posn_To_VCS_Posn(
         pt_begin.x,
         pt_begin.y,
         object.bbox.Get_Center().x,
         object.bbox.Get_Center().y,
         object.bbox.Get_Orientation(),
         edge.point_begin.x,
         edge.point_begin.y
      );

      Convert_TCS_Posn_To_VCS_Posn(
         pt_end.x,
         pt_end.y,
         object.bbox.Get_Center().x,
         object.bbox.Get_Center().y,
         object.bbox.Get_Orientation(),
         edge.point_end.x,
         edge.point_end.y
      );
   }

   /*===========================================================================*\
   * FUNCTION: Select_Object_Edges()
   *===========================================================================
   * RETURN VALUE:
   * VCS_Track_Visible_Edges_T vcs_visible_edges - Visible edges of a track.
   *
   * PARAMETERS:
   * const Point& rear_left            - Object rear left corner
   * const Point& front_left           - Object front left corner
   * const Point& front_right          - Object front right corner
   * const Point& rear_right           - Object rear right corner
   * const F360_Object_Track_T& object - Analysed object
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
   * Function selects coordinates of object visible edges based on reference point.
   *
   * PRECONDITIONS:
   * None.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static VCS_Track_Visible_Edges_T Select_Object_Edges(
      const Point& rear_left,
      const Point& front_left,
      const Point& front_right,
      const Point& rear_right,
      const F360_Object_Track_T& object)
   {
      VCS_Track_Visible_Edges_T vcs_visible_edges;
      switch (object.reference_point)
      {
      case F360_REFERENCE_POINT_REAR_LEFT:
      {
         Prepare_Edge(rear_left, rear_right, object, vcs_visible_edges.first_edge);
         Prepare_Edge(rear_left, front_left, object, vcs_visible_edges.second_edge);
         break;
      }
      case F360_REFERENCE_POINT_REAR:
      {
         Prepare_Edge(rear_left, rear_right, object, vcs_visible_edges.first_edge);
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      case F360_REFERENCE_POINT_REAR_RIGHT:
      {
         Prepare_Edge(rear_left, rear_right, object, vcs_visible_edges.first_edge);
         Prepare_Edge(rear_right, front_right, object, vcs_visible_edges.second_edge);
         break;
      }
      case F360_REFERENCE_POINT_LEFT:
      {
         Prepare_Edge(rear_left, front_left, object, vcs_visible_edges.first_edge);
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      case F360_REFERENCE_POINT_CENTER:
      {
         vcs_visible_edges.first_edge.f_valid = false;
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      case F360_REFERENCE_POINT_RIGHT:
      {
         Prepare_Edge(rear_right, front_right, object, vcs_visible_edges.first_edge);
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      case F360_REFERENCE_POINT_FRONT_LEFT:
      {
         Prepare_Edge(front_left, front_right, object, vcs_visible_edges.first_edge);
         Prepare_Edge(rear_left, front_left, object, vcs_visible_edges.second_edge);
         break;
      }
      case F360_REFERENCE_POINT_FRONT:
      {
         Prepare_Edge(front_left, front_right, object, vcs_visible_edges.first_edge);
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      case F360_REFERENCE_POINT_FRONT_RIGHT:
      {
         Prepare_Edge(front_left, front_right, object, vcs_visible_edges.first_edge);
         Prepare_Edge(rear_right, front_right, object, vcs_visible_edges.second_edge);
         break;
      }
      default:
      {
         vcs_visible_edges.first_edge.f_valid = false;
         vcs_visible_edges.second_edge.f_valid = false;
         break;
      }
      }
      return vcs_visible_edges;
   }

   /*===========================================================================*\
   * FUNCTION: Get_VCS_Visible_Edges()
   *===========================================================================
   * RETURN VALUE:
   * VCS_Track_Visible_Edges_T vcs_visible_edges - Given track visible edges in VCS.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object - Analysed object
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
   * Function, basing on visible edges of track, calculates its coordinates in VCS.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   VCS_Track_Visible_Edges_T Get_VCS_Visible_Edges(const F360_Object_Track_T& object)
   {
      const float32_t half_length = 0.5F * object.bbox.Get_Length();
      const float32_t half_width = 0.5F * object.bbox.Get_Width();

      Point rear_left;
      rear_left.x = -half_length;
      rear_left.y = -half_width;

      Point front_left;
      front_left.x = half_length;
      front_left.y = -half_width;

      Point front_right;
      front_right.x = half_length;
      front_right.y = half_width;

      Point rear_right;
      rear_right.x = -half_length;
      rear_right.y = half_width;

      const VCS_Track_Visible_Edges_T vcs_visible_edges = Select_Object_Edges(rear_left, front_left, front_right, rear_right, object);

      return vcs_visible_edges;
   }
}

