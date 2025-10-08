#include "PlotFactory.h"
#include "../PlotAdapters/MatplotAdapter.h"
PlotFactory::PlotFactory():sptr_plot(nullptr)
{
	
	//std::cout << "\nPlotFactory::PlotFactory";
	
}

PlotFactory::~PlotFactory()
{
	//std::cout << "\nPlotFactory::~PlotFactory()";
}


std::shared_ptr<IPlotAdapter> PlotFactory::create_plot_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam)
{
	//std::cout << "\nDataFactory::create_data_factory "<< plot_type;
	if (plot_type==1)
	{
		if (sptr_plot ==nullptr)
		{			
			sptr_plot = std::shared_ptr<IPlotAdapter>(new MatplotAdapter(sptr_plotparam));
			//std::cout << sptr_plot.get()<<"\n";			
		}
			 
	}
     return sptr_plot;
}












