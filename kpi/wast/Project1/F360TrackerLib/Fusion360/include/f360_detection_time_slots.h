/*===================================================================================*\
* FILE: f360_detection_history_time_slots.h
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   Detections separated in time in couple time slots
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_DETECTION_TIME_SLOTS_H
#define F360_DETECTION_TIME_SLOTS_H

#include "f360_reuse.h"
#include "f360_constants.h"

namespace f360_variant_A
{
struct F360_Detection_Time_Slot_T
{
   uint32_t hist_dets_idx[F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT];
   uint32_t number_of_active_detections;
   float32_t min_time_since_meas;
   float32_t max_time_since_meas;
};

struct F360_Detection_Time_Slots_T
{
   F360_Detection_Time_Slot_T time_since_meas_slots[F360_NUMBER_OF_DETECTIONS_HIST_IDX_TIME_SLOTS];
   uint32_t number_of_dets;
   uint32_t number_of_active_slots;
};

   static_assert((sizeof(F360_Detection_Time_Slot_T::hist_dets_idx) + 12) == sizeof(F360_Detection_Time_Slot_T), "sizeof(F360_Detection_Time_Slot_T) not as expected. Remember to align padding if needed");
   static_assert((sizeof(F360_Detection_Time_Slots_T::time_since_meas_slots) + 8) == sizeof(F360_Detection_Time_Slots_T), "sizeof(F360_Detection_Time_Slots_T) not as expected. Remember to align padding if needed");
}
#endif
