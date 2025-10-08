#ifndef IHDF_ADAPTERS_
#define IHDF_ADAPTERS_

#include<iostream>
#include<memory>
#include<set>
#include "../DBUtils/HDF_DBNodeGenerator.h"
class IHDF_DBAdapters
{
public:
	IHDF_DBAdapters() = default;
	virtual ~IHDF_DBAdapters() {};

	virtual void update_db(std::set<int> sensorlist) = 0;
	virtual void set_db_adapters(std::shared_ptr<HDF_GroupNodeGenerator> &hdf_grp_node) = 0;
};

#endif
