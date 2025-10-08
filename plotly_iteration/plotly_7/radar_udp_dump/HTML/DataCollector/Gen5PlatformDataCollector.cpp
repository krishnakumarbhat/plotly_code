#include "Gen5PlatformDataCollector.h"

#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../../CommonFiles/CommonHeaders/MUDP_Data_Extracter_Config.h"
#include <filesystem>
#include <algorithm>
#include <filesystem>
#ifdef _WIN32
#include <direct.h>// for _mkdir
#endif
#ifdef __GNUC__
#include <sys/stat.h>
#endif

#define AF_DET_MAX_COUNT    (256U) 
#define NUMBER_OF_DETECTIONS (200)

using namespace std;
namespace fs = std::filesystem;
using namespace matplot;
extern RADAR_Plugin_Interface_T* m_radar_plugin;
STLA_Detection_Log_Data_T GPO_Dets_info_t = { 0 };
//STLA_Detection_Log_Data_T STLA_Dets_info = { 0 };

float32_T* array_data_ptrGPO;
auto tracks_fig1 = figure(true);

//Start of GenericFunctions

int comparedataGPO(const void* a, const void* b)
{ 
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptrGPO[value_of_a] < array_data_ptrGPO[value_of_b]) {
        return -1;
    }
    else if (array_data_ptrGPO[value_of_a] == array_data_ptrGPO[value_of_b])
    {

    }
    else
    {
        return (array_data_ptrGPO[value_of_a] > array_data_ptrGPO[value_of_b]);
    }
}

string set_out_html_pathGPO(const char* f_name)
{

    char* path_html;
    std::string tempstring(f_name);

    strcpy((char*)tempstring.c_str(), f_name);

    std::string str(f_name);
    std::size_t found = str.find_last_of("/");


    strcpy((char*)tempstring.c_str(), (str.substr(found + 1)).c_str());
    char* file_pos = strrchr((char*)tempstring.c_str(), '.');
    if (file_pos != NULL)
    {
        *file_pos = '\0'; 
    }
    tempstring.clear();
    strcpy((char*)tempstring.c_str(), (str.substr(0, found)).c_str());
    return tempstring;

}

void Gen5PlatformDataCollector::CheckCollectedDetectionvalues()
{

    for (const auto& [key, value] : ptr_gpo_selfdetection_RL->veh_range_detections[0])
    {
        cout << "---------------" << endl;
        cout << " RL veh SI " << key << endl;
        cout << "---------------" << endl;

        for (auto rangeval : value)
        {
            cout << rangeval << '\t';
        }
    }
    cout << endl;

    cout << "\n*****Resim value detections*******\n";
    for (const auto& [key, value] : ptr_gpo_selfdetection_RL->resim_range_detections[0])
    {
        cout << "---------------" << endl;
        cout << " RL Resim SI " << key << endl;
        cout << "---------------" << endl;

        for (auto rangeval : value)
        {
            cout << rangeval << '\t';
        }
    }
    cout << endl;
}

void Gen5PlatformDataCollector::CollectDetectionDataRL(Self_Detections_Param_T* selfdetecion, int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex)
{

    if (Radar_Posn == 0)
    {
        std::vector<double> vrangeRL_veh,vrangerateRL_veh,vazimuthRL_veh,velevationRL_veh,vsnrRL_veh,vampRL_veh;
        std::vector<double> vrangeRL_resim,vrangerateRL_resim,vazimuthRL_resim,velevationRL_resim,vsnrRL_resim,vampRL_resim;
        
        
        

        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&GPO_Dets_info_t, DETS_COMP);

        /*sort by range*/
        float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
            Self_Dets[i] = GPO_Dets_info_t.target_report[i].range;
        }
        unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
        unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptrGPO = Self_Dets;
        qsort(index, size, sizeof(*index), comparedataGPO);

        //Count valid detection per scan index
        static unsigned32_T Valid = 0;
        for (unsigned32_T i = 0; i < 200; i++)
        {
            double range = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].range);
            if (range != 0)
            {
                Valid++;
            }
        }

        if (fileCount == 0)
        {            
            vRLscanindex.push_back(scanindex);
            ptr_gpo_selfdetection_RL->veh_valid_detectcount[0][scanindex] = Valid;
        }
        if (fileCount == 1)
        {          
            ptr_gpo_selfdetection_RL->resim_valid_detectcount[0][scanindex] = Valid;
        }
            
        for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
            
            if (fileCount == 0)
            {
                double range = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].range);

                if (range != 0)
                {
                    double rangevalue = floor(range * 100) / 100;
                    vrangeRL_veh.push_back(rangevalue);

                    double rangerate= flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].range_rate);
                    double rangeratevalue = floor(rangerate * 100) / 100;
                    vrangerateRL_veh.push_back(rangeratevalue);

                    double azimuth = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].azimuth);
                    double azimuthvalue = floor(azimuth * 100) / 100;
                    vazimuthRL_veh.push_back(azimuthvalue);

                    double elevation = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].elevation);
                    double elevationvalue = floor(elevation * 100) / 100;
                    velevationRL_veh.push_back(elevationvalue);

                    double snr = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].snr);
                    double snrvalue = floor(snr * 100) / 100;
                    vsnrRL_veh.push_back(snrvalue);

                    double amp = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].amp);
                    double ampvalue = floor(amp * 100) / 100;
                    vampRL_veh.push_back(ampvalue);
                  
                }
               
            }
            if (fileCount == 1)
            {
                double range = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].range);
                if (range != 0)
                {
                    double rangevalue = floor(range * 100) / 100;
                    vrangeRL_resim.push_back(rangevalue);

                    double rangerate = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].range_rate);
                    double rangeratevalue = floor(rangerate * 100) / 100;
                    vrangerateRL_resim.push_back(rangeratevalue);

                    double azimuth = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].azimuth);
                    double azimuthvalue = floor(azimuth * 100) / 100;
                    vazimuthRL_resim.push_back(azimuthvalue);

                    double elevation = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].elevation);
                    double elevationvalue = floor(elevation * 100) / 100;
                    velevationRL_resim.push_back(elevationvalue);

                    double snr = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].snr);
                    double snrvalue = floor(snr * 100) / 100;
                    vsnrRL_resim.push_back(snrvalue);

                    double amp = flt_to_dbl(GPO_Dets_info_t.target_report[index[i]].amp);
                    double ampvalue = floor(amp * 100) / 100;
                    vampRL_resim.push_back(ampvalue);
                }
               
            }
        }

        if (fileCount == 0)
        {
            ptr_gpo_selfdetection_RL->veh_range_detections[RL][scanindex] = vrangeRL_veh;
            ptr_gpo_selfdetection_RL->veh_rangerate_detections[RL][scanindex] = vrangerateRL_veh;
            ptr_gpo_selfdetection_RL->veh_azimuth_detections[RL][scanindex] = vazimuthRL_veh;
            ptr_gpo_selfdetection_RL->veh_elevation_detections[RL][scanindex] = velevationRL_veh;
            ptr_gpo_selfdetection_RL->veh_snr_detections[RL][scanindex] = vsnrRL_veh;
            ptr_gpo_selfdetection_RL->veh_amp_detections[RL][scanindex] = vampRL_veh;

        }
        if (fileCount == 1)
        {
            if (inRange(vRLscanindex.front(), vRLscanindex.back(), scanindex))
             {
                ptr_gpo_selfdetection_RL->resim_range_detections[RL][scanindex] = vrangeRL_resim;
                ptr_gpo_selfdetection_RL->resim_rangerate_detections[RL][scanindex] = vrangerateRL_resim;
                ptr_gpo_selfdetection_RL->resim_azimuth_detections[RL][scanindex] = vazimuthRL_resim;
                ptr_gpo_selfdetection_RL->resim_elevation_detections[RL][scanindex] = velevationRL_resim;
                ptr_gpo_selfdetection_RL->resim_snr_detections[RL][scanindex] = vsnrRL_resim;
                ptr_gpo_selfdetection_RL->resim_amp_detections[RL][scanindex] = vampRL_resim;
             }
        }

        Valid = 0;
        vrangeRL_veh.clear();
        vrangerateRL_veh.clear();
        vazimuthRL_veh.clear();
        velevationRL_veh.clear();
        vsnrRL_veh.clear();
        vampRL_veh.clear();


        vrangeRL_resim.clear();
        vrangerateRL_resim.clear();
        vazimuthRL_resim.clear();
        velevationRL_resim.clear();
        vsnrRL_resim.clear();
        vampRL_resim.clear();
    }


}

double Gen5PlatformDataCollector::flt_to_dbl(float32_T con_value)
{
    char chr_fValue[32];
    sprintf(chr_fValue, "%9.8f", con_value);
    double db_val = strtod(chr_fValue, NULL);
    return db_val;
}
bool Gen5PlatformDataCollector::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
{
    return ((resimsi - vehiclesi_end) * (resimsi - vehiclesi_start) <= 0);
}


template<typename T>
std::vector<T> GetAddition_MissingValue(std::vector<T> ref, std::vector<T> comp)
{

    std::vector<T> diff;
    std::sort(ref.begin(), ref.end());
    std::sort(comp.begin(), comp.end());
    std::set_difference(ref.begin(), ref.end(),
        comp.begin(), comp.end(),
        std::back_inserter(diff));
    return diff;

}
template<typename T>
std::vector<T> GetMisMatchValue(std::vector<T> ref, std::vector<T> comp)
{
    std::vector<T> mismatch;
    std::sort(ref.begin(), ref.end());
    std::sort(comp.begin(), comp.end());
    std::set_symmetric_difference(ref.begin(), ref.end(),
        comp.begin(), comp.end(),
        std::back_inserter(mismatch));
    return mismatch;
}



//End of GenericFunctions

//Start of Gen5PlatformDataCollector Functions
Gen5PlatformDataCollector::Gen5PlatformDataCollector()
{

    logfile.open("logfile.txt", ios::out);
    PlotParaMemAllocation();
}

Gen5PlatformDataCollector::~Gen5PlatformDataCollector()
{
    logfile.close();

}

Gen5PlatformDataCollector& Gen5PlatformDataCollector::getInstance()
{
	static Gen5PlatformDataCollector instance;
	return instance;
}
void Gen5PlatformDataCollector::PlotParaMemAllocation()
{


	ptr_gpo_selfdetection_RL = new Self_Detections_Param_T();

}
void Gen5PlatformDataCollector::PlotParaMemDeallocation()
{
	if (ptr_gpo_selfdetection_RL != nullptr)
		delete ptr_gpo_selfdetection_RL;
	
}
void Gen5PlatformDataCollector::CollectData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
	
    CollectDetectionDataRL(ptr_gpo_selfdetection_RL,fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex );
}

void Gen5PlatformDataCollector::PlotDataPrep_Missing_Additional_Detection_count_RL()
{
    cout << "\nGen5PlatformDataCollector::PlotDataPrep_Missing_Additional_Detection_count_RL\n";

    for (auto vehicleval : ptr_gpo_selfdetection_RL->veh_valid_detectcount[0])
    {
        for (auto resimval : ptr_gpo_selfdetection_RL->resim_valid_detectcount[0])
        {
            if (vehicleval.first == resimval.first)
            {
                if (vehicleval.second > resimval.second)
                {
                    //Missing detection count in Resim file
                    IsMissingDetection = true;
                    ptr_gpo_selfdetection_RL->missed_detect_cnt_scanindex[0].push_back(vehicleval.first);
                    int count = (vehicleval.second - resimval.second);
                    ptr_gpo_selfdetection_RL->missed_detect_count_value[0].push_back(count);
                   
                }
                else if (vehicleval.second < resimval.second)
                {
                    //Additional detection count in Resim file
                    IsAdditionalDetection = true;
                    ptr_gpo_selfdetection_RL->additional_detect_cnt_scanindex[0].push_back(vehicleval.first);
                    int count = (vehicleval.second - resimval.second);
                    ptr_gpo_selfdetection_RL->additional_detect_count_value[0].push_back(count);
                }

            }
        }
    }

}

int Gen5PlatformDataCollector::GenerateMissingDetectionCountPlot(Self_Detections_Param_T* selfdetect, const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt)
{
    cout << "\nGen5PlatformDataCollector::GenerateMissingDetectionCountPlot\n";

    CTML::Document document;  
    std::fstream log_report;
    std::string htmlreport_file;
    std::string htmlreport_folder;
    std::string htmlreport_image;

    std::string htmlreport_imagepath(f_name);
    std::string radar_position(current_radar);

    auto mf4_filenamefrom_path = fs::path(f_name).filename();
    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };
    string imagefoldername = "GeneratedImages";

    htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT.html";
    htmlreport_folder = reportpath + "/" + foldname;

    if (!std::filesystem::exists(htmlreport_folder.c_str())) {
#ifdef _WIN32
        _mkdir(htmlreport_folder.c_str());
#endif
#ifdef __GNUC__
        mkdir(htmlreport_folder.c_str(), ACCESSPERMS);
#endif
    }
    if (!log_report.is_open())

        log_report.open(htmlreport_file.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app);
    {

       /* log_report << "<h1> Graph Info </h1>\n" << endl;
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scan index " << "value to be printed" << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " count  " << "value to be printed" << "</p>\n";*/

        if ((radar_position == "RL"))
        {
            sgtitle(" Missing Detection count Plot " + radar_position, "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            ax1->xlabel("scan index");
            ax1->ylabel("detection count");

            std::vector<double> x = selfdetect->missed_detect_cnt_scanindex[0];//scanindex
            std::vector<int> y = selfdetect->missed_detect_count_value[0];//missed detection count


            auto b = bar(x, y)->bar_width(.01f);       

        }

        if ((radar_position == "RR"))
        {
            sgtitle(" Missing Detection count Plot " + radar_position, "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            ax1->xlabel("scan index");
            ax1->ylabel("detection count");

            std::vector<double> x = selfdetect->missed_detect_cnt_scanindex[1];//scanindex       
            std::vector<int> y = selfdetect->missed_detect_count_value[1];//missed detection count
            auto b = bar(x, y)->bar_width(.1f);

        }

        if ((radar_position == "FL"))
        {
            sgtitle(" Missing Detection count Plot " + radar_position, "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            ax1->xlabel("scan index");
            ax1->ylabel("detection count");

            std::vector<double> x = selfdetect->missed_detect_cnt_scanindex[3];//scanindex
            std::vector<int> y = selfdetect->missed_detect_count_value[3];//missed detection count
            auto b = bar(x, y)->bar_width(.1f);

        }

        if ((radar_position == "FR"))
        {
            sgtitle(" Missing Detection count Plot " + radar_position, "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            ax1->xlabel("scan index");
            ax1->ylabel("detection count");

            std::vector<double> x = selfdetect->missed_detect_cnt_scanindex[2];//scanindex       
            std::vector<int> y = selfdetect->missed_detect_count_value[2];//missed detection count
            auto b = bar(x, y)->bar_width(.1f);

        }

        radar_position.append("RL_MissDetect");
        htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
        save((char*)htmlreport_image.c_str(), "png");
        
        htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
        CTML::Node image_trks("img");
        image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
                  .UseClosingTag(false);
        document.AppendNodeToBody(image_trks);
    }
    
    log_report << document.ToString() << endl;

    if (!log_report.is_open())
        log_report.close();

    return 0;
}

int Gen5PlatformDataCollector::GenerateDetectionPlots(Self_Detections_Param_T* selfdetec, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int arraycnt)
{

    cout << "\nGen5PlatformDataCollector::GenerateDetectionPlots\n";
    CTML::Document document;
    std::fstream log_report;
    std::string htmlreport_file;
    std::string htmlreport_folder;
    std::string htmlreport_image;

    std::string htmlreport_imagepath(f_name);
    std::string radar_position(current_radar);

    auto mf4_filenamefrom_path = fs::path(f_name).filename();
    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };
    string imagefoldername = "GeneratedImages";

    htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT.html";
    htmlreport_folder = reportpath + "/" + foldname;
 
    if (!std::filesystem::exists(htmlreport_folder.c_str())) {
#ifdef _WIN32
        _mkdir(htmlreport_folder.c_str());
#endif
#ifdef __GNUC__
        mkdir(htmlreport_folder.c_str(), ACCESSPERMS);
#endif
    }
    if (!log_report.is_open())

        log_report.open(htmlreport_file.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app);
    {

        auto tracks_fig1 = figure(true);
        if ((radar_position == "RL_Range")&&(plotparam=="Missingdetections"))
        {
            sgtitle(" Missed Range in RL ", "blue");   
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            grid(false);
            ax1->ylabel("range");
            ax1->xlabel("scan index");
            //auto pfl11 = scatter(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            //auto pfl11 = plot(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            auto pfl11 = scatter(Scanindex_missing_range_detection[0], missing_range_detection_value_inResim[0]);
            pfl11->color("red");
            pfl11->fill(true);
            pfl11->marker_color("r");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);

            radar_position.append("Missingdetections");
        }

        if ((radar_position == "RL_Azimuth") && (plotparam == "MissingAzimuth"))
        {
            sgtitle(" Missed Azimuth in RL ", "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            grid(false);
            ax1->ylabel("azimuth");
            ax1->xlabel("scan index");
            //auto pfl11 = scatter(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            //auto pfl11 = plot(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            auto pfl11 = scatter(Scanindex_missing_azimuth_detection[0], missing_azimuth_detection_value_inResim[0]);
            pfl11->color("red");
            pfl11->fill(true);
            pfl11->marker_color("r");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);

            radar_position.append("MissingAzimuth");
        }

        if ((radar_position == "RL_Elevation") && (plotparam == "MissingElevation"))
        {
            sgtitle(" Missed amplitude in RL ", "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            grid(false);
            ax1->ylabel("amplitude");
            ax1->xlabel("scan index");
            //auto pfl11 = scatter(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            //auto pfl11 = plot(RL_detection_range_mismatch_scanindex, RL_detection_range_mismatch_value);
            auto pfl11 = scatter(Scanindex_missing_elevation_detection[0], missing_elevation_detection_value_inResim[0]);
            pfl11->color("red");
            pfl11->fill(true);
            pfl11->marker_color("r");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);

            radar_position.append("MissingElevation");
        }

        

        
        htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
        save((char*)htmlreport_image.c_str(), "png");
        
        htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
        CTML::Node image_trks("img");
        image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
                  .UseClosingTag(false);
        document.AppendNodeToBody(image_trks);       
    }
    log_report << document.ToString() << endl;

    if (!log_report.is_open())
        log_report.close();
    return 0;
}


int Gen5PlatformDataCollector::PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt)
{
    cout << "\nGen5PlatformDataCollector::PrintReportInfoinHTML\n";
    
    CTML::Document document;
    DataProxy& datapxy = DataProxy::getInstance();  
    std::fstream log_report;
    std::string htmlreport_file;
    std::string htmlreport_folder;
    std::string htmlreport_image;

    std::string htmlreport_imagepath(f_name);
    std::string radar_position(current_radar); 

    auto mf4_filenamefrom_path = fs::path(f_name).filename();
    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };
    string imagefoldername = "GeneratedImages";
     
    htmlreport_file = reportpath + "/" + foldname + "/"+ mf4_filename + "-REPORT.html";
    htmlreport_folder = reportpath + "/"+ foldname;

 
    if (!std::filesystem::exists(htmlreport_folder.c_str())) {
#ifdef _WIN32
        _mkdir(htmlreport_folder.c_str());
#endif
#ifdef __GNUC__
        mkdir(htmlreport_folder.c_str(), ACCESSPERMS);
#endif
    }
    if (!log_report.is_open())

        log_report.open(htmlreport_file.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app);
    {
        log_report << "<h1> HTML Report </h1>\n" << endl;
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Vehicle File :: " << datapxy.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Resim File :: " << datapxy.resimfilename << "</p>\n";
      
    }

    radar_position.append("RL_MissDetect");
    htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
    save((char*)htmlreport_image.c_str(), "png");

    
    htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png"; 
    CTML::Node image_trks("img");
    image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
              .UseClosingTag(false);
    document.AppendNodeToBody(image_trks);

    return 0;
}


void Gen5PlatformDataCollector::Generateplot(const char* inputVehFileNameBMW, const char* pltfoldername, std::string reportpath)
{

   

    //CheckCollectedDetectionvalues();
	PlotDataPrep_Missing_Additional_Detection_count_RL();
    Collect_RangeMisMatch_inDetections_RL(0, ptr_gpo_selfdetection_RL->veh_range_detections[0], ptr_gpo_selfdetection_RL->resim_range_detections[0]);


    Collect_Addition_MissingRangeDetections_RL(0, ptr_gpo_selfdetection_RL->veh_range_detections[0], ptr_gpo_selfdetection_RL->resim_range_detections[0]);
    Collect_Addition_MissingAzimuthDetections_RL(0, ptr_gpo_selfdetection_RL->veh_azimuth_detections[0], ptr_gpo_selfdetection_RL->resim_azimuth_detections[0]);
    Collect_Addition_MissingElevationDetections_RL(0, ptr_gpo_selfdetection_RL->veh_elevation_detections[0], ptr_gpo_selfdetection_RL->resim_elevation_detections[0]);

    PrintReportInfoinHTML(inputVehFileNameBMW, "RL", pltfoldername, reportpath, 0);
    
   

   /* if (RL_detection_range_mismatch_scanindex.size() != 0 && RL_detection_range_mismatch_value.size()!=0)
    {
        GenerateDetectionPlots(ptr_gpo_selfdetection_RL, inputVehFileNameBMW, "RL_Range", pltfoldername, "detections", reportpath, 0);
    }*/

    GenerateDetectionPlots(ptr_gpo_selfdetection_RL, inputVehFileNameBMW, "RL_Range", pltfoldername, "Missingdetections", reportpath, 0);
    GenerateDetectionPlots(ptr_gpo_selfdetection_RL, inputVehFileNameBMW, "RL_Azimuth", pltfoldername, "MissingAzimuth", reportpath, 0);
    GenerateDetectionPlots(ptr_gpo_selfdetection_RL, inputVehFileNameBMW, "RL_Elevation", pltfoldername, "MissingElevation", reportpath, 0);

    if (IsMissingDetection)
    {
        GenerateMissingDetectionCountPlot(ptr_gpo_selfdetection_RL, inputVehFileNameBMW, "RL", pltfoldername, reportpath, 0);
    }
   


    IsMissingDetection = false;
    IsAdditionalDetection = false;

}

void Gen5PlatformDataCollector::Collect_RangeMisMatch_inDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data)
{

    cout << "\nGen5PlatformDataCollector::Collect_RangeMisMatch_inDetections_RL\n";
    std::vector<double> scanindex;
    std::vector<double> mismatch_detection;

    for (const auto& [key, value] : veh_det_data)
    {
        int key_veh_det_data = key;
        std::vector<double> temp_veh_det_data;
        temp_veh_det_data = value;

        for (const auto& [key, value] : resim_det_data)
        {
            if (key_veh_det_data == key)
            {
                std::vector<double> temp_resim_det_data;
                temp_resim_det_data = value;

                mismatch_detection = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);

                if (mismatch_detection.size() != 0)
                {
                    scanindex.push_back(key_veh_det_data);
                                   
                }
            }
        }
        if (scanindex.size() != 0 && mismatch_detection.size() != 0)
        {
            for (auto si : scanindex)
            {
                for (auto mismatchdetections : mismatch_detection)
                {                   
                    RL_detection_range_mismatch_scanindex.push_back(si);
                    RL_detection_range_mismatch_value.push_back(mismatchdetections);
                }
            }
        }
        mismatch_detection.clear();
        scanindex.clear();
    }


}

void Gen5PlatformDataCollector::Collect_Addition_MissingRangeDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data)
{


    cout << "\nGen5PlatformDataCollector::Collect_Addition_MissingRangeDetections_RL\n";
    std::vector<double> scanindex;
    std::vector<double> mismatch_detection;

    std::vector<double> missing_detection_range_inResimdata;
    std::vector<double> missing_detection_range_inVehdata;
    std::vector<double> additional_detection_range_inResimdata;
    std::vector<double> mismatch_detection_range_invehResimdata;


    for (const auto& [key, value] : veh_det_data)
    {
        int key_veh_det_data = key;
        std::vector<double> temp_veh_det_data;
        temp_veh_det_data = value;

        for (const auto& [key, value] : resim_det_data)
        {
            if (key_veh_det_data == key)
            {
                std::vector<double> temp_resim_det_data;
                temp_resim_det_data = value;


                if (temp_veh_det_data.size() > temp_resim_det_data.size())
                {

                    std::cout << "\n veh data more than resim data : Missing data Prediction \n";
                    missing_detection_range_inResimdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);//first:vehdata,second:resimdata
                    std::cout << "\n Printing Missing Range resim data @SI "<< key_veh_det_data<<endl;
                    for (auto data : missing_detection_range_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_missing_range_detection[0].push_back(key_veh_det_data);
                        missing_range_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() < temp_resim_det_data.size())
                {

                    std::cout << "\n veh data less than resim data : Additional data Prediction & Missing data \n";
                    additional_detection_range_inResimdata = GetAddition_MissingValue(temp_resim_det_data, temp_veh_det_data);//first:resimdata, second:vehicledata
                    std::cout << "\n Printing Additional resim data @SI " << key_veh_det_data << endl;
                    for (auto data : additional_detection_range_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_additional_range_detection[0].push_back(key_veh_det_data);
                        additional_range_detection_value_inResim[0].push_back(data);
                    }

                    
                    missing_detection_range_inVehdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);
                    std::cout << "\n Printing Missing vehicle data @SI " << key_veh_det_data << endl;
                    for (auto data : missing_detection_range_inVehdata)
                    {
                        std::cout << "\n" << data;
                        //Scanindex_missing_range_detection[0].push_back(key_veh_det_data);
                        //missing_range_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() == temp_resim_det_data.size())
                {
                    std::cout << "\n veh data equal to  resim data : Mismatch Data Prediction\n";
                    mismatch_detection_range_invehResimdata = GetMisMatchValue(temp_veh_det_data, temp_resim_det_data);//first:vehciledata, second: resim data
                    std::cout << "\n Printing Mismatch vehResim data @SI " << key_veh_det_data << endl;
                    for (auto data : mismatch_detection_range_invehResimdata)
                    {
                        std::cout << "\n" << data;
                        //scanindex_mismatch_range_detection_inVeh[0].push_back(key_veh_det_data);
                        //scanindex_mismatch_range_detection_inResim[0].push_back(key_veh_det_data);

                        //mismatch_range_detection_inVeh[0].push_back(data);
                    }
                }


              /*  mismatch_detection = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);

                if (mismatch_detection.size() != 0)
                {
                    scanindex.push_back(key_veh_det_data);

                }*/
            }
        }
       /* if (scanindex.size() != 0 && mismatch_detection.size() != 0)
        {
            for (auto si : scanindex)
            {
                for (auto mismatchdetections : mismatch_detection)
                {
                    RL_detection_range_mismatch_scanindex.push_back(si);
                    RL_detection_range_mismatch_value.push_back(mismatchdetections);
                }
            }
        }
        mismatch_detection.clear();
        scanindex.clear();*/
    }



}

void Gen5PlatformDataCollector::Collect_Addition_MissingAzimuthDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data)
{
    cout << "\nGen5PlatformDataCollector::Collect_Addition_MissingAzimuthDetections_RL\n";

    std::vector<double> missing_detection_azimuth_inResimdata;
    std::vector<double> missing_detection_azimuth_inVehdata;
    std::vector<double> additional_detection_azimuth_inResimdata;
    std::vector<double> mismatch_detection_azimuth_invehResimdata;



    for (const auto& [key, value] : veh_det_data)
    {
        int key_veh_det_data = key;
        std::vector<double> temp_veh_det_data;
        temp_veh_det_data = value;

        for (const auto& [key, value] : resim_det_data)
        {
            if (key_veh_det_data == key)
            {
                std::vector<double> temp_resim_det_data;
                temp_resim_det_data = value;


                if (temp_veh_det_data.size() > temp_resim_det_data.size())
                {

                    std::cout << "\n veh data more than resim data : Missing azimuth Prediction \n";
                    missing_detection_azimuth_inResimdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);//first:vehdata,second:resimdata
                    std::cout << "\n Printing Missing azimuth resim data @SI " << key_veh_det_data << endl;
                    for (auto data : missing_detection_azimuth_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_missing_azimuth_detection[0].push_back(key_veh_det_data);
                        missing_azimuth_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() < temp_resim_det_data.size())
                {

                    std::cout << "\n veh data less than resim data : Additional azimuth data Prediction & Missing data \n";
                    additional_detection_azimuth_inResimdata = GetAddition_MissingValue(temp_resim_det_data, temp_veh_det_data);//first:resimdata, second:vehicledata
                    std::cout << "\n Printing Additional azimuth resim data @SI " << key_veh_det_data << endl;
                    for (auto data : additional_detection_azimuth_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_additional_azimuth_detection[0].push_back(key_veh_det_data);
                        additional_azimuth_detection_value_inResim[0].push_back(data);
                    }


                    missing_detection_azimuth_inVehdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);
                    std::cout << "\n Printing Missing azimuth vehicle data @SI " << key_veh_det_data << endl;
                    for (auto data : missing_detection_azimuth_inVehdata)
                    {
                        std::cout << "\n" << data;
                        //Scanindex_missing_range_detection[0].push_back(key_veh_det_data);
                        //missing_range_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() == temp_resim_det_data.size())
                {
                    std::cout << "\n veh data equal to  resim data : Mismatch azimuth Data Prediction\n";
                    mismatch_detection_azimuth_invehResimdata = GetMisMatchValue(temp_veh_det_data, temp_resim_det_data);//first:vehciledata, second: resim data
                    std::cout << "\n Printing Mismatch azimuth vehResim data @SI " << key_veh_det_data << endl;
                    for (auto data : mismatch_detection_azimuth_invehResimdata)
                    {
                        std::cout << "\n" << data;
                        //scanindex_mismatch_range_detection_inVeh[0].push_back(key_veh_det_data);
                        //scanindex_mismatch_range_detection_inResim[0].push_back(key_veh_det_data);

                        //mismatch_range_detection_inVeh[0].push_back(data);
                    }
                }


             
            }
        }
        
    }

}

void Gen5PlatformDataCollector::Collect_Addition_MissingElevationDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data)
{
    cout << "\nGen5PlatformDataCollector::Collect_Addition_MissingElevationDetections_RL\n";

    std::vector<double> missing_detection_elevation_inResimdata;
    std::vector<double> missing_detection_elevation_inVehdata;
    std::vector<double> additional_detection_elevation_inResimdata;
    std::vector<double> mismatch_detection_elevation_invehResimdata;



    for (const auto& [key, value] : veh_det_data)
    {
        int key_veh_det_data = key;
        std::vector<double> temp_veh_det_data;
        temp_veh_det_data = value;

        for (const auto& [key, value] : resim_det_data)
        {
            if (key_veh_det_data == key)
            {
                std::vector<double> temp_resim_det_data;
                temp_resim_det_data = value;


                if (temp_veh_det_data.size() > temp_resim_det_data.size())
                {

                    std::cout << "\n veh data more than resim data : Missing elevation Prediction \n";
                    missing_detection_elevation_inResimdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);//first:vehdata,second:resimdata
                    std::cout << "\n Printing Missing elevation resim data @SI " << key_veh_det_data << endl;
                    for (auto data : missing_detection_elevation_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_missing_elevation_detection[0].push_back(key_veh_det_data);
                        missing_elevation_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() < temp_resim_det_data.size())
                {

                    std::cout << "\n veh data less than resim data : Additional elevation data Prediction & Missing data \n";
                    additional_detection_elevation_inResimdata = GetAddition_MissingValue(temp_resim_det_data, temp_veh_det_data);//first:resimdata, second:vehicledata
                    std::cout << "\n Printing Additional elevation resim data @SI " << key_veh_det_data << endl;
                    for (auto data : additional_detection_elevation_inResimdata)
                    {
                        std::cout << "\n" << data;
                        Scanindex_additional_elevation_detection[0].push_back(key_veh_det_data);
                        additional_elevation_detection_value_inResim[0].push_back(data);
                    }


                    missing_detection_elevation_inVehdata = GetAddition_MissingValue(temp_veh_det_data, temp_resim_det_data);
                    std::cout << "\n Printing Missing elevation vehicle data @SI " << key_veh_det_data << endl;
                    for (auto data : missing_detection_elevation_inVehdata)
                    {
                        std::cout << "\n" << data;
                        //Scanindex_missing_range_detection[0].push_back(key_veh_det_data);
                        //missing_range_detection_value_inResim[0].push_back(data);
                    }
                }

                if (temp_veh_det_data.size() == temp_resim_det_data.size())
                {
                    std::cout << "\n veh data equal to  resim data : Mismatch elevation Data Prediction\n";
                    mismatch_detection_elevation_invehResimdata = GetMisMatchValue(temp_veh_det_data, temp_resim_det_data);//first:vehciledata, second: resim data
                    std::cout << "\n Printing Mismatch elevation vehResim data @SI " << key_veh_det_data << endl;
                    for (auto data : mismatch_detection_elevation_invehResimdata)
                    {
                        std::cout << "\n" << data;
                        //scanindex_mismatch_range_detection_inVeh[0].push_back(key_veh_det_data);
                        //scanindex_mismatch_range_detection_inResim[0].push_back(key_veh_det_data);

                        //mismatch_range_detection_inVeh[0].push_back(data);
                    }
                }



            }
        }

    }

}

void Gen5PlatformDataCollector::Collect_Addition_MissingAmplitudeDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data)
{
    cout << "\nGen5PlatformDataCollector::Collect_Addition_MissingAmplitudeDetections_RL\n";

    std::vector<double> missing_detection_amplitude_inResimdata;
    std::vector<double> missing_detection_amplitude_inVehdata;
    std::vector<double> additional_detection_amplitude_inResimdata;
    std::vector<double> mismatch_detection_amplitude_invehResimdata;
}

//End of Gen5PlatformDataCollector Functions















