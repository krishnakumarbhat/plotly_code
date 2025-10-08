#ifndef DATAFACTORY_INTERFACE
#define DATAFACTORY_INTERFACE

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../HTMLInterface/HTMLInterface.h"
#include "../SharedData/SharedPlotData.h"

#include <iostream>
#include<memory>
class IDataFactory
{

public:
	IDataFactory() = default;
	virtual std::shared_ptr<IDataCollect> create_data_factory(int customerID, std::shared_ptr<PlotParameter[]>& sptr_plotparam) = 0;
	virtual ~IDataFactory() { 
		//std::cout << "~IDataCollect()"; 
	}
};
#endif