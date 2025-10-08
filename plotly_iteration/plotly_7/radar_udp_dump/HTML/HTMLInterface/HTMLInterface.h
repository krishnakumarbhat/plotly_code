#ifndef HTML_INTERFACE
#define HTML_INTERFACE

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"

#include <iostream>
#include<map>
class IDataCollect
{
	

public:
	IDataCollect() = default;
	virtual void collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                                short stream_num,unsigned8_T Radar_Posn, Customer_T custId, 
		                                unsigned int scanindex, unsigned int platform, bool IsRECU) = 0;
	virtual void generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath) = 0;
	virtual void calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition) = 0;
	virtual void calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition) = 0;
	//virtual void receive_master_updateonDatafactory(std::map<int, DataFactory*> map_datacollector_details) = 0;
	virtual ~IDataCollect() { 
		//std::cout << "~IDataCollect()"; 
	}


};
#endif