#ifndef F360_CALC_NEAREST_ASSOC_DET_DISTANCE_SQ_H
#define F360_CALC_NEAREST_ASSOC_DET_DISTANCE_SQ_H
/*===========================================================================*\
* FILE: f360_calc_nearest_assoc_det_distance_sq.h
*============================================================================
* Copyright - 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains functions declaration of Calc_Nearest_Assoc_Det_Distance_Sq().
*
* ABBREVIATIONS:
*   None
*
* TRACEABILITY INFO:
*   Requirements Document(s):
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards" [May 26, 2019]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===========================================================================*/

#include "f360_detection_props.h"
#include "f360_object_track.h"
namespace f360_variant_A
{
   void Calc_Nearest_Assoc_Det_Distance_Sq(
   const F360_Object_Track_T &object_track,
   F360_Detection_Props_T (&detection_props)[MAX_NUMBER_OF_DETECTIONS]
   );
}

#endif
