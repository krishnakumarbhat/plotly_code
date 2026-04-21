/*===================================================================================*\
* FILE: f360_merge_bbox_overlap_test.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Merge_Bbox_Overlap_Test() function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
* None.
*
\*===================================================================================*/

#include "f360_merge_bbox_overlap_test.h"
#include "f360_bounding_box.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Merge_Bbox_Overlap_Test()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Object_Track_T & first_object,
   * const F360_Object_Track_T & second_object,
   * const float32_t half_length_ext,
   * const float32_t half_width_ext
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
   * Function checks if extended bounding boxes for objects overlap or not.
   * For now, use an approximate scheme in which it is checked to see whether any of the corners
   * of one object are inside the bounding box of the other(and vice versa).
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Merge_Bbox_Overlap_Test(
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object,
      const float32_t half_length_ext,
      const float32_t half_width_ext)
   {
      BoundingBox first_bbox{ first_object.bbox };
      BoundingBox second_bbox{ second_object.bbox };

      first_bbox.Extend_Boundaries(half_width_ext, half_width_ext, half_length_ext, half_length_ext);
      second_bbox.Extend_Boundaries(half_width_ext, half_width_ext, half_length_ext, half_length_ext);

      return first_bbox.Collides(second_bbox);
   }
}
