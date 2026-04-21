/*===========================================================================*\
* FILE: f360_calc_obj_size.h
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declaration of Calc_Obj_Size()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_CALC_OBJ_SIZE_H
#define F360_CALC_OBJ_SIZE_H

#include "f360_object_track.h"
#include "f360_detection_props.h"
#include "f360_calibrations.h"
#include "f360_constants.h"
#include "rspp_detection_list.h"

namespace f360_variant_A
{
   void Calc_Obj_Size(
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const rspp_variant_A::RSPP_Detection_List_T& detection_list,
      const F360_Calibrations_T& calib,
      float32_t& measured_length,
      float32_t& measured_width,
      F360_Object_Track_T& object_track);

   void Determine_Max_Dimensions_For_Object(
      const F360_Object_Track_T & obj,
      const F360_Calibrations_T & calib,
      float32_t & max_target_length,
      float32_t & max_target_width);

   void Determine_Min_Dimensions_For_Object(
      const F360_Object_Track_T & obj,
      const F360_Calibrations_T & calib,
      float32_t & min_target_length,
      float32_t & min_target_width);

   struct Target_Dimension_T
   {
      float32_t maximum;
      float32_t minimum;
   };

   struct Dimension_Limits_T
   {
      Target_Dimension_T length;
      Target_Dimension_T width;
   };

}
#endif
