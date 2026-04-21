#ifndef F360_POPULATE_INTERNAL_TRAILER_DETECTOR_LOG_H
#define F360_POPULATE_INTERNAL_TRAILER_DETECTOR_LOG_H
/*===================================================================================*\
 * FILE:  f360_populate_internal_trailer_detector_log.h
 *====================================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/

#include "TrailerDetectorInternalLog.h"
#include "f360_trailer_detector_core.h"
#include "f360_trailer_detector_constants.h"
#include "TrailerDetectorLog.h"
#include "T360_Types.h"

namespace f360_variant_A
{
   void Populate_Internal_Trailer_Detector_Data(Trailer_Detector_Core& trailer_detector,
      const F360_Internal_Trailer_Detector_T& trailer_internal_log, 
      const Trailer_Detector_Log_T& trailer_detector_log);

   void Populate_Internal_Trailer_Detector_Log_Data(F360_Internal_Trailer_Detector_T& trailer_internal_log,
      const Trailer_Detector_Core& trailer_detector);

}

#endif
