#ifndef HDF_DBADAPTERS
#define HDF_DBADAPTERS

#include <memory>
#include <set>

#include "IHDF_DBAdapters.h"
#include "../DBGenerators/HDF_DBGenerators.h"
#include "../SharedData/SharedPlotData.h"
#include "../DBUtils/HDF_DBNodeGenerator.h"

class HDF_Adapters:public IHDF_DBAdapters
{
public:
	HDF_Adapters(std::shared_ptr<PlotParameter[]>& sptr_plotparam);
	HDF_Adapters(HDF_Adapters&) = delete;
	HDF_Adapters& operator=(HDF_Adapters&) = delete;


	std::string string_replace(std::string input, std::string replace_word, std::string replace_by);


	virtual void update_db(std::set<int> sensorlist)override;
	virtual void set_db_adapters(std::shared_ptr<HDF_GroupNodeGenerator>& hdf_grp_node) override;
	std::shared_ptr<PlotParameter[]>& sptr_plotparam_plotadapter;


	virtual ~HDF_Adapters() {}

	std::shared_ptr<HDF_GroupNodeGenerator> sptr_hdf_groupnode_generator;

};
#endif
