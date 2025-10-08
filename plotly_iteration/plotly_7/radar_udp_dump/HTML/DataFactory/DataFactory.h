#ifndef DATA_FACTORY
#define DATA_FACTORY

#include<map>
#include<iostream>
#include<memory>
#include<set>
#include<vector>
#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../DataFactory/DataFactoryInterface.h"
#include "../SharedData/SharedPlotData.h"
#include<memory>
class DataFactory:IDataFactory
{
	

public:

	DataFactory();
	DataFactory(DataFactory&) = delete;
	DataFactory& operator=(DataFactory&) = delete;
	virtual ~DataFactory();


	virtual std::shared_ptr<IDataCollect> create_data_factory(int customerID, std::shared_ptr<PlotParameter[]>& sptr_plotparam)override;
	//shared pointers for customer
	std::shared_ptr<IDataCollect> sptr_customer;
};
#endif