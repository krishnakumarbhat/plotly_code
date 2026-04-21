#ifndef F360_ANY_OTHER_ASSOC_DET_CLOSE_H
#define F360_ANY_OTHER_ASSOC_DET_CLOSE_H

/*===========================================================================*\
* FILE: f360_any_other_assoc_det_close.h
*============================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*----------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains definition of F360_Any_Other_Assoc_Det_Close enum.
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

#include "f360_reuse.h"

namespace f360_variant_A
{
   enum F360_Any_Other_Assoc_Det_Close : int8_t
   {
      F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED = -1,
      F360_ANY_OTHER_ASSOC_DET_CLOSE_NO = 0,
      F360_ANY_OTHER_ASSOC_DET_CLOSE_YES = 1,
   };
}
#endif
