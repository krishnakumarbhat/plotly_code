/*===========================================================================*\
* FILE: RESIM_Config_Control_XML_Parser.cpp
*===========================================================================
* Copyright 2015 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*---------------------------------------------------------------------------
* %full_filespec: SRR3_SIL_Wrapper.cpp~78.1.1.1.22:c++:kok_css2#2 %
* %version: 78.1.1.1.22 %
*---------------------------------------------------------------------------
*
* DESCRIPTION:
*   This Module contains the XML Parser Code to Control RESIM Output Files
*
* ABBREVIATIONS:
*   TODO: List of abbreviations used, or reference(s) to external document(s)
*
* TRACEABILITY INFO:
*   Design Document(s):
*     TODO: Update list of design document(s)
*
*   Requirements Document(s):
*.
*
*   Applicable Standards (in order of precedence: highest first):
*     SW REF 264.15D "Delphi C Coding Standards" [23-Dec-2001]
*     TODO: Update list of other applicable standards
*
* DEVIATIONS FROM STANDARDS:
*   TODO: List of deviations from standards in this file, or
*   None.
*
\*===========================================================================*/
/*===========================================================================*\
 * Other Header Files
\*===========================================================================*/
#include "Emb_Lib_Output_Control.h"
#include "GEN7_SIL_Output_Data.h"
#include "GEN7_SIL_Wrapper.h"
#include <algorithm>
extern "C"
{
// #include "usc_cal.h"
#include "Endian_Swap.h"
   // #include "interface.h"
}
SIL_Customer_CC_T customer = enCUST_CC_RNA_SRR5;
Emb_Lib_Output_Control_T XML_Tag;
uint8_t Enable_XTRK_Output = 0;

/*Reading USC from xml, path will be defined in Embedded xml*/
// USC_Calibrations_Cal_T USC_Calibrations_Cal_DF_V_Read_from_XML;
string embedded_xml_control_field_data[105];
extern uint8_t Get_Current_ECU_Position(void);
// SIL_SRR3_INPUT_DATA_T* Get_Virtual_Fusion_Opp_Dets(void);
// static SIL_SRR3_INPUT_DATA_T* virtual_fusion_opp_reports = Get_Virtual_Fusion_Opp_Dets();
/*===========================================================================*\
 * function prototype
\*===========================================================================*/
void Read_USC_Data_From_XML(string filename);
/*===========================================================================*\
 * Main function
\*===========================================================================*/
Emb_Lib_Output_Control_T *Get_Emb_Lib_Output_Control(char *filepath)
{
   FILE *pFile = NULL;
   if (strcmp(filepath, "NONE") != 0)
   {
      pFile = fopen(filepath, "r");
   }
   Fill_XML_Structure(filepath, pFile);
   if (pFile != NULL)
   {
      // Fill_XML_Structure(path);
      if (XML_Tag.Suppress_BIN_Files == "BIN_FILES_DISABLED")
      {
         // AS_BWW_SUPPRESS_ALL_BIN_FILES(TRUE);
      }
      fclose(pFile);
   }
   return (&XML_Tag);
}

/*Tokenize Nodes between XML Tags*/
string Tokenize_Nodes_between_Tags(string line, int t_signal_val)
{
   int getpos_s;
   string s_val;
   t_signal_val--;

   getpos_s = line.find(">", t_signal_val);
   s_val    = line.substr(getpos_s + 1, (line.find_first_of('<', getpos_s) - getpos_s - 1)).c_str();
   return s_val;
}

/*Get Node values between XML Tags*/
string Get_Config_Control_values(string chk_var, string filename)
{
   string line;
   chk_var.insert(0, "<");
   chk_var.back();
   chk_var.append(">");

   int t_yes_val;
   string s_val = "0";
   // ifstream fin(filename.c_str());
   FILE *pFile = fopen(filename.c_str(), "r");
   char mystring[2048];
   if (pFile == NULL)
   {
      // perror ("[ERROR]:Failed to open embedded xml file \n");
   }
   else
   {
      while (fgets(mystring, 2048, pFile))
      {
         line = mystring;
         // if(fin.eof()) break;
         t_yes_val = line.find(chk_var);
         if (t_yes_val >= 0)
         {
            s_val = Tokenize_Nodes_between_Tags(line, 4);
            break;
         }
      }
      fclose(pFile);
   }
   return s_val;
}

/*Get Node values between XML Tags*/
// string Get_XML_Config_Control_values(string chk_var)
string Get_XML_Config_Control_values(char *chk)
{
   string s_val   = "1";
   string chk_var = chk;
   chk_var.insert(0, "<");
   chk_var.back();
   chk_var.append(">");
   for (int i = 0; i < 105; i++)
   {
      int value_found = embedded_xml_control_field_data[i].find(chk_var);
      if (value_found >= 0)
      {
         s_val = Tokenize_Nodes_between_Tags(embedded_xml_control_field_data[i], 4);
      }
   }
   return s_val;
}

/*Loads all the tags from the XML and stores it in the RAM buffer*/
void Read_All_Control_Values_From_XML(string filename, FILE *pFile)
{
   int comment_start_line;
   int comment_end_line;
   int control_fields_with_tabs;
   int control_fields_without_tabs;
   int control_fields_with_spaces;
   int control_fields_without_spaces;
   int comments_and_tag_info_in_same_line = 0;
   int comments_found                     = FALSE;
   int index                              = 0;
   string line;
   // ifstream fin(filename.c_str());
   char mystring[2048];

   while (fgets(mystring, 2048, pFile))
   {
      line = mystring;
      /**/
      control_fields_with_tabs    = line.find("\t<");
      control_fields_without_tabs = line.find("<");
      control_fields_with_spaces  = line.find(" <");
      /*if control fields apart not found then continue to next line*/
      if (((control_fields_with_tabs * control_fields_without_tabs) < 0 ||
           (control_fields_with_tabs < 0 && control_fields_without_tabs < 0)) &&
          !comments_found)
      {
         if (control_fields_with_spaces < 0)
         {
            continue;
         }
      }
      /*if control field found check -> is it a comment field*/
      comment_start_line = line.find("<!--");
      if (comment_start_line >= 0)
      {
         comments_and_tag_info_in_same_line = std::count(line.begin(), line.end(), '<');
      }
      /*if comment field then search for endof comment. if end of comment is not in same line then continue search for the next
       * line*/
      if ((comment_start_line >= 0 || comments_found) && (comments_and_tag_info_in_same_line < 2))
      {
         comments_found   = TRUE;
         comment_end_line = line.find("-->");
         if (comment_end_line >= 0)
         {
            /*end of comment found*/
            comments_found = FALSE;
         }
      }
      else
      {
         /*if no comment fields then add control fields to buffer*/
         embedded_xml_control_field_data[index++] = line;
         comments_and_tag_info_in_same_line       = 0;
      }
   }
}

/*Fill buffer with XML Config values*/
uint8_t Fill_XML_Structure(string path, FILE *pFile)
{
   if (pFile != NULL)
      Read_All_Control_Values_From_XML(path, pFile);
   XML_Tag.Suppress_BIN_Files       = Get_XML_Config_Control_values("BIN_FILES");
   XML_Tag.Suppress_XML_Trace_Files = Get_XML_Config_Control_values("XML_Trace_Files");
#ifdef CUSTOMER_BMW_SRR5
   XML_Tag.Load_Exteranl_Error_XML = Get_XML_Config_Control_values("LOAD_ANERMA_ERROR");
#endif
   XML_Tag.Suppress_Tracker_internal_log = Get_XML_Config_Control_values("LOAD_TRACKER_INTERNALS");
   XML_Tag.Car_Type                      = Get_XML_Config_Control_values("Car_Type");
   XML_Tag.Suppress_UDP_Transmission     = Get_XML_Config_Control_values("UDP_TRANSMISSION");
   XML_Tag.Suppress_ADCAM_LANEMsg        = Get_XML_Config_Control_values("Ethernet_ADCAM_LANEMsg");
   XML_Tag.Suppress_CAN_Transmission     = Get_XML_Config_Control_values("CAN_TRANSMISSION");
   XML_Tag.Input_file_path_reference     = Get_XML_Config_Control_values("ANERMA_PARAMETER_FILE_REFERENCE");
   XML_Tag.Input_file_path_test          = Get_XML_Config_Control_values("ANERMA_PARAMETER_FILE_TEST");
   XML_Tag.Suppress_Console_Prints       = Get_XML_Config_Control_values("CONSOLE_PRINTS");
   XML_Tag.SM2_MODE_Status               = Get_XML_Config_Control_values("SM2_MODE_Status");
   XML_Tag.DGPS_Decoding_Enable          = Get_XML_Config_Control_values("DGPS_Decode_Status");
   XML_Tag.SM2_Execution_Status          = Get_XML_Config_Control_values("SM2_Execution_Status");
   XML_Tag.SM2_Config_File_Path          = Get_XML_Config_Control_values("SM2_Config_File");
   XML_Tag.MESH_Config_File_Path         = Get_XML_Config_Control_values("MESH_Config_File");
   XML_Tag.CAF_Config_File_Path          = Get_XML_Config_Control_values("SM_Paramater_File");
   // XML_Tag.Suppress_Usc_Xml_Prints = Get_XML_Config_Control_values("Suppress_Usc_Xml_log");
   // XML_Tag.Suppress_Usc_Csv_Prints = Get_XML_Config_Control_values("Suppress_Usc_Csv_log");
   XML_Tag.Auto_Align_Option                = Get_XML_Config_Control_values("Auto_Align_Option");
   XML_Tag.Suppress_Usc_Xml_Prints          = Get_XML_Config_Control_values("Suppress_Usc_Xml_log");
   XML_Tag.Suppress_Usc_Csv_Prints          = Get_XML_Config_Control_values("Suppress_Usc_Csv_log");
   XML_Tag.speed_compensation_factor_option = Get_XML_Config_Control_values("Speed_Compensation_Factor");
   XML_Tag.Caf_Parameter_option             = Get_XML_Config_Control_values("CAF_Hardcode_Factor");
   XML_Tag.SI_Check_report                  = Get_XML_Config_Control_values("SI_MISS_CHECK");
   XML_Tag.Veh_Data_CSV_Path                = Get_XML_Config_Control_values("Veh_Data_CSV_Path");
   XML_Tag.xtrk_enable                      = Get_XML_Config_Control_values("XTRK_ENABLE");
   if (XML_Tag.xtrk_enable == "XTRK_FILES_ENABLED")
   {
      Enable_XTRK_Output = 1;
   }
   if (Get_XML_Config_Control_values("dSPACE_OSI") == "WRITE_BIN")
   {
      XML_Tag.dSPACE_OSI_bin_write_enable = 1;
   }
   else if (Get_XML_Config_Control_values("dSPACE_OSI") == "WRITE_XML")
   {
      XML_Tag.dSPACE_OSI_bin_write_enable = 2;
   }
   else if (Get_XML_Config_Control_values("dSPACE_OSI") == "WRITE_CSV")
   {
      XML_Tag.dSPACE_OSI_bin_write_enable = 3;
   }
   else
   {
      XML_Tag.dSPACE_OSI_bin_write_enable = 0;
   }

   if (path == "NONE")
   {
      XML_Tag.Suppress_Console_Prints = "CONSOLE_PRINTS_ENABLED";
   }
   memset(&XML_Tag.Output_XML_Trace_Files, 0, sizeof(Emb_Lib_Output_XML_Trace_Files_T));
   memset(&XML_Tag.Auto_Align_Hard_Coded_Values, 0, sizeof(Align_Anle_Quality_Hardcoded_Values_T));

#if (defined CUSTOMER_RNA || defined SRR6)
   XML_Tag.SMC_Calibration_Source =
      Get_Config_Control_values("SMC_CALIBRATION_SOURCE", path).c_str(); // control for SMC decode from UDP
   /********************************************************Alignmet Algo input
    * injection*************************************************************/
   /* ALIGNMENT ALGORITHM To Execute */
   string Align_Algo_to_Run_EB = Get_Config_Control_values("CALIBRATION_ALGORITHM_TO_EXECUTE", path).c_str();
   if (Align_Algo_to_Run_EB == "AUTO_ALIGNMENT")
   {
      XML_Tag.Align_Algo_to_Run_EB = 0;
   }
   else if (Align_Algo_to_Run_EB == "SHORT_TRACK_ALIGNMENT")
   {
      XML_Tag.Align_Algo_to_Run_EB = 1;
   }
   else if (Align_Algo_to_Run_EB == "KD_ALIGNMENT")
   {
      XML_Tag.Align_Algo_to_Run_EB = 2;
   }
   else if (Align_Algo_to_Run_EB == "EOL_ALIGNMENT")
   {
      XML_Tag.Align_Algo_to_Run_EB = 3;
   }
   else
   {
   } // do nothing

   /* Autocalibration  Source */
   string Auto_Calib_Source_EB = Get_Config_Control_values("AUTOCALIBRATION_SOURCE", path).c_str();
   if (Auto_Calib_Source_EB == "XML_FIXED")
   {
      XML_Tag.Auto_Calib_Source_EB = 0;
   }
   else if (Auto_Calib_Source_EB == "UDP_LOG")
   {
      XML_Tag.Auto_Calib_Source_EB = 1;
   }
   else if (Auto_Calib_Source_EB == "SIL_START_FROM_UDP")
   {
      XML_Tag.Auto_Calib_Source_EB = 2;
   }
   else if (Auto_Calib_Source_EB == "SIL_START_FROM_XML")
   {
      XML_Tag.Auto_Calib_Source_EB = 3;
   }
   else
   {
   } // do nothing

   /* Autocalibration Values from XML File */
   XML_Tag.Align_values_EB.ALIGNMENT_REAR_LEFT   = atof(Get_Config_Control_values("ALIGNMENT_REAR_LEFT", path).c_str());
   XML_Tag.Align_values_EB.ALIGNMENT_REAR_RIGHT  = atof(Get_Config_Control_values("ALIGNMENT_REAR_RIGHT", path).c_str());
   XML_Tag.Align_values_EB.ALIGNMENT_FRONT_RIGHT = atof(Get_Config_Control_values("ALIGNMENT_FRONT_RIGHT", path).c_str());
   XML_Tag.Align_values_EB.ALIGNMENT_FRONT_LEFT  = atof(Get_Config_Control_values("ALIGNMENT_FRONT_LEFT", path).c_str());

   /* KD / EOL Input Values from XML */
   XML_Tag.EOL_Values_EB.Value_REAR_LEFT.Activated =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ACTIVATED_REAR_LEFT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_REAR_LEFT.Range =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_RANGE_REAR_LEFT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_REAR_LEFT.Angle =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ANGLE_REAR_LEFT", path).c_str());

   XML_Tag.EOL_Values_EB.Value_REAR_RIGHT.Activated =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ACTIVATED_REAR_RIGHT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_REAR_RIGHT.Range =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_RANGE_REAR_RIGHT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_REAR_RIGHT.Angle =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ANGLE_REAR_RIGHT", path).c_str());

   XML_Tag.EOL_Values_EB.Value_FRONT_LEFT.Activated =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ACTIVATED_FRONT_LEFT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_FRONT_LEFT.Range =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_RANGE_FRONT_LEFT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_FRONT_LEFT.Angle =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ANGLE_FRONT_LEFT", path).c_str());

   XML_Tag.EOL_Values_EB.Value_FRONT_RIGHT.Activated =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ACTIVATED_FRONT_RIGHT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_FRONT_RIGHT.Range =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_RANGE_FRONT_RIGHT", path).c_str());
   XML_Tag.EOL_Values_EB.Value_FRONT_RIGHT.Angle =
      atof(Get_Config_Control_values("DOPPLER_GENERATOR_ANGLE_FRONT_RIGHT", path).c_str());
#endif
   /*********************************************************************************************************************/
   if (pFile != NULL)
   {
      XML_Tag.Output_XML_Trace_Files.CAL_Error_Files = atoi(Get_Config_Control_values("Cal_Error_Files", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Active_Faults   = atoi(Get_Config_Control_values("Active_Faults", path).c_str());
      XML_Tag.Output_XML_Trace_Files.UDP_Datas       = atoi(Get_Config_Control_values("UDP_Datas", path).c_str());
      XML_Tag.Output_XML_Trace_Files.OCG_info        = atoi(Get_Config_Control_values("OCG_info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.roadmodel_info  = atoi(Get_Config_Control_values("roadmodel_info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.static_objects  = atoi(Get_Config_Control_values("Static_Objects", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Moving_Objects  = atoi(Get_Config_Control_values("Moving_Objects", path).c_str());

      //	XML_Tag.Output_XML_Trace_Files.Vehicle_Info = atoi(Get_Config_Control_values("Raw_vehicle_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Vehicle_Info          = atoi(Get_Config_Control_values("Vehicle_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Mounting_Info         = atoi(Get_Config_Control_values("Mounting_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.VSE_Data              = atoi(Get_Config_Control_values("VSE_Data", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Interference_Dets     = atoi(Get_Config_Control_values("Interference_Dets", path).c_str());
      XML_Tag.Output_XML_Trace_Files.MNR_blockage          = atoi(Get_Config_Control_values("MNR_blockage", path).c_str());
      XML_Tag.Output_XML_Trace_Files.TD_blockage           = atoi(Get_Config_Control_values("TD_blockage", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Self_Dets             = atoi(Get_Config_Control_values("Self_Dets", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Opp_Dets              = atoi(Get_Config_Control_values("Opp_Dets", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Curvi_Tracks          = atoi(Get_Config_Control_values("Curvi_Tracks", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Alignment_Info        = atoi(Get_Config_Control_values("Alignment_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.ADC_Physical_Data     = atoi(Get_Config_Control_values("ADC_Physical_Data", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Algo_Z7B_core         = atoi(Get_Config_Control_values("Algo_Z7B_core", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Algo_Z7A_core         = atoi(Get_Config_Control_values("Algo_Z7A_core", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Algo_Z4_core          = atoi(Get_Config_Control_values("Algo_Z4_core", path).c_str());
      XML_Tag.Output_XML_Trace_Files.LCDA_Info             = atoi(Get_Config_Control_values("LCDA_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.CTA_Info              = atoi(Get_Config_Control_values("CTA_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.ELK_Info              = atoi(Get_Config_Control_values("ELK_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.HUD_Info              = atoi(Get_Config_Control_values("HUD_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.CED_Info              = atoi(Get_Config_Control_values("CED_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.PATH_Tracking_Info    = atoi(Get_Config_Control_values("PATH_Tracking_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.RECW_Info             = atoi(Get_Config_Control_values("RECW_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.TA_Info               = atoi(Get_Config_Control_values("TA_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.ASW_Info              = atoi(Get_Config_Control_values("ASW_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.ABA_Info              = atoi(Get_Config_Control_values("ABA_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.kd_eol                = atoi(Get_Config_Control_values("kd_eol", path).c_str());
      XML_Tag.Output_XML_Trace_Files.OH_Info               = atoi(Get_Config_Control_values("OH_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Fences_Info           = atoi(Get_Config_Control_values("Fences_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Z4_Z7B_Cust_Data      = atoi(Get_Config_Control_values("Z4_Z7B_Cust_Data", path).c_str());
      XML_Tag.Output_XML_Trace_Files.RCCA_Info             = atoi(Get_Config_Control_values("RCCA_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Freespace_Info        = atoi(Get_Config_Control_values("Freespace_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.VisOb_Info            = atoi(Get_Config_Control_values("VisOb_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.scan_prints           = atoi(Get_Config_Control_values("scan_prints", path).c_str());
      XML_Tag.Output_XML_Trace_Files.plp_algorithems       = atoi(Get_Config_Control_values("plp_algorithems", path).c_str());
      XML_Tag.Output_XML_Trace_Files.plp_analysis          = atoi(Get_Config_Control_values("plp_analysis", path).c_str());
      XML_Tag.Output_XML_Trace_Files.PCAN_Decoder          = atoi(Get_Config_Control_values("PCAN_Decoder", path).c_str());
      XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = atoi(Get_Config_Control_values("STATE_MACHINE", path).c_str());
      XML_Tag.Output_XML_Trace_Files.CAMERA_Decoder        = atoi(Get_Config_Control_values("CAMERA_VCAN_Decoder", path).c_str());
      XML_Tag.Output_XML_Trace_Files.UDP_Log_Data_CSV      = atoi(Get_Config_Control_values("UDP_Log_Data_CSV", path).c_str());
      XML_Tag.Output_XML_Trace_Files.PCAN_DECODER_XML      = atoi(Get_Config_Control_values("PCAN_DECODER_XML", path).c_str());
      XML_Tag.Output_XML_Trace_Files.VCAN_CSV_PRINT        = atoi(Get_Config_Control_values("VCAN_CSV_PRINT", path).c_str());
      XML_Tag.Output_XML_Trace_Files.fusion_scan_out       = atoi(Get_Config_Control_values("fusion_scan_out", path).c_str());
      XML_Tag.Output_XML_Trace_Files.IPC_Error_Infor       = atoi(Get_Config_Control_values("IPC_Error_Infor", path).c_str());
      XML_Tag.Output_XML_Trace_Files.MUX_PCAN_Erro_Info    = atoi(Get_Config_Control_values("PCAN_Error_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.MUX_Enet_Frame_Info = atoi(Get_Config_Control_values("ENET_Frame_Error_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.MUX_Hil_Bb_Errors_info = atoi(Get_Config_Control_values("HIL_BB_Erors_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.CTB_Info               = atoi(Get_Config_Control_values("CTB_Info", path).c_str());
      XML_Tag.Embedded_plot_report                          = Get_XML_Config_Control_values("Embedded_Report");
      XML_Tag.Embedded_plot_config.Detection_Plots          = atoi(Get_Config_Control_values("Detection_Plots", path).c_str());
      XML_Tag.Embedded_plot_config.Tracker_Plots            = atoi(Get_Config_Control_values("Tracker_Plots", path).c_str());
      XML_Tag.Embedded_plot_config.MNR_Plots                = atoi(Get_Config_Control_values("MNR_Plots", path).c_str());
      XML_Tag.Embedded_plot_config.Time_Throughput_Plots = atoi(Get_Config_Control_values("Time_Throughput_Plots", path).c_str());
      XML_Tag.Embedded_plot_config.Feature_Function_Plots =
         atoi(Get_Config_Control_values("Feature_Function_Plots", path).c_str());
      XML_Tag.Embedded_plot_config.Vehicle_Info_Plots     = atoi(Get_Config_Control_values("Vehicle_Info_Plots", path).c_str());
      XML_Tag.Output_XML_Trace_Files.KINK_Info            = atoi(Get_Config_Control_values("KINK_Info", path).c_str());
      XML_Tag.Output_XML_Trace_Files.DGPS_CSV_dump        = atoi(Get_Config_Control_values("DGPS_CSV_dump", path).c_str());
      XML_Tag.Output_XML_Trace_Files.Down_Select_Obj_Info = atoi(Get_Config_Control_values("Down_Select_Obj_Info", path).c_str());

      // Auto_Align_Hard_Coded_Values for Align Angle and Align Quality Factor for 4 +  2 Sensors
      XML_Tag.Auto_Align_Hard_Coded_Values.Front_Left_AA = atof(Get_Config_Control_values("Front_Left_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Front_Left_AQF = atoi(Get_Config_Control_values("Front_Left_Align_QF", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Front_Right_AA =
         atof(Get_Config_Control_values("Front_Right_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Front_Right_AQF =
         atoi(Get_Config_Control_values(" Front_Right_Align_QF", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Left_Center_AA =
         atof(Get_Config_Control_values("Left_Center_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Left_Center_AQF = atoi(Get_Config_Control_values("Left_Center_Align_QF", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Rear_Left_AA  = atof(Get_Config_Control_values("Rear_Left_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Rear_Left_AQF = atoi(Get_Config_Control_values("Rear_Left_Align_QF", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Rear_Right_AA = atof(Get_Config_Control_values("Rear_Right_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Rear_Right_AQF = atoi(Get_Config_Control_values("Rear_Right_Align_QF", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Right_Center_AA =
         atof(Get_Config_Control_values("Right_Center_Align_Angle", path).c_str());
      XML_Tag.Auto_Align_Hard_Coded_Values.Right_Center_AQF =
         atoi(Get_Config_Control_values("Right_Center_Align_QF", path).c_str());

      /*Speed_Compensation_Factor Hard Coded Value From Confic File For Overwrite to all sensors.*/
      XML_Tag.speed_compensation_fctr_values.Front_Left_Speed_Compensation_Factor =
         atof(Get_Config_Control_values("Front_Left_Speed_Compensation_Factor", path).c_str());
      XML_Tag.speed_compensation_fctr_values.Front_Right_Speed_Compensation_Factor =
         atof(Get_Config_Control_values("Front_Right_Speed_Compensation_Factor", path).c_str());
      XML_Tag.speed_compensation_fctr_values.Rear_Left_Speed_Compensation_Factor =
         atof(Get_Config_Control_values("Rear_Left_Speed_Compensation_Factor", path).c_str());
      XML_Tag.speed_compensation_fctr_values.Rear_Right_Speed_Compensation_Factor =
         atof(Get_Config_Control_values("Rear_Right_Speed_Compensation_Factor", path).c_str());

      XML_Tag.dgps_boxoffset_values.HOST_DGPS_BOX_OFFSET = atof(Get_Config_Control_values("HOST_DGPS_BOX_OFFSET", path).c_str());
      XML_Tag.dgps_boxoffset_values.TARGET_DGPS_BOX_OFFSET =
         atof(Get_Config_Control_values("TARGET_DGPS_BOX_OFFSET", path).c_str());
      /*
      SM_DISABLE::STATE MACHINE IS NOT EXECUTED IN RESIM, STATE MACHINE OUTPUTS FROM UDP/CORE0 ARE USED DIRECTLY IN RESIM.
      SM_ENABLE_CAFF_MAGNA_A310_ENDURANCE::STATE MACHINE IS NOT EXECUTED IN RESIM AND FEATURE FUNCTION STATE MACHINE OUTPUT VALUES
      ARE HARDCODE FOR LOGS BELOW SW::A470(CAFF VALUES AND STATE_MACHINE OUTPUTS ARE NOT AVAILABLE IN UDP)
      SM_ENABLE_CAFF_UDP::STATE MACHINE IS EXECUTED  IN RESIM AND CAFF VALUES ARE PICKED UP FROM UDP(VALID CAFF ARE AVAILABLE IN
      UDP FROM SW : MARCH 25 2021) SM_ENABLE_CAFF_HARDCODE_G01BLACK_G01GOLD_G31:: STATE MACHINE IS EXECUTED IN RESIM AND FF CAFF
      VALUES ARE PICKED UP FROM C FILE(CAFF VALUES FROM SW MARCH 25 2021 ARE CORRUPTED)
      */
      string State_Machine_Config_Value                    = Get_Config_Control_values("STATE_MACHINE", path);
      XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = SM_Config_Data::enSM_DISABLE;
      if ("SM_DISABLE" == State_Machine_Config_Value)
         XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = SM_Config_Data::enSM_DISABLE;
      else if ("SM_ENABLE_CAFF_MAGNA_A310_ENDURANCE" == State_Machine_Config_Value)
         XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = SM_Config_Data::enSM_ENABLE_CAFF_MAGNA_A310_ENDURANCE;
      else if ("SM_ENABLE_CAFF_UDP" == State_Machine_Config_Value)
         XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = SM_Config_Data::enSM_ENABLE_CAFF_UDP;
      else if ("SM_ENABLE_CAFF_HARDCODE_G01BLACK_G01GOLD_G31" == State_Machine_Config_Value)
         XML_Tag.Output_XML_Trace_Files.State_Machine_Decoder = SM_Config_Data::enSM_ENABLE_CAFF_HARDCODE_G01BLACK_G01GOLD_G31;

      XML_Tag.Caf_Parameter_Values.Rearaxel = atof(Get_Config_Control_values("Rearaxel", path).c_str());
   }
   XML_Tag.Embedded_plot_report                        = Get_XML_Config_Control_values("Embedded_Report");
   XML_Tag.Embedded_plot_config.Detection_Plots        = atoi(Get_Config_Control_values("Detection_Plots", path).c_str());
   XML_Tag.Embedded_plot_config.Tracker_Plots          = atoi(Get_Config_Control_values("Tracker_Plots", path).c_str());
   XML_Tag.Embedded_plot_config.MNR_Plots              = atoi(Get_Config_Control_values("MNR_Plots", path).c_str());
   XML_Tag.Embedded_plot_config.Time_Throughput_Plots  = atoi(Get_Config_Control_values("Time_Throughput_Plots", path).c_str());
   XML_Tag.Embedded_plot_config.Feature_Function_Plots = atoi(Get_Config_Control_values("Feature_Function_Plots", path).c_str());
   XML_Tag.Embedded_plot_config.Vehicle_Info_Plots     = atoi(Get_Config_Control_values("Vehicle_Info_Plots", path).c_str());
   return 1;
}

uint8_t Convert_Ascii_To_Digits(char value)
{
   uint8_t return_cal = 0;
   switch (value)
   {
      case '0':
         return_cal = 0;
         break;
      case '1':
         return_cal = 1;
         break;
      case '2':
         return_cal = 2;
         break;
      case '3':
         return_cal = 3;
         break;
      case '4':
         return_cal = 4;
         break;
      case '5':
         return_cal = 5;
         break;
      case '6':
         return_cal = 6;
         break;
      case '7':
         return_cal = 7;
         break;
      case '8':
         return_cal = 8;
         break;
      case '9':
         return_cal = 9;
         break;
      case 'A':
         return_cal = 10;
         break;
      case 'B':
         return_cal = 11;
         break;
      case 'C':
         return_cal = 12;
         break;
      case 'D':
         return_cal = 13;
         break;
      case 'E':
         return_cal = 14;
         break;
      case 'F':
         return_cal = 15;
         break;
      default:
         return_cal = -1;
         break;
   }
   return return_cal;
}

void Read_USC_Data_From_XML(string filename)
{
   string usc_data = Get_Config_Control_values("USC_CALIBRATIONS_CAL_DF_V", filename);
   uint8_t usc_data_bytes[8]; // sizeof(USC_Calibrations_Cal_T)];

   uint32_t index = 0;
   for (int i = 0; i < usc_data.size() - 1; i = i + 2)
   {
      char temp1            = (usc_data.at(i));
      char temp2            = (usc_data.at(i + 1));
      uint8_t itemp1        = Convert_Ascii_To_Digits(temp1);
      uint8_t itemp2        = Convert_Ascii_To_Digits(temp2);
      usc_data_bytes[index] = (itemp1) << 4 | (itemp2);
      index++;
   }

   // memcpy(&USC_Calibrations_Cal_DF_V_Read_from_XML, usc_data_bytes,sizeof(USC_Calibrations_Cal_T));
   // Reverse_Array(&USC_Calibrations_Cal_DF_V_Read_from_XML,sizeof(USC_Calibrations_Cal_T),ONE_BYTE);
}

uint8_t *Get_USC_Calibrations_Cal_DF_V_Read_from_XML(void)
{
   uint8_t *temp;
   return ((uint8_t *)temp); //&USC_Calibrations_Cal_DF_V_Read_from_XML);
}

uint8_t Get_USC_Read_From_Xml_Enabled(void)
{
   uint8_t return_val = 0;
   if (XML_Tag.usc_xml_control.Enable_copy_from_xml == "USC_READ_FROM_XML")
   {
      return_val = 1;
   }
   return (return_val);
}

Emb_Lib_Output_Control_T *Get_XML_Tag_pointer()
{
   return &XML_Tag;
}
/*===========================================================================*\
 * File Revision History (top to bottom: first revision to last revision)
 *===========================================================================
 *
 * Date        Name		(Description on following lines: SCR #, etc.)
 * ----------- --------		 -----------------------------------------------------
 * 09/05/2024  krishnakumar    Adapted the file for GEN7 GPO
 *\*===========================================================================*/
