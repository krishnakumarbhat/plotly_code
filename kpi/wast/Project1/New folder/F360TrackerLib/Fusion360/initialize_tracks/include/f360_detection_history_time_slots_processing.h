/*===================================================================================*\
* FILE: f360_detection_history_time_slots_processing.h
*====================================================================================
* Copyright © 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function declarations:
*     - Get_NEES_CFMI_Information_Cloud_Current_Dets()
*     - Get_NEES_CFMI_Information_Cloud_Old_Dets()
*     - Update_NEES_Detection_Slot()
*     - Calc_NEES_CFMI_Determinants_Cloud()
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
\*==========================================================================================*/
#ifndef F360_DETECTION_HISTORY_TIME_SLOTS_PROCESSING_H
#define F360_DETECTION_HISTORY_TIME_SLOTS_PROCESSING_H

#include "f360_detection_time_slots.h"
#include "f360_nees_cfmi_structs.h"

namespace f360_variant_A
{
   void Split_Dets_Into_Time_Slots(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots,
      const float32_t dt_for_split);

   void Create_Time_Slot_For_Current_Detections(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots);

   void Split_Hist_Dets_Into_Time_Slots(
      F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_Detection_Time_Slots_T& det_hist_time_slots,
      const float32_t dt_for_split);
}
#endif
