/****************************************************************************
*
*   Delphi Electronics and Safety
*
*   Copyright (C) 2014 by Delphi Corporation. All Rights Reserved.
*   This file contains Delphi Electonics and Safety Proprietary information.
*   It may not be reproduced or distributed without permission.
*
***************************************************************************/

/******************************************************************************
FileName   : xml_trace.cpp

Author     : Bhavana Karanji

Date       : 19 Feb, 2020

Description: For creating the Embedded LIB UDP XML_Traces files


Version No : Date of update:         Updated by:           Changes:

Issues     : None

******************************************************************************/
#include "stdafx.h"
#if defined(_WIN_PLATFORM)
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#elif defined(__GNUC__)
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#include <stdlib.h>
#include "PackUDPLogging.h"
#include <iostream>
#include <iomanip>
#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"
#include "../../../CommonFiles/plugin/PluginLoader.h"
#include "../../../CommonFiles/inc/ByteStream.h"
#include "../../../CommonFiles/inc/common.h"
//#include "../../../CoreLibraryProjects/radar_stream_decoder/Calibration/Calib.h"
#include "time.h"
//#include "MUDP_interface.h"
#include "xml_trace.h"
#include "../../../ApplicationProjects/Radar_UDPData_List_App/radar_udp_dump/radar_udp_dump_options.h"
#include "../../../CommonFiles/inc/HiLBaseStream.h"
#include "../../../CommonFiles/cca_vigem_inc/vgm_cca.h"
//#include "C:\Users\vjqv6l\Desktop\cca_ViGEM\v2.1.6\include\vgm_cca.h"
//#include "../../../CoreLibraryProjects/CCA_ViGEM/inc/vgm_cca_log.h"

//#include "../../../CoreLibraryProjects/radar_stream_decoder/Common/IRadarStream.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
//#include "../../../CommonFiles/inc/Z_Logging.h"
#include "../../../CoreLibraryProjects/mudp_log/MudpRecord/udp_log_struct.h"
#include"../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config_mode.h"
#include "../../../CommonFiles/udp_headers/rr_cal_log.h"
#include "../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"
#include "../../../CommonFiles/CommonHeaders/dph_rr_adas_config_mode.h"
#include "../../../CommonFiles/helper/genutil.h"
#include "../../../CoreLibraryProjects/mdf_log/inc/apt_mdf_log.h"
#include "../../CommonFiles/helper/RadarDecoder.h"

#define RADAR_POS 19
#define FIXED 6
/** Number of detections per sensor*/
#define NUMBER_OF_DETECTIONS (200)
#define AF_DET_MAX_COUNT_FLR4	 (256)//STLA_FLR4
#define RDD_MAX_NUM_DET    (400U) // FLR4P dets
/** Number of fused objects over the entire field of view*/
#define NUMBER_OF_OBJECTS (64)
#define NUMBER_OF_PARTNER_DETECTIONS_L (64)
//#define NO_OF_EVENTS_Z7B	 (30)
#define NUMBER_OF_OBJECTS_L	 (64)
#define NUMBER_OF_OBJECT_TRACKS_L    (64U) //Tracker
//FLR4P Tracker 
#define TWO    (2U)
#define THREE    (3U)
#define FOUR    (4U)
#define SIX    (6U)
FILE* SIL_XML_out = NULL;
FILE* SIL_CSV_out = NULL;
static bool  flag_out = { 0 };
static bool  flag_ecu_out[MAX_RADAR_COUNT] = { 0 };
char pathname_XML[255] = { 0 };
char* token = NULL;
char* token_1 = NULL;
char* token_2 = NULL;
int SP_fail_count = 0;
using namespace std;

uint32_t g_scanindex = 0;

//---dspace input------//
DSAPCE_TrackObject_T rear_left_object[SIMULATED_TRACKS] = { 0 };
DSAPCE_TrackObject_T rear_right_object[SIMULATED_TRACKS] = { 0 };
DSAPCE_TrackObject_T front_left_object[SIMULATED_TRACKS] = { 0 };
DSAPCE_TrackObject_T front_right_object[SIMULATED_TRACKS] = { 0 };
DSAPCE_TrackObject_T front_center_object[SIMULATED_TRACKS] = { 0 };
DSPACE_MISC_Data_T pMisc = { 0 };
VEHICLE_DATA_T p_veh = { 0 };
STLA_SRR6P_Alignment_Log_Data_T p_veh_stla = { 0 };
STLA_FLR4_Alignment_Log_Data_T p_veh_stla_flr4 = { 0 };
DSPACE_MOUNTING_POSITION_VALUES_T pMountPos_RL = { 0 };
DSPACE_MOUNTING_POSITION_VALUES_T pMountPos_RR	= { 0 };
DSPACE_MOUNTING_POSITION_VALUES_T pMountPos_FR	= { 0 };
DSPACE_MOUNTING_POSITION_VALUES_T pMountPos_FL	= { 0 };
DSPACE_MOUNTING_POSITION_VALUES_T pMountPos_FC	= { 0 };
//-------------------------------------//
TARGETINFO_T Dets_XML = { 0 };
TARGETINFO_MAN_DSPACE_T  Target_Info_Dspace = { 0 };
CurviTrack_T curvi_XML = { 0 };
CDC_FRAME_T cdc_frame_XML = { 0 };
VEHICLE_DATA_T GPP_VEH = { 0 };
static bool CreateOnehdr = true;
H2A_Vehicle_Data_Logging_xml_T VEH_DATA = { 0 };
GPP_CAF_Info_xml_T Gpp_Caf_XML = { 0 };
Tracker_Input_Reports_PARTNER_Logging_xml_self_T Dets_self_XML = { 0 };
Tracker_Input_Reports_PARTNER_Logging_xml_opp_T Dets_Opp_XML = { 0 };
DETECTIONS_T Dets_RL= { 0 };
DETECTIONS_T Dets_RR = { 0 };
DETECTIONS_T Dets_FL = { 0 };
DETECTIONS_T Dets_FR = { 0 };
Tracker_Input_Reports_Logging_xml_MAN_T Dets_self_MAN_XML = { 0 };
Tracker_Input_Reports_Logging_CHANGAN_T pDets_self_CHANAGN_XML = { 0 };
Tracker_Input_Reports_Logging_RNA_T Dets_self_RNA = { 0 };
Z4_Tracker_Input_Reports_Logging_RNA_T Dets_opp_RNA = { 0 };
Tracker_Input_Reports_PARTNER_Logging_CHANGAN_T pDets_opp_CHANAGN_XML = { 0 };
Z4_Tracker_Input_Reports_Logging_xml_man_opp_T Dets_Opp_MAN_XML = { 0 };
Radar_Blockage_DPH_State_T MNR_BLOCKAGE_OUTPUT_XML = { 0 };
STLA_Blockage_Output_Log_Data_T MNR_BLOCKAGE_OUTPUT_XML_STLA = { 0 };
STLA_Blockage_Output_Log_Data_T MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4 = { 0 };
STLA_C2_Diagnostics_Logging_Data_T Diagnostics = { 0 };
DEBUG_TRACKER_OUTPUT_T curvi_track_XML = { 0 };
DPH_ADC_PHYSICAL_DATA_T ADC_Data_XML = { 0 };
Radar_Info_T Radar_Info_XML = { 0 };
LCDA_XML_T LCDA_LOW_DATA = { 0 };
CTA_XML_T CTA_LOW_DATA = { 0 };
CED_XML_T CED_DATA_XML = { 0 };
DPH_TIME_INFO_DATA_T Time_info_xml = { 0 };
Z7A_Data_T Z7a_time_info_xml = { 0 };
DPH_Z4_TIME_INFO_DATA_T Z4_time_info_xml = { 0 };
fs_stat_obj_objectpool_Logging_xml_T Free_space_xml = { 0 };
Z7B_Sector_Logging_Data_T Free_space_man_xml[NO_OF_BLOCKS] = { 0 };
SRR3_ASW_Output_T ASW_XML = { 0 };
LCDA_Output_Logging_xml_T LCDA_XML_Data = { 0 };
CTA_Output_Logging_xml_T CTA_XML_Data = { 0 };
RECW_Output_Logging_xml_T RECW_XML_DATA = { 0 };
DPH_HOST_OBJECT_T HOST_DATA = { 0 };
DPH_ECU_NUM_T ECU_DATA_XML = { 0 };
Z7A_Data_T second_paas_xml = { 0 };
DETECTIONS_T DetSelfOPP_XML = { 0 };
DETECTIONS_T Detdata_RL = { 0 };
DETECTIONS_T Detdata_RR = { 0 };
DETECTIONS_T Detdata_FR = { 0 };
DETECTIONS_T Detdata_FL = { 0 };
DETECTIONS_T Detdata_FC = { 0 };
DETECTIONS_T Detdata = { 0 };
First_Pass_Rdd_Output_Logging_xml_T FRDD_XML = { 0 };
TML_CurviTrack_COMP_T  curvi_tracks_XML = { 0 };
extern RADAR_Plugin_Interface_T *m_radar_plugin;

//gen5-------//

C2_Tracker_Logging_Data_GEN5_T gen5_curvi_track_XML = { 0 };
Tracker_Input_Reports_SELF_Logging_GEN5_T gen5_Dets_xml = { 0 };
TargetReportInfo_Logging_Gen5_T Gen5_Target_info = { 0 };
//honda_srr6p----CYX-4410---- 03/10/2024//
HONDA_SRR6P_Alignment_Log_Data_T honda_srr6p_Alignment_xml = { 0 };

Tracker_Input_Reports_OPP_Logging_GEN5_T gen5_OppDets_xml = { 0 };
C0_TargetReportInfo_Logging_Opp_T Gen5_target_info = { 0 };

//----------------------------

//---------------FLR4P-----------------//

Detection_Log_Data_FLR4P_T Flr4p_Dets_info = { 0 };
SRR7P_Detection_Stream_T SRR7p_Dets_info = { 0 };
SRR7P_ROT_Object_Stream_T SRR7p_tracks_info = { 0 };
Tracker_Log_Data_FLR4P_T Flr4p_Tracker_info = { 0 };
SRR7P_Alignment_Stream_T p_alignment_srr7p = { 0 };

/** Value of PI **/
#define PI 3.14159265359
/** Radian to Degree conversion**/
#define RAD2DEG(x) ((x * 180.00) / PI)

//------------STLA_SRR6P---------------//
//STLA_SRR6P Detections CYW-2730
STLA_Detection_Log_Data_T STLA_Dets_info = { 0 };
STLA_Detection_Log_Data_T STLA_FLR4_Dets_info = { 0 };
STLA_FLR4P_Tracker_Log_Data_T STLA_FLR4P_track_info = { 0 };
Tracker_Time_info_T core0_tracker_time_xml = { 0 };
TrackerIn_Sensor_Det_XML_T core0_tracker_xml = { 0 };
Ego_Vehicle_Info_Core0_XML_T core0_Ego_vehicle_xml = { 0 };
Core_0_Timing_XML_Info_T core0_time_xml = { 0 };
All_Objects_Log_XML_T ALL_object_xml = { 0 };
Tracker_Info_Log_XML_T p_g_tracker_OAL_ping_xml = { 0 };
Core_1_Timing_Info_XML_T Core1_Timimg_info_xml = { 0 };

//-------------Traton_SRR6P--------//
TRATON_Detection_Log_Data_T TRATON_Dets_Info = { 0 };
TRATON_Vehicle_Info_Logging_T p_veh_traton = { 0 };
//Traton Blockage 
TRATON_Blockage_Output_Log_Data_T p_blockage_traton_Info = { 0 };
TRATON_F360_Object_Logging_Output_T p_tracker_traton_Info = { 0 };






//All_Objects_Log_XML_T p_g_tracker_OAL_ping_xml = {0};
FF_SM_LOG_XML_T p_g_SM_ping_xml = { 0 };
Core3_XML_Timing_Info_T p_g_timeinfo_xml = { 0 };
FF_OUTPUT_XML_Data_T p_g_FF_ping_xml = { 0 };
LCDA_Output_Logging_T p_Honda_FF_Xml = { 0 }; //LCDA HONDA FF
CTA_Output_Logging_T p_Honda_CTA_FF_Xml{ 0 }; //CTA HONDA FF
CED_Output_Logging_T p_Honda_CED_FF_Xml{ 0 }; //CED HONDA FF
C2_Cust_Feacture_Functions_Gen5_T p_g_FF_ping_xml_Gen5 = { 0 };
Z7B_Cust_Feacture_Functions_T p_g_FF_ping_xml_TML_Gen5 = { 0 };
BMW_LOW_CTA_FF_Logging_T BMW_LOW_CTA_DATA = { 0 };
BMW_LOW_LCDA_FF_Logging_T BMW_LOW_LCDA_DATA = { 0 };
BMW_LOW_RECW_FF_Logging_T BMW_LOW_RECW_DATA = { 0 };
BMW_LOW_CED_FF_Logging_T BMW_LOW_CED_DATA = { 0 };
BMW_LOW_fs_stat_obj_objectpool_Logging_T BMW_LOW_FS = { 0 };
BMW_LOW_TA_FF_Logging_T BMW_LOW_TA = { 0 };

//BOOL f_osi_enabled = 0;
//BOOL f_z7b_enabled = 0;
//BOOL f_z7a_enabled = 0;
//BOOL f_z4_enabled = 0;
//BOOL f_cdc_enabled = 0;
//BOOL f_z4_cust_enabled = 0;
//BOOL f_z7b_custom_enabled = 0;
 BOOL f_dspace_enabled = 0;
//BOOL f_z4_custom_enabled = 0;
//BOOL f_ECU0_enabled = 0;
//BOOL f_ECU1_enabled = 0;
//BOOL f_ECU3_enabled = 0;
//BOOL f_ECU_VRU_Classifier_enabled = 0;
//BOOL f_c0_core_enabled = 0;
//BOOL f_c0_cust_enabled = 0;
//BOOL f_c1_core_enabled = 0;
//BOOL f_c2_core_enabled = 0;
//BOOL f_c2_cust_enabled = 0;

bool DetectionsFlag = false;	//for Self_Dets
bool DetectionsOppFlag = false; // for Opp_Dets


float32_T *array_data1;
int compare1(const void *a, const void *b)
{
	unsigned8_T value_of_a = *(unsigned8_T *)a;
	unsigned8_T value_of_b = *(unsigned8_T *)b;
	return array_data1[value_of_a] < array_data1[value_of_b] ? -1 : array_data1[value_of_a] > array_data1[value_of_b];


}
/*array index sort*/
float32_T *array_data;
uint8_T raw_srr_dets_indx = 0;
int compare(const void *a, const void *b)
{
	//	DVSU_RECORD_T record = {0};
	//Customer_T custId;
	//custId = (Customer_T)record.payload[23];
	unsigned8_T value_of_a = *(unsigned8_T *)a;
	unsigned8_T value_of_b = *(unsigned8_T *)b;
	if (array_data[value_of_a] < array_data[value_of_b]) {
		return -1;
	}
	else if (array_data[value_of_a] == array_data[value_of_b])
	{
		if (DetectionsFlag == true)
		{
			float32_T val_a = Dets_Opp_XML.dets_float[value_of_a].azimuth_raw;
			float32_T val_b = Dets_Opp_XML.dets_float[value_of_b].azimuth_raw;
			return val_a < val_b ? -1 : val_a > val_b;
		}
	}
	else
	{
		return (array_data[value_of_a] > array_data[value_of_b]);
	}
}

static char* GetCustName(uint8_t custID, int platfrom) {
		static char customerName[40] = { 0 };
	if ((custID == STLA_SCALE1 || custID == STLA_SCALE3 || custID == STLA_SCALE4) && (platfrom == UDP_SOURCE_CUST_DSPACE))
	{
		switch (custID)
		{
		case STLA_SCALE1:sprintf(customerName, "STLA_SCALE1"); break;
		case STLA_SCALE3:sprintf(customerName, "STLA_SCALE3"); break;
		case STLA_SCALE4:sprintf(customerName, "STLA_SCALE4"); break;
		}
	}
	else
	{
		switch (custID) {
		case RNA_SUV: sprintf(customerName, "RNA_SUV"); break;
		case RNA_CDV: sprintf(customerName, "RNA_CDV"); break;
		case HKMC_SRR5: sprintf(customerName, "HKMC_SRR5"); break;
		case GWM_SRR5: sprintf(customerName, "GWM_SRR5"); break;
		case FORD_SRR5: sprintf(customerName, "FORD_SRR5"); break;
		case GEELY_SRR5: sprintf(customerName, "GEELY_SRR5"); break;
		case BMW_LOW: sprintf(customerName, "BMW_LOW"); break;
		case BMW_SAT: sprintf(customerName, "BMW_SAT"); break;
		case BMW_BPIL: sprintf(customerName, "BMW_BPIL"); break;
		case SCANIA_MAN:sprintf(customerName, "MAN_SRR3"); break;
		case NISSAN_GEN5:sprintf(customerName, "NISSAN_GEN5"); break;
		case HONDA_GEN5:sprintf(customerName, "HONDA_GEN5"); break;
		case TML_SRR5:sprintf(customerName, "TML_SRR5"); break;
		case PLATFORM_GEN5:sprintf(customerName, "PLATFORM_GEN5"); break;
		case MOTIONAL_FLR4P:sprintf(customerName, "MOTIONAL_FLR4P"); break;
		case STLA_SRR6P:sprintf(customerName, "STLA_SRR6P"); break;
		case STLA_FLR4:sprintf(customerName, "STLA_FLR4"); break;
		case STLA_FLR4P:sprintf(customerName, "STLA_FLR4P"); break;
		case RIVIAN_SRR6P:sprintf(customerName, "%d_RIVIAN_SRR6P"); break;
		case PLATFORM_GEN7: sprintf(customerName, "PLATFORM_SRR7P"); break;
		default: sprintf(customerName, "INVALID_CUSTOMER"); break;
		}
	}
	return customerName;
}
//-------------------gen5-------------//

unsigned int *gen5_scan_index;

void Gen5_Input_Header_Info(unsigned8_T Radar_Posn, const char* LogFname, Customer_T custId, IRadarStream * g_pIRadarStrm)
{
	char InputLogName[1024] = { 0 };
	int platfrom = g_pIRadarStrm->m_gen5proc_info.frame_header.Platform;
	strcpy(InputLogName, LogFname);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ECU_DATA_XML, ECU_DATA);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&HOST_DATA, HOST_VERSION);

	if (!flag_out)
	{
		fprintf(SIL_XML_out, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		fprintf(SIL_XML_out, "<!-- SRR6 APTIV SIL component configuration -->\n");
		fprintf(SIL_XML_out, "<INPUT_HEADER>");
		char *newline = strchr(pathname_XML, '\n');
		if (newline)
			*newline = 0;

		fprintf(SIL_XML_out, "\n<FILE_NAME>%s</FILE_NAME>\n", InputLogName);
		time_t current_time;
		struct tm* timeinfo;
		time(&current_time);
		timeinfo = localtime(&current_time);
		fprintf(SIL_XML_out, "\t<PC_TIME>%d:%d:%d</PC_TIME>\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		fprintf(SIL_XML_out, "\t<CUSTOMER_NAME>\t %s \t</CUSTOMER_NAME>\n", GetCustName(custId, platfrom));

		//	fprintf(SIL_XML_out, "\t<SW_VERSION>%d-%d-%d-%d\t</SW_VERSION>\n", HOST_DATA.Host_Sw_Version[0], HOST_DATA.Host_Sw_Version[1], HOST_DATA.Host_Sw_Version[2], HOST_DATA.Host_Sw_Version[3]);
		//	fprintf(SIL_XML_out, "\t<ECU_SERIAL_NO>\t%d-%d-%d-%d\t</ECU_SERIAL_NO>\n", ECU_DATA_XML.ECU_Serial_Num[0], ECU_DATA_XML.ECU_Serial_Num[1], ECU_DATA_XML.ECU_Serial_Num[2], ECU_DATA_XML.ECU_Serial_Num[3]);
		fprintf(SIL_XML_out, "</INPUT_HEADER>\n");
		flag_out = TRUE;
	}
}

/*Print Opp Dets*/
void Gen5_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Gen5_target_info, GEN5_TARGET_INFO);
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_OppDets_xml, DETS_OPP);

		/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
		g_pIRadarStrm->getData((unsigned char*)&gen5_OppDets_xml,DET_OPP);*/
#ifndef DISABLE_TRACKER

		float32_T Opp_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			Opp_Dets[i] = gen5_OppDets_xml.dets_float[i].range;
		}
		unsigned8_T size = sizeof(Opp_Dets) / sizeof(*Opp_Dets);
		unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Opp_Dets;
		DetectionsOppFlag = true;
		qsort(index, size, sizeof(*index), compare);
		DetectionsOppFlag = false;

		/*Opp Dets validity levels count*/
		static unsigned8_T Valid = 0;
		static unsigned8_T Invalid = 0;
		static unsigned8_T HVC = 0;
		static unsigned8_T Low_SNR = 0;
		static unsigned8_T Angle_Jump = 0;
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			unsigned8_T valid_level = gen5_OppDets_xml.dets_float[index[i]].valid_level;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		if (Valid != 0 && Invalid != 0 && HVC == 0 && Angle_Jump != 0 && Low_SNR != 0)
		{
			fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
			fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
			fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
			fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
			fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
			Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;
		}
	

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			/*check for valid level of opp dets*/
			if (((gen5_OppDets_xml.dets_float[index[i]].valid_level) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude,\tdetection_snr,\tvalid_level\t</DETECTION_INFO>\n");
				count = 0;
			}
			if ((gen5_OppDets_xml.dets_float[index[i]].valid_level) > 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");

				fprintf(SIL_XML_out, "\t%f", gen5_OppDets_xml.dets_float[index[i]].range);
				fprintf(SIL_XML_out, "\t%f", gen5_OppDets_xml.dets_float[index[i]].range_rate_raw);

				fprintf(SIL_XML_out, "\t%f", gen5_OppDets_xml.dets_float[index[i]].azimuth_raw);

				fprintf(SIL_XML_out, "\t%f", gen5_OppDets_xml.dets_float[index[i]].amplitude);

				fprintf(SIL_XML_out, "\t%f", gen5_OppDets_xml.dets_float[index[i]].detection_snr);

				unsigned8_T valid_level = gen5_OppDets_xml.dets_float[index[i]].valid_level;
				switch (valid_level)//print enum of validity level 
				{
				case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
				case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
				case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
				case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
				case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
				case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
				case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
				case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
				default:fprintf(SIL_XML_out, "\tDNE\t"); break;
				}
				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_END_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
		}
#endif
	
}

void Gen5_Populate_Valid_Data_to_XML(FILE* filename, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, uint32_t scanindex, uint32_t platform)
{
	char  inp_file[1024] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	Open_XML_file(filename);
	Open_CSV_file(filename);
	g_scanindex = scanindex;
	//(_pIRadarStrm->get_complete_Buffer());
	if (CreateOnehdr)
	{
		Gen5_Input_Header_Info(Radar_Posn, InputLogName, custId, g_pIRadarStrm);
		CreateOnehdr = false;
	}
	if (g_pIRadarStrm)
	{

		if (platform == UDP_PLATFORM_SRR6 || platform == UDP_PLATFORM_SRR6_PLUS || platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN || platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || platform == UDP_PLATFORM_STLA_FLR4)
		{

			if (stream_num == C0_CORE_MASTER_STREAM)
			{
				if (custId != STLA_SRR6P && custId != STLA_FLR4 && custId != PLATFORM_GEN5)
				{
					//fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", scanindex, GetRadarPosName(Radar_Posn));
					//Gen5_Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
					Gen5_Opposite_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
				}
			}
			if (stream_num == C2_CORE_MASTER_STREAM)// || stream_num == e_DET_LOGGING_STREAM)
			{
				fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
				if (custId == STLA_SRR6P || custId == PLATFORM_GEN5 ) //For dets STLA_SRR6P and PLatform srr6p 
				{
					STLA_SRR6P_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);  //STLA_SRR6P Detections CYW-2730, platform srr6p detections structure also same as stla srr6p
				}

				if (custId == SCANIA_MAN)
				{
					TRATON_SRR6P_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);

				}
				if (custId == STLA_FLR4)  //For dets STLA_FLR4
				{
					STLA_FLR4_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);  //STLA_FLR4 Detections CYW-3354
				}
				if (custId != STLA_SRR6P && custId != STLA_FLR4 && custId != PLATFORM_GEN5 && custId != SCANIA_MAN)
				{
					Gen5_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
				}
				if (custId == PLATFORM_GEN5 && platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
				{
					FLR4P_Curvi_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId); // Platform srr6p trackes structure same as flr4p trackes structure.
				}
				Gen5_Curvi_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId);
				fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
			}
			if (stream_num == C2_CUST_MASTER_STREAM)
			{
				//fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", scanindex, GetRadarPosName(Radar_Posn));
				FeatureFunction_Info(g_pIRadarStrm, custId, Radar_Posn); // //Feacture Function info for NISSAN and HONDA
				//Gen5_Record_Header_Count(Radar_Posn, g_pIRadarStrm);
			}

		}
		else if (platform == UDP_PLATFORM_FLR4_PLUS || platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
		{
			if ((stream_num == e_DET_LOGGING_STREAM) || (stream_num == e_TRACKER_LOGGING_STREAM))
			{
				fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
				if (stream_num == e_DET_LOGGING_STREAM)
				{
					Gen5_FLR4P_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
				}
				if (stream_num == e_TRACKER_LOGGING_STREAM)
				{
					if (custId == PLATFORM_GEN5)
					{
						FLR4P_Curvi_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId);
					}
					if (custId == STLA_FLR4P)
					{
						STLA_FLR4P_Curvi_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId);
					}
					fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
				}
			}
		}
	}
}
/*------------------------- GEN7----------------------------------------*/
void Gen7_Input_Header_Info(unsigned8_T Radar_Posn, const char* LogFname, Customer_T custId, IRadarStream* g_pIRadarStrm)
{
	char InputLogName[1024] = { 0 };
	int platfrom = g_pIRadarStrm->m_gen7proc_info.frame_header.sourceInfo;
	strcpy(InputLogName, LogFname);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ECU_DATA_XML, ECU_DATA);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&HOST_DATA, HOST_VERSION);

	if (!flag_out)
	{
		fprintf(SIL_XML_out, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		fprintf(SIL_XML_out, "<!-- SRR7 APTIV SIL component configuration -->\n");
		fprintf(SIL_XML_out, "<INPUT_HEADER>");
		char* newline = strchr(pathname_XML, '\n');
		if (newline)
			*newline = 0;

		fprintf(SIL_XML_out, "\n<FILE_NAME>%s</FILE_NAME>\n", InputLogName);
		time_t current_time;
		struct tm* timeinfo;
		time(&current_time);
		timeinfo = localtime(&current_time);
		fprintf(SIL_XML_out, "\t<PC_TIME>%d:%d:%d</PC_TIME>\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		fprintf(SIL_XML_out, "\t<CUSTOMER_NAME>\t %s \t</CUSTOMER_NAME>\n", GetCustName(custId, platfrom));

		//	fprintf(SIL_XML_out, "\t<SW_VERSION>%d-%d-%d-%d\t</SW_VERSION>\n", HOST_DATA.Host_Sw_Version[0], HOST_DATA.Host_Sw_Version[1], HOST_DATA.Host_Sw_Version[2], HOST_DATA.Host_Sw_Version[3]);
		//	fprintf(SIL_XML_out, "\t<ECU_SERIAL_NO>\t%d-%d-%d-%d\t</ECU_SERIAL_NO>\n", ECU_DATA_XML.ECU_Serial_Num[0], ECU_DATA_XML.ECU_Serial_Num[1], ECU_DATA_XML.ECU_Serial_Num[2], ECU_DATA_XML.ECU_Serial_Num[3]);
		fprintf(SIL_XML_out, "</INPUT_HEADER>\n");
		flag_out = TRUE;
	}
}


void Gen7_Populate_Valid_Data_to_XML(FILE* filename, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, uint32_t scanindex, uint32_t platform)
{
	char  inp_file[1024] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	Open_XML_file(filename);
	Open_CSV_file(filename);
	g_scanindex = scanindex;
	//(_pIRadarStrm->get_complete_Buffer());
	if (CreateOnehdr)
	{
		Gen7_Input_Header_Info(Radar_Posn, InputLogName, custId, g_pIRadarStrm);
		CreateOnehdr = false;
	}
	if (g_pIRadarStrm)
	{

		if (platform == UDP_PLATFORM_SRR7_PLUS || platform == UDP_PLATFORM_SRR7_PLUS_HD || platform == UDP_PLATFORM_SRR7_E)
		{

			if ((stream_num == DETECTION_STREAM) || (stream_num == ROT_OBJECT_STREAM) ||  (stream_num == ALIGNMENT_STREAM))
			{
				fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
				if (stream_num == DETECTION_STREAM)
				{
					GEN7_Radar_Self_Info(Radar_Posn, g_pIRadarStrm, custId);
				}
			}
			if ((stream_num == ROT_OBJECT_STREAM))
			{
				 Gen7_SRR7P_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId);

			}
			if (stream_num == ALIGNMENT_STREAM)
			{
				GEN7_Alignment_Info(Radar_Posn, g_pIRadarStrm, custId);
			}
					fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
		}
	}
}
	


void Gen5_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	/*char  Temp[MAX_FILEPATH_NAME_LENGTH] = "C:\\New_folder\\test.csv";
	
	SIL_CSV_out = fopen(Temp, "w");*/
	
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_Dets_xml, DETS_COMP);
	/*m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Radar_Info_XML, RADAR_PARAM);*/
	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		Self_Dets[i] = gen5_Dets_xml.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	
		array_data1 = Self_Dets;
		qsort(index, size, sizeof(*index), compare1);
	/*Self Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		unsigned8_T valid_level = gen5_Dets_xml.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	if (Valid != 0 && Invalid != 0 && HVC == 0 && Angle_Jump != 0 && Low_SNR != 0)
	{
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;
	}

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		/*check for valid detection*/

		if (((gen5_Dets_xml.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,\t,valid_level\t,det_id\t,elevation_raw\t,multi_target_probability\t,rdd_fp_det_idx\t,snr\t,std_azimuth_angle\t,std_elevation_angle\t,std_range\t,std_rcs\t,std_r_dot\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((gen5_Dets_xml.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");
			//char strData[20];
			//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].range);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].azimuth_raw);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].amplitude);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].detection_snr, strData, FIXED); 
			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].detection_snr);
			unsigned8_T valid_level = gen5_Dets_xml.dets_float[index[i]].valid_level;

			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t%f", gen5_Dets_xml.dets_float[index[i]].det_id);

			fprintf(SIL_XML_out, "\t%f", gen5_Dets_xml.dets_float[index[i]].elevation_raw);

			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].multi_target_probability);

			fprintf(SIL_XML_out, "\t\t\t%f", gen5_Dets_xml.dets_float[index[i]].rdd_fp_det_idx);

			fprintf(SIL_XML_out, "\t\t%d", gen5_Dets_xml.dets_float[index[i]].snr);

			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].std_azimuth_angle);

			fprintf(SIL_XML_out, "\t\t%f", gen5_Dets_xml.dets_float[index[i]].std_elevation_angle);

			fprintf(SIL_XML_out, "\t\t\t%f", gen5_Dets_xml.dets_float[index[i]].std_range);

			fprintf(SIL_XML_out, "\t%f", gen5_Dets_xml.dets_float[index[i]].std_rcs);

			fprintf(SIL_XML_out, "\t%f", gen5_Dets_xml.dets_float[index[i]].std_r_dot);

			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
	}
	// Adding Alignment for xml printing here 03/10/2024 -- CYX-4410 --

	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&honda_srr6p_Alignment_xml, Alignment_Data);
	fprintf(SIL_XML_out, "\n\t<ALIGNMENT_DESCRIPTION_INFO_START_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n\t\t<SELF_ALIGNMENT_DESCRIPTION_INFO>\n");
	fprintf(SIL_XML_out, "\tAngle_Slow_Filter \t%f\n", honda_srr6p_Alignment_xml.Align_Info.align_angle_slow_filter);
	fprintf(SIL_XML_out, "\tAngle_Time_Counter \t%u\n", honda_srr6p_Alignment_xml.Align_Info.align_time_counter);
	fprintf(SIL_XML_out, "\tK_Unused16_1 \t%u\n", honda_srr6p_Alignment_xml.Align_Info.K_Unused16_1);	
	fprintf(SIL_XML_out, "\town_align_angle\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle);
	fprintf(SIL_XML_out, "\town_align_angle_elev\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_elev);
	fprintf(SIL_XML_out, "\town_align_angle_raw\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_raw);
	fprintf(SIL_XML_out, "\town_align_angle_raw_elev\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_raw_elev);
	fprintf(SIL_XML_out, "\town_align_angle_ref \t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_ref);
	fprintf(SIL_XML_out, "\town_align_angle_ref_elev\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_ref_elev);
	fprintf(SIL_XML_out, "\town_align_angle_saved \t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_saved);
	fprintf(SIL_XML_out, "\town_align_angle_saved_elev\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_align_angle_saved_elev);
	fprintf(SIL_XML_out, "\town_align_num_attempts\t%u\n", honda_srr6p_Alignment_xml.Align_Info.own_align_num_attempts);
	fprintf(SIL_XML_out, "\town_align_num_attempts_elev\t%u\n", honda_srr6p_Alignment_xml.Align_Info.own_align_num_attempts_elev);
	fprintf(SIL_XML_out, "\town_align_quality_factor\t%u\n", honda_srr6p_Alignment_xml.Align_Info.own_align_quality_factor);
	fprintf(SIL_XML_out, "\town_align_quality_factor_elev\t%u\n", honda_srr6p_Alignment_xml.Align_Info.own_align_quality_factor_elev);
	fprintf(SIL_XML_out, "\tOwn_align_Saved_SCF\t%f\n", honda_srr6p_Alignment_xml.Align_Info.Own_align_Saved_SCF);
	fprintf(SIL_XML_out, "\tOwn_align_SCF\t%f\n", honda_srr6p_Alignment_xml.Align_Info.Own_align_SCF);
	fprintf(SIL_XML_out, "\town_align_state\t%u\n", honda_srr6p_Alignment_xml.Align_Info.own_align_state);
	fprintf(SIL_XML_out, "\town_deltaRdot\t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_deltaRdot);
	bool own_f_misaligned_error = honda_srr6p_Alignment_xml.Align_Info.own_f_misaligned_error;
	switch (own_f_misaligned_error)
	{
	case 0:fprintf(SIL_XML_out, "\n\t\town_f_misaligned_error\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\town_f_misaligned_error\t\t\tTRUE"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	bool own_f_misaligned_error_elev = honda_srr6p_Alignment_xml.Align_Info.own_f_misaligned_error_elev;
	switch (own_f_misaligned_error_elev)
	{
	case 0:fprintf(SIL_XML_out, "\n\t\town_f_misaligned_error_elev\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\town_f_misaligned_error_elev\t\t\tTRUE"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	fprintf(SIL_XML_out, "\town_rmax \t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_rmax);
	fprintf(SIL_XML_out, "\town_rrmax \t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_rrmax);
	fprintf(SIL_XML_out, "\town_v_un \t%f\n", honda_srr6p_Alignment_xml.Align_Info.own_v_un);

	fprintf(SIL_XML_out, "\n\t\t<SELF_ALIGNMENT_DESCRIPTION_INFO_END>\n");

	fprintf(SIL_XML_out, "\n\t\t<OPP_ALIGNMENT_DESCRIPTION_INFO_START>\n");

	fprintf(SIL_XML_out, "\topp_align_angle \t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_align_angle);
	fprintf(SIL_XML_out, "\topp_align_angle_elev \t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_align_angle_elev);
	fprintf(SIL_XML_out, "\topp_align_quality_factor \t%u\n", honda_srr6p_Alignment_xml.Align_Info.opp_align_quality_factor);
	fprintf(SIL_XML_out, "\topp_align_quality_factor_elev \t%u\n", honda_srr6p_Alignment_xml.Align_Info.opp_align_quality_factor_elev);
	fprintf(SIL_XML_out, "\topp_deltaRdot\t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_deltaRdot);
	fprintf(SIL_XML_out, "\topp_look_id\t%u\n", honda_srr6p_Alignment_xml.Align_Info.opp_look_id);
	fprintf(SIL_XML_out, "\topp_look_type \t%u\n", honda_srr6p_Alignment_xml.Align_Info.opp_look_type);
	fprintf(SIL_XML_out, "\topp_rmax\t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_rmax);
	fprintf(SIL_XML_out, "\topp_rrmax\t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_rrmax);
	fprintf(SIL_XML_out, "\topp_ScanIndex \t%u\n", honda_srr6p_Alignment_xml.Align_Info.opp_ScanIndex);
	fprintf(SIL_XML_out, "\topp_v_un\t%f\n", honda_srr6p_Alignment_xml.Align_Info.opp_v_un);

	fprintf(SIL_XML_out, "\n\t\t<OPP_ALIGNMENT_DESCRIPTION_INFO_END>\n");

	fprintf(SIL_XML_out, "\n\t<ALIGNMENT_INFO_END_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	
}

//TRATON_SRR6P Detections CYW-5013.
void TRATON_SRR6P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&TRATON_Dets_Info, DETS_COMP);
	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_DETS_MAN] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETS_MAN; i++)
	{
		Self_Dets[i] = TRATON_Dets_Info.target_report[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETS_MAN] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	fprintf(SIL_XML_out, "\t<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	unsigned8_T Self_LookType = TRATON_Dets_Info.look_type;
	switch (Self_LookType)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookType></LookType>"); break;//do nothing
	}
	unsigned8_T Self_LookID = TRATON_Dets_Info.lookindex;
	switch (Self_LookID)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-MEDIUM_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-LONG_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookIndex></LookIndex>"); break;//do nothing
	}
	//fprintf(SIL_XML_out, "\n\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
	fprintf(SIL_XML_out, "\t</TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	//VEH_DATA_INFO

	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_traton, GPP_VEH_COMP);
	fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\tspeed \t%f\n", p_veh_traton.raw_speed);
	fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh_traton.raw_yaw_rate_rad);
	fprintf(SIL_XML_out, "\tf_reverse\t %u\n", p_veh_traton.f_reverse_gear);
	fprintf(SIL_XML_out, "\tvcs_lat_acc\t %f\n", p_veh_traton.vcs_lat_acceleration);
	fprintf(SIL_XML_out, "\tvcs_long_acc \t%f\n", p_veh_traton.vcs_long_acceleration);
	fprintf(SIL_XML_out, "\tcurvature_rear\t%f\n", p_veh_traton.curvature_rear);
	fprintf(SIL_XML_out, "\tsteering_angle_rad \t%f\n", p_veh_traton.steering_angle_rad);
	fprintf(SIL_XML_out, "\ttimestamp_s\t%f\n", p_veh_traton.timestamp_s);
	fprintf(SIL_XML_out, "\tcurvature_rear\t %f\n", p_veh_traton.curvature_rear);
	fprintf(SIL_XML_out, "\tprndl\t\t\t\t");
	unsigned8_T prndl = p_veh_traton.prndl;
	switch (prndl)
	{
	case 0:fprintf(SIL_XML_out, "PARK"); break;
	case 1:fprintf(SIL_XML_out, "REVERSE"); break;
	case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
	case 3:fprintf(SIL_XML_out, "DRIVE"); break;
	case 4:fprintf(SIL_XML_out, "FOURTH"); break;
	case 5:fprintf(SIL_XML_out, "THIRD"); break;
	case 6:fprintf(SIL_XML_out, "LOW"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}

	unsigned8_T f_reverse_gear = p_veh_traton.f_reverse_gear;
	switch (f_reverse_gear)//enum of f_reverse_gear
	{
	case 0:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
	case 1:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
	default:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tINVALID"); break;
	}
	fprintf(SIL_XML_out, "\t</VEH_DATA_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	//MNR BLOCKAGE Info

	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_blockage_traton_Info, BLOCKAGE_DATA);
	fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_DATA_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\tAvgPm_mnr_avg\t%f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.AvgPm_mnr_avg);
	fprintf(SIL_XML_out, "\tAvgRm_mnr_avg\t%f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.AvgRm_mnr_avg);
	fprintf(SIL_XML_out, "\tbeta_var\t %f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.beta_var);
	fprintf(SIL_XML_out, "\tbeta_var_si\t%f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.beta_var_si);
	//fprintf(SIL_XML_out, "\tBlocked\t%d\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Blocked);
	fprintf(SIL_XML_out, "\tBmnr1\t%u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmnr1);
	fprintf(SIL_XML_out, "\tBmnr2 \t%u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmnr2);
	fprintf(SIL_XML_out, "\tBmnr3\t%u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmnr3);
	fprintf(SIL_XML_out, "\tBmnr4\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmnr4);
	fprintf(SIL_XML_out, "\tBmodmnr1\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmodmnr1);
	fprintf(SIL_XML_out, "\tBmodmnr2\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmodmnr2);
	fprintf(SIL_XML_out, "\tBmodmnr3\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmodmnr3);
	fprintf(SIL_XML_out, "\tBmodmnr4\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Bmodmnr4);
	fprintf(SIL_XML_out, "\tbm_Blocked\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.bm_Blocked);
	fprintf(SIL_XML_out, "\tbm_Degraded\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.bm_Degraded);
	//fprintf(SIL_XML_out, "\tDegraded\t %d\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Degraded);
	//fprintf(SIL_XML_out, "\tForward\t %d\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.Forward);
	fprintf(SIL_XML_out, "\tk_unused_bits2\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.k_unused_bits2);
	fprintf(SIL_XML_out, "\tMaxPm_mnr_avg\t %f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.MaxPm_mnr_avg);
	fprintf(SIL_XML_out, "\tMaxRm_mnr_avg\t %f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.MaxRm_mnr_avg);
	fprintf(SIL_XML_out, "\trange_check_blockage\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.range_check_blockage);
	fprintf(SIL_XML_out, "\trsvd\t %u\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.rsvd);
	fprintf(SIL_XML_out, "\tV_host\t %f\n", p_blockage_traton_Info.Traton_MNR_Blockage_data.V_host);
	

	bool Blocked = p_blockage_traton_Info.Traton_MNR_Blockage_data.Blocked;
	switch (Blocked)//enum of turn_signal
	{
		case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
		case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
		default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	bool Degraded = p_blockage_traton_Info.Traton_MNR_Blockage_data.Degraded;
	switch (Degraded)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tDegraded\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tDegraded\t\t\tTRUE"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	bool Forward = p_blockage_traton_Info.Traton_MNR_Blockage_data.Forward;
	switch (Forward)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}

	// TRACKER Info
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_tracker_traton_Info, TRACK_COMP_CORE);
	fprintf(SIL_XML_out, "\n\t<Tracker Info_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	for (int i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++) 
	{
		fprintf(SIL_XML_out, "\taccuracy_length\t%f\n", p_tracker_traton_Info.object[i].accuracy_length);
		fprintf(SIL_XML_out, "\taccuracy_width\t%f\n", p_tracker_traton_Info.object[i].accuracy_width);
		fprintf(SIL_XML_out, "\tconfidenceLevel\t %f\n", p_tracker_traton_Info.object[i].confidenceLevel);
		fprintf(SIL_XML_out, "\tcurrent_msmt_type\t %u\n", p_tracker_traton_Info.object[i].current_msmt_type);
		fprintf(SIL_XML_out, "\tcurvature\t %f\n", p_tracker_traton_Info.object[i].curvature);
		fprintf(SIL_XML_out, "\texistence_probability\t %f\n", p_tracker_traton_Info.object[i].existence_probability);
		fprintf(SIL_XML_out, "\tf_crossing\t %u\n", p_tracker_traton_Info.object[i].f_crossing);
		fprintf(SIL_XML_out, "\tf_fast_moving\t %u\n", p_tracker_traton_Info.object[i].f_fast_moving);
		fprintf(SIL_XML_out, "\tf_moveable\t %u\n", p_tracker_traton_Info.object[i].f_moveable);
		fprintf(SIL_XML_out, "\tf_moving\t %u\n", p_tracker_traton_Info.object[i].f_moving);
		fprintf(SIL_XML_out, "\tf_oncoming\t %u\n", p_tracker_traton_Info.object[i].f_oncoming);
		fprintf(SIL_XML_out, "\tf_onguardrail\t %u\n", p_tracker_traton_Info.object[i].f_onguardrail);
		fprintf(SIL_XML_out, "\tf_vehicular_trk\t %u\n", p_tracker_traton_Info.object[i].f_vehicular_trk);
		fprintf(SIL_XML_out, "\tinit_scheme\t %u\n", p_tracker_traton_Info.object[i].init_scheme);
		fprintf(SIL_XML_out, "\tK_Unused16_1\t %u\n", p_tracker_traton_Info.object[i].K_Unused16_1);
		fprintf(SIL_XML_out, "\tK_Unused8_1\t %u\n", p_tracker_traton_Info.object[i].K_Unused8_1);
		fprintf(SIL_XML_out, "\tlen1\t %f\n", p_tracker_traton_Info.object[i].len1);
		fprintf(SIL_XML_out, "\tlen2\t %f\n", p_tracker_traton_Info.object[i].len2);
		fprintf(SIL_XML_out, "\tmovable_prob\t %f\n", p_tracker_traton_Info.object[i].movable_prob);
		fprintf(SIL_XML_out, "\tndets\t %u\n", p_tracker_traton_Info.object[i].ndets);
		fprintf(SIL_XML_out, "\tnum_dets_used_in_rr_msmt_update\t %u\n", p_tracker_traton_Info.object[i].num_dets_used_in_rr_msmt_update);
		fprintf(SIL_XML_out, "\tnum_rr_inlier_dets\t %u\n", p_tracker_traton_Info.object[i].num_rr_inlier_dets);
		fprintf(SIL_XML_out, "\tobject_class\t %u\n", p_tracker_traton_Info.object[i].object_class);
		fprintf(SIL_XML_out, "\totg_height\t %f\n", p_tracker_traton_Info.object[i].otg_height);
		fprintf(SIL_XML_out, "\tprobability_bicycle\t %f\n", p_tracker_traton_Info.object[i].probability_bicycle);
		fprintf(SIL_XML_out, "\tprobability_car\t %f\n", p_tracker_traton_Info.object[i].probability_car);
		fprintf(SIL_XML_out, "\tprobability_motorcycle\t %f\n", p_tracker_traton_Info.object[i].probability_motorcycle);
		fprintf(SIL_XML_out, "\tprobability_pedestrian\t %f\n", p_tracker_traton_Info.object[i].probability_pedestrian);
		fprintf(SIL_XML_out, "\tprobability_truck\t %f\n", p_tracker_traton_Info.object[i].probability_truck);
		fprintf(SIL_XML_out, "\tprobability_underdrivable\t %f\n", p_tracker_traton_Info.object[i].probability_underdrivable);
		fprintf(SIL_XML_out, "\tprobability_undet\t %f\n", p_tracker_traton_Info.object[i].probability_undet);
		fprintf(SIL_XML_out, "\treducedID\t %u\n", p_tracker_traton_Info.object[i].reducedID);
		fprintf(SIL_XML_out, "\treducedStatus\t %u\n", p_tracker_traton_Info.object[i].reducedStatus);
		fprintf(SIL_XML_out, "\treference_point\t %u\n", p_tracker_traton_Info.object[i].reference_point);
		fprintf(SIL_XML_out, "\tspeed\t %f\n", p_tracker_traton_Info.object[i].speed);
		for (int j = 0; j < SIX; j++) {
			fprintf(SIL_XML_out, "\tstate_variance\t %f\n", p_tracker_traton_Info.object[i].state_variance[j]);
		}
		
		fprintf(SIL_XML_out, "\tstatus\t %u\n", p_tracker_traton_Info.object[i].status);
		for (int k = 0; k < THREE; k++) {
			fprintf(SIL_XML_out, "\tsupplemental_state_covariance\t %f\n", p_tracker_traton_Info.object[i].supplemental_state_covariance[k]);
		}
		
		fprintf(SIL_XML_out, "\ttang_accel\t %f\n", p_tracker_traton_Info.object[i].tang_accel);
		fprintf(SIL_XML_out, "\ttime_since_cluster_created\t %f\n", p_tracker_traton_Info.object[i].time_since_cluster_created);
		fprintf(SIL_XML_out, "\ttime_since_measurement\t %f\n", p_tracker_traton_Info.object[i].time_since_measurement);
		fprintf(SIL_XML_out, "\ttime_since_stage_start\t %f\n", p_tracker_traton_Info.object[i].time_since_stage_start);
		fprintf(SIL_XML_out, "\ttime_since_track_updated\t %f\n", p_tracker_traton_Info.object[i].time_since_track_updated);
		fprintf(SIL_XML_out, "\ttrkID\t %u\n", p_tracker_traton_Info.object[i].trkID);
		fprintf(SIL_XML_out, "\ttrk_fltr_type\t %u\n", p_tracker_traton_Info.object[i].trk_fltr_type);
		fprintf(SIL_XML_out, "\tunderdrivable_status\t %u\n", p_tracker_traton_Info.object[i].underdrivable_status);
		fprintf(SIL_XML_out, "\tunique_id\t %u\n", p_tracker_traton_Info.object[i].unique_id);
		fprintf(SIL_XML_out, "\tvcs_heading\t %f\n", p_tracker_traton_Info.object[i].vcs_heading);
		fprintf(SIL_XML_out, "\tvcs_pointing\t %f\n", p_tracker_traton_Info.object[i].vcs_pointing);
		fprintf(SIL_XML_out, "\tvcs_xaccel\t %f\n", p_tracker_traton_Info.object[i].vcs_xaccel);
		fprintf(SIL_XML_out, "\tvcs_xposn\t %f\n", p_tracker_traton_Info.object[i].vcs_xposn);
		fprintf(SIL_XML_out, "\tvcs_xvel\t %f\n", p_tracker_traton_Info.object[i].vcs_xvel);
		fprintf(SIL_XML_out, "\tvcs_yaccel\t %f\n", p_tracker_traton_Info.object[i].vcs_yaccel);
		fprintf(SIL_XML_out, "\tvcs_yposn\t %f\n", p_tracker_traton_Info.object[i].vcs_yposn);
		fprintf(SIL_XML_out, "\tvcs_yvel\t %f\n", p_tracker_traton_Info.object[i].vcs_yvel);
		fprintf(SIL_XML_out, "\twid1\t %f\n", p_tracker_traton_Info.object[i].wid1);
		fprintf(SIL_XML_out, "\twid2\t %f\n", p_tracker_traton_Info.object[i].wid2);
	}
	

	

	
//	fprintf(SIL_XML_out, "\n\t<ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
//	fprintf(SIL_XML_out, "\talign_angle_stat_az.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.mean);
//	fprintf(SIL_XML_out, "\talign_angle_stat_az.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.negative_err);
//	fprintf(SIL_XML_out, "\talign_angle_stat_az.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.positive_err);
//	fprintf(SIL_XML_out, "\talign_angle_stat_az.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.std_dev);
//
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.mean);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.negative_err);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.positive_err);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.std_dev);
//
//	fprintf(SIL_XML_out, "\talign_angle_stat_el.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.mean);
//	fprintf(SIL_XML_out, "\talign_angle_stat_el.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.negative_err);
//	fprintf(SIL_XML_out, "\talign_angle_stat_el.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.positive_err);
//	fprintf(SIL_XML_out, "\talign_angle_stat_el.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.std_dev);
//
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.mean);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.negative_err);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.positive_err);
//	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.std_dev);
//
//	fprintf(SIL_XML_out, "\tstatus_align_angle_az \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_az);
//	fprintf(SIL_XML_out, "\tstatus_align_angle_el \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_el);
//	//fprintf(SIL_XML_out, "\t</ALIGNMENT_OUTPUT>\n");
//	fprintf(SIL_XML_out, "\n\t</ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
//	//MNR_BLOCKAGE_PARAMETER
//	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML_STLA, BLOCKAGE_DATA);
//	fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
//	//fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
//	boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Blocked;
//	switch (Blocked)//enum of turn_signal
//	{
//	case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
//	case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
//	default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr4);
//	fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr3);
//	fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr2);
//	fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr1);
//	fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Forward);
//	boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Forward;
//	switch (Forward)//enum of turn_signal
//	{
//	case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
//	case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
//	default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.beta_var_si);
//	fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.beta_var);
//	fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.AvgPm_mnr_avg);
//	fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.AvgRm_mnr_avg);
//	fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.MaxPm_mnr_avg);
//	fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.MaxRm_mnr_avg);
//	fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.V_host);
//	fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
//
//
////	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Diagnostics, Diagnostics_Data);
////	fprintf(SIL_XML_out, "\n\t<Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
//	for (int i = 0; i < 24; i++)
//	{
		//	fprintf(SIL_XML_out, "\tTiming_c2_%d \t%d\n", i, Diagnostics.Timing_Info_c2[i]);
	//	}

		//fprintf(SIL_XML_out, "\t</Diagnostics_INFO>\n");
		//fprintf(SIL_XML_out, "\n\t</Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	//detections
		fprintf(SIL_XML_out, "\n\t<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		// valid detections

		static unsigned8_T Valid = TRATON_Dets_Info.target_count;
		static unsigned8_T Invalid = 0;
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		Invalid = NUMBER_OF_DETS_MAN - Valid;
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);

		/*print sorted self dets*/
		unsigned8_T count = 1;
		for (unsigned16_T i = 0; i < NUMBER_OF_DETS_MAN; i++)
		{
			/*check for  non zero range*/

			if ((TRATON_Dets_Info.target_report[index[i]].range != 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\trange_rate,\tazimuth, \tamplitude,\trcs,\t\tsnr,\texistence_probability,\tbi_static_target,\tsuper_res_target, \tazimuth_std_dev, \televation_std_dev, \tmatch_err_fft, \tvel_un, \tvel_st, \tfirst_pass_index, \tK_Unused16_1, \taz_conf, \tel_conf, \tisSingleTarget, \tisSingleTarget_azimuth, \tf_Doppler_mixed_interval, \tf_valid_Doppler_unfolding, \tbf_type_az, \tbf_type_el, \tsuper_res_target_type, \toutside_sector</DETECTION_INFO>\n");
				count = 0;
			}
			if (TRATON_Dets_Info.target_report[index[i]].range != 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].range);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].range_rate);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].azimuth);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].elevation);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].amp);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].rcs);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].snr);
				fprintf(SIL_XML_out, "\t\t%f", TRATON_Dets_Info.target_report[index[i]].prob_exist);
				fprintf(SIL_XML_out, "\t\t%u", TRATON_Dets_Info.target_report[index[i]].isBistatic);
				fprintf(SIL_XML_out, "\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].super_res_target);
				fprintf(SIL_XML_out, "\t\t\t%f", TRATON_Dets_Info.target_report[index[i]].az_std_dev);
				fprintf(SIL_XML_out, "\t\t\t%f", TRATON_Dets_Info.target_report[index[i]].el_std_dev);
				fprintf(SIL_XML_out, "\t\t\t%f", TRATON_Dets_Info.target_report[index[i]].match_err_fft);
				fprintf(SIL_XML_out, "\t%f", TRATON_Dets_Info.target_report[index[i]].vel_un);
				fprintf(SIL_XML_out, "\t\t%f", TRATON_Dets_Info.target_report[index[i]].vel_st);
				fprintf(SIL_XML_out, "\t\t%hu", TRATON_Dets_Info.target_report[index[i]].first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t%hu", TRATON_Dets_Info.target_report[index[i]].K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%u", TRATON_Dets_Info.target_report[index[i]].az_conf);
				fprintf(SIL_XML_out, "\t%u", TRATON_Dets_Info.target_report[index[i]].el_conf);
				fprintf(SIL_XML_out, "\t\t%u", TRATON_Dets_Info.target_report[index[i]].isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].bf_type_az);
				fprintf(SIL_XML_out, "\t\t%u", TRATON_Dets_Info.target_report[index[i]].bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t\t%u", TRATON_Dets_Info.target_report[index[i]].super_res_target_type);
				fprintf(SIL_XML_out, "\t\t%u", TRATON_Dets_Info.target_report[index[i]].outside_sector);
				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
		}
}



//STLA_SRR6P Detections CYW-2730
void STLA_SRR6P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{	
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&STLA_Dets_info, DETS_COMP);
	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		Self_Dets[i] = STLA_Dets_info.target_report[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	fprintf(SIL_XML_out, "\t<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	unsigned8_T Self_LookType = STLA_Dets_info.look_type;
	switch (Self_LookType)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookType></LookType>"); break;//do nothing
	}
	unsigned8_T Self_LookID = STLA_Dets_info.lookindex;
	switch (Self_LookID)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-MEDIUM_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-LONG_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookIndex></LookIndex>"); break;//do nothing
	}
	//fprintf(SIL_XML_out, "\n\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
	fprintf(SIL_XML_out, "\t</TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//VEH_DATA_INFO
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla, Alignment_Data);
	fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\tabs_speed \t%f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed);
	fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate);
	fprintf(SIL_XML_out, "\tf_reverse\t %f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.f_reverse);
	fprintf(SIL_XML_out, "\tspeed_quality_factor\t %f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed_quality_factor);
	fprintf(SIL_XML_out, "\tyawrate_quality_factor \t%f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate_quality_factor);
	/*fprintf(SIL_XML_out, "\tlane_width_external\t%f\n", p_veh.lane_width_external);
	fprintf(SIL_XML_out, "\tlane_center_offset_external\t  %f\n", p_veh.lane_center_offset_external);
	fprintf(SIL_XML_out, "\thost_vehicle_length \t%f\n", p_veh.host_vehicle_length);
	fprintf(SIL_XML_out, "\thost_vehicle_width\t%f\n", p_veh.host_vehicle_width);
	fprintf(SIL_XML_out, "\tcurve_radius\t %f\n", p_veh.curve_radius);
	fprintf(SIL_XML_out, "\tvcs_long_acc\t %f\n", p_veh.vcs_long_acc);
	fprintf(SIL_XML_out, "\tvcs_lat_acc \t%f\n", p_veh.vcs_lat_acc);
	fprintf(SIL_XML_out, "\tvehicle_data_buff_timestamp\t %d\n", p_veh.vehicle_data_buff_timestamp);*/
	fprintf(SIL_XML_out, "\tprndl\t\t\t\t");
	//unsigned8_T prndl = p_veh.prndl;
	//switch (prndl)
	//{
	//case 0:fprintf(SIL_XML_out, "PARK"); break;
	//case 1:fprintf(SIL_XML_out, "REVERSE"); break;
	//case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
	//case 3:fprintf(SIL_XML_out, "DRIVE"); break;
	//case 4:fprintf(SIL_XML_out, "FOURTH"); break;
	//case 5:fprintf(SIL_XML_out, "THIRD"); break;
	//case 6:fprintf(SIL_XML_out, "LOW"); break;
	//default:fprintf(SIL_XML_out, "INVALID"); break;
	//}
	//unsigned8_T turn_signal = p_veh.turn_signal;
	//switch (turn_signal)//enum of turn_signal
	//{
	//case 0:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tNONE"); break;
	//case 1:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tLEFT"); break;
	//case 2:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tRIGHT"); break;
	//default:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tINVALID"); break;
	//}
	unsigned8_T f_reverse_gear = p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.f_reverse;
	switch (f_reverse_gear)//enum of f_reverse_gear
	{
	case 0:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
	case 1:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
	default:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tINVALID"); break;
	}
	/*fprintf(SIL_XML_out, "\n\tf_trailer_present\t %d\n", p_veh.f_trailer_present);
	fprintf(SIL_XML_out, "\tvcs_lat_offset \t%f\n", p_veh.vcs_lat_offset);
	fprintf(SIL_XML_out, "\tvcs_long_offset\t %f\n", p_veh.vcs_long_offset);*/
	//fprintf(SIL_XML_out, "\t</VEH_DATA_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n\t<ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\talign_angle_stat_az.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.mean);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.mean);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_stat_el.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.mean);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.mean \t\t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.mean);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.negative_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.positive_err \t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.std_dev \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.std_dev);

	fprintf(SIL_XML_out, "\tstatus_align_angle_az \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_az);
	fprintf(SIL_XML_out, "\tstatus_align_angle_el \t\t\t%f\n", p_veh_stla.Align_Info.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_el);
	//fprintf(SIL_XML_out, "\t</ALIGNMENT_OUTPUT>\n");
	fprintf(SIL_XML_out, "\n\t</ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//MNR_BLOCKAGE_PARAMETER
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML_STLA, BLOCKAGE_DATA);
	fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
	boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Blocked;
	switch (Blocked)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr4);
	fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr3);
	fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr2);
	fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Bmnr1);
	fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Forward);
	boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.Forward;
	switch (Forward)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.beta_var_si);
	fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.beta_var);
	fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.AvgPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.AvgRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.MaxPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.MaxRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA.MNR_Blockage_data.V_host);
	fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));


	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Diagnostics, Diagnostics_Data);
	fprintf(SIL_XML_out, "\n\t<Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	for (int i = 0; i < 24; i++)
	{
		fprintf(SIL_XML_out, "\tTiming_c2_%d \t%d\n", i,Diagnostics.Timing_Info_c2[i]);
	}
	
	//fprintf(SIL_XML_out, "\t</Diagnostics_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//detections
	fprintf(SIL_XML_out, "\n\t<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//valid detections
	static unsigned8_T Valid = STLA_Dets_info.target_count;
	static unsigned8_T Invalid = 0;
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	Invalid = NUMBER_OF_DETECTIONS - Valid;
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for  non zero range*/
		
		if ((STLA_Dets_info.target_report[index[i]].range != 0) && (count == 1))
		{	
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\trange_rate,\tazimuth, \tamplitude,\trcs,\t\tsnr,\texistence_probability,\tbi_static_target,\tsuper_res_target, \tazimuth_std_dev, \televation_std_dev, \tmatch_err_fft, \tvel_un, \tvel_st, \tfirst_pass_index, \tK_Unused16_1, \taz_conf, \tel_conf, \tisSingleTarget, \tisSingleTarget_azimuth, \tf_Doppler_mixed_interval, \tf_valid_Doppler_unfolding, \tbf_type_az, \tbf_type_el, \tsuper_res_target_type, \toutside_sector</DETECTION_INFO>\n");
			count = 0;
		}
		if (STLA_Dets_info.target_report[index[i]].range != 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].range);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].range_rate);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].azimuth);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].elevation);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].amp);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].rcs);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].snr);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].prob_exist);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].isBistatic);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].super_res_target);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].az_std_dev);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].el_std_dev);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].match_err_fft);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].vel_un);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].vel_st);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].first_pass_index);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].K_Unused16_1);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].az_conf);
			fprintf(SIL_XML_out, "\t%f", STLA_Dets_info.target_report[index[i]].el_conf);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].isSingleTarget);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_Dets_info.target_report[index[i]].isSingleTarget_azimuth);
			fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_Dets_info.target_report[index[i]].f_Doppler_mixed_interval);
			fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_Dets_info.target_report[index[i]].f_valid_Doppler_unfolding);
			fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_Dets_info.target_report[index[i]].bf_type_az);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].bf_type_el);
			fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_Dets_info.target_report[index[i]].super_res_target_type);
			fprintf(SIL_XML_out, "\t\t%f", STLA_Dets_info.target_report[index[i]].outside_sector);
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
	}
}

//CYW-4609, ADDING DETECTIONS FOR GEN7(SRR7P)
void GEN7_Radar_Self_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_Dets_info, GEN7_DETECTION_STREAM);
	/*sort by range*/
	float32_T Self_Dets[AF_MAX_NUM_DET] = { 0 };
	for (int i = 0; i < AF_MAX_NUM_DET; i++)
	{
		Self_Dets[i] = SRR7p_Dets_info.af_data.ran[i];
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[AF_MAX_NUM_DET] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}

	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);

	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;


	unsigned8_T count = 1;
	
	fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\t\t vel,\t\tamp,\t\t\tsnr,\t\t\trcs,\t\tel_conf,\taz_conf,\tpow,\t\ttheta, \t\tphi,\t\tsin_theta3,\t\tstd_range,\t\tsin_phi3,\tel_af_type,\taz_af_type, \tf_single_target, \tf_superres_target, \tf_1az2el, \tf_bistatic,\tf_peak_det,\taf_type,\trdd_idx,\tspec_res,\tstd_vel,\tstd_theta,\tstd_phi,\tnum_af_det</DETECTION_INFO>\n");
	for (unsigned16_T i = 0; i < AF_MAX_NUM_DET; i++)
	{
		/*for printing the valid detections, check will be added once validity_flag is provided */

			//if (((SRR7p_Dets_info.af_data.ran[i]) != 0) && (count == 1))
		//	{
		//	count = 0;
			//	}
			//	if ((SRR7p_Dets_info.af_data.ran[i]) != 0)
			//	{
		fprintf(SIL_XML_out, "\t<Detection>");
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.ran[i]);
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.vel[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.amp[i]);
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.snr[i]);
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.rcs[i]);
		fprintf(SIL_XML_out, "\t\t%d", SRR7p_Dets_info.af_data.el_conf[i]);
		fprintf(SIL_XML_out, "\t\t%d", SRR7p_Dets_info.af_data.az_conf[i]);
		fprintf(SIL_XML_out, "\t\t\t%f", SRR7p_Dets_info.af_data.pow[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.theta[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.phi[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.sin_theta3[i]);
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.std_ran[i]);
		fprintf(SIL_XML_out, "\t\t%f", SRR7p_Dets_info.af_data.sin_phi3[i]);
		fprintf(SIL_XML_out, "\t%d", SRR7p_Dets_info.af_data.el_af_type[i]);
		fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_Dets_info.af_data.az_af_type[i]);
		fprintf(SIL_XML_out, "\t\t\t\t%d", SRR7p_Dets_info.af_data.f_single_target[i]);
		fprintf(SIL_XML_out, "\t\t\t\t\t%d", SRR7p_Dets_info.af_data.f_superres_target[i]);
		fprintf(SIL_XML_out, "\t\t\t\t\t%d", SRR7p_Dets_info.af_data.f_1az2el[i]);
		fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_Dets_info.af_data.f_bistatic[i]);
		fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_Dets_info.af_data.f_peak_det[i]);
		fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_Dets_info.af_data.af_type[i]);
		fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_Dets_info.af_data.rdd_idx[i]);
		fprintf(SIL_XML_out, "\t\t\t%f", SRR7p_Dets_info.af_data.spec_res[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.std_vel[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.std_theta[i]);
		fprintf(SIL_XML_out, "\t%f", SRR7p_Dets_info.af_data.std_phi[i]);
		fprintf(SIL_XML_out, "\t\t%d", SRR7p_Dets_info.af_data.num_af_det);

		fprintf(SIL_XML_out, "\t\t</Detection>\n");
		//	}
	}

	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	}
}


void GEN7_Alignment_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
		//Alignment Data for SRR7P
		m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_alignment_srr7p, GEN7_ALIGNMENT_STREAM);
		fprintf(SIL_XML_out, "\n\t<ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\tvacs_boresight_az_nominal \t\t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_az_nominal);
		fprintf(SIL_XML_out, "\tvacs_boresight_az_estimated \t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_az_estimated);
		fprintf(SIL_XML_out, "\tvacs_boresight_az_std \t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_az_std);
		fprintf(SIL_XML_out, "\tvacs_boresight_el_nominal \t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_el_nominal);
		fprintf(SIL_XML_out, "\tvacs_boresight_el_estimated \t\t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_el_estimated);
		fprintf(SIL_XML_out, "\tvacs_boresight_el_std \t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_el_std);
		fprintf(SIL_XML_out, "\tstd_amplitude \t\t%f\n", p_alignment_srr7p.static_align_stream_output.std_amplitude);
		fprintf(SIL_XML_out, "\tstd_range\t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.std_range);
		fprintf(SIL_XML_out, "\tstd_r_dot \t\t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.std_r_dot);
		fprintf(SIL_XML_out, "\tradar_to_plate_distance \t\t%f\n", p_alignment_srr7p.static_align_stream_output.radar_to_plate_distance);
		fprintf(SIL_XML_out, "\tplate_angle_scs \t\t%f\n", p_alignment_srr7p.static_align_stream_output.plate_angle_scs);
		fprintf(SIL_XML_out, "\tvehicle_speed \t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vehicle_speed);
		fprintf(SIL_XML_out, "\tvehicle_yawrate \t\t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vehicle_yawrate);
		fprintf(SIL_XML_out, "\tvacs_boresight_az_min \t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_az_min);
		fprintf(SIL_XML_out, "\tvacs_boresight_az_max \t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_az_max);
		fprintf(SIL_XML_out, "\tvacs_boresight_el_min \t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_el_min);
		fprintf(SIL_XML_out, "\tvacs_boresight_el_max \t\t\t%f\n", p_alignment_srr7p.static_align_stream_output.vacs_boresight_el_max);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.status_of_plate);
		fprintf(SIL_XML_out, "\tvcs_polarity \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.vcs_polarity);
		fprintf(SIL_XML_out, "\tmount_loc \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.mount_loc);
		fprintf(SIL_XML_out, "\talignment_mode \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.alignment_mode);
		fprintf(SIL_XML_out, "\tversion_major \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.version_major);
		fprintf(SIL_XML_out, "\tversion_minor \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.version_minor);
		fprintf(SIL_XML_out, "\tversion_patch \t\t\t%d\n", p_alignment_srr7p.static_align_stream_output.version_patch);

		fprintf(SIL_XML_out, "\tvert_misalign_ang \t\t\t%f\n", p_alignment_srr7p.static_align_output.vert_misalign_ang);
		fprintf(SIL_XML_out, "\thoriz_misalign_ang \t\t\t%f\n", p_alignment_srr7p.static_align_output.horiz_misalign_ang);
		fprintf(SIL_XML_out, "\tstd_azimuth \t\t\t%f\n", p_alignment_srr7p.static_align_output.std_azimuth);
		fprintf(SIL_XML_out, "\tstd_amplitude \t\t\t%f\n", p_alignment_srr7p.static_align_output.std_amplitude);
		fprintf(SIL_XML_out, "\tstd_elevation \t\t\t%f\n", p_alignment_srr7p.static_align_output.std_elevation);
		fprintf(SIL_XML_out, "\tstd_range \t\t\t%f\n", p_alignment_srr7p.static_align_output.std_range);
		fprintf(SIL_XML_out, "\tstd_r_dot \t\t\t%f\n", p_alignment_srr7p.static_align_output.std_r_dot);
		fprintf(SIL_XML_out, "\tcomplete_percentage \t\t\t%f\n", p_alignment_srr7p.static_align_output.complete_percentage);
		fprintf(SIL_XML_out, "\tK_Unused16_1 \t\t\t%d\n", p_alignment_srr7p.static_align_output.K_Unused16_1);
		fprintf(SIL_XML_out, "\tquality_factor_az \t\t\t%d\n", p_alignment_srr7p.static_align_output.qf_az);
		fprintf(SIL_XML_out, "\tquality_factor_el \t\t\t%d\n", p_alignment_srr7p.static_align_output.qf_el);
		fprintf(SIL_XML_out, "\tstatus_vert_misalign_ang \t\t\t%d\n", p_alignment_srr7p.static_align_output.status_vert_misalign_ang);
		fprintf(SIL_XML_out, "\tstatus_horiz_misalign_ang \t\t\t%d\n", p_alignment_srr7p.static_align_output.status_horiz_misalign_ang);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%d\n", p_alignment_srr7p.static_align_output.status_of_plate);

		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.meas_plate_az);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.meas_plate_rcs);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.meas_plate_el);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.meas_plate_range);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.plate_az_angle);
		fprintf(SIL_XML_out, "\tstatus_of_plate \t\t\t%f\n", p_alignment_srr7p.static_align_internal.meas_plate_r_dot);


		fprintf(SIL_XML_out, "\n\t</ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

}

///*Print GEN7 Tracks Info*/			
void Gen7_SRR7P_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{	
	m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_tracks_info, GEN7_ROT_OBJECT_STREAM);
	DVSU_RECORD_T record = { 0 };

	if (custId == PLATFORM_GEN7)
	{
#ifndef DISABLE_TRACKER
		/*sort by range*/
		float32_T Track_Objs[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			Track_Objs[i] = SRR7p_tracks_info.rot_output.obj[i].vcs_yposn;
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			/*check for valid Track status level*/
			if (((SRR7p_tracks_info.rot_output.obj[index[i]].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", g_scanindex/*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_long_posn, vcs_long_vel, vcs_lat_vel,\tstatus,\t\tobject_class,\t\tref_point,\tf_moving,\t\tid,\t\ time_since_stage_start,\t\ speed, \t\ vcs_pointing,\tvcs_heading,\tcurvature,\taccuracy_length,\t accuracy_width,\t\tlen1,\t\tlen2,\t\twid1,\t\twid2,\texistence_probability,\tndets,\t\tnum_reduced_dets,\ttrk_fltr_type,\tprobability_2wheel,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_unknown</TRACK_INFO>\n");
				count = 0;
			}
			if ((SRR7p_tracks_info.rot_output.obj[index[i]].status) > 0)
			{
				fprintf(SIL_XML_out, "  <TRACKS>");
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_yposn);
				fprintf(SIL_XML_out, "\t\t %f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_yvel);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_xvel);
				//fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_long_vel_rel);
			//	fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_lat_vel_rel);

				unsigned8_T status = SRR7p_tracks_info.rot_output.obj[index[i]].status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
				case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
				case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED"); break;
				case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
				case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				}
				unsigned8_T object_class = SRR7p_tracks_info.rot_output.obj[index[i]].object_class;

				switch (object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNKNOWN\t"); break;
				case 1:fprintf(SIL_XML_out, "\tCAR\t\t"); break;
				case 2:fprintf(SIL_XML_out, "\tMOTORCYCLE\t"); break;
				case 3:fprintf(SIL_XML_out, "\tTRUCK\t"); break;
				case 4:fprintf(SIL_XML_out, "\tPEDESTRAIN"); break;
				case 5:fprintf(SIL_XML_out, "\tPOLE\t"); break;
				case 6:fprintf(SIL_XML_out, "\tTREE\t"); break;
				case 7:fprintf(SIL_XML_out, "\tANIMAL\t"); break;
				case 8:fprintf(SIL_XML_out, "\tGOD\t"); break;
				case 9:fprintf(SIL_XML_out, "\tBICYCLE\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				}
				fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].reference_point);
				fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].f_moving);
				fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].trkID);
				fprintf(SIL_XML_out, "\t\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].time_since_stage_start);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].speed);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_pointing);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].vcs_heading);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].curvature);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].accuracy_length);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].accuracy_width);
				fprintf(SIL_XML_out, "\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].len1);
				fprintf(SIL_XML_out, "\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].len2);
				fprintf(SIL_XML_out, "\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].wid1);
				fprintf(SIL_XML_out, "\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].wid2);
				fprintf(SIL_XML_out, "\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].existence_probability);
				fprintf(SIL_XML_out, "\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].ndets);
				fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].num_reduced_dets);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].trk_fltr_type);
			//	fprintf(SIL_XML_out, "\t\t\t%d", SRR7p_tracks_info.rot_output.obj[index[i]].object_class);
				fprintf(SIL_XML_out, "\t\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].probability_motorcycle);
				fprintf(SIL_XML_out, "\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].probability_car);
				fprintf(SIL_XML_out, "\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].probability_pedestrian);
				fprintf(SIL_XML_out, "\t\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].probability_truck);
				fprintf(SIL_XML_out, "\t\t\t\t%f", SRR7p_tracks_info.rot_output.obj[index[i]].probability_undet);

				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
		}
#endif
	}
}

//STLA_FLR4 Detections CYW-3354 and AF_DET_MAX_COUNT also added as per STLA_FLR4 V13
void STLA_FLR4_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&STLA_FLR4_Dets_info, DETS_COMP);
	/*sort by range*/
	float32_T Self_Dets[AF_DET_MAX_COUNT_FLR4] = { 0 };
	for (unsigned16_T i = 0; i < AF_DET_MAX_COUNT_FLR4; i++)
	{
		Self_Dets[i] = STLA_FLR4_Dets_info.target_report[i].range;
	}
	unsigned16_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned16_T index[AF_DET_MAX_COUNT_FLR4] = { 0 };
	for (unsigned16_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	fprintf(SIL_XML_out, "\t<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	unsigned8_T Self_LookType = STLA_FLR4_Dets_info.look_type;
	switch (Self_LookType)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookType></LookType>"); break;//do nothing
	}
	unsigned8_T Self_LookID = STLA_FLR4_Dets_info.lookindex;
	switch (Self_LookID)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "LONG_RANGE-MEDIUM_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-LONG_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookIndex>%s</LookIndex>", "MEDIUM_RANGE-MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookIndex></LookIndex>"); break;//do nothing
	}
	//Track_Info 
	unsigned8_T valid_track_counter = 0;
	unsigned8_T count_track = AF_DET_MAX_COUNT_FLR4 - 1;
	for (unsigned8_T index = 0; index < NUMBER_OF_OBJECTS; index++)  // NUMBER_OF_OBJECTS : need to check with latest macro update. at this time there is no tracks.
	{
		if ((curvi_XML.CurviTracks[count_track].Status) > 0)
		{
			valid_track_counter++;
		}
		count_track--;
	}
	fprintf(SIL_XML_out, "\n\t<NO_VALID_TRACKS>\t%d\t</NO_VALID_TRACKS>\n", valid_track_counter);
	fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
	fprintf(SIL_XML_out, "\t</TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//VEH_DATA_INFO
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla_flr4, Alignment_Data);
	fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\tabs_speed \t%f\n", p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed);
	fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate);
	fprintf(SIL_XML_out, "\tf_reverse\t %f\n", p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.f_reverse);
	fprintf(SIL_XML_out, "\tspeed_quality_factor\t %f\n", p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed_quality_factor);
	fprintf(SIL_XML_out, "\tyawrate_quality_factor \t%f\n", p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate_quality_factor);
	//unsigned8_T prndl = p_veh.prndl;
	//switch (prndl)
	//{
	//case 0:fprintf(SIL_XML_out, "PARK"); break;
	//case 1:fprintf(SIL_XML_out, "REVERSE"); break;
	//case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
	//case 3:fprintf(SIL_XML_out, "DRIVE"); break;
	//case 4:fprintf(SIL_XML_out, "FOURTH"); break;
	//case 5:fprintf(SIL_XML_out, "THIRD"); break;
	//case 6:fprintf(SIL_XML_out, "LOW"); break;
	//default:fprintf(SIL_XML_out, "INVALID"); break;
	//}
	//unsigned8_T turn_signal = p_veh.turn_signal;
	//switch (turn_signal)//enum of turn_signal
	//{
	//case 0:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tNONE"); break;
	//case 1:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tLEFT"); break;
	//case 2:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tRIGHT"); break;
	//default:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tINVALID"); break;
	//}
	unsigned8_T f_reverse_gear = p_veh_stla_flr4.Align_Input.Dyn_Align_input.Dyn_Vehicle.f_reverse;
	switch (f_reverse_gear)//enum of f_reverse_gear
	{
	case 0:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
	case 1:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
	default:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tINVALID"); break;
	}
	fprintf(SIL_XML_out, "\n\tf_trailer_present\t %d\n", p_veh.f_trailer_present);
	fprintf(SIL_XML_out, "\tvcs_lat_offset \t%f\n", p_veh.vcs_lat_offset);
	fprintf(SIL_XML_out, "\tvcs_long_offset\t %f\n", p_veh.vcs_long_offset);
	//fprintf(SIL_XML_out, "\t</VEH_DATA_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	fprintf(SIL_XML_out, "\n\t<ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));


	fprintf(SIL_XML_out, "\talign_angle_stat_az.mean \t\t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.mean);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.negative_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.positive_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_az.std_dev \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_az.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.mean \t\t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.mean);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.negative_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.positive_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_az.std_dev \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_az.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_stat_el.mean \t\t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.mean);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.negative_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.positive_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_stat_el.std_dev \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_stat_el.std_dev);

	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.mean \t\t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.mean);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.negative_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.negative_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.positive_err \t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.positive_err);
	fprintf(SIL_XML_out, "\talign_angle_ref_stat_el.std_dev \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.align_angle_ref_stat_el.std_dev);

	fprintf(SIL_XML_out, "\tstatus_align_angle_az \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_az);
	fprintf(SIL_XML_out, "\tstatus_align_angle_el \t\t\t%f\n", p_veh_stla_flr4.Align_Output.Dyn_Align_Output.Dyn_Perf_Output.status_align_angle_el);
	//fprintf(SIL_XML_out, "\t</ALIGNMENT_OUTPUT>\n");
	fprintf(SIL_XML_out, "\n\t</ALIGNMENT_OUTPUT_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//MNR_BLOCKAGE_PARAMETER




	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4, BLOCKAGE_DATA);
	fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
	boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Blocked;
	switch (Blocked)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Bmnr4);
	fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Bmnr3);
	fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Bmnr2);
	fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Bmnr1);
	fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Forward);
	boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.Forward;
	switch (Forward)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.beta_var_si);
	fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.beta_var);
	fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.AvgPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.AvgRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.MaxPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.MaxRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML_STLA_FLR4.MNR_Blockage_data.V_host);
	fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));


	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Diagnostics, Diagnostics_Data);
	fprintf(SIL_XML_out, "\n\t<Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	for (int i = 0; i < 24; i++)
	{
		fprintf(SIL_XML_out, "\tTiming_c2_%d \t%d\n", i,Diagnostics.Timing_Info_c2[i]);
	}

	//fprintf(SIL_XML_out, "\t</Diagnostics_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</Diagnostics_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//MNR_BLOCKAGE_PARAMETER
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML, MNR_BLOCKAGE_OUTPUT);
	//fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	////fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
	//boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked;
	//switch (Blocked)//enum of turn_signal
	//{
	//case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
	//case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
	//default://do nothing
	//	break;
	//}
	//fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr4);
	//fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr3);
	//fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr2);
	//fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr1);
	//fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward);
	//boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward;
	//switch (Forward)//enum of turn_signal
	//{
	//case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
	//case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
	//default://do nothing
	//	break;
	//}
	//fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var_si);
	//fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var);
	//fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgPm_mnr_avg);
	//fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgRm_mnr_avg);
	//fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxPm_mnr_avg);
	//fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxRm_mnr_avg);
	//fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.V_host);
	//fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	//detections
	
	unsigned8_T detectCount = 0;
	for (unsigned16_T i = 0; i < RDD_MAX_NUM_DET; i++)
	{
		if (STLA_FLR4_Dets_info.target_report[i].range > 0)
			detectCount++;
	}

	fprintf(SIL_XML_out, "\n\t<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n\t<target_count>%d</target_count>", STLA_FLR4_Dets_info.target_count);
	fprintf(SIL_XML_out, "\n\t</END_TARGET_REPORT_INFO>\n");
	//valid detections
	fprintf(SIL_XML_out, "\n\t<NO_OF_DETECTION_INFO_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", STLA_FLR4_Dets_info.target_count);
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", detectCount);
	int InValid_Dets = AF_DET_MAX_COUNT_FLR4 - detectCount;
	//int InValid_Dets = AF_DET_MAX_COUNT_FLR4 - STLA_FLR4_Dets_info.target_count;
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", InValid_Dets);
	fprintf(SIL_XML_out, "\n\t</NO_OF_DETECTION_INFO_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned16_T i = 0; i < AF_DET_MAX_COUNT_FLR4; i++)
	{
		/*check for  non zero range*/

		if ((STLA_FLR4_Dets_info.target_report[index[i]].range != 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\trange_rate,\tazimuth, \televation,\tamplitude,\t\trcs, \tsnr,\texistence_probability,\tbi_static_target,\tsuper_res_target, \tazimuth_std_dev, \televation_std_dev, \tmatch_err_fft, \tvel_un, \tvel_st, \tfirst_pass_index, \tK_Unused16_1, \taz_conf, \tel_conf, \tisSingleTarget, \tisSingleTarget_azimuth, \tf_Doppler_mixed_interval, \tf_valid_Doppler_unfolding, \tbf_type_az, \tbf_type_el, \tsuper_res_target_type, \toutside_sector</DETECTION_INFO>\n");
			count = 0;
		}
		if (STLA_FLR4_Dets_info.target_report[i].range != 0) //[index[i]] is change with i Due to mismatch index into XML and CSV
		{
			fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].range);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].range_rate);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].azimuth);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].elevation);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].amp);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].rcs);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].snr);
			fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4_Dets_info.target_report[i].prob_exist);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].isBistatic);
			fprintf(SIL_XML_out, "\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].super_res_target);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4_Dets_info.target_report[i].az_std_dev);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4_Dets_info.target_report[i].el_std_dev);
			fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4_Dets_info.target_report[i].match_err_fft);
			fprintf(SIL_XML_out, "\t%f", STLA_FLR4_Dets_info.target_report[i].vel_un);
			fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4_Dets_info.target_report[i].vel_st);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].first_pass_index);
			fprintf(SIL_XML_out, "\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].K_Unused16_1);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].az_conf);
			fprintf(SIL_XML_out, "\t%d", STLA_FLR4_Dets_info.target_report[i].el_conf);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].isSingleTarget);
			fprintf(SIL_XML_out, "\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].isSingleTarget_azimuth);
			fprintf(SIL_XML_out, "\t\t\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].f_Doppler_mixed_interval);
			fprintf(SIL_XML_out, "\t\t\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].f_valid_Doppler_unfolding);
			fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].bf_type_az);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].bf_type_el);
			fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4_Dets_info.target_report[i].super_res_target_type);
			fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4_Dets_info.target_report[i].outside_sector);
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
	}
}
void Gen5_FLR4P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Flr4p_Dets_info, DETS_COMP);
	//VEH_DATA_INFO
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh, VEH_COMP);
	fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\tabs_speed \t%f\n", p_veh.abs_speed);
	fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh.yawrate);
	fprintf(SIL_XML_out, "\tsteering_angle\t %f\n", p_veh.steering_angle);
	fprintf(SIL_XML_out, "\trear_axle_steering_angle\t %f\n", p_veh.rear_axle_steering_angle);
	fprintf(SIL_XML_out, "\trear_axle_position \t%f\n", p_veh.rear_axle_position);
	fprintf(SIL_XML_out, "\tlane_width_external\t%f\n", p_veh.lane_width_external);
	fprintf(SIL_XML_out, "\tlane_center_offset_external\t  %f\n", p_veh.lane_center_offset_external);
	fprintf(SIL_XML_out, "\thost_vehicle_length \t%f\n", p_veh.host_vehicle_length);
	fprintf(SIL_XML_out, "\thost_vehicle_width\t%f\n", p_veh.host_vehicle_width);
	fprintf(SIL_XML_out, "\tcurve_radius\t %f\n", p_veh.curve_radius);
	fprintf(SIL_XML_out, "\tvcs_long_acc\t %f\n", p_veh.vcs_long_acc);
	fprintf(SIL_XML_out, "\tvcs_lat_acc \t%f\n", p_veh.vcs_lat_acc);
	fprintf(SIL_XML_out, "\tvehicle_data_buff_timestamp\t %d\n", p_veh.vehicle_data_buff_timestamp);
	fprintf(SIL_XML_out, "\tprndl\t\t\t\t");
	unsigned8_T prndl = p_veh.prndl;
	switch (prndl)
	{
	case 0:fprintf(SIL_XML_out, "PARK"); break;
	case 1:fprintf(SIL_XML_out, "REVERSE"); break;
	case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
	case 3:fprintf(SIL_XML_out, "DRIVE"); break;
	case 4:fprintf(SIL_XML_out, "FOURTH"); break;
	case 5:fprintf(SIL_XML_out, "THIRD"); break;
	case 6:fprintf(SIL_XML_out, "LOW"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	unsigned8_T turn_signal = p_veh.turn_signal;
	switch (turn_signal)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tNONE"); break;
	case 1:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tLEFT"); break;
	case 2:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tRIGHT"); break;
	default:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tINVALID"); break;
	}
	unsigned8_T f_reverse_gear = p_veh.f_reverse_gear;
	switch (f_reverse_gear)//enum of f_reverse_gear
	{
	case 0:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
	case 1:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
	default:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tINVALID"); break;
	}
	fprintf(SIL_XML_out, "\n\tf_trailer_present\t %d\n", p_veh.f_trailer_present);
	fprintf(SIL_XML_out, "\tvcs_lat_offset \t%f\n", p_veh.vcs_lat_offset);
	fprintf(SIL_XML_out, "\tvcs_long_offset\t %f\n", p_veh.vcs_long_offset);
	//fprintf(SIL_XML_out, "\t</VEH_DATA_INFO>\n");
	fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	//MNR_BLOCKAGE_PARAMETER
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML, MNR_BLOCKAGE_OUTPUT);
	fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
	boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked;
	switch (Blocked)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr4);
	fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr3);
	fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr2);
	fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr1);
	fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward);
	boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward;
	switch (Forward)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var_si);
	fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var);
	fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxPm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxRm_mnr_avg);
	fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.V_host);
	fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

	//Traget info
	fprintf(SIL_XML_out, "\n\t<TARGET_REPORT_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
	unsigned8_T Self_LookType = Flr4p_Dets_info.look_type;
	switch (Self_LookType)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 1:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "LONG_LOOK"); break;
	case 2:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	case 3:fprintf(SIL_XML_out, "\n\t<LookType>%s</LookType>", "MEDIUM_LOOK"); break;
	default:fprintf(SIL_XML_out, "\n\t<LookType></LookType>"); break;//do nothing
	}

	unsigned8_T detectCount = 0;
	for (unsigned16_T i = 0; i < RDD_MAX_NUM_DET; i++)
	{
		if (Flr4p_Dets_info.target_report[i].range > 0)
			detectCount++;
	}

	fprintf(SIL_XML_out, "\n\t<target_count>%d</target_count>", Flr4p_Dets_info.target_count);
	fprintf(SIL_XML_out, "\n\t</END_TARGET_REPORT_INFO>\n");

	//valid detections
	fprintf(SIL_XML_out, "\n\t<NO_OF_DETECTION_INFO_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	//fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Flr4p_Dets_info.target_count);
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", detectCount);

	//invalid detections
	//int InValid_Dets = RDD_MAX_NUM_DET - Flr4p_Dets_info.target_count;
	int InValid_Dets = RDD_MAX_NUM_DET - detectCount;
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", InValid_Dets);
	fprintf(SIL_XML_out, "\n\t</NO_OF_DETECTION_INFO_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	

	//detection data
	/*sort by range*/
	float32_T Self_Dets[RDD_MAX_NUM_DET] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		Self_Dets[i] = Flr4p_Dets_info.target_report[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[RDD_MAX_NUM_DET] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}

	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);


	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned16_T i = 0; i < RDD_MAX_NUM_DET; i++)
	{
		/*check for  non zero range*/

		if ((Flr4p_Dets_info.target_report[i].range != 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate,\tazimuth,\televation,\tamp, \t\trcs,\t\tsnr,\taz_std_dev, \tel_std_dev, \tprob_exist, \tmatch_err_fft, \tvel_un, \t\tvel_st, \t\tfirst_pass_index,\taz_conf, \tel_conf, \tisBistatic, \tisSingleTarget, \tisSingleTarget_azimuth, \tf_Doppler_mixed_interval, \tf_valid_Doppler_unfolding, \tbf_type_az, \tbf_type_el, \tsuper_res_target, \tsuper_res_target_type, \toutside_sector</DETECTION_INFO>\n");

			count = 0;
		}

		if (Flr4p_Dets_info.target_report[i].range != 0)
		{
			fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].range);
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].range_rate);
			{
				float azimuth = RAD2DEG(Flr4p_Dets_info.target_report[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", azimuth);
			}
			{
				float elevation = RAD2DEG(Flr4p_Dets_info.target_report[i].elevation);
				fprintf(SIL_XML_out, "\t%f", elevation);
			}
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].amp);
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].rcs);
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].snr);
			fprintf(SIL_XML_out, "\t%f", Flr4p_Dets_info.target_report[i].az_std_dev);
			fprintf(SIL_XML_out, "\t\t%f", Flr4p_Dets_info.target_report[i].el_std_dev);
			fprintf(SIL_XML_out, "\t\t%f", Flr4p_Dets_info.target_report[i].prob_exist);
			fprintf(SIL_XML_out, "\t\t%f", Flr4p_Dets_info.target_report[i].match_err_fft);
			fprintf(SIL_XML_out, "\t\t%f", Flr4p_Dets_info.target_report[i].vel_un);
			fprintf(SIL_XML_out, "\t\t%f", Flr4p_Dets_info.target_report[i].vel_st);
			fprintf(SIL_XML_out, "\t\t\t%d", Flr4p_Dets_info.target_report[i].first_pass_index);
			//fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Dets_info.target_report[i].K_UNUSED1_2);
			fprintf(SIL_XML_out, "\t\t\t%d", Flr4p_Dets_info.target_report[i].az_conf);
			fprintf(SIL_XML_out, "\t\t\t%d", Flr4p_Dets_info.target_report[i].el_conf);
			fprintf(SIL_XML_out, "\t\t\t%d", Flr4p_Dets_info.target_report[i].isBistatic);
			fprintf(SIL_XML_out, "\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].isSingleTarget);
			fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].isSingleTarget_azimuth);
			fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].f_Doppler_mixed_interval);
			fprintf(SIL_XML_out, "\t\t\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].f_valid_Doppler_unfolding);
			fprintf(SIL_XML_out, "\t\t\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].bf_type_az);
			fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Dets_info.target_report[i].bf_type_el);
			fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Dets_info.target_report[i].super_res_target);
			fprintf(SIL_XML_out, "\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].super_res_target_type);
			fprintf(SIL_XML_out, "\t\t\t\t\t%d", Flr4p_Dets_info.target_report[i].outside_sector);

			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
	}
}
//FLR4P TRACKS Info
void FLR4P_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Flr4p_Tracker_info, CURVI_TRACK_DATA);
	DVSU_RECORD_T record = { 0 };
	if (custId == PLATFORM_GEN5)
	{
#ifndef DISABLE_TRACKER
		//Track_Info 
		unsigned8_T valid_track_counter = 0;
		unsigned8_T count_track = NUMBER_OF_OBJECT_TRACKS_L - 1;
		for (unsigned8_T index = 0; index < NUMBER_OF_OBJECT_TRACKS_L; index++)  // valid trackes count 
		{
			if ((Flr4p_Tracker_info.object[count_track].status) > 0)
			{
				valid_track_counter++;
			}
			count_track--;
		}
		fprintf(SIL_XML_out, "\n\t<NO_VALID_TRACKS>\t%d\t</NO_VALID_TRACKS>", valid_track_counter);
		/*sort by range*/
		float32_T Track_Objs[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
		{
			Track_Objs[i] = Flr4p_Tracker_info.object[i].vcs_xposn;
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
		{
			/*check for valid Track status level*/
			if (((Flr4p_Tracker_info.object[i].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", g_scanindex/*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_xposn, \tvcs_yposn,\tf_moving,\tvcs_xvel,\t\tvcs_yvel,\t\tvcs_xaccel,\t\tvcs_yaccel,\t\tvcs_heading,\tstatus,\t\t\t\tobject_class,\t\t\t\t\t\tlen1,\t\tlen2,\t\twid1,\t\twid2,\t\ttrkID,\t\tunique_id,\t\treducedID,\tspeed,\t\t\tcurvature,\t\ttang_accel,\tstate_variance[0],\tstate_variance[1],\tstate_variance[2],\tstate_variance[3],\tstate_variance[4],\tstate_variance[5],\tsupplemental_state_covariance[0],\tsupplemental_state_covariance[1],\tsupplemental_state_covariance[2], \tconfidenceLevel,\t time_since_measurement,\t\ttime_since_cluster_created, time_since_track_updated, time_since_stage_start,\t\t\t\tndets,\t\tnum_reduced_dets,\t\texistence_probability,\t\treference_point,\treducedStatus,\tinit_scheme,\tf_crossing,\tf_oncoming,\tf_vehicular_trk,\tf_onguardrail,\tf_fast_moving,\taccuracy_width,\t\taccuracy_length,\t\tunderdrivable_status,\t\tprobability_motorcycle,\tprobability_bicycle,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_undet,\tpadding[0],\tpadding[1],\tcurrent_msmt_type</TRACK_INFO>\n");
				count = 0;
			}
			if ((Flr4p_Tracker_info.object[i].status) > 0 || (count == 0))
			{
				fprintf(SIL_XML_out, "\t<TRACKS_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].vcs_xposn);
				fprintf(SIL_XML_out, "\t\t %f", Flr4p_Tracker_info.object[i].vcs_yposn);
				fprintf(SIL_XML_out, "\t\t%d", Flr4p_Tracker_info.object[i].f_moving);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].vcs_xvel);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].vcs_yvel);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].vcs_xaccel);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].vcs_yaccel);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].vcs_heading);

				unsigned8_T status = Flr4p_Tracker_info.object[i].status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\tINVALID\t\t\t"); break;
				case 1:fprintf(SIL_XML_out, "\t\t\tNEW\t\t\t"); break;
				case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
				case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t\t"); break;
				case 4:fprintf(SIL_XML_out, "\tUPDATED\t\t\t"); break;
				case 5:fprintf(SIL_XML_out, "\tCOASTED\t\t\t"); break;
				default:fprintf(SIL_XML_out,"\tINVALID\t\t\t"); break;
				}
				unsigned8_T object_class = Flr4p_Tracker_info.object[i].object_class;

				switch (object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNDETERMINED\t\t\t\t\t"); break;
				case 1:fprintf(SIL_XML_out, "\tCAR\t\t\t\t\t\t\t\t"); break;
				case 2:fprintf(SIL_XML_out, "\tMOTORCYCLE\t\t\t\t\t\t"); break;
				case 3:fprintf(SIL_XML_out, "\tTRUCK\t\t\t\t\t\t\t\t"); break;
				case 4:fprintf(SIL_XML_out, "\tPEDESTRIAN\t\t\t\t\t\t"); break;
				case 5:fprintf(SIL_XML_out, "\tPOLE\t\t\t\t\t\t\t\t\t"); break;
				case 6:fprintf(SIL_XML_out, "\tTREE\t\t\t\t\t\t\t\t\t"); break;
				case 7:fprintf(SIL_XML_out, "\tANIMAL\t\t\t\t\t\t\t\t"); break;
				case 8:fprintf(SIL_XML_out, "\tGOD\t\t\t\t\t\t\t\t\t"); break;
				case 9:fprintf(SIL_XML_out, "\tBICYCLE\t\t\t\t\t\t\t"); break;
				case 10:fprintf(SIL_XML_out,"\tUNIDENTIFIED_VEHICLE\t"); break;
				default:fprintf(SIL_XML_out,"\tINVALID\t\t\t\t\t\t\t"); break;
				}
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].len1);
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].len2);
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].wid1);
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].wid2);

				fprintf(SIL_XML_out, "\t%d", Flr4p_Tracker_info.object[i].trkID);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].unique_id);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].reducedID);
				fprintf(SIL_XML_out, "\t%f", Flr4p_Tracker_info.object[i].speed);
				fprintf(SIL_XML_out, "\t\t\t%f", Flr4p_Tracker_info.object[i].curvature);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].tang_accel);
				for (int j = 0; j < SIX; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t%f", Flr4p_Tracker_info.object[i].state_variance[j]);
				}
				for (int j = 0; j < THREE; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].supplemental_state_covariance[j]);
				}
				fprintf(SIL_XML_out, "\t\t\t\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].confidenceLevel);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].time_since_measurement);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].time_since_cluster_created);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].time_since_track_updated);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].time_since_stage_start);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", Flr4p_Tracker_info.object[i].ndets);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].num_reduced_dets);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", Flr4p_Tracker_info.object[i].reference_point);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].reducedStatus);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].init_scheme);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].f_crossing);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].f_oncoming);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].f_vehicular_trk);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].f_onguardrail);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].f_fast_moving);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].accuracy_width);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].accuracy_length);
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].underdrivable_status);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Flr4p_Tracker_info.object[i].probability_motorcycle);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].probability_bicycle);
				fprintf(SIL_XML_out, "\t\t%f", Flr4p_Tracker_info.object[i].probability_car);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].probability_pedestrian);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Flr4p_Tracker_info.object[i].probability_truck);
				fprintf(SIL_XML_out, "\t\t\t%f", Flr4p_Tracker_info.object[i].probability_undet);
				for (int j = 0; j < TWO; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t%d", Flr4p_Tracker_info.object[i].padding[j]);
				}
				fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].current_msmt_type);

				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
		}
#endif
	}	
}

//STLA_FLR4P Track Info
void STLA_FLR4P_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&STLA_FLR4P_track_info, TRACK_COMP_CORE);
	if (custId == STLA_FLR4P)
	{
#ifndef DISABLE_TRACKER
		//Track_Info 
		unsigned8_T valid_track_counter = 0;
		unsigned8_T count_track = NUMBER_OF_OBJECT_TRACKS_L - 1;
		for (unsigned8_T index = 0; index < NUMBER_OF_OBJECT_TRACKS_L; index++)  // valid trackes count 
		{
			if ((STLA_FLR4P_track_info.object[count_track].status) > 0)
			{
				valid_track_counter++;
			}
			count_track--;
		}
		fprintf(SIL_XML_out, "\n\t<NO_VALID_TRACKS>\t%d\t</NO_VALID_TRACKS>", valid_track_counter);
		
		float32_T Track_Objs[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
		{
			Track_Objs[i] = STLA_FLR4P_track_info.object[i].vcs_xposn;
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
		{
			
			if (((STLA_FLR4P_track_info.object[i].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_xposn, \tvcs_yposn,\tf_moving,\tvcs_xvel,\t\tvcs_yvel,\t\tvcs_xaccel,\t\tvcs_yaccel,\t\tvcs_heading,\tstatus,\t\t\t\tobject_class,\t\t\t\t\t\tlen1,\t\tlen2,\t\twid1,\t\twid2,\t\ttrkID,\t\tunique_id,\t\treducedID,\tspeed,\t\t\tcurvature,\t\ttang_accel,\tstate_variance[0],\tstate_variance[1],\tstate_variance[2],\tstate_variance[3],\tstate_variance[4],\tstate_variance[5],\tsupplemental_state_covariance[0],\tsupplemental_state_covariance[1],\tsupplemental_state_covariance[2], \tconfidenceLevel,\t time_since_measurement,\t\ttime_since_cluster_created, time_since_track_updated, time_since_stage_start,\t\t\t\tndets,\t\tnum_reduced_dets,\t\texistence_probability,\t\treference_point,\treducedStatus,\tinit_scheme,\tf_crossing,\tf_oncoming,\tf_vehicular_trk,\tf_onguardrail,\tf_fast_moving,\taccuracy_width,\t\taccuracy_length,\t\tunderdrivable_status,\t\tprobability_motorcycle,\tprobability_bicycle,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_undet,\tpadding[0],\tpadding[1],\tcurrent_msmt_type</TRACK_INFO>\n");
				count = 0;
			}
			if ((STLA_FLR4P_track_info.object[i].status) > 0)
			{
				fprintf(SIL_XML_out, "\t<TRACKS_%d>", i);
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].vcs_xposn);
				fprintf(SIL_XML_out, "\t\t %f", STLA_FLR4P_track_info.object[i].vcs_yposn);
				fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4P_track_info.object[i].f_moving);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_xvel);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_yvel);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_xaccel);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_yaccel);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_heading);

				unsigned8_T status = STLA_FLR4P_track_info.object[i].status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\tINVALID\t\t\t"); break;
				case 1:fprintf(SIL_XML_out, "\t\t\tNEW\t\t\t"); break;
				case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
				case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t\t"); break;
				case 4:fprintf(SIL_XML_out, "\tUPDATED\t\t\t"); break;
				case 5:fprintf(SIL_XML_out, "\tCOASTED\t\t\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t\t\t"); break;
				}
				unsigned8_T object_class = STLA_FLR4P_track_info.object[i].object_class;

				switch (object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNDETERMINED\t\t\t\t\t"); break;
				case 1:fprintf(SIL_XML_out, "\tCAR\t\t\t\t\t\t\t\t"); break;
				case 2:fprintf(SIL_XML_out, "\tMOTORCYCLE\t\t\t\t\t\t"); break;
				case 3:fprintf(SIL_XML_out, "\tTRUCK\t\t\t\t\t\t\t\t"); break;
				case 4:fprintf(SIL_XML_out, "\tPEDESTRIAN\t\t\t\t\t\t"); break;
				case 5:fprintf(SIL_XML_out, "\tPOLE\t\t\t\t\t\t\t\t\t"); break;
				case 6:fprintf(SIL_XML_out, "\tTREE\t\t\t\t\t\t\t\t\t"); break;
				case 7:fprintf(SIL_XML_out, "\tANIMAL\t\t\t\t\t\t\t\t"); break;
				case 8:fprintf(SIL_XML_out, "\tGOD\t\t\t\t\t\t\t\t\t"); break;
				case 9:fprintf(SIL_XML_out, "\tBICYCLE\t\t\t\t\t\t\t"); break;
				case 10:fprintf(SIL_XML_out, "\tUNIDENTIFIED_VEHICLE\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t\t\t\t\t\t\t"); break;
				}
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].len1);
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].len2);
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].wid1);
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].wid2);

				fprintf(SIL_XML_out, "\t%d", STLA_FLR4P_track_info.object[i].trkID);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].unique_id);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reducedID);
				fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].speed);
				fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].curvature);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].tang_accel);
				for (int j = 0; j < SIX; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].state_variance[j]);
				}
				for (int j = 0; j < THREE; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].supplemental_state_covariance[j]);
				}
				fprintf(SIL_XML_out, "\t\t\t\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].confidenceLevel);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_measurement);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_cluster_created);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_track_updated);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_stage_start);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", STLA_FLR4P_track_info.object[i].ndets);
				//fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].num_reduced_dets);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reference_point);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reducedStatus);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].init_scheme);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_crossing);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_oncoming);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_vehicular_trk);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_onguardrail);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_fast_moving);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].accuracy_width);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].accuracy_length);
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].underdrivable_status);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_motorcycle);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_bicycle);
				fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].probability_car);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_pedestrian);
				fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_truck);
				fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_undet);
				for (int j = 0; j < TWO; j++)
				{
					fprintf(SIL_XML_out, "\t\t\t%d", STLA_FLR4P_track_info.object[i].padding[j]);
				}
				fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].current_msmt_type);

				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", g_scanindex , GetRadarPosName(Radar_Posn));
		}
#endif
	}
} 

///*Print Curvi Tracks Info*/			
void Gen5_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{	
	//m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_scan_index, SCAN_INDEX_COMP);
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_curvi_track_XML, CURVI_TRACK_DATA);
	//m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_self_XML, DETS_OPP);
//	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_curvi_track_XML, Z7B_TRACKER_DATA);
	DVSU_RECORD_T record = { 0 };


	if (custId == PLATFORM_GEN5 || custId == NISSAN_GEN5 || custId == HONDA_GEN5 || custId == STLA_SRR6P)
	{
#ifndef DISABLE_TRACKER
		/*sort by range*/
		float32_T Track_Objs[GEN5_NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
		{
			Track_Objs[i] = gen5_curvi_track_XML.CurviTracks[i].vcs_long_posn;
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[GEN5_NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
		{
			/*check for valid Track status level*/
			if (((gen5_curvi_track_XML.CurviTracks[index[i]].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", g_scanindex/*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_long_posn, vcs_long_vel, vcs_lat_vel, vcs_long_vel_rel, vcs_lat_vel_rel,\tstatus, object_class, ref_position, f_stationary,\t\t\tid, \t\tage,\t\tref_lat_accel, \t\tref_lat_vel,\tref_lat_posn,\tref_long_accel, \tref_long_vel,\t ref_long_posn,\t\tcurvi_lat_vel_rel, curvi_long_vel_rel, curvi_lat_vel, curvi_long_vel, curvi_lat_posn, curvi_long_posn,\t\twidth,\t\tlength,\t\theading,\texistence_probability,\taccuracy_x,\taccuracy_y,\taccuracy_vx,\taccuracy_vy,\taccuracy_ax,\taccuracy_ay,\taccuracy_width,\taccuracy_length,\taccuracy_heading,\tprobability_2wheel,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_unknown</TRACK_INFO>\n");
				count = 0;
			}
			if ((gen5_curvi_track_XML.CurviTracks[index[i]].status) > 0)
			{
				fprintf(SIL_XML_out, "  <TRACKS>");
				fprintf(SIL_XML_out, "\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].vcs_long_posn);
				fprintf(SIL_XML_out, "\t\t %f", gen5_curvi_track_XML.CurviTracks[index[i]].vcs_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].vcs_lat_vel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].vcs_long_vel_rel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].vcs_lat_vel_rel);

				unsigned8_T status = gen5_curvi_track_XML.CurviTracks[index[i]].status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				case 1:fprintf(SIL_XML_out, "\t\tNEW\t"); break;
				case 2:fprintf(SIL_XML_out, "\t\tMATURE\t"); break;
				case 3:fprintf(SIL_XML_out, "\t\tCOASTED\t"); break;
				case 4:fprintf(SIL_XML_out, "\t\tNUMBER_OF_OBJECT_STATUS\t"); break;
				default:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				}
				unsigned8_T object_class = gen5_curvi_track_XML.CurviTracks[index[i]].object_class;

				switch (object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNKNOWN\t"); break;
				case 1:fprintf(SIL_XML_out, "\tPEDESTRIAN\t"); break;
				case 2:fprintf(SIL_XML_out, "\t2WHEEL\t"); break;
				case 3:fprintf(SIL_XML_out, "\tCAR\t"); break;
				case 4:fprintf(SIL_XML_out, "\tTRUCK\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				}
				fprintf(SIL_XML_out, "\t\t%d", gen5_curvi_track_XML.CurviTracks[index[i]].ref_position);
				fprintf(SIL_XML_out, "\t\t\t\t%d", gen5_curvi_track_XML.CurviTracks[index[i]].f_stationary);
				fprintf(SIL_XML_out, "\t\t\t\t%d", gen5_curvi_track_XML.CurviTracks[index[i]].id);
				fprintf(SIL_XML_out, "\t\t\t\t%d", gen5_curvi_track_XML.CurviTracks[index[i]].age);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_lat_accel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_lat_vel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_lat_posn);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_long_accel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].ref_long_posn);
				fprintf(SIL_XML_out, "\t\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_lat_vel_rel);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_long_vel_rel);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_lat_vel);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_lat_posn);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].curvi_long_posn);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].width);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].length);
				fprintf(SIL_XML_out, "\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].heading);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].existence_probability);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_x);
				fprintf(SIL_XML_out, "\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_y);
				fprintf(SIL_XML_out, "\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_vx);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_vy);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_ax);
				fprintf(SIL_XML_out, "\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_ay);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_width);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_length);
				fprintf(SIL_XML_out, "\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].accuracy_heading);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].object_class_probability.probability_2wheel);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].object_class_probability.probability_car);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].object_class_probability.probability_pedestrian);
				fprintf(SIL_XML_out, "\t\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].object_class_probability.probability_truck);
				fprintf(SIL_XML_out, "\t\t\t%f", gen5_curvi_track_XML.CurviTracks[index[i]].object_class_probability.probability_unknown);
				
				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", g_scanindex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
		}
#endif
	}
}

void Gen5_Record_Header_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId, short stream_num)
{
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_OppDets_xml, DETS_OPP);
	m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Gen5_target_info, GEN5_TARGET_INFO);
	

	//g_pIRadarStrm->getData((unsigned char*)&Dets_self_XML,DET_SELF);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);

	fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", Gen5_target_info.ScanIndex, GetRadarPosName(Radar_Posn));

	/*Target report info for self Radar*/
	//fprintf(SIL_XML_out, "<TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", gen5_Dets_xml.dets_info.ScanIndex, GetRadarPosName(Radar_Posn));

	////char strData[200]={0};
	///////////////floatToString(Dets_XML.timestamp, strData, FIXED);
	//fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Gen5_target_info.timestamp);
	//fprintf(SIL_XML_out, "\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n", Gen5_target_info.ScanIndex);
	//unsigned8_T  Self_LookType = Gen5_target_info.LookType;
	//switch (Self_LookType)//enum of Self Radar LookType
	//{
	//case 0:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
	//case 1:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
	//case 2:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
	//case 3:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
	//default://do nothing
	//	break;
	//}
	//unsigned8_T Self_LookID = Gen5_target_info.LookID;
	//switch (Self_LookID)//enum of Self Radar LookID
	//{
	//case 0:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
	//case 1:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
	//case 2:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
	//case 3:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
	//default://do nothing
	//	break;
	//}
	//fprintf(SIL_XML_out, "\t<TS_CONSIST>\t%d\t</TS_CONSIST>\n", Gen5_target_info.timestamp_consistency);
	//fprintf(SIL_XML_out, "\t<NO_CDC_FRAMES>\t%d\t</NO_CDC_FRAMES>\n", cdc_frame_XML.Number_Of_CDC_Frames);

	//fprintf(SIL_XML_out, "\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n", Gen5_target_info.Count);
	//fprintf(SIL_XML_out, "</TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", Gen5_target_info.ScanIndex, GetRadarPosName(Radar_Posn));
}

void Gen5_Record_Header_Count(unsigned8_T Radar_Posn, IRadarStream  *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Gen5_target_info, TARGET_INFO);
	fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", Gen5_target_info.ScanIndex, GetRadarPosName(Radar_Posn));
}
//------------------------------------------------------------------//


void Populate_Valid_Data_to_XML(FILE* filename, const char* LogFname, IRadarStream * g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId)
{
	char  inp_file[1024] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	Open_XML_file(filename);
	Open_CSV_file(filename);
	//(_pIRadarStrm->get_complete_Buffer());
	if (CreateOnehdr)
	{
		Input_Header_Info(Radar_Posn, InputLogName, custId, g_pIRadarStrm);
		CreateOnehdr = false;
	}
	if (stream_num == 10)
	{
		//Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		Record_Header_Man_Dpace_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		//Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
		//STLA_SRR6P_Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
	}
	if(stream_num == 1 && custId != SCANIA_MAN && custId  != RNA_CDV && custId != RNA_SUV)
	{
		//Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		Record_Header_Man_Dpace_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		//Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 1 && custId != SCANIA_MAN && custId != RNA_CDV && custId != RNA_SUV)
	{
		Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		//Z4_Record_header_Info(g_pIRadarStrm,custId);
		//VF_RR_Record_Header_Info(g_pIRadarStrm);
		//VF_RL_Record_Header_Info(g_pIRadarStrm);
		//VF_FR_Record_Header_Info(g_pIRadarStrm);
		//VF_FL_Record_Header_Info(g_pIRadarStrm);
		Vehicle_Info(Radar_Posn, g_pIRadarStrm);
		Mounting_Info(Radar_Posn, g_pIRadarStrm, custId);
		MNR_Blockage_Info(Radar_Posn, g_pIRadarStrm, custId);
		TD_Blockage_Info(Radar_Posn, g_pIRadarStrm);
		if (custId == CHANGAN_SRR5)
		{
			Self_Radar_Info_CHANGAN(Radar_Posn, g_pIRadarStrm, custId);
			Opposite_Radar_Info_CHANGAN(Radar_Posn, g_pIRadarStrm, custId);
		}
		else
		{
			Self_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
			Opposite_Radar_Info(Radar_Posn, g_pIRadarStrm, custId);
		}
		Z4_Opposite_Radar_Info(Radar_Posn, g_pIRadarStrm);
	}
	if (stream_num == 1 && (custId == RNA_CDV || custId == RNA_SUV))
	{
		Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		Vehicle_Info(Radar_Posn, g_pIRadarStrm);
		Mounting_Info(Radar_Posn, g_pIRadarStrm, custId);
		MNR_Blockage_Info(Radar_Posn, g_pIRadarStrm, custId);
		TD_Blockage_Info(Radar_Posn, g_pIRadarStrm);
		Self_Radar_Info_RNA(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 3 && (custId == RNA_CDV || custId == RNA_SUV))
	{
		Opposite_Radar_Info_RNA(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 1 && custId == SCANIA_MAN)
	{
		Record_Header_Info(Radar_Posn, g_pIRadarStrm, custId, stream_num);
		Vehicle_Info(Radar_Posn, g_pIRadarStrm);
		Mounting_Info(Radar_Posn, g_pIRadarStrm, custId);
		MNR_Blockage_Info(Radar_Posn, g_pIRadarStrm, custId);
		TD_Blockage_Info(Radar_Posn, g_pIRadarStrm);
		Self_Radar_Info_MAN(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 3 && custId == SCANIA_MAN)
	{
		Opposite_Radar_Info_MAN(Radar_Posn, g_pIRadarStrm, custId);
	}

	if (stream_num == 0)
	{
		Z7a_First_pass_Info(Radar_Posn, g_pIRadarStrm, custId);
	}
	//VF_RR_Opposite_Radar_Info(g_pIRadarStrm);
	//VF_RL_Opposite_Radar_Info(g_pIRadarStrm);
	//	VF_FR_Opposite_Radar_Info(g_pIRadarStrm);
	//VF_FL_Opposite_Radar_Info(g_pIRadarStrm);
	if (stream_num == 1 && custId != SCANIA_MAN && custId != CHANGAN_SRR5)
	{
		Curvi_Tracks_Info(Radar_Posn, g_pIRadarStrm, custId);
		Alignment_Info(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 1 && (custId == SCANIA_MAN || custId == CHANGAN_SRR5))
	{
		Curvi_Tracks_Info_MAN(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 1 && custId == TML_SRR5)
	{
		Curvi_Tracks_Info_TML(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 1 && custId == PLATFORM_GEN7)
	{
		GEN7_Radar_Self_Info(Radar_Posn, g_pIRadarStrm, custId);
	}
	if (stream_num == 3)
	{
		ADC_Physical_xml_Data(Radar_Posn, g_pIRadarStrm);
	}
	if (stream_num == 1)
	{
		Algo_Timing_Info_of_Z7B_core(Radar_Posn, g_pIRadarStrm);
	}
	if (custId == BMW_LOW || custId == BMW_BPIL)
	{
		if (stream_num == 7)
		{
			Free_space_info(Radar_Posn, g_pIRadarStrm);

		}
	}
	if (custId == SCANIA_MAN)
	{
		if (stream_num == 7)
		{
			Free_space_info_Man(Radar_Posn, g_pIRadarStrm);

		}
	}
	if (stream_num == 0)
	{
		Algo_Timing_Info_of_Z7A_core(Radar_Posn, g_pIRadarStrm);

	}
	if (stream_num == 3)
	{
		Algo_Timing_Info_of_Z4_core(Radar_Posn, g_pIRadarStrm);

	}
	if (stream_num == 7)
	{
		ASW_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		LCDA_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		CTA_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		RECW_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		CED_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		TA_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		//Freespace_Output_Info(Radar_Posn, g_pIRadarStrm, custId);
		Record_Header_Count(Radar_Posn, g_pIRadarStrm);
	}




	//VF_Record_Header_Count(g_pIRadarStrm);
	//Raw_vehicle_Info(g_pIRadarStrm);

}


void Populate_ECU_Data_to_XML(FILE* filename,const char* LogFname,short stream_num,IRadarStream * g_pIRadarStrm,unsigned8_T Radar_Posn,  Customer_T custId )	
{
	//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
	char  inp_file[1024] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	char InputLogName[1024] = { 0 };
	//static unsigned16_T Previousscanid = 0;
	strcpy(InputLogName, LogFname);
	Open_XML_file(filename);
	//(_pIRadarStrm->get_complete_Buffer());
	Input_Header_ECU_Info(Radar_Posn, InputLogName, custId, g_pIRadarStrm);
	Record_Header_ECU_Info(g_pIRadarStrm, stream_num);
	Vehicle_ECU_Info(g_pIRadarStrm, stream_num);
	Detection_ECU_Info(g_pIRadarStrm, stream_num);
	PCAN_Detection_Info(g_pIRadarStrm, stream_num);
	Tracks_Info(g_pIRadarStrm, stream_num);
	FeatureFunction_Output(g_pIRadarStrm, stream_num);
	Record_Header_ECU_Count(g_pIRadarStrm, stream_num);
	Core_Execution_Time(g_pIRadarStrm, stream_num);
}

/*Open Output XML File*/
void Open_XML_file(FILE* Filename)
{
	SIL_XML_out = Filename;
}
void Open_CSV_file(FILE* Filename)
{
	SIL_CSV_out = Filename;
}
void Input_Header_Info(unsigned8_T Radar_Posn, const char* LogFname, Customer_T custId, IRadarStream * g_pIRadarStrm)
{
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	int platform = g_pIRadarStrm->m_proc_info.frame_header.Platform;
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ECU_DATA_XML, ECU_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&HOST_DATA, HOST_VERSION);
	//g_pIRadarStrm->getData((unsigned char*)&ECU_DATA_XML,ECU_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&HOST_DATA,HOST_VERSION);
	if (!flag_out)
	{
		fprintf(SIL_XML_out, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		fprintf(SIL_XML_out, "<!-- SRR5 APTIV SIL component configuration -->\n");
		fprintf(SIL_XML_out, "<INPUT_HEADER>");
		char *newline = strchr(pathname_XML, '\n');
		if (newline)
			*newline = 0;

		fprintf(SIL_XML_out, "\n<FILE_NAME>%s</FILE_NAME>\n", InputLogName);
		time_t current_time;
		struct tm* timeinfo;
		time(&current_time);
		timeinfo = localtime(&current_time);
		fprintf(SIL_XML_out, "\t<PC_TIME>%d:%d:%d</PC_TIME>\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		fprintf(SIL_XML_out, "\t<CUSTOMER_NAME>\t %s \t</CUSTOMER_NAME>\n", GetCustName(custId, platform));
		//unsigned8_T Radar_Posn = Get_Current_ECU_Position();
		//switch(Radar_Posn)
		//{
		//case 0x00:fprintf(SIL_XML_out,"\t<RADAR_POS>REAR_LEFT</RADAR_POS>\n");break;
		//case 0x01:fprintf(SIL_XML_out,"\t<RADAR_POS>REAR_RIGHT</RADAR_POS>\n");break;
		//case 0x02:fprintf(SIL_XML_out,"\t<RADAR_POS>FRONT_RIGHT</RADAR_POS>\n");break;
		//case 0x03:fprintf(SIL_XML_out,"\t<RADAR_POS>FRONT_LEFT</RADAR_POS>\n");break;
		//case 0x06:fprintf(SIL_XML_out,"\t<RADAR_POS>BPIL_RIGHT</RADAR_POS>\n");break;
		//case 0x07:fprintf(SIL_XML_out,"\t<RADAR_POS>BPIL_LEFT</RADAR_POS>\n");break;
		//case 0x013:fprintf(SIL_XML_out,"\t<RADAR_POS>FRONT_LEFT</RADAR_POS>\n");break;
		//default://do nothing
		//	break;
		//}
		fprintf(SIL_XML_out, "\t<SW_VERSION>%d-%d-%d-%d\t</SW_VERSION>\n", HOST_DATA.Host_Sw_Version[0], HOST_DATA.Host_Sw_Version[1], HOST_DATA.Host_Sw_Version[2], HOST_DATA.Host_Sw_Version[3]);
		fprintf(SIL_XML_out, "\t<ECU_SERIAL_NO>\t%d-%d-%d-%d\t</ECU_SERIAL_NO>\n", ECU_DATA_XML.ECU_Serial_Num[0], ECU_DATA_XML.ECU_Serial_Num[1], ECU_DATA_XML.ECU_Serial_Num[2], ECU_DATA_XML.ECU_Serial_Num[3]);
		fprintf(SIL_XML_out, "</INPUT_HEADER>\n");
		flag_out = TRUE;
	}
}
void Record_Header_Man_Dpace_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId, short stream_num)
{
	unsigned8_T MAN_valid_track_counter_RL = 0;
	unsigned8_T MAN_valid_track_counter_RR = 0;
	unsigned8_T MAN_valid_track_counter_FR = 0;
	unsigned8_T MAN_valid_track_counter_FL = 0;
	unsigned8_T MAN_valid_track_counter_FC = 0;
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	unsigned8_T count = 1;
	unsigned8_T count_RR = 1;
	unsigned8_T count_FR = 1;
	unsigned8_T count_FC = 1;
	unsigned8_T count_FL = 1;
	//unsigned8_T Radar_Posn = 0;


	/*Get data from MAN Dspace stream */
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Target_Info_Dspace, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh, DSPACE_VEH);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMisc, DSPACE_MISC);

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&rear_left_object, DSPACE_TRK);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&rear_right_object, DSPACE_TRK_RR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&front_left_object, DSPACE_TRK_FL);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&front_right_object, DSPACE_TRK_FR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&front_center_object, DSPACE_TRK_FC);

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_RL, DSPACE_MOUNT_POS_RL);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_RR, DSPACE_MOUNT_POS_RR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_FR, DSPACE_MOUNT_POS_FR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_FL, DSPACE_MOUNT_POS_FL);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_FC, DSPACE_MOUNT_POS_FC);
	
	/*m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_RL, DSPACE_DETECTION_RL);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_RR, DSPACE_DETECTION_RR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_FR, DSPACE_DETECTION_FR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pMountPos_FL, DSPACE_MOUNT_POS_FL);*/
	//------------------------------------------------------------------------------------------------
	/*calculate valid tracks for RL*/
		for (unsigned16_T index = 0; index < SIMULATED_TRACKS_RL; index++)
		{
			if ((rear_left_object[index].status) > 0)
			{
				MAN_valid_track_counter_RL++;
			}
		}
	
	/*calculate valid tracks for RR*/
	for (unsigned int i = SIMULATED_TRACKS_RL, j = 0; i < SIMULATED_TRACKS_RR, j < SIMULATED_TRACKS_RL; i++, j++)
	{
		if ((rear_left_object[i].status) > 0)
		{
			MAN_valid_track_counter_RR++;
		}
	}
	/*calculate valid tracks for FR*/
	for (unsigned int i = SIMULATED_TRACKS_RR, j = 0; i < SIMULATED_TRACKS_FR, j < SIMULATED_TRACKS_RL; i++, j++)
	{
		if ((rear_left_object[i].status) > 0)
		{
			MAN_valid_track_counter_FR++;
		}
	}
	/*calculate valid tracks for FL*/
	for (unsigned int i = SIMULATED_TRACKS_FR, j = 0; i < SIMULATED_TRACKS_FL, j < SIMULATED_TRACKS_RL; i++, j++)
	{
		if ((rear_left_object[i].status) > 0)
		{
			MAN_valid_track_counter_FL++;
		}
	}
	/*calculate valid tracks for FC*/
	for (unsigned16_T index = 0; index < SIMULATED_TRACKS; index++)
	{
		if ((front_center_object[index].status) > 0)
		{
			MAN_valid_track_counter_FC++;
		}
	}
	if (custId != STLA_SRR6P)
	{
		//------------------------------------------------------------------------------------------------
		fprintf(SIL_XML_out, "\n<TRACK_OBJECT_DESCRIPTION_START_%d>", Detdata_RL.det_info.ScanIndex);
		//------------------------------------------------------------------------------------------------
		///*VEHICLE_DATA_T*/
		fprintf(SIL_XML_out, "\n<VEH_DATA_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tveh.abs_speed						 %f\n", p_veh.abs_speed);
		fprintf(SIL_XML_out, "\tveh_curve_radius					 %f\n", p_veh.curve_radius);
		fprintf(SIL_XML_out, "\tveh_f_reverse_gear					 %f\n", p_veh.f_reverse_gear);
		fprintf(SIL_XML_out, "\tveh_f_traffic_side                   %f\n", p_veh.f_traffic_side);
		fprintf(SIL_XML_out, "\tveh_f_trailer_present                %f\n", p_veh.f_trailer_present);
		fprintf(SIL_XML_out, "\tveh_host_vehicle_length              %f\n", p_veh.host_vehicle_length);
		fprintf(SIL_XML_out, "\tveh_host_vehicle_width               %f\n", p_veh.host_vehicle_width);
		fprintf(SIL_XML_out, "\tveh_lane_center_offset_external      %f\n", p_veh.lane_center_offset_external);
		fprintf(SIL_XML_out, "\tveh_lane_width_external              %f\n", p_veh.lane_width_external);
		fprintf(SIL_XML_out, "\tveh_prndl							 %f\n", p_veh.prndl);
		fprintf(SIL_XML_out, "\tveh_rear_axle_position               %f\n", p_veh.rear_axle_position);
		fprintf(SIL_XML_out, "\tveh_rear_axle_steering_angle         %f\n", p_veh.rear_axle_steering_angle);
		fprintf(SIL_XML_out, "\tveh_steering_angle                   %f\n", p_veh.steering_angle);
		fprintf(SIL_XML_out, "\tveh_turn_signal                      %f\n", p_veh.turn_signal);
		fprintf(SIL_XML_out, "\tveh_vcs_lat_acc                      %f\n", p_veh.vcs_lat_acc);
		fprintf(SIL_XML_out, "\tveh_vcs_lat_offset                   %f\n", p_veh.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tveh_vcs_long_acc                     %f\n", p_veh.vcs_long_acc);
		fprintf(SIL_XML_out, "\tveh_vcs_long_offset                  %f\n", p_veh.vcs_long_offset);
		fprintf(SIL_XML_out, "\tveh_vehicle_data_buff_timestamp      %f\n", p_veh.vehicle_data_buff_timestamp);
		fprintf(SIL_XML_out, "\tveh_yawrate                          %f\n", p_veh.yawrate);
		//fprintf(SIL_XML_out, "</VEH_DATA_INFO>\n");
		fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

		//------------------------------------------------------------------------------------------------
		/*RL MOUNT POS INFO*/
		fprintf(SIL_XML_out, "\n<RL_MOUNT_POS_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tpMountPos_RL_azimuth_polarity	%f\n", pMountPos_RL.azimuth_polarity);
		fprintf(SIL_XML_out, "\tpMountPos_RL.boresight_angle	%f\n", pMountPos_RL.boresight_angle);
		fprintf(SIL_XML_out, "\tpMountPos_RL.rear_axle_pos		%f\n", pMountPos_RL.rear_axle_pos);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vcs_lat_offset		%f\n", pMountPos_RL.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vcs_lat_position	%f\n", pMountPos_RL.vcs_lat_position);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vcs_long_offset	%f\n", pMountPos_RL.vcs_long_offset);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vcs_lon_position	%f\n", pMountPos_RL.vcs_lon_position);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vehicle_length		%f\n", pMountPos_RL.vehicle_length);
		fprintf(SIL_XML_out, "\tpMountPos_RL.vehicle_width		%f\n", pMountPos_RL.vehicle_width);
		fprintf(SIL_XML_out, "</RL_MOUNT_POS_INFO>\n");


		/*RR MOUNT POS INFO*/
		fprintf(SIL_XML_out, "\n<RR_MOUNT_POS_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tpMountPos_RR_azimuth_polarity	%f\n", pMountPos_RR.azimuth_polarity);
		fprintf(SIL_XML_out, "\tpMountPos_RR.boresight_angle	%f\n", pMountPos_RR.boresight_angle);
		fprintf(SIL_XML_out, "\tpMountPos_RR.rear_axle_pos		%f\n", pMountPos_RR.rear_axle_pos);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vcs_lat_offset		%f\n", pMountPos_RR.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vcs_lat_position	%f\n", pMountPos_RR.vcs_lat_position);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vcs_long_offset	%f\n", pMountPos_RR.vcs_long_offset);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vcs_lon_position	%f\n", pMountPos_RR.vcs_lon_position);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vehicle_length		%f\n", pMountPos_RR.vehicle_length);
		fprintf(SIL_XML_out, "\tpMountPos_RR.vehicle_width		%f\n", pMountPos_RR.vehicle_width);
		fprintf(SIL_XML_out, "</RR_MOUNT_POS_INFO>\n");


		/*FR MOUNT POS INFO*/
		fprintf(SIL_XML_out, "\n<FR_MOUNT_POS_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tpMountPos_FR_azimuth_polarity	%f\n", pMountPos_FR.azimuth_polarity);
		fprintf(SIL_XML_out, "\tpMountPos_FR.boresight_angle	%f\n", pMountPos_FR.boresight_angle);
		fprintf(SIL_XML_out, "\tpMountPos_FR.rear_axle_pos		%f\n", pMountPos_FR.rear_axle_pos);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vcs_lat_offset		%f\n", pMountPos_FR.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vcs_lat_position	%f\n", pMountPos_FR.vcs_lat_position);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vcs_long_offset	%f\n", pMountPos_FR.vcs_long_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vcs_lon_position	%f\n", pMountPos_FR.vcs_lon_position);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vehicle_length		%f\n", pMountPos_FR.vehicle_length);
		fprintf(SIL_XML_out, "\tpMountPos_FR.vehicle_width		%f\n", pMountPos_FR.vehicle_width);
		fprintf(SIL_XML_out, "</FR_MOUNT_POS_INFO>\n");

		/*FL MOUNT POS INFO*/
		fprintf(SIL_XML_out, "\n<FL_MOUNT_POS_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tpMountPos_FL_azimuth_polarity	%f\n", pMountPos_FL.azimuth_polarity);
		fprintf(SIL_XML_out, "\tpMountPos_FL.boresight_angle	%f\n", pMountPos_FL.boresight_angle);
		fprintf(SIL_XML_out, "\tpMountPos_FL.rear_axle_pos		%f\n", pMountPos_FL.rear_axle_pos);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vcs_lat_offset		%f\n", pMountPos_FL.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vcs_lat_position	%f\n", pMountPos_FL.vcs_lat_position);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vcs_long_offset	%f\n", pMountPos_FL.vcs_long_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vcs_lon_position	%f\n", pMountPos_FL.vcs_lon_position);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vehicle_length		%f\n", pMountPos_FL.vehicle_length);
		fprintf(SIL_XML_out, "\tpMountPos_FL.vehicle_width		%f\n", pMountPos_FL.vehicle_width);
		fprintf(SIL_XML_out, "</FL_MOUNT_POS_INFO>\n");

		/*FC MOUNT POS INFO*/
		fprintf(SIL_XML_out, "\n<FC_MOUNT_POS_INFO_%d>\n", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\tpMountPos_FC_azimuth_polarity	%f\n", pMountPos_FC.azimuth_polarity);
		fprintf(SIL_XML_out, "\tpMountPos_FC.boresight_angle	%f\n", pMountPos_FC.boresight_angle);
		fprintf(SIL_XML_out, "\tpMountPos_FC.rear_axle_pos		%f\n", pMountPos_FC.rear_axle_pos);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vcs_lat_offset		%f\n", pMountPos_FC.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vcs_lat_position	%f\n", pMountPos_FC.vcs_lat_position);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vcs_long_offset	%f\n", pMountPos_FC.vcs_long_offset);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vcs_lon_position	%f\n", pMountPos_FC.vcs_lon_position);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vehicle_length		%f\n", pMountPos_FC.vehicle_length);
		fprintf(SIL_XML_out, "\tpMountPos_FC.vehicle_width		%f\n", pMountPos_FC.vehicle_width);
		fprintf(SIL_XML_out, "</FC_MOUNT_POS_INFO>\n");



		//------------------------------------------------------------------------------------------------
		fprintf(SIL_XML_out, "\n<NO_VALID_TRACKS_RL>\t%d\t</NO_VALID_TRACKS_RL>\n", MAN_valid_track_counter_RL);
		fprintf(SIL_XML_out, "<NO_VALID_TRACKS_RR>\t%d\t</NO_VALID_TRACKS_RR>\n", MAN_valid_track_counter_RR);
		fprintf(SIL_XML_out, "<NO_VALID_TRACKS_FR>\t%d\t</NO_VALID_TRACKS_FR>\n", MAN_valid_track_counter_FR);
		fprintf(SIL_XML_out, "<NO_VALID_TRACKS_FL>\t%d\t</NO_VALID_TRACKS_FL>\n", MAN_valid_track_counter_FL);
		fprintf(SIL_XML_out, "<NO_VALID_TRACKS_FC>\t%d\t</NO_VALID_TRACKS_FC>\n", MAN_valid_track_counter_FC);

		/*Rear Left Sensor Dump*/
		if (MAN_valid_track_counter_RL != 0)
		{
			fprintf(SIL_XML_out, "\n<RL_TRK_OBJ_INFO>id,\tstatus,\tage,\tstage_age,\tref_pos,\tvcs_long_posn,");
			fprintf(SIL_XML_out, "\tvcs_long_vel,\tvcs_long_accel,\tvcs_lat_posn,\tvcs_lat_vel,");
			fprintf(SIL_XML_out, "\tvcs_lat_accel,\tvcs_long_vel_rel,\tvcs_lat_vel_rel,\tspeed,");
			fprintf(SIL_XML_out, "\ttangential_accel,\theading,\theading_rate,\t\tlength,\t\t\twidth,");
			fprintf(SIL_XML_out, "\t\texistence_probability, \tobject_class </TRK_OBJ_INFO > \n");

			fprintf(SIL_XML_out, "\n");

			for (int i = 0; i < SIMULATED_TRACKS_RL; i++)
			{
				if ((rear_left_object[i].status) > 0)
				{
					fprintf(SIL_XML_out, "\t\t\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%u,\n",
						rear_left_object[i].id,
						rear_left_object[i].status,
						rear_left_object[i].age,
						rear_left_object[i].stage_age,
						rear_left_object[i].ref_position,
						rear_left_object[i].vcs_long_posn,
						rear_left_object[i].vcs_long_vel,
						rear_left_object[i].vcs_long_accel,
						rear_left_object[i].vcs_lat_posn,
						rear_left_object[i].vcs_lat_vel,
						rear_left_object[i].vcs_lat_accel,
						rear_left_object[i].vcs_long_vel_rel,
						rear_left_object[i].vcs_lat_vel_rel,
						rear_left_object[i].speed,
						rear_left_object[i].tangential_accel,
						rear_left_object[i].heading,
						rear_left_object[i].heading_rate,
						rear_left_object[i].length,
						rear_left_object[i].width,
						rear_left_object[i].existence_probability,
						rear_left_object[i].object_class);
				}
			}
		}

		/*Rear Right Sensor Dump*/
		if (MAN_valid_track_counter_RR != 0)
		{
			fprintf(SIL_XML_out, "\n<RR_TRK_OBJ_INFO>id,\tstatus,\tage,\tstage_age,\tref_pos,\tvcs_long_posn");
			fprintf(SIL_XML_out, "\tvcs_long_vel,\tvcs_long_accel,\tvcs_lat_posn,\tvcs_lat_vel");
			fprintf(SIL_XML_out, "\tvcs_lat_accel,\tvcs_long_vel_rel,\tvcs_lat_vel_rel,\tspeed");
			fprintf(SIL_XML_out, "\ttangential_accel,\theading,\theading_rate,\t\tlength,\t\t\twidth");
			fprintf(SIL_XML_out, "\t\texistence_probability, \tobject_class </RR_TRK_OBJ_INFO > \n");

			fprintf(SIL_XML_out, "\n");

			for (unsigned int i = SIMULATED_TRACKS_RL; i < SIMULATED_TRACKS_RR; i++)
			{
				if ((rear_left_object[i].status) > 0)
				{
					fprintf(SIL_XML_out, "\t\t\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%u,\n",
						rear_left_object[i].id,
						rear_left_object[i].status,
						rear_left_object[i].age,
						rear_left_object[i].stage_age,
						rear_left_object[i].ref_position,
						rear_left_object[i].vcs_long_posn,
						rear_left_object[i].vcs_long_vel,
						rear_left_object[i].vcs_long_accel,
						rear_left_object[i].vcs_lat_posn,
						rear_left_object[i].vcs_lat_vel,
						rear_left_object[i].vcs_lat_accel,
						rear_left_object[i].vcs_long_vel_rel,
						rear_left_object[i].vcs_lat_vel_rel,
						rear_left_object[i].speed,
						rear_left_object[i].tangential_accel,
						rear_left_object[i].heading,
						rear_left_object[i].heading_rate,
						rear_left_object[i].length,
						rear_left_object[i].width,
						rear_left_object[i].existence_probability,
						rear_left_object[i].object_class);
				}
			}
		}

		/*Front Right Sensor Dump*/
		if (MAN_valid_track_counter_FR != 0)
		{
			fprintf(SIL_XML_out, "\n<FR_TRK_OBJ_INFO>id,\tstatus,\tage,\tstage_age,\tref_pos,\tvcs_long_posn");
			fprintf(SIL_XML_out, "\tvcs_long_vel,\tvcs_long_accel,\tvcs_lat_posn,\tvcs_lat_vel");
			fprintf(SIL_XML_out, "\tvcs_lat_accel,\tvcs_long_vel_rel,\tvcs_lat_vel_rel,\tspeed");
			fprintf(SIL_XML_out, "\ttangential_accel,\theading,\theading_rate,\t\tlength,\t\t\twidth");
			fprintf(SIL_XML_out, "\t\texistence_probability, \tobject_class </FR_TRK_OBJ_INFO > \n");

			fprintf(SIL_XML_out, "\n");

			for (unsigned int i = SIMULATED_TRACKS_RR; i < SIMULATED_TRACKS_FR; i++)
			{
				if ((rear_left_object[i].status) > 0)
				{
					fprintf(SIL_XML_out, "\t\t\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%u,\n",
						rear_left_object[i].id,
						rear_left_object[i].status,
						rear_left_object[i].age,
						rear_left_object[i].stage_age,
						rear_left_object[i].ref_position,
						rear_left_object[i].vcs_long_posn,
						rear_left_object[i].vcs_long_vel,
						rear_left_object[i].vcs_long_accel,
						rear_left_object[i].vcs_lat_posn,
						rear_left_object[i].vcs_lat_vel,
						rear_left_object[i].vcs_lat_accel,
						rear_left_object[i].vcs_long_vel_rel,
						rear_left_object[i].vcs_lat_vel_rel,
						rear_left_object[i].speed,
						rear_left_object[i].tangential_accel,
						rear_left_object[i].heading,
						rear_left_object[i].heading_rate,
						rear_left_object[i].length,
						rear_left_object[i].width,
						rear_left_object[i].existence_probability,
						rear_left_object[i].object_class);
				}
			}
		}

		/*Front Left Sensor Dump*/
		if (MAN_valid_track_counter_FL != 0)
		{
			fprintf(SIL_XML_out, "\n<FL_TRK_OBJ_INFO>id,\tstatus,\tage,\tstage_age,\tref_pos,\tvcs_long_posn");
			fprintf(SIL_XML_out, "\tvcs_long_vel,\tvcs_long_accel,\tvcs_lat_posn,\tvcs_lat_vel");
			fprintf(SIL_XML_out, "\tvcs_lat_accel,\tvcs_long_vel_rel,\tvcs_lat_vel_rel,\tspeed");
			fprintf(SIL_XML_out, "\ttangential_accel,\theading,\theading_rate,\t\tlength,\t\t\twidth");
			fprintf(SIL_XML_out, "\t\texistence_probability, \tobject_class </FL_TRK_OBJ_INFO > \n");

			fprintf(SIL_XML_out, "\n");

			for (unsigned int i = SIMULATED_TRACKS_FR; i < SIMULATED_TRACKS_FL; i++)
			{
				if ((rear_left_object[i].status) > 0)
				{
					fprintf(SIL_XML_out, "\t\t\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%u,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t%f,\t\t\t%u,\n",
						rear_left_object[i].id,
						rear_left_object[i].status,
						rear_left_object[i].age,
						rear_left_object[i].stage_age,
						rear_left_object[i].ref_position,
						rear_left_object[i].vcs_long_posn,
						rear_left_object[i].vcs_long_vel,
						rear_left_object[i].vcs_long_accel,
						rear_left_object[i].vcs_lat_posn,
						rear_left_object[i].vcs_lat_vel,
						rear_left_object[i].vcs_lat_accel,
						rear_left_object[i].vcs_long_vel_rel,
						rear_left_object[i].vcs_lat_vel_rel,
						rear_left_object[i].speed,
						rear_left_object[i].tangential_accel,
						rear_left_object[i].heading,
						rear_left_object[i].heading_rate,
						rear_left_object[i].length,
						rear_left_object[i].width,
						rear_left_object[i].existence_probability,
						rear_left_object[i].object_class);
				}
			}
		}

		/*front_center sensor Dump*/
		if (MAN_valid_track_counter_FC != 0)
		{
			fprintf(SIL_XML_out, "\n<FC_TRK_OBJ_INFO>id,\tstatus,\tage,\tstage_age,\tref_pos,\tvcs_long_posn");
			fprintf(SIL_XML_out, "\tvcs_long_vel,\tvcs_long_accel,\tvcs_lat_posn,\tvcs_lat_vel");
			fprintf(SIL_XML_out, "\tvcs_lat_accel,\tvcs_long_vel_rel,\tvcs_lat_vel_rel,\tspeed");
			fprintf(SIL_XML_out, "\ttangential_accel,\theading,\theading_rate,\tlength,\twidth");
			fprintf(SIL_XML_out, "\t\texistence_probability, \tobject_class </FC_TRK_OBJ_INFO > \n");

			fprintf(SIL_XML_out, "\n");

			for (int i = 0; i < SIMULATED_TRACKS; i++)
			{
				if ((front_center_object[i].status) > 0)
				{
					fprintf(SIL_XML_out, "\t\t\t\t%u,\t%u,\t%u,\t%u,\t%u,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%u,\n",
						front_center_object[i].id,
						front_center_object[i].status,
						front_center_object[i].age,
						front_center_object[i].stage_age,
						front_center_object[i].ref_position,
						front_center_object[i].vcs_long_posn,
						front_center_object[i].vcs_long_vel,
						front_center_object[i].vcs_long_accel,
						front_center_object[i].vcs_lat_posn,
						front_center_object[i].vcs_lat_vel,
						front_center_object[i].vcs_lat_accel,
						front_center_object[i].vcs_long_vel_rel,
						front_center_object[i].vcs_lat_vel_rel,
						front_center_object[i].speed,
						front_center_object[i].tangential_accel,
						front_center_object[i].heading,
						front_center_object[i].heading_rate,
						front_center_object[i].length,
						front_center_object[i].width,
						front_center_object[i].existence_probability,
						front_center_object[i].object_class);
				}
			}
		}

		//------------------------------------------------------------------------------------------------
		fprintf(SIL_XML_out, "\n</TRACK_OBJECT_DESCRIPTION_END_%d>\n", Detdata_RL.det_info.ScanIndex);

		//------------------------------------------------------------------------------------------------
	}
	if (custId == STLA_SRR6P || custId == BMW_SP25_L3 || custId == BMW_SP25_L2)
	{	
		if (Radar_Posn == 0)
		{
			m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata, DSPACE_DETECTION_RL);
		}
		if (Radar_Posn == 1)
		{
			m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata, DSPACE_DETECTION_RR);
		}
		if (Radar_Posn == 2)
		{
			m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata, DSPACE_DETECTION_FR);
		}
		if (Radar_Posn == 3)
		{
			m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata, DSPACE_DETECTION_FL);
		}
		if (Radar_Posn == 5)
		{
			m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata, DSPACE_DETECTION_FC);
		}

		fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\t<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
		unsigned8_T LookType = Detdata.det_info.LookType;
		switch (LookType)//enum of Self Radar LookType
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		default://do nothing
			break;
		}
		unsigned8_T LookID = Detdata.det_info.LookID;
		switch (LookID)//enum of Self Radar LookID
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		default://do nothing
			break;
		}

		//Track_Info 
		unsigned8_T valid_track_counter = 0;
		unsigned8_T count_track = NUMBER_OF_OBJECTS - 1;
		for (unsigned8_T index = 0; index < NUMBER_OF_OBJECTS; index++)  // NUMBER_OF_OBJECTS : need to check with latest macro update. at this time there is no tracks.
		{
			if ((curvi_XML.CurviTracks[count_track].Status) > 0)
			{
				valid_track_counter++;
			}
			count_track--;
		}
		fprintf(SIL_XML_out, "\t<NO_VALID_TRACKS>\t%d\t</NO_VALID_TRACKS>\n", valid_track_counter);
		fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Detdata.det_info.timestamp);
		fprintf(SIL_XML_out, "\t</TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));

		//VEH_DATA_INFO
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh, DSPACE_VEH);
		fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));

		fprintf(SIL_XML_out, "\tabs_speed \t%f\n", p_veh.abs_speed);
		fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh.yawrate);
		fprintf(SIL_XML_out, "\tsteering_angle\t %f\n", p_veh.steering_angle);
		fprintf(SIL_XML_out, "\trear_axle_steering_angle\t %f\n", p_veh.rear_axle_steering_angle);
		fprintf(SIL_XML_out, "\trear_axle_position \t%f\n", p_veh.rear_axle_position);
		fprintf(SIL_XML_out, "\tlane_width_external\t%f\n", p_veh.lane_width_external);
		fprintf(SIL_XML_out, "\tlane_center_offset_external\t  %f\n", p_veh.lane_center_offset_external);
		fprintf(SIL_XML_out, "\thost_vehicle_length \t%f\n", p_veh.host_vehicle_length);
		fprintf(SIL_XML_out, "\thost_vehicle_width\t%f\n", p_veh.host_vehicle_width);
		fprintf(SIL_XML_out, "\tcurve_radius\t %f\n", p_veh.curve_radius);
		fprintf(SIL_XML_out, "\tvcs_long_acc\t %f\n", p_veh.vcs_long_acc);
		fprintf(SIL_XML_out, "\tvcs_lat_acc \t%f\n", p_veh.vcs_lat_acc);
		fprintf(SIL_XML_out, "\tvehicle_data_buff_timestamp\t %d\n", p_veh.vehicle_data_buff_timestamp);
		//fprintf(SIL_XML_out, "\tprndl\t%d\n", VEH_DATA_STLA.prndl);

		fprintf(SIL_XML_out, "\tprndl\t\t\t\t");
		unsigned8_T prndl = p_veh.prndl;
		switch (prndl)
		{
		case 0:fprintf(SIL_XML_out, "PARK"); break;
		case 1:fprintf(SIL_XML_out, "REVERSE"); break;
		case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
		case 3:fprintf(SIL_XML_out, "DRIVE"); break;
		case 4:fprintf(SIL_XML_out, "FOURTH"); break;
		case 5:fprintf(SIL_XML_out, "THIRD"); break;
		case 6:fprintf(SIL_XML_out, "LOW"); break;
		default:fprintf(SIL_XML_out, "INVALID"); break;
		}

		//fprintf(SIL_XML_out, "\tturn_signal\t %d\n", VEH_DATA_STLA.turn_signal);
		//fprintf(SIL_XML_out, "\tf_reverse_gear\t %d\n", VEH_DATA_STLA.f_reverse_gear);
		unsigned8_T turn_signal = p_veh.turn_signal;
		switch (turn_signal)//enum of turn_signal
		{
		case 0:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tNONE"); break;
		case 1:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tLEFT"); break;
		case 2:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tRIGHT"); break;
		default:fprintf(SIL_XML_out, "\n\tturn_signal\t\t\tINVALID"); break;
		}
		unsigned8_T f_reverse_gear = p_veh.f_reverse_gear;
		switch (f_reverse_gear)//enum of f_reverse_gear
		{
		case 0:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
		case 1:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
		default:fprintf(SIL_XML_out, "\n\tf_reverse_gear\t\t\tINVALID"); break;
		}
		fprintf(SIL_XML_out, "\n\tf_trailer_present\t %d\n", p_veh.f_trailer_present);
		fprintf(SIL_XML_out, "\tvcs_lat_offset \t%f\n", p_veh.vcs_lat_offset);
		fprintf(SIL_XML_out, "\tvcs_long_offset\t %f\n", p_veh.vcs_long_offset);
		
		//fprintf(SIL_XML_out, "\t</VEH_DATA_INFO>\n");
		fprintf(SIL_XML_out, "\n\t</VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));

		//MNR_BLOCKAGE_PARAMETER
		//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML, MNR_BLOCKAGE_OUTPUT);
		fprintf(SIL_XML_out, "\n\t<MNR_BLOCKAGE_PARAMETER_%d_%s>", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
	
		//fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
		boolean_T Blocked = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked;
		switch (Blocked)//enum of turn_signal
		{
		case 0:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tFALSE"); break;
		case 1:fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\tTRUE"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr4);
		fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr3);
		fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr2);
		fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr1);
		fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward);
		boolean_T Forward = MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward;
		switch (Forward)//enum of turn_signal
		{
		case 0:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tFALSE"); break;
		case 1:fprintf(SIL_XML_out, "\n\t\tForward\t\t\tTRUE"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var_si);
		fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var);
		fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgPm_mnr_avg);
		fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgRm_mnr_avg);
		fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxPm_mnr_avg);
		fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxRm_mnr_avg);
		fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.V_host);
		
		fprintf(SIL_XML_out, "\n\t</MNR_BLOCKAGE_PARAMETER_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));

		//detections
			
        
        fprintf(SIL_XML_out, "\n\t<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
        fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata.det_info.LookID);
        fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata.det_info.LookType);
        fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata.det_info.timestamp);
        
        static unsigned32_T Valid = 0;
        static unsigned32_T Invalid = 0;
        static unsigned32_T HVC = 0;
        static unsigned32_T Low_SNR = 0;
        static unsigned32_T Angle_Jump = 0;
        for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
        {
			unsigned32_T valid_level = Detdata.det[i].valid;
        	if (valid_level == 1)
        	{
        		Valid++;
        	}
        	if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
        	{
        		Invalid++;
        	}
        	if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
        	{
        		HVC++;
        	}
        	if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
        	{
        		Angle_Jump++;
        	}
        	if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
        	{
        		Low_SNR++;
        	}
        }
        fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
        fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
        fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
        fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
        fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
        Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;
        
        for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
        	/*check for  non zero range*/
        	if ((Detdata.det[i].range != 0) && (count == 1))
        	{
        
				//fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
        		fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle,\texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");
        
        		count = 0;
        	}
        	if (Detdata.det[i].range != 0)
        	{
        		fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].range);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].range_rate);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].azimuth);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].elevation);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].detection_snr);
        		fprintf(SIL_XML_out, "\t\t\t%d", Detdata.det[i].det_id);
        		fprintf(SIL_XML_out, "\t%d", Detdata.det[i].valid);
        		fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].amplitude);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_rcs);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_azimuth);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].existence_probability);
        		fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].multi_target_probability);
        		fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].Mixer_Bias);
        		fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].std_range);
        		fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_range_rate);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].std_elevation);
        		fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].classification);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].ambiguity_id);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].rdd_fp_det_idx);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].bistatic);
        		fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].host_veh_clutter);
        		fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].azimuth_confidence);
        		fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].super_res_target);
        		fprintf(SIL_XML_out, "\t\t</Detection>\n");
        	}
        }
        if (count == 0)
        {
			fprintf(SIL_XML_out, "\t</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
        }
		fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", Detdata.det_info.ScanIndex, GetRadarPosName(Radar_Posn));
	}
	else
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_RL, DSPACE_DETECTION_RL);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_RR, DSPACE_DETECTION_RR);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_FR, DSPACE_DETECTION_FR);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_FL, DSPACE_DETECTION_FL);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_FC, DSPACE_DETECTION_FC);
		//DETECTIONS DATA ADDED
		//Detdata RL

		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_RL>", Detdata_RL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata_RL.det_info.LookID);
		fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata_RL.det_info.LookType);
		fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata_RL.det_info.timestamp);

		static unsigned32_T Valid = 0;
		static unsigned32_T Invalid = 0;
		static unsigned32_T HVC = 0;
		static unsigned32_T Low_SNR = 0;
		static unsigned32_T Angle_Jump = 0;
		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			unsigned32_T valid_level = Detdata_RL.det[i].valid;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for  non zero range*/
			if ((Detdata_RL.det[i].range != 0) && (count == 1))
			{

				fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_RL>", Detdata_RL.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle,\texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");

				count = 0;
			}
			if (Detdata_RL.det[i].range != 0)
			{
				fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].range);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].range_rate);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].elevation);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].detection_snr);
				fprintf(SIL_XML_out, "\t\t\t%d", Detdata_RL.det[i].det_id);
				fprintf(SIL_XML_out, "\t%d", Detdata_RL.det[i].valid);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RL.det[i].amplitude);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].std_rcs);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].std_azimuth);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RL.det[i].multi_target_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RL.det[i].Mixer_Bias);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RL.det[i].std_range);
				fprintf(SIL_XML_out, "\t%f", Detdata_RL.det[i].std_range_rate);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].std_elevation);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RL.det[i].classification);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].ambiguity_id);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].rdd_fp_det_idx);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].bistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RL.det[i].host_veh_clutter);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RL.det[i].azimuth_confidence);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RL.det[i].super_res_target);
				/*fprintf(SIL_XML_out, "\t\t%f", Detdata.det->vel_st);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->az_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->el_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->isBistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->bf_type_az);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->super_res_target);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->super_res_target_type);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->outside_sector);*/

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_RL>\n", Detdata_RL.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
		}

		//Detdata RR
		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_RR>", Detdata_RR.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata_RR.det_info.LookID);
		fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata_RR.det_info.LookType);
		fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata_RR.det_info.timestamp);

		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			unsigned32_T valid_level = Detdata_RR.det[i].valid;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for  non zero range*/

			if ((Detdata_RR.det[i].range != 0) && (count_RR == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_RR>", Detdata_RR.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle, \texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");

				count_RR = 0;
			}
			if (Detdata_RR.det[i].range != 0)
			{
				fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].range);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].range_rate);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].elevation);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].detection_snr);
				fprintf(SIL_XML_out, "\t\t\t%d", Detdata_RR.det[i].det_id);
				fprintf(SIL_XML_out, "\t%d", Detdata_RR.det[i].valid);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RR.det[i].amplitude);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].std_rcs);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].std_azimuth);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RR.det[i].multi_target_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_RR.det[i].Mixer_Bias);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RR.det[i].std_range);
				fprintf(SIL_XML_out, "\t%f", Detdata_RR.det[i].std_range_rate);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].std_elevation);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RR.det[i].classification);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].ambiguity_id);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].rdd_fp_det_idx);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].bistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RR.det[i].host_veh_clutter);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_RR.det[i].azimuth_confidence);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_RR.det[i].super_res_target);
				/*fprintf(SIL_XML_out, "\t\t%f", Detdata.det->vel_st);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->az_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->el_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->isBistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->bf_type_az);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->super_res_target);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->super_res_target_type);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->outside_sector);*/

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count_RR == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_RR>\n", Detdata_RR.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
		}

		//Detdata FR
		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FR>", Detdata_FR.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata_FR.det_info.LookID);
		fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata_FR.det_info.LookType);
		fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata_FR.det_info.timestamp);

		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			unsigned32_T valid_level = Detdata_FR.det[i].valid;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for  non zero range*/

			if ((Detdata_FR.det[i].range != 0) && (count_FR == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FR>", Detdata_FR.det_info.ScanIndex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle, \texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");

				count_FR = 0;
			}
			if (Detdata_FR.det[i].range != 0)
			{
				fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].range_rate);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].elevation);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].detection_snr);
				fprintf(SIL_XML_out, "\t\t\t%d", Detdata_FR.det[i].det_id);
				fprintf(SIL_XML_out, "\t%d", Detdata_FR.det[i].valid);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FR.det[i].amplitude);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].std_rcs);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].std_azimuth);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FR.det[i].multi_target_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FR.det[i].Mixer_Bias);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FR.det[i].std_range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FR.det[i].std_range_rate);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].std_elevation);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FR.det[i].classification);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].ambiguity_id);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].rdd_fp_det_idx);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].bistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FR.det[i].host_veh_clutter);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FR.det[i].azimuth_confidence);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FR.det[i].super_res_target);
				/*fprintf(SIL_XML_out, "\t\t%f", Detdata.det->vel_st);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->az_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->el_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->isBistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->bf_type_az);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->super_res_target);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->super_res_target_type);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->outside_sector);*/

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count_FR == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_FR>\n", Detdata_FR.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
		}

		//Detdata FL

		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FL>", Detdata_FL.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata_FL.det_info.LookID);
		fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata_FL.det_info.LookType);
		fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata_FL.det_info.timestamp);

		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			unsigned32_T valid_level = Detdata_FL.det[i].valid;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for  non zero range*/

			if ((Detdata_FL.det[i].range != 0) && (count_FL == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FL>", Detdata_FL.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle, \texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");

				count_FL = 0;
			}
			if (Detdata_FL.det[i].range != 0)
			{
				fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].range_rate);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].elevation);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].detection_snr);
				fprintf(SIL_XML_out, "\t\t\t%d", Detdata_FL.det[i].det_id);
				fprintf(SIL_XML_out, "\t%d", Detdata_FL.det[i].valid);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FL.det[i].amplitude);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].std_rcs);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].std_azimuth);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FL.det[i].multi_target_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FL.det[i].Mixer_Bias);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FL.det[i].std_range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FL.det[i].std_range_rate);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].std_elevation);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FL.det[i].classification);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].ambiguity_id);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].rdd_fp_det_idx);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].bistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FL.det[i].host_veh_clutter);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FL.det[i].azimuth_confidence);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FL.det[i].super_res_target);
				/*fprintf(SIL_XML_out, "\t\t%f", Detdata.det->vel_st);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->az_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->el_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->isBistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->bf_type_az);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->super_res_target);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->super_res_target_type);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->outside_sector);*/

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count_FL == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_FL>\n", Detdata_FL.det_info.ScanIndex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
		}

		//Detdata_FC
		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FC>", Detdata_FC.det_info.ScanIndex);
		fprintf(SIL_XML_out, "\n\t<LookID>%d</LookID>", Detdata_FC.det_info.LookID);
		fprintf(SIL_XML_out, "\n\t<LookType>%d</LookType>", Detdata_FC.det_info.LookType);
		fprintf(SIL_XML_out, "\n\t<timestamp>%f</timestamp>", Detdata_FC.det_info.timestamp);

		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			unsigned32_T valid_level = Detdata_FC.det[i].valid;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for  non zero range*/

			if ((Detdata_FC.det[i].range != 0) && (count_FC == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_FC>", Detdata_FC.det_info.ScanIndex /*Dets_XML.ScanIndex*/, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\t\trange,\trange_rate,\tazimuth, \televation,\tdetection_snr,\tdet_id,\tvalid_level,\tamplitude, \trcs, \tstd_azimuth_angle, \texistence_probability, \tmulti_target_probability, \tMixer_Bias, \tstd_range,\tstd_r_dot, \tstd_elevation_angle, \tclassification, \tambiguity_id, \trdd_fp_det_idx,\tbi_static_target,\thost_veh_clutter,\tazimuth_confidence,\tsuper_res_target  </DETECTION_INFO>\n");

				count_FC = 0;
			}
			if (Detdata_FC.det[i].range != 0)
			{
				fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].range_rate);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].azimuth);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].elevation);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].detection_snr);
				fprintf(SIL_XML_out, "\t\t\t%d", Detdata_FC.det[i].det_id);
				fprintf(SIL_XML_out, "\t%d", Detdata_FC.det[i].valid);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FC.det[i].amplitude);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].std_rcs);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].std_azimuth);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].existence_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FC.det[i].multi_target_probability);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata_FC.det[i].Mixer_Bias);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FC.det[i].std_range);
				fprintf(SIL_XML_out, "\t%f", Detdata_FC.det[i].std_range_rate);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].std_elevation);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FC.det[i].classification);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].ambiguity_id);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].rdd_fp_det_idx);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].bistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FC.det[i].host_veh_clutter);
				fprintf(SIL_XML_out, "\t\t%f", Detdata_FC.det[i].azimuth_confidence);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata_FC.det[i].super_res_target);
				/*fprintf(SIL_XML_out, "\t\t%f", Detdata.det->vel_st);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->first_pass_index);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->K_Unused16_1);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->az_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->el_conf);
				fprintf(SIL_XML_out, "\t%f", Detdata.det->isBistatic);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->isSingleTarget);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->isSingleTarget_azimuth);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_Doppler_mixed_interval);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->f_valid_Doppler_unfolding);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->bf_type_az);
				fprintf(SIL_XML_out, "\t\t%f", Detdata.det->bf_type_el);
				fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det->super_res_target);
				fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det->super_res_target_type);
				fprintf(SIL_XML_out, "\t\t\t\t\t%f", Detdata.det->outside_sector);*/

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count_FC == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_FC>\n", Detdata_FC.det_info.ScanIndex /*Dets_XML.ScanIndex*/);
		}
		fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_END>");
	}
	


}


///*Print Record Header Info*/
void Record_Header_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId, short stream_num)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_self_XML, DET_SELF);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&curvi_XML, Z7B_TRACKER_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&cdc_frame_XML, CDC_FRAME);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);

	//g_pIRadarStrm->getData((unsigned char*)&Dets_self_XML,DET_SELF);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);

	unsigned8_T valid_track_counter = 0;
	unsigned8_T count_track = NUMBER_OF_OBJECTS - 1;
	//Z7B_Logging_Data_T* p_l_CoreData_Ptr = (Z7B_Logging_Data_T*)Get_RR_Core2_Logging_Data_Ptr();
	fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));


#ifndef DISABLE_TRACKER
	if (custId == BMW_LOW || custId == BMW_BPIL || custId == SCANIA_MAN)
	{

		for (unsigned8_T index = 0; index < NUMBER_OF_OBJECTS; index++)
		{
			if ((curvi_XML.CurviTracks[count_track].Status) > 0)
			{
				valid_track_counter++;
			}
			count_track--;
		}
	}
	else if (custId == BMW_SAT)
	{

#if 0
		for (unsigned8_T index = 0; index < NUMBER_OF_OBJECTS; index++)
		{
			if ((curvi_XML.Status) > 0)
			{
				valid_track_counter++;
			}
			count_track--;
		}
#endif
	}
#endif

	if (custId != TML_SRR5)
	{
		fprintf(SIL_XML_out, "\t<NO_VALID_TRACKS>\t%d\t<NO_VALID_TRACKS>\n", valid_track_counter);
	}
	
	/*Target report info for self Radar*/
	fprintf(SIL_XML_out, "<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

	//char strData[200]={0};
	/////////////floatToString(Dets_XML.timestamp, strData, FIXED);
	fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
	fprintf(SIL_XML_out, "\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n", Dets_XML.ScanIndex);
	unsigned8_T  Self_LookType = Dets_XML.LookType;
	switch (Self_LookType)//enum of Self Radar LookType
	{
	case 0:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
	case 1:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
	case 2:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
	case 3:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
	default://do nothing
		break;
	}
	unsigned8_T Self_LookID = Dets_XML.LookID;
	switch (Self_LookID)//enum of Self Radar LookID
	{
	case 0:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
	case 1:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
	case 2:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
	case 3:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
	default://do nothing
		break;
	}
	fprintf(SIL_XML_out, "\t<TS_CONSIST>\t%d\t</TS_CONSIST>\n", Dets_XML.timestamp_consistency);
	fprintf(SIL_XML_out, "\t<NO_CDC_FRAMES>\t%d\t</NO_CDC_FRAMES>\n", cdc_frame_XML.Number_Of_CDC_Frames);

	fprintf(SIL_XML_out, "\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n", Dets_XML.Count);
	fprintf(SIL_XML_out, "</TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	if (custId == BMW_LOW)
	{
#ifndef DISABLE_TRACKER
		/*Target report info for opposite Radar*/

		fprintf(SIL_XML_out, "\n<TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		//	floatToString(Detections_Opp_SIL.dets_info.timestamp, strData, FIXED);


		fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_Opp_XML.dets_info.timestamp);
		fprintf(SIL_XML_out, "\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n", Dets_Opp_XML.dets_info.ScanIndex);
		unsigned8_T Opp_LookType = Dets_Opp_XML.dets_info.LookType;
		switch (Opp_LookType)//enum of Opp Radar LookType
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		default://do nothing
			break;
		}
		unsigned8_T Opp_LookID = Dets_Opp_XML.dets_info.LookID;
		switch (Opp_LookID)//enum of Opp Radar LookID
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		default://do nothing
			break;
		}
		fprintf(SIL_XML_out, "\t<TS_CONSIST>\t%d\t</TS_CONSIST>\n", Dets_XML.timestamp_consistency);
		fprintf(SIL_XML_out, "\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n", Dets_Opp_XML.dets_info.Count);
		fprintf(SIL_XML_out, "</TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

#endif
	}
}

void Z7a_First_pass_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	int SP_fail_count = 0;
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&FRDD_XML, FIRST_PASS_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&second_paas_xml, Z7A_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);

	//g_pIRadarStrm->getData((unsigned char*)&FRDD_XML,FIRST_PASS_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&second_paas_xml,Z7A_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	if (custId == BMW_LOW || custId == BMW_BPIL)
	{
		fprintf(SIL_XML_out, "<TARGET_REPORT_INFO_SELF_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

		fprintf(SIL_XML_out, "\t<NO_FP_DETS>\t%d\t</NO_FP_DETS>\n", FRDD_XML.first_pass_num_of_targets);
		for (int index = 0; index < FIRST_PASS_MAX_TARGET_REPORTS_L; index++)
		{

			if (second_paas_xml.second_pass_fail_flag[index] == 1)
			{
				SP_fail_count++;
			}
		}

		fprintf(SIL_XML_out, "\t<SP_FAIL_FLAG>\t%d\t</SP_FAIL_FLAG>\n", SP_fail_count);

	}
	//	fprintf(SIL_XML_out,"</TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n",Dets_XML.ScanIndex,GetRadarPosName(Radar_Posn));
}
///*Print Z4 Record Header Info*/
void Z4_Record_header_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);

	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
	//g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
	//	Z7B_Logging_Data_T* p_l_CoreData_Ptr = (Z7B_Logging_Data_T*)Get_RR_Core2_Logging_Data_Ptr();
	if (custId == BMW_LOW)
	{
		fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "<TARGET_REPORT_INFO_OPP_RADAR_%d>\n", Dets_XML.ScanIndex);
#ifndef DISABLE_TRACKER
		//char strData[20];
		//floatToString(Dets_Opp_XML.dets_info.timestamp, strData, FIXED);
		fprintf(SIL_XML_out, "\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_Opp_XML.dets_info.timestamp);

		fprintf(SIL_XML_out, "\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n", Dets_Opp_XML.dets_info.ScanIndex);
		unsigned8_T LookType = Dets_Opp_XML.dets_info.LookType;
		switch (LookType)//enum of LookType
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n"); break;
		default://do nothing
			break;
		}
		unsigned8_T LookID = Dets_Opp_XML.dets_info.LookID;
		switch (LookID)//enum of LookID
		{
		case 0:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 1:fprintf(SIL_XML_out, "\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		case 2:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n"); break;
		case 3:fprintf(SIL_XML_out, "\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n"); break;
		default://do nothing
			break;
		}
		fprintf(SIL_XML_out, "\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n", Dets_Opp_XML.dets_info.Count);
		fprintf(SIL_XML_out, "</TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
#endif
	}
}
///*Print VF RR Record Header Info*/
//void VF_RR_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream *  g_pIRadarStrm)
//{
//	g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
//	g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
//	g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
//	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
//	fprintf(SIL_XML_out,"\n<RECORD_HEADER_%d_%s>\n",Dets_XML.ScanIndex,GetRadarPosName(Radar_Posn));
//	//fprintf(SIL_XML_out,"\t<UTC>\t%d\t</UTC>\n",Get_ND_UTC());
//	fprintf(SIL_XML_out,"<TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n", Dets_XML.ScanIndex,GetRadarPosName(Radar_Posn));
//
//	//char strData[20];
//	//floatToString(Dets_XML.timestamp, strData, FIXED);
//	fprintf(SIL_XML_out,"\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
//
//	fprintf(SIL_XML_out,"\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n",Dets_XML.ScanIndex);
//	unsigned8_T LookType = Dets_XML.LookType;
//	switch(LookType)//enum of LookType
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		default://do nothing
//		break;
//	}
//	unsigned8_T LookID = Dets_XML.LookID;
//	switch(LookID)//enum of LookID
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out,"\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n",Dets_XML.Count);
//	fprintf(SIL_XML_out,"</TARGET_REPORT_INFO_OPP_RADAR_%d_%s>\n",Dets_XML.ScanIndex,GetRadarPosName(Radar_Posn));
//}
/*Print VF RL Record Header Info*/
//void VF_RL_Record_Header_Info(IRadarStream *  g_pIRadarStrm)
//{
//	g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
//	g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
//	g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
//	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
//
//	fprintf(SIL_XML_out,"\n<RECORD_HEADER_%d>\n",Dets_XML.ScanIndex);
//	//fprintf(SIL_XML_out,"\t<UTC>\t%d\t</UTC>\n",Get_ND_UTC());
//	fprintf(SIL_XML_out,"<TARGET_REPORT_INFO_OPP_RADAR_%d>\n", Dets_XML.ScanIndex);
//
//	//char strData[20];
//	//floatToString(Dets_XML.timestamp, strData, FIXED);
//	fprintf(SIL_XML_out,"\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
//
//	fprintf(SIL_XML_out,"\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n",Dets_XML.ScanIndex);
//	unsigned8_T LookType = Dets_XML.LookType;
//	switch(LookType)//enum of LookType
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		default://do nothing
//		break;
//	}
//	unsigned8_T LookID = Dets_XML.LookID;
//	switch(LookID)//enum of LookID
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out,"\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n",Dets_XML.Count);
//	fprintf(SIL_XML_out,"</TARGET_REPORT_INFO_OPP_RADAR_%d>\n",Dets_XML.ScanIndex);
//}

///*Print VF FR Record Header Info*/
//void VF_FR_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream *  g_pIRadarStrm)
//{
//	g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
//	g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
//	g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
//	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
//
//	fprintf(SIL_XML_out,"\n<RECORD_HEADER_%d>\n",Dets_XML.ScanIndex);
//	//fprintf(SIL_XML_out,"\t<UTC>\t%d\t</UTC>\n",Get_ND_UTC());
//	fprintf(SIL_XML_out,"<TARGET_REPORT_INFO_OPP_RADAR_%d>\n", Dets_XML.ScanIndex);
//
//	//char strData[20];
//	//floatToString(Dets_XML.timestamp, strData, FIXED);
//	fprintf(SIL_XML_out,"\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
//
//	fprintf(SIL_XML_out,"\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n",Dets_XML.ScanIndex);
//	unsigned8_T LookType = Dets_XML.LookType;
//	switch(LookType)//enum of LookType
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		default://do nothing
//		break;
//	}
//	unsigned8_T LookID = Dets_XML.LookID;
//	switch(LookID)//enum of LookID
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out,"\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n",Dets_XML.Count);
//	fprintf(SIL_XML_out,"</TARGET_REPORT_INFO_OPP_RADAR_%d>\n",Dets_XML.ScanIndex);
//}

//*Print VF FL Record Header Info*/
//void VF_FL_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream *  g_pIRadarStrm)
//{
//	
//	g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
//	g_pIRadarStrm->getData((unsigned char*)&curvi_XML,Z7B_TRACKER_DATA);
//	g_pIRadarStrm->getData((unsigned char*)&cdc_frame_XML,CDC_FRAME);
//	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
//	fprintf(SIL_XML_out,"\n<RECORD_HEADER_%d>\n",Dets_XML.ScanIndex);
//	//fprintf(SIL_XML_out,"\t<UTC>\t%d\t</UTC>\n",Get_ND_UTC());
//	fprintf(SIL_XML_out,"<TARGET_REPORT_INFO_OPP_RADAR_%d>\n", Dets_XML.ScanIndex);
//
//	//char strData[20];
//	//floatToString(SIL_SRR3_INPUT_DATA_XML->detection_fl_radar->dets_info.timestamp, strData, FIXED);
//	fprintf(SIL_XML_out,"\t<MASTERZIET>\t%f\t</MASTERZIET>\n", Dets_XML.timestamp);
//
//	fprintf(SIL_XML_out,"\t<SCAN_INDEX>\t%d\t</SCAN_INDEX>\n",Dets_XML.ScanIndex);
//	unsigned8_T LookType = Dets_XML.LookType;
//	switch(LookType)//enum of LookType
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tLONG_LOOK\t</LOOK_TYPE>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_TYPE>\tMEDIUM_LOOK\t</LOOK_TYPE>\n");break;
//		default://do nothing
//		break;
//	}
//	unsigned8_T LookID = Dets_XML.LookID;
//	switch(LookID)//enum of LookID
//	{
//		case 0:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 1:fprintf(SIL_XML_out,"\t<LOOK_ID>\tLONG_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		case 2:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-LONG_LOOK\t</LOOK_ID>\n");break;
//		case 3:fprintf(SIL_XML_out,"\t<LOOK_ID>\tMEDIUM_RANGE-MEDIUM_LOOK\t</LOOK_ID>\n");break;
//		default://do nothing
//		break;
//	}
//	fprintf(SIL_XML_out,"\t<NO_VALID_DETS>\t%d\t</NO_VALID_DETS>\n",Dets_XML.Count);
//	fprintf(SIL_XML_out,"</TARGET_REPORT_INFO_OPP_RADAR_%d>\n",Dets_XML.ScanIndex);
//}
///*Print Vehicle Info*/
void Vehicle_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&GPP_VEH, VEH);

	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&GPP_VEH,VEH);*/
	fprintf(SIL_XML_out, "\n<VEHICLE_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<VEHICLE_DESCRIPTION_INFO>\t\t<VEHICLE_PARAMETER>");

	//char strData[200]={0};
	//floatToString(GPP_VEH.abs_speed, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tabs_speed\t\t\t%f", GPP_VEH.abs_speed);

	//floatToString(GPP_VEH.yawrate, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tyawrate\t\t\t\t%f", GPP_VEH.yawrate);

	//floatToString(GPP_VEH.steering_angle, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tsteering_angle\t\t\t%f", GPP_VEH.steering_angle);

	//	floatToString(GPP_VEH.rear_axle_steering_angle, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\trear_axle_steering_angle\t%f", GPP_VEH.rear_axle_steering_angle);

	//floatToString(GPP_VEH.rear_axle_position, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\trear_axle_position\t\t%f", GPP_VEH.rear_axle_position);

	//floatToString(GPP_VEHs.lane_width_external, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tlane_width_external\t\t%f", GPP_VEH.lane_width_external);

	//floatToString(GPP_VEH.lane_center_offset_external, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tlane_center_offset_external\t%f", GPP_VEH.lane_center_offset_external);

	//floatToString(GPP_VEH.host_vehicle_length, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\thost_vehicle_length\t\t%f", GPP_VEH.host_vehicle_length);

	//floatToString(GPP_VEH.host_vehicle_width, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\thost_vehicle_width\t\t%f", GPP_VEH.host_vehicle_width);

	//floatToString(GPP_VEH.curve_radius, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tcurve_radius\t\t\t%f", GPP_VEH.curve_radius);

	//	floatToString(GPP_VEH.vcs_long_acc, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tvcs_long_acc\t\t\t%f", GPP_VEH.vcs_long_acc);

	//	floatToString(GPP_VEH.vcs_lat_acc, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tvcs_lat_acc\t\t\t%f", GPP_VEH.vcs_lat_acc);

	fprintf(SIL_XML_out, "\n\t\tvehicle_data_buff_timestamp\t%d", GPP_VEH.vehicle_data_buff_timestamp);
	fprintf(SIL_XML_out, "\n\t\tprndl\t\t\t\t");
	unsigned8_T prndl = GPP_VEH.prndl;
	switch (prndl)
	{
	case 0:fprintf(SIL_XML_out, "PARK"); break;
	case 1:fprintf(SIL_XML_out, "REVERSE"); break;
	case 2:fprintf(SIL_XML_out, "NEUTRAL"); break;
	case 3:fprintf(SIL_XML_out, "DRIVE"); break;
	case 4:fprintf(SIL_XML_out, "FOURTH"); break;
	case 5:fprintf(SIL_XML_out, "THIRD"); break;
	case 6:fprintf(SIL_XML_out, "LOW"); break;
	default:fprintf(SIL_XML_out, "INVALID"); break;
	}
	unsigned8_T turn_signal = GPP_VEH.turn_signal;
	switch (turn_signal)//enum of turn_signal
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tturn_signal\t\t\tNONE"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tturn_signal\t\t\tLEFT"); break;
	case 2:fprintf(SIL_XML_out, "\n\t\tturn_signal\t\t\tRIGHT"); break;
	default:fprintf(SIL_XML_out, "\n\t\tturn_signal\t\t\tINVALID"); break;
	}
	unsigned8_T f_reverse_gear = GPP_VEH.f_reverse_gear;
	switch (f_reverse_gear)//enum of f_reverse_gear
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tf_reverse_gear\t\t\tNO_REVERSE_GEAR"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tf_reverse_gear\t\t\tREVERSE_GEAR"); break;
	default:fprintf(SIL_XML_out, "\n\t\tf_reverse_gear\t\t\tINVALID"); break;
	}
	unsigned8_T f_trailer_present = GPP_VEH.f_trailer_present;
	switch (f_trailer_present)//enum of f_trailer_present
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tf_trailer_present\t\ttrailer absent"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tf_trailer_present\t\ttrailer present"); break;
	default:fprintf(SIL_XML_out, "\n\t\tf_trailer_present\t\tINVALID"); break;
	}
	unsigned8_T f_traffic_side = GPP_VEH.f_traffic_side;
	switch (f_traffic_side)//enum of f_traffic_side
	{
	case 0:fprintf(SIL_XML_out, "\n\t\tf_traffic_side\t\t\tRight hand traffic"); break;
	case 1:fprintf(SIL_XML_out, "\n\t\tf_traffic_side\t\t\tLeft hand traffic"); break;
	default:fprintf(SIL_XML_out, "\n\t\tf_traffic_side\t\t\tINVALID"); break;
	}
	fprintf(SIL_XML_out, "\n</VEHICLE_DESCRIPTION_INFO>\t\t</VEHICLE_PARAMETER>");
	fprintf(SIL_XML_out, "\n</VEHICLE_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}
#if 0
/*Print core 0 vehicle Info*/
void Raw_vehicle_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	g_pIRadarStrm->getData((unsigned char*)&Dets_XML, TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&VEH_DATA, VEHICLE_DATA);
	unsigned8_T index = 0;
	fprintf(SIL_XML_out, "\n<RAW_VEHICLE_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<RAW_VEHICLE_DESCRIPTION_INFO>\t\t<RAW_VEHICLE_PARAMETER>");

	//char strData[20];
	//floatToString(VEH_DATA.Vehicle_Data_Time_Stamp, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tVehicle_Data_Time_Stamp\t\t%f", VEH_DATA.Vehicle_Data_Time_Stamp);

	//floatToString(VEH_DATA.Yaw_Rate_Compensated, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tYaw_Rate_Compensated\t\t%f", VEH_DATA.Yaw_Rate_Compensated);

	//floatToString(VEH_DATA.Yaw_Rate_Uncompensated, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tYaw_Rate_Uncompensated\t\t%f", VEH_DATA.Yaw_Rate_Uncompensated);

	//floatToString(VEH_DATA.Radius_of_curvature, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tRadius_of_curvature\t\t%f", VEH_DATA.Radius_of_curvature);

	//floatToString(VEH_DATA.Veh_Speed_mps, strData, FIXED);;
	fprintf(SIL_XML_out, "\n\tVeh_Speed_mps\t\t\t%f", VEH_DATA.Veh_Speed_mps);

	//floatToString(VEH_DATA.Steering_Angle, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tSteering_Angle\t\t\t%f", VEH_DATA.Steering_Angle);

	//floatToString(VEH_DATA.long_accel, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tlong_accel\t\t\t%f", VEH_DATA.long_accel);

	//floatToString(VEH_DATA.lat_accel, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tlat_accel\t\t\t%f", VEH_DATA.lat_accel);

	//floatToString(VEH_DATA.Rear_Steering_Angle, strData, FIXED);
	fprintf(SIL_XML_out, "\n\tRear_Steering_Angle\t\t%f", VEH_DATA.Rear_Steering_Angle);

	fprintf(SIL_XML_out, "\n\tLane_Width\t\t\t%d", VEH_DATA.Lane_Width);
	fprintf(SIL_XML_out, "\n\tLane_Center_Offset\t\t%d", VEH_DATA.Lane_Center_Offset);
	fprintf(SIL_XML_out, "\n\tSteering_Angle_Offset\t\t%d", VEH_DATA.Steering_Angle_Offset);
	fprintf(SIL_XML_out, "\n\tVeh_TurnSignalState\t\t%d", VEH_DATA.Veh_TurnSignalState);
	fprintf(SIL_XML_out, "\n\tVeh_ReverseDirection\t\t%d", VEH_DATA.Veh_ReverseDirection);
	fprintf(SIL_XML_out, "\n\ttraffic_side\t\t\t%d", VEH_DATA.traffic_side);
	fprintf(SIL_XML_out, "\n</RAW_VEHICLE_DESCRIPTION_INFO>\t\t</RAW_VEHICLE_PARAMETER>");
	fprintf(SIL_XML_out, "\n</RAW_VEHICLE_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}

#endif
///*Print Mounting Info*/
void Mounting_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Gpp_Caf_XML, GPP_CAF_INFO);

	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Gpp_Caf_XML,GPP_CAF_INFO);*/

	if (custId == BMW_LOW || custId == BMW_SAT || custId == SCANIA_MAN)
	{
		fprintf(SIL_XML_out, "\n<MOUNTING_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n<MOUNT_DESCRIPTION_INFO>");
		fprintf(SIL_XML_out, "\t<MOUNTING_PARAMETER>");
		unsigned8_T index = 0;
		//char strData[20];
		//floatToString(Gpp_Caf_XML.k_veh_width, strData, FIXED);
		/*fprintf(SIL_XML_out, "\n\t\tk_veh_width\t\t%f", Gpp_Caf_XML.k_veh_width);

		//floatToString(Gpp_Caf_XML.k_veh_length, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tk_veh_length\t\t%f", Gpp_Caf_XML.k_veh_length);

		//floatToString(Gpp_Caf_XML.vcs_offset_lat, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_offset_lat\t\t%f", Gpp_Caf_XML.vcs_offset_lat);

		//floatToString(Gpp_Caf_XML.vcs_offset_long, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_offset_long\t\t%f", Gpp_Caf_XML.vcs_offset_long);*/
		fprintf(SIL_XML_out, "\n<MOUNT_SELF_INFO>");

		//floatToString(Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tazimuth_polarity\t%f", Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_boresight_angle[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tboresight_angle\t\t%f", Gpp_Caf_XML.k_in_sensor_boresight_angle[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_lat_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_long_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index]);
		fprintf(SIL_XML_out, "\n</MOUNT_SELF_INFO>");
		index++;
		fprintf(SIL_XML_out, "\n<MOUNT_OPP_INFO>");

		//floatToString(Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tazimuth_polarity\t%f", Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_boresight_angle[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tboresight_angle\t\t%f", Gpp_Caf_XML.k_in_sensor_boresight_angle[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_lat_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index]);

		//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index], strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tvcs_long_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index]);

		fprintf(SIL_XML_out, "\n</MOUNT_OPP_INFO>");

		fprintf(SIL_XML_out, "\n</MOUNT_DESCRIPTION_INFO>\t</MOUNTING_PARAMETER>");
		fprintf(SIL_XML_out, "\n</MOUNTING_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
	else
		if (custId == BMW_BPIL)
		{
			fprintf(SIL_XML_out, "\n<MOUNTING_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<MOUNT_DESCRIPTION_INFO>");
			fprintf(SIL_XML_out, "\t<MOUNTING_PARAMETER>");
			unsigned8_T index = 1;
			//char strData[20];
			//floatToString(Gpp_Caf_XML.k_veh_width, strData, FIXED);
			/*fprintf(SIL_XML_out, "\n\t\tk_veh_width\t\t%f", Gpp_Caf_XML.k_veh_width);//As part of New symbol Interface update these varibles removed

			//floatToString(Gpp_Caf_XML.k_veh_length, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tk_veh_length\t\t%f", Gpp_Caf_XML.k_veh_length);

			//floatToString(Gpp_Caf_XML.vcs_offset_lat, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvcs_offset_lat\t\t%f", Gpp_Caf_XML.vcs_offset_lat);

			//floatToString(Gpp_Caf_XML.vcs_offset_long, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvcs_offset_long\t\t%f", Gpp_Caf_XML.vcs_offset_long); */
			fprintf(SIL_XML_out, "\n<MOUNT_SELF_INFO>");

			//floatToString(Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index], strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tazimuth_polarity\t%f", Gpp_Caf_XML.k_in_sensor_azimuth_polarity[index]);

			//floatToString(Gpp_Caf_XML.k_in_sensor_boresight_angle[index], strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tboresight_angle\t\t%f", Gpp_Caf_XML.k_in_sensor_boresight_angle[index]);

			//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index], strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvcs_lat_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_lat_posn[index]);

			//floatToString(Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index], strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvcs_long_posn\t\t%f", Gpp_Caf_XML.k_in_sensor_vcs_long_posn[index]);
			fprintf(SIL_XML_out, "\n</MOUNT_SELF_INFO>");
			index--;
			fprintf(SIL_XML_out, "\n</MOUNT_DESCRIPTION_INFO>\t</MOUNTING_PARAMETER>");
			fprintf(SIL_XML_out, "\n</MOUNTING_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
}
///*Print MNR blockage Info*/			
void MNR_Blockage_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	if (custId == BMW_LOW || custId == BMW_SAT)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML, MNR_BLOCKAGE_OUTPUT);

		/*	g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
			g_pIRadarStrm->getData((unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML,MNR_BLOCKAGE_OUTPUT);*/

		fprintf(SIL_XML_out, "\n<MNR_BLOCKAGE_OUTPUT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n<MNR_BLOCKAGE_OUTPUT_INFO>\t<MNR_BLOCKAGE_PARAMETER>");
		fprintf(SIL_XML_out, "\n\t\tBlocked\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Blocked);
		fprintf(SIL_XML_out, "\n\t\tBmnr4\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr4);
		fprintf(SIL_XML_out, "\n\t\tBmnr3\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr3);
		fprintf(SIL_XML_out, "\n\t\tBmnr2\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr2);
		fprintf(SIL_XML_out, "\n\t\tBmnr1\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Bmnr1);
		fprintf(SIL_XML_out, "\n\t\tForward\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.Forward);
		//char strData[20];
		//floatToString(Blockage_output_XML.mnr_blockage.beta_var_si, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tbeta_var_si\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var_si);

		//floatToString(Blockage_output_XML.mnr_blockage.beta_var, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tbeta_var\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.beta_var);

		//floatToString(Blockage_output_XML.mnr_blockage.AvgPm_mnr_avg, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tAvgPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgPm_mnr_avg);

		//floatToString(Blockage_output_XML.mnr_blockage.AvgRm_mnr_avg, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tAvgRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.AvgRm_mnr_avg);

		//floatToString(Blockage_output_XML.mnr_blockage.MaxPm_mnr_avg, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tMaxPm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxPm_mnr_avg);

		//floatToString(Blockage_output_XML.mnr_blockage.MaxRm_mnr_avg, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tMaxRm_mnr_avg\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.MaxRm_mnr_avg);

		//floatToString(Blockage_output_XML.mnr_blockage.V_host, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\tV_host\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.mnr_blockage.V_host);
		fprintf(SIL_XML_out, "\n</MNR_BLOCKAGE_OUTPUT_INFO>\t</MNR_BLOCKAGE_PARAMETER>");
		fprintf(SIL_XML_out, "\n</MNR_BLOCKAGE_OUTPUT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
///*Print TD blockage Info*/			
void TD_Blockage_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML, MNR_BLOCKAGE_OUTPUT);
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&MNR_BLOCKAGE_OUTPUT_XML,MNR_BLOCKAGE_OUTPUT);*/
	fprintf(SIL_XML_out, "\n<TD_BLOCKAGE_OUTPUT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<TD_BLOCKAGE_OUTPUT_INFO>\t\t<TD_BLOCKAGE_PARAMETER>");
	fprintf(SIL_XML_out, "\n\t\tf_Blockage_Partial\t\t%u", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.f_Blockage_Partial);
	fprintf(SIL_XML_out, "\n\t\tTD_Scan_Timer\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.TD_Scan_Timer);
	fprintf(SIL_XML_out, "\n\t\tTD_Scan_Counter\t\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.TD_Scan_Counter);
	fprintf(SIL_XML_out, "\n\t\ttracks_dropped_by_TDC\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.tracks_dropped_by_TDC);
	fprintf(SIL_XML_out, "\n\t\tblkg_clear_scan_timer\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.blkg_clear_scan_timer);
	fprintf(SIL_XML_out, "\n\t\ttrack_clear_scan_timer\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.track_clear_scan_timer);
	fprintf(SIL_XML_out, "\n\t\ttrack_clear_counter\t\t%d", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.track_clear_counter);

	//	char strData[20];
	//	floatToString(Blockage_output_XML.td_blockage.TD_max_range, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tTD_max_range\t\t\t%f", MNR_BLOCKAGE_OUTPUT_XML.td_blockage.TD_max_range);

	fprintf(SIL_XML_out, "\n</TD_BLOCKAGE_OUTPUT_INFO>\t\t</TD_BLOCKAGE_PARAMETER>");
	fprintf(SIL_XML_out, "\n</TD_BLOCKAGE_OUTPUT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}






///*Print Self Dets*/	MAN_SRR3
void Self_Radar_Info_MAN(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_self_MAN_XML, DETS);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);

	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_OBJECTS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
	{
		Self_Dets[i] = Dets_self_MAN_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}


	array_data = Self_Dets;
	DetectionsFlag = true;
	//sort(index, index+size, greater<int>());
	qsort(index, size, sizeof(*index), compare);
	DetectionsFlag = false;
	/*Self Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
	{
		unsigned8_T valid_level = Dets_self_MAN_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
	{
		/*check for valid detection*/

		if (((Dets_self_MAN_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,valid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_self_MAN_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");
			//char strData[20];
			//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_self_MAN_XML.dets_float[index[i]].range);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_self_MAN_XML.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_self_MAN_XML.dets_float[index[i]].azimuth_raw);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_self_MAN_XML.dets_float[index[i]].amplitude);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].detection_snr, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_self_MAN_XML.dets_float[index[i]].detection_snr);
			unsigned8_T valid_level = Dets_self_MAN_XML.dets_float[index[i]].valid_level;

			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
//changan self dets 
void Self_Radar_Info_CHANGAN(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pDets_self_CHANAGN_XML, DET_SELF);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Radar_Info_XML, RADAR_PARAM);

	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		Self_Dets[i] = pDets_self_CHANAGN_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}

	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);

	/*Self Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = pDets_self_CHANAGN_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid detection*/
		if (((pDets_self_CHANAGN_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,\televation_raw\t,valid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((pDets_self_CHANAGN_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");
			fprintf(SIL_XML_out, "\t%f", pDets_self_CHANAGN_XML.dets_float[index[i]].range);
			fprintf(SIL_XML_out, "\t%f", pDets_self_CHANAGN_XML.dets_float[index[i]].range_rate_raw);
			fprintf(SIL_XML_out, "\t%f", pDets_self_CHANAGN_XML.dets_float[index[i]].azimuth_raw);
			fprintf(SIL_XML_out, "\t%f", pDets_self_CHANAGN_XML.dets_float[index[i]].amplitude);
			fprintf(SIL_XML_out, "\t%f", pDets_self_CHANAGN_XML.dets_float[index[i]].detection_snr);
			unsigned8_T valid_level = pDets_self_CHANAGN_XML.dets_float[index[i]].valid_level;

			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
//RNA self dets 
void Self_Radar_Info_RNA(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_self_RNA, DET_SELF);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Radar_Info_XML, RADAR_PARAM);

	/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		Self_Dets[i] = Dets_self_RNA.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}

	array_data1 = Self_Dets;
	qsort(index, size, sizeof(*index), compare1);

	/*Self Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_self_RNA.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted self dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid detection*/
		if (((Dets_self_RNA.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,\televation_raw\t,valid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_self_RNA.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");
			fprintf(SIL_XML_out, "\t%f", Dets_self_RNA.dets_float[index[i]].range);
			fprintf(SIL_XML_out, "\t%f", Dets_self_RNA.dets_float[index[i]].range_rate_raw);
			fprintf(SIL_XML_out, "\t%f", Dets_self_RNA.dets_float[index[i]].azimuth_raw);
			fprintf(SIL_XML_out, "\t%f", Dets_self_RNA.dets_float[index[i]].amplitude);
			fprintf(SIL_XML_out, "\t%f", Dets_self_RNA.dets_float[index[i]].detection_snr);
			unsigned8_T valid_level = Dets_self_RNA.dets_float[index[i]].valid_level;

			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
///*Print Self Dets*/			
void Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	/*DVSU_RECORD_T record = {0};
	Customer_T custId;
	custId = (Customer_T)record.payload[23];*/

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_self_XML, DET_SELF);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Radar_Info_XML, RADAR_PARAM);

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_RL, DSPACE_DETECTION_RL);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_RR, DSPACE_DETECTION_RR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_FR, DSPACE_DETECTION_FR);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_FL, DSPACE_DETECTION_FL);

	/*g_pIRadarStrm->getData((unsigned char*)&Dets_self_XML,DET_SELF);
g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);
g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
g_pIRadarStrm->getData((unsigned char*)&Radar_Info_XML,RADAR_PARAM);*/
/*sort by range*/
	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		Self_Dets[i] = Dets_self_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}

	if (custId == BMW_LOW || custId == BMW_BPIL|| custId == TML_SRR5)
	{
		array_data = Self_Dets;
		DetectionsFlag = true;
		//sort(index, index+size, greater<int>());
		qsort(index, size, sizeof(*index), compare);
		DetectionsFlag = false;
	}
	else
	{
		array_data1 = Self_Dets;
		qsort(index, size, sizeof(*index), compare1);
	}

	/*Self Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_self_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted self dets*/
	unsigned8_T count = 1;
	if (!f_dspace_enabled)
	{
		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for valid detection*/
			if (custId == BMW_LOW || custId == BMW_SAT || custId == TML_SRR5)
			{
				if (((Dets_self_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
				{
					fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
					fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,valid_level\t,det_id\t,elevation_raw\t,multi_target_probability,rdd_fp_det_idx\t,snr\t,std_azimuth_angle,std_elevation_angle,std_range\t,std_rcs\t,std_r_dot\t</DETECTION_INFO>\n");
					count = 0;
				}
			}
			else if (custId == BMW_BPIL)
			{
				if (((Dets_self_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
				{
					fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
					fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,\televation_raw\t,valid_level\t,det_id\t,elevation_raw\t,multi_target_probability\t,rdd_fp_det_idx\t,snr\t,std_azimuth_angle\t,std_elevation_angle\t,std_range\t,std_rcs\t,std_r_dot\t</DETECTION_INFO>\n");
					count = 0;
				}
			}
			if ((Dets_self_XML.dets_float[index[i]].valid_level) > 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");
				//char strData[20];
				//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].range);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].range_rate_raw);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].azimuth_raw);

				//	floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].amplitude);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].detection_snr, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].detection_snr);
				unsigned8_T valid_level = Dets_self_XML.dets_float[index[i]].valid_level;

				switch (valid_level)//print enum of validity level 
				{
				case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
				case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
				case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
				case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
				case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
				case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
				case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
				case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
				default:fprintf(SIL_XML_out, "\tDNE\t"); break;
				}
				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].det_id);

				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].elevation_raw);

				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].multi_target_probability);

				fprintf(SIL_XML_out, "\t\t\t%f", Dets_self_XML.dets_float[index[i]].rdd_fp_det_idx);

				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].snr);

				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].std_azimuth_angle);

				fprintf(SIL_XML_out, "\t\t%f", Dets_self_XML.dets_float[index[i]].std_elevation_angle);

				fprintf(SIL_XML_out, "\t\t\t%f", Dets_self_XML.dets_float[index[i]].std_range);

				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].std_rcs);

				fprintf(SIL_XML_out, "\t%f", Dets_self_XML.dets_float[index[i]].std_r_dot);

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
	}
	else
	{
		/*check for valid detection*/
		for (unsigned32_T i = 0; i < NUMBER_OF_DETS; i++)
		{
			if (custId == BMW_LOW || custId == BMW_SAT || custId == TML_SRR5)
			{
				if (((Dets_self_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
				{
					fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
					fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,valid_level\t,det_id\t,elevation_raw\t,multi_target_probability,rdd_fp_det_idx\t,snr\t,std_azimuth_angle,std_elevation_angle,std_range\t,std_rcs\t,std_r_dot\t</DETECTION_INFO>\n");
					count = 0;
				}
			}
			else if (custId == BMW_BPIL)
			{
				if (((Dets_self_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
				{
					fprintf(SIL_XML_out, "\n<DETECTION_SELF_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
					fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw,\t azimuth_raw,\t amplitude,\t detection_snr,\televation_raw\t,valid_level\t,det_id\t,elevation_raw\t,multi_target_probability\t,rdd_fp_det_idx\t,snr\t,std_azimuth_angle\t,std_elevation_angle\t,std_range\t,std_rcs\t,std_r_dot\t</DETECTION_INFO>\n");
					count = 0;
				}
			}
			//if ((Dets_RL.dets_float[index[i]].valid_level) > 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");
				//char strData[20];
				//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->range);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->range_rate);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->azimuth);

				//	floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->amplitude);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].detection_snr, strData, FIXED); 
				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->detection_snr);
				unsigned8_T valid_level = Dets_RL.det->valid;

				switch (valid_level)//print enum of validity level 
				{
				case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
				case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
				case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
				case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
				case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
				case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
				case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
				case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
				default:fprintf(SIL_XML_out, "\tDNE\t"); break;
				}
				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->det_id);

				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->elevation);

				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->multi_target_probability);

				fprintf(SIL_XML_out, "\t\t\t%f", Dets_RL.det->rdd_fp_det_idx);

				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->snr);

				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->std_azimuth);

				fprintf(SIL_XML_out, "\t\t%f", Dets_RL.det->std_elevation);

				fprintf(SIL_XML_out, "\t\t\t%f", Dets_RL.det->std_range);

				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->std_rcs);

				fprintf(SIL_XML_out, "\t%f", Dets_RL.det->std_range_rate);

				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_SELF_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
/*print opp dets*/
void Opposite_Radar_Info_MAN(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{

	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_MAN_XML, OPP_DETS);

	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_MAN_XML,DET_OPP);*/
#ifndef DISABLE_TRACKER
	///Z7B_Logging_Data_T* p_l_CoreData_Ptr = (Z7B_Logging_Data_T*)Get_RR_Core2_Logging_Data_Ptr();
	// H2A_Operational_Msg_T* p_l_H2A_CoreData_Ptr = (H2A_Operational_Msg_T*)Get_H2A_App_Msg_Ptr();
	/*sort by range*/
	float32_T Opp_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		Opp_Dets[i] = Dets_Opp_MAN_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Opp_Dets) / sizeof(*Opp_Dets);
	unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = Opp_Dets;
	DetectionsOppFlag = true;
	qsort(index, size, sizeof(*index), compare);
	DetectionsOppFlag = false;

	/*Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		unsigned8_T valid_level = Dets_Opp_MAN_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		/*check for valid level of opp dets*/
		if (((Dets_Opp_MAN_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude,\tdetection_snr,\tvalid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_Opp_MAN_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//	char strData[20];
			//	floatToString(Dets_Opp_MAN_XML.dets_float[index[i]].range, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_MAN_XML.dets_float[index[i]].range);

			//	floatToString(Dets_Opp_MAN_XML.dets_float[index[i]].range_rate_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_MAN_XML.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_Opp_MAN_XML.dets_float[index[i]].azimuth_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_MAN_XML.dets_float[index[i]].azimuth_raw);

			//floatToString(Dets_Opp_MAN_XML.dets_float[index[i]].amplitude, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_MAN_XML.dets_float[index[i]].amplitude);

			//	floatToString(Dets_Opp_MAN_XML.dets_float[index[i]].detection_snr, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_MAN_XML.dets_float[index[i]].detection_snr);

			unsigned8_T valid_level = Dets_Opp_MAN_XML.dets_float[index[i]].valid_level;
			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
#endif
}
/*Print CHAGAN Opp Dets*/
void Opposite_Radar_Info_RNA(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_opp_RNA, DET_OPP);

#ifndef DISABLE_TRACKER
	float32_T Opp_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		Opp_Dets[i] = Dets_opp_RNA.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Opp_Dets) / sizeof(*Opp_Dets);
	unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = Opp_Dets;
	DetectionsOppFlag = true;
	qsort(index, size, sizeof(*index), compare);
	DetectionsOppFlag = false;

	/*Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		unsigned8_T valid_level = Dets_opp_RNA.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		/*check for valid level of opp dets*/
		if (((Dets_opp_RNA.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude,\tdetection_snr,\tvalid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_opp_RNA.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//	char strData[20];
			//	floatToString(Dets_opp_RNA.dets_float[index[i]].range, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_opp_RNA.dets_float[index[i]].range);

			//	floatToString(Dets_opp_RNA.dets_float[index[i]].range_rate_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_opp_RNA.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_opp_RNA.dets_float[index[i]].azimuth_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_opp_RNA.dets_float[index[i]].azimuth_raw);

			//floatToString(Dets_opp_RNA.dets_float[index[i]].amplitude, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_opp_RNA.dets_float[index[i]].amplitude);

			//	floatToString(Dets_opp_RNA.dets_float[index[i]].detection_snr, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", Dets_opp_RNA.dets_float[index[i]].detection_snr);

			unsigned8_T valid_level = Dets_opp_RNA.dets_float[index[i]].valid_level;
			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
#endif

}
/*Print CHAGAN Opp Dets*/
void Opposite_Radar_Info_CHANGAN(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&pDets_opp_CHANAGN_XML, DET_OPP);

#ifndef DISABLE_TRACKER
	float32_T Opp_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		Opp_Dets[i] = pDets_opp_CHANAGN_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(Opp_Dets) / sizeof(*Opp_Dets);
	unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = Opp_Dets;
	DetectionsOppFlag = true;
	qsort(index, size, sizeof(*index), compare);
	DetectionsOppFlag = false;

	/*Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		unsigned8_T valid_level = pDets_opp_CHANAGN_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
	{
		/*check for valid level of opp dets*/
		if (((pDets_opp_CHANAGN_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude,\tdetection_snr,\tvalid_level\t</DETECTION_INFO>\n");
			count = 0;
		}
		if ((pDets_opp_CHANAGN_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//	char strData[20];
			//	floatToString(pDets_opp_CHANAGN_XML.dets_float[index[i]].range, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", pDets_opp_CHANAGN_XML.dets_float[index[i]].range);

			//	floatToString(pDets_opp_CHANAGN_XML.dets_float[index[i]].range_rate_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", pDets_opp_CHANAGN_XML.dets_float[index[i]].range_rate_raw);

			//floatToString(pDets_opp_CHANAGN_XML.dets_float[index[i]].azimuth_raw, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", pDets_opp_CHANAGN_XML.dets_float[index[i]].azimuth_raw);

			//floatToString(pDets_opp_CHANAGN_XML.dets_float[index[i]].amplitude, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", pDets_opp_CHANAGN_XML.dets_float[index[i]].amplitude);

			//	floatToString(pDets_opp_CHANAGN_XML.dets_float[index[i]].detection_snr, strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", pDets_opp_CHANAGN_XML.dets_float[index[i]].detection_snr);

			unsigned8_T valid_level = pDets_opp_CHANAGN_XML.dets_float[index[i]].valid_level;
			switch (valid_level)//print enum of validity level 
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
#endif

}

/*Print Opp Dets*/
void Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	//DVSU_RECORD_T record = {0};

	//DVSU_RECORD_T record = {0};


	if (custId == BMW_LOW || custId == TML_SRR5)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);

		/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
		g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
#ifndef DISABLE_TRACKER
		///Z7B_Logging_Data_T* p_l_CoreData_Ptr = (Z7B_Logging_Data_T*)Get_RR_Core2_Logging_Data_Ptr();
	   // H2A_Operational_Msg_T* p_l_H2A_CoreData_Ptr = (H2A_Operational_Msg_T*)Get_H2A_App_Msg_Ptr();
		/*sort by range*/
		float32_T Opp_Dets[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
		}
		unsigned8_T size = sizeof(Opp_Dets) / sizeof(*Opp_Dets);
		unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Opp_Dets;
		DetectionsOppFlag = true;
		qsort(index, size, sizeof(*index), compare);
		DetectionsOppFlag = false;

		/*Opp Dets validity levels count*/
		static unsigned8_T Valid = 0;
		static unsigned8_T Invalid = 0;
		static unsigned8_T HVC = 0;
		static unsigned8_T Low_SNR = 0;
		static unsigned8_T Angle_Jump = 0;
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
		{
			/*check for valid level of opp dets*/
			if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude,\tdetection_snr,\tvalid_level\t</DETECTION_INFO>\n");
				count = 0;
			}
			if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");

				//	char strData[20];
			//	floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

				//	floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);

				//	floatToString(Dets_Opp_XML.dets_float[index[i]].detection_snr, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].detection_snr);

				unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
				switch (valid_level)//print enum of validity level 
				{
				case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
				case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
				case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
				case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
				case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
				case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
				case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
				case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
				default:fprintf(SIL_XML_out, "\tDNE\t"); break;
				}
				fprintf(SIL_XML_out, "\t\t</Detection>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
#endif
	}
}

/*Print z4 Opp Dets*/
void Z4_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	DVSU_RECORD_T record = { 0 };
	Customer_T custId;
	custId = (Customer_T)record.payload[23];

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);

	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
	if (custId == BMW_LOW || custId == SCANIA_MAN)
	{
#ifndef DISABLE_TRACKER
		/*sort by range*/
		//Z7B_Logging_Data_T* p_l_CoreData_Ptr = (Z7B_Logging_Data_T*)Get_RR_Core2_Logging_Data_Ptr();
		//H2A_Operational_Msg_T* p_l_H2A_CoreData_Ptr = (H2A_Operational_Msg_T*)Get_H2A_App_Msg_Ptr();
		float32_T Z4_Opp_Dets[NUMBER_OF_DETECTIONS] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			Z4_Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
		}
		unsigned8_T size = sizeof(Z4_Opp_Dets) / sizeof(*Z4_Opp_Dets);
		unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Z4_Opp_Dets;
		qsort(index, size, sizeof(*index), compare);

		/*Z4 Opp Dets validity levels count*/
		static unsigned8_T Valid = 0;
		static unsigned8_T Invalid = 0;
		static unsigned8_T HVC = 0;
		static unsigned8_T Low_SNR = 0;
		static unsigned8_T Angle_Jump = 0;
		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			if (valid_level == 1)
			{
				Valid++;
			}
			if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
			{
				Invalid++;
			}
			if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
			{
				HVC++;
			}
			if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
			{
				Angle_Jump++;
			}
			if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
			{
				Low_SNR++;
			}
		}
		fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
		fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
		fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
		fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
		Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

		/*print sorted Z4 opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			/*check for valid level of Z4 opp dets*/
			if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude, valid_level </DETECTION_INFO>\n");
				count = 0;
			}
			if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
			{
				fprintf(SIL_XML_out, "\t<Detection>");

				//char strData[20];
				//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED);	
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED);			
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

				//floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED);			
				fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);

				unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
				switch (valid_level)
				{
				case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
				case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
				case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
				case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
				case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
				case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
				case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
				case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
				case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
				default:fprintf(SIL_XML_out, "\tDNE\t"); break;
				}
				fprintf(SIL_XML_out, "\t</Detection>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_START_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
#endif
	}
}

///*Print RR Virtual Fusion Opp Dets*/
void VF_RR_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
	/*sort by range*/
	float32_T VF_RR_Opp_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		VF_RR_Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(VF_RR_Opp_Dets) / sizeof(*VF_RR_Opp_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = VF_RR_Opp_Dets;
	qsort(index, size, sizeof(*index), compare);

	/*RR VF Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted RR VF Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid level of VF RR opp dets*/
		if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d>", Dets_XML.ScanIndex);
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude, valid_level </DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//char strData[20];
			//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			switch (valid_level)
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_END_%d>\n", Dets_XML.ScanIndex);
	}
}

///*Print RL Virtual Fusion Opp Dets*/
void VF_RL_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
	/*sort by range*/
	float32_T VF_RL_Opp_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		VF_RL_Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(VF_RL_Opp_Dets) / sizeof(*VF_RL_Opp_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = VF_RL_Opp_Dets;
	qsort(index, size, sizeof(*index), compare);

	/*RL VF Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted RL VF Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid level of VF RL opp dets*/
		if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d>", Dets_XML.ScanIndex);
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,    amplitude,   valid_level </DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//	char strData[20];
				//floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			switch (valid_level)
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_END_%d>\n", Dets_XML.ScanIndex);
	}
}

///*Print FR Virtual Fusion Opp Dets*/
void VF_FR_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
	/*sort by range*/
	float32_T VF_FR_Opp_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		VF_FR_Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(VF_FR_Opp_Dets) / sizeof(*VF_FR_Opp_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = VF_FR_Opp_Dets;
	qsort(index, size, sizeof(*index), compare);

	/*FR VF Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted FR VF Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid level of VF FR opp dets*/
		if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d>", Dets_XML.ScanIndex);
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude, valid_level </DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//	char strData[20];
			//	floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

			//floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			switch (valid_level)
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_END_%d>\n", Dets_XML.ScanIndex);
	}
}

/*Print FL Virtual Fusion Opp Dets*/
void VF_FL_Opposite_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Dets_Opp_XML,DET_OPP);*/
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_Opp_XML, DET_OPP);
	/*sort by range*/
	float32_T VF_FL_Opp_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		VF_FL_Opp_Dets[i] = Dets_Opp_XML.dets_float[i].range;
	}
	unsigned8_T size = sizeof(VF_FL_Opp_Dets) / sizeof(*VF_FL_Opp_Dets);
	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = VF_FL_Opp_Dets;
	qsort(index, size, sizeof(*index), compare);

	/*FL VF Opp Dets validity levels count*/
	static unsigned8_T Valid = 0;
	static unsigned8_T Invalid = 0;
	static unsigned8_T HVC = 0;
	static unsigned8_T Low_SNR = 0;
	static unsigned8_T Angle_Jump = 0;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
		if (valid_level == 1)
		{
			Valid++;
		}
		if ((valid_level == 2) || (valid_level == 4) || (valid_level == 6) || (valid_level == 8) || (valid_level == 10) || (valid_level == 12) || (valid_level == 14))
		{
			Invalid++;
		}
		if ((valid_level == 3) || (valid_level == 7) || (valid_level == 11) || (valid_level == 15))
		{
			HVC++;
		}
		if ((valid_level == 5) || (valid_level == 7) || (valid_level == 13) || (valid_level == 15))
		{
			Angle_Jump++;
		}
		if ((valid_level == 9) || (valid_level == 11) || (valid_level == 13) || (valid_level == 15))
		{
			Low_SNR++;
		}
	}
	fprintf(SIL_XML_out, "\n\t<NO_OF_VALID_DETS>%d</NO_OF_VALID_DETS>", Valid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_INVALID_DETS>%d</NO_OF_INVALID_DETS>", Invalid);
	fprintf(SIL_XML_out, "\n\t<NO_OF_HOST_VEHICLE_CLUTTER>%d</NO_OF_HOST_VEHICLE_CLUTTER>", HVC);
	fprintf(SIL_XML_out, "\n\t<NO_OF_ANGLE_JUMP>%d</NO_OF_ANGLE_JUMP>", Angle_Jump);
	fprintf(SIL_XML_out, "\n\t<NO_OF_LOW_SNR_DETS>%d</NO_OF_LOW_SNR_DETS>\n", Low_SNR);
	Valid = Invalid = HVC = Low_SNR = Angle_Jump = 0;

	/*print sorted FL VF Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	{
		/*check for valid level of VF FL opp dets*/
		if (((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<DETECTION_OPP_DESCRIPTION_START_%d>", Dets_XML.ScanIndex);
			fprintf(SIL_XML_out, "\n<DETECTION_INFO>\trange,\t\trange_rate_raw, azimuth_raw,   amplitude, valid_level </DETECTION_INFO>\n");
			count = 0;
		}
		if ((Dets_Opp_XML.dets_float[index[i]].valid_level) > 0)
		{
			fprintf(SIL_XML_out, "\t<Detection>");

			//char strData[20];
		//	floatToString(Dets_Opp_XML.dets_float[index[i]].range, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].range_rate_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].range_rate_raw);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].azimuth_raw, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].azimuth_raw);

			//	floatToString(Dets_Opp_XML.dets_float[index[i]].amplitude, strData, FIXED); 
			fprintf(SIL_XML_out, "\t%f", Dets_Opp_XML.dets_float[index[i]].amplitude);
			unsigned8_T valid_level = Dets_Opp_XML.dets_float[index[i]].valid_level;
			switch (valid_level)
			{
			case 1:fprintf(SIL_XML_out, "\tValid\t"); break;
			case 2:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 3:fprintf(SIL_XML_out, "\tHost Vehicle Clutter (HVC)\t"); break;
			case 4:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 5:fprintf(SIL_XML_out, "\tAngle Jump (AJ)\t"); break;
			case 6:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 7:fprintf(SIL_XML_out, "\tHVC, AJ\t"); break;
			case 8:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 9:fprintf(SIL_XML_out, "\tLow SNR (LS)*\t"); break;
			case 10:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 11:fprintf(SIL_XML_out, "\tHVC, LS*\t"); break;
			case 12:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 13:fprintf(SIL_XML_out, "\tAJ, LS*\t"); break;
			case 14:fprintf(SIL_XML_out, "\tInvalid\t"); break;
			case 15:fprintf(SIL_XML_out, "\tHVC, AJ, LS*\t"); break;
			default:fprintf(SIL_XML_out, "\tDNE\t"); break;
			}
			fprintf(SIL_XML_out, "\t</Detection>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</DETECTION_OPP_DESCRIPTION_END_%d>\n", Dets_XML.ScanIndex);
	}
}

///*Print Curvi Tracks Info MAN*/			
void Curvi_Tracks_Info_MAN(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&curvi_XML, Z7B_TRACKER_DATA);

	DVSU_RECORD_T record = { 0 };

#ifndef DISABLE_TRACKER
	/*sort by range*/
	float32_T Track_Objs[NUMBER_OF_OBJECTS_L] = { 0 };
	for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
	{
		Track_Objs[i] = curvi_XML.CurviTracks[i].LonPos;//curvi_XML.CurviTracks[i].vcs_long_posn[i];
	}
	unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
	unsigned8_T index[NUMBER_OF_OBJECTS_L] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		index[i] = i;
	}
	array_data = Track_Objs;
	qsort(index, size, sizeof(*index), compare);

	/*print sorted Opp dets*/
	unsigned8_T count = 1;
	for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
	{
		/*check for valid Track status level*/
		if (((curvi_XML.CurviTracks[i].Status) > 0) && (count == 1))
		{
			fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<TRACK_INFO> DownselectionFlag, ClassificationType, HostCurvatureSlow, vcs_long_vel_rel, vcs_lat_vel_rel,\tstatus, ref_position, f_stationary,\tid, age,\t\tref_lat_accel, ref_lat_vel,   ref_lat_posn, ref_long_accel, ref_long_vel,    ref_long_posn,\t\t\tcurvi_lat_vel_rel, curvi_long_vel_rel, curvi_lat_vel, curvi_long_vel, curvi_lat_posn, curvi_long_posn,\t\twidth,\t\tlength,\t\theading,\texistence_probability </TRACK_INFO>\n");
			count = 0;
		}
		if ((curvi_XML.CurviTracks[i].Status) > 0)
		{
			fprintf(SIL_XML_out, "  <TRACKS>");

			//	char strData[200];
				//floatToString(curvi_XML.CurviTracks[i].vcs_long_posn[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, " \t %f", curvi_XML.CurviTracks[i].DownselectionFlag);

			//floatToString(curvi_XML.CurviTracks[i].vcs_long_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t%f\t", curvi_XML.CurviTracks[i].ClassificationType);

			//floatToString(curvi_XML.CurviTracks[i].vcs_lat_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, " %f\t", curvi_XML.CurviTracks[i].HostCurvatureSlow);

			//	floatToString(curvi_XML.CurviTracks[i].vcs_long_vel_rel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "%f\t", curvi_XML.CurviTracks[i].long_vel_rel);

			//	floatToString(curvi_XML.CurviTracks[i].vcs_lat_vel_rel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "%f", curvi_XML.CurviTracks[i].lat_vel_rel);

			unsigned8_T status = curvi_XML.CurviTracks[i].Status;
			switch (status)
			{
			case 0:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
			case 1:fprintf(SIL_XML_out, "\t\tNEW\t"); break;
			case 2:fprintf(SIL_XML_out, "\t\tMATURE\t"); break;
			case 3:fprintf(SIL_XML_out, "\t\tCOASTED\t"); break;
			case 4:fprintf(SIL_XML_out, "\t\tNUMBER_OF_OBJECT_STATUS\t"); break;
			default:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
			}

			fprintf(SIL_XML_out, "\t%d", curvi_XML.CurviTracks[i].RefPosition);
			fprintf(SIL_XML_out, "\t%d", curvi_XML.CurviTracks[i].Stationary);
			fprintf(SIL_XML_out, "\t\t%d", curvi_XML.CurviTracks[i].id);
			fprintf(SIL_XML_out, "  %d", curvi_XML.CurviTracks[i].age);

			//floatToString(curvi_XML.CurviTracks[i].ref_lat_accel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t\t\%f", curvi_XML.CurviTracks[i].LatAcc);

			//floatToString(curvi_XML.CurviTracks[i].ref_lat_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LatVel);

			//floatToString(curvi_XML.CurviTracks[i].ref_lat_posn[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LatAcc);

			//floatToString(curvi_XML.CurviTracks[i].ref_long_accel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonAcc);

			//floatToString(curvi_XML.CurviTracks[i].ref_long_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonVel);

			//floatToString(curvi_XML.CurviTracks[i].ref_long_posn[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonPos);

			//floatToString(curvi_XML.CurviTracks[i].curvi_lat_vel_rel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t\t%f", curvi_XML.CurviTracks[i].Curvi_LatVel_rel);

			//	floatToString(curvi_XML.CurviTracks[i].curvi_long_vel_rel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].Curvi_LonVel_rel);

			//	floatToString(curvi_XML.CurviTracks[i].curvi_lat_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LatVel);

			//	floatToString(curvi_XML.CurviTracks[i].curvi_long_vel[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LonVel);

			//floatToString(curvi_XML.CurviTracks[i].curvi_lat_posn[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LatPos);
			//floatToString(curvi_XML.CurviTracks[i].curvi_long_posn[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LonPos);

			//floatToString(curvi_XML.CurviTracks[i].width[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].Width);

			//	floatToString(curvi_XML.CurviTracks[i].length[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Length);

			//floatToString(curvi_XML.CurviTracks[i].heading[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Heading);

			//	floatToString(curvi_XML.CurviTracks[i].existence_probability[index[i]], strData, FIXED);
			fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].existence_probability);
			fprintf(SIL_XML_out, "\t</TRACKS>\n");
		}
	}
	if (count == 0)
	{
		fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	}
#endif
}
/*Print Curvi Tracks Info_TML*/
void Curvi_Tracks_Info_TML(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&curvi_tracks_XML, Z7B_TRACKER_DATA);
	DVSU_RECORD_T record = { 0 };
	if (custId == TML_SRR5)
	{
#ifndef DISABLE_TRACKER
		/*sort by range*/
		float32_T Track_Objs[NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
		{
			Track_Objs[i] = curvi_tracks_XML.CurviTracks[i].vcs_long_posn;
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		unsigned8_T valid_track_counter = 0;
		unsigned8_T count_track = NUMBER_OF_OBJECTS - 1;

		if (custId == TML_SRR5)
		{

			for (unsigned8_T index = 0; index < NUMBER_OF_OBJECTS; index++)
			{
				if ((curvi_tracks_XML.CurviTracks[count_track].status) > 0)
				{
					valid_track_counter++;
				}
				count_track--;
			}
		}
		fprintf(SIL_XML_out, "\n<TRACKS_START>\n");
		fprintf(SIL_XML_out, "\t<NO_VALID_TRACKS>\t%d\t<NO_VALID_TRACKS>\n", valid_track_counter);

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i <NUMBER_OF_OBJECTS_L; i++)
		{
			/*check for valid Track status level*/
			if (((curvi_tracks_XML.CurviTracks[index[i]].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>",Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_long_posn, vcs_long_vel, vcs_lat_vel, vcs_long_vel_rel, vcs_lat_vel_rel,\tstatus, object_class, ref_position, f_stationary,\t\t\tid, \t\tage,\t\tref_lat_accel, \t\tref_lat_vel,\tref_lat_posn,\tref_long_accel, \tref_long_vel,\t ref_long_posn,\t\tcurvi_lat_vel_rel, curvi_long_vel_rel, curvi_lat_vel, curvi_long_vel, curvi_lat_posn, curvi_long_posn,\t\twidth,\t\tlength,\t\theading,\texistence_probability,\taccuracy_x,\taccuracy_y,\taccuracy_vx,\taccuracy_vy,\taccuracy_ax,\taccuracy_ay,\taccuracy_width,\taccuracy_length,\taccuracy_heading,\tprobability_2wheel,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_unknown</TRACK_INFO>\n");
				count = 0;
			}
			if ((curvi_tracks_XML.CurviTracks[index[i]].status) > 0)
			{
				fprintf(SIL_XML_out, "  <TRACKS>");
				fprintf(SIL_XML_out, "\t%f", curvi_tracks_XML.CurviTracks[index[i]].vcs_long_posn);
				fprintf(SIL_XML_out, "\t\t %f", curvi_tracks_XML.CurviTracks[index[i]].vcs_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].vcs_lat_vel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].vcs_long_vel_rel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].vcs_lat_vel_rel);

				unsigned8_T status = curvi_tracks_XML.CurviTracks[index[i]].status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				case 1:fprintf(SIL_XML_out, "\t\tNEW\t"); break;
				case 2:fprintf(SIL_XML_out, "\t\tMATURE\t"); break;
				case 3:fprintf(SIL_XML_out, "\t\tCOASTED\t"); break;
				case 4:fprintf(SIL_XML_out, "\t\tNUMBER_OF_OBJECT_STATUS\t"); break;
				default:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				}
				unsigned8_T object_class = curvi_tracks_XML.CurviTracks[index[i]].object_class;

				switch (object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNKNOWN\t"); break;
				case 1:fprintf(SIL_XML_out, "\tPEDESTRIAN\t"); break;
				case 2:fprintf(SIL_XML_out, "\t2WHEEL\t"); break;
				case 3:fprintf(SIL_XML_out, "\tCAR\t"); break;
				case 4:fprintf(SIL_XML_out, "\tTRUCK\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				}
				fprintf(SIL_XML_out, "\t\t%d", curvi_tracks_XML.CurviTracks[index[i]].ref_position);
				fprintf(SIL_XML_out, "\t\t\t\t%d", curvi_tracks_XML.CurviTracks[index[i]].f_stationary);
				fprintf(SIL_XML_out, "\t\t\t\t%d", curvi_tracks_XML.CurviTracks[index[i]].id);
				fprintf(SIL_XML_out, "\t\t\t\t%d", curvi_tracks_XML.CurviTracks[index[i]].age);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_lat_accel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_lat_vel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_lat_posn);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_long_accel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].ref_long_posn);
				fprintf(SIL_XML_out, "\t\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_lat_vel_rel);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_long_vel_rel);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_lat_vel);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_long_vel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_lat_posn);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].curvi_long_posn);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].width);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].length);
				fprintf(SIL_XML_out, "\t%f", curvi_tracks_XML.CurviTracks[index[i]].heading);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].existence_probability);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_x);
				fprintf(SIL_XML_out, "\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_y);
				fprintf(SIL_XML_out, "\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_vx);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_vy);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_ax);
				fprintf(SIL_XML_out, "\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_ay);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_width);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_length);
				fprintf(SIL_XML_out, "\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].accuracy_heading);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].object_class_probability.probability_2wheel);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].object_class_probability.probability_car);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].object_class_probability.probability_pedestrian);
				fprintf(SIL_XML_out, "\t\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].object_class_probability.probability_truck);
				fprintf(SIL_XML_out, "\t\t\t%f", curvi_tracks_XML.CurviTracks[index[i]].object_class_probability.probability_unknown);

				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
		fprintf(SIL_XML_out, "<TRACKS_END>\n");
#endif
	}
}

///*Print Curvi Tracks Info*/			
void Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&curvi_XML, Z7B_TRACKER_DATA);

	DVSU_RECORD_T record = { 0 };


	if (custId == BMW_LOW || custId == BMW_BPIL || custId == SCANIA_MAN)
	{
#ifndef DISABLE_TRACKER
		float32_T Track_Objs[NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
		{
			Track_Objs[i] = curvi_XML.CurviTracks[i].LonPos;//curvi_XML.CurviTracks[i].vcs_long_posn[i];
		}
		unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
		unsigned8_T index[NUMBER_OF_OBJECTS_L] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Track_Objs;
		qsort(index, size, sizeof(*index), compare);

		/*print sorted Opp dets*/
		unsigned8_T count = 1;
		for (unsigned8_T i = 0; i < NUMBER_OF_OBJECTS_L; i++)
		{
			/*check for valid Track status level*/
			if (((curvi_XML.CurviTracks[i].Status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
				fprintf(SIL_XML_out, "\n<TRACK_INFO> DownselectionFlag, ClassificationType, HostCurvatureSlow, vcs_long_vel_rel, vcs_lat_vel_rel,\tstatus, ref_position, f_stationary,\tid, age,\t\tref_lat_accel, ref_lat_vel,   ref_lat_posn, ref_long_accel, ref_long_vel,    ref_long_posn,\t\t\tcurvi_lat_vel_rel, curvi_long_vel_rel, curvi_lat_vel, curvi_long_vel, curvi_lat_posn, curvi_long_posn,\t\twidth,\t\tlength,\t\theading,\texistence_probability </TRACK_INFO>\n");
				count = 0;
			}
			if ((curvi_XML.CurviTracks[i].Status) > 0)
			{
				fprintf(SIL_XML_out, "  <TRACKS>");
				fprintf(SIL_XML_out, " \t %f", curvi_XML.CurviTracks[i].DownselectionFlag);
				fprintf(SIL_XML_out, "\t\t%f\t", curvi_XML.CurviTracks[i].ClassificationType);
				fprintf(SIL_XML_out, " %f\t", curvi_XML.CurviTracks[i].HostCurvatureSlow);
				fprintf(SIL_XML_out, "%f\t", curvi_XML.CurviTracks[i].long_vel_rel);
				fprintf(SIL_XML_out, "%f", curvi_XML.CurviTracks[i].lat_vel_rel);

				unsigned8_T status = curvi_XML.CurviTracks[i].Status;
				switch (status)
				{
				case 0:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				case 1:fprintf(SIL_XML_out, "\t\tNEW\t"); break;
				case 2:fprintf(SIL_XML_out, "\t\tMATURE\t"); break;
				case 3:fprintf(SIL_XML_out, "\t\tCOASTED\t"); break;
				case 4:fprintf(SIL_XML_out, "\t\tNUMBER_OF_OBJECT_STATUS\t"); break;
				default:fprintf(SIL_XML_out, "\t\tINVALID\t"); break;
				}

				fprintf(SIL_XML_out, "\t%d", curvi_XML.CurviTracks[i].RefPosition);
				fprintf(SIL_XML_out, "\t%d", curvi_XML.CurviTracks[i].Stationary);
				fprintf(SIL_XML_out, "\t\t%d", curvi_XML.CurviTracks[i].id);
				fprintf(SIL_XML_out, "  %d", curvi_XML.CurviTracks[i].age);


				fprintf(SIL_XML_out, "\t\t\t\%f", curvi_XML.CurviTracks[i].LatAcc);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LatVel);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LatPos);


				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonAcc);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonVel);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].LonPos);


				fprintf(SIL_XML_out, "\t\t\t%f", curvi_XML.CurviTracks[i].Curvi_LatVel_rel);
				fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].Curvi_LonVel_rel);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LatVel);


				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LonVel);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LatPos);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Curvi_LonPos);


				fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].Width);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Length);
				fprintf(SIL_XML_out, "\t%f", curvi_XML.CurviTracks[i].Heading);
				fprintf(SIL_XML_out, "\t\t%f", curvi_XML.CurviTracks[i].existence_probability);
				fprintf(SIL_XML_out, "\t</TRACKS>\n");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
#endif
	}
}

///*Print Alignment Info*/			
void Alignment_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	// g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&Radar_Info_XML,RADAR_PARAM);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Radar_Info_XML, RADAR_PARAM);

	fprintf(SIL_XML_out, "\n<ALIGNMENT_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<ALIGNMENT_DESCRIPTION_INFO>\t<ALIGNMENT_DESCRIPTION>\n\t<ALIGNMENT_SELF_INFO>");

	//char strData[200];
	//floatToString(Radar_Info_XML.own_align_angle, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\talign_angle\t\t%f", Radar_Info_XML.own_align_angle);

	//floatToString(Radar_Info_XML.own_align_angle_saved, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tangle_saved\t\t%f", Radar_Info_XML.own_align_angle_saved);

	//floatToString(Radar_Info_XML.own_align_angle_ref, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tangle_ref\t\t%f", Radar_Info_XML.own_align_angle_ref);

	//floatToString(Radar_Info_XML.own_align_angle_raw, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tangle_raw\t\t%f", Radar_Info_XML.own_align_angle_raw);

	fprintf(SIL_XML_out, "\n\t\tquality_factor");
	unsigned8_T own_align_quality_factor = Radar_Info_XML.own_align_quality_factor;
	switch (own_align_quality_factor)
	{
	case 0:fprintf(SIL_XML_out, "\t\tUNKNOWN\t"); break;
	case 1:fprintf(SIL_XML_out, "\t\tACCURATE\t"); break;
	case 2:fprintf(SIL_XML_out, "\t\tFAULTY\t"); break;
	case 3:fprintf(SIL_XML_out, "\t\tINACCURATE\t"); break;
	default://do nothing
		break;
	}
	if (custId == BMW_LOW || custId == BMW_SAT || custId == SCANIA_MAN)
	{
		fprintf(SIL_XML_out, "\n\t</ALIGNMENT_SELF_INFO>\n\t<ALIGNMENT_OPP_INFO>");

		//floatToString(Radar_Info_XML.opp_align_angle, strData, FIXED);
		fprintf(SIL_XML_out, "\n\t\talign_angle\t\t%f", Radar_Info_XML.opp_align_angle);

		fprintf(SIL_XML_out, "\n\t\tquality_factor");
		unsigned8_T opp_align_quality_factor = Radar_Info_XML.opp_align_quality_factor;
		switch (opp_align_quality_factor)
		{
		case 0:fprintf(SIL_XML_out, "\t\tUNKNOWN\t"); break;
		case 1:fprintf(SIL_XML_out, "\t\tACCURATE\t"); break;
		case 2:fprintf(SIL_XML_out, "\t\tFAULTY\t"); break;
		case 3:fprintf(SIL_XML_out, "\t\tINACCURATE\t"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\t<ALIGNMENT_OPP_INFO>\n</ALIGNMENT_DESCRIPTION_INFO>\t</ALIGNMENT_DESCRIPTION>\n");
	}
	fprintf(SIL_XML_out, "</ALIGNMENT_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}


void ADC_Physical_xml_Data(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&ADC_Data_XML,ADC_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ADC_Data_XML, ADC_DATA);
	fprintf(SIL_XML_out, "\n<ADC_PHYSICAL_DATA_INFO_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<ADC_PHYSICAL_DATA_INFO_DESCRIPTION>\t<ADC_PHYSICAL_DATA_PARAMETER>");

	//	char strData[20];
		//floatToString(ADC_Data_XML.ADC_Physical_Data.VTUNE_Volts_PLL_Freq_Hi, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tVTUNE_Volts_PLL_Freq_Hi\t\t%f", ADC_Data_XML.ADC_Physical_Data.VTUNE_Volts_PLL_Freq_Hi);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.VTUNE_Volts_PLL_Freq_Lo, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tVTUNE_Volts_PLL_Freq_Lo\t\t%f", ADC_Data_XML.ADC_Physical_Data.VTUNE_Volts_PLL_Freq_Lo);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.TSENSE1_Calib_Temp, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tTSENSE1_Calib_Temp\t\t%f", ADC_Data_XML.ADC_Physical_Data.TSENSE1_Calib_Temp);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.TSENSE0_Calib_Temp, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tTSENSE0_Calib_Temp\t\t%f", ADC_Data_XML.ADC_Physical_Data.TSENSE0_Calib_Temp);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.RXN_MUX_Temp_Sense_Degrees, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tRXN_MUX_Temp_Sense_Degrees\t%f", ADC_Data_XML.ADC_Physical_Data.RXN_MUX_Temp_Sense_Degrees);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.BattF_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tBattF_Volts\t\t\t%f", ADC_Data_XML.ADC_Physical_Data.BattF_Volts);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.ECUOnBdTemp, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tECUOnBdTemp\t\t\t%f", ADC_Data_XML.ADC_Physical_Data.ECUOnBdTemp);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.Supply_1_2_5V_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tSupply_1_2_5V_Volts\t\t%f", ADC_Data_XML.ADC_Physical_Data.Supply_1_2_5V_Volts);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.Supply_1_8V_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tSupply_1_8V_Volts\t\t%f", ADC_Data_XML.ADC_Physical_Data.Supply_1_8V_Volts);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.Supply_5V_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tSupply_5V_Volts\t\t\t%f", ADC_Data_XML.ADC_Physical_Data.Supply_5V_Volts);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.Supply_3_3V_RAW_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tSupply_3_3V_RAW_Volts\t\t%f", ADC_Data_XML.ADC_Physical_Data.Supply_3_3V_RAW_Volts);

	//floatToString(ADC_Data_XML.ADC_Physical_Data.Supply_3_3V_DAC_Volts, strData, FIXED);
	fprintf(SIL_XML_out, "\n\t\tSupply_3_3V_DAC_Volts\t\t%f", ADC_Data_XML.ADC_Physical_Data.Supply_3_3V_DAC_Volts);

	fprintf(SIL_XML_out, "\n</ADC_PHYSICAL_DATA_INFO_DESCRIPTION>\t</ADC_PHYSICAL_DATA_PARAMETER>\n");
	fprintf(SIL_XML_out, "</ADC_PHYSICAL_DATA_INFO_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}

///*Print Algo Timing Info of z7b core*/			
void Algo_Timing_Info_of_Z7B_core(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Time_info_xml,TIME_INFO_DATA);*/
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Time_info_xml, TIME_INFO_DATA);
	unsigned8_T count = NO_OF_EVENTS_Z7B - 1;
	fprintf(SIL_XML_out, "\n<ALGO_TIMING_Z7B_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<ALGO_TIMING_DESCRIPTION_INFO> ALGO_TIMING_INFO FOR Z7B core, 30 EVENTS </ALGO_TIMING_DESCRIPTION_INFO>\n");
	for (unsigned8_T i = 0; i < NO_OF_EVENTS_Z7B; i++)
	{
		fprintf(SIL_XML_out, "\n\t\t\t\t\t%d", Time_info_xml.Timing_Info[count]);
		count--;
	}
	fprintf(SIL_XML_out, "\n</ALGO_TIMING_Z7B_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}
void Free_space_info_Man(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Free_space_man_xml, FREE_SPACE_DATA);

	fprintf(SIL_XML_out, "\n<FREESPACE_ALGO_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	int k = 0;
	for (int j = 0; j < NO_OF_BLOCKS; j++)
	{
		if (j == 0)
			fprintf(SIL_XML_out, "\n<FS_OBJECT_INTER_INFO> <\orientation_angle_0>,  <polar_params_beam_range_0>,  <pos_centre_vect_lat_0>,<pos_centre_vect_lon_0>, \t\t\t<distance_out_0>\t\t\t, \t<Clearance_Measurement_0>,");
		else
			fprintf(SIL_XML_out, "\n<FS_OBJECT_INTER_INFO> <\orientation_angle_1>,  <polar_params_beam_range_1>,  <pos_centre_vect_lat_1>,<pos_centre_vect_lon_1>, \t\t\t<distance_out_1>\t\t\t, \t<Clearance_Measurement_1>,");

		for (int index = 0; index < NO_OF_SECTORS; index++)
		{
			fprintf(SIL_XML_out, "\n<FS_OBJECT_INTERN_INFO>");
			fprintf(SIL_XML_out, "\t\t%f\t\t", Free_space_man_xml[j].CL_FS_params.coordinate_system.orientation_angle);
			fprintf(SIL_XML_out, " \t\t%f\t\t\t", Free_space_man_xml[j].CL_FS_params.polar_params_beam_range);
			fprintf(SIL_XML_out, "\t\t%f\t\t\t", Free_space_man_xml[j].CL_FS_params.coordinate_system.origin.vect_lat);
			fprintf(SIL_XML_out, " \t\t%f\t\t\t", Free_space_man_xml[j].CL_FS_params.coordinate_system.origin.vect_lon);
			fprintf(SIL_XML_out, " \t\t%f\t\t\t", Free_space_man_xml[j].CL_JLR_Sector_data[index].distance_out);
			fprintf(SIL_XML_out, " \t\t%d\t\t\t", Free_space_man_xml[j].CL_JLR_Sector_data[index].Free_Space_Status_Clearance_Measurement);
		}
	}
	fprintf(SIL_XML_out, "\n<END_FS_OBJECT_INTERN_INFO_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

	fprintf(SIL_XML_out, "\n</FREESPACE_ALGO_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n");
}


void Free_space_info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	/*g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	g_pIRadarStrm->getData((unsigned char*)&Free_space_xml,FREE_SPACE_DATA);*/
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Free_space_xml, FREE_SPACE_DATA);
	//Z7B_Cust_Logging_Data_T * z7b_customer_data = (Z7B_Cust_Logging_Data_T*)Get_Cust_Core2_Logging_Data_Ptr();
	float32_T FF_Object_props[FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging] = { 0 };
	float32_T FF_Intern_object[FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging] = { 0 };
	for (unsigned8_T index = 0; index < FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging; index++)
	{
		FF_Object_props[index] = Free_space_xml.objects[index].obj_props.ref_lat_posn;
	}
	for (unsigned8_T index = 0; index < FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging; index++)
	{
		FF_Intern_object[index] = Free_space_xml.objects[index].obj_intern.pos_centre_fcs.vect_lat;
	}
	unsigned8_T size_props = sizeof(FF_Object_props) / sizeof(*FF_Object_props);
	unsigned8_T index_props[FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging] = { 0 };
	unsigned8_T size_intern = sizeof(FF_Intern_object) / sizeof(*FF_Intern_object);
	unsigned8_T index_intern[FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging] = { 0 };
	for (unsigned8_T index = 0; index < size_props; index++)
	{
		index_props[index] = index;
	}
	array_data = FF_Object_props;
	qsort(index_props, size_props, sizeof(*index_props), compare);
	for (unsigned8_T indexcounter = 0; indexcounter < size_intern; indexcounter++)
	{
		index_intern[indexcounter] = indexcounter;
	}
	array_data = FF_Intern_object;
	//std ::sort(index_intern, index_intern+size_intern, greater<int>());
	qsort(index_intern, size_intern, sizeof(*index_intern), compare);
	fprintf(SIL_XML_out, "\n<FREESPACE_ALGO_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<FS_OBJECT_INTER_INFO> <\theading_fcs>,  <pos_centre_fcs_long>,  <pos_centre_fcs_lat>, <b_obj_reached_max_size>, <sector_index_max>, <sector_index_min>,\t<N_det_assoc_current_cycle>");

	for (int index = 0; index < FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging; index++)
	{
		fprintf(SIL_XML_out, "\n<FS_OBJECT_INTERN_INFO>");
		fprintf(SIL_XML_out, "\t\t%f\t", Free_space_xml.objects[index_intern[index]].obj_intern.heading_fcs);
		//fprintf(SIL_XML_out,"\t\pos_centre_fcs\t\t%d",z7b_customer_data->FREESPACE_FF.objects[i].obj_intern.pos_centre_fcs);  
		fprintf(SIL_XML_out, " \t%f\t", Free_space_xml.objects[index_intern[index]].obj_intern.pos_centre_fcs.vect_lon);
		fprintf(SIL_XML_out, " \t%f\t", Free_space_xml.objects[index_intern[index]].obj_intern.pos_centre_fcs.vect_lat);
		fprintf(SIL_XML_out, " \t%d\t", Free_space_xml.objects[index_intern[index]].obj_intern.b_obj_reached_max_size);
		fprintf(SIL_XML_out, " \t\t%d\t", Free_space_xml.objects[index_intern[index]].obj_intern.sector_index_max);
		fprintf(SIL_XML_out, " \t%d\t", Free_space_xml.objects[index_intern[index]].obj_intern.sector_index_min);
		fprintf(SIL_XML_out, " \t\t%d\t", Free_space_xml.objects[index_intern[index]].obj_intern.N_det_assoc_current_cycle);
	}
	fprintf(SIL_XML_out, "\n<END_FS_OBJECT_INTERN_INFO_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n\n<START_FS_OBJECT_PROPS_INFO_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<FS_OBJECT_PROPS_INFO>\t\t<state>    \t<ref_lat_posn>,\t    <ref_long_posn>, \t<ref_position>, <existence_probability>,\t<id>,\t <priority_number>,\t <priority_value>,\t <width>, \t<length>, \t<heading>, \t<vector_2d_vect_lon>, <vector_2d_vect_lat>,   <stage_age>,   <age>");
	for (int i = 0; i < FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging; i++)
	{
		fprintf(SIL_XML_out, "\n<FS_OBJECT_PROPS_INFO>");
		fprintf(SIL_XML_out, " \t\t%d\t", Free_space_xml.objects[index_props[i]].obj_props.state);
		fprintf(SIL_XML_out, "\t%f\t", Free_space_xml.objects[index_props[i]].obj_props.ref_lat_posn);
		fprintf(SIL_XML_out, " \t%f\t", Free_space_xml.objects[index_props[i]].obj_props.ref_long_posn);
		fprintf(SIL_XML_out, " \t%d\t", Free_space_xml.objects[index_props[i]].obj_props.ref_position);
		fprintf(SIL_XML_out, " \t%f\t", Free_space_xml.objects[index_props[i]].obj_props.existence_probability);

		fprintf(SIL_XML_out, "\t%d\t", Free_space_xml.objects[index_props[i]].obj_props.id);
		fprintf(SIL_XML_out, "\t%f\t", Free_space_xml.objects[index_props[i]].obj_props.priority_number);
		fprintf(SIL_XML_out, "\t%f\t", Free_space_xml.objects[index_props[i]].obj_props.priority_value);
		fprintf(SIL_XML_out, " %f\t", Free_space_xml.objects[index_props[i]].obj_props.width);
		fprintf(SIL_XML_out, " %f\t", Free_space_xml.objects[index_props[i]].obj_props.length);
		fprintf(SIL_XML_out, "%f\t", Free_space_xml.objects[index_props[i]].obj_props.heading);
		fprintf(SIL_XML_out, " %f\t", Free_space_xml.objects[index_props[i]].obj_props.vcs_pos.vect_lon);
		fprintf(SIL_XML_out, " \t%f\t", Free_space_xml.objects[index_props[i]].obj_props.vcs_pos.vect_lat);
		fprintf(SIL_XML_out, " \t%d\t", Free_space_xml.objects[index_props[i]].obj_props.stage_age);
		fprintf(SIL_XML_out, "\t%d\t", Free_space_xml.objects[index_props[i]].obj_props.age);
	}
	fprintf(SIL_XML_out, "\n<END_FS_OBJECT_PROPS_INFO_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n</FREESPACE_ALGO_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n");
}

///*Print Algo Timing Info of z7a core*/			
void Algo_Timing_Info_of_Z7A_core(unsigned8_T Radar_Posn, IRadarStream  *  g_pIRadarStrm)
{
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&Z7a_time_info_xml,Z7A_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Z7a_time_info_xml, Z7A_DATA);
	unsigned8_T count = NO_OF_EVENTS_Z7A - 1;
	if (Z7a_time_info_xml.ScanIndex != 0)
	{
		fprintf(SIL_XML_out, "\n<ALGO_TIMING_Z7B_DESCRIPTION_START_%d_%s>", Z7a_time_info_xml.ScanIndex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n<ALGO_TIMING_DESCRIPTION_INFO> ALGO_TIMING_INFO FOR Z7A_Z7B core, 10 events </ALGO_TIMING_DESCRIPTION_INFO>\n");
		for (unsigned8_T i = 0; i < NO_OF_EVENTS_Z7A; i++)
		{
			fprintf(SIL_XML_out, "\n\t\t\t\t\t%d", Z7a_time_info_xml.Timing_Info[count]);
			count--;
		}
		fprintf(SIL_XML_out, "\n</ALGO_TIMING_Z7A_Z7B_DESCRIPTION_END_%d_%s>\n", Z7a_time_info_xml.ScanIndex, GetRadarPosName(Radar_Posn));
	}
}
//
///*Print Algo Timing Info of z4 core*/			
void Algo_Timing_Info_of_Z4_core(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm)
{
	/* g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	   g_pIRadarStrm->getData((unsigned char*)&Z4_time_info_xml,TIME_INFO_Z4_DATA);*/

	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Z4_time_info_xml, TIME_INFO_Z4_DATA);

	unsigned8_T count = NO_OF_EVENTS_Z4 - 1;
	fprintf(SIL_XML_out, "\n<ALGO_TIMING_Z4_Z7B_DESCRIPTION_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
	fprintf(SIL_XML_out, "\n<ALGO_TIMING_DESCRIPTION_INFO> ALGO_TIMING_INFO FOR Z4_Z7B core, 10 EVENTS </ALGO_TIMING_DESCRIPTION_INFO>\n");
	for (unsigned8_T i = 0; i < NO_OF_EVENTS_Z4; i++)
	{
		fprintf(SIL_XML_out, "\n\t\t\t\t\t%d", Z4_time_info_xml.Timing_Info[count]);
		count--;
	}
	fprintf(SIL_XML_out, "\n</ALGO_TIMING_Z4_Z7B_DESCRIPTION_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}

void ASW_Output_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&ASW_XML,AUDI_ASW);
	//g_pIRadarStrm->getData((unsigned char*)&CED_DATA_XML,CED_FF_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ASW_XML, AUDI_ASW);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&CED_DATA_XML, CED_FF_DATA);
	//if(custId ==  BMW_BPIL)
	//{
	//	if(CED_DATA_XML.CED_FF.CED_INTPUT.f_CED_Status == 1)
	//	{
	//		fprintf(SIL_XML_out,"\n<ASW_EVENT_START_%d>",Dets_XML.ScanIndex);
	//		fprintf(SIL_XML_out,"\n<ASW_DESCRIPTION_INFO>\t\t\t<ASW_EVENT_DETAILS>");
	//		fprintf(SIL_XML_out,"\n\t\tASW_status\t\t%d",CED_DATA_XML.CED_FF.CED_INTPUT.f_CED_Status);  
	//		fprintf(SIL_XML_out,"\n\t\tASW_alert_left\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_alert_left);  
	//		fprintf(SIL_XML_out,"\n\t\tASW_alert_right\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_alert_right);  
	//		fprintf(SIL_XML_out,"\n\t\tASW_id_left\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_id_left);  
	//		fprintf(SIL_XML_out,"\n\t\tASW_id_right\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_id_right);  
	//		for(int i=0 ; i< MAX_NUMBER_BYTES_OBJECT_COUNTER ; i++)
	//		{
	//			fprintf(SIL_XML_out,"\n\t\tASW_Obj_Counter[%d]\t%d",i,CED_DATA_XML.CED_FF.CED_OUTPUT.interesting_objects.Object_Counter[i]);  
	//		}
	//		fprintf(SIL_XML_out,"\n\t\tASW_ttc_left\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_ttc_left);  
	//		fprintf(SIL_XML_out,"\n\t\tASW_ttc_right\t\t%d",CED_DATA_XML.CED_FF.CED_OUTPUT.CED_ttc_right);  
	//		fprintf(SIL_XML_out,"\n</ASW_DESCRIPTION_INFO>\t</ASW_EVENT_DETAILS>\n");
	//		fprintf(SIL_XML_out,"</ASW_EVENT_END_%d>\n",Dets_XML.ScanIndex);	
	//	}
	//}
	if (custId == BMW_LOW)
	{
		if (CED_DATA_XML.CED_FF.CED_INTPUT.f_CED_Status == 1)
		{
			fprintf(SIL_XML_out, "\n<ASW_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<ASW_DESCRIPTION_INFO>\t\t<ASW_EVENT_DETAILS>");
			fprintf(SIL_XML_out, "\n\t\tASW_status\t\t%d", CED_DATA_XML.CED_FF.CED_INTPUT.f_CED_Status);
			fprintf(SIL_XML_out, "\n\t\tASW_alert_left\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_alert_left);
			fprintf(SIL_XML_out, "\n\t\tASW_alert_right\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_alert_right);
			fprintf(SIL_XML_out, "\n\t\tASW_id_left\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_id_left);
			fprintf(SIL_XML_out, "\n\t\tASW_id_right\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_id_right);
			for (int i = 0; i < MAX_NUMBER_BYTES_OBJECT_COUNTER; i++)
			{
				fprintf(SIL_XML_out, "\n\t\tASW_Obj_Counter[%d]\t%d", i, CED_DATA_XML.CED_FF.CED_OUTPUT.interesting_objects.Object_Counter[i]);
			}
			fprintf(SIL_XML_out, "\n\t\tASW_ttc_left\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_ttc_left);
			fprintf(SIL_XML_out, "\n\t\tASW_ttc_right\t\t%d", CED_DATA_XML.CED_FF.CED_OUTPUT.CED_ttc_right);
			fprintf(SIL_XML_out, "\n</ASW_DESCRIPTION_INFO>\t\t</ASW_EVENT_DETAILS>\n");
			fprintf(SIL_XML_out, "</ASW_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}
}

/*Print LCDA Output Info*/
void LCDA_Output_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	if (custId == TML_SRR5)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_FF_ping_xml_TML_Gen5, LCDA_Output);
		{
			fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\tLCDA_Output_Info\t\t<LCDA_PARAMERTERS> ");

			fprintf(SIL_XML_out, "\n<LCDA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n\t<LCDA_OUTPUT>");
			fprintf(SIL_XML_out, "\n\tbsw_alert_left\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.bsw_alert_left);
			fprintf(SIL_XML_out, "\n\tbsw_alert_right\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.bsw_alert_right);
			fprintf(SIL_XML_out, "\n\tbsw_id_left\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.bsw_id_left);
			fprintf(SIL_XML_out, "\n\tbsw_id_right\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.bsw_id_right);
			fprintf(SIL_XML_out, "\n\tcvw_alert_left\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_alert_left);
			fprintf(SIL_XML_out, "\n\tcvw_alert_right\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_alert_right);
			fprintf(SIL_XML_out, "\n\tcvw_id_left\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_id_left);
			fprintf(SIL_XML_out, "\n\tcvw_id_right\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_id_right);
			fprintf(SIL_XML_out, "\n\tcvw_ttc_left\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_ttc_left);
			fprintf(SIL_XML_out, "\n\tcvw_ttc_right\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.cvw_ttc_right);
			fprintf(SIL_XML_out, "\n\tf_bsw_enabled\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.f_bsw_enabled);
			fprintf(SIL_XML_out, "\n\tf_cvw_enabled\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.f_cvw_enabled);
			fprintf(SIL_XML_out, "\n\tf_lcda_enabled\t%d", p_g_FF_ping_xml_TML_Gen5.LCDA_Output.f_lcda_enabled);

			fprintf(SIL_XML_out, "\n<LCDA_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

		}
	}

	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&LCDA_XML_Data,LCDA_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&LCDA_LOW_DATA,LCDA_XML_DATA);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	//m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&LCDA_LOW_DATA, LCDA_XML_DATA);
#ifndef DISABLE_FF
	if (custId == BMW_LOW)
	{
		//if ((/*(pl_fences_ptr_XML->LCDA_FF.LCDA_OUTPUT.f_lcda_enabled)||*/(LCDA_LOW_DATA.LCDA_FF.LCDA_INPUT.f_lcda_enable_bsw) || (LCDA_LOW_DATA.LCDA_FF.LCDA_INPUT.f_lcda_enable_cvw) || (LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_alert[0]) || (LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_alert[1]) || (LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_alert[0]) || (LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_alert[1])) == 1)
		//{
		//	fprintf(SIL_XML_out, "\n<LCDA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		//	fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\t<LCDA_EVENT_DETAILS>");

		//	/*fprintf(SIL_XML_out,"\n\t\tenabled");
		//	if(pl_fences_ptr_XML->LCDA_Output.f_lcda_enabled == 1)
		//	{  fprintf(SIL_XML_out,"\t\t\tENABLED");  }
		//	else { fprintf(SIL_XML_out,"\t\t\tDISABLED"); }*/

		//	fprintf(SIL_XML_out, "\n\t\tbsw_enabled");
		//	if (LCDA_LOW_DATA.LCDA_FF.LCDA_INPUT.f_lcda_enable_bsw == 1)
		//	{
		//		fprintf(SIL_XML_out, "\t\tENABLED");
		//	}
		//	else { fprintf(SIL_XML_out, "\t\tDISABLED"); }

		//	fprintf(SIL_XML_out, "\n\t\tcvw_enabled");
		//	if (LCDA_LOW_DATA.LCDA_FF.LCDA_INPUT.f_lcda_enable_cvw == 1)
		//	{
		//		fprintf(SIL_XML_out, "\t\tENABLED");
		//	}
		//	else { fprintf(SIL_XML_out, "\t\tDISABLED"); }

		//	fprintf(SIL_XML_out, "\n\t\tbsw_alert_left");
		//	unsigned8_T bsw_alert_left = LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_alert[0];
		//	switch (bsw_alert_left)//enum of bsw_alert_left
		//	{
		//	case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		//	case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		//	case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		//	default://do nothing
		//		break;
		//	}

		//	fprintf(SIL_XML_out, "\n\t\tbsw_alert_right");
		//	unsigned8_T bsw_alert_right = LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_alert[1];
		//	switch (bsw_alert_right)//enum of bsw_alert_right
		//	{
		//	case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		//	case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		//	case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		//	default://do nothing
		//		break;
		//	}

		//	fprintf(SIL_XML_out, "\n\t\tcvw_alert_left");
		//	unsigned8_T cvw_alert_left = LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_alert[0];
		//	switch (cvw_alert_left)//enum of cvw_alert_left
		//	{
		//	case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		//	case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		//	case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		//	default://do nothing
		//		break;
		//	}

		//	fprintf(SIL_XML_out, "\n\t\tcvw_alert_right");
		//	unsigned8_T cvw_alert_right = LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_alert[1];
		//	switch (cvw_alert_right)//enum of cvw_alert_right
		//	{
		//	case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		//	case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		//	case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		//	default://do nothing
		//		break;
		//	}

		//	fprintf(SIL_XML_out, "\n\t\tbsw_id_left\t\t%d", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_id[0]);
		//	fprintf(SIL_XML_out, "\n\t\tbsw_id_right\t\t%d", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.bsw_id[1]);
		//	fprintf(SIL_XML_out, "\n\t\tcvw_id_left\t\t%d", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_id[0]);
		//	fprintf(SIL_XML_out, "\n\t\tcvw_id_right\t\t%d", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_id[1]);

		//	char strData[20];
		//	//floatToString(LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_ttc[0], strData, FIXED);
		//	fprintf(SIL_XML_out, "\n\t\tttc_left\t\t%f", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_ttc[0]);

		//	//floatToString(LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_ttc[1], strData, FIXED);
		//	fprintf(SIL_XML_out, "\n\t\tttc_right\t\t%f", LCDA_LOW_DATA.LCDA_FF.LCDA_OUTPUT.cvw_ttc[1]);

		//	fprintf(SIL_XML_out, "\n</LCDA_DESCRIPTION_INFO>\t</LCDA_EVENT_DETAILS>\n");
		//	fprintf(SIL_XML_out, "</LCDA_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		//}
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_LCDA_DATA, LCDA_OUTPUT);
		if (((BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_bsw_enabled) || (BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_cvw_enabled) || (BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_lcda_enabled) || (BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_slc_enabled) || (BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_awa_enabled)) == 1)
		{
			fprintf(SIL_XML_out, "\n<LCDA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\tLCDA_Output_Info\t<LCDA_EVENT_DETAILS>");

			fprintf(SIL_XML_out, "\n\t\tlcda_object_vy_right\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_vy_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_vx_right\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_vx_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_ttc_right\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_ttc_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_py_right\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_py_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_px_right\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_px_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_vy_left\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_vy_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_vx_left\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_vx_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_ttc_left\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_ttc_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_py_left\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_py_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_px_left\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_px_left);
			fprintf(SIL_XML_out, "\n\t\tlane_center_offset\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lane_center_offset);
			fprintf(SIL_XML_out, "\n\t\tlane_width\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lane_width);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_existance_probability_right\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_existance_probability_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_existance_probability_left\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_existance_probability_left);
			fprintf(SIL_XML_out, "\n\t\tK_Unused8_1\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.K_Unused8_1);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_lane_change_probability_right\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_lane_change_probability_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_id_right\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_id_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_type_right\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_type_right);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_lane_change_probability_left\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_lane_change_probability_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_id_left\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_id_left);
			fprintf(SIL_XML_out, "\n\t\tlcda_object_type_left\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.lcda_object_type_left);
			fprintf(SIL_XML_out, "\n\t\tf_awa_enabled\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_awa_enabled);
			fprintf(SIL_XML_out, "\n\t\tf_slc_enabled\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_slc_enabled);
			fprintf(SIL_XML_out, "\n\t\tf_cvw_enabled\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_cvw_enabled);
			fprintf(SIL_XML_out, "\n\t\tf_bsw_enabled\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_bsw_enabled);
			fprintf(SIL_XML_out, "\n\t\tf_lcda_enabled\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.f_lcda_enabled);

			for (int i = 0; i < NUMBER_OF_SIDES; i++)
			{
				fprintf(SIL_XML_out, "\n\t\tawa_dec\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.awa_dec[i]);
				fprintf(SIL_XML_out, "\n\t\tawa_ttc\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.awa_ttc[i]);
				fprintf(SIL_XML_out, "\n\t\tslc_lane_change_probability\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.slc_lane_change_probability[i]);
				fprintf(SIL_XML_out, "\n\t\tslc_ttc\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.slc_ttc[i]);
				fprintf(SIL_XML_out, "\n\t\tcvw_ttc\t\t%f", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.cvw_ttc[i]);
				fprintf(SIL_XML_out, "\n\t\tawa_id\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.awa_id[i]);
				fprintf(SIL_XML_out, "\n\t\tawa_alert\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.awa_alert[i]);
				fprintf(SIL_XML_out, "\n\t\tslc_id\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.slc_id[i]);
				fprintf(SIL_XML_out, "\n\t\tslc_alert\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.slc_alert[i]);
				fprintf(SIL_XML_out, "\n\t\tcvw_id\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.cvw_id[i]);
				fprintf(SIL_XML_out, "\n\t\tcvw_alert\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.cvw_alert[i]);
				fprintf(SIL_XML_out, "\n\t\tbsw_id\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.bsw_id[i]);
				fprintf(SIL_XML_out, "\n\t\tbsw_alert\t\t%d", BMW_LOW_LCDA_DATA.LCDA_OUTPUT.bsw_alert[i]);
			}
			fprintf(SIL_XML_out, "\n</LCDA_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}
	if (custId == BMW_BPIL)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&LCDA_XML_Data, LCDA_DATA);
		if (((LCDA_XML_Data.f_lcda_enabled) || (LCDA_XML_Data.f_bsw_enabled) || (LCDA_XML_Data.f_cvw_enabled) || (LCDA_XML_Data.bsw_alert_left) || (LCDA_XML_Data.bsw_alert_right) || (LCDA_XML_Data.cvw_alert_left) || (LCDA_XML_Data.cvw_alert_right)) == 1)
		{
			fprintf(SIL_XML_out, "\n<LCDA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\t<LCDA_EVENT_DETAILS>");

			fprintf(SIL_XML_out, "\n\t\tenabled");
			if (LCDA_XML_Data.f_lcda_enabled == 1)
			{
				fprintf(SIL_XML_out, "\t\t\tENABLED");
			}
			else { fprintf(SIL_XML_out, "\t\t\tDISABLED"); }

			fprintf(SIL_XML_out, "\n\t\tbsw_enabled");
			if (LCDA_XML_Data.f_bsw_enabled == 1)
			{
				fprintf(SIL_XML_out, "\t\tENABLED");
			}
			else { fprintf(SIL_XML_out, "\t\tDISABLED"); }

			fprintf(SIL_XML_out, "\n\t\tcvw_enabled");
			if (LCDA_XML_Data.f_cvw_enabled == 1)
			{
				fprintf(SIL_XML_out, "\t\tENABLED");
			}
			else { fprintf(SIL_XML_out, "\t\tDISABLED"); }

			fprintf(SIL_XML_out, "\n\t\tbsw_alert_left");
			unsigned8_T bsw_alert_left = LCDA_XML_Data.bsw_alert_left;
			switch (bsw_alert_left)//enum of bsw_alert_left
			{
			case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
			case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
			case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
			default://do nothing
				break;
			}

			fprintf(SIL_XML_out, "\n\t\tbsw_alert_right");
			unsigned8_T bsw_alert_right = LCDA_XML_Data.bsw_alert_right;
			switch (bsw_alert_right)//enum of bsw_alert_right
			{
			case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
			case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
			case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
			default://do nothing
				break;
			}

			fprintf(SIL_XML_out, "\n\t\tcvw_alert_left");
			unsigned8_T cvw_alert_left = LCDA_XML_Data.cvw_alert_left;
			switch (cvw_alert_left)//enum of cvw_alert_left
			{
			case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
			case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
			case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
			default://do nothing
				break;
			}

			fprintf(SIL_XML_out, "\n\t\tcvw_alert_right");
			unsigned8_T cvw_alert_right = LCDA_XML_Data.cvw_alert_right;
			switch (cvw_alert_right)//enum of cvw_alert_right
			{
			case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
			case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
			case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
			default://do nothing
				break;
			}

			fprintf(SIL_XML_out, "\n\t\tbsw_id_left\t\t%d", LCDA_XML_Data.bsw_id_left);
			fprintf(SIL_XML_out, "\n\t\tbsw_id_right\t\t%d", LCDA_XML_Data.bsw_id_right);
			fprintf(SIL_XML_out, "\n\t\tcvw_id_left\t\t%d", LCDA_XML_Data.cvw_id_left);
			fprintf(SIL_XML_out, "\n\t\tcvw_id_right\t\t%d", LCDA_XML_Data.cvw_id_right);

			char strData[20];
			//floatToString(pl_fences_ptr_XML->LCDA_Output.cvw_ttc_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tttc_left\t\t%f", LCDA_XML_Data.cvw_ttc_left);

			//floatToString(pl_fences_ptr_XML->LCDA_Output.cvw_ttc_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tttc_right\t\t%f", LCDA_XML_Data.cvw_ttc_right);

			fprintf(SIL_XML_out, "\n</LCDA_DESCRIPTION_INFO>\t</LCDA_EVENT_DETAILS>\n");
			fprintf(SIL_XML_out, "</LCDA_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}

#endif
}


///*Print CTA Output Info*/			
void CTA_Output_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{
	
	if (custId == TML_SRR5)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_FF_ping_xml_TML_Gen5, RCTA_Output);

		{
			fprintf(SIL_XML_out, "\n<RCTA_DESCRIPTION_INFO>\t\tCTA_Output_Info\t\t<LCDA_PARAMERTERS> ");

			fprintf(SIL_XML_out, "\n<RCTA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n\t<RCTA_OUTPUT>");
			fprintf(SIL_XML_out, "\n\tbsw_alert_left\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.cta_id_left);
			fprintf(SIL_XML_out, "\n\tcta_id_right\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.cta_id_right);
			fprintf(SIL_XML_out, "\n\tcta_ttc_left\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.cta_ttc_left);
			fprintf(SIL_XML_out, "\n\tcta_ttc_right\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.cta_ttc_right);
			fprintf(SIL_XML_out, "\n\tf_cta_alert_left\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.f_cta_alert_left);
			fprintf(SIL_XML_out, "\n\tf_cta_alert_right\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.f_cta_alert_right);
			fprintf(SIL_XML_out, "\n\tf_cta_enabled\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.f_cta_enabled);
			fprintf(SIL_XML_out, "\n\tf_cta_warn_left\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.f_cta_warn_left);
			fprintf(SIL_XML_out, "\n\tf_cta_warn_right\t%d", p_g_FF_ping_xml_TML_Gen5.RCTA_Output.f_cta_warn_right);

			fprintf(SIL_XML_out, "\n<RCTA_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&CTA_XML_Data,CTA_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&CTA_LOW_DATA,CTA_XML_DATA);

#ifndef DISABLE_FF
	if (custId == BMW_BPIL)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&CTA_XML_Data, CTA_DATA);
		if (((CTA_XML_Data.f_cta_enabled) || (CTA_XML_Data.f_cta_alert_left) || (CTA_XML_Data.f_cta_alert_right)) == 1)
		{
			fprintf(SIL_XML_out, "\n<CTA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<CTA_DESCRIPTION_INFO>\t\t\t<CTA_EVENT_DETAILS>");

			fprintf(SIL_XML_out, "\n\t\tenabled");
			if (CTA_XML_Data.f_cta_enabled == 1)
			{
				fprintf(SIL_XML_out, "\t\t\t\tENABLED");
			}
			else {
				fprintf(SIL_XML_out, "\t\t\t\tDISABLED");
			}

			fprintf(SIL_XML_out, "\n\t\talert_left\t\t\t%d", CTA_XML_Data.f_cta_alert_left);
			fprintf(SIL_XML_out, "\n\t\talert_right\t\t\t%d", CTA_XML_Data.f_cta_alert_right);
			fprintf(SIL_XML_out, "\n\t\twarn_left\t\t\t%d", CTA_XML_Data.f_cta_warn_left);
			fprintf(SIL_XML_out, "\n\t\twarn_left\t\t\t%d", CTA_XML_Data.f_cta_warn_right);
			fprintf(SIL_XML_out, "\n\t\tid_left\t\t\t\t%d", CTA_XML_Data.cta_id_left);
			fprintf(SIL_XML_out, "\n\t\tid_left\t\t\t\t%d", CTA_XML_Data.cta_id_right);
			//char strData[20];
			//floatToString(CTA_XML_Data.cta_ttc_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\ttc_left\t\t\t\t%f", CTA_XML_Data.cta_ttc_left);
			fprintf(SIL_XML_out, "\n\t\ttc_left\t\t\t\t%f", CTA_XML_Data.cta_ttc_right);
			//floatToString(CTA_XML_Data.cta_intersectionX_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tintersectX_left\t\t\t%f", CTA_XML_Data.cta_intersectionX_left);
			//floatToString(CTA_XML_Data.cta_intersectionY_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tintersectY_left\t\t\t%f", CTA_XML_Data.cta_intersectionY_left);
			//floatToString(CTA_XML_Data.cta_radialDistance_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tradial_Distance_left\t\t%f", CTA_XML_Data.cta_radialDistance_left);
			//floatToString(CTA_XML_Data.cta_objPoseX_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tobjPoseX_left\t\t\t%f", CTA_XML_Data.cta_objPoseX_left);
			//floatToString(CTA_XML_Data.cta_objPoseY_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tobjPoseY_left\t\t\t%f", CTA_XML_Data.cta_objPoseY_left);
			//floatToString(CTA_XML_Data.cta_objVelocityX_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tVelocityX_left\t\t\t%f", CTA_XML_Data.cta_objVelocityX_left);
			//floatToString(CTA_XML_Data.cta_objVelocityY_left, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\twarn_right\t\t\t%d", CTA_XML_Data.f_cta_warn_right);
			fprintf(SIL_XML_out, "\n\t\tid_right\t\t\t%d", CTA_XML_Data.cta_id_right);
			//floatToString(CTA_XML_Data.cta_ttc_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tttc_right\t\t\t%f", CTA_XML_Data.cta_ttc_right);
			//(CTA_XML_Data.cta_intersectionX_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tintersectX_right\t\t%f", CTA_XML_Data.cta_intersectionX_right);
			//floatToString(CTA_XML_Data.cta_intersectionY_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tintersectY_right\t\t%f", CTA_XML_Data.cta_intersectionY_right);
			//(CTA_XML_Data.cta_radialDistance_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tradial_Distance_right\t\t%f", CTA_XML_Data.cta_radialDistance_right);
			//floatToString(CTA_XML_Data.cta_objPoseX_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tPoseX_right\t\t\t%f", CTA_XML_Data.cta_objPoseX_right);
			//floatToString(CTA_XML_Data.cta_objPoseY_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tPoseY_right\t\t\t%f", CTA_XML_Data.cta_objPoseY_right);
			//(CTA_XML_Data.cta_objVelocityX_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tVelocityX_right\t\t\t%f", CTA_XML_Data.cta_objVelocityX_right);
			//floatToString(CTA_XML_Data.cta_objVelocityY_right, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tVelocityY_right\t\t\t%f", CTA_XML_Data.cta_objVelocityY_right);
			fprintf(SIL_XML_out, "\n\t\tKritikalitaetslevel_rechts\t%d", CTA_XML_Data.RCTA_Kritikalitaetslevel_rechts);
			fprintf(SIL_XML_out, "\n\t\tRCTA_Kritikalitaetslevel_links\t%d", CTA_XML_Data.RCTA_Kritikalitaetslevel_links);
			fprintf(SIL_XML_out, "\n</CTA_DESCRIPTION_INFO>\t\t\t</CTA_EVENT_DETAILS>\n");
			fprintf(SIL_XML_out, "</CTA_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}

	if (custId == BMW_LOW)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_CTA_DATA, CTA_OUTPUT);

		if (((BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_enabled) || (BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_warn_left) || (BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_warn_right)) == 1)
		{	
			fprintf(SIL_XML_out, "\n<CTA_DESCRIPTION_INFO>\tCTA info\t<CTA_EVENT_DETAILS>");
			fprintf(SIL_XML_out, "\n<CTA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

			fprintf(SIL_XML_out, "\n\t\tcrash_prob_left\t\t\t%f", BMW_LOW_CTA_DATA.CTA_OUTPUT.crash_prob_left);
			fprintf(SIL_XML_out, "\n\t\tcrash_prob_right\t\t\t%f", BMW_LOW_CTA_DATA.CTA_OUTPUT.crash_prob_right);
			fprintf(SIL_XML_out, "\n\t\tcta_id_left\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.cta_id_left);
			fprintf(SIL_XML_out, "\n\t\tcta_id_right\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.cta_id_right);
			fprintf(SIL_XML_out, "\n\t\tcta_radarposition\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.cta_radarposition);
			fprintf(SIL_XML_out, "\n\t\tcta_ttc_left\t\t\t%f", BMW_LOW_CTA_DATA.CTA_OUTPUT.cta_ttc_left);
			fprintf(SIL_XML_out, "\n\t\tcta_ttc_right\t\t\t%f", BMW_LOW_CTA_DATA.CTA_OUTPUT.cta_ttc_right);
			fprintf(SIL_XML_out, "\n\t\tf_cta_alert_left\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_alert_left);
			fprintf(SIL_XML_out, "\n\t\tf_cta_alert_right\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_alert_right);
			fprintf(SIL_XML_out, "\n\t\tf_cta_enabled\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_enabled);
			fprintf(SIL_XML_out, "\n\t\tf_cta_warn_left\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_warn_left);
			fprintf(SIL_XML_out, "\n\t\tf_cta_warn_right\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.f_cta_warn_right);
			fprintf(SIL_XML_out, "\n\t\tK_Unused8_1\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.K_Unused8_1);
			fprintf(SIL_XML_out, "\n\t\tRCTB_brake_request\t\t\t%f", BMW_LOW_CTA_DATA.CTA_OUTPUT.RCTB_brake_request);
			fprintf(SIL_XML_out, "\n\t\tRCTB_qualifier\t\t\t%d", BMW_LOW_CTA_DATA.CTA_OUTPUT.RCTB_qualifier);

			fprintf(SIL_XML_out, "\n</CTA_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}

	}

#endif

}

void CED_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId) // Feature Function info for Safe Exit [SFE/CED]
{
	if (custId == BMW_LOW)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_CED_DATA, CED_OUT);

		{
			fprintf(SIL_XML_out, "\n<SFE_CED_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<SFE_CED_DESCRIPTION_INFO>\t\tSFE_CED_Output_Info\t\t<SFE_CED_PARAMERTERS>");

			fprintf(SIL_XML_out, "\n\tK_Unused8_1\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.K_Unused8_1);
			fprintf(SIL_XML_out, "\n\tSFE_CED_alert_left\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_alert_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_alert_right\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_alert_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_dir_left\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_dir_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_dir_right\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_dir_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_front_lat_left\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_front_lat_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_front_lat_right\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_front_lat_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_front_path_match_left\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_front_path_match_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_front_path_match_right\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_front_path_match_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_id_left\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_id_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_id_right\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_id_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_rear_lat_left\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_rear_lat_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_rear_lat_right\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_rear_lat_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_rear_path_match_left\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_rear_path_match_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_rear_path_match_right\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_rear_path_match_right);
			fprintf(SIL_XML_out, "\n\tSFE_CED_Status\t\t\t%d", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_Status);
			fprintf(SIL_XML_out, "\n\tSFE_CED_ttc_left\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_ttc_left);
			fprintf(SIL_XML_out, "\n\tSFE_CED_ttc_right\t\t\t%f", BMW_LOW_CED_DATA.CED_OUTPUT.SFE_CED_ttc_right);
			
			fprintf(SIL_XML_out, "\n<SFE_CED_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}
}

void TA_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId)// Feature Function info for TA
{
	if (custId == BMW_LOW)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_TA, TA_OUTPUT);
		{
			fprintf(SIL_XML_out, "\n<TA_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<TA_DESCRIPTION_INFO>\t\tTA_Output_Info\t\t<TA_PARAMERTERS>");

			fprintf(SIL_XML_out, "\n\tK_Unused8_1\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.K_Unused8_1);
			fprintf(SIL_XML_out, "\n\trta_dynamic_area_status\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_dynamic_area_status);
			fprintf(SIL_XML_out, "\n\trta_turning_area_status\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_turning_area_status);
			fprintf(SIL_XML_out, "\n\tf_diagnostic_mode\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.f_diagnostic_mode);
			fprintf(SIL_XML_out, "\n\tf_rta_enable\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.f_rta_enable);
			fprintf(SIL_XML_out, "\n\tf_fta_enable\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.f_fta_enable);
			fprintf(SIL_XML_out, "\n\tfta_maneuver_direction\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_maneuver_direction);
			fprintf(SIL_XML_out, "\n\trta_id_right\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_id_right);
			fprintf(SIL_XML_out, "\n\trta_id_left\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_id_left);
			fprintf(SIL_XML_out, "\n\trta_alert_right\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_alert_right);
			fprintf(SIL_XML_out, "\n\trta_alert_left\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.rta_alert_left);
			fprintf(SIL_XML_out, "\n\tfta_target_id\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_target_id);
			fprintf(SIL_XML_out, "\n\tfta_brake_conditioning\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_brake_conditioning);
			fprintf(SIL_XML_out, "\n\tfta_brake_threshold_reduction\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_brake_threshold_reduction);
			fprintf(SIL_XML_out, "\n\tfta_symbol_request\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_symbol_request);
			fprintf(SIL_XML_out, "\n\tfta_alert_level\t\t\t%d", BMW_LOW_TA.TA_OUTPUT.fta_alert_level);
			fprintf(SIL_XML_out, "\n\tta_current_deceleration_estimate\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.ta_current_deceleration_estimate);
			fprintf(SIL_XML_out, "\n\tta_ttp_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.ta_ttp_right);
			fprintf(SIL_XML_out, "\n\tta_ttp_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.ta_ttp_left);
			fprintf(SIL_XML_out, "\n\trta_existence_probability_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_existence_probability_right);
			fprintf(SIL_XML_out, "\n\trta_existence_probability_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_existence_probability_left);
			fprintf(SIL_XML_out, "\n\trta_lat_vel_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_lat_vel_right);
			fprintf(SIL_XML_out, "\n\trta_long_vel_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_long_vel_right);
			fprintf(SIL_XML_out, "\n\trta_lat_vel_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_lat_vel_left);
			fprintf(SIL_XML_out, "\n\trta_long_vel_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_long_vel_left);
			fprintf(SIL_XML_out, "\n\trta_ttc_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_ttc_right);
			fprintf(SIL_XML_out, "\n\trta_ttc_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_ttc_left);
			fprintf(SIL_XML_out, "\n\trta_lat_posn_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_lat_posn_right);
			fprintf(SIL_XML_out, "\n\trta_long_posn_right\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_long_posn_right);
			fprintf(SIL_XML_out, "\n\trta_lat_posn_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_lat_posn_left);
			fprintf(SIL_XML_out, "\n\trta_long_posn_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_long_posn_left);
			fprintf(SIL_XML_out, "\n\tfta_ttc\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.fta_ttc);
			fprintf(SIL_XML_out, "\n\tfta_target_gap\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.fta_target_gap);
			fprintf(SIL_XML_out, "\n\tfta_brake_deceleration_request\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.fta_brake_deceleration_request);
			fprintf(SIL_XML_out, "\n\trta_alert_left\t\t\t%f", BMW_LOW_TA.TA_OUTPUT.rta_alert_left);

			fprintf(SIL_XML_out, "\n<TA_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}

	}
	

}
void Freespace_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId)// Feature Function info for  Freespace
{
	if (custId == BMW_LOW)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_FS, FREE_SPACE_DATA);
		{
			fprintf(SIL_XML_out, "\n<Freespace_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<Freespace_DESCRIPTION_INFO>\t\tFreespace_Output_Info\t\t<Freespace_PARAMERTERS>");

			for (int i = 0; i < FS_STAT_OBJECT_MAX_NUMBER_OF_OBJECTS_Logging; i++)
			{
				fprintf(SIL_XML_out, "\n\tb_obj_reached_max_size\t\t\t%d", BMW_LOW_FS.objects[i].obj_intern.b_obj_reached_max_size);
				fprintf(SIL_XML_out, "\n\theading_fcs\t\t\t%f", BMW_LOW_FS.objects[i].obj_intern.heading_fcs);
				fprintf(SIL_XML_out, "\n\tN_det_assoc_current_cycle\t\t\t%d", BMW_LOW_FS.objects[i].obj_intern.N_det_assoc_current_cycle);
				fprintf(SIL_XML_out, "\n\tvect_lat\t\t\t%f", BMW_LOW_FS.objects[i].obj_intern.pos_centre_fcs.vect_lat);
				fprintf(SIL_XML_out, "\n\tvect_lon\t\t\t%f", BMW_LOW_FS.objects[i].obj_intern.pos_centre_fcs.vect_lon);
				fprintf(SIL_XML_out, "\n\tsector_index_max\t\t\t%d", BMW_LOW_FS.objects[i].obj_intern.sector_index_max);
				fprintf(SIL_XML_out, "\n\tsector_index_min\t\t\t%d", BMW_LOW_FS.objects[i].obj_intern.sector_index_min);
				fprintf(SIL_XML_out, "\n\tK_Unused8_1\t\t\t%d", BMW_LOW_FS.objects[i].obj_intern.K_Unused8_1);

				fprintf(SIL_XML_out, "\n\texistence_probability\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.existence_probability);
				fprintf(SIL_XML_out, "\n\theading\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.heading);
				fprintf(SIL_XML_out, "\n\tid\t\t%d", BMW_LOW_FS.objects[i].obj_props.id);
				fprintf(SIL_XML_out, "\n\tlength\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.length);
				fprintf(SIL_XML_out, "\n\tpriority_number\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.priority_number);
				fprintf(SIL_XML_out, "\n\tpriority_value\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.priority_value);
				fprintf(SIL_XML_out, "\n\tref_lat_posn\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.ref_lat_posn);
				fprintf(SIL_XML_out, "\n\tref_position\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.ref_position);
				fprintf(SIL_XML_out, "\n\tref_long_posn\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.ref_long_posn);
				fprintf(SIL_XML_out, "\n\tstage_age\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.stage_age);
				fprintf(SIL_XML_out, "\n\tstate\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.state);
				fprintf(SIL_XML_out, "\n\tvect_lat\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.vcs_pos.vect_lat);
				fprintf(SIL_XML_out, "\n\tvect_lon\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.vcs_pos.vect_lon);
				fprintf(SIL_XML_out, "\n\twidth\t\t\t%f", BMW_LOW_FS.objects[i].obj_props.width);
				fprintf(SIL_XML_out, "\n\tK_Unused16_2\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.K_Unused16_2);
				fprintf(SIL_XML_out, "\n\tK_Unused8_2\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.K_Unused8_2);
				fprintf(SIL_XML_out, "\n\tK_Unused16_1\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.K_Unused16_1);
				fprintf(SIL_XML_out, "\n\tage\t\t\t%d", BMW_LOW_FS.objects[i].obj_props.age);
			}
			
			fprintf(SIL_XML_out, "\n<Freespace_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}

	}
}

///*Print RECW Output Info*/			
void RECW_Output_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId)
{

	if (custId == TML_SRR5)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_FF_ping_xml_TML_Gen5, RCW_Output);
		{
			//fprintf(SIL_XML_out, "\n<RECW_EVENT_START_%d_%s>", Gen5_target_info.ScanIndex, GetRadarPosName(Radar_Posn));

			fprintf(SIL_XML_out, "\n<RECW_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

			fprintf(SIL_XML_out, "\n<RECW_DESCRIPTION_INFO>\t\tRECW_Output_Info\t\t<RECW_PARAMERTERS>");

			fprintf(SIL_XML_out, "\n\td_rcw_obj_class_plausibility\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_class_plausibility);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_crash_probability\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_crash_probability);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_guete\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_guete);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_heading\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_heading);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_length\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_length);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_pos_x_rel\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_pos_x_rel);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_pos_y_rel\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_pos_y_rel);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_radial_dist\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_radial_dist);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_vel_abs_rel\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_vel_abs_rel);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_vel_x_rel\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_vel_x_rel);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_vel_y_rel\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_vel_y_rel);
			fprintf(SIL_XML_out, "\n\td_rcw_obj_width\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_obj_width);
			fprintf(SIL_XML_out, "\n\td_rcw_ttc\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.d_rcw_ttc);
			fprintf(SIL_XML_out, "\n\ti_rcw_acute_alert_signal\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_acute_alert_signal);
			fprintf(SIL_XML_out, "\n\ti_rcw_alert_signal\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_alert_signal);
			fprintf(SIL_XML_out, "\n\ti_rcw_driver_information_signal\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_driver_information_signal);
			fprintf(SIL_XML_out, "\n\ti_rcw_enabled\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_enabled);
			fprintf(SIL_XML_out, "\n\ti_rcw_id\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_id);
			fprintf(SIL_XML_out, "\n\ti_rcw_obj_class\t\t\t%d", p_g_FF_ping_xml_TML_Gen5.RCW_Output.i_rcw_obj_class);

			fprintf(SIL_XML_out, "\n<RECW_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}

	if (custId == BMW_LOW)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&BMW_LOW_RECW_DATA, RECW_OUTPUT);
		{
			//fprintf(SIL_XML_out, "\n<RECW_EVENT_START_%d_%s>", Gen5_target_info.ScanIndex, GetRadarPosName(Radar_Posn));

			fprintf(SIL_XML_out, "\n<RECW_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));

			fprintf(SIL_XML_out, "\n<RECW_DESCRIPTION_INFO>\t\tRECW_Output_Info\t\t<RECW_PARAMERTERS>");

			fprintf(SIL_XML_out, "\n\ti_ttc_warning_threshold\t\t\t%f", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_ttc_warning_threshold);
			fprintf(SIL_XML_out, "\n\td_recw_obj_crash_probability\t\t\t%f", BMW_LOW_RECW_DATA.RECW_OUTPUT.d_recw_obj_crash_probability);
			fprintf(SIL_XML_out, "\n\td_recw_obj_vel_abs_rel\t\t\t%f", BMW_LOW_RECW_DATA.RECW_OUTPUT.d_recw_obj_vel_abs_rel);
			fprintf(SIL_XML_out, "\n\td_recw_ttc\t\t\t%f", BMW_LOW_RECW_DATA.RECW_OUTPUT.d_recw_ttc);
			fprintf(SIL_XML_out, "\n\td_recb_nominal_acceleration_applied\t\t\t%f", BMW_LOW_RECW_DATA.RECW_OUTPUT.d_recb_nominal_acceleration_applied);
			fprintf(SIL_XML_out, "\n\ti_recw_id\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recw_id);
			fprintf(SIL_XML_out, "\n\ti_recb_integrity\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recb_integrity);
			fprintf(SIL_XML_out, "\n\ti_recb_SSM\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recb_SSM);
			fprintf(SIL_XML_out, "\n\ti_recb_qualifier_nominal_acceleration\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recb_qualifier_nominal_acceleration);
			fprintf(SIL_XML_out, "\n\ti_recb_breaking_signal\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recb_breaking_signal);
			fprintf(SIL_XML_out, "\n\ti_recw_acute_alert_signal\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recw_acute_alert_signal);
			fprintf(SIL_XML_out, "\n\ti_recw_alert_signal\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recw_alert_signal);
			fprintf(SIL_XML_out, "\n\ti_recw_enabled\t\t\t%d", BMW_LOW_RECW_DATA.RECW_OUTPUT.i_recw_enabled);
			
			fprintf(SIL_XML_out, "\n<RECW_EVENT_END_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}

	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&RECW_XML_DATA,RECW_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&RECW_XML_DATA, RECW_DATA);
	if (custId == BMW_BPIL)
	{
		if (((RECW_XML_DATA.i_recw_enabled) || (RECW_XML_DATA.i_recw_alert_signal)) == 1)
		{
			fprintf(SIL_XML_out, "\n<RECW_EVENT_START_%d_%s>", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
			fprintf(SIL_XML_out, "\n<RECW_DESCRIPTION_INFO>\t<RECW_EVENT_DETAILS>");

			fprintf(SIL_XML_out, "\n\t\tenabled");
			if (RECW_XML_DATA.i_recw_enabled == 1)
			{
				fprintf(SIL_XML_out, "\t\ENABLED");
			}
			else {
				fprintf(SIL_XML_out, "\tDISABLED");
			}

			fprintf(SIL_XML_out, "\n\t\talert_signal\t%d", RECW_XML_DATA.i_recw_alert_signal);
			fprintf(SIL_XML_out, "\n\t\tacute_alert_signal\t%d", RECW_XML_DATA.i_recw_acute_alert_signal);
			fprintf(SIL_XML_out, "\n\t\tid\t%d", RECW_XML_DATA.i_recw_id);

			//	char strData[20];
			//floatToString(RECW_XML_DATA.d_recw_ttc, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tttc\t%f", RECW_XML_DATA.d_recw_ttc);
			//floatToString(RECW_XML_DATA.d_recw_obj_vel_x_rel, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvel_x_rel\t%f", RECW_XML_DATA.d_recw_obj_vel_x_rel);
			//	floatToString(RECW_XML_DATA.d_recw_obj_vel_y_rel, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tvel_y_rel\t%f", RECW_XML_DATA.d_recw_obj_vel_y_rel);
			//floatToString(RECW_XML_DATA.d_recw_obj_pos_x_rel, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tpos_x_rel\t%f", RECW_XML_DATA.d_recw_obj_pos_x_rel);
			//	floatToString(RECW_XML_DATA.d_recw_obj_pos_y_rel, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tpos_y_rel\t%f", RECW_XML_DATA.d_recw_obj_pos_y_rel);
			//floatToString(RECW_XML_DATA.d_recw_obj_guete, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tguete\t%f", RECW_XML_DATA.d_recw_obj_guete);
			//floatToString(RECW_XML_DATA.d_recw_obj_crash_prob, strData, FIXED);
			fprintf(SIL_XML_out, "\n\t\tcrash_prob\t%f", RECW_XML_DATA.d_recw_obj_crash_prob);
			fprintf(SIL_XML_out, "\n\t\tclass\t%d", RECW_XML_DATA.i_recw_obj_class);
			//floatToString(pl_fences_ptr_XML->RECW_Output.d_recw_obj_class_plausibility, strData, FIXED);gcvt(x, 6, buf);
			char buf[100];
			//	gcvt(RECW_XML_DATA.d_recw_obj_class_plausibility, 6, buf);

			fprintf(SIL_XML_out, "\n\t\tclass_plausibility\t%f", RECW_XML_DATA.d_recw_obj_class_plausibility);
			fprintf(SIL_XML_out, "\n</RECW_DESCRIPTION_INFO>\t</RECW_EVENT_DETAILS>\n");
			fprintf(SIL_XML_out, "</RECW_EVENT_END_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
		}
	}
}
void Input_Header_ECU_Info(unsigned8_T Radar_Posn, const char* LogFname, Customer_T custId, IRadarStream * g_pIRadarStrm)
{
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	int platform = g_pIRadarStrm->m_proc_info.frame_header.Platform;
	if (!flag_ecu_out[Radar_Posn])
	{
		fprintf(SIL_XML_out, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
		fprintf(SIL_XML_out, "<!-- SRR5 APTIV SIL component configuration -->\n");
		fprintf(SIL_XML_out, "<INPUT_HEADER>");
		char *newline = strchr(pathname_XML, '\n');
		if (newline)
			*newline = 0;

		fprintf(SIL_XML_out, "\n<FILE_NAME>%s</FILE_NAME>\n", InputLogName);
		time_t current_time;
		struct tm* timeinfo;
		time(&current_time);
		timeinfo = localtime(&current_time);
		fprintf(SIL_XML_out, "\t<PC_TIME>%d:%d:%d</PC_TIME>\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		fprintf(SIL_XML_out, "\t<CUSTOMER_NAME>\t %s \t</CUSTOMER_NAME>\n", GetCustName(custId, platform));
		fprintf(SIL_XML_out, "</INPUT_HEADER>\n");
		flag_ecu_out[Radar_Posn] = TRUE;
	}
}


void FeatureFunction_Info(IRadarStream * g_pIRadarStrm, Customer_T custId, unsigned8_T Radar_Posn) //Feacture Function info for NISSAN and HONDA
{	
	if (custId == HONDA_GEN5)
	{	
		/******************LCDA DATA ***************************************************************/
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_FF_Xml, LCDA_Output);

		fprintf(SIL_XML_out, "\n\n<LCDA_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\t\t\t<LCDA_PARAMERTERS> ");

		fprintf(SIL_XML_out, "\n\t<LCDA_OUTPUT>");
		//fprintf(SIL_XML_out, "\n\tbsw_alert_left\t%d", p_Honda_LCDA_FF_Xml.Lcda_Honda.bsw_alert_left);
		//fprintf(SIL_XML_out, "\n\tbsw_alert_right\t%d", p_Honda_LCDA_FF_Xml.Lcda_Honda.bsw_alert_right);
		fprintf(SIL_XML_out, "\n\tbsw_alert_left");
		unsigned8_T bsw_alert_left = p_Honda_FF_Xml.bsw_alert_left;
		switch (bsw_alert_left)//enum of bsw_alert_left
		{
		case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\tbsw_alert_right");
		unsigned8_T bsw_alert_right = p_Honda_FF_Xml.bsw_alert_right;
		switch (bsw_alert_right)//enum of bsw_alert_right
		{
		case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\tbsw_id_left\t%d", p_Honda_FF_Xml.bsw_id_left);
		fprintf(SIL_XML_out, "\n\tbsw_id_right\t%d", p_Honda_FF_Xml.bsw_id_right);
		for (int i = 0; i < NUMBER_OF_LCDA_OBJECTS; i++)
		{
			fprintf(SIL_XML_out, "\n\tlka_alert_condition_%d\t%d",i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_alert_condition);
			fprintf(SIL_XML_out, "\n\tlka_change_status_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_change_status);
			fprintf(SIL_XML_out, "\n\tlka_curvi_pos_lat_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_curvi_pos_lat);
			fprintf(SIL_XML_out, "\n\tlka_curvi_pos_long_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_curvi_pos_long);
			fprintf(SIL_XML_out, "\n\tlka_curvi_vel_lat_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_curvi_vel_lat);
			fprintf(SIL_XML_out, "\n\tlka_curvi_vel_long_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_curvi_vel_long);
			fprintf(SIL_XML_out, "\n\tlka_motion_class_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_motion_class);
			fprintf(SIL_XML_out, "\n\tlka_object_class_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_object_class);
			fprintf(SIL_XML_out, "\n\tlka_obj_id_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_obj_id);
			fprintf(SIL_XML_out, "\n\tlka_ttc_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Left[i].lka_ttc);

			fprintf(SIL_XML_out, "\n\tlka_alert_condition_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_alert_condition);
			fprintf(SIL_XML_out, "\n\tlka_change_status_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_change_status);
			fprintf(SIL_XML_out, "\n\tlka_curvi_pos_lat_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_curvi_pos_lat);
			fprintf(SIL_XML_out, "\n\tlka_curvi_pos_long_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_curvi_pos_long);
			fprintf(SIL_XML_out, "\n\tlka_curvi_vel_lat_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_curvi_vel_lat);
			fprintf(SIL_XML_out, "\n\tlka_curvi_vel_long_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_curvi_vel_long);
			fprintf(SIL_XML_out, "\n\tlka_motion_class_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_motion_class);
			fprintf(SIL_XML_out, "\n\tlka_object_class_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_object_class);
			fprintf(SIL_XML_out, "\n\tlka_obj_id_%d\t%d", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_obj_id);
			fprintf(SIL_XML_out, "\n\tlka_ttc_%d\t%f", i, p_Honda_FF_Xml.customer_output.LKA_Object_Right[i].lka_ttc);
		}
		//fprintf(SIL_XML_out, "\n\tcvw_alert_left\t%d", p_Honda_LCDA_FF_Xml.Lcda_Honda.cvw_alert_left);
		//fprintf(SIL_XML_out, "\n\tcvw_alert_right\t%d", p_Honda_LCDA_FF_Xml.Lcda_Honda.cvw_alert_right);

		fprintf(SIL_XML_out, "\n\tcvw_alert_left");
		unsigned8_T cvw_alert_left = p_Honda_FF_Xml.cvw_alert_left;
		switch (cvw_alert_left)//enum of cvw_alert_left
		{
		case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\tcvw_alert_right");
		unsigned8_T cvw_alert_right = p_Honda_FF_Xml.cvw_alert_right;
		switch (cvw_alert_right)//enum of cvw_alert_right
		{
		case 0:fprintf(SIL_XML_out, "\t\tNONE"); break;
		case 1:fprintf(SIL_XML_out, "\t\tLEVEL_1"); break;
		case 2:fprintf(SIL_XML_out, "\t\tLEVEL_2"); break;
		default://do nothing
			break;
		}

		fprintf(SIL_XML_out, "\n\tcvw_id_left\t%d", p_Honda_FF_Xml.cvw_id_left);
		fprintf(SIL_XML_out, "\n\tcvw_id_right\t%d", p_Honda_FF_Xml.cvw_id_right);
		fprintf(SIL_XML_out, "\n\tcvw_ttc_left\t%f", p_Honda_FF_Xml.cvw_ttc_left);
		fprintf(SIL_XML_out, "\n\tcvw_ttc_right\t%f", p_Honda_FF_Xml.cvw_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_bsw_enabled\t%d", p_Honda_FF_Xml.f_bsw_enabled);
		fprintf(SIL_XML_out, "\n\tf_cvw_enabled\t%d", p_Honda_FF_Xml.f_cvw_enabled);
		fprintf(SIL_XML_out, "\n\tf_lcda_enabled\t%d", p_Honda_FF_Xml.f_lcda_enabled);
		fprintf(SIL_XML_out, "\n\t</LCDA_SM_OUTPUT>");
		fprintf(SIL_XML_out, "\n</LCDA_DESCRIPTION_INFO>\t\t\t\t</LCDA_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n</LCDA_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		
		/******************CTA DATA ***************************************************************/
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_CTA_FF_Xml, RCTA_Output);// RCTA = CTA for HONDA

		fprintf(SIL_XML_out, "\n\n<CTA_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n\t<CTA_OUTPUT>");
		fprintf(SIL_XML_out, "\n\tf_cta_warn_right\t\t%d", p_Honda_CTA_FF_Xml.f_cta_warn_right);
		fprintf(SIL_XML_out, "\n\tf_cta_warn_left\t\t%d", p_Honda_CTA_FF_Xml.f_cta_warn_left);
		fprintf(SIL_XML_out, "\n\tcta_heading_left\t\t%f", p_Honda_CTA_FF_Xml.cta_heading_left);
		fprintf(SIL_XML_out, "\n\tcta_heading_right\t\t%f", p_Honda_CTA_FF_Xml.cta_heading_right);
		fprintf(SIL_XML_out, "\n\tcta_id_left\t\t%d", p_Honda_CTA_FF_Xml.cta_id_left);
		fprintf(SIL_XML_out, "\n\tcta_id_right\t\t%d", p_Honda_CTA_FF_Xml.cta_id_right);
		fprintf(SIL_XML_out, "\n\tcta_intersection_point_x_left\t\t%f", p_Honda_CTA_FF_Xml.cta_intersection_point_x_left);
		fprintf(SIL_XML_out, "\n\tcta_intersection_point_x_right\t\t%f", p_Honda_CTA_FF_Xml.cta_intersection_point_x_right);
		fprintf(SIL_XML_out, "\n\tcta_objPoseX_left\t\t%f", p_Honda_CTA_FF_Xml.cta_objPoseX_left);
		fprintf(SIL_XML_out, "\n\tcta_objPoseX_right\t\t%f", p_Honda_CTA_FF_Xml.cta_objPoseX_right);
		fprintf(SIL_XML_out, "\n\tcta_objPoseY_left\t\t%f", p_Honda_CTA_FF_Xml.cta_objPoseY_left);
		fprintf(SIL_XML_out, "\n\tcta_objPoseY_right\t\t%f", p_Honda_CTA_FF_Xml.cta_objPoseY_right);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityX_left\t\t%f", p_Honda_CTA_FF_Xml.cta_objVelocityX_left);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityX_right\t\t%f", p_Honda_CTA_FF_Xml.cta_objVelocityX_right);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityY_left\t\t%f", p_Honda_CTA_FF_Xml.cta_objVelocityY_left);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityY_right\t\t%f", p_Honda_CTA_FF_Xml.cta_objVelocityY_right);
		fprintf(SIL_XML_out, "\n\tcta_ttc_left\t\t%f", p_Honda_CTA_FF_Xml.cta_ttc_left);
		fprintf(SIL_XML_out, "\n\tcta_ttc_right\t\t%f", p_Honda_CTA_FF_Xml.cta_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_cta_alert_left\t\t%d", p_Honda_CTA_FF_Xml.f_cta_alert_left);
		fprintf(SIL_XML_out, "\n\tf_cta_alert_right\t\t%d", p_Honda_CTA_FF_Xml.f_cta_alert_right);
		fprintf(SIL_XML_out, "\n\tf_cta_enabled\t\t%d", p_Honda_CTA_FF_Xml.f_cta_enabled);
		fprintf(SIL_XML_out, "\n\t</CTA_OUTPUT>");
		fprintf(SIL_XML_out, "\n</CTA_DESCRIPTION_INFO>\t\t</CTA_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n</CTA_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		/******************CED DATA ***************************************************************/
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_CED_FF_Xml, CED_Output);
		fprintf(SIL_XML_out, "\n<CED_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n\t<CED_OUTPUT>");

		fprintf(SIL_XML_out, "\n\tCED_alert_left\t\t\t%d", p_Honda_CED_FF_Xml.CED_alert_left);
		fprintf(SIL_XML_out, "\n\tCED_alert_right\t\t\t%d", p_Honda_CED_FF_Xml.CED_alert_right);
		fprintf(SIL_XML_out, "\n\tCED_dir_left\t\t\t%d", p_Honda_CED_FF_Xml.CED_dir_left);
		fprintf(SIL_XML_out, "\n\tCED_dir_right\t\t\t%d", p_Honda_CED_FF_Xml.CED_dir_right);
		fprintf(SIL_XML_out, "\n\tCED_id_left\t\t\t%d", p_Honda_CED_FF_Xml.CED_id_left);
		fprintf(SIL_XML_out, "\n\tCED_id_right\t\t\t%d", p_Honda_CED_FF_Xml.CED_id_right);
		fprintf(SIL_XML_out, "\n\tCED_object_predicted_lat_pos_left\t\t\t%f", p_Honda_CED_FF_Xml.CED_object_predicted_lat_pos_left);
		fprintf(SIL_XML_out, "\n\tCED_object_predicted_lat_pos_right\t\t\t%f", p_Honda_CED_FF_Xml.CED_object_predicted_lat_pos_right);
		fprintf(SIL_XML_out, "\n\tCED_ttc_left\t\t\t%f", p_Honda_CED_FF_Xml.CED_ttc_left);
		fprintf(SIL_XML_out, "\n\tCED_ttc_right\t\t\t%f", p_Honda_CED_FF_Xml.CED_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_ced_enable\t\t\t%d", p_Honda_CED_FF_Xml.f_ced_enable);

		fprintf(SIL_XML_out, "\n\t</CED_OUTPUT>");
		fprintf(SIL_XML_out, "\n</CED_DESCRIPTION_INFO>\t\t</CED_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n<CED_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	}
	
#if 1

	/******************SAFE EXIT ***************************************************************/

	//============================CED FEATURE================================
	if (custId == NISSAN_GEN5)
	{
		m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_FF_ping_xml_Gen5, FF_Output_Logging_Data);
		fprintf(SIL_XML_out, "\n<CED_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n\t<CED_OUTPUT>");

		fprintf(SIL_XML_out, "\n\tCED_alert_left\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_alert_left);
		fprintf(SIL_XML_out, "\n\tCED_alert_right\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_alert_right);
		fprintf(SIL_XML_out, "\n\tCED_dir_left\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_dir_left);
		fprintf(SIL_XML_out, "\n\tCED_dir_right\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_dir_right);
		fprintf(SIL_XML_out, "\n\tCED_id_left\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_id_left);
		fprintf(SIL_XML_out, "\n\tCED_id_right\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_id_right);
		fprintf(SIL_XML_out, "\n\tCED_object_predicted_lat_pos_left\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_object_predicted_lat_pos_left);
		fprintf(SIL_XML_out, "\n\tCED_object_predicted_lat_pos_right\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_object_predicted_lat_pos_right);
		fprintf(SIL_XML_out, "\n\tCED_ttc_left\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_ttc_left);
		fprintf(SIL_XML_out, "\n\tCED_ttc_right\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.CED_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_ced_enable\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CED_OUTPUT.f_ced_enable);

		fprintf(SIL_XML_out, "\n\t</CED_OUTPUT>");
		fprintf(SIL_XML_out, "\n</CED_DESCRIPTION_INFO>\t\t</CED_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n<CED_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		/******************CTA ***************************************************************/

		fprintf(SIL_XML_out, "\n\n<CTA_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n\t<CTA_OUTPUT>");
		fprintf(SIL_XML_out, "\n\tcta_heading_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_heading_left);
		fprintf(SIL_XML_out, "\n\tcta_heading_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_heading_right);
		fprintf(SIL_XML_out, "\n\tcta_id_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_id_left);
		fprintf(SIL_XML_out, "\n\tcta_id_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_id_right);
		fprintf(SIL_XML_out, "\n\tcta_intersection_point_x_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_intersection_point_x_left);
		fprintf(SIL_XML_out, "\n\tcta_intersection_point_x_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_intersection_point_x_right);
		fprintf(SIL_XML_out, "\n\tcta_objPoseX_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objPoseX_left);
		fprintf(SIL_XML_out, "\n\tcta_objPoseX_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objPoseX_right);
		fprintf(SIL_XML_out, "\n\tcta_objPoseY_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objPoseY_left);
		fprintf(SIL_XML_out, "\n\tcta_objPoseY_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objPoseY_right);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityX_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objVelocityX_left);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityX_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objVelocityX_right);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityY_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objVelocityY_left);
		fprintf(SIL_XML_out, "\n\tcta_objVelocityY_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_objVelocityY_right);
		fprintf(SIL_XML_out, "\n\tcta_ttc_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_ttc_left);
		fprintf(SIL_XML_out, "\n\tcta_ttc_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.cta_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_cta_alert_left\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.f_cta_alert_left);
		fprintf(SIL_XML_out, "\n\tf_cta_alert_right\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.f_cta_alert_right);
		fprintf(SIL_XML_out, "\n\tf_cta_enabledt\t\t%f", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.CTA_OUTPUT.f_cta_enabled);
		fprintf(SIL_XML_out, "\n\t</CTA_OUTPUT>");
		fprintf(SIL_XML_out, "\n</CTA_DESCRIPTION_INFO>\t\t</CTA_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n</CTA_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		/******************LCDA ***************************************************************/
		/*if (psuppress_XML_Prints->Output_XML_Trace_Files.LCDA_Info)
		{*/

		fprintf(SIL_XML_out, "\n\n<LCDA_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
		fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\t\t\t<LCDA_PARAMERTERS> ");

		fprintf(SIL_XML_out, "\n\t<LCDA_OUTPUT>");
		fprintf(SIL_XML_out, "\n\tbsw_alert_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.bsw_alert_left);
		fprintf(SIL_XML_out, "\n\tbsw_alert_right\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.bsw_alert_right);
		fprintf(SIL_XML_out, "\n\tbsw_id_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.bsw_id_left);
		fprintf(SIL_XML_out, "\n\tbsw_id_right\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.bsw_id_right);
		//fprintf(SIL_XML_out, "\n\tbsw_alert_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output);
		
		fprintf(SIL_XML_out, "\n\tlka_alert_condition\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_alert_condition);
		fprintf(SIL_XML_out, "\n\tlka_change_status\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_change_status);
		fprintf(SIL_XML_out, "\n\tlka_curvi_pos_lat\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_curvi_pos_lat);
		fprintf(SIL_XML_out, "\n\tlka_curvi_pos_long\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_curvi_pos_long);
		fprintf(SIL_XML_out, "\n\tlka_curvi_vel_lat\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_curvi_vel_lat);
		fprintf(SIL_XML_out, "\n\tlka_curvi_vel_long\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_curvi_vel_long);
		fprintf(SIL_XML_out, "\n\tlka_motion_class\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_motion_class);
		fprintf(SIL_XML_out, "\n\tlka_object_class\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_object_class);
		fprintf(SIL_XML_out, "\n\tlka_obj_id\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_obj_id);
		fprintf(SIL_XML_out, "\n\tlka_ttc\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Left[NUMBER_OF_LCDA_OBJECTS].lka_ttc);

		fprintf(SIL_XML_out, "\n\tlka_alert_condition\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_alert_condition);
		fprintf(SIL_XML_out, "\n\tlka_change_status\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_change_status);
		fprintf(SIL_XML_out, "\n\tlka_curvi_pos_lat\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_curvi_pos_lat);
		fprintf(SIL_XML_out, "\n\tlka_curvi_pos_long\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_curvi_pos_long);
		fprintf(SIL_XML_out, "\n\tlka_curvi_vel_lat\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_curvi_vel_lat);
		fprintf(SIL_XML_out, "\n\tlka_curvi_vel_long\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_curvi_vel_long);
		fprintf(SIL_XML_out, "\n\tlka_motion_class\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_motion_class);
		fprintf(SIL_XML_out, "\n\tlka_object_class\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_object_class);
		fprintf(SIL_XML_out, "\n\tlka_obj_id\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_obj_id);
		fprintf(SIL_XML_out, "\n\tlka_ttc\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.customer_output.LKA_Object_Right[NUMBER_OF_LCDA_OBJECTS].lka_ttc);
		
		fprintf(SIL_XML_out, "\n\tcvw_alert_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_alert_left);
		fprintf(SIL_XML_out, "\n\tcvw_alert_right\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_alert_right);
		fprintf(SIL_XML_out, "\n\tcvw_id_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_id_left);
		fprintf(SIL_XML_out, "\n\tcvw_id_right\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_id_right);
		fprintf(SIL_XML_out, "\n\tcvw_ttc_left\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_ttc_left);
		fprintf(SIL_XML_out, "\n\tcvw_ttc_right\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.cvw_ttc_right);
		fprintf(SIL_XML_out, "\n\tf_bsw_enabled\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.f_bsw_enabled);
		fprintf(SIL_XML_out, "\n\tf_cvw_enabled\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.f_cvw_enabled);
		fprintf(SIL_XML_out, "\n\tf_lcda_enabled\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.LCDA_OUTPUT.f_lcda_enabled);
		fprintf(SIL_XML_out, "\n\t</LCDA_SM_OUTPUT>");
		fprintf(SIL_XML_out, "\n</LCDA_DESCRIPTION_INFO>\t\t\t\t</LCDA_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n</LCDA_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		/******************TOD ***************************************************************/
		
		//fprintf(SIL_XML_out, "\n\n<TOD_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

		fprintf(SIL_XML_out, "\n<TOD_DESCRIPTION_INFO>\t\t\t\t<TOD_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n\t<TOD_OUTPUT>");
		
		fprintf(SIL_XML_out, "\n\tangle_object\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].angle_object);
		fprintf(SIL_XML_out, "\n\tclassification_object\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].classification_object);
		fprintf(SIL_XML_out, "\n\tclassification_proba\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].classification_proba);
		fprintf(SIL_XML_out, "\n\texistance_probability\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].existance_probability);
		fprintf(SIL_XML_out, "\n\tinside_road_border\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].inside_road_border);
		fprintf(SIL_XML_out, "\n\tobject_accel_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_accel_x);
		fprintf(SIL_XML_out, "\n\tobject_accel_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_accel_y);
		fprintf(SIL_XML_out, "\n\tobject_anchor_point\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_anchor_point);
		fprintf(SIL_XML_out, "\n\tobject_L1_shape\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_L1_shape);
		fprintf(SIL_XML_out, "\n\tobject_L2_shape\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_L2_shape);
		fprintf(SIL_XML_out, "\n\tobject_motion_class\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_motion_class);
		fprintf(SIL_XML_out, "\n\tobject_rad_cross_section\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_rad_cross_section);
		fprintf(SIL_XML_out, "\n\tobject_status\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_status);
		fprintf(SIL_XML_out, "\n\tobject_time_to_collision\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].object_time_to_collision);
		fprintf(SIL_XML_out, "\n\tobj_id\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].obj_id);
		fprintf(SIL_XML_out, "\n\trelative_distance_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].relative_distance_x);
		fprintf(SIL_XML_out, "\n\trelative_distance_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].relative_distance_y);
		fprintf(SIL_XML_out, "\n\trelative_speed_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].relative_speed_x);
		fprintf(SIL_XML_out, "\n\trelative_speed_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].relative_speed_y);
		fprintf(SIL_XML_out, "\n\trelative_target_yaw_rate\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.TODS_OUTPUT[TODS_RNA_NUMBER_OF_OBJECT].relative_target_yaw_rate);
		
		fprintf(SIL_XML_out, "\n\t</TOD_OUTPUT>");
		fprintf(SIL_XML_out, "\n</TOD_DESCRIPTION_INFO>\t\t</TOD_PARAMERTERS> ");

		fprintf(SIL_XML_out, "\n<FRONT_TODS_Output_DESCRIPTION_INFO>\t\t<FRONT_TODS_Output_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n\t<FRONT_TODS_Output_OUTPUT>");
		
		fprintf(SIL_XML_out, "\n\tangle_object\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].angle_object);
		fprintf(SIL_XML_out, "\n\tclassification_object\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].classification_object);
		fprintf(SIL_XML_out, "\n\tclassification_proba\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].classification_proba);
		fprintf(SIL_XML_out, "\n\texistance_probability\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].existance_probability);
		fprintf(SIL_XML_out, "\n\tinside_road_border\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].inside_road_border);
		fprintf(SIL_XML_out, "\n\tobject_accel_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_accel_x);
		fprintf(SIL_XML_out, "\n\tobject_accel_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_accel_y);
		fprintf(SIL_XML_out, "\n\tobject_anchor_point\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_anchor_point);
		fprintf(SIL_XML_out, "\n\tobject_L1_shape\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_L1_shape);
		fprintf(SIL_XML_out, "\n\tobject_L2_shape\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_L2_shape);
		fprintf(SIL_XML_out, "\n\tobject_motion_class\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_motion_class);
		fprintf(SIL_XML_out, "\n\tobject_rad_cross_section\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_rad_cross_section);
		fprintf(SIL_XML_out, "\n\tobject_status\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_status);
		fprintf(SIL_XML_out, "\n\tobject_time_to_collision\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].object_time_to_collision);
		fprintf(SIL_XML_out, "\n\tobj_id\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].obj_id);
		fprintf(SIL_XML_out, "\n\trelative_distance_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].relative_distance_x);
		fprintf(SIL_XML_out, "\n\trelative_distance_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].relative_distance_y);
		fprintf(SIL_XML_out, "\n\trelative_speed_x\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].relative_speed_x);
		fprintf(SIL_XML_out, "\n\trelative_speed_y\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].relative_speed_y);
		fprintf(SIL_XML_out, "\n\trelative_target_yaw_rate\t\t\t%d", p_g_FF_ping_xml_Gen5.FF_Output_Logging_Data_resim.FRONT_TODS_Output[FRONT_TODS_RNA_NUMBER_OF_OBJECT].relative_target_yaw_rate);
		
		fprintf(SIL_XML_out, "\n\t</FRONT_TODS_Output_OUTPUT>");
		fprintf(SIL_XML_out, "\n</FRONT_TODS_Output_DESCRIPTION_INFO>\t\t</FRONT_TODS_Output_PARAMERTERS> ");

		//fprintf(SIL_XML_out, "\n</TOD_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

		/******************RECW ***************************************************************/
		/*if (psuppress_XML_Prints->Output_XML_Trace_Files.RECW_Info)
		{*/

		fprintf(SIL_XML_out, "\n\n<RECW_FEATURE_INFO_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));

		fprintf(SIL_XML_out, "\n<RECW_DESCRIPTION_INFO>\t\t\t<RECW_PARAMERTERS>");
		fprintf(SIL_XML_out, "\n\t<RECW_INPUT>");
		fprintf(SIL_XML_out, "\n\tf_recw_switch\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_input_log.f_recw_switch);
		fprintf(SIL_XML_out, "\n\t</RECW_INPUT>");

		fprintf(SIL_XML_out, "\n\t<RECW_OUTPUT>");

		fprintf(SIL_XML_out, "\n\ti_recw_obj_class\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_obj_class);
		fprintf(SIL_XML_out, "\n\ti_recw_id\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_id);
		fprintf(SIL_XML_out, "\n\ti_recw_acute_alert_signal\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_acute_alert_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_alert_signal\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_alert_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_driver_information_signal\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_driver_information_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_enabled\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_enabled);
		fprintf(SIL_XML_out, "\n\td_recw_obj_class_plausibility\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_class_plausibility);
		fprintf(SIL_XML_out, "\n\td_recw_obj_crash_probability\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_crash_probability);
		fprintf(SIL_XML_out, "\n\td_recw_obj_guete\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_guete);
		fprintf(SIL_XML_out, "\n\td_recw_obj_heading\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_heading);
		fprintf(SIL_XML_out, "\n\td_recw_obj_radial_dist\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_radial_dist);
		fprintf(SIL_XML_out, "\n\td_recw_obj_pos_x_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_pos_x_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_pos_y_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_pos_y_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_abs_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_abs_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_y_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_y_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_x_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_x_rel);
		fprintf(SIL_XML_out, "\n\td_recw_ttc\t\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_ttc);
		fprintf(SIL_XML_out, "\n\t</RECW_OUTPUT>");
		fprintf(SIL_XML_out, "\n</RECW_DESCRIPTION_INFO>\t\t</RECW_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n</RECW_FEATURE_INFO_END_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
	}
#endif
}
void FeatureFunction_Output(IRadarStream *  g_pIRadarStrm, short stream_num)
{
	//uint8_t path_index;
	if (stream_num == 12)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
	}
	if (stream_num == 13)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_FF_ping_xml,FF_OUTPUT_XML_DATA);
		//g_pIRadarStrm->getData((unsigned char*)&p_g_SM_ping_xml,FF_SM_LOG_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_FF_ping_xml, FF_OUTPUT_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_SM_ping_xml, FF_SM_LOG_XML_DATA);
	}
#if 1

	/******************SAFE EXIT ***************************************************************/

		//============================CED FEATURE================================
	if (stream_num == 13)
	{
		fprintf(SIL_XML_out, "\n<CED_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		fprintf(SIL_XML_out, "\n<CED_DESCRIPTION_INFO>\t\t<CED_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n\t<CED_INPUT>");
		fprintf(SIL_XML_out, "\n\tf_CED_Status");
		fprintf(SIL_XML_out, "\t\t\t%d", p_g_FF_ping_xml.ced_log.ced_input_log.f_CED_Status);
		fprintf(SIL_XML_out, "\n\t</CED_INPUT>");

		fprintf(SIL_XML_out, "\n\t<CED_OUTPUT>");
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_ttc_right\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_ttc_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_ttc_left\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_ttc_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_lat_right\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_lat_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_lat_left\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_lat_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_ttc_right\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_ttc_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_ttc_left\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_ttc_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_lat_right\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_lat_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_lat_left\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_lat_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_ttc_right\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_ttc_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_ttc_left\t\t%f", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_ttc_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_Status\t\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_Status);
		fprintf(SIL_XML_out, "\n\tSFE_CED_alert_left\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_alert_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_alert_right\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_alert_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_dir_right\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_dir_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_dir_left\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_dir_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_status\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_status);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_alert_right\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_alert_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_alert_left\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_alert_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_id_right\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_id_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_rear_id_left\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_rear_id_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_status\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_status);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_alert_right\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_alert_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_alert_left\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_alert_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_id_right\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_id_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_front_id_left\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_front_id_left);
		fprintf(SIL_XML_out, "\n\tSFE_CED_id_right\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_id_right);
		fprintf(SIL_XML_out, "\n\tSFE_CED_id_left\t\t\t%d", p_g_FF_ping_xml.ced_log.ced_output.SFE_CED_id_left);
		fprintf(SIL_XML_out, "\n\t</CED_OUTPUT>");
		//fprintf(SIL_XML_out, "\n\t<CED_SM_INPUT>");
		/*fprintf(SIL_XML_out, "\n\tVeh_Speed\t\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.Veh_Speed);
		fprintf(SIL_XML_out, "\n\tDirection_of_travel\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.Direction_of_travel);
		fprintf(SIL_XML_out, "\n\tStatus_trailer\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.Status_trailer);
		fprintf(SIL_XML_out, "\n\tcontrol_safe_exit\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.control_safe_exit);
		fprintf(SIL_XML_out, "\n\tcontrol_safer_exit_sound\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.control_safer_exit_sound);
		fprintf(SIL_XML_out, "\n\tstatus_seat_occupying_DR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_seat_occupying_DR);
		fprintf(SIL_XML_out, "\n\tstatus_seat_occupying_PS\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_seat_occupying_PS);
		fprintf(SIL_XML_out, "\n\tstatus_seat_occupying_RDR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_seat_occupying_RDR);
		fprintf(SIL_XML_out, "\n\tstatus_seat_occupying_RPS\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_seat_occupying_RPS);
		fprintf(SIL_XML_out, "\n\tstatus_belt_switch_DR\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_belt_switch_DR);
		fprintf(SIL_XML_out, "\n\tstatus_belt_switch_PS\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_belt_switch_PS);
		fprintf(SIL_XML_out, "\n\tstatus_belt_switch_RDR\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_belt_switch_RDR);
		fprintf(SIL_XML_out, "\n\tstatus_belt_switch_RPS\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_belt_switch_RPS);
		fprintf(SIL_XML_out, "\n\tstatus_door_lock_DR\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_door_lock_DR);
		fprintf(SIL_XML_out, "\n\tstatus_door_lock_PS\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_door_lock_PS);
		fprintf(SIL_XML_out, "\n\tstatus_door_lock_RDR\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_door_lock_RDR);
		fprintf(SIL_XML_out, "\n\tstatus_door_lock_RPS\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_door_lock_RPS);
		fprintf(SIL_XML_out, "\n\tstatus_inside_pushbutton_DR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_inside_pushbutton_DR);
		fprintf(SIL_XML_out, "\n\tstatus_inside_pushbutton_PS\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_inside_pushbutton_PS);
		fprintf(SIL_XML_out, "\n\tstatus_inside_pushbutton_RDR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_inside_pushbutton_RDR);
		fprintf(SIL_XML_out, "\n\tstatus_inside_pushbutton_RPS\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.status_inside_pushbutton_RPS);
		fprintf(SIL_XML_out, "\n\tFL_SFE_CED_Degraded\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.FL_SFE_CED_Degraded);
		fprintf(SIL_XML_out, "\n\tFR_SFE_CED_Degraded\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.FR_SFE_CED_Degraded);
		fprintf(SIL_XML_out, "\n\tRL_SFE_CED_Degraded\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.RL_SFE_CED_Degraded);
		fprintf(SIL_XML_out, "\n\tRR_SFE_CED_Degraded\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_input.RR_SFE_CED_Degraded);
		fprintf(SIL_XML_out, "\n\t</CED_SM_INPUT>");
		fprintf(SIL_XML_out, "\n\t<CED_SM_OUTPUT>");*/
		/*fprintf(SIL_XML_out, "\n\trequest_interier_display_doors\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.request_interier_display_doors);
		fprintf(SIL_XML_out, "\n\trequest_automatic_door_stop\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.request_automatic_door_stop);
		fprintf(SIL_XML_out, "\n\trequest_door_lock\t\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.request_door_lock);
		fprintf(SIL_XML_out, "\n\trequestExteriorMirrorDisplayWarningDriverSFE\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.requestExteriorMirrorDisplayWarningDriverSFE);
		fprintf(SIL_XML_out, "\n\trequestExteriorMirrorDisplayWarningPassengerSFE\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.requestExteriorMirrorDisplayWarningPassengerSFE);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_FAO\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_FAO);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_DUR\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_DUR);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_LCN\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_LCN);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_TYP\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_TYP);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_ANG\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_ANG);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_DIST\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_DIST);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_HND\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_HND);
		fprintf(SIL_XML_out, "\n\tRequest_Gong_ID\t\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.Request_Gong_ID);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62ASafeExitOrientation\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62ASafeExitOrientation);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AGongType\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AGongType);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AGongLocalisation\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AGongLocalisation);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusDoorSwitchDRD\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AStatusDoorSwitchDRD);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusDoorSwitchPSD\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AStatusDoorSwitchPSD);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusDoorSwitchDVDR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AStatusDoorSwitchDVDR);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusDoorSwitchPSDR\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AStatusDoorSwitchPSDR);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusSafeExit\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ced_sm_output.EdrDrasyEventID62AStatusSafeExit);
		fprintf(SIL_XML_out, "\n\t</CED_SM_OUTPUT>");*/
		fprintf(SIL_XML_out, "\n</CED_DESCRIPTION_INFO>\t\t</CED_PARAMERTERS> ");


		fprintf(SIL_XML_out, "\n</CED_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

		/******************CTA ***************************************************************/

		fprintf(SIL_XML_out, "\n\n<CTA_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		fprintf(SIL_XML_out, "\n<CTA_DESCRIPTION_INFO>\t\t<CTA_PARAMERTERS> ");
		fprintf(SIL_XML_out, "\n\t<CTA_INPUT>");
		fprintf(SIL_XML_out, "\n\tf_cta_switch\t\t%d", p_g_FF_ping_xml.cta_log.cta_input_log.f_cta_switch);
		fprintf(SIL_XML_out, "\n\tcta_radar_position\t%d", p_g_FF_ping_xml.cta_log.cta_input_log.cta_radar_position);
		fprintf(SIL_XML_out, "\n\t</CTA_INPUT>");

		fprintf(SIL_XML_out, "\n\t<CTA_OUTPUT>");
		fprintf(SIL_XML_out, "\n\tcrash_prob_left\t\t%f", p_g_FF_ping_xml.cta_log.cta_core_output.crash_prob_left);
		fprintf(SIL_XML_out, "\n\tcrash_prob_right\t%f", p_g_FF_ping_xml.cta_log.cta_core_output.crash_prob_right);
		fprintf(SIL_XML_out, "\n\twarn_level_left\t\t%d", p_g_FF_ping_xml.cta_log.cta_core_output.warn_level_left);
		fprintf(SIL_XML_out, "\n\twarn_level_right\t%d", p_g_FF_ping_xml.cta_log.cta_core_output.warn_level_right);
		fprintf(SIL_XML_out, "\n\tf_cta_enabled\t\t%d", p_g_FF_ping_xml.cta_log.cta_core_output.f_cta_enabled);
		fprintf(SIL_XML_out, "\n\t</CTA_OUTPUT>");
		/*fprintf(SIL_XML_out, "\n\t<CTA_SM_INPUT>");
		fprintf(SIL_XML_out, "\n\tspeed_speedometer\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.speed_speedometer);
		fprintf(SIL_XML_out, "\n\tspeed_vehicle\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.speed_vehicle);
		fprintf(SIL_XML_out, "\n\tdrivingTorque_driverRequest\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.drivingTorque_driverRequest);
		fprintf(SIL_XML_out, "\n\tgradient_actuation_acceleratorPedal\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.gradient_actuation_acceleratorPedal);
		fprintf(SIL_XML_out, "\n\tcontrol_cross_traffic_brake\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.control_cross_traffic_brake);
		fprintf(SIL_XML_out, "\n\tst_drvdir_dvch\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.st_drvdir_dvch);
		fprintf(SIL_XML_out, "\n\tdrvdir_veh_cfd\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.drvdir_veh_cfd);
		fprintf(SIL_XML_out, "\n\tst_tria\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.st_tria);
		fprintf(SIL_XML_out, "\n\tstatus_rollerDynamometerMode\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.status_rollerDynamometerMode);
		fprintf(SIL_XML_out, "\n\tqualifier_speed_vehicle\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.qualifier_speed_vehicle);
		fprintf(SIL_XML_out, "\n\tdrivingDirection_vehicle\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.drivingDirection_vehicle);
		fprintf(SIL_XML_out, "\n\trequest_dimming_warning_laneChange\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.request_dimming_warning_laneChange);
		fprintf(SIL_XML_out, "\n\tqualifier_gradient_actuation_acceleratorPedal\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.qualifier_gradient_actuation_acceleratorPedal);
		fprintf(SIL_XML_out, "\n\tstatus_endOfLineMode\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.status_endOfLineMode);
		fprintf(SIL_XML_out, "\n\tstatus_activation_function_parking\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.status_activation_function_parking);
		fprintf(SIL_XML_out, "\n\trequest_states_function_parking\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_input.request_states_function_parking);
		fprintf(SIL_XML_out, "\n\t</CTA_SM_INPUT>");
		fprintf(SIL_XML_out, "\n\t<CTA_SM_OUTPUT>");*/
		/*fprintf(SIL_XML_out, "\n\ttarget_state_longitudinalGuidance\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.target_state_longitudinalGuidance);
		fprintf(SIL_XML_out, "\n\ttargetAcceleration_target\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.targetAcceleration_target);
		fprintf(SIL_XML_out, "\n\tqualifier_function_ctb\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.qualifier_function_ctb);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_ID\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_ID);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_HND\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_HND);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_DIST\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_DIST);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_ANG\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_ANG);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_TYP\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_TYP);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_LCN\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_LCN);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_FAO\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_FAO);
		fprintf(SIL_XML_out, "\n\twarning_acoustic GO_DUR\t\t%d", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.GO_DUR);
		fprintf(SIL_XML_out, "\n\twarning_acoustic IGP_updated\t\t%d,", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.warning_acoustic.IGP_updated);
		fprintf(SIL_XML_out, "\n\tCounterRequestCTB\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.CounterRequestCTB);
		fprintf(SIL_XML_out, "\n\tstatus_brakeRequest\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.status_brakeRequest);
		fprintf(SIL_XML_out, "\n\tdisplay_warning_graphic\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.display_warning_graphic);
		fprintf(SIL_XML_out, "\n\trequest_warning_displayElement_left\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.request_warning_displayElement_left);
		fprintf(SIL_XML_out, "\n\trequest_warning_displayElement_right\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.request_warning_displayElement_right);
		fprintf(SIL_XML_out, "\n\trequest_warning_dimming\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.request_warning_dimming);
		fprintf(SIL_XML_out, "\n\tqualifier_targetAccel_limit\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.qualifier_targetAccel_limit);
		fprintf(SIL_XML_out, "\n\trequest_brake_integrity\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.request_brake_integrity);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62CTBStatus\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.EdrDrasyEventID62CTBStatus);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62CTBOrientation\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.EdrDrasyEventID62CTBOrientation);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62CTBBraking\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.EdrDrasyEventID62CTBBraking);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62GongType\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.EdrDrasyEventID62GongType);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62GongLocalisation\t\t%f", p_g_SM_ping_xml.ff_sm_log.cta_sm_output.EdrDrasyEventID62GongLocalisation);
		fprintf(SIL_XML_out, "\n\t</CTA_SM_OUTPUT>");*/
		fprintf(SIL_XML_out, "\n</CTA_DESCRIPTION_INFO>\t\t</CTA_PARAMERTERS> ");


		fprintf(SIL_XML_out, "\n</CTA_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		/******************LCDA ***************************************************************/
		/*if (psuppress_XML_Prints->Output_XML_Trace_Files.LCDA_Info)
		{*/

		//fprintf(SIL_XML_out, "\n\n<LCDA_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		//fprintf(SIL_XML_out, "\n<LCDA_DESCRIPTION_INFO>\t\t\t\t<LCDA_PARAMERTERS> ");
		//fprintf(SIL_XML_out, "\n\t<LCDA_INPUT>");
		//fprintf(SIL_XML_out, "\n\tnavigation_data_number_of_lanes\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.navigation_data_number_of_lanes);
		//fprintf(SIL_XML_out, "\n\tnavigation_data_road_type\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.navigation_data_road_type);
		//fprintf(SIL_XML_out, "\n\tlcda_warntrigger_hmi\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.lcda_warntrigger_hmi);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable_environment_plausibilization\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable_environment_plausibilization);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable_dropback\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable_dropback);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable_slc\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable_slc);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable_cvw\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable_cvw);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable_bsw\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable_bsw);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enable\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_input_log.f_lcda_enable);
		////fprintf(SIL_XML_out, "\n\tlane_model_lane_center_offset\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_input_log.lane_model_lane_center_offset);
		////fprintf(SIL_XML_out, "\n\tlane_model_lane_width\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_input_log.lane_model_lane_width);
		//fprintf(SIL_XML_out, "\n\t</LCDA_INPUT>");

		//fprintf(SIL_XML_out, "\n\t<LCDA_OUTPUT>");
		//fprintf(SIL_XML_out, "\n\tlcda_object_lane_change_probability_right\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_lane_change_probability_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_id_right\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_id_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_lane_change_probability_left\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_lane_change_probability_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_id_left\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_id_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_type_right\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_type_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_type_left\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_type_left);
		//fprintf(SIL_XML_out, "\n\tf_slc_enabled\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.f_slc_enabled);
		//fprintf(SIL_XML_out, "\n\tf_cvw_enabled\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.f_cvw_enabled);
		//fprintf(SIL_XML_out, "\n\tf_bsw_enabled\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.f_bsw_enabled);
		//fprintf(SIL_XML_out, "\n\tf_lcda_enabled\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.f_lcda_enabled);
		//fprintf(SIL_XML_out, "\n\tslc_alert[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.slc_alert[0]);
		//fprintf(SIL_XML_out, "\n\tslc_alert[1]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.slc_alert[1]);
		//fprintf(SIL_XML_out, "\n\tcvw_alert[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_alert[0]);
		//fprintf(SIL_XML_out, "\n\tcvw_alert[1]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_alert[1]);
		//fprintf(SIL_XML_out, "\n\tbsw_alert[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.bsw_alert[0]);
		//fprintf(SIL_XML_out, "\n\tbsw_alert[1]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.bsw_alert[1]);
		//fprintf(SIL_XML_out, "\n\tlcda_object_existance_probability_right\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_existance_probability_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_existance_probability_left\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_existance_probability_left);
		//fprintf(SIL_XML_out, "\n\tslc_id[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.slc_id[0]);
		//fprintf(SIL_XML_out, "\n\tslc_id[1]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.slc_id[1]);
		//fprintf(SIL_XML_out, "\n\tcvw_id[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_id[0]);
		//fprintf(SIL_XML_out, "\n\tcvw_id[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_id[1]);
		//fprintf(SIL_XML_out, "\n\tbsw_id[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.bsw_id[0]);
		//fprintf(SIL_XML_out, "\n\tbsw_id[0]\t\t\t\t\t%d", p_g_FF_ping_xml.lcda_log.lcda_output.bsw_id[1]);
		//fprintf(SIL_XML_out, "\n\tlcda_object_vy_right\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_vy_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_vx_right\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_vx_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_ttc_right\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_ttc_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_py_right\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_py_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_px_right\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_px_right);
		//fprintf(SIL_XML_out, "\n\tlcda_object_vy_left\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_vy_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_vx_left\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_vx_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_ttc_left\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_ttc_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_py_left\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_py_left);
		//fprintf(SIL_XML_out, "\n\tlcda_object_px_left\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.lcda_object_px_left);

		//fprintf(SIL_XML_out, "\n\tslc_lane_change_probability[0]\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.slc_lane_change_probability[0]);
		//fprintf(SIL_XML_out, "\n\ttslc_lane_change_probability[1]\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.slc_lane_change_probability[1]);
		//fprintf(SIL_XML_out, "\n\tslc_ttc[0]\t\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.slc_ttc[0]);
		//fprintf(SIL_XML_out, "\n\tslc_ttc[1]\t\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.slc_ttc[1]);
		//fprintf(SIL_XML_out, "\n\tcvw_ttc[0]\t\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_ttc[0]);
		//fprintf(SIL_XML_out, "\n\tcvw_ttc[1]\t\t\t\t\t%f", p_g_FF_ping_xml.lcda_log.lcda_output.cvw_ttc[1]);

		//fprintf(SIL_XML_out, "\n\t</LCDA_OUTPUT>");
		//fprintf(SIL_XML_out, "\n\t<LCDA_SM_INPUT>");
		//fprintf(SIL_XML_out, "\n\tVeh_Speed\t\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.lcda_sm_input.Veh_Speed);
		//fprintf(SIL_XML_out, "\n\tDirection_of_travel\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.lcda_sm_input.Direction_of_travel);
		//fprintf(SIL_XML_out, "\n\tStatus_trailer\t\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.lcda_sm_input.Status_trailer);
		//fprintf(SIL_XML_out, "\n\t</LCDA_SM_INPUT>");
		//fprintf(SIL_XML_out, "\n\t<LCDA_SM_OUTPUT>");
		//fprintf(SIL_XML_out, "\n\tStatus_Blind_Spot_Area\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.lcda_sm_output.Status_Blind_Spot_Area);
		//fprintf(SIL_XML_out, "\n\tStatus_Dynamic_Area\t\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.lcda_sm_output.Status_Dynamic_Area);
		//fprintf(SIL_XML_out, "\n\tStatus_Simultaneous_Lane_Change\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.lcda_sm_output.Status_Simultaneous_Lane_Change);
		//fprintf(SIL_XML_out, "\n\tExtended_Qualifier\t\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.lcda_sm_output.Extended_Qualifier);
		//fprintf(SIL_XML_out, "\n\t</LCDA_SM_OUTPUT>");
		//fprintf(SIL_XML_out, "\n</LCDA_DESCRIPTION_INFO>\t\t\t\t</LCDA_PARAMERTERS> ");


		//fprintf(SIL_XML_out, "\n</LCDA_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


	/******************RECW ***************************************************************/
	/*if (psuppress_XML_Prints->Output_XML_Trace_Files.RECW_Info)
	{*/

		fprintf(SIL_XML_out, "\n\n<RECW_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		fprintf(SIL_XML_out, "\n<RECW_DESCRIPTION_INFO>\t\t\t<RECW_PARAMERTERS>");
		fprintf(SIL_XML_out, "\n\t<RECW_INPUT>");
		fprintf(SIL_XML_out, "\n\tf_recw_switch\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_input_log.f_recw_switch);
		fprintf(SIL_XML_out, "\n\t</RECW_INPUT>");

		fprintf(SIL_XML_out, "\n\t<RECW_OUTPUT>");

		fprintf(SIL_XML_out, "\n\ti_recw_obj_class\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_obj_class);
		fprintf(SIL_XML_out, "\n\ti_recw_id\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_id);
		fprintf(SIL_XML_out, "\n\ti_recw_acute_alert_signal\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_acute_alert_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_alert_signal\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_alert_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_driver_information_signal\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_driver_information_signal);
		fprintf(SIL_XML_out, "\n\ti_recw_enabled\t\t\t\t%d", p_g_FF_ping_xml.recw_Log.recw_post_run.i_recw_enabled);
		fprintf(SIL_XML_out, "\n\td_recw_obj_class_plausibility\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_class_plausibility);
		fprintf(SIL_XML_out, "\n\td_recw_obj_crash_probability\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_crash_probability);
		fprintf(SIL_XML_out, "\n\td_recw_obj_guete\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_guete);
		fprintf(SIL_XML_out, "\n\td_recw_obj_heading\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_heading);
		fprintf(SIL_XML_out, "\n\td_recw_obj_radial_dist\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_radial_dist);
		fprintf(SIL_XML_out, "\n\td_recw_obj_pos_x_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_pos_x_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_pos_y_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_pos_y_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_abs_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_abs_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_y_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_y_rel);
		fprintf(SIL_XML_out, "\n\td_recw_obj_vel_x_rel\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_obj_vel_x_rel);
		fprintf(SIL_XML_out, "\n\td_recw_ttc\t\t\t\t%f", p_g_FF_ping_xml.recw_Log.recw_post_run.d_recw_ttc);
		fprintf(SIL_XML_out, "\n\t</RECW_OUTPUT>");
		/*fprintf(SIL_XML_out, "\n\t<RECW_SM_INPUT>");
		fprintf(SIL_XML_out, "\n\tspeed\t\t\t\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.recw_sm_input.speed);
		fprintf(SIL_XML_out, "\n\tDirection_of_travel\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.recw_sm_input.Direction_of_travel);
		fprintf(SIL_XML_out, "\n\tStatus_trailer\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.recw_sm_input.Status_trailer);
		fprintf(SIL_XML_out, "\n\tdynamometerMode\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.recw_sm_input.dynamometerMode);
		fprintf(SIL_XML_out, "\n\tstatus_endOfLineMode\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_input.status_endOfLineMode);
		fprintf(SIL_XML_out, "\n\t</RECW_SM_INPUT>");
		fprintf(SIL_XML_out, "\n\t<RECW_SM_OUTUT>");
		fprintf(SIL_XML_out, "\n\tSollbeschleunigung_Ziel\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Sollbeschleunigung_Ziel);
		fprintf(SIL_XML_out, "\n\trecw_state_PCAN\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.recw_state_PCAN);
		fprintf(SIL_XML_out, "\n\tStatus_PreCrash_Heckradar\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Status_PreCrash_Heckradar);
		fprintf(SIL_XML_out, "\n\tStatus_Auffahrwarnung_Heck\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Status_Auffahrwarnung_Heck);
		fprintf(SIL_XML_out, "\n\tAnnaeherungsgeschwindigkeit_PreCrash_Heckradar\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Annaeherungsgeschwindigkeit_PreCrash_Heckradar);
		fprintf(SIL_XML_out, "\n\tAufprallzeit_PreCrash_Heckradar\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Aufprallzeit_PreCrash_Heckradar);
		fprintf(SIL_XML_out, "\n\tAnforderung_Bremse_Qualifier\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Anforderung_Bremse_Qualifier);
		fprintf(SIL_XML_out, "\n\tQualifier_Sollbeschleunigung_Ziel_Limit\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Qualifier_Sollbeschleunigung_Ziel_Limit);
		fprintf(SIL_XML_out, "\n\tSoll_Zustand_Laengsfuehrung\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Soll_Zustand_Laengsfuehrung);
		fprintf(SIL_XML_out, "\n\tAnforderung_Bremse_Integritaet\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.Anforderung_Bremse_Integritaet);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusCollisionWarningSideRadarRear\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.EdrDrasyEventID62AStatusCollisionWarningSideRadarRear);
		fprintf(SIL_XML_out, "\n\tEdrDrasyEventID62AStatusPreCrashSideRadarRear\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.recw_sm_output.EdrDrasyEventID62AStatusPreCrashSideRadarRear);
		fprintf(SIL_XML_out, "\n\t</RECW_SM_OUTUT>");*/
		fprintf(SIL_XML_out, "\n</RECW_DESCRIPTION_INFO>\t\t</RECW_PARAMERTERS> ");


		fprintf(SIL_XML_out, "\n</RECW_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


		//******************SCW ***************************************************************/
		/*if (psuppress_XML_Prints->Output_XML_Trace_Files.SCW_Info)
		{*/

		//	fprintf(SIL_XML_out, "\n\n<SCW_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		//
		//
		//	fprintf(SIL_XML_out, "\n<SCW_DESCRIPTION_INFO>\t\t\t\t<SCW_PARAMERTERS>");
		//	fprintf(SIL_XML_out, "\n\t<SCW_INPUT>");
		//	fprintf(SIL_XML_out, "\n\tf_scw_enable\t\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_input_log.f_scw_enable);
		//	fprintf(SIL_XML_out, "\n\tf_scw_enable_dynamic\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_input_log.f_scw_enable_dynamic);
		//	fprintf(SIL_XML_out, "\n\tf_scw_enable_guardrail\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_input_log.f_scw_enable_guardrail);
		//	fprintf(SIL_XML_out, "\n\t</SCW_INPUT>");

		//	fprintf(SIL_XML_out, "\n\t<SCW_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\tscw_object_id_right\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_id_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_type_right\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_type_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_id_left\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_id_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_type_left\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_type_left);
		//	fprintf(SIL_XML_out, "\n\tf_scw_guardrail_enabled\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.f_scw_guardrail_enabled);
		//	fprintf(SIL_XML_out, "\n\tf_scw_dyn_enabled\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.f_scw_dyn_enabled);
		//	fprintf(SIL_XML_out, "\n\tf_scw_enabled\t\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.f_scw_enabled);
		//	fprintf(SIL_XML_out, "\n\tscw_object_age_right\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_age_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_age_left\t\t\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_age_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_existance_probability_right\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_existance_probability_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_existance_probability_left\t\t%d", p_g_FF_ping_xml.scw_log.scw_output.scw_object_existance_probability_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ay_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ay_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ax_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ax_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_vy_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_vy_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_vx_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_vx_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ttc_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ttc_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_heading_right\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_heading_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_length_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_length_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_width_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_width_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_py_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_py_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_px_right\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_px_right);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ay_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ay_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ax_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ax_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_vy_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_vy_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_vx_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_vx_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_ttc_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_ttc_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_heading_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_heading_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_length_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_length_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_width_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_width_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_py_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_py_left);
		//	fprintf(SIL_XML_out, "\n\tscw_object_px_left\t\t\t\t%f", p_g_FF_ping_xml.scw_log.scw_output.scw_object_px_left);
		//	fprintf(SIL_XML_out, "\n\t</SCW_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\t<SCW_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\t</SCW_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\t<SCW_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\tevent_data_qualifier\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.scw_sm_output.event_data_qualifier);
		//	fprintf(SIL_XML_out, "\n\textended_qualifier\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.scw_sm_output.extended_qualifier);
		//	fprintf(SIL_XML_out, "\n\tstate_scw\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.scw_sm_output.state_scw);
		//	fprintf(SIL_XML_out, "\n\tstatus_guardrail\t\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.scw_sm_output.status_guardrail);
		//	fprintf(SIL_XML_out, "\n\t</SCW_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n</SCW_DESCRIPTION_INFO>\t\t\t\t</SCW_PARAMERTERS> ");
		//

		//	fprintf(SIL_XML_out, "\n</SCW_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		//

		////fprintf(SIL_XML_out, "\n============================TA FEATURE================================");

		///*if (psuppress_XML_Prints->Output_XML_Trace_Files.TA_Info)
		//{*/

		//	fprintf(SIL_XML_out, "\n\n<TA_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		//

		//	fprintf(SIL_XML_out, "\n<TA_DESCRIPTION_INFO>\t\t\t\t\t<TA_PARAMERTERS>");
		//	fprintf(SIL_XML_out, "\n\t<TA_INPUT>");
		//	fprintf(SIL_XML_out, "\n\tf_fta_enable\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_input_log.f_fta_enable);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_input_log.f_rta_enable);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable_turning_area\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_input_log.f_rta_enable_turning_area);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable_dynamic_area\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_input_log.f_rta_enable_dynamic_area);
		//	fprintf(SIL_XML_out, "\n\tacceleration_longitudinal_cog\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.acceleration_longitudinal_cog);
		//	fprintf(SIL_XML_out, "\n\tacceleration_longitudinal_cog_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.acceleration_longitudinal_cog_qualifier);
		//	fprintf(SIL_XML_out, "\n\tacceleration_lateral_cog\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.acceleration_lateral_cog);
		//	fprintf(SIL_XML_out, "\n\tacceleration_lateral_cog_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.acceleration_lateral_cog_qualifier);
		//	fprintf(SIL_XML_out, "\n\tacceleration_driving_direction\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.acceleration_driving_direction);
		//	fprintf(SIL_XML_out, "\n\tstatus_braking_driver\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_braking_driver);
		//	fprintf(SIL_XML_out, "\n\taccelerator_pedal_angle\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.accelerator_pedal_angle);
		//	fprintf(SIL_XML_out, "\n\taccelerator_pedal_angle_gradient\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.accelerator_pedal_angle_gradient);
		//	fprintf(SIL_XML_out, "\n\taccelerator_pedal_angle_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.accelerator_pedal_angle_qualifier);
		//	fprintf(SIL_XML_out, "\n\tego_trajectory_curvaturett\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.ego_trajectory_curvature);
		//	fprintf(SIL_XML_out, "\n\tego_trajectory_curvature_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.ego_trajectory_curvature_qualifier);
		//	fprintf(SIL_XML_out, "\n\tcontrol_prewarning\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.control_prewarning);
		//	fprintf(SIL_XML_out, "\n\tcontrol_move_off_prevention\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.control_move_off_prevention);
		//	fprintf(SIL_XML_out, "\n\tdriving_direction_vehicle_confirmed\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.driving_direction_vehicle_confirmed);
		//	fprintf(SIL_XML_out, "\n\tstatus_warn_brake_coordinator\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_warn_brake_coordinator);
		//	fprintf(SIL_XML_out, "\n\tqualifier_function_brake_chain\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.qualifier_function_brake_chain);
		//	fprintf(SIL_XML_out, "\n\tqualifier_function_warning_chain\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.qualifier_function_warning_chain);
		//	fprintf(SIL_XML_out, "\n\tsteering_angle_driver\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.steering_angle_driver);
		//	fprintf(SIL_XML_out, "\n\tsteering_angle_driver_qualifier\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.steering_angle_driver_qualifier);
		//	fprintf(SIL_XML_out, "\n\tsteering_angle_front_axle\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.steering_angle_front_axle);
		//	fprintf(SIL_XML_out, "\n\tsteering_angle_front_axle_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.steering_angle_front_axle_qualifier);
		//	fprintf(SIL_XML_out, "\n\tsteering_angle_front_axle_condition\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.steering_angle_front_axle_condition);
		//	fprintf(SIL_XML_out, "\n\tstatus_condition_vehicle\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_condition_vehicle);
		//	fprintf(SIL_XML_out, "\n\tstatus_condition_vehicle_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_condition_vehicle_qualifier);
		//	fprintf(SIL_XML_out, "\n\tvelocity_vehicle_longitudinal\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.velocity_vehicle_longitudinal);
		//	fprintf(SIL_XML_out, "\n\tvelocity_vehicle_longitudinal_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.velocity_vehicle_longitudinal_qualifier);
		//	fprintf(SIL_XML_out, "\n\tyawrate\t\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.yawrate);
		//	fprintf(SIL_XML_out, "\n\tyawrate_qualifier\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.yawrate_qualifier);
		//	fprintf(SIL_XML_out, "\n\tstatus_control_longitudinal_guidance\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_control_longitudinal_guidance);
		//	fprintf(SIL_XML_out, "\n\tstatus_interface_driverassistance_limit\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.status_interface_driverassistance_limit);
		//	fprintf(SIL_XML_out, "\n\tdriver_attention_level\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.driver_attention_level);
		//	fprintf(SIL_XML_out, "\n\tdriver_attention_level_qualifier\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.driver_attention_level_qualifier);
		//	fprintf(SIL_XML_out, "\n\tdriver_attention_eyelid_closure\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.driver_attention_eyelid_closure);
		//	fprintf(SIL_XML_out, "\n\tdriver_attention_vision_aversion\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.driver_attention_vision_aversion);
		//	fprintf(SIL_XML_out, "\n\tlcda_warntrigger_hmi\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_input_log.bmw_bordnet_signals.lcda_warntrigger_hmi);
		//	fprintf(SIL_XML_out, "\n\t</TA_INPUT>");

		//	fprintf(SIL_XML_out, "\n\t<TA_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\tfta_brake_deceleration_request\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.fta_brake_deceleration_request);
		//	fprintf(SIL_XML_out, "\n\tfta_target_gap\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.fta_target_gap);
		//	fprintf(SIL_XML_out, "\n\trta_long_posn_left\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_long_posn_left);
		//	fprintf(SIL_XML_out, "\n\trta_lat_posn_left\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_lat_posn_left);
		//	fprintf(SIL_XML_out, "\n\trta_long_posn_right\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_long_posn_right);
		//	fprintf(SIL_XML_out, "\n\trta_lat_posn_right\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_lat_posn_right);
		//	fprintf(SIL_XML_out, "\n\trta_ttc_left\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_ttc_left);
		//	fprintf(SIL_XML_out, "\n\trta_ttc_right\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_ttc_right);
		//	fprintf(SIL_XML_out, "\n\trta_long_vel_left\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_long_vel_left);
		//	fprintf(SIL_XML_out, "\n\trta_lat_vel_left\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_lat_vel_left);
		//	fprintf(SIL_XML_out, "\n\trta_long_vel_right\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_long_vel_right);
		//	fprintf(SIL_XML_out, "\n\trta_lat_vel_right\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_lat_vel_right);
		//	fprintf(SIL_XML_out, "\n\trta_existence_probability_left\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_existence_probability_left);
		//	fprintf(SIL_XML_out, "\n\trta_existence_probability_right\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.rta_existence_probability_right);
		//	fprintf(SIL_XML_out, "\n\tta_ttc_left\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.ta_ttc_left);
		//	fprintf(SIL_XML_out, "\n\tta_collision_probability_left\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.ta_collision_probability_left);
		//	fprintf(SIL_XML_out, "\n\tta_ttc_right\t\t\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.ta_ttc_right);
		//	fprintf(SIL_XML_out, "\n\tta_collision_probability_right\t\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.ta_collision_probability_right);
		//	fprintf(SIL_XML_out, "\n\tta_current_deceleration_estimate\t\t\t%f", p_g_FF_ping_xml.ta_log.ta_output.ta_current_deceleration_estimate);
		//	fprintf(SIL_XML_out, "\n\tfta_alert_level\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_alert_level);
		//	fprintf(SIL_XML_out, "\n\tfta_symbol_request\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_symbol_request);
		//	fprintf(SIL_XML_out, "\n\tfta_brake_threshold_reduction\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_brake_threshold_reduction);
		//	fprintf(SIL_XML_out, "\n\tfta_brake_conditioning\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_brake_conditioning);
		//	fprintf(SIL_XML_out, "\n\tfta_brake_integrity_level\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_brake_integrity_level);
		//	fprintf(SIL_XML_out, "\n\tf_fta_enable\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.f_fta_enable);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.f_rta_enable);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable_turning_area\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.f_rta_enable_turning_area);
		//	fprintf(SIL_XML_out, "\n\tf_rta_enable_dynamic_area\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.f_rta_enable_dynamic_area);
		//	fprintf(SIL_XML_out, "\n\tfta_target_id\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.fta_target_id);
		//	fprintf(SIL_XML_out, "\n\trta_dynamic_area_status\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_dynamic_area_status);
		//	fprintf(SIL_XML_out, "\n\trta_turning_area_status\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_turning_area_status);
		//	fprintf(SIL_XML_out, "\n\trta_alert_left\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_alert_left);
		//	fprintf(SIL_XML_out, "\n\trta_alert_right\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_alert_right);
		//	fprintf(SIL_XML_out, "\n\trta_id_left\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_id_left);
		//	fprintf(SIL_XML_out, "\n\trta_id_right\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.rta_id_right);
		//	fprintf(SIL_XML_out, "\n\tta_alert_left\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_alert_left);
		//	fprintf(SIL_XML_out, "\n\tta_id_left\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_id_left);
		//	fprintf(SIL_XML_out, "\n\tta_alert_right\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_alert_right);
		//	fprintf(SIL_XML_out, "\n\tta_id_right\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_id_right);
		//	fprintf(SIL_XML_out, "\n\tta_turn_maneuver\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_turn_maneuver);
		//	fprintf(SIL_XML_out, "\n\tta_turn_signal\t\t\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_turn_signal);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_run_ta_cal_pointer_7\t\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_run_ta_cal_pointer_7);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_run_ta_input_tracker_output_pointer_6\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_run_ta_input_tracker_output_pointer_6);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_run_ta_input_vehicle_data_pointer_5\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_run_ta_input_vehicle_data_pointer_5);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_run_ta_input_pointer_4\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_run_ta_input_pointer_4);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_init_ta_cal_pointer_3\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_init_ta_cal_pointer_3);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_init_ta_input_tracker_output_pointer_2\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_init_ta_input_tracker_output_pointer_2);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_init_ta_input_vehicle_data_pointer_1\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_init_ta_input_vehicle_data_pointer_1);
		//	fprintf(SIL_XML_out, "\n\tf_error_ta_init_ta_input_pointer_0\t\t\t%d", p_g_FF_ping_xml.ta_log.ta_output.ta_errors_output.f_error_ta_init_ta_input_pointer_0);
		//	fprintf(SIL_XML_out, "\n\t</TA_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\t<TA_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\tEgo_Speed\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.ta_sm_input.Ego_Speed);
		//	fprintf(SIL_XML_out, "\n\tDirection_Of_Travel_TAP\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ta_sm_input.Direction_Of_Travel_TAP);
		//	fprintf(SIL_XML_out, "\n\tStatus_Turning_Area_TAP\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ta_sm_input.Status_Turning_Area_TAP);
		//	fprintf(SIL_XML_out, "\n\tStatus_Dynamic_Area_TAP\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.ta_sm_input.Status_Dynamic_Area_TAP);
		//	fprintf(SIL_XML_out, "\n\t</TA_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\t<TA_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\tStatus_Event_Data_Qualifier\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ta_sm_output.Status_Event_Data_Qualifier);
		//	fprintf(SIL_XML_out, "\n\tStatus_Extended_Qualifier\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ta_sm_output.Status_Extended_Qualifier);
		//	fprintf(SIL_XML_out, "\n\tStatus_Critical_Object\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ta_sm_output.Status_Critical_Object);
		//	fprintf(SIL_XML_out, "\n\tStatus_Non_Critical_Object\t\t\t%u", p_g_SM_ping_xml.ff_sm_log.ta_sm_output.Status_Non_Critical_Object);
		//	fprintf(SIL_XML_out, "\n\t</TA_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\t<PFGS_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\tv_veh_cog\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.v_veh_cog);
		//	fprintf(SIL_XML_out, "\n\tSteering_angle\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.Steering_angle);
		//	fprintf(SIL_XML_out, "\n\tyaw_rate\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.yaw_rate);
		//	fprintf(SIL_XML_out, "\n\tcontrolActiveSafety\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.controlActiveSafety);
		//	fprintf(SIL_XML_out, "\n\tST_DYNOMOD\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.ST_DYNOMOD);
		//	fprintf(SIL_XML_out, "\n\tST_EOLMOD\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.ST_EOLMOD);
		//	fprintf(SIL_XML_out, "\n\tStDrivDirVeh\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.StDrivDirVeh);
		//	fprintf(SIL_XML_out, "\n\tdriving_direction_choice\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_input.driving_direction_choice);
		//	fprintf(SIL_XML_out, "\n\t</PFGS_SM_INPUT>");
		//	fprintf(SIL_XML_out, "\n\t<PFGS_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n\tgapTargetObjectPPPSide\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.gapTargetObjectPPPSide);
		//	fprintf(SIL_XML_out, "\n\ttargetDecelerationPPPSide\t\t\t%f", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.targetDecelerationPPPSide);
		//	fprintf(SIL_XML_out, "\n\tcrc_HeadwayobservationPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.crc_HeadwayobservationPPPSide);
		//	fprintf(SIL_XML_out, "\n\talive_HeadwayobservationPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.alive_HeadwayobservationPPPSide);
		//	fprintf(SIL_XML_out, "\n\tchl_HeadwayobservationPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.chl_HeadwayobservationPPPSide);
		//	fprintf(SIL_XML_out, "\n\tqualifierFunctionPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.qualifierFunctionPPPSide);
		//	fprintf(SIL_XML_out, "\n\treductionThValueBrakeAssistPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.reductionThValueBrakeAssistPPPSide);
		//	fprintf(SIL_XML_out, "\n\tforecastSafetyBrakePPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.forecastSafetyBrakePPPSide);
		//	fprintf(SIL_XML_out, "\n\twarningPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.warningPPPSide);
		//	fprintf(SIL_XML_out, "\n\trequestSymbolPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.requestSymbolPPPSide);
		//	fprintf(SIL_XML_out, "\n\tintegrityBrakeRequestPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.integrityBrakeRequestPPPSide);
		//	fprintf(SIL_XML_out, "\n\tidTargetObjectPPPSide\t\t\t%d", p_g_SM_ping_xml.ff_sm_log.pfgs_sm_output.idTargetObjectPPPSide);
		//	fprintf(SIL_XML_out, "\n\t</PFGS_SM_OUTPUT>");
		//	fprintf(SIL_XML_out, "\n</TA_DESCRIPTION_INFO>\t\t\t\t\t</TA_PARAMERTERS> ");
		//	
		//
		//	fprintf(SIL_XML_out, "\n</TA_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		//	
		////}

		////fprintf(SIL_XML_out, "\n============================PATH TRACKING FEATURE================================");
		///*if (psuppress_XML_Prints->Output_XML_Trace_Files.PATHTRACKING_Info)
		//{*/
		//
		//	fprintf(SIL_XML_out, "\n\n<PATH_TRACKING_FEATURE_INFO_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		//	
		//
		//	fprintf(SIL_XML_out, "\n\t<PATH_TRACKING_PERSISTENT_OUTPUT>\t\t<PATH_TRACKING_PARAMERTERS> ");
		//	fprintf(SIL_XML_out, "\n\tf_function_execution_state\t\t\t\t%d", p_g_FF_ping_xml.path_tracking_log.PATH_TRACKING_Persistent.f_function_execution_state);
		//	fprintf(SIL_XML_out, "\n\tclosest_to_ego_path_long_f_valid_path_exists\t\t%d", p_g_FF_ping_xml.path_tracking_log.PATH_TRACKING_Persistent.closest_to_ego_path_long.f_valid_path_exists);
		//	fprintf(SIL_XML_out, "\n\tclosest_to_ego_path_long_heading\t\t\t%d", p_g_FF_ping_xml.path_tracking_log.PATH_TRACKING_Persistent.closest_to_ego_path_long.heading);
		//	fprintf(SIL_XML_out, "\n\tclosest_to_ego_path_lat_f_valid_path_exists\t\t%d", p_g_FF_ping_xml.path_tracking_log.PATH_TRACKING_Persistent.closest_to_ego_path_lat.f_valid_path_exists);
		//	fprintf(SIL_XML_out, "\n\tclosest_to_ego_path_lat_heading\t\t\t\t%d", p_g_FF_ping_xml.path_tracking_log.PATH_TRACKING_Persistent.closest_to_ego_path_lat.heading);

		//	fprintf(SIL_XML_out, "\n\t</PATH_TRACKING_PERSISTENT_OUTPUT>\t\t</PATH_TRACKING_PARAMERTERS> ");
		//	fprintf(SIL_XML_out, "\n");
		//	fprintf(SIL_XML_out, "\n<PATH_TRACKING_OUTPUT_INFO>\trange_at_zero,\textrapolated_range_at_front_axle,\tpath_heading,\tttc_at_zero,\tpath_direction,\ttrack_match_age,\ttrack_match,\ttrack_match_last_cycle,\tf_matched_path_linear,\t</PATHTRACKING_OUTPUT_INFO> ");
		//	for (uint8_t path_index = 0; path_index < NUMBER_OF_OBJECTS; path_index++)
		//	{
		//		fprintf(SIL_XML_out, "\n\t<PATH_TRACKING_OUTPUT>");
		//		fprintf(SIL_XML_out, "\t%f", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].range_at_zero);
		//		fprintf(SIL_XML_out, "\t%f", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].extrapolated_range_at_front_axle);
		//		fprintf(SIL_XML_out, "\t\t\t\t%f", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].path_heading);
		//		fprintf(SIL_XML_out, "\t%f", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].ttc_at_zero);
		//		fprintf(SIL_XML_out, "\t%d", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].path_direction);
		//		fprintf(SIL_XML_out, "\t\t%d", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].track_match_age);
		//		fprintf(SIL_XML_out, "\t\t\t%d", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].track_match);
		//		fprintf(SIL_XML_out, "\t\t%d", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].track_match_last_cycle);
		//		fprintf(SIL_XML_out, "\t\t\t%d", p_g_FF_ping_xml.path_tracking_log.path_tracking_output[path_index].f_matched_path_linear);
		//		fprintf(SIL_XML_out, "\t\t\t</PATH_TRACKING_OUTPUT>");
		//	
		//	}
		//
		//	fprintf(SIL_XML_out, "\n</PATH_TRACKING_FEATURE_INFO_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
	}
#endif
}
#define FIXED 6
// reverses a string 'str' of length 'len' 
void reverse(char *str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}
int intToStr(int x, char str[], int d)
{
	int i = 0;

	while (x)
	{
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}

	// If number of digits required is more, then 
	// add 0s at the beginning 
	while (i < d)
		str[i++] = '0';

	reverse(str, i);

	str[i] = '\0';

	return i;
}

#define OBJ_CLASS_ENUM 5
void Tracks_Info(IRadarStream *  g_pIRadarStrm, short stream_num)
{
	if (stream_num == 12)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ALL_object_xml, ALL_OBJECTXML_DATA);

		/*g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
		g_pIRadarStrm->getData((unsigned char*)&ALL_object_xml,ALL_OBJECTXML_DATA);*/
	}
	//uint8_t Track_index;
	unsigned8_T count = 1, ped_count = 1, truck_count = 1, car_count = 1, wheel_count = 1;
	char strData[32]; //size of float32
	/*sort by longitudinal_velocity*/
	float32_T Track_Objs[MAX_F360_OBJECTS] = { 0 };
	for (unsigned8_T i = 0; i < MAX_F360_OBJECTS; i++)
	{
		Track_Objs[i] = ALL_object_xml.obj[i].vcs_xposn;
	}
	unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
	unsigned8_T indexe[MAX_F360_OBJECTS] = { 0 };
	for (unsigned8_T i = 0; i < size; i++)
	{
		indexe[i] = i;
	}
	array_data = Track_Objs;
	qsort(indexe, size, sizeof(*indexe), compare);

	static unsigned8_T INVALID;
	static unsigned8_T NEW;
	static unsigned8_T NEW_COASTED;
	static unsigned8_T NEW_UPDATED;
	static unsigned8_T UPDATED;
	static unsigned8_T COASTED;

	for (unsigned8_T i = 0; i < MAX_F360_OBJECTS; i++)
	{
		unsigned8_T status = ALL_object_xml.obj[indexe[i]].status;
		if (status == 0)
		{
			INVALID++;
		}
		if (status == 1)
		{
			NEW++;
		}
		if (status == 2)
		{
			NEW_COASTED++;
		}
		if (status == 3)
		{
			NEW_UPDATED++;
		}
		if (status == 4)
		{
			UPDATED++;
		}
		if (status == 5)
		{
			COASTED++;
		}
	}
	INVALID = NEW = NEW_COASTED = NEW_UPDATED = COASTED = UPDATED = 0;
#if 1
	if (stream_num == 12)
	{
		for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
		{

			if (((ALL_object_xml.obj[indexe[Track_index]].status) > 0) && (count == 1))
			{
				fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
				fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tObject_Class,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
				count = 0;
			}
			if ((ALL_object_xml.obj[indexe[Track_index]].status) > 0)
			{
				fprintf(SIL_XML_out, "\n  <TRACKS>");

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xposn, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yposn, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

				switch (ALL_object_xml.obj[indexe[Track_index]].object_class)
				{
				case 0:fprintf(SIL_XML_out, "\tUNKNOWN\t"); break;
				case 1:fprintf(SIL_XML_out, "\tPEDESTRIAN"); break;
				case 2:fprintf(SIL_XML_out, "\t2WHEEL\t"); break;
				case 3:fprintf(SIL_XML_out, "\tCAR\t"); break;
				case 4:fprintf(SIL_XML_out, "\tTRUCK\t"); break;
				default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				}

				switch (ALL_object_xml.obj[indexe[Track_index]].status)
				{
				case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
				case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
				case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
				case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
				case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
				case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
				}

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].len1, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].wid1, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1);

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xvel, strData, FIXED);
				fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yvel, strData, FIXED);
				fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

				//floatToString(ALL_object_xml.obj[indexe[Track_index]].heading, strData, FIXED);
				fprintf(SIL_XML_out, "\t\t\t%f", ALL_object_xml.obj[indexe[Track_index]].heading);

				fprintf(SIL_XML_out, "\t</TRACKS>");
			}
		}
		if (count == 0)
		{
			fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_TRACK_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
		}
		/*Tracker Object Class Description*/
		for (unsigned8_T obj_class_count = 1; obj_class_count < OBJ_CLASS_ENUM; obj_class_count++)
		{

			switch (obj_class_count)
			{
			case 1:
			{
				for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
				{
					if (ALL_object_xml.obj[indexe[Track_index]].object_class == obj_class_count)
					{
						if (((ALL_object_xml.obj[indexe[Track_index]].status) > 0) && (ped_count == 1))
						{
							fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_PEDESTRIAN_CLASS_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
							fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
							ped_count = 0;
						}
						if ((ALL_object_xml.obj[indexe[Track_index]].status) > 0)
						{
							fprintf(SIL_XML_out, "\n  <TRACKS>");

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);

							//	floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

							switch (ALL_object_xml.obj[indexe[Track_index]].status)
							{
							case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
							case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
							case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
							case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
							case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
							case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
							}

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].len1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].wid1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].heading, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t\t%d", ALL_object_xml.obj[indexe[Track_index]].heading);

							fprintf(SIL_XML_out, "\t</TRACKS>");
						}

					}
				}
				if (ped_count == 0)
				{
					fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_TRACK_PEDESTRIAN_CLASS_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
				}
			}

			break;
			case 2:
			{
				for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
				{
					if (ALL_object_xml.obj[indexe[Track_index]].object_class == obj_class_count)
					{
						if (((ALL_object_xml.obj[indexe[Track_index]].status) > 0) && (wheel_count == 1))
						{
							fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_2WHEEL_CLASS_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
							fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
							wheel_count = 0;
						}
						if ((ALL_object_xml.obj[indexe[Track_index]].status) > 0)
						{
							fprintf(SIL_XML_out, "\n  <TRACKS>");

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

							switch (ALL_object_xml.obj[indexe[Track_index]].status)
							{
							case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
							case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
							case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
							case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
							case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
							case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
							}

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].len1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].wid1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].heading, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t\t%f", ALL_object_xml.obj[indexe[Track_index]].heading);

							fprintf(SIL_XML_out, "\t</TRACKS>");
						}

					}
				}

				if (wheel_count == 0)
				{
					fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_TRACK_2WHEEL_CLASS_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
				}
			}
			break;
			case 3:
			{
				for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
				{
					if (ALL_object_xml.obj[indexe[Track_index]].object_class == obj_class_count)
					{
						if (((ALL_object_xml.obj[indexe[Track_index]].status) > 0) && (car_count == 1))
						{
							fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_CAR_CLASS_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
							fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
							car_count = 0;
						}
						if ((ALL_object_xml.obj[indexe[Track_index]].status) > 0)
						{
							fprintf(SIL_XML_out, "\n  <TRACKS>");

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

							switch (ALL_object_xml.obj[indexe[Track_index]].status)
							{
							case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
							case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
							case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
							case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
							case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
							case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
							}

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].len1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].wid1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1, strData);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].heading, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t\t%f", ALL_object_xml.obj[indexe[Track_index]].heading);

							fprintf(SIL_XML_out, "\t</TRACKS>");
						}

					}
				}
				if (car_count == 0)
				{
					fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_CAR_CLASS_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
				}
			}

			break;
			case 4:
			{
				for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
				{
					if (ALL_object_xml.obj[indexe[Track_index]].object_class == obj_class_count)
					{
						if (((ALL_object_xml.obj[indexe[Track_index]].status) > 0) && (truck_count == 1))
						{
							fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_CLASS_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
							fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
							truck_count = 0;
						}
						if ((ALL_object_xml.obj[indexe[Track_index]].status) > 0)
						{
							fprintf(SIL_XML_out, "\n  <TRACKS>");

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yposn, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

							switch (ALL_object_xml.obj[indexe[Track_index]].status)
							{
							case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
							case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
							case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
							case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
							case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
							case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
							}

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].len1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].wid1, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_xvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].vcs_yvel, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

							//floatToString(ALL_object_xml.obj[indexe[Track_index]].heading, strData, FIXED);
							fprintf(SIL_XML_out, "\t\t\t%f", ALL_object_xml.obj[indexe[Track_index]].heading);

							fprintf(SIL_XML_out, "\t</TRACKS>");
						}

					}
				}
				if (truck_count == 0)
				{
					fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_TRACK_CLASS_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
				}
			}

			break;

			}
		}
	}
#endif
}

/*Print Record Header Info*/
void Record_Header_ECU_Info(IRadarStream * g_pIRadarStrm, short stream_num)
{

	if (stream_num == 12)
	{
		/*g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
		g_pIRadarStrm->getData((unsigned char*)&ALL_object_xml,ALL_OBJECTXML_DATA);*/
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ALL_object_xml, ALL_OBJECTXML_DATA);
	}
	if (stream_num == 11)
	{
		//g_pIRadarStrm->getData((unsigned char*)&core0_tracker_xml,RAW_SRR_DETS);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_tracker_xml, RAW_SRR_DETS);

	}
	//uint8_t sensor_index;
	char strData[32];
	//fprintf(SIL_XML_out,"\n\n<Run_CounterIAL>\t%d\t</Run_CounterIAL>",p_g_tracker_IAL_ping_xml.radar_ecu_CORE0_Hdr.run_counter);
	//fprintf(SIL_XML_out,"\n<Run_CounterOAL>\t%d\t</Run_CounterOAL>",p_g_tracker_OAL_ping_xml.radar_ecu_CORE1_Hdr.run_counter);
	if (stream_num == 12)
	{

		fprintf(SIL_XML_out, "\n<RECORD_HEADER_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

		fprintf(SIL_XML_out, "\n\t<REDUCED_NO_VALID_TRACKS>\t%d\t</REDUCED_NO_VALID_TRACKS>", p_g_tracker_OAL_ping_xml.reduced_num_active_objs);
		fprintf(SIL_XML_out, "\n\t<NO_VALID_TRACKS>\t\t%d\t</NO_VALID_TRACKS>", p_g_tracker_OAL_ping_xml.num_active_objs);
	}
	if (stream_num == 11 && p_g_tracker_OAL_ping_xml.tracker_index != 0)
	{
		for (uint8_t sensor_index = 0; sensor_index < MAX_NUMBER_OF_SRR_SENSORS; sensor_index++)
		{
			switch (sensor_index)
			{
			case 0:fprintf(SIL_XML_out, "\n\n<TARGET_REPORT_INFO_FRONT_LEFT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 1:fprintf(SIL_XML_out, "\n\n<TARGET_REPORT_INFO_FRONT_RIGHT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 2:fprintf(SIL_XML_out, "\n\n<TARGET_REPORT_INFO_REAR_LEFT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 3:fprintf(SIL_XML_out, "\n\n<TARGET_REPORT_INFO_REAR_RIGHT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			}


			//floatToString(core0_tracker_xml.timestamp, strData, FIXED);
			fprintf(SIL_XML_out, "\n\tTIMESTAMP\t\t\t%f", core0_tracker_xml.timestamp);
			fprintf(SIL_XML_out, "\n\tVCS_BORESIGHT_AZ_ALIGN_ANGLE\t%f", core0_tracker_xml.vcs_boresight_az_align_angle);
			fprintf(SIL_XML_out, "\n\tVCS_BORESIGHT_EL_ALIGN_ANGLE\t%f", core0_tracker_xml.vcs_boresight_el_align_angle);
			fprintf(SIL_XML_out, "\n\tLOOK_INDEX\t\t\t%d", core0_tracker_xml.look_index);
			fprintf(SIL_XML_out, "\n\tSENSOR_ID\t\t\t%d", core0_tracker_xml.sensor_id);
			fprintf(SIL_XML_out, "\n\tLOOK_ID\t\t\t\t%d", core0_tracker_xml.look_id);
			fprintf(SIL_XML_out, "\n\tLOOK_TYPE\t\t\t%d", core0_tracker_xml.look_type);
			fprintf(SIL_XML_out, "\n\tRANGE_TYPE\t\t\t%d", core0_tracker_xml.range_type);
			fprintf(SIL_XML_out, "\n\tF_AZ_ALIGN_APPILED\t\t%d", core0_tracker_xml.f_az_align_applied);
			fprintf(SIL_XML_out, "\n\tF_FULL_BLOCKAGE\t\t\t%d", core0_tracker_xml.f_full_blockage);
			fprintf(SIL_XML_out, "\n\tNUM_DETS\t\t\t%d", core0_tracker_xml.num_dets);


			switch (sensor_index)
			{

			case 0:fprintf(SIL_XML_out, "\n</TARGET_REPORT_INFO_FRONT_LEFT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 1:fprintf(SIL_XML_out, "\n</TARGET_REPORT_INFO_FRONT_RIGHT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 2:fprintf(SIL_XML_out, "\n</TARGET_REPORT_INFO_REAR_LEFT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			case 3:fprintf(SIL_XML_out, "\n</TARGET_REPORT_INFO_REAR_RIGHT_%d>", p_g_tracker_OAL_ping_xml.tracker_index); break;
			}
		}
	}

}
/*Print Vehicle Info*/
void Vehicle_ECU_Info(IRadarStream * g_pIRadarStrm, short stream_num)
{
	if (stream_num == 12)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
	}
	if (stream_num == 11)
	{
		//g_pIRadarStrm->getData((unsigned char*)&core0_Ego_vehicle_xml,EGO_VEHICLE_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_Ego_vehicle_xml, EGO_VEHICLE_XML_DATA);
	}
#if 1
	if (p_g_tracker_OAL_ping_xml.tracker_index != 0)
	{
		if (stream_num == 11)
		{
			fprintf(SIL_XML_out, "\n\n<VEHICLE_DESCRIPTION_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);


			fprintf(SIL_XML_out, "\n<VEHICLE_DESCRIPTION_INFO>\t\t<VEHICLE_PARAMETER>");
			fprintf(SIL_XML_out, "\n\tRAW_SPEED\t\t\t%f", core0_Ego_vehicle_xml.raw_speed);
			fprintf(SIL_XML_out, "\n\tYAW_RATE\t\t\t%f", core0_Ego_vehicle_xml.yaw_rate);
			fprintf(SIL_XML_out, "\n\tDISTANCE_REAR_AXLE_TO_VCS\t%f", core0_Ego_vehicle_xml.dist_rear_axle_to_vcs);
			fprintf(SIL_XML_out, "\n\tPRNDL\t\t\t\t%d", core0_Ego_vehicle_xml.prndl);
			fprintf(SIL_XML_out, "\n\tREVRSE_GEAR\t\t\t%d", core0_Ego_vehicle_xml.reverse_gear);
			fprintf(SIL_XML_out, "\n\tSPEED_CORRECTION_FACTOR\t\t%f", core0_Ego_vehicle_xml.speed_correction_factor);
			fprintf(SIL_XML_out, "\n</VEHICLE_DESCRIPTION_INFO>\t\t</VEHICLE_PARAMETER>");
			fprintf(SIL_XML_out, "\n</VEHICLE_DESCRIPTION_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
		}
	}
#endif	

}
/*-------------------GEN7 Detection Data----------------------------------*/



void Detection_ECU_Info(IRadarStream * g_pIRadarStrm, short stream_num)
{
	if (stream_num == 12)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
	}
	if (stream_num == 11)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_Ego_vehicle_xml, EGO_VEHICLE_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_tracker_time_xml, Tracker_Time);
		//g_pIRadarStrm->getData((unsigned char*)&core0_Ego_vehicle_xml,EGO_VEHICLE_XML_DATA);
		// g_pIRadarStrm->getData((unsigned char*)&core0_tracker_time_xml,Tracker_Time); 
	}
	uint8_t sensor_index, count = 1;
	unsigned16_T dets_index;
	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
	//Emb_Lib_Output_Control_T* psuppress_XML_Prints = pEmb_Lib_Output_Control;;
	//fprintf(SIL_XML_out, "\n<UDP SI Error status>\t%d\t%d\t%d\t%d\n", SI_error_UDP[0], SI_error_UDP[1], SI_error_UDP[2], SI_error_UDP[3]);
	//fprintf(SIL_XML_out, "<PCAN SI Error status>\t%d\t%d\t%d\t%d\n", SI_error_PCAN[0], SI_error_PCAN[1], SI_error_PCAN[2], SI_error_PCAN[3]);
	for (int sensor_index = 0; sensor_index < MAX_NUMBER_OF_SRR_SENSORS; sensor_index++)
	{
		/*sort by Range*/

		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
		{
			Self_Dets[i] = core0_tracker_xml.raw_dets[i].range;
		}
		unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
		unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
		for (unsigned8_T i = 0; i < size; i++)
		{
			index[i] = i;
		}
		array_data = Self_Dets;
		raw_srr_dets_indx = sensor_index;
		DetectionsFlag = true;
		qsort(index, size, sizeof(*index), compare);
		DetectionsFlag = false;
		/*If  range values are equal then sort by azimuth*/
		/*for (dets_index = 0; dets_index < NUMBER_OF_DETECTIONS; dets_index++)
		{
			if (dets_index >= (NUMBER_OF_DETECTIONS - (core0_tracker_xml.num_dets)))
			{
				int temp;
				if(core0_tracker_xml.raw_dets[index[dets_index]].range == core0_tracker_xml.raw_dets[index[dets_index+1]].range)
				{
					if (core0_tracker_xml.raw_dets[index[dets_index]].azimuth > core0_tracker_xml.raw_dets[index[dets_index + 1]].azimuth)
					{
						temp = index[dets_index + 1];
						index[dets_index+1]= index[dets_index];
						index[dets_index] = temp;
					}

				}
			}
		}*///causing crash need to check this algorithm

		if (stream_num == 12)
		{
			if (core0_tracker_xml.num_dets > 0)
			{
				switch (sensor_index)
				{
				case 0:
					fprintf(SIL_XML_out, "\n\n<DETECTION_FRONT_LEFT_DESCRIPTION_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
					write_Dets_Info(sensor_index, index, g_pIRadarStrm, stream_num);
					fprintf(SIL_XML_out, "\n</DETECTION_FRONT_LEFT_DESCRIPTION_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

					break;
				case 1:
					fprintf(SIL_XML_out, "\n\n<DETECTION_FRONT_RIGHT_DESCRIPTION_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
					write_Dets_Info(sensor_index, index, g_pIRadarStrm, stream_num);
					fprintf(SIL_XML_out, "\n</DETECTION_FRONT_RIGHT_DESCRIPTION_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

					break;
				case 2:
					fprintf(SIL_XML_out, "\n\n<DETECTION_REAR_LEFT_DESCRIPTION_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
					write_Dets_Info(sensor_index, index, g_pIRadarStrm, stream_num);
					fprintf(SIL_XML_out, "\n</DETECTION_REAR_LEFT_DESCRIPTION_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);

					break;
				case 3:
					fprintf(SIL_XML_out, "\n\n<DETECTION_REAR_RIGHT_DESCRIPTION_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
					write_Dets_Info(sensor_index, index, g_pIRadarStrm, stream_num);
					fprintf(SIL_XML_out, "\n</DETECTION_REAR_RIGHT_DESCRIPTION_END_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
					break;
				}
			}

		}
		if (stream_num == 11)
		{
			fprintf(SIL_XML_out, "\n\n\t<TRACKER_RUN_TIMING_INFO>%d</TRACKER_RUN_TIMING_INFO>\n", core0_tracker_time_xml.tracker_timestamp_us);
		}
	}
}

void write_Dets_Info(uint8_t sensor_index, unsigned8_T *index, IRadarStream * g_pIRadarStrm, short stream_num) {
	if (stream_num == 12)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
	}
	if (stream_num == 11)
	{
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_tracker_xml, RAW_SRR_DETS);
		//g_pIRadarStrm->getData((unsigned char*)&core0_tracker_xml,RAW_SRR_DETS);
	}
	//  g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_IAL_ping_xml,EGO_VEHICLE_XML_DATA);
	  //unsigned16_T dets_index;
	if (stream_num == 12)
	{
		fprintf(SIL_XML_out, "\n<RAW_DETS_INFO>\t\tRANGE,\t\tRANGE_RATE,\tAZIMUTH,\tAZIMUTH_UNCORR,\tELEVATION,\tAMPLITUDE,\tCONFID_AZIMUTH,\tCONFID_ELEVATION,\tVALID,\tHOST_CLUTTER,\tSUPER_RES_TARGET,\t</RAW_DETS_INFO>");
		for (unsigned16_T dets_index = 0; dets_index < NUMBER_OF_DETECTIONS; dets_index++)
		{
			if (dets_index >= (NUMBER_OF_DETECTIONS - (core0_tracker_xml.num_dets)))
			{
				fprintf(SIL_XML_out, "\n\t<RAW_DETS>");
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].range);
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].range_rate);
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].azimuth);
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].azimuth_uncorr);
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].elevation);
				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].amplitude);
				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].confid_azimuth);
				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].confid_elevation);
				fprintf(SIL_XML_out, "\t\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].valid);
				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].host_clutter);
				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].super_res_target);
				fprintf(SIL_XML_out, "\t</RAW_DETS>");
			}
		}
	}
}



void PCAN_Detection_Info(IRadarStream * g_pIRadarStrm, short stream_num)
{
	if (stream_num == 12)
	{
		g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,RAW_SRR_DETS);
	}
	//if(stream_num == 11)
	//{
	// g_pIRadarStrm->getData((unsigned char*)&core0_tracker_xml,RAW_SRR_DETS);
	//}
	////uint8_t Sensor_Index = 0;
	////unsigned16_T dets_index = 0;
	//if(stream_num == 11)
	//{
	//fprintf(SIL_XML_out, "\n\n<============PCAN_DETECTION_INFO============>");
	//for (uint8_t Sensor_Index = 0; Sensor_Index < MAX_NUMBER_OF_SRR_SENSORS; Sensor_Index++)
	//{
	//
	//		fprintf(SIL_XML_out, "\n\n************* SENSOR_INDEX=%d *************", Sensor_Index);

	//		//fprintf(SIL_XML_out, "\n\t<PCAN_Detection_Info>");
	//		fprintf(SIL_XML_out, "\n\tTimeStamp\t\t\t%f", core0_tracker_xml.timestamp);
	//		fprintf(SIL_XML_out, "\n\tLook_ID\t\t\t\t%d", core0_tracker_xml.look_id);
	//		fprintf(SIL_XML_out, "\n\tLookIndex\t\t\t%d", core0_tracker_xml.look_index);
	//		//fprintf(SIL_XML_out, "\n\tTargetReportCnt\t\t\t%d", p_g_tracker_OAL_ping_xml.raw_srr_dets[Sensor_Index].TargetReportCnt);
	//		fprintf(SIL_XML_out, "\n\tLookType\t\t\t%d", core0_tracker_xml.look_type);
	//		fprintf(SIL_XML_out, "\n\tnum_dets\t\t\t%d\t", core0_tracker_xml.num_dets);
	//		fprintf(SIL_XML_out, "\n\tsensor_id\t\t\t%d\t", core0_tracker_xml.sensor_id);
	//		//fprintf(SIL_XML_out, "\n\ttimestamp_consistency\t\t%d", core0_tracker_xml.timestamp_consistency);
	//
	//	/*sort by Range*/
	//	float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };

	//		for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
	//		{
	//			Self_Dets[i] = core0_tracker_xml.raw_dets[i].range;
	//		}
	//	
	//	unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
	//	unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
	//	for (unsigned8_T i = 0; i < size; i++)
	//	{
	//		index[i] = i;
	//	}
	//	array_data = Self_Dets;
	//	raw_srr_dets_indx = Sensor_Index;
	//	DetectionsFlag = true;
	//	qsort(index, size, sizeof(*index), compare);
	//	DetectionsFlag = false;
	//
	//		fprintf(SIL_XML_out, "\n<RAW_DETS_INFO>\t\tAZIMUTH,\tRANGE,\t\tRANGE_RATE,\tELEVATION,\tAMPLITUDE,\tCONFID_AZIMUTH,\tCONFID_ELEVATION,\tVALID,\tHOST_CLUTTER,\t</RAW_DETS_INFO>");
	//		for (unsigned16_T dets_index = 0; dets_index < NUMBER_OF_DETECTIONS; dets_index++)
	//		{
	//			if (core0_tracker_xml.raw_dets[index[dets_index]].range)
	//			{
	//				fprintf(SIL_XML_out, "\n\t<RAW_DETS>");
	//				fprintf(SIL_XML_out, "\t%f", core0_tracker_xml.raw_dets[index[dets_index]].azimuth);
	//				//fprintf(SIL_XML_out, "\t%f", pcan_Detections[Sensor_Index].Detections.raw_dets[dets_index].azimuth_uncorr);
	//				fprintf(SIL_XML_out, "\t\t%f", core0_tracker_xml.raw_dets[index[dets_index]].range);
	//				fprintf(SIL_XML_out, "\t\t%f", core0_tracker_xml.raw_dets[index[dets_index]].range_rate);
	//				fprintf(SIL_XML_out, "\t\t%f", core0_tracker_xml.raw_dets[index[dets_index]].elevation);
	//				fprintf(SIL_XML_out, "\t\t%f", core0_tracker_xml.raw_dets[index[dets_index]].amplitude);
	//				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].confid_azimuth);
	//				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].confid_elevation);
	//				fprintf(SIL_XML_out, "\t\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].valid);
	//				fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[index[dets_index]].host_clutter);
	//				//fprintf(SIL_XML_out, "\t%d\t", core0_tracker_xml.raw_dets[dets_index].super_res_target);
	//				fprintf(SIL_XML_out, "\t</RAW_DETS>");
	//			}

	//		}
	//		fprintf(SIL_XML_out, "\n\n</============PCAN_DETECTION_INFO============>");
	//}
	//}
}
void Record_Header_ECU_Count(IRadarStream  *  g_pIRadarStrm, short stream_num)
{

	if (stream_num == 12)
	{
		fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
	}
}

void Core_Execution_Time(IRadarStream  *  g_pIRadarStrm, short stream_num)
{
	if (stream_num == 12)
	{
		/*	g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,Tracker_Info_XML_DATA);
			g_pIRadarStrm->getData((unsigned char*)&Core1_Timimg_info_xml,ECU_TIMING_INFO);*/
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml, Tracker_Info_XML_DATA);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Core1_Timimg_info_xml, ECU_TIMING_INFO);
	}
	if (stream_num == 11)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,RAW_SRR_DETS);
		//g_pIRadarStrm->getData((unsigned char*)&core0_time_xml,ECU_TIMING_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&core0_time_xml, ECU_TIMING_INFO);
	}
	if (stream_num == 13)
	{
		//g_pIRadarStrm->getData((unsigned char*)&p_g_timeinfo_xml,ECU_TIMING_INFO);
		m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_timeinfo_xml, ECU_TIMING_INFO);
		//g_pIRadarStrm->getData((unsigned char*)&p_g_tracker_OAL_ping_xml,ECU_TIMING_INFO);
	}
	fprintf(SIL_XML_out, "\n\n<TIMING_DESCRIPTION_START__%d>", p_g_tracker_OAL_ping_xml.tracker_index);
	unsigned32_T start_time;
	unsigned32_T end_time;
	unsigned32_T core_time;
	unsigned32_T CORE1_time;
	unsigned32_T CORE3_time;
	unsigned32_T Tracker_Timing_info;
	unsigned32_T RECW_Timing_info;
	unsigned32_T CTA_Timing_info;
	unsigned32_T LCDA_Timing_info;
	unsigned32_T TA_Timing_info;
	unsigned32_T SFE_Timing_info;
	unsigned32_T SCW_Timing_info;
	unsigned32_T CED_Timing_info;
	unsigned16_T Tracker_index;
	if (stream_num == 12)
	{
		start_time = Core1_Timimg_info_xml.core_starttime.time_seconds * 1000 + Core1_Timimg_info_xml.core_starttime.time_fraction;
		end_time = Core1_Timimg_info_xml.core_endtime.time_seconds * 1000 + Core1_Timimg_info_xml.core_endtime.time_fraction;
		CORE1_time = end_time - start_time;
	}
	if (stream_num == 13)
	{
		start_time = p_g_timeinfo_xml.timing_info.core_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.core_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.core_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.core_endtime.time_fraction;
		CORE3_time = end_time - start_time;
	}
	//..Tracker_index = p_g_tracker_IAL_ping_xml.tracker_info.tracker_index;
	//tracker time info
	if (stream_num == 12)
	{
		start_time = Core1_Timimg_info_xml.tracker_starttime.time_seconds * 1000 + Core1_Timimg_info_xml.tracker_starttime.time_fraction;
		end_time = Core1_Timimg_info_xml.tracker_endtime.time_seconds * 1000 + Core1_Timimg_info_xml.tracker_endtime.time_fraction;
		Tracker_Timing_info = end_time - start_time;
	}

	//FF time info
	/*RECW Timing Info*/
	if (stream_num == 13)
	{
		start_time = p_g_timeinfo_xml.timing_info.RECW_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.RECW_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.RECW_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.RECW_endtime.time_fraction;
		RECW_Timing_info = end_time - start_time;
		/*CTA Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.CTA_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.CTA_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.CTA_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.CTA_endtime.time_fraction;
		CTA_Timing_info = end_time - start_time;
		/*LCDA Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.LCDA_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.LCDA_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.LCDA_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.LCDA_endtime.time_fraction;
		LCDA_Timing_info = end_time - start_time;
		/*Ta Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.TA_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.TA_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.TA_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.TA_endtime.time_fraction;
		TA_Timing_info = end_time - start_time;
		/*SFE Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.SFE_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.SFE_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.SFE_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.SFE_endtime.time_fraction;
		SFE_Timing_info = end_time - start_time;
		/*SCW Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.SCW_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.SCW_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.SCW_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.SCW_endtime.time_fraction;
		SCW_Timing_info = end_time - start_time;
		/*CED Timing Info*/
		start_time = p_g_timeinfo_xml.timing_info.CED_starttime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.CED_starttime.time_fraction;
		end_time = p_g_timeinfo_xml.timing_info.CED_endtime.time_seconds * 1000 + p_g_timeinfo_xml.timing_info.CED_endtime.time_fraction;
		CED_Timing_info = end_time - start_time;
	}
	if (stream_num == 11)
	{
		fprintf(SIL_XML_out, "\n\n<CORE_TIMING_INFO>");
		//core time info
		start_time = core0_time_xml.core_starttime.time_seconds * 1000 + core0_time_xml.core_starttime.time_fraction;
		end_time = core0_time_xml.core_endtime.time_seconds * 1000 + core0_time_xml.core_endtime.time_fraction;
		core_time = (end_time - start_time);
		fprintf(SIL_XML_out, "\n\t<CORE0_TIMING_INFO>\t%d\t<\CORE0_TIMING_INFO>", core_time);
	}
	if (stream_num == 12)
	{
		fprintf(SIL_XML_out, "\n\t<CORE1_TIMING_INFO>\t%d\t<\CORE1_TIMING_INFO>", CORE1_time);
		fprintf(SIL_XML_out, "\n\t<CORE2_TIMING_INFO>\t%d\t<\CORE2_TIMING_INFO>", 0);//Radar_ECU_CORE2_T structure not available
	}
	if (stream_num == 13)
	{
		fprintf(SIL_XML_out, "\n\t<CORE3_TIMING_INFO>\t%d\t<\CORE3_TIMING_INFO>", CORE3_time);
		fprintf(SIL_XML_out, "\n</CORE_TIMING_INFO>\n");
	}
	//tracker time info
	fprintf(SIL_XML_out, "\n<ALGO_TIMING_INFO>");
	if (stream_num == 12)
	{
		fprintf(SIL_XML_out, "\n\t<TRACKER_TIMING_INFO>\t%d\t<\TRACKER_TIMING_INFO>", Tracker_Timing_info);
		//FF time info
	}
	if (stream_num == 13)
	{
		fprintf(SIL_XML_out, "\n\t<RECW_TIMING_INFO>\t%d\t<\RECW_TIMING_INFO>", RECW_Timing_info);

		fprintf(SIL_XML_out, "\n\t<CTA_TIMING_INFO>\t%d\t<\CTA_TIMING_INFO>", CTA_Timing_info);

		fprintf(SIL_XML_out, "\n\t<LCDA_TIMING_INFO>\t%d\t<\LCDA_TIMING_INFO>", LCDA_Timing_info);

		fprintf(SIL_XML_out, "\n\t<TA_TIMING_INFO>\t%d\t<\TA_TIMING_INFO>", TA_Timing_info);

		fprintf(SIL_XML_out, "\n\t<SFE_TIMING_INFO>\t%d\t<\SFE_TIMING_INFO>", SFE_Timing_info);

		fprintf(SIL_XML_out, "\n\t<SCW_TIMING_INFO>\t%d\t<\SCW_TIMING_INFO>", SCW_Timing_info);

		fprintf(SIL_XML_out, "\n\t<CED_TIMING_INFO>\t%d\t<\CED_TIMING_INFO>", CED_Timing_info);
	}
	fprintf(SIL_XML_out, "\n</ALGO_TIMING_INFO>");

	if (stream_num == 12)
	{
		fprintf(SIL_XML_out, "\n</TIMING_DESCRIPTION_END__%d>", p_g_tracker_OAL_ping_xml.tracker_index);
	}
}

void Record_Header_Count(unsigned8_T Radar_Posn, IRadarStream  *  g_pIRadarStrm)
{
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d_%s>\n", Dets_XML.ScanIndex, GetRadarPosName(Radar_Posn));
}

void VF_Record_Header_Count(IRadarStream  *  g_pIRadarStrm)
{
	//g_pIRadarStrm->getData((unsigned char*)&Dets_XML,TARGET_INFO);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Dets_XML, TARGET_INFO);
	if (Dets_XML.ScanIndex != 0)
	{
		fprintf(SIL_XML_out, "\n</RECORD_HEADER_%d>\n", Dets_XML.ScanIndex);
	}
}

/*===========================================================================================================*\
* File Revision History (top to bottom: first revision to last revision)
*============================================================================================================
*
*   Date        userid      Name        (Description on following lines: Jira/SCR #, etc.)
* ------------------------------------------------------------------------------------------------------------
* 19-Jan-2023   xjjjzg     Ajith       CYW-2730: STLA SRR6P Detections Support added.Tracks Support available in MUDP.
*
*\*==========================================================================================================*/