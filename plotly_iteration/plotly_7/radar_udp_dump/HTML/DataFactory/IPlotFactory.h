#ifndef PLOTFACTORY_INTERFACE
#define PLOTFACTORY_INTERFACE

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../SharedData/SharedPlotData.h"
#include "../PlotAdapters/IPlotAdapter.h"
#include <iostream>
#include<memory>
class IPlotFactory
{
	

public:
	IPlotFactory() = default;
	virtual std::shared_ptr<IPlotAdapter> create_plot_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam) = 0;
	virtual ~IPlotFactory() {
		//std::cout << "~IPlotFactory()"; 
	}

};
#endif