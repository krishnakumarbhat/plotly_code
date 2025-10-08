#ifndef PLOT_FACTORY
#define PLOT_FACTORY

#include<map>
#include<iostream>
#include<memory>
#include<set>
#include<vector>
#include<memory>
#include "../DataFactory/IPlotFactory.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../DataFactory/DataFactoryInterface.h"
#include "../SharedData/SharedPlotData.h"


class PlotFactory:public IPlotFactory
{
	

public:

	PlotFactory();
	PlotFactory(PlotFactory&) = delete;
	PlotFactory& operator=(PlotFactory&) = delete;
	virtual ~PlotFactory();
	

	virtual std::shared_ptr<IPlotAdapter> create_plot_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam)override;

	std::shared_ptr<IPlotAdapter> sptr_plot;

};
#endif