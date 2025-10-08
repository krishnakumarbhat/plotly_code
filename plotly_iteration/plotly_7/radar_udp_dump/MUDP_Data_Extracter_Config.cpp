#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>


#include "../../../CommonFiles/inc/fixmac.h"
#include"../../../CommonFiles/helper/genutil.h"

#include "../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"

//#define _GNU_SOURCE


#include"../../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config_mode.h"
#include "HTML/InputParser/JsonInputParser.h"

//XML_DPH_RR_ADAS_CONFIG_T *xml_config = NULL;
float cfg_Version = 0.0;
// Additional includes ends here.
using namespace std;

extern  char  outputPathName[_MAX_PATH];

//////////////////// Define Base class for rr adas configurations /////////////////////
uint8_t cfg_Version_major = 0;
uint8_t cfg_Version_minor = 0;
rr_MUDP_baseConfig::rr_MUDP_baseConfig()
{

}
rr_MUDP_baseConfig::~rr_MUDP_baseConfig()
{

}
/////////////////////// End of class definition rrBaseConfig

char* GetRadarPosName(unsigned int radarPos) {
	static char RadarName[40] = { 0 };

	switch (radarPos) {
	case 0: sprintf(RadarName, "RL"); break;
	case 1: sprintf(RadarName, "RR"); break;
	case 2: sprintf(RadarName, "FR"); break;
	case 3: sprintf(RadarName, "FL"); break;
	case 5: sprintf(RadarName, "FC"); break;
	case 6: sprintf(RadarName, "BP_R"); break;
	case 7: sprintf(RadarName, "BP_L"); break;
	case 8: sprintf(RadarName, "FLR4P_FC"); break;
	case 19: sprintf(RadarName, "ECU"); break;
	default: sprintf(RadarName, "INVALID_RADAR"); break;

	}

	return RadarName;
}

bool ToBool(const std::string & s) {
	bool value = false;
	if (s == "True")
		value = true;
	else if (s == "False")
		value = false;
	return value;
}

bool Check_CFG_Version_Integrity(XML_MUDP_CONFIG_T * p_xml)
{
	bool b_ret = TRUE;

	if (p_xml->Data_Extracter_Mode[CSV_DUMP_EXTRACTION] == "" || p_xml->Data_Extracter_Mode[Quality_Check] == "" || p_xml->Data_Extracter_Mode[CSV] == "" || p_xml->Data_Extracter_Mode[XML] == "" || p_xml->Data_Extracter_Mode[PACKET_Loss] == "" || p_xml->Data_Extracter_Mode[CCA_MF4_DUMP] == "")
	{
		if (p_xml->Data_Extracter_Mode[CSV_DUMP_EXTRACTION] == "1" || p_xml->Data_Extracter_Mode[Quality_Check] == "1"  && (p_xml->Data_Extracter_Mode[CSV] == "" || p_xml->Data_Extracter_Mode[XML] == "" || p_xml->Data_Extracter_Mode[PACKET_Loss] == "" || p_xml->Data_Extracter_Mode[CCA_MF4_DUMP] == ""))
		{
			if (cfg_Version_major == DPH_RR_ADAS_CONFIG_MAJOR_VERSION && cfg_Version_minor == DPH_RR_ADAS_CONFIG_MINOR_VERSION)
			{
				b_ret &= TRUE;
			}
			else
				b_ret &= FALSE;
		}
		else
			b_ret &= FALSE;
	}
	else
	{
		if (cfg_Version_major == DPH_RR_ADAS_CONFIG_MAJOR_VERSION && cfg_Version_minor == DPH_RR_ADAS_CONFIG_MINOR_VERSION)
		{
			if (p_xml->MUDP_Select_Sensor_Status[MUDP_FLR4P_FC_09] == "")
			{
				b_ret &= FALSE;
			}
			return b_ret;
		}
		else
		{
			b_ret &= FALSE;
		}
	}

	if(p_xml->MUDP_Radar_ECU == "0")
	{
		for (int i=1; i<MAX_ECU_COUNT; i++)
		{
			if(p_xml ->RADAR_ECU_Stream_Options[i] == "1")
			{
				printf("Please Enable the Radar_Ecu status inoder to dump Radar_Ecu data \n");
				printf("\rExiting the resimulation.\n\n\r");
				exit(0);
			}
		}
	}
	
	//if(p_xml->MUDP_Radar_ECU == "1")
	//{
	//	for (int i=2; i<MAX_STREAM; i++)
	//	{
	//		if(p_xml ->Radar_Stream_Optionss[i] == "1")
	//		{
	//			printf("Please Disable all  Radar_Stream_Optionss since  Radar_Ecu status is enabled\n");
	//			printf("\rExiting the resimulation.\n\n\r");
	//			exit(0);
	//		}
	//	}
	//}
	if ( p_xml->RADAR_ECU_Stream_Options[ecu0] == "1"  ||  p_xml->RADAR_ECU_Stream_Options[ecu1] == "1" ||  p_xml->RADAR_ECU_Stream_Options[ecu3] == "1" || p_xml->RADAR_ECU_Stream_Options[ecu_VRU_Classifier] == "1")
	{
		if(p_xml ->Radar_Stream_Options[HDR] == "1")
		{
			printf("Please enable either RADAR_ECU_Stream_Option option or enable HDR option \n");
			printf("\rExiting the resimulation.\n\n\r");
			exit(0);
		}
	}
	return b_ret;
}



string collect_string_master_val(string line, int t_signal_val)
{
	int getpos_s;
	string s_val;
	t_signal_val--;

	getpos_s = line.find(">", t_signal_val);
	s_val = line.substr(getpos_s+1,(line.find_first_of('<', getpos_s) - getpos_s - 1)).c_str();

	return s_val;
}


string getMasterConfiguration(string chk_var, string buff)
{
	string line;
	chk_var.insert(0, "<");
	chk_var.back();
	chk_var.append(">");

	stringstream ss(buff);
	int t_yes_val;
	string s_val;

	getline(ss, line);
	while(ss.eofbit)
	{ 
		if(ss.eof()){
			break;
		}
		t_yes_val = line.find(chk_var);
		if(t_yes_val >= 0)
		{
			s_val=collect_string_master_val(line, 4);
			break;
		}
		getline(ss, line);
	} 
	return s_val;
}
MUDP_status_e MUDP_read_master_config(MUDP_CONFIG_T *pCnfg, const char* config_file_name)
{
	bool b_Cfg = FALSE;
	/*bool SRR_FLag = FALSE;*/
	if (file_exist(config_file_name, 0) == -1){
		return MUDP_NOK;
	}
	if (config_file_name && strlen(config_file_name) > 0)
	{

		XML_MUDP_CONFIG_T *xml_config = new XML_MUDP_CONFIG_T;

		MUDP_status_e status = MUDP_read_master_config_XML(xml_config, config_file_name);

		b_Cfg = Check_CFG_Version_Integrity(xml_config);

		if(b_Cfg == FALSE)
		{
			printf("Please use the configuration xml file version %s and try again...!!\n", DPH_RR_ADAS_CONFIG_VERSION);
			return MUDP_NOK;			
		}
		/*Read Main SIL Config XML*/

		if(status == MUDP_OK)
		{
			memset(pCnfg, 0, sizeof(MUDP_CONFIG_T));
			if (strcmp(xml_config->gentype.c_str(), "GEN7") == 0)
				strcpy(pCnfg->GenType, "GEN7");
			else if (strcmp(xml_config->gentype.c_str(), "GEN5") == 0)
				strcpy(pCnfg->GenType, "GEN5");
			else 
				strcpy(pCnfg->GenType, "GEN6");
			printf("Selected_Generation: %s \n", pCnfg->GenType);

			if (strcmp(xml_config->Radar_Stream_Options[HDR].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[HDR] = HDR;
			if (strcmp(xml_config->Radar_Stream_Options[CCA_HDR].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[CCA_HDR] = CCA_HDR;
			if (strcmp(xml_config->Radar_Stream_Options[CDC_iq].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[CDC_iq] = CDC_iq;
			if (strcmp(xml_config->Radar_Stream_Options[Z4_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Z4_Core] = Z4_Core;
			if (strcmp(xml_config->Radar_Stream_Options[Z7A_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Z7A_Core] = Z7A_Core;
			if (strcmp(xml_config->Radar_Stream_Options[Z7B_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Z7B_Core] = Z7B_Core;
			if (strcmp(xml_config->Radar_Stream_Options[Z4_Customer].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Z4_Customer] = Z4_Customer;
			if (strcmp(xml_config->Radar_Stream_Options[Z7B_Customer].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Z7B_Customer] = Z7B_Customer;
			if (strcmp(xml_config->Radar_Stream_Options[OSI_Stream].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[OSI_Stream] = OSI_Stream;
			if (strcmp(xml_config->Radar_Stream_Options[Dspace].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[Dspace] = Dspace;
			//gen5 
			if (strcmp(xml_config->Radar_Stream_Options[C0_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[C0_Core] = C0_Core;
			if (strcmp(xml_config->Radar_Stream_Options[C0_Cust].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[C0_Cust] = C0_Cust;
			if (strcmp(xml_config->Radar_Stream_Options[C1_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[C1_Core] = C1_Core;
			/*else if (strcmp(xml_config->Radar_Stream_Options[C1_Core].c_str(), "") == 0)
				pCnfg->Radar_Stream_Options[C1_Core] = C1_Core;*/
			if (strcmp(xml_config->Radar_Stream_Options[C2_Core].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[C2_Core] = C2_Core;
			else if (strcmp(xml_config->Radar_Stream_Options[C2_Core].c_str(), "") == 0) //CSV_DUMP_EXTRACTION
				pCnfg->Radar_Stream_Options[C2_Core] = C2_Core;
			if (strcmp(xml_config->Radar_Stream_Options[C2_Cust].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[C2_Cust] = C2_Cust;
			//FLR4P
			if (strcmp(xml_config->Radar_Stream_Options[BC_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[BC_CORE] = BC_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[VSE_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[VSE_CORE] = VSE_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[STATUS_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[STATUS_CORE] = STATUS_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[RDD_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[RDD_CORE] = RDD_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[DET_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[DET_CORE] = DET_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[HDR_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[HDR_CORE] = HDR_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[TRACKER_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[TRACKER_CORE] = TRACKER_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[DEBUG_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[DEBUG_CORE] = DEBUG_CORE;
			//GEN7
			if (strcmp(xml_config->Radar_Stream_Options[ALIGNMENT_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ALIGNMENT_CORE] = ALIGNMENT_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[MMIC_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[MMIC_CORE] = MMIC_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[BLOCKAGE_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[BLOCKAGE_CORE] = BLOCKAGE_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[RADAR_CAPABILITY_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[RADAR_CAPABILITY_CORE] = RADAR_CAPABILITY_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[DOWN_SELECTION_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[DOWN_SELECTION_CORE] = DOWN_SELECTION_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ID_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ID_CORE] = ID_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[TOI_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[TOI_CORE] = TOI_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[DYNAMIC_ALIGNMENT_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[DYNAMIC_ALIGNMENT_CORE] = DYNAMIC_ALIGNMENT_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_SAFETY_FAULTS_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_SAFETY_FAULTS_CORE] = ROT_SAFETY_FAULTS_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_TRACKER_INFO_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_TRACKER_INFO_CORE] = ROT_TRACKER_INFO_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_VEHICLE_INFO_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_VEHICLE_INFO_CORE] = ROT_VEHICLE_INFO_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE] = ROT_OBJECT_STREAM_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_ISO_OBJECT_STREAM_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_ISO_OBJECT_STREAM_CORE] = ROT_ISO_OBJECT_STREAM_CORE;
			if (strcmp(xml_config->Radar_Stream_Options[ROT_PROCESSED_DETECTION_STREAM_CORE].c_str(), "1") == 0)
				pCnfg->Radar_Stream_Options[ROT_PROCESSED_DETECTION_STREAM_CORE] = ROT_PROCESSED_DETECTION_STREAM_CORE;
			//--------
			if (strcmp(xml_config->RADAR_ECU_Stream_Options[ecu0].c_str(), "1") == 0)
				pCnfg->RADAR_ECU_Stream_Options[ecu0] = ecu0;
			if (strcmp(xml_config->RADAR_ECU_Stream_Options[ecu1].c_str(), "1") == 0)
				pCnfg->RADAR_ECU_Stream_Options[ecu1] = ecu1;
			if (strcmp(xml_config->RADAR_ECU_Stream_Options[ecu3].c_str(), "1") == 0)
				pCnfg->RADAR_ECU_Stream_Options[ecu3] = ecu3;
			if (strcmp(xml_config->RADAR_ECU_Stream_Options[ecu_VRU_Classifier].c_str(), "1") == 0)
				pCnfg->RADAR_ECU_Stream_Options[ecu_VRU_Classifier] = ecu_VRU_Classifier;

			if(strcmp(xml_config->MUDP_Radar_ECU.c_str(),"1")==0){
				pCnfg->MUDP_Radar_ECU_status = 1;

			}
			else
				pCnfg->MUDP_Radar_ECU_status = 0;


			for(int i= 0; i < MUDP_MAX_STATUS_COUNT; i++)
			{
				if((strcmp(xml_config->MUDP_Select_Sensor_Status[i].c_str(),"1") == 0))
				
				{
					pCnfg->MUDP_Select_Sensor_Status[i] = 1;
					pCnfg->corner_Sensor_Status = 1;
					
				}
				else
				{
					pCnfg->MUDP_Select_Sensor_Status[i] = 0;
					
				}

			}
			
			for(int j= 0; j < MAX_XML_DATA; j++)
			{
				if((strcmp(xml_config->Output_Xml_Trace_Option[j].c_str(),"1") == 0))
				{
					pCnfg->Output_Xml_Trace_Option[j] = 1;
				}
				else
				{
					pCnfg->Output_Xml_Trace_Option[j] = 0;
				}
			}
			

			//for logger
			strcpy(pCnfg->logTracingPath, xml_config->logTracingPath.c_str());

			if(	pCnfg->MUDP_Select_Sensor_Status[MUDP_BP_R] == 1 || pCnfg->MUDP_Select_Sensor_Status[MUDP_BP_L] == 1)
			{
				pCnfg->Bpillar_status = 1;

			}
			else 
			{
				pCnfg->Bpillar_status = 0;

			}
			if (strcmp(xml_config->Data_Extracter_Mode[CSV].c_str(), "1") == 0)
			{
				pCnfg->data_extracter_mode[CSV] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[CSV] = 0;
			}
			if (strcmp(xml_config->Data_Extracter_Mode[CSV_DUMP_EXTRACTION].c_str(), "1") == 0)  // CSV_DUMP_EXTRACTION
			{
				pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] = 0;
			}
			
			if (strcmp(xml_config->Data_Extracter_Mode[XML].c_str(), "1") == 0)
			{
				pCnfg->data_extracter_mode[XML] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[XML] = 0;
			}
			if (strcmp(xml_config->Data_Extracter_Mode[CCA_MF4_DUMP].c_str(), "1") == 0)
			{
				pCnfg->data_extracter_mode[CCA_MF4_DUMP] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[CCA_MF4_DUMP] = 0;
			}
			if (strcmp(xml_config->Data_Extracter_Mode[PACKET_Loss].c_str(), "1") == 0)
			{
				pCnfg->data_extracter_mode[PACKET_Loss] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[PACKET_Loss] = 0;
			}
			if (strcmp(xml_config->Data_Extracter_Mode[Quality_Check].c_str(), "1") == 0)
			{
				pCnfg->data_extracter_mode[Quality_Check] = 1;
			}
			else
			{
				pCnfg->data_extracter_mode[Quality_Check] = 0;
			}
			if (strcmp(xml_config->Packet_Loss_Statistics.c_str(), "1") == 0)
			{
				pCnfg->packet_loss_statistics = 1;
			}
			else
			{
				pCnfg->packet_loss_statistics = 0;
			}

			//for HTML
			//CodeChange#7 : checking html report enabled

			if (strcmp(xml_config->HTML_Report.c_str(), "1") == 0) //CodeChange#3: check HTML report is enabled by user 
			{
				pCnfg->bhtml_report = 1;
				std::cout << std::endl << " HTML Report Generate " << pCnfg->bhtml_report<<std::endl; //Print#2

				/*
				* -----------------------------------------------------------------------------
				*      HTML in MUDP RQ#1 : Preconditional check for HTML Repot Generation
				*      JIRA  Ticket      : CXZ-2520
				*      Comment Owner     : Bharanidharan S (gjlkfw)
				*      Date              : July 23 2024
				* -----------------------------------------------------------------
				*      RQ#1.1 :If HTML Report is enabled then check xmldump and csvdump
				*              proceed with HTML report generation if any and only if xmldump and csvdump are disabled
				*
				*      RQ#1.2 :If either xmldump and csvdump are enabled provide console print " disable xmldump and
				*               csvdump for HTML report Generation" and exit the application
				*
				*
				*      RQ#1.3 :if xmldump and csvdump are diabled and HTML report is enabled then in business logic
				*              enable xmldump .
				* --------------------------------------------------------------------------
				* Reason for above changes:
				*
				*         MUDP uses decoder for xml and csv dump. Decoder is single threaded library, hence we
				*         are enabling either xmldump or csv dump. Enabling both xml and csv dump will provide
				*         undefined behaviour. Hence while generating HTML report as pre condition check we are
				*         checking both xml and csv dump enableres and proceeding html generation only if both are
				*         disabled.
				*         HTML data collection logic uses xml dump logic hence in business logic we are forcing fully
				*         enabling the xml dump enablers to true
				* return MUDP_NOK;
				*--------------------------------------------------------------------------------
				*/

				if ((strcmp(xml_config->Data_Extracter_Mode[XML].c_str(), "1") == 0) && (strcmp(xml_config->Data_Extracter_Mode[CSV_DUMP_EXTRACTION].c_str(), "1") == 0))
				{
					std::cout << std::endl << "Disable xml and csv dump for HTML Generation" << std::endl;
					return MUDP_NOK;
				}

				else if (strcmp(xml_config->Data_Extracter_Mode[XML].c_str(), "1") == 0)
				{
					std::cout << std::endl << "Disable xml dump for HTML Generation" << std::endl;
					return MUDP_NOK;
				}
				else if (strcmp(xml_config->Data_Extracter_Mode[CSV].c_str(), "1") == 0)
				{
					std::cout << std::endl << "Disable csv dump for HTML Generation" << std::endl;
					return MUDP_NOK;
				}

				if ((strcmp(xml_config->Data_Extracter_Mode[XML].c_str(), "1") != 0) && (strcmp(xml_config->Data_Extracter_Mode[CSV_DUMP_EXTRACTION].c_str(), "1") != 0))
				{
					//std::cout << std::endl << "Disabled xml & csv dump for HTML Generation" << std::endl;
					pCnfg->data_extracter_mode[XML] = 1;
					xml_config->Data_Extracter_Mode[XML] = "1";
				}

			}
			else
			{
				pCnfg->bhtml_report = 0;
				std::cout << std::endl << " HTML Report Generate " << pCnfg->bhtml_report; //Print#3

			}

			if (strcmp(xml_config->Data_Extracter_Mode[CSV_DUMP_EXTRACTION].c_str(), "1") == 0 )
			{
				pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] = 1;
				pCnfg->MUDP_input_path = CONTINUOUS_Mode;
				strcpy(pCnfg->MUDP_input_type, "Continuous_FILE_Input");
				pCnfg->Packetloss_report_format = XML_format;
				pCnfg->vector_Lib = 1;
				for (int i = 0; i < MAX_RADAR_COUNT ;i++)
				{
					if (i < 4)
						pCnfg->MUDP_Select_Sensor_Status[i] = 1;
					else
						pCnfg->MUDP_Select_Sensor_Status[i] = 0;
				}
			
				pCnfg->Bpillar_status = 0;
				pCnfg->MUDP_Radar_ECU_status = 0;

			}
			//CodeChange#5: checking html report enabled
			else if (pCnfg->data_extracter_mode[PACKET_Loss] == 1 || pCnfg->packet_loss_statistics == 1 || pCnfg->data_extracter_mode[Quality_Check] == 1
				|| pCnfg->data_extracter_mode[CSV] == 1 || pCnfg->data_extracter_mode[XML] == 1 || pCnfg->bhtml_report == 1
				|| pCnfg->data_extracter_mode[CCA_MF4_DUMP] == 1 || pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1) // CSV_DUMP_EXTRACTION
			{
				if (strcmp(xml_config->Input_File_mode.c_str(), "Continuous_FILE_Input") == 0) {
					pCnfg->MUDP_input_path = CONTINUOUS_Mode;
					strcpy(pCnfg->MUDP_input_type, xml_config->Input_File_mode.c_str());
				}
				else
				{
					pCnfg->MUDP_input_path = SEQUENTIAL_Mode;
				}
				if (strcmp(xml_config->Report_File_Format.c_str(), "text_fomat") == 0) {
					pCnfg->Packetloss_report_format = Text_format;
					strcpy(pCnfg->Report_type, xml_config->Report_File_Format.c_str());
				}
				else
				{
					pCnfg->Packetloss_report_format = XML_format;
				}
				if (strcmp(xml_config->Vector_Lib.c_str(), "1") == 0)
				{
					pCnfg->vector_Lib = 1;
				}
				else
				{
					pCnfg->vector_Lib = 0;
				}
			}
			else
			{
				pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] = 0;
				printf("[WARNING: Enable data_extracter_mode in config .............. \n");
				return MUDP_NOK;
			}

			if (strcmp(xml_config->Data_Extracter_Mode[Quality_Check].c_str(), "1") == 0 && (pCnfg->data_extracter_mode[PACKET_Loss] == 0 && pCnfg->packet_loss_statistics == 0) )
			{
				pCnfg->data_extracter_mode[Quality_Check] = 1;
				pCnfg->MUDP_input_path = CONTINUOUS_Mode;
				strcpy(pCnfg->MUDP_input_type, "Continuous_FILE_Input");
				pCnfg->Packetloss_report_format = XML_format;
				pCnfg->vector_Lib = 1;
				
			}
			//CodeChange#6 : checking html report enabled
			else if (pCnfg->data_extracter_mode[PACKET_Loss] == 1 || pCnfg->packet_loss_statistics == 1 || pCnfg->data_extracter_mode[Quality_Check] == 1
				|| pCnfg->data_extracter_mode[CSV] == 1 || pCnfg->data_extracter_mode[XML] == 1 || pCnfg->bhtml_report ==1 
				|| pCnfg->data_extracter_mode[CCA_MF4_DUMP] == 1 || pCnfg->data_extracter_mode[CSV_DUMP_EXTRACTION] == 1) // CSV_DUMP_EXTRACTION
			{
				if (strcmp(xml_config->Input_File_mode.c_str(), "Continuous_FILE_Input") == 0) {
					pCnfg->MUDP_input_path = CONTINUOUS_Mode;
					strcpy(pCnfg->MUDP_input_type, xml_config->Input_File_mode.c_str());
				}
				else
				{
					pCnfg->MUDP_input_path = SEQUENTIAL_Mode;
				}
				if (strcmp(xml_config->Report_File_Format.c_str(), "text_fomat") == 0) {
					pCnfg->Packetloss_report_format = Text_format;
					strcpy(pCnfg->Report_type, xml_config->Report_File_Format.c_str());
				}
				else
				{
					pCnfg->Packetloss_report_format = XML_format;
				}
				if (strcmp(xml_config->Vector_Lib.c_str(), "1") == 0)
				{
					pCnfg->vector_Lib = 1;
				}
				else
				{
					pCnfg->vector_Lib = 0;
				}
			}


			else
			{
				pCnfg->data_extracter_mode[Quality_Check] = 0;
				printf("[WARNING: Enable data_extracter_mode in config .............. \n");
			    return MUDP_NOK;
			}
			xml_config->binary_path_location = "default";
			

			if(strcmp(xml_config->MUDP_output_path_options.c_str(),"NEW_OUTPUT_PATH")==0){
				pCnfg->MUDP_output_path = NEW_OUTPUT_PATH_FILE;
				strcpy(pCnfg->MUDP_output_folder_path, xml_config->MUDP_output_path_location.c_str());
			}
			else if(strcmp(xml_config->MUDP_output_path_options.c_str(), "SAME_AS_OUTPUT") == 0)
			{				
				pCnfg->MUDP_output_path = SAME_AS_OUTPUT_PATH;

				std::string path(outputPathName);
				std::replace(path.begin(), path.end(), '\\', '/');

				strcpy(pCnfg->MUDP_output_folder_path, path.c_str());
				
				
			}
			else
			{
				pCnfg->MUDP_output_path = SAME_AS_INPUT_PATH;
			}

			if(strcmp(xml_config->Cca_Lib.c_str(),"1")== 0)
			{
				 pCnfg->cca_Lib = 1;
			}
			 else
			 {
				 pCnfg->cca_Lib = 0;
			 }

			return MUDP_OK;

		}	
		else
		{
			//dph_rr_adas_set_default_config(pCnfg);
			return status;
		}
	}
	else
	{

		return MUDP_NOK;
	}
}
MUDP_status_e MUDP_read_master_config_XML(XML_MUDP_CONFIG_T *pConfig, const char* config_file_name)
{
	if (config_file_name && strlen(config_file_name) > 0)
	{
		/*Read Main SIL Config XML*/

		int len = 0;
		char* pBuffer = NULL;
		ifstream fin;

		fin.open(config_file_name);
		if(!fin)
		{
			fin.close();
			return MUDP_NOK;
		}
		//Get total number of characters in the whole xml file.
		fin.seekg(0, fin.end);
		len = fin.tellg();
		fin.seekg(0, fin.beg);

		pBuffer = new char[len];
		memset(pBuffer, 0x0, len);
		fin.read(pBuffer, len);
		fin.close();

		pConfig->XML_version = getMasterConfiguration("DATA_Extracter_Config_XML_Version", pBuffer);
		pConfig->gentype = getMasterConfiguration("GENERATION_SELECTION", pBuffer);
		cfg_Version_major = stoi(pConfig->XML_version);
		size_t pos = pConfig->XML_version.find(".");
		if (pConfig->XML_version.find(".") != string::npos)
			cfg_Version_minor = stoi(pConfig->XML_version.substr(pos + 1));

		//cfg_Version = (float)atof(pConfig->XML_version.c_str());
		if (false == pConfig->XML_version.empty()){
			
			pConfig->Radar_Stream_Options[HDR] = getMasterConfiguration("HDR", pBuffer);
			pConfig->Radar_Stream_Options[CCA_HDR] = getMasterConfiguration("CCA_HDR", pBuffer);
			pConfig->Radar_Stream_Options[CDC_iq] = getMasterConfiguration("CDC", pBuffer);
			pConfig->Radar_Stream_Options[Z4_Core] = getMasterConfiguration("Z4_Core", pBuffer);
			pConfig->Radar_Stream_Options[Z7A_Core] = getMasterConfiguration("Z7A_Core", pBuffer);
			pConfig->Radar_Stream_Options[Z7B_Core] = getMasterConfiguration("Z7B_Core", pBuffer);
			pConfig->Radar_Stream_Options[Dspace] = getMasterConfiguration("DSPACE", pBuffer);
			pConfig->Radar_Stream_Options[Z4_Customer] = getMasterConfiguration("Z4_Customer", pBuffer);
			pConfig->Radar_Stream_Options[Z7B_Customer] = getMasterConfiguration("Z7B_Customer", pBuffer);
			pConfig->Radar_Stream_Options[OSI_Stream] = getMasterConfiguration("OSI_STREAM", pBuffer);
			pConfig->Radar_Stream_Options[C0_Core] = getMasterConfiguration("Z4_Core", pBuffer);
			pConfig->Radar_Stream_Options[C0_Cust] = getMasterConfiguration("Z4_Customer", pBuffer);
			pConfig->Radar_Stream_Options[C1_Core] = getMasterConfiguration("Z7A_Core", pBuffer);
			pConfig->Radar_Stream_Options[C2_Core] = getMasterConfiguration("Z7B_Core", pBuffer);
			pConfig->Radar_Stream_Options[C2_Cust] = getMasterConfiguration("Z7B_Customer", pBuffer);
			pConfig->Radar_Stream_Options[BC_CORE] = getMasterConfiguration("BC_CORE", pBuffer);
			pConfig->Radar_Stream_Options[VSE_CORE] = getMasterConfiguration("VSE_CORE", pBuffer);
			pConfig->Radar_Stream_Options[STATUS_CORE] = getMasterConfiguration("STATUS_CORE", pBuffer);
			pConfig->Radar_Stream_Options[RDD_CORE] = getMasterConfiguration("RDD_CORE", pBuffer);
			pConfig->Radar_Stream_Options[DET_CORE] = getMasterConfiguration("DET_CORE", pBuffer);
			pConfig->Radar_Stream_Options[HDR_CORE] = getMasterConfiguration("HDR_CORE", pBuffer);
			pConfig->Radar_Stream_Options[TRACKER_CORE] = getMasterConfiguration("TRACKER_CORE", pBuffer);
			pConfig->Radar_Stream_Options[DEBUG_CORE] = getMasterConfiguration("DEBUG_CORE", pBuffer);
			pConfig->Radar_Stream_Options[MMIC_CORE] = getMasterConfiguration("MMIC_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ALIGNMENT_CORE] = getMasterConfiguration("ALIGNMENT_CORE", pBuffer);
			pConfig->Radar_Stream_Options[BLOCKAGE_CORE] = getMasterConfiguration("BLOCKAGE_CORE", pBuffer);
			pConfig->Radar_Stream_Options[RADAR_CAPABILITY_CORE] = getMasterConfiguration("RADAR_CAPABILITY_CORE", pBuffer);
			pConfig->Radar_Stream_Options[DOWN_SELECTION_CORE] = getMasterConfiguration("DOWN_SELECTION_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ID_CORE] = getMasterConfiguration("ID_CORE", pBuffer);
			pConfig->Radar_Stream_Options[TOI_CORE] = getMasterConfiguration("TOI_CORE", pBuffer);
			pConfig->Radar_Stream_Options[DYNAMIC_ALIGNMENT_CORE] = getMasterConfiguration("DYNAMIC_ALIGNMENT_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_SAFETY_FAULTS_CORE] = getMasterConfiguration("ROT_SAFETY_FAULTS_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_TRACKER_INFO_CORE] = getMasterConfiguration("ROT_TRACKER_INFO_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_VEHICLE_INFO_CORE] = getMasterConfiguration("ROT_VEHICLE_INFO_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE] = getMasterConfiguration("ROT_OBJECT_STREAM_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_ISO_OBJECT_STREAM_CORE] = getMasterConfiguration("ROT_ISO_OBJECT_STREAM_CORE", pBuffer);
			pConfig->Radar_Stream_Options[ROT_PROCESSED_DETECTION_STREAM_CORE] = getMasterConfiguration("ROT_PROCESSED_DETECTION_STREAM_CORE", pBuffer);
			pConfig->RADAR_ECU_Stream_Options[ecu0] = getMasterConfiguration("ECU0", pBuffer);
			pConfig->RADAR_ECU_Stream_Options[ecu1] = getMasterConfiguration("ECU1", pBuffer);
			pConfig->RADAR_ECU_Stream_Options[ecu3] = getMasterConfiguration("ECU3", pBuffer);
			pConfig->RADAR_ECU_Stream_Options[ecu_VRU_Classifier] = getMasterConfiguration("ECU_VRU_Classifier", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RL] = getMasterConfiguration("REAR_LEFT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RR] = getMasterConfiguration("REAR_RIGHT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FR] = getMasterConfiguration("FRONT_RIGHT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FL] = getMasterConfiguration("FRONT_LEFT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FC] = getMasterConfiguration("FRONT_CENTER", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_BP_R] = getMasterConfiguration("BP_RIGHT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_BP_L] = getMasterConfiguration("BP_LEFT", pBuffer);
			pConfig->MUDP_Select_Sensor_Status[MUDP_FLR4P_FC_09] = getMasterConfiguration("FLR4P_FC", pBuffer);
			pConfig->MUDP_Radar_ECU = getMasterConfiguration("RADAR_ECU", pBuffer);
			pConfig->Data_Extracter_Mode[CSV] = getMasterConfiguration("CSV_MODE", pBuffer);
			pConfig->Data_Extracter_Mode[XML] = getMasterConfiguration("XML_TRACE_MODE", pBuffer);
			pConfig->Output_Xml_Trace_Option[Hil_Port_Xml] = getMasterConfiguration("HIL_port_xml",pBuffer);
         	//pConfig->Data_Extracter_Mode[kpi_error] = getMasterConfiguration("KPI_Error_Info", pBuffer);
			pConfig->Data_Extracter_Mode[PACKET_Loss] = getMasterConfiguration("Detail_Error_Info", pBuffer);
			pConfig->Data_Extracter_Mode[Quality_Check] = getMasterConfiguration("Log_Quality_Summary", pBuffer);
			pConfig->Data_Extracter_Mode[CCA_MF4_DUMP] = getMasterConfiguration("CCA_MF4_DUMP_MODE", pBuffer);
			pConfig->Output_Xml_Trace_Option[Sensor_Xml] = getMasterConfiguration("Sensor_xml",pBuffer);
			pConfig->Data_Extracter_Mode[CSV_DUMP_EXTRACTION] = getMasterConfiguration("CSV_DUMP_EXTRACTION", pBuffer);// CSV_MODE_Six parameters
			pConfig->Packet_Loss_Statistics = getMasterConfiguration("PACKET_LOSS_STATISTICS", pBuffer);
			pConfig->Cca_Lib = getMasterConfiguration("CCA_Library", pBuffer);
			pConfig->Vector_Lib = getMasterConfiguration("Vector_Library", pBuffer);
			pConfig->Input_File_mode = getMasterConfiguration("INPUT_FILE_MODE", pBuffer);
			pConfig->Report_File_Format = getMasterConfiguration("PACKET_LOSS_Report_Format",pBuffer);
			pConfig->MUDP_output_path_options = getMasterConfiguration("Output_Path_Options", pBuffer);
			pConfig->MUDP_output_path_location = getMasterConfiguration("Output_Path_Location", pBuffer);
			pConfig->logTracingPath = getMasterConfiguration("LogTarcingPath", pBuffer);

			pConfig->HTML_Report = getMasterConfiguration("HTML_REPORT_NONINTERACTIVE", pBuffer); //CodeChange#2: Reading HTML Tag status from config file
			pConfig->HTML_Scanindex_scaling= getMasterConfiguration("HTML_SCANINDEX_SCALING", pBuffer);
		
			//std::cout << std::endl << " pConfig->HTML_Report " << pConfig->HTML_Report; //Print#1

			// In Packet_loss.xml there is no option to enable or disable Output_Xml_Trace_Option[Sensor_Xml]
			// If Packet_loss.xml is using as configuration, by deafult Output_Xml_Trace_Option[Sensor_Xml] should be enabled
			if ("" == getMasterConfiguration("Sensor_xml", pBuffer)) 
			{
				pConfig->Output_Xml_Trace_Option[Sensor_Xml] = 1;
			}


			if (strcmp(pConfig->HTML_Report.c_str(), "1") == 0)
			{
				
				
				
				
				
		

				//Gen7
				if (strcmp(pConfig->gentype.c_str(), "GEN7") == 0)
				{
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RL] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RR] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FR] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FL] = "1";
					pConfig->Radar_Stream_Options[DET_CORE] = "1";
					pConfig->Radar_Stream_Options[ROT_OBJECT_STREAM_CORE] = "1";
					pConfig->Radar_Stream_Options[DYNAMIC_ALIGNMENT_CORE] = "1";
					pConfig->Radar_Stream_Options[RADAR_CAPABILITY_CORE] = "1";
					pConfig->Radar_Stream_Options[ID_CORE] = "1";
					pConfig->Radar_Stream_Options[STATUS_CORE] = "1";

				}
				//GEN6
				if(strcmp(pConfig->gentype.c_str(), "GEN6") == 0)
			
				{
					std::cout << std::endl << " GEN6 condition met" << std::endl;
					std::cout << std::endl << " Force Set of Z7B_Core TRACKER_CORE DET_CORE" << std::endl;
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FC] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RL] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_RR] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FR] = "1";
					pConfig->MUDP_Select_Sensor_Status[MUDP_SRR_FL] = "1";

					//pConfig->Radar_Stream_Options[Dspace] = "1";
					//pConfig->Data_Extracter_Mode[XML] = "1";

					/*pConfig->Radar_Stream_Options[TRACKER_CORE] = "1";
					pConfig->Radar_Stream_Options[DET_CORE] = "1";
					pConfig->Radar_Stream_Options[Z7B_Core] = "1";*/
					
					//pConfig->Radar_Stream_Options[Z7B_Customer] = "1";//for Feature Function
				}
				

				


				pConfig->Vector_Lib = "1";
			}
			

			if (strcmp(pConfig->HTML_Scanindex_scaling.c_str(), "1") == 0)
			{
				JsonInputParser& json = JsonInputParser::getInstance();
				json.bscanindex_scaling = 1;
			}
			else
			{
				JsonInputParser& json = JsonInputParser::getInstance();
				json.bscanindex_scaling = 0;
			}

			if(pBuffer){
				delete pBuffer;
				pBuffer = NULL;
			}

			return MUDP_OK;
		}
	}
	else
	{
		return MUDP_NOK;
	}
	return MUDP_OK;
}






/*===========================================================================================================*\
* File Revision History (top to bottom: first revision to last revision)
*============================================================================================================
*
*   Date         userid             Name               (Description on following lines: Jira/SCR #, etc.)
* -------------- --------  -----------------------  -----------------------------------------------------------
* 21-May-2019   fj5y98            BHAVANA				AGV-2562: Update UDP Dump tool to read XML and file input
* 
*\*==========================================================================================================*/
