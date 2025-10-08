/****************************************************************************
*
*   Aptiv TCI Active Safety
*
*   Copyright (C) 2021 by Aptiv TCI. All Rights Reserved.
*   This file contains Aptiv TCI Active Safety Proprietary information.
*   It may not be reproduced or distributed without permission.
*
***************************************************************************/

/******************************************************************************
FileName   : ccaVigemDump.cpp

Author     : Kiran K R

Date       : 21 Jun, 2021

Description: struct and functions for extracting the mdf4/dvsu in csv formats


Version No : Date of update:         Updated by:           Changes:

Issues     : None

******************************************************************************/
#include "ccaVigemDump.h"

using namespace std;

ofstream out_fETH;
ofstream out_fCAN;
std::string fName_ETH;
std::string fName_CAN;
std::string Eth_Header = "RecordNo,Abs_Ts (ns), Rel_Ts (ns),BusSpecID,EtherType,vLAN_id,VersionInfo,Radar_Position,customerID,Platform,sourceTxCnt,sourceTxTime,reservedSrc2,reservedSrc3,streamTxCnt,streamNumber,streamVersion,streamRefIndex,streamDataLen,streamChunkIdx,streamMaxChunks,custHdr_UTC,custHdr_Timestamp,custHdr_PktNo,custHdr_SensorID,custHdr_SensorStatus,custHdr_DetCount,custHdr_CalRunningCount,custHdr_MaxCalChunk,custHdr_CalSource";
std::string Can_Header = "RecordNo,Abs_Ts (ns), Rel_Ts (ns),BusSpecID,CAN_ID,CAN_Type,DataLength";
uint64_t  abs_timestamp_ns = 0;
int64_t  rel_timestamp_ns = 0;

static uint32_t ethRecordCounter = 0;
static uint32_t canRecordCounter = 0;
static uint32_t busSpecId = 0;
uint16_t etherType = 0x0000;
uint16_t vLAN_Id = 0x0000;
uint8_t canChNo = 0;
uint8_t canDataLen = 0;
uint8_t is_CANfd = 0;
uint8_t is_extended = 0;
uint8_t canDataBytes[MAX_CAN_BYTES] = { 0 };
uint32_t canID = MAX_CAN_EXT_ID;
static char strEthMsg[2500] = { 0 };
static char strCANMsg[2500] = { 0 };
UDPRecord_Header aptivHdr = { 0 };
udp_custom_frame_header_t custHdrObj = { 0 };

static char* GetCustIDName(uint8_t custID, uint8_t platform)
{
	char customerName[40] = { 0 };
	static char retCustomerName[40] = { 0 };

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
	case SCANIA_MAN: sprintf(customerName, "MAN_SSR3"); break;
	case SGM_358_SRR5: sprintf(customerName, "SGM_358_SRR5"); break;
	case TML_SRR5: sprintf(customerName, "TML_SRR5"); break;
	default: sprintf(customerName, "INVALID_CUSTOMER"); break;
	}

	sprintf(retCustomerName, "%d_%s", custID, customerName);
	return retCustomerName;
}
static char* GetStreamNumberName(uint8_t Stream_Num)
{
	char	Str_Name[40] = { 0 };
	static char	retStr_Name[40] = { 0 };
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
	case CALIBRATION_DATA: sprintf(Str_Name, "CALIBRATION_DATA"); break;
	case DSPACE_CUSTOMER_DATA: sprintf(Str_Name, "DSPACE_CUSTOMER_DATA"); break;
	default: sprintf(Str_Name, "INVALID_STREAM", Stream_Num); break;
	}
	sprintf(retStr_Name, "%u_%s", Stream_Num, Str_Name);
	return retStr_Name;
}
void ClenUpBuffers()
{
	memset(strCANMsg, 0x00, sizeof(strCANMsg));
	memset(strEthMsg, 0x00, sizeof(strEthMsg));
	memset(canDataBytes, 0x0, sizeof(canDataBytes));
	memset(&aptivHdr, 0x0, sizeof(aptivHdr));
	memset(&custHdrObj, 0x0, sizeof(custHdrObj));
	ethRecordCounter = 0;
	canRecordCounter = 0;
}
int csvFileCreate(std::string sFileName)
{
	std::string key(".MF4");
	std::size_t found = sFileName.rfind(key);
	std::size_t found2 = 0;
	std::cout << "Procesing file:" << sFileName << std::endl;
	if (found == std::string::npos) // checking the file extension ".MF4" or ".mf4"
	{
		key = ".mf4";
		found2 = sFileName.rfind(key);
		if (found2 == std::string::npos)
		{
			return -1; // this is the case where the input file is not mf4.
		}
		found = found2;
	}

	fName_ETH = sFileName;
	fName_ETH.replace(found, 15, "_ETHrecords.csv");
	out_fETH.open(fName_ETH);
	if (out_fETH.is_open())
	{
		out_fETH << Eth_Header << endl;
	}
	else
	{
		std::cout << "Error: can not create ETH dump file: " << std::endl;
	}

	fName_CAN = sFileName;
	fName_CAN.replace(found, 15, "_CANrecords.csv");
	out_fCAN.open(fName_CAN);
	char canHdr[1500] = { 0 };
	if (out_fCAN.is_open())
	{
		strcpy(canHdr, Can_Header.c_str());
		for (int i = 0; i < MAX_CAN_BYTES; i++)
		{
			sprintf(canHdr, "%s,byte_%d", canHdr, i);
		}
		Can_Header = canHdr;
		out_fCAN << Can_Header << endl;
	}
	else
	{
		std::cout << "Error: can not create CAN dump file: " << std::endl;
	}
	ClenUpBuffers();
	return 0;
}
int csvFileClose()
{
	out_fETH.close();
	out_fCAN.close();
	ClenUpBuffers();
	return 0;
}

int printCANdataBytes()
{
	if (out_fCAN.is_open())
	{
		string canType = "CAN";
		if (is_CANfd) canType = "CAN_FD";
		memset(strCANMsg, 0x00, sizeof(strCANMsg));
		{
			sprintf(strCANMsg, "%lu,%llu,%lli,%lu,0x%08X,%s,%d",
				canRecordCounter, abs_timestamp_ns, rel_timestamp_ns,
				busSpecId, canID, canType.c_str(), canDataLen);

			for (int i = 0; i < canDataLen; i++)
			{
				sprintf(strCANMsg, "%s,0x%02X", strCANMsg, canDataBytes[i]);
			}

			out_fCAN << strCANMsg << endl;
		}
	}
	memset(canDataBytes, 0x0, sizeof(canDataBytes)); // Clearing the CAN buffer.
	return 0;
}
int printAptivUDPheader()
{
	if (out_fETH.is_open())
	{
		memset(strEthMsg, 0x00, sizeof(strEthMsg));
		sprintf(strEthMsg, "%lu,%llu,%lli,%lu,0x%04X,0x%04X,0x%04X,%d,%s,%d,%u,%lu,%u,%u,%u,%s,%u,%lu,%u,%u,%u,%lu,%lu,%u,%u,%u,%u,%u,%u,%u",
			ethRecordCounter, abs_timestamp_ns, rel_timestamp_ns,
			busSpecId, etherType, vLAN_Id, aptivHdr.versionInfo,
			aptivHdr.Radar_Position, GetCustIDName(aptivHdr.customerID, aptivHdr.Platform), aptivHdr.Platform,
			aptivHdr.sourceTxCnt, aptivHdr.sourceTxTime, aptivHdr.reservedSrc2, aptivHdr.reservedSrc3,
			aptivHdr.streamTxCnt, GetStreamNumberName(aptivHdr.streamNumber), aptivHdr.streamVersion,
			aptivHdr.streamRefIndex, aptivHdr.streamDataLen, aptivHdr.streamChunkIdx, aptivHdr.streamChunks,
			custHdrObj.utc_time, custHdrObj.timestamp, custHdrObj.packetnumber, 
			custHdrObj.sensorid, custHdrObj.sensorstatus, custHdrObj.detectioncnt, 
			custHdrObj.Cal_Running_Cnt, custHdrObj.Total_Cal_Chunk_Cnt, custHdrObj.CalSource);
		out_fETH << strEthMsg << endl;
	}
	return 0;
}
int handleCCAEthMessage(uint32_t index, CcaMessage *msg, CcaTime timestamp, uint32_t busid)
{
	memset(&aptivHdr, 0x0, sizeof(aptivHdr));
	memset(&custHdrObj, 0x0, sizeof(custHdrObj));
	// convert general message object to ethernet message object
//	CcaEthernetMessage ethMsg = CcaMessage_createEthernetMessage(*msg);
	if (msg == CCA_INVALID_HANDLE)
	{
		std::cout << "CcaMessage_createEthernetMessage failed: " << CcaLib_getLastError() << std::endl;
		return 0;
	}

	// get payload from message
	CcaMessagePayload payload;
	if (!CcaEthernetMessage_payload(msg, &payload))
	{
		cout << "Error: cannot get Ethernet message payload" << endl;
		return 0;
	}
	uint16_t versionInfoEndianCheck = *(uint16_t*)(payload.Elements + 28);
	int8_t is_bigEndian = -1; // 1-> BigEndian,  0-> LittleEndian, -1 is invalid.
	if ((versionInfoEndianCheck == 0xA318) || (versionInfoEndianCheck == 0xA218))
	{
		is_bigEndian = 0; // this is the case of little-endian (e.g. RECU UDP data)
		aptivHdr.versionInfo = versionInfoEndianCheck;
	}
	else if ((versionInfoEndianCheck == 0x18A3) || (versionInfoEndianCheck == 0x18A2))
	{
		is_bigEndian = 1; // this is the case of big-endian (e.g. Sensor UDP data)
		aptivHdr.versionInfo = B2L_s(versionInfoEndianCheck);
	}
	// get ether type from message
	if (!CcaEthernetMessage_etherType(msg, &etherType))
	{
		cout << "Error: cannot get Ether type" << endl;
		return 0;
	}
	// get vLan ID from message
	if (!CcaEthernetMessage_vlanTagId(msg, &vLAN_Id))
	{
		cout << "Error: cannot get vLAN ID" << endl;
		return 0;
	}

	// get MAC addresses from message
	uint8_t *sourceMAC;
	if (!CcaEthernetMessage_sourceAddress(msg, &sourceMAC))
	{
		cout << "Error: cannot get source MAC address from the Ethernet message" << endl;
		return 0;
	}

	uint8_t *destMAC;
	if (!CcaEthernetMessage_destinationAddress(msg, &destMAC))
	{
		cout << "Error: cannot get destination MAC address from the Ethernet message" << endl;
		return 0;
	}
	ethRecordCounter++;
	busSpecId = busid;
	abs_timestamp_ns = *(uint64_t*)(timestamp);
	rel_timestamp_ns = abs_timestamp_ns - start_timestamp_ns;
	if (is_bigEndian != -1) // This is the case where the EThernet data received is Aptiv UDP data.
	{
		aptivHdr.Platform = *(uint8_t*)(payload.Elements + 36);
		aptivHdr.Radar_Position = *(uint8_t*)(payload.Elements + 37);
		aptivHdr.reservedSrc2 = *(uint8_t*)(payload.Elements + 38);;
		aptivHdr.reservedSrc3 = *(uint8_t*)(payload.Elements + 39);;
		aptivHdr.streamTxCnt = *(uint8_t*)(payload.Elements + 46);
		aptivHdr.streamNumber = *(uint8_t*)(payload.Elements + 47);
		aptivHdr.streamVersion = *(uint8_t*)(payload.Elements + 48);
		aptivHdr.streamChunks = *(uint8_t*)(payload.Elements + 49);
		aptivHdr.streamChunkIdx = *(uint8_t*)(payload.Elements + 50);
		aptivHdr.customerID = *(uint8_t*)(payload.Elements + 51);

		aptivHdr.sourceTxCnt = *(uint16_t*)(payload.Elements + 30);
		aptivHdr.streamDataLen = *(uint16_t*)(payload.Elements + 44);

		aptivHdr.sourceTxTime = *(uint32_t*)(payload.Elements + 32);
		aptivHdr.streamRefIndex = *(uint32_t*)(payload.Elements + 40);

		custHdrObj.utc_time = *(uint32_t*)(payload.Elements + 56);
		custHdrObj.timestamp = *(uint32_t*)(payload.Elements + 60);
		custHdrObj.packetnumber = *(uint16_t*)(payload.Elements + 64);
		custHdrObj.sensorid = *(uint8_t*)(payload.Elements + 66);
		custHdrObj.sensorstatus = *(uint8_t*)(payload.Elements + 67);
		custHdrObj.detectioncnt = *(uint8_t*)(payload.Elements + 68);
		custHdrObj.Cal_Running_Cnt = *(uint8_t*)(payload.Elements + 69);
		custHdrObj.Total_Cal_Chunk_Cnt = *(uint8_t*)(payload.Elements + 70);
		custHdrObj.CalSource = *(uint8_t*)(payload.Elements + 71);

		if (is_bigEndian == 1)
		{
			aptivHdr.sourceTxCnt = B2L_s(*(uint16_t*)(payload.Elements + 30));
			aptivHdr.streamDataLen = B2L_s(*(uint16_t*)(payload.Elements + 44));

			aptivHdr.sourceTxTime = B2L_l(*(uint32_t*)(payload.Elements + 32));
			aptivHdr.streamRefIndex = B2L_l(*(uint32_t*)(payload.Elements + 40));

			custHdrObj.utc_time = B2L_l(*(uint32_t*)(payload.Elements + 56));
			custHdrObj.timestamp = B2L_l(*(uint32_t*)(payload.Elements + 60));
			custHdrObj.packetnumber = B2L_s(*(uint16_t*)(payload.Elements + 64));
		}
		printAptivUDPheader();
	}
	return 0;
}

int handleCCACanMessage(uint32_t index, CcaMessage *msg, CcaTime timestamp, uint32_t busid)
{
	// convert general message object to can message object
	//CcaCanMessage canMsg = CcaMessage_createCanMessage(*msg);
	if (msg == CCA_INVALID_HANDLE)
	{
		std::cout << "CcaMessage_createCanMessage failed: " << CcaLib_getLastError() << std::endl;
		return 0;
	}

	vgm_bool isNormalMessage = 0;
	CcaCanMessage_isNormalMessage(msg, &isNormalMessage);

	if (0 == isNormalMessage)
	{
		//	std::cout << "Error: Not a Normal CAN message: " << CcaLib_getLastError() << std::endl;
		return 0;
	}
	// get payload from message
	CcaMessagePayload payload;
	if (!CcaCanMessage_payload(msg, &payload))
	{
		cout << "Error: cannot get CAN message payload" << endl;
		return 0;
	}
	// get CAN ID
	uint32_t canIdentifier = 0;
	CcaCanMessage_id(msg, &canIdentifier);
	canID = (canIdentifier & MAX_CAN_EXT_ID);

	// get EDL bit: 1 for CAN FD and 0 for CAN.
	is_CANfd = 0;
	CcaCanMessage_edl(msg, &is_CANfd);

	// get Extended (1) or standard (0)CAN info: 
	is_extended = 0;
	CcaCanMessage_isExtended(msg, &is_extended);

	canRecordCounter++;
	busSpecId = busid;
	abs_timestamp_ns = *(uint64_t*)(timestamp);
	rel_timestamp_ns = abs_timestamp_ns - start_timestamp_ns;
	canDataLen = payload.Count;
	memset(canDataBytes, 0x0, sizeof(canDataBytes));
	memcpy(canDataBytes, payload.Elements, payload.Count);
	printCANdataBytes();
	return 0;
}


