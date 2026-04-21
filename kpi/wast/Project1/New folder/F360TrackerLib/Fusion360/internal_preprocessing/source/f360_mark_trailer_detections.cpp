/*===========================================================================*\
* FILE: f360_mark_trailer_detections.cpp
*============================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains functionality for marking the detections on the trailer.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "f360_math.h"
#include <algorithm>
#include "f360_mark_trailer_detections.h"

namespace f360_variant_A
{
   static Point Cal_Center(const Point point_tow_hitch, const BoundingBox &bbox);

   static Point Cal_Center(const Point point_tow_hitch, const BoundingBox &bbox)
   {
      const Point ref_tcs = {0.5F * bbox.Get_Length(), 0.0F};

      Point new_center{ -ref_tcs.x , -ref_tcs.y };
      new_center.Rotate_About_Origin(bbox.Get_Orientation());
      new_center.Translate(point_tow_hitch.x, point_tow_hitch.y);

      return new_center;

   }

   void Detect_Det_On_Trailer(const F360_Host_T &f360_host,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
      const Trailer_Detector_Flt_Fus_Output &trailer,
      F360_Detection_Props_T (&det_Props)[MAX_NUMBER_OF_DETECTIONS])
   {
      const uint32_t dets_nr = raw_detections.number_of_valid_detections;

      if((0U < dets_nr) && (TRAILER_PRESENCE_STATE_DETECTED == trailer.trailer_presence) && (0.0F < trailer.trailer_length) && (0.0F < trailer.trailer_width))
      {
         BoundingBox trailer_bbox;

         const float32_t distance_rear_axle_to_tow_hitch = 1.2F;    // distance between rear axle to to hitch (m), TODO: moved into calbration files 
         const Point point_tow_hitch = {-(f360_host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch), 0.0F};
         trailer_bbox.Set_Length(trailer.trailer_length);
         trailer_bbox.Set_Width(trailer.trailer_width);
         trailer_bbox.Set_Orientation(-trailer.trailer_angle);  //add minus here as the trailer angle is defined opposite to the VCS
         trailer_bbox.Set_Center(Cal_Center(point_tow_hitch, trailer_bbox));

         uint32_t det_idx = static_cast<uint32_t>(raw_detections.vcslong_det_idx_min);

         const float32_t extend_length = 1.0F;  // extend length for the trailer bounding box
         const float32_t extend_width = 0.4F;   // extend width for the trailer bounding box
         trailer_bbox.Extend_Boundaries(extend_width, extend_width, extend_length, extend_length);

         for (uint32_t i = 0U; i < dets_nr; i++)
         {

            if (i > 0U)
            {
               det_idx = static_cast<uint32_t>(raw_detections.detections[det_idx].processed.next_sorted_idx);
            }

            F360_Detection_Props_T& det_prop = det_Props[det_idx];
            const rspp_variant_A::RSPP_Detection_T& raw_det = raw_detections.detections[det_idx];

            if(-(f360_host.dist_rear_axle_to_vcs_m + distance_rear_axle_to_tow_hitch) + extend_length < raw_det.processed.vcs_position_x)
            {
               break;
            }

            if (trailer_bbox.Contains(det_prop.vcs_position))
            {
               det_prop.f_ok_to_use = false;
               det_prop.f_det_on_trailer = true;
            }
         
         }
      }
   }
}

