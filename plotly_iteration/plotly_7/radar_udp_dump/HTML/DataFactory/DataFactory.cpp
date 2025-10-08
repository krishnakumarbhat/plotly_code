#include "DataFactory.h"
#include<algorithm>
#include "../InputParser/JsonInputParser.h"
#include "../DataCollector/STLA_SCALE1.h"
#include "../DataCollector/HONDA_SRR6P.h"
#include "../DataCollector/GPO_GEN7.h"
#include "../DataCollector/TRATON.h"
#include "../DataCollector/DC.h"


DataFactory::DataFactory():sptr_customer(nullptr)
{
	
	//std::cout << "\nDataFactory::DataFactory()";
	//sptr_customer = nullptr;
}

DataFactory::~DataFactory()
{
	//std::cout << "\nDataFactory::~DataFactory()";
}



std::shared_ptr<IDataCollect> DataFactory::create_data_factory(int customerID, std::shared_ptr<PlotParameter[]>& sptr_plotparam)
{

	JsonInputParser& json = JsonInputParser::getInstance();
	
	

	


	if (customerID == 84 || customerID == 85|| customerID == 87 || customerID == 128)
	{
		if (sptr_customer == nullptr)
		{
			//std::cout << "\nDataFactory::create_data_factory uptr_customer " << customerID;			
			sptr_customer = std::make_shared<StlaScale1DataCollector>(sptr_plotparam);
			//std::cout << sptr_customer.get()<<"\n";

		}

	}

	// 2nd CS Implemented for HONDA taking cust_id 83 (0x53)from HONDA_GEN5

	if (customerID == 83)
	{
		if (sptr_customer == nullptr)
		{
			sptr_customer = std::make_shared<HondaSRR6pDataCollector>(sptr_plotparam);
		
		}
		
	}

	if (customerID == 129)
	{
		if (sptr_customer == nullptr)
		{
			sptr_customer = std::make_shared<GpoGen7DataCollector>(sptr_plotparam);

		}

	}

	// 2nd CS Implemented for TRATON taking cust_id 11 (0x0b)from SCANIA_MAN

	if (customerID == 11)
	{
		if (sptr_customer == nullptr)
		{
			sptr_customer = std::make_shared<TratonDataCollector>(sptr_plotparam);

		}

	}

	if (customerID == 161)
	{
		//std::cout << "\nDataFactory::DataFactory() customerID == 161";
		if (sptr_customer == nullptr)
		{
			sptr_customer = std::make_shared<DCDataCollector>(sptr_plotparam);

		}

	}


     return sptr_customer;
}















