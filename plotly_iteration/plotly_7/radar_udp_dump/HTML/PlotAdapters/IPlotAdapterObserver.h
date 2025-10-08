#ifndef IPLOT_ADAPTER_OBSERVER
#define IPLOT_ADAPTER_OBSERVER

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../SharedData/SharedPlotData.h"
#include <iostream>
#include<memory>
#include<set>
class IPlotAdapterObserver
{
	

public:
	IPlotAdapterObserver() = default;
	virtual void trigger_plot_generation(std::set<int> radarpositions, const char* f_name, const char* pltfolder, int filecount, std::string reportpath) = 0;
	virtual ~IPlotAdapterObserver() {
		//std::cout << "~IDataCollect()"; 
	}


};
#endif