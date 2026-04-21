/*===================================================================================*\
* FILE: f360_mark_detections_with_neighbors.cpp
*====================================================================================
* Copyright © 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains functions definitions of Mark_Detections_With_Neighbors()
*   and its support functions
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
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/
#include "f360_mark_detections_with_neighbors.h"
#include "f360_calc_nearest_assoc_det_distance_sq.h"

namespace f360_variant_A
{

   /*===========================================================================*\
   * Function definitions
   \*===========================================================================*/
   /*===========================================================================*\
   * FUNCTION: Mark_Detections_With_Neighbors
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track - reference to one object
   * const F360_Calibrations_T & calibrations - reference to calibration data structure
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS] - reference to detection property data structure
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function which operaters on track's associated detections. Detection which
   * has any other associated detections at close distance (below 3.0m distance
   * threshold) is marked by changing
   * F360_Detection_Props_T.any_other_assoc_det_close status. The
   * F360_Detection_Props_T.dist_to_closest_assoc_det_sq is also changed
   * to equal the square of the closest distance.
   *
   * PRECONDITIONS:
   * F360_Detection_Props_T.dist_to_closest_assoc_det_sq must be cleared for
   * the detections so that it contains a very large value.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Mark_Detections_With_Neighbors(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calibrations,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      uint32_t const assoc_dets_count = object_track.ndets;

      if (assoc_dets_count > 1U)
      {
         // Compute the smallest distance to other associated detections
         Calc_Nearest_Assoc_Det_Distance_Sq(object_track, detection_props);

         /* Check if nearest associated detection is close enough to be classified
         as a neighbor and set the any_other_assoc_det_close property of the
         detections to YES or NO accordingly */
         Check_If_Detection_Has_Neighbor(object_track, calibrations, detection_props);
      }
      else if (assoc_dets_count == 1U)
      {
         /* If only one detection then we can not compute closest distance and determine
         any neigbors. Set any_other_assoc_det_close property of detection to UNDETERMINED */
         uint32_t const curr_det_index = object_track.detids[0U] - 1U;
         F360_Detection_Props_T * const curr_det = &detection_props[curr_det_index];
         curr_det->any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
      }
      else
      {
         // No associated dets - do nothing
      }
   }


   /*===========================================================================*\
   * FUNCTION: Check_If_Detection_Has_Neighbor
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Object_Track_T const & object_track - reference to one object
   * const F360_Calibrations_T & calibrations - reference to calibration data structure
   * F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS] - reference to detection property data structure
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function checks all detections that are associated to the objeckt_track.
   * Detections which have an other detection close to it are marked as having
   * a neighbour by modifying the any_other_assoc_det_close property of the detection.
   *
   * PRECONDITIONS:
   * The distance to the closest other detection (dist_to_closest_assoc_det_sq
   * in detection_props) has to be pre-computed.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Check_If_Detection_Has_Neighbor(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calibrations,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS])
      {
      uint32_t const assoc_dets_count = object_track.ndets;

      // Loop over all associated detections and set the any_other_assoc_det_close property
      for (uint32_t curr_det_slot_index = 0U; curr_det_slot_index < assoc_dets_count; curr_det_slot_index++)
      {
         const float32_t has_neighbor_distance_threshold_sq = calibrations.max_dist_for_neighbor_detections * calibrations.max_dist_for_neighbor_detections;

         const uint32_t curr_det_index = object_track.detids[curr_det_slot_index] - 1U;
         F360_Detection_Props_T * const curr_det = &detection_props[curr_det_index];
         if (curr_det->dist_to_closest_assoc_det_sq < has_neighbor_distance_threshold_sq)
         {
            curr_det->any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_YES;
         }
         else
         {
            curr_det->any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_NO;
         }
      }
   }

}
