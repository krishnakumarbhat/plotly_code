#ifndef RSPP_DETECTION_LIST_VARIANT_D_H
#define RSPP_DETECTION_LIST_VARIANT_D_H
/*===================================================================================*\
* FILE: rspp_detection_list.h
*====================================================================================
* Copyright (C) 2023 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/

#include "rspp_detection.h"
#include "rspp_constants.h"
#include "rspp_reuse.h"

namespace rspp_variant_D
{
   typedef struct RSPP_Detection_List_Tag
   {
      RSPP_Detection_T detections[MAX_NUMBER_OF_DETECTIONS];
      uint32_t number_of_valid_detections;
      int16_t vcslong_det_idx_min; // Sorted vcs-long index of detection with most negative vcs-long position
      int16_t vcslong_det_idx_max; // Sorted vcs-long index of detection with most positive vcs-long position
      int16_t vcslong_sorted_ref_det_idx[MAX_NR_OF_VCS_LONG_SORTED_DETS_REF_POINTS_ELEMENTS];
   } RSPP_Detection_List_T;
}

#endif
