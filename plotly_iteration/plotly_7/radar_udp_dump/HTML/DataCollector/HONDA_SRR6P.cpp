#include<filesystem>
#include "HONDA_SRR6P.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../PlotGenerator/MatplotNonScaleplot.h"
#include "../InputParser/JsonInputParser.h"


#define AF_DET_MAX_COUNT    (256U) 
#define NUMBER_OF_DETECTIONS (64)
#define PI 3.14159265359
#define RAD2DEG(x) ((x * 180.00) / PI)
#define RDD_MAX_NUM_DET    (400U) // FLR4P dets

//const unsigned float PI = 3.14159265359;
namespace fs = std::filesystem;
extern RADAR_Plugin_Interface_T* m_radar_plugin;
float32_T* array_data_ptr_hondasrr6p;

C2_Tracker_Logging_Data_GEN5_T p_hondasrr6p_curvi_track_data_info = { 0 };
Tracker_Input_Reports_SELF_Logging_GEN5_T p_hondasrr6p_detection_data_info = { 0 };
TRATON_Blockage_Output_Log_Data_T p_hondasrr6p_mnr_blockage_data_info = { 0 }; // same stream can be used for Honda
HONDA_SRR6P_Alignment_Log_Data_T p_hondasrr6p_align_data_info = { 0 };

//Feature functions for HONDA
LCDA_Output_Logging_T p_Honda_LCDA_FF_XML1{ 0 };
CTA_Output_Logging_T p_Honda_CTA_FF_XML1{ 0 };
CED_Output_Logging_T p_Honda_CED_FF_XML1{ 0 };


double hondasrr6p_round_value(double var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
   /* double value = (int)(var * 100 + .5);
    return (double)value / 100;*/

    double value = floor(var * 10) / 10;
    return value;
}

int comparedata_hondasrr6p(const void* a, const void* b)
{
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptr_hondasrr6p[value_of_a] < array_data_ptr_hondasrr6p[value_of_b]) {
        return -1;
    }
    else if (array_data_ptr_hondasrr6p[value_of_a] == array_data_ptr_hondasrr6p[value_of_b])
    {

    }
    else
    {
        return (array_data_ptr_hondasrr6p[value_of_a] > array_data_ptr_hondasrr6p[value_of_b]);
    }
}

HondaSRR6pDataCollector::HondaSRR6pDataCollector(std::shared_ptr<PlotParameter[]>& sptr_plotparam) :sptr_plotparam_honda_srr6p(sptr_plotparam)
{
    (sptr_plotparam_honda_srr6p.get() + 0)->vec_align_angle_az[0].clear();
    (sptr_plotparam_honda_srr6p.get() + 0)->vec_align_angle_az[1].clear();
}

HondaSRR6pDataCollector::~HondaSRR6pDataCollector()
{

}



void HondaSRR6pDataCollector::collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    int rrpos = int(Radar_Posn);

    JsonInputParser& json = JsonInputParser::getInstance();

    if (platform == UDP_PLATFORM_SRR6_PLUS && stream_num == C2_CORE_MASTER_STREAM && custId == HONDA_GEN5)
    {
       collect_detection_data_honda_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
       collect_mnrblockage_data_honda_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
       collect_tracker_data_honda_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
       collect_alignment_data_honda_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
    
    }
    if (stream_num == C2_CUST_MASTER_STREAM)
    {
        CollectLCDA_FF(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
        CollectCTA_FF(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
        CollectCED_FF(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform);
    
    }
 
}

void HondaSRR6pDataCollector::generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{


}

void HondaSRR6pDataCollector::calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    JsonInputParser& jsonpar = JsonInputParser::getInstance();
    MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start, log2_scanindex_end;
    long long log1_duration, log2_duration;
    int tool_duration_func_inputlog;
    int log1_duration_fin{ 0 }, log2_duration_fin{ 0 };

    if ((sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.size() != 0)
    {
        log1_scanindex_start = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.front();
        log1_scanindex_end = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.back();
        log1_duration = log1_scanindex_end - log1_scanindex_start;
        log1_duration_fin = log1_duration * .05;

        if (sptr_plotparam_honda_srr6p.get() != nullptr)
        {
            jsonpar.Log1_duration = to_string(log1_duration_fin);

        }
    }

    if ((sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.size() != 0)
    {
        log2_scanindex_start = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.front();
        log2_scanindex_end = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.back();
        log2_duration = log2_scanindex_end - log2_scanindex_start;
        log2_duration_fin = log2_duration * .05;

        if (sptr_plotparam_honda_srr6p.get() != nullptr)
        {
            jsonpar.Log2_duration = to_string(log2_duration_fin);
        }
    }





    if (sptr_plotparam_honda_srr6p != nullptr)
    {
        if (log1_duration_fin != 0 || log2_duration_fin != 0)
        {
            tool_duration_func_inputlog = (stoi(jsonpar.html_runtime_sec)) / (log1_duration_fin + log2_duration_fin);
            jsonpar.Log1_duration = to_string(log1_duration_fin);
            jsonpar.Log2_duration = to_string(log2_duration_fin);
            jsonpar.tool_runtime_func_inputtime = to_string(tool_duration_func_inputlog);
            matplotnonscale.print_reporttoolrun_timing_info(f_name, "HTML", pltfolder, reportpath, radarposition);
        }
        else
        {
            jsonpar.Log1_duration = "Log duration not available";
            jsonpar.Log2_duration = "Log duration not available";
            jsonpar.tool_runtime_func_inputtime = "Run time not calculates as Log duration not available";
            matplotnonscale.print_reporttoolrun_timing_info(f_name, "HTML", pltfolder, reportpath, radarposition);

        }

    }


}

void HondaSRR6pDataCollector::calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    //std::cout << "\ncalculate_Logduration";  
    JsonInputParser& jsonparser = JsonInputParser::getInstance();


    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start, log2_scanindex_end;
    long long log1_duration, log2_duration;
    double tool_duration_func_inputlog;
    long long log1_duration_fin, log2_duration_fin;


    if (sptr_plotparam_honda_srr6p.get() != nullptr)
    {

        if ((sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.size() != 0)
        {
            log1_scanindex_start = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.front();
            log1_scanindex_end = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_veh_ref_scale.back();
            log1_duration = log1_scanindex_end - log1_scanindex_start;
            log1_duration_fin = log1_duration * .05;

            if (sptr_plotparam_honda_srr6p.get() != nullptr)
            {
                jsonparser.Log1_duration = to_string(log1_duration_fin);

            }
        }


        if ((sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.size() != 0)
        {
            log2_scanindex_start = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.front();
            log2_scanindex_end = (sptr_plotparam_honda_srr6p.get() + radarposition)->si_resim_ref_scale.back();
            log2_duration = log2_scanindex_end - log2_scanindex_start;
            log2_duration_fin = log2_duration * .05;

            if (sptr_plotparam_honda_srr6p.get() != nullptr)
            {
                jsonparser.Log2_duration = to_string(log2_duration_fin);
            }
        }




    }


}


double HondaSRR6pDataCollector::flt_to_dbl(float32_T con_value)
{
    char chr_fValue[64];//64
    sprintf(chr_fValue, "%9.8f", con_value);//9.8
    double db_val = strtod(chr_fValue, NULL);
    return db_val;
}

//double HondaSRR6pDataCollector::flt_to_dbl(float32_T con_value)
//{
//    return static_cast<double>(con_value);
//}

bool HondaSRR6pDataCollector::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
{
    return ((resimsi - vehiclesi_end) * (resimsi - vehiclesi_start) <= 0);
}

inline void HondaSRR6pDataCollector::remove_old_html_reports(const char* f_name, const char* pltfolder, std::string reportpath)
{
    auto mf4_filenamefrom_path = fs::path(f_name).filename();
    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };

    std::vector<string> sensor_report_extension;
    sensor_report_extension.push_back("-REPORT_RL.html");
    sensor_report_extension.push_back("-REPORT_RR.html");
    sensor_report_extension.push_back("-REPORT_FL.html");
    sensor_report_extension.push_back("-REPORT_FR.html");
    sensor_report_extension.push_back("-REPORT_FC.html");
    sensor_report_extension.push_back("-Overall_BOXREPORT.html");

    std::string htmlreport_file;
    std::string htmlreport_image;
    for (auto file_extension : sensor_report_extension)
    {
        htmlreport_file = reportpath + "/" + pltfolder + "/" + mf4_filename + file_extension;
        if (std::filesystem::exists(htmlreport_file))
        {
            int result = remove(htmlreport_file.c_str());
        }
    }

    htmlreport_image = reportpath + "/" + pltfolder + "/" + "GeneratedImages";
    if (std::filesystem::exists(htmlreport_image))
    {
        int result = remove(htmlreport_image.c_str());
    }
}



void HondaSRR6pDataCollector::collect_detection_data_honda_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{

    if (m_radar_plugin != nullptr && sptr_plotparam_honda_srr6p.get() != nullptr)
    {
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_hondasrr6p_detection_data_info, DETS_COMP);
        int radar_position = int(Radar_Posn);

        float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
            Self_Dets[i] = p_hondasrr6p_detection_data_info.dets_float[i].range;
        }
        unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
        unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptr_hondasrr6p = Self_Dets;
        qsort(index, size, sizeof(*index), comparedata_hondasrr6p);

        static unsigned8_T Valid = 0;

        for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
        {
            unsigned8_T valid_level = p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level;
            if (valid_level == 1)
            {
                Valid++;
            }
        }
      
        /*print sorted self dets*/
        unsigned8_T count = 1;
        for (unsigned16_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
        {
            if ((p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level > 0) && (count == 1))
            {
                if (fileCount == 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->si_veh_ref_scale.push_back(scanindex);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->valid_detection_count[fileCount].push_back(double(p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level));
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level)));
                }
                if (fileCount == 1)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->si_resim_ref_scale.push_back(scanindex);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->valid_detection_count[fileCount].push_back(double(p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level));
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(p_hondasrr6p_detection_data_info.dets_float[index[i]].valid_level)));
                }
                count = 0;
            }
            if (p_hondasrr6p_detection_data_info.dets_float[index[i]].range != 0)
            {
                if (fileCount == 0)
                {
                    scanindexRangeRef[radar_position].push_back(scanindex);

                    double rangevalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].range);
                    if (rangevalue >= -1 && rangevalue <= 500)
                    {
                        //if(radar_position==3)
                        rangevalue = hondasrr6p_round_value(rangevalue);
                        //std::cout << " file0 rangevalue " << rangevalue << std::endl;
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->range[fileCount].push_back(rangevalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                    }

                    double rangeratevalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].range_rate_raw);
                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        rangeratevalue = hondasrr6p_round_value(rangeratevalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }

                    double ampvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].amplitude);
                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        ampvalue = hondasrr6p_round_value(ampvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }

                    double rcsvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].std_rcs);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        rcsvalue = hondasrr6p_round_value(rcsvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }

                    double snrvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].snr);
                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        snrvalue = hondasrr6p_round_value(snrvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }

                    double azimuth = flt_to_dbl(RAD2DEG(p_hondasrr6p_detection_data_info.dets_float[index[i]].azimuth_raw));
                    if (azimuth >= -90 && azimuth <= 90)
                    {
                        azimuth = hondasrr6p_round_value(azimuth);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }

                    double elevation = flt_to_dbl(RAD2DEG(p_hondasrr6p_detection_data_info.dets_float[index[i]].elevation_raw));
                    if (elevation >= -40 && elevation <= 40)
                    {
                        elevation = hondasrr6p_round_value(elevation);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation[fileCount].push_back(elevation);
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                    }

                    double az_conf_value = double(p_hondasrr6p_detection_data_info.dets_float[index[i]].azimuth_confidence);
                    az_conf_value = hondasrr6p_round_value(az_conf_value);
                    //double el_conf_value = double(p_hondasrr6p_detection_data_info.dets_float[index[i]].el_conf);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                    //(sptr_plotparam_honda_srr6p.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);




                    if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].bi_static_target))
                    {
                        isbistatic_count_veh++;
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_isbistatic_count_veh++;
                    }
                    /*if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].isSingleTarget)) {
                        issingletarget_count_veh++;
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_issingletarget_count_veh++;
                    }*/
                    if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].super_res_target)) {
                        issuperrestarget_count_veh++;
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_issuperrestarget_count_veh++;
                    }

                    //(sptr_plotparam_honda_srr6p.get() + radar_position)->isSingleTarget_azimuth[fileCount].push_back(double((p_hondasrr6p_detection_data_info.dets_float[index[i]].isSingleTarget_azimuth)));




                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -90 && azimuth <= 90) ||
                        elevation >= -40 && elevation <= 40)
                    {

                        (sptr_plotparam_honda_srr6p.get() + radar_position)->si_veh.push_back(scanindex);


                    }

                }

                if (fileCount == 1)
                {
                    if (!scanindexRangeRef[radar_position].empty())
                    {
                        if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                        {
                            double rangevalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].range);
                            if (rangevalue >= -1 && rangevalue <= 500)
                            {
                                rangevalue = hondasrr6p_round_value(rangevalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->range[fileCount].push_back(rangevalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                            }

                            double rangeratevalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].range_rate_raw);
                            if (rangeratevalue >= -30 && rangeratevalue <= 30)
                            {
                                rangeratevalue = hondasrr6p_round_value(rangeratevalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                            }

                            double ampvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].amplitude);
                            if (ampvalue >= -100 && ampvalue <= 100)
                            {
                                ampvalue = hondasrr6p_round_value(ampvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                            }

                            double rcsvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].std_rcs);
                            if (rcsvalue >= -100 && rcsvalue <= 100)
                            {
                                rcsvalue = hondasrr6p_round_value(rcsvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                            }

                            double snrvalue = flt_to_dbl(p_hondasrr6p_detection_data_info.dets_float[index[i]].snr);
                            if (snrvalue >= -100 && snrvalue <= 100)
                            {
                                snrvalue = hondasrr6p_round_value(snrvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                            }

                            double azimuth = flt_to_dbl(RAD2DEG(p_hondasrr6p_detection_data_info.dets_float[index[i]].azimuth_raw));
                            if (azimuth >= -90 && azimuth <= 90)
                            {
                                azimuth = hondasrr6p_round_value(azimuth);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                            }

                            double elevation = flt_to_dbl(RAD2DEG(p_hondasrr6p_detection_data_info.dets_float[index[i]].elevation_raw));
                            if (elevation >= -40 && elevation <= 40)
                            {
                                elevation = hondasrr6p_round_value(elevation);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation[fileCount].push_back(elevation);
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                            }

                            double az_conf_value = double(p_hondasrr6p_detection_data_info.dets_float[index[i]].azimuth_confidence);
                            az_conf_value = hondasrr6p_round_value(az_conf_value);
                            //double el_conf_value = double(p_hondasrr6p_detection_data_info.dets_float[index[i]].el_conf);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                            //(sptr_plotparam_honda_srr6p.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);






                            if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].bi_static_target)) {
                                isbistatic_count_resim++;
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_isbistatic_count_resim++;
                            }
                           /* if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].isSingleTarget)) {
                                issingletarget_count_resim++;
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_issingletarget_count_resim++;
                            }*/
                            if (int(p_hondasrr6p_detection_data_info.dets_float[index[i]].super_res_target)) {
                                issuperrestarget_count_resim++;
                                (sptr_plotparam_honda_srr6p.get() + radar_position)->cum_issuperrestarget_count_resim++;
                            }

                            //(sptr_plotparam_honda_srr6p.get() + radar_position)->isSingleTarget_azimuth[fileCount].push_back(double((p_hondasrr6p_detection_data_info.dets_float[index[i]].isSingleTarget_azimuth)));

                            if ((rangevalue >= -1 && rangevalue <= 500) ||
                                (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                (ampvalue >= -100 && ampvalue <= 100) ||
                                (snrvalue >= -100 && snrvalue <= 100) ||
                                (azimuth >= -90 && azimuth <= 90) ||
                                elevation >= -40 && elevation <= 40)
                            {

                                (sptr_plotparam_honda_srr6p.get() + radar_position)->si_resim.push_back(scanindex);

                            }


                        }
                    }

                }

            }
        }
        if (count == 0)
        {
            if (fileCount == 0)
            {
                if (isbistatic_count_veh > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_veh);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                    isbistatic_count_veh = 0;
                }
                if (issingletarget_count_veh > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_veh);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                    issingletarget_count_veh = 0;
                }
                if (issuperrestarget_count_veh > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_veh);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                    issuperrestarget_count_veh = 0;
                }

            }
            if (fileCount == 1)
            {
                if (isbistatic_count_resim > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_resim);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                    isbistatic_count_resim = 0;
                }
                if (issingletarget_count_resim > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_resim);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                    issingletarget_count_resim = 0;
                }
                if (issuperrestarget_count_resim > 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_resim);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                    issuperrestarget_count_resim = 0;
                }
            }




            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->range_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->range_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->elevation_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->rcs_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->amp_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->amp_scaleplot[fileCount]));
            }
            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->snr_scaleplot[fileCount].size() != 0)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->snr_scaleplot[fileCount]));
            }

            if ((sptr_plotparam_honda_srr6p.get() + radar_position)->range[fileCount].size() != 0)
            {
                std::vector<double>::iterator itrmax_value;


                itrmax_value = std::max_element((sptr_plotparam_honda_srr6p.get() + radar_position)->range[fileCount].begin(),
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->range[fileCount].end());

                (sptr_plotparam_honda_srr6p.get() + radar_position)->range_max[fileCount].push_back(*itrmax_value);
                (sptr_plotparam_honda_srr6p.get() + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);




            }

            (sptr_plotparam_honda_srr6p.get() + radar_position)->range_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->range_rate_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->azimuth_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->elevation_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->rcs_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->amp_scaleplot[fileCount].clear();
            (sptr_plotparam_honda_srr6p.get() + radar_position)->snr_scaleplot[fileCount].clear();

        }

    }

}

void HondaSRR6pDataCollector::collect_mnrblockage_data_honda_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    if (sptr_plotparam_honda_srr6p.get() != nullptr && m_radar_plugin != nullptr)
    {
        int radar_position = int(Radar_Posn);
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_hondasrr6p_mnr_blockage_data_info, BLOCKAGE_DATA);


        double AvgRm_mnr_avg = flt_to_dbl(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.AvgRm_mnr_avg);
        AvgRm_mnr_avg = hondasrr6p_round_value(AvgRm_mnr_avg);
        double MaxRm_mnr_avg = flt_to_dbl(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.MaxRm_mnr_avg);
        MaxRm_mnr_avg = hondasrr6p_round_value(MaxRm_mnr_avg);

        unsigned char Bmnr1 = p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Bmnr1;
        /*unsigned char Bmnr2 = p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Bmnr2;
        unsigned char Bmnr3 = p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Bmnr3;
        unsigned char Bmnr4 = p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Bmnr4;*/

        //(sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Bmnr1[fileCount].push_back(Bmnr1);
        /*(sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Bmnr2[fileCount].push_back(Bmnr2);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Bmnr3[fileCount].push_back(Bmnr3);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Bmnr4[fileCount].push_back(Bmnr4);*/

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_AvgRm_mnr_avg[fileCount].push_back(AvgRm_mnr_avg);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_MaxRm_mnr_avg[fileCount].push_back(MaxRm_mnr_avg);

        

        /* if (fileCount==1 && radar_position==3)
         {
             ffile1 << "\n" << AvgRm_mnr_avg;
         }*/


         //(sptr_plotparam_honda_srr6p.get() + radar_position)->vec_range_check_blockage[fileCount].push_back(p_mnr_blockage_Srr6p_flr4.range_check_blockage);

        double AvgPm_mnr_avg = flt_to_dbl(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.AvgPm_mnr_avg);
        AvgPm_mnr_avg = hondasrr6p_round_value(AvgPm_mnr_avg);
        double MaxPm_mnr_avg = flt_to_dbl(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.MaxPm_mnr_avg);
        MaxPm_mnr_avg = hondasrr6p_round_value(MaxPm_mnr_avg);
        //unsigned char Bmnr1 = p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Bmnr1;

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_AvgPm_mnr_avg[fileCount].push_back(AvgPm_mnr_avg);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_MaxPm_mnr_avg[fileCount].push_back(MaxPm_mnr_avg);
       



        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Degraded[fileCount].push_back(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Degraded);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_Blocked[fileCount].push_back(p_hondasrr6p_mnr_blockage_data_info.Traton_MNR_Blockage_data.Blocked);

        (sptr_plotparam_honda_srr6p.get() + radar_position)->mnr_scanidex[fileCount].push_back(scanindex);
    }

}

void HondaSRR6pDataCollector::collect_tracker_data_honda_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    //std::cout << "\n collect_tracker_data_flr4p";
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_hondasrr6p_curvi_track_data_info, TRACK_COMP_CORE);
    if (custId == HONDA_GEN5)
    {
        int radar_position = int(Radar_Posn);
        //Track_Info 
        unsigned8_T valid_track_counter = 0;
        unsigned8_T count_track = NUMBER_OF_OBJECT_TRACKS_L - 1;
        for (unsigned8_T index = 0; index < NUMBER_OF_OBJECT_TRACKS_L; index++)  // valid trackes count 
        {
            if ((p_hondasrr6p_curvi_track_data_info.CurviTracks[count_track].status) > 0)
            {
                valid_track_counter++;
            }
            count_track--;
        }


        float32_T Track_Objs[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
        {
            Track_Objs[i] = p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_posn;
        }
        unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
        unsigned8_T index[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptr_hondasrr6p = Track_Objs;
        qsort(index, size, sizeof(*index), comparedata_hondasrr6p);


        unsigned8_T count = 1;
        for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
        {

            if (((p_hondasrr6p_curvi_track_data_info.CurviTracks[i].status) > 0) && (count == 1))
            {
                //fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d_%s>", g_scanindex, GetRadarPosName(Radar_Posn));
                //fprintf(SIL_XML_out, "\n<TRACK_INFO> vcs_xposn, \tvcs_yposn,\tf_moving,\tvcs_xvel,\t\tvcs_yvel,\t\tvcs_xaccel,\t\tvcs_yaccel,\t\tvcs_heading,\tstatus,\t\t\t\tobject_class,\t\t\t\t\t\tlen1,\t\tlen2,\t\twid1,\t\twid2,\t\ttrkID,\t\tunique_id,\t\treducedID,\tspeed,\t\t\tcurvature,\t\ttang_accel,\tstate_variance[0],\tstate_variance[1],\tstate_variance[2],\tstate_variance[3],\tstate_variance[4],\tstate_variance[5],\tsupplemental_state_covariance[0],\tsupplemental_state_covariance[1],\tsupplemental_state_covariance[2], \tconfidenceLevel,\t time_since_measurement,\t\ttime_since_cluster_created, time_since_track_updated, time_since_stage_start,\t\t\t\tndets,\t\tnum_reduced_dets,\t\texistence_probability,\t\treference_point,\treducedStatus,\tinit_scheme,\tf_crossing,\tf_oncoming,\tf_vehicular_trk,\tf_onguardrail,\tf_fast_moving,\taccuracy_width,\t\taccuracy_length,\t\tunderdrivable_status,\t\tprobability_motorcycle,\tprobability_bicycle,\tprobability_car,\tprobability_pedestrian,\tprobability_truck,\tprobability_undet,\tpadding[0],\tpadding[1],\tcurrent_msmt_type</TRACK_INFO>\n");
                count = 0;
             /*   if (fileCount == 0)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vcs_si_veh_ref_scale.push_back(scanindex);
                }
                if (fileCount == 1)
                {
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vcs_si_resim_ref_scale.push_back(scanindex);
                }*/
            }
            if ((p_hondasrr6p_curvi_track_data_info.CurviTracks[i].status) > 0)
            {
                if (fileCount == 0)
                {
                    scanindexRangeRef_tracker[radar_position].push_back(scanindex);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vcs_si_veh.push_back(scanindex);

                    double xposvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_posn);
                    xposvalue = hondasrr6p_round_value(xposvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos[fileCount].push_back(xposvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos_scaleplot[fileCount].push_back(xposvalue);


                    double yposvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_posn);
                    yposvalue = hondasrr6p_round_value(yposvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos[fileCount].push_back(yposvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos_scaleplot[fileCount].push_back(yposvalue);


                    double xvelvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_vel);
                    xvelvalue = hondasrr6p_round_value(xvelvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel[fileCount].push_back(xvelvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel_scaleplot[fileCount].push_back(xvelvalue);

                    double yvelvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_vel);
                    yvelvalue = hondasrr6p_round_value(yvelvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel[fileCount].push_back(yvelvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel_scaleplot[fileCount].push_back(yvelvalue);


                    double xaccvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_accel);
                    xaccvalue = hondasrr6p_round_value(xaccvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc[fileCount].push_back(xaccvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc_scaleplot[fileCount].push_back(xaccvalue);

                    double yaccvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_accel);
                    yaccvalue = hondasrr6p_round_value(yaccvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc[fileCount].push_back(yaccvalue);
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc_scaleplot[fileCount].push_back(yaccvalue);


                }

                if (fileCount == 1)
                {
                    if (!scanindexRangeRef_tracker[radar_position].empty())
                    {
                        if (inRange(scanindexRangeRef_tracker[radar_position].front(), scanindexRangeRef_tracker[radar_position].back(), scanindex))
                        {
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vcs_si_resim.push_back(scanindex);

                            double xposvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_posn);
                            xposvalue = hondasrr6p_round_value(xposvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos[fileCount].push_back(xposvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos_scaleplot[fileCount].push_back(xposvalue);

                            double yposvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_posn);
                            yposvalue = hondasrr6p_round_value(yposvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos[fileCount].push_back(yposvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos_scaleplot[fileCount].push_back(yposvalue);

                            double xvelvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_vel);
                            xvelvalue = hondasrr6p_round_value(xvelvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel[fileCount].push_back(xvelvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel_scaleplot[fileCount].push_back(xvelvalue);

                            double yvelvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_vel);
                            yvelvalue = hondasrr6p_round_value(yvelvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel[fileCount].push_back(yvelvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel_scaleplot[fileCount].push_back(yvelvalue);


                            double xaccvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_long_accel);
                            xaccvalue = hondasrr6p_round_value(xaccvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc[fileCount].push_back(xaccvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc_scaleplot[fileCount].push_back(xaccvalue);

                            double yaccvalue = flt_to_dbl(p_hondasrr6p_curvi_track_data_info.CurviTracks[i].vcs_lat_accel);
                            yaccvalue = hondasrr6p_round_value(yaccvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc[fileCount].push_back(yaccvalue);
                            (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc_scaleplot[fileCount].push_back(yaccvalue);

                        }
                    }
                }


                if (count == 0)
                {

                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_xpos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos_scaleplot[fileCount]));
                    }

                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_ypos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos_scaleplot[fileCount]));
                    }


                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_xvel[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel_scaleplot[fileCount]));
                    }

                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_ypos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel_scaleplot[fileCount]));
                    }

                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_xacc[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc_scaleplot[fileCount]));
                    }

                    if ((sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc_scaleplot[fileCount].size() != 0)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_position)->map_vcs_yacc[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc_scaleplot[fileCount]));
                    }


                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xpos_scaleplot[fileCount].clear();
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_ypos_scaleplot[fileCount].clear();

                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xvel_scaleplot[fileCount].clear();
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yvel_scaleplot[fileCount].clear();

                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_xacc_scaleplot[fileCount].clear();
                    (sptr_plotparam_honda_srr6p.get() + radar_position)->vsc_yacc_scaleplot[fileCount].clear();
                }






                /* fprintf(SIL_XML_out, "\t<TRACKS_%d>", i);
                 fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].vcs_xposn);
                 fprintf(SIL_XML_out, "\t\t %f", STLA_FLR4P_track_info.object[i].vcs_yposn);
                 fprintf(SIL_XML_out, "\t\t%d", STLA_FLR4P_track_info.object[i].f_moving);
                 fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_xvel);
                 fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_yvel);
                 fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_xaccel);
                 fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_yaccel);
                 fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].vcs_heading);*/

                 /* unsigned8_T status = STLA_FLR4P_track_info.object[i].status;
                  switch (status)
                  {
                  case 0:fprintf(SIL_XML_out, "\tINVALID\t\t\t"); break;
                  case 1:fprintf(SIL_XML_out, "\t\t\tNEW\t\t\t"); break;
                  case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
                  case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t\t"); break;
                  case 4:fprintf(SIL_XML_out, "\tUPDATED\t\t\t"); break;
                  case 5:fprintf(SIL_XML_out, "\tCOASTED\t\t\t"); break;
                  default:fprintf(SIL_XML_out, "\tINVALID\t\t\t"); break;
                  }*/
                  /* unsigned8_T object_class = STLA_FLR4P_track_info.object[i].object_class;

                   switch (object_class)
                   {
                   case 0:fprintf(SIL_XML_out, "\tUNDETERMINED\t\t\t\t\t"); break;
                   case 1:fprintf(SIL_XML_out, "\tCAR\t\t\t\t\t\t\t\t"); break;
                   case 2:fprintf(SIL_XML_out, "\tMOTORCYCLE\t\t\t\t\t\t"); break;
                   case 3:fprintf(SIL_XML_out, "\tTRUCK\t\t\t\t\t\t\t\t"); break;
                   case 4:fprintf(SIL_XML_out, "\tPEDESTRIAN\t\t\t\t\t\t"); break;
                   case 5:fprintf(SIL_XML_out, "\tPOLE\t\t\t\t\t\t\t\t\t"); break;
                   case 6:fprintf(SIL_XML_out, "\tTREE\t\t\t\t\t\t\t\t\t"); break;
                   case 7:fprintf(SIL_XML_out, "\tANIMAL\t\t\t\t\t\t\t\t"); break;
                   case 8:fprintf(SIL_XML_out, "\tGOD\t\t\t\t\t\t\t\t\t"); break;
                   case 9:fprintf(SIL_XML_out, "\tBICYCLE\t\t\t\t\t\t\t"); break;
                   case 10:fprintf(SIL_XML_out, "\tUNIDENTIFIED_VEHICLE\t"); break;
                   default:fprintf(SIL_XML_out, "\tINVALID\t\t\t\t\t\t\t"); break;
                   }*/
                   /*fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].len1);
                   fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].len2);
                   fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].wid1);
                   fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].wid2);

                   fprintf(SIL_XML_out, "\t%d", STLA_FLR4P_track_info.object[i].trkID);
                   fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].unique_id);
                   fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reducedID);
                   fprintf(SIL_XML_out, "\t%f", STLA_FLR4P_track_info.object[i].speed);
                   fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].curvature);
                   fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].tang_accel);*/
                   //for (int j = 0; j < SIX; j++)
                   //{
                   //    fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].state_variance[j]);
                   //}
                   //for (int j = 0; j < THREE; j++)
                   //{
                   //    fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].supplemental_state_covariance[j]);
                   //}
                   //fprintf(SIL_XML_out, "\t\t\t\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].confidenceLevel);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_measurement);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_cluster_created);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_track_updated);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].time_since_stage_start);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", STLA_FLR4P_track_info.object[i].ndets);
                   ////fprintf(SIL_XML_out, "\t\t\t\t%d", Flr4p_Tracker_info.object[i].num_reduced_dets);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].existence_probability);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reference_point);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].reducedStatus);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].init_scheme);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_crossing);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_oncoming);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_vehicular_trk);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_onguardrail);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].f_fast_moving);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].accuracy_width);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].accuracy_length);
                   //fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].underdrivable_status);
                   //fprintf(SIL_XML_out, "\t\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_motorcycle);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_bicycle);
                   //fprintf(SIL_XML_out, "\t\t%f", STLA_FLR4P_track_info.object[i].probability_car);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_pedestrian);
                   //fprintf(SIL_XML_out, "\t\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_truck);
                   //fprintf(SIL_XML_out, "\t\t\t%f", STLA_FLR4P_track_info.object[i].probability_undet);
                  /* for (int j = 0; j < TWO; j++)
                   {
                       fprintf(SIL_XML_out, "\t\t\t%d", STLA_FLR4P_track_info.object[i].padding[j]);
                   }
                   fprintf(SIL_XML_out, "\t\t\t\t%d", STLA_FLR4P_track_info.object[i].current_msmt_type);

                   fprintf(SIL_XML_out, "\t</TRACKS>\n");*/
            }
        }
        //if (count == 0)
        //{
        //    //fprintf(SIL_XML_out, "</TRACK_DESCRIPTION_TRACK_END_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
        //}

    }


}
void HondaSRR6pDataCollector::collect_alignment_data_honda_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{

    if (sptr_plotparam_honda_srr6p.get() != nullptr && m_radar_plugin != nullptr)
    {
        int radar_position = int(Radar_Posn);
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_hondasrr6p_align_data_info, Alignment_Data);
        

        double align_angle_az = flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle));
        align_angle_az = hondasrr6p_round_value(align_angle_az);
        double align_angle_el = flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle_elev));
        align_angle_el = hondasrr6p_round_value(align_angle_el);

        //std::cout << " \nalign_angle_az " << align_angle_az << " align_angle_el " << align_angle_el;

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_az[fileCount].push_back(align_angle_az);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_el[fileCount].push_back(align_angle_el);

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_quality_factor_az[fileCount].push_back(int(p_hondasrr6p_align_data_info.Align_Info.own_align_quality_factor));
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_quality_factor_el[fileCount].push_back(int(p_hondasrr6p_align_data_info.Align_Info.own_align_quality_factor_elev));
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_opp_align_quality_factor_el[fileCount].push_back(int(p_hondasrr6p_align_data_info.Align_Info.opp_align_quality_factor_elev));

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_numpts_el[fileCount].push_back(p_hondasrr6p_align_data_info.Align_Info.own_align_num_attempts_elev);
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_numpts_az[fileCount].push_back(p_hondasrr6p_align_data_info.Align_Info.own_align_num_attempts);

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_raw_az[fileCount].push_back(flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle_raw)));
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_ref_az[fileCount].push_back(flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle_ref)));

        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_raw_el[fileCount].push_back(flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle_raw_elev)));
        (sptr_plotparam_honda_srr6p.get() + radar_position)->vec_align_angle_ref_el[fileCount].push_back(flt_to_dbl(RAD2DEG(p_hondasrr6p_align_data_info.Align_Info.own_align_angle_ref_elev)));

        (sptr_plotparam_honda_srr6p.get() + radar_position)->alignment_scanidex[fileCount].push_back(scanindex);
    }

}

void HondaSRR6pDataCollector::CollectLCDA_FF(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
    int radar_postion = int(Radar_Posn);
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_LCDA_FF_XML1, LCDA_Output);    
    if (fileCount == 0)
    {       
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_resim_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        if (!(sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.empty())
        {
            if ((inRange((sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.front(), (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.back(), scanindex)))
            {
                if (p_Honda_LCDA_FF_XML1.f_bsw_enabled || p_Honda_LCDA_FF_XML1.f_cvw_enabled || p_Honda_LCDA_FF_XML1.f_lcda_enabled)
                {                   
                    if ((int)p_Honda_LCDA_FF_XML1.bsw_alert_left)
                    {

                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_left_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_alert_left[fileCount].push_back(p_Honda_LCDA_FF_XML1.bsw_alert_left);
                    }

                    if ((int)p_Honda_LCDA_FF_XML1.bsw_alert_right)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_right_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_alert_right[fileCount].push_back(p_Honda_LCDA_FF_XML1.bsw_alert_right);

                    }

                    if ((int)p_Honda_LCDA_FF_XML1.cvw_alert_left)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_left_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_alert_left[fileCount].push_back(p_Honda_LCDA_FF_XML1.cvw_alert_left);
                    }

                    if ((int)p_Honda_LCDA_FF_XML1.cvw_alert_right)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_right_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_alert_right[fileCount].push_back(p_Honda_LCDA_FF_XML1.cvw_alert_right);
                    }
                }
            }
       }
    }

    if (fileCount == 0)
    {        
        if (p_Honda_LCDA_FF_XML1.f_bsw_enabled || p_Honda_LCDA_FF_XML1.f_cvw_enabled || p_Honda_LCDA_FF_XML1.f_lcda_enabled)
        {           
            if ((int)p_Honda_LCDA_FF_XML1.bsw_alert_left)
            {

                (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_left_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_alert_left[fileCount].push_back(p_Honda_LCDA_FF_XML1.bsw_alert_left);
            }

            if ((int)p_Honda_LCDA_FF_XML1.bsw_alert_right)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_right_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->bsw_alert_right[fileCount].push_back(p_Honda_LCDA_FF_XML1.bsw_alert_right);

            }

            if ((int)p_Honda_LCDA_FF_XML1.cvw_alert_left)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_left_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_alert_left[fileCount].push_back(p_Honda_LCDA_FF_XML1.cvw_alert_left);
            }

            if ((int)p_Honda_LCDA_FF_XML1.cvw_alert_right)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_right_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cvw_alert_right[fileCount].push_back(p_Honda_LCDA_FF_XML1.cvw_alert_right);
            }
        }
    }

}

void HondaSRR6pDataCollector::CollectCTA_FF(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
    int radar_postion = int(Radar_Posn);
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_CTA_FF_XML1, RCTA_Output);// RCTA = CTA for HONDA
    if (fileCount == 0)
    {
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_resim_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        if (!(sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.empty())
        {
            if ((inRange((sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.front(), (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.back(), scanindex)))
            {

                if (((p_Honda_CTA_FF_XML1.f_cta_enabled) || (p_Honda_CTA_FF_XML1.f_cta_warn_left) || (p_Honda_CTA_FF_XML1.f_cta_warn_right)) == 1)
                {
                    if ((int)p_Honda_CTA_FF_XML1.f_cta_alert_left)
                    {

                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_left_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_alert_left[fileCount].push_back(int(p_Honda_CTA_FF_XML1.f_cta_alert_left));


                    }

                    if ((int)p_Honda_CTA_FF_XML1.f_cta_alert_right)
                    {

                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_right_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_alert_right[fileCount].push_back(int(p_Honda_CTA_FF_XML1.f_cta_alert_right));

                    }
                }
            }
        }
    }
    if (fileCount == 0)
    {
        if (((p_Honda_CTA_FF_XML1.f_cta_enabled) || (p_Honda_CTA_FF_XML1.f_cta_warn_left) || (p_Honda_CTA_FF_XML1.f_cta_warn_right)) == 1)
        {
            if ((int)p_Honda_CTA_FF_XML1.f_cta_alert_left)
            {

                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_left_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_alert_left[fileCount].push_back(int(p_Honda_CTA_FF_XML1.f_cta_alert_left));


            }

            if ((int)p_Honda_CTA_FF_XML1.f_cta_alert_right)
            {

                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_right_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->cta_alert_right[fileCount].push_back(int(p_Honda_CTA_FF_XML1.f_cta_alert_right));

            }
        }
    }
}

void HondaSRR6pDataCollector::CollectCED_FF(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
    int radar_postion = int(Radar_Posn);
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_Honda_CED_FF_XML1, CED_Output);
    if (fileCount == 0)
    {
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_resim_ref_scale.push_back(scanindex);
    }
    if (fileCount == 1)
    {
        if (!(sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.empty())
        {
            if ((inRange((sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.front(), (sptr_plotparam_honda_srr6p.get() + radar_postion)->vcs_si_veh_ref_scale.back(), scanindex)))
            {
                if (p_Honda_CED_FF_XML1.f_ced_enable == 1)
                {
                    if ((int)p_Honda_CED_FF_XML1.CED_alert_left)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_left_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_alert_left[fileCount].push_back(int(p_Honda_CED_FF_XML1.CED_alert_left));
                    }

                    if ((int)p_Honda_CED_FF_XML1.CED_alert_right)
                    {
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_right_si[fileCount].push_back(scanindex);
                        (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_alert_right[fileCount].push_back(int(p_Honda_CED_FF_XML1.CED_alert_right));
                    }
                }
            }
        }
    }
    if (fileCount == 0)
    {
        if (p_Honda_CED_FF_XML1.f_ced_enable == 1)
        {
            if ((int)p_Honda_CED_FF_XML1.CED_alert_left)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_left_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_alert_left[fileCount].push_back(int(p_Honda_CED_FF_XML1.CED_alert_left));
            }

            if ((int)p_Honda_CED_FF_XML1.CED_alert_right)
            {
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_right_si[fileCount].push_back(scanindex);
                (sptr_plotparam_honda_srr6p.get() + radar_postion)->ced_alert_right[fileCount].push_back(int(p_Honda_CED_FF_XML1.CED_alert_right));
            }

        }
    }
}

