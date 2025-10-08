#include "JsonInputParser.h"

#include "../../../CommonFiles/helper/genutil.h"
#include<iostream>
#include <filesystem>
namespace pt = boost::property_tree;
JsonInputParser::JsonInputParser()
{
}

JsonInputParser::~JsonInputParser()
{
}

/*---------------------------------------------------------------------------------------
Function name : getInstance

Description   : It returns instance of JsonInputParser

Return type   : JsonInputParser (class type)

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
JsonInputParser& JsonInputParser::getInstance()
{
	static JsonInputParser instance;
	return instance;
}

/*---------------------------------------------------------------------------------------
Function name : read_HTML_Arguments

arguments     :
Description   : reads inputs from command prompt 

Return type   : int

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
int JsonInputParser::read_HTML_Arguments(int argc, char* argv[])
{
	std::cout<<std::endl << "\n Reading HTML_Arguments" << std::endl;

	std::filesystem::directory_entry report_pathhtml{reportpath };
	if (report_pathhtml.exists())
	{
		//<**Added for Aptiv Header to update in the console End (GPC-1449)**>

		std::cout << std::endl << "Report path : " << report_pathhtml << " exists" << std::endl;

		//<**Added for Aptiv Header to update in the console End (GPC-1449)**>
		
		
	}
	else
	{
		std::cout << std::endl << "---------------------------------------------------" << std::endl;
		std::cout << std::endl << " Arguments InComplete" << std::endl;
		std::cout << std::endl << " Expected Arguments (3) : ResimHTMLReport.exe HTMLConfig.xml HTMLInputs.JSON OutputPath" << std::endl;
		std::cout << std::endl << " Arg[0]--> HTMLConfig.xml " << std::endl;
		std::cout << std::endl << " Arg[1]--> HTMLInputs.JSON " << std::endl;
		std::cout << std::endl << " Arg[2]--> OutputPath " << std::endl;
		std::cout << std::endl << "----------------------------------------------------" << std::endl;

		std::cout << std::endl << " ----------------------------------------------------" << std::endl;
		std::cout << std::endl << "Report path does not exists please check the path " << std::endl;
		std::cout << std::endl << " ----------------------------------------------------" << std::endl;
		return -1;
	}

	int count = 2;

	if (argc != 4)
	{
		std::cout << std::endl << "---------------------------------------------------" << std::endl;
		std::cout << std::endl << " Arguments InComplete" << std::endl;
		std::cout << std::endl << " Expected Arguments (3) : ResimHTMLReport.exe HTMLConfig.xml HTMLInputs.JSON OutputPath" << std::endl;
		std::cout << std::endl << " Arg[0]--> HTMLConfig.xml " << std::endl;
		std::cout << std::endl << " Arg[1]--> HTMLInputs.JSON " << std::endl;
		std::cout << std::endl << " Arg[2]--> OutputPath " << std::endl;
		std::cout << std::endl << "----------------------------------------------------" << std::endl;
		return -1;

	}
	if (argc == 4)
	{
		std::cout << "\nTwo Aruguments" << endl;

		if (argc > count)
		{
			strcpy(inp_file, argv[count]);
			std::cout << "\n inp_file" << inp_file << endl;
		}

		std::string htmlFilePath = inp_file;
		std::replace(htmlFilePath.begin(), htmlFilePath.end(), '\\', '/');
		if (htmlFilePath.back() == '/') {
			htmlFilePath.pop_back();
		}
		
		strncpy(inp_file, htmlFilePath.c_str(), (htmlFilePath.length() + 1));
		
		
		pt::ptree root;
		pt::read_json(inp_file, root);	// Load the json file in this ptree

		//std::string scanindexScaling = root.get<std::string>("scanindexScaling");
		/*reportpath = root.get<std::string>("REPORT_PATH");
		std::replace(reportpath.begin(), reportpath.end(), '\\', '/');
		std::cout << "\n reportpath " << reportpath;*/




		if (1)
		{
			veh_sil = true;
			std::cout << "\n\t\t Vehicle Vs SIL Report";
			std::cout << "\n------------------------------------------------------------------------------------";

			for (pt::ptree::value_type& veh_mf4file : root.get_child("INPUT_MF4"))
			{
				std::string filepath = veh_mf4file.second.data();
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
				mvehicle_mf4File.push_back(filepath);
			}

			for (pt::ptree::value_type& resim_mf4file : root.get_child("OUTPUT_MF4"))
			{
				std::string filepath = resim_mf4file.second.data();
				std::replace(filepath.begin(), filepath.end(), '\\', '/');
				mresim_mf4File.push_back(filepath);
			}

			/*if (scanindexScaling == "1")
			{

				bscanindex_scaling = 1;
				cout << "\n scanindexScaling";
			}
			else
			{
				bscanindex_scaling = 0;
			}*/
		}

		return 0;

	
	}


	



	if (mvehicle_mf4File.size() != mresim_mf4File.size())
	{
		std::cout << "Vehicle Files and Resim File counts are not matching. Provide equal and proper inputs to proceed" << std::endl;
		return -1;
	}

}

/*---------------------------------------------------------------------------------------
Function name : read_HTML_JSON_data

arguments     :
Description   : reads HTML JSON and collect input and
                resimulated file list

Return type   : void

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
void JsonInputParser::read_HTML_JSON_data()
{


}

/*---------------------------------------------------------------------------------------
Function name : get_html_inputs_mf4_count

arguments     : none
Description   : returns the total input files provided from html generation

Return type   : int

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
int JsonInputParser::get_html_inputs_mf4_count()
{
	return 0;
}

/*---------------------------------------------------------------------------------------
Function name : get_html_output_mf4_count

arguments     : none
Description   : returns the total output files provided from html generation

Return type   : int

Authors :
Intial version 1.0 Bharanidharan Subramani
-----------------------------------------------------------------------------------------*/
int JsonInputParser::get_html_output_mf4_count()
{
	return 0;
}

int JsonInputParser::CheckFileExists(const char* inp_file)
{
	int ret = 0;

	/*if (_access(inp_file, F_OK) != 0)
	{
		printf("[WARNING]: File or directory Doesn't Exist. (%s) \n", inp_file);
		return -1;
	}*/
	if (file_exist(inp_file, 0) == -1)
	{
		cout << "\n[WARNING]: File doesn't have Read permission";
		ret = -1;
		//printf("\n[WARNING]: File doesn't have Read permission. (%s) \n", inp_file);

	}

	return ret;
}
