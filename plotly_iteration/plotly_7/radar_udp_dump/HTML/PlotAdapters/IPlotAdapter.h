#ifndef IPLOT_ADAPTER
#define IPLOT_ADAPTER

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../SharedData/SharedPlotData.h"
#include <iostream>
#include<memory>
class IPlotAdapter
{
	

public:
	IPlotAdapter() = default;
	virtual void generate_plot() = 0;
	virtual void trigger_plot_generation(std::set<int> radarpositions, const char* f_name, const char* pltfolder, int filecount, std::string reportpath) = 0;

	virtual ~IPlotAdapter() {
		//std::cout << "~IPlotAdapter()"; 
	}


};
#endif