/*===================================================================================*\
* FILE: f360_calc_obj_height.cpp
*====================================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential � Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*
* 
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_calc_obj_height.h"
#include "f360_reuse.h"
#include "f360_math.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION:
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_List_T& detection_list
   * const F360_Calibrations_T& calib
   * F360_Object_Track_T& object_track
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
   * 
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Calc_Obj_Height(
      const rspp_variant_A::RSPP_Detection_T(&dets)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Calibrations_T& calib,
      F360_Object_Track_T& object_track)
   {
      uint32_t n_valid_dets = 0U;
      float32_t det_heights_sum = 0.0F;
      if ((object_track.f_moving == true))
      {
         for (uint32_t i = 0U; i < object_track.ndets; i++)
         {
            const uint32_t det_idx = object_track.detids[i] - 1U;
            if ((dets[det_idx].raw.confid_azimuth != rspp_variant_A::RSPP_CONF_AZIMUTH_LOW) && (dets[det_idx].processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING))
            {
               /*
               In vehicle coordinate system the detection height above the ground have negative height value, so the absolute value is taken.
               Ground-bounced detections, which have positive height value, are compensated and it is fine to take the value as it is.
               See Polarion's Software Detailed Design documentation for detailed description.
               */
               det_heights_sum += std::abs(dets[det_idx].processed.vcs_position_z);
               n_valid_dets++;
            }
         }
      }
   
      // Update the number of historic detections with detection information from current scan. If there are no valid current detections, let it decrease with the forgetting factor.
      const float32_t historic_ndets = calib.ud_mov_forgetting_factor * object_track.ud_mov_historic_ndets + static_cast<float32_t>(n_valid_dets);
      const float32_t historic_heights_sum = calib.ud_mov_forgetting_factor * object_track.ud_mov_historic_ndets * object_track.otg_height + det_heights_sum;
   
      // Limit the effect of historical information.
      object_track.ud_mov_historic_ndets = std::min(historic_ndets, calib.ud_mov_max_n_historic_dets);
   
      const float32_t heights_mean = (historic_ndets < F360_EPSILON) ? 0.0F : (historic_heights_sum / historic_ndets);
      object_track.otg_height = (n_valid_dets == 0U) ? calib.ud_mov_forgetting_factor * heights_mean : heights_mean;
   }
}
