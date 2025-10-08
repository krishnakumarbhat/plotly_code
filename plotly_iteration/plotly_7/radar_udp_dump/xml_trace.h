#ifndef XML_TRACE_H
#define XML_TRACE_H

#include "fixmac.h"
#include "Resim_Config.h"
void Populate_ECU_Data_to_XML(FILE* filename ,const char* LogFname,short stream_num, IRadarStream * g_pIRadarStrm,unsigned8_T Radar_Posn,  Customer_T custId);
void Populate_Valid_Data_to_XML(FILE* filename ,const char* LogFname, IRadarStream * g_pIRadarStrm,short stream_num,unsigned8_T Radar_Posn,  Customer_T custId);void Open_XML_file(FILE*);
void Input_Header_Info(unsigned8_T Radar_Posn, const char* LogFname,Customer_T custId,IRadarStream * g_pIRadarStrm );
void Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm, Customer_T custId,short stream_num );
void Record_Header_Man_Dpace_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm, Customer_T custId, short stream_num);
void Z4_Record_header_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId );
void VF_RR_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_FR_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_FL_Record_Header_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Raw_vehicle_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Vehicle_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Mounting_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId);
void TD_Blockage_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId);
void Opposite_Radar_Info_CHANGAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Opposite_Radar_Info_RNA(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Opposite_Radar_Info_MAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void VF_RR_Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_RL_Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_FR_Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_FL_Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Curvi_Tracks_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId);
void Curvi_Tracks_Info_CHANGAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Curvi_Tracks_Info_MAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Curvi_Tracks_Info_TML(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Alignment_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId);
void ADC_Physical_xml_Data(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Algo_Timing_Info_of_Z7B_core(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Algo_Timing_Info_of_Z7A_core(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Algo_Timing_Info_of_Z4_core(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void ASW_Output_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId); 
void LCDA_Output_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId); //Feature Function info Lane Change Decision Aid[LCDA]
void CTA_Output_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId); // Feature Function info Cross Traffic Alert[CTA]
void RECW_Output_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm,Customer_T custId); // Feature Function info Rear End Collision Warning[RECW]
void CED_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);// Feature Function info for Safe Exit [SFE/CED]
void TA_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);// Feature Function info for TA
void Freespace_Output_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);// Feature Function info for  Freespace
void FeatureFunction_Output(IRadarStream * g_pIRadarStrm,short stream_num);
void FeatureFunction_Info(IRadarStream * g_pIRadarStrm, Customer_T custId, unsigned8_T Radar_Posn);//Feacture Function info for NISSAN and HONDA
void Record_Header_ECU_Info(IRadarStream * g_pIRadarStrm,short stream_num);
void Vehicle_ECU_Info(IRadarStream * g_pIRadarStrm,short stream_num);
void Tracks_Info(IRadarStream * g_pIRadarStrm,short stream_num);
void Detection_ECU_Info(IRadarStream * g_pIRadarStrm,short stream_num);
void PCAN_Detection_Info(IRadarStream * g_pIRadarStrm,short stream_num);
void Core_Execution_Time(IRadarStream * g_pIRadarStrm,short stream_num);
void Record_Header_ECU_Count(IRadarStream * g_pIRadarStrm,short stream_num);
void write_Dets_Info(uint8_t sensor_index, unsigned8_T *index, IRadarStream * g_pIRadarStrm,short stream_num);
void Input_Header_ECU_Info(unsigned8_T Radar_Posn, const char* LogFname,Customer_T custId,IRadarStream * g_pIRadarStrm );
void Record_Header_Count(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void VF_Record_Header_Count(IRadarStream * g_pIRadarStrm);
void Free_space_info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Free_space_info_Man(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm);
void Z7a_First_pass_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm, Customer_T custId);
void Z4_Opposite_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm);
void Self_Radar_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm, Customer_T custId);
void Self_Radar_Info_CHANGAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Self_Radar_Info_RNA(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void Self_Radar_Info_MAN(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId);
void MNR_Blockage_Info(unsigned8_T Radar_Posn,IRadarStream * g_pIRadarStrm, Customer_T custId);
void VF_RL_Record_Header_Info(IRadarStream * g_pIRadarStrm);
extern int compare(const void *a, const void *b);


/////--------------gen5------------//

void Gen5_Input_Header_Info(unsigned8_T Radar_Posn, const char* LogFname, Customer_T custId, IRadarStream * g_pIRadarStrm);
void Gen5_Populate_Valid_Data_to_XML(FILE* filename, const char* LogFname, IRadarStream * g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId,uint32_t scanindex, uint32_t platform);
void Gen7_Populate_Valid_Data_to_XML(FILE* filename, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, uint32_t scanindex, uint32_t platform);
void Open_XML_file(FILE*);
void Open_CSV_file(FILE*);
void Gen5_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId);
void Gen5_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId);
void GEN7_Radar_Self_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);
void Gen7_SRR7P_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);
void GEN7_Alignment_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);

void Gen5_FLR4P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId);//detections for FLR4P
void FLR4P_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);//Trackes for FLR4P
void STLA_FLR4P_Curvi_Tracks_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);
void STLA_SRR6P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream *  g_pIRadarStrm, Customer_T custId);//STLA_SRR6P Detections CYW-2730
void TRATON_SRR6P_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId); //TRATON SRR6P detections CYW-5013
void STLA_FLR4_Self_Radar_Info(unsigned8_T Radar_Posn, IRadarStream* g_pIRadarStrm, Customer_T custId);//STLA_FLR4 Detections CYW-3354
void Gen5_Record_Header_Info(unsigned8_T Radar_Posn, IRadarStream * g_pIRadarStrm, Customer_T custId, short stream_num);
void Gen5_Record_Header_Count(unsigned8_T Radar_Posn, IRadarStream  *  g_pIRadarStrm);
//------------------------------------//

#endif