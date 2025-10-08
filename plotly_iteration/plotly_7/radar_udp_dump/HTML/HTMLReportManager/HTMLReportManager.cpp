#include "HTMLReportManager.h"


HtmlReportManager::HtmlReportManager(std::shared_ptr<PlotParameter[]>& plotparam)
	:sptr_datacollector(nullptr),
	sptr_plotadapter(nullptr),
	sptr_datafactory(std::shared_ptr<DataFactory>(new DataFactory)),
	sptr_plotfactory(std::shared_ptr<PlotFactory>(new PlotFactory)),
	sptr_hdf_db_factory(std::shared_ptr<HDF_DBFactory>(new HDF_DBFactory)),
	sptr_plotparam(plotparam),
	map_datacollector_details{},
	map_plotadapter_details{},
	map_registered_CustID{},
	set_radar_positions{},
	set_customerID{},
	previous_customerID{0},
	current_customerID{0},
	btrackerplot{false},
	bdetectionplot{false},
	bscanindex_scaling{false},
	bcustomerCheck_onetime{false},
	iscustomer1_ID_registered{false},
	iscustomer2_ID_registered{false}
{
	
	//std::cout << "\nHtmlReportManager::HtmlReportManager()";
	//btrackerplot = false;
	//bdetectionplot = false;
	//bscanindex_scaling = false;
	//previous_customerID = 0;
	//current_customerID = 0;
	//bcustomerCheck_onetime = false;
	//iscustomer1_ID_registered=iscustomer2_ID_registered=false;
	//sptr_datacollector = nullptr;
	//sptr_plotadapter = nullptr;
	//sptr_datafactory = std::shared_ptr<DataFactory>(new DataFactory);
	//sptr_plotfactory = std::shared_ptr<PlotFactory>(new PlotFactory);

}

HtmlReportManager::~HtmlReportManager()
{
	//std::cout << "\nHtmlReportManager::~HtmlReportManager()";
}

void HtmlReportManager::register_datacollector(int customerID, std::shared_ptr<IDataCollect>& sptr)
{
	//std::cout << "\nHtmlReportManager::register_datacollector customerID "
	//	      << customerID << " #pointer address  "<<sptr.get() ;

	map_datacollector_details.insert(std::pair<int, IDataCollect*>(customerID, sptr.get()));
}

void HtmlReportManager::register_plotadaptes(int plotytpe, std::shared_ptr<IPlotAdapter>& sptr)
{
	/*std::cout << "\nHtmlReportManager::register_plotadaptes "
		      << " plotytpe " << plotytpe << " #pointer address"<<sptr.get();*/

	map_plotadapter_details.insert(std::pair<int, IPlotAdapter*>(plotytpe, sptr.get()));
}

void HtmlReportManager::register_dbadaptes(int plotytpe, std::shared_ptr<IHDF_DBAdapters>& sptr)
{
	map_hdf_db_adapter_details.insert(std::pair<int,IHDF_DBAdapters*>(plotytpe, sptr.get()));
}

void HtmlReportManager::clear_datacollector_registry()
{
	//map_datacollector_details.clear();
}

void HtmlReportManager::dispatch_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{

	//std::cout << "\n HtmlReportManager::dispatch_parsedmf4_data";
	current_customerID = int(custId);

	if (current_customerID == 87)
	{
		//std::cout << "\n### dispatch_parsedmf4_data current_customerID == 87";
	}
	register_customer(current_customerID);
	

	for (auto datacollectors: map_datacollector_details)
	{
		if (datacollectors.first == int(custId))
		{			
			datacollectors.second->collect_parsedmf4_data(fileCount,LogFname, g_pIRadarStrm,stream_num,Radar_Posn, custId,scanindex, platform,IsRECU);
			set_radar_positions.insert(int(Radar_Posn));
			set_customerID.insert(int(custId));
		}
	}

}

void HtmlReportManager::register_customer(int currentcustomerID)
{
	//std::cout << "\n HtmlReportManager::register_customer currentcustomerID"<< currentcustomerID;
	//std::cout << "\n size of map_registered_CustID " << map_registered_CustID.size();
	for (auto& reg_cusID : map_registered_CustID)
		{	
			if (reg_cusID.first == currentcustomerID)				
			{		
				if (reg_cusID.second == false)
				{
					reg_cusID.second = true;
					
					sptr_datacollector = sptr_datafactory->create_data_factory(currentcustomerID, sptr_plotparam);

					//adapters will have address of concrete factories
					sptr_plotadapter   = sptr_plotfactory->create_plot_factory(1, sptr_plotparam);
					//sptr_dbadapters = sptr_hdf_db_factory->create_db_factory(1, sptr_plotparam);

					register_datacollector(currentcustomerID, sptr_datacollector);
					register_plotadaptes(1, sptr_plotadapter);

					//register_dbadaptes(1, sptr_dbadapters);
			    }
				else
				{
					register_datacollector(currentcustomerID, sptr_datacollector);
					register_plotadaptes(1, sptr_plotadapter);
				}
				
			}
		}
	
}

void HtmlReportManager::register_customer_id_toReportManager(int currentcustomerID, bool bvistied)
{
	//std::cout << "\n register_customer_id_toReportManager";
	map_registered_CustID[currentcustomerID] = bvistied;
	//std::cout << "\n map_registered_CustID size " << map_registered_CustID.size();

	
}

void HtmlReportManager::trigger_plotgeneration(std::vector<int> vec_custID,const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{
	//std::cout << "\n HtmlReportManager::trigger_plotgeneration";
	for (auto& custID : vec_custID)
	{
		//std::cout << "\n HtmlReportManager::trigger_plotgeneration custID "<< custID;
		//std::cout << "\n HtmlReportManager::trigger_plotgeneration custID map_datacollector_details" << map_datacollector_details.size();
	
		for (auto plotadapters : map_plotadapter_details)
		{		
				//std::cout << "\n HtmlReportManager::trigger_plotgeneration datacollectors.first == custID" << custID;
				plotadapters.second->trigger_plot_generation(set_radar_positions,f_name, pltfolder, filecount, reportpath);
		
		}
	}
}

void HtmlReportManager::trigger_dbgeneration(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{

	for (auto& custID : vec_custID)
	{
		//std::cout << "\n HtmlReportManager::trigger_plotgeneration custID "<< custID;
		//std::cout << "\n HtmlReportManager::trigger_plotgeneration custID map_datacollector_details" << map_datacollector_details.size();

		for (auto hdf_db_adpters : map_hdf_db_adapter_details)
		{
			//std::cout << "\n HtmlReportManager::trigger_plotgeneration datacollectors.first == custID" << custID;
			//hdf_db_adpters.second->trigger_plot_generation(set_radar_positions, f_name, pltfolder, filecount, reportpath);
			hdf_db_adpters.second->update_db(set_radar_positions);

		}
	}
}

void HtmlReportManager::trigger_calculate_logduration(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{

	for (auto& custID : vec_custID)
	{
		//std::cout << "\ntrigger_calculate_logduration";
		for (auto datacollectors : map_datacollector_details)
		{
			if (datacollectors.first == custID)
			{
				datacollectors.second->calculate_Logduration(f_name, pltfolder, filecount, reportpath, radarposition);
			}
		}
	}
}

void HtmlReportManager::trigger_print_toolruntime_toreport(std::vector<int> vec_custID, const char* f_name, const char* pltfolder, int filecount, std::string reportpath,int radarposition)
{


	for (auto& custID : vec_custID)
	{
		for (auto datacollectors : map_datacollector_details) 
		{
			if (datacollectors.first == custID)
			{
				datacollectors.second->calculate_tool_runtime(f_name, pltfolder, filecount, reportpath, radarposition);
			}
		}
	}
	
}

void HtmlReportManager::notify_datacollectors_datafactory_objects()
{

	for (auto &datacollectors: map_datacollector_details)
	{
		int customerID = datacollectors.first;
		
	}
}

void HtmlReportManager::notify_plotadapter_plotfactory_objects()
{

}

void HtmlReportManager::notify_HDF_dbgeneration()
{
}

void HtmlReportManager::set_hdf_group_node_generator(std::shared_ptr<HDF_GroupNodeGenerator>& sptr_hdf_groupnode_generator_arg)
{
	sptr_hdf_groupnode_generator = sptr_hdf_groupnode_generator_arg;

	for (auto hdf_db_adpters : map_hdf_db_adapter_details)
	{
		
		hdf_db_adpters.second->set_db_adapters(sptr_hdf_groupnode_generator_arg);

	}
}

std::shared_ptr<HDF_GroupNodeGenerator>& HtmlReportManager::get_hdf_group_node_generator()
{
	return sptr_hdf_groupnode_generator;
}

