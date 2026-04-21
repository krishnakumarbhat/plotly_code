/*===========================================================================*\
* FILE: f360_is_cluster_affected_by_angle_jumps.cpp
*============================================================================
* Copyright - 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition for Is_Cluster_Affected_By_Angle_Jumps.
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [30-Mar-2018]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_preconditions.h"
#include "f360_is_cluster_affected_by_angle_jumps.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Cluster_Affected_By_Angle_Jumps()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Cluster_T& cluster,
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
   * const F360_Detection_Hist_T& detection_hist,
   * const F360_Calibrations_T& calibrations
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
   bool Is_Cluster_Affected_By_Angle_Jumps(
      const F360_Cluster_T& cluster,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& detection_hist,
      const F360_Calibrations_T& calibrations
   )
   {
      int16_t num_angle_jumps = 0;

      for (int16_t idx = 0; idx < cluster.ndets; idx++)
      {
         if (det_props[cluster.detids[idx] - 1].f_potential_angle_jump)
         {
            num_angle_jumps++;
         }
      }

      for (int16_t idx = 0; idx < cluster.num_old_dets; idx++)
      {
         if (detection_hist.det_data[cluster.old_det_idx[idx]].f_potential_angle_jump)
         {
            num_angle_jumps++;
         }
      }

      bool f_cluster_affected;
      if (num_angle_jumps > 0)
      {
         const int16_t num_all_dets = cluster.ndets + cluster.num_old_dets;
         const int16_t num_correct_dets = num_all_dets - num_angle_jumps;

         f_cluster_affected = !((num_correct_dets >= calibrations.k_obj_init_min_number_of_correct_dets) &&
            (static_cast<float32_t>(num_correct_dets) > (static_cast<float32_t>(num_all_dets) * calibrations.k_obj_init_min_correct_dets_ratio)));
      }
      else
      {
         f_cluster_affected = false;
      }

      return f_cluster_affected;
   }
}
