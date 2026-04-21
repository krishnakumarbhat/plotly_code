#ifndef F360_MARK_DETECTIONS_WITH_NEIGBOURS_H
#define F360_MARK_DETECTIONS_WITH_NEIGBOURS_H
/*===========================================================================*\
* FILE: f360_mark_detections_with_neighbors.h
*============================================================================
* Copyright © 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains functions declarations of Mark_Detections_With_Neighbors().
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

#include "f360_detection_props.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"
namespace f360_variant_A {
   void Mark_Detections_With_Neighbors(
      const F360_Object_Track_T & object_track,
      const F360_Calibrations_T & calibrations,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   );

   void Check_If_Detection_Has_Neighbor(
      const F360_Object_Track_T  & object_track,
      const F360_Calibrations_T & calibrations,
      F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   );
}
#endif
