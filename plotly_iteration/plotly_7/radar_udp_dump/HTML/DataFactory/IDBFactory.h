#ifndef DBFACTORY_INTERFACE
#define DBFACTORY_INTERFACE

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../SharedData/SharedPlotData.h"
#include "../PlotAdapters/IPlotAdapter.h"
#include "../DBAdapters/IHDF_DBAdapters.h"
#include <iostream>
#include<memory>
class IDBFactory
{


public:
	IDBFactory() = default;
	virtual std::shared_ptr<IHDF_DBAdapters> create_db_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam) = 0;
	virtual ~IDBFactory() {
		//std::cout << "~IPlotFactory()"; 
	}

};
#endif