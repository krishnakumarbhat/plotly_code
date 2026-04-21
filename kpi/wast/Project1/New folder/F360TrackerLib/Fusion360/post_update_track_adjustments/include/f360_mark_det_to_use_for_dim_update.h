#ifndef F360_MARK_DET_TO_USE_FOR_DIM_UPDATE_H
#define F360_MARK_DET_TO_USE_FOR_DIM_UPDATE_H
/*===========================================================================*\
* FILE: f360_mark_det_to_use_for_dim_update.h
*============================================================================
* Copyright (C) 2019 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declaration of Mark_Det_To_Use_For_Dim_Update().
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Requirements Document(s):
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

#include "f360_object_track.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   void Mark_Det_To_Use_For_Dim_Update(
      const F360_Object_Track_T &object_track,
      const F360_Calibrations_T & calib,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]);
}
#endif
