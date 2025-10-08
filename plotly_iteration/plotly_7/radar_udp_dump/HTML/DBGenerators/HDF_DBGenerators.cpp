#include "HDF_DBGenerators.h"

//using namespace HighFive;

//File hdf_db("HDF_DB.h5",File::Truncate);
//hdf db file should be made global to the project
//to avoid undefined behaviour

HDF_DBGenerator::HDF_DBGenerator():db_initialization(true)
{

}

HDF_DBGenerator& HDF_DBGenerator::get_instance()
{
	static HDF_DBGenerator instance;
	return instance;
}

void HDF_DBGenerator::create_HDF_DB(std::string hdf_db_filename)
{
	hdf_db_name = hdf_db_filename;
}

void HDF_DBGenerator::attach_dataset_togroup(std::string group, std::vector<std::vector<double>> twodim_dataset)
{
	//std::cout << std::endl << " HDF_DBGenerator::attach_dataset_togroup ";

	//hdf_db.createDataSet(group, twodim_dataset);

}

void HDF_DBGenerator::attach_dataset_attribute_togroup(std::string group, std::vector<std::string> dataset_attribute)
{
	//std::cout << std::endl << " HDF_DBGenerator::attach_dataset_attribute_togroup";
	//hdf_db.createDataSet(group, dataset_attribute);
}

HDF_DBGenerator::~HDF_DBGenerator()
{
}
