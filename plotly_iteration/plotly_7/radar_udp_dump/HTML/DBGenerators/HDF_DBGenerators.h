#ifndef HDF_DB_GENERATOR
#define HDF_DB_GENERATOR

#include<vector>
#include<string>
//#include<highfive/highfive.hpp>
#include<set>

//using namespace HighFive;

class HDF_DBGenerator
{

	HDF_DBGenerator();
public:
	HDF_DBGenerator(HDF_DBGenerator&) = delete;
	HDF_DBGenerator& operator=(HDF_DBGenerator&) = delete;

	static HDF_DBGenerator &get_instance();

	void create_HDF_DB(std::string hdf_db_filename);
	void attach_dataset_togroup(std::string group, std::vector<std::vector<double>> twodim_dataset);
	void attach_dataset_attribute_togroup(std::string group, std::vector<std::string> dataset_attribute);
	virtual ~HDF_DBGenerator();

	std::string hdf_db_name;
	bool db_initialization;

	
};
#endif