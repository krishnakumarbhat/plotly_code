/*===========================================================================*\
* FILE: f360_mark_out_det_pairs.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains Mark_Out_Det_Pairs() function definition
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_mark_out_det_pairs.h"
#include "f360_math_func.h"
#include "f360_get_wall_time.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Mark_Out_Det_Pairs()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T (&dets)[MAX_NUMBER_OF_DETECTIONS]
   * const int32_t num_valid_dets
   * F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_TRKR_TIMING_INFO_T &timing_info
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
   * This function marks out unassociated detections that can be matched with another detection in the same range and range rate bin.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Mark_Out_Det_Pairs(
      const rspp_variant_A::RSPP_Detection_T (&dets)[MAX_NUMBER_OF_DETECTIONS],
      const int32_t num_valid_dets,
      F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_TRKR_TIMING_INFO_T &timing_info)
   {
      const float32_t start_time = get_wall_time();
      // The detection pairs are super resolution detection that will be received one after the other and their range and range rates are exactly equal
      for (int32_t det_idx = 0; det_idx < num_valid_dets - 1; det_idx++)
      {
         const rspp_variant_A::RSPP_Detection_T &current_detection = dets[det_idx];
         F360_Detection_Props_T &current_detection_prop = det_props[det_idx];
         const rspp_variant_A::RSPP_Detection_T &next_detection = dets[det_idx + 1];
         F360_Detection_Props_T &next_detection_prop = det_props[det_idx + 1];
         Mark_Out_Det_Pair(current_detection, next_detection, current_detection_prop, next_detection_prop);

      }
      timing_info.mark_out_det_pairs = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Mark_Out_Det_Pair()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &current_detection
   * const rspp_variant_A::RSPP_Detection_T &next_detection
   * F360_Detection_Props_T &current_detection_prop
   * F360_Detection_Props_T &next_detection_prop
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
   * This function marks two detections as pair if they pass the range and range rate conditions.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Mark_Out_Det_Pair(
      const rspp_variant_A::RSPP_Detection_T &current_detection,
      const rspp_variant_A::RSPP_Detection_T &next_detection,
      F360_Detection_Props_T &current_detection_prop,
      F360_Detection_Props_T &next_detection_prop)
   {
      if ((std::abs(current_detection.raw.range - next_detection.raw.range) < F360_EPSILON) &&
         (std::abs(current_detection.raw.range_rate - next_detection.raw.range_rate) < F360_EPSILON) &&
         (current_detection.raw.sensor_id == next_detection.raw.sensor_id))
      {
         // Don't let these detections start new tracks.
         current_detection_prop.f_det_pair = (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == current_detection.processed.motion_status);
         next_detection_prop.f_det_pair = (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == next_detection.processed.motion_status);
      }
   }
}
