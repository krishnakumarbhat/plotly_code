
#pragma once

//#include <windows.h>
#include "../../CommonFiles/plugin/PluginLoader.h"
#include "../../CommonFiles/inc/common.h"
//#include "../../CommonFiles/inc/IRadarStream.h"
//#include "../../CoreLibraryProjects/radar_stream_decoder/Common/IRadarStream.h"
#include "../../Common/IRadarStream.h"
#include "../../CoreLibraryProjects/mudp_log/MudpRecord/udp_log_struct.h"
#include "../../CommonFiles/udp_headers/rr_cal_log.h"
#include "../../CommonFiles/inc/AUDI/sil_srr3_debug_types.h"
#include "../../CommonFiles/CommonHeaders/dph_rr_adas_shared_types.h"
#include "../../CommonFiles/inc/Resim_Config.h"
#include "../../CoreLibraryProjects/mudp_decoder/latch_defines.h"
#include "../../CommonFiles/helper/compute.h"
#include"../../../CoreLibraryProjects/mudp_decoder_calib/TrackerIntenalDecoder.h"



class PacketLossTrackerIntenalDecoder{

public:
	
	PacketLossTrackerIntenalDecoder(F360_GDSR_Tracker_T **pSCal);
	PacketLossTrackerIntenalDecoder();
	~PacketLossTrackerIntenalDecoder();
	int Init();
	void FreePlugin();
	F360_GDSR_status_e start_F360TrackerDecode(UDP_FRAME_PAYLOAD_T const &udp_frame_payload);
	F360_GDSR_status_e start_GDSRTrackerDecode(UDP_FRAME_PAYLOAD_T const &udp_frame_payload);
	F360_GDSR_status_e start_GEN5_GDSRTrackerDecode(UDP_FRAME_PAYLOAD_T const &udp_frame_payload);
	
private:
	PacketLossTrackerIntenalDecoder *pGdsrF360Tracker;
	F360_GDSR_Tracker_T *pF360Gdsr[MAX_RADAR_COUNT];
	TrackerIntenalDecoder *m_ptrackerIntDecoder;
};
