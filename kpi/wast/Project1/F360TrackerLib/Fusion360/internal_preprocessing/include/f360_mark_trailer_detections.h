/*===========================================================================*\
* FILE: f360_mark_trailer_detections.h
*============================================================================
* Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* 
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#ifndef F360_MARK_TRAILER_DETECTIONS_H
#define F360_MARK_TRAILER_DETECTIONS_H

#include "f360_reuse.h"
#include "f360_iterator.h"
#include "f360_point.h"
#include "f360_host.h"
#include "rspp_detection_list.h"
#include "f360_detection_props.h"
#include "f360_bounding_box.h"
#include "f360_trailer_detector_flt_fus_output.h"

namespace f360_variant_A
{
        void Detect_Det_On_Trailer(const F360_Host_T &f360_host,
        const rspp_variant_A::RSPP_Detection_List_T &raw_detections,
        const Trailer_Detector_Flt_Fus_Output &trailer,
        F360_Detection_Props_T (&det_Props)[MAX_NUMBER_OF_DETECTIONS]);
}

#endif
