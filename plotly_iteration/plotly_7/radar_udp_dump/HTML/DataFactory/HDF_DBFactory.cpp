#include "../DataFactory/HDF_DBFactory.h"
#include "../DBAdapters/HDF_DBAdapters.h"
HDF_DBFactory::HDF_DBFactory()
{
}

HDF_DBFactory::~HDF_DBFactory()
{
}

std::shared_ptr<IHDF_DBAdapters> HDF_DBFactory::create_db_factory(int plot_type, std::shared_ptr<PlotParameter[]>& sptr_plotparam)
{
	
	if (plot_type == 1)
	{
		if (sptr_plot == nullptr)
		{
			sptr_plot = std::shared_ptr<IHDF_DBAdapters>(new HDF_Adapters(sptr_plotparam));
						
		}

	}
	return sptr_plot;
}
