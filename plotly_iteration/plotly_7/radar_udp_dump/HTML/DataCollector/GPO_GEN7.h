#ifndef GPO_GEN7_DATACOLLECTOR
#define GPO_GEN7_DATACOLLECTOR

#include<iostream>
#include<memory>
#include <fstream>

#include "../SharedData/SharedPlotData.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../helper/genutil.h"
#include "../HTMLReportManager/HTMLReportManager.h"
#include "../helper/genutil.h"
#include "../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"


class GpoGen7DataCollector:public IDataCollect
{
	
public:
	GpoGen7DataCollector(std::shared_ptr<PlotParameter[]> &sptr_plotparam);
	
	virtual ~GpoGen7DataCollector();
	virtual void collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                                short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                                unsigned int scanindex, unsigned int platform, bool IsRECU)override;
	virtual void generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)override;

	virtual void calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition) override;
	virtual void calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)override;
	
	
	double flt_to_dbl(float32_T con_value);
	bool inRange(double vehiclesi_start, double vehiclesi_end, double resimsi);
	inline void remove_old_html_reports(const char* f_name, const char* pltfolder,std::string reportpath);
	
	/*void collect_c2timing_info_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                             short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                             unsigned int scanindex, unsigned int platform, bool IsRECU);*/

	void collect_detection_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
								      short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
								      unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_detection_data_srr7p_Satellite(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_detection_data_srr7p_Standalone(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_tracker_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_alignment_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
										short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
										unsigned int scanindex, unsigned int platform, bool IsRECU);


	void collect_mnrblockage_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
									 short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
								    unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_interference_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		unsigned int scanindex, unsigned int platform, bool IsRECU);

	void collect_boresightangle_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		unsigned int scanindex, unsigned int platform, bool IsRECU);

	
	std::shared_ptr<PlotParameter[]>& sptr_plotparam_gen7;
	
	std::vector<double> scanindexRangeRef[6], scanindexRangeRef_tracker[6];
	ofstream ffile1, ffile2;

	int isbistatic_count_veh{ 0 };
	int issingletarget_count_veh{ 0 };
	int issuperrestarget_count_veh{ 0 };
	int isbistatic_count_resim{ 0 };
	int issingletarget_count_resim{ 0 };
	int issuperrestarget_count_resim{ 0 };
	double boresight_az_angle;
	double boresight_el_angle;
}; 
#endif
