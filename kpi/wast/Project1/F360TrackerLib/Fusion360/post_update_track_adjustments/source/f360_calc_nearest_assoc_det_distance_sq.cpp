/*===========================================================================*\
* FILE: f360_calc_nearest_assoc_det_distance_sq.cpp
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
*   This file contains function definition of Calc_Nearest_Assoc_Det_Distance_Sq().
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
#include "f360_calc_nearest_assoc_det_distance_sq.h"
#include "f360_math_func.h"   

namespace f360_variant_A
{
   static void Update_Detection_Dist_To_Closest_Assoc_Det_Sq(
      F360_Detection_Props_T &detection,
      const float32_t dist_between_dets_sq);

   static float32_t Calc_Distance_Sq_Between_Detections(
      const F360_Detection_Props_T &detection_A,
      const F360_Detection_Props_T &detection_B);

   /*===========================================================================*\
   * Function definitions
   \*===========================================================================*/

   /*===========================================================================*\
   * FUNCTION: Calc_Nearest_Assoc_Det_Distance_Sq
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Object_Track_T const * const object_track,
   * F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function which operates on track's associated detections. It loops over associated
   * detections and calculates distance square to other associated detections. These
   * distances are saved in F360_Detection_Props_T.dist_to_closest_assoc_det_sq. Current
   * implementation provides O(n^2/2) complexity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Calc_Nearest_Assoc_Det_Distance_Sq(const F360_Object_Track_T &object_track,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      const uint32_t ndets = object_track.ndets;

      // Loop over all possible detection pair combinations
      for (uint32_t i = 0U; i < ndets - 1U; i++)
      {
         for (uint32_t j = i + 1U; j < ndets; j++)
         {
            const uint32_t det_idx1 = object_track.detids[i] - 1U;
            const uint32_t det_idx2 = object_track.detids[j] - 1U;

            const float32_t dist_between_dets_sq = Calc_Distance_Sq_Between_Detections(detection_props[det_idx1], detection_props[det_idx2]);
            Update_Detection_Dist_To_Closest_Assoc_Det_Sq(detection_props[det_idx1], dist_between_dets_sq);
            Update_Detection_Dist_To_Closest_Assoc_Det_Sq(detection_props[det_idx2], dist_between_dets_sq);
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Update_Detection_Dist_To_Closest_Assoc_Det_Sq
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Detection_Props_T &detection,
   * const float32_t dist_between_dets_sq
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function checks if given distance is lower than already stored distance to closest
   * detection squared and if it is true - distance is being updated.
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   static void Update_Detection_Dist_To_Closest_Assoc_Det_Sq(
      F360_Detection_Props_T &detection,
      const float32_t dist_between_dets_sq)
   {
      if (dist_between_dets_sq < detection.dist_to_closest_assoc_det_sq) // (dist_to_closest_assoc_det_sq is initially set to INFTY, a very large number inside Clear_Detections_Props())
      {
         detection.dist_to_closest_assoc_det_sq = dist_between_dets_sq;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Distance_Sq_Between_Detections
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Detection_Props_T &detection_A,
   * const F360_Detection_Props_T &detection_B
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function returns distance squared between two given detections.
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   static float32_t Calc_Distance_Sq_Between_Detections(
      const F360_Detection_Props_T &detection_A,
      const F360_Detection_Props_T &detection_B)
   {
      float32_t const diff_vcs_pos_x = detection_A.vcs_position.x - detection_B.vcs_position.x;
      float32_t const diff_vcs_pos_y = detection_A.vcs_position.y - detection_B.vcs_position.y;

      float32_t const dist_between_dets_sq = F360_Get_Hypotenuse_Squared(diff_vcs_pos_x, diff_vcs_pos_y);
      return dist_between_dets_sq;
   }

}
