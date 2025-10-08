#ifndef HDFDB_FACTORY
#define HDFDB_FACTORY

#include<map>
#include<iostream>
#include<memory>
#include<set>
#include<vector>
#include<memory>

#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../SharedData/SharedPlotData.h"
#include "../DataFactory/IDBFactory.h"


class HDF_DBFactory :public IDBFactory
{


public:

	HDF_DBFactory();
	HDF_DBFactory(HDF_DBFactory&) = delete;
	HDF_DBFactory& operator=(HDF_DBFactory&) = delete;
	virtual ~HDF_DBFactory();


	virtual std::shared_ptr<IHDF_DBAdapters> create_db_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam)override;

	std::shared_ptr<IHDF_DBAdapters> sptr_plot;

};
#endif