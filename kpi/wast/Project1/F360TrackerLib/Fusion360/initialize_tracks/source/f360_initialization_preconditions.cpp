/*===========================================================================*\
* FILE: f360_initialization_preconditions.cpp
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
*   This file contains function definition for Initialization_Preconditions.
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
#include "f360_nees_cfmi_helpers.h"
#include "f360_initialization_preconditions.h"
#include "f360_is_cluster_affected_by_angle_jumps.h"
#include "f360_math_func.h"

namespace f360_variant_A
{
   /*===========================================================================*\
    * FUNCTION: Initialization_Preconditions()
    *===========================================================================
    * RETURN VALUE:
    * bool
    *
    * PARAMETERS:
    * const F360_Calibrations_T& calibrations
    * const F360_Cluster_T& cluster_to_init
    * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
    * const F360_Detection_Props_T& (&det_props)[MAX_NUMBER_OF_DETECTIONS]
    * const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
    * const F360_Detection_Hist_T& detection_hist
    * const F360_Tracker_Info_T& tracker_info
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
    * Check if cluster is valid for initialization
    *
    * PRECONDITIONS:
    * All the Pointers should Point to valid structures.
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   bool Initialization_Preconditions(
      const F360_Calibrations_T& calibrations,
      const F360_Cluster_T& cluster_to_init,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detection_list,
      const F360_Detection_Hist_T& detection_hist,
      const F360_Tracker_Info_T& tracker_info)
   {
      const int16_t n_all_dets = cluster_to_init.ndets + cluster_to_init.num_old_dets;
      const int16_t n_mov_dets = cluster_to_init.num_types_of_dets[0];
      const int16_t n_amb_dets = cluster_to_init.num_types_of_dets[1];
      const int16_t n_current_dets = cluster_to_init.ndets;

      // Basic preconditions.
      int32_t min_mov_dets;
      if (Check_If_Cluster_Has_Any_Det_In_All_Looks(cluster_to_init, raw_detection_list, sensors, detection_hist))
      {
         min_mov_dets = calibrations.k_init_trk_preconditions_min_mov_dets_sr;
      }
      else
      {
         min_mov_dets = calibrations.k_init_trk_preconditions_min_mov_dets_lr;
      }

      bool f_preconditions_valid = (cluster_to_init.f_dealiased) &&
         (n_current_dets >= calibrations.k_init_trk_preconditions_min_current_dets) &&
         ((n_all_dets >= calibrations.k_init_trk_preconditions_min_mov_amb_dets) ||
         (n_mov_dets >= min_mov_dets) ||
         (n_amb_dets >= calibrations.k_init_trk_preconditions_min_amb_dets)) &&
         (false == Is_Cluster_Affected_By_Angle_Jumps(cluster_to_init, det_props, detection_hist, calibrations));

      // Extended NEES CFMI preconditions (if enabled).
      if ((f_preconditions_valid) && (calibrations.f_nees_cfmi_extended_preconditions))
      {
         f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);
      }

      return f_preconditions_valid;
   }
}
