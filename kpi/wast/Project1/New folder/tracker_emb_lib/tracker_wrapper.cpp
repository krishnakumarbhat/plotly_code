#include "tracker_wrapper.h" 

#include"../../src/F360Tracker/f360_Radar_Tracker.h" 
#include <iostream>

#define ROT_STANDALONE
#include "../../app/dss/src/F360Tracker/SharedTrackerAPI/core/f360_variant_definition.h"




void initializeTracker(uint8_t radar_position) {
    Initialize_F360_Radar_Tracker(radar_position);
    std::cout << "[TRACKER][INIT]" << std::endl;
}


void runTracker(M2D_Msg_T *p_ipc_m2d, D2M_Msg_T *p_dss_to_mss_l3, GLOB_TS_STRUCT_TYPE *det_master_ts) {
    Run_F360_Radar_Tracker(p_ipc_m2d, p_dss_to_mss_l3, det_master_ts);
    std::cout << "[TRACKER][RUN]" << std::endl;


}
