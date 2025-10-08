#include "HDF_DBNodeGenerator.h"
#include<string>
#include<iostream>
HDF_GroupNodeGenerator::HDF_GroupNodeGenerator(int infilelist_count, int outfilelist_count) :inputfilelist_totalcount(infilelist_count),
                                                                                             outputfilelist_totalcount(outfilelist_count)
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::HDF_GroupNodeGenerator ";

	map_sensornumber_sensorname[0] = "RL";
	map_sensornumber_sensorname[1] = "RR";
	map_sensornumber_sensorname[2] = "FL";
	map_sensornumber_sensorname[3] = "FR";
	map_sensornumber_sensorname[5] = "FC";
}

HDF_GroupNodeGenerator::~HDF_GroupNodeGenerator()
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::~HDF_GroupNodeGenerator() ";
}

void HDF_GroupNodeGenerator::set_sensorlist(std::vector<int> vec_sensor_list_arg)
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::set_sensorlist ";
	std::cout << std::endl << " vec_sensor_list_arg  "<< vec_sensor_list_arg.size()<<std::endl;

	
	vec_sensor_list = vec_sensor_list_arg;

	for (auto val: vec_sensor_list)
	{
		std::cout << val;
	}
}

void HDF_GroupNodeGenerator::createInNodeGroup_detection()
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::createInNodeGroup_detection ";

	for (int infilecount = 0; infilecount < inputfilelist_totalcount; infilecount++)
	{
		for (auto sensor_index : vec_sensor_list)
		{
			std::string sensorname = map_sensornumber_sensorname[sensor_index];
			std::vector<std::string> vec_ingroup_name;

			for (auto detection_prp : vec_detection_node_properties)
			{
				std::string group_path{};
				std::string filecnt = std::to_string(infilecount);
				group_path = "ingroup/ing_det/file" + filecnt +"/"+ sensorname + detection_prp;
				vec_ingroup_name.push_back(group_path);

			}

			mapofmap_input_group[infilecount][sensor_index] = vec_ingroup_name;
			vec_ingroup_name.clear();
		}
	}

}

void HDF_GroupNodeGenerator::createOutNodeGroup_detection()
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::createOutNodeGroup_detection ";

	for (int outfilecount = 0; outfilecount < outputfilelist_totalcount; outfilecount++)
	{
		for (auto sensor_index : vec_sensor_list)
		{
			std::string sensorname = map_sensornumber_sensorname[sensor_index];
			std::vector<std::string> vec_outgroup_name;

			for (auto detection_prp : vec_detection_node_properties)
			{
				std::string group_path{};
				std::string filecnt = std::to_string(outfilecount);
				group_path = "outgroup/outg_det/file" + filecnt +"/"+ sensorname + detection_prp;
				vec_outgroup_name.push_back(group_path);

			}

			mapofmap_output_group[outfilecount][sensor_index] = vec_outgroup_name;
			vec_outgroup_name.clear();
		}

	}
}

std::string HDF_GroupNodeGenerator::get_input_groups(int infilecount, int sensor, int detectionproperties)
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::get_input_groups "<< infilecount<< sensor<< detectionproperties;

	auto itr = mapofmap_input_group.find(infilecount);
	if ( itr != mapofmap_input_group.end())
	{
     	auto itr2 = itr->second.find(sensor);
		if (itr2 != itr->second.end())
		{
			return itr2->second.at(detectionproperties);
		}

	}

	
}

std::string HDF_GroupNodeGenerator::get_output_groups(int outfilecount, int sensor, int detectionproperties)
{
	std::cout << std::endl << " HDF_GroupNodeGenerator::get_output_groups " << outfilecount << sensor << detectionproperties;

	auto itr = mapofmap_output_group.find(outfilecount);
	if (itr != mapofmap_output_group.end())
	{
		auto itr2 = itr->second.find(sensor);
		if (itr2 != itr->second.end())
		{
			return itr2->second.at(detectionproperties);
		}

	}
}
