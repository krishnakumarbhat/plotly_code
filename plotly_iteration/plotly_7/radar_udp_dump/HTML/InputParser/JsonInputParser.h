#ifndef JSON_INPUT_PARSER
#define JSON_INPUT_PARSER

/******************************************************************************
FileName   : JSONParser.cpp

Author     : Bharanidharan Subramani

Date       : 23 June, 2024

Description: This is responsible for reading HTML JSON inputs.
This class implement using Meyer's Singleton 

HTML JSON File:

INPUTS: where list of input mf4 files are provided
OUTPUTS: where list of output/resimulated mf4 files are provided

*****************************************************************
Version No : Date of update:         Updated by:           Changes:

******************************************************************************/
#include <string>
#include <vector>
#include <set>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../../CommonFiles/helper/genutil.h"


#include "../../CommonFiles/CommonHeaders/Radar_Config.h"
using namespace std;
class JsonInputParser
{

    JsonInputParser();
public:
    JsonInputParser(JsonInputParser&) = delete;
    JsonInputParser& operator=(JsonInputParser&) = delete;
    static JsonInputParser& getInstance();
    ~JsonInputParser();

    //Methods

    int read_HTML_Arguments(int argc, char* argv[]);
    void read_HTML_JSON_data();

    int CheckFileExists(const char* inp_file);

    int get_html_inputs_mf4_count();
    int get_html_output_mf4_count();

    //Members
    std::string reportpath{};
    std::vector<std::string> mvehicle_mf4File;
    std::vector<std::string> mresim_mf4File;
    bool binputfilename_htmlJson;
    
    bool veh_sil;
    bool veh_hil;
    bool sil_hil;

    bool btrackerplot;
    bool bdetectionplot;
    bool bscanindex_scaling;

    std::string vehiclefilename;
    std::string resimfilename;

    std::string html_runtime_min, html_runtime_sec;
    std::string Log1_duration, Log2_duration;
    std::string tool_runtime_sec;
    std::string tool_runtime_min;
    std::string tool_runtime_func_inputtime;

    std::set<int> set_customerID;
    std::map<Customer_T, int> map_custID;
    unsigned short logversion;

    char  inp_file[1024] = { 0 };
    char  out_file[1024] = { 0 };


    char veh_fileName[_MAX_PATH] = { 0 };
    char Resim_fileName[_MAX_PATH] = { 0 };
    char inputVehFileName[_MAX_PATH] = { 0 };
    char inputResimFileName[_MAX_PATH] = { 0 };

    int totalinputpair_count = 0;
    std::vector<std::string> mf4filecount;


};

#endif
