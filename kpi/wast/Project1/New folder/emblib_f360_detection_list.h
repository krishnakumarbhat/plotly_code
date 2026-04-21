/*===================================================================================*\
* FILE: f360_detection_list.h
*====================================================================================
* Copyright 2024 Aptiv Technologies, Inc., All Rights Reserved.
* Aptiv Confidential
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
*   This file contains F360_Detection_List_T structure declaration
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s): defineFusion360Types.m
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_DETECTION_LIST_VARIANT_A_H
#define F360_DETECTION_LIST_VARIANT_A_H

#include "emblib_f360_detection.h"
#include "f360_constants.h"
#include "f360_reuse.h"

namespace f360_variant_C
{
typedef struct F360_Detection_List_Tag
{
   F360_Detection_T detections[MAX_NUMBER_OF_DETECTIONS];
   F360_UI32N_T number_of_valid_detections;
   F360_SI16N_T vcslong_det_idx_min; // Sorted vcs-long index of detection with most negative vcs-long position
   F360_SI16N_T vcslong_det_idx_max; // Sorted vcs-long index of detection with most positive vcs-long position
   F360_SI16N_T vcslong_sorted_ref_det_idx[MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS_ELEMENTS];

} F360_Detection_List_T;

static_assert((sizeof(F360_Detection_List_T::detections) + 4 + 2 + 2 + 68) == sizeof(F360_Detection_List_T),
              "sizeof(F360_Detection_List_T) not as expected. Remember to align padding if needed");
} // namespace f360_variant_C

#endif
