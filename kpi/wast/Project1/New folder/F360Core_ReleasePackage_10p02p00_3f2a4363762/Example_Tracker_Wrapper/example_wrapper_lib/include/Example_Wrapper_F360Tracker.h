/*===================================================================================*\
Disclaimer:
This file is intended as an example showing how to integrate the tracker, it is not to be used as is for production.
All values used/assigned below are example values from previous programs, they are not to be reused.
\*===================================================================================*/

/*===================================================================================*\
 * FILE:  Example_Wrapper_F360Tracker.h
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose."
 *------------------------------------------------------------------------------------
 * DESCRIPTION:
 * This file contains declarations of functions defined in Example_Wrapper_F360Tracker.cpp
 *
 *
 * Applicable Standards (in order of precedence: highest first):
 * ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
 * ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/
#ifndef EXAMPLE_WRAPPER_F360TRACKER_H
#define EXAMPLE_WRAPPER_F360TRACKER_H

#include "tracker_IAL.h"
#include "CAF_Parameters.h"

void Initialize_Tracker(CAF_Param_T* p_CAF_Parameter);
void Run_Tracker_Wrapper(Tracker_IAL_T* p_tracker_IAL);

#endif // EXAMPLE_WRAPPER_F360TRACKER_H
