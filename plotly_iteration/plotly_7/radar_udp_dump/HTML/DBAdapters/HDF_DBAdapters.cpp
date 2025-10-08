#include "HDF_DBAdapters.h"

HDF_Adapters::HDF_Adapters(std::shared_ptr<PlotParameter[]>& sptr_plotparam):sptr_plotparam_plotadapter(sptr_plotparam)
{

}

std::string HDF_Adapters::string_replace(std::string input, std::string replace_word, std::string replace_by)
{
	

	// Find the first occurrence of the substring 
	size_t pos = input.find(replace_word);

	// Iterate through the string and replace all 
	// occurrences 
	while (pos != string::npos) {
		// Replace the substring with the specified string 
		input.replace(pos, replace_word.size(), replace_by);

		// Find the next occurrence of the substring 
		pos = input.find(replace_word,
			pos + replace_by.size());
	}

	return input;
}

void HDF_Adapters::update_db(std::set<int> sensorlist)
{
	//total file count : query value from HTML report Manager

	HDF_DBGenerator& hdf_db = HDF_DBGenerator::get_instance();
	for (int filecount=0;filecount < 1;filecount++)
	{
		for (auto sensor:sensorlist)
		{
			


			std::cout << std::endl << "HDF_Adapters::update_db Sensor " << sensor;

			std::string grp_name_input{}, grp_name_output{}, grp_name_attribute{};
			std::vector<std::vector<double>> twodataset[2] = {};
			std::vector<double> det_property_data[2] = {}, det_property_si[2] = {};
			std::vector<std::string> node_attributes{};


		

			//-----------------------------------------Range scatter data--------------------------------------------------------//
			
			//get detection properties range values in two D vectors
			det_property_data[0] = (sptr_plotparam_plotadapter.get() + sensor)->range[0];
			det_property_si[0] = (sptr_plotparam_plotadapter.get() + sensor)->si_veh;
			
			det_property_data[1] = (sptr_plotparam_plotadapter.get() + sensor)->range[1];
			det_property_si[1] = (sptr_plotparam_plotadapter.get() + sensor)->si_resim;
				
			twodataset[0].push_back(det_property_data[0]);
			twodataset[0].push_back(det_property_si[0]);

			twodataset[1].push_back(det_property_data[1]);
			twodataset[1].push_back(det_property_si[1]);
		


			//get input and output group node information from DBNodeGenerator
			grp_name_input = sptr_hdf_groupnode_generator->get_input_groups(filecount, sensor, 0);
			std::cout << std::endl <<"grp_name_input "<< grp_name_input;
			grp_name_output = sptr_hdf_groupnode_generator->get_output_groups(filecount, sensor, 0);

			std::cout << std::endl << "grp_name_output " << grp_name_output;

			//frame node attribute :Graph title and xaxis and yaxis info  


			string plottitle1;
			std::map<int, string> titleinfomap;
			titleinfomap[0] = "RL Range";
			titleinfomap[1] = "RR Range";
			titleinfomap[2] = "FR Range";
			titleinfomap[3] = "FL Range";
			titleinfomap[5] = "FC Range";
			for (auto val : titleinfomap)
			{
				if (val.first == sensor)
				{
					plottitle1 = val.second;
				}  
			}

			node_attributes.push_back(plottitle1);
			node_attributes.push_back("range");
			node_attributes.push_back("scanindex");

			//grp_name_attribute = grp_name_input + "/attribute";

			grp_name_attribute=string_replace(grp_name_input, "/g_range", "/g_range_attribute");
			
		
            //attach range data of file1 to DB
			hdf_db.attach_dataset_togroup(grp_name_input, twodataset[0]);
			hdf_db.attach_dataset_attribute_togroup(grp_name_attribute, node_attributes);
		

			std::cout << std::endl << "after removal grp_name_attribute " << grp_name_attribute;
			std::cout << std::endl << "after removal grp_name_input " << grp_name_input;
		

			//attach range data of file2 to DB
			hdf_db.attach_dataset_togroup(grp_name_output, twodataset[1]);
			//hdf_db.attach_dataset_attribute_togroup(grp_name_output, node_attributes);
		}
	}

}

void HDF_Adapters::set_db_adapters(std::shared_ptr<HDF_GroupNodeGenerator>& hdf_grp_node)
{
	sptr_hdf_groupnode_generator = hdf_grp_node;
}
