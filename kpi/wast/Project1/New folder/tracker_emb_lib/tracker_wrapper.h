#ifndef TRACKER_WRAPPER_H
#define TRACKER_WRAPPER_H
// #define ROT_STANDALONE
//#define ROT_FUSION_MASTER


#ifndef F360_CONFIG_H
#define F360_CONFIG_H

// #ifdef ROT_STANDALONE
// static constexpr F360_UI16N_T NUMBER_OF_SRR_DETECTIONS = 0U;
// #endif

// #ifdef ROT_FUSION_MASTER
// static constexpr F360_UI16N_T NUMBER_OF_SRR_DETECTIONS = 0U;
// #endif

#endif // F360_CONFIG_H


#include <stdint.h> 
#include "../../app/common/drivers/ipc_data.h" // Include the header file


#define ROT_STANDALONE
#include "../../app/dss/src/F360Tracker/SharedTrackerAPI/core/f360_variant_definition.h"


// Declaraion  function prototypes
void initializeTracker(uint8_t radar_position);
void runTracker(M2D_Msg_T *p_ipc_m2d, D2M_Msg_T *p_dss_to_mss_l3, GLOB_TS_STRUCT_TYPE *det_master_ts);

#endif 

