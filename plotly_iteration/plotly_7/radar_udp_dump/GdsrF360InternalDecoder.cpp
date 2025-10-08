#include"../../../ApplicationProjects/Radar_UDPData_List_App/radar_udp_dump/GdsrF360InternalDecoder.h"
#include"../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config_mode.h"
#include <fstream>


extern MUDP_CONFIG_T *MUDP_master_config ;
static bool MidHighSensorsEnabled = false;

#include<iostream>


PacketLossTrackerIntenalDecoder::PacketLossTrackerIntenalDecoder(){

	for (int n = 0; n < MAX_RADAR_COUNT; n++){

		pF360Gdsr[n] = new F360_GDSR_Tracker_T;

		if(pF360Gdsr[n]){
			pF360Gdsr[n]->F360_GDSR_ScanId = 0;
			pF360Gdsr[n]->pF360_GDSR_Buffer = nullptr;
		}
	}
	m_ptrackerIntDecoder = new TrackerIntenalDecoder(pF360Gdsr);

}

PacketLossTrackerIntenalDecoder::~PacketLossTrackerIntenalDecoder(){
	/*if(pGdsrF360Tracker){
		delete pGdsrF360Tracker ;
		pGdsrF360Tracker = nullptr;
	}*/
}

static uint8_t MapgdsrSourceToRadarPos(uint8_t  hdrRadarPos)
{
	uint8_t pos = 0xFF;
	// For SRR3 sensors, radar position is derived from the sourceInfo of UDP header: 21, 22, 23, 24
	if(hdrRadarPos >= UDP_PLATFORM_CUST_SRR3_RL && hdrRadarPos <= UDP_PLATFORM_CUST_SRR3_FC){
		pos = (uint8_t)(hdrRadarPos - UDP_PLATFORM_CUST_SRR3_RL);
	}
	// For DSPACE UDP logs, radar position is hardcoded to 18
	else if(hdrRadarPos == UDP_SOURCE_CUST_DSPACE){
		pos = RDR_POS_DSPACE;
	}
	else if (hdrRadarPos == BPILLAR_STATUS_BP_L || hdrRadarPos == UDP_PLATFORM_FORD_SRR5) {
		pos = RDR_POS_FC;
	}
	// For SRR5 sensors including radar ECU, radar position is derived from the Radar_Pos of UDP Header: 1, 2, 3, 4 and for ECU 20
	else{
		pos = (uint8_t)(hdrRadarPos - 1);
	}
	return pos;
}
bool getMidHighSensorsEnabled()
{
	// Set MidHighSensorsEnabled if any of one sensor is enabled and Radar ECU status is 1
	// or B-Pillar Status is 1
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1 || MUDP_master_config->Bpillar_status == 1) {
		for(int senID = 0; senID < 4; senID++) {
			if(MUDP_master_config->MUDP_Select_Sensor_Status[senID] == 1) {
				MidHighSensorsEnabled = true;
				return MidHighSensorsEnabled;
			}
		}
	}
}

F360_GDSR_status_e PacketLossTrackerIntenalDecoder::start_F360TrackerDecode(UDP_FRAME_PAYLOAD_T const &udp_frame_payload){
	F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
	unsigned char radar_index = RDR_POS_ECU; 
	status = m_ptrackerIntDecoder->DecodeF360Tracker(&udp_frame_payload, pF360Gdsr, radar_index);
	return status;
}

F360_GDSR_status_e PacketLossTrackerIntenalDecoder::start_GDSRTrackerDecode(UDP_FRAME_PAYLOAD_T const &udp_frame_payload){
	F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
	unsigned char radar_index = RDR_POS_ECU; 
	const UDP_FRAME_STRUCTURE_T *ptr_bytestream = (const UDP_FRAME_STRUCTURE_T *)&udp_frame_payload;
	radar_index = MapgdsrSourceToRadarPos(ptr_bytestream->frame_header.Radar_Position);

	/*status = m_ptrackerIntDecoder->DecodeGDSRTracker(&udp_frame_payload, pF360Gdsr, radar_index);
	return status;*/
	if(getMidHighSensorsEnabled() == true && (MUDP_master_config->MUDP_Radar_ECU_status == 1 || MUDP_master_config->MUDP_Radar_ECU_status == 0 )&& MUDP_master_config->Bpillar_status == 1){ // HIGH varaint
		if(radar_index != BPIL_POS_L && radar_index != BPIL_POS_R ){
			status = DPH_F360_GDSR_FRAME_UNAVAILABLE;
		}else{
			status = m_ptrackerIntDecoder->DecodeGDSRTracker(&udp_frame_payload, pF360Gdsr, radar_index);
		}
	}
	else{
		status = m_ptrackerIntDecoder->DecodeGDSRTracker(&udp_frame_payload, pF360Gdsr, radar_index); // LOW varaint
	}
	return status;
}
F360_GDSR_status_e PacketLossTrackerIntenalDecoder::start_GEN5_GDSRTrackerDecode(UDP_FRAME_PAYLOAD_T const &gen5_udp_frame_payload) {
	F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
	unsigned char radar_index = RDR_POS_ECU;
	const SRR6_UDP_FRAME_STRUCTURE_T *ptr_bytestream = (const SRR6_UDP_FRAME_STRUCTURE_T *)&gen5_udp_frame_payload;

	radar_index = MapgdsrSourceToRadarPos(ptr_bytestream->frame_header.Radar_Position);

	status = m_ptrackerIntDecoder->DecodeGDSRTracker(&gen5_udp_frame_payload, pF360Gdsr, radar_index);
	return status;
}