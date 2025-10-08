#ifndef GPOPLATFORM_DATACOLLECTOR
#define GPOPLATFORM_DATACOLLECTOR

#include<iostream>
#include<memory>

#include "../SharedData/SharedPlotData.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../helper/genutil.h"
#include "../HTMLReportManager/HTMLReportManager.h"
#include "../helper/genutil.h"
#include "../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"

class Gen5PlatformDataCollector:public IDataCollect
{
	
public:
	Gen5PlatformDataCollector();
	Gen5PlatformDataCollector(Gen5PlatformDataCollector&) = delete;
	Gen5PlatformDataCollector& operator=(Gen5PlatformDataCollector&) = delete;
	virtual ~Gen5PlatformDataCollector();
	static Gen5PlatformDataCollector& getInstance();
	

	virtual void collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                                short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                                unsigned int scanindex, unsigned int platform, bool IsRECU)override;
	virtual void generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)override;

	virtual void calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition) override;
	virtual void calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)override;
	//virtual void receive_master_updateonDatafactory(std::map<int, DataFactory*> map_datacollector_details)override;
	
	double flt_to_dbl(float32_T con_value);
	bool inRange(double vehiclesi_start, double vehiclesi_end, double resimsi);
	inline void remove_old_html_reports(const char* f_name, const char* pltfolder,std::string reportpath);
	

	void collect_vehicle_data_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
							  short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
							  unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_vehicle_data_flr4(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                      short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                       unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_c2timing_info_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                            short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                             unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_c2timing_info_flr4(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                            short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                            unsigned int scanindex, unsigned int platform, bool IsRECU);


	void collect_detection_data_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
								short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
								unsigned int scanindex, unsigned int platform, bool IsRECU);


	//Below two methods for data collection

	void collect_detection_data_gpo_flr4p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                           short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                           unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_tracker_data_gpo(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
								short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
								unsigned int scanindex, unsigned int platform, bool IsRECU);



	


	
	

	//std::unique_ptr<Vehicle_Plot_Param[]> pvehicle_info;

	Vehicle_Plot_Param_T*pvehicle_info;
	Timing_Info_T* ptiming_info;
	SelfDetection_Plot_Parameters_T* pselfdetection;
	Tool_RunTime_info_T* ptoolruntime;
	Tracker_Plot_Parameter_T* ptracker;
	std::vector<double> scanindexRangeRef[6];

}; 
#endif
