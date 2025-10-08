#ifndef HDF_DBNODE_GENERATOR
#define HDF_DBNODE_GENERATOR

#include<iostream>
#include<vector>
#include<map>

class HDF_GroupNodeGenerator
{

private:

	int inputfilelist_totalcount{}, outputfilelist_totalcount{};
	std::vector<int> vec_sensor_list{};
	std::vector<std::string> vec_detection_node_properties{ "/g_range","/g_rangerate","/g_azimuth","/g_elevation" };

	std::map<int, std::string> map_sensornumber_sensorname;


public:

	std::map<int, std::map<int, std::vector<std::string>>> mapofmap_input_group{};
	std::map<int, std::map<int, std::vector<std::string>>> mapofmap_output_group{};

	HDF_GroupNodeGenerator(int,int);
	~HDF_GroupNodeGenerator();

	void set_sensorlist(std::vector<int> vec_sensor_list_arg);
	void createInNodeGroup_detection();
	void createOutNodeGroup_detection();

	std::string get_input_groups(int infilecount, int sensor, int detectionproperties);
	std::string get_output_groups(int outfilecount, int sensor, int detectionproperties);
};
#endif