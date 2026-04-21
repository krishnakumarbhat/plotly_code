#ifndef F360_RADAR_TRACKER_H
#define F360_RADAR_TRACKER_H
/*===========================================================================*\
 * File: f360_radar_tracker.h
 *===========================================================================
 * Copyright 2018 Aptiv, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 * Created on: August 24, 2018
 * Author: Eric Schrock (qj91h7)
 *---------------------------------------------------------------------------
 *
 * Description:
 *    Embedded wrapper for the F360 radar tracker.
 *
\*===========================================================================*/

/*===========================================================================*\
 * Standard Header Includes
\*===========================================================================*/

/*===========================================================================*\
 * Project Header Includes
\*===========================================================================*/
#include "ROT_ISOobject_stream.h"
#include "emblib_ROT_cals.h"
#include "emblib_f360_data_interface.h"
// #include "helpers/mem_pool.h"
#include "emblib_ipc_data.h"
#include "reuse.h"

/*===========================================================================*\
 * Exported Preprocessor Constants
\*===========================================================================*/

/*===========================================================================*\
 * Exported Type Declarations
\*===========================================================================*/

/*===========================================================================*\
 * Exported Function Prototypes
\*===========================================================================*/
#ifdef __cplusplus
extern "C"
{
#endif
   /**
    * @note This MUST be called prior to Run_F360_Radar_Tracker/Initialize_F360_Radar_Tracker.
    * If this is not called, memory buffers will not be allocated for processing!
    * **/
   extern void Initialize_F360_Radar_Tracker(uint8_t radar_position);
   extern void Initialize_host_calib(void);
   extern void Run_F360_Radar_Tracker(Emblib_M2D_Msg_T *p_ipc_m2d, Emblib_D2M_Msg_T *p_dss_to_mss_l3,
                                      GLOB_TS_STRUCT_TYPE *det_master_ts);
#ifdef __cplusplus
}
#endif

#endif /* F360_RADAR_TRACKER_H */
