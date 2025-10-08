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
FileName   : radar_udp_dump.cpp

Author     : Khushnuma Ghazal

Date       : 22 Feb, 2019

Description: struct and functions for extracting the mdf4/dvsu in csv formats


Version No : Date of update:         Updated by:           Changes:

Issues     : None 

******************************************************************************/
#include "stdafx.h"
#include <conio.h>
#include <stdlib.h>
#include <windows.h>
#include "PackUDPLogging.h"
#include <iostream>
#include <iomanip>


#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"
#include "../../../CommonFiles/plugin/PluginLoader.h"
#include "../../../CommonFiles/inc/ByteStream.h"
#include "../../../CommonFiles/inc/common.h"
#include "../../../CoreLibraryProjects/radar_stream_decoder/Calibration/Calib.h"
#include "time.h"
#include "radar_udp_dump_options.h"
#include "../../../CommonFiles/inc/HiLBaseStream.h"

#include "../../../CommonFiles/inc/IRadarStream.h"
#include "../../../CommonFiles/inc/Z_Logging.h"
#include "../../../CoreLibraryProjects/mudp_log/MudpRecord/udp_log_struct.h"
#include"../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config_mode.h"
#include "../../../CommonFiles/udp_headers/rr_cal_log.h"
#include "../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"
#include "../../../CommonFiles/CommonHeaders/dph_rr_adas_config_mode.h"
#include "../../../CommonFiles/helper/genutil.h"
#include "../../../CoreLibraryProjects/mdf_log/inc/apt_mdf_log.h"
#define RADAR_POS 19

using namespace std;

typedef struct UDP_RecordsInfo{
	 uint64_t timestamp;
	 uint32_t streamRefIndex; 
	 uint8_t  Radar_Position;
	 uint8_t  streamNumber; 
	 uint8_t  MaxChunks; 
	 uint8_t  streamChunkIdx; 
}UDP_RecordsInfo_t;

const int MAX_BACK_BUFFER = 100000;
static SRR3_MUDPLOG_Interface_T*    m_pLogPlugin = NULL;
static SRR3_Decoder_Interface_T*    m_pDecoderPlugin = NULL;
static MUDP_CONFIG_T *MUDP_master_config = NULL;
DVSU_RECORD_T recordOut;
IRadarStream * g_pIRadarStrm[MAX_LOGGING_SOURCE] = {0};

Sensor_Version_T m_version_info;
void DumpToOutputDVSU();
int CreateOutputDVSUFile(const char* LogFname);
int ExtractAndConvert(const char* LogFname,DVSU_RECORD_T* pRec);
void*  mudp_log_out = NULL;
static sOutputFileNames_T fNameobj;
ARSTREAMS m_TxQueue;
Stream_Data_Ptr_T*   m_pDestStreams = new Stream_Data_Ptr_T;
Stream_Data_Ptr_T*   m_pSrcStreams = new Stream_Data_Ptr_T;
extern bool bOnce[MAX_RADAR_COUNT] ;
static int once_list[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
#if defined (WIN32) && !defined (__MINGW32__)
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Mdf_Log.lib")

#endif//defined (WIN32) && !defined (__MINGW32__)
#pragma comment(lib,"radar_stream_decoder.lib")
#include <algorithm>
using namespace std;

#define MILLI2MICRO(tm)  (tm*1000)

static int ReadHiLInfrastructure(int argc, char* argv[], SRR3_HiL_Infrastructure_T& hil_param);
void create_ecu_stream_buffers();

static unsigned char g_sensorpos = 21;
static unsigned char release_inf      = 6;   // Modify this for every major change (version 2 for new udp building block)
static unsigned char release_op       = 0;   
static unsigned char release_minor    = 3;   //modify this for every minor change

char  xml_file_path[1024] = { 0 };
static long SCAN_INDEX_COUNT[MAX_RADAR_COUNT] = {0};
static unsigned int ChecksumErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] ={0};
static unsigned int UDPPacketLossErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE]={0};
static unsigned int ScanindexDropCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE]={0};
static unsigned int CRCErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] ={0};
static unsigned int TotalscanIndexDropCount[MAX_RADAR_COUNT]={0};
static unsigned int TotalscanIndex[MAX_RADAR_COUNT]={0};
static int Stream_Index_Prev[MAX_RADAR_COUNT]={-1};
Stream_Data_Ptr_T*   s_pSrcStreams[MAX_RADAR_COUNT];
static unsigned int Scan_Index_Buff[MAX_SRR3_SOURCE][MAX_LOGGING_SOURCE] = {0};
static bool CreateOnce = true;
static bool onesTxCnt = true;
UINT8 setViGEMMf4 = 0; 
static bool F_CDC_DATA = true;
static bool F_Z7a_core = true;
static bool F_Z7b_core = true;
static bool F_z4_core = true;
static bool F_z4_cust = true;
static bool F_z7b_cust = true;
static bool F_Ecu_0 = true;
static bool F_Ecu_1 = true;
static bool F_Ecu_3 = true;
static bool F_Ecu_calib = true;
void* mudp_log_in_ptr = NULL;
//  Stream_Data_Ptr_T*   m_pDestStreams = new Stream_Data_Ptr_T;
//	Stream_Data_Ptr_T*   m_pSrcStreams = new Stream_Data_Ptr_T;
void getDetectionsData(DVSU_RECORD_T* pRec );

typedef enum {
	INVALID_OPT = -1 , 
	UDP_HDR = 0, 
	UDP_CUST_HDR, 
	UDP_Z7B, 
	UDP_Z7A,
	UDP_Z4,
	UDP_Z7B_CUSTOM,
	UDP_Z4_CUSTOM,
	UDP_CDC,
	UDP_Z7A_RDD,
	UDP_CALIB_USC,
	UDP_Z7B_CAF,
	UDP_Z7B_RADAR_INFO,
	UDP_Z7B_VEH,
	UDP_Z7B_DETS,
	UDP_Z7B_TRKS,
	UDP_Z4_OPP_DETS,
	UDP_Z7B_OH,
	UDP_Z7B_FENCES,
	UDP_Z7B_RECW,
	UDP_Z7B_RCTA,
	UDP_Z7B_LCDA,
	UDP_Z7B_CTA,
	UDP_Z7B_PCR,
	UDP_Z7B_AUDI_OH,
	UDP_Z7B_AUDI_FENCES,
	UDP_Z7B_AUDI_DET,
	UDP_Z7B_AUDI_TRK,
	UDP_Z7B_AUDI_EGO,
	UDP_Z7B_MOUNT_POS,
	UDP_Z7B_RDD,
	UDP_Z7B_RADAR_BLOCKAGE,
	UDP_Z7B_RADAR_PARAM,
	UDP_Z4_AUDI_MOUNTING_POS,
	UDP_CONVERT,
	UDP_FF_STATUS,
	UDP_ECU0,
	UDP_ECU1,
	UDP_ECU2,
	UDP_ECU3,
	UDP_Z7B_VEH_UDP,
	INVALID_STREAM
}enDumpOpt;

typedef enum{
	INVALID_OPTION = 0 , 
	HEADER = 1, 
	UDP_CUSTOM_HDR, 
	UDP_Z7B_CORE, 
	UDP_Z7A_CORE,
	UDP_Z4_CORE,
	UDP_CUSTOM_Z7B,
	UDP_CUSTOM_Z4,
	UDP_CDC_STRM,
	UDP_RDR_ECU0,
	UDP_RDR_ECU1,
	UDP_RDR_ECU2,
	UDP_RDR_ECU3,
	INVALID_STRM
}streamopt;
BOOL f_z7b_enabled = 0;
BOOL f_z7a_enabled = 0;
BOOL f_z4_enabled = 0;
BOOL f_cdc_enabled = 0;
BOOL f_z4_cust_enabled = 0;
BOOL f_z7b_custom_enabled = 0;
BOOL f_z4_custom_enabled = 0;
BOOL f_ECU0_enabled = 0;
BOOL f_ECU1_enabled = 0;
BOOL f_ECU3_enabled = 0;

void DumpCalib(FILE* fptr, enDumpOpt opt, const unsigned char* bytestream);

char  inp_file[1024] = {0};
char  out_file[1024] = {0};
unsigned char f_mode = 0; //0 - single file (/f) , 1 = multifile (/flist)
enDumpOpt dump_option = INVALID_OPT; //0 = /hdr
int dump[INVALID_STRM] = {0}; 
Customer_T cust_id = AUDI;
enumRadarDecoderOpt e_Options = ALL;
FILE*  fCreate = NULL ;
FILE*  fpDump = NULL ;
FILE*  fpMF4Dump[MAX_RADAR_COUNT][INVALID_STRM] = {0};
char FileNameRL[MAX_FILEPATH_NAME_LENGTH] ;
char FileNameRR[MAX_FILEPATH_NAME_LENGTH] ;
char  FileNameFR[MAX_FILEPATH_NAME_LENGTH] ;
char  FileNameFL[MAX_FILEPATH_NAME_LENGTH] ;
char  FileName_Pos[MAX_FILEPATH_NAME_LENGTH] ;
char  FileCreate_Pos[MAX_FILEPATH_NAME_LENGTH] ;
static apt_mdfFile*    g_mdfUDP = NULL;
static Radar_UDP_Frame_Latch_T pack_working_prev[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA];
static UDP_RecordsInfo_t udp_records_prev[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE];
static long CDC_ScanIdCurrent[MAX_RADAR_COUNT] [SRR3_Z7B_CUST_LOGGING_DATA]= {0};
static long CDC_scan_idx_previous[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA] = {0};
static long scan_idx_current[MAX_RADAR_COUNT] = {0};
static long first_scan_idx[MAX_RADAR_COUNT] = {0};
static long scan_idx_previous[MAX_RADAR_COUNT] = {-1};
static long scan_idx_Z_previous[MAX_RADAR_COUNT] = {-1};
float  Percentage_of_scan_idx_Drop[MAX_RADAR_COUNT] = {0};
static int once[MAX_RADAR_COUNT] = {0}; 
static int ScanIdCount[MAX_RADAR_COUNT] = {0};
static int ScanIdDropCount[MAX_RADAR_COUNT] = {0};
static bool printStrmHeaderOnes = true;
static bool printOnesline = true;
//unsigned char g_RacamUdpBuffer[RACAMUDP_MAX_ST][RACAM_UDP_RECORD_SIZE] ; // fix size for now //Review:;
unsigned char g_RacamUdpBuffer[RACAM_UDP_RECORD_SIZE] ; // fix size for now //Review:;
static int ProcessedChunks[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
static int ExtractAndListDVSU(FILE*pOutFile ,int Dump_Option,UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* pRec , bool mf4flag );
static int ExtractAndListMUDP(FILE*pOutFile , enDumpOpt opt, UDPLogStruct* pLogStruct );

static char* GetCustIDName(uint8_t custID){
	static char customerName[40] = {0};

	switch(custID){
	case RNA_SUV: sprintf(customerName, "RNA_SUV"); break;
	case RNA_CDV: sprintf(customerName, "RNA_CDV"); break;
	case HKMC_SRR5: sprintf(customerName, "HKMC_SRR5"); break;
	case GWM_SRR5: sprintf(customerName, "GWM_SRR5"); break;
	case FORD_SRR5: sprintf(customerName, "FORD_SRR5"); break;
	case GEELY_SRR5: sprintf(customerName, "GEELY_SRR5"); break; 
	case BMW_LOW: sprintf(customerName, "BMW_LOW"); break;
	case BMW_SAT: sprintf(customerName, "BMW_SAT"); break;
	case BMW_BPIL: sprintf(customerName, "BMW_BPIL"); break;
	default: sprintf(customerName, "INVALID_CUSTOMER"); break;
	}
	return customerName;
}   
char * GetStreamNumberName(uint8_t Stream_Num){
	static char	Str_Name[40] = {0};
	switch(Stream_Num){
	case	CDC_8_IQ : 
	case	CDC_DATA :
		sprintf(Str_Name, "CDC_DATA"); break;
	case   Z7A_Z7B_IPC_SRR5_116_CORE : 
	case Z7A_Z7B_IPC_SRR5_210_CORE : 
	case Z7A_Z7B_IPC_MRR360_116RBIN_64D_CORE : 
	case Z7A_Z7B_IPC_MRR360_116RBIN_200D_CORE: 
	case Z7A_Z7B_IPC_MRR360_116RBIN_150D_CORE: 
	case Z7A_SRR5P_116RB_64D_CORE : 
	case Z7A_SRR5P_116RB_150D_CORE: 
	case Z7A_SRR5P_116RB_200D_CORE: 
	case Z7A_LOGGING_DATA:
		sprintf(Str_Name, "Z7A_LOGGING_DATA"); break;
	case Z7B_SIDE_64D_SAT_CORE  : 
	case Z7B_SIDE_200D_SAT_CORE : 
	case Z7B_FRNT_128D_SAT_CORE : 
	case Z7B_64SD_64FD_64TRK_STAND_CORE : 
	case Z7B_64SD_64TRK_STAND_CORE : 
	case Z7B_BPIL_150D_64T_CORE : 
	case Z7B_LOGGING_DATA:
		sprintf(Str_Name, "Z7B_LOGGING_DATA"); break;
	case Z4_LOGGING_DATA:
	case Z4_Z7B_CORE : 
	case Z4_Z7B_64_FD_CORE : 
		sprintf(Str_Name, "Z4_LOGGING_DATA"); break;
	case SRR3_Z7B_CUST_LOGGING_DATA:
	case Z7B_CUST : sprintf(Str_Name, "Z7B_CUST_LOGGING_DATA"); break;
	case Z4_CUST_LOGGING_DATA:
	case Z4_CUST : 
		sprintf(Str_Name, "Z4_CUST_LOGGING_DATA"); break;
	case ECU_CORE0 : sprintf(Str_Name, "ECU_CORE0"); break;
	case ECU_CORE1 : sprintf(Str_Name, "ECU_CORE1"); break;
	case ECU_CORE3 : sprintf(Str_Name, "ECU_CORE3"); break;
	case ECU_OG : sprintf(Str_Name, "ECU_OG"); break;
	case ECU_CAL_STREAM : sprintf(Str_Name, "ECU_CAL_STREAM"); break;
	case ECU_INTERNAL_CORE1 : sprintf(Str_Name, "F360_TRACKER"); break;
	case GDSR_TRACKER_INTERNAL : sprintf(Str_Name, "GDSR_TRACKER"); break;
	default : sprintf(Str_Name, "INVALID_STREAM"); break;
	}
	return Str_Name;
	}

char* GetStreamName(unsigned int SteamNo){
	static char StreamName[40] = {0};
	switch(SteamNo){
	case Z7A_LOGGING_DATA: sprintf(StreamName, "Z7A_LOGGING_DATA"); break;
	case Z7B_LOGGING_DATA: sprintf(StreamName, "Z7B_LOGGING_DATA"); break;
	case Z4_LOGGING_DATA: sprintf(StreamName, "Z4_LOGGING_DATA"); break;
	case CDC_DATA: sprintf(StreamName, "CDC_DATA"); break;
	case CALIBRATION_DATA: sprintf(StreamName, "CALIB_STREAM"); break;
	case Z7B_CUST_TRACKER_DATA: sprintf(StreamName, "Z7B_CUST_TRACKER_DATA"); break;
	case SRR3_Z7B_CUST_LOGGING_DATA: sprintf(StreamName, "Z7B_CUST_LOGGING_DATA"); break;
	case Z4_CUST_LOGGING_DATA: sprintf(StreamName, "Z4_CUST_LOGGING_DATA"); break;
	case DSPACE_CUSTOMER_DATA:sprintf(StreamName, "DSPACE_CUSTOMER_DATA"); break;
	case RADAR_ECU_CORE_0: sprintf(StreamName, "ECU_CORE_0"); break;
	case RADAR_ECU_CORE_1:sprintf(StreamName, "ECU_CORE_1"); break;
	case RADAR_ECU_CORE_3:sprintf(StreamName, "ECU_CORE_3"); break;
	case RADAR_ECU_OG: sprintf(StreamName, "RADAR_ECU_OG"); break;
	case RADAR_ECU_CALIB: sprintf(StreamName, "RADAR_ECU_CALIB"); break;
	case RADAR_ECU_INTERNAL_CORE1: sprintf(StreamName, "F360_INTERNAL_CORE1"); break;
	case SENSOR_GDSR_TRACKER_INTERNAL: sprintf(StreamName, "GDSR_TRACKER"); break;
	default : sprintf(StreamName, "INVALID_STREAM"); break;
	}
	return StreamName;
} 
static int MaptoEcustreamNumber(uint8_t Strm)
{
	int retStream = 0xFF;
	switch(Strm)
	{
	case ECU_CORE0:			retStream = RADAR_ECU_CORE_0;	break;
	case ECU_CORE1:			retStream = RADAR_ECU_CORE_1;	break;
	case ECU_CORE3:			retStream = RADAR_ECU_CORE_3;	break;
	case ECU_OG:			retStream = RADAR_ECU_OG;		break;
	case ECU_CAL_STREAM:	retStream = RADAR_ECU_CALIB;	break;
	case ECU_INTERNAL_CORE1:retStream = RADAR_ECU_INTERNAL_CORE1;	break;
	default:												break;
	} 
	return retStream;
}
static int MaptostreamNumber(uint8_t Strm, uint8_t CustID)
{
	int retStream = 0xFF;
	switch(CustID)
	{
	case (RNA_CDV):
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_210_CORE:	retStream = Z7A_LOGGING_DATA;break;
		case Z7B_64SD_64TRK_STAND_CORE:	retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_64_FD_CORE:			retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:					retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	case (RNA_SUV):
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_210_CORE:	retStream = Z7A_LOGGING_DATA;break;
		case Z7B_64SD_64TRK_STAND_CORE:	retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_64_FD_CORE:			retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:					retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	case HKMC_SRR5:
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:		retStream = Z7A_LOGGING_DATA;break;
		case Z7B_64SD_64FD_64TRK_STAND_CORE:retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:					retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:						retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:						retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:						retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	case BMW_LOW:
		switch(Strm)
		{
		case Z7A_SRR5P_116RB_64D_CORE:retStream = Z7A_LOGGING_DATA;break;
		case Z7B_64SD_64FD_64TRK_STAND_CORE:retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:retStream =  Z4_LOGGING_DATA;break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		case CDC_8_IQ:retStream = CDC_DATA;break;
		default: break;
		}
		break;
	case BMW_SAT:
		switch(Strm)
		{
		case Z7A_SRR5P_116RB_200D_CORE:retStream = Z7A_LOGGING_DATA;break;
		case Z7B_SIDE_200D_SAT_CORE:retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:retStream =  Z4_LOGGING_DATA;break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		case CDC_8_IQ:retStream = CDC_DATA;break;
		default: break;
		}
		break;

	case BMW_BPIL:
		switch(Strm)
		{
		case Z7A_SRR5P_116RB_150D_CORE:retStream = Z7A_LOGGING_DATA;break;
		case Z7B_BPIL_150D_64T_CORE:retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:retStream =  Z4_LOGGING_DATA;break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		case CDC_8_IQ:retStream = CDC_DATA;break;
		default: break;
		}
		break;
	case GWM_SRR5:
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:		retStream = Z7A_LOGGING_DATA;break;
		case Z7B_64SD_64FD_64TRK_STAND_CORE:retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:					retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:						retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:						retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:						retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	case GEELY_SRR5:
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA;break;
		case Z7B_FRNT_128D_SAT_CORE:	retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:					retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	case FORD_SRR5:
		switch(Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA;break;
		case Z7B_SIDE_64D_SAT_CORE:		retStream = Z7B_LOGGING_DATA;break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA;break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA;break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA;break;
		case CDC_8_IQ:					retStream = CDC_DATA;break;
		case CALIBRATION_DATA :			retStream = CALIBRATION_DATA;break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL;break;
		default: break;
		}
		break;
	default: retStream = 0xFF; break;
	}
	return retStream;
}

static char* GetstreamIDName(int streamId){
	static char StreamName[40] = {0};

	switch(streamId){
	case UDP_Z7B_CORE: sprintf(StreamName, "UDP_Z7B"); break;
	case HEADER: sprintf(StreamName, "UDP_HDR"); break;
	case UDP_Z7A_CORE: sprintf(StreamName, "UDP_Z7A"); break;
	case UDP_Z4_CORE: sprintf(StreamName, "UDP_Z4"); break;
	case UDP_CDC_STRM: sprintf(StreamName, "UDP_CDC"); break;
	case UDP_CUSTOM_Z7B: sprintf(StreamName, "UDP_Z7B_CUSTOM"); break;
	case UDP_CUSTOM_Z4: sprintf(StreamName, "UDP_Z4_CUSTOM"); break; 
	case UDP_RDR_ECU0: sprintf(StreamName, "UDP_ECU0"); break;
	case UDP_RDR_ECU1: sprintf(StreamName, "UDP_ECU1"); break;
	case UDP_RDR_ECU3: sprintf(StreamName, "UDP_ECU3"); break;
	default: sprintf(StreamName, "INVALID_STREAM"); break;

	}
	return StreamName;
}   


static void FreePlugin()
{

	if(m_pLogPlugin->hModule)
	{
		::FreeLibrary(m_pLogPlugin->hModule);
		memset(&m_pLogPlugin,0, sizeof(m_pLogPlugin));
	}

	if(m_pDecoderPlugin->hModule)
	{
		::FreeLibrary(m_pDecoderPlugin->hModule);
		memset(&m_pDecoderPlugin,0, sizeof(m_pDecoderPlugin));
	}
}
static bool TestAndFixEndianness(UDPRecord_Header* pmet) 
{
	bool must_swap = false;

	if ( byteswap(pmet->versionInfo) == UDP_RECORD_VERSIONINFO ) 
	{
		// swap to host endianness
		must_swap = true;
		pmet->versionInfo    = byteswap(pmet->versionInfo);
		pmet->sourceTxCnt    = byteswap(pmet->sourceTxCnt); 
		pmet->sourceTxTime   = byteswap(pmet->sourceTxTime);
		pmet->streamRefIndex = byteswap(pmet->streamRefIndex);
		pmet->streamDataLen  = byteswap(pmet->streamDataLen);
	}
	else
	{
		// already correct, or header error
		must_swap = false;
	}
	return must_swap;
}
void usage(void)
{
	printf("\n"
		"----------------------------------------------------------------------------------------------------------------\n"
		"USAGE:\n"
		"MUDP_Log_DataExtracter.exe MUDP_DATA_Extracter.xml f_logs.txt\n"
		"f_logs contains input log argument listed as column [Input Log argument](.dvsu)/(.dvl) with no delimiter\n"
		"----------------------------------------------------------------------------------------------------------------\n"
		);
}

int ReadArguments(int argc, char* argv[])
{

	int count = 1;

	if (argc < 2)
	{
		usage();
		return -1;
	}

	if (argc == 3)
	{
		do
		{
			if (argc > count)
			{
				strcpy(xml_file_path, argv[count]);
			}
			++count;

			if (argc > count)
			{
				strcpy(inp_file, argv[count]);

			}
			++count;

		} while (0);
	}
	else
	{
		usage();
		return -1;
	}

	return 0;
}

void CheckSameScanIdforChunk(UDPRecord_Header* p_udp_hdr, UINT64 timestamp_ns)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned int Current_StreamIndex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
	static bool f_block_firstCheck[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
	
	if(p_udp_hdr->Platform >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position -1;
		Strm_no =	MaptostreamNumber(p_udp_hdr->streamNumber,p_udp_hdr->customerID);
	}
	else{
		Source_Id = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no =	p_udp_hdr->streamNumber;

	} 
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
	}
	else{
		if(Source_Id == RADAR_POS)
			return ;
	}
	Current_StreamIndex[Source_Id][Strm_no] = p_udp_hdr->streamRefIndex;
	
	if(f_block_firstCheck[Source_Id][Strm_no] == 1 && MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1 ){
		if(udp_records_prev[Source_Id][Strm_no].Radar_Position == Source_Id){
			if(udp_records_prev[Source_Id][Strm_no].streamNumber == Strm_no && p_udp_hdr->streamChunkIdx == udp_records_prev[Source_Id][Strm_no].streamChunkIdx +1){
				if(Strm_no != CDC_DATA){
					if(p_udp_hdr->streamChunkIdx >= 0 && p_udp_hdr->streamChunkIdx <= p_udp_hdr->streamChunks )
					{
						if(udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no]){
							fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
							fprintf(fCreate,"[%s] :Stream index changed for the [%s] at chunkID:%u, \t ",GetRadarPosName(Source_Id), GetStreamName(Strm_no),p_udp_hdr->streamChunkIdx);
							fprintf(fCreate,"expected stream index_%u, received stream index_%u, \n ",udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_udp_hdr->streamRefIndex);
						}
						/*if(udp_records_prev[Source_Id][Strm_no].timestamp != timestamp_ns){

						fprintf(fCreate,"***************************************************************************\n");
						fprintf(fCreate,"[%s] :TimeStamp for the [%s] at chunkID:%u, streamRefid_%u \t ",GetRadarPosName(Source_Id), GetStreamName(Strm_no),p_udp_hdr->streamChunkIdx,p_udp_hdr->streamRefIndex);
						fprintf(fCreate,"expected timestamp_%lld, received timestamp_%lld, \n ",udp_records_prev[Source_Id][Strm_no].timestamp,timestamp_ns);
						}*/
					}
				}
				else{
					if(udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no]){
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						fprintf(fCreate,"[%s] :Stream index changed for the [%s] at chunkID:%u, \t ",GetRadarPosName(Source_Id), GetStreamName(Strm_no),p_udp_hdr->streamChunkIdx);
						fprintf(fCreate,"expected stream index_%u, received stream index_%u, \n ",udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_udp_hdr->streamRefIndex);
					}
				}
			}
		}
	}
	//UDP_RecordsInfo_t
	udp_records_prev[Source_Id][Strm_no].streamRefIndex = p_udp_hdr->streamRefIndex;
	udp_records_prev[Source_Id][Strm_no].MaxChunks = p_udp_hdr->streamChunks;
	udp_records_prev[Source_Id][Strm_no].streamChunkIdx = p_udp_hdr->streamChunkIdx;
	udp_records_prev[Source_Id][Strm_no].streamNumber = Strm_no;
	udp_records_prev[Source_Id][Strm_no].Radar_Position = Source_Id;
	udp_records_prev[Source_Id][Strm_no].timestamp = timestamp_ns;
	f_block_firstCheck[Source_Id][Strm_no] = 1;

}

void get_Source_TxnCnt(UDPRecord_Header* p_udp_hdr)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned short Current_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
	static unsigned short Prev_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
	static int TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0};
	
	if(p_udp_hdr->Platform >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position -1;
		Strm_no =	MaptostreamNumber(p_udp_hdr->streamNumber,p_udp_hdr->customerID);
	}
	else{
		Source_Id = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no =	p_udp_hdr->streamNumber;
	} 
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
	}
	else{
		if(Source_Id == RADAR_POS)
			return ;
	}
	if(onesTxCnt)
	{
		for(int i =0; i < MAX_RADAR_COUNT; i++)
		{
			for(int j =0; j < MAX_LOGGING_SOURCE; j++)
			{
				TxnCnt[i][j] = -1;
				Prev_TxnCnt[i][j] = 0;
			}
		}
		onesTxCnt = false;
	}

	Current_TxnCnt[Source_Id][Strm_no] = p_udp_hdr->streamTxCnt;
	if(MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1 ){
		if(TxnCnt[Source_Id][Strm_no] != -1 && TxnCnt[Source_Id][Strm_no] != 256)
		{
			if(Current_TxnCnt[Source_Id][Strm_no] != TxnCnt[Source_Id][Strm_no])
			{
				fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
				fprintf(fCreate,"[%s],scanindex_%u,[%s] : expected_strm_txn_count[%d] and received_strm_txn_count[%d]\n ",GetRadarPosName(Source_Id), p_udp_hdr->streamRefIndex,GetStreamName(Strm_no), TxnCnt[Source_Id][Strm_no],Current_TxnCnt[Source_Id][Strm_no]);
			}
		}
	}
	
	Prev_TxnCnt[Source_Id][Strm_no] = Current_TxnCnt[Source_Id][Strm_no];
	TxnCnt[Source_Id][Strm_no] = (Prev_TxnCnt[Source_Id][Strm_no])+1;
}
void get_scanindex_info( unsigned int ScanIdCurrent, unsigned char radarPos, unsigned char StreamNo)
{
	bool firstblock[MAX_RADAR_COUNT] = {0};
	static int f_Firstscan[MAX_RADAR_COUNT] = {0};
	unsigned8_T f_cdc_Cmplt[MAX_RADAR_COUNT] = {0};
	unsigned8_T f_Z_Cmplt[MAX_RADAR_COUNT] = {0};
	unsigned int scanIndexDropCount[MAX_RADAR_COUNT]={0};

	if(!f_Firstscan[radarPos]){
		first_scan_idx[radarPos] = ScanIdCurrent; 
		f_Firstscan[radarPos] = true;
		firstblock[radarPos] = true;
	}
	scan_idx_current[radarPos] = ScanIdCurrent;
	if(StreamNo != CDC_DATA){
		if((scan_idx_Z_previous[radarPos] != scan_idx_current[radarPos] ) && (firstblock[radarPos] != 1)){
			f_Z_Cmplt[radarPos] = 1;
			ScanIdCount[radarPos]++;
		}
	}else{
		if(scan_idx_previous[radarPos] != scan_idx_current[radarPos] && (firstblock[radarPos] != 1)){
			f_cdc_Cmplt[radarPos] = 1;
		}
	}
	if(MUDP_master_config->MUDP_Select_Sensor_Status[radarPos] == 1 ){
		if(f_Z_Cmplt[radarPos] == 1 && scan_idx_Z_previous[radarPos] != 0){
			if((scan_idx_current[radarPos] != scan_idx_Z_previous[radarPos]) && ( scan_idx_Z_previous[radarPos] != 0))
			{
				scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos]);  
				if (scanIndexDropCount[radarPos] > 2)
				{
					ScanIdDropCount[radarPos]+= scanIndexDropCount[radarPos];
					TotalscanIndexDropCount[radarPos]+= scanIndexDropCount[radarPos];
					fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
					fprintf(fCreate,"[%s]: ScanIndex Drop count[%d] observed at the current ScanIndex_%u\n",GetRadarPosName(radarPos),scanIndexDropCount[radarPos],scan_idx_current[radarPos]);
				}
				if (scanIndexDropCount[radarPos] != 1 && scanIndexDropCount[radarPos] != 0){
					if(scan_idx_current[radarPos] == 0 && scan_idx_Z_previous[radarPos] != 0){
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						fprintf(fCreate,"[%s] : Reset happend at the scanindex_[%d], previous scanindex_[%d]\n", GetRadarPosName(radarPos),scan_idx_current[radarPos],scan_idx_Z_previous[radarPos] );
					}
				}
			}
		}
		if(f_cdc_Cmplt[radarPos] == 1 && scan_idx_previous[radarPos] != -1){
			if((scan_idx_current[radarPos] != scan_idx_previous[radarPos]) && ( scan_idx_previous[radarPos] != -1))
			{
				scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_previous[radarPos]);  
				if (scanIndexDropCount[radarPos] > 2)
				{
					ScanIdDropCount[radarPos]++;
					//TotalscanIndexDropCount[radarPos]++;
					//fprintf(fCreate,"[%s]: ScanIndex Drop count[%d] observed at the current ScanIndex_%u for CDC\n",GetRadarPosName(radarPos),ScanIdDropCount[radarPos],scan_idx_current[radarPos]);
				}
				if (scanIndexDropCount[radarPos] != 1 && scanIndexDropCount[radarPos] != 0){
					if(scan_idx_current[radarPos] == 0 && scan_idx_Z_previous[radarPos] != 0){
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						fprintf(fCreate,"[%s] : Reset happend at the scanindex_[%d], previous scanindex_[%d]\n", GetRadarPosName(radarPos),scan_idx_current[radarPos],scan_idx_Z_previous[radarPos] );
					}
				}
			}
		}
	}
	scanIndexDropCount[radarPos] = 0;
	if(StreamNo != CDC_DATA){
		scan_idx_Z_previous[radarPos] = ScanIdCurrent;
	}else{
		scan_idx_previous[radarPos] = ScanIdCurrent;
	}
	firstblock[radarPos] = false;
}

void CDC_PacketLossCount( Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type){
	int Radar_Pos = radar_type;
	CDC_ScanIdCurrent[Radar_Pos][CDC_DATA]  = p_pack->proc_info.frame_header.streamRefIndex;
	if(MUDP_master_config->MUDP_Select_Sensor_Status[Radar_Pos] == 1 ){
		if(first_scan_idx[radar_type]+1 != p_pack->proc_info.frame_header.streamRefIndex)
		{
			if((p_pack->proc_info.frame_header.streamVersion >= 4)){
				if(CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] == CDC_scan_idx_previous[Radar_Pos][CDC_DATA]+1)
				{
					if(pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks != p_pack->proc_info.frame_header.streamChunks)
					{
						UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
						ScanindexDropCount[radar_type][CDC_DATA]++;
						TotalscanIndexDropCount[radar_type]++;
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						fprintf(fCreate,"[%s]:ScanIndex_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d\n",GetRadarPosName(radar_type),p_pack->proc_info.frame_header.streamRefIndex,p_pack->proc_info.frame_header.streamChunks,pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks);
					}
				}
			}
			else{
				if(p_pack->proc_info.f_failure_block == 1){
					UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
					ScanindexDropCount[radar_type][CDC_DATA]++;
					TotalscanIndexDropCount[radar_type]++;
				}
			}
		}
	}
	CDC_scan_idx_previous[Radar_Pos][CDC_DATA] = CDC_ScanIdCurrent[Radar_Pos][CDC_DATA];
	pack_working_prev[Radar_Pos][CDC_DATA].proc_info = p_pack->proc_info;
}

void Get_PacketLossCount( Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type, unsigned8_T stream_no){
	unsigned int MapStreamNo = 0xFF;
	bool fSCANIdCnt = 0;
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		MapStreamNo = MaptoEcustreamNumber(p_pack->proc_info.frame_header.streamNumber);
	}
	else{
		MapStreamNo = MaptostreamNumber(p_pack->proc_info.frame_header.streamNumber, p_pack->proc_info.frame_header.customerID);
	}
	if(MUDP_master_config->MUDP_Select_Sensor_Status[radar_type] == 1 ){
		if(MapStreamNo == stream_no){
			if(first_scan_idx[radar_type] != p_pack->proc_info.frame_header.streamRefIndex)
			{
				if(stream_no != CDC_DATA){
					if(p_pack->proc_info.xsum_status == XSUM_INVALID){
						ChecksumErrorCount[radar_type][stream_no]++;
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						fprintf(fCreate," [%s] ChecksumError observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type),GetStreamName(stream_no),p_pack->proc_info.frame_header.streamRefIndex);
						fSCANIdCnt = 1;
					}	
					if(p_pack->proc_info.nProcessed_chks != p_pack->proc_info.frame_header.streamChunks){
						int DiffChunks = (unsigned int)(p_pack->proc_info.frame_header.streamChunks - p_pack->proc_info.nProcessed_chks);
						DiffChunks = abs(DiffChunks);
						if(DiffChunks !=  0){
							fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
							UDPPacketLossErrorCount[radar_type][stream_no]+= DiffChunks;
							fprintf(fCreate," [%s] chunks lost count:%d failure for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type),DiffChunks,GetStreamName(stream_no),p_pack->proc_info.frame_header.streamRefIndex);
							fSCANIdCnt = 1;
						}
					}
					if(p_pack->proc_info.crc_status == CRC_INVALID ){
						fprintf(fCreate,"----------------------------------------------------------------------------------------------------------------\n");
						CRCErrorCount[radar_type][stream_no]++;
						fprintf(fCreate," [%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type),GetStreamName(stream_no),p_pack->proc_info.frame_header.streamRefIndex);
						fSCANIdCnt = 1;
					}
					if(fSCANIdCnt == 1){
						if( Stream_Index_Prev[radar_type] != -1 && p_pack->proc_info.frame_header.streamRefIndex > Stream_Index_Prev[radar_type])
						{
							TotalscanIndexDropCount[radar_type]++;
						}
						ScanindexDropCount[radar_type][stream_no]++;
					}
				}
			}
		}
	}
	Stream_Index_Prev[radar_type] = p_pack->proc_info.frame_header.streamRefIndex ;
}

void ResetScanIndexpacketCounts(){
	for(int pos= 0; pos < MAX_RADAR_COUNT; pos++){
		for(int i = 0; i < MAX_LOGGING_SOURCE; i++){
			UDPPacketLossErrorCount[pos][i] = 0;
			ChecksumErrorCount[pos][i] = 0;
			ScanindexDropCount[pos][i] = 0;
		}

		TotalscanIndexDropCount[pos] = 0;
		TotalscanIndex[pos] = 0;
		ScanIdDropCount[pos] = 0;
		scan_idx_Z_previous[pos] = 0;
		scan_idx_previous[pos] = 0;
		scan_idx_current[pos] = 0;
	}
	 F_CDC_DATA = true;
	 F_Z7a_core = true;
	 F_Z7b_core = true;
	 F_z4_core = true;
	 F_z4_cust = true;
	 F_z7b_cust = true;
	 F_Ecu_0 = true;
	 F_Ecu_1 = true;
	 F_Ecu_3 = true;
	 F_Ecu_calib = true;
	 printStrmHeaderOnes = true;
	 printOnesline = true;
	 onesTxCnt = true;
}

void TotalScanIndexerrorFile(FILE* Sfp){
	int radarCnt = 0;
	float Total_percent_failure ={0.0};
	unsigned int TotalScanIDCnt = 0;
	unsigned int TotalscanIndexDrop = 0;
	bool printFLAG = 0;
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		radarCnt = MAX_RADAR_COUNT;
	}
	if(MUDP_master_config->Bpillar_status == 1){
		radarCnt = MUDP_MAX_STATUS_COUNT;	
	}	
	else{
		radarCnt = MAX_SRR3_SOURCE;
	}

	for(int pos= 0; pos < radarCnt; pos++){
		if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
			if(pos != RDR_POS_ECU){
				continue;
			}
		}
		else if(MUDP_master_config->MUDP_Select_Sensor_Status[pos] != 1){
			continue;
		}
		if(ScanIdCount[pos] != 0){
			TotalScanIDCnt = ScanIdCount[pos] + TotalScanIDCnt;
			TotalscanIndexDrop = TotalscanIndexDropCount[pos] + TotalscanIndexDrop+ ScanIdDropCount[pos];
			printFLAG = true;
			
		}
	}
	if(printFLAG){
		fprintf(Sfp, "*******************************************************************************************\n");
		fprintf(Sfp, "Total Scan Index of the log = %u\n",TotalScanIDCnt);
		fprintf(Sfp, "Total Scans of Resim failure of the log = %u\n", TotalscanIndexDrop);
		Total_percent_failure = ((float)TotalscanIndexDrop/(float)TotalScanIDCnt);
		Total_percent_failure = (100 * Total_percent_failure);
		fprintf(Sfp, "Total percentage of Resim fail of the log : %f\n", Total_percent_failure);
		fprintf(Sfp, "*******************************************************************************************\n");
	}
}

void CloseScanIndexerrorFile(FILE* Sfp, const char* LogFname){
	int Idx = 0;
	float Core_percent_failure[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0.0};
	float per_dividend[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = {0.0};
	float percent_failure[MAX_RADAR_COUNT] ={0.0};
	unsigned int count = 0;
	int radarCnt = 0;
	int streamNo = 0;
	
	int MAX_streamNo = DSPACE_CUSTOMER_DATA;
	if(Sfp==NULL) {	
		perror("Error in opening Error_Log file...\n");		
	}
	else {
		time_t now = time(NULL);
		struct tm *t = localtime(&now);
		fprintf(Sfp, "\n");
		fprintf(Sfp, "*******************************************************************************************\n");
		fprintf(Sfp, "Below shows the Statistics of ScanIndexloss/packetloss/Xsum failures of the\n");
		fprintf(Sfp, "log:%s\n", LogFname);
	}
	if(MUDP_master_config->Bpillar_status == 1){
		radarCnt = MUDP_MAX_STATUS_COUNT;	
	}
	else if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		radarCnt = MAX_RADAR_COUNT;
		streamNo = RADAR_ECU_CORE_0;
		MAX_streamNo = RADAR_ECU_CALIB + 1;
	}	
	else{
		radarCnt = MAX_SRR3_SOURCE;
	}

	if(Sfp){
		TotalScanIndexerrorFile(Sfp);
		for(int pos= 0; pos < radarCnt; pos++){
			if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
				if(pos != RDR_POS_ECU){
					continue;
				}
			}
			else if(MUDP_master_config->MUDP_Select_Sensor_Status[pos] != 1){
				continue;
			}
			fprintf(Sfp, "PACKET LOSS INFO FOR[%s] :\n", GetRadarPosName(pos));
			fprintf(Sfp, "Total Scan Index for [%s] = %u\n",GetRadarPosName(pos),ScanIdCount[pos]);
			fprintf(Sfp, "Total Missing Scan Index for [%s] = %u\n",GetRadarPosName(pos),ScanIdDropCount[pos]);
			if(ScanIdCount[pos] != 0){
				for(int i = streamNo; i < MAX_streamNo; i++){

					if( i == Z7B_LOGGING_DATA_DUMMY || i == CALIBRATION_DATA || i == DEBUG_DATA || i == Z7B_CUST_TRACKER_DATA || i == RADAR_ECU_OG) 
					{
						continue;
					}

					if(ChecksumErrorCount[pos][i] >= UDPPacketLossErrorCount[pos][i])
					{
						ScanindexDropCount[pos][i] = ChecksumErrorCount[pos][i];
					}
					else if(ChecksumErrorCount[pos][i] <= UDPPacketLossErrorCount[pos][i]){
						ScanindexDropCount[pos][i] = UDPPacketLossErrorCount[pos][i];
					}
					if(ScanindexDropCount[pos][i] > ScanIdCount[pos]){
						int offset = ScanindexDropCount[pos][i] - ScanIdCount[pos];
						ScanIdCount[pos] = ScanIdCount[pos]+offset;
					}
					//string strm = GetStreamName(i);
					per_dividend[pos][i] = ((float)ScanindexDropCount[pos][i]/(float)ScanIdCount[pos]);
					Core_percent_failure[pos][i]  = (100.00 * per_dividend[pos][i]);
					fprintf(Sfp, "[%s]- %s, chunks lost = %u,  Xsum failure = %u, CRC_failure:%u Scan Index Drops = %u, percentage of failure = %f\n",GetRadarPosName(pos),GetStreamName(i),UDPPacketLossErrorCount[pos][i],
					ChecksumErrorCount[pos][i], CRCErrorCount[pos][i],ScanindexDropCount[pos][i],Core_percent_failure[pos][i]);

					UDPPacketLossErrorCount[pos][i] = 0;
					ChecksumErrorCount[pos][i] = 0;
					ScanindexDropCount[pos][i] = 0;
					CRCErrorCount[pos][i] = 0;
				}
				percent_failure[pos] = ((float)TotalscanIndexDropCount[pos]/(float)ScanIdCount[pos]);
				percent_failure[pos] = (100 * percent_failure[pos]);
				fprintf(Sfp, "Total Scans of Resim failure for [%s]= %u\n", GetRadarPosName(pos),TotalscanIndexDropCount[pos]);
				fprintf(Sfp, "Total percentage of Resim fail  [%s]: %f\n", GetRadarPosName(pos),percent_failure[pos]);
				fprintf(Sfp, " \n");
			}
			else{
				fprintf(Sfp, "Total Scan Index of the log present in this log are zero Hence No satistics can be printed\n");
			    fprintf(Sfp, " \n");
			}
			
			TotalscanIndexDropCount[pos] = 0;
			ScanIdCount[pos] = 0;
			ScanIdDropCount[pos] = 0;
		}
	}

	if(Sfp!=NULL) {
		fprintf(Sfp, "*******************************************************************************************\n");
		fclose(Sfp);
		Sfp = NULL;
	}
}
int CreateOutputCSVFile(char* inputLogFname, Customer_T cust_Id)
{
	int ret=0;
	char  logname[_MAX_PATH] = { 0 };
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
	char * FileName[20];
	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = {0};
	
	
	if(cust_id == BMW_BPIL){
		if(MUDP_master_config->data_extracter_mode == 1){
			if(MUDP_master_config->Bpillar_status != 1 && MUDP_master_config->MUDP_Radar_ECU_status == 1){
				printf("Please Enable BPIL radar, since the BPIL log is provided\n");
				printf("\rExiting the resimulation.\n\n\r");
				exit(0);
			}
		}
	}
	
	else if(cust_id == BMW_SAT){
		if(MUDP_master_config->Bpillar_status == 1){

			//*******DO NOTHING*********//
			/*printf("Please do not Enable BPIL radar, since the BMW_MID input log is provided\n");
			printf("\rExiting the resimulation.\n\n\r");
			exit(0);*/
		}
	}
	else{
		if(MUDP_master_config->Bpillar_status == 1){
			printf("Please do not Enable BPIL radar, since the %s input log is provided\n", GetCustIDName(cust_id));
			printf("\rExiting the resimulation.\n\n\r");
			exit(0);
		}
		if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
			printf("Please do not Enable Radar ECU status, since the %s input log is provided\n", GetCustIDName(cust_id));
			printf("\rExiting the resimulation.\n\n\r");
			exit(0);
		}
	}
	
	if(MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE)
	{
		if((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE")==0) || (strcmp(MUDP_master_config->MUDP_output_folder_path,"")==0) ){
			if(once)
				fprintf(stderr," \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n",inputLogFname);
			once = false ;
		}
		else{
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if(st_Len != 0)
			{
				if(newpath[st_Len - 1] != '\\'){
					newpath[st_Len] = '\\';
				}
				split_path(inputLogFname, drvName, dir, fName, ext);
				st_Len = strlen(fName);
				fName[st_Len - 3] = 0;
				fName[st_Len - 2] = 0;
				fName[st_Len - 1] = 0;
				strcat(newpath, fName);


				if(MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogFname,"%s_71.dvsu", newpath);
				else
					sprintf(inputLogFname,"%s.dvsu", newpath);
				split_path(inputLogFname, drvName, dir, fName, ext);
			}
		}
	}
	else
	{
		split_path(inputLogFname, drvName, dir, fName, ext);
	}
	if(MUDP_master_config->data_extracter_mode == 0)
	{
		if(	MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT)
		{
			sprintf(FileCreate_Pos, "%s%sPacket_Loss_Info.txt",drvName, dir);
			fCreate = fopen(FileCreate_Pos, "a");
			if(!fCreate)
			{
				fprintf(stderr,"Error Creating Output file: %s",FileCreate_Pos);
				ret = -1;
			}else{
				time_t now = time(NULL);
				struct tm *t = localtime(&now);
				fprintf(fCreate, "\n");
				fprintf(fCreate, "*****************************************************************************************\n");
				fprintf(fCreate, "This file provides all the Error Info present in the\n");
				fprintf(fCreate,  "log : %s\n",inputLogFname);
				fprintf(fCreate, "Date: %d-%d-%d %d:%d:%d\n",t->tm_mon+1,t->tm_mday,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec);
				fprintf(fCreate, "*****************************************************************************************\n");
				fprintf(fCreate, "\n");
			}
		}
	}
	else if(MUDP_master_config->data_extracter_mode == 1)
	{
		if( cust_Id == BMW_SAT || cust_Id == BMW_BPIL){
			if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
				for(int i = 0; i< INVALID_STRM; i++)
				{
					if(dump[i] != 0)
					{
						sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName,GetRadarPosName(RADAR_POS), GetstreamIDName(i));
				FileName[RADAR_POS] = FileName_Pos;
						fpMF4Dump[RADAR_POS][i] = fopen(FileName[RADAR_POS],"w");
					}
				}
				
				for (int i=1; i<MAX_ECU_COUNT; i++)
				{
					if(MUDP_master_config->RADAR_ECU_Stream_Options[i] == i){
						for(int j = 0; j< INVALID_STRM; j++)
						{
							if(dump[j] != 0)
							{
								
								if(!fpMF4Dump[RADAR_POS][j])
								{
									fprintf(stderr,"Error Creating Output file: %s",FileName_Pos);
									ret = -1;
								}
							}
						}
					}
				}
			
				if(dump[HEADER] == 1)
				{
					fpMF4Dump[RADAR_POS][HEADER] = fopen(FileName[RADAR_POS],"w");
					if(!fpMF4Dump[RADAR_POS][HEADER])
					{
						fprintf(stderr,"Error Creating Output file: %s",FileName_Pos);
						ret = -1;
					}
				}
			}
		}


		if(MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT){
			for(int i = 0; i< INVALID_STRM; i++)
			{
				if(dump[i] != 0)
				{
					sprintf(FileName_Pos, "%s%s%s_%s.csv", drvName, dir, fName, GetstreamIDName(i));

					fpDump = fopen(FileName_Pos,"w");
				}
			}
			if(!fpDump)
			{
				fprintf(stderr,"Error Creating Output file: %s",FileName_Pos);
				ret = -1;
			}

		}
		else if(MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT){
			for(int i=0; i<MUDP_MAX_STATUS_COUNT;i++)
			{      
				if(MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
				{

					if(MUDP_master_config->RADAR_ECU_Stream_Options[i+1] != i+1){
						for(int j = 0; j< INVALID_STRM; j++)
						{
							if(dump[j] != 0)
							{
								sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

								FileName[i] = FileName_Pos;
								fpMF4Dump[i][j] = fopen(FileName[i],"w");
							}
						}
					}
					/*for(int j = 0; j< INVALID_STRM; j++)
						{*/
					if(!fpMF4Dump[i])
					{
						fprintf(stderr,"Error Creating Output file: %s\n\n",FileName[i]);
						fprintf(stderr," Please Rerun by closing the CSV file %s\n\n",FileName[i]);
						ret = -1;
					}
					//}

				}
			}

		}
	}
	return ret;
}
void setStreamOptions(void){

	if(MUDP_master_config->Radar_Stream_Options[HDR] == HDR)
	{
		dump[HEADER] = 1;
	}
	if(MUDP_master_config->Radar_Stream_Options[Z7B_Core] == Z7B_Core )
	{
		dump[UDP_Z7B_CORE] = 1 ;
		f_z7b_enabled = 1;
	}
	if(MUDP_master_config->Radar_Stream_Options[Z7A_Core] == Z7A_Core )
	{
		dump[UDP_Z7A_CORE] = 1 ;
		f_z7a_enabled = 1;
	}
	if(MUDP_master_config->Radar_Stream_Options[Z4_Core] == Z4_Core )
	{
		dump[UDP_Z4_CORE] = 1 ;
		f_z4_enabled = 1;
	}
	if(MUDP_master_config->Radar_Stream_Options[CDC_iq] == CDC_iq)
	{
		dump[UDP_CDC_STRM] = 1 ;
		f_cdc_enabled = 1;
	}        
	if(MUDP_master_config->Radar_Stream_Options[Z7B_Customer] == Z7B_Customer )
	{
		dump[UDP_CUSTOM_Z7B] = 1;
		f_z7b_custom_enabled = 1;
	}
	if(MUDP_master_config->Radar_Stream_Options[Z4_Customer] == Z4_Customer )
	{
		dump[UDP_CUSTOM_Z4] = 1;
		f_z4_custom_enabled = 1;
	}
	if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] == ecu0 )
	{
		dump[UDP_RDR_ECU0] = 1;
		f_ECU0_enabled = 1;
	}
	if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] == ecu1 )
	{
		dump[UDP_RDR_ECU1] = 1;
		f_ECU1_enabled = 1;
	}
	if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] == ecu3)
	{
		dump[UDP_RDR_ECU3] = 1;
		f_ECU3_enabled = 1;
	}
}

void GetInputOption(const char *Fname)
{
	string LogFName(Fname);
	size_t pos = LogFName.find(".srr3");
	bool four_dvsu_check = false;
	bool mf4_flist = false;
	if(pos == string::npos){
		pos = LogFName.find("_71.dvsu");
		if(pos != string::npos)
			four_dvsu_check = true;
		if(pos == string::npos){
			pos = LogFName.find("_72.dvsu");
			if(pos != string::npos)
				four_dvsu_check = true;
		}
		if(pos == string::npos){
			pos = LogFName.find("_73.dvsu");
			if(pos != string::npos)
				four_dvsu_check = true;
		}
		if(pos == string::npos){
			pos = LogFName.find("_74.dvsu");
			if(pos != string::npos)
				four_dvsu_check = true;
		}
		if(four_dvsu_check == true && pos != string::npos){
			MUDP_master_config->MUDPInput_Options = FOUR_DVSU_INPUT;
		}
	}

	if (pos == string::npos && !four_dvsu_check){
		pos = LogFName.find(".dvsu");
		if(pos != string::npos){
			four_dvsu_check = false;
			MUDP_master_config->MUDPInput_Options = SINGLE_DVSU_INPUT;
		}

	}
	if (pos == string::npos && !four_dvsu_check){
		pos = LogFName.find(".mf4");
		if(pos != string::npos){
			mf4_flist = true;
		}
		if(pos == string::npos){
			pos = LogFName.find(".MF4");
			if(pos != string::npos)
				mf4_flist = true;
		}
		if(mf4_flist)
			MUDP_master_config->MUDPInput_Options = MDF4_INPUT;
	}              
}

void Print_StrmVrsn_Info(DVSU_RECORD_T record)
{
	Customer_T custID = (Customer_T)record.payload[23];
	uint8_t Stream_Num =	0;
	unsigned short Stream_Version = record.payload[20];
	if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		Stream_Num = MaptostreamNumber(record.payload[19],cust_id);
	}
	else{
		Stream_Num = (New_Udp_source_T)record.payload[19];
	} 
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		Stream_Num = record.payload[19];
	}

	if(printStrmHeaderOnes)
	{
		cout<<"************************************************************"<<endl;
		cout<<" Stream Version info table for the provided log"<<endl;
		cout<<"______________________________________________________"<<endl;
		cout<<"|"<<"CustomerName"<<"|"<<"StreamName"<<setw(16)<<"|"<<"StreamVersion"<<"|"<<endl;
		cout<<"______________________________________________________"<<endl;
		printStrmHeaderOnes =false;
	}
	if(Stream_Num == CDC_DATA && F_CDC_DATA == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(18)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_CDC_DATA = false;
	}       
	else if((Stream_Num == Z7A_LOGGING_DATA) && (F_Z7a_core == 1))
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(10)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Z7a_core = false;
	}
	else if((Stream_Num == Z7B_LOGGING_DATA ) && (F_Z7b_core ==1))
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(10)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Z7b_core = false;
	}
	else if((Stream_Num == Z4_LOGGING_DATA) && ( F_z4_core == 1))
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(11)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_z4_core = false;
	}
	else if(Stream_Num == SRR3_Z7B_CUST_LOGGING_DATA && F_z7b_cust == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(5)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_z7b_cust = false;
	}
	else if(Stream_Num == Z4_CUST_LOGGING_DATA && F_z4_cust == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(6)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_z4_cust = false;
	}
	else if(Stream_Num == 90 && F_Ecu_0 == 1)
	{
		cout<<"|"<<GetCustIDName(cust_id)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(17)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Ecu_0 = false;
	}
	else if(Stream_Num == 92 && F_Ecu_3 == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(17)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Ecu_3 = false;
	}
	else if(Stream_Num == 91 && F_Ecu_1 == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(17)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Ecu_1 = false;
	}
	else if(Stream_Num == 94 && F_Ecu_calib == 1)
	{
		cout<<"|"<<GetCustIDName(custID)<<setw(6)<<"|"<<GetStreamNumberName(Stream_Num)<<setw(12)<<"|"<<Stream_Version<<setw(13)<<"|"<<endl;
		F_Ecu_calib = false;
	}
	if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
		if(printOnesline){
			if(!F_Ecu_calib && !F_Ecu_1 && !F_Ecu_3 &&  !F_Ecu_0 && !F_CDC_DATA && !F_Z7a_core && !F_Z7b_core &&  !F_z4_core && !F_z7b_cust && !F_z4_cust){
				cout<<"______________________________________________________"<<endl;
				printOnesline = false;
			}
		}
	}else{
		if(printOnesline){
			if(!F_CDC_DATA && !F_Z7a_core && !F_Z7b_core &&  !F_z4_core && !F_z7b_cust && !F_z4_cust ){
				cout<<"______________________________________________________"<<endl;
				printOnesline = false;
			}
		}
	}

}
int Dump_Option(unsigned int strm_no, int val)
{
 bool set_Dump = false;
	if((strm_no == Z7B_LOGGING_DATA) && (f_z7b_enabled )&& (val == UDP_Z7B_CORE ))
	{
		set_Dump = true;
	}
	else if((strm_no == Z7A_LOGGING_DATA) && (f_z7a_enabled )&& (val == UDP_Z7A_CORE ))
	{
		set_Dump = true;
	}
	else if	((strm_no == Z4_LOGGING_DATA) && (f_z4_enabled )&&( val == UDP_Z4_CORE))
	{
		set_Dump = true;
	}
	else if	((strm_no == CDC_DATA) && (f_cdc_enabled) && (val == UDP_CDC_STRM))
	{
		set_Dump = true;
	}
	else if	((strm_no == SRR3_Z7B_CUST_LOGGING_DATA) &&( f_z7b_custom_enabled) && (val == UDP_CUSTOM_Z7B)) 
	{
		set_Dump = true;
	}
	else if	((strm_no == Z4_CUST_LOGGING_DATA) && (f_z4_custom_enabled)  &&( val == UDP_CUSTOM_Z4))
	{
		set_Dump = true;
	}
	else if	((strm_no == RADAR_ECU_CORE_0) && (f_ECU0_enabled )&&( val == UDP_RDR_ECU0))    
	{
		set_Dump = true;
	}
	else if	((strm_no == RADAR_ECU_CORE_1) && (f_ECU1_enabled)  && (val == UDP_RDR_ECU1))
	{
		set_Dump = true;
	}
	else if	((strm_no == RADAR_ECU_CORE_3) && (f_ECU3_enabled ) && (val == UDP_RDR_ECU3))
	{
		set_Dump = true;
	}
	

	return set_Dump;
}
int DumpMUDPData( const char* LogFname){
	int ret = 0;
	DVSU_RECORD_T record = {0};
	unsigned int iMudpStrNo = 0;
	char sourceId = 0;
	unsigned int Stream_No = 0;
	UINT64 timestamp_ns = 0;
	char InpuLogName[1024] = {0};
	UINT8 Bussource = 0x07;
	INT64 ViGEMtimestamp_ns = 0;
	void * dataPtr = NULL;
	strcpy(InpuLogName, LogFname );
	do{
		if(	MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT){
			if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_open(LogFname,&mudp_log_in_ptr))
			{
				fprintf(stderr,"Error Opening input file: %s",LogFname);
				ret = -1;
				break;
			}
		}
		if(MUDP_master_config->MUDPInput_Options == MDF4_INPUT){
			if(!g_mdfUDP){
				g_mdfUDP = new apt_mdfFile;
			}
			if(!g_mdfUDP->m_MdfLibValid_Flag)
			{
				printf("MF4 output files cannot be created, please verify whether correct MDF license & MDF lib file is available in the required path\n");
			}
			if(g_mdfUDP->m_MdfLibValid_Flag){
				g_mdfUDP->getMf4_ToolID(LogFname, &setViGEMMf4);
				//setViGEMMf4 = 1;
				if(setViGEMMf4){
					if(eOK != g_mdfUDP->OpenVIGEMmdfFile(LogFname, &timestamp_ns, false)){ 
						printf("[ERROR]: File has no UDP data <%s>.\n", LogFname);
						return -1;
					}
				}
				else{
					if(eOK != g_mdfUDP->OpenUDPmdfFile(LogFname, false)){
						printf("[ERROR]: File has no UDP data <%s>.\n", LogFname);
						return -1;
					}
				}
			}
			while(1)
			{
				memset(&record, 0, sizeof(record));
				timestamp_ns = 0;
				if(setViGEMMf4){
					if(eOK != g_mdfUDP->read_VigemMDF(&dataPtr, &ViGEMtimestamp_ns, &Bussource)){
						break;
					}

					else{
						timestamp_ns = (UINT64)ViGEMtimestamp_ns;
						if(Bussource == 0x07){
							if(dataPtr != NULL)
							{
								memcpy(&record, dataPtr, sizeof(DVSU_RECORD_T)); 
							}
						}
						else if(Bussource == 0x02){
							continue;
						}
					}
				}
				else{
					if(eOK != g_mdfUDP->read_MDF(&record, &timestamp_ns)){
						break;
					}
				}
				if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) )
				{ 
					//if(MUDP_master_config->MUDP_Radar_ECU_status != 1 ){
					if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
						sourceId = (record.payload[9] - 1);
						cust_id = (Customer_T)record.payload[23];
					  if(sourceId == 19)
						{
							Stream_No =MaptoEcustreamNumber(record.payload[19]);
						} 
						else
						{
							Stream_No = MaptostreamNumber(record.payload[19], cust_id);
						} 
						
					}
					else{
						sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
						cust_id = (Customer_T)record.payload[23];
						if(sourceId == 19)
						{
							Stream_No =MaptoEcustreamNumber(record.payload[19]);
						} 
						else
						{
							Stream_No = MaptostreamNumber(record.payload[19], cust_id);
						} 
					}
					
				    Print_StrmVrsn_Info(record);
					if(CreateOnce){
						if(-1 == CreateOutputCSVFile(InpuLogName, cust_id)){
							return -1;
						}
						CreateOnce = false;   
					}

					if(MUDP_master_config->data_extracter_mode == 1)
					{
						if(MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && sourceId == RADAR_POS) ){

							for(int Opt = 0; Opt< INVALID_STRM; Opt++)
							{
								if(dump[Opt] != 0)
								{
									bool set_flag = 0;
									if(fpMF4Dump[sourceId][Opt]){
										set_flag = Dump_Option(Stream_No,Opt);
										if(set_flag || (dump[HEADER] == 1 && Opt == HEADER)){
											if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId][Opt],Opt, timestamp_ns,dump_option, &record,true)){

												ret = -1;
												break;
											}
										}
									}
								}
							}

						}
					}
					else
					{
						if(fCreate){
							for(int Opt = 0; Opt< INVALID_STRM; Opt++)
							{
								if(dump[Opt] != 0)
								{
									bool set_flag = 0;
									if(fpMF4Dump[sourceId][Opt]){
										set_flag = Dump_Option(Stream_No,Opt);
										if(set_flag || (dump[HEADER] == 1 && Opt == HEADER)){
											if( -1 == ExtractAndListDVSU(fCreate,Opt,timestamp_ns, dump_option, &record,true)){
												ret = -1;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if(strstr(LogFname,".dvsu") != NULL)
		{
			while(1)
			{
				memset(&record,0,sizeof(record));
				if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_read(mudp_log_in_ptr,(unsigned char*)&record,NULL))
					break;
				UINT64 timestamp_ns = record.pcTime;
				if(MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT){
					//if(MUDP_master_config->MUDP_Radar_ECU_status != 1 ){
					if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
						sourceId = (record.payload[9] - 1);
					}
					else{
						sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
					} 
					cust_id = (Customer_T)record.payload[23];
					if(CreateOnce){
						if(-1 == CreateOutputCSVFile(InpuLogName, cust_id)){
							return -1;
						}
						CreateOnce = false;   
					}
					if(MUDP_master_config->data_extracter_mode == 1)
					{
								for(int Opt = 0; Opt< INVALID_STRM; Opt++)
								{
									if(dump[Opt] != 0)
									{
										bool set_flag = 0;
										if(fpMF4Dump[sourceId][Opt]){
											set_flag = Dump_Option(Stream_No,Opt);
											if(set_flag || (dump[HEADER] == 1 && Opt == HEADER)){
												if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId][Opt],Opt,timestamp_ns, dump_option, &record,true)){
													ret = -1;
													break;
												}
											}
										}
									}
								}
					}
					else
					{
						if(fCreate){
							for(int Opt = 0; Opt< INVALID_STRM; Opt++)
							{
								if(dump[Opt] != 0)
								{
									bool set_flag = 0;
									if(fpMF4Dump[sourceId][Opt]){
										set_flag = Dump_Option(Stream_No,Opt);
										if(set_flag || (dump[HEADER] == 1 && Opt == HEADER)){
											if( -1 == ExtractAndListDVSU(fCreate,Opt,timestamp_ns, dump_option, &record,true)){
												ret = -1;
												break;
											}

										}
									}
								}
							}
						}
					}

				}
				else            
				{
					if(CreateOnce){
						if(-1 == CreateOutputCSVFile(InpuLogName, cust_id)){
							return -1;
						}
						CreateOnce = false;   
					}
					if(MUDP_master_config->data_extracter_mode == 1)
					{
						for(int i = 0; i< INVALID_STRM; i++)
						{
							if(dump[i] != 0)
							{
								bool set_flag = 0;
								if(fpMF4Dump[sourceId][i]){
									set_flag = Dump_Option(Stream_No,i);
									if(set_flag || (dump[HEADER] == 1 && i == HEADER)){
										if( -1 == ExtractAndListDVSU(fpDump, dump[i],timestamp_ns,dump_option, &record,false)){
											ret = -1;
											break;
										}
									}
								}
							}
						}
					}
					else
					{
						if(fCreate){
							for(int Opt = 0; Opt< INVALID_STRM; Opt++)
							{
								if(dump[Opt] != 0)
								{
									bool set_flag = 0;
									if(fpMF4Dump[sourceId][Opt]){
										set_flag = Dump_Option(Stream_No,Opt);
										if(set_flag || (dump[HEADER] == 1 && Opt == HEADER)){
											if( -1 == ExtractAndListDVSU(fCreate,dump[Opt], timestamp_ns,dump_option, &record,true)){
												ret = -1;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}while(0);
	if(mudp_log_in_ptr){
		m_pLogPlugin->SRR3_mudp_close(&mudp_log_in_ptr);
	}
	if(fpDump != NULL)
		fclose(fpDump);
	return ret;

}
int Dump(const char* LogFname, const char* ouputfname)
{
	int ret = 0;
	char sourceId = 0;
	void*  mudp_log_in = NULL; //single log. when working for 4 sensor, use this as the base name and load all the four sensor logs
	DVSU_RECORD_T record = {0};
	unsigned int iMudpStrNo = 0;
	UINT64 timestamp_ns = 0;
	UDPLogStruct* pLogStruct = NULL;
	char * FileName[4];
	fpDump = NULL;
	bool Fourdvsu_check = false;
	//CheckDVSU(LogFname) ;
	if(dump_option == UDP_CONVERT)
	{
		m_version_info.strm_ver.z7b = 33;
		m_version_info.strm_ver.z7a = 21;
		m_version_info.strm_ver.z4 = 20;
		m_version_info.strm_ver.z4_cust = 4;
		m_version_info.strm_ver.z7b_cust = 8;
		m_version_info.strm_ver.z7b_tracker = 7;
		if(-1 == CreateOutputDVSUFile(LogFname))
		{
			ret = -1;
		}
	}

	do{
		if(dump_option == UDP_ECU2)
		{
			fprintf(stderr,"Error Creating Output file: %s",ouputfname);
			return -1;
		}
		if(dump_option != UDP_CONVERT)
		{
			if(strstr(LogFname,".dvsu") != NULL){
				if(Fourdvsu_check){
					fpDump = fopen(ouputfname,"w");
					if(!fpDump)
					{
						fprintf(stderr,"Error Creating Output file: %s",ouputfname);
						ret = -1;
						break;
					}
				}
				if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_open(LogFname,&mudp_log_in))
				{
					fprintf(stderr,"Error Opening input file: %s",LogFname);
					ret = -1;
					break;
				}
			}
			if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) || Fourdvsu_check == false){
				if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3))
				{
					char drvName[_MAX_DRIVE];
					char dir[_MAX_DIR];
					char fName[_MAX_FNAME];
					char ext[_MAX_EXT];
					split_path(ouputfname, drvName, dir, fName, ext); 

					sprintf(FileNameRL, "%s%s%s_71.csv", drvName, dir, fName);
					sprintf(FileNameRR, "%s%s%s_72.csv", drvName, dir, fName);
					sprintf(FileNameFR, "%s%s%s_73.csv", drvName, dir, fName);
					sprintf(FileNameFL, "%s%s%s_74.csv", drvName, dir, fName); 
					FileName[0] = FileNameRL ;
					FileName[1] = FileNameRR ;
					FileName[2] = FileNameFR ;
					FileName[3] = FileNameFL ;

					/*for(int i=0;i<4;i++)
					{

						fpMF4Dump[i] = fopen(FileName[i],"w");
						if(!fpMF4Dump[i])
						{
							fprintf(stderr,"Error Creating Output file: %s",FileName[i]);
							ret = -1;
							break;
						}
					}*/
				}
				else
				{
					fpDump = fopen(ouputfname,"w");
					if(!fpDump)
					{
						fprintf(stderr,"Error Creating Output file: %s",ouputfname);
						ret = -1;
						break;
					}
				}
			}   
			if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) ){
				if(!g_mdfUDP){
					g_mdfUDP = new apt_mdfFile;
				}
				if(!g_mdfUDP->m_MdfLibValid_Flag)
				{
					printf("MF4 output files cannot be created, please verify whether correct MDF license & MDF lib file is available in the required path\n");
				}
				if(g_mdfUDP->m_MdfLibValid_Flag){
					if(eOK != g_mdfUDP->OpenUDPmdfFile(LogFname, false)){
						printf("[ERROR]: File has no UDP data <%s>.\n", LogFname);
						return -1;
					}
					while(1)
					{
						memset(&record, 0, sizeof(record));
						timestamp_ns = 0;
						if(eOK != g_mdfUDP->read_MDF(&record, &timestamp_ns)){
							break;
						}
						if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) )
						{ 
							if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3)){
								if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
									sourceId = (record.payload[9] - 1);
								}
								else{
									sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
								} 
								/*if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId], dump_option, &record,true)){
									ret = -1;
									break;
								}*/
							}
							/*else            
							{
								if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
									ret = -1;
									break;
								}
							}*/
						}
					}
				}
			}
		}
		if(strstr(LogFname,".mrr2") != NULL) //Buffer for Stream Data
		{
			while(1)
			{
				if ( E_MUDPLOG_OK  != m_pLogPlugin->SRR3_mudp_read(mudp_log_in,(unsigned char*)&g_RacamUdpBuffer[0],&iMudpStrNo))
					break;


				if(((iMudpStrNo >=0 )&&(iMudpStrNo < RACAMUDP_MAX_ST))/* && (MUDP_master_config->Debug_Extracter_Data == 1)*/)
				{
					if( -1 == ExtractAndListMUDP(fpDump,dump_option,(UDPLogStruct*) &g_RacamUdpBuffer[0])){
						ret = -1;
						break;
					}
				}
			}
		}
		else if(strstr(LogFname,".dvsu") != NULL)
		{
			while(1)
			{
				memset(&record,0,sizeof(record));
				if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_read(mudp_log_in,(unsigned char*)&record,NULL))
					break;
				if(dump_option == UDP_CONVERT)
				{
					if(-1 == ExtractAndConvert(LogFname, &record)){
						ret = -1;
						break;
					}
					DumpToOutputDVSU();

				}
				else
				{
					if(Fourdvsu_check == false){
						if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3)){
							if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
								sourceId = (record.payload[9] - 1);
							}
							else{
								sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
							} 
							/*if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId], dump_option, &record,true)){
								ret = -1;
								break;
							}*/
						}
						/*else            
						{
						if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
						ret = -1;
						break;
						}*/
					}
					/*else            
					{
						if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
							ret = -1;
							break;
						}
					}*/
				}
			}
			/*if( -1 == ExtractAndListDVSU(fpDump,dump_option, &record,false)){
			ret = -1;
			break;
			}  */         
		}
	}while(0);

	/*if(g_pIRadarStrm)
	{
		delete g_pIRadarStrm;
		g_pIRadarStrm[stream_no] = NULL;
	}*/
	if(mudp_log_in){
		m_pLogPlugin->SRR3_mudp_close(&mudp_log_in);
	}

	if(fpDump != NULL)
		fclose(fpDump);

	return ret;
}

int ExtractAndListDVSU(FILE*pOutFile , int Dump_Opt,UINT64 Timestamp,enDumpOpt opt, DVSU_RECORD_T* pRec ,bool mf4flag )
{
	UDPRecord_Header  udp_frame_hdr = {0};
	udp_custom_frame_header_t  cust_frame_hdr = {0};
	unsigned char format = 1; //This is to flag the endianness to be big endian
	static int once[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] ={ 0};
	unsigned char* bytestream = &pRec->payload[0];
	INT64 timestamp = (INT64)Timestamp;
	bool setECUflag = 0;
	unsigned char sourceId = 0;
	if((pRec->payload[0] != 0xA1 || pRec->payload[0] != 0xA2 || pRec->payload[0] != 0xA3) && ( pRec->payload[1] != 0x18)
		&& (pRec->payload[1] != 0xA2 || pRec->payload[1] != 0xA3) && ( pRec->payload[0] != 0x18)){
			return 0;
	}
	if(pRec->payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		sourceId = (pRec->payload[9] - 1);
	}
	else{
		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
	} 
	if(sourceId == RDR_POS_ECU)
	{
		format = 0;
	}
	if (MUDP_master_config->data_extracter_mode == 1)
	{
		switch(Dump_Opt)
		{
		case HEADER:
			m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
			m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0], &cust_frame_hdr);
			//	print_udp_header(pOutFile, &udp_frame_hdr,(unsigned long)timestamp);
			print_udp_header_All(pOutFile, &udp_frame_hdr, &cust_frame_hdr,(INT64)timestamp);
			return 0;

		case UDP_CUSTOM_HDR:
			// m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0],&cust_frame_hdr);
			//print_udp_cust_header(pOutFile,&cust_frame_hdr);
			return 0;

				default:
					break;

				}
		
		
	}
	short source_no = -1;
	short stream_no = -1;
	if (MUDP_master_config->data_extracter_mode == 0){
		m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
		if(MUDP_master_config->MUDP_Radar_ECU_status == 1){
			if(sourceId == RADAR_POS){
				setECUflag = 1;
			}
		}
		else{
			setECUflag = 1;
		}
		if(setECUflag){
			get_Source_TxnCnt( &udp_frame_hdr);
			CheckSameScanIdforChunk(&udp_frame_hdr, timestamp);
		}
	}
	srr3_api_status_e status = Status_Latch_Error;
	status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream,timestamp,&source_no, &stream_no, format);
	if(status != Status_Latch_Completed)
		return 0;

	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
	unsigned char version = 0;
	if (MUDP_master_config->data_extracter_mode == 1){
		bool set_flag = 0;
		set_flag = Dump_Option(stream_no,Dump_Opt);
		if(set_flag)
      
		{
			status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out,(unsigned char)source_no, (unsigned char)stream_no);
		}
		else
		{
			return 0;
		}
	}
	else{
		if(stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA  || stream_no == Z4_CUSTOM || stream_no == Z4_LOGGING_DATA || stream_no == CDC_DATA 
			|| stream_no == SRR3_Z7B_CUST_LOGGING_DATA || stream_no == Z4_CUST_LOGGING_DATA || stream_no == RADAR_ECU_CORE_0 || stream_no == RADAR_ECU_CORE_1 ||stream_no == RADAR_ECU_CORE_3  ){
				status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out,(unsigned char)source_no, (unsigned char)stream_no);
		}else{
			return 0;
		}
		unsigned int ScanId = p_latch_out->proc_info.frame_header.streamRefIndex;
		if(setECUflag){
			get_scanindex_info( ScanId, sourceId, stream_no);
		}
		if(p_latch_out->proc_info.f_failure_block == 1 && stream_no!= CDC_DATA ){
			Get_PacketLossCount(p_latch_out,sourceId,stream_no);
		}
		if(stream_no == CDC_DATA)
		{
			if(MUDP_master_config->MUDP_Radar_ECU_status != 1){
				CDC_PacketLossCount(p_latch_out,sourceId);
			}

		}

		return 0;

	}
	cust_id = (Customer_T)p_latch_out->proc_info.frame_header.customerID;
	
	if(f_z7b_enabled && stream_no == Z7B_LOGGING_DATA)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7B,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_z7a_enabled &&  stream_no == Z7A_LOGGING_DATA){
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7A,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_z4_enabled &&  stream_no == Z4_LOGGING_DATA){
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z4,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_cdc_enabled &&  stream_no == CDC_DATA){
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,CDC,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_z7b_custom_enabled &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA){
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7B_CUSTOM,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_z4_cust_enabled &&  stream_no == Z4_CUST_LOGGING_DATA)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z4_CUSTOM,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}
	else if(f_z4_custom_enabled  &&  stream_no == Z4_CUST_LOGGING_DATA)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z4_CUSTOM,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}	
	else if(f_ECU0_enabled &&  stream_no == RADAR_ECU_CORE_0)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,ECU_C0,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}	
	else if(f_ECU1_enabled &&  stream_no == RADAR_ECU_CORE_1)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,ECU_C1,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}		
	else if(f_ECU3_enabled &&  stream_no == RADAR_ECU_CORE_3)
	{
		if(!g_pIRadarStrm[stream_no])
		{
			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,ECU_C3,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
		}
	}

	if(!g_pIRadarStrm[stream_no]){
		fprintf(stderr,"Error! No suitable decoder found for platform = %u, stream = %u , version = %u\n",p_latch_out->proc_info.frame_header.Platform,p_latch_out->proc_info.frame_header.streamNumber,
			p_latch_out->proc_info.frame_header.streamVersion);
		return -1;
	}

	switch(Dump_Opt)
	{
	case UDP_Z7B_CORE:
	case UDP_Z7A_CORE:
	case UDP_Z4_CORE:
	case UDP_CDC_STRM:
	case UDP_CUSTOM_Z7B:
	case UDP_CUSTOM_Z4:
	case UDP_RDR_ECU0:
	case UDP_RDR_ECU1:
	case UDP_RDR_ECU3:
	default:
		e_Options = ALL;
		break;
	}
		
	
	if(mf4flag == true){
		if((once_list[sourceId][stream_no] == 0)&&((Dump_Opt==UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA)||(Dump_Opt==UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA)||(Dump_Opt==UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA)||(Dump_Opt==UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA)||(Dump_Opt==UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
			||(Dump_Opt==UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Dump_Opt==UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Dump_Opt==UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Dump_Opt==UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3))) 
			
		{
			g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
			once_list[sourceId][stream_no] = 1;
		}
	}
	else{
		if((once[sourceId][stream_no] == 0)&&((Dump_Opt==UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA)||(Dump_Opt==UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA)||(Dump_Opt==UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA)||(Dump_Opt==UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA)||(Dump_Opt==UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
			||(Dump_Opt==UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Dump_Opt==UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Dump_Opt==UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Dump_Opt==UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3))) 
			
		{
			g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
			once[sourceId][stream_no] = 1;
		}
	}
	
	g_pIRadarStrm[stream_no]->FillStreamData(&p_latch_out->data[0],p_latch_out->proc_info.nDataLen,&p_latch_out->proc_info);
#ifdef RADAR_STREAM_UT
	static bool first = false;
	if(!first){
		first = true;
		g_pIRadarStrm->PrintHeader(pOutFile,ALL);
	}
	if(e_Options == ALL){
		g_pIRadarStrm->Convert_Data(*g_pIRadarStrm);
		g_pIRadarStrm->PrintData(pOutFile,ALL);
	}
	return 0;
#endif
	g_pIRadarStrm[stream_no]->PrintData(pOutFile,e_Options);
	//g_pIRadarStrm->getData(&p_latch_out->data[0],e_Options);
	return 0;

}
class line : public string {};

std::istream &operator>>(std::istream &is, line &l)
{
	std::getline(is, l);
	return is;
}
int ExtractAndListMUDP(FILE*pOutFile , enDumpOpt opt, UDPLogStruct* pLogStruct )
{
	DVSU_RECORD_T DVSURecord = {0};
	if(NULL == pLogStruct)
		return -1;
	DVSURecord.pcTime = pLogStruct->timestamps.pcTime;
	static int once = 0;
	//forming the DVSU_REC
	memcpy((unsigned char*)&DVSURecord.payload[0],(unsigned char*)&pLogStruct->header,sizeof(UDPRecord_Header));
	memcpy(&DVSURecord.payload[sizeof(UDPRecord_Header)],(unsigned char*)&pLogStruct->payload[0], byteswap(pLogStruct->header.streamDataLen));

	UDPRecord_Header  udp_frame_hdr = {0};
	udp_custom_frame_header_t  cust_frame_hdr = {0};
	unsigned char* bytestream = &DVSURecord.payload[0];
	unsigned __int64 timestamp = (unsigned __int64)DVSURecord.pcTime;
	unsigned char sourceId = 0;

	if(DVSURecord.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		sourceId = (DVSURecord.payload[9] - 1);
	}
	else{
		sourceId = DVSURecord.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
	} 

	switch(opt)
	{
	case UDP_HDR:

		m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream,&udp_frame_hdr);

		print_udp_header(pOutFile,&udp_frame_hdr, (unsigned long)timestamp);
		return 0;

		/*case UDP_CUST_HDR:
		m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0],&cust_frame_hdr);
		print_udp_cust_header(pOutFile,&cust_frame_hdr);
		return;*/

	case UDP_CALIB_USC:
		DumpCalib(pOutFile,opt,(unsigned char*)bytestream);
		return 0;

	default:
		break;
	}

	short source_no = -1;
	short stream_no = -1;
	srr3_api_status_e status = Status_Latch_Error;
	status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream,timestamp,&source_no, &stream_no,1);
	if(status != Status_Latch_Completed)
		return 0;

	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
	unsigned char version = 0;
	if((stream_no == Z7B_LOGGING_DATA && opt == UDP_Z7B) || (stream_no == Z7A_LOGGING_DATA && opt == UDP_Z7A)
		|| (stream_no == Z4_LOGGING_DATA && opt == UDP_Z4) || (stream_no == CDC_DATA && opt == UDP_CDC)|| (stream_no == Z7A_LOGGING_DATA && opt == UDP_Z7A_RDD)|| (stream_no == Z7B_LOGGING_DATA && opt == UDP_Z7B_RDD))
	{
		status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out,(unsigned char)source_no, (unsigned char)stream_no);
	}
	else
	{
		return 0;
	}

	switch(opt)
	{
	case UDP_Z7B:
		{
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7B,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}

		}
		break;

	case UDP_Z7A:
		{
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7A,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm[stream_no]){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}
		}
		break;
	case UDP_Z7A_RDD:
		{
			e_Options = RDD_MRR2;
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7A,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm[stream_no]){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}
		}
		break;
	case UDP_Z7B_RDD:
		{
			e_Options = RDD_MRR2;
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z7B,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm[stream_no]){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}
		}
		break;
	case UDP_Z4:
		{
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,Z4,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm[stream_no]){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}
		}
		break;

	case UDP_CDC:
		{
			if(!g_pIRadarStrm[stream_no])
			{
				g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform,CDC,p_latch_out->proc_info.frame_header.streamVersion,cust_id);
				if(g_pIRadarStrm[stream_no]){
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
				}
			}			
		}
		break;

	default:
		return -1;
	}

	if(!g_pIRadarStrm[stream_no])
	{
		fprintf(stderr,"Error! No suitable decoder found for Platform = %u, stream = %u , version = %u\n",p_latch_out->proc_info.frame_header.Platform,p_latch_out->proc_info.frame_header.streamNumber,
			p_latch_out->proc_info.frame_header.streamVersion);
		return -1;
	}

	g_pIRadarStrm[stream_no]->FillStreamData(&p_latch_out->data[0],p_latch_out->proc_info.nDataLen,(void*)&p_latch_out->proc_info);
	g_pIRadarStrm[stream_no]->PrintData(pOutFile,e_Options);

	return 0;

}
int main(int argc, char* argv[])
{
	
	fprintf(stdout, "\MUDP_Log_DataExtracter.exe version %d.%d.%d \n", release_inf,release_op,release_minor);
	fprintf(stdout, "/*=========================================================================================================*/\n");
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	do
	{

		if ( ReadArguments(argc,argv))
		{
			return -1; //error
		}
		string xml_file_chk = xml_file_path;
		size_t pos = xml_file_chk.find(".xml");
		MUDP_master_config = new MUDP_CONFIG_T;
		if (MUDP_OK != MUDP_read_master_config(MUDP_master_config, (const char*)xml_file_path))

		{
			printf("[INFO] Reading Master configuration file fails. Proceeding with Default Configuration.\n");

		}

		std::ifstream inputFile(inp_file);
		istream_iterator<line> itFname(inputFile);
		istream_iterator<line> end;
		strcpy(logname,(*itFname).c_str());
		strcpy(m_setFileName,(*itFname).c_str());
		string log_file_chk = logname; 

		if(!inputFile)
		{
			perror("[INFO] Error in Reading Log List...\n Exiting Resimulation...\n\n");

		} 
		//strcpy(logname,(*itFname).c_str());

		if ( LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Log("SRR3_MUDP_Log.dll",&m_pLogPlugin)){
			break;
		}

		if ( LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Decoder("srr3_decoder_dph.dll",&m_pDecoderPlugin)){
			break;
		}

		m_pDecoderPlugin->SRR3_API_Init();
		setStreamOptions();
		size_t dvlpos = log_file_chk.find(".dvl");
		if(dvlpos == string::npos)
		{
			GetInputOption(logname);
		} 
		int retVal = 0;
		for(; itFname != end; ++itFname)
		{
			strcpy(logname,(*itFname).c_str());
			if(!CreateOnce){
				if(strcmp(m_setFileName, logname) !=0){
					CreateOnce = true;
					strcpy(m_setFileName,(*itFname).c_str());
				}
			}
			if (file_exist(logname, 0) == -1)
			{
				printf("\n[INFO] Dump is unsuccesful , since FILE is not found (%s)\n\n", logname);
				printf("/*=========================================================================================================*/\n");
				retVal = -1;
			}
			if(retVal == 0){
				ResetScanIndexpacketCounts();
				if(MUDP_master_config->data_extracter_mode == 1){
					
					printf("Data Extraction operation started for the Log :%s!!\n\n", logname);
				}
				else{
					printf("Packet_Loss Extraction started for the Log :%s!!\n\n", logname);
				}
				if( 0 == DumpMUDPData(logname)){
					if(MUDP_master_config->data_extracter_mode == 1){
						printf("\nDump Operation Completed for (%s)!!\n", logname);
						printf("/*=========================================================================================================*/\n");
						for(int i = 0; i<MAX_RADAR_COUNT ; i++){
						bOnce[i] = 0;
							for(int j =0; j<MAX_LOGGING_SOURCE; j++)
							{
							
							once_list[i][j] = 0;
							}
						}
					}
					if(fCreate){
						if(MUDP_master_config->data_extracter_mode == 0){
							CloseScanIndexerrorFile(fCreate, logname);
							printf("Packet_Loss information extraction completed for the Log :%s!!\n\n", logname);
							printf("/*=========================================================================================================*/\n");
						}
					}
					printf("\n\Reading next log file from the input list...\n\n");

				}
				else{
					printf("\nDump Unsuccessful...\n");
					printf("/*=========================================================================================================*/\n");
				}
				if(g_mdfUDP){
					delete g_mdfUDP;
					g_mdfUDP = NULL;
				}
			}

		}
	}while(0);

	FreePlugin();
	if(g_mdfUDP){
		delete g_mdfUDP;
		g_mdfUDP = NULL;
	}

	return 0;
}

void getDetectionsData(DVSU_RECORD_T* pRec){
	UDPRecord_Header  udp_frame_hdr = {0};
	udp_custom_frame_header_t  cust_frame_hdr = {0};
	void* pBuffer = NULL;
	unsigned int size = 0;
	unsigned char ver = 0;
	unsigned char sourceId = 0;

	if(pRec->payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		sourceId = (pRec->payload[9] - 1);
	}
	else{
		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
	} 


	//UDP_PLUGIN_API srr3_api_status_e  SRR3_API_GetFrameHdr(const void* const p_bytestream , UDPRecord_Header* p_udp_hdr)

	unsigned char* bytestream = &pRec->payload[0];

	m_pDecoderPlugin->SRR3_API_GetFrameHdr(bytestream,&udp_frame_hdr);
	short source_no = -1;
	short stream_no = -1;

	if(Status_Latch_Completed == m_pDecoderPlugin->SRR3_API_LatchData(bytestream,pRec->pcTime,&source_no, &stream_no,1)){ ///for example read detection from rear right radar when latch is complete

		Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
		unsigned char version = 0;
		if(stream_no == Z7B_LOGGING_DATA){
			if ( Status_OK == m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out,(unsigned char)source_no, (unsigned char)stream_no)){


				UDPRecord_Header* pudp_frame_hdr =  &p_latch_out->proc_info.frame_header;

				switch(stream_no){
				case Z7B_LOGGING_DATA:
					if (!m_pSrcStreams->m_pStreams[stream_no]){
						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21,Z7B,pudp_frame_hdr->streamVersion,cust_id);
					}

					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
					size = m_pDestStreams->m_pStreams[stream_no]->get_size();
					ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
					break;

				case Z7A_LOGGING_DATA:
					if (!m_pSrcStreams->m_pStreams[stream_no]){
						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21,Z7A, pudp_frame_hdr->streamVersion,cust_id);
					}

					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
					size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
					ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
					break;

				case Z4_LOGGING_DATA:
					if (!m_pSrcStreams->m_pStreams[stream_no]){
						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21,Z4, pudp_frame_hdr->streamVersion,cust_id);
					}

					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
					size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
					ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
					break;

				case SRR3_Z7B_CUST_LOGGING_DATA:
					if (!m_pSrcStreams->m_pStreams[stream_no]){
						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21,Z7B_TRACKER ,pudp_frame_hdr->streamVersion,cust_id);
					}

					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
					size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
					ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
					break;

				default:
					pBuffer = NULL;
					break;
				}


				/*if(pBuffer){

				PackUdpLogging(pBuffer,
				size,
				pudp_frame_hdr->sourceInfo,
				pudp_frame_hdr->streamNumber,
				pudp_frame_hdr->streamRefIndex,
				ver,
				*pDestQueue);
				}*/
			}
		}
	}

}

void DumpCalib(FILE* fptr, enDumpOpt opt, const unsigned char* bytestream)
{
	UDPRecord_Header  udp_frame_hdr = {0};
	m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream,&udp_frame_hdr);
	if(udp_frame_hdr.streamNumber == CALIBRATION_DATA)
	{
		switch(opt)
		{
		case UDP_CALIB_USC:
			ExtractandListCalib(fptr,&udp_frame_hdr,bytestream+sizeof(udp_frame_hdr));
			break;

		default:
			break;
		}
	}

}

int ExtractAndConvert(const char* m_LogFname,DVSU_RECORD_T* pRec)
{
	int ret = 0;
	//ARSTREAMS *pSrcQueue = &m_WorkingQueue;

	for_each(m_TxQueue.begin(),m_TxQueue.end(),DeleteContent<CByteStream>());
	m_TxQueue.clear();

	ARSTREAMS *pDestQueue = &m_TxQueue;

	short source_no = -1;
	short stream_no = -1;
	srr3_api_status_e status = Status_Latch_Error;

	unsigned char* bytestream = &pRec->payload[0];
	unsigned __int64 timestamp = (unsigned __int64)pRec->pcTime;
	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
	unsigned int iStrVerNoZ7A = 0,iStrVerNoZ7B = 0,iStrVerNoZ4 = 0,iStrVerNoZ7B_Custom = 0,iStrVerNoZ4_Custom = 0;
	unsigned int SourceId_z4 = 0,StreamRefIndex = 0;
	/*status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream,timestamp,&source_no, &stream_no,1);
	if(status != Status_Latch_Completed)
	return 0;
	;*/
	unsigned char sourceId = 0;

	if(pRec->payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
		sourceId = (pRec->payload[9] - 1);
	}
	else{
		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
	} 


	////g_pIRadarStrm->FillStreamData(&p_latch_out->data[0],p_latch_out->proc_info.nDataLen);


	//Non-CompATIBLE SOURCE DESTINAITON
	for_each(pDestQueue->begin(),pDestQueue->end(),DeleteContent<CByteStream>());
	pDestQueue->clear();

	void* pBuffer = NULL;
	const int MAX_BACK_BUFFER = 100000;
	static unsigned char final_buffer[MAX_BACK_BUFFER] = {0};

	memset(final_buffer,0,MAX_BACK_BUFFER);

	unsigned int size = 0;
	unsigned char ver = 0;
	UDPRecord_Header udp_hdr = {0};
	m_pDecoderPlugin->SRR3_API_GetFrameHdr(bytestream,&udp_hdr);
	if(udp_hdr.streamNumber == CDC_DATA){
		m_pLogPlugin->SRR3_mudp_write(mudp_log_out,(unsigned char*)pRec);		
		return 0;
	}

	if(Status_Latch_Completed != m_pDecoderPlugin->SRR3_API_LatchData(bytestream,
		::timeGetTime(),&source_no,&stream_no,1))
	{
		return 0;
	}



	if ( Status_OK == m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out,(unsigned char)source_no, (unsigned char)stream_no))
	{

		UDPRecord_Header* pudp_frame_hdr =  &p_latch_out->proc_info.frame_header;

		switch(stream_no)
		{
		case Z7B_LOGGING_DATA:
			if (!m_pSrcStreams->m_pStreams[stream_no])
			{
				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7B,pudp_frame_hdr->streamVersion,cust_id);
				fprintf(stdout,"\n z7b version in File :: %d",pudp_frame_hdr->streamVersion);
			}
			if(pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7b)
			{
				fprintf(stdout,"\n z7b version in File is greater than Sensors,Stopping Execution!!!");
				return FALSE;
			}
			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7B,pudp_frame_hdr->streamVersion,cust_id);
			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
			break;

		case Z7A_LOGGING_DATA:
			if (!m_pSrcStreams->m_pStreams[stream_no])
			{
				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7A, pudp_frame_hdr->streamVersion,cust_id);
				fprintf(stdout,"\n z7a version in File :: %d",pudp_frame_hdr->streamVersion);
			}
			if(pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7a )
			{
				fprintf(stdout,"\n z7a version in File is greater than Sensors,Stopping Execution!!!");
				return FALSE;
			}
			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7A, pudp_frame_hdr->streamVersion,cust_id);
			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
			break;

		case Z4_LOGGING_DATA:
			if (!m_pSrcStreams->m_pStreams[stream_no])
			{
				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z4, pudp_frame_hdr->streamVersion,cust_id);
				fprintf(stdout,"\n z4 version in File :: %d",pudp_frame_hdr->streamVersion);
			}
			if(pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z4 )
			{
				fprintf(stdout,"\n z4 version in File is greater than Sensors,Stopping Execution!!!");
				return FALSE;

			}
			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z4, pudp_frame_hdr->streamVersion,cust_id);
			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
			break;

		case SRR3_Z7B_CUST_LOGGING_DATA:
			if (!m_pSrcStreams->m_pStreams[stream_no])
			{
				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7B_CUSTOM, pudp_frame_hdr->streamVersion,cust_id);
				fprintf(stdout,"\n z7b_Custom version in File :: %d",pudp_frame_hdr->streamVersion);
			}
			if(pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7b_cust )
			{
				fprintf(stdout,"\n z7b_Custom version in File is greater than Sensors,Stopping Execution!!!");
				return FALSE;
			}
			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z7B_CUSTOM, pudp_frame_hdr->streamVersion,cust_id);
			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
			break;

		case Z4_CUST_LOGGING_DATA:
			if (!m_pSrcStreams->m_pStreams[stream_no])
			{
				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z4_CUSTOM, pudp_frame_hdr->streamVersion,cust_id);
				fprintf(stdout,"\n z4_Custom version in File :: %d",pudp_frame_hdr->streamVersion);
			}
			if(pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z4_cust )
			{
				fprintf(stdout,"\n z4_Custom version in File is greater than Sensors,Stopping Execution!!!");
				return FALSE;
			}
			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform,Z4_CUSTOM, pudp_frame_hdr->streamVersion,cust_id);
			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();
			iStrVerNoZ4_Custom = pudp_frame_hdr->streamVersion;
			SourceId_z4 = pudp_frame_hdr->Radar_Position;
			StreamRefIndex=pudp_frame_hdr->streamRefIndex;
			break;

		default:
			pBuffer = NULL;
			break;
		}


		if(pBuffer)
		{

			IRadarStream::reverse_copy_data((unsigned char*)pBuffer,final_buffer,size);
			PackUdpLogging(final_buffer,
				size,
				pudp_frame_hdr->Radar_Position,
				pudp_frame_hdr->streamNumber,
				pudp_frame_hdr->streamRefIndex,
				ver,
				*pDestQueue);

			memset(pBuffer,0,MAX_BACK_BUFFER);
			/*memset((void*)&recordOut,0,sizeof(recordOut));
			memcpy(&recordOut.payload[0], pDestQueue, MAX_MUDP_PACKET_SIZE);
			m_pLogPlugin->SRR3_mudp_write(mudp_log_out,(unsigned char*)&recordOut);	
			ret = 0;*/
		}
	}



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	// If the z4_audi stream not available in older logs, take the data from z4 and fill it to z4_custom and pass it to pDestQueue
	pBuffer = NULL;
	if((0 == iStrVerNoZ4_Custom)) 
	{
		if ((NULL != m_pSrcStreams->m_pStreams[stream_no]) && ((iStrVerNoZ4 < 13) && (iStrVerNoZ4 != 0)) && (1 == m_version_info.strm_ver.z4_cust))
		{
			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
			size    = m_pDestStreams->m_pStreams[stream_no]->get_size();
			ver     = m_pDestStreams->m_pStreams[stream_no]->get_version();

			if(pBuffer)
			{
				PackUdpLogging(pBuffer,
					size,
					SourceId_z4,
					Z4_CUST_LOGGING_DATA,
					StreamRefIndex,
					m_version_info.strm_ver.z4_cust,
					*pDestQueue);
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return ret;
}

int CreateOutputDVSUFile(const char* m_LogFname)
{

	char m_strDVSUOutputFile[1024];

	memset(m_strDVSUOutputFile,0,1024);
	int length = strlen(m_LogFname);
	int TargetLength = length-5; //Remove ".dvsu" from original name
	strncpy(m_strDVSUOutputFile,m_LogFname,TargetLength);
	strcat(m_strDVSUOutputFile,"_Out.dvsu");
	//CHiLController::GetInstance()->m_pLogPlugin->SRR3_mudp_create(m_strDVSUOutputFile,MAX_MUDP_PACKET_SIZE,&mudp_log_out);

	if ( E_MUDPLOG_OK  != m_pLogPlugin->SRR3_mudp_create(m_strDVSUOutputFile,MAX_MUDP_PACKET_SIZE,&mudp_log_out))
		return -1;
	create_ecu_stream_buffers();
	return 0;
}

void create_ecu_stream_buffers()
{
	/*m_pDestStreams->m_pStreams[Z7B_LOGGING_DATA] = (IRadarStream*)z_logging::GetInstance()->Create(21,Z7B, m_version_info.strm_ver.z7b,cust_id);
	m_pDestStreams->m_pStreams[Z7A_LOGGING_DATA] = (IRadarStream*)z_logging::GetInstance()->Create(22,Z7A, m_version_info.strm_ver.z7a,cust_id);
	m_pDestStreams->m_pStreams[Z4_LOGGING_DATA] = (IRadarStream*)z_logging::GetInstance()->Create(23,Z4, m_version_info.strm_ver.z4,cust_id);
	m_pDestStreams->m_pStreams[SRR3_Z7B_CUST_LOGGING_DATA] = (IRadarStream*)z_logging::GetInstance()->Create(24,Z7B_CUSTOM, m_version_info.strm_ver.z7b_cust,cust_id);*/
}

void DumpToOutputDVSU()
{
	const ARSTREAMS& streams = m_TxQueue;
	ARSTREAMS::const_iterator it = streams.begin();

	for(; it != streams.end();++it)
	{	
		memset((void*)&recordOut,0,sizeof(recordOut));
		memcpy(&recordOut.payload[0], ((*it)->GetStreamPtr()), MAX_MUDP_PACKET_SIZE);
		m_pLogPlugin->SRR3_mudp_write(mudp_log_out,(unsigned char*)&recordOut);		
	}
}
