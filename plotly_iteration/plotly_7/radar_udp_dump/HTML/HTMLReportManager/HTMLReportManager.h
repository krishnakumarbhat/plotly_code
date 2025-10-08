#ifndef HTML_REPORT_MANAGER
#define HTML_REPORT_MANAGER

#include<iostream>
#include<map>
#include<memory>
#include<set>
#include<vector>

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"

#include "../HTMLInterface/HTMLInterface.h"
#include "../DataFactory/DataFactory.h"
#include "../DataFactory/PlotFactory.h"
#include "../DataFactory/HDF_DBFactory.h"

#include "../PlotAdapters/IPlotAdapter.h"
#include "../DBAdapters/IHDF_DBAdapters.h"
#include "../PlotAdapters//IPlotAdapterObserver.h"
#include "../DBUtils/HDF_DBNodeGenerator.h"

class HtmlReportManager
{
	
public:

	HtmlReportManager(std::shared_ptr<PlotParameter[]> &plotparam);
	virtual ~HtmlReportManager();
	
	//register methods for adapters
	void register_datacollector(int customerID, std::shared_ptr<IDataCollect>& sptr);
	void register_plotadaptes(int plotytpe, std::shared_ptr<IPlotAdapter>& sptr);
	void register_dbadaptes(int plotytpe, std::shared_ptr<IHDF_DBAdapters>& sptr);

	void register_customer(int currentcustomerID);
	void register_customer_id_toReportManager(int currentcustomerID, bool bvistied = false);
	void clear_datacollector_registry();

	void dispatch_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm,
		                          short stream_num, unsigned8_T Radar_Posn, Customer_T custId,
		                          unsigned int scanindex, unsigned int platform, bool IsRECU);
	

	//trigger methods to invoke concrete class (example plot generation , db generation )
	void trigger_plotgeneration(std::vector<int> vec_custID,const char* f_name, const char* pltfolder, int filecount, std::string reportpath);

	void trigger_dbgeneration(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath);

	void trigger_calculate_logduration(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath,int radarposition);
	void trigger_print_toolruntime_toreport(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition);


	//notufy methods to call trigger methods
	void notify_datacollectors_datafactory_objects();
	void notify_plotadapter_plotfactory_objects();
	void notify_HDF_dbgeneration();

	void set_hdf_group_node_generator(std::shared_ptr<HDF_GroupNodeGenerator> &sptr_hdf_groupnode_generator_arg);
	std::shared_ptr<HDF_GroupNodeGenerator> &get_hdf_group_node_generator();

	std::shared_ptr<IDataCollect> sptr_datacollector;

	//pointers to interface of adapters
	std::shared_ptr<IPlotAdapter> sptr_plotadapter;
	std::shared_ptr<IHDF_DBAdapters> sptr_dbadapters;

	//pointers to factory classes
	std::shared_ptr<DataFactory> sptr_datafactory;
	std::shared_ptr<PlotFactory> sptr_plotfactory;
	std::shared_ptr<HDF_DBFactory> sptr_hdf_db_factory;

	//group node generator
	std::shared_ptr<HDF_GroupNodeGenerator> sptr_hdf_groupnode_generator;


	std::shared_ptr<PlotParameter[]> &sptr_plotparam;

	std::map<int, IDataCollect*> map_datacollector_details;
	std::map<int, IPlotAdapter*> map_plotadapter_details;
	std::map<int, IHDF_DBAdapters*> map_hdf_db_adapter_details;

	std::map<int, bool> map_registered_CustID;
	std::set<int> set_radar_positions;
	std::set<int> set_customerID;

	std::string vehiclefilename;
	std::string resimfilename;
	std::string html_runtime_min, html_runtime_sec;
	std::string Log1_duration, Log2_duration;
	std::string tool_runtime_sec;
	std::string tool_runtime_min;
	std::string tool_runtime_func_inputtime;

	int previous_customerID, current_customerID;

	bool btrackerplot;
	bool bdetectionplot;
	bool bscanindex_scaling;
	bool bcustomerCheck_onetime;
	bool iscustomer1_ID_registered, iscustomer2_ID_registered;
	
};
#endif
