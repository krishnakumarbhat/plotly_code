/*===================================================================================*\
* FILE: f360_mark_det_to_use_for_dim_update.cpp
*====================================================================================
* Copyright (C) 2019 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Mark_Det_To_Use_For_Dim_Update().
*
* ABBREVIATIONS:
*   List of abbreviations used, or reference(s) to external document(s)
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
#include "f360_mark_det_to_use_for_dim_update.h"
#include "f360_update_object_track_properties.h"
#include "f360_mark_detections_with_neighbors.h"

namespace f360_variant_A
{
   /******************************
   *   Function prototypes
   *******************************/
   static bool Check_If_Det_Should_Be_Used_In_Dim_Update_Moving_Tracks(
      const rspp_variant_A::RSPP_Detection_T& single_det_raw,
      const F360_Detection_Props_T &single_det);

   static bool Check_If_Det_Should_Be_Used_In_Dim_Update_Stopped_Tracks(
      const rspp_variant_A::RSPP_Detection_T& single_det_raw,
      const F360_Detection_Props_T &single_det);

   /*===========================================================================*\
   * FUNCTION: Mark_Det_To_Use_For_Dim_Update()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *  const F360_Object_Track_T &object_track,
   *  const F360_Calibrations_T & calib,
   *  const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   *  F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   *
   * EXTERNAL REFERENCES:
   * Logic introduced by this function presented in table form could be found in excel sheet under this link:
   * http://s03.delphiauto.net/04/RSE_2/active/ActiveSafetyAlgorithmGroup/F360Core/SysSWITnV/ENG06%20Software%20Detailed%20Design%20and%20Construct/Concepts/Mark_Det_To_Use_For_Dim_Update_logic.xlsx
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function mark detections to be used in track dimension update or not.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Mark_Det_To_Use_For_Dim_Update(
      const F360_Object_Track_T &object_track,
      const F360_Calibrations_T & calib,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      const uint32_t assoc_dets_count = object_track.ndets;

      Mark_Detections_With_Neighbors(object_track, calib, detection_props);

      // Loop over associated detections.
      for (uint32_t assoc_det_slot_index = 0U; assoc_det_slot_index < assoc_dets_count; assoc_det_slot_index++)
      {
         const uint32_t det_index = object_track.detids[assoc_det_slot_index] - 1U;
         bool f_curr_det_use_in_dimension_update;

         if (object_track.f_moving)
         {
            f_curr_det_use_in_dimension_update = Check_If_Det_Should_Be_Used_In_Dim_Update_Moving_Tracks(raw_detect_list.detections[det_index], detection_props[det_index]);
         }
         else if (object_track.f_moveable)
         {
            f_curr_det_use_in_dimension_update = Check_If_Det_Should_Be_Used_In_Dim_Update_Stopped_Tracks(raw_detect_list.detections[det_index], detection_props[det_index]);
         }
         else
         {
            f_curr_det_use_in_dimension_update = true;
         }

         detection_props[det_index].f_use_in_dimension_update = f_curr_det_use_in_dimension_update;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Check_If_Det_Should_Be_Used_In_Dim_Update_Moving_Tracks()
   *===========================================================================
   * RETURN VALUE:
   * bool f_detection_should_be_used
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &single_det_raw,
   * const F360_Detection_Props_T &single_det
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
   * Evaluate if detection should be used for track object dimension update for moving tracks.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   static bool Check_If_Det_Should_Be_Used_In_Dim_Update_Moving_Tracks(
      const rspp_variant_A::RSPP_Detection_T &single_det_raw,
      const F360_Detection_Props_T &single_det
   )
   {
      bool f_detection_should_be_used;

      if (single_det.any_other_assoc_det_close != F360_ANY_OTHER_ASSOC_DET_CLOSE_NO)
      {
         f_detection_should_be_used = single_det.f_rr_inlier;
      }
      else
      {
         f_detection_should_be_used = (single_det_raw.processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING);
      }
      return f_detection_should_be_used;
   }

   /*===========================================================================*\
   * FUNCTION: Check_If_Det_Should_Be_Used_In_Dim_Update_Stopped_Tracks()
   *===========================================================================
   * RETURN VALUE:
   * bool f_detection_should_be_used
   *
   * PARAMETERS:
   *  const rspp_variant_A::RSPP_Detection_T& single_det_raw,
   *  const F360_Detection_Props_T &single_det
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
   * Evaluate if detection should be used for track object dimension update for stopped tracks.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   static bool Check_If_Det_Should_Be_Used_In_Dim_Update_Stopped_Tracks(
      const rspp_variant_A::RSPP_Detection_T& single_det_raw,
      const F360_Detection_Props_T &single_det)
   {
      bool f_detection_should_be_used;

      if (single_det.any_other_assoc_det_close != F360_ANY_OTHER_ASSOC_DET_CLOSE_NO)
      {
         f_detection_should_be_used = ((single_det_raw.processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING) || single_det.f_rr_inlier);
      }
      else
      {
         f_detection_should_be_used = ((single_det_raw.processed.motion_status == rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING) && single_det.f_rr_inlier);
      }
      return f_detection_should_be_used;
   }
}
