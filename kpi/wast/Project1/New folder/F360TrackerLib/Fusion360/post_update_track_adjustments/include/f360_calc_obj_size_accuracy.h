/*===================================================================================*\
* FILE: f360_calc_obj_size_accuracy.h
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
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
*   This file contains Calc_Obj_Size_Accuracy() function declaration
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
#ifndef F360_CALC_SIZE_ACCURACY_H
#define F360_CALC_SIZE_ACCURACY_H

#include "f360_reuse.h"
#include "f360_object_track.h"
#include "f360_calibrations.h"

namespace f360_variant_A
{
   void Calc_Obj_Size_Accuracy(
      const float32_t measured_len,
      const float32_t measured_wid,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & obj);

   void Calc_Obj_Size_Innovation(
      const float32_t measured_len,
      const float32_t measured_wid,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & obj);
}
#endif
