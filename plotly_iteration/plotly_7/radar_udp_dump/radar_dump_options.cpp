// srr3_dump_options.cpp 

#include "stdafx.h"
//#include <conio.h>
//#include <windows.h>
#include "../../../CommonFiles/inc/SRR3_MUDP_API.h"
#include "../../../CommonFiles/CommonHeaders/dph_rr_adas_config_mode.h"
#include <algorithm>
#include <math.h>
#include <sstream>
#include "../../../CoreLibraryProjects/mdf_log/inc/apt_mdf_log.h"
using namespace std;
#pragma comment(lib,"Winmm.lib")

#define MILLI2MICRO(tm)  (tm*1000)
bool bOnce[MAX_RADAR_COUNT] = {0};
extern uint64_t obsoluteTimestamp_t;

static char* GetCustIDName(uint8_t custID, uint8_t platform ){
		static char customerName[40] = {0};
	if ((custID == STLA_SCALE1 || custID == STLA_SCALE3 || custID == STLA_SCALE4) && (platform == UDP_SOURCE_CUST_DSPACE))
	{
		switch (custID)
		{
		case STLA_SCALE1:sprintf(customerName, "%d_STLA_SCALE1", custID); break;
		case STLA_SCALE3:sprintf(customerName, "%d_STLA_SCALE3", custID); break;
		case STLA_SCALE4:sprintf(customerName, "%d_STLA_SCALE4", custID); break;
		}
	}
	else
	{
		switch (custID) {
		case VOLVO: sprintf(customerName, "%d_VOLVO", custID); break;
		case BMW: sprintf(customerName, "%d_BMW", custID); break;
		case GM: sprintf(customerName, "%d_GM", custID); break;
		case HYUNDAI: sprintf(customerName, "%d_HYUNDAI", custID); break;
		case FORD: sprintf(customerName, "%d_FORD", custID); break;
		case SGM: sprintf(customerName, "%d_SGM", custID); break;
		case VGTT: sprintf(customerName, "%d_VGTT", custID); break;
		case AUDI: sprintf(customerName, "%d_AUDI", custID); break;
		case JLR: sprintf(customerName, "%d_JLR", custID); break;
		case CHANGAN: sprintf(customerName, "%d_CHANGAN", custID); break;
		case SCANIA_MAN: sprintf(customerName, "%d_MAN_SRR3", custID); break;
		case GEELY: sprintf(customerName, "%d_GEELY", custID); break;
		case HKMC_GEN2: sprintf(customerName, "%d_HKMC_GEN2", custID); break;
		case GWM_SRR3: sprintf(customerName, "%d_GWM_SRR3", custID); break;
		case MAXUS: sprintf(customerName, "%d_MAXUS", custID); break;
		case RNA_SUV: sprintf(customerName, "%d_RNA_SUV", custID); break;
		case RNA_CDV: sprintf(customerName, "%d_RNA_CDV", custID); break;
		case HKMC_SRR5: sprintf(customerName, "%d_HKMC_SRR5", custID); break;
		case GWM_SRR5: sprintf(customerName, "%d_GWM_SRR5", custID); break;
		case FORD_SRR5: sprintf(customerName, "%d_FORD_SRR5", custID); break;
		case GEELY_SRR5: sprintf(customerName, "%d_GEELY_SRR5", custID); break;
			//case BMW_ECU: sprintf(customerName, "%d_BMW_ECU", custID); break;
		case BMW_LOW: sprintf(customerName, "%d_BMW_LOW", custID); break;
		case BMW_SAT: sprintf(customerName, "%d_BMW_SAT", custID); break;
		case BMW_BPIL: sprintf(customerName, "%d_BMW_BPIL", custID); break;
		case CHANGAN_SRR5: sprintf(customerName, "%d_CHANGAN_SRR5", custID); break;
		case HKMC_SRR: sprintf(customerName, "%d_HKMC_SRR", custID); break;
		case SGM_358_SRR5: sprintf(customerName, "%d_SGM_358_SRR5", custID); break;
		case NISSAN_GEN5: sprintf(customerName, "%d_NISSAN_GEN5", custID); break;//
		case PLATFORM_GEN5: sprintf(customerName, "%d_PLATFORM_GEN5", custID); break;
		case HONDA_GEN5:sprintf(customerName, "%d_HONDA_GEN5", custID); break;
		case MOTIONAL_FLR4P:sprintf(customerName, "%d_MOTIONAL_FLR4P", custID); break;
		case TML_SRR5: sprintf(customerName, "%d_TML_SRR5", custID); break;
		case STLA_SRR6P:sprintf(customerName, "%d_STLA_SRR6P", custID); break;
		case STLA_FLR4:sprintf(customerName, "%d_STLA_FLR4", custID); break;
		case STLA_FLR4P:sprintf(customerName, "%d_STLA_FLR4P", custID); break;
		case STLA_SCALE4:sprintf(customerName, "%d_STLA_SCALE4", custID); break;
		case BMW_SP25_L2:sprintf(customerName, "%d_BMW_SP25_L2", custID); break;
		case BMW_SP25_L3:sprintf(customerName, "%d_BMW_SP25_L3", custID); break;
		case RIVIAN_SRR6P:sprintf(customerName, "%d_RIVIAN_SRR6P", custID); break;
		default: sprintf(customerName, "%d_INVALID_CUSTOMER", custID); break;
		}
	}

	return customerName;
}
static char* GetPlatformNumberName(uint8_t platform)
{
	static char strRet[100] = { 0 };

	switch (platform)
	{
	case 48: return "48_SRR6";
	case 49: return "49_FLR4";
	case 50: return "50_SRR6P";
	case 51: return "51_FLR4P_SATELLITE";
	case 52: return "52_SRR6P_1x_CAN";
	case 53: return "53_SRR6P_1x_ETH";
	case 54: return "54_SRR6P_1x_1GBPS_ETH";
	case 55: return "55_FLR4P_STANDALONE";
	case 63: return "63_dSPACE";
	//GEN7
	case 72: return "72_SRR7P";
	case 73: return "73_FLR7";
	case 75: return "75_SRR7P_HD";
	case 76: return "75_SRR7E";
	
	default:sprintf(strRet, "UNKNOWN_PLATFORM_ID_%d", platform); break;

	}
	return strRet;
}
static char* GetRadarNumberName(uint8_t Radar_Position)
{
	static char strRet[100] = { 0 };

	switch (Radar_Position)
	{
	case 1: return "1_RL";
	case 2: return "2_RR";
	case 3: return "3_FR";
	case 4: return "4_FL";
	case 5: return "5_RC";
	case 6: return "6_FC";
	case 7: return "7_BP_R";
	case 8: return "8_BP_L";
	case 9: return "9_FLR4P_FC";
	case 20: return "20_ECU";

	default:sprintf(strRet, "UNKNOWN_RADAR_%d", Radar_Position); break;

	}
	return strRet;
}
static char* GetGEN7streamNumberName(uint8_t streamNo)
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
static char* GetSRR6streamNumber(uint8_t streamNo)
{
	static char strRet[100] = { 0 };
	static char str_0[] = "00_C0_RESIM_CORE_MASTER_STREAM";
	static char str_1[] = "01_C0_RESIM_CUST_MASTER_STREAM";
	static char str_2[] = "02_C1_RESIM_CORE_MASTER_STREAM";
	static char str_3[] = "03_C2_RESIM_CORE_MASTER_STREAM";
	static char str_4[] = "04_C2_RESIM_CUST_MASTER_STREAM";
	static char str_5[] = "05_OSI_GEN6_STREAM";
	static char str_15[] = "15-CDC_8_IQ";
	static char str_16[] = "16_CDC_12_IQ";
	static char str_38[] = "38_GDSR_GEN5_STREAM";
	static char str_96[] = "96_CALIB_STREAM";

	static char str_21[] = "21_BC_CORE_LOGGING_STREAM";
	static char str_22[] = "22_VSE_CORE_LOGGING_STREAM";
	static char str_23[] = "23_STATUS_CORE_LOGGING_STREAM";
	static char str_24[] = "24_RDD_CORE_LOGGING_STREAM";
	static char str_25[] = "25_DET_LOGGING_STREAM";
	static char str_26[] = "26_HDR_LOGGING_STREAM";
	static char str_28[] = "28_TRACKER_LOGGING_STREAM";
	static char str_34[] = "34_ALIGNMENT_LOGGING_STREAM";
	static char str_39[] = "39_DEBUG_LOGGING_STREAM";

	switch (streamNo)
	{
	case 0: return str_0;
	case 1: return str_1;
	case 2: return str_2;
	case 3: return str_3;
	case 4: return str_4;
	case 5: return str_5;
	case 15: return str_15;
	case 16: return str_16;
	case 21: return str_21;
	case 22: return str_22;
	case 23: return str_23;
	case 24: return str_24;
	case 25: return str_25;
	case 26: return str_26;
	case 28: return str_28;
	case 34: return str_34;
	case 38: return str_38;
	case 39: return str_39;
	case 96: return str_96;
	default:sprintf(strRet, "UNKNOWN_STREAM_NUMBER_%d", streamNo); break;

	}
	return strRet;

}
static char* GetSRR5streamNumber(uint8_t streamNo){
	static char strRet[100] = {0};

	static char str_0[]  = "0-Z7A_CORE";
	static char str_20[] = "20-Z7A_116_CORE";
	static char str_21[] = "21-Z7A_210_CORE";
	static char str_23[] = "23-Z7A_64D_CORE";
	static char str_24[] = "24-Z7A_150D_CORE";
	static char str_25[] = "25-Z7A_200D_CORE";
	static char str_27[] = "27-Z7A_64D_CORE";
	static char str_28[] = "28-Z7A_SRR5P_116RB_150D_CORE";
	static char str_29[] = "29-Z7A_SRR5P_116RB_200D_CORE";
	static char str_111[] = "111_GDSR_Tracker";
	static char str_67[] = "67_OSI_Tracker";

	static char str_1[]  = "1-Z7B_CORE";
	static char str_10[] = "10-DSPACE_CUST";
	static char str_40[] = "40-Z7B_64D_SAT_CORE";
	static char str_41[] = "41-Z7B_200D_SAT_CORE";
	static char str_42[] = "42-Z7B_FRNT_128D_SAT_CORE";
	static char str_43[] = "43-Z7B_64FD_64TRK_CORE";
	static char str_44[] = "44-Z7B_64TRK_CORE";
	static char str_45[] = "45-Z7B_BPIL_150D_CORE";
	static char str_46[] = "46-Z7B_FRNT_64SD_64FD_SAT_CORE";
	static char str_49[] = "49_Z7B_64SD_64FD_64OBJ_STAND_CORE";

	static char str_3[]  = "3-Z4_CORE";
	static char str_50[] = "50-Z4_CORE";
	static char str_51[] = "51-Z4_64_FD_CORE";

	static char str_4[]  = "4-CALIB_STREAM";

	static char str_7[]  = "7-Z7B_CUST";
	static char str_70[] = "70-Z7B_CUST";

	static char str_9[]  = "9-Z4_CUST";
	static char str_80[] = "80-Z4_CUST";

	static char str_6[]  = "6-CDC_SRR3";
	static char str_15[] = "15-CDC_8_IQ";
	static char str_16[] = "16-CDC_12_IQ";

	static char str_ECU_Core0[] = "90_ECU_TRACKER_IAL_CORE0";
	static char str_ECU_Core1[] = "91_ECU_TRACKER_OAL_CORE1";
	static char str_ECU_Core3[] = "92_ECU_FF_OAL_CORE3";
	static char str_ECU_OG[]	= "93_ECU_OG_INTERNAL";
	static char str_ECU_Cal[]	= "94_ECU_CALIB";
	static char str_ECU_F360[]  = "95_ECU_F360_INTERNAL";
	static char str_ECU_VRU_CLSSIFIER[] = "96_ECU_VRU_CLSSIFIER";

	switch (streamNo){

	case 0: return str_0;
	case 10: return str_10;
	case 20: return str_20;
	case 21: return str_21;
	case 23: return str_23;
	case 24: return str_24;
	case 25: return str_25;		
	case 27: return str_27;
	case 28: return str_28;
	case 29: return str_29;
	case 111: return str_111;
	case 67: return str_67;


	case 1: return str_1;
	case 40: return str_40;
	case 41: return str_41;
	case 42: return str_42;
	case 43: return str_43;
	case 44: return str_44;
	case 45: return str_45;
	case 46: return str_46;
	case 49:return str_49;

	case 3: return str_3;
	case 50: return str_50;
	case 51: return str_51;

	case 7: return str_7;
	case 70: return str_70;

	case 9: return str_9;
	case 80: return str_80;

	case 6: return str_6;
	case 15: return str_15;
	case 16: return str_16;

	case 4: return str_4;

	case 90: return str_ECU_Core0;
	case 91: return str_ECU_Core1;
	case 92: return str_ECU_Core3;
	case 93: return str_ECU_OG;
	case 94: return str_ECU_Cal;
	case 95 :return str_ECU_F360;
	case 96: return str_ECU_VRU_CLSSIFIER;
	default:sprintf(strRet, "UNKNOWN_STREAM_NUMBER_%d", streamNo); break;


	}
	return strRet;
}

static char* GetcalsourceName(uint8_t CalSource, uint8_t streamNo, bool platformGEN5) {
	static char CalSourceName[40] = {0};
	bool is_calibStream = false;

	if (!platformGEN5 && streamNo == CALIBRATION_DATA)  // SRR5
	{
		if(CalSource == 0 || CalSource == 1 || CalSource > 47){
			switch (CalSource) {
			case 0: sprintf(CalSourceName, "0%d_USC", CalSource); break;
			case 1: sprintf(CalSourceName, "0%d_SMC", CalSource); break;
			default: sprintf(CalSourceName, "%d_UNKNOWN_CALSOURCE", CalSource); break;
			}
			return CalSourceName;
		}
		is_calibStream = true;
	}
	else if(platformGEN5 && streamNo == CALIB_DATA) // SRR6
			is_calibStream = true;
	
	if (is_calibStream){
		switch (CalSource) {
		case USC_FLASH: sprintf(CalSourceName, "0%d_USC_FLASH", CalSource); break;
		case SMC_FLASH: sprintf(CalSourceName, "0%d_SMC_FLASH", CalSource); break;
		case RSDS_FEATURE_FUNCTION_CAL: sprintf(CalSourceName, "0%d_RSDS_FEATURE_FUNCTION_CAL", CalSource); break;
		case RSDS_TRACKER_CAL: sprintf(CalSourceName, "0%d_RSDS_TRACKER_CAL", CalSource); break;
		case DSD_CAL: sprintf(CalSourceName, "0%d_DSD_CAL", CalSource); break;
		case TRACKER_CAL_DF_V: sprintf(CalSourceName, "0%d_TRACKER_CAL_DF_V", CalSource); break;
		case CTA_CAL_DF_V: sprintf(CalSourceName, "0%d_CTA_CAL_DF_V", CalSource); break;
		case LCDA_CAL_DF_V: sprintf(CalSourceName, "0%d_LCDA_CAL_DF_V", CalSource); break;
		case FENCES_CAL_DF_V: sprintf(CalSourceName, "0%d_FENCES_CAL_DF_V", CalSource); break;
		case RECW_CAL_DF_V: sprintf(CalSourceName, "0%d_RECW_CAL_DF_V", CalSource); break;
		case ASW_CAL_DF_V: sprintf(CalSourceName, "%d_ASW_CAL_DF_V", CalSource); break;
		case OH_CAL_DF_V: sprintf(CalSourceName, "%d_OH_CAL_DF_V", CalSource); break;
		case ABA_CAL_DF_V: sprintf(CalSourceName, "%d_ABA_CAL_DF_V", CalSource); break;
		case USC_CAL_DF_V: sprintf(CalSourceName, "%d_USC_CAL_DF_V", CalSource); break;
		case DYN_ALIGN_CAL_DF_V: sprintf(CalSourceName, "%d_DYN_ALIGN_CAL_DF_V", CalSource); break;
		case STAT_ALIGN_CAL_DF_V: sprintf(CalSourceName, "%d_STAT_ALIGN_CAL_DF_V", CalSource); break;
		case VISOB_CAL_DF_V: sprintf(CalSourceName, "%d_VISOB_CAL_DF_V", CalSource); break;
		case FREE_SPACE_CAL_DF_V: sprintf(CalSourceName, "%d_FREE_SPACE_CAL_DF_V", CalSource); break;
		case IPC_FAILURE_COUNTS: sprintf(CalSourceName, "%d_IPC_FAILURE_COUNTS", CalSource); break;
		case CORE2_TIMING_INFO: sprintf(CalSourceName, "%d_CORE2_TIMING_INFO", CalSource); break;
		case TRACKER_ERR: sprintf(CalSourceName, "%d_TRACKER_ERR", CalSource); break;
		case OH_AUDI_ERR: sprintf(CalSourceName, "%d_OH_AUDI_ERR", CalSource); break;
		case FENCES_ERR: sprintf(CalSourceName, "%d_FENCES_ERR", CalSource); break;
		case CTA_ERR: sprintf(CalSourceName, "%d_CTA_ERR", CalSource); break;
		case SWA_ERR: sprintf(CalSourceName, "%d_SWA_ERR", CalSource); break;
		case ABA_ERR: sprintf(CalSourceName, "%d_ABA_ERR", CalSource); break;
		case C2_MEM_PROFILING: sprintf(CalSourceName, "%d_C2_MEM_PROFILING", CalSource); break;
		case ARM_DIG: sprintf(CalSourceName, "%d_ARM_DIG", CalSource); break;
		case HUD_CAL_DF_V: sprintf(CalSourceName, "%d_HUD_CAL_DF_V", CalSource); break;//
		case ELK_CAL_DF_V: sprintf(CalSourceName, "%d_ELK_CAL_DF_V", CalSource); break;
		case RCCW_Cal_DF_V:sprintf(CalSourceName, "%d_RCCW_Cal_DF_V", CalSource); break;
		case HKMC_FOW_Cal_DF_V:sprintf(CalSourceName, "%d_FOW_Cal_DF_V", CalSource); break;
		case HKMC_ABSD_Cal_DF_V: sprintf(CalSourceName, "%d_HMC_ABSD_Cal_DF_V", CalSource); break;
		case RR_BCARPLUS_Cal_DF_V:sprintf(CalSourceName, "%d_RR_BCARPLUS_Cal_DF_V", CalSource); break;
		case RR_ACSF_Cal_DF_V:sprintf(CalSourceName, "%d_RR_ACSF_Cal_DF_V", CalSource); break;
		case UDP_LOGGING_VERSION:sprintf(CalSourceName, "%d_UDP_LOGGING_VERSION", CalSource); break;
		case Dataset_Vehicle_Config:sprintf(CalSourceName, "%d_Dataset_Vehicle_Config", CalSource); break;
		case Dataset_VehicleConfig_BSW:sprintf(CalSourceName, "%d_Dataset_VehicleConfig_BSW", CalSource); break;
		case BSW_Parameter:sprintf(CalSourceName, "%d_BSW_Parameter", CalSource); break;
		case RCTA_Parameter:sprintf(CalSourceName, "%d_RCTA_Parameter", CalSource); break;
		case PATH_TRACKING_CAL_F_V:sprintf(CalSourceName, "%d_PATH_TRACKING_CAL_F_V", CalSource); break;
		case FLAT_PLAT_ALIGNMENT:sprintf(CalSourceName, "%d_FLAT_PLAT_ALIGNMENT", CalSource); break;
		case FLAT_PLAT_ALIGNMENT_INTERNALS:sprintf(CalSourceName, "%d_FLAT_PLAT_ALIGNMENT_INTERNALS", CalSource); break;
		case IPC_USC_F18C:sprintf(CalSourceName, "%d_IPC_USC_F18C", CalSource); break;
		case CAFF_LOGGING_STREAM:sprintf(CalSourceName, "%d_CAFF_LOGGING_STREAM", CalSource); break;
		case APPLICATION_ERROR_LOGGING:sprintf(CalSourceName, "%d_APPLICATION_ERROR_LOGGING", CalSource); break;
		case CALIBRATION_ERROR:sprintf(CalSourceName, "%d_CALIBRATION_ERROR", CalSource); break;
		case SMC_RAM:sprintf(CalSourceName, "%d_SMC_RAM", CalSource); break;
		case TODS_CAL_DF_V:sprintf(CalSourceName, "%d_TODS_CAL_DF_V", CalSource); break;
		case CED_CAL_V:sprintf(CalSourceName, "%d_CED_CAL_V", CalSource); break;
		case PT_CAL_V:sprintf(CalSourceName, "%d_PT_CAL_V", CalSource); break;
		case Freespace_Cal_V:sprintf(CalSourceName, "%d_Freespace_Cal_V", CalSource); break;
		default: sprintf(CalSourceName, "%d_UNKNOWN_CALSOURCE", CalSource); break;
		}
	}
	else
	{
		sprintf(CalSourceName, "%d", CalSource);
	}

	return CalSourceName;
}


//void print_udp_header(FILE* fptr, unsigned char*  p_stream);

void print_udp_header(FILE* fptr, UDPRecord_Header* p_udp_hdr, unsigned long pcTime)
{
	static bool bOnce = 1;

	if(bOnce){
		fprintf(fptr,"PC_Time,versionInfo,Platform,Radar_Position,customerID,sourceTxCnt,sourceTxTime,streamNumber,streamVersion,streamRefIndex,streamDataLen,streamTxCnt,streamChunkIdx,streamChunks\n");
		bOnce = 0;

	}
	fprintf(fptr,"%u,%x,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", 
		pcTime,
		p_udp_hdr->versionInfo,
		p_udp_hdr->Platform,
		p_udp_hdr->Radar_Position,
		p_udp_hdr->customerID,
		p_udp_hdr->sourceTxCnt,
		p_udp_hdr->sourceTxTime,
		p_udp_hdr->streamNumber,
		p_udp_hdr->streamVersion,
		p_udp_hdr->streamRefIndex,
		p_udp_hdr->streamDataLen,
		p_udp_hdr->streamTxCnt,
		p_udp_hdr->streamChunkIdx,
		p_udp_hdr->streamChunks
		);

}

void print_gen7_udp_header_ALL(FILE* fptr, GEN7_UDPRecord_Header_T* p_gen7_udp_hdr, signed64_T pcTime)
{
	unsigned char sourceId = 0;
	if (p_gen7_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_PLUS || p_gen7_udp_hdr->sourceInfo == UDP_PLATFORM_FLR7 || p_gen7_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_PLUS_HD || p_gen7_udp_hdr->sourceInfo == UDP_PLATFORM_SRR7_E)
	{
		sourceId = p_gen7_udp_hdr->sensorId;
	}
	if (!bOnce[sourceId]) {
		fprintf(fptr, "PC_Time,versionInfo,reserved,Radar_Pos,Platform,CustID,streamNumber,streamVersion,ScanIndex,streamDataLen,streamTxCnt,streamChunkIdx,Total_num_Chunks,SourceTxCount,SourceTxTime \n");// ,stream_chunk_size,packetTxCnt,packetTxTime,utc_time,timestamp,CalRunningCnt,CalTotalChunk,Cal_Source,detectioncnt,sensorstatus,mode\n");
		bOnce[sourceId] = 1;
	}
	fprintf(fptr, "%u,%x,%u,%s,%s,%s,%s,%u,%u,%u,%u,%u,%u,%u,%u\n", //,%u,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u\n",
		pcTime,
		p_gen7_udp_hdr->versionInfo,
		p_gen7_udp_hdr->reserved,
		GetRadarNumberName(p_gen7_udp_hdr->sensorId),
		GetPlatformNumberName(p_gen7_udp_hdr->sourceInfo),  //sourceInfo -> Platform ID
		"NTD_PLATFORM_GEN7", //GetCustIDName(p_gen7_udp_hdr->customerId, p_gen5_udp_hdr->Platform), //p_gen7_udp_hdr->sourceInfo_1,
		GetGEN7streamNumberName(p_gen7_udp_hdr->streamNumber),
		p_gen7_udp_hdr->streamVersion,
		p_gen7_udp_hdr->streamRefIndex,
		p_gen7_udp_hdr->streamDataLen,
		p_gen7_udp_hdr->streamTxCnt,
		p_gen7_udp_hdr->streamChunkIdx,
		p_gen7_udp_hdr->streamChunks,
		p_gen7_udp_hdr->sourceTxCnt,
		p_gen7_udp_hdr->sourceTxTime//,
		//p_gen7_udp_hdr->stream_chunk_size,
		//p_gen7_udp_hdr->packetTxCnt,
		//p_gen7_udp_hdr->packetTxTime,
		//p_gen7_udp_hdr->utc_time,
		//p_gen7_udp_hdr->timestamp,
		//p_gen7_udp_hdr->Cal_Running_Cnt,
		//p_gen7_udp_hdr->Total_Cal_Chunk_Cnt,
		//GetcalsourceName(p_gen5_udp_hdr->Cal_Source, p_gen5_udp_hdr->stream_Id, 1), //p_gen5_udp_hdr->Cal_Source,
		//p_gen7_udp_hdr->detectioncnt,
		//p_gen7_udp_hdr->sensorstatus,
		//p_gen7_udp_hdr->mode
	);

}

void print_gen5_udp_header_ALL(FILE* fptr , GEN5_UDPRecord_Header_T* p_gen5_udp_hdr, signed64_T pcTime)
{
	unsigned char sourceId = 0;
	if (p_gen5_udp_hdr->Platform == UDP_PLATFORM_SRR6 || p_gen5_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS || p_gen5_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN || p_gen5_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || p_gen5_udp_hdr->Platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || p_gen5_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS || p_gen5_udp_hdr->Platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE || p_gen5_udp_hdr->Platform == UDP_PLATFORM_STLA_FLR4)
	{
		sourceId = p_gen5_udp_hdr->Radar_Position;
	}
	if (!bOnce[sourceId]) {
		fprintf(fptr, "PC_Time,versionInfo,reserved,Radar_Pos,Platform,CustID,streamNumber,streamVersion,ScanIndex,streamDataLen,streamTxCnt,streamChunkIdx,Total_num_Chunks,stream_chunk_size,packetTxCnt,packetTxTime,utc_time,timestamp,CalRunningCnt,CalTotalChunk,Cal_Source,detectioncnt,sensorstatus,mode\n");
		bOnce[sourceId] = 1;
	}
	fprintf(fptr, "%u,%x,%u,%u,%u,%s,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%s,%u,%u,%u\n",
		pcTime,
		p_gen5_udp_hdr->Aptiv_HDR_Version,
		p_gen5_udp_hdr->reserved,
		p_gen5_udp_hdr->Radar_Position,
		p_gen5_udp_hdr->Platform,
		GetCustIDName(p_gen5_udp_hdr->customerId, p_gen5_udp_hdr->Platform), //p_gen5_udp_hdr->customerId,
		GetSRR6streamNumber(p_gen5_udp_hdr->stream_Id),
		p_gen5_udp_hdr->streamVersion,
		p_gen5_udp_hdr->scan_index,
		p_gen5_udp_hdr->streamDataLen,
		p_gen5_udp_hdr->streamTxCnt,
		p_gen5_udp_hdr->streamChunkIdx,
		p_gen5_udp_hdr->Total_num_Chunks,
		p_gen5_udp_hdr->stream_chunk_size,
		p_gen5_udp_hdr->packetTxCnt,
		p_gen5_udp_hdr->packetTxTime,
		p_gen5_udp_hdr->utc_time,
		p_gen5_udp_hdr->timestamp,
		p_gen5_udp_hdr->Cal_Running_Cnt,
		p_gen5_udp_hdr->Total_Cal_Chunk_Cnt,
		GetcalsourceName(p_gen5_udp_hdr->Cal_Source, p_gen5_udp_hdr->stream_Id,1), //p_gen5_udp_hdr->Cal_Source,
		p_gen5_udp_hdr->detectioncnt,
		p_gen5_udp_hdr->sensorstatus,
		p_gen5_udp_hdr->mode
	);

}

void print_udp_header_All(FILE* fptr, UDPRecord_Header* p_udp_hdr, udp_custom_frame_header_t* p_cust_hdr, signed64_T pcTime)
{
	unsigned char sourceId = 0;
	if (p_udp_hdr->Platform >= UDP_PLATFORM_CUST_SRR3_RL && p_udp_hdr->Platform <= UDP_PLATFORM_CUST_SRR3_FC)
	{
		sourceId = p_cust_hdr->sensorid - 1;
	}
	else if (p_udp_hdr->Platform == UDP_SOURCE_CUST_DSPACE)
	{
		sourceId = RDR_POS_DSPACE;
	}
	else if (p_udp_hdr->Platform == BPILLAR_STATUS_BP_L || p_udp_hdr->Platform == UDP_PLATFORM_FORD_SRR5) {
		sourceId = RDR_POS_FC;
	}
	else
	{
		sourceId = p_udp_hdr->Radar_Position - 1;
	}	

	if(p_udp_hdr->reservedSrc2 == 0 && p_udp_hdr->Platform <= 26){ // SRR3 Implementation
		if(!bOnce[sourceId]){
			fprintf(fptr,"PC_Time,versionInfo,Platform,CustID,sourceTxCnt,sourceTxTime,streamNumber,streamVersion,streamRefIndex,streamDataLen,streamTxCnt,streamChunkIdx,streamChunks,packetnumber,utc_time,aptiv_hdr_timestamp,mf4_frame_timestamp,sensorid\n");
			bOnce[sourceId] = 1;
		}
		fprintf(fptr,"%u,%x,%u,%s,%u,%u,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%lld,%u\n", 
			pcTime,
			p_udp_hdr->versionInfo,
			p_udp_hdr->Platform,
			GetCustIDName(p_udp_hdr->customerID, p_udp_hdr->Platform),
			p_udp_hdr->sourceTxCnt,
			p_udp_hdr->sourceTxTime,
			GetSRR5streamNumber(p_udp_hdr->streamNumber),
			p_udp_hdr->streamVersion,
			p_udp_hdr->streamRefIndex,
			p_udp_hdr->streamDataLen,
			p_udp_hdr->streamTxCnt,
			p_udp_hdr->streamChunkIdx,
			p_udp_hdr->streamChunks,
			p_cust_hdr->packetnumber,
		    p_cust_hdr->utc_time,
		    p_cust_hdr->timestamp,
			obsoluteTimestamp_t,
			p_cust_hdr->sensorid
			);
	}
	else{ // SRR5 Implementation
		if(!bOnce[sourceId]){
			fprintf(fptr,"PC_Time,versionInfo,Radar_Pos,reservedSrc2,reservedSrc3,Platform,CustID,sourceTxCnt,sourceTxTime,streamNumber,streamVersion,streamRefIndex,streamDataLen,streamTxCnt,streamChunkIdx,streamChunks,pkct_no,utc_time,aptiv_hdr_timestamp,mf4_frame_timestamp,sensorid,CalRunningCnt,CalTotalChunk,Cal_Source,detectioncnt,sensorstatus,MajorRevision,MinorRevision\n");
			bOnce[sourceId]= 1;
		}
		fprintf(fptr,"%lld,%x,%u,%u,%u,%u,%s,%u,%u,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%lld,%u,%u,%u,%s,%u,%u,%u,%u\n", 
			pcTime,
			p_udp_hdr->versionInfo,
			p_udp_hdr->Radar_Position,
			p_udp_hdr->reservedSrc2,
			p_udp_hdr->reservedSrc3,
			p_udp_hdr->Platform,
			GetCustIDName(p_udp_hdr->customerID, p_udp_hdr->Platform),
			p_udp_hdr->sourceTxCnt,
			p_udp_hdr->sourceTxTime,
			GetSRR5streamNumber(p_udp_hdr->streamNumber),
			p_udp_hdr->streamVersion,
			p_udp_hdr->streamRefIndex,
			p_udp_hdr->streamDataLen,
			p_udp_hdr->streamTxCnt,
			p_udp_hdr->streamChunkIdx,
			p_udp_hdr->streamChunks,
			p_cust_hdr->packetnumber,
			p_cust_hdr->utc_time,
			p_cust_hdr->timestamp,
			obsoluteTimestamp_t,
			p_cust_hdr->sensorid,
			p_cust_hdr->Cal_Running_Cnt,
			p_cust_hdr->Total_Cal_Chunk_Cnt,
			GetcalsourceName(p_cust_hdr->CalSource, p_udp_hdr->streamNumber,0), //p_cust_hdr->CalSource,
			p_cust_hdr->detectioncnt,
			p_cust_hdr->sensorstatus,
			p_cust_hdr->UDP_Version_info.MajorRevision,
			p_cust_hdr->UDP_Version_info.MinorRevision
			);
	}
}
