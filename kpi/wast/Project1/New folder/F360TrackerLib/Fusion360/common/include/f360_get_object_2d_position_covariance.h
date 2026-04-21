/*===========================================================================*\
* FILE: f360_get_object_2d_position_covariance.h
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function declaration of F360_Get_Object_2D_Position_Covariance()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_F360_GET_OBJECT_2D_POSITION_COVARIANCE_H
#define F360_F360_GET_OBJECT_2D_POSITION_COVARIANCE_H

#include "f360_object_track.h"

namespace f360_variant_A
{
   void F360_Get_Object_2D_Position_Covariance(
      const F360_Object_Track_T& obj,
      float32_t(&covar)[2U][2U]);
}


#endif
