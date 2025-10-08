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
CYW-3073     25-01-2023              Mahendrababu        - Added Multi CSV support for streams data extraction
CYW-3069     31-01-2023              Mahendrababu        - Added Multi CSV support for Header data extraction

******************************************************************************/
#include "stdafx.h"
//#include <conio.h>
#include <stdlib.h>
//#include <windows.h>
#include "PackUDPLogging.h"
#include "ccaVigemDump.h"
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>
#include <time.h>
//#include <io.h>
#include <stdio.h>
#if defined(_WIN_PLATFORM)
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include<chrono>
#endif
#include<matplot/matplot.h>
#include<filesystem>
#include<memory>
//#include <highfive/highfive.hpp>
#include<set>
//using namespace HighFive;

#include "../../../CommonFiles/Utility/commonUtils.h"
#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"
#include "../../../CommonFiles/plugin/PluginLoader.h"
#include "../../../CommonFiles/inc/ByteStream.h"
#include "../../../CommonFiles/inc/common.h"
//#include "../../../CoreLibraryProjects/radar_stream_decoder/Calibration/Calib.h"
#include "time.h"
//#include "MUDP_interface.h"
#include "xml_trace.h"
#include "radar_udp_dump_options.h"
#include "../../../CommonFiles/inc/HiLBaseStream.h"
#include "../../../CommonFiles/cca_vigem_inc/vgm_cca.h"
//#include "C:\Users\vjqv6l\Desktop\cca_ViGEM\v2.1.6\include\vgm_cca.h"
//#include "../../../CoreLibraryProjects/CCA_ViGEM/inc/vgm_cca_log.h"
#include "../../../CoreLibraryProjects/mudp_decoder_calib/CalibDecoder.h"
//#include"../../../CoreLibraryProjects/radar_stream_decoder/Common/IRadarStream.h"
#include "../../Common/IRadarStream.h"

//#include "../../../CommonFiles/inc/Z_Logging.h"
#include "../../Common/Z_Logging.h"
#include "../../../CoreLibraryProjects/mudp_log/MudpRecord/udp_log_struct.h"
#include"../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config_mode.h"
#include "../../../CommonFiles/udp_headers/rr_cal_log.h"
#include "../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"
#include "../../../CommonFiles/CommonHeaders/dph_rr_adas_config_mode.h"
#include "../../../CommonFiles/helper/genutil.h"
//#include "../../../CommonFiles/CommonHeaders/types.h"
#include "../../../CoreLibraryProjects/mdf_log/inc/apt_mdf_log.h"
#include "../../../ApplicationProjects/Radar_UDPData_List_App/radar_udp_dump/GdsrF360InternalDecoder.h"
#include "../../../CoreLibraryProjects/mudp_decoder_calib/TrackerIntenalDecoder.h"
#include "../../CommonFiles/udp_headers/Gen5_Udp_record.h"
#include "../../CommonFiles/udp_headers/Gen7_Udp_record.h"
#include"OSI_Input_Structure.h"
#include "../helper/RadarDecoder.h"

#include "Splitter.h"
#include<filesystem>
#include<memory>
#include "HTML/DataProxy/DataProxy.h"
//#HTMLInMUDP
#include "HTML/InputParser/JsonInputParser.h"
#include "HTML/HTMLReportManager/HTMLReportManager.h"
#include "HTML/DataCollector/STLA_SCALE1.h"
//#include "DataCollector/GPO_GEN7.h"

#pragma comment(lib, "mudp_decoder_calib.lib")
F360_Statistics stat_f360;
GDSR_Statistics stat_gdsr;
bool gdsrpacket_loss_status;
bool f360packet_loss_status;
static uint32_t scanindex_log_quality_prev[MAX_RADAR_COUNT] = { 0 };
static uint32_t scanindex_log_quality_current[MAX_RADAR_COUNT] = { 0 };
static unsigned16_T sensor_log_quality;
#define RADAR_POS 19

z_logging *i_ZRadarStreams = nullptr;

using namespace std;

#pragma region Boost for JSON
#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;
std::shared_ptr<HtmlReportManager>ref_HTML = nullptr;
std::map<std::string, map<int, std::string> > reprocessingInputFileStreams;

std::map<std::string, vector<std::string>> JsonInputFile;
#pragma endregion Boost for JSON

typedef struct UDP_RecordsInfo {
	uint64_t timestamp;
	uint32_t streamRefIndex;
	uint8_t  Radar_Position;
	uint8_t  streamNumber;
	uint8_t  MaxChunks;
	uint8_t  streamChunkIdx;
}UDP_RecordsInfo_t;

static vector<int> m_vec(50);
static int i = 0;

bool envBMW = false;
bool envLinux = false;

char softwareVersion[_MAX_PATH] = { 0 };
char applicationName[_MAX_PATH] = { 0 };
char applicationId[_MAX_PATH] = { 0 };
char sessionId[_MAX_PATH] = { 0 };
char contextId[_MAX_PATH] = { 0 };
char contextName[_MAX_PATH] = { 0 };
char jobId[_MAX_PATH] = { 0 };
unsigned char ManSourceid = 0;
int flag1[MAX_RADAR_COUNT] = { 0 };
int radar_cal_source_curr[MAX_RADAR_COUNT] = { 0 };
int radar_cal_source_prev[MAX_RADAR_COUNT] = { 0 };
int radar_scan_next[MAX_RADAR_COUNT] = { -1 };
int radar_scan_prev[MAX_RADAR_COUNT] = { -1 };
int radar_chunk_id_prev[MAX_RADAR_COUNT] = { 0 };
int radar_chunk_id_curr[MAX_RADAR_COUNT] = { 0 };
int radar_total_chunk_count[MAX_RADAR_COUNT] = { 0 };
int count1[MAX_RADAR_COUNT] = { 0 };
int once1[MAX_RADAR_COUNT] = { 0 };
bool beginSummaryonce = true;
bool endSummaryOnce = true;
bool beginpercentagesummary = true;
bool endpercentagesummary = true;

unsigned int total_scanindex_drop = 0;
int check_scan_coun_flag = 0;

// continuous cycle miss check variables
int nCurrentChunkid[MAX_RADAR_COUNT] = { -1 };
int npreviousChunkid[MAX_RADAR_COUNT] = { -1 };
int nCurrentScan_Index[MAX_RADAR_COUNT] = { -1 };
int missedchunkscan[MAX_RADAR_COUNT] = { -1 };
int setchunkflag[MAX_RADAR_COUNT] = { 0 };
bool checkflag = true;
int nCurrscan[MAX_RADAR_COUNT] = { -1 };

// streamloss info variables
int Flr4pStreamMissinfo[MAX_RADAR_COUNT][FLR4P_MAX_LOGGING_ID] = { -1 };
int scancurrent[MAX_RADAR_COUNT] = { -1 };
int scanprev[MAX_RADAR_COUNT] = { -1 };
int Strmscancurrent[MAX_RADAR_COUNT][MAX_RADAR_COUNT] = { -1 };
int Strmscanprev[MAX_RADAR_COUNT][MAX_RADAR_COUNT] = { -1 };
int strmcurr = -1, strmprev = -1, ignorefirst = 0;;
int Strmbcscancurrent = -1;
int Strmvsescancurrent = -1;
int Strmstatusscancurrent = -1;
int Strmrddscancurrent = -1;
int Strmdetscancurrent= -1; 
int Strmhdrscancurrent = -1;
int StrmTrackerscancurrent = -1;
vector<tuple<string, int, int, int>>Print_Stream_Version_Mismatch_Messages;

int Strmc0_corescancurrent[MAX_RADAR_COUNT] = { -1 };
int Strmc0_custscancurrent[MAX_RADAR_COUNT] = { -1 };
int Strmc1_corescancurrent[MAX_RADAR_COUNT] = { -1 };
int Strmc2_corescancurrent[MAX_RADAR_COUNT] = { -1 };
int Strmc2_custscancurrent[MAX_RADAR_COUNT] = { -1 };

int Strmc0_corescanprev[MAX_RADAR_COUNT] = { -1 };
int Strmc0_custscanprev[MAX_RADAR_COUNT] = { -1 };
int Strmc1_corescanprev[MAX_RADAR_COUNT] = { -1 };
int Strmc2_corescanprev[MAX_RADAR_COUNT] = { -1 };
int Strmc2_custscanprev[MAX_RADAR_COUNT] = { -1 };

int Strmbcscanprev = -1;
int Strmvsescanprev = -1;
int Strmstatusscanprev = -1;
int Strmrddscanprev = -1;
int Strmdetscanprev = -1;
int Strmhdrscanprev = -1;
int StrmTrackerscanprev = -1;


//GEN7 Complete Stream Loss Info Variables
int GEN7_Prev_Stream_Scanindex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 }; 
int GEN7_Curr_Stream_Scanindex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

//GEN7 PCAP Reading
static int Sorted_ScanIndex_Buffer_Count[MAX_RADAR_COUNT] = { 0 };

char  outputPathName[_MAX_PATH] = { 0 };

const int MAX_BACK_BUFFER = 100000;
static SRR3_MUDPLOG_Interface_T*    m_pLogPlugin = NULL;
static SRR3_Decoder_Interface_T*    m_pDecoderPlugin = NULL;
MUDP_CONFIG_T *MUDP_master_config = NULL;
DVSU_RECORD_T recordOut;
map <int, vector<int>> m_map;
int Ecu_scan_next = 0;
int Ecu_scan_prev = 0;// empty multimap container 
IRadarStream * g_pIRadarStrm[MAX_LOGGING_SOURCE] = { 0 };
//unsigned64_T Starttimestamp_t = 0;
Sensor_Version_T m_version_info;
void DumpToOutputDVSU();
void CheckCsvSplitting(std::string csvPath);
void GetCsvSplitFileDetails(Customer_T , unsigned int, unsigned int);

static int Async_KPI_count[MAX_BUSID_NAME] = { 0 };
int CreateOutputDVSUFile(const char* LogFname);
int ExtractAndConvert(const char* LogFname, DVSU_RECORD_T* pRec);
void*  mudp_log_out = NULL;
static sOutputFileNames_T fNameobj;
ARSTREAMS m_TxQueue;
SessionInfo_T SessionInfo;
//Stream_Data_Ptr_T*   m_pDestStreams = new Stream_Data_Ptr_T;
//Stream_Data_Ptr_T*   m_pSrcStreams = new Stream_Data_Ptr_T;
extern bool bOnce[MAX_RADAR_COUNT];
static int once_list[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
#if defined (WIN32) && !defined (__MINGW32__)
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Mdf_Log.lib")

#endif//defined (WIN32) && !defined (__MINGW32__)
//#pragma comment(lib,"radar_stream_decoder.lib")
#include <algorithm>
using namespace std;
namespace fs = std::filesystem; //#HTMLInMUDP
#define MILLI2MICRO(tm)  (tm*1000)
#define Z7A_APPLICATION_TIME (8)
#define Z7B_APPLICATION_TIME (28)
#define THRESHOLD_Z7A_TIME (47)
#define THRESHOLD_Z7B_TIME (48)
#define THRESHOLD_ECU_TIME (50)
#define TIMESTAMP_SYN_STATUS_MASK (0x8000000000000000)
#define MAX_CSV_ROWS 1048576
static int ReadHiLInfrastructure(int argc, char* argv[], SRR3_HiL_Infrastructure_T& hil_param);

void create_ecu_stream_buffers();

static unsigned char g_sensorpos = 21;
#define release_year	24		// The release year in yy format.
#define release_week	43	   // The release week.
static unsigned char release_inf = 19;   // Modify this for every major change (version 2 for new udp building block)
static unsigned char release_op = 0;
static unsigned char release_minor = 22;   //modify this for every minor change
// Version number 8 and 9 should written as 8 and 9 only , not 08 and 09 (Octal Representation)
static unsigned int CS_number = 23083;

bool set_once_flag = FALSE;

//<**Added for Aptiv Header to update in the console (GPC-1449)**>

#define html_release_year	25		// The release year in yy format.
#define html_release_week	01	   // The release week.
static unsigned char html_release_inf = 2;   // Modify this for every major change (version 2 for new udp building block)
static unsigned char html_release_op = 0;
static unsigned char html_release_minor = 3;   //modify this for every minor change
// Version number 8 and 9 should written as 8 and 9 only , not 08 and 09 (Octal Representation)
static unsigned int html_CS_number = 23757;

//<**Added for Aptiv Header to update in the console End (GPC-1449)**>


#define MAX_LEN 2
char *califr_Fname[MAX_LEN] = { 0 };
char *faseth_Fname[MAX_LEN] = { 0 };
char *debug_Fname[MAX_LEN] = { 0 };
char *ref_Fname[MAX_LEN] = { 0 };

int fPathCnt = 0;

int cdc_partially_drop;
bool UDP_Packetloss_Tag_Opened;
bool No_UDP_Data_Flag;
int Dump_MUDP_Status;

string radarEnabled;

char  xml_file_path[1024] = { 0 };
static long SCAN_INDEX_COUNT[MAX_RADAR_COUNT] = { 0 };
static unsigned int ChecksumErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
static unsigned int UDPPacketLossErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
static unsigned int ScanindexDropCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
static unsigned int CRCErrorCount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
static unsigned int TotalscanIndexDropCount_udp[MAX_RADAR_COUNT] = { 0 };
unsigned int TotalscanIndexDropCount_cdc[MAX_RADAR_COUNT] = { 0 };
static unsigned int TotalscanIndex[MAX_RADAR_COUNT] = { 0 };
static unsigned int  SrcTxnDropCount[MAX_RADAR_COUNT] = { 0 };
static unsigned int counter[MAX_RADAR_COUNT] = { 0 };
static int f_Firstscan[MAX_RADAR_COUNT] = { 0 };
static int f_First_refid[MAX_RADAR_COUNT] = { 0 };
static int f_firstdata[MAX_RADAR_COUNT] = { 0 };
static UDPRecord_Header  prev_udp_frame_hdr[MAX_RADAR_COUNT] = { 0 };
static GEN5_UDPRecord_Header_T gen5_prev_udp_frame_hdr[MAX_RADAR_COUNT] = { 0 };
static GEN7_UDPRecord_Header_T gen7_prev_udp_frame_hdr[MAX_RADAR_COUNT] = { 0 };
static int Stream_Index_Prev[MAX_RADAR_COUNT] = { -1 };
Stream_Data_Ptr_T*   s_pSrcStreams[MAX_RADAR_COUNT];
static unsigned int Scan_Index_Buff[MAX_SRR3_SOURCE][MAX_LOGGING_SOURCE] = { 0 };
static bool CreateOnce = true;
static bool CallPktOnce = true;
static bool CreatePktOnce = false;
static bool CreateCsvOnce = true;
static bool DspaceOnce = true;
static bool set_once = true;
unsigned int src_tx_once[MAX_RADAR_COUNT] = { 0 };
//extern bool CreateOnehdr = true;
static bool CreatxmlOnce = true;
static bool CreateHilxmlOnce = true;
static bool CreateCcaCANOnce = true;
unsigned int Timingoverflowcount[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
unsigned int Timingoverrun_Error_print_once = 1;
static bool Enddataflag[MAX_RADAR_COUNT] = { 0 };
//static bool CreateCcaOnce = 0;
static bool CreateCcaUDPOnce = true;
static bool onesTxCnt = false;
static int TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
//static bool Createone = true;
static bool _printonce = true;
static bool printkeyword = true;
static bool srcTxCnt = false;
static bool  UDPflag = false;
static bool  Continuousflag = false;
static bool Cflg = false;
static bool onesCDCScanId[MAX_RADAR_COUNT] = { 0 };
UINT8 setViGEMMf4 = 0;
UINT8 setAuteraMf4 = 0;
UINT8 setMf4 = 0;
static bool F_CDC = true;
static bool F_CALIB = true;
static bool F_C0 = true;
static bool F_C1 = true;
static bool F_C2 = true;
static bool F_C3 = true;
static bool F_C4 = true;
static bool F_C5 = true;

static bool F_STLA_CDC = true;
static bool F_STLA_CALIB = true;
static bool F_STLA_C0 = true;
static bool F_STLA_C1 = true;
static bool F_STLA_C2 = true;
static bool F_STLA_C3 = true;
static bool F_STLA_C4 = true;
static bool F_STLA_C5 = true;

static bool F_BC = true;
static bool F_VSE = true;
static bool F_STATUS = true;
static bool F_RDD = true;
static bool F_DET = true;
static bool F_HDR = true;
static bool F_TRACKER= true;
static bool F_DEBUG = true;
static bool F_ALIGNMENT = true;
static bool F_CALIB_flr4p = true;
static bool F_CDC_flr4p = true;

//GEN7 Print Stream Checks
static int printed_stream_details[40] = { 0 };

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
static bool F_Ecu_VRU = true;
static bool F_dspace = true;
void* mudp_log_in_ptr = NULL;
int Report_Status = 0;
int ScanDrop_status = 0;
static bool IsUDPdata = false;

enum status { CDC_STATUS, 
	OSI_STREAM_STATUS,
	Z4_Core_STATUS,
	Z7A_Core_STATUS,
	Z7B_Core_STATUS,
	Z4_Customer_STATUS,
	Z7B_Customer_STATUS,
	BC_CORE_STATUS,
	VSE_CORE_STATUS,
	STATUS_CORE_STATUS,
	RDD_CORE_STATUS,
	DET_CORE_STATUS,
	HDR_CORE_STATUS,
	TRACKER_CORE_STATUS,
	DEBUG_CORE_STATUS,
	TOTAL_STREAMS};

int strmsizestatus[30] = { 0 };

uint64_t obsoluteTimestamp_t = 0;
//  Stream_Data_Ptr_T*   m_pDestStreams = new Stream_Data_Ptr_T;
//	Stream_Data_Ptr_T*   m_pSrcStreams = new Stream_Data_Ptr_T;
void getDetectionsData(DVSU_RECORD_T* pRec);
bool checkMidHighSensorsEnabled();
CcaSessionReaderOptions sessReaderOpt;
uint64_t  start_timestamp_100ns = 0;
uint64_t  start_timestamp_ns = 0;
//PacketLossTrackerIntenalDecoder *pGdsrF360Tracker;

//extern uint8_t MapSourceToRadarPos(uint8_t  hdrRadarPos);
RADAR_Plugin_Interface_T *m_radar_plugin;
Load_Status_Enum Load_Stream_Decoder(char* plugin_fname, RADAR_Plugin_Interface **ptr);
#ifdef __GNUC__
typedef unsigned int DWORD;

double GetTickCount(void)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now))
		return 0;
	return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

#endif
const char CURSOR[5] = "|/-\\";
static DWORD StartTime = GetTickCount();
static DWORD EndTime = 0;
static int8 idx = 0;
static UDP_FRAME_PAYLOAD_T m_Udp_Payload[4] = { 0 };
typedef enum {
	INVALID_OPT = -1,
	UDP_HDR = 0,
	CCA_HEADER,
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
	CDC_FRM,
	INVALID_STREAM
}enDumpOpt;
typedef enum {
	INVALID_XML_OPT = -1,
	HIL_P_XML = 0,
	SENSOR_P_XML,
	INVALID_XML
}enXmlOpt;
typedef enum {
	INVALID_OPTION = 0,
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
	UDP_RDR_ECU_VRU_Classifier,
	UDP_DSPACE,
	UDP_CCA_HDR,
	UDP_OSI,
	UDP_GEN5_C0_CORE,
	UDP_GEN5_C0_CUST,
	UDP_GEN5_C1_CORE,
	UDP_GEN5_C2_CORE,
	UDP_GEN5_C2_CUST,
	UDP_GEN5_BC_CORE,
	UDP_GEN5_VSE_CORE,
	UDP_GEN5_STATUS_CORE,
	UDP_GEN5_RDD_CORE,
	UDP_GEN5_DET_CORE,
	UDP_GEN5_HDR_CORE,
	UDP_GEN5_TRACKER_CORE,
	UDP_GEN5_DEBUG_CORE,
	UDP_GEN7_DET_STREAM,
	UDP_GEN7_HDR_STREAM,
	UDP_GEN7_STATUS_STREAM,
	UDP_GEN7_RDD_STREAM,
	UDP_GEN7_VSE_STREAM,
	UDP_GEN7_DEBUG_STREAM,
	UDP_GEN7_MMIC_CORE,
	UDP_GEN7_ALIGNMENT_CORE,
	UDP_GEN7_RFFT_CORE,
	UDP_GEN7_BLOCKAGE_CORE,
	UDP_GEN7_ADC_CORE,
	UDP_GEN7_CALIB_CORE,
	UDP_GEN7_RADAR_CAPABILITY_CORE,
	UDP_GEN7_DOWN_SELECTION_CORE,
	UDP_GEN7_ID_CORE,
	UDP_GEN7_TOI_CORE,
	UDP_GEN7_DYNAMIC_ALIGNMENT_CORE,
	UDP_GEN7_ROT_SAFETY_FAULTS_CORE,
	UDP_GEN7_ROT_TRACKER_INFO_CORE,
	UDP_GEN7_ROT_VEHICLE_INFO_CORE,
	UDP_GEN7_ROT_OBJECT_CORE,
	UDP_GEN7_ROT_ISO_OBJECT_CORE,
	UDP_GEN7_ROT_PROCESSED_DETECTION_CORE,
	INVALID_STRM
}streamopt;
 BOOL f_z7b_enabled = 0;
 BOOL f_osi_enabled = 0;
 BOOL f_c0_core_enabled = 0;
 BOOL f_c0_cust_enabled = 0;
 BOOL f_c1_core_enabled = 0;
 BOOL f_c2_core_enabled = 0;
 BOOL f_c2_cust_enabled = 0;
 BOOL f_bc_core_enabled = 0;
 BOOL f_vse_core_enabled = 0;
 BOOL f_status_core_enabled = 0;
 BOOL f_cdc_core_enabled = 0;
 BOOL f_rdd_core_enabled = 0;
 BOOL f_det_core_enabled = 0;
 BOOL f_hdr_core_enabled = 0;
 BOOL f_tracker_core_enabled = 0;
 BOOL f_debug_core_enabled = 0;
 //GEN7
 BOOL f_gen7_mmic_enabled = 0;
 BOOL f_gen7_alignment_enabled = 0;
 BOOL f_gen7_blockage_enabled = 0;
 BOOL f_gen7_radar_capability_enabled = 0;
 BOOL f_gen7_down_selection_enabled = 0;
 BOOL f_gen7_id_enabled = 0;
 BOOL f_gen7_toi_enabled = 0;
 BOOL f_gen7_dynamic_alignment_enabled = 0;
 BOOL f_gen7_calib_enabled = 0;
 BOOL f_gen7_ROT_vehicle_info_enabled = 0;
 BOOL f_gen7_ROT_safety_faults_enabled = 0;
 BOOL f_gen7_ROT_tracker_info_enabled = 0;
 BOOL f_gen7_ROT_object_stream_enabled = 0;
 BOOL f_gen7_ROT_ROT_ISO_object_stream_enabled = 0;
 BOOL f_gen7_ROT_processed_detection_stream_enabled = 0;

BOOL f_Hil_xml = 0;
BOOL f_Sensor_xml = 0;
 BOOL f_z7a_enabled;
 BOOL f_z4_enabled;
 BOOL f_cdc_enabled;
 BOOL f_z4_cust_enabled;
 BOOL f_z7b_custom_enabled;
 BOOL f_z4_custom_enabled;
 extern BOOL f_dspace_enabled;
 BOOL f_ECU0_enabled;
 BOOL f_ECU1_enabled;
 BOOL f_ECU3_enabled;
 BOOL f_ECU_VRU_Classifier_enabled;

 Customer_T customer_id;
//#if defined(_WIN_PLATFORM)
//void split_path(const char* Path,char* Drive,char* Directory,char*Filename,char* Extension)
//{
//	_splitpath_s(Path,Drive,_MAX_DRIVE,Directory,_MAX_DIR,Filename,_MAX_FNAME,Extension,_MAX_EXT);
//}
//#elif defined(__GNUC__)
//void split_pathLinux(const char* Path, char* Drive, char** Directory, char **Filename, char** Extension)
//{
//	char* ts1 = strdup(Path);
//	char* ts2 = strdup(Path);
//
//	char* fullPath = dirname(ts1);
//	strcat(fullPath, "/");
//	char* filenameWithExt = basename(ts2);
//
//	string directory_name(fullPath);
//	std::size_t posDrive = directory_name.find('/', 1);
//
//	string file_name(filenameWithExt);
//	std::size_t found = file_name.find(".");
//
//	char* driveName = strdup(directory_name.substr(0, posDrive).c_str());
//	char* directoryName = strdup(directory_name.substr(posDrive, directory_name.size()).c_str());
//	char* fName = strdup(file_name.substr(0, found).c_str());
//	char* extName = strdup(file_name.substr(found , file_name.size()-1).c_str());
//
//	strcpy(*Drive, driveName);
//	strcpy(*Directory, directoryName);
//	strcpy(*Filename, fName);
//	strcpy(*Extension, extName);
//
//	free(ts1);
//	free(ts2);
//	free(driveName);
//	free(directoryName);
//	free(fName);
//	free(extName);
//}
//#endif
void DumpCalib(FILE* fptr, enDumpOpt opt, const unsigned char* bytestream);

char  inp_file[1024] = { 0 };
char  out_file[1024] = { 0 };
unsigned char f_mode = 0; //0 - single file (/f) , 1 = multifile (/flist)
enDumpOpt dump_option = INVALID_OPT; //0 = /hdr
int dump[INVALID_STRM] = { 0 };
int xml_option[INVALID_XML] = { 0 };

Customer_T cust_id = AUDI;
Customer_T Stream_cust_id = INVALID_CUSTOMER;
enumRadarDecoderOpt e_Options = ALL;

FILE*  fCreate;
FILE*  KCreate = NULL;
FILE*  UdpfCreate = NULL;
FILE*  CanfCreate = NULL;
FILE*  fpDump = NULL;
FILE*  fxml = NULL;
FILE*  hxml = NULL;
FILE*  fpMF4Dump[MAX_RADAR_COUNT][INVALID_STRM] = { 0 };
FILE*  fpxml[MAX_RADAR_COUNT] = { 0 };
FILE * sfpxml = 0;
//FILE*  Hpxml[MAX_RADAR_COUNT] = {0};
FILE*  fpCCAmf4Dump = NULL;
FILE*  fpUdpmf4Dump = NULL;
FILE*  fpCCADump = NULL;
char * CSVFName[MAX_RADAR_COUNT][INVALID_STRM] = { 0 };
char FileNameRL[MAX_FILEPATH_NAME_LENGTH];
char FileNameRR[MAX_FILEPATH_NAME_LENGTH];
char  FileNameFR[MAX_FILEPATH_NAME_LENGTH];
char  FileNameFL[MAX_FILEPATH_NAME_LENGTH];
char  FileName_Pos[MAX_FILEPATH_NAME_LENGTH];
char  XMLFName_Pos[MAX_FILEPATH_NAME_LENGTH];
char  HILXMLFName_Pos[MAX_FILEPATH_NAME_LENGTH];
char  FileName_CCA_Pos[MAX_FILEPATH_NAME_LENGTH];
char  FileCreate_Pos[MAX_FILEPATH_NAME_LENGTH];
char  UDPCreate_Pos[MAX_FILEPATH_NAME_LENGTH];
char  CANCreate_Pos[MAX_FILEPATH_NAME_LENGTH];
static apt_mdfFile*    g_mdfUDP = NULL;
//cca_ViGEM*  g_Cca_DbgViGEM_file = nullptr;
static Radar_UDP_Frame_Latch_T pack_working_prev[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA];
static GEN5_Radar_UDP_Frame_Latch_T gen5_pack_working_prev[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA];
static GEN7_Radar_UDP_Frame_Latch_T gen7_pack_working_prev[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE];
static UDP_RecordsInfo_t udp_records_prev[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE];

static long CDC_ScanIdCurrent[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA] = { 0 };
static long CDC_scan_idx_previous[MAX_RADAR_COUNT][SRR3_Z7B_CUST_LOGGING_DATA] = { 0 };
static char CDC_Chunk_Loss_Index_Track[MAX_RADAR_COUNT][500] = { 0 }; //Keeping track of Chunks Recieved to print the missing Chunk ID in Data Quality Report
static long scan_idx_current[MAX_RADAR_COUNT] = { 0 };
static long refidx_current[MAX_RADAR_COUNT] = { 0 };
static long first_scan_idx[MAX_RADAR_COUNT] = { 0 };
static long first_refidx[MAX_RADAR_COUNT] = { 0 };
static long scan_idx_previous[MAX_RADAR_COUNT] = { -1 };
static long scan_idx_Z_previous[MAX_RADAR_COUNT] = { -1 };
static long refidx_Z_previous[MAX_RADAR_COUNT] = { -1 };
static long scan_idx_M_previous[MAX_RADAR_COUNT] = { -1 };
static long refidx_M_previous[MAX_RADAR_COUNT] = { -1 };
static long scanid_previous[MAX_RADAR_COUNT] = { -1 };
static long g_ScanID_Z_Prev[MAX_RADAR_COUNT] = { -1 };
static int g_ScanID_ECU_Prev = 0;

//----------GEN7 get_scanindex_info api declarations 
static long gen7_scan_idx_current[MAX_RADAR_COUNT] = { 0 };
static long gen7_scan_idx_previous[MAX_RADAR_COUNT] = { -1 };
static long gen7_cdc_scan_idx_previous[MAX_RADAR_COUNT] = { -1 };
//-------------------------------------------------------

float  Percentage_of_scan_idx_Drop[MAX_RADAR_COUNT] = { 0 };
static int once[MAX_RADAR_COUNT] = { 0 };
static int ScanIdCount[MAX_RADAR_COUNT] = { 0 };
static bool ScanIndexCount[MAX_RADAR_COUNT][65555] = { 0 };
static int ScanIdDropCount_udp[MAX_RADAR_COUNT] = { 0 };
static int ScanIdDropCount_cdc[MAX_RADAR_COUNT] = { 0 };
static int CDC_Complete_Drop_ScanID[MAX_RADAR_COUNT] = { 0 };
static int CDC_Chunk_Drop_ScanID[MAX_RADAR_COUNT] = { 0 };
static int ScanIdDropCount_CalibStream[MAX_RADAR_COUNT] = { 0 };
static int maximum = 0;
static int minimum = 0;
static int diff = 0;
//continous drop check
unsigned int Continous_ScanDrop_udp = 0;
unsigned int Continous_ScanDrop_cdc = 0;
unsigned int Continous_ScanDrop_calib = 0;
bool continous_check = false;
static char ScanStatus[MAX_RADAR_COUNT] = { 0 };

typedef struct Streamver_Tag
{
	unsigned short HDR;
	unsigned short CDC;
	unsigned short DSPACE;
	unsigned short CCA_HDR;
	unsigned short OSI_STREAM;
	unsigned short Z4_Core;
	unsigned short Z7A_Core;
	unsigned short Z7B_Core;
	unsigned short Z4_Customer;
	unsigned short Z7B_Customer;
	unsigned short C0_Core;
	unsigned short C0_Cust;
	unsigned short C1_Core;
	unsigned short C2_Core;
	unsigned short C2_Cust;
	unsigned short BC_CORE;
	unsigned short VSE_CORE;
	unsigned short STATUS_CORE;
	unsigned short RDD_CORE;
	unsigned short DET_CORE;
	unsigned short HDR_CORE;
	unsigned short TRACKER_CORE;
	unsigned short DEBUG_CORE;
	unsigned short MMIC_CORE;
	unsigned short ALIGNMENT_CORE;
	unsigned short BLOCKAGE_CORE;
	unsigned short CALIB_CORE;
	unsigned short RADAR_CAPABILITY_CORE;
	unsigned short ROT_SAFETY_FAULTS_CORE;
	unsigned short ROT_TRACKER_INFO_CORE;
	unsigned short ROT_VEHICLE_INFO_CORE;
	unsigned short ROT_OBJECT_CORE;
	unsigned short ROT_ISO_OBJECT_CORE;
	unsigned short ROT_PROCESSED_DETECTION_CORE;
	unsigned short DOWN_SELECTION_CORE;
	unsigned short ID_CORE;
	unsigned short TOI_CORE;
	unsigned short DYNAMIC_ALIGNMENT_CORE;
	unsigned short ecu0;
	unsigned short ecu1;
	unsigned short ecu2;
	unsigned short ECU_VRU_Classifier;

}Streamver_T;

Streamver_T Streamver;
unsigned int StrmValue;
int num_rows[MAX_RADAR_COUNT][INVALID_STRM] = { 0 };
int LogNum[MAX_RADAR_COUNT][INVALID_STRM] = { 0 };

static bool printStrmHeaderOnes = true;
static bool printOnesline = true;
//unsigned char g_RacamUdpBuffer[RACAMUDP_MAX_ST][RACAM_UDP_RECORD_SIZE] ; // fix size for now //Review:;
unsigned char g_RacamUdpBuffer[RACAM_UDP_RECORD_SIZE]; // fix size for now //Review:;
static int ProcessedChunks[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
//static int ExtractAndListDVSU(FILE*pOutFile, int Dump_Option, const char* LogFname, FILE*pxmlFile, FILE*hxmlFile, UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* pRec, bool mf4flag);
static int ExtractAndListDVSU(Streamver_T Streamver, const char* LogFname, FILE **pxmlFile, FILE*hxmlFile, UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* pRec, bool mf4flag, std::shared_ptr<HtmlReportManager>& sptr_manager = ref_HTML, int filecnt = 0);//#HTMLInMUDP
void CDC_Summary();
void GEN7_CDC_Summary();
static int ExtractPacketLoss(FILE*pOutFile, const char* LogFname, UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* record, bool mf4flag);
static int ExtractAndListMUDP(FILE*pOutFile, enDumpOpt opt, UDPLogStruct* pLogStruct);





typedef struct scandrop_Tag
{
	std::string BMW_Status;
	std::string Observation;
	std::string Status;
	std::string log_status;
	std::string CDC_Saturation_Status;
	std::string  DQ_Radar_Fail_Status[MAX_RADAR_COUNT];
	std::string  RESIM_Radar_Fail_Status[MAX_RADAR_COUNT];
	unsigned int  ChecksumErrorCount[MAX_RADAR_COUNT];
	unsigned int Checksumcount_Sensors;
	unsigned int Checksumcount_ECU;
	unsigned int  ChecksumErrorCount_F360;
	unsigned int  ChecksumErrorCount_GDSR;
	unsigned int total_scanindex;
	int udp_scandrop[MAX_RADAR_COUNT];
	int cdc_scandrop[MAX_RADAR_COUNT];
	int calib_scandrop[MAX_RADAR_COUNT];
	int Total_Scan_Index[MAX_RADAR_COUNT];
	int CDC_Max_Chunks[MAX_RADAR_COUNT];
	int F360_ScanDrop[MAX_RADAR_COUNT];
	int GDSR_ScanDrop[MAX_RADAR_COUNT];
	int udp_scanindex_drop;
	int cdc_scanindex_drop;
	int calib_scanindex_drop;
	int F360_scanindex_drop;
	int gdsr_scanindex_drop;
	int log_cdc_partially_drop;

}scandrop_T;


std::map<std::string, scandrop_T> loggedmap;
std::map<int, std::map<std::string, scandrop_T>> Summerymap;
#if defined(_WIN_PLATFORM)
char logfilename[_MAX_FNAME];
#elif defined(__GNUC__)
char *logfilename = new char[_MAX_FNAME];
#endif
int count_flag = 0;
int inc_flag = 0;
int count_flag_log = 0;
int cdc_count_flag = 0;

std::vector<csvDetails> gcsvDetails;

void setstreammismatchcount(int stream_no)
{
	switch (stream_no)
	{
	case 0:
		strmsizestatus[0]++;
		break;
	case 1:
		strmsizestatus[1]++;
		break;
	case 2:
		strmsizestatus[2]++;
		break;
	case 3:
		strmsizestatus[3]++;
		break;
	case 4:
		strmsizestatus[4]++;
		break;
	case 5:
		strmsizestatus[5]++;
		break;
	case 6:
		strmsizestatus[6]++;
		break;
	case 7:
		strmsizestatus[7]++;
		break;
	case 8:
		strmsizestatus[8]++;
		break;
	case 9:
		strmsizestatus[9]++;
		break;
	case 10:
		strmsizestatus[10]++;
		break;
	case 11:
		strmsizestatus[11]++;
		break;
	case 12:
		strmsizestatus[12]++;
		break;
	case 13:
		strmsizestatus[13]++;
		break;
	case 14:
		strmsizestatus[14]++;
		break;
	case 15:
		strmsizestatus[15]++;
		break;
	case 16:
		strmsizestatus[16]++;
		break;
	case 17:
		strmsizestatus[17]++;
		break;
	case 18:
		strmsizestatus[18]++;
		break;
	case 19:
		strmsizestatus[19]++;
		break;
	case 20:
		strmsizestatus[20]++;
		break;
	case 21:
		strmsizestatus[21]++;
		break;
	case 22:
		strmsizestatus[22]++;
		break;
	case 23:
		strmsizestatus[23]++;
		break;
	case 24:
		strmsizestatus[24]++;
		break;
	case 25:
		strmsizestatus[25]++;
		break;
	case 26:
		strmsizestatus[26]++;
		break;
	case 27:
		strmsizestatus[27]++;
		break;
	case 28:
		strmsizestatus[28]++;
		break;
	case 29:
		strmsizestatus[29]++;
		break;
	
	default:
		printf("\nunknown stream %d size mismatch\n",stream_no) ;
	}
}
void printCursor()
{
	EndTime = GetTickCount();
	if (EndTime - StartTime >= 70)
	{
		StartTime = EndTime;
		printf("\r%c  ", CURSOR[idx]);
		if (++idx > 4)
			idx = 0;
	}
}
static char* GetCustIDName(uint8_t custID, uint8_t platform) {
		static char customerName[40] = { 0 };
	if ((custID == STLA_SCALE1 || custID == STLA_SCALE3 || custID == STLA_SCALE4) && (platform == UDP_SOURCE_CUST_DSPACE))
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
		case CHANGAN_SRR5: sprintf(customerName, "CHANGAN_SRR5"); break;
		case HKMC_SRR: sprintf(customerName, "HKMC_SRR"); break;
		case SCANIA_MAN: sprintf(customerName, "MAN_SRR5"); break;
		//GEN6 Customers
		case NISSAN_GEN5: sprintf(customerName, "NISSAN_GEN5"); break;
		case PLATFORM_GEN5:
			if (platform == UDP_PLATFORM_SRR6)
				sprintf(customerName, "GPO_SRR6");
			else if (platform == UDP_PLATFORM_SRR6 ||platform == UDP_PLATFORM_SRR6_PLUS ||platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN
				||platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH ||platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
				sprintf(customerName, "GPO_SRR6P"); 
			else if(platform == UDP_PLATFORM_STLA_FLR4)
				sprintf(customerName, "GPO_FLR4");
			else if (platform == UDP_PLATFORM_FLR4_PLUS || platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
				sprintf(customerName, "GPO_FLR4P");
			break;
		case HONDA_GEN5:sprintf(customerName, "HONDA_GEN5"); break;
		case MOTIONAL_FLR4P:sprintf(customerName, "MOTIONAL_FLR4P"); break;
		case TML_SRR5: sprintf(customerName, "TML_SRR5"); break;
		case STLA_SRR6P:sprintf(customerName, "STLA_SRR6P"); break;
		case STLA_FLR4:sprintf(customerName, "STLA_FLR4"); break;
		case STLA_FLR4P:sprintf(customerName, "STLA_FLR4P"); break;
		case STLA_SCALE4:sprintf(customerName, "STLA_SCALE4"); break;
		case BMW_SP25_L2:sprintf(customerName, "BMW_SP25_L2"); break;
		case BMW_SP25_L3:sprintf(customerName, "BMW_SP25_L3"); break;
		case RIVIAN_SRR6P:sprintf(customerName, "RIVIAN_SRR6P"); break;
		//GEN7 Customers
		case PLATFORM_GEN7:sprintf(customerName, "PLATFORM_GEN7"); break;
		default: sprintf(customerName, "INVALID_CUSTOMER"); break;
		}
	}
	
	return customerName;
}
char* GetBusIdName(uint32_t Busid) {
	static char BusidName[60] = { 0 };
	if (Busid != 0) {
		switch (Busid) {
		case 542: sprintf(BusidName, "Low_H_CAN_FD"); break;
		case 545: sprintf(BusidName, "Low_V_CAN_FD"); break;
		case 543: sprintf(BusidName, "MID_RL_CAN_FD"); break;
		case 544: sprintf(BusidName, "MID_RR_CAN_FD"); break;
		case 546: sprintf(BusidName, "MID_FL_CAN_FD"); break;
		case 547: sprintf(BusidName, "MID_FR_CAN_FD"); break;
		case 526: sprintf(BusidName, "FAS_CAN_FD"); break;
		case 548: sprintf(BusidName, "Sensor_CAN_FD"); break;
		case 549: sprintf(BusidName, "SRR_L_CAN_FD"); break;
		case 550: sprintf(BusidName, "SRR_R_CAN_FD"); break;
		case 466: sprintf(BusidName, "FAS_CAN"); break;
		case 356: sprintf(BusidName, "FA_CAN_1"); break;
		case 424: sprintf(BusidName, "Body2_CAN"); break;
		case 358: sprintf(BusidName, "Flexray"); break;
		case 69634: sprintf(BusidName, "Rear_Left"); break;
		case 69635: sprintf(BusidName, "Rear_Right"); break;
		case 69633: sprintf(BusidName, "Front_Right"); break;
		case 69632: sprintf(BusidName, "Front_Left"); break;
		case 69636: sprintf(BusidName, "Bpil_Left"); break;
		case 69637: sprintf(BusidName, "Bpil_Right"); break;
		case 69638: sprintf(BusidName, "ECU"); break;
		default: sprintf(BusidName, "INVALID_RADAR"); break;
		}
	}
	return BusidName;
	//return sourceid;
}

char* GetAsyncBusIdName(uint32_t Busid) {
	static char BusidName[60] = { 0 };

	switch (Busid) {
	case 4: sprintf(BusidName, "Low_H_CAN_FD"); break;
	case 8: sprintf(BusidName, "Low_V_CAN_FD"); break;
	case 9: sprintf(BusidName, "MID_RL_CAN_FD"); break;
	case 10: sprintf(BusidName, "MID_RR_CAN_FD"); break;
	case 11: sprintf(BusidName, "MID_FL_CAN_FD"); break;
	case 12: sprintf(BusidName, "MID_FR_CAN_FD"); break;
	case 13: sprintf(BusidName, "FAS_CAN_FD"); break;
	case 14: sprintf(BusidName, "Sensor_CAN_FD"); break;
	case 15: sprintf(BusidName, "SRR_L_CAN_FD"); break;
	case 16: sprintf(BusidName, "SRR_R_CAN_FD"); break;
	case 20: sprintf(BusidName, "FAS_CAN"); break;
	case 21: sprintf(BusidName, "FA_CAN_1"); break;
	case 22: sprintf(BusidName, "Body2_CAN"); break;
	case 0: sprintf(BusidName, "Rear_Left"); break;
	case 1: sprintf(BusidName, "Rear_Right"); break;
	case 2: sprintf(BusidName, "Front_Right"); break;
	case 3: sprintf(BusidName, "Front_Left"); break;
	case 6: sprintf(BusidName, "Bpil_Left"); break;
	case 7: sprintf(BusidName, "Bpil_Right"); break;
	case 19: sprintf(BusidName, "ECU"); break;
	default: sprintf(BusidName, "INVALID_RADAR"); break;
	}

	return BusidName;
	//return sourceid;
}

//int Getsourcename(uint32_t Busid,int sourceid)
//{
//	if(Busid == 69634)
//	{
//		sourceid = 0;
//	}
//	else
//		if(Busid == 69635)
//	{
//			sourceid = 1;
//	}
//		return sourceid;
//}
#if defined(_WIN_PLATFORM)
int CreatePathDir(const char *path)
{
	int status = 0;
	if (path != NULL)
	{
		status = CreateDirectory(path, NULL);
		return status;
	}
}
#endif

char* GetGEN7streamNumberName(uint8_t streamNo)
{
	static char strRet[100] = { 0 };
	static char str_1[] = "01_DETECTION_STREAM";
	static char str_2[] = "02_HEADER_STREAM";
	static char str_3[] = "03_STATUS_STREAM";
	static char str_4[] = "04_RDD_STREAM";
	static char str_5[] = "05_VSE_STREAM";
	static char str_6[] = "06_CDC_STREAM";
	static char str_7[] = "07_DEBUG_STREAM";
	static char str_8[] = "08_MMIC_STREAM";
	static char str_9[] = "09_ALIGNMENT_STREAM";
	static char str_11[] = "11_RFFT_STREAM";
	static char str_12[] = "12_BLOCKAGE_STREAM";
	static char str_13[] = "13_ADC_STREAM";
	static char str_14[] = "14_CALIB_STREAM";
	static char str_20[] = "20_RADAR_CAPABILITY_STREAM";
	static char str_21[] = "21_DOWN_SELECTION_STREAM";
	static char str_30[] = "30_ROT_OBJECT_STREAM";
	static char str_31[] = "31_ROT_ISO_OBJECT_STREAM";
	static char str_32[] = "32_ROT_PROCESSED_DETECTION_STREAM";
	static char str_33[] = "33_ROT_TRACKER_INFO";
	static char str_34[] = "34_ROT_VEHICLE_INFO";
	static char str_35[] = "35_ROT_SAFETY_FAULTS";
	static char str_36[] = "36_ID_STREAM";
	static char str_40[] = "40_TOI_STREAM";
	static char str_45[] = "45_DYNAMIC_ALIGNMENT_STREAM";

	switch (streamNo)
	{
	case 1: return str_1;
	case 2: return str_2;
	case 3: return str_3;
	case 4: return str_4;
	case 5: return str_5;
	case 6: return str_6;
	case 7: return str_7;
	case 8: return str_8;
	case 9: return str_9;
	case 11: return str_11;
	case 12: return str_12;
	case 13: return str_13;
	case 14: return str_14;
	case 20: return str_20;
	case 21: return str_21;
	case 30: return str_30;
	case 31: return str_31;
	case 32: return str_32;
	case 33: return str_33;
	case 34: return str_34;
	case 35: return str_35;
	case 36: return str_36;
	case 40: return str_40;
	case 45: return str_45;
	default:sprintf(strRet, "UNKNOWN_STREAM_NUMBER_%d", streamNo); break;

	}
	return strRet;
}

char* Get_GEN7StreamName(unsigned int SteamNo)
{
	static char StreamName[40] = { 0 };
	switch (SteamNo) {
	case DETECTION_STREAM: sprintf(StreamName, "DETECTION_STREAM"); break;
	case HEADER_STREAM: sprintf(StreamName, "HEADER_STREAM"); break;
	case STATUS_STREAM: sprintf(StreamName, "STATUS_STREAM"); break;
	case RDD_STREAM: sprintf(StreamName, "RDD_STREAM"); break;
	case VSE_STREAM: sprintf(StreamName, "VSE_STREAM"); break;
	case CDC_STREAM: sprintf(StreamName, "CDC_STREAM"); break;
	case DEBUG_STREAM: sprintf(StreamName, "DEBUG_STREAM"); break;
	case MMIC_STREAM: sprintf(StreamName, "MMIC_STREAM"); break;
	case ALIGNMENT_STREAM: sprintf(StreamName, "ALIGNMENT_STREAM"); break;
	case RFFT_STREAM: sprintf(StreamName, "RFFT_STREAM"); break;
	case BLOCKAGE_STREAM: sprintf(StreamName, "BLOCKAGE_STREAM"); break;
	case ADC_STREAM: sprintf(StreamName, "ADC_STREAM"); break;
	case CALIB_STREAM: sprintf(StreamName, "CALIB_STREAM"); break;
	case RADAR_CAPABILITY_STREAM: sprintf(StreamName, "RADAR_CAPABILITY_STREAM"); break;
	case ROT_SAFETY_FAULTS_STREAM:sprintf(StreamName, "ROT_SAFETY_FAULTS"); break;
	case ROT_TRACKER_INFO_STREAM:sprintf(StreamName, "ROT_TRACKER_INFO"); break;
	case ROT_VEHICLE_INFO_STREAM:sprintf(StreamName, "ROT_VEHICLE_INFO"); break;
	case ROT_OBJECT_STREAM:sprintf(StreamName, "ROT_OBJECT_STREAM"); break;
	case ROT_ISO_OBJECT_STREAM:sprintf(StreamName, "ROT_ISO_OBJECT_STREAM"); break;
	case ROT_PROCESSED_DETECTION_STREAM:sprintf(StreamName, "ROT_PROCESSED_DETECTION_STREAM"); break;
	case DOWN_SELECTION_STREAM: sprintf(StreamName, "DOWN_SELECTION_STREAM"); break;
	case ID_STREAM: sprintf(StreamName, "ID_STREAM"); break;
	case TOI_STREAM: sprintf(StreamName, "TOI_STREAM"); break;
	case DYNAMIC_ALIGNMENT_STREAM: sprintf(StreamName, "DYNAMIC_ALIGNMENT_STREAM"); break;
	case ROT_INTERNALS_STREAM: sprintf(StreamName, "ROT_INTERNALS_STREAM"); break;
	default: sprintf(StreamName, "INVALID_STREAM : %d", SteamNo); break;
	}
	return StreamName;

}

 char* GetSRR6streamNumber(uint8_t streamNo)
{
	static char strRet[100] = { 0 };
	static char str_0[] = "00_C0_RESIM_CORE_MASTER_STREAM";
	static char str_1[] = "01_C0_RESIM_CUST_MASTER_STREAM";
	static char str_2[] = "02_C1_RESIM_CORE_MASTER_STREAM";
	static char str_3[] = "03_C2_RESIM_CORE_MASTER_STREAM";
	static char str_4[] = "04_C2_RESIM_CUST_MASTER_STREAM";
	static char str_15[] = "15-CDC_8_IQ";
	static char str_16[] = "16_CDC_12_IQ";
	static char str_96[] = "96_CALIB_STREAM";

	switch (streamNo)
	{
	case 0: return str_0;
	case 1: return str_1;
	case 2: return str_2;
	case 3: return str_3;
	case 4: return str_4;
	case 15: return str_15;
	case 16: return str_16;
	case 96: return str_96;
	default:sprintf(strRet, "UNKNOWN_STREAM_NUMBER_%d", streamNo); break;

	}
	return strRet;
}

char * GetStreamNumberName(uint8_t Stream_Num) {
	static char	Str_Name[40] = { 0 };
	switch (Stream_Num) {
	case	CDC_8_IQ:
	case	CDC_DATA:
		sprintf(Str_Name, "CDC_DATA"); break;
	case   Z7A_Z7B_IPC_SRR5_116_CORE:
	case Z7A_Z7B_IPC_SRR5_210_CORE:
	case Z7A_Z7B_IPC_MRR360_116RBIN_64D_CORE:
	case Z7A_Z7B_IPC_MRR360_116RBIN_200D_CORE:
	case Z7A_Z7B_IPC_MRR360_116RBIN_150D_CORE:
	case Z7A_SRR5P_116RB_64D_CORE:
	case Z7A_SRR5P_116RB_150D_CORE:
	case Z7A_SRR5P_116RB_200D_CORE:
	case Z7A_LOGGING_DATA:
		sprintf(Str_Name, "Z7A_LOGGING_DATA"); break;
	case Z7B_SIDE_64D_SAT_CORE:
	case Z7B_SIDE_200D_SAT_CORE:
	case Z7B_FRNT_128D_SAT_CORE:
	case Z7B_64SD_64FD_64TRK_STAND_CORE:
	case Z7B_64SD_64TRK_STAND_CORE:
	case Z7B_BPIL_150D_64T_CORE:
	case Z7B_FRNT_64SD_64FD_SAT_CORE:
	case Z7B_LOGGING_DATA:
		sprintf(Str_Name, "Z7B_LOGGING_DATA"); break;
	case Z4_LOGGING_DATA:
	case Z4_Z7B_CORE:
	case Z4_Z7B_64_FD_CORE:
		sprintf(Str_Name, "Z4_LOGGING_DATA"); break;
	case SRR3_Z7B_CUST_LOGGING_DATA:
	case Z7B_CUST: sprintf(Str_Name, "Z7B_CUST_LOGGING_DATA"); break;
	case Z4_CUST_LOGGING_DATA:
	case Z4_CUST:
		sprintf(Str_Name, "Z4_CUST_LOGGING_DATA"); break;
	case ECU_CORE0: sprintf(Str_Name, "ECU_CORE0"); break;
	case ECU_CORE1: sprintf(Str_Name, "ECU_CORE1"); break;
	case ECU_CORE3: sprintf(Str_Name, "ECU_CORE3"); break;
	case ECU_OG: sprintf(Str_Name, "ECU_OG"); break;
	case ECU_CAL_STREAM: sprintf(Str_Name, "ECU_CAL_STREAM"); break;
	case ECU_INTERNAL_CORE1: sprintf(Str_Name, "F360_TRACKER"); break;
	case ECU_VRU_CLASSIFIER: sprintf(Str_Name, "ECU_VRU_CLASSIFIER"); break;
	case GDSR_TRACKER_INTERNAL: sprintf(Str_Name, "GDSR_TRACKER"); break;
	case OSI_STREAM:sprintf(Str_Name, "OSI_TRACKER"); break;

	default: sprintf(Str_Name, "%u_INVALID_STREAM", Stream_Num); break;
	}
	return Str_Name;
}

char* Get_FLR4PGEN5StreamName(unsigned int SteamNo)
{
	static char StreamName[40] = { 0 };
	switch (SteamNo) {
	case e_BC_CORE_LOGGING_STREAM: sprintf(StreamName, "BC_CORE_LOGGING_STREAM"); break;
	case e_VSE_CORE_LOGGING_STREAM: sprintf(StreamName, "VSE_CORE_LOGGING_STREAM"); break;
	case e_STATUS_CORE_LOGGING_STREAM: sprintf(StreamName, "STATUS_CORE_LOGGING_STREAM"); break;
	case e_RDD_CORE_LOGGING_STREAM: sprintf(StreamName, "RDD_CORE_LOGGING_STREAM"); break;
	case e_DET_LOGGING_STREAM: sprintf(StreamName, "DET_LOGGING_STREAM"); break;
	case e_HDR_LOGGING_STREAM: sprintf(StreamName, "HDR_LOGGING_STREAM"); break;
	case e_CDC_LOGGING_STREAM: sprintf(StreamName, "CDC_DATA"); break;
	case e_TRACKER_LOGGING_STREAM: sprintf(StreamName, "TRACKER_LOGGING_STREAM"); break;
	case e_DEBUG_LOGGING_STREAM: sprintf(StreamName, "DEBUG_LOGGING_STREAM"); break;
	case e_ALIGNMENT_LOGGING_STREAM: sprintf(StreamName, "ALIGNMENT_LOGGING_STREAM"); break;
	case DSPACE_CUSTOMER_DATA:sprintf(StreamName, "DSPACE_CUSTOMER_DATA"); break;
	case e_CALIB_STREAM: sprintf(StreamName, "CALIB_STREAM"); break;
	default: sprintf(StreamName, "INVALID_STREAM : %d", SteamNo); break;
	}
	return StreamName;

}

char* Get_GEN5StreamName(unsigned int SteamNo)
{
	static char StreamName[40] = { 0 };
	switch (SteamNo) {
	case C0_CORE_MASTER_STREAM: sprintf(StreamName, "C0_CORE_MASTER_STREAM"); break;
	case C0_CUST_MASTER_STREAM: sprintf(StreamName, "C0_CUST_MASTER_STREAM"); break;
	case C1_CORE_MASTER_STREAM: sprintf(StreamName, "C1_CORE_MASTER_STREAM"); break;
	case C2_CORE_MASTER_STREAM: sprintf(StreamName, "C2_CORE_MASTER_STREAM"); break;
	case GEN5_CALIB_STREAM: sprintf(StreamName, "CALIB_STREAM"); break;
	case C2_CUST_MASTER_STREAM: sprintf(StreamName, "C2_CUST_MASTER_STREAM"); break;
	case OSI_GEN6_STREAM: sprintf(StreamName, "OSI_GEN6_STREAM"); break;
	case CDC_STREAM: sprintf(StreamName, "CDC_DATA"); break;
	case DSPACE_CUSTOMER_DATA:sprintf(StreamName, "DSPACE_CUSTOMER_DATA"); break;
	default: sprintf(StreamName, "INVALID_STREAM : %d", SteamNo); break;
	}
	return StreamName;

}

char* GetStreamName(unsigned int SteamNo) {
	static char StreamName[40] = { 0 };
	switch (SteamNo) {
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
	case RADAR_ECU_VRU_CLASSIFIER: sprintf(StreamName, "ECU_VRU_CLASSIFIER"); break;
	case SENSOR_GDSR_TRACKER_INTERNAL: sprintf(StreamName, "GDSR_TRACKER"); break;
	case SENSOR_OSI_TRACKER:sprintf(StreamName, "OSI_TRACKER"); break;
	default: sprintf(StreamName, "INVALID_STREAM : %d", SteamNo); break;
	}
	return StreamName;
}
static int MaptoEcustreamNumber(uint8_t Strm)
{
	int retStream = 0xFF;
	switch (Strm)
	{
	case ECU_CORE0:			retStream = RADAR_ECU_CORE_0;	break;
	case ECU_CORE1:			retStream = RADAR_ECU_CORE_1;	break;
	case ECU_CORE3:			retStream = RADAR_ECU_CORE_3;	break;
	case ECU_OG:			retStream = RADAR_ECU_OG;		break;
	case ECU_CAL_STREAM:	retStream = RADAR_ECU_CALIB;	break;
	case ECU_INTERNAL_CORE1:retStream = RADAR_ECU_INTERNAL_CORE1;	break;
	case ECU_VRU_CLASSIFIER:	retStream = RADAR_ECU_VRU_CLASSIFIER;	break;
	default:												break;
	}
	return retStream;
}
static uint8_t MapSourceToRadarPos(uint8_t  hdrRadarPos)
{
	uint8_t pos = 0xFF;
	// For SRR3 sensors, radar position is derived from the sourceInfo of UDP header: 21, 22, 23, 24
	if (hdrRadarPos >= UDP_PLATFORM_CUST_SRR3_RL && hdrRadarPos <= UDP_PLATFORM_CUST_SRR3_FC) {
		pos = (uint8_t)(hdrRadarPos - UDP_PLATFORM_CUST_SRR3_RL);
	}
	// For DSPACE UDP logs, radar position is hardcoded to 18
	else if (hdrRadarPos == UDP_SOURCE_CUST_DSPACE) {
		pos = RDR_POS_DSPACE;
	}
	else if (hdrRadarPos == BPILLAR_STATUS_BP_L || hdrRadarPos == UDP_PLATFORM_FORD_SRR5) {
		pos = RDR_POS_FC;
	}
	else if (hdrRadarPos == BPILLAR_STATUS_BP_R || hdrRadarPos == UDP_PLATFORM_FORD_SRR5) {
		pos = RDR_POS_FC;
	}
	else {
		pos = (uint8_t)(hdrRadarPos - 1);
	}
	return pos;
}
static int MaptoBusid(uint32_t busid) {
	int BUSId = 0xFF;
	switch (busid)
	{
	case 542: BUSId = 4; break;
	case 545: BUSId = 8; break;
	case 543: BUSId = 9; break;
	case 544: BUSId = 10; break;
	case 546: BUSId = 11; break;
	case 547: BUSId = 12; break;
	case 526: BUSId = 13; break;
	case 548: BUSId = 14; break;
	case 549: BUSId = 15; break;
	case 550: BUSId = 16; break;
	case 466: BUSId = 20; break;
	case 356: BUSId = 21; break;
	case 424: BUSId = 22; break;
	case 358: BUSId = 23; break;
	case 69634:BUSId = 0; break;
	case 69635: BUSId = 1; break;
	case 69633: BUSId = 2; break;
	case 69632: BUSId = 3; break;
	case 69636: BUSId = 6; break;
	case 69637: BUSId = 7; break;
	case 69638: BUSId = 19; break;
	default: BUSId = 17; break;
	}
	return BUSId;
}
static int MaptostreamNumber(uint8_t Strm, uint8_t CustID)
{
	int retStream = 0xFF;
	switch (CustID)
	{
	case (RNA_CDV):
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_210_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64TRK_STAND_CORE:	retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_64_FD_CORE:			retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case (RNA_SUV):
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_210_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64TRK_STAND_CORE:	retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_64_FD_CORE:			retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case HKMC_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:		retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64FD_64TRK_STAND_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:					retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:						retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:						retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:						retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case BMW_LOW:
		switch (Strm)
		{
		case Z7A_SRR5P_116RB_64D_CORE:retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64FD_64TRK_STAND_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		case CDC_8_IQ:retStream = CDC_DATA; break;
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case BMW_SAT:
		switch (Strm)
		{
		case Z7A_SRR5P_116RB_200D_CORE:retStream = Z7A_LOGGING_DATA; break;
		case Z7B_SIDE_200D_SAT_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		case CDC_8_IQ:retStream = CDC_DATA; break;
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;

	case BMW_BPIL:
		switch (Strm)
		{
		case Z7A_SRR5P_116RB_150D_CORE:retStream = Z7A_LOGGING_DATA; break;
		case Z7B_BPIL_150D_64T_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:retStream = Z4_CUST_LOGGING_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		case CDC_8_IQ:retStream = CDC_DATA; break;
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case GWM_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:		retStream = Z7A_LOGGING_DATA; break;
		case Z7B_FRNT_64SD_64FD_SAT_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:					retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:						retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:						retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:						retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case GEELY_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_FRNT_128D_SAT_CORE:	retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case CHANGAN_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_FRNT_128D_SAT_CORE:	retStream = Z7B_LOGGING_DATA; break;
		case Z7B_FRNT_64SD_64FD_SAT_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case SGM_358_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_FRNT_128D_SAT_CORE:	retStream = Z7B_LOGGING_DATA; break;
		case Z7B_FRNT_64SD_64FD_SAT_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		default: break;
		}
	case FORD_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:	retStream = Z7A_LOGGING_DATA; break;
		case Z7B_SIDE_64D_SAT_CORE:		retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:				retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:					retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:					retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:					retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case HKMC_SRR:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:				retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64FD_64OBJ_STAND_CORE:		retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:							retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:								retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:								retStream = Z4_CUST_LOGGING_DATA; break;
		case  CDC_8_IQ:								retStream = CDC_DATA; break;//
		case CALIBRATION_DATA:						retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:					retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:							retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case SCANIA_MAN:
		switch (Strm)
		{
		case Z7A_CORE_0:								retStream = Z7A_LOGGING_DATA; break;
		case Z7B_CORE_1:								retStream = Z7B_LOGGING_DATA; break;
		case Z4_CORE_3:								retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST_7:								retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST_9:								retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_SRR3_6:								retStream = CDC_DATA; break;
		case CALIB_STREAM_4:							retStream = CALIBRATION_DATA; break;
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case TML_SRR5:
		switch (Strm)
		{
		case Z7A_Z7B_IPC_SRR5_116_CORE:		retStream = Z7A_LOGGING_DATA; break;
		case Z7B_64SD_64FD_64OBJ_STAND_CORE:retStream = Z7B_LOGGING_DATA; break;
		case Z4_Z7B_CORE:					retStream = Z4_LOGGING_DATA; break;
		case Z7B_CUST:						retStream = SRR3_Z7B_CUST_LOGGING_DATA; break;
		case Z4_CUST:						retStream = Z4_CUST_LOGGING_DATA; break;
		case CDC_8_IQ:						retStream = CDC_DATA; break;
		case CALIBRATION_DATA:			retStream = CALIBRATION_DATA; break;
		case GDSR_TRACKER_INTERNAL:		retStream = SENSOR_GDSR_TRACKER_INTERNAL; break;
		case OSI_STREAM:                  retStream = SENSOR_OSI_TRACKER; break;
		default: break;
		}
		break;
	case BMW_SP25_L2:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case BMW_SP25_L3:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case STLA_SRR6P:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case STLA_FLR4:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case STLA_FLR4P:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
	case PLATFORM_GEN5:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
	case STLA_SCALE4:
		switch (Strm)
		{
		case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
		default: break;
		}
		break;
		case RIVIAN_SRR6P:
			switch (Strm)
			{
			case DSPACE_CUSTOMER_DATA: retStream = DSPACE_CUSTOMER_DATA; break;
			default: break;
			}
		break;
	}
	return retStream;
}

static char* GetstreamIDName(int streamId) {
	static char StreamName[40] = { 0 };

	switch (streamId) {
	case UDP_Z7B_CORE: sprintf(StreamName, "UDP_Z7B"); break;
	case HEADER: sprintf(StreamName, "UDP_HDR"); break;
	case UDP_Z7A_CORE: sprintf(StreamName, "UDP_Z7A"); break;
	case UDP_Z4_CORE: sprintf(StreamName, "UDP_Z4"); break;
	case UDP_CDC_STRM: sprintf(StreamName, "UDP_CDC"); break;
	case UDP_CUSTOM_Z7B: sprintf(StreamName, "UDP_Z7B_CUSTOM"); break;
	case UDP_CUSTOM_Z4: sprintf(StreamName, "UDP_Z4_CUSTOM"); break;
	case UDP_RDR_ECU0: sprintf(StreamName, "UDP_ECU0"); break;
	case UDP_DSPACE:sprintf(StreamName, "DSPACE"); break;
	case UDP_RDR_ECU1: sprintf(StreamName, "UDP_ECU1"); break;
	case UDP_RDR_ECU3: sprintf(StreamName, "UDP_ECU3"); break;
	case UDP_RDR_ECU_VRU_Classifier: sprintf(StreamName, "UDP_ECU_VRU_Classifier"); break;
	case UDP_OSI: sprintf(StreamName, "UDP_OSI"); break;
	case UDP_GEN5_C0_CORE: sprintf(StreamName, "UDP_GEN5_C0_CORE"); break;
	case UDP_GEN5_C0_CUST: sprintf(StreamName, "UDP_GEN5_C0_CUST"); break;
	case UDP_GEN5_C1_CORE: sprintf(StreamName, "UDP_GEN5_C1_CORE"); break;
	case UDP_GEN5_C2_CORE: sprintf(StreamName, "UDP_GEN5_C2_CORE"); break;
	case UDP_GEN5_C2_CUST: sprintf(StreamName, "UDP_GEN5_C2_CUST"); break;
	case UDP_GEN5_BC_CORE: sprintf(StreamName, "UDP_GEN5_BC_CORE"); break;
	case UDP_GEN5_VSE_CORE: sprintf(StreamName, "UDP_GEN5_VSE_CORE"); break;
	case UDP_GEN5_STATUS_CORE: sprintf(StreamName, "UDP_GEN5_STATUS_CORE"); break;
	case UDP_GEN5_RDD_CORE: sprintf(StreamName, "UDP_GEN5_RDD_CORE"); break;
	case UDP_GEN5_DET_CORE: 
		if (strcmp(MUDP_master_config->GenType, "GEN7") == 0)
		{
			sprintf(StreamName, "UDP_GEN7_DET_CORE"); break;
		}
		else
		{
			sprintf(StreamName, "UDP_GEN5_DET_CORE"); break;
		}
	case UDP_GEN5_HDR_CORE: sprintf(StreamName, "UDP_GEN5_HDR_CORE"); break;
	case UDP_GEN5_TRACKER_CORE: sprintf(StreamName, "UDP_GEN5_TRACKER_CORE"); break;
	case UDP_GEN5_DEBUG_CORE: sprintf(StreamName, "UDP_GEN5_DEBUG_CORE"); break;
	//GEN7 Streams
	case UDP_GEN7_DET_STREAM: sprintf(StreamName, "UDP_GEN7_DET_STREAM"); break;
	case UDP_GEN7_HDR_STREAM: sprintf(StreamName, "UDP_GEN7_HDR_STREAM"); break;
	case UDP_GEN7_STATUS_STREAM: sprintf(StreamName, "UDP_GEN7_STATUS_STREAM"); break;
	case UDP_GEN7_RDD_STREAM: sprintf(StreamName, "UDP_GEN7_RDD_STREAM"); break;
	case UDP_GEN7_VSE_STREAM: sprintf(StreamName, "UDP_GEN7_VSE_STREAM"); break;
	case UDP_GEN7_DEBUG_STREAM: sprintf(StreamName, "UDP_GEN7_DEBUG_STREAM"); break;
	case UDP_GEN7_MMIC_CORE: sprintf(StreamName, "UDP_GEN7_MMIC_STREAM"); break;
	case UDP_GEN7_ALIGNMENT_CORE: 
		if (strcmp(MUDP_master_config->GenType, "GEN7") == 0)
		{
			sprintf(StreamName, "UDP_GEN7_ALIGNMENT_STREAM"); break;
		}
		else
		{
			sprintf(StreamName, "UDP_GEN5_ALIGNMENT_STREAM"); break;
		}
	case UDP_GEN7_RFFT_CORE: sprintf(StreamName, "UDP_GEN7_RFFT_STREAM"); break;
	case UDP_GEN7_BLOCKAGE_CORE: sprintf(StreamName, "UDP_GEN7_BLOCKAGE_STREAM"); break;
	case UDP_GEN7_ADC_CORE: sprintf(StreamName, "UDP_GEN7_ADC_STREAM"); break;
	case UDP_GEN7_CALIB_CORE: sprintf(StreamName, "UDP_GEN7_CALIB_STREAM"); break;
	case UDP_GEN7_RADAR_CAPABILITY_CORE: sprintf(StreamName, "UDP_GEN7_RADAR_CAPABILITY_STREAM"); break;
	case UDP_GEN7_DOWN_SELECTION_CORE: sprintf(StreamName, "UDP_GEN7_DOWN_SELECTION_STREAM"); break;
	case UDP_GEN7_ID_CORE: sprintf(StreamName, "UDP_GEN7_ID_STREAM"); break;
	case UDP_GEN7_TOI_CORE: sprintf(StreamName, "UDP_GEN7_TOI_STREAM"); break;
	case UDP_GEN7_DYNAMIC_ALIGNMENT_CORE: sprintf(StreamName, "UDP_GEN7_DYNAMIC_ALIGNMENT_STREAM"); break;
	case UDP_GEN7_ROT_SAFETY_FAULTS_CORE:sprintf(StreamName, "UDP_GEN7_ROT_SAFETY_FAULTS"); break;
	case UDP_GEN7_ROT_TRACKER_INFO_CORE:sprintf(StreamName, "UDP_GEN7_ROT_TRACKER_INFO"); break;
	case UDP_GEN7_ROT_VEHICLE_INFO_CORE:sprintf(StreamName, "UDP_GEN7_ROT_VEHICLE_INFO"); break;
	case UDP_GEN7_ROT_OBJECT_CORE:sprintf(StreamName, "UDP_GEN7_ROT_OBJECT_STREAM"); break;
	case UDP_GEN7_ROT_ISO_OBJECT_CORE:sprintf(StreamName, "UDP_GEN7_ROT_ISO_OBJECT_STREAM"); break;
	case UDP_GEN7_ROT_PROCESSED_DETECTION_CORE:sprintf(StreamName, "UDP_GEN7_ROT_PROCESSED_DETECTION_STREAM"); break;
	default: sprintf(StreamName, "INVALID_STREAM"); break;

	}
	return StreamName;
}


static void FreePlugin()
{

	if (m_pLogPlugin->hModule)
	{
		UnLoadLib(m_pLogPlugin->hModule);
		memset(&m_pLogPlugin, 0, sizeof(m_pLogPlugin));
	}

	if (m_pDecoderPlugin->hModule)
	{
		UnLoadLib(m_pDecoderPlugin->hModule);
		memset(&m_pDecoderPlugin, 0, sizeof(m_pDecoderPlugin));
	}
}
static bool TestAndFixEndianness(UDPRecord_Header* pmet)
{
	bool must_swap = false;

	if (byteswap(pmet->versionInfo) == UDP_RECORD_VERSIONINFO)
	{
		// swap to host endianness
		must_swap = true;
		pmet->versionInfo = byteswap(pmet->versionInfo);
		pmet->sourceTxCnt = byteswap(pmet->sourceTxCnt);
		pmet->sourceTxTime = byteswap(pmet->sourceTxTime);
		pmet->streamRefIndex = byteswap(pmet->streamRefIndex);
		pmet->streamDataLen = byteswap(pmet->streamDataLen);
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
		"=========================================================================================================\n"
		"USAGE:\n"
		"MUDP_Log_DataExtracter.exe MUDP_DATA_Extracter.xml f_logs.txt\n"
		"f_logs contains input log argument listed as column [Input Log argument](.dvsu)/(.dvl) with no delimiter\n"
		"=========================================================================================================\n"
	);
}
int NoUDPdata(CcaMessageClass msgClass, char LogName[1024])
{
	static  CcaMessageClass PrevmsgClass = CCA_IF_GENERIC;
	if (CCA_IF_CAN == msgClass && CCA_IF_CAN == PrevmsgClass && IsUDPdata == false)
	{
		printf("Log file has no UDP data(%s)!!! hence extraction cannot be done\n", LogName);
		return -1;
	}
	PrevmsgClass = msgClass;
	return 0;
}
void gen7_Print_StrmVrsn_Info(DVSU_RECORD_T record, uint8_t Stream_Num)
{
	Customer_T custID = (Customer_T)record.payload[3];
	customer_id = custID;
	unsigned short Stream_Version_Log = record.payload[20];
	int platform = record.payload[8];
	int Decoder_Ver = m_radar_plugin->Get_StreamVer(custID, Stream_Num, platform);
	
	//Stream_Hdr_T gen7_payload_hdr = { 0 };
	unsigned char UdpDataBytes[16] = { 0 };

	int Stream_Chunk_ID;

	Stream_Chunk_ID = record.payload[24]; // (record.payload[24] << 8) | Stream_Chunk_ID;

	if ((record.payload[0] == 0xa5 && record.payload[1] == 0x1c) || (record.payload[0] == 0x1c && record.payload[1] == 0xa5))
	{
		memcpy(&UdpDataBytes, &record.payload[28],sizeof(Stream_Hdr_T));
	}

	const Stream_Hdr_T* gen7_A2_payload_hdr = (const Stream_Hdr_T*)UdpDataBytes;
	int Stream_Size = gen7_A2_payload_hdr->size + sizeof(Stream_Hdr_T); //Payload Size is Combimed Size of Aptiv A2 Header and Payload
	
	if (printStrmHeaderOnes)
	{
		cout << "************************************************************" << endl;
		cout << " Stream Version info table for the provided log" << endl;
		cout << "_________________________________________________________________________________________________" << endl;
		cout << "|" << "Customer Name" << "\t" << "|" << "Stream Name" << setw(17) << "|" << "Log Version" << setw(5) << "|" << "Decoder Version" << setw(4) << "|" << "Stream Size" << "\t" << "|" << endl;
		cout << "_________________________________________________________________________________________________" << endl;
		printStrmHeaderOnes = false;
	}

	if (custID == PLATFORM_GEN7 && Stream_Chunk_ID == 0)
	{
		string stream_name = Get_GEN7StreamName(Stream_Num);
		int stream_name_length = stream_name.length();
		if (Stream_Num == CDC_STREAM || Stream_Num == CALIB_STREAM)
		{
			Stream_Size = 0;
		}
		if (Stream_Num <= 40 && printed_stream_details[Stream_Num] == 0)
		{
			if (stream_name_length <= 13)
			{
				cout << GetCustIDName(custID, platform) << "\t" << " " << stream_name << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			}
			else if (stream_name_length <= 21)
			{
				cout << GetCustIDName(custID, platform) << "\t" << " " << stream_name << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			}
			else
			{
				cout << GetCustIDName(custID, platform) << "\t" << " " << stream_name << "\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			}

			//cout << GetCustIDName(custID, platform) << "\t" << " " << Get_GEN7StreamName(Stream_Num) << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;

			if (Stream_Version_Log > Decoder_Ver)
			{
				tuple<string, int, int, int> StreamInfo = make_tuple( Get_GEN7StreamName(Stream_Num) , platform , Stream_Version_Log, Decoder_Ver );
				Print_Stream_Version_Mismatch_Messages.push_back(StreamInfo);
			    //cout << "NOTE! " << Get_GEN7StreamName(Stream_Num) << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			printed_stream_details[Stream_Num] = 1;
			GetCsvSplitFileDetails(custID, Stream_Num, 3);
		}
	}
}
void gen5_Print_StrmVrsn_Info(DVSU_RECORD_T record, uint8_t Stream_Num)
{
	Customer_T custID =  (Customer_T)record.payload[3];
	customer_id = custID;
	unsigned short Stream_Version_Log = record.payload[24];
	int platform = record.payload[2];
	int Decoder_Ver = m_radar_plugin->Get_StreamVer(custID, Stream_Num , platform);
	int Stream_Size = record.payload[20]; // CYW-3836 - adding new column stream size on console
	if (custID == MOTIONAL_FLR4P || custID == STLA_FLR4P || (custID == PLATFORM_GEN5 && (record.payload[2] == UDP_PLATFORM_FLR4_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
	{
		Stream_Size = (record.payload[21] << 8) | Stream_Size;
	}
	else
	{
		Stream_Size = (Stream_Size << 8) | record.payload[21];
	}
	if (printStrmHeaderOnes)
	{
		cout << "************************************************************" << endl;
		cout << " Stream Version info table for the provided log" << endl;
		cout << "_________________________________________________________________________________________________" << endl;
		cout << "|" << "Customer Name" << "\t" << "|" << "Stream Name" << setw(17) << "|" << "Log Version" << setw(5) << "|" << "Decoder Version" << setw(4) << "|" << "Stream Size" << "\t" << "|" << endl;
		cout << "_________________________________________________________________________________________________" << endl;
		printStrmHeaderOnes = false;
	}

	if (custID != MOTIONAL_FLR4P && custID != STLA_FLR4P && record.payload[2] != UDP_PLATFORM_FLR4_PLUS && record.payload[2] != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
	{	
		if (record.payload[2] == UDP_PLATFORM_STLA_FLR4)
		{
			if (Stream_Num == CDC_STREAM && F_STLA_CDC == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver /*<< "\t\t" << Stream_Size*/ << endl;
				
				/*if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}*/
				F_STLA_CDC = false;
			}
			if (Stream_Num == GEN5_CALIB_STREAM && F_STLA_CALIB == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver /*<< "\t\t" << Stream_Size*/ << endl;
				
				/*if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}*/
				F_STLA_CALIB = false;
			}
			if (Stream_Num == C0_CORE_MASTER_STREAM && F_STLA_C0 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  <<", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C0 = false;
			}
			if (Stream_Num == C0_CUST_MASTER_STREAM && F_STLA_C1 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" <<  Stream_Version_Log << "\t\t" <<  Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C1 = false;
			}
			if (Stream_Num == C1_CORE_MASTER_STREAM && F_STLA_C2 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" <<  Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C2 = false;
			}
			if (Stream_Num == C2_CORE_MASTER_STREAM && F_STLA_C3 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" <<  Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C3 = false;
			}
			if (Stream_Num == C2_CUST_MASTER_STREAM && F_STLA_C4 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" <<  Stream_Version_Log << "\t\t" <<  Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C4 = false;
			}
			if (Stream_Num == OSI_GEN6_STREAM && F_STLA_C5 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_STLA_C5 = false;
			}
		}
		else 
		{
			if (Stream_Num == CDC_STREAM && F_CDC == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t\t" <<  Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				/*if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}*/
				F_CDC = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == GEN5_CALIB_STREAM && F_CALIB == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				/*if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}*/
				F_CALIB = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == C0_CORE_MASTER_STREAM && F_C0 == 1)
			{
				cout<< GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t"  << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C0 = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == C0_CUST_MASTER_STREAM && F_C1 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C1 = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == C1_CORE_MASTER_STREAM && F_C2 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C2 = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == C2_CORE_MASTER_STREAM && F_C3 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C3 = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == C2_CUST_MASTER_STREAM && F_C4 == 1)
			{
				cout << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t"  << Stream_Version_Log << "\t\t"  << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C4 = false;
				GetCsvSplitFileDetails(custID, Stream_Num, 1);
			}
			if (Stream_Num == OSI_GEN6_STREAM && F_C5 == 1)
			{
				cout  << GetCustIDName(custID,platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t"  << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
				
				if (Stream_Version_Log > Decoder_Ver)
				{
					cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
				}
				F_C5 = false;
			}	
		}
	}
	else if (custID == MOTIONAL_FLR4P || custID == STLA_FLR4P || custID == PLATFORM_GEN5)
	{
		if (Stream_Num == e_BC_CORE_LOGGING_STREAM && F_BC == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_BC = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_VSE_CORE_LOGGING_STREAM && F_VSE == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_VSE = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_STATUS_CORE_LOGGING_STREAM && F_STATUS == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_STATUS = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_RDD_CORE_LOGGING_STREAM && F_RDD == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_RDD = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_DET_LOGGING_STREAM && F_DET == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_DET = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_HDR_LOGGING_STREAM && F_HDR == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			
			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_HDR = false;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);
		}
		if (Stream_Num == e_TRACKER_LOGGING_STREAM && F_TRACKER == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;
			GetCsvSplitFileDetails(custID, Stream_Num, 2);

			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_TRACKER = false;
		}
		if (Stream_Num == e_DEBUG_LOGGING_STREAM && F_DEBUG == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;

			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_DEBUG = false;
		}
		if (Stream_Num == e_ALIGNMENT_LOGGING_STREAM && F_ALIGNMENT == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << "\t\t" << Stream_Size << endl;

			if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_FLR4PGEN5StreamName(Stream_Num)  << ", Platfrom ID - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}
			F_ALIGNMENT = false;
		}
		if (Stream_Num == e_CALIB_STREAM && F_CALIB_flr4p == 1)
		{
			cout << GetCustIDName(custID,platform) << "\t" << " " << Get_FLR4PGEN5StreamName(Stream_Num) << "\t\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver << endl;

			/*if (Stream_Version_Log > Decoder_Ver)
			{
				cout << "NOTE! ", Platfrom ID - " << platform - " << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}*/
			F_CALIB_flr4p = false;
		}
		if (Stream_Num == CDC_STREAM && F_CDC_flr4p == 1)
		{
			cout << GetCustIDName(custID, platform) << "\t" << " " << Get_GEN5StreamName(Stream_Num) << "\t\t" << Stream_Version_Log << "\t\t" << Decoder_Ver /*<< "\t\t" << Stream_Size*/ << endl;

			/*if (Stream_Version_Log > Decoder_Ver)
			{
				cout <<"NOTE! " << Get_GEN5StreamName(Stream_Num)  << platform << ", log having new version - " << Stream_Version_Log << "," << "need to update decoder, available decoder version is " << Decoder_Ver << endl;
			}*/
			F_CDC_flr4p = false;
		}
	}
}
void Print_StrmVrsn_Info(DVSU_RECORD_T record, uint8_t Stream_Num)
{
	Customer_T custID = (Customer_T)record.payload[23];
	customer_id = custID;
	unsigned short Stream_Version = record.payload[20];
	int platform = record.payload[8];

	if (printStrmHeaderOnes)
	{
		cout << "************************************************************" << endl;
		cout << " Stream Version info table for the provided log" << endl;
		cout << "______________________________________________________" << endl;
		cout << "|" << "CustomerName" << "|" << "StreamName" << setw(16) << "|" << "StreamVersion" << "|" << endl;
		cout << "______________________________________________________" << endl;
		printStrmHeaderOnes = false;
	}

	if (Stream_Num == CDC_DATA && F_CDC_DATA == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(18) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_CDC_DATA = false;
	}
	else if ((Stream_Num == Z7A_LOGGING_DATA) && (F_Z7a_core == 1))
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(10) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Z7a_core = false;
	}
	else if ((Stream_Num == Z7B_LOGGING_DATA) && (F_Z7b_core == 1))
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(10) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Z7b_core = false;
	}
	else if ((Stream_Num == Z4_LOGGING_DATA) && (F_z4_core == 1))
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(11) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_z4_core = false;
		GetCsvSplitFileDetails(custID, Stream_Num, 0);
	}
	else if (Stream_Num == SRR3_Z7B_CUST_LOGGING_DATA && F_z7b_cust == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(5) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_z7b_cust = false;
	}
	else if (Stream_Num == Z4_CUST_LOGGING_DATA && F_z4_cust == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(6) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_z4_cust = false;
	}
	else if (Stream_Num == 11 && F_Ecu_0 == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(17) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Ecu_0 = false;
	}
	else if (Stream_Num == 13 && F_Ecu_3 == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(17) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Ecu_3 = false;
	}
	else if (Stream_Num == 12 && F_Ecu_1 == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(17) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Ecu_1 = false;
	}
	else if (Stream_Num == 15 && F_Ecu_calib == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(12) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Ecu_calib = false;
	}
	else if (Stream_Num == 18 && F_Ecu_VRU == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(12) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_Ecu_VRU = false;
	}
	else if (Stream_Num == DSPACE_CUSTOMER_DATA && F_dspace == 1)
	{
		cout << "|" << GetCustIDName(custID,platform) << setw(6) << "|" << GetStreamName(Stream_Num) << setw(12) << "|" << Stream_Version << setw(13) << "|" << endl;
		F_dspace = false;
		GetCsvSplitFileDetails(custID, Stream_Num, 0);
	}
	if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
		if (printOnesline) {
			if (!F_Ecu_VRU && !F_Ecu_calib && !F_Ecu_1 && !F_Ecu_3 && !F_Ecu_0 && !F_CDC_DATA && !F_Z7a_core && !F_Z7b_core && !F_z4_core && !F_z7b_cust && !F_z4_cust) {
				cout << "______________________________________________________" << endl;
				printOnesline = false;
			}
			else if (!F_dspace)
			{
				cout << "______________________________________________________" << endl;
				printOnesline = false;
			}
		}
	}
	else {
		if (printOnesline) {
			if (!F_CDC_DATA && !F_Z7a_core && !F_Z7b_core && !F_z4_core && !F_z7b_cust && !F_z4_cust) {
				cout << "______________________________________________________" << endl;
				printOnesline = false;
			}
			else if (!F_dspace)
			{
				cout << "______________________________________________________" << endl;
				printOnesline = false;
			}
		}
	}

}
void printccaUDPHdr(FILE* pOutFile, char* LogFname, DVSU_RECORD_T* pRec, uint32_t Busid, INT64 udpstarttimestamp, INT64 UDP_CCAtime)
{
	uint32_t BUS_ID = MaptoBusid(Busid);
	float deltatime = 0;
	INT64 CCA_micro = 0;
	INT64 Start_micro = 0;
	INT64 Relative_time = 0;
	float64_T Next_CCA_milli = 0.0;
	static INT64 Prev_CCA_milli = 0;
	INT64 Next_CCA_micro = 0;
	CCA_micro = UDP_CCAtime / 1000;
	Start_micro = udpstarttimestamp / 1000;
	const UDP_FRAME_STRUCTURE_T *ptr_bytestream = (const UDP_FRAME_STRUCTURE_T *)&(pRec->payload[0]);
	Next_CCA_micro = (CCA_micro - Prev_CCA_milli);
	Next_CCA_milli = ((float)Next_CCA_micro) / 1000.00;
	Prev_CCA_milli = CCA_micro;
	Relative_time = CCA_micro - Start_micro;
	if ((Next_CCA_milli > 15) && (UdpfCreate) && (Next_CCA_milli < 10000))
	{
		fprintf(UdpfCreate, "Delta  Timestamp is greater than 15millisec for the log : %s \n", LogFname);
		fprintf(UdpfCreate, "Scanindex_%d \n", B2L_l(ptr_bytestream->frame_header.streamRefIndex));
		fprintf(UdpfCreate, "*****************************************************************************************\n");
	}
	if (!UDPflag) {
		fprintf(pOutFile, "BUS_ID,Sensor,Strm_No,Scan_Index,Src_Txn_Cnt,Chunk_Index,Abs_Start_Time(micro),Abs_PLP_Time(micro),Relative_PLP_Time,Delta_Time(mili)\n");
		UDPflag = true;
	}
	fprintf(pOutFile, "%u,%s,%d,%u,%u,%u,%lld,%lld,%lld,%f\n",
		Busid,
		GetBusIdName(Busid),
		ptr_bytestream->frame_header.streamNumber,
		B2L_l(ptr_bytestream->frame_header.streamRefIndex),
		B2L_s(ptr_bytestream->frame_header.sourceTxCnt),
		ptr_bytestream->frame_header.streamChunkIdx,
		Start_micro,
		CCA_micro,
		Relative_time,
		Next_CCA_milli
	);
}
void printccaCanHdr(FILE* pOutFile, char* LogFname, uint32_t Busid, dvlCanMessageFD CanFd, INT64 starttimestamp, INT64 CCAtime)
{
	uint32_t sourceId = MaptoBusid(Busid);
	INT64 CCA_micro = 0;
	INT64 CAN_CCA_micro = 0;
	INT64 Relative_time = 0;
	static INT64 Prev_CCA_micro = 0;
	INT64 Start_micro = 0;
	float64_T Next_CCA_milli = 0.0;
	CCA_micro = CCAtime / 1000;
	Start_micro = starttimestamp / 1000;
	CAN_CCA_micro = (CCA_micro - Prev_CCA_micro);
	Next_CCA_milli = ((float)CAN_CCA_micro) / 1000.00;
	Prev_CCA_micro = CCA_micro;
	Relative_time = CCA_micro - Start_micro;
	if ((Next_CCA_milli > 7) && (CanfCreate) && (Next_CCA_milli < 10000))
	{
		fprintf(CanfCreate, "Delta Timestamp is greater than 7millisec for the log : %s \n", LogFname);
		fprintf(CanfCreate, "*****************************************************************************************\n");
	}
	if (!Cflg) {
		fprintf(pOutFile, "Can_Channel,CAN_channel_Name,Can_id,Can_length,Abs_Start_Time(micro),Abs_PLP_Time(micro),Relative_PLP_Time,Delta_Time(mili)\n");
		Cflg = 1;
	}
	fprintf(pOutFile, "%u,%s,%u,%u,%lld,%lld,%lld,%f\n",
		Busid,
		GetBusIdName(Busid),
		CanFd.id,
		CanFd.length,
		Start_micro,
		CCA_micro,
		Relative_time,
		Next_CCA_milli
	);

}

int CreateoutputCcaCSV(char* LogFname, CcaMessageClass msgClass, uint32_t BUS_Id) {
	int ret = 0;
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif
	char * FileName;
	char* FilePtr;
	int32_t sourceid = MaptoBusid(BUS_Id);
	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };
	std::string path(LogFname);
	std::replace(path.begin(), path.end(), '\\', '/');
	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", LogFname);
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}

#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				/*fName[st_Len - 3] = 0;
				fName[st_Len - 2] = 0;
				fName[st_Len - 1] = 0;*/
				strcat(newpath, fName);


				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(LogFname, "%s_71.dvsu", newpath);
				else
					sprintf(LogFname, "%s.dvsu", newpath);
				std::string pathLogfilename(LogFname);
				std::replace(pathLogfilename.begin(), pathLogfilename.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(pathLogfilename.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(pathLogfilename.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}
	else
	{
#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
	}
	if (msgClass == CCA_IF_CAN) {
		sprintf(FileName_CCA_Pos, "%s%s%s_CAN.csv", drvName, dir, fName);
		if (sourceid != 17)
		{
			FileName = FileName_CCA_Pos;
			fpCCAmf4Dump = fopen(FileName, "w+");
			if (!fpCCAmf4Dump)
			{
				fprintf(stderr, "Error Creating Output file: %s\n\n", FileName);
				fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", FileName);
				ret = -1;
			}
		}
	}
	else
	{
		sprintf(FileName_CCA_Pos, "%s%s%s_UDP.csv", drvName, dir, fName);
		if (sourceid != 17)
		{
			FilePtr = FileName_CCA_Pos;
			fpUdpmf4Dump = fopen(FilePtr, "w+");
			if (!fpUdpmf4Dump)
			{
				fprintf(stderr, "Error Creating Output file: %s\n\n", FilePtr);
				fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", FilePtr);
				ret = -1;
			}
		}
	}
	sprintf(UDPCreate_Pos, "%s%sPLP_Timestamp_Failure_UDP_Info.txt", drvName, dir);
	UdpfCreate = fopen(UDPCreate_Pos, "a");
	if (!UdpfCreate)
	{
		fprintf(stderr, "Error Creating Output file: %s", UDPCreate_Pos);
		ret = -1;
	}
	sprintf(CANCreate_Pos, "%s%sPLP_Timestamp_Failure_CAN_Info.txt", drvName, dir);
	CanfCreate = fopen(CANCreate_Pos, "a");
	if (!CanfCreate)
	{
		fprintf(stderr, "Error Creating Output file: %s", CANCreate_Pos);
		ret = -1;
	}

#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif
	return ret;
}
int CreateOutputSensorXMLFile(char*inputLogname, Customer_T CustId)
{
	int retx = 0;
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif
	char XML_Dir[_MAX_FNAME] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
	char * XMLFName[20];

	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };
	std::string path(inputLogname);
	std::replace(path.begin(), path.end(), '\\', '/');
	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", inputLogname);
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}
#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				fName[st_Len - 3] = 0;
				fName[st_Len - 2] = 0;
				fName[st_Len - 1] = 0;
				strcat(newpath, fName);


				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogname, "%s_71.dvsu", newpath);
				else
					sprintf(inputLogname, "%s.dvsu", newpath);
			std:string filepath(inputLogname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}
	else
	{
#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
	}
	sprintf(XML_Dir, "MUDP_XML_TRACE");
	strcat(dir, XML_Dir);
	string dirName(dir);
	sprintf(newpath, "%s%s", drvName, dirName.c_str());
#if defined(_WIN_PLATFORM)
	CreatePathDir(newpath);

#elif defined(__GNUC__)
	if (-1 == (mkdir(newpath, ACCESSPERMS)))
	{
		//to do
	}
#endif

	if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT) {

		for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++)
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1 && MUDP_master_config->MUDP_Radar_ECU_status == 1)
			{
				sprintf(XMLFName_Pos, "%s/%s_Sensors_%s.xml", newpath, fName, GetRadarPosName(RADAR_POS));
				fpxml[i] = fopen(XMLFName_Pos, "w");
				if (!fpxml[i])
				{
					fprintf(stderr, "Error Creating Output file: %s", XMLFName_Pos);
					retx = -1;
				}
			}

		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RL] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FL])
		{
			sprintf(XMLFName_Pos, "%s/%s_Sensors.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}
		else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_R] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_L])
		{
			sprintf(XMLFName_Pos, "%s/%s_Bpil_Sensors.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}
		else if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier])
		{
			sprintf(XMLFName_Pos, "%s/%s_ECU.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}
		if (!fpxml)
		{
			fprintf(stderr, "Error Creating Output file: %s", XMLFName_Pos);
			retx = -1;
		}

	}

	else if (MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT || MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM || MUDP_master_config->Radar_Stream_Options[Dspace]) {
		//if (!MUDP_master_config->Radar_Stream_Options[Dspace])
		{
			for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++)
			{
				if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
				{
					sprintf(XMLFName_Pos, "%s/%s_Sensors_%s.xml", newpath, fName, GetRadarPosName(i));
					fpxml[i] = fopen(XMLFName_Pos, "w");
					if (!fpxml[i])
					{
						fprintf(stderr, "Error Creating Output file: %s\n\n", XMLFName_Pos);
						fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", XMLFName_Pos);
						retx = -1;
					}
				}
				if (i == 19 && MUDP_master_config->MUDP_Radar_ECU_status == 1)
				{
					sprintf(XMLFName_Pos, "%s/%s_Sensors_%s.xml", newpath, fName, GetRadarPosName(i));
					fpxml[i] = fopen(XMLFName_Pos, "w");
					if (!fpxml[i])
					{
						fprintf(stderr, "Error Creating Output file: %s\n\n", XMLFName_Pos);
						fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", XMLFName_Pos);
						retx = -1;
					}
				}
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RL] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FL] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FC])
		{
			sprintf(XMLFName_Pos, "%s/%s_Sensors.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}
		/*else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_R] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_L])
		{
			sprintf(XMLFName_Pos, "%s/%s_Bpil_Sensors.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}
		else if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier])
		{
			sprintf(XMLFName_Pos, "%s/%s_ECU.xml", newpath, fName);

			sfpxml = fopen(XMLFName_Pos, "w");
		}*/
		
		if (!fpxml)
		{
			fprintf(stderr, "Error Creating Output file: %s\n\n", XMLFName_Pos);
			fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", XMLFName_Pos);
			retx = -1;
		}

	}

	

#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif
	return retx;
}
int CreateOutputHILPortXMLFile(char* inputLogname, Customer_T CustId)
{
	int retx = 0;
	char HIL_XML_Dir[_MAX_FNAME] = { 0 };
	char  logname[_MAX_PATH] = { 0 };
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif
	//char * HilXMLFName[20];

	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };

	std::string path(inputLogname);
	std::replace(path.begin(), path.end(), '\\', '/');
	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", inputLogname);
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}
#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				fName[st_Len - 3] = 0;
				fName[st_Len - 2] = 0;
				fName[st_Len - 1] = 0;
				strcat(newpath, fName);


				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogname, "%s_71.dvsu", newpath);
				else
					sprintf(inputLogname, "%s.dvsu", newpath);

				std::string filepath(inputLogname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}
	else
	{
#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
	}
	sprintf(HIL_XML_Dir, "MUDP_XML_TRACE");
	strcat(dir, HIL_XML_Dir);
	string dirName(dir);
	sprintf(newpath, "%s%s", drvName, dirName.c_str());
#if defined(_WIN_PLATFORM)
	CreatePathDir(newpath);

#elif defined(__GNUC__)
	if (-1 == (mkdir(newpath, ACCESSPERMS)))
	{
		//to do
	}
#endif

	if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT) {
		if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RL] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FL])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_Sensors.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_R] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_L])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_Bpil_Sensors.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		else if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_ECU.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		if (!hxml)
		{
			fprintf(stderr, "Error Creating Output file: %s", HILXMLFName_Pos);
			retx = -1;
		}


	}
	else if (MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT || MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM) {
		if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RL] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_RR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FR] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FL])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_Sensors.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_R] || MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_BP_L])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_Bpil_Sensors.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		else if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] || MUDP_master_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier])
		{
			sprintf(HILXMLFName_Pos, "%s/%s_ECU.xml", newpath, fName);

			hxml = fopen(HILXMLFName_Pos, "w");
		}
		if (!hxml)
		{
			fprintf(stderr, "Error Creating Output file: %s", HILXMLFName_Pos);
			retx = -1;
		}

	}

#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif

	return retx;
}

int CreateDspacecsvFile(char * inputLogFname, Customer_T custId)
{
	int ret = 0;
	char  logname[_MAX_PATH] = { 0 };
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif
	char * FileName[20];

	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };
	std::string path(inputLogFname);
	std::replace(path.begin(), path.end(), '\\', '/');
	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", inputLogFname);
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}

#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				//fName[st_Len - 3] = 0;
				//fName[st_Len - 2] = 0;
				//fName[st_Len - 1] = 0;
				strcat(newpath, fName);
				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogFname, "%s_71.dvsu", newpath);
				else
					sprintf(inputLogFname, "%s.dvsu", newpath);
				string filepath(inputLogFname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}
	else
	{
#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif

	}

	for (int i = 0; i < INVALID_STRM; i++)
	{
		if (dump[i] != 0)
		{
			sprintf(FileName_Pos, "%s%s%s_%s.csv", drvName, dir, fName, GetstreamIDName(i));
			fpDump = fopen(FileName_Pos, "w");
		}
	}
	if (!fpDump)
	{
		fprintf(stderr, "Error Creating Output file: %s", FileName_Pos);
		ret = -1;
	}



#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif
	return ret;
}

int CreateOutputPacketlossReport(char* inputLogFname)
{

	int ret = 0;
	char Logfilepath[_MAX_FNAME];

	char  logname[_MAX_PATH] = { 0 };
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif

	strcpy(Logfilepath, inputLogFname);
	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };
	std::string path(inputLogFname);
	std::replace(path.begin(), path.end(), '\\', '/');

	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", inputLogFname);
				strcpy(Logfilepath, inputLogFname);
				static int once = true;
				char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };
				std::string filepath(inputLogFname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
			split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
			split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}

#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				//fName[st_Len - 3] = 0;
				//fName[st_Len - 2] = 0;
				//fName[st_Len - 1] = 0;
				strcat(newpath, fName);
				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogFname, "%s_71.dvsu", newpath);
				else
					sprintf(inputLogFname, "%s.dvsu", newpath);

				string filepath(inputLogFname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}

	else
	{

#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif

	}
	if (CreatePktOnce)
	{
		if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
		{
			if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT || MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					sprintf(FileCreate_Pos, "%s%Data_Logging_Quality.txt", drvName, dir);
				}
				else
					if (MUDP_master_config->Packetloss_report_format == XML_format)
					{
						sprintf(FileCreate_Pos, "%s%sData_Logging_Quality.xml", drvName, dir);
					}
				fCreate = fopen(FileCreate_Pos, "a");
				if (!fCreate)
				{
					fprintf(stderr, "Error Creating Output file: %s", FileCreate_Pos);
					ret = -1;
				}
				else {
					time_t now = time(NULL);
					struct tm *t = localtime(&now);
					if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						if (printkeyword == true)
						{
							float xmlver = 1.0;
							string UTFVer = "UTF-8";
							char c = '"';
							fprintf(fCreate, "<?xml version=%c%.1f%c encoding= %c%s%c ?>\n", c, xmlver, c, c, UTFVer.c_str(), c);
							fprintf(fCreate, "<PacketLossReport>\n");

						}
					}

					// BMW Supporting changes 
					if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
					{
						//if (printkeyword == true)
						//{
						//	fprintf(fCreate, "\t<Tool_Version_number> %d_%d_%d_%d </Tool_Version_number>\n", release_year, release_week, release_inf, release_minor);
						//	fprintf(fCreate, "\t<Report_Created_Date>%d-%d-%d %d:%d:%d</Report_Created_Date>\n", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
						//	fprintf(fCreate,"\t<Selected_Generation>%s</Selected_Generation>\n", MUDP_master_config->GenType);
						//	fprintf(fCreate, "\t<Sensors_Enabled> %s </Sensors_Enabled>", radarEnabled.c_str());
						//	//fprintf(fCreate, "\t<Sensor_Enabled>%s</Sensor_Enabled>",radarEnabled);
						//	printkeyword = false;
						//}
					}
					if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						if (printkeyword == true)
						{					
							fprintf(fCreate, "\t<Tool_Version_number> %d_%d_%d_%d </Tool_Version_number>\n", release_year, release_week, release_inf, release_minor);
							fprintf(fCreate, "\t<Changeset_number> %d </Changeset_number>\n", CS_number);
							fprintf(fCreate, "\t<Report_Created_Date>%d-%d-%d %d:%d:%d</Report_Created_Date>\n", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
							fprintf(fCreate, "\t<Selected_Generation>%s</Selected_Generation>\n", MUDP_master_config->GenType);
							fprintf(fCreate, "\t<Sensors_Enabled> %s </Sensors_Enabled>\n", radarEnabled.c_str());
							fprintf(fCreate, "<Keywords>\n");
							fprintf(fCreate, "\t\t\tReset happened --> Sensor Reset\n");
							fprintf(fCreate, "\t\t\tScan Index Drop --> Scan Index Drops\n");
							fprintf(fCreate, "\t\t\tChecksumError --> In case of checksum failure\n");
							fprintf(fCreate, "\t\t\tCRC mismatch --> In case of CRC mismatch\n");
							fprintf(fCreate, "\t\t\tchunks lost -->  Chunk Loss\n");
							fprintf(fCreate, "\t\t\tReceived duplicate F360 --> Received duplicate F360 internal data chunk with wrong data\n");
							fprintf(fCreate, "\t\t\tduplicate cal_source -->when duplicate cal source is observed\n");
							fprintf(fCreate, "\t\t\tOVERRUN_FAIL , Timingoverruns -- > When timing overrun happens\n");
							fprintf(fCreate, "\t\t\tNo data available in the log\n");
							fprintf(fCreate, "\t\t\tAsynchronous_Bit\n");
							fprintf(fCreate, "\t\t\tPACKET_LOSS_FAIL \n");
							fprintf(fCreate, "\t\t\tMissedScanIndexCount \n");
							fprintf(fCreate, "</Keywords>\n");
							printkeyword = false;
						}
						if (MUDP_master_config->Packetloss_report_format == Text_format)
						{
							fprintf(fCreate, "\n");
							fprintf(fCreate, "<Log_%d_%s>\n", ++inc_flag, logfilename);
							fprintf(fCreate, "\t<LOGPATH>%s</LOGPATH>\n", inputLogFname);
							//fprintf(fCreate, "\t<Report_Created_Date>%d-%d-%d %d:%d:%d</Report_Created_Date>\n", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
							//fprintf(fCreate, "\t\t<Customername>%s</Customername>\n",GetCustIDName(custID,platform));
							fprintf(fCreate, "\n");
						}
						else
						{
							fprintf(fCreate, "\n");
							fprintf(fCreate, "<Log_%d_%s>\n", ++inc_flag, logfilename);
							fprintf(fCreate, "\t<LOGPATH>%s</LOGPATH>\n", Logfilepath);
							//fprintf(fCreate, "\t<Report_Created_Date>%d-%d-%d %d:%d:%d</Report_Created_Date>\n", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);
							//fprintf(fCreate, "\t\t<Customername>%s</Customername>\n",GetCustIDName(custID,platform));
							fprintf(fCreate, "\n");
						}
					}
				}
			}
		}
		CreatePktOnce = false;
		
	}
	return 0;
}


int CreateOutputCSVFile(char* inputLogFname, DVSU_RECORD_T record)
{
	int ret = 0;
	Customer_T custId = INVALID_CUSTOMER;
	int Platform_Id = 0;

	if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
	{
		custId = PLATFORM_GEN7; //(Customer_T)record.payload[3];
		Platform_Id = record.payload[8];
	}
	else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
	{
		 custId = (Customer_T)record.payload[3];
		 Platform_Id = record.payload[2];
	}
	else
	{
		 custId = (Customer_T)record.payload[23];
		 Platform_Id = record.payload[8];
	}
	char  logname[_MAX_PATH] = { 0 };
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
#endif
	char * FileName[20];

	static int once = true;
	char newpath[MAX_FILEPATH_NAME_LENGTH] = { 0 };

	//if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
	if (MUDP_master_config->data_extracter_mode[CSV] == 1 && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0 && MUDP_master_config->packet_loss_statistics == 0 && MUDP_master_config->data_extracter_mode[Quality_Check] == 0)
	{
		if (custId == BMW_BPIL || custId == STLA_SCALE4 || (custId == PLATFORM_GEN5 && (record.payload[2] == UDP_PLATFORM_SRR6_PLUS || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_CAN || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_ETH || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH))) {
			if (MUDP_master_config->data_extracter_mode[CSV] == 1) {
				if (MUDP_master_config->Bpillar_status != 1 && MUDP_master_config->MUDP_Radar_ECU_status == 1) {
					printf("Please Enable BPIL radar, since the BPIL log is provided\n");
					printf("\rExiting the resimulation.\n\n\r");
					exit(0);
				}
			}
		}

		else if (custId == BMW_SAT || custId == STLA_SRR6P) {
			if (MUDP_master_config->Bpillar_status == 1) {

				//*******DO NOTHING*********//
				/*printf("Please do not Enable BPIL radar, since the BMW_MID input log is provided\n");
				printf("\rExiting the resimulation.\n\n\r");
				exit(0);*/
			}
		}
		else {
			if (MUDP_master_config->Bpillar_status == 1) {
			//	printf("Please do not Enable BPIL radar, since the %s input log is provided\n", GetCustIDName(custId, Platform_Id));
			//	printf("\rExiting the resimulation.\n\n\r");
			//	exit(0);
			}
			if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
				printf("Please do not Enable Radar ECU status, since the %s input log is provided\n", GetCustIDName(custId, Platform_Id));
				printf("\rExiting the resimulation.\n\n\r");
				exit(0);
			}
		}
	}
	std::string path(inputLogFname);
	std::replace(path.begin(), path.end(), '\\', '/');
	if (MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE || MUDP_master_config->MUDP_output_path == SAME_AS_OUTPUT_PATH)
	{
		if ((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE") == 0) || (strcmp(MUDP_master_config->MUDP_output_folder_path, "") == 0)) {
			if (once)
				fprintf(stderr, " \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n", inputLogFname);
			once = false;
		}
		else {
			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
			unsigned int st_Len = 0;
			st_Len = strlen(newpath);
			if (st_Len != 0)
			{
				if (newpath[st_Len - 1] != '\\') {
					newpath[st_Len] = '\\';
				}

#if defined(_WIN_PLATFORM)
				split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif
				st_Len = strlen(fName);
				//fName[st_Len - 3] = 0;
				//fName[st_Len - 2] = 0;
				//fName[st_Len - 1] = 0;
				strcat(newpath, fName);
				if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
					sprintf(inputLogFname, "%s_71.dvsu", newpath);
				else
					sprintf(inputLogFname, "%s.dvsu", newpath);

				string filepath(inputLogFname);
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
#if defined(_WIN_PLATFORM)
				split_path(filepath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
				split_pathLinux(filepath.c_str(), &drvName, &dir, &fName, &ext);
#endif
			}
		}
	}
	else
	{

#if defined(_WIN_PLATFORM)
		split_path(path.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
		split_pathLinux(path.c_str(), &drvName, &dir, &fName, &ext);
#endif

	}

	if (CreateCsvOnce)
	{
		if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
		{
			if (custId == BMW_SAT || custId == BMW_BPIL) {
				if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
					for (int i = 1; i < MAX_ECU_COUNT; i++)
					{
						for (int j = 0; j < INVALID_STRM; j++)
						{
							if (dump[j] != 0)
							{
								sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(RADAR_POS), GetstreamIDName(j));
								FileName[RADAR_POS] = FileName_Pos;
								if (MUDP_master_config->RADAR_ECU_Stream_Options[i] == i) {

									fpMF4Dump[RADAR_POS][j] = fopen(FileName[RADAR_POS], "w");

									CSVFName[RADAR_POS][j] = new char[strlen(FileName_Pos) + 1];
									strcpy(CSVFName[RADAR_POS][j], FileName_Pos);

									if (!fpMF4Dump[RADAR_POS][j])
									{
										fprintf(stderr, "Error Creating Output file: %s", FileName_Pos);
										ret = -1;
									}
								}
							}
						}
					}
					if (dump[HEADER] == 1)
					{
						fpMF4Dump[RADAR_POS][HEADER] = fopen(FileName[RADAR_POS], "w");

						CSVFName[RADAR_POS][HEADER] = new char[strlen(FileName[RADAR_POS]) + 1];
						strcpy(CSVFName[RADAR_POS][HEADER], FileName[RADAR_POS]);

						if (!fpMF4Dump[RADAR_POS][HEADER])
						{
							fprintf(stderr, "Error Creating Output file: %s", FileName_Pos);
							ret = -1;
						}
					}
				}
			}
		}


		if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT 
			|| MUDP_master_config->MUDPInput_Options == MDF4_INPUT || MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM) {

			if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
			{
				if (record.payload[8] == UDP_SOURCE_CUST_DSPACE)
				{
					for (int j = 0; j < INVALID_STRM; j++)
					{
						if (dump[j] != 0)
						{
							sprintf(FileName_Pos, "%s%s%s_%s.csv", drvName, dir, fName, GetstreamIDName(j));

							fpMF4Dump[0][j] = fopen(FileName_Pos, "w");

							CSVFName[0][j] = new char[strlen(FileName_Pos) + 1];
							strcpy(CSVFName[0][j], FileName_Pos);
						}
					}
				}
				else
				{
					for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++)
					{
						if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
						{
							if (MUDP_master_config->RADAR_ECU_Stream_Options[i + 1] != i + 1) {
								for (int j = 0; j < INVALID_STRM; j++)
								{
									if (dump[j] != 0 && ((j <= UDP_GEN7_ROT_PROCESSED_DETECTION_CORE && j >= UDP_OSI) || j == HEADER || j == UDP_CDC_STRM))
									{
										sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

										FileName[i] = FileName_Pos;
										fpMF4Dump[i][j] = fopen(FileName[i], "w");

										CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
										strcpy(CSVFName[i][j], FileName_Pos);

										/*if ((i == MUDP_SRR_FC) && (j <= UDP_GEN5_C2_CUST && j >= UDP_OSI || j == HEADER || j == UDP_CDC_STRM) && (MUDP_master_config->Radar_Stream_Options[BC_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == 0)
											&& (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DET_CORE] == 0)
											&& (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == 0))
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}
										if ((i != MUDP_SRR_FC) && (j <= UDP_GEN7_DYNAMIC_ALIGNMENT_CORE && j >= UDP_OSI || j == HEADER || j == UDP_CDC_STRM))
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}*/

									}
								}
							}
							if (!fpMF4Dump[i])
							{
								fprintf(stderr, "Error Creating Output file: %s\n\n", FileName[i]);
								fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", FileName[i]);
								ret = -1;
							}

						}
					}
				}
			}
			else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
			{
				if (record.payload[2] == UDP_SOURCE_CUST_DSPACE)
				{
					for (int j = 0; j < INVALID_STRM; j++)
					{
						if (dump[j] != 0)
						{
							sprintf(FileName_Pos, "%s%s%s_%s.csv", drvName, dir, fName, GetstreamIDName(j));

							fpMF4Dump[0][j] = fopen(FileName_Pos, "w");

							CSVFName[0][j] = new char[strlen(FileName_Pos) + 1];
							strcpy(CSVFName[0][j], FileName_Pos);
						}
					}
				}
				else
				{
					for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++)
					{
						if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
						{
							if (MUDP_master_config->RADAR_ECU_Stream_Options[i + 1] != i + 1) {
								for (int j = 0; j < INVALID_STRM; j++)
								{
									if(MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 && dump [j]!=0 && (j==20) )
									{
										sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

										FileName[i] = FileName_Pos;
										fpMF4Dump[i][j] = fopen(FileName[i], "w");

										CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
										strcpy(CSVFName[i][j], FileName_Pos);
									}
									else if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 0 && dump[j] != 0 && ((j <= UDP_GEN5_C2_CUST && j >= UDP_OSI) || j == HEADER || j == UDP_CDC_STRM))
									{
										if ((i == MUDP_SRR_FC) && (j <= UDP_GEN5_C2_CUST && j >= UDP_OSI || j == HEADER || j == UDP_CDC_STRM))/* && (MUDP_master_config->Radar_Stream_Options[BC_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == 0)
											&& (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DET_CORE] == 0)
											&& (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == 0))*/
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}
										if ((i != MUDP_SRR_FC) && (j <= UDP_GEN5_C2_CUST && j >= UDP_OSI || j == HEADER || j == UDP_CDC_STRM))
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}
										
									}
									else if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 0 && dump[j] != 0 && ((j <= UDP_GEN7_ALIGNMENT_CORE && j >= UDP_GEN5_BC_CORE) || j == HEADER || j == UDP_CDC_STRM) && ((i == MUDP_SRR_FC) || (i == MUDP_FLR4P_FC_09)))
									{
										sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

										FileName[i] = FileName_Pos;
										fpMF4Dump[i][j] = fopen(FileName[i], "w");

										CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
										strcpy(CSVFName[i][j], FileName_Pos);
									}
									if (Platform_Id == UDP_PLATFORM_FLR4_PLUS_STANDALONE || Platform_Id == UDP_PLATFORM_FLR4_PLUS) // CYW- 3901
									{
										if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 0 && dump[j] != 0 && ((j <= UDP_GEN7_ALIGNMENT_CORE && j >= UDP_GEN5_BC_CORE) || j == HEADER || j == UDP_CDC_STRM) && ((i <= MUDP_SRR_FL && i >= MUDP_SRR_RL)))
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}
									}
								}
							}
							if (!fpMF4Dump[i])
							{
								fprintf(stderr, "Error Creating Output file: %s\n\n", FileName[i]);
								fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", FileName[i]);
								ret = -1;
							}

						}
					}
				}
			}
			else if ((record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3)
				|| (record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
			{
				if (record.payload[8] == UDP_SOURCE_CUST_DSPACE)
				{
					for (int j = 0; j < INVALID_STRM; j++)
					{
						if (dump[j] != 0)
						{
							sprintf(FileName_Pos, "%s%s%s_%s.csv", drvName, dir, fName, GetstreamIDName(j));

							fpMF4Dump[0][j] = fopen(FileName_Pos, "w");

							CSVFName[0][j] = new char[strlen(FileName_Pos) + 1];
							strcpy(CSVFName[0][j], FileName_Pos);
						}
					}
				}
				else
				{
					for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++)
					{
						if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
						{
							if (MUDP_master_config->RADAR_ECU_Stream_Options[i + 1] != i + 1) {
								for (int j = 0; j < INVALID_STRM; j++)
								{
									if (dump[j] != 0 && j < 16)
									{
										sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

										FileName[i] = FileName_Pos;
										fpMF4Dump[i][j] = fopen(FileName[i], "w");

										CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
										strcpy(CSVFName[i][j], FileName_Pos);
									}
								}
							}
							if ((MUDP_master_config->MUDP_Radar_ECU_status == 1) && (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1))
							{
								{
									for (int j = 0; j < INVALID_STRM; j++)
									{
										if ((i <= MUDP_FLR4P_FC_09 && i >= MUDP_SRR_RL) && (dump[j] != 0) && (j <= UDP_GEN5_DEBUG_CORE && j >= UDP_OSI)) // for SCALE4 extarction 
										{
											sprintf(FileName_Pos, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(i), GetstreamIDName(j));

											FileName[i] = FileName_Pos;
											fpMF4Dump[i][j] = fopen(FileName[i], "w");

											CSVFName[i][j] = new char[strlen(FileName_Pos) + 1];
											strcpy(CSVFName[i][j], FileName_Pos);
										}
									}
								}
							}
								if (!fpMF4Dump[i])
								{
									fprintf(stderr, "Error Creating Output file: %s\n\n", FileName[i]);
									fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", FileName[i]);
									ret = -1;
								}

						}
					}
				}
			}
		

		}
	}
#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif
	return ret;
}

int ReadEthMessage(uint32_t index, CcaMessage* msg, CcaTime timestamp, uint32_t busid, uint32_t flags)
{
	// convert general message object to ethernet message object
	//CcaEthernetMessage ethMsg = CcaMessage_createEthernetMessage(*msg);
	char chBytes[200] = { 0 };
	char ethBuffer[2000] = { 0 };
	uint64_t timeStamp_ns = 0;
	// get payload from message
	CcaMessagePayload payload;
	CcaEthernetMessage_payload(msg, &payload);
	memcpy(ethBuffer, payload.Elements, payload.Count);

	const UDP_FRAME_STRUCTURE_T *ptr_bytestream = (const UDP_FRAME_STRUCTURE_T *)&(ethBuffer[28]);
	if (msg == CCA_INVALID_HANDLE)
	{
		std::cout << "Error creating eth message from CcaMessage: " << CcaLib_getLastError() << std::endl;
		return -1;
	}
	// get time from message
	CcaTime timestamp_cca = timestamp;
	char c_timestamp[BUFSIZ] = { '\0' };
	CcaTime_toIsoUtc(&timestamp, c_timestamp, sizeof(c_timestamp));

	// get MAC addresses from message
	uint8_t* sourceMAC;
	CcaEthernetMessage_sourceAddress(msg, &sourceMAC);

	uint8_t* destMAC;
	CcaEthernetMessage_destinationAddress(msg, &destMAC);

	vgm_bool vlanFlag = 0;
	CcaEthernetMessage_isVlanTagged(msg, &vlanFlag);

	uint16_t vlanTagId = 0;
	CcaEthernetMessage_vlanTagId(msg, &vlanTagId);

	vgm_bool dblTagFlag = 0;
	CcaEthernetMessage_isDoubleTagged(msg, &dblTagFlag);

	// get ether type
	uint16_t etherType = 0;
	CcaEthernetMessage_etherType(msg, &etherType);

	//timeStamp_ns = (*(uint64_t*)timestamp * 100); // cca timestamp;
	timeStamp_ns = *(uint64_t*)(timestamp); // cca timestamp;
										 //timeStamp_ns = (timestamp.ts * 100) - start_timestamp_ns ; // Vector timestamp;

	return 0;
}
int kz7b = 0;

int Gen7_Dump_Option_Fun(unsigned int strm_no, Streamver_T Streamver)
{
	bool set_Dump = false;
	if ((strm_no == CDC_STREAM) && (f_cdc_enabled) && (Streamver.CDC == UDP_CDC_STRM))
	{
		set_Dump = true;
		StrmValue = UDP_CDC_STRM;
	}
	else if ((strm_no == DETECTION_STREAM) && (f_det_core_enabled) && (Streamver.DET_CORE == UDP_GEN5_DET_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DET_CORE;
	}
	else if ((strm_no == RDD_STREAM) && (f_rdd_core_enabled) && (Streamver.RDD_CORE == UDP_GEN5_RDD_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_RDD_CORE;
	}
	else if ((strm_no == HEADER_STREAM) && (f_hdr_core_enabled) && (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_HDR_CORE;
	}
	else if ((strm_no == STATUS_STREAM) && (f_status_core_enabled) && (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_STATUS_CORE;
	}
	else if ((strm_no == DEBUG_STREAM) && (f_debug_core_enabled) && (Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DEBUG_CORE;
	}
	else if ((strm_no == VSE_STREAM) && (f_vse_core_enabled) && (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_VSE_CORE;
	}
	else if ((strm_no == MMIC_STREAM) && (f_gen7_mmic_enabled) && (Streamver.MMIC_CORE == UDP_GEN7_MMIC_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_MMIC_CORE;
	}
	else if ((strm_no == ALIGNMENT_STREAM) && (f_gen7_alignment_enabled) && (Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ALIGNMENT_CORE;
	}
	else if ((strm_no == BLOCKAGE_STREAM) && (f_gen7_blockage_enabled) && (Streamver.BLOCKAGE_CORE == UDP_GEN7_BLOCKAGE_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_BLOCKAGE_CORE;
	}
	else if ((strm_no == CALIB_STREAM) && (f_gen7_calib_enabled) && (Streamver.CALIB_CORE == UDP_GEN7_CALIB_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_CALIB_CORE;
	}
	else if ((strm_no == RADAR_CAPABILITY_STREAM) && (f_gen7_radar_capability_enabled) && (Streamver.RADAR_CAPABILITY_CORE == UDP_GEN7_RADAR_CAPABILITY_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_RADAR_CAPABILITY_CORE;
	}
	else if ((strm_no == DOWN_SELECTION_STREAM) && (f_gen7_down_selection_enabled) && (Streamver.DOWN_SELECTION_CORE == UDP_GEN7_DOWN_SELECTION_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_DOWN_SELECTION_CORE;
	}
	else if ((strm_no == ID_STREAM) && (f_gen7_id_enabled) && (Streamver.ID_CORE == UDP_GEN7_ID_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ID_CORE;
	}
	else if ((strm_no == TOI_STREAM) && (f_gen7_toi_enabled) && (Streamver.TOI_CORE == UDP_GEN7_TOI_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_TOI_CORE;
	}
	else if ((strm_no == ROT_SAFETY_FAULTS_STREAM) && (f_gen7_ROT_safety_faults_enabled) && (Streamver.ROT_SAFETY_FAULTS_CORE == UDP_GEN7_ROT_SAFETY_FAULTS_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_SAFETY_FAULTS_CORE;
	}
	else if ((strm_no == ROT_VEHICLE_INFO_STREAM) && (f_gen7_ROT_vehicle_info_enabled) && (Streamver.ROT_VEHICLE_INFO_CORE == UDP_GEN7_ROT_VEHICLE_INFO_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_VEHICLE_INFO_CORE;
	}
	else if ((strm_no == ROT_TRACKER_INFO_STREAM) && (f_gen7_ROT_tracker_info_enabled) && (Streamver.ROT_TRACKER_INFO_CORE == UDP_GEN7_ROT_TRACKER_INFO_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_TRACKER_INFO_CORE;
	}
	else if ((strm_no == ROT_OBJECT_STREAM) && (f_gen7_ROT_object_stream_enabled) && (Streamver.ROT_OBJECT_CORE == UDP_GEN7_ROT_OBJECT_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_OBJECT_CORE;
	}
	else if ((strm_no == ROT_ISO_OBJECT_STREAM) && (f_gen7_ROT_ROT_ISO_object_stream_enabled) && (Streamver.ROT_ISO_OBJECT_CORE == UDP_GEN7_ROT_ISO_OBJECT_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_ISO_OBJECT_CORE;
	}
	else if ((strm_no == ROT_PROCESSED_DETECTION_STREAM) && (f_gen7_ROT_processed_detection_stream_enabled) && (Streamver.ROT_PROCESSED_DETECTION_CORE == UDP_GEN7_ROT_PROCESSED_DETECTION_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ROT_PROCESSED_DETECTION_CORE;
	}
	else if ((strm_no == DYNAMIC_ALIGNMENT_STREAM) && (f_gen7_dynamic_alignment_enabled) && (Streamver.DYNAMIC_ALIGNMENT_CORE == UDP_GEN7_DYNAMIC_ALIGNMENT_CORE))
	{
	set_Dump = true;
	StrmValue = UDP_GEN7_DYNAMIC_ALIGNMENT_CORE;
	}


	/*else if ((strm_no == e_VSE_CORE_LOGGING_STREAM) && (f_vse_core_enabled) && (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_VSE_CORE;
	}
	else if ((strm_no == e_STATUS_CORE_LOGGING_STREAM) && (f_status_core_enabled) && (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_STATUS_CORE;
	}
	
	else if ((strm_no == e_DET_LOGGING_STREAM) && (f_det_core_enabled) && (Streamver.DET_CORE == UDP_GEN5_DET_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DET_CORE;
	}
	else if ((strm_no == e_TRACKER_LOGGING_STREAM) && (f_tracker_core_enabled) && (Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_TRACKER_CORE;
	}
	else if ((strm_no == e_DEBUG_LOGGING_STREAM) && (f_debug_core_enabled) && (Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DEBUG_CORE;
	}*/
	else
	{
		StrmValue = INVALID_OPTION;
	}



	return set_Dump;
}

int Gen5_Dump_Option_Fun(unsigned int strm_no, Streamver_T Streamver)
{
	bool set_Dump = false;
	if ((strm_no == C0_CORE_MASTER_STREAM) && (f_c0_core_enabled) && (Streamver.C0_Core == UDP_GEN5_C0_CORE) && (cust_id != STLA_FLR4P))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_C0_CORE;
	}
	else if ((strm_no == C0_CUST_MASTER_STREAM) && (f_c0_cust_enabled) && (Streamver.C0_Cust == UDP_GEN5_C0_CUST) && (cust_id != STLA_FLR4P))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_C0_CUST;
	}
	else if ((strm_no == C1_CORE_MASTER_STREAM) && (f_c1_core_enabled) && (Streamver.C1_Core == UDP_GEN5_C1_CORE) && (cust_id != STLA_FLR4P))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_C1_CORE;
	}
	else if ((strm_no == C2_CORE_MASTER_STREAM) && (f_c2_core_enabled) && (Streamver.C2_Core == UDP_GEN5_C2_CORE) && (cust_id != STLA_FLR4P))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_C2_CORE;
	}
	else if ((strm_no == C2_CUST_MASTER_STREAM) && (f_c2_cust_enabled) && (Streamver.C2_Cust == UDP_GEN5_C2_CUST) && (cust_id != STLA_FLR4P))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_C2_CUST;
	}
	else if ((strm_no == CDC_STREAM) && (f_cdc_enabled) && (Streamver.CDC == UDP_CDC_STRM))
	{
		set_Dump = true;
		StrmValue = UDP_CDC_STRM;
	}
	else if ((strm_no == e_BC_CORE_LOGGING_STREAM) && (f_bc_core_enabled) && (Streamver.BC_CORE == UDP_GEN5_BC_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_BC_CORE;
	}
	else if ((strm_no == e_VSE_CORE_LOGGING_STREAM) && (f_vse_core_enabled) && (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_VSE_CORE;
	}
	else if ((strm_no == e_STATUS_CORE_LOGGING_STREAM) && (f_status_core_enabled) && (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_STATUS_CORE;
	}
	else if ((strm_no == e_RDD_CORE_LOGGING_STREAM) && (f_rdd_core_enabled) && (Streamver.RDD_CORE == UDP_GEN5_RDD_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_RDD_CORE;
	}
	else if ((strm_no == e_DET_LOGGING_STREAM) && (f_det_core_enabled) && (Streamver.DET_CORE == UDP_GEN5_DET_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DET_CORE;
	}
	else if ((strm_no == e_HDR_LOGGING_STREAM) && (f_hdr_core_enabled) && (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_HDR_CORE;
	}
	else if ((strm_no == e_TRACKER_LOGGING_STREAM) && (f_tracker_core_enabled) && (Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_TRACKER_CORE;
	}
	else if ((strm_no == e_DEBUG_LOGGING_STREAM) && (f_debug_core_enabled) && (Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN5_DEBUG_CORE;
	}
	else if ((strm_no == e_ALIGNMENT_LOGGING_STREAM) && (f_gen7_alignment_enabled) && (Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_GEN7_ALIGNMENT_CORE;
	}
	else
	{
		StrmValue = INVALID_OPTION;
	}

	return set_Dump;
}

int Dump_Option_Fun(unsigned int strm_no, Streamver_T Streamver)
{
	bool set_Dump = false;
	if ((strm_no == SENSOR_OSI_TRACKER) && (f_osi_enabled) && (Streamver.OSI_STREAM == UDP_OSI))
	{
		set_Dump = true;
		StrmValue = UDP_OSI;
	}
	if ((strm_no == Z7B_LOGGING_DATA) && (f_z7b_enabled) && (Streamver.Z7B_Core == UDP_Z7B_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_Z7B_CORE;
	}
	else if ((strm_no == Z7A_LOGGING_DATA) && (f_z7a_enabled) && (Streamver.Z7A_Core == UDP_Z7A_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_Z7A_CORE;
	}
	else if ((strm_no == Z4_LOGGING_DATA) && (f_z4_enabled) && (Streamver.Z4_Core == UDP_Z4_CORE))
	{
		set_Dump = true;
		StrmValue = UDP_Z4_CORE;
	}
	else if ((strm_no == CDC_DATA) && (f_cdc_enabled) && (Streamver.CDC == UDP_CDC_STRM))
	{
		set_Dump = true;
		StrmValue = UDP_CDC_STRM;
	}
	else if ((strm_no == SRR3_Z7B_CUST_LOGGING_DATA) && (f_z7b_custom_enabled) && (Streamver.Z7B_Customer == UDP_CUSTOM_Z7B))
	{
		set_Dump = true;
		StrmValue = UDP_CUSTOM_Z7B;
	}
	else if ((strm_no == Z4_CUST_LOGGING_DATA) && (f_z4_custom_enabled) && (Streamver.Z4_Customer == UDP_CUSTOM_Z4))
	{
		set_Dump = true;
		StrmValue = UDP_CUSTOM_Z4;
	}
	else if ((strm_no == RADAR_ECU_CORE_0) && (f_ECU0_enabled) && (Streamver.ecu0 == UDP_RDR_ECU0))
	{
		set_Dump = true;
		StrmValue = UDP_RDR_ECU0;
	}
	else if ((strm_no == RADAR_ECU_CORE_1) && (f_ECU1_enabled) && (Streamver.ecu1 == UDP_RDR_ECU1))
	{
		set_Dump = true;
		StrmValue = UDP_RDR_ECU1;
	}
	else if ((strm_no == RADAR_ECU_CORE_3) && (f_ECU3_enabled) && (Streamver.ecu2 == UDP_RDR_ECU3))
	{
		set_Dump = true;
		StrmValue = UDP_RDR_ECU3;
	}
	else if ((strm_no == RADAR_ECU_VRU_CLASSIFIER) && (f_ECU_VRU_Classifier_enabled) && (Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier))
	{
		set_Dump = true;
		StrmValue = UDP_RDR_ECU_VRU_Classifier;
	}
	else if ((strm_no == 11 || strm_no == DSPACE_CUSTOMER_DATA) && (f_dspace_enabled) && (Streamver.DSPACE == UDP_DSPACE))
	{
		set_Dump = true;
		StrmValue = UDP_DSPACE;
	}
	else
	{
		StrmValue = INVALID_OPTION;
	}



	return set_Dump;
}

FILE* fptrmap, * File_Pointer;
int Sorting_Data(DVSU_RECORD_T& UDPRec, int MDF_Return_Code, bool Print_Sorted_Data /*, DVSU_RECORD_T* Sorting_Rec*/)
{
	#define MAX_RADAR_COUNT     6	//Restricting to 6 for Time Complexity - As GEN7 is for 5 Radars - Can be increased when required

	DVSU_RECORD_T Sorting_Rec = { 0 };
	memcpy(&Sorting_Rec, &UDPRec, sizeof(UDPRec));
	memcpy(&Sorting_Rec.payload, &UDPRec.payload, sizeof(UDPRec.payload));

	GEN7_UDP_FRAME_STRUCTURE_T* udp_frame_hdr = { 0 };
	static int Radar_Pos;
	static int Platform_ID[MAX_RADAR_COUNT] = { 0 };
	static int Current_ScanID[MAX_RADAR_COUNT] = { 0 };
	//static int Prev_ScanID[MAX_RADAR_COUNT] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };		//If MAX RADAR COUNT is 20
	static int Prev_ScanID[MAX_RADAR_COUNT] = { -1, -1, -1, -1, -1, -1 };
	static int Stream_Num[MAX_RADAR_COUNT] = { 0 };
	static int Prev_Stream_Num[MAX_RADAR_COUNT] = { -1, -1, -1, -1, -1, -1 };
	static int Prev_SrcTxCount[MAX_RADAR_COUNT] = { -1 };
	static int Current_SrcTxCount[MAX_RADAR_COUNT] = { -1 };
	static bool Resim_Started_Flag[MAX_RADAR_COUNT];
	static bool Resim_Started;
	static bool Print_Sorted_Data_Txt_File = Print_Sorted_Data;
	static int Return_Code = MDF_Return_Code;
	static bool Reset_Flag[MAX_RADAR_COUNT];
	using Sorted_Scanindex_MultiMap = std::multimap<int, map<int, vector<DVSU_RECORD_T>>>;
	using Scanindex_Map = std::map<int, vector<DVSU_RECORD_T>>;

	static vector<DVSU_RECORD_T> UDP_Record_Vector[MAX_RADAR_COUNT]; //Chunks Data
	static Scanindex_Map Sorted_Map[MAX_RADAR_COUNT]; //Scanindex Map - Key -> Scanindex, Value -> Chunk's Data Vector 
	static Sorted_Scanindex_MultiMap Sorted_Scan_ID_Map[MAX_RADAR_COUNT]; //Radar Wise Sorted Scanindex Map Key -> Radar_Pos, Value -> Scanindex and Chunk Data Map
	static Sorted_Scanindex_MultiMap Reset_Sorted_Scan_ID_Map[MAX_RADAR_COUNT]; //Radar Wise Sorted Scanindex Map Key -> Radar_Pos, Value -> Scanindex and Chunk Data Map

	std::vector<DVSU_RECORD_T>::iterator Vector_itr; //Iterator for Vector
	Scanindex_Map::iterator Map_itr; //Iterator for Map
	Sorted_Scanindex_MultiMap::iterator Multi_Map_itr; //Iterator for MultiMap

	unsigned char* udp_frame_payload = UDPRec.payload;

	if ((Sorting_Rec.payload[0] == 0xa5 && Sorting_Rec.payload[1] == 0x1c) || (Sorting_Rec.payload[1] == 0xa5 && Sorting_Rec.payload[0] == 0x1c))
	{
		udp_frame_hdr = (GEN7_UDP_FRAME_STRUCTURE_T*)Sorting_Rec.payload;

		Radar_Pos = udp_frame_hdr->frame_header.sensorId;
		Resim_Started = TRUE;
		Sorted_ScanIndex_Buffer_Count[Radar_Pos - 1] = Sorted_Scan_ID_Map[Radar_Pos].size();
		if (MUDP_master_config->MUDP_Select_Sensor_Status[Radar_Pos - 1]) //Radar Position from Config is -1 from Frame Header
		{
			Current_ScanID[Radar_Pos] = udp_frame_hdr->frame_header.streamRefIndex;
			Platform_ID[Radar_Pos] = udp_frame_hdr->frame_header.sourceInfo; //Platform ID for GEN7 is Logged at SourceInfo
			Stream_Num[Radar_Pos] = udp_frame_hdr->frame_header.streamNumber;
			int stream_num = GEN7MaptoStreamNumber(udp_frame_hdr->frame_header.streamNumber, PLATFORM_GEN7);


			////<------Local Copy to Validate Data
			if (Print_Sorted_Data_Txt_File == TRUE)
			{
				File_Pointer = fopen("Before_Sorting.txt", "a+");
				fprintf(File_Pointer, "scanindex = %d,\t\tchunk_id = %d,\t\tStream_no = %d,\t\tStream_name = %s  \t\n", udp_frame_hdr->frame_header.streamRefIndex, udp_frame_hdr->frame_header.streamChunkIdx, udp_frame_hdr->frame_header.streamNumber, Get_GEN7StreamName(stream_num));
				fclose(File_Pointer);
			}
			////End Of Pritning Scanindexes to Local Text File for Validating ---------->

			if (Stream_Num[Radar_Pos] == e_CALIB_GEN7_STREAM || Stream_Num[Radar_Pos] == e_ROT_INTERNALS_STREAM || Stream_Num[Radar_Pos] == e_ROT_TRACKER_INFO_STREAM && Prev_ScanID[Radar_Pos] != -1)   //CALIB Stream Scanindex is different, Blockage and ROT Tracker is having Scanindex as 0. So, changing them
			{
				Current_ScanID[Radar_Pos] = Prev_ScanID[Radar_Pos];
			}

			if (Prev_Stream_Num[Radar_Pos] != Stream_Num[Radar_Pos] && Prev_Stream_Num[Radar_Pos] != -1) //Inserting Streams Data (Vector Data) to Map 
			{
				if (Sorted_Map[Radar_Pos].find(Prev_Stream_Num[Radar_Pos]) == Sorted_Map[Radar_Pos].end())
				{
					Sorted_Map[Radar_Pos].insert({ Prev_Stream_Num[Radar_Pos], UDP_Record_Vector[Radar_Pos] });
				}
				else
				{
					Map_itr = Sorted_Map[Radar_Pos].find(Prev_Stream_Num[Radar_Pos]);
					for (auto Data : UDP_Record_Vector[Radar_Pos])
					{
						Map_itr->second.push_back(Data);
					}
				}
				UDP_Record_Vector[Radar_Pos].clear();
			}

			if (Current_ScanID[Radar_Pos] != Prev_ScanID[Radar_Pos] && Prev_ScanID[Radar_Pos] != -1 /*&& (Reset_Flag[Radar_Pos] == true && (Current_ScanID[Radar_Pos] <= 65535 || Prev_ScanID[Radar_Pos] <= 65535))*/) //Inserting Scanindex Data (Map Data) to Multi Map
			{
				if (Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]) == Sorted_Scan_ID_Map[Radar_Pos].end())
				{
					Sorted_Scan_ID_Map[Radar_Pos].insert({ Prev_ScanID[Radar_Pos], Sorted_Map[Radar_Pos] });
				}
				else
				{
					Multi_Map_itr = Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]);
					for (auto Stream_Data : Sorted_Map[Radar_Pos])
					{
						if (Multi_Map_itr->second.find(Stream_Data.first) == Multi_Map_itr->second.end())
						{
							Multi_Map_itr->second.insert(Stream_Data);
						}
						else
						{
							Map_itr = Multi_Map_itr->second.find(Stream_Data.first);
							for (auto x : Stream_Data.second)
							{
								Map_itr->second.push_back(x);
							}
						}
					}
				}
				Sorted_Map[Radar_Pos].clear();

				//Handling Reset

				/*((Current_ScanID[Radar_Pos] == 65535 || Prev_ScanID[Radar_Pos] == 65535) &&
					(Current_ScanID[Radar_Pos] == 0 || Prev_ScanID[Radar_Pos] == 0) &&
					Reset_Sorted_Scan_ID_Map[Radar_Pos].size() != 0 && Sorted_Map[Radar_Pos].size() != 0)

					(Reset_Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]) != Reset_Sorted_Scan_ID_Map[Radar_Pos].end()
					|| Reset_Sorted_Scan_ID_Map[Radar_Pos].find(Current_ScanID[Radar_Pos]) != Reset_Sorted_Scan_ID_Map[Radar_Pos].end()) && Reset_Flag[Radar_Pos] == true


					*/

				if (abs(Current_ScanID[Radar_Pos] - Prev_ScanID[Radar_Pos]) >= 60000 && Reset_Flag[Radar_Pos] == FALSE)
				{
					Reset_Sorted_Scan_ID_Map[Radar_Pos] = Sorted_Scan_ID_Map[Radar_Pos];
					Sorted_Scan_ID_Map[Radar_Pos].clear();
					Reset_Flag[Radar_Pos] = true;
				}
				/*else if (abs(Current_ScanID[Radar_Pos] - Prev_ScanID[Radar_Pos]) <= 60000 && Reset_Flag[Radar_Pos] == TRUE)
				{
					if (Reset_Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]) != Reset_Sorted_Scan_ID_Map[Radar_Pos].end())
					{
						Reset_Sorted_Scan_ID_Map[Radar_Pos].insert({ Prev_ScanID[Radar_Pos], Sorted_Map[Radar_Pos] });
					}
					else
					{
						Multi_Map_itr = Reset_Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]);
						for (auto Stream_Data : Sorted_Map[Radar_Pos])
						{
							if (Multi_Map_itr->second.find(Stream_Data.first) == Multi_Map_itr->second.end())
							{
								Multi_Map_itr->second.insert(Stream_Data);
							}
							else
							{
								Map_itr = Multi_Map_itr->second.find(Stream_Data.first);
								for (auto x : Stream_Data.second)
								{
									Map_itr->second.push_back(x);
								}
							}
						}
					}
					Sorted_Map[Radar_Pos].clear();
				}*/
			}


			UDP_Record_Vector[Radar_Pos].push_back(UDPRec); //Pushing Chunk data to Vector

			Prev_ScanID[Radar_Pos] = Current_ScanID[Radar_Pos]; //Assigning Prev Scanindex as Current Scanindex
			Prev_Stream_Num[Radar_Pos] = Stream_Num[Radar_Pos]; //Assigning Prev StreamNumber as Current StreamNumber

			if (Reset_Sorted_Scan_ID_Map[Radar_Pos].size() != 0)
			{
				Resim_Started_Flag[Radar_Pos] = TRUE;
				for (Multi_Map_itr = Reset_Sorted_Scan_ID_Map[Radar_Pos].begin(); Multi_Map_itr != Reset_Sorted_Scan_ID_Map[Radar_Pos].end(); Multi_Map_itr++)
				{
					for (auto const& UDP_Rec_Payload : Multi_Map_itr->second)
					{
						UDPRec = { 0 };			//Resetting UDP Rec to 0. 

						Map_itr = Multi_Map_itr->second.begin();

						//Vector_itr = Map_itr->second.begin();

						UDPRec = *Map_itr->second.begin();		//Updating UDPRec With the Sorted Chunk -> First Chunk in the Map
						Map_itr->second.erase(Map_itr->second.begin());	//Clearing the Copied Chunk Data from the Map 

						////<------Local Copy to Validate Data
						if (Print_Sorted_Data_Txt_File == TRUE)
						{
							udp_frame_hdr = { 0 };	//Resetting UDP_Frame_Hdr to 0.
							udp_frame_hdr = (GEN7_UDP_FRAME_STRUCTURE_T*)UDPRec.payload;
							int Sorted_stream_num = GEN7MaptoStreamNumber(udp_frame_hdr->frame_header.streamNumber, PLATFORM_GEN7);
							File_Pointer = fopen("After_Sorting_While_Reading.txt", "a+");
							//fprintf(File_Pointer, "I = %d \n", Multi_Map_itr->first);
							fprintf(File_Pointer, "scanindex = %d,\t\tchunk_id = %d,\t\tStream_no = %d,\t\tStream_name = %s  \t\n", udp_frame_hdr->frame_header.streamRefIndex, udp_frame_hdr->frame_header.streamChunkIdx, udp_frame_hdr->frame_header.streamNumber, Get_GEN7StreamName(Sorted_stream_num));
							fclose(File_Pointer);
						}
						////End Of Pritning Scanindexes to Local Text File for Validating ---------->

						if (Map_itr->second.size() == 0)		//If Sorted_Map (Map) is Empty Clear the Map
						{
							Multi_Map_itr->second.erase(Map_itr->first);
						}

						if (Multi_Map_itr->second.size() == 0)		//If Sorted_Scan_ID_Map (Multi Map) is Empty Clear the Multi Map
						{
							Reset_Sorted_Scan_ID_Map[Radar_Pos].erase(begin(Reset_Sorted_Scan_ID_Map[Radar_Pos]));
						}
						return 0;
					}
				}
			}
			else if (Sorted_Scan_ID_Map[Radar_Pos].size() >= 8)
			{
				Resim_Started_Flag[Radar_Pos] = TRUE;
				for (Multi_Map_itr = Sorted_Scan_ID_Map[Radar_Pos].begin(); Multi_Map_itr != Sorted_Scan_ID_Map[Radar_Pos].end(); Multi_Map_itr++)
				{
					for (auto const& UDP_Rec_Payload : Multi_Map_itr->second)
					{
						UDPRec = { 0 };			//Resetting UDP Rec to 0. 

						Map_itr = Multi_Map_itr->second.begin();

						//Vector_itr = Map_itr->second.begin();

						UDPRec = *Map_itr->second.begin();		//Updating UDPRec With the Sorted Chunk -> First Chunk in the Map
						Map_itr->second.erase(Map_itr->second.begin());	//Clearing the Copied Chunk Data from the Map 

						////<------Local Copy to Validate Data
						if (Print_Sorted_Data_Txt_File == TRUE)
						{
							udp_frame_hdr = { 0 };	//Resetting UDP_Frame_Hdr to 0.
							udp_frame_hdr = (GEN7_UDP_FRAME_STRUCTURE_T*)UDPRec.payload;
							int Sorted_stream_num = GEN7MaptoStreamNumber(udp_frame_hdr->frame_header.streamNumber, PLATFORM_GEN7);
							File_Pointer = fopen("After_Sorting_While_Reading.txt", "a+");
							//fprintf(File_Pointer, "I = %d \n", Multi_Map_itr->first);
							fprintf(File_Pointer, "scanindex = %d,\t\tchunk_id = %d,\t\tStream_no = %d,\t\tStream_name = %s  \t\n", udp_frame_hdr->frame_header.streamRefIndex, udp_frame_hdr->frame_header.streamChunkIdx, udp_frame_hdr->frame_header.streamNumber, Get_GEN7StreamName(Sorted_stream_num));
							fclose(File_Pointer);
						}
						////End Of Pritning Scanindexes to Local Text File for Validating ---------->

						if (Map_itr->second.size() == 0)		//If Sorted_Map (Map) is Empty Clear the Map
						{
							Multi_Map_itr->second.erase(Map_itr->first);
						}

						if (Multi_Map_itr->second.size() == 0)		//If Sorted_Scan_ID_Map (Multi Map) is Empty Clear the Multi Map
						{
							Sorted_Scan_ID_Map[Radar_Pos].erase(begin(Sorted_Scan_ID_Map[Radar_Pos]));
						}
						return 0;
					}
				}
			}
			else if (Resim_Started_Flag[Radar_Pos] == TRUE && Sorted_Scan_ID_Map[Radar_Pos].size() != 0)
			{
				for (Multi_Map_itr = Sorted_Scan_ID_Map[Radar_Pos].begin(); Multi_Map_itr != Sorted_Scan_ID_Map[Radar_Pos].end(); Multi_Map_itr++)
				{
					for (auto const& UDP_Rec_Payload : Multi_Map_itr->second)
					{
						UDPRec = { 0 };			//Resetting UDP Rec to 0. 

						Map_itr = Multi_Map_itr->second.begin();

						UDPRec = *Map_itr->second.begin();		//Updating UDPRec With the Sorted Chunk -> First Chunk in the Map
						Map_itr->second.erase(Map_itr->second.begin());	//Clearing the Copied Chunk Data from the Map 

						////<------Local Copy to Validate Data
						if (Print_Sorted_Data_Txt_File == TRUE)
						{
							udp_frame_hdr = { 0 };	//Resetting UDP_Frame_Hdr to 0.
							udp_frame_hdr = (GEN7_UDP_FRAME_STRUCTURE_T*)UDPRec.payload;
							int Sorted_stream_num = GEN7MaptoStreamNumber(udp_frame_hdr->frame_header.streamNumber, PLATFORM_GEN7);
							File_Pointer = fopen("After_Sorting_While_Reading.txt", "a+");
							//fprintf(File_Pointer, "I = %d \n", Multi_Map_itr->first);
							fprintf(File_Pointer, "scanindex = %d,\t\tchunk_id = %d,\t\tStream_no = %d,\t\tStream_name = %s  \t\n", udp_frame_hdr->frame_header.streamRefIndex, udp_frame_hdr->frame_header.streamChunkIdx, udp_frame_hdr->frame_header.streamNumber, Get_GEN7StreamName(Sorted_stream_num));
							fclose(File_Pointer);
						}
						////End Of Pritning Scanindexes to Local Text File for Validating ---------->

						if (Map_itr->second.size() == 0)		//If Sorted_Map (Map) is Empty Clear the Map
						{
							Multi_Map_itr->second.erase(Map_itr->first);
						}

						if (Multi_Map_itr->second.size() == 0)		//If Sorted_Scan_ID_Map (Multi Map) is Empty Clear the Multi Map
						{
							Sorted_Scan_ID_Map[Radar_Pos].erase(begin(Sorted_Scan_ID_Map[Radar_Pos]));
						}
						return 0;
					}
				}
			}
			UDPRec = { 0 };			//Resetting UDP Rec to 0. -> Upto Sorting Reset Should be done to ignore Repeatative Data
		}
		else
		{
			return 0;
		}
	}
	else if (MDF_Return_Code != 0 && Resim_Started == TRUE)
	{
		for (int Radar_Pos = 0; Radar_Pos < MAX_RADAR_COUNT; Radar_Pos++)
		{
			if (Prev_Stream_Num[Radar_Pos] != -1 && UDP_Record_Vector[Radar_Pos].size() != 0)
			{
				if (Sorted_Map[Radar_Pos].find(Prev_Stream_Num[Radar_Pos]) == Sorted_Map[Radar_Pos].end())
				{
					Sorted_Map[Radar_Pos].insert({ Prev_Stream_Num[Radar_Pos], UDP_Record_Vector[Radar_Pos] });
				}
				else
				{
					Map_itr = Sorted_Map[Radar_Pos].find(Prev_Stream_Num[Radar_Pos]);
					for (auto Data : UDP_Record_Vector[Radar_Pos])
					{
						Map_itr->second.push_back(Data);
					}
				}
				Prev_Stream_Num[Radar_Pos] = -1;
				UDP_Record_Vector[Radar_Pos].clear();
			}

			if (Sorted_Map[Radar_Pos].size() != 0)
			{
				if (Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]) == Sorted_Scan_ID_Map[Radar_Pos].end())
				{
					Sorted_Scan_ID_Map[Radar_Pos].insert(pair<int, map<int, vector<DVSU_RECORD_T>>>(Prev_ScanID[Radar_Pos], Sorted_Map[Radar_Pos]));
				}
				else
				{
					Multi_Map_itr = Sorted_Scan_ID_Map[Radar_Pos].find(Prev_ScanID[Radar_Pos]);
					for (auto Stream_Data : Sorted_Map[Radar_Pos])
					{
						Multi_Map_itr->second.insert(Stream_Data);
					}
				}
				Sorted_Map[Radar_Pos].clear();
			}
		}

		for (int Radar_Pos = 0; Radar_Pos < MAX_RADAR_COUNT; Radar_Pos++)
		{
			if (Resim_Started_Flag[Radar_Pos] == TRUE 
				&& ((MUDP_master_config->MUDP_input_path == INPUT_FILE_TYPE_T::SEQUENTIAL_Mode && Sorted_Scan_ID_Map[Radar_Pos].size() != 0) || (MUDP_master_config->MUDP_input_path == INPUT_FILE_TYPE_T::CONTINUOUS_Mode && Sorted_Scan_ID_Map[Radar_Pos].size() != 1)))
			{
				for (Multi_Map_itr = Sorted_Scan_ID_Map[Radar_Pos].begin(); Multi_Map_itr != Sorted_Scan_ID_Map[Radar_Pos].end(); Multi_Map_itr++)
				{
					for (auto const& UDP_Rec_Payload : Multi_Map_itr->second)
					{
						UDPRec = { 0 };			//Resetting UDP Rec to 0. 

						Map_itr = Multi_Map_itr->second.begin();

						UDPRec = *Map_itr->second.begin();		//Updating UDPRec With the Sorted Chunk -> First Chunk in the Map
						Map_itr->second.erase(Map_itr->second.begin());	//Clearing the Copied Chunk Data from the Map 

						////<------Local Copy to Validate Data
						if (Print_Sorted_Data_Txt_File == TRUE)
						{
							udp_frame_hdr = { 0 };	//Resetting UDP_Frame_Hdr to 0.
							udp_frame_hdr = (GEN7_UDP_FRAME_STRUCTURE_T*)UDPRec.payload;
							int Sorted_stream_num = GEN7MaptoStreamNumber(udp_frame_hdr->frame_header.streamNumber, PLATFORM_GEN7);
							File_Pointer = fopen("After_Sorting_While_Reading.txt", "a+");
							//fprintf(File_Pointer, "I = %d \n", Multi_Map_itr->first);
							fprintf(File_Pointer, "scanindex = %d,\t\tchunk_id = %d,\t\tStream_no = %d,\t\tStream_name = %s  \t\n", udp_frame_hdr->frame_header.streamRefIndex, udp_frame_hdr->frame_header.streamChunkIdx, udp_frame_hdr->frame_header.streamNumber, Get_GEN7StreamName(Sorted_stream_num));
							fclose(File_Pointer);
						}
						////End Of Pritning Scanindexes to Local Text File for Validating ---------->

						if (Map_itr->second.size() == 0)		//If Sorted_Map (Map) is Empty Clear the Map
						{
							Multi_Map_itr->second.erase(Map_itr->first);
						}

						if (Multi_Map_itr->second.size() == 0)		//If Sorted_Scan_ID_Map (Multi Map) is Empty Clear the Multi Map
						{
							Sorted_Scan_ID_Map[Radar_Pos].erase(begin(Sorted_Scan_ID_Map[Radar_Pos]));
						}
						Sorted_ScanIndex_Buffer_Count[Radar_Pos - 1] = Sorted_Scan_ID_Map[Radar_Pos].size();
						return 0;
					}
				}
			}
		}

		for (int Radar_Pos = 0; Radar_Pos < 6; Radar_Pos++)
		{
			Prev_ScanID[Radar_Pos] = -1; //Resetting Previous Scanindex as reached end of the file
			Current_ScanID[Radar_Pos] = 0;
			Prev_Stream_Num[Radar_Pos] = -1;
			Resim_Started_Flag[Radar_Pos] = FALSE;
			Reset_Flag[Radar_Pos] = FALSE;
			UDP_Record_Vector[Radar_Pos].clear();
		}

		for (int Radar_Pos = 0; Radar_Pos < MAX_RADAR_COUNT; Radar_Pos++)
		{
			//if (/*Resim_Started_Flag[Radar_Pos] == TRUE && */Sorted_Scan_ID_Map[Radar_Pos].size() == 0)
			if (((MUDP_master_config->MUDP_input_path == INPUT_FILE_TYPE_T::SEQUENTIAL_Mode && Sorted_Scan_ID_Map[Radar_Pos].size() == 0) 
				|| (MUDP_master_config->MUDP_input_path == INPUT_FILE_TYPE_T::CONTINUOUS_Mode && Sorted_Scan_ID_Map[Radar_Pos].size() != 1)))
			{
				return MDF_Return_Code;
			}
		}
	}
	else if (MDF_Return_Code != 0 && Resim_Started_Flag[Radar_Pos] == TRUE)
	{
		return MDF_Return_Code;
	}
	return 0;
}


int cca_Read_Rec(std::string log_path, void** pdata_ptr, INT64 *TimeStamp_ns, std::shared_ptr<HtmlReportManager>& sptr_manager, int filecnt)//#HTMLInMUDP
{
	CcaMessage msg;
	CcaMessageClass msgClass;
	CcaTime timestamp;
	uint32_t busid;
	CcaMessagePayload payload;
	CcaMessagePayload payload1;
	bool CreateCCA = true;
	uint32_t flags;
	char version[60] = { '\0' };
	char InputLogName[1024] = { 0 };
	ErrorCode mdfErr = eOK;
	dvlMessageUnion dvl_msg;
	UDP_FRAME_PAYLOAD_T udp_frame_payload;
	DVSU_RECORD_T mf4_udp_frame = { 0 };
	DVSU_RECORD_T record = { 0 };
	INT64 timestamp_ns = 0;
	int64_t tym = 0;
	unsigned int Stream_No = 0;
	UINT8 bustype = 0;
	unsigned int ETH_FRAME_UDP_PAYLOAD_OFFSET = 28;
	int ret = -1;
	void* dataptr = NULL;
	int sourceId = 0;
	CcaLib_init();
	Customer_T Cust_ID = AUDI;
	PacketLossTrackerIntenalDecoder *pGdsrF360Tracker = new PacketLossTrackerIntenalDecoder();
	cout << CcaLib_getVersion() << " - " << CCA_API_VERSION << endl;
	CcaLib_init();

	uint64_t start_time_ns = 0;
	// Create File handle with given path
	CcaFile ccaFileHandle = CcaFile_create(log_path.c_str());
	strcpy(InputLogName, log_path.c_str());
	// Create and init session reader options
	CcaSessionReaderOptions sessReaderOpt;
	CCA_INIT_SESSION_READER_OPTIONS(sessReaderOpt);

	// Create cca session reader object from cca file handle
	CcaSessionReader sessReader = CcaSessionReader_createByFile(ccaFileHandle, &sessReaderOpt);
	// open session reader
	if (!CcaSessionReader_open(sessReader))
	{
		cout << "Error: cannot open session reader" << endl;
		CcaSessionReader_destroy(sessReader);
		return EXIT_FAILURE;
	}

	// create a cca message object 
	//CcaMessage msg; 
	char f_Once = 0;
	uint32_t index = 1;

	while (CcaSessionReader_read(sessReader, &msg))
	{

		CcaMessageClass msgClass;
		uint32_t busid;
		CcaTime timestamp;
		CcaMessagePayload payload;
		uint32_t flags;
		uint32_t id = 0;
		vgm_bool is_ext = 0;
		char InputLogName[1024] = { 0 };
		const UDP_FRAME_STRUCTURE_T* ptr_stream;
		Cust_ID = (Customer_T)record.payload[23];
		// get common properties from message object
		CcaMessage_class(msg, &msgClass);
		CcaMessage_timestamp(msg, &timestamp);
		CcaMessage_busid(msg, &busid);
		if (msgClass == CCA_IF_CAN)
		{
			CcaCanMessage_payload(msg, &payload);
		}
		else if (msgClass == CCA_IF_ETHERNET)
		{
			CcaEthernetMessage_payload(msg, &payload);
		}
	//	CcaMessage_flags(msg, &flags);
		strcpy(InputLogName, log_path.c_str());
		uint32_t BUS_ID = MaptoBusid(busid);


		if (msgClass == CCA_IF_UNKNOWN || msgClass == CCA_IF_EVENT || msgClass == CCA_IF_CCASYSTEM) {
			//start_timestamp_100ns = *(uint64_t*)timestamp;
			//start_timestamp_ns = (uint64_t)(start_timestamp_100ns * 100);
			start_timestamp_ns = *(uint64_t*)timestamp;
			if (start_timestamp_ns && f_Once == 0) {
				printf("\rStart time (ns) = %llu\n", start_timestamp_ns);
				f_Once = 1;
			}
		}
		if (CCA_IF_ETHERNET == msgClass) {
			IsUDPdata = true;
		}
		if (CCA_IF_CAN == msgClass)
		{
			memset(&dvl_msg, 0, sizeof(dvlMessageUnion));
		//	CcaMessage canMsg = CcaMessage_createCanMessage(msg);
			/*	if(-1 == NoUDPdata(msgClass, InputLogName)){
			CcaCanMessage_destroy(canMsg);
			CcaFile_destroy(ccaFileHandle);
			CcaSessionReader_destroy(sessReader);
			}*/
			if (msg == CCA_INVALID_HANDLE)
			{
				std::cout << "Error creating CAN message from CcaMessage: " << CcaLib_getLastError() << std::endl;
				CcaMessage_destroy(msg);
				CcaFile_destroy(ccaFileHandle);
				CcaSessionReader_destroy(sessReader);
				return -1;
			}
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
			{

				if (CallPktOnce) {
					if (-1 == CreateOutputPacketlossReport(InputLogName)) {
						return -1;
					}
					CallPktOnce = false;
				}
			}
			if ((MUDP_master_config->data_extracter_mode[CSV] == 1) && (MUDP_master_config->Radar_Stream_Options[CCA_HDR] == CCA_HDR)) {
				if (busid != 0) {
					if (CreateCcaCANOnce)
					{
						if (-1 == CreateoutputCcaCSV(InputLogName, msgClass, busid))
						{
							return -1;
						}
						CreateCcaCANOnce = false;
					}
				}
			}
			CcaCanMessage_id(msg, &id);
			uint8 dlc = 0;
			//CcaCanMessage_dlc(canMsg, &dlc);

			CcaCanMessage_payload(msg, &payload);
			CcaCanMessage_isExtended(msg, &is_ext);
			dvl_msg.dvlCANFD.info.timestamp = tym - start_time_ns;
			dvl_msg.dvlCANFD.info.messageSize = payload.Count;
			dvl_msg.dvlCANFD.info.messageType = DVL_MESSAGE_TYPE_CAN_FD;
			//dvl_msg.dvlCAN.channel = (f_swap_required)?(channel_swap[0x0F & busid]):(busid);

			//if(bus_map.fnid(busid) == bus_map.end())
			//{
			//	//dvl_msg.dvlCAN.channel = RESERVED_CHANNEL_ID;
			//	//for now just skip these CAN frames..
			//	continue;
			//}else{
			dvl_msg.dvlCANFD.channel = 6;//bus_map[busid]; 
										 //}

			dvl_msg.dvlCANFD.extFrame = (uint8_t)is_ext;
			dvl_msg.dvlCANFD.flags = 0;
			dvl_msg.dvlCANFD.id = id;
			dvl_msg.dvlCANFD.length = payload.Count;
			memset(dvl_msg.dvlCANFD.data, 0, DVL_MESSAGE_PAYLOAD_SIZE_CANFD);
			memcpy(dvl_msg.dvlCANFD.data, payload.Elements, payload.Count);

			//CcaMessage_destroy(msg);
			CcaTime timestamp_can_cca = timestamp;
			char can_timestamp[BUFSIZ] = { '\0' };
			CcaTime_toIsoUtc(&timestamp, can_timestamp, sizeof(can_timestamp));
			//timestamp_ns = ((*(uint64_t*)timestamp) * 100); // cca timestamp;
			timestamp_ns = *(uint64_t*)(timestamp); // cca timestamp;
			if (busid != 0)
			{
				if (fpCCAmf4Dump)
				{
					printccaCanHdr(fpCCAmf4Dump, InputLogName, busid, dvl_msg.dvlCANFD, start_timestamp_ns, timestamp_ns);
				}
			}
		}



		/*filter out some other UDP frames that are in the logs*/

		else if ((CCA_IF_ETHERNET == msgClass) && (payload.Count > 1400))
		{
		//	CcaEthernetMessage ethMsg = CcaMessage_createEthernetMessage(msg);
			if (msg == CCA_INVALID_HANDLE)
			{
				std::cout << "Error creating eth message from CcaMessage: " << CcaLib_getLastError() << std::endl;
				CcaMessage_destroy(msg);
				CcaFile_destroy(ccaFileHandle);
				CcaSessionReader_destroy(sessReader);
				return -1;
			}
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
			{

				if (CallPktOnce) {
					if (-1 == CreateOutputPacketlossReport(InputLogName)) {
						return -1;
					}
					CallPktOnce = false;
				}
			}
			if ((MUDP_master_config->data_extracter_mode[CSV] == 1) && (MUDP_master_config->Radar_Stream_Options[CCA_HDR] == CCA_HDR)) {
				if (busid != 0) {
					if (CreateCcaUDPOnce)
					{
						if (-1 == CreateoutputCcaCSV(InputLogName, msgClass, busid))
						{
							return -1;
						}
						CreateCcaUDPOnce = false;
					}
				}
			}
			CcaEthernetType type;
			CcaEthernetMessage_payload(msg, &payload);
			CcaEthernetMessage_type(msg, &type);
			int src_first = payload.Elements[20];
			int src_second = payload.Elements[21];
			int src_rslt = (src_first << 8) + src_second;
			int des_first = payload.Elements[22];
			int des_second = payload.Elements[23];
			int des_rslt = (des_first << 8) + des_second;
			CcaTime timestamp_cca = timestamp;
			if (CreatxmlOnce)
			{
				if (MUDP_master_config->data_extracter_mode[XML] == 1 && f_Sensor_xml == 1)
				{
					if (src_rslt == 5555 && des_rslt == 5555)
					{
						if (-1 == CreateOutputSensorXMLFile(InputLogName, Cust_ID)) {
							return -1;
						}
						CreatxmlOnce = false;
					}

				}
			}
			if (CreateHilxmlOnce)
			{
				if (MUDP_master_config->data_extracter_mode[XML] == 1 && f_Hil_xml == 1)
				{
					if (src_rslt == 6001 && des_rslt == 6001)
					{
						if (-1 == CreateOutputHILPortXMLFile(InputLogName, Cust_ID)) {
							return -1;
						}
						CreateHilxmlOnce = false;
					}
				}
			}

			char c_timestamp[BUFSIZ] = { '\0' };
			CcaTime_toIsoUtc(&timestamp, c_timestamp, sizeof(c_timestamp));
			//timestamp_ns = ((*(uint64_t*)timestamp) * 100); // cca timestamp;
			timestamp_ns = *(uint64_t*)(timestamp); // cca timestamp;
			obsoluteTimestamp_t = timestamp_ns;
			if (payload.Count == 1546) {
				ETH_FRAME_UDP_PAYLOAD_OFFSET = 70; // with plp header + ipv4 offset
			}
			else if (payload.Count == 1500) {
				ETH_FRAME_UDP_PAYLOAD_OFFSET = 28; // with plp header + ipv4 offset
			}
			else if (payload.Count == 1504) {
				ETH_FRAME_UDP_PAYLOAD_OFFSET = 32; // with plp header + ipv4 offset
			}
			memset(udp_frame_payload.frame, 0, UDP_FRAME_PAYLOAD_SIZE);
			memset(&record.pcTime, 0x00, sizeof(&record.pcTime));
			memset(&record.payload, 0x00, sizeof(&record.payload));
			memcpy(udp_frame_payload.frame, payload.Elements + ETH_FRAME_UDP_PAYLOAD_OFFSET, UDP_FRAME_PAYLOAD_SIZE);
			//memcpy(&record.payload[0], &udp_frame_payload.frame, UDP_FRAME_PAYLOAD_SIZE);
			if ((udp_frame_payload.frame[0] == 0xA5 && udp_frame_payload.frame[1] == 0x1c) || (udp_frame_payload.frame[0] == 0x1c && udp_frame_payload.frame[1] == 0xA5) && strcmp(MUDP_master_config->GenType, "GEN7") == 0)
			{
				memcpy(&record.payload[0], &udp_frame_payload.frame, UDP_FRAME_PAYLOAD_SIZE);
				if (strcmp(MUDP_master_config->GenType, "GEN7") == 0 && MUDP_master_config->Radar_Stream_Options[HDR] != 1) //Sorting Data for other than HDR Dump
				{
					Sorting_Data(record, 0, FALSE /*, &Sorted_udpRec*/); //Send TRUE -> to Print Chunks Info Before Sorting and After Sorting -> for DEBUGGING
				}
				
			}
			else if ((udp_frame_payload.frame[0] == 0xA4 && udp_frame_payload.frame[1] == 0x2c) || (udp_frame_payload.frame[0] == 0x2c && udp_frame_payload.frame[1] == 0xA4) && strcmp(MUDP_master_config->GenType, "GEN6") == 0)
			{
				memcpy(&record.payload[0], &udp_frame_payload.frame, UDP_FRAME_PAYLOAD_SIZE);
			}
			else if ((udp_frame_payload.frame[0] == 0xA3 && udp_frame_payload.frame[1] == 0x18) || (udp_frame_payload.frame[0] == 0x18 && udp_frame_payload.frame[1] == 0xA3) && strcmp(MUDP_master_config->GenType, "GEN5") == 0)
			{
				memcpy(&record.payload[0], &udp_frame_payload.frame, UDP_FRAME_PAYLOAD_SIZE);
			}
			else if ((udp_frame_payload.frame[0] == 0xA2 && udp_frame_payload.frame[1] == 0x18) || (udp_frame_payload.frame[0] == 0x18 && udp_frame_payload.frame[1] == 0xA2))
			{
				memcpy(&record.payload[0], &udp_frame_payload.frame, UDP_FRAME_PAYLOAD_SIZE);
			}
			else
			{
				continue;	//if the udp_frame_payload doesn't match with any APTIV Header Version Ignore the chunk
			}
			//CcaMessage_destroy(msg);
			if (busid != 0)
			{
				if (fpUdpmf4Dump)
				{
					printccaUDPHdr(fpUdpmf4Dump, InputLogName, &record, busid, start_timestamp_ns, timestamp_ns);
				}
			}
 			if ((record.payload[0] != 0xA3 && record.payload[1] != 0x18) && (record.payload[0] != 0x18 && record.payload[1] != 0xA3)  
				&& (record.payload[0] != 0xA4 && record.payload[1] != 0x2C) && (record.payload[0] != 0x2C && record.payload[1] != 0xA4)
				&& (record.payload[0] != 0xA5 && record.payload[1] != 0x1C) && (record.payload[0] != 0x1C && record.payload[1] != 0xA5)) {
				//CcaEthernetMessage_destroy(ethMsg);
				continue;
			}
			else {
				if ((record.payload[0] != 0xA4 && record.payload[1] != 0x2C) && (record.payload[0] != 0x2C && record.payload[1] != 0xA4))
				{
					if ((record.payload[8] == 0 && record.payload[9] == 0))
					{
						continue;
					}
				}
				else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
				{
					if ((record.payload[2] == 0 && record.payload[4] == 0))
					{
						continue;
					}
					else if (record.payload[4] == 5 && record.payload[3] == 84) //Ignoring RadarPos = 5 and Cust ID as STLA_SCALE1/SRR6P As, Radar Pos 5(RC) is not Supported for STLA in RESIM (CYW-4899)
					{
						continue;
					}
				}
			}
			if ((strstr(log_path.c_str(), ".MF4") != NULL) || (strstr(log_path.c_str(), ".mf4") != NULL) || (strstr(log_path.c_str(), ".vpcap") != NULL) || (strstr(log_path.c_str(), ".pcap") != NULL) || (strstr(log_path.c_str(), ".PCAP") != NULL) || (strstr(log_path.c_str(), ".pcapng") != NULL))
			{
				if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
				{
					if (record.payload[8] == UDP_PLATFORM_SRR7_PLUS || record.payload[8] == UDP_PLATFORM_FLR7 || record.payload[8] == UDP_PLATFORM_SRR7_PLUS_HD || record.payload[8] == UDP_PLATFORM_SRR7_E)
					{
						sourceId = record.payload[27] - 1;
						cust_id = PLATFORM_GEN7;//(Customer_T)record.payload[3];
						Stream_No = GEN7MaptoStreamNumber(record.payload[19], cust_id);//(GEN5_Radar_Logging_Data_Source_T)record.payload[22];

					}
				}
				if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
				{
					if (record.payload[2] == UDP_PLATFORM_SRR6 || record.payload[2] == UDP_PLATFORM_SRR6_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE || record.payload[2] == UDP_PLATFORM_STLA_FLR4 || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_CAN || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_ETH || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
					{
						sourceId = record.payload[4] - 1;
						cust_id = (Customer_T)record.payload[3];
						Stream_No = GEN5MaptoStreamNumber(record.payload[22], cust_id);//(GEN5_Radar_Logging_Data_Source_T)record.payload[22];

					}
				}
				else if ((record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3))
				{
				    if (record.payload[8] >= UDP_PLATFORM_SRR5 || record.payload[8] >= UDP_PLATFORM_MCIP_SRR5) // Checking is the platform is other than SRR3
				    {
				    	sourceId = (record.payload[9] - 1);
				    	cust_id = (Customer_T)record.payload[23];
				    	if (sourceId == 19)
				    	{
				    		Stream_No = MaptoEcustreamNumber(record.payload[19]);
				    	}
				    	else
				    	{
				    		Stream_No = MaptostreamNumber(record.payload[19], cust_id);
				    	}
				    
				    }
				}
				else if((record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
				{
					sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
					cust_id = (Customer_T)record.payload[23];
					if (sourceId == 19)
					{
						Stream_No = MaptoEcustreamNumber(record.payload[19]);
					}
					else
					{
						Stream_No = MaptostreamNumber(record.payload[19], cust_id);
					}
				}
				strcpy(InputLogName, log_path.c_str());
				if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
				{
					gen7_Print_StrmVrsn_Info(record, Stream_No);
					//if ((Customer_T)record.payload[3] != 0)
					//{
					//	gen7_Print_StrmVrsn_Info(record, Stream_No);
					//}
				}
				else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
				{
					if ((Customer_T)record.payload[3] != 0)
					{
						gen5_Print_StrmVrsn_Info(record, Stream_No);
					}
				}
				else
				{
					if ((Customer_T)record.payload[23] != 0)
					{
						Print_StrmVrsn_Info(record, Stream_No);
					}
					
				}

				//GDSR TRACKER DECODE
				if ((record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3))
				{
					if ((record.payload[8] >= UDP_PLATFORM_SRR5 || record.payload[8] >= UDP_PLATFORM_MCIP_SRR5) && record.payload[8] != UDP_SOURCE_CUST_DSPACE)
					{
						sourceId = (record.payload[9] - 1);
						F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
						unsigned char radar_index = RDR_POS_ECU;
						cust_id = (Customer_T)record.payload[23];

						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							if (sourceId == RDR_POS_ECU)
							{
								if (Stream_No == RADAR_ECU_INTERNAL_CORE1)
								{
									F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
									static int frame_loss_count = 0;
									memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);
									if (MUDP_master_config->MUDP_Radar_ECU_status == 1)
										status = pGdsrF360Tracker->start_F360TrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);
								}
							}
							else
							{
								unsigned char radar_index = 0;
								if (Stream_No == SENSOR_GDSR_TRACKER_INTERNAL) {

									F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
									static int frame_loss_count = 0;
									memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

									radar_index = (m_Udp_Payload[0].frame[9] - 1);

									if (checkMidHighSensorsEnabled() == true && (MUDP_master_config->MUDP_Radar_ECU_status == 1 || MUDP_master_config->MUDP_Radar_ECU_status == 0) && MUDP_master_config->Bpillar_status == 1) { // HIGH varaint
										if (radar_index != BPIL_POS_L && radar_index != BPIL_POS_R) {
											status = DPH_F360_GDSR_FRAME_UNAVAILABLE;
										}
										else {
											if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_index] != 0)
												status = pGdsrF360Tracker->start_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);
										}
									}
									else {
										if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_index] != 0)
											status = pGdsrF360Tracker->start_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]); // LOW varaint
									}
								}
							}
						}
					}
				}
				else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
				{
					if (record.payload[2] == UDP_PLATFORM_SRR6 || record.payload[2] == UDP_PLATFORM_SRR6_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE || record.payload[2] == UDP_PLATFORM_STLA_FLR4 || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_CAN || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_ETH || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
					{
						sourceId = record.payload[4] - 1;
						cust_id = (Customer_T)record.payload[3];
						Stream_No = GEN5MaptoStreamNumber(record.payload[22], cust_id); //(GEN5_Radar_Logging_Data_Source_T)record.payload[22];
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							unsigned char gen5_radar_index = 0;
							if (Stream_No == GEN5_SENSOR_GDSR_TRACKER_INTERNAL)
							{
								F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
								static int frame_loss_count = 0;
								memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

								gen5_radar_index = (m_Udp_Payload[0].frame[4] - 1);

								if (MUDP_master_config->MUDP_Select_Sensor_Status[gen5_radar_index] != 0)
									status = pGdsrF360Tracker->start_GEN5_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);
							}
						}
					}
				}
				else if((record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
				{
					if (record.payload[8] >= UDP_PLATFORM_CUST_SRR3_RL && record.payload[8] <= UDP_PLATFORM_CUST_SRR3_FC) {
						sourceId = (uint8_t)(record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL);
					}
					// For DSPACE UDP logs, radar position is assigned as RDR_POS_DSPACE
					else if (record.payload[8] == UDP_SOURCE_CUST_DSPACE) {
						sourceId = RDR_POS_DSPACE;
					}
					else if (record.payload[8] == BPILLAR_STATUS_BP_L || record.payload[8] == UDP_PLATFORM_FORD_SRR5) {
						sourceId = RDR_POS_FC;
					}
					cust_id = (Customer_T)record.payload[23];
					if (sourceId == 19)
					{
						Stream_No = MaptoEcustreamNumber(record.payload[19]);
					}
					else
					{
						Stream_No = MaptostreamNumber(record.payload[19], cust_id);
					}
				}
				else if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
				{
					if (record.payload[8] == UDP_PLATFORM_SRR7_PLUS || record.payload[8] == UDP_PLATFORM_FLR7 || record.payload[8] == UDP_PLATFORM_SRR7_PLUS_HD || record.payload[8] == UDP_PLATFORM_SRR7_E)
					{
						sourceId = record.payload[27] - 1;
						cust_id = PLATFORM_GEN7; //(Customer_T)record.payload[3];
						Stream_No = GEN7MaptoStreamNumber(record.payload[19], cust_id); //(GEN5_Radar_Logging_Data_Source_T)record.payload[22];
						/*if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							unsigned char gen7_radar_index = 0;
							if (Stream_No == GEN5_SENSOR_GDSR_TRACKER_INTERNAL)
							{
								F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
								static int frame_loss_count = 0;
								memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

								gen7_radar_index = (m_Udp_Payload[0].frame[4] - 1);

								if (MUDP_master_config->MUDP_Select_Sensor_Status[gen7_radar_index] != 0)
									status = pGdsrF360Tracker->start_GEN5_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const&)m_Udp_Payload[0]);
							}
						}*/
					}
				}

				if (MUDP_master_config->data_extracter_mode[XML] == 1 && f_Sensor_xml == 1)
				{
					if (CreatxmlOnce)
					{
						if (-1 == CreateOutputSensorXMLFile(InputLogName, cust_id)) {
							return -1;
						}
						CreatxmlOnce = false;
					}
				}
				if (MUDP_master_config->Radar_Stream_Options[CCA_HDR] != CCA_HDR)
				{
					if (CreateOnce) {
						if (-1 == CreateOutputCSVFile(InputLogName, record)) {
							//CcaEthernetMessage_destroy(ethMsg);
							return -1;
						}
						CreateOnce = false;
					}
				}
				if ((MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1|| MUDP_master_config->data_extracter_mode[XML] == 1) || ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
				{
					if (((MUDP_master_config->data_extracter_mode[CSV] == 0) && (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)&& (MUDP_master_config->data_extracter_mode[XML] == 0)) && ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
					{
						if (fCreate && (Stream_No != SENSOR_GDSR_TRACKER_INTERNAL) && (Stream_No != RADAR_ECU_INTERNAL_CORE1) && (Stream_No != GEN5_SENSOR_GDSR_TRACKER_INTERNAL)) {
							if (-1 == ExtractAndListDVSU(Streamver, InputLogName, NULL, NULL, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
								ret = -1;
								break;
							}
						}
					}
					else
					{
						if (!(f_dspace_enabled))
						{
							if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && sourceId == RADAR_POS))
							{
                                   //#HTMLInMUDP
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;
									break;

								}

							}
						}
						else
						{
							if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0) && (MUDP_master_config->packet_loss_statistics == 0) && (MUDP_master_config->data_extracter_mode[Quality_Check] == 0))
							{     //#HTMLInMUDP
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;
									break;

								}
							}
							else
							{
								printf("\n[WARNING]: Do not enable packetloss report attributes in the config\n ");
								ret = -1;
								break;
							}

						}

					}

				}
			}
		}
		else {
			continue;
		}
	}
	CcaFile_destroy(ccaFileHandle);
	CcaSessionReader_destroy(sessReader);

	//Sorting Data for GEN7 Customers. As, There is no end of file in CCA Lib. So, To process remaining chunks calling SortingData and ExtractAndListDVSU
	if (strcmp(MUDP_master_config->GenType, "GEN7") == 0 && MUDP_master_config->Radar_Stream_Options[HDR] != 1)
	{
		for (int Radar_Pos = 0; Radar_Pos < MAX_RADAR_COUNT; Radar_Pos++)
		{
			while (Sorted_ScanIndex_Buffer_Count[Radar_Pos] != 0)
			{
				if (strcmp(MUDP_master_config->GenType, "GEN7") == 0 && MUDP_master_config->Radar_Stream_Options[HDR] != 1) //Sorting Data for other than HDR Dump
				{
					memset(&record.pcTime, 0x00, sizeof(&record.pcTime));
					memset(&record.payload, 0x00, sizeof(&record.payload));
					Sorting_Data(record, 1, FALSE /*, &Sorted_udpRec*/);  //Send TRUE -> to Print Chunks Info Before Sorting and After Sorting -> for DEBUGGING. 
				}
				if ((MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) || ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
				{
					if (((MUDP_master_config->data_extracter_mode[CSV] == 0) && (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1) && (MUDP_master_config->data_extracter_mode[XML] == 0)) && ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
					{
						if (fCreate && (Stream_No != SENSOR_GDSR_TRACKER_INTERNAL) && (Stream_No != RADAR_ECU_INTERNAL_CORE1) && (Stream_No != GEN5_SENSOR_GDSR_TRACKER_INTERNAL)) {
							if (-1 == ExtractAndListDVSU(Streamver, InputLogName, NULL, NULL, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
								ret = -1;
								break;
							}
						}
					}
					else
					{
						if (!(f_dspace_enabled))
						{
							if (MUDP_master_config->MUDP_Select_Sensor_Status[Radar_Pos] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && Radar_Pos == RADAR_POS))
							{

								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;
									break;

								}

							}
						}
						else
						{
							if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0) && (MUDP_master_config->packet_loss_statistics == 0) && (MUDP_master_config->data_extracter_mode[Quality_Check] == 0))
							{
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;
									break;

								}
							}
							else
							{
								printf("\n[WARNING]: Do not enable packetloss report attributes in the config\n ");
								ret = -1;
								break;
							}

						}

					}

				}
			}
		}
	}

	if (pGdsrF360Tracker) {
		delete pGdsrF360Tracker;
		pGdsrF360Tracker = nullptr;
	}
	return 0;
}
int ccaVigemRecDump(std::string log_path, void** pdata_ptr, INT64 *TimeStamp_ns)
{
	CcaMessage msg;
	CcaMessageClass msgClass;
	CcaTime timestamp;
	uint32_t busid;
	CcaMessagePayload payload;
	uint32_t flags;
	INT64 timestamp_ns = 0;
	UINT8 bustype = 0;
	uint64_t start_time_ns = 0;
	int ret = -1;
	start_timestamp_ns = 0;
	CcaLib_init();
	char f_Once = 0;
	uint32_t index = 1;
	csvFileCreate(log_path);
	CcaLib_init();
	CcaFile ccaFileHandle = CcaFile_create(log_path.c_str());
	CcaSessionReaderOptions sessReaderOpt;
	CCA_INIT_SESSION_READER_OPTIONS(sessReaderOpt);
	CcaSessionReader sessReader = CcaSessionReader_createByFile(ccaFileHandle, &sessReaderOpt);
	if (!CcaSessionReader_open(sessReader))
	{
		cout << "Error: cannot open session reader" << endl;
		CcaSessionReader_destroy(sessReader);
		return EXIT_FAILURE;
	}

	while (CcaSessionReader_read(sessReader, &msg))
	{
		uint32_t flags;
		uint32_t id = 0;
		vgm_bool is_ext = 0;
		vgm_bool is_timeStamp = 0;
		// get common properties from message object
		CcaMessage_class(msg, &msgClass);
		is_timeStamp = CcaMessage_timestamp(msg, &timestamp);
		CcaMessage_busid(msg, &busid);
		CcaCanMessage_payload(msg, &payload);
	//	CcaMessage_flags(msg, &flags);

		if (msgClass == CCA_IF_UNKNOWN || msgClass == CCA_IF_EVENT || msgClass == CCA_IF_CCASYSTEM)
		{
			//start_timestamp_100ns = *(uint64_t*)timestamp;
			//start_timestamp_ns = (uint64_t)(*(uint64_t*)timestamp * 100);
			start_timestamp_ns = *(uint64_t*)timestamp;
			if (start_timestamp_ns && f_Once == 0)
			{
				printf("\rStart time (ns) = %llu\n", start_timestamp_ns);
				f_Once = 1;
			}
		}
		if (CCA_IF_ETHERNET == msgClass)
		{
			handleCCAEthMessage(index, &msg, timestamp, busid);
		}
		if (CCA_IF_CAN == msgClass)
		{
			handleCCACanMessage(index, &msg, timestamp, busid);
		}
		printCursor();
	}
	csvFileClose();
	CcaFile_destroy(ccaFileHandle);
	CcaSessionReader_destroy(sessReader);
	return 0;
}
/*---------------------------------------------------------------------------------------
Function name : validate_HTMLInput_Filename

argument      : filename
Description   : check filename is same as HTMLInputs.json,
                based on the validity , bool variable is set in JsonInputParser

Return type   : void

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
void validate_HTMLInput_Filename(char  arginp_file[1024])
{
//#HTMLInMUDP
	/*--------------------------------------------------------------------
		                  RQ#2: Validating HTML Arguments
	 --------------------------------------------------------------------*/
	
	JsonInputParser& json = JsonInputParser::getInstance();

	if (strcmp(arginp_file, "HTMLInputs.json") == 0)
	{
		std::cout << std::endl << "Third argument is "<< arginp_file<<std::endl;
		json.binputfilename_htmlJson = true;
	}
	else if (strcmp(arginp_file, "f_lists.txt") == 0)
	{
		std::cout << std::endl << "Third argument is " << arginp_file << std::endl;
		json.binputfilename_htmlJson = false;
	}
	

}

/*---------------------------------------------------------------------------------------
Function name : validate_HTML_Arguments

argument      : bool:bhtml_report
argument      : filename
Requirement   :
* ------------------------------------------------------------
*        RQ#3 Validating HTML Arguments
* ------------------------------------------------------------
		read configuration file anc extract bhtml_report
		positive case:
			   if bhtml_report == true and htmlinput filename is correct  then
			   call jsonparser function to collect data
	   negative case:1
			  if bhtml_report == true and htmlinput filename is correct then
			  console print "input argument validation : fail and correct 3rd arg to  htmljson" and exit app
	   negative case:2
			  if bhtml_report == fasle and htmlinput filename is correct then
			  console print " input argument validation : fail correct 3rd arg to  flist " and exit app
------------------------------------------------------------------
	   
Description   : Validate whether HTML Arguments are correct or not
Return type   : int

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
int validate_HTML_Arguments(bool bhtml_report)
{
//#HTMLInMUDP
	JsonInputParser& json = JsonInputParser::getInstance();

	//positive case:
	if (bhtml_report == true && json.binputfilename_htmlJson == true) {
	
		std::cout << std::endl << " HTML input argument validation : PASS" << std::endl;
		return 1;
	
	}
	//negative case:1
	if (bhtml_report == true && json.binputfilename_htmlJson == false) {
	
		std::cout << std::endl << " HTML input argument validation : FAIL" << std::endl;
		std::cout << std::endl << " correct 3rd arg to  htmljson " << std::endl;
		return MUDP_NOK;
	}
	//negative case:2
	if (bhtml_report == false && json.binputfilename_htmlJson == true) {
	
		std::cout << std::endl << " HTML input argument validation : FAIL" << std::endl;
		std::cout << std::endl << " correct 3rd arg to  flist " << std::endl;
		return MUDP_NOK;
	}

	

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
	else if (argc == 4)
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
			if (argc > count)
			{
				strcpy(outputPathName, argv[count]);
			}

			JsonInputParser& json = JsonInputParser::getInstance();
			json.reportpath=outputPathName;
			std::replace(json.reportpath.begin(), json.reportpath.end(), '\\', '/');

			

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

void gen7_CheckSameScanIdforChunk(GEN7_UDPRecord_Header_T* p_gen7_udp_hdr, UINT64 timestamp_ns)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned int Current_StreamIndex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static bool f_block_firstCheck[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	Source_Id = p_gen7_udp_hdr->sensorId - 1;
	Strm_no = GEN7MaptoStreamNumber(p_gen7_udp_hdr->streamNumber, PLATFORM_GEN7 /*p_gen7_udp_hdr->customerId*/);

	if (Strm_no == 0XFF)
	{
		return;
	}

	Current_StreamIndex[Source_Id][Strm_no] = p_gen7_udp_hdr->streamRefIndex;

	if (f_block_firstCheck[Source_Id][Strm_no] == 1 && MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (udp_records_prev[Source_Id][Strm_no].Radar_Position == Source_Id) {
			if (udp_records_prev[Source_Id][Strm_no].streamNumber == Strm_no && p_gen7_udp_hdr->streamChunkIdx == udp_records_prev[Source_Id][Strm_no].streamChunkIdx + 1) {
				if (p_gen7_udp_hdr->streamChunkIdx >= 0 && p_gen7_udp_hdr->streamChunkIdx <= p_gen7_udp_hdr->streamChunks)
				{
					if (udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no] && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)) {
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s] :ScanIndex changed for the [%s] at chunkID:%u, ", GetRadarPosName(Source_Id), Get_GEN7StreamName(Strm_no), p_gen7_udp_hdr->streamChunkIdx);
						fprintf(fCreate, "\t\texpected ScanIndex_%u, received ScanIndex_%u, \n ", udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_gen7_udp_hdr->streamRefIndex);
					}
				}
			}
		}
	}
	//UDP_RecordsInfo_t
	udp_records_prev[Source_Id][Strm_no].streamRefIndex = p_gen7_udp_hdr->streamRefIndex;
	udp_records_prev[Source_Id][Strm_no].MaxChunks = p_gen7_udp_hdr->streamChunks;
	udp_records_prev[Source_Id][Strm_no].streamChunkIdx = p_gen7_udp_hdr->streamChunkIdx;
	udp_records_prev[Source_Id][Strm_no].streamNumber = Strm_no;
	udp_records_prev[Source_Id][Strm_no].Radar_Position = Source_Id;
	udp_records_prev[Source_Id][Strm_no].timestamp = timestamp_ns;
	f_block_firstCheck[Source_Id][Strm_no] = 1;

}

void gen5_CheckSameScanIdforChunk(GEN5_UDPRecord_Header_T* p_gen5_udp_hdr, UINT64 timestamp_ns)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned int Current_StreamIndex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static bool f_block_firstCheck[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	Source_Id = p_gen5_udp_hdr->Radar_Position - 1;
	Strm_no = GEN5MaptoStreamNumber(p_gen5_udp_hdr->stream_Id, p_gen5_udp_hdr->customerId);
	
	if (Strm_no == 0XFF)
	{
		return;
	}

	Current_StreamIndex[Source_Id][Strm_no] = p_gen5_udp_hdr->scan_index;

	if (f_block_firstCheck[Source_Id][Strm_no] == 1 && MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (udp_records_prev[Source_Id][Strm_no].Radar_Position == Source_Id) {
			if (udp_records_prev[Source_Id][Strm_no].streamNumber == Strm_no && p_gen5_udp_hdr->streamChunkIdx == udp_records_prev[Source_Id][Strm_no].streamChunkIdx + 1) {
				if (p_gen5_udp_hdr->streamChunkIdx >= 0 && p_gen5_udp_hdr->streamChunkIdx <= p_gen5_udp_hdr->Total_num_Chunks)
				{
					if (udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no] && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)) {
						if (p_gen5_udp_hdr->Platform != UDP_PLATFORM_FLR4_PLUS || p_gen5_udp_hdr->Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] :Stream index changed for the [%s] at chunkID:%u, \t ", GetRadarPosName(Source_Id), Get_GEN5StreamName(Strm_no), p_gen5_udp_hdr->streamChunkIdx);
							fprintf(fCreate, "\t\texpected stream index_%u, received stream index_%u, \n ", udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_gen5_udp_hdr->scan_index);
						}
						else if (p_gen5_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS || p_gen5_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] :Stream index changed for the [%s] at chunkID:%u, \t ", GetRadarPosName(Source_Id), Get_FLR4PGEN5StreamName(Strm_no), p_gen5_udp_hdr->streamChunkIdx);
							fprintf(fCreate, "\t\texpected stream index_%u, received stream index_%u, \n ", udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_gen5_udp_hdr->scan_index);
						}
					}
				}		
			}
		}
	}
	//UDP_RecordsInfo_t
	udp_records_prev[Source_Id][Strm_no].streamRefIndex = p_gen5_udp_hdr->scan_index;
	udp_records_prev[Source_Id][Strm_no].MaxChunks = p_gen5_udp_hdr->Total_num_Chunks;
	udp_records_prev[Source_Id][Strm_no].streamChunkIdx = p_gen5_udp_hdr->streamChunkIdx;
	udp_records_prev[Source_Id][Strm_no].streamNumber = Strm_no;
	udp_records_prev[Source_Id][Strm_no].Radar_Position = Source_Id;
	udp_records_prev[Source_Id][Strm_no].timestamp = timestamp_ns;
	f_block_firstCheck[Source_Id][Strm_no] = 1;

}

void CheckSameScanIdforChunk(UDPRecord_Header* p_udp_hdr, UINT64 timestamp_ns)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned int Current_StreamIndex[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static bool f_block_firstCheck[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (p_udp_hdr->Platform == UDP_PLATFORM_SRR5 || p_udp_hdr->Platform == UDP_PLATFORM_MCIP_SRR5) { // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position - 1;
		if (Source_Id != RADAR_POS)
		{
			Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
		}
		else
		{
			Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
		}
	}
	else if (p_udp_hdr->Platform == UDP_SOURCE_CUST_DSPACE)
	{
		Source_Id = RDR_POS_DSPACE;
		Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
	}
	else {
		if(p_udp_hdr->customerID == SCANIA_MAN)
		Source_Id = ManSourceid;
		else
			Source_Id = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no = p_udp_hdr->streamNumber;

	}
	if (Strm_no == 0XFF)
	{
		return;
	}
	/*if(MUDP_master_config->MUDP_Radar_ECU_status == 1 && Strm_no == 255){
	Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
	}
	else{
	if(Source_Id == RADAR_POS)
	return ;
	}*/
	Current_StreamIndex[Source_Id][Strm_no] = p_udp_hdr->streamRefIndex;

	if (f_block_firstCheck[Source_Id][Strm_no] == 1 && MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (udp_records_prev[Source_Id][Strm_no].Radar_Position == Source_Id) {
			if (udp_records_prev[Source_Id][Strm_no].streamNumber == Strm_no && p_udp_hdr->streamChunkIdx == udp_records_prev[Source_Id][Strm_no].streamChunkIdx + 1) {
				if (Strm_no != CDC_DATA) {
					if (p_udp_hdr->streamChunkIdx >= 0 && p_udp_hdr->streamChunkIdx <= p_udp_hdr->streamChunks)
					{
						if (udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no] && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)) {
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] :Stream index changed for the [%s] at chunkID:%u, \t ", GetRadarPosName(Source_Id), GetStreamName(Strm_no), p_udp_hdr->streamChunkIdx);
							fprintf(fCreate, "\t\texpected stream index_%u, received stream index_%u, \n ", udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_udp_hdr->streamRefIndex);
						}

						/*if(udp_records_prev[Source_Id][Strm_no].timestamp != timestamp_ns){

						fprintf(fCreate,"***************************************************************************\n");
						fprintf(fCreate,"[%s] :TimeStamp for the [%s] at chunkID:%u, streamRefid_%u \t ",GetRadarPosName(Source_Id), GetStreamName(Strm_no),p_udp_hdr->streamChunkIdx,p_udp_hdr->streamRefIndex);
						fprintf(fCreate,"expected timestamp_%lld, received timestamp_%lld, \n ",udp_records_prev[Source_Id][Strm_no].timestamp,timestamp_ns);
						}*/
					}
				}
				else {
					if (p_udp_hdr->Platform >= UDP_PLATFORM_SRR5 || p_udp_hdr->Platform == UDP_PLATFORM_MCIP_SRR5)
					{
						if (udp_records_prev[Source_Id][Strm_no].streamRefIndex != Current_StreamIndex[Source_Id][Strm_no] && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) {
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] :Stream index changed for the [%s] at chunkID:%u, \t ", GetRadarPosName(Source_Id), GetStreamName(Strm_no), p_udp_hdr->streamChunkIdx);
							fprintf(fCreate, "\t\texpected stream index_%u, received stream index_%u, \n ", udp_records_prev[Source_Id][Strm_no].streamRefIndex, p_udp_hdr->streamRefIndex);
						}
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

void get_Source_TxnCnt(UDPRecord_Header* p_udp_hdr, Customer_T cust_id)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned short Current_Src_TxnCnt[MAX_RADAR_COUNT] = { 0 };
	static unsigned short Prev_Src_TxnCnt[MAX_RADAR_COUNT] = { 0 };
	unsigned int srcDropCount[MAX_RADAR_COUNT] = { 0 };
	static int SrcTxnCnt[MAX_RADAR_COUNT] = { 0 };

	if (p_udp_hdr->Platform == UDP_PLATFORM_SRR5 || p_udp_hdr->Platform == UDP_PLATFORM_MCIP_SRR5) { // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position - 1;
		if (Source_Id != RADAR_POS)
		{
			Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
		}
		else
		{
			Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
		}
	}
	else if (p_udp_hdr->Platform == UDP_SOURCE_CUST_DSPACE)
	{
		Source_Id = RDR_POS_DSPACE;
		Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
	}
	else {
		if (p_udp_hdr->customerID == SCANIA_MAN)
			Source_Id = ManSourceid;
		else
			Source_Id = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no = p_udp_hdr->streamNumber;
	}
	if (Strm_no == 0XFF) {
		return;
	}

	if (srcTxCnt)
	{
		for (int i = 0; i < MAX_RADAR_COUNT; i++)
		{
			SrcTxnCnt[i] = -1;
			Prev_Src_TxnCnt[i] = 0;
		}
		srcTxCnt = false;
	}
	if (cust_id != BMW_LOW || cust_id != BMW_SAT || cust_id != BMW_BPIL)
	{
		if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
		{
			if (_printonce)
			{
				fprintf(fCreate, "\t<Detailed_Error_Information>\n");
				_printonce = false;
			}
		}
	}
	int scan = p_udp_hdr->streamRefIndex;
	if (src_tx_once[Source_Id] == 1 && p_udp_hdr->sourceTxCnt == 0)
	{
		return;
	}
	Current_Src_TxnCnt[Source_Id] = p_udp_hdr->sourceTxCnt;
	if (MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (SrcTxnCnt[Source_Id] != -1 && SrcTxnCnt[Source_Id] != 65536)
		{
			if (Current_Src_TxnCnt[Source_Id] != SrcTxnCnt[Source_Id])
			{
				if (Strm_no != CDC_DATA && Strm_no != CALIBRATION_DATA && Strm_no != RADAR_ECU_CALIB) {
					srcDropCount[Source_Id] = abs(Current_Src_TxnCnt[Source_Id] - Prev_Src_TxnCnt[Source_Id]);
					SrcTxnDropCount[Source_Id] += srcDropCount[Source_Id];
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s],scanindex_%u,[%s] : expected_src_txn_count[%d] and received_src_txn_count[%d]\n ", GetRadarPosName(Source_Id), p_udp_hdr->streamRefIndex, GetStreamName(Strm_no), SrcTxnCnt[Source_Id], Current_Src_TxnCnt[Source_Id]);
					}
				}
			}
		}
	}

	Prev_Src_TxnCnt[Source_Id] = Current_Src_TxnCnt[Source_Id];
	if (Prev_Src_TxnCnt[Source_Id] == 0)
	{
		src_tx_once[Source_Id]++;
	}
	SrcTxnCnt[Source_Id] = (Prev_Src_TxnCnt[Source_Id]) + 1;

}

void gen7_get_Stream_TxnCnt(GEN7_UDPRecord_Header_T* p_udp_hdr)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned short Current_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static unsigned short Prev_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
	{
		if (_printonce)
		{
			fprintf(fCreate, "\t<Detailed_Error_Information>\n");
			_printonce = false;
		}
	}

	if (p_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_PLUS || p_udp_hdr->sourceInfo == UDP_PLATFORM_FLR7 || p_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_PLUS_HD
		|| p_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_E) { // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->sensorId - 1;
		Strm_no = GEN7MaptoStreamNumber(p_udp_hdr->streamNumber, PLATFORM_GEN7/*p_udp_hdr->customerId*/);
	}

	if (Strm_no == 0XFF || Strm_no == 19) {
		return;
	}

	if (onesTxCnt)
	{
		for (int i = 0; i < MAX_RADAR_COUNT; i++)
		{
			for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
			{
				TxnCnt[i][j] = -1;
				Prev_TxnCnt[i][j] = 0;
			}
		}
		onesTxCnt = false;
	}

	Current_TxnCnt[Source_Id][Strm_no] = p_udp_hdr->streamTxCnt;
	if (MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (TxnCnt[Source_Id][Strm_no] != -1 && TxnCnt[Source_Id][Strm_no] != 256)
		{
			if ((Current_TxnCnt[Source_Id][Strm_no] != TxnCnt[Source_Id][Strm_no]) && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1))
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, "\t\t[%s],scanindex_%u,[%s] : expected_strm_txn_count[%d] and received_strm_txn_count[%d]\n ", GetRadarPosName(Source_Id), p_udp_hdr->streamRefIndex, Get_GEN7StreamName(Strm_no), TxnCnt[Source_Id][Strm_no], Current_TxnCnt[Source_Id][Strm_no]);
				
			}
		}
	}

	Prev_TxnCnt[Source_Id][Strm_no] = Current_TxnCnt[Source_Id][Strm_no];
	TxnCnt[Source_Id][Strm_no] = (Prev_TxnCnt[Source_Id][Strm_no]) + 1;

}

void gen5_get_Stream_TxnCnt(GEN5_UDPRecord_Header_T* p_udp_hdr)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned short Current_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static unsigned short Prev_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
	{
		if (_printonce)
		{
			fprintf(fCreate, "\t<Detailed_Error_Information>\n");
			_printonce = false;
		}
	}

	if (p_udp_hdr->Platform == UDP_PLATFORM_SRR6 || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN 
		|| p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS 
		|| p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE || p_udp_hdr->Platform == UDP_PLATFORM_STLA_FLR4) { // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position - 1;
		Strm_no = GEN5MaptoStreamNumber(p_udp_hdr->stream_Id, p_udp_hdr->customerId);		
	}	
	if (Strm_no == 0XFF || Strm_no == 19) {
		return;
	}
	if (onesTxCnt)
	{
		for (int i = 0; i < MAX_RADAR_COUNT; i++)
		{
			for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
			{
				TxnCnt[i][j] = -1;
				Prev_TxnCnt[i][j] = 0;
			}
		}
		onesTxCnt = false;
	}

	Current_TxnCnt[Source_Id][Strm_no] = p_udp_hdr->streamTxCnt;
	if (MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (TxnCnt[Source_Id][Strm_no] != -1 && TxnCnt[Source_Id][Strm_no] != 256)
		{
			if ((Current_TxnCnt[Source_Id][Strm_no] != TxnCnt[Source_Id][Strm_no]) && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1))
			{
				if (p_udp_hdr->Platform != UDP_PLATFORM_FLR4_PLUS && p_udp_hdr->Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s],scanindex_%u,[%s] : expected_strm_txn_count[%d] and received_strm_txn_count[%d]\n ", GetRadarPosName(Source_Id), p_udp_hdr->scan_index, Get_GEN5StreamName(Strm_no), TxnCnt[Source_Id][Strm_no], Current_TxnCnt[Source_Id][Strm_no]);
					/*if (Strm_no != GEN5_CALIB_STREAM)
					{
						loggedmap[logfilename].BMW_Status = "FAIL"; // not using this status for quality tool stream txn cunt  
					}*/
				}
				else if (p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS || p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s],scanindex_%u,[%s] : expected_strm_txn_count[%d] and received_strm_txn_count[%d]\n ", GetRadarPosName(Source_Id), p_udp_hdr->scan_index, Get_FLR4PGEN5StreamName(Strm_no), TxnCnt[Source_Id][Strm_no], Current_TxnCnt[Source_Id][Strm_no]);
				}
			}
		}
	}

	Prev_TxnCnt[Source_Id][Strm_no] = Current_TxnCnt[Source_Id][Strm_no];
	TxnCnt[Source_Id][Strm_no] = (Prev_TxnCnt[Source_Id][Strm_no]) + 1;

}

void gen7_get_CDC_Max_Chunk(GEN7_UDPRecord_Header_T* p_udp_hdr)
{
	unsigned char Source_Id = MapSourceToRadarPos(p_udp_hdr->sensorId);;
	unsigned int Strm_no = GEN7MaptoStreamNumber(p_udp_hdr->streamNumber, PLATFORM_GEN7);
	unsigned int Platform_ID = p_udp_hdr->sourceInfo;
	unsigned int maxcdcchunks = 0;

	if (Platform_ID == UDP_PLATFORM_SRR7_PLUS || Platform_ID == UDP_PLATFORM_FLR7 || Platform_ID == UDP_PLATFORM_SRR7_PLUS_HD || Platform_ID == UDP_PLATFORM_SRR7_E)
	{
		if (Strm_no == CDC_GEN7_STREAM)
		{
			maxcdcchunks = p_udp_hdr->streamChunks;
			if (loggedmap[logfilename].CDC_Max_Chunks[Source_Id] < maxcdcchunks)
			{
				loggedmap[logfilename].CDC_Max_Chunks[Source_Id] = maxcdcchunks;
			}
		}
	}
}

void gen5_get_CDC_Max_Chunk(GEN5_UDPRecord_Header_T* p_udp_hdr)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned int maxcdcchunks = 0;
	//static unsigned short Current_CDC_Max_Chunk[MAX_RADAR_COUNT] = { 0 };

	// Checking the platform is GEN6 or not
	if (p_udp_hdr->Platform == UDP_PLATFORM_SRR6 || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN
		|| p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || p_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS
		|| p_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE || p_udp_hdr->Platform == UDP_PLATFORM_STLA_FLR4) 
	{
		Source_Id = p_udp_hdr->Radar_Position - 1;
		Strm_no = GEN5MaptoStreamNumber(p_udp_hdr->stream_Id, p_udp_hdr->customerId);
	}
	if ((p_udp_hdr->reserved == 1 && p_udp_hdr->customerId == PLATFORM_GEN5) || p_udp_hdr->customerId == STLA_FLR4P)
		maxcdcchunks = p_udp_hdr->Total_num_Chunks + 256;
	else
		maxcdcchunks = p_udp_hdr->Total_num_Chunks;

	if (Strm_no == CDC_12_IQ || Strm_no == CDC_DATA || Strm_no == CDC_8_IQ)
	{
		if (loggedmap[logfilename].CDC_Max_Chunks[Source_Id] < maxcdcchunks)
		{
			//Current_CDC_Max_Chunk[Source_Id] = p_udp_hdr->Total_num_Chunks;
			loggedmap[logfilename].CDC_Max_Chunks[Source_Id] = maxcdcchunks;
		}
	}
	//CDC_Chunk_Loss_Index_Track[Source_Id][p_udp_hdr->streamChunkIdx] = TRUE;
}

void get_Stream_TxnCnt(UDPRecord_Header* p_udp_hdr)
{
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	unsigned short Current_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	static unsigned short Prev_TxnCnt[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };


	if (p_udp_hdr->Platform == UDP_PLATFORM_SRR5 || p_udp_hdr->Platform == UDP_PLATFORM_MCIP_SRR5) { // Checking is the platform is other than SRR3
		Source_Id = p_udp_hdr->Radar_Position - 1;
		if (Source_Id != RADAR_POS)
		{
			Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
		}
		else
		{
			Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
		}
	}
	else if (p_udp_hdr->Platform == UDP_SOURCE_CUST_DSPACE)
	{
		Source_Id = RDR_POS_DSPACE;
		Strm_no = MaptostreamNumber(p_udp_hdr->streamNumber, p_udp_hdr->customerID);
	}
	else {
		if (p_udp_hdr->customerID == SCANIA_MAN)
			Source_Id = ManSourceid;
		else
			Source_Id = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no = p_udp_hdr->streamNumber;
	}
	if (Strm_no == 0XFF || Strm_no == DSPACE_CUSTOMER_DATA) {
		return;
	}
	/*if(MUDP_master_config->MUDP_Radar_ECU_status == 1 && Strm_no == 255){
	Strm_no = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
	}*/
	//else{
	//	if(Source_Id == RADAR_POS)
	//		return ;
	//}

	if (onesTxCnt)
	{
		for (int i = 0; i < MAX_RADAR_COUNT; i++)
		{
			for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
			{
				TxnCnt[i][j] = -1;
				Prev_TxnCnt[i][j] = 0;
			}
		}
		onesTxCnt = false;
	}

	Current_TxnCnt[Source_Id][Strm_no] = p_udp_hdr->streamTxCnt;
	if (MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1) {
		if (TxnCnt[Source_Id][Strm_no] != -1 && TxnCnt[Source_Id][Strm_no] != 256)
		{
			if ((Current_TxnCnt[Source_Id][Strm_no] != TxnCnt[Source_Id][Strm_no]) && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1))
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, "\t\t[%s],scanindex_%u,[%s] : expected_strm_txn_count[%d] and received_strm_txn_count[%d]\n ", GetRadarPosName(Source_Id), p_udp_hdr->streamRefIndex, GetStreamName(Strm_no), TxnCnt[Source_Id][Strm_no], Current_TxnCnt[Source_Id][Strm_no]);
			}
		}
	}

	Prev_TxnCnt[Source_Id][Strm_no] = Current_TxnCnt[Source_Id][Strm_no];
	TxnCnt[Source_Id][Strm_no] = (Prev_TxnCnt[Source_Id][Strm_no]) + 1;

}
void GEN7_ScanIndex_Count(unsigned int ScanIdCurrent, unsigned char radarPos, unsigned char StreamNo, uint8_t  Platform)
{
	bool firstblock[MAX_RADAR_COUNT] = { 0 };



	if (!f_First_refid[radarPos]) {
		first_refidx[radarPos] = ScanIdCurrent;
		f_First_refid[radarPos] = true;
		firstblock[radarPos] = true;
	}
	refidx_current[radarPos] = ScanIdCurrent;
	if (StreamNo != CDC_GEN7_STREAM) {
		if (ScanIndexCount[radarPos][ScanIdCurrent] != 1)	// Ignoring BLOCKAGE, ROT_TRACKER_INFO Streams as there is some logging issue observed
		{			
			if (StreamNo != BLOCKAGE_STREAM && StreamNo != ROT_TRACKER_INFO_STREAM)
			{
				if ((refidx_M_previous[radarPos] != refidx_current[radarPos]) && (firstblock[radarPos] != 1)) {
					ScanIdCount[radarPos]++;
					ScanIndexCount[radarPos][ScanIdCurrent] = 1;
				}
			}
			/*else
			{
				if ((Platform != UDP_PLATFORM_FLR4_PLUS) && (Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
				{
					if ((refidx_Z_previous[radarPos] != refidx_current[radarPos]) && (firstblock[radarPos] != 1)) {
						ScanIdCount[radarPos]++;
						ScanIndexCount[radarPos][ScanIdCurrent] = 1;
					}
					else if (DSPACE_CUSTOMER_DATA)
					{
						if (refidx_Z_previous[radarPos] != refidx_current[radarPos])
						{
							ScanIdCount[radarPos]++;
							ScanIndexCount[radarPos][ScanIdCurrent] = 1;
						}
					}
				}
			}*/
		}
	}
	if (StreamNo != CDC_DATA) 
	{
		refidx_Z_previous[radarPos] = ScanIdCurrent;
		//if ((/*(StreamNo == e_BC_CORE_LOGGING_STREAM) ||*/ (StreamNo == e_DET_LOGGING_STREAM) || (StreamNo == e_TRACKER_LOGGING_STREAM) || (StreamNo == e_DEBUG_LOGGING_STREAM)) && ((Platform == UDP_PLATFORM_FLR4_PLUS) || (Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
		//{
		//	refidx_M_previous[radarPos] = ScanIdCurrent;
		//}
		//else
		//{
		//	if ((Platform != UDP_PLATFORM_FLR4_PLUS) && (Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
		//	{
		//		
		//	}
		//}
	}
	firstblock[radarPos] = false;
}
void GEN5_ScanIndex_Count(unsigned int ScanIdCurrent, unsigned char radarPos, unsigned char StreamNo, uint8_t  Platform)
{
	bool firstblock[MAX_RADAR_COUNT] = { 0 };

	

	if (!f_First_refid[radarPos]) {
		first_refidx[radarPos] = ScanIdCurrent;
		f_First_refid[radarPos] = true;
		firstblock[radarPos] = true;
	}
	refidx_current[radarPos] = ScanIdCurrent;
	if (StreamNo != CDC_DATA) {
		if (ScanIndexCount[radarPos][ScanIdCurrent] != 1)
		{
			if ((/*(StreamNo == e_BC_CORE_LOGGING_STREAM) ||*/ (StreamNo == e_DET_LOGGING_STREAM) || (StreamNo == e_TRACKER_LOGGING_STREAM) || (StreamNo == e_DEBUG_LOGGING_STREAM)) && ((Platform == UDP_PLATFORM_FLR4_PLUS) || (Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
			{
				if ((refidx_M_previous[radarPos] != refidx_current[radarPos]) && (firstblock[radarPos] != 1)) {
					ScanIdCount[radarPos]++;
					ScanIndexCount[radarPos][ScanIdCurrent] = 1;
				}
			}
			else
			{
				if ((Platform != UDP_PLATFORM_FLR4_PLUS) && (Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
				{
					if (StreamNo == C0_CORE_MASTER_STREAM || StreamNo == C1_CORE_MASTER_STREAM || StreamNo == C2_CORE_MASTER_STREAM)
					{
						if ((refidx_Z_previous[radarPos] != refidx_current[radarPos]) && (firstblock[radarPos] != 1)) {
							ScanIdCount[radarPos]++;
							//printf("Radar Pos %d : Scanindex Count: %d \n", radarPos, ScanIdCount[radarPos]);
							ScanIndexCount[radarPos][ScanIdCurrent] = 1;
						}
						else if (DSPACE_CUSTOMER_DATA)
						{
							if (refidx_Z_previous[radarPos] != refidx_current[radarPos])
							{
								ScanIdCount[radarPos]++;
								ScanIndexCount[radarPos][ScanIdCurrent] = 1;
							}
						}
					}
				}
			}
		}
	}
	if (StreamNo != CDC_DATA) {
		if ((/*(StreamNo == e_BC_CORE_LOGGING_STREAM) ||*/ (StreamNo == e_DET_LOGGING_STREAM) || (StreamNo == e_TRACKER_LOGGING_STREAM) || (StreamNo == e_DEBUG_LOGGING_STREAM)) && ((Platform == UDP_PLATFORM_FLR4_PLUS) || (Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
		{
			refidx_M_previous[radarPos] = ScanIdCurrent;
		}
		else
		{
			if ((Platform != UDP_PLATFORM_FLR4_PLUS) && (Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
			{
				refidx_Z_previous[radarPos] = ScanIdCurrent;
			}
		}
	}
	firstblock[radarPos] = false;
}
void ScanIndex_Count(UDPRecord_Header *p_udp_frame_hdr)
{
	int ScanID_Now[MAX_RADAR_COUNT] = { 0 };
	unsigned char Source_Id = 0;
	unsigned int Strm_no = 0;
	BOOL f_block_end = 0;

	if (p_udp_frame_hdr->Platform == UDP_PLATFORM_SRR5 || p_udp_frame_hdr->Platform == UDP_PLATFORM_MCIP_SRR5) // Checking is the platform is other than SRR3
	{
		Source_Id = p_udp_frame_hdr->Radar_Position - 1;
		if (Source_Id != RADAR_POS)
		{
			Strm_no = MaptostreamNumber(p_udp_frame_hdr->streamNumber, p_udp_frame_hdr->customerID);
		}
		else
		{
			Strm_no = MaptoEcustreamNumber(p_udp_frame_hdr->streamNumber);
		}
	}
	else if (p_udp_frame_hdr->Platform == UDP_SOURCE_CUST_DSPACE)
	{
		Source_Id = RDR_POS_DSPACE;
		Strm_no = MaptostreamNumber(p_udp_frame_hdr->streamNumber, p_udp_frame_hdr->customerID);
	}
	else
	{
		if (p_udp_frame_hdr->customerID == SCANIA_MAN)
			Source_Id = ManSourceid;
		else
			Source_Id = p_udp_frame_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		Strm_no = p_udp_frame_hdr->streamNumber;
	}
	if (Strm_no == 0XFF)
	{
		return;
	}
	ScanID_Now[Source_Id] = p_udp_frame_hdr->streamRefIndex;
	if (Source_Id != RADAR_POS)
	{
		if (g_ScanID_Z_Prev[Source_Id] != ScanID_Now[Source_Id] /*&& g_ScanID_Z_Prev[Source_Id] != 0*/)
		{
			if (g_ScanID_Z_Prev[Source_Id] != ScanID_Now[Source_Id] && g_ScanID_Z_Prev[Source_Id] != 0)
			{
				f_block_end = 1;
				g_ScanID_Z_Prev[Source_Id] = 0;
			}
			else if (Strm_no == DSPACE_CUSTOMER_DATA)
			{
				f_block_end = 1;
				g_ScanID_Z_Prev[Source_Id] = -1;
			}
		}
		if (Strm_no != CDC_DATA)
		{
			g_ScanID_Z_Prev[Source_Id] = ScanID_Now[Source_Id];
		}

		if (f_block_end == 1)
		{
			if (ScanStatus[Source_Id] == 1)
			{
				ScanIdCount[Source_Id]++;
			}
			else
			{
				if (Strm_no == CDC_DATA)
				{
					ScanStatus[Source_Id] = 1;
				}
				else if (Strm_no == DSPACE_CUSTOMER_DATA)
				{
					ScanStatus[Source_Id] = 1;
					ScanIdCount[Source_Id]++;
				}
			}
		}
	}
	else
	{
		if (g_ScanID_ECU_Prev != ScanID_Now[Source_Id] && g_ScanID_ECU_Prev != 0)
		{
			f_block_end = 1;
			g_ScanID_ECU_Prev = 0;
		}
		g_ScanID_ECU_Prev = ScanID_Now[Source_Id];
		if (f_block_end == 1)
		{
			ScanIdCount[Source_Id]++;
		}
	}
}

void GEN7_get_scanindex_info(unsigned int timestamp, unsigned int ScanIdCurrent, unsigned char radarPos, unsigned char StreamNo, uint8_t  Platform)
{
	bool firstblock[MAX_RADAR_COUNT] = { 0 };
	unsigned8_T f_CDC_Cmplt[MAX_RADAR_COUNT] = { 0 };
	unsigned8_T f_UDP_Cmplt[MAX_RADAR_COUNT] = { 0 };
	//unsigned8_T f_M_Cmplt[MAX_RADAR_COUNT] = { 0 };
	unsigned int scanIndexDropCount[MAX_RADAR_COUNT] = { 0 };
	bool resetFlag = 0;

	if (!f_Firstscan[radarPos]) {
		first_scan_idx[radarPos] = ScanIdCurrent;
		f_Firstscan[radarPos] = true;
		firstblock[radarPos] = true;
	}

	gen7_scan_idx_current[radarPos] = ScanIdCurrent;

	if (StreamNo != CDC_DATA) {
		if (Platform == UDP_PLATFORM_SRR7_PLUS || Platform == UDP_PLATFORM_FLR7 || Platform == UDP_PLATFORM_SRR7_PLUS_HD || Platform == UDP_PLATFORM_SRR7_E)
		{
			if ((gen7_scan_idx_previous[radarPos] != gen7_scan_idx_current[radarPos]) && (firstblock[radarPos] != 1)) {
				f_UDP_Cmplt[radarPos] = 1;
				//ScanIdCount[radarPos]++;
			}
		}
	}
	else {
		if (gen7_scan_idx_previous[radarPos] != gen7_scan_idx_current[radarPos] && (firstblock[radarPos] != 1)) {
			f_CDC_Cmplt[radarPos] = 1;
		}
	}

	std::string posName = GetRadarPosName(radarPos);

	if (MUDP_master_config->MUDP_Select_Sensor_Status[radarPos] == 1) {
		if (f_UDP_Cmplt[radarPos] == 1 && gen7_scan_idx_previous[radarPos] != 0) {
			if (Platform == UDP_PLATFORM_SRR7_PLUS || Platform == UDP_PLATFORM_FLR7 || Platform == UDP_PLATFORM_SRR7_PLUS_HD || Platform == UDP_PLATFORM_SRR7_E) {
				if ((gen7_scan_idx_current[radarPos] != gen7_scan_idx_previous[radarPos]) && (gen7_scan_idx_previous[radarPos] != 0))
				{
					if (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos] >= 2) {
						scanIndexDropCount[radarPos] = abs(gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]);
					}
					if (gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && (gen7_scan_idx_current[radarPos] == 0 || (gen7_scan_idx_current[radarPos] < gen7_scan_idx_previous[radarPos]))) {
						resetFlag = 1;
						auto str = std::to_string(gen7_scan_idx_previous[radarPos]);
						//loggedmap[logfilename].BMW_Status = "FAIL";
						loggedmap[logfilename].Observation += "[" + posName + "] Sensor reset found after the ScanIndex_" + str + " .";
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] : Reset happened at the scanindex_[%d], previous scanindex_[%d] [ERROR_SENSOR_RESET]\n", GetRadarPosName(radarPos), gen7_scan_idx_current[radarPos], gen7_scan_idx_previous[radarPos]);
							memset(&ScanIndexCount[radarPos], 0, sizeof(ScanIndexCount[radarPos]));
						}
					}
					if (!resetFlag) {
						if (scanIndexDropCount[radarPos] >= 2 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_udp[radarPos] = scanIndexDropCount[radarPos] - 1;
							//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", ScanIdDropCount_udp[radarPos]);
							TotalscanIndexDropCount_udp[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", TotalscanIndexDropCount_udp[radarPos]);
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t[%s]:All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u [ERROR_UDP_COMPLETELY_LOST]\n", GetRadarPosName(radarPos), scanIndexDropCount[radarPos], gen7_scan_idx_current[radarPos]/*, gen7_scan_idx_previous[radarPos]*/);
							}
							//scanIndexDropCount[radarPos] = abs(gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]);
						}
						if (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos] > 3 && gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] != 0)
						{
							loggedmap[logfilename].BMW_Status = "FAIL";
							loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex -  " + to_string(gen7_scan_idx_previous[radarPos]) + " : CurrentScanindex -  " + to_string(gen7_scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + " ."; // [MISMATCH_SCAN] sensor RL : Prevscan nnnnn : Currscan nnnnn 
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in UDP : Sensor = (%s), Scan Index = %d [ERROR_UDP_PARTIALLY_LOST] \n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
								Continous_ScanDrop_udp = 1;
							}
						}
					}
				}
			}
		}
		if (f_CDC_Cmplt[radarPos] == 1 && gen7_scan_idx_previous[radarPos] != 0) {
			if ((gen7_scan_idx_current[radarPos] != gen7_scan_idx_previous[radarPos]) && (gen7_scan_idx_previous[radarPos] != 0))
			{
				if (Platform == UDP_PLATFORM_CUST_SRR3_RL || Platform == UDP_PLATFORM_CUST_SRR3_RR || Platform == UDP_PLATFORM_CUST_SRR3_FR || Platform == UDP_PLATFORM_CUST_SRR3_FL || Platform == UDP_PLATFORM_CUST_SRR3_FC) {
					if (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos] >= 2) {
						scanIndexDropCount[radarPos] = abs(gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]);
					}
					if ((gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] == 1) ||
						(gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] == 0)) {
						resetFlag = 1;
					}
					if (!resetFlag /*&& onesCDCScanId[radarPos] == false*/) {
						if (scanIndexDropCount[radarPos] >= 2 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_cdc[radarPos] = scanIndexDropCount[radarPos] - 1;
							TotalscanIndexDropCount_cdc[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//TotalscanIndexDropCount[radarPos]++;
							//loggedmap[logfilename].BMW_Status = "FAIL";

							//if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							//{
							//	fprintf(fCreate, "\t\t=========================================================================================================\n");
							//	fprintf(fCreate, "\t\t[%s]: All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u for CDC, previous ScanIndex_%u\n", GetRadarPosName(radarPos), ScanIdDropCount_cdc[radarPos], gen7_scan_idx_current[radarPos], gen7_scan_idx_previous[radarPos]);
							//}
							for (int i = 1; i < (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]); i++)
							{
								if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d CDC Completely Lost, [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), ((gen7_scan_idx_previous[radarPos]) + i));
								}
								CDC_Complete_Drop_ScanID[radarPos] = (gen7_scan_idx_previous[radarPos]) + i;
							}
						}
					}
				}
				else {
					scanIndexDropCount[radarPos] = abs(gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]);
					if ((gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] == 1) ||
						(gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] == 0)) {
						resetFlag = 1;
					}
					if (!resetFlag /*&& onesCDCScanId[radarPos] == false*/) {
						if (scanIndexDropCount[radarPos] >= 2 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_cdc[radarPos] = scanIndexDropCount[radarPos] - 1;
							TotalscanIndexDropCount_cdc[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//TotalscanIndexDropCount[radarPos]++;
							//loggedmap[logfilename].BMW_Status = "FAIL";							
							//if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							//{
							//	fprintf(fCreate, "\t\t=========================================================================================================\n");
							//	fprintf(fCreate, "\t\t[%s]: All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u for CDC, previous ScanIndex_%u\n", GetRadarPosName(radarPos), ScanIdDropCount_cdc[radarPos], gen7_scan_idx_current[radarPos], gen7_scan_idx_previous[radarPos]);
							//}							
							for (int i = 1; i < (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos]); i++)
							{
								if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d CDC Completely Lost, [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), ((gen7_scan_idx_previous[radarPos]) + i));
								}
								CDC_Complete_Drop_ScanID[radarPos] = (gen7_scan_idx_previous[radarPos]) + i;
							}
						}
					}
				}
				if (gen7_scan_idx_current[radarPos] - gen7_scan_idx_previous[radarPos] > 3 && gen7_scan_idx_previous[radarPos] != -1 && gen7_scan_idx_previous[radarPos] != 65535 && gen7_scan_idx_current[radarPos] != 0)
				{
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex-  " + to_string(gen7_scan_idx_previous[radarPos]) + ": CurrentScanindex-  " + to_string(gen7_scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + " . [ERROR_CDC_FULLY_DROPPED]";
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in CDC : Sensor = (%s), Scan Index = %d [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
					}
					Continous_ScanDrop_cdc = 1;
				}
				if ((CDC_Complete_Drop_ScanID[radarPos] - CDC_Chunk_Drop_ScanID[radarPos]) == 1 || (CDC_Complete_Drop_ScanID[radarPos] - CDC_Chunk_Drop_ScanID[radarPos]) == -1)
				{
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
					if (loggedmap[logfilename].Observation.length() == 0)
					{
						std::string posName = GetRadarPosName(radarPos);
						loggedmap[logfilename].Observation = "CDC Chunk Loss Occured in Continoues Cycles [" + posName + "]\n";
					}
					CDC_Complete_Drop_ScanID[radarPos] = 0;
					CDC_Chunk_Drop_ScanID[radarPos] = 0;
				}
			}
		}
	}
	scanIndexDropCount[radarPos] = 0;
	if (StreamNo != CDC_DATA) {
		if (Platform == UDP_PLATFORM_SRR7_PLUS || Platform == UDP_PLATFORM_FLR7 || Platform == UDP_PLATFORM_SRR7_PLUS_HD || Platform == UDP_PLATFORM_SRR7_E)
		{
			gen7_scan_idx_previous[radarPos] = ScanIdCurrent;
		}
	}
	else {
		//if(onesCDCScanId[radarPos] == false)
		gen7_scan_idx_previous[radarPos] = ScanIdCurrent;
	}
	firstblock[radarPos] = false;
	//onesCDCScanId[radarPos] = false;
}

void get_scanindex_info(unsigned int timestamp,unsigned int ScanIdCurrent, unsigned char radarPos, unsigned char StreamNo, uint8_t  Platform)
{
	bool firstblock[MAX_RADAR_COUNT] = { 0 };
	unsigned8_T f_cdc_Cmplt[MAX_RADAR_COUNT] = { 0 };
	unsigned8_T f_Z_Cmplt[MAX_RADAR_COUNT] = { 0 };
	unsigned8_T f_M_Cmplt[MAX_RADAR_COUNT] = { 0 };
	unsigned int scanIndexDropCount[MAX_RADAR_COUNT] = { 0 };
	bool resetFlag = 0;

	if (!f_Firstscan[radarPos]) {
		first_scan_idx[radarPos] = ScanIdCurrent;
		f_Firstscan[radarPos] = true;
		firstblock[radarPos] = true;
	}
	scan_idx_current[radarPos] = ScanIdCurrent;
	if (StreamNo != CDC_DATA) {
		if ((/*(StreamNo == e_BC_CORE_LOGGING_STREAM) ||*/ (StreamNo == e_DET_LOGGING_STREAM) || (StreamNo == e_TRACKER_LOGGING_STREAM) || (StreamNo == e_DEBUG_LOGGING_STREAM)) && ((Platform == UDP_PLATFORM_FLR4_PLUS) || (Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
		{
			if ((scan_idx_M_previous[radarPos] != scan_idx_current[radarPos]) && (firstblock[radarPos] != 1)) {
				f_M_Cmplt[radarPos] = 1;
				//ScanIdCount[radarPos]++;
			}
		}
		else
		{
			if ((scan_idx_Z_previous[radarPos] != scan_idx_current[radarPos]) && (firstblock[radarPos] != 1)) {
				f_Z_Cmplt[radarPos] = 1;
				//ScanIdCount[radarPos]++;
			}
		}
	}
	else {
		if (scan_idx_previous[radarPos] != scan_idx_current[radarPos] && (firstblock[radarPos] != 1)) {
			f_cdc_Cmplt[radarPos] = 1;
		}
	}
	std::string posName = GetRadarPosName(radarPos);

	if (MUDP_master_config->MUDP_Select_Sensor_Status[radarPos] == 1) {
		if (f_Z_Cmplt[radarPos] == 1 && scan_idx_Z_previous[radarPos] != 0) {
			if (Platform == UDP_PLATFORM_CUST_SRR3_RL || Platform == UDP_PLATFORM_CUST_SRR3_RR || Platform == UDP_PLATFORM_CUST_SRR3_FR || Platform == UDP_PLATFORM_CUST_SRR3_FL || Platform == UDP_PLATFORM_CUST_SRR3_FC) {
				if ((scan_idx_current[radarPos] != scan_idx_Z_previous[radarPos]) && (scan_idx_Z_previous[radarPos] != 0))
				{
					if (scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos] >= 2) {
						scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos]);
					}
					if (scan_idx_Z_previous[radarPos] != -1 && scan_idx_Z_previous[radarPos] != 65535 && (scan_idx_current[radarPos] == 0 || (scan_idx_current[radarPos] < scan_idx_Z_previous[radarPos]))) {
						resetFlag = 1;
						auto str = std::to_string(scan_idx_Z_previous[radarPos]);
						//loggedmap[logfilename].BMW_Status = "FAIL";
						loggedmap[logfilename].Observation += "[" + posName + "] Sensor reset found after the ScanIndex_" + str + " .";
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] : Reset happened at the scanindex_[%d], previous scanindex_[%d] [ERROR_SENSOR_RESET]\n", GetRadarPosName(radarPos), scan_idx_current[radarPos], scan_idx_Z_previous[radarPos]);
							memset(&ScanIndexCount[radarPos], 0, sizeof(ScanIndexCount[radarPos]));
						}
					}
					if (!resetFlag) {
						if (scanIndexDropCount[radarPos] >= 2 && scan_idx_Z_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_udp[radarPos] = scanIndexDropCount[radarPos] - 1;
							//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", ScanIdDropCount_udp[radarPos]);
							TotalscanIndexDropCount_udp[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", TotalscanIndexDropCount_udp[radarPos]);
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t[%s]:All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u [ERROR_UDP_COMPLETELY_LOST]\n", GetRadarPosName(radarPos), scanIndexDropCount[radarPos], scan_idx_current[radarPos]/*, scan_idx_Z_previous[radarPos]*/);
							}
							//scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos]);
						}
						if (scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos] > 3 && scan_idx_Z_previous[radarPos] != -1 && scan_idx_Z_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
						{
							int diff = scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos];
							loggedmap[logfilename].BMW_Status = "FAIL";
							loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex -  " + to_string(scan_idx_Z_previous[radarPos]) + " : CurrentScanindex -  " + to_string(scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + "  MissedScanIndexCount : " +to_string(diff)+" ."; // [MISMATCH_SCAN] sensor RL : Prevscan nnnnn : Currscan nnnnn 
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in UDP : Sensor = (%s), Scan Index = %d [ERROR_UDP_PARTIALLY_LOST] \n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
								Continous_ScanDrop_udp = 1;
							}
						}
					}
				}
			}
			else {
				if ((scan_idx_current[radarPos] != scan_idx_Z_previous[radarPos]) && (scan_idx_Z_previous[radarPos] != 0))
				{
					scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos]);
					if (scan_idx_Z_previous[radarPos] != -1 && scan_idx_Z_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 0) {
						resetFlag = 1;
						auto str = std::to_string(scan_idx_Z_previous[radarPos]);
						//loggedmap[logfilename].BMW_Status = "FAIL";
						loggedmap[logfilename].Observation += "[" + posName + "] Sensor reset found after the ScanIndex_" + str + " .";
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s] : Reset happened at the scanindex_[%d], previous scanindex_[%d] [ERROR_SENSOR_RESET]\n", GetRadarPosName(radarPos), scan_idx_current[radarPos], scan_idx_Z_previous[radarPos]);
							memset(&ScanIndexCount[radarPos], 0, sizeof(ScanIndexCount[radarPos]));
						}
					}

				if (!resetFlag) {
					if (scanIndexDropCount[radarPos] >= 2 && scan_idx_Z_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
					{
						

						ScanIdDropCount_udp[radarPos] = scanIndexDropCount[radarPos] - 1;
						//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", ScanIdDropCount_udp[radarPos]);

						TotalscanIndexDropCount_udp[radarPos] += (scanIndexDropCount[radarPos] - 1);
						//printf("ScanIdDropCount_udp[radarPos] = [%d]\n", TotalscanIndexDropCount_udp[radarPos]);
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s]:All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u [ERROR_UDP_COMPLETELY_LOST]\n", GetRadarPosName(radarPos), scanIndexDropCount[radarPos], scan_idx_current[radarPos]/*, scan_idx_Z_previous[radarPos]*/);
						}
						//scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos]);
					}


						if (scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos] > 3 && scan_idx_Z_previous[radarPos] != -1 && scan_idx_Z_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
						{
							int diff = scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos];
							loggedmap[logfilename].BMW_Status = "FAIL";
							loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
							loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex -  " + to_string(scan_idx_Z_previous[radarPos]) + " : CurrentScanindex -  " + to_string(scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + " MissedScanIndexCount : " + to_string(diff) + " ."; // [MISMATCH_SCAN] sensor RL : Prevscan nnnnn : Currscan nnnnn 
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in UDP : Sensor = (%s), Scan Index = %d [ERROR_UDP_PARTIALLY_LOST] \n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
								Continous_ScanDrop_udp = 1;
							}
						}
					}
				}
			}
		}
		if (f_cdc_Cmplt[radarPos] == 1 && scan_idx_previous[radarPos] != 0) {
			if ((scan_idx_current[radarPos] != scan_idx_previous[radarPos]) && (scan_idx_previous[radarPos] != 0))
			{
				if (Platform == UDP_PLATFORM_CUST_SRR3_RL || Platform == UDP_PLATFORM_CUST_SRR3_RR || Platform == UDP_PLATFORM_CUST_SRR3_FR || Platform == UDP_PLATFORM_CUST_SRR3_FL || Platform == UDP_PLATFORM_CUST_SRR3_FC) {
					if (scan_idx_current[radarPos] - scan_idx_Z_previous[radarPos] >= 2) {
						scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_previous[radarPos]);
					}
					if ((scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 1) ||
						(scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 0)) {
						resetFlag = 1;
					}
					if (!resetFlag /*&& onesCDCScanId[radarPos] == false*/) {
						if (scanIndexDropCount[radarPos] >= 2 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_cdc[radarPos] = scanIndexDropCount[radarPos] - 1;
							TotalscanIndexDropCount_cdc[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//TotalscanIndexDropCount[radarPos]++;
							//loggedmap[logfilename].BMW_Status = "FAIL";
							
							//if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							//{
							//	fprintf(fCreate, "\t\t=========================================================================================================\n");
							//	fprintf(fCreate, "\t\t[%s]: All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u for CDC, previous ScanIndex_%u\n", GetRadarPosName(radarPos), ScanIdDropCount_cdc[radarPos], scan_idx_current[radarPos], scan_idx_previous[radarPos]);
							//}
							for (int i = 1; i < (scan_idx_current[radarPos] - scan_idx_previous[radarPos]); i++)
							{
								if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d CDC Completely Lost, [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), ((scan_idx_previous[radarPos]) + i));
								}
								CDC_Complete_Drop_ScanID[radarPos] = (scan_idx_previous[radarPos]) + i;
							}
						}
					}
				}
				else {
					scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_previous[radarPos]);
					if ((scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 1) ||
						(scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 0)) {
						resetFlag = 1;
					}
					if (!resetFlag /*&& onesCDCScanId[radarPos] == false*/) {
						if (scanIndexDropCount[radarPos] >= 2 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
						{
							ScanIdDropCount_cdc[radarPos] = scanIndexDropCount[radarPos] - 1;
							TotalscanIndexDropCount_cdc[radarPos] += (scanIndexDropCount[radarPos] - 1);
							//TotalscanIndexDropCount[radarPos]++;
							//loggedmap[logfilename].BMW_Status = "FAIL";							
							//if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							//{
							//	fprintf(fCreate, "\t\t=========================================================================================================\n");
							//	fprintf(fCreate, "\t\t[%s]: All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u for CDC, previous ScanIndex_%u\n", GetRadarPosName(radarPos), ScanIdDropCount_cdc[radarPos], scan_idx_current[radarPos], scan_idx_previous[radarPos]);
							//}							
							for (int i = 1; i < (scan_idx_current[radarPos] - scan_idx_previous[radarPos]); i++)
							{
								if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d CDC Completely Lost, [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), ((scan_idx_previous[radarPos]) + i));
								}
								CDC_Complete_Drop_ScanID[radarPos] = (scan_idx_previous[radarPos]) + i;
							}
						}
					}
				}
				if (scan_idx_current[radarPos] - scan_idx_previous[radarPos] > 3 && scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
				{
					int diff = scan_idx_current[radarPos] - scan_idx_previous[radarPos];
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex-  " + to_string(scan_idx_previous[radarPos]) + ": CurrentScanindex-  " + to_string(scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + " MissedScanIndexCount : " + to_string(diff)+" [ERROR_CDC_FULLY_DROPPED] .";
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in CDC : Sensor = (%s), Scan Index = %d [ERROR_CDC_FULLY_DROPPED] \n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
					}
					Continous_ScanDrop_cdc = 1;
				}
				if ((CDC_Complete_Drop_ScanID[radarPos] - CDC_Chunk_Drop_ScanID[radarPos]) == 1 || (CDC_Complete_Drop_ScanID[radarPos] - CDC_Chunk_Drop_ScanID[radarPos]) == -1)
				{
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
					if (loggedmap[logfilename].Observation.length() == 0)
					{
						std::string posName = GetRadarPosName(radarPos);
						loggedmap[logfilename].Observation = "CDC Chunk Loss Occured in Continoues Cycles [" + posName + "]\n";
					}
					CDC_Complete_Drop_ScanID[radarPos] = 0;
					CDC_Chunk_Drop_ScanID[radarPos] = 0;
				}
			}
		}
		if (f_M_Cmplt[radarPos] == 1 && scan_idx_M_previous[radarPos] != 0) {
			if ((scan_idx_current[radarPos] != scan_idx_M_previous[radarPos]) && (scan_idx_M_previous[radarPos] != 0))
			{
				scanIndexDropCount[radarPos] = abs(scan_idx_current[radarPos] - scan_idx_M_previous[radarPos]);
				
				if (scan_idx_M_previous[radarPos] != -1 && scan_idx_M_previous[radarPos] != 65535 && scan_idx_current[radarPos] == 0) {
					resetFlag = 1;
					auto str = std::to_string(scan_idx_M_previous[radarPos]);
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s] : Reset happened at the scanindex_[%d], previous scanindex_[%d] [ERROR_SENSOR_RESET]\n", GetRadarPosName(radarPos), scan_idx_current[radarPos], scan_idx_Z_previous[radarPos]);
						memset(&ScanIndexCount[radarPos], 0, sizeof(ScanIndexCount[radarPos]));
					}
				}

				if (!resetFlag) {
					if (scanIndexDropCount[radarPos] >= 2 && scan_idx_M_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
					{
						ScanIdDropCount_udp[radarPos] = scanIndexDropCount[radarPos] - 1;
						TotalscanIndexDropCount_udp[radarPos] += (scanIndexDropCount[radarPos] - 1);
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s]:All packets lossed for Scanindexes count[%d] observed at the current Scan Index_%u [ERROR_UDP_COMPLETELY_LOST]\n", GetRadarPosName(radarPos), scanIndexDropCount[radarPos], scan_idx_current[radarPos]/*, scan_idx_Z_previous[radarPos]*/);
						}
					}


					if (scan_idx_current[radarPos] - scan_idx_M_previous[radarPos] > 3 && scan_idx_M_previous[radarPos] != -1 && scan_idx_M_previous[radarPos] != 65535 && scan_idx_current[radarPos] != 0)
					{
						int diff = scan_idx_current[radarPos] - scan_idx_M_previous[radarPos];
						loggedmap[logfilename].BMW_Status = "FAIL";
						loggedmap[logfilename].DQ_Radar_Fail_Status[radarPos] = "FAIL";
						loggedmap[logfilename].RESIM_Radar_Fail_Status[radarPos] = "FAIL";
						loggedmap[logfilename].Observation += " [MISMATCH_SCAN] in Sensor [" + posName + "] : PreviousScanIndex -  " + to_string(scan_idx_M_previous[radarPos]) + " : CurrentScanindex -  " + to_string(scan_idx_current[radarPos]) + " -> Timestamp :" + to_string(timestamp) + "  MissedScanIndexCount : " + to_string(diff)+" ."; // [MISMATCH_SCAN] sensor RL : Prevscan nnnnn : Currscan nnnnn 
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in UDP : Sensor = (%s), Scan Index = %d [ERROR_UDP_PARTIALLY_LOST]\n", GetRadarPosName(radarPos), radar_scan_prev[radarPos]);
							Continous_ScanDrop_udp = 1;
						}
					}
				}
			}
		}
	}
	// ECU scanindex overflow check for  Scan index  reset 
	if (radarPos == RADAR_POS)
	{
		if (MUDP_master_config->MUDP_Radar_ECU_status == 1)
		{
			if (scan_idx_previous[radarPos] != -1 && scan_idx_previous[radarPos] != 2147483647 && scan_idx_current[radarPos] == 0)
			{
				auto str = std::to_string(scan_idx_previous[radarPos]);
				//	loggedmap[logfilename].BMW_Status = "FAIL";
				//	loggedmap[logfilename].Observation += "Sensor reset found after the scanIndex_" + str + "in sensor " + posName + ".";
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s] : Reset happened at the scanindex_[%d], previous scanindex_[%d] [ERROR_SENSOR_RESET]\n", GetRadarPosName(radarPos), scan_idx_current[radarPos], scan_idx_Z_previous[radarPos]);
					memset(&ScanIndexCount[radarPos], 0, sizeof(ScanIndexCount[radarPos]));
				}
			}
		}
	}
	scanIndexDropCount[radarPos] = 0;
	if (StreamNo != CDC_DATA) {
		if ((/*(StreamNo == e_BC_CORE_LOGGING_STREAM) || */(StreamNo == e_DET_LOGGING_STREAM) || (StreamNo == e_TRACKER_LOGGING_STREAM) || (StreamNo == e_DEBUG_LOGGING_STREAM)) && ((Platform == UDP_PLATFORM_FLR4_PLUS) || (Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
		{
			scan_idx_M_previous[radarPos] = ScanIdCurrent;
		}
		else
		{
			if ((Platform != UDP_PLATFORM_FLR4_PLUS) && (Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
			{
				scan_idx_Z_previous[radarPos] = ScanIdCurrent;
			}
		}
	}
	else {
		//if(onesCDCScanId[radarPos] == false)
		scan_idx_previous[radarPos] = ScanIdCurrent;
	}
	firstblock[radarPos] = false;
	//onesCDCScanId[radarPos] = false;
}

void gen7_CDC_PacketLossCount(GEN7_Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type) {
	int Radar_Pos = radar_type;
	CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] = p_pack->proc_info.frame_header.streamRefIndex;

	if ((first_scan_idx[radar_type] + 1 != p_pack->proc_info.frame_header.streamRefIndex) && first_scan_idx[radar_type] != 0)
	{
			if (CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] == CDC_scan_idx_previous[Radar_Pos][CDC_DATA] + 1)
			{
				if (gen7_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks != p_pack->proc_info.frame_header.streamChunks)
				{
					//UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
					//	ScanindexDropCount[radar_type][CDC_DATA]++;
					//	TotalscanIndexDropCount_cdc[radar_type]++;
					loggedmap[logfilename].Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[Radar_Pos] = "FAIL";
					TotalscanIndexDropCount_cdc[Radar_Pos]++;
					cdc_partially_drop++;
					loggedmap[logfilename].log_cdc_partially_drop = cdc_partially_drop;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]:Scan Index_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d  [ERROR_CDC_PARTIALLY_DROPPED] \n", GetRadarPosName(radar_type), gen7_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.frame_header.streamRefIndex, p_pack->proc_info.frame_header.streamChunks, gen7_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks);
					}
					CDC_Chunk_Drop_ScanID[Radar_Pos] = gen7_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.frame_header.streamRefIndex;
				}
				if ((CDC_Complete_Drop_ScanID[Radar_Pos] - CDC_Chunk_Drop_ScanID[Radar_Pos]) == 1 || (CDC_Complete_Drop_ScanID[Radar_Pos] - CDC_Chunk_Drop_ScanID[Radar_Pos]) == -1)
				{
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[Radar_Pos] = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[Radar_Pos] = "FAIL";
					std::string posName = GetRadarPosName(radar_type);
					loggedmap[logfilename].Observation = "CDC completely dropped in Sensor[" + posName + "] : at ScanIndex - " + to_string(CDC_Complete_Drop_ScanID[Radar_Pos]) + "\n";
					CDC_Complete_Drop_ScanID[Radar_Pos] = 0;
					CDC_Chunk_Drop_ScanID[Radar_Pos] = 0;
				}
			}
		if (p_pack->proc_info.crc_status == CRC_INVALID) {     //CRC Check Added for  CYW-2815

			CRCErrorCount[radar_type][CDC_DATA]++;
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), Get_GEN7StreamName(CDC_DATA), p_pack->proc_info.frame_header.streamRefIndex);
			}

		}
		else {
			if (p_pack->proc_info.f_failure_block == 1) {
				//	UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
			}
		}
	}
	CDC_scan_idx_previous[Radar_Pos][CDC_DATA] = CDC_ScanIdCurrent[Radar_Pos][CDC_DATA];
	gen7_pack_working_prev[Radar_Pos][CDC_DATA].proc_info = p_pack->proc_info;

}

void Gen7_CompleteStreamlossinfo(GEN7_UDPRecord_Header_T  p_gen7_udp_frame_hdr)
{
	int platform = p_gen7_udp_frame_hdr.sourceInfo;
	unsigned int StreamNo = GEN7MaptoStreamNumber(p_gen7_udp_frame_hdr.streamNumber, PLATFORM_GEN7);
	unsigned char Source_Id = MapSourceToRadarPos(p_gen7_udp_frame_hdr.sensorId);

	if (platform == UDP_PLATFORM_SRR7_PLUS || platform == UDP_PLATFORM_FLR7 || platform == UDP_PLATFORM_SRR7_PLUS_HD || platform == UDP_PLATFORM_SRR7_E)
	{
		GEN7_Curr_Stream_Scanindex[Source_Id][StreamNo] = p_gen7_udp_frame_hdr.streamRefIndex;

		for (int stream = 0; stream <= MAX_LOGGING_SOURCE; stream++)
		{
			if (/*GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo] != -1 || */GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo] != 0)
			{
				int scanindex_diff = GEN7_Curr_Stream_Scanindex[Source_Id][StreamNo] - GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo];

				if (scanindex_diff > 1 && scanindex_diff <= 5)	//Printing Each Scanindex Info if the diff is less than 5
				{
					for (int diff = 1; diff < scanindex_diff; diff++)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost, [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo] + diff, Get_GEN7StreamName(StreamNo));
					}
					break;
				}
				else if (scanindex_diff > 5)	//printing missed stream details with num of cycles difference if the difference is more than 5
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo], Get_GEN7StreamName(StreamNo), scanindex_diff);
				}
			}
		}
		GEN7_Prev_Stream_Scanindex[Source_Id][StreamNo] = p_gen7_udp_frame_hdr.streamRefIndex;
	}
}

void Gen5_CompleteStreamlossinfo(GEN5_UDPRecord_Header_T  p_gen5_udp_frame_hdr)
{
	int platform = p_gen5_udp_frame_hdr.Platform;
	unsigned int StreamNo = 0xFF;
	unsigned char Source_Id = 0;
	int cyclediff = 0;
	Source_Id = MapSourceToRadarPos(p_gen5_udp_frame_hdr.Radar_Position);
	StreamNo = GEN5MaptoStreamNumber(p_gen5_udp_frame_hdr.stream_Id, p_gen5_udp_frame_hdr.customerId);
	int  i = 0;
	scancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
	strmcurr = StreamNo;
	if (p_gen5_udp_frame_hdr.Platform == 55 || p_gen5_udp_frame_hdr.Platform == 51)
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_SRR_FC] == 1 )
		{

			if (StreamNo == e_BC_CORE_LOGGING_STREAM)
			{
				Strmbcscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmbcscancurrent - Strmbcscanprev > 1 && Strmbcscanprev != -1)
				{
					cyclediff = Strmbcscancurrent - Strmbcscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmbcscancurrent, Get_FLR4PGEN5StreamName(StreamNo));
					}
				}
				Strmbcscanprev = Strmbcscancurrent;
			}
			if (StreamNo == e_VSE_CORE_LOGGING_STREAM)
			{
				Strmvsescancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmvsescancurrent - Strmvsescanprev > 1 && Strmvsescanprev != -1)
				{
					cyclediff = Strmvsescancurrent - Strmvsescanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmvsescancurrent, Get_FLR4PGEN5StreamName(StreamNo));
					}
				}
				Strmvsescanprev = Strmvsescancurrent;
			}
			if (StreamNo == e_STATUS_CORE_LOGGING_STREAM)
			{
				Strmstatusscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmstatusscancurrent - Strmstatusscanprev > 1 && Strmstatusscanprev != -1)
				{
					cyclediff = Strmstatusscancurrent - Strmstatusscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmstatusscancurrent, Get_FLR4PGEN5StreamName(StreamNo), cyclediff);
					}
				}
				Strmstatusscanprev = Strmstatusscancurrent;
			}
			if (StreamNo == e_RDD_CORE_LOGGING_STREAM)
			{

				Strmrddscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmrddscancurrent - Strmrddscanprev > 1 && Strmrddscanprev != -1)
				{
					cyclediff = Strmrddscancurrent - Strmrddscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmrddscancurrent, Get_FLR4PGEN5StreamName(StreamNo), cyclediff);
					}
				}
				Strmrddscanprev = Strmrddscancurrent;
			}
			if (StreamNo == e_DET_LOGGING_STREAM)
			{
				Strmdetscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmdetscancurrent - Strmdetscanprev > 1 && Strmdetscanprev != -1)
				{
					cyclediff = Strmdetscancurrent - Strmdetscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmdetscancurrent, Get_FLR4PGEN5StreamName(StreamNo), cyclediff);
					}
				}
				Strmdetscanprev = Strmdetscancurrent;
			}
			if (StreamNo == e_HDR_LOGGING_STREAM)
			{
				Strmhdrscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (Strmhdrscancurrent - Strmhdrscanprev > 1 && Strmhdrscanprev != -1)
				{
					cyclediff = Strmhdrscancurrent - Strmhdrscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), Strmhdrscancurrent, Get_FLR4PGEN5StreamName(StreamNo), cyclediff);
					}
				}
				Strmhdrscanprev = Strmhdrscancurrent;
			}
			if (StreamNo == e_TRACKER_LOGGING_STREAM)
			{
				StrmTrackerscancurrent = p_gen5_udp_frame_hdr.scan_index;
				if (StrmTrackerscancurrent - StrmTrackerscanprev > 1 && StrmTrackerscanprev != -1)
				{
					cyclediff = StrmTrackerscancurrent - StrmTrackerscanprev - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(MUDP_SRR_FC), StrmTrackerscancurrent, Get_FLR4PGEN5StreamName(StreamNo), cyclediff);
					}
				}
				StrmTrackerscanprev = StrmTrackerscancurrent;
			}

		}
	}
	else
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[Source_Id] == 1 )
		{
			if (StreamNo == C0_CORE_MASTER_STREAM)
			{
				Strmc0_corescancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
				if (Strmc0_corescancurrent[Source_Id] - Strmc0_corescanprev[Source_Id] > 1 && Strmc0_corescanprev[Source_Id] != -1)
				{
					cyclediff = Strmc0_corescancurrent[Source_Id] - Strmc0_corescanprev[Source_Id] - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), Strmc0_corescancurrent[Source_Id], Get_GEN5StreamName(StreamNo));
					}
				}
				Strmc0_corescanprev[Source_Id] = Strmc0_corescancurrent[Source_Id];
			}

			if (StreamNo == C0_CUST_MASTER_STREAM)
			{
				Strmc0_custscancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
				if (Strmc0_custscancurrent[Source_Id] - Strmc0_custscanprev[Source_Id] > 1 && Strmc0_custscanprev[Source_Id] != -1)
				{
					cyclediff = Strmc0_custscancurrent[Source_Id] - Strmc0_custscanprev[Source_Id] - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), Strmc0_custscancurrent[Source_Id], Get_GEN5StreamName(StreamNo));
					}
				}
				Strmc0_custscanprev[Source_Id] = Strmc0_custscancurrent[Source_Id];
			}
			if (StreamNo == C1_CORE_MASTER_STREAM)
			{
				Strmc1_corescancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
				if (Strmc1_corescancurrent[Source_Id] - Strmc1_corescanprev[Source_Id] > 1 && Strmc1_corescanprev[Source_Id] != -1)
				{
					cyclediff = Strmc1_corescancurrent[Source_Id] - Strmc1_corescanprev[Source_Id] - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), Strmc1_corescancurrent[Source_Id], Get_GEN5StreamName(StreamNo));
					}
				}
				Strmc1_corescanprev[Source_Id] = Strmc1_corescancurrent[Source_Id];
			}

			if (StreamNo == C2_CORE_MASTER_STREAM)
			{
				Strmc2_corescancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
				if (Strmc2_corescancurrent[Source_Id] - Strmc2_corescanprev[Source_Id] > 1 && Strmc2_corescanprev[Source_Id] != -1)
				{
					cyclediff = Strmc2_corescancurrent[Source_Id] - Strmc2_corescanprev[Source_Id] - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), Strmc2_corescancurrent[Source_Id], Get_GEN5StreamName(StreamNo));
					}
				}
				Strmc2_corescanprev[Source_Id] = Strmc2_corescancurrent[Source_Id];
			}
			if (StreamNo == C2_CUST_MASTER_STREAM)
			{
				Strmc2_custscancurrent[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
				if (Strmc2_custscancurrent[Source_Id] - Strmc2_custscanprev[Source_Id] > 1 && Strmc2_custscanprev[Source_Id] != -1)
				{
					cyclediff = Strmc2_custscancurrent[Source_Id] - Strmc2_custscanprev[Source_Id] - 1;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]: At SCANINDEX_%d stream  %s Completely Lost  For total cycles [%d], [ERROR_UDP_PARTIALLY_DROPPED] \n", GetRadarPosName(Source_Id), Strmc2_custscancurrent[Source_Id], Get_GEN5StreamName(StreamNo));
					}
				}
				Strmc2_custscanprev[Source_Id] = Strmc2_custscancurrent[Source_Id];
			}
		}
	}
	
}
void gen5_CDC_PacketLossCount(GEN5_Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type) {
	int Radar_Pos = radar_type;
	int TotalnoChunks = 0;
	CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] = p_pack->proc_info.frame_header.scan_index;

	if ((first_scan_idx[radar_type] + 1 != p_pack->proc_info.frame_header.scan_index) && first_scan_idx[radar_type] != 0)
	{
		if (p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR5 || p_pack->proc_info.frame_header.Platform == UDP_PLATFORM_MCIP_SRR5) {
			if (CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] == CDC_scan_idx_previous[Radar_Pos][CDC_DATA] + 1)
			{

				if ((p_pack->proc_info.frame_header.reserved == 1 && p_pack->proc_info.frame_header.customerId == PLATFORM_GEN5 )|| p_pack->proc_info.frame_header.customerId == STLA_FLR4P)
					 TotalnoChunks = p_pack->proc_info.frame_header.Total_num_Chunks + 256;			
				else
					 TotalnoChunks = p_pack->proc_info.frame_header.Total_num_Chunks;
				if (gen5_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks != TotalnoChunks)
				{
					//UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
					//	ScanindexDropCount[radar_type][CDC_DATA]++;
					//	TotalscanIndexDropCount_cdc[radar_type]++;
					loggedmap[logfilename].Status = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[Radar_Pos] = "FAIL";
					TotalscanIndexDropCount_cdc[Radar_Pos]++;
					cdc_partially_drop++;
					loggedmap[logfilename].log_cdc_partially_drop = cdc_partially_drop;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]:Scan Index_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d [ERROR_CDC_PARTIALLY_DROPPED] \n", GetRadarPosName(radar_type), gen5_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.frame_header.scan_index,TotalnoChunks, gen5_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks);
					}
					CDC_Chunk_Drop_ScanID[Radar_Pos] = gen5_pack_working_prev[Radar_Pos][CDC_DATA].proc_info.frame_header.scan_index;
				}

				if ((CDC_Complete_Drop_ScanID[Radar_Pos] - CDC_Chunk_Drop_ScanID[Radar_Pos]) == 1 || (CDC_Complete_Drop_ScanID[Radar_Pos] - CDC_Chunk_Drop_ScanID[Radar_Pos]) == -1)
				{
					loggedmap[logfilename].BMW_Status = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[Radar_Pos] = "FAIL";
					loggedmap[logfilename].DQ_Radar_Fail_Status[Radar_Pos] = "FAIL";
					std::string posName = GetRadarPosName(radar_type);
					loggedmap[logfilename].Observation = "CDC completely dropped in Sensor[" + posName + "] : at ScanIndex - " + to_string(CDC_Complete_Drop_ScanID[Radar_Pos]) + "\n";
					//loggedmap[logfilename].Observation = "CDC completely dropped at some ScanIndexes. \t"; //in Sensor[" + posName + "] : ScanIndex - " + to_string(CDC_Complete_Drop_ScanID[Radar_Pos]);
					CDC_Complete_Drop_ScanID[Radar_Pos] = 0;
					CDC_Chunk_Drop_ScanID[Radar_Pos] = 0;
				}
			}
		}
		if ((p_pack->proc_info.crc_status == CRC_INVALID) && (p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR5 || p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR6 || p_pack->proc_info.frame_header.Platform == UDP_PLATFORM_MCIP_SRR5)) {     //CRC Check Added for  CYW-2815

			CRCErrorCount[radar_type][CDC_DATA]++;
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), GetStreamName(CDC_DATA), p_pack->proc_info.frame_header.scan_index);
			}

		}
		else if (p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_pack->proc_info.frame_header.Platform <= UDP_PLATFORM_CUST_SRR3_FC)
		{
			if (p_pack->proc_info.f_failure_block == 1 && p_pack->proc_info.udpPack_status == e_UDP_PACKETS_MISSED)
			{
				//	UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s]:Scan Index_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d\n", GetRadarPosName(radar_type), p_pack->proc_info.frame_header.scan_index, p_pack->proc_info.frame_header.streamChunkIdx, p_pack->proc_info.nProcessed_chks);
				}
			}
		}
		else {
			if (p_pack->proc_info.f_failure_block == 1) {
				//	UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
			}
		}
	}
	CDC_scan_idx_previous[Radar_Pos][CDC_DATA] = CDC_ScanIdCurrent[Radar_Pos][CDC_DATA];
	gen5_pack_working_prev[Radar_Pos][CDC_DATA].proc_info = p_pack->proc_info;

}

void CDC_PacketLossCount(Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type) {
	int Radar_Pos = radar_type;
	CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] = p_pack->proc_info.frame_header.streamRefIndex;


	if ((first_scan_idx[radar_type] + 1 != p_pack->proc_info.frame_header.streamRefIndex) && first_scan_idx[radar_type] != 0)
	{
		if ((p_pack->proc_info.frame_header.streamVersion >= 4) &&( p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR5 || p_pack->proc_info.frame_header.Platform == UDP_PLATFORM_MCIP_SRR5)) {
			if (CDC_ScanIdCurrent[Radar_Pos][CDC_DATA] == CDC_scan_idx_previous[Radar_Pos][CDC_DATA] + 1)
			{
				if (pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks != p_pack->proc_info.frame_header.streamChunks)
				{
					//UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s]:Scan Index_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d\n", GetRadarPosName(radar_type), p_pack->proc_info.frame_header.streamRefIndex, p_pack->proc_info.frame_header.streamChunks, pack_working_prev[Radar_Pos][CDC_DATA].proc_info.nProcessed_chks);
					}
				}
			}
		}
		if ((p_pack->proc_info.crc_status == CRC_INVALID) && (p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR5 || p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_SRR6 || p_pack->proc_info.frame_header.Platform == UDP_PLATFORM_MCIP_SRR5)) {     //CRC Check Added for  CYW-2815

			CRCErrorCount[radar_type][CDC_DATA]++;
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), GetStreamName(CDC_DATA), p_pack->proc_info.frame_header.streamRefIndex);
				
			}
			
		}
		else if (p_pack->proc_info.frame_header.Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_pack->proc_info.frame_header.Platform <= UDP_PLATFORM_CUST_SRR3_FC)
		{
			if (p_pack->proc_info.f_failure_block == 1 && p_pack->proc_info.udpPack_status == e_UDP_PACKETS_MISSED)
			{
				//	UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s]:Scan Index_%u, Total CDC chunks expected_%d, Total CDC chunks received_%d\n", GetRadarPosName(radar_type), p_pack->proc_info.frame_header.streamRefIndex, p_pack->proc_info.frame_header.streamChunkIdx, p_pack->proc_info.nProcessed_chks);
				}
			}
		}
		
		else {
			if (p_pack->proc_info.f_failure_block == 1) {
				//	UDPPacketLossErrorCount[radar_type][CDC_DATA]++;
				//	ScanindexDropCount[radar_type][CDC_DATA]++;
				//	TotalscanIndexDropCount_cdc[radar_type]++;
			}
		}
	}
	CDC_scan_idx_previous[Radar_Pos][CDC_DATA] = CDC_ScanIdCurrent[Radar_Pos][CDC_DATA];
	pack_working_prev[Radar_Pos][CDC_DATA].proc_info = p_pack->proc_info;

}

void gen7_Chunkslossinfo(GEN7_UDPRecord_Header_T  p_gen7_udp_frame_hdr)
{
	int platform = p_gen7_udp_frame_hdr.sourceInfo;
	unsigned int MapStreamNo = 0xFF;
	unsigned char Source_Id = 0;

	Source_Id = MapSourceToRadarPos(p_gen7_udp_frame_hdr.sensorId);
	MapStreamNo = GEN7MaptoStreamNumber(p_gen7_udp_frame_hdr.streamNumber, PLATFORM_GEN7);

	for (int i = 0; i < MAX_RADAR_COUNT; i++)
	{
		if ((setchunkflag[i] == 0) && (Source_Id == i))
		{
			if (MapStreamNo == CDC_DATA && p_gen7_udp_frame_hdr.streamChunkIdx == 0)
			{
				setchunkflag[i] = 1;
				break;
			}
			else if (MapStreamNo != CDC_DATA && p_gen7_udp_frame_hdr.streamChunkIdx == 0)
			{
				setchunkflag[i] = 1;
				break;
			}
			else
			{
				return;
			}
		}
	}

	std::string posName = GetRadarPosName(Source_Id);
	if (setchunkflag[Source_Id] == true)
	{
		if (MapStreamNo == CDC_STREAM && p_gen7_udp_frame_hdr.streamChunkIdx == 0)
		{
			nCurrentScan_Index[Source_Id] = p_gen7_udp_frame_hdr.streamRefIndex;
		}
		nCurrscan[Source_Id] = p_gen7_udp_frame_hdr.streamRefIndex;

		npreviousChunkid[Source_Id] = nCurrentChunkid[Source_Id];

		nCurrentChunkid[Source_Id] = p_gen7_udp_frame_hdr.streamChunkIdx;

		std::string posName = GetRadarPosName(Source_Id);
		if ((nCurrentChunkid[Source_Id] - npreviousChunkid[Source_Id] > 1) && npreviousChunkid[Source_Id] != -1)
		{
			/*if (MUDP_master_config->MUDP_input_path == SEQUENTIAL_Mode)
			{
				TotalscanIndexDropCount_udp[Source_Id]++;
				ScanindexDropCount[Source_Id][MapStreamNo]++;
			}*/
			if ((missedchunkscan[Source_Id] != nCurrentScan_Index[Source_Id]) && (nCurrentScan_Index[Source_Id] - missedchunkscan[Source_Id] == 2) && missedchunkscan[Source_Id] != -1)
			{
				loggedmap[logfilename].BMW_Status = "FAIL";
				loggedmap[logfilename].RESIM_Radar_Fail_Status[Source_Id] = "FAIL";
				loggedmap[logfilename].DQ_Radar_Fail_Status[Source_Id] = "FAIL";
				loggedmap[logfilename].Observation += "UDP Packetloss found in sensor[" + posName + "] At scanindex - " + to_string(nCurrentScan_Index[Source_Id]) + " -> Timestamp :" + to_string(p_gen7_udp_frame_hdr.sourceTxTime) + " .";
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t UDP Packetloss found in sensor[%s] At scanindex - %d\n", GetRadarPosName(Source_Id), p_gen7_udp_frame_hdr.streamRefIndex);
				}
			}
			missedchunkscan[Source_Id] = nCurrscan[Source_Id];
		}
	}
}

void gen5_Chunkslossinfo(GEN5_UDPRecord_Header_T  p_gen5_udp_frame_hdr)
{
	int platform = p_gen5_udp_frame_hdr.Platform;
	unsigned int MapStreamNo = 0xFF;
	unsigned char Source_Id = 0;

	Source_Id = MapSourceToRadarPos(p_gen5_udp_frame_hdr.Radar_Position);
	MapStreamNo = GEN5MaptoStreamNumber(p_gen5_udp_frame_hdr.stream_Id, p_gen5_udp_frame_hdr.customerId);

	for (int i = 0; i < MAX_RADAR_COUNT; i++)
	{
		if ((setchunkflag[i] == 0) && (Source_Id == i))
		{
			if (MapStreamNo == CDC_DATA && p_gen5_udp_frame_hdr.streamChunkIdx == 0)
			{
				setchunkflag[i] = 1;
				break;
			}
			else if (MapStreamNo != CDC_DATA && p_gen5_udp_frame_hdr.streamChunkIdx == 0)
			{
				setchunkflag[i] = 1;
				break;
			}
			else
			{
				return;
			}
		}
	}

	std::string posName = GetRadarPosName(Source_Id);
	if (setchunkflag[Source_Id] == true)
	{
		if (MapStreamNo == CDC_STREAM && p_gen5_udp_frame_hdr.streamChunkIdx == 0)
		{
			nCurrentScan_Index[Source_Id] = p_gen5_udp_frame_hdr.scan_index;
		}
		nCurrscan[Source_Id] = p_gen5_udp_frame_hdr.scan_index;

		npreviousChunkid[Source_Id] = nCurrentChunkid[Source_Id];

		nCurrentChunkid[Source_Id] = p_gen5_udp_frame_hdr.streamChunkIdx;

		std::string posName = GetRadarPosName(Source_Id);
		if ((nCurrentChunkid[Source_Id] - npreviousChunkid[Source_Id] > 1) && npreviousChunkid[Source_Id] != -1)
		{
			/*if (MUDP_master_config->MUDP_input_path == SEQUENTIAL_Mode)
			{
				TotalscanIndexDropCount_udp[Source_Id]++;
				ScanindexDropCount[Source_Id][MapStreamNo]++;
			}*/
			if ((missedchunkscan[Source_Id] != nCurrentScan_Index[Source_Id]) && (nCurrentScan_Index[Source_Id] - missedchunkscan[Source_Id] == 3) && missedchunkscan[Source_Id] != -1)
			{
				loggedmap[logfilename].BMW_Status = "FAIL";
				loggedmap[logfilename].RESIM_Radar_Fail_Status[Source_Id] = "FAIL";
				loggedmap[logfilename].DQ_Radar_Fail_Status[Source_Id] = "FAIL";
				loggedmap[logfilename].Observation += "UDP Packetloss found in sensor[" + posName + "] At scanindex - " + to_string(nCurrentScan_Index[Source_Id]) + " -> Timestamp :" + to_string( p_gen5_udp_frame_hdr.timestamp)+" .";
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t UDP Packetloss found in sensor[%s] At scanindex - %d\n", GetRadarPosName(Source_Id), p_gen5_udp_frame_hdr.scan_index);
				}
			}
			missedchunkscan[Source_Id] = nCurrscan[Source_Id];
		}
	}
}

void Chunkslossinfo(UDPRecord_Header  udp_frame_hdr, udp_custom_frame_header_t  cust_frame_hdr)
{
	int platform = udp_frame_hdr.Platform;
	unsigned int MapStreamNo = 0xFF;
	unsigned int Source_Id = 0;
	if (platform == UDP_PLATFORM_SRR5 || platform == UDP_PLATFORM_MCIP_SRR5)
	{
		Source_Id = MapSourceToRadarPos(udp_frame_hdr.Radar_Position);
	}
	else if (platform == UDP_SOURCE_CUST_DSPACE)
	{
		Source_Id = RDR_POS_DSPACE;
	}
	else
	{
		if (udp_frame_hdr.customerID == SCANIA_MAN)
			Source_Id = ManSourceid;
		else
			Source_Id = udp_frame_hdr.Platform - UDP_PLATFORM_CUST_SRR3_RL;
	}

	if (Source_Id == RADAR_POS) {
		MapStreamNo = MaptoEcustreamNumber(udp_frame_hdr.streamNumber);
	}
	else {
		MapStreamNo = MaptostreamNumber(udp_frame_hdr.streamNumber, udp_frame_hdr.customerID);
	}

	for (int i = 0; i < MAX_RADAR_COUNT; i++)
	{
		if ((setchunkflag[i] == 0) && (Source_Id == i))
		{
			if (MapStreamNo == CDC_DATA && udp_frame_hdr.streamChunkIdx == 1)
			{
				setchunkflag[i] = 1;
				break;
			}
			else if (MapStreamNo != CDC_DATA && udp_frame_hdr.streamChunkIdx == 0)
			{
				setchunkflag[i] = 1;
				break;
			}
			else
			{
				return;
			}
		}
	}

	std::string posName = GetRadarPosName(Source_Id);
	if (setchunkflag[Source_Id] == true)
	{
		if (MapStreamNo == CDC_DATA && udp_frame_hdr.streamChunkIdx == 1)
		{
			nCurrentScan_Index[Source_Id] = udp_frame_hdr.streamRefIndex;
		}
		else if(MapStreamNo != CDC_DATA && udp_frame_hdr.streamChunkIdx == 0)
		{
			nCurrscan[Source_Id] = udp_frame_hdr.streamRefIndex;
		}

		npreviousChunkid[Source_Id] = nCurrentChunkid[Source_Id];

		nCurrentChunkid[Source_Id] = udp_frame_hdr.streamChunkIdx;

		std::string posName = GetRadarPosName(Source_Id);
		if ((nCurrentChunkid[Source_Id] - npreviousChunkid[Source_Id] > 1) && npreviousChunkid[Source_Id] != -1)
		{
			/*if (MUDP_master_config->MUDP_input_path == SEQUENTIAL_Mode)
			{
				TotalscanIndexDropCount_udp[Source_Id]++;
				ScanindexDropCount[Source_Id][MapStreamNo]++;
			}*/
			if ((missedchunkscan[Source_Id] != nCurrentScan_Index[Source_Id]) && (nCurrentScan_Index[Source_Id] - missedchunkscan[Source_Id] == 3) && missedchunkscan[Source_Id] != -1)
			{
				loggedmap[logfilename].BMW_Status = "FAIL";
				loggedmap[logfilename].RESIM_Radar_Fail_Status[Source_Id] = "FAIL";
				loggedmap[logfilename].DQ_Radar_Fail_Status[Source_Id] = "FAIL";
				if (MapStreamNo == CDC_DATA)
				{
					loggedmap[logfilename].Observation += "CDC Packetloss found in sensor[" + posName + "] At scanindex - " + to_string(nCurrentScan_Index[Source_Id]) + " -> Timestamp :" + to_string(cust_frame_hdr.timestamp)+" .";
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t CDC Packetloss found in sensor[%s] At scanindex - %d \n", GetRadarPosName(Source_Id), udp_frame_hdr.streamRefIndex);
					}
					}
				else
				{
					loggedmap[logfilename].Observation += "UDP Packetloss found in sensor[" + posName + "] At scanindex - " + to_string(nCurrentScan_Index[Source_Id] - 1) + " -> Timestamp :" + to_string(cust_frame_hdr.timestamp)+ " .";
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t CDC Packetloss found in sensor[%s] At scanindex - %d \n", GetRadarPosName(Source_Id), udp_frame_hdr.streamRefIndex);
					}
				}

			}
			if (MapStreamNo == CDC_DATA)
			{
				missedchunkscan[Source_Id] = nCurrentScan_Index[Source_Id] ;
			}
			else {
				missedchunkscan[Source_Id] = nCurrscan[Source_Id];
			}

		}
	}
}

void gen7_Get_PacketLossCount(GEN7_Radar_UDP_Frame_Latch_T* p_gen7_pack, unsigned16_T radar_type, unsigned8_T stream_no) 
{
	unsigned int MapStreamNo = 0xFF;
	bool fSCANIdCnt = 0;
	unsigned char Source_Id = 0;
	Source_Id = p_gen7_pack->proc_info.frame_header.sensorId;
	static int scanindex_prev[MAX_RADAR_COUNT] = { 0 };
	static int scanindex_current[MAX_RADAR_COUNT] = { 0 };
	MapStreamNo = GEN7MaptoStreamNumber(p_gen7_pack->proc_info.frame_header.streamNumber, PLATFORM_GEN7);
	if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_type] == 1) {
		if (MapStreamNo == stream_no) {
			if (stream_no != CDC_STREAM)
			{
				if ((first_scan_idx[radar_type] != p_gen7_pack->proc_info.frame_header.streamRefIndex) && first_scan_idx[radar_type] != 0)
				{
					if (p_gen7_pack->proc_info.xsum_status == XSUM_INVALID) {		//CheckSum Checks
						ChecksumErrorCount[radar_type][stream_no]++;
						loggedmap[logfilename].ChecksumErrorCount[radar_type]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							if (p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_FLR7
								|| p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS_HD || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_E)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t [%s] ChecksumError observed for stream :[%s]: Scan Index : %u  \n", GetRadarPosName(radar_type), Get_GEN7StreamName(stream_no), p_gen7_pack->proc_info.frame_header.streamRefIndex);

							}
						}
						fSCANIdCnt = 1;
					}

					if (p_gen7_pack->proc_info.nProcessed_chks != p_gen7_pack->proc_info.frame_header.streamChunks) {	//Latched Chunks Checks
						int DiffChunks = (unsigned int)(p_gen7_pack->proc_info.frame_header.streamChunks - p_gen7_pack->proc_info.nProcessed_chks);
						DiffChunks = abs(DiffChunks);
						if (DiffChunks != 0) {

							UDPPacketLossErrorCount[radar_type][stream_no] += DiffChunks;
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								if (p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_FLR7
									|| p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS_HD || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_E)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t [%s] chunks lost count:%d failure for stream :[%s]: Scan Index : %u [ERROR_UDP_PARTIALLY_MISSED] \n", GetRadarPosName(radar_type), DiffChunks, Get_GEN7StreamName(stream_no), p_gen7_pack->proc_info.frame_header.streamRefIndex);
									
									if (stream_no != GEN5_CALIB_STREAM &&
										((Stream_Index_Prev[radar_type] > p_gen7_pack->proc_info.frame_header.streamRefIndex) && (Stream_Index_Prev[radar_type] - p_gen7_pack->proc_info.frame_header.streamRefIndex >= 2)))
									{
										loggedmap[logfilename].BMW_Status = "FAIL";
										std::string posName = GetRadarPosName(radar_type);
										loggedmap[logfilename].Observation += "[" + posName + "]UDP Continoues Drops Observed at Scanindex: " + to_string(p_gen7_pack->proc_info.frame_header.streamRefIndex) + ". \n";
										loggedmap[logfilename].RESIM_Radar_Fail_Status[radar_type] = "FAIL";
										loggedmap[logfilename].DQ_Radar_Fail_Status[radar_type] = "FAIL";
									}
								}
							}
							fSCANIdCnt = 1;
						}
					}
					if (p_gen7_pack->proc_info.crc_status == CRC_INVALID) {

						CRCErrorCount[radar_type][stream_no]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							if (p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_FLR7
								|| p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_PLUS_HD || p_gen7_pack->proc_info.frame_header.sourceInfo == UDP_PLATFORM_SRR7_E)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), Get_GEN7StreamName(stream_no), p_gen7_pack->proc_info.frame_header.streamRefIndex);
							}
						}
						fSCANIdCnt = 1;
					}
					if (fSCANIdCnt == 1) {
						TotalscanIndexDropCount_udp[radar_type]++;
						ScanindexDropCount[radar_type][stream_no]++;
					}
				}
			}
		}
	}
	Stream_Index_Prev[radar_type] = p_gen7_pack->proc_info.frame_header.streamRefIndex;
}

void gen5_Get_PacketLossCount(GEN5_Radar_UDP_Frame_Latch_T* p_gen5_pack, unsigned16_T radar_type, unsigned8_T stream_no) {
	unsigned int MapStreamNo = 0xFF;
	bool fSCANIdCnt = 0;
	unsigned char Source_Id = 0;
	Source_Id = p_gen5_pack->proc_info.frame_header.Radar_Position;
	static int scanindex_prev[MAX_RADAR_COUNT] = { 0 };
	static int scanindex_current[MAX_RADAR_COUNT] = { 0 };
	MapStreamNo = GEN5MaptoStreamNumber(p_gen5_pack->proc_info.frame_header.stream_Id, p_gen5_pack->proc_info.frame_header.customerId);
	if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_type] == 1) {
		if (MapStreamNo == stream_no) {
			if (stream_no != CDC_STREAM)
			{
				if ((first_scan_idx[radar_type] != p_gen5_pack->proc_info.frame_header.scan_index) && first_scan_idx[radar_type] != 0)
				{
					if (p_gen5_pack->proc_info.xsum_status == XSUM_INVALID) {
						ChecksumErrorCount[radar_type][stream_no]++;
						loggedmap[logfilename].ChecksumErrorCount[radar_type]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							if (p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS && p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t [%s] ChecksumError observed for stream :[%s]: Scan Index : %u  \n", GetRadarPosName(radar_type), Get_GEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
							}
							else if (p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS || p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t [%s] ChecksumError observed for stream :[%s]: Scan Index : %u  \n", GetRadarPosName(radar_type), Get_FLR4PGEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
							}
						}
						fSCANIdCnt = 1;
					}
					if (p_gen5_pack->proc_info.nProcessed_chks != p_gen5_pack->proc_info.frame_header.Total_num_Chunks) {
						int DiffChunks = (unsigned int)(p_gen5_pack->proc_info.frame_header.Total_num_Chunks - p_gen5_pack->proc_info.nProcessed_chks);
						DiffChunks = abs(DiffChunks);
						if (DiffChunks != 0) {

							UDPPacketLossErrorCount[radar_type][stream_no] += DiffChunks;
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								if (p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS && p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t [%s] chunks lost count:%d failure for stream :[%s]: Scan Index : %u [ERROR_UDP_PARTIALLY_MISSED] \n", GetRadarPosName(radar_type), DiffChunks, Get_GEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
									if (stream_no != GEN5_CALIB_STREAM && 
										((Stream_Index_Prev[radar_type] > p_gen5_pack->proc_info.frame_header.scan_index) && (Stream_Index_Prev[radar_type] - p_gen5_pack->proc_info.frame_header.scan_index >= 3)))
									{
										loggedmap[logfilename].BMW_Status = "FAIL";
										std::string posName = GetRadarPosName(radar_type);
										loggedmap[logfilename].Observation += "[" + posName + "]UDP Continoues Drops Observed at Scanindex: " + to_string(p_gen5_pack->proc_info.frame_header.scan_index) + ". \n";
										loggedmap[logfilename].RESIM_Radar_Fail_Status[radar_type] = "FAIL";
										loggedmap[logfilename].DQ_Radar_Fail_Status[radar_type] = "FAIL";
									}									
								}
								else if (p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS || p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
								{
									fprintf(fCreate, "\t\t=========================================================================================================\n");
									fprintf(fCreate, "\t\t [%s] chunks lost count:%d failure for stream :[%s]: Scan Index : %u [ERROR_UDP_PARTIALLY_MISSED] \n", GetRadarPosName(radar_type), DiffChunks, Get_FLR4PGEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
								}

								if (p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_STLA_FLR4)
								{
									if (Stream_Index_Prev[radar_type] > p_gen5_pack->proc_info.frame_header.scan_index)
									{
										fprintf(fCreate, "\t\t=========================================================================================================\n");
										fprintf(fCreate, "\t\t [%s] UDP Logging Order for stream :[%s]: changed at Scan Index : %u [ERROR_UDP_PARTIALLY_MISSED] \n", GetRadarPosName(radar_type), Get_GEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
									}
								}
							}
							fSCANIdCnt = 1;
						}
					}
					if (p_gen5_pack->proc_info.crc_status == CRC_INVALID) {

						CRCErrorCount[radar_type][stream_no]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							if (p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS && p_gen5_pack->proc_info.frame_header.Platform != UDP_PLATFORM_FLR4_PLUS_STANDALONE)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), Get_GEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
							}
							else if (p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS || p_gen5_pack->proc_info.frame_header.Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), Get_FLR4PGEN5StreamName(stream_no), p_gen5_pack->proc_info.frame_header.scan_index);
							}
						}
						fSCANIdCnt = 1;
					}
					if (fSCANIdCnt == 1) {
						TotalscanIndexDropCount_udp[radar_type]++;
						ScanindexDropCount[radar_type][stream_no]++;
					}
				}
			}
		}
	}
	Stream_Index_Prev[radar_type] = p_gen5_pack->proc_info.frame_header.scan_index;
}

void Get_PacketLossCount(Radar_UDP_Frame_Latch_T* p_pack, unsigned16_T radar_type, unsigned8_T stream_no) {
	unsigned int MapStreamNo = 0xFF;
	bool fSCANIdCnt = 0;
	unsigned char Source_Id = 0;
	Source_Id = p_pack->proc_info.frame_header.Radar_Position;
	if (Source_Id == RADAR_POS) {
		MapStreamNo = MaptoEcustreamNumber(p_pack->proc_info.frame_header.streamNumber);
	}
	else {
		MapStreamNo = MaptostreamNumber(p_pack->proc_info.frame_header.streamNumber, p_pack->proc_info.frame_header.customerID);
	}
	if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_type] == 1) {
		if (MapStreamNo == stream_no) {
			if ((first_scan_idx[radar_type] != p_pack->proc_info.frame_header.streamRefIndex) && first_scan_idx[radar_type] != 0)
			{
				if (stream_no != CDC_DATA) {
					if (p_pack->proc_info.xsum_status == XSUM_INVALID) {
						ChecksumErrorCount[radar_type][stream_no]++;
						scanindex_log_quality_current[radar_type] = p_pack->proc_info.frame_header.streamRefIndex;
						if ((scanindex_log_quality_current[radar_type] - scanindex_log_quality_prev[radar_type] == 1))
						{
							loggedmap[logfilename].BMW_Status = "FAIL";
							loggedmap[logfilename].RESIM_Radar_Fail_Status[radar_type] = "FAIL";
							loggedmap[logfilename].DQ_Radar_Fail_Status[radar_type] = "FAIL";
						}
						scanindex_log_quality_prev[radar_type] = p_pack->proc_info.frame_header.streamRefIndex;
						//sensor_log_quality = radar_type;
						loggedmap[logfilename].ChecksumErrorCount[radar_type]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t [%s] ChecksumError observed for stream :[%s]: Scan Index : %u  \n", GetRadarPosName(radar_type), GetStreamName(stream_no), p_pack->proc_info.frame_header.streamRefIndex);
						}
						fSCANIdCnt = 1;
					}
					if (p_pack->proc_info.nProcessed_chks != p_pack->proc_info.frame_header.streamChunks) {
						int DiffChunks = (unsigned int)(p_pack->proc_info.frame_header.streamChunks - p_pack->proc_info.nProcessed_chks);
						DiffChunks = abs(DiffChunks);
						if (DiffChunks != 0) {

							UDPPacketLossErrorCount[radar_type][stream_no] += DiffChunks;
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								fprintf(fCreate, "\t\t=========================================================================================================\n");
								fprintf(fCreate, "\t\t [%s] chunks lost count:%d failure for stream :[%s]: Scan Index : %u [ERROR_UDP_PARTIALLY_MISSED] \n", GetRadarPosName(radar_type), DiffChunks, GetStreamName(stream_no), p_pack->proc_info.frame_header.streamRefIndex);
							}
							fSCANIdCnt = 1;
						}
					}
					if (p_pack->proc_info.crc_status == CRC_INVALID) {

						CRCErrorCount[radar_type][stream_no]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, " \t\t[%s] CRC mismatch observed for stream :[%s]: ScanIndex : %u  \n", GetRadarPosName(radar_type), GetStreamName(stream_no), p_pack->proc_info.frame_header.streamRefIndex);
						}
						fSCANIdCnt = 1;
					}
					if (fSCANIdCnt == 1) {
						TotalscanIndexDropCount_udp[radar_type]++;
						ScanindexDropCount[radar_type][stream_no]++;
					}
				}
			}
		}
	}
	Stream_Index_Prev[radar_type] = p_pack->proc_info.frame_header.streamRefIndex;
}

void ResetScanIndexpacketCounts() {
	for (int pos = 0; pos < MAX_RADAR_COUNT; pos++) {
		for (int i = 0; i < MAX_LOGGING_SOURCE; i++) {
			UDPPacketLossErrorCount[pos][i] = 0;
			ChecksumErrorCount[pos][i] = 0;
			ScanindexDropCount[pos][i] = 0;
			printed_stream_details[i] = { 0 };
			Strmc0_corescancurrent[i] = { -1 };
			Strmc0_custscancurrent[i] = { -1 };
			Strmc1_corescancurrent[i] = { -1 };
			Strmc2_corescancurrent[i] = { -1 };
			Strmc2_custscancurrent[i] = { -1 };

			Strmc0_corescanprev[i] = { -1 };
			Strmc0_custscanprev[i] = { -1 };
			Strmc1_corescanprev[i] = { -1 };
			Strmc2_corescanprev[i] = { -1 };
			Strmc2_custscanprev[i] = { -1 };
		}

		TotalscanIndexDropCount_udp[pos] = 0;
		TotalscanIndexDropCount_cdc[pos] = 0;
		TotalscanIndex[pos] = 0;
		ScanIdDropCount_udp[pos] = 0;
		Async_KPI_count[pos] = 0;
		ScanIdDropCount_cdc[pos] = 0;
		SrcTxnDropCount[pos] = 0;
		scan_idx_Z_previous[pos] = 0;
		refidx_Z_previous[pos] = 0;
		scan_idx_M_previous[pos] = 0;
		refidx_M_previous[pos] = 0;
		//g_ScanID_Z_Prev[pos] = 0;
		scan_idx_previous[pos] = 0;
		scan_idx_current[pos] = 0;
		gen7_scan_idx_previous[pos] = -1;
		gen7_scan_idx_current[pos] = 0;
		gen7_cdc_scan_idx_previous[pos] = -1;
		refidx_current[pos] = 0;
		first_scan_idx[pos] = 0;
		first_refidx[pos] = 0;
		ScanIdCount[pos] = 0;
		f_Firstscan[pos] = 0;
		f_First_refid[pos] = 0;
		ScanIdDropCount_CalibStream[0] = 0;
		cdc_partially_drop = 0;
		UDP_Packetloss_Tag_Opened = FALSE;
		//radarEnabled.clear();
		//scanIndexDropCount[pos] =0;
	}
	UDPflag = false;
	Cflg = false;
	if (MUDP_master_config->MUDP_input_path == SEQUENTIAL_Mode) {
		m_pDecoderPlugin->SRR3_API_Init();
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
	F_dspace = true;
	printStrmHeaderOnes = true;
	printOnesline = true;
	onesTxCnt = true;
	srcTxCnt = true;
	F_CDC = true;
	F_CALIB = true;
	F_C0 = true;
	F_C1 = true;
	F_C2 = true;
	F_C3 = true;
	F_C4 = true;
	F_STLA_CDC = true;
	F_STLA_CALIB = true;
	F_STLA_C0 = true;
	F_STLA_C1 = true;
	F_STLA_C2 = true;
	F_STLA_C3 = true;
	F_STLA_C4 = true;
	F_STLA_C5 = true;
	F_BC = true;
	F_VSE = true;
	F_STATUS = true;
	F_RDD = true;
	F_DET = true;
	F_HDR = true;
	F_TRACKER = true;
	F_DEBUG = true;
	F_CALIB_flr4p = true;	
}

void TotalScanIndexerrorFile(FILE* Sfp) {
	int radarCnt = 0;
	float Total_percent_failure_udp = { 0.0 };
	float Total_percent_failure_cdc = { 0.0 };
	unsigned int TotalScanIDCnt = 0;
	unsigned int TotalscanIndexDrop_udp = 0;
	unsigned int TotalscanIndexDrop_cdc = 0;
	unsigned int Total_ScanIdDropCount_CalibStream = 0;
	int MAXstreamNo = 0;

	bool printFLAG = 0;
	if (MUDP_master_config->packet_loss_statistics == 1)
	{
		if (MUDP_master_config->Bpillar_status == 1 && MUDP_master_config->MUDP_Radar_ECU_status == 0) {
			radarCnt = MUDP_MAX_STATUS_COUNT;
		}
		else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_FLR4P_FC_09] == 1)
		{
			radarCnt = MUDP_MAX_STATUS_COUNT;
		}
		else if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
			radarCnt = MAX_RADAR_COUNT;
			MAXstreamNo = MAX_RADAR_COUNT;
		}
		else {
			radarCnt = MAX_SRR3_SOURCE;
		}
	}
	else
	{
		radarCnt = MAX_RADAR_COUNT;
	}
	for (int pos = 0; pos < radarCnt; pos++) {
		if (MUDP_master_config->packet_loss_statistics == 1)
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[pos] != 1) {
				if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
					if (pos != RDR_POS_ECU)
						continue;
				}
				else {
					continue;
				}
			}
		}
		if (ScanIdCount[pos] != 0) {
			TotalScanIDCnt = ScanIdCount[pos] + TotalScanIDCnt;
			//TotalscanIndexDrop = TotalscanIndexDropCount[pos] + TotalscanIndexDrop + ScanIdDropCount_cdc[pos] + ScanIdDropCount_udp[pos];
			for (int i = 0; i < MAXstreamNo; i++)
			{
				int offset = UDPPacketLossErrorCount[pos][i];
				TotalscanIndexDropCount_udp[pos] += offset;
			}
			//printf("Updating TotalScanIndexDrop_udp = [%d] - A\n", TotalscanIndexDrop_udp);
			//printf("TotalscanIndexDropCount_udp[pos] = [%d]\n", TotalscanIndexDropCount_udp[pos]);
			TotalscanIndexDrop_udp = TotalscanIndexDropCount_udp[pos] + TotalscanIndexDrop_udp;
			//printf("Updating TotalScanIndexDrop_udp = [%d] - B\n", TotalscanIndexDrop_udp);
			TotalscanIndexDrop_cdc = TotalscanIndexDropCount_cdc[pos] + TotalscanIndexDrop_cdc;
			Total_ScanIdDropCount_CalibStream = ScanIdDropCount_CalibStream[pos] + Total_ScanIdDropCount_CalibStream;			
			printFLAG = true;
		}
	}

	loggedmap[logfilename].total_scanindex = TotalScanIDCnt;
	
	int min = 0, max = 0;
	for (int pos = 0; pos < radarCnt; pos++)
	{
		if (ScanIdCount[pos] != 0)
		{
			loggedmap[logfilename].Total_Scan_Index[pos] = ScanIdCount[pos];
			if (max == 0)
			{
				maximum = ScanIdCount[pos];
				max = 1;
			}
			check_scan_coun_flag++;
			if (maximum < ScanIdCount[pos])
			{
				maximum = ScanIdCount[pos];
			}
		}

	}
	//minimum = ScanIdCount[0];

	for (int pos = 0; pos < radarCnt; pos++)
	{
		if (ScanIdCount[pos] != 0)
		{
			if (min == 0)
			{
				minimum = ScanIdCount[pos];
				min = 1;
			}
			if (minimum > ScanIdCount[pos])
			{
				minimum = ScanIdCount[pos];
			}
		}

	}
	if (check_scan_coun_flag != 1)
	{
		diff = maximum - minimum;
	}

	if (diff != 0 && diff > 3)
	{
		loggedmap[logfilename].BMW_Status = "FAIL";
		//loggedmap[logfilename].Observation += "Sensor Radar cycles diff is more than three cycles. ";

		for (int pos = 0; pos < radarCnt; pos++)
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[pos] == 1) {
				if ((maximum - ScanIdCount[pos]) > 3)
				{
					std::string posName = GetRadarPosName(pos);
					loggedmap[logfilename].Observation += "Sensor Radar cycles diff is more than three cycles at Radar Pos [" + posName + "]. ";
					loggedmap[logfilename].DQ_Radar_Fail_Status[pos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[pos] = "FAIL";
				}
			}
		}
	}

	if (MUDP_master_config->Packetloss_report_format == XML_format)
	{
		if (TotalscanIndexDrop_udp != 0 || TotalscanIndexDrop_cdc != 0 || Total_ScanIdDropCount_CalibStream != 0 || (diff != 0 && diff > 3))//|| Total_percent_failure_udp != 0 && Total_percent_failure_cdc != 0 || loggedmap[logfilename].BMW_Status == "FAIL"
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				if (UDP_Packetloss_Tag_Opened == FALSE) {
					fprintf(Sfp, "\t\t<UDP_Packet_Loss_Report>\n");
					UDP_Packetloss_Tag_Opened = TRUE;
				}
			}
		}
	}
	if (printFLAG) {

		if ((TotalscanIndexDrop_cdc != 0) || (TotalscanIndexDrop_udp != 0) || (Total_ScanIdDropCount_CalibStream != 0))
		{
			if (MUDP_master_config->Packetloss_report_format == Text_format)
			{
				fprintf(Sfp, "*******************************************************************************************\n");
				fprintf(Sfp, "Total Scan Index of the log = %u\n", TotalScanIDCnt);
			}
			else
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					fprintf(Sfp, "\t\t\t<Total_Scan_Index_In_The_File>%u</Total_Scan_Index_In_The_File>\n", TotalScanIDCnt);
				}

				 // loggedmap[logfilename].total_scanindex = TotalScanIDCnt;
			}
		}
		//fprintf(Sfp, "Total Scans of Resim failure of the log = %u\n", TotalscanIndexDrop);
		if (TotalscanIndexDrop_cdc != 0)
		{
			if (MUDP_master_config->Packetloss_report_format == Text_format)
			{
				fprintf(Sfp, "CDC Cycles Completely missed= %u\n", TotalscanIndexDrop_cdc);
				loggedmap[logfilename].cdc_scanindex_drop = TotalscanIndexDrop_cdc;
			}
			else
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					fprintf(Sfp, "\t\t\t<Total_CDC_Cycles_Completely_Missed>%u</Total_CDC_Cycles_Completely_Missed>\n", TotalscanIndexDrop_cdc);
				}
			}
			loggedmap[logfilename].cdc_scanindex_drop = TotalscanIndexDrop_cdc;
		}
	}
	if (TotalscanIndexDrop_udp != 0)
	{
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Total Scans of Resim failure of the log in UDP Mode = %u\n", TotalscanIndexDrop_udp);
			loggedmap[logfilename].udp_scanindex_drop = TotalscanIndexDrop_udp;
			//printf("A - udp_scanindex_drop = [%d]\n", TotalscanIndexDrop_udp);
		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Total_UDP_Cycles_Completely_Missed>%u</Total_UDP_Cycles_Completely_Missed>\n", TotalscanIndexDrop_udp);
			}
			loggedmap[logfilename].udp_scanindex_drop = TotalscanIndexDrop_udp;
			//printf("B - udp_scanindex_drop = [%d]\n", TotalscanIndexDrop_udp);
		}
	}
	//

	if (Total_ScanIdDropCount_CalibStream != 0)
	{
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Total Scans of Resim failure of the log in UDP Mode = %u\n", Total_ScanIdDropCount_CalibStream);
			loggedmap[logfilename].calib_scanindex_drop = Total_ScanIdDropCount_CalibStream;
		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Total_CALIB_Cycles_Completely_Missed>%u</Total_CALIB_Cycles_Completely_Missed>\n", Total_ScanIdDropCount_CalibStream);
			}
			loggedmap[logfilename].calib_scanindex_drop = Total_ScanIdDropCount_CalibStream;
		}
	}
	if (TotalScanIDCnt == 0)
	{
		printf("\n\n[ ERROR] : The provided log has only ONE SCAN INDEX \n\n");
		//fprintf(Sfp, "\t\t\tThe provided log has only ONE SCAN INDEX \n\n");
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Report Status == FAIL\n");
			fprintf(Sfp, "*******************************************************************************************\n");

		}
		else
		{
			loggedmap[logfilename].Status = "FAIL";
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Report_Status>PACKET_LOSS_FAIL</Report_Status>\n\n");
			}

		}
		return;
	}
	Total_percent_failure_udp = ((float)TotalscanIndexDrop_udp / (float)TotalScanIDCnt);
	Total_percent_failure_cdc = ((float)TotalscanIndexDrop_cdc / (float)TotalScanIDCnt);
	Total_percent_failure_udp = (100 * Total_percent_failure_udp);
	Total_percent_failure_cdc = (100 * Total_percent_failure_cdc);
	if (Total_percent_failure_cdc != 0)
	{
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Total percentage of Resim fail of the log for CDC : %f\n", Total_percent_failure_cdc);
		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Total_CDC_Mode_Failure>%.2f%%</Total_CDC_Mode_Failure>\n", Total_percent_failure_cdc);
			}
		}
	}
	if (Total_percent_failure_udp != 0)
	{
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Total percentage of Resim fail of the log for UDP : %f\n", Total_percent_failure_udp);
		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Total_UDP_Mode_Failure>%.2f%%</Total_UDP_Mode_Failure>\n", Total_percent_failure_udp);
			}
		}
	}

	if (TotalscanIndexDrop_cdc == 0 && TotalscanIndexDrop_udp == 0 && Total_percent_failure_udp == 0 && Total_percent_failure_cdc == 0 && Total_ScanIdDropCount_CalibStream == 0 && ((diff == 0 || diff <= 3)) && loggedmap[logfilename].BMW_Status != "FAIL")
	{
		Report_Status = 1;
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			//fprintf(Sfp,"Report Status == PASS\n");
			fprintf(Sfp, "*******************************************************************************************\n");
		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Report_Status>PASS</Report_Status>\n\n");
			}
			loggedmap[logfilename].Status = "PASS";
		}
	}
	else
	{
		Continous_ScanDrop_udp = 0;
		Continous_ScanDrop_cdc = 0;
		Continous_ScanDrop_calib = 0;
		Report_Status = 0;
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "Report Status == FAIL\n");
			fprintf(Sfp, "*******************************************************************************************\n");

		}
		else
		{

			loggedmap[logfilename].Status = "FAIL";
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				fprintf(Sfp, "\t\t\t<Report_Status>PACKET_LOSS_FAIL</Report_Status>\n\n");
			}

		}

	}
}


void UpdateF360TrackerStatistics() {

	unsigned int Total_F360_Scandrop = 0;
	//Report_Status = false;
	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{

		fprintf(fCreate, "Below shows the Statistics of packetlosss of F360 Tracker internal packets\n");
		fprintf(fCreate, "*******************************************************************************************\n");
	}
	for (int i = 0; i < MAX_RADAR_COUNT; ++i) {

		/*if(MUDP_master_config->Packetloss_report_format != Text_format)
		{
		if(stat_f360.totalpacketloss[i] != 0 && stat_f360.duplicate_data[i] != 0)
		{
		fprintf(fCreate," \t\t\t\t<F360_Packet_loss_info_%s>\n",GetRadarPosName(i));
		}
		}*/
		if ((strcmp(GetRadarPosName(i), "ECU")) == 0) {
			if (stat_f360.totalpacketloss[i] != 0)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(fCreate, " The total f360 packet loss for [%s] = %u\n", GetRadarPosName(i), stat_f360.totalpacketloss[i]);

				}
				else
				{
					fprintf(fCreate, " \t\t\t\t\t<The_total_f360_packet_loss> %u</The_total_f360_packet_loss> \n", stat_f360.totalpacketloss[i]);
					fprintf(fCreate, " \t\t\t\t\t<ChecksumErrorCount> %u</ChecksumErrorCount> \n", stat_f360.totalcheckSumFailure[i]);
					loggedmap[logfilename].F360_ScanDrop[i] = stat_f360.totalpacketloss[i];
					Total_F360_Scandrop = stat_f360.totalpacketloss[i] + Total_F360_Scandrop;
					loggedmap[logfilename].F360_scanindex_drop = Total_F360_Scandrop;
					loggedmap[logfilename].ChecksumErrorCount_F360 = stat_f360.totalChecksumErrors;
					//fprintf(fCreate,"\t</Packet_Loss_Statistics>\n");
					//fprintf(fCreate,"\t</Packet_Loss_Info>\n");
				}
			}
			if (stat_f360.duplicate_data[i] != 0)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(fCreate, " The total f360 duplicate packet for [%s] = %u\n", GetRadarPosName(i), stat_f360.duplicate_data[i]);
				}
				else
				{
					fprintf(fCreate, " \t\t\t\t\t<The_total_f360_Duplicate_packet> %u</The_total_f360_Duplicate_packet> \n", stat_f360.duplicate_data[i]);
				}
			}
		}
	}
	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{
		fprintf(fCreate, "*******************************************END OF F360 TRACKER**********************************************\n");
	}
	else
	{
		//fprintf(fCreate," \t\t\t\t</F360_Packet_loss_info_%s>\n",GetRadarPosName(i));
		fprintf(fCreate, "\t\t</F360_Error_Report>\n");
	}

}
void UpdateGDSRTrackerStatistics() {

	unsigned int Total_Gdsr_Scanindex_drop = 0;
	//Report_Status = false;
	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{

		fprintf(fCreate, "Below shows the Statistics of packetlosss of GDSR Tracker internal packets\n");
		fprintf(fCreate, "*******************************************************************************************\n");
	}
	for (int i = 0; i < MAX_RADAR_COUNT; ++i) {
		if ((strcmp(GetRadarPosName(i), "INVALID_RADAR") == 0)) {
			continue;
		}
		else if ((strcmp(GetRadarPosName(i), "ECU")) == 0) {
			continue;
		}
		else {
			if (stat_gdsr.totalpacketloss[i] > 0)
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(fCreate, " The total gdsr packet loss for [%s] = %u\n", GetRadarPosName(i), stat_gdsr.totalpacketloss[i]);
				}
				else
				{
					fprintf(fCreate, " \t\t\t\t<GDSR_packet_loss_info_%s>\n", GetRadarPosName(i));
					fprintf(fCreate, " \t\t\t\t\t<The_total_GDSR_packet_loss>%u</The_total_GDSR_packet_loss> \n", stat_gdsr.totalpacketloss[i]);
					fprintf(fCreate, " \t\t\t\t\t<ChecksumErrorCount>%u</ChecksumErrorCount> \n", stat_gdsr.totalcheckSumFailure[i]);
					fprintf(fCreate, " \t\t\t\t</GDSR_Packet_loss_info_%s>\n", GetRadarPosName(i));
					loggedmap[logfilename].GDSR_ScanDrop[i] = stat_gdsr.totalpacketloss[i];
					loggedmap[logfilename].ChecksumErrorCount_GDSR = stat_gdsr.totalChecksumErrors;
					Total_Gdsr_Scanindex_drop = stat_gdsr.totalpacketloss[i] + Total_Gdsr_Scanindex_drop;
					loggedmap[logfilename].gdsr_scanindex_drop = Total_Gdsr_Scanindex_drop;
				}
			if (stat_gdsr.scanIndexfailure[i].size() > 0) {
				uint32_t s = stat_gdsr.scanIndexfailure[i].at(0);
				for (auto it = stat_gdsr.scanIndexfailure[i].begin(); it != stat_gdsr.scanIndexfailure[i].end(); it++) {
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(fCreate, " The gdsr ScanIndex loss for [%s] = %u\n", GetRadarPosName(i), *it);
					}
					else
					{
						fprintf(fCreate, " \t\t\t\t<GDSR_packet_loss_info_%s>\n", GetRadarPosName(i));
						fprintf(fCreate, " \t\t\t\t\t<The_gdsr_ScanIndex_loss>%u</The_gdsr_ScanIndex_loss> \n", *it);
						fprintf(fCreate, " \t\t\t\t</GDSR_Packet_loss_info_%s>\n", GetRadarPosName(i));

					}
				}
			}
		}

		//fprintf(fCreate, "Total Missing Scan Index for  [%s] = %u\n",GetRadarPosName(i),ScanIdDropCount_udp[i]);
	}

	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{

		fprintf(fCreate, "*******************************************END OF GDSR TRACKER INTERNAL*********************************************\n");
	}
	else
	{
		fprintf(fCreate, "\t\t</GDSR_Error_Report>\n");
	}
}
void Asynchprint_msg(unsigned int TotalAsynchronous_count, FILE* Sfp)
{

	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{
		fprintf(Sfp, "Total Async Bit set = %u\n", TotalAsynchronous_count);
	}
	else
	{
		fprintf(Sfp, "\t\t\t\t<Total_Asynchronous_Bit_set>%u</Total_Asynchronous_Bit_set>\n", TotalAsynchronous_count);
	}

	for (int i = 0; i < MAX_BUSID_NAME; i++)
	{
		if (i != 5 && i != 17 && i != 18)
		{
			if (Async_KPI_count[i] != 0)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(Sfp, "Total Async Bit set for [%s] = %u\n", GetAsyncBusIdName(i), Async_KPI_count[i]);
				}
				else
				{
					fprintf(Sfp, "\t\t\t\t<Total_Asynchronous_Bit_set_for_%s_to_Failure_Count_Async_%s>%u</Total_Asynchronous_Bit_set_for_%s_to_Failure_Count_Async_%s>\n", GetAsyncBusIdName(i), GetAsyncBusIdName(i), Async_KPI_count[i], GetAsyncBusIdName(i), GetAsyncBusIdName(i));
				}
			}
		}
	}
	if (MUDP_master_config->Packetloss_report_format == Text_format)
	{

		fprintf(Sfp, "*******************************************END of Asynchronous Error information*********************************************\n");
	}
	else
	{
		fprintf(Sfp, "\t\t</Asynchronous_Error_Information>\n");
		fprintf(Sfp, "\t</Packet_Loss_Statistics>\n");
		//fprintf(Sfp, "</Packet_Loss_Info>\n");
		fprintf(fCreate, "</Log_%d_%s>\n", inc_flag, logfilename);
	}
}
void Timing_Overrun_Print(FILE* Sfp, unsigned int pos)
{
	if (MUDP_master_config->packet_loss_statistics == 1)
	{
		if (Timingoverflowcount[pos][Z7A_LOGGING_DATA] != 0)
		{
			fprintf(Sfp, "\t\t\t\t<Timing_Overruns_counts_for_core1>%d</Timing_Overruns_counts_for_core1>\n", Timingoverflowcount[pos][Z7A_LOGGING_DATA]);
		}
		if (Timingoverflowcount[pos][Z7B_LOGGING_DATA] != 0)
		{
			fprintf(Sfp, "\t\t\t\t<Timing_Overruns_counts_for_core2>%d</Timing_Overruns_counts_for_core2>\n", Timingoverflowcount[pos][Z7A_LOGGING_DATA]);
		}
		if (Timingoverflowcount[pos][RADAR_ECU_CORE_1] != 0)
		{
			fprintf(Sfp, "\t\t\t\t<Timing_Overruns_counts_for_ECU>%d</Timing_Overruns_counts_for_ECU>\n", Timingoverflowcount[pos][Z7A_LOGGING_DATA]);
		}
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "*******************************************END of Timingoverrun information*********************************************\n");
		}
		else
		{
			fprintf(Sfp, "\t\t\t</Timingoverrun_Error_Report_%s>\n\n", GetRadarPosName(pos));

		}
	}

}
void CloseScanIndexerrorFile(FILE* Sfp, const char* LogFname) {
	int Idx = 0;
	float Core_percent_failure[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0.0 };
	float per_dividend[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0.0 };
	float percent_failure_udp[MAX_RADAR_COUNT] = { 0.0 };
	float percent_failure_cdc[MAX_RADAR_COUNT] = { 0.0 };
	unsigned int TotalAsynchronous_count = 0;
	unsigned int count = 0;
	int radarCnt = 0;
	int streamNo = 0;
	bool printone = true;
	//loggedmap[logfilename].Total_Scan_Index[pos] = ScanIdCount[pos];
	int MAX_streamNo = DSPACE_CUSTOMER_DATA;
	if (Sfp == NULL) {

		perror("Error in opening Error_Log file...\n");
	}
	else {
		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			fprintf(Sfp, "\n");
			fprintf(Sfp, "*******************************************************************************************\n");
			fprintf(Sfp, "Below shows the Statistics of ScanIndexloss/packetloss/Xsum failures of the\n");
			fprintf(Sfp, "log:%s\n", LogFname);
		}
		else
		{
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) {
				if (_printonce)
				{
					fprintf(fCreate, "\t<Detailed_Error_Information>\n");
					_printonce = false;
				}
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(Sfp, "\t</Detailed_Error_Information>\n");
			}
			if (MUDP_master_config->packet_loss_statistics == 1)
				fprintf(Sfp, "\t<Packet_Loss_Statistics>\n");
		}
	}
	if (MUDP_master_config->packet_loss_statistics == 1)
	{
		if (MUDP_master_config->Bpillar_status == 1 && MUDP_master_config->MUDP_Radar_ECU_status == 0) {
			radarCnt = MUDP_MAX_STATUS_COUNT;
		}
		else if (MUDP_master_config->MUDP_Select_Sensor_Status[MUDP_FLR4P_FC_09] == 1)
		{
			radarCnt = MUDP_MAX_STATUS_COUNT;
		}
		else if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
			radarCnt = MAX_RADAR_COUNT;
			//streamNo = RADAR_ECU_CORE_0;
			MAX_streamNo = RADAR_ECU_CALIB + 1;
		}
		else {
			radarCnt = MAX_SRR3_SOURCE;
		}
	}
	else
	{
		radarCnt = MAX_RADAR_COUNT;
	}


	if (MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
	{
		if (Sfp) {
			TotalScanIndexerrorFile(Sfp);

			for (int pos = 0; pos < radarCnt; pos++) {
				loggedmap[logfilename].Total_Scan_Index[pos] = ScanIdCount[pos];
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->MUDP_Select_Sensor_Status[pos] != 1) {
						if (MUDP_master_config->MUDP_Radar_ECU_status == 1) {
							if (pos != RDR_POS_ECU)
								continue;
						}
						else {
							continue;
						}
					}
				}
				if (Report_Status == 0)
				{
					if ((TotalscanIndexDropCount_udp[pos] != 0) || (TotalscanIndexDropCount_cdc[pos] != 0) || ScanIdDropCount_CalibStream[pos] != 0)
					{
						if (MUDP_master_config->Packetloss_report_format == Text_format)
						{
							if (MUDP_master_config->packet_loss_statistics == 1)
							{
								fprintf(Sfp, "PACKET LOSS INFO FOR[%s] :\n", GetRadarPosName(pos));
								fprintf(Sfp, "Total Scan Index for [%s] = %u\n", GetRadarPosName(pos), ScanIdCount[pos]);
								if (pos != RDR_POS_ECU) {

									fprintf(Sfp, "Total Missing Scan Index for UDP [%s] = %u\n", GetRadarPosName(pos), ScanIdDropCount_udp[pos]);
									fprintf(Sfp, "Total Missing Scan Index for CDC[%s] = %u\n", GetRadarPosName(pos), ScanIdDropCount_cdc[pos]);

								}
								else
								{
									fprintf(Sfp, "Total Missing Scan Index for  [%s] = %u\n", GetRadarPosName(pos), ScanIdDropCount_udp[pos]);
									fprintf(Sfp, "Total Async Bit set for [%s] = %u\n", GetRadarPosName(pos), Async_KPI_count[pos]);
								}
								fprintf(Sfp, "Total Source_Txn_Cnt Drop for [%s] = %u\n", GetRadarPosName(pos), SrcTxnDropCount[pos]);
							}

						}
						else
						{
							if (MUDP_master_config->packet_loss_statistics == 1)
							{
								fprintf(Sfp, "\t\t\t<Packet_Loss_Info_%s>\n", GetRadarPosName(pos));
								fprintf(Sfp, "\t\t\t\t<Total_Scan_Index>%u</Total_Scan_Index>\n", ScanIdCount[pos]);

								if (pos != RDR_POS_ECU) {
									fprintf(Sfp, "\t\t\t\t<CDC_Cycles_Completely_Missed>%u</CDC_Cycles_Completely_Missed>\n", ScanIdDropCount_cdc[pos]);
									loggedmap[logfilename].cdc_scandrop[pos] = ScanIdDropCount_cdc[pos];
									fprintf(Sfp, "\t\t\t\t<UDP_Cycles_Completely_Missed>%u</UDP_Cycles_Completely_Missed>\n", ScanIdDropCount_udp[pos]);
									fprintf(Sfp, "\t\t\t\t<CALIB_Cycles_Completely_Missed>%u</CALIB_Cycles_Completely_Missed>\n", ScanIdDropCount_CalibStream[pos]);
									fprintf(Sfp, "\t\t\t\t<ChecksumErrorCount>%u</ChecksumErrorCount>\n", loggedmap[logfilename].ChecksumErrorCount[pos]);
									loggedmap[logfilename].Checksumcount_Sensors += loggedmap[logfilename].ChecksumErrorCount[pos];
									loggedmap[logfilename].udp_scandrop[pos] = ScanIdDropCount_udp[pos];
									loggedmap[logfilename].calib_scandrop[pos] = ScanIdDropCount_CalibStream[pos];

								}
								else
								{
									fprintf(Sfp, "\t\t\t\t<Total_Missing_Scan_Index>%u</Total_Missing_Scan_Index>\n", ScanIdDropCount_udp[pos]);
									fprintf(Sfp, "\t\t\t\t<ChecksumErrorCount>%u</ChecksumErrorCount>\n", loggedmap[logfilename].ChecksumErrorCount[pos]);
									loggedmap[logfilename].udp_scandrop[pos] = ScanIdDropCount_udp[pos];
									loggedmap[logfilename].Checksumcount_ECU += loggedmap[logfilename].ChecksumErrorCount[pos];
								}
								//fprintf(Sfp, "\t\t\t\t<Total_Source_Txn_Cnt_Drop>%u</Total_Source_Txn_Cnt_Drop>\n", SrcTxnDropCount[pos]);
							}
						}
					}

				}

				if (ScanIdCount[pos] != 0) {
					for (int i = 0; i < MAX_streamNo; i++) {

						if (i == Z7B_LOGGING_DATA_DUMMY || i == DEBUG_DATA || i == Z7B_CUST_TRACKER_DATA || i == DSPACE_CUSTOMER_DATA)
						{
							continue;
						}
						if (pos != RDR_POS_ECU)
						{
							if (i >= RADAR_ECU_CORE_0 && i <= MAX_streamNo)
							{
								continue;
							}
						}
						else if (pos == RDR_POS_ECU)
						{

							if (i >= Z7A_LOGGING_DATA && i < RADAR_ECU_CORE_0)
							{
								continue;
							}
						}
						if (ChecksumErrorCount[pos][i] >= UDPPacketLossErrorCount[pos][i])
						{
							ScanindexDropCount[pos][i] = ChecksumErrorCount[pos][i];
						}
						else if (ChecksumErrorCount[pos][i] <= UDPPacketLossErrorCount[pos][i]) {
							ScanindexDropCount[pos][i] = UDPPacketLossErrorCount[pos][i];
						}
						if (ScanindexDropCount[pos][i] > ScanIdCount[pos]) {
							int offset = ScanindexDropCount[pos][i] - ScanIdCount[pos];
							ScanIdCount[pos] = ScanIdCount[pos] + offset;
						}
						//string strm = GetStreamName(i);
						per_dividend[pos][i] = ((float)ScanindexDropCount[pos][i] / (float)ScanIdCount[pos]);
						Core_percent_failure[pos][i] = (100.00 * per_dividend[pos][i]);
						//	fprintf(Sfp, "[%s]- %s, chunks lost = %u,  Xsum failure = %u, CRC_failure:%u Scan Index Drops = %u, percentage of failure = %f\n",GetRadarPosName(pos),GetStreamName(i),UDPPacketLossErrorCount[pos][i],
						//	ChecksumErrorCount[pos][i], CRCErrorCount[pos][i],ScanindexDropCount[pos][i],Core_percent_failure[pos][i]);

						UDPPacketLossErrorCount[pos][i] = 0;
						ChecksumErrorCount[pos][i] = 0;
						ScanindexDropCount[pos][i] = 0;
						CRCErrorCount[pos][i] = 0;
					}
					percent_failure_udp[pos] = ((float)TotalscanIndexDropCount_udp[pos] / (float)ScanIdCount[pos]);
					percent_failure_cdc[pos] = ((float)TotalscanIndexDropCount_cdc[pos] / (float)ScanIdCount[pos]);
					percent_failure_udp[pos] = (100 * percent_failure_udp[pos]);
					percent_failure_cdc[pos] = (100 * percent_failure_cdc[pos]);


					if (pos != RDR_POS_ECU) {
						if (Report_Status == 0)
						{
							if ((TotalscanIndexDropCount_udp[pos] != 0) || (percent_failure_udp[pos] != 0) || (TotalscanIndexDropCount_cdc[pos] != 0) || (percent_failure_cdc[pos] != 0) || (ScanIdDropCount_CalibStream[pos] != 0))
							{
								if (MUDP_master_config->packet_loss_statistics == 1)
								{
									if (MUDP_master_config->Packetloss_report_format == Text_format)
									{
										fprintf(Sfp, "Total Scans of Resim failure for [%s] for UDP = %u\n", GetRadarPosName(pos), (TotalscanIndexDropCount_udp[pos]));
										fprintf(Sfp, "Total percentage of Resim fail for UDP [%s]: %f\n", GetRadarPosName(pos), percent_failure_udp[pos]);
										fprintf(Sfp, "Total Scans of Resim failure for [%s] for CDC= %u\n", GetRadarPosName(pos), (TotalscanIndexDropCount_cdc[pos]));
										fprintf(Sfp, "Total percentage of Resim fail for CDC  [%s]: %f\n", GetRadarPosName(pos), percent_failure_cdc[pos]);
										fprintf(Sfp, " \n");
									}
									else
									{
										fprintf(Sfp, "\t\t\t\t<Total_CDC_Missed>%u</Total_CDC_Missed>\n", (TotalscanIndexDropCount_cdc[pos]));
										fprintf(Sfp, "\t\t\t\t<Total_UDP_Missed>%u</Total_UDP_Missed>\n", (TotalscanIndexDropCount_udp[pos]));
										fprintf(Sfp, "\t\t\t\t<CDC_Mode_Failure> %.2f%%</CDC_Mode_Failure>\n", percent_failure_cdc[pos]);
										fprintf(Sfp, "\t\t\t\t<UDP_Mode_Failure>%.2f%%</UDP_Mode_Failure>\n", percent_failure_udp[pos]);

										fprintf(Sfp, "\t\t\t</Packet_Loss_Info_%s>\n", GetRadarPosName(pos));
										fprintf(Sfp, " \n");
									}
								}
							}
						}
					}
					else
					{
						if (Report_Status == 0)
						{
							if ((TotalscanIndexDropCount_udp[pos] != 0) || (percent_failure_udp[pos] != 0))
							{
								if (MUDP_master_config->packet_loss_statistics == 1)
								{
									if (MUDP_master_config->Packetloss_report_format == Text_format)
									{
										fprintf(Sfp, "Total Scans of Resim failure for [%s] = %u\n", GetRadarPosName(pos), (TotalscanIndexDropCount_udp[pos]));
										fprintf(Sfp, "Total percentage of Resim fail for  [%s]: %f\n", GetRadarPosName(pos), percent_failure_udp[pos]);
										//fprintf(Sfp, "Total Scans of Resim failure for [%s] for CDC= %u\n", GetRadarPosName(pos),(TotalscanIndexDropCount_cdc[pos] + ScanIdDropCount_cdc[pos]));
										//fprintf(Sfp, "Total percentage of Resim fail for CDC  [%s]: %f\n", GetRadarPosName(pos),percent_failure_cdc[pos]);
										fprintf(Sfp, " \n");
									}
									else
									{
										fprintf(Sfp, "\t\t\t\t<Total_UDP_Missed>%u</Total_UDP_Missed>\n", (TotalscanIndexDropCount_udp[pos]));
										fprintf(Sfp, "\t\t\t\t<UDP_Mode_Failure>%.2f</UDP_Mode_Failure>\n", percent_failure_udp[pos]);
										fprintf(Sfp, "\t\t\t</Packet_Loss_Info_%s>\n", GetRadarPosName(pos));
										fprintf(Sfp, " \n");
									}
								}
							}
						}
					}
				}
				else {
				if (TotalscanIndexDropCount_udp != 0 || TotalscanIndexDropCount_cdc != 0 || ScanIdDropCount_CalibStream != 0 || (diff != 0 && diff > 3))//|| Total_percent_failure_udp != 0 && Total_percent_failure_cdc != 0 || loggedmap[logfilename].BMW_Status == "FAIL"
				{
					if (MUDP_master_config->packet_loss_statistics == 1)
					{
						if (UDP_Packetloss_Tag_Opened == FALSE) {
							fprintf(Sfp, "\t\t<UDP_Packet_Loss_Report>\n");
							UDP_Packetloss_Tag_Opened = TRUE;
						}
						
					}
				}
				if (MUDP_master_config->packet_loss_statistics == 1)
					{
						fprintf(Sfp, "\t\t\tNo data available in  %s\n", GetRadarPosName(pos));				
					}
				}

				TotalscanIndexDropCount_udp[pos] = 0;
				TotalscanIndexDropCount_cdc[pos] = 0;
				ScanIdCount[pos] = 0;
				ScanIdDropCount_cdc[pos] = 0;
				ScanIdDropCount_udp[pos] = 0;
				SrcTxnDropCount[pos] = 0;
				ScanIdDropCount_CalibStream[pos] = 0;
			}
		}
		if (printone)
		{
			if (/*Report_Status == 0 || */UDP_Packetloss_Tag_Opened != FALSE)
			{

				if (MUDP_master_config->Packetloss_report_format == XML_format)
				{
					if (MUDP_master_config->packet_loss_statistics == 1)
					{
						fprintf(Sfp, "\t\t</UDP_Packet_Loss_Report>\n");
					}
				}
			}
			printone = false;
		}


		for (int pos = 0; pos < radarCnt; pos++) {
			if (Timingoverflowcount[pos][Z7A_LOGGING_DATA] == 0 && Timingoverflowcount[pos][Z7B_LOGGING_DATA] == 0 && Timingoverflowcount[pos][RADAR_ECU_CORE_1] == 0)
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for Timingoverrun Tracker == PASS\n");
						//fprintf(fCreate, "*******************************************END OF GDSR TRACKER**********************************************\n");
					}
					else
					{
						/*fprintf(Sfp,"\t\t<GDSR_Error_Report>\n");
						fprintf(Sfp,"\t\t\t<Report_Status_for_GDSR_Tracker>PASS</Report_Status_for_GDSR_Tracker>\n");
						fprintf(Sfp,"\t\t</GDSR_Error_Report>\n");*/
					}
				}
			}
			else
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for Timingoverrun == FAIL\n");
					}
					else
					{
						if (Timingoverrun_Error_print_once == 1)
						{
							fprintf(Sfp, "\t\t<Timingoverrun_Error_Report>\n");
							fprintf(Sfp, "\t\t\t<Report_Status_for_Timingoverrun>OVERRUN_FAIL</Report_Status_for_Timingoverrun>\n\n");
							Timingoverrun_Error_print_once = 2;
						}

					}
					fprintf(Sfp, "\t\t\t<Timingoverrun_Error_Report_%s>\n", GetRadarPosName(pos));
					Timing_Overrun_Print(Sfp, pos);
				}

			}
		}
		if (Timingoverrun_Error_print_once == 2)
		{
			fprintf(Sfp, "\t\t</Timingoverrun_Error_Report>\n");
		}



		if ((MUDP_master_config->corner_Sensor_Status != 0) || (MUDP_master_config->Bpillar_status != 0)) {
			if (!gdsrpacket_loss_status)
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for GDSR Tracker == PASS\n");
						fprintf(fCreate, "*******************************************END OF GDSR TRACKER**********************************************\n");
					}
					else
					{
						/*fprintf(Sfp,"\t\t<GDSR_Error_Report>\n");
						fprintf(Sfp,"\t\t\t<Report_Status_for_GDSR_Tracker>PASS</Report_Status_for_GDSR_Tracker>\n");
						fprintf(Sfp,"\t\t</GDSR_Error_Report>\n");*/
					}
				}

			}
			else
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for GDSR Tracker == FAIL\n");
					}
					else
					{
						fprintf(Sfp, "\t\t<GDSR_Error_Report>\n");
						fprintf(Sfp, "\t\t\t<Report_Status_for_GDSR_Tracker>FAIL</Report_Status_for_GDSR_Tracker>\n");

					}
				}
			}

		}
		if (MUDP_master_config->packet_loss_statistics == 1)
		{
			if (gdsrpacket_loss_status)
			{
				UpdateGDSRTrackerStatistics();
			}
		}
		if ((MUDP_master_config->MUDP_Radar_ECU_status != 0)) {
			if (!f360packet_loss_status)
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for F360 Tracker == PASS\n");
						fprintf(fCreate, "*******************************************END OF F360 TRACKER**********************************************\n");
					}
					else
					{
						/*fprintf(Sfp,"\t\t<F360_Error_Report>\n");
						fprintf(Sfp,"\t\t\t<Report_Status_for_F360>PASS</Report_Status_for_F360>\n");
						fprintf(Sfp,"\t\t</F360_Error_Report>\n");*/
					}
				}
			}
			else
			{
				if (MUDP_master_config->packet_loss_statistics == 1)
				{
					if (MUDP_master_config->Packetloss_report_format == Text_format)
					{
						fprintf(Sfp, "Report Status for F360 Tracker == FAIL\n");
					}
					else
					{
						fprintf(Sfp, "\t\t<F360_Error_Report>\n");
						fprintf(Sfp, "\t\t\t<Report_Status_for_F360>FAIL</Report_Status_for_F360>\n");

					}
				}
			}

		}
		if (MUDP_master_config->packet_loss_statistics == 1)
		{
			if (f360packet_loss_status)
			{
				UpdateF360TrackerStatistics();
			}
		}


		for (int i = 0; i < MAX_BUSID_NAME; i++)
		{

			TotalAsynchronous_count += Async_KPI_count[i];
		}
		if (!TotalAsynchronous_count)
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(Sfp, "\n");
					fprintf(Sfp, "Report Status for Asynchronous set bit == PASS\n");

					fprintf(Sfp, "******************************************* END of Asynchronous set bit information*********************************************\n");
				}
				else
				{
					fprintf(Sfp, "\n");
					/*fprintf(Sfp, "\t\t<Asynchronous_Error_Information>\n");
					fprintf(Sfp,"\t\t\t<Report_Status_for_Asynchronous_set_bit>PASS</Report_Status_for_Asynchronous_set_bit>\n");
					fprintf(Sfp, "\t\t</Asynchronous_Error_Information>\n");*/
					fprintf(Sfp, "\t</Packet_Loss_Statistics>\n");
					//fprintf(Sfp, "</Log_Quality_Check>\n");
					fprintf(fCreate, "</Log_%d_%s>\n", inc_flag, logfilename);

				}
			}

		}
		else
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(Sfp, "\n");
					fprintf(Sfp, "Asynchronous Error Information");
					fprintf(Sfp, "Report Status for Asynchronous set bit == FAIL\n");
					//fprintf(fCreate, "*******************************************END OF GDSR TRACKER**********************************************\n");
				}
				else
				{
					fprintf(Sfp, "\n");
					fprintf(Sfp, "\t\t<Asynchronous_Error_Information>\n");
					fprintf(Sfp, "\t\t\t<Report_Status_for_Asynchronous_set_bit>FAIL</Report_Status_for_Asynchronous_set_bit>\n");

				}
				if (TotalAsynchronous_count)
				{
					Asynchprint_msg(TotalAsynchronous_count, Sfp);
				}
			}
		}

	}
	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 && MUDP_master_config->packet_loss_statistics == 0)
	{
		fprintf(fCreate, "</Log_%d_%s>\n", inc_flag, logfilename);
	}
	if (Sfp != NULL) {
		if ((Report_Status == 1) && (TotalAsynchronous_count) && (f360packet_loss_status) && (gdsrpacket_loss_status))
		{
			if (MUDP_master_config->packet_loss_statistics == 1)
			{
				if (MUDP_master_config->Packetloss_report_format == Text_format)
				{
					fprintf(Sfp, "The Log :(%s) has no packet loss\n", LogFname);
					fprintf(Sfp, " \n");
				}
				else
				{
					fprintf(Sfp, "<No_packet_loss_for_Log>(%s)</No_packet_loss_for_Log>\n", LogFname);
					fprintf(Sfp, " \n");
				}
			}

		}

		if (MUDP_master_config->Packetloss_report_format == Text_format)
		{
			fprintf(Sfp, "*******************************************************************************************\n");
		}
		fclose(Sfp);
		Sfp = NULL;

	}
}


void setXmlOptions(void) {

	if (MUDP_master_config->Output_Xml_Trace_Option[Hil_Port_Xml] == 1)
	{
		f_Hil_xml = 1;
		xml_option[HIL_P_XML] = 1;
	}
	if (MUDP_master_config->Output_Xml_Trace_Option[Sensor_Xml] == 1)
	{
		f_Sensor_xml = 1;
		xml_option[SENSOR_P_XML] = 1;
	}
}
void setStreamOptions(void) {
	if(strcmp(MUDP_master_config->GenType, "GEN5") == 0)
	{
		
		if (MUDP_master_config->Radar_Stream_Options[OSI_Stream] == OSI_Stream)
		{
			dump[UDP_OSI] = 1;
			f_osi_enabled = 1;
			Streamver.OSI_STREAM = UDP_OSI;
		}
		if (MUDP_master_config->Radar_Stream_Options[Z7B_Core] == Z7B_Core)
		{
			dump[UDP_Z7B_CORE] = 1;
			f_z7b_enabled = 1;
			Streamver.Z7B_Core = UDP_Z7B_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[Z7A_Core] == Z7A_Core)
		{
			dump[UDP_Z7A_CORE] = 1;
			f_z7a_enabled = 1;
			Streamver.Z7A_Core = UDP_Z7A_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[Z4_Core] == Z4_Core)
		{
			dump[UDP_Z4_CORE] = 1;
			f_z4_enabled = 1;
			Streamver.Z4_Core = UDP_Z4_CORE;
		}

		if (MUDP_master_config->Radar_Stream_Options[Z7B_Customer] == Z7B_Customer)
		{
			dump[UDP_CUSTOM_Z7B] = 1;
			f_z7b_custom_enabled = 1;
			Streamver.Z7B_Customer = UDP_CUSTOM_Z7B;
		}

		if (MUDP_master_config->Radar_Stream_Options[Z4_Customer] == Z4_Customer)
		{
			dump[UDP_CUSTOM_Z4] = 1;
			f_z4_custom_enabled = 1;
			Streamver.Z4_Customer = UDP_CUSTOM_Z4;
		}
		if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] == ecu0)
		{
			dump[UDP_RDR_ECU0] = 1;
			f_ECU0_enabled = 1;
			Streamver.ecu0 = UDP_RDR_ECU0;
		}
		if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu1] == ecu1)
		{
			dump[UDP_RDR_ECU1] = 1;
			f_ECU1_enabled = 1;
			Streamver.ecu1 = UDP_RDR_ECU1;
		}
		if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu3] == ecu3)
		{
			dump[UDP_RDR_ECU3] = 1;
			f_ECU3_enabled = 1;
			Streamver.ecu2 = UDP_RDR_ECU3;
		}
		if (MUDP_master_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier] == ecu_VRU_Classifier)
		{
			dump[UDP_RDR_ECU_VRU_Classifier] = 1;
			f_ECU_VRU_Classifier_enabled = 1;
			Streamver.ECU_VRU_Classifier = UDP_RDR_ECU_VRU_Classifier;
		}
	}

	else if(strcmp(MUDP_master_config->GenType, "GEN6") == 0) {
		if (MUDP_master_config->Radar_Stream_Options[C0_Core] == C0_Core)
		{
			dump[UDP_GEN5_C0_CORE] = 1;
			f_c0_core_enabled = 1;
			Streamver.C0_Core = UDP_GEN5_C0_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[C0_Cust] == C0_Cust)
		{
			dump[UDP_GEN5_C0_CUST] = 1;
			f_c0_cust_enabled = 1;
			Streamver.C0_Cust = UDP_GEN5_C0_CUST;
		}
		if (MUDP_master_config->Radar_Stream_Options[C1_Core] == C1_Core)
		{
			dump[UDP_GEN5_C1_CORE] = 1;
			f_c1_core_enabled = 1;
			Streamver.C1_Core = UDP_GEN5_C1_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[C2_Core] == C2_Core)
		{
			dump[UDP_GEN5_C2_CORE] = 1;
			f_c2_core_enabled = 1;
			Streamver.C2_Core = UDP_GEN5_C2_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[C2_Cust] == C2_Cust)
		{
			dump[UDP_GEN5_C2_CUST] = 1;
			f_c2_cust_enabled = 1;
			Streamver.C2_Cust = UDP_GEN5_C2_CUST;
		}
		if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 && MUDP_master_config->Radar_Stream_Options[C2_Core] == 0)
		{
			dump[UDP_GEN5_C2_CORE] = 1;
			f_c2_core_enabled = 1;
			Streamver.C2_Core = UDP_GEN5_C2_CORE;

		}
		if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 && MUDP_master_config->Radar_Stream_Options[C1_Core] == 0)
		{
			dump[UDP_GEN5_C1_CORE] = 1;
			f_c1_core_enabled = 1;
			Streamver.C1_Core = UDP_GEN5_C1_CORE;
		}

		//MOTIONAL FLR4P
		if (MUDP_master_config->Radar_Stream_Options[BC_CORE] == BC_CORE)
		{
			dump[UDP_GEN5_BC_CORE] = 1;
			f_bc_core_enabled = 1;
			Streamver.BC_CORE = UDP_GEN5_BC_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == VSE_CORE)
		{
			dump[UDP_GEN5_VSE_CORE] = 1;
			f_vse_core_enabled = 1;
			Streamver.VSE_CORE = UDP_GEN5_VSE_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == STATUS_CORE)
		{
			dump[UDP_GEN5_STATUS_CORE] = 1;
			f_status_core_enabled = 1;
			Streamver.STATUS_CORE = UDP_GEN5_STATUS_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == RDD_CORE)
		{
			dump[UDP_GEN5_RDD_CORE] = 1;
			f_rdd_core_enabled = 1;
			Streamver.RDD_CORE = UDP_GEN5_RDD_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DET_CORE] == DET_CORE)
		{
			dump[UDP_GEN5_DET_CORE] = 1;
			f_det_core_enabled = 1;
			Streamver.DET_CORE = UDP_GEN5_DET_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == HDR_CORE)
		{
			dump[UDP_GEN5_HDR_CORE] = 1;
			f_hdr_core_enabled = 1;
			Streamver.HDR_CORE = UDP_GEN5_HDR_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == TRACKER_CORE)
		{
			dump[UDP_GEN5_TRACKER_CORE] = 1;
			f_tracker_core_enabled = 1;
			Streamver.TRACKER_CORE = UDP_GEN5_TRACKER_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == DEBUG_CORE)
		{
			dump[UDP_GEN5_DEBUG_CORE] = 1;
			f_debug_core_enabled = 1;
			Streamver.DEBUG_CORE = UDP_GEN5_DEBUG_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ALIGNMENT_CORE] == ALIGNMENT_CORE)
		{
			dump[UDP_GEN7_ALIGNMENT_CORE] = 1;
			f_gen7_alignment_enabled = 1;
			Streamver.ALIGNMENT_CORE = UDP_GEN7_ALIGNMENT_CORE;
		}
	}

	else if (strcmp(MUDP_master_config->GenType, "GEN7") == 0) {
		if (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == VSE_CORE)
		{
			dump[UDP_GEN5_VSE_CORE] = 1;
			f_vse_core_enabled = 1;
			Streamver.VSE_CORE = UDP_GEN5_VSE_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == STATUS_CORE)
		{
			dump[UDP_GEN5_STATUS_CORE] = 1;
			f_status_core_enabled = 1;
			Streamver.STATUS_CORE = UDP_GEN5_STATUS_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == RDD_CORE)
		{
			dump[UDP_GEN5_RDD_CORE] = 1;
			f_rdd_core_enabled = 1;
			Streamver.RDD_CORE = UDP_GEN5_RDD_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DET_CORE] == DET_CORE)
		{
			dump[UDP_GEN5_DET_CORE] = 1;
			f_det_core_enabled = 1;
			Streamver.DET_CORE = UDP_GEN5_DET_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == HDR_CORE)
		{
			dump[UDP_GEN5_HDR_CORE] = 1;
			f_hdr_core_enabled = 1;
			Streamver.HDR_CORE = UDP_GEN5_HDR_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == TRACKER_CORE)
		{
			dump[UDP_GEN5_TRACKER_CORE] = 1;
			f_tracker_core_enabled = 1;
			Streamver.TRACKER_CORE = UDP_GEN5_TRACKER_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == DEBUG_CORE)
		{
			dump[UDP_GEN5_DEBUG_CORE] = 1;
			f_debug_core_enabled = 1;
			Streamver.DEBUG_CORE = UDP_GEN5_DEBUG_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[MMIC_CORE] == MMIC_CORE)
		{
			dump[UDP_GEN7_MMIC_CORE] = 1;
			f_gen7_mmic_enabled = 1;
			Streamver.MMIC_CORE = UDP_GEN7_MMIC_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ALIGNMENT_CORE] == ALIGNMENT_CORE)
		{
			dump[UDP_GEN7_ALIGNMENT_CORE] = 1;
			f_gen7_alignment_enabled = 1;
			Streamver.ALIGNMENT_CORE = UDP_GEN7_ALIGNMENT_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[BLOCKAGE_CORE] == BLOCKAGE_CORE)
		{
			dump[UDP_GEN7_BLOCKAGE_CORE] = 1;
			f_gen7_blockage_enabled = 1;
			Streamver.BLOCKAGE_CORE = UDP_GEN7_BLOCKAGE_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[RADAR_CAPABILITY_CORE] == RADAR_CAPABILITY_CORE)
		{
			dump[UDP_GEN7_RADAR_CAPABILITY_CORE] = 1;
			f_gen7_radar_capability_enabled = 1;
			Streamver.RADAR_CAPABILITY_CORE = UDP_GEN7_RADAR_CAPABILITY_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DOWN_SELECTION_CORE] == DOWN_SELECTION_CORE)
		{
			dump[UDP_GEN7_DOWN_SELECTION_CORE] = 1;
			f_gen7_down_selection_enabled = 1;
			Streamver.DOWN_SELECTION_CORE = UDP_GEN7_DOWN_SELECTION_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ID_CORE] == ID_CORE)
		{
			dump[UDP_GEN7_ID_CORE] = 1;
			f_gen7_id_enabled = 1;
			Streamver.ID_CORE = UDP_GEN7_ID_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[TOI_CORE] == TOI_CORE)
		{
			dump[UDP_GEN7_TOI_CORE] = 1;
			f_gen7_toi_enabled = 1;
			Streamver.TOI_CORE = UDP_GEN7_TOI_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[DYNAMIC_ALIGNMENT_CORE] == DYNAMIC_ALIGNMENT_CORE)
		{
			dump[UDP_GEN7_DYNAMIC_ALIGNMENT_CORE] = 1;
			f_gen7_dynamic_alignment_enabled = 1;
			Streamver.DYNAMIC_ALIGNMENT_CORE = UDP_GEN7_DYNAMIC_ALIGNMENT_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_SAFETY_FAULTS_CORE] == ROT_SAFETY_FAULTS_CORE)
		{
			dump[UDP_GEN7_ROT_SAFETY_FAULTS_CORE] = 1;
			f_gen7_ROT_safety_faults_enabled = 1;
			Streamver.ROT_SAFETY_FAULTS_CORE = UDP_GEN7_ROT_SAFETY_FAULTS_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_TRACKER_INFO_CORE] == ROT_TRACKER_INFO_CORE)
		{
			dump[UDP_GEN7_ROT_TRACKER_INFO_CORE] = 1;
			f_gen7_ROT_tracker_info_enabled = 1;
			Streamver.ROT_TRACKER_INFO_CORE = UDP_GEN7_ROT_TRACKER_INFO_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_VEHICLE_INFO_CORE] == ROT_VEHICLE_INFO_CORE)
		{
			dump[UDP_GEN7_ROT_VEHICLE_INFO_CORE] = 1;
			f_gen7_ROT_vehicle_info_enabled = 1;
			Streamver.ROT_VEHICLE_INFO_CORE = UDP_GEN7_ROT_VEHICLE_INFO_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE] == ROT_OBJECT_STREAM_CORE)
		{
			dump[UDP_GEN7_ROT_OBJECT_CORE] = 1;
			f_gen7_ROT_object_stream_enabled = 1;
			Streamver.ROT_OBJECT_CORE = UDP_GEN7_ROT_OBJECT_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_ISO_OBJECT_STREAM_CORE] == ROT_ISO_OBJECT_STREAM_CORE)
		{
			dump[UDP_GEN7_ROT_ISO_OBJECT_CORE] = 1;
			f_gen7_ROT_ROT_ISO_object_stream_enabled = 1;
			Streamver.ROT_ISO_OBJECT_CORE = UDP_GEN7_ROT_ISO_OBJECT_CORE;
		}
		if (MUDP_master_config->Radar_Stream_Options[ROT_PROCESSED_DETECTION_STREAM_CORE] == ROT_PROCESSED_DETECTION_STREAM_CORE)
		{
			dump[UDP_GEN7_ROT_PROCESSED_DETECTION_CORE] = 1;
			f_gen7_ROT_processed_detection_stream_enabled = 1;
			Streamver.ROT_PROCESSED_DETECTION_CORE = UDP_GEN7_ROT_PROCESSED_DETECTION_CORE;
		}

	}
	
	if (MUDP_master_config->Radar_Stream_Options[HDR] == HDR)
	{
		dump[HEADER] = 1;
		Streamver.HDR = HEADER;
	}

	if (MUDP_master_config->Radar_Stream_Options[Dspace] == Dspace)
	{
		dump[UDP_DSPACE] = 1;
		f_dspace_enabled = 1;
		Streamver.DSPACE = UDP_DSPACE;
	}

	if (MUDP_master_config->Radar_Stream_Options[CDC_iq] == CDC_iq)
	{
		dump[UDP_CDC_STRM] = 1;
		f_cdc_enabled = 1;
		Streamver.CDC = UDP_CDC_STRM;
	}
}


void GetInputOption(const char *Fname)
{
	string LogFName(Fname);
	size_t pos = LogFName.find(".srr3");
	bool four_dvsu_check = false;
	bool mf4_flist = false;
	bool vpcap_vigem = false;
	if (pos == string::npos) {
		pos = LogFName.find("_71.dvsu");
		if (pos != string::npos)
			four_dvsu_check = true;
		if (pos == string::npos) {
			pos = LogFName.find("_72.dvsu");
			if (pos != string::npos)
				four_dvsu_check = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find("_73.dvsu");
			if (pos != string::npos)
				four_dvsu_check = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find("_74.dvsu");
			if (pos != string::npos)
				four_dvsu_check = true;
		}
		if (four_dvsu_check == true && pos != string::npos) {
			MUDP_master_config->MUDPInput_Options = FOUR_DVSU_INPUT;
		}
	}

	if (pos == string::npos && !four_dvsu_check) {
		pos = LogFName.find(".dvsu");
		if (pos != string::npos) {
			four_dvsu_check = false;
			MUDP_master_config->MUDPInput_Options = SINGLE_DVSU_INPUT;
		}

	}
	if (pos == string::npos && !four_dvsu_check) {
		pos = LogFName.find(".mf4");
		if (pos != string::npos) {
			mf4_flist = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find(".MF4");
			if (pos != string::npos)
				mf4_flist = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find(".vpcap");
			if (pos != string::npos)
				vpcap_vigem = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find(".pcapng");
			if (pos != string::npos)
				vpcap_vigem = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find(".pcap");
			if (pos != string::npos)
				vpcap_vigem = true;
		}
		if (pos == string::npos) {
			pos = LogFName.find(".PCAP");
			if (pos != string::npos)
				vpcap_vigem = true;
		}

		if (vpcap_vigem)
			MUDP_master_config->MUDPInput_Options = VPCAP_VIGEM;
		if (mf4_flist)
			MUDP_master_config->MUDPInput_Options = MDF4_INPUT;
	}
}
bool checkMidHighSensorsEnabled()
{
	// Set MidHighSensorsEnabled if any of one sensor is enabled and Radar ECU status is 1
	// or B-Pillar Status is 1
	if (MUDP_master_config->MUDP_Radar_ECU_status == 1 || MUDP_master_config->Bpillar_status == 1) {
		for (int senID = 0; senID < 4; senID++) {
			if (MUDP_master_config->MUDP_Select_Sensor_Status[senID] == 1) {
				return true;
			}
		}
	}
}
//int CreateKpiFile(char* InputLogName)
//{
//	#if defined(_WIN_PLATFORM)
//	char drvName[_MAX_DRIVE];
//	char dir[_MAX_DIR];
//	char fName[_MAX_FNAME];
//	char ext[_MAX_EXT];
//	
//#elif defined(__GNUC__)
//	char *drvName = new char[_MAX_DRIVE];
//	char *dir = new char[_MAX_DIR];
//	char *fName = new char[_MAX_FNAME];
//	char *ext = new char[_MAX_EXT];
//#endif
//static int setflag = true;
//char newpath[MAX_FILEPATH_NAME_LENGTH] = {0};
//char  FileCreate_kpi[MAX_FILEPATH_NAME_LENGTH] ;
//int kret = 0;
//if(MUDP_master_config->MUDP_output_path == NEW_OUTPUT_PATH_FILE)
//	{
//		if((strcmp(MUDP_master_config->MUDP_output_folder_path, "NONE")==0) || (strcmp(MUDP_master_config->MUDP_output_folder_path,"")==0) ){
//			if(setflag)
//				fprintf(stderr," \n [INFO] File base name for NEW_OUTPUT_PATH is not found,Hence Default path: %s is considered for generating dvsu \n",InputLogName);
//			setflag = false ;
//		}
//		else{
//			strcpy(newpath, MUDP_master_config->MUDP_output_folder_path);
//			unsigned int st_Len = 0;
//			st_Len = strlen(newpath);
//			if(st_Len != 0)
//			{
//				if(newpath[st_Len - 1] != '\\'){
//					newpath[st_Len] = '\\';
//				}
//#if defined(_WIN_PLATFORM)
//				split_path(InputLogName, drvName, dir, fName, ext);
//#elif defined(__GNUC__)
//				split_pathLinux(InputLogName, &drvName, &dir, &fName, &ext);
//#endif
//				st_Len = strlen(fName);
//				//fName[st_Len - 3] = 0;
//				//fName[st_Len - 2] = 0;
//				//fName[st_Len - 1] = 0;
//				strcat(newpath, fName);
//				if(MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT)
//					sprintf(InputLogName,"%s_71.dvsu", newpath);
//				else
//					sprintf(InputLogName,"%s.dvsu", newpath);
//#if defined(_WIN_PLATFORM)
//				split_path(InputLogName, drvName, dir, fName, ext);
//#elif defined(__GNUC__)
//				split_pathLinux(InputLogName, &drvName, &dir, &fName, &ext);
//#endif
//			}
//		}
//	}
//	else
//	{
//#if defined(_WIN_PLATFORM)
//		split_path(InputLogName, drvName, dir, fName, ext);
//#elif defined(__GNUC__)
//		split_pathLinux(InputLogName, &drvName, &dir, &fName, &ext);
//#endif
//
//	}
//	if(	MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == MDF4_INPUT ||MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM)
//		{
//			sprintf(FileCreate_kpi, "%s%s_KpiError_Count.txt",drvName, dir);
//			KCreate = fopen(FileCreate_kpi, "a");
//			
//			if(!KCreate)
//			{
//				fprintf(stderr,"Error Creating Output file: %s",FileCreate_kpi);
//				kret = -1;
//			}else{
//				time_t now = time(NULL);
//				struct tm *t = localtime(&now);
//				fprintf(KCreate, "\n");
//				fprintf(KCreate, "*****************************************************************************************\n");
//				fprintf(KCreate, "This file provides all the KPIError Info present in the\n");
//				fprintf(KCreate,  "log : %s\n",InputLogName);
//				fprintf(KCreate, "Date: %d-%d-%d %d:%d:%d\n",t->tm_mon+1,t->tm_mday,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec);
//				fprintf(KCreate, "*****************************************************************************************\n");
//				fprintf(KCreate, "\n");
//				
//			}
//	}
//	#if defined(__GNUC__)
//		delete []drvName;
//		delete []dir;
//		delete []fName;
//		delete []ext ;
//    #endif
//	return kret;
//}
void gettimeStampSyncKPI(unsigned32_T Busid, char* inputLogFname, INT64 relative_Time, uint8_t Asynchronous_Bit)
{
	int ret = 0;
	int status = 0;
	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
	{
		if (_printonce)
		{
			fprintf(fCreate, "\t<Detailed_Error_Information>\n");
			_printonce = false;
		}
	}

	if (Asynchronous_Bit == 1)
	{

		if (MUDP_master_config->packet_loss_statistics == 1)
		{

			/*fprintf(fCreate, "\t\tAsynchronous_Bit is set for the Busid :%s and PLP timestamp: %lld\n",GetAsyncBusIdName(Busid),relative_Time);
			fprintf(fCreate, "\t\t=========================================================================================================\n");*/
		}
		Async_KPI_count[Busid]++;
	}
	else
	{
		//fprintf(KCreate, "=========================================================================================================\n");
		////Do Nothing////
	}
}

void RadarDataCheck(Customer_T cust_id)
{
	int status = 0;
	if (cust_id == BMW_SAT)
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL] == 1)
		{
			if (ScanIdCount[SRR_RL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR] == 1)
		{
			if (ScanIdCount[SRR_RR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL] == 1)
		{
			if (ScanIdCount[SRR_FL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR] == 1)
		{
			if (ScanIdCount[SRR_FR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Radar_ECU_status == 1)
		{
			if (ScanIdCount[RADAR_POS] == 0)
			{
				std::string posName = GetRadarPosName(RADAR_POS);
				loggedmap[logfilename].Observation += "No UDP DATA Found in  [" + posName + "].";
				status = 1;
			}
		}
	}

	else if (cust_id == BMW_BPIL)
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL] == 1)
		{
			if (ScanIdCount[SRR_RL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR] == 1)
		{
			if (ScanIdCount[SRR_RR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL] == 1)
		{
			if (ScanIdCount[SRR_FL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR] == 1)
		{
			if (ScanIdCount[SRR_FR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Radar_ECU_status == 1)
		{
			if (ScanIdCount[RADAR_POS] == 0)
			{
				std::string posName = GetRadarPosName(RADAR_POS);
				loggedmap[logfilename].Observation += "No UDP DATA Found in  [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_R] == 1)
		{
			if (ScanIdCount[BP_R] == 0)
			{
				std::string posName = GetRadarPosName(BP_R);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_L] == 1)
		{
			if (ScanIdCount[BP_L] == 0)
			{
				std::string posName = GetRadarPosName(BP_L);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
	}
	else if (cust_id == SCANIA_MAN)
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL] == 1)
		{
			if (ScanIdCount[SRR_RL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR] == 1)
		{
			if (ScanIdCount[SRR_RR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL] == 1)
		{
			if (ScanIdCount[SRR_FL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR] == 1)
		{
			if (ScanIdCount[SRR_FR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC] == 1)
		{
			if (ScanIdCount[SRR_FC] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FC);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
	}
	else
	{
		for (int pos = 0; pos < MAX_RADAR_COUNT; pos++)
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[pos] == 1)
			{
				if (ScanIdCount[pos] == 0)
				{
					std::string posName = GetRadarPosName(pos);
					loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
					loggedmap[logfilename].DQ_Radar_Fail_Status[pos] = "FAIL";
					loggedmap[logfilename].RESIM_Radar_Fail_Status[pos] = "FAIL";
					status = 1;
				}
			}
		}
		/*if(MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL] == 1)
		{
			if (ScanIdCount[SRR_RL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR] == 1)
		{
			if (ScanIdCount[SRR_RR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_RR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL] == 1)
		{
			if (ScanIdCount[SRR_FL] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FL);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR] == 1)
		{
			if (ScanIdCount[SRR_FR] == 0)
			{
				std::string posName = GetRadarPosName(SRR_FR);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}
		if (MUDP_master_config->MUDP_Select_Sensor_Status[FLR4P_FC_09] == 1)
		{
			if (ScanIdCount[FLR4P_FC_09] == 0)
			{
				std::string posName = GetRadarPosName(FLR4P_FC_09);
				loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
				status = 1;
			}
		}*/
	}

	if (status == 1)
	{
		loggedmap[logfilename].BMW_Status = "FAIL";
		loggedmap[logfilename].Status = "FAIL";
	}
}

int printFileStatusNoUDPData(char* inputLogFname) {	

	No_UDP_Data_Flag = true;
	
	for (int i = 0; i < MAX_RADAR_COUNT; i++)
	{
		if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1)
		{
			std::string posName = GetRadarPosName(i);
			loggedmap[logfilename].Observation += "No UDP DATA Found in Sensor [" + posName + "].";
		}
	}

	loggedmap[logfilename].BMW_Status = "FAIL";
	loggedmap[logfilename].Status = "FAIL";
	//loggedmap[logfilename].Observation += "No UDP DATA Found in the Log\n";
	Summerymap[count_flag] = loggedmap;
	//loggedmap.clear();
	//count_flag++;

	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
	{

		if (CallPktOnce) {
			if (-1 == CreateOutputPacketlossReport(inputLogFname)) {
				
				return -1;
			}
			CallPktOnce = false;
		}
	}

	return 0;
}

//#HTMLInMUDP
int DumpMUDPData(const char* LogFname, std::shared_ptr<HtmlReportManager>& sptr_manager, int filecnt) {

	//std::cout << "\n DumpMUDPData";
	int ret = 0;
	DVSU_RECORD_T record = { 0 };
	unsigned int iMudpStrNo = 0;
	char sourceId = 0;
	unsigned int RadarPos = 0;
	unsigned int Stream_No = 0;
	UINT64 timestamp_ns = 0;
	ErrorCode mdfErrsts = eOK;
	char InputLogName[1024] = { 0 };
	UINT8 Bussource = 0x07;
	INT32 busspecid = 0;
	INT64 ViGEMtimestamp_ns = 0;
	void * dataPtr = NULL;
	INT32 errCode = 0;
	unsigned32_T BusSource = 0;
	unsigned char UdpDataBytes[2000] = { 0 };
	unsigned char ByteBuffer[2000] = { 0 };
	INT64 relativetimestamp_t = 0;
	UINT8 AsynBit = 0;
	//UINT8 setViGEMMf4 = 1;
	void * DataPtr = ByteBuffer;
	CCA_EthProtocol_Info_T proRec = { 0 };
	strcpy(InputLogName, LogFname);
	PacketLossTrackerIntenalDecoder *pGdsrF360Tracker = new PacketLossTrackerIntenalDecoder();

	if (!g_mdfUDP) {
		g_mdfUDP = new apt_mdfFile("Default");
	}
	errCode = g_mdfUDP->getMf4_ToolID(LogFname, &setViGEMMf4, &setAuteraMf4);

	/*if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
	{

		if (CallPktOnce) {
			if (-1 == CreateOutputPacketlossReport(InputLogName)) {
				return -1;
			}
			CallPktOnce = false;
		}
	}*/

	do {
		if (MUDP_master_config->MUDPInput_Options == FOUR_DVSU_INPUT || MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT) {
			if (E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_open(LogFname, &mudp_log_in_ptr))
			{
				fprintf(stderr, "Error Opening input file: %s", LogFname);
				ret = -1;
				break;
			}
		}
		if (MUDP_master_config->data_extracter_mode[CCA_MF4_DUMP] == 1)
		{
			ccaVigemRecDump(LogFname, &dataPtr, &ViGEMtimestamp_ns);
			break;
		}
		else if (MUDP_master_config->MUDPInput_Options == VPCAP_VIGEM || MUDP_master_config->cca_Lib == 1 && setAuteraMf4 == 0)
		{
			printf("\nIf AUTERA log is used as the input please enable Vector_Library instead of CCA_Library in the xml\n");
			cca_Read_Rec(LogFname, &dataPtr, &ViGEMtimestamp_ns, sptr_manager, filecnt);//#HTMLInMUDP
			break;
		}
		else if (MUDP_master_config->MUDPInput_Options == MDF4_INPUT)
		{
			if (!g_mdfUDP) {
				g_mdfUDP = new apt_mdfFile("Default");
			}
			if (!g_mdfUDP->m_MdfLibValid_Flag)
			{
				printf("MF4 output files cannot be created, please verify whether correct MDF license & MDF lib file is available in the required path\n");
			}
			if (g_mdfUDP->m_MdfLibValid_Flag) {
				errCode = g_mdfUDP->getMf4_ToolID(LogFname, &setViGEMMf4, &setAuteraMf4);
				if (errCode == eUnfinalizedMdf)
				{
					printf("\n[ERROR]: The input MF4 file is un-finalized. Hence can not be processed. <%s>", LogFname);
					printFileStatusNoUDPData(InputLogName);
					return -1;
				}
				else if (errCode == eFatalFormatError || errCode != eOK)
				{
					printf("\n [ERROR]: Input MF4 is Unknown Format or Not Valid MF4 File. Hence Cannot be processed. <%s>", LogFname);
					printFileStatusNoUDPData(InputLogName);
					return -1;
				}

				if (setViGEMMf4) {
					/*if(!g_Cca_DbgViGEM_file){
					g_Cca_DbgViGEM_file = new cca_ViGEM;
					}*/
					if (setAuteraMf4 == 1)
					{
						printf("\nCCA library is not support for Autera logs, So Dumping using MDF library\n");
					}
					if (MUDP_master_config->vector_Lib == 1 || setAuteraMf4 == 1) {
						errCode = g_mdfUDP->OpenVIGEMmdfFile(LogFname, (INT64*)(&timestamp_ns), false);

						if (errCode == eUnfinalizedMdf)
						{
							printf("\n[ERROR]: The input MF4 file is un-finalized. Hence can not be processed. <%s>", LogFname);
							return -1;
						}
						if (errCode != eOK) {
							printf("\n[ERROR]: File has no UDP data <%s>.\n", LogFname);
							printFileStatusNoUDPData(InputLogName);
							return - 1;
							//return eOK;
							//return eGeneralError;
						}

					}
					/*if (g_Cca_DbgViGEM_file) {
					g_Cca_DbgViGEM_file->ReadMDfFile(LogFname);
					}*/
					if (MUDP_master_config->cca_Lib == 1 && setAuteraMf4 == 0) {
						cca_Read_Rec(LogFname, &dataPtr, &ViGEMtimestamp_ns, sptr_manager, filecnt);//#HTMLInMUDP
						break;
					}
				}
				else {
					errCode = g_mdfUDP->OpenUDPmdfFile(LogFname, false);
					if (errCode == eUnfinalizedMdf)
					{
						printf("\n[ERROR]: The input MF4 file is un-finalized. Hence can not be processed. <%s>", LogFname);
						return -1;
					}
					if (errCode != eOK) {
						printf("\n[ERROR]: File has no UDP data <%s>.\n", LogFname);
						printFileStatusNoUDPData(InputLogName);
						return -1;
					}
				}
				g_mdfUDP->rewindUDP_MDF();
			}

			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
			{

				if (CallPktOnce) {
					if (-1 == CreateOutputPacketlossReport(InputLogName)) {
						return -1;
					}
					CallPktOnce = false;
				}
			}

			while (1)
			{
				memset(&record, 0, sizeof(record));
				//timestamp_ns = 0;
				int ret = -1;
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[XML] == 1 || MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
				{
					memset(&record, 0, sizeof(record));
					int ret = -1;

					if (setViGEMMf4) {
						memset(&proRec, 0x00, sizeof(CCA_EthProtocol_Info_T));
						memset(ByteBuffer, 0x0, sizeof(ByteBuffer));
						mdfErrsts = g_mdfUDP->readMDFrec(&DataPtr, &proRec);
						relativetimestamp_t = proRec.rel_timeStamp_ns;
						obsoluteTimestamp_t = proRec.abs_timeStamp_ns;
						if (mdfErrsts != eOK) {
							if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode && (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1))
							{
								if (mdfErrsts == eReachedEndOfData || mdfErrsts == eErrorCodeUnspec)
								{

									for (int i = 0; i < MAX_RADAR_COUNT; i++) {
										Enddataflag[i] = true;
									}
								}
							}
							break;

						}
						else
						{
							if (DataPtr == nullptr) // continue to read next record when returned with null pointer.
							{
								continue;
							}
							if (proRec.bus_SpecId == 0)
							{
								continue;
							}
							relativetimestamp_t = proRec.rel_timeStamp_ns;
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
							{
								if (proRec.srcBusType == eBusEthernet)
								{
									AsynBit = proRec.Async_bit;
								}
								else
									if (proRec.srcBusType == eBusCAN)
									{
										AsynBit = proRec.Async_bit;
									}
							}
							if (proRec.srcBusType == eBusEthernet) {						
								if (DataPtr != NULL)
								{
									memcpy(&UdpDataBytes, DataPtr, proRec.dataByteCount);
									if (proRec.bus_SpecId == 48)
									{
										if(UdpDataBytes[32] == 7)
										  UdpDataBytes[32] = 8;
									}
									//memcpy(&record, &UdpDataBytes, UDP_MAX_PAYLOAD);
									if (proRec.ether_Type == IPV4_ETHER_TYPE) {
										if (UdpDataBytes[0] == IPV4_VER_LENGTH) {
											//memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											if ((UdpDataBytes[28] == 0xA5 && UdpDataBytes[29] == 0x1c) || (UdpDataBytes[28] == 0x1c && UdpDataBytes[29] == 0xA5) && strcmp(MUDP_master_config->GenType, "GEN7") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA4 && UdpDataBytes[29] == 0x2c) || (UdpDataBytes[28] == 0x2c && UdpDataBytes[29] == 0xA4) && strcmp(MUDP_master_config->GenType, "GEN6") == 0)
											{
												if (UdpDataBytes[32] == 5 && UdpDataBytes[31] == 84) //Ignoring RadarPos = 5 and Cust ID as STLA_SCALE1/SRR6P As, Radar Pos 5(RC) is not Supported for STLA in RESIM (CYW-4899)
												{
													continue;
												}
												else
												{
													memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
												}
											}
											else if ((UdpDataBytes[28] == 0xA3 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA3) && strcmp(MUDP_master_config->GenType, "GEN5") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA2 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA2))
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											// memcpy(&UdpDataBytes[0],&UdpDataBytes[28],UDP_MAX_PAYLOAD);
										}
									}
									else if (proRec.ether_Type == VLAN_ETHER_TYPE) {
										if (UdpDataBytes[4] == IPV4_VER_LENGTH) {
											//memcpy(&record.payload[0], &UdpDataBytes[32], UDP_MAX_PAYLOAD);
											if ((UdpDataBytes[28] == 0xA5 && UdpDataBytes[29] == 0x1c) || (UdpDataBytes[28] == 0x1c && UdpDataBytes[29] == 0xA5) && strcmp(MUDP_master_config->GenType, "GEN7") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA4 && UdpDataBytes[29] == 0x2c) || (UdpDataBytes[28] == 0x2c && UdpDataBytes[29] == 0xA4) && strcmp(MUDP_master_config->GenType, "GEN6") == 0)
											{
												if (UdpDataBytes[32] == 5 && UdpDataBytes[31] == 84) //Ignoring RadarPos = 5 and Cust ID as STLA_SCALE1/SRR6P As, Radar Pos 5(RC) is not Supported for STLA in RESIM (CYW-4899)
												{
													continue;
												}
												else
												{
													memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
												}
											}
											else if ((UdpDataBytes[28] == 0xA3 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA3) && strcmp(MUDP_master_config->GenType, "GEN5") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[32], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA2 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA2))
											{
												memcpy(&record.payload[0], &UdpDataBytes[32], UDP_MAX_PAYLOAD);
											}
										}

										if (UdpDataBytes[0] == IPV4_VER_LENGTH) { 
											if ((UdpDataBytes[28] == 0xA5 && UdpDataBytes[29] == 0x1c) || (UdpDataBytes[28] == 0x1c && UdpDataBytes[29] == 0xA5) && strcmp(MUDP_master_config->GenType, "GEN7") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA4 && UdpDataBytes[29] == 0x2c) || (UdpDataBytes[28] == 0x2c && UdpDataBytes[29] == 0xA4) && strcmp(MUDP_master_config->GenType, "GEN6") == 0)
											{
												if (UdpDataBytes[32] == 5 && UdpDataBytes[31] == 84) //Ignoring RadarPos = 5 and Cust ID as STLA_SCALE1/SRR6P As, Radar Pos 5(RC) is not Supported for STLA in RESIM (CYW-4899)
												{
													continue;
												}
												else
												{
													memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
												}
											}
											else if((UdpDataBytes[28] == 0xA3 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA3) && strcmp(MUDP_master_config->GenType, "GEN5") == 0)
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											else if ((UdpDataBytes[28] == 0xA2 && UdpDataBytes[29] == 0x18) || (UdpDataBytes[28] == 0x18 && UdpDataBytes[29] == 0xA2))
											{
												memcpy(&record.payload[0], &UdpDataBytes[28], UDP_MAX_PAYLOAD);
											}
											// memcpy(&UdpDataBytes[0],&UdpDataBytes[28],UDP_MAX_PAYLOAD);
										}
									}
									else
									{
										if ((UdpDataBytes[0] == 0xA3 && UdpDataBytes[1] == 0x18) || (UdpDataBytes[0] == 0x18 && UdpDataBytes[1] == 0xA3))
										{
											memcpy(&record.payload[0], &UdpDataBytes[0], UDP_MAX_PAYLOAD);
										}
										if ((UdpDataBytes[0] == 0xA2 && UdpDataBytes[1] == 0x18) || (UdpDataBytes[0] == 0x18 && UdpDataBytes[1] == 0xA2))
										{
											memcpy(&record.payload[0], &UdpDataBytes[0], UDP_MAX_PAYLOAD);
										}
									}


								}
								if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
								{
									BusSource = MaptoBusid(proRec.bus_SpecId);
									gettimeStampSyncKPI(BusSource, InputLogName, relativetimestamp_t, AsynBit);
								}
							}
							else if (proRec.srcBusType == eBusCAN) {
								if (DataPtr != NULL)
								{
									if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
									{
										BusSource = MaptoBusid(proRec.bus_SpecId);
										gettimeStampSyncKPI(BusSource, InputLogName, relativetimestamp_t, AsynBit);
									}

								}
								continue;
							}

							else {
								continue;
							}

						}
					}
					else {
						mdfErrsts = g_mdfUDP->read_MDF(&record, &timestamp_ns); 

						if (strcmp(MUDP_master_config->GenType, "GEN7") == 0 && MUDP_master_config->Radar_Stream_Options[HDR] != 1) //Sorting Data for other than HDR Dump
						{
							mdfErrsts = Sorting_Data(record, mdfErrsts, FALSE /*, &Sorted_udpRec*/);	//Send TRUE -> to Print Chunks Info Before Sorting and After Sorting -> for DEBUGGING
						}

						{
							relativetimestamp_t = record.pcTime;
							obsoluteTimestamp_t = timestamp_ns;					
							/*--------------------------------------------------
							uint16_t *udpByteSize = (uint16_t *)(&record.payload[28]);
							*udpByteSize -= 44;
							if (record.payload[22] == e_RDR_VSE_CORE_LOGGING_STREAM)
							{
								*udpByteSize = 88; record.payload[26] = 0;
							}
							if (record.payload[22] == e_RDR_STATUS_CORE_LOGGING_STREAM)
							{
								*udpByteSize = 276; record.payload[26] = 0;
							}
							if (record.payload[22] == e_RDR_HDR_LOGGING_STREAM)
							{
								*udpByteSize = 116; record.payload[26] = 0;
							}
							const GEN5_UDP_FRAME_STRUCTURE_T *p_udp_frame = (const GEN5_UDP_FRAME_STRUCTURE_T *)record.payload;
							//-------------------------------------------------*/
							if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
							{
								if (mdfErrsts == eReachedEndOfData || mdfErrsts == eErrorCodeUnspec)
								{
									for (int i = 0; i < MAX_RADAR_COUNT; i++) {
										Enddataflag[i] = true;
									}
								}
							}

							if (mdfErrsts != eOK) {
								if (mdfErrsts == eFileReadError)
									printf("\n Unable to read from Log file : <%s>\n", LogFname);
								break;
							}
							if (record.payload[0] == 0) // continue to read next record when returned with null pointer.
							{
								continue;
							}

						}
					}

				}

				if ((strstr(LogFname, ".MF4") != NULL) || (strstr(LogFname, ".mf4") != NULL))
				{

					if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5))
					{
						if (record.payload[8] == UDP_PLATFORM_SRR7_PLUS || record.payload[8] == UDP_PLATFORM_FLR7 || record.payload[8] == UDP_PLATFORM_SRR7_PLUS_HD || record.payload[8] == UDP_PLATFORM_SRR7_E)
						{
							sourceId = record.payload[27] - 1;
							cust_id = PLATFORM_GEN7; // (Customer_T)record.payload[3];
							Stream_No = GEN7MaptoStreamNumber(record.payload[19], cust_id);//(GEN5_Radar_Logging_Data_Source_T)record.payload[22];

							//GDSR TRACKER 
							/*if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								unsigned char gen5_radar_index = 0;
								Stream_No = GEN5MaptoStreamNumber(record.payload[22], cust_id);
								if (Stream_No == GEN5_SENSOR_GDSR_TRACKER_INTERNAL)
								{
									F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
									static int frame_loss_count = 0;
									memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

									gen5_radar_index = (m_Udp_Payload[0].frame[4] - 1);

									if (MUDP_master_config->MUDP_Select_Sensor_Status[gen5_radar_index] != 0)
										status = pGdsrF360Tracker->start_GEN5_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const&)m_Udp_Payload[0]);
								}
							}*/
						}
					}
					if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4))
					{
						if (record.payload[2] == UDP_PLATFORM_SRR6 || record.payload[2] == UDP_PLATFORM_SRR6_PLUS || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_CAN || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_ETH || record.payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || record.payload[2] == UDP_PLATFORM_FLR4_PLUS || record.payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE || record.payload[2] == UDP_PLATFORM_STLA_FLR4)
						{
							sourceId = record.payload[4] - 1;
							cust_id = (Customer_T)record.payload[3];
							Stream_No = GEN5MaptoStreamNumber(record.payload[22], cust_id);//(GEN5_Radar_Logging_Data_Source_T)record.payload[22];
							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								unsigned char gen5_radar_index = 0;
								Stream_No = GEN5MaptoStreamNumber(record.payload[22], cust_id);
								if (Stream_No == GEN5_SENSOR_GDSR_TRACKER_INTERNAL)
								{
									F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
									static int frame_loss_count = 0;
									memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

									gen5_radar_index = (m_Udp_Payload[0].frame[4] - 1);

									if (MUDP_master_config->MUDP_Select_Sensor_Status[gen5_radar_index] != 0)
										status = pGdsrF360Tracker->start_GEN5_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);
								}
							}
						}
					}
					else if((record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3))
					{
						if ((record.payload[8] == UDP_PLATFORM_SRR5 || record.payload[8] == UDP_PLATFORM_MCIP_SRR5))// && record.payload[8] != UDP_SOURCE_CUST_DSPACE) // Checking is the platform is other than SRR3
						{
							sourceId = (record.payload[9] - 1);
							F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
							unsigned char radar_index = RDR_POS_ECU;
							cust_id = (Customer_T)record.payload[23];

							if (sourceId == RDR_POS_ECU)
							{
								Stream_No = MaptoEcustreamNumber(record.payload[19]);
							}
							else
							{
								Stream_No = MaptostreamNumber(record.payload[19], cust_id);
							}

							if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
							{
								if (sourceId == RDR_POS_ECU)
								{
									//Stream_No = MaptoEcustreamNumber(record.payload[19]);
									if (Stream_No == RADAR_ECU_INTERNAL_CORE1) {
										F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
										static int frame_loss_count = 0;
										memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);
										if (MUDP_master_config->MUDP_Radar_ECU_status == 1)
											status = pGdsrF360Tracker->start_F360TrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);

									}
								}
								else
								{
									unsigned char radar_index = 0;
									Stream_No = MaptostreamNumber(record.payload[19], cust_id);
									if (Stream_No == SENSOR_GDSR_TRACKER_INTERNAL) {
										F360_GDSR_status_e status = DPH_F360_GDSR_FRAME_ERROR;
										static int frame_loss_count = 0;
										memcpy(&m_Udp_Payload[0].frame[0], &record.payload[0], UDP_FRAME_PAYLOAD_SIZE);

										radar_index = (m_Udp_Payload[0].frame[9] - 1);

										if (checkMidHighSensorsEnabled() == true && (MUDP_master_config->MUDP_Radar_ECU_status == 1 || MUDP_master_config->MUDP_Radar_ECU_status == 0) && MUDP_master_config->Bpillar_status == 1) { // HIGH varaint
											if (radar_index != BPIL_POS_L && radar_index != BPIL_POS_R) {
												status = DPH_F360_GDSR_FRAME_UNAVAILABLE;
											}
											else {
												if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_index] != 0)
													status = pGdsrF360Tracker->start_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);
											}
										}
										else {
											if (MUDP_master_config->MUDP_Select_Sensor_Status[radar_index] != 0)
												status = pGdsrF360Tracker->start_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]); // LOW varaint
										}
									}
									/*if((MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] != 0) || (MUDP_master_config->Bpillar_status != 0)){
									status = pGdsrF360Tracker->start_GDSRTrackerDecode((UDP_FRAME_PAYLOAD_T const &)m_Udp_Payload[0]);

									}*/
								}
							}

						}
						
						else if (record.payload[8] == UDP_SOURCE_CUST_DSPACE)
						{
							sourceId = RDR_POS_DSPACE;
							cust_id = (Customer_T)record.payload[23];
							
							if (sourceId == RDR_POS_DSPACE)
							{
								Stream_No = MaptostreamNumber(record.payload[19], cust_id);
							}
						}
					}

					else if((record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
					{
						if (record.payload[8] >= UDP_PLATFORM_CUST_SRR3_RL && record.payload[8] <= UDP_PLATFORM_CUST_SRR3_FC) {
							sourceId = (uint8_t)(record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL);
						}
						// For DSPACE UDP logs, radar position is assigned as RDR_POS_DSPACE
						else if (record.payload[8] == UDP_SOURCE_CUST_DSPACE) {
							sourceId = RDR_POS_DSPACE;
						}
						else if (record.payload[8] == BPILLAR_STATUS_BP_L || record.payload[8] == UDP_PLATFORM_FORD_SRR5) {
							sourceId = RDR_POS_FC;
						}
						cust_id = (Customer_T)record.payload[23];
						if (sourceId == 19)
						{
							Stream_No = MaptoEcustreamNumber(record.payload[19]);
						}
						else
						{
							Stream_No = MaptostreamNumber(record.payload[19], cust_id);
						}
					}

					
					if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1C) || (record.payload[0] == 0x1C && record.payload[1] == 0xA5)) //GEN7 Print Stream Version Info on Console
					{
						gen7_Print_StrmVrsn_Info(record, Stream_No);
						//if ((Customer_T)record.payload[3] != 0)
						//{
						//	gen7_Print_StrmVrsn_Info(record, Stream_No);
						//}
					}
					else if ((record.payload[0] == 0xA4 && record.payload[1] == 0x2C) || (record.payload[0] == 0x2C && record.payload[1] == 0xA4)) //GEN6 Print Stream Version Info on Console
					{
						if ((Customer_T)record.payload[3] != INVALID_CUSTOMER)
						{
							gen5_Print_StrmVrsn_Info(record, Stream_No);
						}
					}
					if ((record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3)
						|| (record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
					{
						if ((Customer_T)record.payload[23] != INVALID_CUSTOMER)
						{
							Print_StrmVrsn_Info(record, Stream_No);
						}
					}
					/*else if ((Customer_T)record.payload[23] != INVALID_CUSTOMER)
					{
						Print_StrmVrsn_Info(record, Stream_No);
					}*/
					if (MUDP_master_config->data_extracter_mode[XML] == 1)
					{
						if (CreatxmlOnce)
						{
							if (-1 == CreateOutputSensorXMLFile(InputLogName, cust_id)) {
								return -1;
							}
							CreatxmlOnce = false;
						}
					}

					if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
					{
						if (CreateOnce) {
							if ((record.payload[0] == 0xA5 && record.payload[1] == 0x1c) || (record.payload[0] == 0x1c && record.payload[1] == 0xA5)
								|| (record.payload[0] == 0xA4 && record.payload[1] == 0x2c) || (record.payload[0] == 0x2c && record.payload[1] == 0xA4)
								|| (record.payload[0] == 0xA3 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA3)
								|| (record.payload[0] == 0xA2 && record.payload[1] == 0x18) || (record.payload[0] == 0x18 && record.payload[1] == 0xA2))
							{
								if (-1 == CreateOutputCSVFile(InputLogName, record)) {
									return -1;
								}
								CreateOnce = false;
							}
						}
					}
					
					if ((MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) || ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
					{
						if (((MUDP_master_config->data_extracter_mode[CSV] == 0) && (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 0) && (MUDP_master_config->data_extracter_mode[XML] == 0)) && ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
						{
							if (fCreate && (Stream_No != SENSOR_GDSR_TRACKER_INTERNAL) && (Stream_No != RADAR_ECU_INTERNAL_CORE1) && (Stream_No != GEN5_SENSOR_GDSR_TRACKER_INTERNAL)) {
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, NULL, NULL, relativetimestamp_t, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;//#HTMLInMUDP
									break;
								}
							}
						}
						else
						{
							if (!(f_dspace_enabled))
							{
								if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && sourceId == RADAR_POS))
								{

                                      //#HTMLInMUDP
									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, relativetimestamp_t, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;
										break;

									}

								}
							}
							else
							{
								if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0) && (MUDP_master_config->packet_loss_statistics == 0) && (MUDP_master_config->data_extracter_mode[Quality_Check] == 0))
								{    //#HTMLInMUDP
									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, relativetimestamp_t, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;
										break;

									}
								}
								else
								{
									printf("\n[WARNING]: Do not enable packetloss report attributes in the config\n ");
									ret = -1;
									break;
								}

							}

						}

					}

				}
			}
		}
		else if (strstr(LogFname, ".dvsu") != NULL)
		{
			while (1)
			{
				memset(&record, 0, sizeof(record));
				if (E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_read(mudp_log_in_ptr, (unsigned char*)&record, NULL))
					break;
				UINT64 timestamp_ns = record.pcTime;
				if (MUDP_master_config->MUDPInput_Options == SINGLE_DVSU_INPUT) {

					if (record.payload[8] >= UDP_PLATFORM_CUST_SRR3_RL && record.payload[8] <= UDP_PLATFORM_CUST_SRR3_FC) {
						sourceId = (uint8_t)(record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL);
					}
					// For DSPACE UDP logs, radar position is assigned as RDR_POS_DSPACE
					else if (record.payload[8] == UDP_SOURCE_CUST_DSPACE) {
						sourceId = RDR_POS_DSPACE;
					}
					else if (record.payload[8] == BPILLAR_STATUS_BP_L || record.payload[8] == UDP_PLATFORM_FORD_SRR5) {
						sourceId = RDR_POS_FC;
					}
					// For SRR5 sensors including radar ECU, radar position is derived from the Radar_Pos of UDP Header: 1, 2, 3, 4 and for ECU 20
					else {
						sourceId = (uint8_t)(record.payload[9] - 1);
					}
					cust_id = (Customer_T)record.payload[23];
					if (sourceId == 19)
					{
						Stream_No = MaptoEcustreamNumber(record.payload[19]);
					}
					else
					{
						Stream_No = MaptostreamNumber(record.payload[19], cust_id);
					}
					if ((Customer_T)record.payload[23] != 0)
					{
						Print_StrmVrsn_Info(record, Stream_No);
					}

					if (MUDP_master_config->data_extracter_mode[XML] == 1)
					{
						if (CreatxmlOnce)
						{
							if (-1 == CreateOutputSensorXMLFile(InputLogName, cust_id)) {
								return -1;
							}
							CreatxmlOnce = false;
						}
					}

					if (CreateOnce) {
						if (-1 == CreateOutputCSVFile(InputLogName, record)) {
							return -1;
						}
						CreateOnce = false;
					}

					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
					{

						if (CallPktOnce) {
							if (-1 == CreateOutputPacketlossReport(InputLogName)) {
								return -1;
							}
							CallPktOnce = false;
						}
					}

					if ((MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) || ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1)))
					{
						if (((MUDP_master_config->data_extracter_mode[CSV] == 0) && (MUDP_master_config->data_extracter_mode[XML] == 0)) && ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
						{
							if (fCreate && (Stream_No != SENSOR_GDSR_TRACKER_INTERNAL) && (Stream_No != RADAR_ECU_INTERNAL_CORE1)) {
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, NULL, NULL, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;//#HTMLInMUDP
									break;
								}
							}
						}
						else
						{
							if (!(f_dspace_enabled))
							{
								if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && sourceId == RADAR_POS))
								{

									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;//#HTMLInMUDP
										break;

									}

								}
							}
							else
							{
								if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0) && (MUDP_master_config->packet_loss_statistics == 0) && (MUDP_master_config->data_extracter_mode[Quality_Check] == 0))
								{
									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;//#HTMLInMUDP
										break;

									}
								}
								else
								{
									printf("\n[WARNING]: Do not enable packetloss report attributes in the config\n ");
									ret = -1;
									break;
								}

							}

						}

					}

				}
				else
				{
					if (record.payload[8] >= UDP_PLATFORM_CUST_SRR3_RL && record.payload[8] <= UDP_PLATFORM_CUST_SRR3_FC) {
						sourceId = (uint8_t)(record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL);
					}
					// For DSPACE UDP logs, radar position is assigned as RDR_POS_DSPACE
					else if (record.payload[8] == UDP_SOURCE_CUST_DSPACE) {
						sourceId = RDR_POS_DSPACE;
					}
					else if (record.payload[8] == BPILLAR_STATUS_BP_L || record.payload[8] == UDP_PLATFORM_FORD_SRR5) {
						sourceId = RDR_POS_FC;
					}
					// For SRR5 sensors including radar ECU, radar position is derived from the Radar_Pos of UDP Header: 1, 2, 3, 4 and for ECU 20
					else {
						sourceId = (uint8_t)(record.payload[9] - 1);
					}
					cust_id = (Customer_T)record.payload[23];
					if (sourceId == 19)
					{
						Stream_No = MaptoEcustreamNumber(record.payload[19]);
					}
					else
					{
						Stream_No = MaptostreamNumber(record.payload[19], cust_id);
					}


					if ((Customer_T)record.payload[23] != 0)
					{
						Print_StrmVrsn_Info(record, Stream_No);
					}

					if (MUDP_master_config->data_extracter_mode[XML] == 1)
					{
						if (CreatxmlOnce)
						{
							if (-1 == CreateOutputSensorXMLFile(InputLogName, cust_id)) {
								return -1;
							}
							CreatxmlOnce = false;
						}
					}
					if (CreateOnce) {
						if (-1 == CreateOutputCSVFile(InputLogName, record)) {
							return -1;
						}
						CreateOnce = false;
					}
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
					{

						if (CallPktOnce) {
							if (-1 == CreateOutputPacketlossReport(InputLogName)) {
								return -1;
							}
							CallPktOnce = false;
						}
					}

					if ((MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) || ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1)))
					{
						if (((MUDP_master_config->data_extracter_mode[CSV] == 0) && (MUDP_master_config->data_extracter_mode[XML] == 0)) && ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1) || (MUDP_master_config->packet_loss_statistics == 1) || (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)))
						{
							if (fCreate && (Stream_No != SENSOR_GDSR_TRACKER_INTERNAL) && (Stream_No != RADAR_ECU_INTERNAL_CORE1)) {
								if (-1 == ExtractAndListDVSU(Streamver, InputLogName, NULL, NULL, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
									ret = -1;//#HTMLInMUDP
									break;
								}
							}
						}
						else
						{
							if (!(f_dspace_enabled))
							{
								if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1 || (MUDP_master_config->MUDP_Radar_ECU_status == 1 && sourceId == RADAR_POS))
								{

									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;//#HTMLInMUDP
										break;

									}

								}
							}
							else
							{
								if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0) && (MUDP_master_config->packet_loss_statistics == 0) && (MUDP_master_config->data_extracter_mode[Quality_Check] == 0))
								{
									if (-1 == ExtractAndListDVSU(Streamver, InputLogName, fpxml, hxml, timestamp_ns, dump_option, &record, true, sptr_manager, filecnt)) {
										ret = -1;//#HTMLInMUDP
										break;

									}
								}
								else
								{
									printf("\n[WARNING]: Do not enable packetloss report attributes in the config\n ");
									ret = -1;
									break;
								}

							}

						}

					}
				}
			}
		}
	} while (0);


	if ((MUDP_master_config->data_extracter_mode[Quality_Check] == 1))
	{
		RadarDataCheck(cust_id);
	}

	if (mudp_log_in_ptr) {
		m_pLogPlugin->SRR3_mudp_close(&mudp_log_in_ptr);
	}
	if (fpDump != NULL)
		fclose(fpDump);
	if (g_mdfUDP) {
		delete g_mdfUDP;
		g_mdfUDP = NULL;
	}
	if (pGdsrF360Tracker) {
		delete pGdsrF360Tracker;
		pGdsrF360Tracker = nullptr;
	}
	return ret;

}
void MUDPCopyright()
{
	printf("\r\t%%                                                              %%\n");
	printf("\r\t%%   APTIV CORPORATION PROPRIETARY,  Copyright    2017 - 2023   %%\n");
	printf("\r\t%%                                                              %%\n");
}
// int Dump(const char* LogFname, const char* ouputfname)
// {
// int ret = 0;
// char sourceId = 0;
// void*  mudp_log_in = NULL; //single log. when working for 4 sensor, use this as the base name and load all the four sensor logs
// DVSU_RECORD_T record = {0};
// unsigned int iMudpStrNo = 0;
// UINT64 timestamp_ns = 0;
// UDPLogStruct* pLogStruct = NULL;
// char * FileName[4];
// fpDump = NULL;
// bool Fourdvsu_check = false;
// //CheckDVSU(LogFname) ;
// if(dump_option == UDP_CONVERT)
// {
// m_version_info.strm_ver.z7b = 33;
// m_version_info.strm_ver.z7a = 21;
// m_version_info.strm_ver.z4 = 20;
// m_version_info.strm_ver.z4_cust = 4;
// m_version_info.strm_ver.z7b_cust = 8;
// m_version_info.strm_ver.z7b_tracker = 7;
// if(-1 == CreateOutputDVSUFile(LogFname))
// {
// ret = -1;
// }
// }

// do{
// if(dump_option == UDP_ECU2)
// {
// fprintf(stderr,"Error Creating Output file: %s",ouputfname);
// return -1;
// }
// if(dump_option != UDP_CONVERT)
// {
// if(strstr(LogFname,".dvsu") != NULL){
// if(Fourdvsu_check){
// fpDump = fopen(ouputfname,"w");
// if(!fpDump)
// {
// fprintf(stderr,"Error Creating Output file: %s",ouputfname);
// ret = -1;
// break;
// }
// }
// if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_open(LogFname,&mudp_log_in))
// {
// fprintf(stderr,"Error Opening input file: %s",LogFname);
// ret = -1;
// break;
// }
// }
// if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) || Fourdvsu_check == false){
// if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3))
// {
// char drvName[_MAX_DRIVE];
// char dir[_MAX_DIR];
// char fName[_MAX_FNAME];
// char ext[_MAX_EXT];

// #if defined(_WIN_PLATFORM)
// split_path(ouputfname, drvName, dir, fName, ext); 
// #elif defined(__GNUC__)
// split_pathLinux(ouputfname, &drvName, &dir, &fName, &ext);
// #endif
// sprintf(FileNameRL, "%s%s%s_71.csv", drvName, dir, fName);
// sprintf(FileNameRR, "%s%s%s_72.csv", drvName, dir, fName);
// sprintf(FileNameFR, "%s%s%s_73.csv", drvName, dir, fName);
// sprintf(FileNameFL, "%s%s%s_74.csv", drvName, dir, fName); 
// FileName[0] = FileNameRL ;
// FileName[1] = FileNameRR ;
// FileName[2] = FileNameFR ;
// FileName[3] = FileNameFL ;

// /*for(int i=0;i<4;i++)
// {
// fpMF4Dump[i] = fopen(FileName[i],"w");
// if(!fpMF4Dump[i])
// {
// fprintf(stderr,"Error Creating Output file: %s",FileName[i]);
// ret = -1;
// break;
// }
// }*/
// }
// else
// {
// fpDump = fopen(ouputfname,"w");
// if(!fpDump)
// {
// fprintf(stderr,"Error Creating Output file: %s",ouputfname);
// ret = -1;
// break;
// }
// }
// }   
// if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) ){
// if(!g_mdfUDP){
// g_mdfUDP = new apt_mdfFile;
// }
// if(!g_mdfUDP->m_MdfLibValid_Flag)
// {
// printf("MF4 output files cannot be created, please verify whether correct MDF license & MDF lib file is available in the required path\n");
// }
// if(g_mdfUDP->m_MdfLibValid_Flag){
// if(eOK != g_mdfUDP->OpenUDPmdfFile(LogFname, false)){
// printf("[ERROR]: File has no UDP data <%s>.\n", LogFname);
// return -1;
// }
// while(1)
// {
// memset(&record, 0, sizeof(record));
// timestamp_ns = 0;
// if(eOK != g_mdfUDP->read_MDF(&record, &timestamp_ns)){
// break;
// }
// if((strstr(LogFname,".MF4") != NULL) || (strstr(LogFname,".mf4") != NULL) )
// { 
// if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3)){
// if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
// sourceId = (record.payload[9] - 1);
// }
// else{
// sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
// } 
// /*if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId], dump_option, &record,true)){
// ret = -1;
// break;
// }*/
// }
// /*else            
// {
// if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
// ret = -1;
// break;
// }
// }*/
// }
// }
// }
// }
// }
// if(strstr(LogFname,".mrr2") != NULL) //Buffer for Stream Data
// {
// while(1)
// {
// if ( E_MUDPLOG_OK  != m_pLogPlugin->SRR3_mudp_read(mudp_log_in,(unsigned char*)&g_RacamUdpBuffer[0],&iMudpStrNo))
// break;


// if(((iMudpStrNo >=0 )&&(iMudpStrNo < RACAMUDP_MAX_ST))/* && (MUDP_master_config->Debug_Extracter_Data == 1)*/)
// {
// if( -1 == ExtractAndListMUDP(fpDump,dump_option,(UDPLogStruct*) &g_RacamUdpBuffer[0])){
// ret = -1;
// break;
// }
// }
// }
// }
// else if(strstr(LogFname,".dvsu") != NULL)
// {
// while(1)
// {
// memset(&record,0,sizeof(record));
// if ( E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_read(mudp_log_in,(unsigned char*)&record,NULL))
// break;
// if(dump_option == UDP_CONVERT)
// {
// if(-1 == ExtractAndConvert(LogFname, &record)){
// ret = -1;
// break;
// }
// DumpToOutputDVSU();

// }
// else
// {
// if(Fourdvsu_check == false){
// if((dump_option != UDP_HDR) && (dump_option !=UDP_ECU0) && (dump_option !=UDP_ECU1) && (dump_option !=UDP_ECU3)){
// if(record.payload[8] >= UDP_PLATFORM_SRR5){ // Checking is the platform is other than SRR3
// sourceId = (record.payload[9] - 1);
// }
// else{
// sourceId = record.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
// } 
// /*if( -1 == ExtractAndListDVSU(fpMF4Dump[sourceId], dump_option, &record,true)){
// ret = -1;
// break;
// }*/
// }
// /*else            
// {
// if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
// ret = -1;
// break;
// }
// }*/
// }
// /*else            
// {
// if( -1 == ExtractAndListDVSU(fpDump, dump_option, &record,false)){
// ret = -1;
// break;
// }
// }*/
// }
// }
// /*if( -1 == ExtractAndListDVSU(fpDump,dump_option, &record,false)){
// ret = -1;
// break;
// }  */         
// }
// }while(0);

// /*if(g_pIRadarStrm)
// {
// delete g_pIRadarStrm;
// g_pIRadarStrm = NULL;
// }*/
// if(mudp_log_in){
// m_pLogPlugin->SRR3_mudp_close(&mudp_log_in);
// }

// if(fpDump != NULL)
// fclose(fpDump);

// return ret;
// }

srr3_api_status_e  GEN7_GetPreviousData(GEN7_UDPRecord_Header_T* p_gen7_udp_hdr, const char* LogFname, Customer_T custId)
{
	char sourceId = 0;
	unsigned int Stream_No = 0;
	Stream_No = GEN7MaptoStreamNumber(p_gen7_udp_hdr->streamNumber, custId);
	sourceId = p_gen7_udp_hdr->sensorId - 1;

	srr3_api_status_e status = Status_Latch_Error;
	static int processedchunk[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (!f_firstdata[sourceId]) {
		memcpy(&gen7_prev_udp_frame_hdr[sourceId], p_gen7_udp_hdr, sizeof(GEN7_UDPRecord_Header_T));
		f_firstdata[sourceId] = true;
	}

	if (gen7_prev_udp_frame_hdr[sourceId].streamNumber == p_gen7_udp_hdr->streamNumber)
	{
		if (gen7_prev_udp_frame_hdr[sourceId].streamRefIndex == p_gen7_udp_hdr->streamRefIndex)
		{
			if (p_gen7_udp_hdr->streamChunkIdx > 0)
			{
				processedchunk[sourceId][Stream_No]++;
			}
		}
	}
	if (Enddataflag[sourceId] && MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode)
	{
		if (Stream_No != CDC_DATA)
		{
			if ((processedchunk[sourceId][Stream_No] + 1) == p_gen7_udp_hdr->streamChunks)
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
			else
			{
				status = Status_Latch_InProcess;
			}

			if (processedchunk[sourceId][Stream_No] != p_gen7_udp_hdr->streamChunkIdx)
			{
				int offset = abs(processedchunk[sourceId][Stream_No] - p_gen7_udp_hdr->streamChunkIdx);
				UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
				//	TotalscanIndexDropCount_udp[sourceId]++;
				//	ScanindexDropCount[sourceId][Stream_No]++;
				processedchunk[sourceId][Stream_No] = 0;
			}
			else
			{
				status = Status_Latch_Error;
			}
		}
		if (Stream_No == CDC_DATA)
		{
			if (CDC_ScanIdCurrent[sourceId][CDC_DATA] == CDC_scan_idx_previous[sourceId][CDC_DATA] + 1)
			{
				if (gen7_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks != p_gen7_udp_hdr->streamChunks)
				{
					UDPPacketLossErrorCount[sourceId][CDC_DATA]++;
					ScanindexDropCount[sourceId][CDC_DATA]++;
					TotalscanIndexDropCount_cdc[sourceId]++;
				}
			}
			else
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
		}

		int diffRefIdx = (int)(p_gen7_udp_hdr->streamRefIndex - gen7_prev_udp_frame_hdr[sourceId].streamRefIndex);
		if (diffRefIdx >= 2)
		{
			int Diff = (p_gen7_udp_hdr->streamRefIndex - gen7_prev_udp_frame_hdr[sourceId].streamRefIndex);
			if (Diff >= 7)
			{
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					fprintf(fCreate, " The given file (%s)is not continuous!!\n", LogFname);
			}
			if (Stream_No != CDC_DATA)
			{
				ScanIdDropCount_udp[sourceId]++;
			}
			else
			{
				ScanIdDropCount_cdc[sourceId]++;
			}
		}
	}
	else
	{
		if ((gen7_prev_udp_frame_hdr[sourceId].streamNumber == p_gen7_udp_hdr->streamNumber) && (gen7_prev_udp_frame_hdr[sourceId].streamRefIndex == p_gen7_udp_hdr->streamRefIndex))
		{
			if (p_gen7_udp_hdr->streamChunkIdx + 1 == p_gen7_udp_hdr->streamChunks && p_gen7_udp_hdr->streamChunks != processedchunk[sourceId][Stream_No] + 1)
			{
				int offset = abs(processedchunk[sourceId][Stream_No] - p_gen7_udp_hdr->streamChunks);
				UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
				processedchunk[sourceId][Stream_No] = 0;
			}
			else
			{
				status = Status_Latch_Error;
			}
		}
	}

	if (Stream_No != CDC_DATA) {
		if (p_gen7_udp_hdr->streamChunks == (processedchunk[sourceId][Stream_No] + 1))
		{
			processedchunk[sourceId][Stream_No] = 0;
		}
	}
	else {
		if (!(p_gen7_udp_hdr->sourceInfo >= UDP_PLATFORM_SRR7_PLUS && p_gen7_udp_hdr->sourceInfo >= UDP_PLATFORM_SRR7_E))
		{
			if (gen7_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks == p_gen7_udp_hdr->streamChunks) {
				processedchunk[sourceId][Stream_No] = 0;
			}
		}
	}
	memcpy(&gen7_prev_udp_frame_hdr[sourceId], p_gen7_udp_hdr, sizeof(GEN7_UDPRecord_Header_T));
	//gen7_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks = p_udp_hdr->streamChunks;
	return status;
}

srr3_api_status_e  GEN5_GetPreviousData(GEN5_UDPRecord_Header_T* p_gen5_udp_hdr, const char* LogFname, Customer_T custId)
{
	char sourceId = 0;
	unsigned int Stream_No = 0;
	Stream_No = GEN5MaptoStreamNumber(p_gen5_udp_hdr->stream_Id, custId);
	sourceId = p_gen5_udp_hdr->Radar_Position-1;

	srr3_api_status_e status = Status_Latch_Error;
	static int processedchunk[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (!f_firstdata[sourceId]) {
		memcpy(&gen5_prev_udp_frame_hdr[sourceId], p_gen5_udp_hdr, sizeof(GEN5_UDPRecord_Header_T));
		f_firstdata[sourceId] = true;
	}

	if (gen5_prev_udp_frame_hdr[sourceId].stream_Id == p_gen5_udp_hdr->stream_Id)
	{
		if (gen5_prev_udp_frame_hdr[sourceId].scan_index == p_gen5_udp_hdr->scan_index)
		{
			if (p_gen5_udp_hdr->streamChunkIdx > 0)
			{
				processedchunk[sourceId][Stream_No]++;
			}
		}
	}
	if (Enddataflag[sourceId] && MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode)
	{
		if (Stream_No != CDC_DATA)
		{
			if ((processedchunk[sourceId][Stream_No] + 1) == p_gen5_udp_hdr->Total_num_Chunks)
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
			else
			{
				status = Status_Latch_InProcess;
			}

			if (processedchunk[sourceId][Stream_No] != p_gen5_udp_hdr->streamChunkIdx)
			{
				int offset = abs(processedchunk[sourceId][Stream_No] - p_gen5_udp_hdr->streamChunkIdx);
				UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
			//	TotalscanIndexDropCount_udp[sourceId]++;
			//	ScanindexDropCount[sourceId][Stream_No]++;
				processedchunk[sourceId][Stream_No] = 0;
			}
			else
			{
				status = Status_Latch_Error;
			}
		}
		if (Stream_No == CDC_DATA)
		{
			if (CDC_ScanIdCurrent[sourceId][CDC_DATA] == CDC_scan_idx_previous[sourceId][CDC_DATA] + 1 && CDC_ScanIdCurrent[sourceId][CDC_DATA] != 0 && CDC_scan_idx_previous[sourceId][CDC_DATA] != 0)
			{
				if (gen5_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks != p_gen5_udp_hdr->Total_num_Chunks)
				{
					UDPPacketLossErrorCount[sourceId][CDC_DATA]++;
					ScanindexDropCount[sourceId][CDC_DATA]++;
					TotalscanIndexDropCount_cdc[sourceId]++;
				}
			}
			else
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
		}

		int diffRefIdx = (int)(p_gen5_udp_hdr->scan_index - gen5_prev_udp_frame_hdr[sourceId].scan_index);
		if (diffRefIdx >= 2)
		{
			int Diff = (p_gen5_udp_hdr->scan_index - gen5_prev_udp_frame_hdr[sourceId].scan_index);
			if (Diff >= 5)
			{
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					fprintf(fCreate, " The given file (%s)is not continuous!!\n", LogFname);
			}
			if (Stream_No != CDC_DATA)
			{
				ScanIdDropCount_udp[sourceId]++;
			}
			else
			{
				ScanIdDropCount_cdc[sourceId]++;
			}
		}
	}
	else
	{
		if ((gen5_prev_udp_frame_hdr[sourceId].stream_Id == p_gen5_udp_hdr->stream_Id) && (gen5_prev_udp_frame_hdr[sourceId].scan_index == p_gen5_udp_hdr->scan_index))
		{
			if (p_gen5_udp_hdr->streamChunkIdx + 1 == p_gen5_udp_hdr->Total_num_Chunks && p_gen5_udp_hdr->Total_num_Chunks != processedchunk[sourceId][Stream_No] + 1)
			{
				int offset = abs(processedchunk[sourceId][Stream_No] - p_gen5_udp_hdr->Total_num_Chunks);
				UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
				processedchunk[sourceId][Stream_No] = 0;
			}
			else
			{
				status = Status_Latch_Error;
			}
		}
	}

	if (Stream_No != CDC_DATA) {
		if (p_gen5_udp_hdr->Total_num_Chunks == (processedchunk[sourceId][Stream_No] + 1))
		{
			processedchunk[sourceId][Stream_No] = 0;
		}
	}
	else {
		if (!(p_gen5_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_gen5_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_FC))
		{
			if (gen5_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks == p_gen5_udp_hdr->Total_num_Chunks) {
				processedchunk[sourceId][Stream_No] = 0;
			}
		}
	}
	memcpy(&gen5_prev_udp_frame_hdr[sourceId], p_gen5_udp_hdr, sizeof(GEN5_UDPRecord_Header_T));
	//gen5_pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks = p_udp_hdr->streamChunks;
	return status;
}

srr3_api_status_e  GetPreviousData(UDPRecord_Header* p_udp_hdr, const char* LogFname, Customer_T custId)
{
	char sourceId = 0;
	unsigned int Stream_No = 0;
	if ((p_udp_hdr->Platform == 41) || (p_udp_hdr->Platform == 47))
	{
		sourceId = p_udp_hdr->Radar_Position - 1;
		if (sourceId == 19)
		{
			Stream_No = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
		}
		else
		{
			Stream_No = MaptostreamNumber(p_udp_hdr->streamNumber, custId);
		}
	}
	else if (p_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_udp_hdr->Platform <= UDP_PLATFORM_CUST_SRR3_FC)
	{
		if (p_udp_hdr->customerID == SCANIA_MAN)
			sourceId = ManSourceid;
		else
			sourceId = p_udp_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;
		if (sourceId == 19)
		{
			Stream_No = MaptoEcustreamNumber(p_udp_hdr->streamNumber);
		}
		else
		{
			Stream_No = MaptostreamNumber(p_udp_hdr->streamNumber, custId);
		}
	}
	else
	{
		sourceId = p_udp_hdr->Radar_Position - UDP_PLATFORM_CUST_SRR3_RL;
		Stream_No = p_udp_hdr->streamNumber;
	}
	srr3_api_status_e status = Status_Latch_Error;
	static int processedchunk[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };

	if (!f_firstdata[sourceId]) {
		memcpy(&prev_udp_frame_hdr[sourceId], p_udp_hdr, sizeof(UDPRecord_Header));
		f_firstdata[sourceId] = true;
	}

	if (prev_udp_frame_hdr[sourceId].streamNumber == p_udp_hdr->streamNumber)
	{
		if (prev_udp_frame_hdr[sourceId].streamRefIndex == p_udp_hdr->streamRefIndex)
		{
			if (p_udp_hdr->streamChunkIdx > 0)
			{
				processedchunk[sourceId][Stream_No]++;
			}
		}

	}
	if (Enddataflag[sourceId] && MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode)
	{
		if (Stream_No != CDC_DATA)
		{
			if ((processedchunk[sourceId][Stream_No] + 1) == p_udp_hdr->streamChunks)
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
			else
			{
				status = Status_Latch_InProcess;
			}
			if ((prev_udp_frame_hdr[sourceId].streamNumber == p_udp_hdr->streamNumber) && (prev_udp_frame_hdr[sourceId].streamRefIndex == p_udp_hdr->streamRefIndex))
			{
					if (processedchunk[sourceId][Stream_No] != p_udp_hdr->streamChunkIdx)
					{
						int offset = abs(processedchunk[sourceId][Stream_No] - p_udp_hdr->streamChunks);
						UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
						processedchunk[sourceId][Stream_No] = 0;
					}
					else
					{
						status = Status_Latch_Error;
					}				
			}
		}
		if (Stream_No == CDC_DATA)
		{
			if (CDC_ScanIdCurrent[sourceId][CDC_DATA] == CDC_scan_idx_previous[sourceId][CDC_DATA] + 1)
			{
				if (pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks != p_udp_hdr->streamChunks)
				{
					UDPPacketLossErrorCount[sourceId][CDC_DATA]++;				
				}
			}
			else
			{
				status = Status_Latch_Completed;
				Enddataflag[sourceId] = false;
			}
		}
		
		int diffRefIdx = (int)(p_udp_hdr->streamRefIndex - prev_udp_frame_hdr[sourceId].streamRefIndex) ;
		if (diffRefIdx >= 2)
		{
			int Diff = (p_udp_hdr->streamRefIndex - prev_udp_frame_hdr[sourceId].streamRefIndex);
			if (Diff >= 5)
			{
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					fprintf(fCreate, " The given file (%s)is not continuous!!\n", LogFname);
			}
			if (Stream_No != CDC_DATA)
			{
				ScanIdDropCount_udp[sourceId]++;
			}
			else
			{
				ScanIdDropCount_cdc[sourceId]++;
			}
		}
	}
	else
	{
		if ((prev_udp_frame_hdr[sourceId].streamNumber == p_udp_hdr->streamNumber) && (prev_udp_frame_hdr[sourceId].streamRefIndex == p_udp_hdr->streamRefIndex))
		{			
				if (p_udp_hdr->streamChunkIdx + 1 == p_udp_hdr->streamChunks && p_udp_hdr->streamChunks != processedchunk[sourceId][Stream_No] + 1)
				{
					int offset = abs(processedchunk[sourceId][Stream_No] - p_udp_hdr->streamChunks);
					UDPPacketLossErrorCount[sourceId][Stream_No] += offset;
					processedchunk[sourceId][Stream_No] = 0;
				}
				else
				{
					status = Status_Latch_Error;
				}
			
		}
	}

	if (Stream_No != CDC_DATA) {
		if (p_udp_hdr->streamChunks == (processedchunk[sourceId][Stream_No] + 1))
		{
			processedchunk[sourceId][Stream_No] = 0;
		}
	}
	else {
		if (!(p_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_FC))
		{
			if (pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks == p_udp_hdr->streamChunks) {
				processedchunk[sourceId][Stream_No] = 0;
			}
		}
	}
	memcpy(&prev_udp_frame_hdr[sourceId], p_udp_hdr, sizeof(UDPRecord_Header));
	//pack_working_prev[sourceId][CDC_DATA].proc_info.nProcessed_chks = p_udp_hdr->streamChunks;
	return status;
}
//void RECUCalibPktConsistencyCheck(const UDP_FRAME_PAYLOAD_T *pudp_frame_payload_in){
//	
//	F360_GDSR_status_e F360_GDSR_status = DPH_F360_GDSR_INPROCESS;
//	udp_custom_frame_header_t  custom_frame_hdr = {0};
//	UDPRecord_Header  udp_frame_hdr = {0};
//	udp_custom_frame_header_t  cust_frame_hdr = {0};
//	uint8_t radarIndex = 0;
//	unsigned char sensor_id = 0;
//	int stream_chunck_id_prev = 0;
//	static int stream_chunck_id_curr = 0;
//	int total_stream_chunks = 0;
//	int count = 0;
//	static int once = 0;
//	uint8_t udpStreamNo = 0;
//
//	const unsigned char* ptr_bytestream = (const unsigned char*)pudp_frame_payload_in;	//&record.payload[0];
//	const UDP_FRAME_STRUCTURE_T *p_rdr_udp_frame = (const UDP_FRAME_STRUCTURE_T *)ptr_bytestream;
//	//if(!m_pDecoderPlugin){
//	//	if ( Init() ){
//	//		return ;
//	//	}
//	//}
//	m_pDecoderPlugin->SRR3_API_GetFrameHdr(ptr_bytestream, &udp_frame_hdr);
//	m_pDecoderPlugin->SRR3_API_GetCustFrameHdr(ptr_bytestream, &cust_frame_hdr);
//
//	int platform = udp_frame_hdr.Platform;
//	if(platform >= UDP_PLATFORM_SRR5)
//	{
//		radarIndex = MapSourceToRadarPos(udp_frame_hdr.Radar_Position);
//	}
//	else
//	{
//		radarIndex = MapSourceToRadarPos(udp_frame_hdr.Platform );
//	} 
//
//	if(udp_frame_hdr.streamNumber != ECU_CAL_STREAM){
//		return;
//	}
//
//	stream_chunck_id_prev = stream_chunck_id_curr;
//	stream_chunck_id_curr = udp_frame_hdr.streamChunkIdx;
//	total_stream_chunks = udp_frame_hdr.streamChunks;
//	
//	
//	if((i == total_stream_chunks-1))
//	{
//		i = 0;
//		m_vec.clear();
//		m_vec.resize(50);
//		once = 0;
//		
//	}
//
//	if(!once)
//	{
//	Ecu_scan_prev = udp_frame_hdr.streamRefIndex ;
//	Ecu_scan_next = udp_frame_hdr.streamRefIndex;
//	m_vec[stream_chunck_id_curr] = 1;
//	once = 1;
//	return;
//	}
//
//
//	if(( i < total_stream_chunks -1)&& (Ecu_scan_prev == Ecu_scan_next ))
//	{
//		Ecu_scan_prev = Ecu_scan_next ;
//		scan_next = udp_frame_hdr.streamRefIndex;
//		m_vec[stream_chunck_id_curr] = 1;
//		i++;
//		return;
//	}
//	else{
//		m_map.insert(pair <int, vector<int>>(Ecu_scan_prev, m_vec)); 
//		int count = 0;
//		int arr[RECUstreamChunkssize] = {0};
//		vector<int> :: iterator ptr ;
//		map <int, vector<int>>:: iterator itr = m_map.begin();
//		int scan = itr->first;
//		int j = 0;
//		for(itr ; itr != m_map.end(); itr++){
//			for(ptr = itr->second.begin(); ptr != itr->second.end() ; ptr++){
//				if(*ptr == 1){
//					arr[j] = 1;
//					j++;
//			}
//				
//			}
//		}
//
//		for( int j = 0; j< RECUstreamChunkssize ; j++){
//			if(arr[j] == 0){
//				LOGMSG(eERROR, "RECU Calibration Stream::scan_Index missed = %d, stream_chunk missed = %d ",scan , j);
//			}
//		}
//		i = 0;
//		once = 0;
//		m_vec.clear();
//		m_vec.resize(50);
//		m_map.clear();
//	}
//}


void gen5_SensorCalibPktLossCheck(const GEN5_UDPRecord_Header_T *p_gen5_udp_hdr)
{
	int platform = p_gen5_udp_hdr->Platform;
	int radarIndex = 0;

	if (p_gen5_udp_hdr->stream_Id != GEN5_CALIB_STREAM)
	{
		return;
	}
	radarIndex = MapSourceToRadarPos(p_gen5_udp_hdr->Platform);
	

	radar_chunk_id_prev[radarIndex] = radar_chunk_id_curr[radarIndex];
	radar_chunk_id_curr[radarIndex] = p_gen5_udp_hdr->Cal_Running_Cnt;
	radar_total_chunk_count[radarIndex] = p_gen5_udp_hdr->Total_Cal_Chunk_Cnt;

	radar_cal_source_prev[radarIndex] = radar_cal_source_curr[radarIndex];
	radar_cal_source_curr[radarIndex] = p_gen5_udp_hdr->Cal_Source;

	if (!flag1[radarIndex]) {
		if (p_gen5_udp_hdr->Cal_Running_Cnt == 0)
		{
			flag1[radarIndex] = true;
		}
		else
			return;
	}
	radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
	radar_scan_next[radarIndex] = p_gen5_udp_hdr->scan_index;
	if (radar_cal_source_prev[radarIndex] != radar_cal_source_curr[radarIndex] && radar_chunk_id_curr[radarIndex] != 0)
	{
		if (radar_scan_prev[radarIndex] != -1)
		{
			if (((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) > 1 && radar_scan_prev[radarIndex] != 65535 && radar_scan_next[radarIndex] == 0)
			{
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s][%s] ::Reset happend at  = %d   \n", GetStreamName(p_gen5_udp_hdr->stream_Id));
				}
			}
			else
				if (((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) > 1)
				{
					for (int i = 1; i < ((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])); ++i) {
						ScanIdDropCount_CalibStream[radarIndex]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s][%s] ::scan_Index missed = %d   \n", GetStreamName(p_gen5_udp_hdr->stream_Id), GetRadarPosName(radarIndex), (unsigned16_T)(radar_scan_prev[radarIndex] + i));
						}
					}
				}
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				if (radar_scan_next[radarIndex] - radar_scan_prev[radarIndex] > 2)
				{
					Continous_ScanDrop_calib = 1;
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in CALIBSTREAM : Sensor = (%s), Scan Index = %d \n", GetRadarPosName(radarIndex), radar_scan_prev[radarIndex]);

				}
			}

		}
	}
	else
		if ((radar_cal_source_prev[radarIndex] == radar_cal_source_curr[radarIndex]))
		{
			if ((radar_chunk_id_prev[radarIndex] == radar_chunk_id_curr[radarIndex])) {
				radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
				radar_scan_next[radarIndex] = p_gen5_udp_hdr->scan_index;
				++count1[radarIndex];
				if (!once1[radarIndex])
				{
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s][%s] :: In scan_Index  = %d duplicate cal_source found = %d \n", GetStreamName(p_gen5_udp_hdr->stream_Id), GetRadarPosName(radarIndex), radar_scan_next[radarIndex], radar_cal_source_curr[radarIndex]);
					}
				}
			}
			else {
				count1[radarIndex] = 0;
			}

			if (!count1[radarIndex])
			{
				radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
				radar_scan_next[radarIndex] = p_gen5_udp_hdr->scan_index;

			}

			if ((((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) == 1) && (radar_chunk_id_curr[radarIndex] < radar_total_chunk_count[radarIndex])) {

				if ((radar_chunk_id_curr[radarIndex] - radar_chunk_id_prev[radarIndex]) != 1)
				{
					if (!once1[radarIndex])
					{
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s][%s]::Cal_Running_Cnt is not incremental\n", GetStreamName(p_gen5_udp_hdr->stream_Id), GetRadarPosName(radarIndex));
						}

					}
					once1[radarIndex] = 0;
				}
			}
			if (radar_scan_prev[radarIndex] != -1)
			{
				if ((((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) != 1) && (radar_scan_prev[radarIndex] != -1)) {
					for (int i = 1; i < ((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])); ++i) {
						ScanIdDropCount_CalibStream[radarIndex]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t[%s][%s] ::scan_Index missed = %d  in calsource = %d\n", GetStreamName(p_gen5_udp_hdr->stream_Id), GetRadarPosName(radarIndex), (unsigned16_T)(radar_scan_prev[radarIndex] + i), radar_cal_source_prev[radarIndex]);
							fprintf(fCreate, "\t\t=========================================================================================================\n");
						}
					}
				}
			}

		}

}

void SensorCalibPktLossCheck(const UDPRecord_Header *udp_frame_hdr, const udp_custom_frame_header_t *cust_frame_hdr)
{
	int platform = udp_frame_hdr->Platform;
	int radarIndex = 0;

	if (udp_frame_hdr->streamNumber != CALIBRATION_DATA)
	{
		return;
	}

	if ((platform == UDP_PLATFORM_SRR5) || (platform >= UDP_PLATFORM_MCIP_SRR5))
	{
		radarIndex = MapSourceToRadarPos(udp_frame_hdr->Radar_Position);
	}
	else if (platform == UDP_SOURCE_CUST_DSPACE)
	{
		radarIndex = RDR_POS_DSPACE;
	}
	else
	{
		if (udp_frame_hdr->customerID == SCANIA_MAN)
			radarIndex = ManSourceid;
		else
			radarIndex = udp_frame_hdr->Platform - UDP_PLATFORM_CUST_SRR3_RL;		
	}

	radar_chunk_id_prev[radarIndex] = radar_chunk_id_curr[radarIndex];
	radar_chunk_id_curr[radarIndex] = cust_frame_hdr->Cal_Running_Cnt;
	radar_total_chunk_count[radarIndex] = cust_frame_hdr->Total_Cal_Chunk_Cnt;

	radar_cal_source_prev[radarIndex] = radar_cal_source_curr[radarIndex];
	radar_cal_source_curr[radarIndex] = cust_frame_hdr->CalSource;

	if (!flag1[radarIndex]) {
		if (cust_frame_hdr->Cal_Running_Cnt == 0)
		{
			flag1[radarIndex] = true;
		}
		else
			return;
	}
	radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
	radar_scan_next[radarIndex] = udp_frame_hdr->streamRefIndex;
	if (radar_cal_source_prev[radarIndex] != radar_cal_source_curr[radarIndex] && radar_chunk_id_curr[radarIndex] != 0)
	{

		if (radar_scan_prev[radarIndex] != -1)
		{
			if (((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) > 1 && radar_scan_prev[radarIndex] != 65535 && radar_scan_next[radarIndex] == 0)
			{
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
				{
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\t[%s][%s] ::Reset happend at  = %d   \n", GetStreamName(udp_frame_hdr->streamNumber));
				}
			}
			else
				if (((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) > 1)
				{
					for (int i = 1; i < ((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])); ++i) {
						ScanIdDropCount_CalibStream[radarIndex]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s][%s] ::scan_Index missed = %d   \n", GetStreamName(udp_frame_hdr->streamNumber), GetRadarPosName(radarIndex), (unsigned16_T)(radar_scan_prev[radarIndex] + i));
						}
					}
				}
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				if (radar_scan_next[radarIndex] - radar_scan_prev[radarIndex] > 2)
				{
					Continous_ScanDrop_calib = 1;
					fprintf(fCreate, "\t\t=========================================================================================================\n");
					fprintf(fCreate, "\t\tSTREAMLOSS for two or more  continous cycles in CALIBSTREAM : Sensor = (%s), Scan Index = %d \n", GetRadarPosName(radarIndex), radar_scan_prev[radarIndex]);

				}
			}

		}
	}
	else
		if ((radar_cal_source_prev[radarIndex] == radar_cal_source_curr[radarIndex]))
		{
			if ((radar_chunk_id_prev[radarIndex] == radar_chunk_id_curr[radarIndex])) {
				radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
				radar_scan_next[radarIndex] = udp_frame_hdr->streamRefIndex;
				++count1[radarIndex];
				if (!once1[radarIndex])
				{
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
					{
						fprintf(fCreate, "\t\t=========================================================================================================\n");
						fprintf(fCreate, "\t\t[%s][%s] :: In scan_Index  = %d duplicate cal_source found = %d \n", GetStreamName(udp_frame_hdr->streamNumber), GetRadarPosName(radarIndex), radar_scan_next[radarIndex], radar_cal_source_curr[radarIndex]);
					}
				}
			}
			else {
				count1[radarIndex] = 0;
			}

			if (!count1[radarIndex])
			{
				radar_scan_prev[radarIndex] = radar_scan_next[radarIndex];
				radar_scan_next[radarIndex] = udp_frame_hdr->streamRefIndex;

			}

			if ((((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) == 1) && (radar_chunk_id_curr[radarIndex] < radar_total_chunk_count[radarIndex])) {

				if ((radar_chunk_id_curr[radarIndex] - radar_chunk_id_prev[radarIndex]) != 1)
				{
					if (!once1[radarIndex])
					{
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t=========================================================================================================\n");
							fprintf(fCreate, "\t\t[%s][%s]::Cal_Running_Cnt is not incremental\n", GetStreamName(udp_frame_hdr->streamNumber), GetRadarPosName(radarIndex));
						}

					}
					once1[radarIndex] = 0;
				}
			}
			if (radar_scan_prev[radarIndex] != -1)
			{
				if ((((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])) != 1) && (radar_scan_prev[radarIndex] != -1)) {
					for (int i = 1; i < ((unsigned16_T)(radar_scan_next[radarIndex] - radar_scan_prev[radarIndex])); ++i) {
						ScanIdDropCount_CalibStream[radarIndex]++;
						if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
						{
							fprintf(fCreate, "\t\t[%s][%s] ::scan_Index missed = %d  in calsource = %d\n", GetStreamName(udp_frame_hdr->streamNumber), GetRadarPosName(radarIndex), (unsigned16_T)(radar_scan_prev[radarIndex] + i), radar_cal_source_prev[radarIndex]);
							fprintf(fCreate, "\t\t=========================================================================================================\n");
						}
					}
				}
			}

		}

}


void GetTimingOverrunInfo(IRadarStream * g_pIRadarStrm, short stream_no, unsigned int ScanId, unsigned char sourceId)
{
	unsigned32_T  end_time = 0, Tracker_Timing_info = 0, start_time = 0;
	DPH_TIME_INFO_DATA_T Z7b_Timing_Info = { 0 };
	Z7A_Data_T Z7a_timing_info = { 0 };
	Core_1_Timing_Info_XML_T ECU_Timing_Info = { 0 };
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Z7b_Timing_Info, TIME_INFO_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Z7a_timing_info, Z7A_DATA);
	m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ECU_Timing_Info, ECU_TIMING_INFO);
	//g_pIRadarStrm->getData((unsigned char*)&Z7b_Timing_Info,TIME_INFO_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&Z7a_timing_info,Z7A_DATA);
	//g_pIRadarStrm->getData((unsigned char*)&ECU_Timing_Info,ECU_TIMING_INFO);
	if ((stream_no == Z7B_LOGGING_DATA) && (Z7b_Timing_Info.Timing_Info[28] != 0))
	{
		if (Z7b_Timing_Info.Timing_Info[Z7B_APPLICATION_TIME] > THRESHOLD_Z7B_TIME)
		{
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, "\t\t [%s]Timingoverruns for core2 with time[%d] and scanid[%d] \n", GetRadarPosName(sourceId), Z7b_Timing_Info.Timing_Info[Z7B_APPLICATION_TIME], ScanId);
			}
			Timingoverflowcount[sourceId][stream_no]++;

		}
	}
	else if (stream_no == RADAR_ECU_CORE_1)
	{
		start_time = ECU_Timing_Info.tracker_starttime.time_seconds * 1000 + ECU_Timing_Info.tracker_starttime.time_fraction;
		end_time = ECU_Timing_Info.tracker_endtime.time_seconds * 1000 + ECU_Timing_Info.tracker_endtime.time_fraction;
		Tracker_Timing_info = end_time - start_time;
		if (Tracker_Timing_info > THRESHOLD_ECU_TIME)
		{
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, "\t\t [%s]Timingoverruns for ECU with time[%d] and scanid[%d] \n", GetRadarPosName(sourceId), Tracker_Timing_info, ScanId);
			}
			Timingoverflowcount[sourceId][stream_no]++;
		}
	}
	else if ((stream_no == Z7A_LOGGING_DATA) && (Z7a_timing_info.Timing_Info[8] != 0))
	{
		if (Z7a_timing_info.Timing_Info[Z7A_APPLICATION_TIME] > THRESHOLD_Z7A_TIME)
		{
			if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
			{
				fprintf(fCreate, "\t\t=========================================================================================================\n");
				fprintf(fCreate, "\t\t [%s]Timingoverruns for core1 with time[%d] and scanid[%d]\n ", GetRadarPosName(sourceId), Z7a_timing_info.Timing_Info[Z7A_APPLICATION_TIME], ScanId);
			}
			Timingoverflowcount[sourceId][stream_no]++;
		}
	}
}

//int ExtractPacketLoss(FILE*pOutFile, const char* LogFname, UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* pRec, bool mf4flag)
//{
//	UDPRecord_Header  udp_frame_hdr = { 0 };
//	udp_custom_frame_header_t  cust_frame_hdr = { 0 };
//	unsigned char format = 1; //This is to flag the endianness to be big endian
//	static int once = 0;
//	unsigned char* bytestream = &pRec->payload[0];
//	INT64 timestamp = (INT64)Timestamp;
//	bool setECUflag = 0;
//	cust_id = (Customer_T)pRec->payload[23];
//	unsigned char sourceId = 0;
//	srr3_api_status_e status = Status_Latch_Error;
//	if ((pRec->payload[0] != 0xA1 || pRec->payload[0] != 0xA2 || pRec->payload[0] != 0xA3) && (pRec->payload[1] != 0x18)
//		&& (pRec->payload[1] != 0xA2 || pRec->payload[1] != 0xA3) && (pRec->payload[0] != 0x18)) {
//		return 0;
//	}
//	if (pRec->payload[8] == 0 && pRec->payload[9] == 0) {
//		return 0;
//	}
//	if (pRec->payload[8] >= UDP_PLATFORM_SRR5) { // Checking is the platform is other than SRR3
//		sourceId = (pRec->payload[9] - 1);
//	}
//	else {
//		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
//	}
//	if (sourceId == RDR_POS_ECU)
//	{
//		format = 0;
//		if (MUDP_master_config->MUDP_Radar_ECU_status != 1)
//		{
//			return 0;
//		}
//	}
//	else
//	{
//		if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] != 1)
//		{
//			return 0;
//		}
//	}
//
//	short source_no = -1;
//	short stream_no = -1;
//	//	srr3_api_status_e status = Status_Latch_Error;
//		//status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream,timestamp,&source_no, &stream_no, format);
//	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
//		m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
//		m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)bytestream, &cust_frame_hdr);
//		if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode /*&& stream_no != 4*/) {
//			status = GetPreviousData(&udp_frame_hdr, LogFname, cust_id);
//		}
//
//		get_Source_TxnCnt(&udp_frame_hdr, cust_id);
//		get_Stream_TxnCnt(&udp_frame_hdr);
//		CheckSameScanIdforChunk(&udp_frame_hdr, timestamp);
//		SensorCalibPktLossCheck(&udp_frame_hdr, &cust_frame_hdr);
//
//	}
//	if (((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)||(MUDP_master_config->packet_loss_statistics == 1)) && MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode)
//	{
//		if (Enddataflag[sourceId] == 0 && status != Status_Latch_Completed)
//		{
//			status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
//		}
//	}
//	else
//	{
//		status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
//	}
//	if (status != Status_Latch_Completed)
//		return 0;
//
//	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
//	unsigned char version = 0;
//
//	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1) {
//		if (stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA || stream_no == CALIBRATION_DATA || stream_no == Z4_CUSTOM || stream_no == Z4_LOGGING_DATA || stream_no == CDC_DATA
//			|| stream_no == SRR3_Z7B_CUST_LOGGING_DATA || stream_no == Z4_CUST_LOGGING_DATA || stream_no == RADAR_ECU_CORE_0 || stream_no == RADAR_ECU_CORE_1 || stream_no == RADAR_ECU_CORE_3 || stream_no == RADAR_ECU_CALIB) {
//			status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out, (unsigned char)source_no, (unsigned char)stream_no);
//		}
//		else {
//			return 0;
//		}
//		unsigned int ScanId = p_latch_out->proc_info.frame_header.streamRefIndex;
//		/*if(setECUflag){*/
//		get_scanindex_info(ScanId, sourceId, stream_no);
//		//}
//		if (p_latch_out->proc_info.f_failure_block == 1 && stream_no != CDC_DATA) {
//			Get_PacketLossCount(p_latch_out, sourceId, stream_no);
//		}
//		if (stream_no == CDC_DATA)
//		{
//			CDC_PacketLossCount(p_latch_out, sourceId);
//
//		}
//
//		//return 0;
//
//	}
//	cust_id = (Customer_T)p_latch_out->proc_info.frame_header.customerID;
//
//	if (stream_no == Z7B_LOGGING_DATA)
//	{
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
//		}
//	}
//
//	else if (stream_no == Z7A_LOGGING_DATA) {
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
//		}
//	}
//	else if (stream_no == RADAR_ECU_CORE_1)
//	{
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_C1, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
//		}
//	}
//	if (g_pIRadarStrm[stream_no] != 0)
//	{
//		g_pIRadarStrm[stream_no]->FillStreamData(&p_latch_out->data[0], p_latch_out->proc_info.nDataLen, &p_latch_out->proc_info);
//		unsigned int ScanId = p_latch_out->proc_info.frame_header.streamRefIndex;
//		if (stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA || stream_no == RADAR_ECU_CORE_1)
//
//		{
//			GetTimingOverrunInfo(g_pIRadarStrm[stream_no], stream_no, ScanId, sourceId);
//		}
//	}
//
//
//	return 0;
//
//}
int GetPlatform(unsigned char index0, unsigned char index1)
{
	if (((index0 == 0xA1 || index0 == 0xA2 || index0 == 0xA3) && (index1 == 0x18))
		||( (index1 == 0xA2 || index1 == 0xA3) && (index0 == 0x18))) {
		return UDP_PLATFORM_SRR5;
	}	
	else if ((index0 == 0xA4 || index1 == 0x2C) || (index0 == 0x2C|| index1 == 0xA4))
		return UDP_PLATFORM_SRR6;
	else if ((index0 == 0xA5 || index1 == 0x1C) || (index0 == 0x1C || index1 == 0xA5))
		return UDP_PLATFORM_SRR7_PLUS;
	return 0;
}
int GetNextLogname(char sourceId,int StrmValue,short stream_no)
{
	++num_rows[sourceId][StrmValue];
	if (StrmValue != UDP_CDC_STRM && num_rows[sourceId][StrmValue] == MAX_CSV_ROWS)
	{
		char nextFname[_MAX_PATH] = { 0 };
		char basename[_MAX_FNAME] = { 0 };

		if (fpMF4Dump[sourceId][StrmValue] != NULL)
			fclose(fpMF4Dump[sourceId][StrmValue]);

		string FName = CSVFName[sourceId][StrmValue];
		size_t len = FName.length();
		if (len + 4 >= _MAX_PATH)
		{
			printf("\n[ERROR]: Unable to open file.Max file path length allowed is 255 characters < % s>\n\r", FName.c_str());
			return -1;
		}
		FName.copy(basename, len - 4);
		sprintf(nextFname, "%s_%03u%s", basename, LogNum[sourceId][StrmValue]++, ".csv");

		fpMF4Dump[sourceId][StrmValue] = fopen(nextFname, "w");
		if (!fpMF4Dump[sourceId])
		{
			fprintf(stderr, "Error Creating Output file: %s\n\n", nextFname);
			fprintf(stderr, " Please Rerun by closing the CSV file %s\n\n", nextFname);
			return -1;
		}

		if (stream_no == -1)
			bOnce[sourceId] = 0;
		else
			m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);

		num_rows[sourceId][StrmValue] = 1;
	}
	return 0;
}//#HTMLInMUDP
int ExtractAndListDVSU(Streamver_T Streamver, const char* LogFname, FILE** pxmlFile, FILE* hxmlFile, UINT64 Timestamp, enDumpOpt opt, DVSU_RECORD_T* pRec, bool mf4flag, std::shared_ptr<HtmlReportManager>& sptr_manager, int filecnt )
{
	UDPRecord_Header  udp_frame_hdr = { 0 };
	GEN5_UDPRecord_Header_T gen5_udp_frame_hdr = { 0 };
	GEN7_UDPRecord_Header_T gen7_udp_frame_hdr = { 0 };
	Stream_Hdr_T gen7_payload_hdr = { 0 };
	udp_custom_frame_header_t  cust_frame_hdr = { 0 };
	Ref_Logging_Data_T *Debug = NULL;
	unsigned char format = 1; //This is to flag the endianness to be big endian
	static int once[MAX_RADAR_COUNT][MAX_LOGGING_SOURCE] = { 0 };
	unsigned char* bytestream = &pRec->payload[0];
	INT64 timestamp = (INT64)Timestamp;
	bool setECUflag = 0;
	unsigned char sourceId = 0;
	char InputLogName[1024] = { 0 };
	strcpy(InputLogName, LogFname);
	unsigned int platform = 0;
	unsigned int retValue = GetPlatform(pRec->payload[0], pRec->payload[1]);
	if ((retValue == UDP_PLATFORM_SRR5) || (retValue == UDP_PLATFORM_MCIP_SRR5))
	{
		if (pRec->payload[8] == 0 && pRec->payload[9] == 0) {
			return 0;
		}
	}
	else if (retValue == UDP_PLATFORM_SRR6)
	{
		if (pRec->payload[2] == 0 && pRec->payload[4] == 0)
			return 0;
	}
	/*else
	{
		return 0;
	}*/
	if ((pRec->payload[0] == 0x2C || pRec->payload[1] == 0xA4) && ((pRec->payload[2] == UDP_PLATFORM_FLR4_PLUS)|| (pRec->payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE)))
	{
		format = 0; // little endian
	}
	else if (pRec->payload[0] == 0x1C || pRec->payload[1] == 0xA5)
	{
		format = 0; // little endian
	}

	if (pRec->payload[2] == UDP_PLATFORM_FLR4_PLUS)
	{
		platform = UDP_PLATFORM_FLR4_PLUS;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
	{
		platform = UDP_PLATFORM_FLR4_PLUS_STANDALONE;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_SRR6)
	{
		platform = UDP_PLATFORM_SRR6;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_SRR6_PLUS)
	{
		platform = UDP_PLATFORM_SRR6_PLUS;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_CAN)
	{
		platform = UDP_PLATFORM_SRR6_PLUS_1X_CAN;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_ETH)
	{
		platform = UDP_PLATFORM_SRR6_PLUS_1X_ETH;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
	{
		platform = UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH;
	}
	else if (pRec->payload[2] == UDP_PLATFORM_STLA_FLR4)
	{
		platform = UDP_PLATFORM_STLA_FLR4;
	}
	
	//if (pRec->payload[8] >= UDP_PLATFORM_SRR5) { // Checking is the platform is other than SRR3
	//	sourceId = (pRec->payload[9] - 1);
	//}
	//else {
	//	sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
	//}

	if (retValue == UDP_PLATFORM_SRR7_PLUS)
	{
		sourceId = (uint8_t)pRec->payload[27] - 1;
	}
	else if (retValue == UDP_PLATFORM_SRR6)
	{
		sourceId = (uint8_t)pRec->payload[4] - 1;
	}
	else
	{
		if (pRec->payload[8] >= UDP_PLATFORM_CUST_SRR3_RL && pRec->payload[8] <= UDP_PLATFORM_CUST_SRR3_FC) {
			sourceId = (uint8_t)(pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL);
		}
		// For DSPACE UDP logs, radar position is assigned as RDR_POS_DSPACE
		else if (pRec->payload[8] == UDP_SOURCE_CUST_DSPACE) {

			sourceId = RDR_POS_DSPACE;

		}
		else if (pRec->payload[8] == BPILLAR_STATUS_BP_L || pRec->payload[8] == UDP_PLATFORM_FORD_SRR5) {
			sourceId = RDR_POS_FC;
		}
		// For SRR5 sensors including radar ECU, radar position is derived from the Radar_Pos of UDP Header: 1, 2, 3, 4 and for ECU 20
		else {
			sourceId = (uint8_t)(pRec->payload[9] - 1);
		}
	}

	if (sourceId == RDR_POS_ECU || MUDP_master_config->Radar_Stream_Options[Dspace] == Dspace)
	{
		format = 0;
	}

	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[CSV] == 1)
	{
		if (sourceId == RDR_POS_ECU)
		{
			if (MUDP_master_config->MUDP_Radar_ECU_status != 1)
			{
				return 0;
			}
		}
		else
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] != 1)
			{
				return 0;
			}
		}
	}

	if (retValue == UDP_PLATFORM_SRR7_PLUS)
	{
		m_pDecoderPlugin->SRR7_API_GetFrameHdr((unsigned char*)bytestream, &gen7_udp_frame_hdr);
		if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0 && MUDP_master_config->packet_loss_statistics == 0)
		{
			for (int i = 0; i < 4; i++)
			{
				MUDP_master_config->MUDP_Select_Sensor_Status[i] = 1;
			}
		}
	}
	else if (retValue == UDP_PLATFORM_SRR6)
	{
		m_pDecoderPlugin->SRR6_API_GetFrameHdr((unsigned char*)bytestream, &gen5_udp_frame_hdr);
		if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0 && MUDP_master_config->packet_loss_statistics == 0)
		{
			for (int i = 0; i < 4; i++)
			{
				MUDP_master_config->MUDP_Select_Sensor_Status[i] = 1;
			}
		}
	}
	else if (retValue != UDP_PLATFORM_SRR6 && retValue != UDP_PLATFORM_SRR7_PLUS)
	{
		m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
		m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0], &cust_frame_hdr);

		if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 && MUDP_master_config->data_extracter_mode[PACKET_Loss] == 0 && MUDP_master_config->packet_loss_statistics == 0)
		{
			for (int i = 0; i < 4; i++)
			{
				MUDP_master_config->MUDP_Select_Sensor_Status[i] = 1;
			}

			if (udp_frame_hdr.customerID == BMW_SAT)
			{
				MUDP_master_config->MUDP_Radar_ECU_status = 1;
			}
			if (udp_frame_hdr.customerID == BMW_BPIL)
			{
				MUDP_master_config->MUDP_Select_Sensor_Status[6] = 1;
				MUDP_master_config->MUDP_Select_Sensor_Status[7] = 1;
			}
			if (udp_frame_hdr.customerID == SCANIA_MAN)
			{
				MUDP_master_config->MUDP_Select_Sensor_Status[5] = 1;
			}
		}
	}
//temporary changes for testing on side radars  
	Customer_T CustID = (Customer_T)gen5_udp_frame_hdr.customerId;
	uint8_t  stream_no_new = (GEN5_Radar_Logging_Data_Source_T)GEN5MaptoStreamNumber(gen5_udp_frame_hdr.stream_Id, gen5_udp_frame_hdr.customerId);
	if (CustID == STLA_SCALE4 && stream_no_new == C2_CORE_MASTER_STREAM && gen5_udp_frame_hdr.scan_index == 0 &&(sourceId == BP_R || sourceId == BP_L))
		return 0;
	
	if (udp_frame_hdr.customerID == SCANIA_MAN)
	{
		if (udp_frame_hdr.streamNumber == 10)
		{
			sourceId = cust_frame_hdr.sensorid ;
			ManSourceid = cust_frame_hdr.sensorid;
		}
		else
		{
			sourceId = cust_frame_hdr.sensorid - 1;
			ManSourceid = cust_frame_hdr.sensorid - 1;
		}
	}

	if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
	{
		switch (Streamver.HDR)
		{

		case HEADER:
			if (retValue == UDP_PLATFORM_SRR7_PLUS)
			{
				int ret = GetNextLogname(sourceId, Streamver.HDR, -1);
				if (ret != 0)
					return ret;
				print_gen7_udp_header_ALL(fpMF4Dump[sourceId][Streamver.HDR], &gen7_udp_frame_hdr, (signed64_T)timestamp);
			}
			else if (retValue == UDP_PLATFORM_SRR6)
			{

				int ret = GetNextLogname(sourceId, Streamver.HDR, -1);
				if (ret != 0)
					return ret;
				print_gen5_udp_header_ALL(fpMF4Dump[sourceId][Streamver.HDR], &gen5_udp_frame_hdr, (signed64_T)timestamp);
			}
			else 
			{
				//	print_udp_header(pOutFile, &udp_frame_hdr,(unsigned long)timestamp);
				int ret = GetNextLogname(sourceId, Streamver.HDR, -1);
				if (ret != 0)
					return ret;
				print_udp_header_All(fpMF4Dump[sourceId][Streamver.HDR], &udp_frame_hdr, &cust_frame_hdr, (signed64_T)timestamp);
			}
			//break;
			//return 0;

		case UDP_CUSTOM_HDR:
			// m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0],&cust_frame_hdr);
			//print_udp_cust_header(pOutFile,&cust_frame_hdr);
			break;

		default:
			break;
		}
	}

	short source_no = -1;
	short stream_no = -1;

	srr3_api_status_e status = Status_Latch_Error;

	//CALIB, ROT_INTERNALS Streams Check Should be removed once CALIB Stream issues are resolved for GEN7
	if (gen7_udp_frame_hdr.streamNumber != e_CALIB_GEN7_STREAM /* && gen7_udp_frame_hdr.streamNumber != e_CDC_STREAM*/ 
		&& gen7_udp_frame_hdr.streamNumber != e_BLOCKAGE_STREAM && gen7_udp_frame_hdr.streamNumber != e_ROT_INTERNALS_STREAM && gen7_udp_frame_hdr.streamNumber != e_ROT_TRACKER_INFO_STREAM)
	{

		if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
			if ((MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1) || (MUDP_master_config->MUDP_Radar_ECU_status == 1))
			{
				if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
				{
					//CALIB, ID, BLOCKAGE, MMIC Streams Check Should be removed once CALIB Stream issues are resolved for GEN7
					if (pRec->payload[19] != CALIB_STREAM && pRec->payload[19] != BLOCKAGE_STREAM && pRec->payload[19] != ROT_TRACKER_INFO_STREAM)
					{
						//if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode) {
						status = GEN7_GetPreviousData(&gen7_udp_frame_hdr, LogFname, cust_id);
						////}
						gen7_get_CDC_Max_Chunk(&gen7_udp_frame_hdr);
						gen7_get_Stream_TxnCnt(&gen7_udp_frame_hdr);
						gen7_CheckSameScanIdforChunk(&gen7_udp_frame_hdr, timestamp);
						//gen5_SensorCalibPktLossCheck(&gen5_udp_frame_hdr);
						gen7_Chunkslossinfo(gen7_udp_frame_hdr);
						Gen7_CompleteStreamlossinfo(gen7_udp_frame_hdr);
					}
				}
				else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
				{
					//if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode) {
					status = GEN5_GetPreviousData(&gen5_udp_frame_hdr, LogFname, cust_id);
					//}
					gen5_get_CDC_Max_Chunk(&gen5_udp_frame_hdr);
					gen5_get_Stream_TxnCnt(&gen5_udp_frame_hdr);
					gen5_CheckSameScanIdforChunk(&gen5_udp_frame_hdr, timestamp);
					gen5_SensorCalibPktLossCheck(&gen5_udp_frame_hdr);
					gen5_Chunkslossinfo(gen5_udp_frame_hdr);
					Gen5_CompleteStreamlossinfo(gen5_udp_frame_hdr);
				}
				else
				{
					//if (MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode /*&& stream_no != 4*/) {
					status = GetPreviousData(&udp_frame_hdr, LogFname, cust_id);
					//}

					get_Source_TxnCnt(&udp_frame_hdr, cust_id);
					get_Stream_TxnCnt(&udp_frame_hdr);
					CheckSameScanIdforChunk(&udp_frame_hdr, timestamp);
					SensorCalibPktLossCheck(&udp_frame_hdr, &cust_frame_hdr);
					Chunkslossinfo(udp_frame_hdr, cust_frame_hdr);
				}
			}
		}
		// code through test  scanindex miss/continuous miss 
		//if (udp_frame_hdr.streamNumber == 6 && /*udp_frame_hdr.streamChunkIdx == 3&& */sourceId == SRR_FL && (udp_frame_hdr.streamRefIndex > 26070 && udp_frame_hdr.streamRefIndex < 26075))
		//{
		//	return 0;
		//}


		// code through test  end

		if ((MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) && MUDP_master_config->MUDP_input_path == CONTINUOUS_Mode && MUDP_master_config->data_extracter_mode[XML] == 0 && MUDP_master_config->data_extracter_mode[CSV] == 0)
		{
			if (Enddataflag[sourceId] == 0 && status != Status_Latch_Completed)
			{
				if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
					status = m_pDecoderPlugin->SRR7_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
				else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
					status = m_pDecoderPlugin->SRR6_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
				else
					status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);

			}
		}
		else
		{
			if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
				status = m_pDecoderPlugin->SRR7_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
			else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
				status = m_pDecoderPlugin->SRR6_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
			else
				status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream, timestamp, &source_no, &stream_no, format);
		}


		if ((pRec->payload[0] == 0xA3 && pRec->payload[1] == 0x18) || (pRec->payload[0] == 0x18 && pRec->payload[1] == 0xA3) || (pRec->payload[0] == 0xA1 && pRec->payload[1] == 0x18) || (pRec->payload[0] == 0xA2 && pRec->payload[1] == 0x18))
		{
			ScanIndex_Count(&udp_frame_hdr);
		}

		if (status != Status_Latch_Completed)
			return 0;

		Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
		GEN5_Radar_UDP_Frame_Latch_T* p_gen5_latch_out = NULL;
		GEN7_Radar_UDP_Frame_Latch_T* p_gen7_latch_out = NULL;
		unsigned char version = 0;

		if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
		{
			if (stream_no >= DETECTION_STREAM || stream_no <= ROT_PROCESSED_DETECTION_STREAM)
				status = m_pDecoderPlugin->SRR7_API_GetLastLatchData(&p_gen7_latch_out, (unsigned char)source_no, (unsigned char)stream_no);
			else {
				return 0;
			}
		}
		else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
		{
			if (stream_no == C0_CORE_MASTER_STREAM || stream_no == C0_CUST_MASTER_STREAM || stream_no == C1_CORE_MASTER_STREAM || stream_no == C2_CORE_MASTER_STREAM || stream_no == C2_CUST_MASTER_STREAM || stream_no == CDC_GEN5_DATA
				|| (stream_no == e_BC_CORE_LOGGING_STREAM) || (stream_no == e_VSE_CORE_LOGGING_STREAM) || (stream_no == e_STATUS_CORE_LOGGING_STREAM) || (stream_no == e_RDD_CORE_LOGGING_STREAM) || (stream_no == e_DET_LOGGING_STREAM) 
				|| (stream_no == e_HDR_LOGGING_STREAM) || (stream_no == e_TRACKER_LOGGING_STREAM) || (stream_no == e_DEBUG_LOGGING_STREAM) || (stream_no == e_ALIGNMENT_LOGGING_STREAM))
				status = m_pDecoderPlugin->SRR6_API_GetLastLatchData(&p_gen5_latch_out, (unsigned char)source_no, (unsigned char)stream_no);
			else {
				return 0;
			}
		}
		else if ((pRec->payload[0] == 0xA3 && pRec->payload[1] == 0x18) || (pRec->payload[0] == 0x18 && pRec->payload[1] == 0xA3) || (pRec->payload[0] == 0xA1 && pRec->payload[1] == 0x18) || (pRec->payload[0] == 0xA2 && pRec->payload[1] == 0x18))
		{
			if (stream_no == DSPACE_CUSTOMER_DATA || stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA || stream_no == CALIBRATION_DATA || stream_no == Z4_CUSTOM || stream_no == Z4_LOGGING_DATA || stream_no == CDC_DATA
				|| stream_no == SENSOR_OSI_TRACKER || stream_no == SRR3_Z7B_CUST_LOGGING_DATA || stream_no == Z4_CUST_LOGGING_DATA || stream_no == RADAR_ECU_CORE_0 || stream_no == RADAR_ECU_CORE_1 || stream_no == RADAR_ECU_CORE_3 || stream_no == RADAR_ECU_CALIB || stream_no == RADAR_ECU_VRU_CLASSIFIER) {
				status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out, (unsigned char)source_no, (unsigned char)stream_no);
			}
			else
			{
				return 0;
			}
		}
		else {
			return 0;
		}

		if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)

		{

			if ((MUDP_master_config->MUDP_Select_Sensor_Status[sourceId] == 1) || (MUDP_master_config->MUDP_Radar_ECU_status == 1))
			{
				if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
				{
					unsigned int ScanId = p_gen7_latch_out->proc_info.frame_header.streamRefIndex;

					GEN7_ScanIndex_Count(ScanId, sourceId, stream_no, p_gen7_latch_out->proc_info.frame_header.sourceInfo);

					GEN7_get_scanindex_info(p_gen7_latch_out->proc_info.frame_header.sourceTxTime, ScanId, sourceId, stream_no, p_gen7_latch_out->proc_info.frame_header.sourceInfo);

					if (p_gen7_latch_out->proc_info.f_failure_block == 1 && stream_no != CDC_GEN7_STREAM) {
						gen7_Get_PacketLossCount(p_gen7_latch_out, sourceId, stream_no);
					}
					if (stream_no == CDC_DATA)
					{
						gen7_CDC_PacketLossCount(p_gen7_latch_out, sourceId);
					}
				}
				else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
				{
					unsigned int ScanId = p_gen5_latch_out->proc_info.frame_header.scan_index;
					GEN5_ScanIndex_Count(ScanId, sourceId, stream_no, p_gen5_latch_out->proc_info.frame_header.Platform);
					get_scanindex_info(p_gen5_latch_out->proc_info.frame_header.timestamp, ScanId, sourceId, stream_no, p_gen5_latch_out->proc_info.frame_header.Platform);
					if (p_gen5_latch_out->proc_info.f_failure_block == 1 && stream_no != CDC_DATA) {
						gen5_Get_PacketLossCount(p_gen5_latch_out, sourceId, stream_no);
					}
					if (stream_no == CDC_DATA)
					{
						gen5_CDC_PacketLossCount(p_gen5_latch_out, sourceId);
					}

					////Synchronization Check for CDC and UDP
					//static int test_synchronization[10][MAX_STRM_COUNT];
					//static int prev_scan[MAX_RADAR_COUNT];
					//int radar_pos = (gen5_udp_frame_hdr.Radar_Position) - 1;
					//int stream_id = gen5_udp_frame_hdr.stream_Id;
					//int scanindex = gen5_udp_frame_hdr.scan_index;
					//std::string posName = GetRadarPosName(radar_pos);
					//if (stream_id != C2_CUST_MASTER_STREAM)
					//{
					//	test_synchronization[radar_pos][stream_id] = ScanId;
					//}
					//if (stream_id == C2_CUST_MASTER_STREAM)
					//{
					//	test_synchronization[radar_pos][stream_id] = ScanId;
					//	if (test_synchronization[radar_pos][C0_CORE_MASTER_STREAM] != test_synchronization[radar_pos][C1_CORE_MASTER_STREAM] || test_synchronization[radar_pos][C0_CORE_MASTER_STREAM] != test_synchronization[radar_pos][C2_CORE_MASTER_STREAM] || test_synchronization[radar_pos][C0_CORE_MASTER_STREAM] != test_synchronization[radar_pos][C2_CUST_MASTER_STREAM])
					//	{
					//		if (test_synchronization[radar_pos][C0_CORE_MASTER_STREAM] == 0 || test_synchronization[radar_pos][C1_CORE_MASTER_STREAM] == 0 || test_synchronization[radar_pos][C2_CORE_MASTER_STREAM] == 0 || test_synchronization[radar_pos][C2_CUST_MASTER_STREAM] == 0)
					//		{
					//		}
					//		else
					//		{
					//			fprintf(fCreate, "\t\t=========================================================================================================\n");
					//			fprintf(fCreate, "\t\t[%s]:Scan Index_%u, CDC and UDP are not in Synchronization. \n", GetRadarPosName(radar_pos), ScanId);

					//			int diff_scanindex = test_synchronization[radar_pos][stream_id] - prev_scan[radar_pos];
					//			if (diff_scanindex == 1 || diff_scanindex == 0)
					//			{
					//				loggedmap[logfilename].BMW_Status = "FAIL";
					//				loggedmap[logfilename].Observation += "CDC and UDP Synchronization mismatch found in sensor[" + posName + "] At scanindex - " + to_string(ScanId) + " -> Timestamp :" + to_string(gen5_udp_frame_hdr.timestamp) + " .";
					//			}
					//			prev_scan[radar_pos] = scanindex;
					//		}						
					//	}
					//}
				}
				else
				{
					unsigned int ScanId = p_latch_out->proc_info.frame_header.streamRefIndex;
					get_scanindex_info(p_latch_out->proc_info.cust_frame_header.timestamp, ScanId, sourceId, stream_no, p_latch_out->proc_info.frame_header.Platform);
					if (p_latch_out->proc_info.f_failure_block == 1 && stream_no != CDC_DATA) {
						Get_PacketLossCount(p_latch_out, sourceId, stream_no);
					}
					if (stream_no == CDC_DATA)
					{
						CDC_PacketLossCount(p_latch_out, sourceId);
					}

					//Synchronization Check for CDC and UDP
					//static int test_synchronization[10][MAX_STRM_COUNT];
					//static int prev_scan[MAX_RADAR_COUNT];
					//int radar_pos = (udp_frame_hdr.Radar_Position) - 1;
					//int stream_id = MaptostreamNumber(udp_frame_hdr.streamNumber, udp_frame_hdr.customerID);//udp_frame_hdr.streamNumber;
					//int scanindex = udp_frame_hdr.streamRefIndex;
					//std::string posName = GetRadarPosName(radar_pos);
					//if (stream_id != Z4_CUSTOM)
					//{
					//	test_synchronization[radar_pos][stream_id] = ScanId;
					//}
					//if (stream_id == Z4_CUSTOM)
					//{
					//	test_synchronization[radar_pos][stream_id] = ScanId;
					//	if (test_synchronization[radar_pos][Z7B] != test_synchronization[radar_pos][Z7A]
					//		|| test_synchronization[radar_pos][Z7B] != test_synchronization[radar_pos][Z4]
					//		|| test_synchronization[radar_pos][Z7B] != (test_synchronization[radar_pos][CDC]) + 1
					//		|| test_synchronization[radar_pos][Z7B] != test_synchronization[radar_pos][Z4_CUSTOM]
					//		|| test_synchronization[radar_pos][Z7B] != test_synchronization[radar_pos][Z7B_CUSTOM])
					//	{
					//		if (test_synchronization[radar_pos][Z7A] == 0 || test_synchronization[radar_pos][Z7B] == 0 || test_synchronization[radar_pos][Z4_CUSTOM] == 0 || test_synchronization[radar_pos][Z7B_CUSTOM] == 0 || test_synchronization[radar_pos][Z4] == 0)
					//		{
					//		}
					//		else
					//		{
					//			fprintf(fCreate, "\t\t=========================================================================================================\n");
					//			fprintf(fCreate, "\t\t[%s]:Scan Index_%u, CDC and UDP are not in Synchronization. \n", GetRadarPosName(radar_pos), ScanId);

					//			int diff_scanindex = test_synchronization[radar_pos][stream_id] - prev_scan[radar_pos];
					//			if (diff_scanindex == 1 || diff_scanindex == 0)
					//			{
					//				loggedmap[logfilename].BMW_Status = "FAIL";
					//				loggedmap[logfilename].Observation += "CDC and UDP Synchronization mismatch found in sensor[" + posName + "] At scanindex - " + to_string(ScanId) + " -> Timestamp :" + to_string(gen5_udp_frame_hdr.timestamp) + " .";
					//			}
					//			prev_scan[radar_pos] = scanindex;
					//		}

					//	}
					//}
				}
			}
		}
	

	if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
	{
		cust_id = PLATFORM_GEN7; //(Customer_T)p_gen7_latch_out->proc_info.frame_header.customerId;
	}
	else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
	{
		cust_id = (Customer_T)p_gen5_latch_out->proc_info.frame_header.customerId;
	}
	else
	{
		cust_id = (Customer_T)p_latch_out->proc_info.frame_header.customerID;
	}
	
	if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
	{
		if ((pRec->payload[0] != 0xA4 && pRec->payload[1] != 0x2C) && (pRec->payload[0] != 0x2C && pRec->payload[1] != 0xA4) 
			&& (pRec->payload[0] != 0xA5 && pRec->payload[1] != 0x1C) && (pRec->payload[0] != 0x1C && pRec->payload[1] != 0xA5))
		{
			if (stream_no == Z7B_LOGGING_DATA)
			{
				if (!g_pIRadarStrm[stream_no])
				{
					//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
					g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
				}



			}

			else if (stream_no == Z7A_LOGGING_DATA) {
				if (!g_pIRadarStrm[stream_no])
				{
					//	g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
					g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
				}
			}
			else if (stream_no == RADAR_ECU_CORE_1)
			{
				if (!g_pIRadarStrm[stream_no])
				{
					//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_C1, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
					g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, ECU_C1, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
				}

			}
			if (g_pIRadarStrm[stream_no] != 0)
			{
				//	g_pIRadarStrm[stream_no]->FillStreamData(&p_latch_out->data[0], p_latch_out->proc_info.nDataLen, &p_latch_out->proc_info);
				m_radar_plugin->RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_latch_out);
				unsigned int ScanId = p_latch_out->proc_info.frame_header.streamRefIndex;
				if (stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA || stream_no == RADAR_ECU_CORE_1)
				{
					GetTimingOverrunInfo(g_pIRadarStrm[stream_no], stream_no, ScanId, sourceId);
				}

			}
		}
	}


	if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 ||MUDP_master_config->data_extracter_mode[XML] == 1)
	{
		if (MUDP_master_config->Radar_Stream_Options[OSI_Stream] == 1)
		{
			if ((once_list[sourceId][stream_no] == 0) && (Streamver.OSI_STREAM == UDP_OSI && stream_no == SENSOR_OSI_TRACKER || stream_no == OSI_GEN6_STREAM) &&
				(pRec->payload[2] != UDP_PLATFORM_FLR4_PLUS && pRec->payload[2] != UDP_PLATFORM_FLR4_PLUS_STANDALONE))
			{
				Print_OSI_INPUT_STRUCTURE_Traces_Header(fpMF4Dump[sourceId][Streamver.OSI_STREAM], NULL);
				once_list[sourceId][stream_no] = 1;
			}
			if ((Streamver.OSI_STREAM == UDP_OSI) && (stream_no == SENSOR_OSI_TRACKER || stream_no == OSI_GEN6_STREAM))
			{
				Debug = new Ref_Logging_Data_T;
				if (Debug) {
					if ((pRec->payload[0] != 0xA4 && pRec->payload[1] != 0x2C) && (pRec->payload[0] != 0x2C && pRec->payload[1] != 0xA4))
					{
						Debug = (Ref_Logging_Data_T*)p_latch_out->data;
						fprintf(fpMF4Dump[sourceId][Streamver.OSI_STREAM], "\n");
						fprintf(fpMF4Dump[sourceId][Streamver.OSI_STREAM], "%u,", p_latch_out->proc_info.frame_header.streamRefIndex);
						Print_OSI_INPUT_STRUCTURE_Traces_Data(fpMF4Dump[sourceId][Streamver.OSI_STREAM], Debug);
					}
					else
					{
						Debug = (Ref_Logging_Data_T*)p_gen5_latch_out->data;
						fprintf(fpMF4Dump[sourceId][Streamver.OSI_STREAM], "\n");
						fprintf(fpMF4Dump[sourceId][Streamver.OSI_STREAM], "%u,", p_gen5_latch_out->proc_info.frame_header.scan_index);
						Print_OSI_INPUT_STRUCTURE_Traces_Data(fpMF4Dump[sourceId][Streamver.OSI_STREAM], Debug);
					}
				}
				if (Debug)
				{
					Debug = {};
					delete Debug;
				}
			}

		}
		if ((pRec->payload[0] == 0xA5 && pRec->payload[1] == 0x1C) || (pRec->payload[0] == 0x1C && pRec->payload[1] == 0xA5))
		{
			bool set_flag = Gen7_Dump_Option_Fun(stream_no, Streamver);
			platform = pRec->payload[8];
			//XML Traces Flag
			if (MUDP_master_config->data_extracter_mode[XML] == 1 && MUDP_master_config->data_extracter_mode[CSV] != 1)
			{
				if (stream_no == DETECTION_STREAM || stream_no == HEADER_STREAM || stream_no == STATUS_STREAM || stream_no == RDD_STREAM || stream_no == VSE_STREAM || stream_no == DYNAMIC_ALIGNMENT_STREAM ||
					stream_no == DEBUG_STREAM || stream_no == MMIC_STREAM || stream_no == RADAR_CAPABILITY_STREAM || stream_no == BLOCKAGE_STREAM || stream_no == DOWN_SELECTION_STREAM || stream_no == ID_STREAM || stream_no == TOI_STREAM || stream_no == ROT_VEHICLE_INFO_STREAM || stream_no == ROT_SAFETY_FAULTS_STREAM)
				{

					set_flag = TRUE;
					f_gen7_id_enabled = TRUE;
					f_gen7_toi_enabled = TRUE;
					f_gen7_radar_capability_enabled = TRUE;
					f_det_core_enabled = TRUE;
					f_hdr_core_enabled = TRUE;
					f_vse_core_enabled = TRUE;
					f_gen7_mmic_enabled = TRUE;
					f_gen7_alignment_enabled = TRUE;
					f_gen7_dynamic_alignment_enabled = TRUE;
					f_gen7_blockage_enabled = TRUE;
					f_hdr_core_enabled = TRUE;
					f_debug_core_enabled = TRUE;
					f_gen7_ROT_vehicle_info_enabled = TRUE;
					f_gen7_ROT_safety_faults_enabled = TRUE;

				}
			}
			
			if (set_flag == TRUE)
			{ 
				if (platform == UDP_PLATFORM_SRR7_PLUS || platform == UDP_PLATFORM_SRR7_PLUS_HD || platform == UDP_PLATFORM_SRR7_E)
				{
					if (f_det_core_enabled && stream_no == DETECTION_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, DETECTION_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_rdd_core_enabled && stream_no == RDD_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, RDD_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_hdr_core_enabled && stream_no == HEADER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, HEADER_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_status_core_enabled && stream_no == STATUS_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, STATUS_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_debug_core_enabled && stream_no == DEBUG_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, DEBUG_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}

					//GEN7 CDC Data Dump
					if (f_cdc_enabled && stream_no == CDC_GEN7_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, CDC_GEN7_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					
					if (f_vse_core_enabled && stream_no == VSE_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, VSE_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_mmic_enabled && stream_no == MMIC_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, MMIC_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_alignment_enabled && stream_no == ALIGNMENT_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ALIGNMENT_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_blockage_enabled && stream_no == BLOCKAGE_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, BLOCKAGE_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
				/*	if (f_gen7_calib_enabled && stream_no == CALIB_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, CALIB_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					*/
					if (f_gen7_radar_capability_enabled && stream_no == RADAR_CAPABILITY_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, RADAR_CAPABILITY_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_down_selection_enabled && stream_no == DOWN_SELECTION_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, DOWN_SELECTION_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_id_enabled && stream_no == ID_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ID_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_toi_enabled && stream_no == TOI_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, TOI_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_safety_faults_enabled && stream_no == ROT_SAFETY_FAULTS_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_SAFETY_FAULTS_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_vehicle_info_enabled && stream_no == ROT_VEHICLE_INFO_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_VEHICLE_INFO_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_tracker_info_enabled && stream_no == ROT_TRACKER_INFO_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_TRACKER_INFO_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_object_stream_enabled && stream_no == ROT_OBJECT_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_OBJECT_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_ROT_ISO_object_stream_enabled && stream_no == ROT_ISO_OBJECT_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_ISO_OBJECT_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_gen7_ROT_processed_detection_stream_enabled && stream_no == ROT_PROCESSED_DETECTION_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, ROT_PROCESSED_DETECTION_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}

					if (f_gen7_alignment_enabled && stream_no == DYNAMIC_ALIGNMENT_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN7RadarSilCreateStream(p_gen7_latch_out->proc_info.frame_header.sourceInfo, DYNAMIC_ALIGNMENT_STREAM, p_gen7_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}


					//if (f_cdc_enabled && stream_no == CDC_GEN5_DATA) {
					//	if (!g_pIRadarStrm[stream_no])
					//	{
					//		//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
					//		g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, CDC_GEN5_DATA, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					//	}
					//}
				}

				/*if (!g_pIRadarStrm[stream_no]) {
					fprintf(stderr, "Error! No suitable decoder found for platform = %u, stream = %u , version = %u\n", p_gen5_latch_out->proc_info.frame_header.Platform, stream_no,
						p_gen5_latch_out->proc_info.frame_header.streamVersion);
					return -1;
				}*/
				if (Streamver.DET_CORE == UDP_GEN5_DET_CORE || Streamver.VSE_CORE == UDP_GEN5_VSE_CORE || Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE || Streamver.RDD_CORE == UDP_GEN5_RDD_CORE
					|| Streamver.HDR_CORE == UDP_GEN5_HDR_CORE || Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE || Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE
					|| Streamver.MMIC_CORE == UDP_GEN7_MMIC_CORE || Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE || Streamver.BLOCKAGE_CORE == UDP_GEN7_BLOCKAGE_CORE
					|| Streamver.RADAR_CAPABILITY_CORE == UDP_GEN7_RADAR_CAPABILITY_CORE || Streamver.DOWN_SELECTION_CORE == UDP_GEN7_DOWN_SELECTION_CORE
					|| Streamver.ID_CORE == UDP_GEN7_ID_CORE || Streamver.TOI_CORE == UDP_GEN7_TOI_CORE || Streamver.DYNAMIC_ALIGNMENT_CORE == UDP_GEN7_DYNAMIC_ALIGNMENT_CORE || Streamver.ROT_SAFETY_FAULTS_CORE == UDP_GEN7_ROT_SAFETY_FAULTS_CORE || Streamver.ROT_VEHICLE_INFO_CORE == UDP_GEN7_ROT_VEHICLE_INFO_CORE)
				{
					e_Options = ALL;
				}

				if (mf4flag == true) {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1/* || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1*/)
					{
						if ((once_list[sourceId][stream_no] == 0) && ((Streamver.DET_CORE == UDP_GEN5_DET_CORE && stream_no == DETECTION_STREAM) || (Streamver.CDC == UDP_CDC_STRM && stream_no == CDC_STREAM)
							|| (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE && stream_no == HEADER_STREAM) || (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE && stream_no == STATUS_STREAM) ||
							(Streamver.RDD_CORE == UDP_GEN5_RDD_CORE && stream_no == RDD_STREAM) || (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE && stream_no == VSE_STREAM) ||
							(Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE && stream_no == DEBUG_STREAM) || (Streamver.CDC == UDP_CDC_STRM && stream_no == CDC_GEN7_STREAM) ||
							(Streamver.MMIC_CORE == UDP_GEN7_MMIC_CORE && stream_no == MMIC_STREAM) || (Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE && stream_no == ALIGNMENT_STREAM) ||
							(Streamver.BLOCKAGE_CORE == UDP_GEN7_BLOCKAGE_CORE && stream_no == BLOCKAGE_STREAM) || (Streamver.RADAR_CAPABILITY_CORE == UDP_GEN7_RADAR_CAPABILITY_CORE && stream_no == RADAR_CAPABILITY_STREAM) ||
							(Streamver.DOWN_SELECTION_CORE == UDP_GEN7_DOWN_SELECTION_CORE && stream_no == DOWN_SELECTION_STREAM) || (Streamver.ID_CORE == UDP_GEN7_ID_CORE && stream_no == ID_STREAM) ||
							(Streamver.TOI_CORE == UDP_GEN7_TOI_CORE && stream_no == TOI_STREAM) || (Streamver.DYNAMIC_ALIGNMENT_CORE == UDP_GEN7_DYNAMIC_ALIGNMENT_CORE && stream_no == DYNAMIC_ALIGNMENT_STREAM) || (Streamver.ROT_SAFETY_FAULTS_CORE == UDP_GEN7_ROT_SAFETY_FAULTS_CORE && stream_no == ROT_SAFETY_FAULTS_STREAM) || 
							(Streamver.ROT_VEHICLE_INFO_CORE == UDP_GEN7_ROT_VEHICLE_INFO_CORE && stream_no == ROT_VEHICLE_INFO_STREAM) || (Streamver.ROT_TRACKER_INFO_CORE == UDP_GEN7_ROT_TRACKER_INFO_CORE && stream_no == ROT_TRACKER_INFO_STREAM) || (Streamver.ROT_OBJECT_CORE == UDP_GEN7_ROT_OBJECT_CORE && stream_no == ROT_OBJECT_STREAM) ||
							(Streamver.ROT_ISO_OBJECT_CORE == UDP_GEN7_ROT_ISO_OBJECT_CORE && stream_no == ROT_ISO_OBJECT_STREAM) || (Streamver.ROT_PROCESSED_DETECTION_CORE == UDP_GEN7_ROT_PROCESSED_DETECTION_CORE && stream_no == ROT_PROCESSED_DETECTION_STREAM)))

						{
							//bool set_flag = Gen7_Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								if (g_pIRadarStrm[stream_no])
								{
									m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								}
							}
							once_list[sourceId][stream_no] = 1;
						}

					}
				}
				else {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1/* || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1*/)
					{
						if ((once[sourceId][stream_no] == 0) && ((Streamver.DET_CORE == UDP_GEN5_DET_CORE && stream_no == DETECTION_STREAM) || (Streamver.CDC == UDP_CDC_STRM && stream_no == CDC_STREAM)
							|| (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE && stream_no == HEADER_STREAM) || (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE && stream_no == STATUS_STREAM) ||
							(Streamver.RDD_CORE == UDP_GEN5_RDD_CORE && stream_no == RDD_STREAM) || (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE && stream_no == VSE_STREAM) ||
							(Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE && stream_no == DEBUG_STREAM) || (Streamver.CDC == UDP_CDC_STRM && stream_no == CDC_GEN7_STREAM) ||
							(Streamver.MMIC_CORE == UDP_GEN7_MMIC_CORE && stream_no == MMIC_STREAM) || (Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE && stream_no == ALIGNMENT_STREAM) ||
							(Streamver.BLOCKAGE_CORE == UDP_GEN7_BLOCKAGE_CORE && stream_no == BLOCKAGE_STREAM) || (Streamver.RADAR_CAPABILITY_CORE == UDP_GEN7_RADAR_CAPABILITY_CORE && stream_no == RADAR_CAPABILITY_STREAM) ||
							(Streamver.DOWN_SELECTION_CORE == UDP_GEN7_DOWN_SELECTION_CORE && stream_no == DOWN_SELECTION_STREAM) || (Streamver.ID_CORE == UDP_GEN7_ID_CORE && stream_no == ID_STREAM) ||
							(Streamver.TOI_CORE == UDP_GEN7_TOI_CORE && stream_no == TOI_STREAM) || (Streamver.DYNAMIC_ALIGNMENT_CORE == UDP_GEN7_DYNAMIC_ALIGNMENT_CORE && stream_no == DYNAMIC_ALIGNMENT_STREAM)
							|| (Streamver.ROT_SAFETY_FAULTS_CORE == UDP_GEN7_ROT_SAFETY_FAULTS_CORE && stream_no == ROT_SAFETY_FAULTS_STREAM) || (Streamver.ROT_VEHICLE_INFO_CORE == UDP_GEN7_ROT_VEHICLE_INFO_CORE && stream_no == ROT_VEHICLE_INFO_STREAM))
							|| (Streamver.ROT_TRACKER_INFO_CORE == UDP_GEN7_ROT_TRACKER_INFO_CORE && stream_no == ROT_TRACKER_INFO_STREAM)|| (Streamver.ROT_OBJECT_CORE == UDP_GEN7_ROT_OBJECT_CORE && stream_no == ROT_OBJECT_STREAM)||
							(Streamver.ROT_ISO_OBJECT_CORE == UDP_GEN7_ROT_ISO_OBJECT_CORE && stream_no == ROT_ISO_OBJECT_STREAM)|| (Streamver.ROT_PROCESSED_DETECTION_CORE == UDP_GEN7_ROT_PROCESSED_DETECTION_CORE && stream_no == ROT_PROCESSED_DETECTION_STREAM))
						{
							//bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								if (g_pIRadarStrm[stream_no])
								{
									m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								}
							}
							once[sourceId][stream_no] = 1;
						}
					}
				}
				int status = -1;
				if (g_pIRadarStrm[stream_no])
				{
					status = m_radar_plugin->GEN7RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_gen7_latch_out);
				}
				//status = m_radar_plugin->GEN5RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_gen5_latch_out);

				if (status == -1)
				{
					if (g_pIRadarStrm[stream_no])
					{
						setstreammismatchcount(stream_no);
						//printf("\nstream number %d size mismatch\n", stream_no);
						return 0;
					}
					/*printf("\nstream number %d size mismatch\n", stream_no);
					return 0;*/
				}

				if (MUDP_master_config->data_extracter_mode[CSV] == 1/* || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1*/)
				{
					//bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
					if (set_flag == TRUE)
					{
						int ret = GetNextLogname(sourceId, StrmValue, stream_no);
						if (ret != 0)
							return ret;
						if (g_pIRadarStrm[stream_no])
						{
							m_radar_plugin->RadarMudpPrintData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], CSVFName[sourceId][StrmValue], e_Options);
						}
					}
				}
				//XML Traces for GEN7 
				if (MUDP_master_config->data_extracter_mode[XML] == 1)
				{//sfpxml
					if (CreatxmlOnce == false && sourceId != RDR_POS_ECU)
					{
						bool set_flag = Gen7_Dump_Option_Fun(stream_no, Streamver);
						if (set_flag == TRUE)
						{ //HTMLInMUDP
							if (sptr_manager == nullptr)
							{
								if (sfpxml != NULL)
								{
									Gen7_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform);
								}
								if (pxmlFile[sourceId] != NULL)
									Gen7_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform);
							}
							else
							{
								std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
								if (g_pIRadarStrm[stream_no] != NULL)
								{
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform, 0);
								}
							}
					}
						else if ((MUDP_master_config->Radar_Stream_Options[HDR] == 0) && (MUDP_master_config->Radar_Stream_Options[TOI_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[MMIC_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[ALIGNMENT_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[BLOCKAGE_CORE] == 0)
							&& (MUDP_master_config->Radar_Stream_Options[RADAR_CAPABILITY_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DOWN_SELECTION_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[C2_Cust] == 0) && (MUDP_master_config->Radar_Stream_Options[BC_CORE] == 0)
							&& (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DET_CORE] == 0)
							&& (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[MAX_STREAM] == 0))
						{
							if (sptr_manager == nullptr)//#HTMLInMUDP
							{
								if (sfpxml != NULL)
								{
									Gen7_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform);
								}
								if (pxmlFile[sourceId] != NULL)
									Gen7_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform);

							}
							else
							{
								std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
								rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen7_latch_out->proc_info.frame_header.streamRefIndex, platform, 0);

							}
						}
					}
				}

				//if (MUDP_master_config->data_extracter_mode[XML] == 1)
				//{//sfpxml
				//	if (CreatxmlOnce == false && sourceId != RDR_POS_ECU)
				//	{
				//		bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
				//		if (set_flag == TRUE)
				//		{
				//			if (sfpxml != NULL)
				//			{
				//				Gen5_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
				//			}
				//			if (pxmlFile[sourceId] != NULL)
				//				Gen5_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
				//		}
				//		else if ((MUDP_master_config->Radar_Stream_Options[HDR] == 0) && (MUDP_master_config->Radar_Stream_Options[CDC_iq] == 0) && (MUDP_master_config->Radar_Stream_Options[Z4_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[Z7A_Core] == 0)
				//			&& (MUDP_master_config->Radar_Stream_Options[Z7B_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[Z4_Customer] == 0) && (MUDP_master_config->Radar_Stream_Options[Z7B_Customer] == 0) && (MUDP_master_config->Radar_Stream_Options[CCA_HDR] == 0)
				//			&& (MUDP_master_config->Radar_Stream_Options[Dspace] == 0) && (MUDP_master_config->Radar_Stream_Options[OSI_Stream] == 0) && (MUDP_master_config->Radar_Stream_Options[C0_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C0_Cust] == 0)
				//			&& (MUDP_master_config->Radar_Stream_Options[C1_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C2_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C2_Cust] == 0) && (MUDP_master_config->Radar_Stream_Options[BC_CORE] == 0)
				//			&& (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DET_CORE] == 0)
				//			&& (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[MAX_STREAM] == 0))
				//		{
				//			if (sfpxml != NULL)
				//			{
				//				Gen5_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
				//			}
				//			if (pxmlFile[sourceId] != NULL)
				//				Gen5_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);

				//		}
				//	}
				//}
				
			}
		}
		else if ((pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C) || (pRec->payload[0] == 0x2C && pRec->payload[1] == 0xA4))
		{
			bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
			if (MUDP_master_config->data_extracter_mode[XML] == 1 && MUDP_master_config->data_extracter_mode[CSV] != 1)
			{
				if (stream_no == C0_CORE_MASTER_STREAM || stream_no == C0_CUST_MASTER_STREAM || stream_no == C1_CORE_MASTER_STREAM || stream_no == C2_CORE_MASTER_STREAM || stream_no == C2_CUST_MASTER_STREAM || 
					stream_no == e_BC_CORE_LOGGING_STREAM || stream_no == e_VSE_CORE_LOGGING_STREAM || stream_no == e_STATUS_CORE_LOGGING_STREAM || stream_no == e_RDD_CORE_LOGGING_STREAM 
					|| stream_no == e_DET_LOGGING_STREAM || stream_no == e_HDR_LOGGING_STREAM || stream_no == e_TRACKER_LOGGING_STREAM || stream_no == e_DEBUG_LOGGING_STREAM || stream_no == e_ALIGNMENT_LOGGING_STREAM)
				{
					set_flag = TRUE;
					f_z7b_enabled = TRUE;
					f_c0_core_enabled = TRUE;
					f_c0_cust_enabled = TRUE;
					f_c1_core_enabled = TRUE;
					f_c2_core_enabled = TRUE;
					f_c2_cust_enabled = TRUE;
					f_bc_core_enabled = TRUE;
					f_vse_core_enabled = TRUE;
					f_status_core_enabled = TRUE;
					f_rdd_core_enabled = TRUE;
					f_det_core_enabled = TRUE;
					f_hdr_core_enabled = TRUE;
					f_tracker_core_enabled = TRUE;
					f_debug_core_enabled = TRUE;
					f_gen7_alignment_enabled = TRUE;
					f_z7a_enabled = TRUE;
					f_z4_enabled = TRUE;
					f_z7b_custom_enabled = TRUE;
					f_z4_custom_enabled = TRUE;
					f_ECU0_enabled = TRUE;
					f_ECU1_enabled = TRUE;
					f_ECU3_enabled = TRUE;
					f_dspace_enabled = TRUE;
				}
			}

			if (set_flag == TRUE)
			{
				//Master streams 
				if (platform == UDP_PLATFORM_SRR6 || platform == UDP_PLATFORM_STLA_FLR4 || platform == UDP_PLATFORM_SRR6_PLUS || platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN
					|| platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH)
				{
					if (f_c0_core_enabled && stream_no == C0_CORE_MASTER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, C0_GEN5_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_c0_cust_enabled && stream_no == C0_CUST_MASTER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no])
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, C0_GEN5_CUST, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_c1_core_enabled && stream_no == C1_CORE_MASTER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, C1_GEN5_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_c2_core_enabled && stream_no == C2_CORE_MASTER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, C2_GEN5_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
							//Stream_cust_id = cust_id;
						}
					}
					if (f_c2_cust_enabled && stream_no == C2_CUST_MASTER_STREAM)
					{
						if (!g_pIRadarStrm[stream_no])
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, C2_GEN5_CUST, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_cdc_enabled && stream_no == CDC_GEN5_DATA) {
						if (!g_pIRadarStrm[stream_no])
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, CDC_GEN5_DATA, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
				}
				//component streams  - for FLR4P
				if (platform == UDP_PLATFORM_FLR4_PLUS || platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
				{
					if (f_bc_core_enabled && stream_no == e_BC_CORE_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_BC_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_vse_core_enabled && stream_no == e_VSE_CORE_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_VSE_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_status_core_enabled && stream_no == e_STATUS_CORE_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_STATUS_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_rdd_core_enabled && stream_no == e_RDD_CORE_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_RDD_CORE, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
							
						}
					}
					if (f_det_core_enabled && stream_no == e_DET_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_DET_LOGGING, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_hdr_core_enabled && stream_no == e_HDR_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_HDR_LOGGING, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_tracker_core_enabled && stream_no == e_TRACKER_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_TRACKER_LOGGING, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
					if (f_debug_core_enabled && stream_no == e_DEBUG_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_DEBUG_LOGGING, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
							//Stream_cust_id = cust_id;
						}
					}
					if (f_gen7_alignment_enabled && stream_no == e_ALIGNMENT_LOGGING_STREAM)
					{
						if (!g_pIRadarStrm[stream_no] || (Stream_cust_id != cust_id))
						{
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, e_ALIGNMENT_LOGGING, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
							//Stream_cust_id = cust_id;
						}
					}
					if (f_cdc_enabled && stream_no == CDC_GEN5_DATA) {
						if (!g_pIRadarStrm[stream_no])
						{
							//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
							g_pIRadarStrm[stream_no] = m_radar_plugin->GEN5RadarSilCreateStream(p_gen5_latch_out->proc_info.frame_header.Platform, CDC_GEN5_DATA, p_gen5_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						}
					}
				}
				/*if (!g_pIRadarStrm[stream_no]) {
					fprintf(stderr, "Error! No suitable decoder found for platform = %u, stream = %u , version = %u\n", p_gen5_latch_out->proc_info.frame_header.Platform, stream_no,
						p_gen5_latch_out->proc_info.frame_header.streamVersion);
					return -1;
				}*/
				if (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE || Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE || Streamver.RDD_CORE == UDP_GEN5_RDD_CORE || Streamver.DET_CORE == UDP_GEN5_DET_CORE
					|| Streamver.HDR_CORE == UDP_GEN5_HDR_CORE || Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE || Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE || Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE)
				{
					e_Options = ALL;
				}

				if (mf4flag == true) {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
					{
						if ((once_list[sourceId][stream_no] == 0) && ((Streamver.C0_Core == UDP_GEN5_C0_CORE &&  stream_no == C0_CORE_MASTER_STREAM) || (Streamver.C0_Cust == UDP_GEN5_C0_CUST  &&  stream_no == C0_CUST_MASTER_STREAM) ||
							(Streamver.C1_Core == UDP_GEN5_C1_CORE  &&  stream_no == C1_CORE_MASTER_STREAM) || (Streamver.C2_Core == UDP_GEN5_C2_CORE  &&  stream_no == C2_CORE_MASTER_STREAM) || (Streamver.C2_Cust == UDP_GEN5_C2_CUST &&  stream_no == C2_CUST_MASTER_STREAM)
							|| (Streamver.CDC == UDP_CDC_STRM  && stream_no == CDC_STREAM) || (Streamver.BC_CORE == UDP_GEN5_BC_CORE &&  stream_no == e_BC_CORE_LOGGING_STREAM) || (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE &&  stream_no == e_VSE_CORE_LOGGING_STREAM)
							|| (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE &&  stream_no == e_STATUS_CORE_LOGGING_STREAM) || (Streamver.RDD_CORE == UDP_GEN5_RDD_CORE &&  stream_no == e_RDD_CORE_LOGGING_STREAM) || (Streamver.DET_CORE == UDP_GEN5_DET_CORE &&  stream_no == e_DET_LOGGING_STREAM) 
							|| (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE &&  stream_no == e_HDR_LOGGING_STREAM) || (Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE &&  stream_no == e_TRACKER_LOGGING_STREAM) || (Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE && stream_no == e_DEBUG_LOGGING_STREAM)
							|| (Streamver.ALIGNMENT_CORE == UDP_GEN7_ALIGNMENT_CORE && stream_no == e_ALIGNMENT_LOGGING_STREAM)))

						{
							bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								
								if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 && stream_no == C2_CORE_MASTER_STREAM)
								{
									m_radar_plugin->RadarMudpPrintCsvHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][UDP_GEN5_C2_CORE], e_Options);
									
								}
								else 
								{
									if (g_pIRadarStrm[stream_no] && MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] != 1)
									{
										m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
									}
									//m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								}
								
							}


							once_list[sourceId][stream_no] = 1;
						}

					}
				}
				else {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
					{
						if ((once[sourceId][stream_no] == 0) && ((Streamver.C0_Core == UDP_GEN5_C0_CORE &&  stream_no == C0_CORE_MASTER_STREAM) || (Streamver.C0_Cust == UDP_GEN5_C0_CUST  &&  stream_no == C0_CUST_MASTER_STREAM)
							|| (Streamver.C1_Core == UDP_GEN5_C1_CORE &&  stream_no == C1_CORE_MASTER_STREAM) || (Streamver.C2_Core == UDP_GEN5_C2_CORE  &&  stream_no == C2_CORE_MASTER_STREAM) || (Streamver.C2_Cust == UDP_GEN5_C2_CUST &&  stream_no == C2_CUST_MASTER_STREAM)
							|| (Streamver.CDC == UDP_CDC_STRM  && stream_no == CDC_STREAM) || (Streamver.BC_CORE == UDP_GEN5_BC_CORE &&  stream_no == e_BC_CORE_LOGGING_STREAM) || (Streamver.VSE_CORE == UDP_GEN5_VSE_CORE &&  stream_no == e_VSE_CORE_LOGGING_STREAM)
							|| (Streamver.STATUS_CORE == UDP_GEN5_STATUS_CORE &&  stream_no == e_STATUS_CORE_LOGGING_STREAM) || (Streamver.RDD_CORE == UDP_GEN5_RDD_CORE &&  stream_no == e_RDD_CORE_LOGGING_STREAM)
							|| (Streamver.DET_CORE == UDP_GEN5_DET_CORE &&  stream_no == e_DET_LOGGING_STREAM) || (Streamver.HDR_CORE == UDP_GEN5_HDR_CORE &&  stream_no == e_HDR_LOGGING_STREAM) || (Streamver.TRACKER_CORE == UDP_GEN5_TRACKER_CORE &&  stream_no == e_TRACKER_LOGGING_STREAM)
							|| (Streamver.DEBUG_CORE == UDP_GEN5_DEBUG_CORE && stream_no == e_DEBUG_LOGGING_STREAM)))
						{
							bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
								{
									m_radar_plugin->RadarMudpPrintCsvHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][UDP_GEN5_C2_CORE], e_Options);
									
								}
								else
								{
									if (g_pIRadarStrm[stream_no])
									{
										m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
									}
									//m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								}
								
							}
							once[sourceId][stream_no] = 1;
						}
					}
				}
				int status = -1;
				if (g_pIRadarStrm[stream_no])
				{
					status = m_radar_plugin->GEN5RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_gen5_latch_out);
				}
				//status = m_radar_plugin->GEN5RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_gen5_latch_out);

				if (status == -1)
				{
					if (g_pIRadarStrm[stream_no])
					{	
						setstreammismatchcount(stream_no);
						//printf("\nstream number %d size mismatch\n", stream_no);
						return 0;
					}
					/*printf("\nstream number %d size mismatch\n", stream_no);
					return 0;*/
				}

				if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
				{
					bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
					if (set_flag == TRUE)
					{
						/**HONDA special: six parameters with scanindex from C2 core and MPRB calculations from C1 core printdata calling strat.**/
						if (MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1)
						{
							if (stream_no == C1_CORE_MASTER_STREAM)
							{
								set_once_flag =TRUE;
							}
							if (set_once_flag == TRUE)
							{
								if ((g_pIRadarStrm[C1_CORE_MASTER_STREAM] != 0) && (g_pIRadarStrm[C2_CORE_MASTER_STREAM] != 0))
								{
									m_radar_plugin->RadarMudpPrintCsvData(g_pIRadarStrm, C1_CORE_MASTER_STREAM, fpMF4Dump[sourceId][UDP_GEN5_C2_CORE], CSVFName[sourceId][UDP_GEN5_C2_CORE], e_Options);
									m_radar_plugin->RadarMudpPrintCsvData(g_pIRadarStrm, C2_CORE_MASTER_STREAM, fpMF4Dump[sourceId][UDP_GEN5_C2_CORE], CSVFName[sourceId][UDP_GEN5_C2_CORE], e_Options);
									set_once_flag = FALSE;
								}
								
							}
							
							/**HONDA special: six parameters with scanindex from C2 core and MPRB calculations from C1 core printdata calling end.**/
						}
						else
						{
							int ret = GetNextLogname(sourceId, StrmValue, stream_no);
							if (ret != 0)
								return ret;
							if (g_pIRadarStrm[stream_no])
							{
								m_radar_plugin->RadarMudpPrintData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], CSVFName[sourceId][StrmValue], e_Options);
							}
							//m_radar_plugin->RadarMudpPrintData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], CSVFName[sourceId][StrmValue], e_Options);
							
						}
					}
				}
				if (MUDP_master_config->data_extracter_mode[XML] == 1)
				{//sfpxml
					if (CreatxmlOnce == false && sourceId != RDR_POS_ECU)
					{
						bool set_flag = Gen5_Dump_Option_Fun(stream_no, Streamver);
						if (set_flag == TRUE)
						{//#HTMLInMUDP
							if (sptr_manager == nullptr)
							{
								if (sfpxml != NULL)
								{
									Gen5_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
								}
								if (pxmlFile[sourceId] != NULL)
									Gen5_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
							}
							else
							{
								std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
								rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform, 0);
							}
						}
						else if ((MUDP_master_config->Radar_Stream_Options[HDR] == 0) && (MUDP_master_config->Radar_Stream_Options[CDC_iq] == 0) && (MUDP_master_config->Radar_Stream_Options[Z4_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[Z7A_Core] == 0)
								&& (MUDP_master_config->Radar_Stream_Options[Z7B_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[Z4_Customer] == 0) && (MUDP_master_config->Radar_Stream_Options[Z7B_Customer] == 0) && (MUDP_master_config->Radar_Stream_Options[CCA_HDR] == 0)
								&& (MUDP_master_config->Radar_Stream_Options[Dspace] == 0) && (MUDP_master_config->Radar_Stream_Options[OSI_Stream] == 0) && (MUDP_master_config->Radar_Stream_Options[C0_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C0_Cust] == 0)
								&& (MUDP_master_config->Radar_Stream_Options[C1_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C2_Core] == 0) && (MUDP_master_config->Radar_Stream_Options[C2_Cust] == 0) && (MUDP_master_config->Radar_Stream_Options[BC_CORE] == 0)
								&& (MUDP_master_config->Radar_Stream_Options[VSE_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[STATUS_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[RDD_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DET_CORE] == 0)
								&& (MUDP_master_config->Radar_Stream_Options[HDR_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[TRACKER_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[DEBUG_CORE] == 0) && (MUDP_master_config->Radar_Stream_Options[MAX_STREAM] == 0))
						{
							if (sptr_manager == nullptr)//#HTMLInMUDP
							{
								if (sfpxml != NULL)
								{
									Gen5_Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
								}
								if (pxmlFile[sourceId] != NULL)
									Gen5_Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform);
							}
							else
							{
								std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
								rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id, p_gen5_latch_out->proc_info.frame_header.scan_index, platform, 0);

							}
						}
					}
				}
					/*if (CreateHilxmlOnce == false && sourceId != RDR_POS_ECU)
					{
						Populate_Valid_Data_to_XML(hxmlFile, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
					}*/
				//	if (CreatxmlOnce == false && sourceId == RDR_POS_ECU)
				//	{
				//		//if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] == 1 && stream_no == )
				//		if (pxmlFile[sourceId] != NULL)
				//			Populate_ECU_Data_to_XML(pxmlFile[sourceId], InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
				//		if (sfpxml != NULL)
				//			Populate_ECU_Data_to_XML(sfpxml, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
				//	}
				//	if (CreateHilxmlOnce == false && sourceId == RDR_POS_ECU)
				//	{
				//		Populate_ECU_Data_to_XML(hxmlFile, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
				//	}
				//}
			}
		}
		else
		{
			bool set_flag = Dump_Option_Fun(stream_no, Streamver);
			if (MUDP_master_config->data_extracter_mode[XML] == 1 && MUDP_master_config->data_extracter_mode[CSV] != 1)
			{
				if (stream_no == Z7B_LOGGING_DATA || stream_no == Z7A_LOGGING_DATA || stream_no == Z4_LOGGING_DATA || stream_no == SRR3_Z7B_CUST_LOGGING_DATA || stream_no == RADAR_ECU_CORE_0 || stream_no == RADAR_ECU_CORE_1 || stream_no == RADAR_ECU_CORE_3 || stream_no == DSPACE_CUSTOMER_DATA)
				{
					set_flag = TRUE;
					f_z7b_enabled = TRUE;
					f_c0_core_enabled = TRUE;
					f_c0_cust_enabled = TRUE;
					f_c1_core_enabled = TRUE;
					f_c2_core_enabled = TRUE;
					f_c2_cust_enabled = TRUE;
					f_z7a_enabled = TRUE;
					f_z4_enabled = TRUE;
					f_z7b_custom_enabled = TRUE;
					f_z4_custom_enabled = TRUE;
					f_ECU0_enabled = TRUE;
					f_ECU1_enabled = TRUE;
					f_ECU3_enabled = TRUE;
					f_dspace_enabled = TRUE;
				}
			}
			if (set_flag == TRUE)
			{
				if (f_z7b_enabled && stream_no == Z7B_LOGGING_DATA)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
						
					}
				}
				else if (f_z7a_enabled &&  stream_no == Z7A_LOGGING_DATA) {
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_z4_enabled &&  stream_no == Z4_LOGGING_DATA) {
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z4, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z4, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_cdc_enabled &&  stream_no == CDC_DATA) {
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_z7b_custom_enabled &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA) {
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B_CUSTOM, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z7B_CUSTOM, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_z4_custom_enabled &&  stream_no == Z4_CUST_LOGGING_DATA)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z4_CUSTOM, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, Z4_CUSTOM, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_ECU0_enabled &&  stream_no == RADAR_ECU_CORE_0)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_C0, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, ECU_C0, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_ECU1_enabled &&  stream_no == RADAR_ECU_CORE_1)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//	g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_C1, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, ECU_C1, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_ECU3_enabled &&  stream_no == RADAR_ECU_CORE_3)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_C3, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, ECU_C3, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_ECU_VRU_Classifier_enabled &&  stream_no == RADAR_ECU_VRU_CLASSIFIER)
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, ECU_VRUCLASSIFIER, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, ECU_VRUCLASSIFIER, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}
				else if (f_dspace_enabled && (stream_no == DSPACE_CUSTOMER_DATA || stream_no == 11))
				{
					if (!g_pIRadarStrm[stream_no])
					{
						//g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, R_DSPACE, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
						g_pIRadarStrm[stream_no] = m_radar_plugin->RadarSilCreateStream(p_latch_out->proc_info.frame_header.Platform, R_DSPACE, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
					}
				}

				if (!g_pIRadarStrm[stream_no]) {
					fprintf(stderr, "Error! No suitable decoder found for platform = %u, stream = %u , version = %u\n", p_latch_out->proc_info.frame_header.Platform, stream_no,
						p_latch_out->proc_info.frame_header.streamVersion);
					return -1;
				}
				if (Streamver.DSPACE == UDP_DSPACE || Streamver.ecu0 == UDP_RDR_ECU0 || Streamver.ecu1 == UDP_RDR_ECU1 || Streamver.ecu2 == UDP_RDR_ECU3 || Streamver.Z4_Core == UDP_Z4_CORE ||
					Streamver.Z4_Customer == UDP_CUSTOM_Z4 || Streamver.Z7A_Core == UDP_Z7A_CORE || Streamver.Z7B_Customer == UDP_CUSTOM_Z7B || Streamver.Z7B_Core == UDP_Z7B_CORE || Streamver.CDC == UDP_CDC_STRM || Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier ||
					Streamver.C0_Core == UDP_GEN5_C0_CORE || Streamver.C0_Cust == UDP_GEN5_C0_CUST || Streamver.C1_Core == UDP_GEN5_C1_CORE || Streamver.C2_Core == UDP_GEN5_C2_CORE || Streamver.C2_Cust == UDP_GEN5_C2_CUST)
				{
					e_Options = ALL;
				}
				/*		if ((Streamver.DSPACE == UDP_DSPACE) && (p_latch_out->proc_info.frame_header.Platform == UDP_SOURCE_CUST_DSPACE))
				{
				e_Options = DSPACE_TRK;
				}*/

				//switch (Dump_Opt)
				//{
				//case UDP_Z7B_CORE:
				//case UDP_Z7A_CORE:
				//case UDP_Z4_CORE:
				//case UDP_CDC_STRM:
				//case UDP_CUSTOM_Z7B:
				//case UDP_CUSTOM_Z4:
				//case UDP_RDR_ECU0:
				//case UDP_RDR_ECU1:
				//case UDP_RDR_ECU3:
				//case UDP_RDR_ECU_VRU_Classifier:
				//case UDP_DSPACE:
				//default:
				//	e_Options = ALL;
				//	break;
				//}

				// Streamver.C0_Core == UDP_GEN5_C0_CORE || Streamver.C0_Cust == UDP_GEN5_C0_CUST || Streamver.C1_Core == UDP_GEN5_C1_CORE || Streamver.C2_Core == UDP_GEN5_C2_CORE || Streamver.C2_Cust == UDP_GEN5_C2_CUST )
				if (mf4flag == true) {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1)
					{
						if ((once_list[sourceId][stream_no] == 0) && ((Streamver.Z7B_Core == UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA) || (Streamver.Z7A_Core == UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA) || (Streamver.Z4_Core == UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA) || (Streamver.Z7B_Customer == UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA) || (Streamver.Z4_Customer == UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
							|| (Streamver.CDC == UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Streamver.DSPACE == UDP_DSPACE && (stream_no == DSPACE_CUSTOMER_DATA || stream_no == 11)) || (Streamver.ecu0 == UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Streamver.ecu1 == UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Streamver.ecu2 == UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3) || (Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier && stream_no == RADAR_ECU_VRU_CLASSIFIER)))

						{
							bool set_flag = Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								//m_radar_plugin->RadarMudpPrintCsvHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
							}

							once_list[sourceId][stream_no] = 1;
						}

					}
				}
				else {
					if (MUDP_master_config->data_extracter_mode[CSV] == 1)
					{
						if ((once[sourceId][stream_no] == 0) && ((Streamver.Z7B_Core == UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA) || (Streamver.Z7A_Core == UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA) || (Streamver.Z4_Core == UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA) || (Streamver.Z7B_Customer == UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA) || (Streamver.Z4_Customer == UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
							|| (Streamver.CDC == UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Streamver.DSPACE == UDP_DSPACE &&  stream_no == DSPACE_CUSTOMER_DATA || stream_no == 11) || (Streamver.ecu0 == UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Streamver.ecu1 == UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Streamver.ecu2 == UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3) || (Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier && stream_no == RADAR_ECU_VRU_CLASSIFIER)))

						{
							bool set_flag = Dump_Option_Fun(stream_no, Streamver);
							if (set_flag == TRUE)
							{
								m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
								//m_radar_plugin->RadarMudpPrintCsvHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
							}
							once[sourceId][stream_no] = 1;
						}
					}
				}


				int status = -1;
				status = m_radar_plugin->RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_latch_out);

				if (status == -1)
				{
					setstreammismatchcount(stream_no);
					//printf("\nstream number %d size mismatch\n", stream_no);
					return 0;
				}

#ifdef RADAR_STREAM_UT
				static bool first = false;
				if (!first) {
					first = true;
					g_pIRadarStrm[stream_no]->PrintHeader(pOutFile, ALL);
				}
				if (e_Options == ALL) {
					g_pIRadarStrm[stream_no]->Convert_Data(*g_pIRadarStrm);
					g_pIRadarStrm[stream_no]->PrintData(pOutFile, ALL);

				}
				return 0;
#endif

				if (MUDP_master_config->data_extracter_mode[CSV] == 1)
				{
					bool set_flag = Dump_Option_Fun(stream_no, Streamver);
					if (set_flag == TRUE)
					{
						int ret = GetNextLogname(sourceId, StrmValue, stream_no);
						if (ret != 0)
							return ret;
						m_radar_plugin->RadarMudpPrintData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], CSVFName[sourceId][StrmValue], e_Options);
						//m_radar_plugin->RadarMudpPrintCsvData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options); // csv six parameters
					}
				}
				
				



			}
			if (MUDP_master_config->data_extracter_mode[XML] == 1)
				{//sfpxml
					if (CreatxmlOnce == false && sourceId != RDR_POS_ECU)
					{	
						if (Dspace)
						{
							
							

								if (sptr_manager == nullptr)//#HTMLInMUDP
								{
									if (sfpxml != NULL)
										Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, 0, cust_id);
									if (pxmlFile[0] != NULL)
										Populate_Valid_Data_to_XML(pxmlFile[0], InputLogName, g_pIRadarStrm[stream_no], stream_no, 0, cust_id);
								}
								else
								{
									std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, 0, cust_id, 0, platform, 0);

								}
								if (sptr_manager == nullptr)//#HTMLInMUDP
								{
									if (sfpxml != NULL)
										Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, 1, cust_id);
									if (pxmlFile[1] != NULL)
										Populate_Valid_Data_to_XML(pxmlFile[1], InputLogName, g_pIRadarStrm[stream_no], stream_no, 1, cust_id);
								}
								else
								{
									std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, 1, cust_id, 0, platform, 0);

								}

								if (sptr_manager == nullptr)//#HTMLInMUDP
								{

									if (sfpxml != NULL)
										Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, 2, cust_id);
									if (pxmlFile[2] != NULL)
										Populate_Valid_Data_to_XML(pxmlFile[2], InputLogName, g_pIRadarStrm[stream_no], stream_no, 2, cust_id);
								}
								else
								{
									std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, 2, cust_id, 0, platform, 0);

								}

								if (sptr_manager == nullptr)//#HTMLInMUDP
								{

									if (sfpxml != NULL)
										Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, 3, cust_id);
									if (pxmlFile[3] != NULL)
										Populate_Valid_Data_to_XML(pxmlFile[3], InputLogName, g_pIRadarStrm[stream_no], stream_no, 3, cust_id);
								}
								else
								{
									std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, 3, cust_id, 0, platform, 0);

								}

								if (sptr_manager == nullptr)//#HTMLInMUDP
								{

									if (sfpxml != NULL)
										Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, 5, cust_id);
									if (pxmlFile[5] != NULL)
										Populate_Valid_Data_to_XML(pxmlFile[5], InputLogName, g_pIRadarStrm[stream_no], stream_no, 5, cust_id);
								}
								else
								{
									std::shared_ptr<HtmlReportManager>& rep = sptr_manager;
									rep->dispatch_parsedmf4_data(filecnt, InputLogName, g_pIRadarStrm[stream_no], stream_no, 5, cust_id, 0, platform, 0);

								}
						}
						
						else 
						{
							if (sfpxml != NULL)
								Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
							if (pxmlFile[sourceId] != NULL)
								Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
						}
						
					}
					if (CreateHilxmlOnce == false && sourceId != RDR_POS_ECU)
					{
						Populate_Valid_Data_to_XML(hxmlFile, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
					}
					if (CreatxmlOnce == false && sourceId == RDR_POS_ECU)
					{
						//if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] == 1 && stream_no == )
						if (pxmlFile[sourceId] != NULL)
							Populate_ECU_Data_to_XML(pxmlFile[sourceId], InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
						if (sfpxml != NULL)
							Populate_ECU_Data_to_XML(sfpxml, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
					}
					if (CreateHilxmlOnce == false && sourceId == RDR_POS_ECU)
					{
						Populate_ECU_Data_to_XML(hxmlFile, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
					}
				}

		}
	//if (Streamver.DSPACE == UDP_DSPACE || Streamver.ecu0 == UDP_RDR_ECU0 || Streamver.ecu1 == UDP_RDR_ECU1 || Streamver.ecu2 == UDP_RDR_ECU3 || Streamver.Z4_Core == UDP_Z4_CORE ||
	//	Streamver.Z4_Customer == UDP_CUSTOM_Z4 || Streamver.Z7A_Core == UDP_Z7A_CORE || Streamver.Z7B_Customer == UDP_CUSTOM_Z7B || Streamver.Z7B_Core == UDP_Z7B_CORE || Streamver.CDC == UDP_CDC_STRM || Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier||
	//	Streamver.C0_Core == UDP_GEN5_C0_CORE || Streamver.C0_Cust == UDP_GEN5_C0_CUST || Streamver.C1_Core == UDP_GEN5_C1_CORE || Streamver.C2_Core == UDP_GEN5_C2_CORE || Streamver.C2_Cust == UDP_GEN5_C2_CUST )
	//{
	//	e_Options = ALL;
	//}
	///*		if ((Streamver.DSPACE == UDP_DSPACE) && (p_latch_out->proc_info.frame_header.Platform == UDP_SOURCE_CUST_DSPACE))
	//{
	//e_Options = DSPACE_TRK;
	//}*/

	////switch (Dump_Opt)
	////{
	////case UDP_Z7B_CORE:
	////case UDP_Z7A_CORE:
	////case UDP_Z4_CORE:
	////case UDP_CDC_STRM:
	////case UDP_CUSTOM_Z7B:
	////case UDP_CUSTOM_Z4:
	////case UDP_RDR_ECU0:
	////case UDP_RDR_ECU1:
	////case UDP_RDR_ECU3:
	////case UDP_RDR_ECU_VRU_Classifier:
	////case UDP_DSPACE:
	////default:
	////	e_Options = ALL;
	////	break;
	////}

	//// Streamver.C0_Core == UDP_GEN5_C0_CORE || Streamver.C0_Cust == UDP_GEN5_C0_CUST || Streamver.C1_Core == UDP_GEN5_C1_CORE || Streamver.C2_Core == UDP_GEN5_C2_CORE || Streamver.C2_Cust == UDP_GEN5_C2_CUST )
	//if (mf4flag == true) {
	//	if (MUDP_master_config->data_extracter_mode[CSV] == 1)
	//	{
	//		if ((once_list[sourceId][stream_no] == 0) && ((Streamver.Z7B_Core == UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA) || (Streamver.Z7A_Core == UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA) || (Streamver.Z4_Core == UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA) || (Streamver.Z7B_Customer == UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA) || (Streamver.Z4_Customer == UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
	//			|| (Streamver.CDC == UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Streamver.DSPACE == UDP_DSPACE && (stream_no == DSPACE_CUSTOMER_DATA || stream_no == 11)) || (Streamver.ecu0 == UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Streamver.ecu1 == UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Streamver.ecu2 == UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3) || (Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier && stream_no == RADAR_ECU_VRU_CLASSIFIER)|| 
	//			(Streamver.C0_Core == UDP_GEN5_C0_CORE &&  stream_no == C0_CORE_MASTER_STREAM) || (Streamver.C0_Cust == UDP_GEN5_C0_CUST  &&  stream_no == C0_CUST_MASTER_STREAM ) || (Streamver.C1_Core == UDP_GEN5_C1_CORE  &&  stream_no == C1_CORE_MASTER_STREAM) || (Streamver.C2_Core == UDP_GEN5_C2_CORE  &&  stream_no == C2_CORE_MASTER_STREAM) || (Streamver.C2_Cust == UDP_GEN5_C2_CUST &&  stream_no == C2_CUST_MASTER_STREAM)))

	//		{
	//			bool set_flag = Dump_Option_Fun(stream_no, Streamver);
	//			if (set_flag == TRUE)
	//			{
	//				m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
	//			}

	//			once_list[sourceId][stream_no] = 1;
	//		}

	//	}
	//}
	//else {
	//	if (MUDP_master_config->data_extracter_mode[CSV] == 1)
	//	{
	//		if ((once[sourceId][stream_no] == 0) && ((Streamver.Z7B_Core == UDP_Z7B_CORE && stream_no == Z7B_LOGGING_DATA) || (Streamver.Z7A_Core == UDP_Z7A_CORE && stream_no == Z7A_LOGGING_DATA) || (Streamver.Z4_Core == UDP_Z4_CORE  &&  stream_no == Z4_LOGGING_DATA) || (Streamver.Z7B_Customer == UDP_CUSTOM_Z7B  &&  stream_no == SRR3_Z7B_CUST_LOGGING_DATA) || (Streamver.Z4_Customer == UDP_CUSTOM_Z4 && stream_no == Z4_CUST_LOGGING_DATA)
	//			|| (Streamver.CDC == UDP_CDC_STRM &&  stream_no == CDC_DATA) || (Streamver.DSPACE == UDP_DSPACE &&  stream_no == DSPACE_CUSTOMER_DATA || stream_no == 11) || (Streamver.ecu0 == UDP_RDR_ECU0 && stream_no == RADAR_ECU_CORE_0) || (Streamver.ecu1 == UDP_RDR_ECU1 && stream_no == RADAR_ECU_CORE_1) || (Streamver.ecu2 == UDP_RDR_ECU3 && stream_no == RADAR_ECU_CORE_3) || (Streamver.ECU_VRU_Classifier == UDP_RDR_ECU_VRU_Classifier && stream_no == RADAR_ECU_VRU_CLASSIFIER)|| 
	//			(Streamver.C0_Core == UDP_GEN5_C0_CORE &&  stream_no == C0_CORE_MASTER_STREAM) || (Streamver.C0_Cust == UDP_GEN5_C0_CUST  &&  stream_no == C0_CUST_MASTER_STREAM) || (Streamver.C1_Core == UDP_GEN5_C1_CORE  &&  stream_no == C1_CORE_MASTER_STREAM) || (Streamver.C2_Core == UDP_GEN5_C2_CORE  &&  stream_no == C2_CORE_MASTER_STREAM) || (Streamver.C2_Cust == UDP_GEN5_C2_CUST &&  stream_no == C2_CUST_MASTER_STREAM)))

	//		{
	//			bool set_flag = Dump_Option_Fun(stream_no, Streamver);
	//			if (set_flag == TRUE)
	//			{
	//				m_radar_plugin->RadarMudpPrintHeader(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
	//			}
	//			once[sourceId][stream_no] = 1;
	//		}
	//	}
	//}

	//g_pIRadarStrm[stream_no]->FillStreamData(&p_latch_out->data[0], p_latch_out->proc_info.nDataLen, &p_latch_out->proc_info);
//	int status = -1;
//	if (pRec->payload[0] == 0xA4 && pRec->payload[1] == 0x2C)
//	{
//		status = m_radar_plugin->GEN5RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_gen5_latch_out);
//	}
//	else
//	{
//		status = m_radar_plugin->RadarMudpFillStreamData(g_pIRadarStrm, stream_no, p_latch_out);
//	}
//
//	if (status == -1)
//	{
//		printf("\nstream number %d size mismatch\n", stream_no);
//		return 0;
//	}
//
//#ifdef RADAR_STREAM_UT
//	static bool first = false;
//	if (!first) {
//		first = true;
//		g_pIRadarStrm[stream_no]->PrintHeader(pOutFile, ALL);
//	}
//	if (e_Options == ALL) {
//		g_pIRadarStrm[stream_no]->Convert_Data(*g_pIRadarStrm);
//		g_pIRadarStrm[stream_no]->PrintData(pOutFile, ALL);
//
//	}
//	return 0;
//#endif
//
//	if (MUDP_master_config->data_extracter_mode[CSV] == 1)
//	{
//		bool set_flag = Dump_Option_Fun(stream_no, Streamver);
//		if (set_flag == TRUE)
//		{
//			m_radar_plugin->RadarMudpPrintData(g_pIRadarStrm, stream_no, fpMF4Dump[sourceId][StrmValue], e_Options);
//		}
//	}
//	if (MUDP_master_config->data_extracter_mode[XML] == 1)
//	{//sfpxml
//		if (CreatxmlOnce == false && sourceId != RDR_POS_ECU)
//		{
//			if (sfpxml != NULL)
//				Populate_Valid_Data_to_XML(sfpxml, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
//			if (pxmlFile[sourceId] != NULL)
//				Populate_Valid_Data_to_XML(pxmlFile[sourceId], InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
//		}
//		if (CreateHilxmlOnce == false && sourceId != RDR_POS_ECU)
//		{
//			Populate_Valid_Data_to_XML(hxmlFile, InputLogName, g_pIRadarStrm[stream_no], stream_no, sourceId, cust_id);
//		}
//		if (CreatxmlOnce == false && sourceId == RDR_POS_ECU)
//		{
//			//if(MUDP_master_config->RADAR_ECU_Stream_Options[ecu0] == 1 && stream_no == )
//			if (pxmlFile[sourceId] != NULL)
//				Populate_ECU_Data_to_XML(pxmlFile[sourceId], InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
//			if (sfpxml != NULL)
//				Populate_ECU_Data_to_XML(sfpxml, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
//		}
//		if (CreateHilxmlOnce == false && sourceId == RDR_POS_ECU)
//		{
//			Populate_ECU_Data_to_XML(hxmlFile, InputLogName, stream_no, g_pIRadarStrm[stream_no], sourceId, cust_id);
//		}
//	}


		}

		//g_pIRadarStrm->getxmlData(&p_latch_out->data[0],e_Options);
		
	}
	if (cust_id == HONDA_GEN5 && (g_pIRadarStrm[C1_CORE_MASTER_STREAM] != 0) && (g_pIRadarStrm[C2_CORE_MASTER_STREAM] != 0))
	{
		/**HONDA special: six parameters with scanindex from C2 core and MPRB calculations from C1 core destruction**/
		for (i = 0; i < MAX_LOGGING_SOURCE; i++)  // Calling Destructor for IRadarStream Object
		{
			delete g_pIRadarStrm[i];
			g_pIRadarStrm[i] = NULL;
		}
	}
	else if(MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 0)
	{
		for (i = 0; i < MAX_LOGGING_SOURCE; i++)  // Calling Destructor for IRadarStream Object
		{
			delete g_pIRadarStrm[i];
			g_pIRadarStrm[i] = NULL;
		}
	}

	return 0;

}
class line : public string {};

std::istream &operator >> (std::istream &is, line &l)
{
	std::getline(is, l);
	return is;
}
//int ExtractAndListMUDP(FILE*pOutFile, enDumpOpt opt, UDPLogStruct* pLogStruct)
//{
//	DVSU_RECORD_T DVSURecord = { 0 };
//	if (NULL == pLogStruct)
//		return -1;
//	DVSURecord.pcTime = pLogStruct->timestamps.pcTime;
//	static int once = 0;
//	//forming the DVSU_REC
//	memcpy((unsigned char*)&DVSURecord.payload[0], (unsigned char*)&pLogStruct->header, sizeof(UDPRecord_Header));
//	memcpy(&DVSURecord.payload[sizeof(UDPRecord_Header)], (unsigned char*)&pLogStruct->payload[0], byteswap(pLogStruct->header.streamDataLen));
//
//	UDPRecord_Header  udp_frame_hdr = { 0 };
//	udp_custom_frame_header_t  cust_frame_hdr = { 0 };
//	unsigned char* bytestream = &DVSURecord.payload[0];
//	unsigned64_T timestamp = (unsigned64_T)DVSURecord.pcTime;
//	unsigned char sourceId = 0;
//
//	if (DVSURecord.payload[8] >= UDP_PLATFORM_SRR5) { // Checking is the platform is other than SRR3
//		sourceId = (DVSURecord.payload[9] - 1);
//	}
//	else {
//		sourceId = DVSURecord.payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
//	}
//
//	switch (opt)
//	{
//	case UDP_HDR:
//
//		m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
//
//		print_udp_header(pOutFile, &udp_frame_hdr, (unsigned long)timestamp);
//		return 0;
//
//		/*case UDP_CUST_HDR:
//		m_pDecoderPlugin->SRR3_API_GetCustFrameHdr((unsigned char*)&pRec->payload[0],&cust_frame_hdr);
//		print_udp_cust_header(pOutFile,&cust_frame_hdr);
//		return;*/
//
//	case UDP_CALIB_USC:
//		DumpCalib(pOutFile, opt, (unsigned char*)bytestream);
//		return 0;
//
//	default:
//		break;
//	}
//
//	short source_no = -1;
//	short stream_no = -1;
//	srr3_api_status_e status = Status_Latch_Error;
//	status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream, timestamp, &source_no, &stream_no, 1);
//	if (status != Status_Latch_Completed)
//		return 0;
//
//	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
//	unsigned char version = 0;
//	if ((stream_no == Z7B_LOGGING_DATA && opt == UDP_Z7B) || (stream_no == Z7A_LOGGING_DATA && opt == UDP_Z7A)
//		|| (stream_no == Z4_LOGGING_DATA && opt == UDP_Z4) || (stream_no == CDC_DATA && opt == UDP_CDC) || (stream_no == Z7A_LOGGING_DATA && opt == UDP_Z7A_RDD) || (stream_no == Z7B_LOGGING_DATA && opt == UDP_Z7B_RDD))
//	{
//		status = m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out, (unsigned char)source_no, (unsigned char)stream_no);
//	}
//	else
//	{
//		return 0;
//	}
//
//	switch (opt)
//	{
//	case UDP_Z7B:
//	{
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = z_logging::GetInstance(sourceId)->Create(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id);
//			if (g_pIRadarStrm[stream_no]) {
//				g_pIRadarStrm[stream_no]->PrintHeader(pOutFile, e_Options);
//
//			}
//		}
//
//	}
//	break;
//
//	case UDP_Z7A:
//	{
//		if (!g_pIRadarStrm)
//		{
//			g_pIRadarStrm[stream_no] = m_radar_plugin->CreateStream(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
//			if (g_pIRadarStrm[stream_no]) {
//				m_radar_plugin->MUDP_PrintHeader(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//				//g_pIRadarStrm[stream_no]->PrintHeader(pOutFile,e_Options);
//			}
//		}
//	}
//	break;
//	case UDP_Z7A_RDD:
//	{
//		e_Options = RDD_MRR2;
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = m_radar_plugin->CreateStream(p_latch_out->proc_info.frame_header.Platform, Z7A, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
//			if (g_pIRadarStrm[stream_no]) {
//				m_radar_plugin->MUDP_PrintHeader(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//
//			}
//		}
//	}
//break;
//	case UDP_Z7B_RDD:
//	{
//		e_Options = RDD_MRR2;
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = m_radar_plugin->CreateStream(p_latch_out->proc_info.frame_header.Platform, Z7B, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
//			if (g_pIRadarStrm) {
//				m_radar_plugin->MUDP_PrintHeader(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//			}
//		}
//	}
//	break;
//	case UDP_Z4:
//	{
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = m_radar_plugin->CreateStream(p_latch_out->proc_info.frame_header.Platform, Z4, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
//			if (g_pIRadarStrm) {
//				m_radar_plugin->MUDP_PrintHeader(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//			}
//		}
//	}
//	break;
//
//	case UDP_CDC:
//	{
//		if (!g_pIRadarStrm[stream_no])
//		{
//			g_pIRadarStrm[stream_no] = m_radar_plugin->CreateStream(p_latch_out->proc_info.frame_header.Platform, CDC, p_latch_out->proc_info.frame_header.streamVersion, cust_id, sourceId);
//			if (g_pIRadarStrm) {
//				m_radar_plugin->MUDP_PrintHeader(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//			}
//		}
//	}
//	break;
//
//	default:
//		return -1;
//	}
//
//	if (!g_pIRadarStrm[stream_no])
//	{
//		fprintf(stderr, "Error! No suitable decoder found for Platform = %u, stream = %u , version = %u\n", p_latch_out->proc_info.frame_header.Platform, p_latch_out->proc_info.frame_header.streamNumber,
//			p_latch_out->proc_info.frame_header.streamVersion);
//		return -1;
//	}
//	m_radar_plugin->MUDP_FillStreamData(g_pIRadarStrm, stream_no, p_latch_out);
//	m_radar_plugin->MUDP_PrintData(g_pIRadarStrm, stream_no, pOutFile, e_Options);
//
//
//	return 0;
//
//}

void OverallSummary()
{
	std::fstream fCreate(FileCreate_Pos, std::ios::app);



	std::map<int, std::map<std::string, scandrop_T>>::iterator itrMap;
	std::map<std::string, scandrop_T>::iterator itrscan;
	int count = 0;

	struct {
		int width;
		std::string header;
	} columns[] = { { 6,"Log_No" },{ 100, "Log name" },{ 15, "RESIM_STATUS" },{ 150, "Observation" } };

	if (beginSummaryonce)
	{
		fCreate << std::endl;
		fCreate << "<RESIM_Log_Quality_Check_Summary>";
		fCreate << std::endl;

		for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');

		for (auto& col : columns) fCreate << std::setw(col.width) << col.header << " | ";
		fCreate << "\n" << std::setfill('-');
		for (auto& col : columns) fCreate << std::setw(col.width) << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');
		//beginSummaryonce = false;
	}


	if (!Summerymap.empty())
	{
		itrMap = Summerymap.begin();
		for (itrMap; itrMap != Summerymap.end(); itrMap++)
		{
			int status = 0;
			if (!itrMap->second.empty())
			{
				itrscan = itrMap->second.begin();
				for (itrscan; itrscan != itrMap->second.end(); itrscan++)
				{
					fCreate << setw(columns[0].width) << ++count_flag_log << "   ";
					fCreate << setw(columns[1].width) << itrscan->first.c_str() << "   ";
					if (strcmp(itrscan->second.BMW_Status.c_str(), "FAIL") == 0)
					{
						fCreate << setw(columns[2].width) << "FAIL" << "   ";
					}
					else
					{
						fCreate << setw(columns[2].width) << "PASS" << "   ";
					}

					std::string temp;
					std::istringstream ss(itrscan->second.Observation.c_str());
					fCreate << "\n";
					while (std::getline(ss, temp, '.')) {
						count = 280;
						fCreate << setw(count) << temp << "\n";

					}
				}
			}

			fCreate << "\n" << std::setfill(' ');
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
		//	endSummaryOnce = false;
		}
	}
	else
	{
		if (endSummaryOnce == true)
		{
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
		}
	}

	//Summerymap.clear();
	
	fCreate << "</RESIM_Log_Quality_Check_Summary>\n";
	fCreate.close();

}

void GEN7_CDC_Summary()
{
	std::fstream fCreate(FileCreate_Pos, std::ios::app);
	std::map<int, std::map<std::string, scandrop_T>>::iterator CDCMap;
	std::map<std::string, scandrop_T>::iterator itrscan;
	int count = 0;

	struct {
		int width;
		std::string header;
	} columns[] = { { 6,"Log_No" },{ 100, "Log name" },{ 25, "CDC SATURATION STATUS" },{ 13, "RL_CDC" },{ 13, "RR_CDC" },{ 13, "FR_CDC" },{ 13, "FL_CDC" },{ 15, "FC_CDC" },{ 13, "BPIL_L_CDC" },{ 13, "BPIL_R_CDC" },{ 20, "MAX_CDC SRR7P/FLR7" } };

	if (beginSummaryonce)
	{
		fCreate << std::endl;
		fCreate << "<CDC_Quality_Check_Summary>";
		fCreate << std::endl;

		for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');

		for (auto& col : columns) fCreate << std::setw(col.width) << col.header << " | ";
		fCreate << "\n" << std::setfill('-');
		for (auto& col : columns) fCreate << std::setw(col.width) << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');
		//beginSummaryonce = false;
	}

	if (!Summerymap.empty())
	{
		CDCMap = Summerymap.begin();
		for (CDCMap; CDCMap != Summerymap.end(); CDCMap++)
		{
			int status = 0;
			if (!CDCMap->second.empty())
			{
				itrscan = CDCMap->second.begin();
				for (itrscan; itrscan != CDCMap->second.end(); itrscan++)
				{
					fCreate << setw(columns[0].width) << ++cdc_count_flag << "   ";
					fCreate << setw(columns[1].width) << itrscan->first.c_str() << "   ";

					for (int i = 0; i < MAX_RADAR_COUNT; i++)
					{
						if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == TRUE)
						{
							string saturation_status = itrscan->second.CDC_Saturation_Status;
							if (itrscan->second.CDC_Max_Chunks[i] >= 111)
							{
								itrscan->second.CDC_Saturation_Status = "SATURATED";
							}
							else
							{
								if (strcmp(itrscan->second.CDC_Saturation_Status.c_str(), "SATURATED") != 0)
								{
									itrscan->second.CDC_Saturation_Status = "NOT SATURATED";
								}
							}
						}
					}

					if (strcmp(itrscan->second.CDC_Saturation_Status.c_str(), "SATURATED") == 0)
					{
						fCreate << setw(columns[2].width) << "SATURATED" << "   ";
					}
					else
					{
						fCreate << setw(columns[2].width) << "NOT SATURATED" << "   ";
					}

					//fCreate << setw(columns[3].width) << itrscan->second.Status.c_str() << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL])

					{
						fCreate << std::right << setw(columns[3].width) << itrscan->second.CDC_Max_Chunks[SRR_RL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[3].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR])
					{
						fCreate << std::right << setw(columns[4].width) << itrscan->second.CDC_Max_Chunks[SRR_RR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[4].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR])
					{
						fCreate << std::right << setw(columns[5].width) << itrscan->second.CDC_Max_Chunks[SRR_FR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[5].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL])
					{
						fCreate << std::right << setw(columns[6].width) << itrscan->second.CDC_Max_Chunks[SRR_FL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[6].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC])
					{
						fCreate << std::right << setw(columns[7].width) << itrscan->second.CDC_Max_Chunks[SRR_FC] << "   ";
					}
					else
						fCreate << std::right << setw(columns[7].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_L])
					{

						fCreate << std::right << setw(columns[8].width) << itrscan->second.CDC_Max_Chunks[BP_L] << "   ";
					}
					else
						fCreate << std::right << setw(columns[8].width) << "NA";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_R])
					{

						fCreate << std::right << setw(columns[9].width) << itrscan->second.CDC_Max_Chunks[BP_R] << "   ";

					}
					else
						fCreate << std::right << setw(columns[9].width) << "NA";


					/*std::string temp;
					std::istringstream ss(itrscan->second.Observation.c_str());
					fCreate << "\n";
					while (std::getline(ss, temp, '.')) {
						count = 230;
						fCreate << setw(count) << temp << "\n";

					}*/
					fCreate << std::right << setw(columns[10].width) << " 113 ";
				}
			}

			fCreate << "\n" << std::setfill(' ');
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
			//	endSummaryOnce = false;
		}
	}
	else
	{
		if (endSummaryOnce == true)
		{
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
		}
	}

	fCreate << "</CDC_Quality_Check_Summary>\n";
	fCreate.close();
}

void CDC_Summary()
{
	std::fstream fCreate(FileCreate_Pos, std::ios::app);
	std::map<int, std::map<std::string, scandrop_T>>::iterator CDCMap;
	std::map<std::string, scandrop_T>::iterator itrscan;
	int count = 0;

	struct {
		int width;
		std::string header;
	} columns[] = { { 6,"Log_No" },{ 100, "Log name" },{ 25, "CDC SATURATION STATUS" },{ 13, "RL_CDC" },{ 13, "RR_CDC" },{ 13, "FR_CDC" },{ 13, "FL_CDC" },{ 15, "FLR4/FLR4P_CDC" },{ 13, "BPIL_L_CDC" },{ 13, "BPIL_R_CDC" },{ 25, "MAX_CDC SRR6P/FLR4/FLR4P" }};

	if (beginSummaryonce)
	{
		fCreate << std::endl;
		fCreate << "<CDC_Quality_Check_Summary>";
		fCreate << std::endl;

		for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');

		for (auto& col : columns) fCreate << std::setw(col.width) << col.header << " | ";
		fCreate << "\n" << std::setfill('-');
		for (auto& col : columns) fCreate << std::setw(col.width) << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');
		//beginSummaryonce = false;
	}

	if (!Summerymap.empty())
	{
		CDCMap = Summerymap.begin();
		for (CDCMap; CDCMap != Summerymap.end(); CDCMap++)
		{
			int status = 0;
			if (!CDCMap->second.empty())
			{
				itrscan = CDCMap->second.begin();
				for (itrscan; itrscan != CDCMap->second.end(); itrscan++)
				{
					fCreate << setw(columns[0].width) << ++cdc_count_flag << "   ";
					fCreate << setw(columns[1].width) << itrscan->first.c_str() << "   ";

					for (int i = 0; i < MAX_RADAR_COUNT; i++)
					{
						if (i != 5 && itrscan->second.CDC_Max_Chunks[i])
						{
							string saturation_status = itrscan->second.CDC_Saturation_Status;
							if (itrscan->second.CDC_Max_Chunks[i] >= 108)
							{
								itrscan->second.CDC_Saturation_Status = "SATURATED";
							}
							else
							{
								itrscan->second.CDC_Saturation_Status = "NOT SATURATED";
							}
						}
						else if (i == 5 && itrscan->second.CDC_Max_Chunks[i])
						{
							if (itrscan->second.CDC_Max_Chunks[i] >= 41)
							{
								itrscan->second.CDC_Saturation_Status = "SATURATED";
							}
							else
							{
								itrscan->second.CDC_Saturation_Status = "NOT SATURATED";
							}
						}
					}

					if (strcmp(itrscan->second.CDC_Saturation_Status.c_str(), "SATURATED") == 0)
					{
						fCreate << setw(columns[2].width) << "SATURATED" << "   ";
					}
					else
					{
						fCreate << setw(columns[2].width) << "NOT SATURATED" << "   ";
					}

					//fCreate << setw(columns[3].width) << itrscan->second.Status.c_str() << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL])

					{
						fCreate << std::right << setw(columns[3].width) << itrscan->second.CDC_Max_Chunks[SRR_RL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[3].width) <<"NA"<<"   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR])
					{
						fCreate << std::right << setw(columns[4].width) << itrscan->second.CDC_Max_Chunks[SRR_RR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[4].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR])
					{
						fCreate << std::right << setw(columns[5].width) << itrscan->second.CDC_Max_Chunks[SRR_FR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[5].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL])
					{
						fCreate << std::right << setw(columns[6].width) << itrscan->second.CDC_Max_Chunks[SRR_FL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[6].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC])
					{
						fCreate << std::right << setw(columns[7].width) << itrscan->second.CDC_Max_Chunks[SRR_FC] << "   ";
					}

					//FLR4 or FLR4P Might be 
					else if (MUDP_master_config->MUDP_Select_Sensor_Status[FLR4P_FC_09])
					{
						fCreate << std::right << setw(columns[7].width) << itrscan->second.CDC_Max_Chunks[FLR4P_FC_09] << "   ";
					}
					else
						fCreate << std::right << setw(columns[7].width) << "NA" <<"   ";
					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_L])
					{

						fCreate << std::right << setw(columns[8].width) << itrscan->second.CDC_Max_Chunks[BP_L] << "   ";
					}
					else
						fCreate << std::right << setw(columns[8].width) << "NA" ;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_R])
					{

						fCreate << std::right << setw(columns[9].width) << itrscan->second.CDC_Max_Chunks[BP_R] << "   ";

					}
					else

						fCreate << std::right << setw(columns[9].width) << "NA" ;
					

					/*std::string temp;
					std::istringstream ss(itrscan->second.Observation.c_str());
					fCreate << "\n";
					while (std::getline(ss, temp, '.')) {
						count = 230;
						fCreate << setw(count) << temp << "\n";

					}*/
					fCreate << std::right << setw(columns[10].width) << " 110/42/95 ";
				}
			}

			fCreate << "\n" << std::setfill(' ');
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
			//	endSummaryOnce = false;
		}
	}
	else
	{
		if (endSummaryOnce == true)
		{
			for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
			fCreate << "\n" << std::setfill(' ');
		}
	}

	fCreate << "</CDC_Quality_Check_Summary>\n";
	fCreate.close();
}

float roundof(float var)
{
	// 37.66666 * 100 =3766.66
	// 3766.66 + .5 =3767.16    for rounding off value
	// then type cast to int so value is 3767
	// then divided by 100 so the value converted into 37.67
	float value = (int)(var * 100 + .5);
	return (float)value / 100;
}
void PrintPercentageSummary()
{
	std::fstream fCreate(FileCreate_Pos, std::ios::app);



	std::map<int, std::map<std::string, scandrop_T>>::iterator itrMap;
	std::map<std::string, scandrop_T>::iterator itrscan;
	int count = 0;
	int missedsamples = 0;
	float missedpercentage = {0.0 };
	float corruption = 0.0f;

	int DQ_FAIL_Radar_Counter[MAX_RADAR_COUNT] = { 0 };
	int RESIM_FAIL_Radar_Counter[MAX_RADAR_COUNT] = { 0 };

	struct {
		int width;
		std::string header;
	} columns[] = { { 6,"Log_No" },{ 100, "Log name" },{ 28, "Total Number Of Scan Indexes" },{ 19, "Missed Scan Indexes" },{ 33, "Percentage Of Missed Scan Indexes" },{ 25, "Percentage of Corruption" },{ 13, "RL % Loss" },{ 13, "FL % Loss" },{ 13, "RR % Loss" },{ 13, "FR % Loss" },{ 13, "FC % Loss" },{ 16, "FLR4P_FC % Loss"}, { 16, "RL % Corruption" },{ 16, "FL % Corruption" },{ 16, "RR % Corruption" },{ 16, "FR % Corruption" },{ 16, "FC % Corruption" }, { 22, "FLR4P_FC % Corruption" } };

	if (beginpercentagesummary)
	{
		/*fCreate << std::endl;
		fCreate << "<Log_Quality_Check_Summary>";
		fCreate << std::endl;*/

		for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');

		for (auto& col : columns) fCreate << std::setw(col.width) << col.header << " | ";
		fCreate << "\n" << std::setfill('-');
		for (auto& col : columns) fCreate << std::setw(col.width) << "" << "-+-";
		fCreate << "\n" << std::setfill(' ');
		beginpercentagesummary = false;
	}
	if (!Summerymap.empty())
	{
		itrMap = Summerymap.begin();
		for (itrMap; itrMap != Summerymap.end(); itrMap++)
		{
			int status = 0;
			if (!itrMap->second.empty())
			{
				itrscan = itrMap->second.begin();
				for (itrscan; itrscan != itrMap->second.end(); itrscan++)
				{
					fCreate << setw(columns[0].width) << ++count << "   ";
					fCreate << setw(columns[1].width) << itrscan->first.c_str() << "   ";
					/*if (itrscan->second.Total_Scan_Index)
					{*/
						fCreate <<setw(columns[2].width) << itrscan->second.total_scanindex << "   ";
					/*}
					else
						fCreate << std::right << setw(columns[2].width) << "   ";*/
				
					

					missedsamples = itrscan->second.cdc_scanindex_drop + itrscan->second.udp_scanindex_drop + itrscan->second.calib_scanindex_drop;
					fCreate <<std::right<<setw(columns[3].width) << missedsamples << "   ";
					
					missedpercentage = float(missedsamples * 100) / float(itrscan->second.total_scanindex);
					corruption = float(itrscan->second.Checksumcount_Sensors) / float(itrscan->second.total_scanindex);

					if (itrscan->second.total_scanindex != 0)
					{
						fCreate << std::right << setw(columns[4].width) << roundof(missedpercentage) << "   ";
						fCreate << std::right << setw(columns[5].width) << roundof(corruption) << "   ";
					}
					else
					{
						fCreate << std::right << setw(columns[4].width) << 0 << "   ";
						fCreate << std::right << setw(columns[5].width) << 0 << "   ";
					}


					//Sensor Wise Summary Prints
					// RL
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL]) {
						missedsamples = 0;
						missedpercentage = 0;
						missedsamples = itrscan->second.cdc_scandrop[SRR_RL] + itrscan->second.udp_scandrop[SRR_RL] + itrscan->second.calib_scandrop[SRR_RL];
						if(missedsamples)
							missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[SRR_RL]);

						fCreate << std::right << setw(columns[6].width) << roundof(missedpercentage) << "   ";
					}
					else
						fCreate << std::right << setw(columns[6].width) << "NA";
					

					// FL
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL]) {
						missedsamples = 0;
						missedpercentage = 0;
						missedsamples = itrscan->second.cdc_scandrop[SRR_FL] + itrscan->second.udp_scandrop[SRR_FL] + itrscan->second.calib_scandrop[SRR_FL];
						if (missedsamples)
							missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[SRR_FL]);

						fCreate << std::right << setw(columns[7].width) << roundof(missedpercentage) << "   ";
					}
					else
						fCreate << std::right << setw(columns[7].width) << "NA";
			

					// RR
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR]) {
						missedsamples = 0;
						missedpercentage = 0;
						missedsamples = itrscan->second.cdc_scandrop[SRR_RR] + itrscan->second.udp_scandrop[SRR_RR] + itrscan->second.calib_scandrop[SRR_RR];
						if (missedsamples)
							missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[SRR_RR]);

						fCreate << std::right << setw(columns[8].width) << roundof(missedpercentage) << "   ";
					}
					else
						fCreate << std::right << setw(columns[8].width) << "NA";
					

					// FR
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR]) {
						missedsamples = 0;
						missedpercentage = 0;
						missedsamples = itrscan->second.cdc_scandrop[SRR_FR] + itrscan->second.udp_scandrop[SRR_FR] + itrscan->second.calib_scandrop[SRR_FR];
						if (missedsamples)
							missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[SRR_FR]);

						fCreate << std::right << setw(columns[9].width) << roundof(missedpercentage) << "   ";
					}
					else
						fCreate << std::right << setw(columns[9].width) << "NA";
					

					// FC
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC]) {
						if (itrscan->second.Total_Scan_Index[SRR_FC]) {

							missedpercentage = 0;
							missedsamples = 0;
							missedsamples = itrscan->second.cdc_scandrop[SRR_FC] + itrscan->second.udp_scandrop[SRR_FC] + itrscan->second.calib_scandrop[SRR_FC];
							if (missedsamples)
								missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[SRR_FC]);
							fCreate << std::right << setw(columns[10].width) << roundof(missedpercentage) << "   ";							
						}
						else {
							fCreate << std::right << setw(columns[10].width) << "0" << "   ";
						}
					}
					else
						fCreate << std::right << setw(columns[10].width) << "NA";
				

					// FLR4P_FC_09
					 if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC] ) {
						 if (itrscan->second.Total_Scan_Index[FLR4P_FC_09]) {
							 missedsamples = 0;
							 missedpercentage = 0;
							 missedsamples = itrscan->second.cdc_scandrop[FLR4P_FC_09] + itrscan->second.udp_scandrop[FLR4P_FC_09] + itrscan->second.calib_scandrop[FLR4P_FC_09];
							 if (missedsamples)
								 missedpercentage = float(missedsamples * 100) / float(itrscan->second.Total_Scan_Index[FLR4P_FC_09]);
							 fCreate << std::right << setw(columns[11].width) << roundof(missedpercentage) << "   ";
						 }
						 else {
							 fCreate << std::right << setw(columns[11].width) << "0" << "   ";
						 }
					}
					else
						fCreate << std::right << setw(columns[11].width) << "NA";

					// RL Corruption 
					// corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL]) 
					{
						corruption = 0;
						if (itrscan->second.Total_Scan_Index[SRR_RL])
						{
							corruption = float(itrscan->second.ChecksumErrorCount[SRR_RL]) / float(itrscan->second.Total_Scan_Index[SRR_RL]);
							
						}
						fCreate << std::right << setw(columns[12].width) << roundof(corruption) << "   ";
					}
					else
						fCreate << std::right << setw(columns[12].width) << "NA" << "   ";
					
					// FL Corruption
					//corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL])
					{
						corruption = 0;
						if (itrscan->second.Total_Scan_Index[SRR_FL])
						{
							corruption = float(itrscan->second.ChecksumErrorCount[SRR_FL]) / float(itrscan->second.Total_Scan_Index[SRR_FL]);
							
						}
						fCreate << std::right << setw(columns[13].width) << roundof(corruption) << "   ";
					}
					else
						fCreate << std::right << setw(columns[13].width) << "NA" << "   ";
					

					// RR Corruption
					//corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR])
					{
						corruption = 0;
						if (itrscan->second.Total_Scan_Index[SRR_RR])
						{
							corruption = float(itrscan->second.ChecksumErrorCount[SRR_RR]) / float(itrscan->second.Total_Scan_Index[SRR_RR]);
							
						}
						fCreate << std::right << setw(columns[14].width) << roundof(corruption) << "   ";
					}
					else
						fCreate << std::right << setw(columns[14].width) << "NA" << "   ";
					

					// FR Corruption
					//corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR]) 
					{
						corruption = 0;
						if (itrscan->second.Total_Scan_Index[SRR_FR])
						{

							corruption = float(itrscan->second.ChecksumErrorCount[SRR_FR]) / float(itrscan->second.Total_Scan_Index[SRR_FR]);
							
						}
						fCreate << std::right << setw(columns[15].width) << roundof(corruption) << "   ";
					}
					else {
						fCreate << std::right << setw(columns[15].width) << "NA" << "   ";
					}
				

					// FC Corruption 
					//corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC] )
					{
						corruption = 0;
						if (itrscan->second.Total_Scan_Index[SRR_FC])
						{
							corruption = float(itrscan->second.ChecksumErrorCount[SRR_FC]) / float(itrscan->second.Total_Scan_Index[SRR_FC]);

							
						}
						fCreate << std::right << setw(columns[16].width) << roundof(corruption) << "   ";
						
					}
					else 
						fCreate << std::right << setw(columns[16].width) << "NA" << "   ";
					
					corruption = 0;
					if (MUDP_master_config->MUDP_Select_Sensor_Status[FLR4P_FC_09] )
					{
					 if (itrscan->second.Total_Scan_Index[FLR4P_FC_09])
					{

						corruption = float(itrscan->second.ChecksumErrorCount[FLR4P_FC_09]) / float(itrscan->second.Total_Scan_Index[FLR4P_FC_09]);
						
					}
					 fCreate << std::right << setw(columns[17].width) << roundof(corruption) << "   ";
					}
					else 
						fCreate << std::right << setw(columns[17].width) << "NA" << "   ";


					//Overall Radar Summary 
					for (int pos = 0; pos < MAX_RADAR_COUNT; pos++)
					{
						if (itrscan->second.DQ_Radar_Fail_Status[pos] == "FAIL")
						{
							DQ_FAIL_Radar_Counter[pos]++;
						}

						if (itrscan->second.RESIM_Radar_Fail_Status[pos] == "FAIL")
						{
							RESIM_FAIL_Radar_Counter[pos]++;
						}
					}
					
				}
			}
			fCreate << "\n" << std::setfill(' ');
		}
	}

	fCreate << "\n" << std::setfill(' ');
	for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
	fCreate << "\n" << std::setfill(' ');
		
	fCreate << "</Overall_Log_Quality_Check_Summary>\n";

	if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
	{
		fCreate << std::endl;
		fCreate << "<Overall_Radar_Report_Summary>\n" << std::endl;
		
		for (int pos = 0; pos < MAX_RADAR_COUNT; pos++)
		{
			if (DQ_FAIL_Radar_Counter[pos] != 0)
			{
				fCreate << std::setfill(' ') << "Num of Logs Failed for Radar Pos :" << GetRadarPosName(pos) << " : " << DQ_FAIL_Radar_Counter[pos] << "\n";
			}
		}

		fCreate << "</Overall_Radar_Report_Summary>\n" << std::endl;

		//fCreate << std::endl;
		fCreate << "<Overall_RESIM_Radar_Report_Summary>\n" << std::endl;

		for (int pos = 0; pos < MAX_RADAR_COUNT; pos++)
		{
			if (pos != 4)
			{
				if (RESIM_FAIL_Radar_Counter[pos] != 0)
				{
					fCreate << std::setfill(' ') << "Num of Logs Failed for RESIM for Radar Pos :" << GetRadarPosName(pos) << " : " << RESIM_FAIL_Radar_Counter[pos] << "\n";
				}
			}
		}

		fCreate << "</Overall_RESIM_Radar_Report_Summary>\n" << std::endl;
	}


	//fCreate << std::endl;
	if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
	{
		fCreate << "</PacketLossReport>\n" << std::endl;
	}


#if defined(__GNUC__)
	delete[] logfilename;
#endif
	Summerymap.clear();
	fCreate.close();

}

void PrintSummary()
{
	std::fstream fCreate(FileCreate_Pos, std::ios::app);

	fCreate << std::endl;
	fCreate << "<Overall_Log_Quality_Check_Summary>";
	fCreate << std::endl;

	std::map<int, std::map<std::string, scandrop_T>>::iterator itrMap;
	std::map<std::string, scandrop_T>::iterator itrscan;


	struct {
		int width;
		std::string header;
	} columns[] = { { 6,"Log_No" },{ 100, "Log name" },{ 15, "STATUS of RESIM" },{ 26, "STATUS of packet loss tool" },{ 13, "RL_SCANS" },{ 13, "RR_SCANS" },{ 13, "FR_SCANS" },{ 13, "FL_SCANS" },{ 13, "FC_SCANS" } ,{ 13, "BPR_SCANS" },{ 13, "BPL_SCANS" },{ 18, "FLR4P_FC_SCANS" },{ 13, "ECU_SCANS" },
		{19,"CDC_PARTIALLY_DROP"}, {19,"CDC_FULLY_DROP"}, {19, "CDC_DROP"},{19, "UDP_DROP"},{19, "CALIB_DROP"},{19,"F360_DROP"},{19,"GDSR_DROP"} ,{24,"SENSOR_CHEKSUM_FAILURES"},{24,"ECU_CHECKSUM_FAILURES"},{24,"F360_CHECKSUM_FAILURES"},{24,"GDSR_CHECKSUM_FAILURES"}};

	for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
	fCreate << "\n" << std::setfill(' ');

	int FC_SCANINDEX_COUNT = sizeof(ScanIndexCount[5]);

	for (auto& col : columns) fCreate << std::setw(col.width) << col.header << " | ";
	fCreate << "\n" << std::setfill('-');
	for (auto& col : columns) fCreate << std::setw(col.width) << "" << "-+-";
	fCreate << "\n" << std::setfill(' ');
	count_flag = 0;
	if (!Summerymap.empty())
	{
		itrMap = Summerymap.begin();
		for (itrMap; itrMap != Summerymap.end(); itrMap++)
		{
			if (!itrMap->second.empty())
			{
				itrscan = itrMap->second.begin();
				for (itrscan; itrscan != itrMap->second.end(); itrscan++)
				{

					fCreate << setw(columns[0].width) << ++count_flag << "   ";
					fCreate << setw(columns[1].width) << itrscan->first.c_str() << "   ";
					//fCreate << setw(columns[2].width) << itrscan->second.BMW_Status << "   ";
					//fCreate << setw(columns[3].width) << itrscan->second.Status.c_str() << "   ";

					if (strcmp(itrscan->second.BMW_Status.c_str(), "FAIL") == 0)
					{
						fCreate << setw(columns[2].width) << "FAIL" << "   ";
					}
					else
					{
						fCreate << setw(columns[2].width) << "PASS"  << "   ";
					}
					fCreate << setw(columns[3].width) << itrscan->second.Status.c_str() << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RL])
					{
						fCreate << std::right << setw(columns[4].width) << itrscan->second.Total_Scan_Index[SRR_RL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[4].width ) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_RR])
					{
						fCreate << std::right << setw(columns[5].width) << itrscan->second.Total_Scan_Index[SRR_RR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[5].width ) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FR])
					{
						fCreate << std::right << setw(columns[6].width) << itrscan->second.Total_Scan_Index[SRR_FR] << "   ";
					}
					else
						fCreate << std::right << setw(columns[6].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FL])
					{
						fCreate << std::right << setw(columns[7].width) << itrscan->second.Total_Scan_Index[SRR_FL] << "   ";
					}
					else
						fCreate << std::right << setw(columns[7].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[SRR_FC])
					{
						fCreate << std::right << setw(columns[8].width) << itrscan->second.Total_Scan_Index[SRR_FC] << "   ";
					}
					else
						fCreate << std::right << setw(columns[8].width ) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_R])
					{
						fCreate << std::right << setw(columns[9].width) << itrscan->second.Total_Scan_Index[BP_R] << "   ";
					}
					else
						fCreate << std::right << setw(columns[9].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[BP_L])
					{
						fCreate << std::right << setw(columns[10].width) << itrscan->second.Total_Scan_Index[BP_L] << "   ";
					}
					else
						fCreate << std::right << setw(columns[10].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[FLR4P_FC_09])
					{
						fCreate << std::right << setw(columns[11].width) << itrscan->second.Total_Scan_Index[FLR4P_FC_09] << "   ";
					}
					else
						fCreate << std::right << setw(columns[11].width) << "NA" << "   ";

					if (MUDP_master_config->MUDP_Select_Sensor_Status[RADAR_POS])
					{
						fCreate << std::right << setw(columns[12].width) << itrscan->second.Total_Scan_Index[RADAR_POS] << "   ";
					}
					else
						fCreate << std::right << setw(columns[12].width) << "NA" << "   ";

						fCreate << std::right << setw(columns[13].width) << itrscan->second.log_cdc_partially_drop << "   ";

						fCreate << std::right << setw(columns[13].width) << (itrscan->second.cdc_scanindex_drop - itrscan->second.log_cdc_partially_drop) << "   ";

					
						fCreate << std::right << setw(columns[13].width) << itrscan->second.cdc_scanindex_drop << "   ";						

					
						fCreate << std::right << setw(columns[14].width) << itrscan->second.udp_scanindex_drop << "   ";
					
						fCreate << std::right << setw(columns[15].width) << itrscan->second.calib_scanindex_drop << "   ";
					
						fCreate << std::right << setw(columns[16].width) << itrscan->second.F360_scanindex_drop << "   ";
																						
						fCreate << std::right << setw(columns[17].width) << itrscan->second.gdsr_scanindex_drop << "   ";
					
						fCreate << std::right << setw(columns[18].width) << itrscan->second.Checksumcount_Sensors << "   ";
	
						fCreate << std::right << setw(columns[19].width) << itrscan->second.Checksumcount_ECU << "   ";
					
				
						fCreate << std::right << setw(columns[20].width) << itrscan->second.ChecksumErrorCount_F360 << "   ";
					
						fCreate << std::right << setw(columns[21].width) << itrscan->second.ChecksumErrorCount_GDSR << "   " << "\n";
				

				}
			}
		}
	}
	for (auto& col : columns)fCreate << std::setw(col.width) << std::setfill('-') << "" << "-+-";
	fCreate << "\n" << std::setfill(' ');
	fCreate << "\n";
//	fCreate << "</Log_Quality_Check_Summary>\n";
//	//fCreate << std::endl;
//	if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1)
//	{
//		fCreate << "</PacketLossReport>\n" << std::endl;
//	}
//
//
//#if defined(__GNUC__)
//	delete[] logfilename;
//#endif
////	Summerymap.clear();
	fCreate.close();
}


void logCopy(char *logname)
{
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
	split_path(logname, drvName, dir, fName, ext);
	strcat(fName, ext);
	strcpy(logfilename, fName);

#elif defined(__GNUC__)
	char *drvName = new char[_MAX_DRIVE];
	char *dir = new char[_MAX_DIR];
	char *fName = new char[_MAX_FNAME];
	char *ext = new char[_MAX_EXT];
	split_pathLinux(logname, &drvName, &dir, &fName, &ext);
	strcat(fName, ext);
	strcpy(logfilename, fName);
	delete[] fName;
	delete[] drvName;
	delete[] dir;
	delete[] ext;
#endif		
}
//Kpierrorcount(){
//}

void JSONMemoryAllocation(void)
{
	debug_Fname[0] = new char[_MAX_PATH];
	faseth_Fname[0] = new char[_MAX_PATH];
	califr_Fname[0] = new char[_MAX_PATH];
	ref_Fname[0] = new char[_MAX_PATH];
	memset(debug_Fname[0], 0x00, _MAX_PATH);
	memset(faseth_Fname[0], 0x00, _MAX_PATH);
	memset(califr_Fname[0], 0x00, _MAX_PATH);
	memset(ref_Fname[0], 0x00, _MAX_PATH);

	debug_Fname[1] = new char[_MAX_PATH];
	faseth_Fname[1] = new char[_MAX_PATH];
	califr_Fname[1] = new char[_MAX_PATH];
	ref_Fname[1] = new char[_MAX_PATH];
	memset(debug_Fname[1], 0x00, _MAX_PATH);
	memset(faseth_Fname[1], 0x00, _MAX_PATH);
	memset(califr_Fname[1], 0x00, _MAX_PATH);
	memset(ref_Fname[1], 0x00, _MAX_PATH);
}

MUDP_status_e ParseJsonFile(string path)
{
	MUDP_status_e exitRet = MUDP_OK;

	try
	{
		// Create a root
		pt::ptree root;

		// Load the json file in this ptree
		pt::read_json(path, root);

		for (pt::ptree::value_type &reprocessingInputFileStream : root.get_child("reprocessingInputFileStreams"))
		{
			std::map<int, std::string> files;
			int i = 0;
			for (pt::ptree::value_type &file : reprocessingInputFileStream.second.get_child("files"))
			{
				files.insert(pair<int, std::string>(i++, file.second.data()));
			}
			std::string name = reprocessingInputFileStream.second.get<std::string>("key");
			reprocessingInputFileStreams.insert({ name, files });
		}
		exitRet = MUDP_OK;
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
		exitRet = MUDP_JSON_FILE_ERROR;// to do
	}

	return exitRet;
}

MUDP_status_e MUDP_JSON_Parser(void)
{
	char fKey[_MAX_EXT] = { 0 };
	int CALIFRcnt = 0;
	int FASETHcnt = 0;
	int DEBUGcnt = 0;
	int REFERENCEcnt = 0;

	MUDP_status_e ret = MUDP_INIT_ERROR;
	CALIFRcnt = reprocessingInputFileStreams["BN_CALIFR"].size();
	DEBUGcnt = reprocessingInputFileStreams["SRR_DEBUG"].size();
	REFERENCEcnt = reprocessingInputFileStreams["SRR_REFERENCE"].size();
	std::map<std::string, map<int, std::string>>::iterator itr = reprocessingInputFileStreams.begin();
	for (itr; itr != reprocessingInputFileStreams.end(); itr++)
	{
		if (itr->first == "BN_IUKETH")
		{
			FASETHcnt = reprocessingInputFileStreams["BN_IUKETH"].size();
		}
		else
			if (itr->first == "BN_FASETH")
			{
				FASETHcnt = reprocessingInputFileStreams["BN_FASETH"].size();
			}
	}


	if ((CALIFRcnt == FASETHcnt) && (FASETHcnt == DEBUGcnt) && (CALIFRcnt == DEBUGcnt)) {
		fPathCnt = CALIFRcnt;
		ret = MUDP_OK;
	}
	else {
		ret = MUDP_NOK;
	}

	return ret;
}

void GetJsonInputFileName(int value)
{
	for (int i = 0; i < MAX_LEN; i++)
	{
		//	memset(califr_Fname[0], 0x0, ); // clear the buffer
		memset(debug_Fname[i], 0x00, _MAX_PATH);
		memset(faseth_Fname[i], 0x00, _MAX_PATH);
		memset(califr_Fname[i], 0x00, _MAX_PATH);
		memset(ref_Fname[i], 0x00, _MAX_PATH);
	}

	strcpy(califr_Fname[0], reprocessingInputFileStreams["BN_CALIFR"][value].c_str());
	strcpy(debug_Fname[0], reprocessingInputFileStreams["SRR_DEBUG"][value].c_str());

	if (reprocessingInputFileStreams["SRR_REFERENCE"].size())
	{
		strcpy(ref_Fname[0], reprocessingInputFileStreams["SRR_REFERENCE"][value].c_str());
	}

	if (reprocessingInputFileStreams["BN_CALIFR"].size() > (value + 1))
	{
		strcpy(califr_Fname[1], reprocessingInputFileStreams["BN_CALIFR"][value + 1].c_str());
		strcpy(debug_Fname[1], reprocessingInputFileStreams["SRR_DEBUG"][value + 1].c_str());
		if (reprocessingInputFileStreams["SRR_REFERENCE"].size())
		{
			strcpy(ref_Fname[0], reprocessingInputFileStreams["SRR_REFERENCE"][value + 1].c_str());
		}
	}
	std::map<std::string, map<int, std::string>>::iterator itr = reprocessingInputFileStreams.begin();

	for (itr; itr != reprocessingInputFileStreams.end(); itr++)
	{
		if (itr->first == "BN_IUKETH")
		{
			strcpy(faseth_Fname[0], reprocessingInputFileStreams["BN_IUKETH"][value].c_str());
			if (reprocessingInputFileStreams["BN_IUKETH"].size() > (value + 1)) {
				strcpy(faseth_Fname[1], reprocessingInputFileStreams["BN_IUKETH"][value + 1].c_str());
			}
		}
		else if (itr->first == "BN_FASETH")
		{
			strcpy(faseth_Fname[0], reprocessingInputFileStreams["BN_FASETH"][value].c_str());
			if (reprocessingInputFileStreams["BN_FASETH"].size() > (value + 1)) {
				strcpy(faseth_Fname[1], reprocessingInputFileStreams["BN_FASETH"][value + 1].c_str());
			}
		}
	}
}

void DeleteJsonPathName()
{
	for (int i = 0; i < MAX_LEN; i++)
	{
		if (califr_Fname[i])
		{
			delete[] califr_Fname[i];
			califr_Fname[i] = nullptr;
		}
		if (faseth_Fname[i])
		{
			delete[] faseth_Fname[i];
			faseth_Fname[i] = nullptr;
		}
		if (debug_Fname[i])
		{
			delete[] debug_Fname[i];
			debug_Fname[i] = nullptr;
		}
		if (ref_Fname[i])
		{
			delete[] ref_Fname[i];
			ref_Fname[i] = nullptr;
		}
	}

}
void printstrmsizecount()
{
	if (strcmp(MUDP_master_config->GenType, "GEN5") == 0)
	{
		for (int i = 0; i < MAX_LOGGING_SOURCE; i++)
		{
			if (strmsizestatus[i])
			{
				printf("\nSize mismatch observed in %s in %d  cycles",GetStreamName(i), strmsizestatus[i]);
			}
		}
	}

	if (strcmp(MUDP_master_config->GenType, "GEN6") == 0)
	{
		for (int i = 0; i < MAX_LOGGING_SOURCE; i++)
		{
			if (strmsizestatus[i])
			{
				if(customer_id == MOTIONAL_FLR4P || customer_id == STLA_FLR4P || customer_id == PLATFORM_GEN5)
				     printf("\nSize mismatch observed in %s in %d  cycles", Get_FLR4PGEN5StreamName(i), strmsizestatus[i]);
				else
					printf("\nSize mismatch observed in %s in %d  cycles", Get_GEN5StreamName(i), strmsizestatus[i]);
			}
		}
	}
	printf("\n");

}
int  Process_MUDP_Data(char* logname, char* m_setFileName, std::shared_ptr<HtmlReportManager>& sptr_manager, int filecount)//#HTMLInMUDP
{
	string log_file_chk = logname;
	setXmlOptions();
	setStreamOptions();
	bool Sensor_Check = false;
	size_t dvlpos = log_file_chk.find(".dvl");
	if (dvlpos == string::npos)
	{
		GetInputOption(logname);
	}
	int retVal = 0;

	if ((!CreateOnce) || (!_printonce) || (!DspaceOnce) || (!CreateCcaCANOnce) || (!CreateCcaUDPOnce) || (CreatxmlOnce) || (CreateHilxmlOnce)) {
		if (strcmp(m_setFileName, logname) != 0) {
			CreateOnce = true;
			CreatxmlOnce = true;
			CreateCcaUDPOnce = true;
			CreateCcaCANOnce = true;
			CreateHilxmlOnce = true;
			DspaceOnce = true;
			CallPktOnce = true;
			//Createone = true;
			_printonce = true;
			for (int i = 0; i < MAX_RADAR_COUNT; i++) {
				//onesCDCScanId[i] = true;
				m_pDecoderPlugin->SRR3_API_Reset(i);
			}
		}
	}
	if (file_exist(logname, 0) == -1)
	{
		printf("\n[INFO] Dump is unsuccesful , since FILE is not found (%s)\n\n", logname);
		printf("/*=========================================================================================================*/\n");
		retVal = -1;
	}
	if (retVal == 0) {
		ResetScanIndexpacketCounts();
		if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) {
			printf("Data Extraction operation started for the Log :%s!!\n\n", logname);
			logCopy(logname);
		}
		if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
			printf("Packet_Loss Extraction started for the Log :%s!!\n\n", logname);
			logCopy(logname);
		}
		for (i = 0; i < MUDP_MAX_STATUS_COUNT; i++) //Changes for GPC-884
		{
			if (MUDP_master_config->MUDP_Select_Sensor_Status[i] != 0)
			{
				Sensor_Check = TRUE;
				break;
			}
		}
		if (Sensor_Check == FALSE && MUDP_master_config->MUDP_Radar_ECU_status == 1)
		{
			Sensor_Check = TRUE;
		}
		if (Sensor_Check)
		{
			//check for bhtml_report
			
			Dump_MUDP_Status = DumpMUDPData(logname, sptr_manager, filecount);
			if (Dump_MUDP_Status == 0) {
				if (MUDP_master_config->data_extracter_mode[CSV] == 1 || MUDP_master_config->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1 || MUDP_master_config->data_extracter_mode[XML] == 1) {
					printstrmsizecount();

					for (int i = 0; i < (int)Print_Stream_Version_Mismatch_Messages.size(); i++)
					{
						tuple<string, int, int, int> StreamInfoTuple = Print_Stream_Version_Mismatch_Messages[i];
						cout << "NOTE! " << get<0>(StreamInfoTuple) << ", Platfrom ID - " << get<1>(StreamInfoTuple) << ", log having new version - " << get<2>(StreamInfoTuple) << "," << "need to update decoder, available decoder version is " << get<3>(StreamInfoTuple) << endl;
					}
					Print_Stream_Version_Mismatch_Messages.clear();

					printf("/*=========================================================================================================*/\n");
					printf("\nDump Operation Completed for (%s)!!\n", logname);
					printf("/*=========================================================================================================*/\n");
					for (int i = 0; i < MAX_RADAR_COUNT; i++) {
						bOnce[i] = 0;
						for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
						{
							once_list[i][j] = 0;
						}
					}
				}
				if (fCreate) {
					for (int i = 0; i < (int)Print_Stream_Version_Mismatch_Messages.size(); i++)
					{
						tuple<string, int, int, int> StreamInfoTuple = Print_Stream_Version_Mismatch_Messages[i];
						cout << "NOTE! " << get<0>(StreamInfoTuple) << ", Platfrom ID - " << get<1>(StreamInfoTuple) << ", log having new version - " << get<2>(StreamInfoTuple) << "," << "need to update decoder, available decoder version is  " << get<3>(StreamInfoTuple) << "\n";
					}
					Print_Stream_Version_Mismatch_Messages.clear();

					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
						CloseScanIndexerrorFile(fCreate, logname);
						printf("/*=========================================================================================================*/\n");
						printf("Packet_Loss information extraction completed for the Log :%s!!\n\n", logname);
						printf("/*=========================================================================================================*/\n");
						for (int i = 0; i < MAX_RADAR_COUNT; i++) {

							radar_scan_next[i] = -1;
							radar_scan_prev[i] = -1;
							radar_chunk_id_curr[i] = 0;
							radar_chunk_id_prev[i] = 0;
							radar_cal_source_prev[i] = 0;
							radar_cal_source_curr[i] = 0;
							flag1[i] = 0;
							count1[i] = 0;
							check_scan_coun_flag = 0;
							Timingoverrun_Error_print_once = 1;
							for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
							{
								Timingoverflowcount[i][j] = 0;

							}
							nCurrentChunkid[i] = -1;
							npreviousChunkid[i] = -1;
							nCurrentScan_Index[i] = -1;
							missedchunkscan[i] = -1;
							src_tx_once[i] = 0;
							memset(&ScanIndexCount[i], 0, sizeof(ScanIndexCount[i]));
							Strmc0_corescancurrent[i] = { -1 };
							Strmc0_custscancurrent[i] = { -1 };
							Strmc1_corescancurrent[i] = { -1 };
							Strmc2_corescancurrent[i] = { -1 };
							Strmc2_custscancurrent[i] = { -1 };

							Strmc0_corescanprev[i] = { -1 };
							Strmc0_custscanprev[i] = { -1 };
							Strmc1_corescanprev[i] = { -1 };
							Strmc2_corescanprev[i] = { -1 };
							Strmc2_custscanprev[i] = { -1 };
						}
					}
				}
				//printf("\n Reading next log file from the input list...\n\n");
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
					Summerymap[count_flag] = loggedmap;
					/*	if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
						{
							OverallSummary();

						}*/
					loggedmap.clear();
					count_flag++;
				}
			}
			else if (Dump_MUDP_Status == -1 && No_UDP_Data_Flag == true)
			{
				if (fCreate) {
					for (int i = 0; i < (int)Print_Stream_Version_Mismatch_Messages.size(); i++)
					{
						tuple<string, int, int, int> StreamInfoTuple = Print_Stream_Version_Mismatch_Messages[i];
						cout << "NOTE! " << get<0>(StreamInfoTuple) << ", Platfrom ID - " << get<1>(StreamInfoTuple) << ", log having new version - " << get<2>(StreamInfoTuple) << "," << "need to update decoder, available decoder version is " << get<3>(StreamInfoTuple) << endl;
					}
					Print_Stream_Version_Mismatch_Messages.clear();

					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
						CloseScanIndexerrorFile(fCreate, logname);
						printf("/*=========================================================================================================*/\n");
						printf("Packet_Loss information extraction completed for the Log :%s!!\n\n", logname);
						printf("/*=========================================================================================================*/\n");
						for (int i = 0; i < MAX_RADAR_COUNT; i++) {

							radar_scan_next[i] = -1;
							radar_scan_prev[i] = -1;
							radar_chunk_id_curr[i] = 0;
							radar_chunk_id_prev[i] = 0;
							radar_cal_source_prev[i] = 0;
							radar_cal_source_curr[i] = 0;
							flag1[i] = 0;
							count1[i] = 0;
							check_scan_coun_flag = 0;
							Timingoverrun_Error_print_once = 1;
							for (int j = 0; j < MAX_LOGGING_SOURCE; j++)
							{
								Timingoverflowcount[i][j] = 0;

							}
							nCurrentChunkid[i] = -1;
							npreviousChunkid[i] = -1;
							nCurrentScan_Index[i] = -1;
							missedchunkscan[i] = -1;
							src_tx_once[i] = 0;
							memset(&ScanIndexCount[i], 0, sizeof(ScanIndexCount[i]));
						}
					}
				}
				//printf("\n Reading next log file from the input list...\n\n");
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
				{
					Summerymap[count_flag] = loggedmap;
					/*	if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
						{
							OverallSummary();

						}*/
					loggedmap.clear();
					count_flag++;
				}
			}
		}
		else if(!Sensor_Check)
		{
			printf("\nPlease check sensors are Enabled or Disabled in MUDP Config\n");
		}
		else {
			printf("\nDump Unsuccessful...\n");
			printf("/*=========================================================================================================*/\n");
			//	printf("\n Reading next log file from the input list...\n\n");
			retVal = -1;
		}

		if (g_mdfUDP) {
			delete g_mdfUDP;
			g_mdfUDP = NULL;
		}

	}
	return retVal;
}
void ResetMF4FileObj()//#HTMLInMUDP
{
	setViGEMMf4 = 0;
	setAuteraMf4 = 0;
	if (g_mdfUDP != NULL)
		g_mdfUDP = NULL;
}
int main(int argc, char* argv[])
{
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	MUDP_status_e exitRet = MUDP_OK;
	char  logname[_MAX_PATH] = { 0 };
	char  m_setFileName[_MAX_PATH] = { 0 };
	char inp_ext[_MAX_EXT] = { 0 };
	MUDP_status_e ret = MUDP_INIT_ERROR;
	int fileIndexTrk = -1;
	int status = 0;
	std::string csvPath;

   //CP#1#START : Add HTML Console Print

	
	//<**This vector is used to compute the total time taken for all pairs of logs**>
	std::vector<int> Total_Pairs_Run_Time_In_Mins;
	std::vector<int> Total_Pairs_Run_Time_In_Secs;
	

	//CP#1#END :
	do
	{
		if (ReadArguments(argc, argv))
		{
			return -1; //error
		}
		string xml_file_chk = xml_file_path;
		size_t pos = xml_file_chk.find(".xml");
		MUDP_master_config = new MUDP_CONFIG_T;
		if (MUDP_OK != MUDP_read_master_config(MUDP_master_config, (const char*)xml_file_path))
		{
			status += -1;
			printf("Exiting Resimulation\n");
			return MUDP_XML_CONFIG_ERROR;

		}
		if (MUDP_master_config->vector_Lib == 1 && MUDP_master_config->data_extracter_mode[Quality_Check] == 0) {
			fprintf(stdout, "Vector Library is integrated for the Analysis!!\n");
		}
		if (MUDP_master_config->cca_Lib == 1 && MUDP_master_config->data_extracter_mode[CCA_MF4_DUMP] == 0) {
			fprintf(stdout, "CCA Library is integrated for the Analysis!!\n");
		}

		printf("Sensor_Enabled: ");
		for (unsigned int i = 0; i < MUDP_MAX_STATUS_COUNT; i++) {

			if (MUDP_master_config->MUDP_Select_Sensor_Status[i] == 1) {
				radarEnabled += GetRadarPosName(i);
				radarEnabled += " ";
			}
		}
		cout << radarEnabled<<endl;

		if (MUDP_master_config->bhtml_report == 1 || MUDP_master_config->bhtml_report == 0)
		{
			//validate_HTML_Arguments(MUDP_master_config->bhtml_report);
		}
		

		if (MUDP_master_config->bhtml_report == 0)
		{
			printf("\n\r\t**************************************************************** \n");
			printf("\r\t%%                                                              %%\n");
			printf("\r\t%%  		 MUDP_EXTRACTOR_%02u_%02u_%02u_%02u                     %%\n", release_year, release_week, release_inf, release_minor);
			MUDPCopyright();
			printf("\r\t**************************************************************** \n\n");
			printf("\rMajor version: %02u\n", release_inf);
			printf("\rMinor version: %02u\n", release_minor);
			printf("\rRelease week : %02u\n", release_week);
			printf("\rRelease year : 20%02u\n\n", release_year);

			fprintf(stdout, "MUDP_Log_DataExtracter.exe version %d.%d.%d \n", release_inf, release_op, release_minor);
			fprintf(stdout, "/*=========================================================================================================*/\n\n");

		}
		

#if defined(_WIN_PLATFORM)
		char drvName[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fName[_MAX_FNAME];
		char ext[_MAX_EXT];
		split_path(inp_file, drvName, dir, fName, ext);
#elif defined(__GNUC__)
		char *drvName = new char[_MAX_DRIVE];
		char *dir = new char[_MAX_DIR];
		char *fName = new char[_MAX_FNAME];
		char *ext = new char[_MAX_EXT];
		split_pathLinux(inp_file, &drvName, &dir, &fName, &ext);
#endif
		std::ifstream inputFile(inp_file);
		istream_iterator<line> itFname(inputFile);
		istream_iterator<line> end;

		if (MUDP_master_config->bhtml_report == 0)
		{
			std::ifstream inputFile(inp_file);
			istream_iterator<line> itFname(inputFile);
			istream_iterator<line> end;
			strcpy(logname, (*itFname).c_str());
			strcpy(m_setFileName, (*itFname).c_str());
			string log_file_chk = logname;

			//csvPath = *itFname;
			//printf("CSV file name : %s ", csvPath.c_str());

			if (!inputFile)
			{
				status += -1;
				perror("[INFO] Error in Reading Log List...\n Exiting Resimulation...\n\n");
			}
		}
		//strcpy(logname,(*itFname).c_str());
#if defined(_WIN_PLATFORM)
		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Log("SRR3_MUDP_Log.dll", &m_pLogPlugin)) {
			break;
		}

		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Decoder("srr3_decoder_dph.dll", &m_pDecoderPlugin)) {
			break;
		}
		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_Stream_Decoder("radar_stream_decoder.dll", &m_radar_plugin))
		{
			printf("Failed to load plugin - radar_stream_decoder.dll \n ");
			break;
		}
#elif defined(__GNUC__)
		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Log("libSRR3_MUDP_Log.so", &m_pLogPlugin)) {
			break;
		}

		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_SRR3_Decoder("libsrr3_decoder_dph.so", &m_pDecoderPlugin)) {
			break;
		}
		if (LIB_FAIL == PluginLoader::GetPluginLoader()->Load_Stream_Decoder("libradar_stream_decoder.so", &m_radar_plugin))
		{
			printf("Failed to load plugin - libradar_stream_decoder.so \n ");
			break;
		}
#endif


		if (MUDP_master_config->bhtml_report == 1)
		{
			/*printf("\n\r\t**************************************************************** \n");
			printf("\r\t                                                             \n");
			printf("\r\t 		 HTML_REPORT                     \n");
			printf("\r\t**************************************************************** \n\n");*/

			//<**Added for Aptiv Header to update in the console (GPC-1449)**>
			fprintf(stdout, "/*=========================================================================================================*/\n\n");

			printf("\n\r\t**************************************************************** \n");
			printf("\r\t%%                                                                %%\n");
			printf("\r\t%%  		 HTML_REPORT_%02u_%02u_%02u_%02u                      %%\n", html_release_year, html_release_week, html_release_inf, html_release_minor);
			MUDPCopyright();
			printf("\r\t***************************************************************** \n\n");
			printf("\rMajor version: %02u\n", html_release_inf);
			printf("\rMinor version: %02u\n", html_release_minor);
			printf("\rRelease week : %02u\n", html_release_week);
			printf("\rRelease year : 20%02u\n\n", html_release_year);

			fprintf(stdout, "ResimHTMLReport.exe version %d.%d.%d \n", html_release_inf, html_release_op, html_release_minor);
			fprintf(stdout, "/*=========================================================================================================*/\n\n");

			//<**Added for Aptiv Header to update in the console (GPC-1449)**>

			JsonInputParser& json = JsonInputParser::getInstance();

			if (json.read_HTML_Arguments(argc, argv))
			{
				return -1;
			}

			if (json.mvehicle_mf4File.size() == json.mresim_mf4File.size())
			{
				json.totalinputpair_count = json.mvehicle_mf4File.size();
				cout << "\n Total HTML Report Count  " << json.totalinputpair_count << endl;
				cout << "\n------------------------------------------------------------------------------------";
			}


			for (int inputpaircnt = 0; inputpaircnt < json.totalinputpair_count; inputpaircnt++)
			{

				cout << "\n totalinputpair_count " << json.totalinputpair_count;
				std::string statvehFilePath = json.mvehicle_mf4File[inputpaircnt];
				std::replace(statvehFilePath.begin(), statvehFilePath.end(), '\\', '/');
				if (statvehFilePath.back() == '/') {
					statvehFilePath.pop_back();
				}

				strncpy(json.inputVehFileName, statvehFilePath.c_str(), statvehFilePath.length() + 1);

				std::string statresimFilePath = json.mresim_mf4File[inputpaircnt];
				std::replace(statresimFilePath.begin(), statresimFilePath.end(), '\\', '/');
				if (statresimFilePath.back() == '/') {
					statresimFilePath.pop_back();
				}

				strncpy(json.Resim_fileName, statresimFilePath.c_str(), statresimFilePath.length() + 1);

				if (-1 == json.CheckFileExists(json.inputVehFileName)) {
					return -1;
				}
				else
				{
					json.mf4filecount.push_back(json.inputVehFileName);
				}
				if (-1 == json.CheckFileExists(json.Resim_fileName)) {
					return -1;
				}
				else
				{
					json.mf4filecount.push_back(json.Resim_fileName);

				}

				auto mf4_filenamefrom_path = std::filesystem::path(json.inputVehFileName).filename();
				std::string mf4_filename=mf4_filenamefrom_path.u8string();

				int retval = 0;
				std::string plotfolder = "HTML_Report__";
				//plotfolder += std::to_string(mf4_filename);
				plotfolder += mf4_filename;
				std::vector<int> vec_customerID;

				if (1)
				{


					JsonInputParser& jsonparser = JsonInputParser::getInstance();

					std::shared_ptr<PlotParameter[]> sptr_plotparameter(new PlotParameter[6]);
					std::shared_ptr<HtmlReportManager> sptr_reportmanager(new HtmlReportManager(sptr_plotparameter));

					sptr_reportmanager->register_customer_id_toReportManager(84, false);
					sptr_reportmanager->register_customer_id_toReportManager(85, false);
					sptr_reportmanager->register_customer_id_toReportManager(87, false);
					sptr_reportmanager->register_customer_id_toReportManager(128, false);
					sptr_reportmanager->register_customer_id_toReportManager(129, false);
					//Implementation for traton taking custid (0x0B) 11 [SCANIA_MAN]
					sptr_reportmanager->register_customer_id_toReportManager(11, false);
					sptr_reportmanager->register_customer_id_toReportManager(83, false);
					sptr_reportmanager->register_customer_id_toReportManager(161, false);
					




					cout << "\n\t\tStarted Parsing pair  " << inputpaircnt + 1;
					cout << "\n------------------------------------------------------------------------------------";
					std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

					for (int mdffilCnt = 0; mdffilCnt < json.mf4filecount.size(); mdffilCnt++)
					{
						if (mdffilCnt == 0)
						{
							sptr_reportmanager->vehiclefilename = std::filesystem::path(json.mf4filecount[mdffilCnt]).filename().string();
							jsonparser.vehiclefilename = sptr_reportmanager->vehiclefilename;
						}
						if (mdffilCnt == 1)
						{
							sptr_reportmanager->resimfilename = std::filesystem::path(json.mf4filecount[mdffilCnt]).filename().string();
							jsonparser.resimfilename = sptr_reportmanager->resimfilename;
						}

						cout << "\nStarted Parsing... " << std::filesystem::path(json.mf4filecount[mdffilCnt]).filename();

						//Process_MUDP_Data(sptr_reportmanager,mdffilCnt, mf4filecount[mdffilCnt]);
						//status += Process_MUDP_Data(logname, m_setFileName);
						if (sptr_reportmanager != nullptr)
						{
							std::cout << "\n jsonparser.mf4filecount[mdffilCnt]" << jsonparser.mf4filecount[mdffilCnt];

							strcpy(logname, jsonparser.mf4filecount[mdffilCnt].c_str());
							strcpy(m_setFileName, jsonparser.mf4filecount[mdffilCnt].c_str());


							retval = Process_MUDP_Data(logname, logname, sptr_reportmanager, mdffilCnt);
							//retval = DumpMUDPData(sptr_reportmanager, mdffilCnt, jsonparser.mf4filecount[mdffilCnt]);
							cout << "...Completed Parsing ";
						}
						else
						{
							cout << "...No Parsing due to memmeory issue ";
						}




						ResetMF4FileObj();

					}

					//sptrHDF->createInNodeGroup_detection();
					//sptrHDF->createOutNodeGroup_detection();

					//std::cout << std::endl << "---------------------------------";
					//std::string grp= sptrHDF->get_input_groups(0, 0, 0);//filecount sensorposition detectionproperty_index
					//std::cout << std::endl << " Group (0,0,0) " << grp;
					//std::cout << std::endl << "---------------------------------";

					//sptr_reportmanager->set_hdf_group_node_generator(sptrHDF);

					if ((sptr_reportmanager->set_radar_positions.size() >= 1) && (sptr_reportmanager->set_customerID.size() >= 1))
					{
						auto it = sptr_reportmanager->set_customerID.begin();
						vec_customerID.push_back(*it);

					}


					cout << "\n Completed Parsing MF4 Files with Pair " << inputpaircnt + 1;
					for (auto& radars : sptr_reportmanager->set_radar_positions)
					{
						//std::cout << "\n radars " << radars;
						sptr_reportmanager->trigger_calculate_logduration(vec_customerID, jsonparser.inputVehFileName, plotfolder.c_str(), inputpaircnt, jsonparser.reportpath, int(radars));
					}

					cout << "\n------------------------------------------------------------------------------------";
					cout << "\n Generating plots in Progress for Pair" << inputpaircnt + 1 << "  please wait.........";
					cout << "\n------------------------------------------------------------------------------------";
					sptr_reportmanager->trigger_plotgeneration(vec_customerID, jsonparser.inputVehFileName, plotfolder.c_str(), inputpaircnt, jsonparser.reportpath);

					//	cout << "\n------------------------------------------------------------------------------------";
						//cout << "\n Generating DB in Progress for Pair" << inputpaircnt + 1 << "  please wait.........";
						//cout << "\n------------------------------------------------------------------------------------";

						//sptr_reportmanager->trigger_dbgeneration(vec_customerID, inputVehFileName, plotfolder.c_str(), inputpaircnt, reportpath);

					std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
					auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
					sptr_reportmanager->html_runtime_sec = to_string(duration);
					jsonparser.html_runtime_sec = sptr_reportmanager->html_runtime_sec;
					printf(" \nTime taken for pair no %lld report generation is %lld seconds\n", inputpaircnt + 1, duration);

					auto durationinminutes = std::chrono::duration_cast<std::chrono::minutes>(t2 - t1).count();
					sptr_reportmanager->html_runtime_min = to_string(durationinminutes);
					jsonparser.html_runtime_min = sptr_reportmanager->html_runtime_min;
					printf(" \nTime taken for pair no %lld report generation is %lld min\n", inputpaircnt + 1, durationinminutes);

					Total_Pairs_Run_Time_In_Mins.push_back(durationinminutes);
					Total_Pairs_Run_Time_In_Secs.push_back(duration);

					for (auto& radars : sptr_reportmanager->set_radar_positions)
					{
						sptr_reportmanager->trigger_print_toolruntime_toreport(vec_customerID, jsonparser.inputVehFileName, plotfolder.c_str(), inputpaircnt, jsonparser.reportpath, radars);
					}
					cout << "\n------------------------------------------------------------------------------------";

					jsonparser.mf4filecount.clear();
					sptr_reportmanager->vehiclefilename = "";
					sptr_reportmanager->resimfilename = "";

					cout << "\n------------------------------------------------------------------------------------";
					cout << "\n HTML Report Generation Completed in ";
					//<**Added for Aptiv Header to update in the console (GPC-1449)**>
					cout << jsonparser.reportpath;
					//<**Added for Aptiv Header to update in the console End (GPC-1449)**>
					cout << "\n------------------------------------------------------------------------------------";
					sptr_reportmanager->clear_datacollector_registry();

					//FreePlugin();

				}

			}
			//<**Added for Aptiv Header to update in the console (GPC-1449)**>

			int total_time_in_mins = std::accumulate(Total_Pairs_Run_Time_In_Mins.begin(), Total_Pairs_Run_Time_In_Mins.end(), 0);
			int total_time_in_secs = std::accumulate(Total_Pairs_Run_Time_In_Secs.begin(), Total_Pairs_Run_Time_In_Secs.end(), 0);
			cout << "\n------------------------------------------------------------------------------------";
			printf(" \nTotal Time taken for  %lld pair/s of report generation is %lld secs\n", json.totalinputpair_count, total_time_in_secs);
			printf(" \nTotal Time taken for  %lld pair/s of report generation is %lld min\n", json.totalinputpair_count, total_time_in_mins);
			cout << "\n------------------------------------------------------------------------------------";

			//<**Added for Aptiv Header to update in the console End (GPC-1449)**>




		}



       //CP#3#START : execute below if else if only if HTML report is false
		if (MUDP_master_config->bhtml_report == 0)
		{
			if (strcmp(ext, ".txt") == 0) // this is where normal text file list is processed.
			{
				//std::chrono::high_resolution_clock::time_point start_log = std::chrono::high_resolution_clock::now();
				bool bUpdateCSVPath = false;
				for (; itFname != end; )
				{
					std::chrono::high_resolution_clock::time_point start_log = std::chrono::high_resolution_clock::now();

					if (MUDP_master_config->data_extracter_mode[CSV] == 1)
					{
						CreateCsvOnce = true;
					}
					if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
					{
						CreatePktOnce = true;
					}

					//strcpy(templog,(*itFname).c_str())
					string templog = *itFname;
					replace(templog.begin(), templog.end(), '\\', '/');
					if (templog.back() == '\r')
					{
						templog.pop_back();
					}
					strcpy(logname, (templog).c_str());

					//csvPath = *itFname;
					//printf("CSV file name : %s ", csvPath.c_str());	
					/*if(bUpdateCSVPath==false){
					csvPath = *itFname;
					//printf("CSV file name : %s ", csvPath.c_str());
					   }*/

					std::shared_ptr<HtmlReportManager> sptr_manager{ nullptr };
					status += Process_MUDP_Data(logname, m_setFileName, sptr_manager,0);
					strcpy(m_setFileName, logname);
					std::chrono::high_resolution_clock::time_point end_log = std::chrono::high_resolution_clock::now();
					auto duration_log = std::chrono::duration_cast<std::chrono::seconds>(end_log - start_log).count();
					printf("Time taken to process log %lld seconds  \n", duration_log);
					++itFname;
					if (itFname != end)
					{
						printf("\nReading next log file from the input list...\n\n");
					}

			}
		}
		else if (strcmp(ext, ".json") == 0) // this is where json file list is processed. CYW-1039
		{
			std::chrono::high_resolution_clock::time_point start_log = std::chrono::high_resolution_clock::now();
		    bool bUpdateCSVPath = false;
			JSONMemoryAllocation();
			exitRet = ParseJsonFile(inp_file);

			if (exitRet != MUDP_OK) {
				return exitRet;
			}

			ret = MUDP_JSON_Parser();
			if (ret != MUDP_OK)
			{
				status += -1;
				printf("\n[ERROR]: Number of file paths provided are not equal.\n\r");
				return MUDP_LOG_LIST_READ_ERROR;
			}
			fileIndexTrk = 0;

			for (int i = fileIndexTrk; i < fPathCnt; i++)
			{
				if (MUDP_master_config->data_extracter_mode[CSV] == 1)
				{
					CreateCsvOnce = true;
				}
				if (MUDP_master_config->data_extracter_mode[PACKET_Loss] == 1 || MUDP_master_config->packet_loss_statistics == 1 || MUDP_master_config->data_extracter_mode[Quality_Check] == 1)
				{
					CreatePktOnce = true;
				}
				GetJsonInputFileName(i);
				strcpy(logname, debug_Fname[0]);
				std::string path = logname;
#if defined(_WIN_PLATFORM)
				std::replace(path.begin(), path.end(), '/', '\\');

#endif
					strcpy(logname, path.c_str());
					if (i == 0)
					{
						strcpy(m_setFileName, logname);
					}
					/* if (bUpdateCSVPath == false) {
					 csvPath = path;
					 //printf("CSV file name : %s ", csvPath.c_str());
						}*/

						//CP#6#START : add two arguments reportmanager pointer and file count
						// if HTML tag true pass valid reportmanager pointer and file count else null to both
						// //CP#7#START below Logic should kept in loop 
					 //Process_MUDP_Data(sptr_reportmanager,mdffilCnt, mf4filecount[mdffilCnt]);

					std::shared_ptr<HtmlReportManager> sptr_manager{ nullptr };
					status += Process_MUDP_Data(logname, m_setFileName, sptr_manager,0);
					//CP#6#END :
					std::chrono::high_resolution_clock::time_point end_log = std::chrono::high_resolution_clock::now();
					auto duration_log = std::chrono::duration_cast<std::chrono::seconds>(end_log - start_log).count();
					printf("Time taken to process log %lld seconds  \n", duration_log);

					strcpy(m_setFileName, logname);
					if (i < fPathCnt - 1)
					{
						printf("\nReading next log file from the input list...\n\n");
					}
				}
				DeleteJsonPathName();
			}
		}
      //CP#3#END
#if defined(__GNUC__)
		delete[]drvName;
		delete[]dir;
		delete[]fName;
		delete[]ext;
#endif

		} while (0);


		//CP#4#START : execute below only if HTML report is false
		if (MUDP_master_config->bhtml_report == 0)
		{
			if (MUDP_master_config->data_extracter_mode[Quality_Check] == 1) {
			if (strcmp(MUDP_master_config->GenType, "GEN7") == 0)
			{
				GEN7_CDC_Summary();
			}
			else
			{
				CDC_Summary();
			}
			
				OverallSummary();
				PrintSummary();
				PrintPercentageSummary();

			}
		}
		//CP#4#END
#if defined(__gnuc__)
			delete[] logfilename;
#endif
		//CP#5#START : execute below only if HTML report is false
			if (MUDP_master_config->bhtml_report == 0)
			{
				for (int i = 0; i < MUDP_MAX_STATUS_COUNT; i++) {
					for (int j = 0; j < INVALID_STRM; j++) {
						if (CSVFName[i][j]) {
							delete[] CSVFName[i][j];
							CSVFName[i][j] = NULL;
						}
					}
				}
				for (int i = 0; i < MAX_RADAR_COUNT; i++)
				{
					if (g_ScanID_Z_Prev[i] != 0)
					{
						g_ScanID_Z_Prev[i] = 0;
					}

				}
				//CP#5#END
				std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

				if (status == MUDP_OK)
				{
					printf("\nMUDP Extraction Completed Successfully......\n\n");
				}
				else
				{
					printf("\nMUDP Extraction Completed with errors......\n\n");
				}
				printf(" Time taken by the Application is %lld seconds\n", duration);
			}
		FreePlugin();
		if (g_mdfUDP) {
			delete g_mdfUDP;
			g_mdfUDP = NULL;
		}
		//printf("CSV file name : %s ", csvPath.c_str());
		if (MUDP_master_config->bhtml_report == 0)
		{
			CheckCsvSplitting(csvPath);
		}

		return 0;
	}

//void getDetectionsData(DVSU_RECORD_T* pRec) {
//	UDPRecord_Header  udp_frame_hdr = { 0 };
//	udp_custom_frame_header_t  cust_frame_hdr = { 0 };
//	void* pBuffer = NULL;
//	unsigned int size = 0;
//	unsigned char ver = 0;
//	unsigned char sourceId = 0;
//
//	if (pRec->payload[8] >= UDP_PLATFORM_SRR5) { // Checking is the platform is other than SRR3
//		sourceId = (pRec->payload[9] - 1);
//	}
//	else {
//		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
//	}
//
//
//	//UDP_PLUGIN_API srr3_api_status_e  SRR3_API_GetFrameHdr(const void* const p_bytestream , UDPRecord_Header* p_udp_hdr)
//
//	unsigned char* bytestream = &pRec->payload[0];
//
//	m_pDecoderPlugin->SRR3_API_GetFrameHdr(bytestream, &udp_frame_hdr);
//	short source_no = -1;
//	short stream_no = -1;
//
//	if (Status_Latch_Completed == m_pDecoderPlugin->SRR3_API_LatchData(bytestream, pRec->pcTime, &source_no, &stream_no, 1)) { ///for example read detection from rear right radar when latch is complete
//
//		Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
//		unsigned char version = 0;
//		if (stream_no == Z7B_LOGGING_DATA) {
//			if (Status_OK == m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out, (unsigned char)source_no, (unsigned char)stream_no)) {
//
//
//				UDPRecord_Header* pudp_frame_hdr = &p_latch_out->proc_info.frame_header;
//
//				switch (stream_no) {
//				case Z7B_LOGGING_DATA:
//					if (!m_pSrcStreams->m_pStreams[stream_no]) {
//						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21, Z7B, pudp_frame_hdr->streamVersion, cust_id);
//					}
//
//					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//					size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//					ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//					break;
//
//				case Z7A_LOGGING_DATA:
//					if (!m_pSrcStreams->m_pStreams[stream_no]) {
//						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21, Z7A, pudp_frame_hdr->streamVersion, cust_id);
//					}
//
//					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//					size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//					ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//					break;
//
//				case Z4_LOGGING_DATA:
//					if (!m_pSrcStreams->m_pStreams[stream_no]) {
//						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21, Z4, pudp_frame_hdr->streamVersion, cust_id);
//					}
//
//					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//					size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//					ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//					break;
//
//				case SRR3_Z7B_CUST_LOGGING_DATA:
//					if (!m_pSrcStreams->m_pStreams[stream_no]) {
//						m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(21, Z7B_TRACKER, pudp_frame_hdr->streamVersion, cust_id);
//					}
//
//					m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//					*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//					pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//					size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//					ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//					break;
//
//				default:
//					pBuffer = NULL;
//					break;
//				}
//
//
//				/*if(pBuffer){
//
//				PackUdpLogging(pBuffer,
//				size,
//				pudp_frame_hdr->sourceInfo,
//				pudp_frame_hdr->streamNumber,
//				pudp_frame_hdr->streamRefIndex,
//				ver,
//				*pDestQueue);
//				}*/
//			}
//		}
//	}
//
//}

//void DumpCalib(FILE* fptr, enDumpOpt opt, const unsigned char* bytestream)
//{
//	UDPRecord_Header  udp_frame_hdr = { 0 };
//	m_pDecoderPlugin->SRR3_API_GetFrameHdr((unsigned char*)bytestream, &udp_frame_hdr);
//	if (udp_frame_hdr.streamNumber == CALIBRATION_DATA)
//	{
//		switch (opt)
//		{
//		case UDP_CALIB_USC:
//			ExtractandListCalib(fptr, &udp_frame_hdr, bytestream + sizeof(udp_frame_hdr));
//			//ExtractandListCalib(fptr,&udp_frame_hdr,bytestream+sizeof(udp_frame_hdr));
//			break;
//
//		default:
//			break;
//		}
//	}
//
//}

//int ExtractAndConvert(const char* m_LogFname, DVSU_RECORD_T* pRec)
//{
//	int ret = 0;
//	//ARSTREAMS *pSrcQueue = &m_WorkingQueue;
//
//	for_each(m_TxQueue.begin(), m_TxQueue.end(), DeleteContent<CByteStream>());
//	m_TxQueue.clear();
//
//	ARSTREAMS *pDestQueue = &m_TxQueue;
//
//	short source_no = -1;
//	short stream_no = -1;
//	srr3_api_status_e status = Status_Latch_Error;
//
//	unsigned char* bytestream = &pRec->payload[0];
//	unsigned64_T timestamp = (unsigned64_T)pRec->pcTime;
//	Radar_UDP_Frame_Latch_T* p_latch_out = NULL;
//	unsigned int iStrVerNoZ7A = 0, iStrVerNoZ7B = 0, iStrVerNoZ4 = 0, iStrVerNoZ7B_Custom = 0, iStrVerNoZ4_Custom = 0;
//	unsigned int SourceId_z4 = 0, StreamRefIndex = 0;
//	/*status = m_pDecoderPlugin->SRR3_API_LatchData(bytestream,timestamp,&source_no, &stream_no,1);
//	if(status != Status_Latch_Completed)
//	return 0;
//	;*/
//	unsigned char sourceId = 0;
//
//	if (pRec->payload[8] >= UDP_PLATFORM_SRR5) { // Checking is the platform is other than SRR3
//		sourceId = (pRec->payload[9] - 1);
//	}
//	else {
//		sourceId = pRec->payload[8] - UDP_PLATFORM_CUST_SRR3_RL;
//	}
//
//
//	////g_pIRadarStrm->FillStreamData(&p_latch_out->data[0],p_latch_out->proc_info.nDataLen);
//
//
//	//Non-CompATIBLE SOURCE DESTINAITON
//	for_each(pDestQueue->begin(), pDestQueue->end(), DeleteContent<CByteStream>());
//	pDestQueue->clear();
//
//	void* pBuffer = NULL;
//	const int MAX_BACK_BUFFER = 100000;
//	static unsigned char final_buffer[MAX_BACK_BUFFER] = { 0 };
//
//	memset(final_buffer, 0, MAX_BACK_BUFFER);
//
//	unsigned int size = 0;
//	unsigned char ver = 0;
//	UDPRecord_Header udp_hdr = { 0 };
//	m_pDecoderPlugin->SRR3_API_GetFrameHdr(bytestream, &udp_hdr);
//	if (udp_hdr.streamNumber == CDC_DATA) {
//		m_pLogPlugin->SRR3_mudp_write(mudp_log_out, (unsigned char*)pRec);
//		return 0;
//	}
//
//	if (Status_Latch_Completed != m_pDecoderPlugin->SRR3_API_LatchData(bytestream,
//		time_t(), &source_no, &stream_no, 1))
//	{
//		return 0;
//	}
//
//
//
//	if (Status_OK == m_pDecoderPlugin->SRR3_API_GetLastLatchData(&p_latch_out, (unsigned char)source_no, (unsigned char)stream_no))
//	{
//
//		UDPRecord_Header* pudp_frame_hdr = &p_latch_out->proc_info.frame_header;
//
//		switch (stream_no)
//		{
//		case Z7B_LOGGING_DATA:
//			if (!m_pSrcStreams->m_pStreams[stream_no])
//			{
//				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7B, pudp_frame_hdr->streamVersion, cust_id);
//				fprintf(stdout, "\n z7b version in File :: %d", pudp_frame_hdr->streamVersion);
//			}
//			if (pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7b)
//			{
//				fprintf(stdout, "\n z7b version in File is greater than Sensors,Stopping Execution!!!");
//				return FALSE;
//			}
//			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7B, pudp_frame_hdr->streamVersion, cust_id);
//			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//			break;
//
//		case Z7A_LOGGING_DATA:
//			if (!m_pSrcStreams->m_pStreams[stream_no])
//			{
//				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7A, pudp_frame_hdr->streamVersion, cust_id);
//				fprintf(stdout, "\n z7a version in File :: %d", pudp_frame_hdr->streamVersion);
//			}
//			if (pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7a)
//			{
//				fprintf(stdout, "\n z7a version in File is greater than Sensors,Stopping Execution!!!");
//				return FALSE;
//			}
//			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7A, pudp_frame_hdr->streamVersion, cust_id);
//			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//			break;
//
//		case Z4_LOGGING_DATA:
//			if (!m_pSrcStreams->m_pStreams[stream_no])
//			{
//				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z4, pudp_frame_hdr->streamVersion, cust_id);
//				fprintf(stdout, "\n z4 version in File :: %d", pudp_frame_hdr->streamVersion);
//			}
//			if (pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z4)
//			{
//				fprintf(stdout, "\n z4 version in File is greater than Sensors,Stopping Execution!!!");
//				return FALSE;
//
//			}
//			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z4, pudp_frame_hdr->streamVersion, cust_id);
//			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//			break;
//
//		case SRR3_Z7B_CUST_LOGGING_DATA:
//			if (!m_pSrcStreams->m_pStreams[stream_no])
//			{
//				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7B_CUSTOM, pudp_frame_hdr->streamVersion, cust_id);
//				fprintf(stdout, "\n z7b_Custom version in File :: %d", pudp_frame_hdr->streamVersion);
//			}
//			if (pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z7b_cust)
//			{
//				fprintf(stdout, "\n z7b_Custom version in File is greater than Sensors,Stopping Execution!!!");
//				return FALSE;
//			}
//			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z7B_CUSTOM, pudp_frame_hdr->streamVersion, cust_id);
//			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//			break;
//
//		case Z4_CUST_LOGGING_DATA:
//			if (!m_pSrcStreams->m_pStreams[stream_no])
//			{
//				m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z4_CUSTOM, pudp_frame_hdr->streamVersion, cust_id);
//				fprintf(stdout, "\n z4_Custom version in File :: %d", pudp_frame_hdr->streamVersion);
//			}
//			if (pudp_frame_hdr->streamVersion > m_version_info.strm_ver.z4_cust)
//			{
//				fprintf(stdout, "\n z4_Custom version in File is greater than Sensors,Stopping Execution!!!");
//				return FALSE;
//			}
//			m_pSrcStreams->m_pStreams[stream_no] = (IRadarStream*)z_logging::GetInstance(sourceId)->Create(pudp_frame_hdr->Platform, Z4_CUSTOM, pudp_frame_hdr->streamVersion, cust_id);
//			m_pSrcStreams->m_pStreams[stream_no]->FillStreamData(p_latch_out->data, p_latch_out->proc_info.nDataLen);
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//			iStrVerNoZ4_Custom = pudp_frame_hdr->streamVersion;
//			SourceId_z4 = pudp_frame_hdr->Radar_Position;
//			StreamRefIndex = pudp_frame_hdr->streamRefIndex;
//			break;
//
//		default:
//			pBuffer = NULL;
//			break;
//		}
//
//
//		if (pBuffer)
//		{
//
//			IRadarStream::reverse_copy_data((unsigned char*)pBuffer, final_buffer, size);
//			PackUdpLogging(final_buffer,
//				size,
//				pudp_frame_hdr->Radar_Position,
//				pudp_frame_hdr->streamNumber,
//				pudp_frame_hdr->streamRefIndex,
//				ver,
//				*pDestQueue);
//
//			memset(pBuffer, 0, MAX_BACK_BUFFER);
//			/*memset((void*)&recordOut,0,sizeof(recordOut));
//			memcpy(&recordOut.payload[0], pDestQueue, MAX_MUDP_PACKET_SIZE);
//			m_pLogPlugin->SRR3_mudp_write(mudp_log_out,(unsigned char*)&recordOut);
//			ret = 0;*/
//		}
//	}
//
//
//
//	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
//	// If the z4_audi stream not available in older logs, take the data from z4 and fill it to z4_custom and pass it to pDestQueue
//	pBuffer = NULL;
//	if ((0 == iStrVerNoZ4_Custom))
//	{
//		if ((NULL != m_pSrcStreams->m_pStreams[stream_no]) && ((iStrVerNoZ4 < 13) && (iStrVerNoZ4 != 0)) && (1 == m_version_info.strm_ver.z4_cust))
//		{
//			*m_pDestStreams->m_pStreams[stream_no] = *m_pSrcStreams->m_pStreams[stream_no];
//			pBuffer = m_pDestStreams->m_pStreams[stream_no]->get_complete_Buffer();
//			size = m_pDestStreams->m_pStreams[stream_no]->get_size();
//			ver = m_pDestStreams->m_pStreams[stream_no]->get_version();
//
//			if (pBuffer)
//			{
//				PackUdpLogging(pBuffer,
//					size,
//					SourceId_z4,
//					Z4_CUST_LOGGING_DATA,
//					StreamRefIndex,
//					m_version_info.strm_ver.z4_cust,
//					*pDestQueue);
//			}
//		}
//	}
//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	return ret;
//}

int CreateOutputDVSUFile(const char* m_LogFname)
{

	char m_strDVSUOutputFile[1024];

	memset(m_strDVSUOutputFile, 0, 1024);
	int length = strlen(m_LogFname);
	int TargetLength = length - 5; //Remove ".dvsu" from original name
	strncpy(m_strDVSUOutputFile, m_LogFname, TargetLength);
	strcat(m_strDVSUOutputFile, "_Out.dvsu");
	//CHiLController::GetInstance()->m_pLogPlugin->SRR3_mudp_create(m_strDVSUOutputFile,MAX_MUDP_PACKET_SIZE,&mudp_log_out);

	if (E_MUDPLOG_OK != m_pLogPlugin->SRR3_mudp_create(m_strDVSUOutputFile, MAX_MUDP_PACKET_SIZE, &mudp_log_out))
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

	for (; it != streams.end(); ++it)
	{
		memset((void*)&recordOut, 0, sizeof(recordOut));
		memcpy(&recordOut.payload[0], ((*it)->GetStreamPtr()), MAX_MUDP_PACKET_SIZE);
		m_pLogPlugin->SRR3_mudp_write(mudp_log_out, (unsigned char*)&recordOut);
	}
}

void CheckCsvSplitting(std::string csvPath)
{
#if defined(_WIN_PLATFORM)
	char drvName[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fName[_MAX_FNAME];
	char ext[_MAX_EXT];
	split_path(csvPath.c_str(), drvName, dir, fName, ext);
#elif defined(__GNUC__)
	char* drvName = new char[_MAX_DRIVE];
	char* dir = new char[_MAX_DIR];
	char* fName = new char[_MAX_FNAME];
	char* ext = new char[_MAX_EXT];
	split_pathLinux(csvPath.c_str(), &drvName, &dir, &fName, &ext);
#endif

	if ((1 != MUDP_master_config->data_extracter_mode[PACKET_Loss]) && (1 != MUDP_master_config->data_extracter_mode[Quality_Check]) && (1 != MUDP_master_config->data_extracter_mode[XML]) && (1 != MUDP_master_config->Radar_Stream_Options[HDR]))
	{
		for (auto iterator = gcsvDetails.begin(); iterator != gcsvDetails.end(); iterator++)
		{
			char cFileName[MAX_FILEPATH_NAME_LENGTH];
			char cStreamName[25] = "\0";

			if ((0 == iterator->streamName.compare("C0_CORE_MASTER_STREAM")) || (0 == iterator->streamName.compare("C1_CORE_MASTER_STREAM")) 
				|| (0 == iterator->streamName.compare("C2_CORE_MASTER_STREAM")) || (0 == iterator->streamName.compare("DETECTION_STREAM")) || (0 == iterator->streamName.compare("RDD_STREAM")))
				//if ((C0_CORE_MASTER_STREAM == iterator->iStreamNum) || (C1_CORE_MASTER_STREAM == iterator->iStreamNum) || (C2_CORE_MASTER_STREAM == iterator->iStreamNum) || (e_RDD_CORE_LOGGING_STREAM == iterator->iStreamNum))
			{
				if (strcmp(MUDP_master_config->GenType, "GEN7") == 0)
				{
					switch (iterator->iStreamNum)
					{
						case DETECTION_STREAM:
							std::strcpy(cStreamName, "UDP_GEN7_DET_CORE");
							break;
						case RDD_STREAM:
							std::strcpy(cStreamName, "UDP_GEN7_RDD_CORE");
							break;
					}
				}
				else
				{
					switch (iterator->iStreamNum)
					{
						case C0_CORE_MASTER_STREAM:
							std::strcpy(cStreamName, "UDP_GEN5_C0_CORE");
							break;
						case C1_CORE_MASTER_STREAM:
							std::strcpy(cStreamName, "UDP_GEN5_C1_CORE");
							break;
						case C2_CORE_MASTER_STREAM:
							std::strcpy(cStreamName, "UDP_GEN5_C2_CORE");
							break;
						default:
							break;
					}
				}

				for (int iCounter = 0; iCounter < MAX_RADAR_COUNT; iCounter++)
				{
					memset(cFileName, '\0', MAX_FILEPATH_NAME_LENGTH);

					if (1 == MUDP_master_config->MUDP_Select_Sensor_Status[iCounter])
					{
						sprintf(cFileName, "%s%s%s_%s_%s.csv", drvName, dir, fName, GetRadarPosName(iCounter), cStreamName);
											
						if (-1 != CSV_file_exist(cFileName, 0))
						{
							CSVSplitter* splitterPtr = new CSVSplitter();
							if (splitterPtr)
							{
								splitterPtr->SetCSVData(false, false, false, cFileName);
								splitterPtr->ReadCSVHeaderCount();

								if (splitterPtr->getCSVColumnCount() > MAX_EXCEL_CSV_COLUMN_COUNT)
									splitterPtr->SplitCSV();

								delete splitterPtr;
								splitterPtr = nullptr;
							}
						}
					}
				}
			}
			else if (0 == iterator->streamName.compare("DSPACE_CUSTOMER_DATA"))
				//else if (DSPACE_CUSTOMER_DATA == iterator->iStreamNum)
			{
				memset(cFileName, '\0', MAX_FILEPATH_NAME_LENGTH);
				std::strcpy(cStreamName, "DSPACE");
				sprintf(cFileName, "%s%s%s_%s.csv", drvName, dir, fName, cStreamName);

				if (-1 != CSV_file_exist(cFileName, 0))
				{
					CSVSplitter* splitterPtr = new CSVSplitter();
					if (splitterPtr)
					{
						splitterPtr->SetCSVData(false, false, false, cFileName);
						splitterPtr->ReadCSVHeaderCount();

						if (splitterPtr->getCSVColumnCount() > MAX_EXCEL_CSV_COLUMN_COUNT)
							splitterPtr->SplitCSV();

						delete splitterPtr;
						splitterPtr = nullptr;
					}
				}
			}
		}
	}
#if defined(__GNUC__)
	delete[]drvName;
	delete[]dir;
	delete[]fName;
	delete[]ext;
#endif
}

void GetCsvSplitFileDetails(Customer_T custID, unsigned int Stream_Num, unsigned int id)
{
	csvDetails tempObj;
	tempObj.iCustomerID = custID;
	tempObj.iStreamNum = Stream_Num;

	switch (id)
	{
	case 0 : 
		tempObj.streamName = GetStreamName(Stream_Num);		
		break;
	case 1 : 
		tempObj.streamName = Get_GEN5StreamName(Stream_Num); 
		break;
	case 2 : 
		tempObj.streamName = Get_FLR4PGEN5StreamName(Stream_Num); 
		break;
	case 3:
		tempObj.streamName = Get_GEN7StreamName(Stream_Num);
		break;
	default:
		tempObj.streamName = "INVALID_STREAM";
		break;
	}
	
	gcsvDetails.push_back(tempObj);
}

