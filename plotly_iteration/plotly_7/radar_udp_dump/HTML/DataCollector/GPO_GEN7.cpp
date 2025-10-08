
#include<filesystem>

#include "GPO_GEN7.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../PlotGenerator/MatplotNonScaleplot.h"
#include "../InputParser/JsonInputParser.h"

#define AF_DET_MAX_COUNT    (256U) 
#define NUMBER_OF_DETECTIONS (200)
#define PI 3.14159265359
#define RAD2DEG(x) ((x * 180.00) / PI)
#define RDD_MAX_NUM_DET    (400U) // FLR4P dets

//const unsigned float PI = 3.14159265359;
namespace fs = std::filesystem;
extern RADAR_Plugin_Interface_T* m_radar_plugin;
float32_T* array_data_ptr_gpo_gen7;

SRR7P_Detection_Stream_T SRR7p_Dets_info_GEN7 = { 0 };
SRR7P_Detection_Stream_T_Standalone SRR7p_Dets_info_GEN7_Standalone = { 0 };
SRR7P_Detection_Stream_T_SAT SRR7p_Dets_info_GEN7_Satellite = { 0 };
SRR7P_ROT_Object_Stream_T SRR7p_tracks_info_GEN7 = { 0 };
SRR7P_Alignment_Stream_T p_alignment_GEN7 = { 0 };
SRR7P_DRA_Core_Log_Data_T p_DYalignment_GEN7 = { 0 };
SRR7P_Blockage_Stream_T p_blockage_GEN7 = { 0 };
SRR7P_ID_Stream_T p_interference_GEN7 = { 0 };
SRR7P_Status_Stream_T p_status_GEN7 = { 0 };

double Gen7_round_value(double var)
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
int comparedata_gpo_gen7(const void* a, const void* b)
{
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptr_gpo_gen7[value_of_a] < array_data_ptr_gpo_gen7[value_of_b]) {
        return -1;
    }
    else if (array_data_ptr_gpo_gen7[value_of_a] == array_data_ptr_gpo_gen7[value_of_b])
    {

    }
    else
    {
        return (array_data_ptr_gpo_gen7[value_of_a] > array_data_ptr_gpo_gen7[value_of_b]);
    }
}

GpoGen7DataCollector::GpoGen7DataCollector(std::shared_ptr<PlotParameter[]>& sptr_plotparam) :sptr_plotparam_gen7(sptr_plotparam)
{
	//std::cout << "\nStlaScale1DataCollector::StlaScale1DataCollector()";	
  /*  ffile1.open("mnrdump.txt", ios::out | ios::app);
    ffile2.open("file2aligndump.txt", ios::out | ios::app);
    ffile1 << "\n AvgRm_mnr_avg ";
    ffile2 << "\n scan index # align_angle_az";
    */
   
}

GpoGen7DataCollector::~GpoGen7DataCollector()
{
	//std::cout << "\nStlaScale1DataCollector::~StlaScale1DataCollector()";
   /* ffile1.close();
    ffile2.close();*/
	
}



void GpoGen7DataCollector::collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    //std::cout << "\nStlaScale1DataCollector::collect_parsedmf4_data";
    JsonInputParser& json = JsonInputParser::getInstance();
    uint8_t strm_ver = g_pIRadarStrm->m_gen7proc_info.frame_header.streamVersion;
    if (platform == UDP_PLATFORM_SRR7_PLUS || platform == UDP_PLATFORM_SRR7_PLUS_HD || platform == UDP_PLATFORM_SRR7_E )
    {
        if (stream_num == DETECTION_STREAM)
        {

            if (strm_ver <= 21)     //for versions until V21
            {
                collect_detection_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
            }

            if (strm_ver > 21 && strm_ver < 100)    //for satellite variant 
            {
                collect_detection_data_srr7p_Satellite(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
            }
            if (strm_ver >= 100)    //Version>=100 for standalone variant
            {
                collect_detection_data_srr7p_Standalone(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
            }

        }
        if (stream_num == ROT_OBJECT_STREAM)
        {


            collect_tracker_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);

        }
        if (stream_num == DYNAMIC_ALIGNMENT_STREAM )
        {


            collect_alignment_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);

        }
        if (stream_num == RADAR_CAPABILITY_STREAM)
        {


            collect_mnrblockage_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);

        }
        if (stream_num == ID_STREAM)
        {


            collect_interference_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);

        }

            if (stream_num == STATUS_STREAM)
            {


                collect_boresightangle_data_srr7p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);

           }


        

    }
}

void GpoGen7DataCollector::generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{


}

void GpoGen7DataCollector::calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    JsonInputParser& jsonpar = JsonInputParser::getInstance();
    MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start,log2_scanindex_end;
    long long log1_duration,log2_duration;
    int tool_duration_func_inputlog;
    int log1_duration_fin{ 0 }, log2_duration_fin{ 0 };

    if ((sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.size() != 0)
    {
        log1_scanindex_start = (sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.front();
        log1_scanindex_end = (sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.back();
        log1_duration = log1_scanindex_end - log1_scanindex_start;
        log1_duration_fin = log1_duration * .05;

        if (sptr_plotparam_gen7.get() != nullptr)
        {
            jsonpar.Log1_duration = to_string(log1_duration_fin);

        }
    }

    if ((sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.size() != 0)
    {
        log2_scanindex_start = (sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.front();
        log2_scanindex_end = (sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.back();
        log2_duration = log2_scanindex_end - log2_scanindex_start;
        log2_duration_fin = log2_duration * .05;

        if (sptr_plotparam_gen7.get() != nullptr)
        {
            jsonpar.Log2_duration = to_string(log2_duration_fin);
        }
    }
   
    


  
    if (sptr_plotparam_gen7 != nullptr)
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
            jsonpar.tool_runtime_func_inputtime = "Run time not calculated as Log duration not available";
            matplotnonscale.print_reporttoolrun_timing_info(f_name, "HTML", pltfolder, reportpath, radarposition);

        }

    }
    

}

void GpoGen7DataCollector::calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    //std::cout << "\ncalculate_Logduration";  
    JsonInputParser& jsonparser = JsonInputParser::getInstance();
  

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start, log2_scanindex_end;
    long long log1_duration, log2_duration;
    double tool_duration_func_inputlog;
    long long log1_duration_fin, log2_duration_fin;


    if (sptr_plotparam_gen7.get() !=nullptr)
    {

        if ((sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.size() != 0)
        {
            log1_scanindex_start = (sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.front();
            log1_scanindex_end = (sptr_plotparam_gen7.get() + radarposition)->si_veh_ref_scale.back();
            log1_duration = log1_scanindex_end - log1_scanindex_start;
            log1_duration_fin = log1_duration * .05;

            if (sptr_plotparam_gen7.get() != nullptr)
            {
                jsonparser.Log1_duration = to_string(log1_duration_fin);
                
            }
        }
     
       
        if ((sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.size()!=0)
        {
            log2_scanindex_start = (sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.front();
            log2_scanindex_end = (sptr_plotparam_gen7.get() + radarposition)->si_resim_ref_scale.back();
            log2_duration = log2_scanindex_end - log2_scanindex_start;
            log2_duration_fin = log2_duration * .05;

            if (sptr_plotparam_gen7.get() != nullptr)
            {              
                jsonparser.Log2_duration = to_string(log2_duration_fin);
            }
        }
           

        
     
    }
    

}

double GpoGen7DataCollector::flt_to_dbl(float32_T con_value)
{
	char chr_fValue[32];//32
	sprintf(chr_fValue, "%9.8f", con_value);//9.8
	double db_val = strtod(chr_fValue, NULL);
	return db_val;
}

bool GpoGen7DataCollector::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
{
	return ((resimsi - vehiclesi_end) * (resimsi - vehiclesi_start) <= 0);
}

inline void GpoGen7DataCollector::remove_old_html_reports(const char* f_name, const char* pltfolder,std::string reportpath)
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



void GpoGen7DataCollector::collect_detection_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{


    if (m_radar_plugin != nullptr && sptr_plotparam_gen7.get() != nullptr)
    {
        m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_Dets_info_GEN7, GEN7_DETECTION_STREAM);
        int radar_position = int(Radar_Posn);

        float32_T Self_Dets[AF_MAX_NUM_DET] = { 0 };
        for (int i = 0; i < AF_MAX_NUM_DET; i++)
        {
            Self_Dets[i] = Self_Dets[i] = SRR7p_Dets_info_GEN7.af_data.ran[i];
        }
        unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
        unsigned8_T index[AF_MAX_NUM_DET] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptr_gpo_gen7 = Self_Dets;
        qsort(index, size, sizeof(*index), comparedata_gpo_gen7);


        /*print sorted self dets*/
        unsigned8_T count = 1;
        for (unsigned16_T i = 0; i < AF_MAX_NUM_DET; i++)
        {
            if ((SRR7p_Dets_info_GEN7.af_data.ran[i] != 0) && (count == 1))
            {
                if (fileCount == 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->si_veh_ref_scale.push_back(scanindex);
                    (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7.af_data.num_af_det)));
                    (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7.af_data.num_af_det)));
                    (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7.num_fp_detections)));
                    (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7.num_sp_detections)));
                    (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7.rest_count)));
                }
                if (fileCount == 1)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->si_resim_ref_scale.push_back(scanindex);
                    (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7.af_data.num_af_det)));
                    (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7.af_data.num_af_det)));
                    (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7.num_fp_detections)));
                    (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7.num_sp_detections)));
                    (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7.rest_count)));
                }
                count = 0;
            }
            if (SRR7p_Dets_info_GEN7.af_data.ran[i] != 0)
            {
                if (fileCount == 0)
                {
                    scanindexRangeRef[radar_position].push_back(scanindex);

                    double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.ran[i]);
                    if (rangevalue >= -1 && rangevalue <= 500)
                    {
                        rangevalue = Gen7_round_value(rangevalue);
                        (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                        (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                    }

                    double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.vel[i]);
                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                        (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }


                    double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.amp[i]);
                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }


                    double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.rcs[i]);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }


                    double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.snr[i]);
                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }

                    double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7.af_data.theta[i]));
                    if (azimuth >= -75 && azimuth <= 75)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                        (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }

                    double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7.af_data.phi[i]));
                    if (elevation >= -20 && elevation <= 20)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                        (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                    }

                    double az_conf_value = double(SRR7p_Dets_info_GEN7.af_data.az_conf[i]);
                    double el_conf_value = double(SRR7p_Dets_info_GEN7.af_data.el_conf[i]);
                    (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                    (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);




                        if (SRR7p_Dets_info_GEN7.af_data.f_bistatic)
                        {
                            isbistatic_count_veh++;
                          //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                        }
                      
                        if (SRR7p_Dets_info_GEN7.af_data.f_single_target) {
                            issingletarget_count_veh++;
                         //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                        }

                        if (SRR7p_Dets_info_GEN7.af_data.f_superres_target) {
                            issuperrestarget_count_veh++;
                         //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                        }

                    


                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -75 && azimuth <= 75) ||
                        elevation >= -20 && elevation <= 20)
                    {

                        (sptr_plotparam_gen7.get() + radar_position)->si_veh.push_back(scanindex);


                    }

                }
                if (fileCount == 1)
                {
                    if (!scanindexRangeRef[radar_position].empty())
                    {
                        if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                        {
                            double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.ran[i]);
                            if (rangevalue >= -1 && rangevalue <= 500)
                            {
                                rangevalue = Gen7_round_value(rangevalue);
                                (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                                (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                            }

                            double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.vel[i]);
                            if (rangeratevalue >= -30 && rangeratevalue <= 30)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                            }


                            double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.amp[i]);
                            if (ampvalue >= -100 && ampvalue <= 100)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);

                            }

                            double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.rcs[i]);
                            if (rcsvalue >= -100 && rcsvalue <= 100)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                            }

                            double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7.af_data.snr[i]);
                            if (snrvalue >= -100 && snrvalue <= 100)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                            }

                            double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7.af_data.theta[i]));
                            if (azimuth >= -75 && azimuth <= 75)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                                (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                            }

                            double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7.af_data.phi[i]));
                            if (elevation >= -20 && elevation <= 20)
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                                (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                            }

                            double az_conf_value = double(SRR7p_Dets_info_GEN7.af_data.az_conf[i]);
                            double el_conf_value = double(SRR7p_Dets_info_GEN7.af_data.el_conf[i]);
                            (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                            (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);


                            if (SRR7p_Dets_info_GEN7.af_data.f_bistatic)
                            {
                                isbistatic_count_resim++;
                                //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                            }

                            if (SRR7p_Dets_info_GEN7.af_data.f_single_target) {
                                issingletarget_count_resim++;
                                //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                            }

                            if (SRR7p_Dets_info_GEN7.af_data.f_superres_target) {
                                issuperrestarget_count_resim++;
                                //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                            }


                            if ((rangevalue >= -1 && rangevalue <= 500) ||
                                (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                (ampvalue >= -100 && ampvalue <= 100) ||
                                (snrvalue >= -100 && snrvalue <= 100) ||
                                (azimuth >= -75 && azimuth <= 75) ||
                                elevation >= -20 && elevation <= 20)
                            {

                                (sptr_plotparam_gen7.get() + radar_position)->si_resim.push_back(scanindex);

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

           //       std::cout << "\n f1 isbistatic_count_veh " << isbistatic_count_veh;
             //     std::cout << "\n f1 issingletarget_count_veh " << issingletarget_count_veh;
            //      std::cout << "\n f1 issuperrestarget_count_veh " << issuperrestarget_count_veh;

                if (isbistatic_count_veh > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_veh);
                    (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                    isbistatic_count_veh = 0;
                }

                if (issingletarget_count_veh > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_veh);
                    (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                    issingletarget_count_veh = 0;
                }

                if (issuperrestarget_count_veh > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_veh);
                    (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                    issuperrestarget_count_veh = 0;
                }

            }
            if (fileCount == 1)
            {
                if (isbistatic_count_resim > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_resim);
                    (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                    isbistatic_count_resim = 0;
                }
                if (issingletarget_count_resim > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_resim);
                    (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                    issingletarget_count_resim = 0;
                }

                if (issuperrestarget_count_resim > 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_resim);
                    (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                    issuperrestarget_count_resim = 0;
                }
            }



                if ((sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_range[fileCount][scanindex] = (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount];
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount]));
                }

                if ((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].size() != 0)
                {
                    std::vector<double>::iterator itrmax_value;


                    itrmax_value = std::max_element((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].begin(),
                        (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].end());

                    (sptr_plotparam_gen7.get() + radar_position)->range_max[fileCount].push_back(*itrmax_value);
                    (sptr_plotparam_gen7.get() + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);




                }

                (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].clear();

            }

        }

    }

void GpoGen7DataCollector::collect_detection_data_srr7p_Standalone(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {


        if (m_radar_plugin != nullptr && sptr_plotparam_gen7.get() != nullptr)
        {
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_Dets_info_GEN7_Standalone, GEN7_DETECTION_STREAM);
            int radar_position = int(Radar_Posn);

            float32_T Self_Dets[AF_MAX_NUM_DET] = { 0 };
            for (int i = 0; i < AF_MAX_NUM_DET; i++)
            {
                Self_Dets[i] = Self_Dets[i] = SRR7p_Dets_info_GEN7_Standalone.af_data.ran[i];
            }
            unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
            unsigned8_T index[AF_MAX_NUM_DET] = { 0 };
            for (unsigned8_T i = 0; i < size; i++)
            {
                index[i] = i;
            }
            array_data_ptr_gpo_gen7 = Self_Dets;
            qsort(index, size, sizeof(*index), comparedata_gpo_gen7);


            /*print sorted self dets*/
            unsigned8_T count = 1;
            for (unsigned16_T i = 0; i < AF_MAX_NUM_DET; i++)
            {
                if ((SRR7p_Dets_info_GEN7_Standalone.af_data.ran[i] != 0) && (count == 1))
                {
                    if (fileCount == 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->si_veh_ref_scale.push_back(scanindex);
                        (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7_Standalone.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.detection_log_data.num_af_stage_one)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.detection_log_data.num_af_stage_two)));
                        (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.det_list_property.rest_count)));
                    }
                    if (fileCount == 1)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->si_resim_ref_scale.push_back(scanindex);
                        (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7_Standalone.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.detection_log_data.num_af_stage_one)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.detection_log_data.num_af_stage_two)));
                        (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Standalone.det_list_property.rest_count)));
                    }
                    count = 0;
                }
                if (SRR7p_Dets_info_GEN7_Standalone.af_data.ran[i] != 0)
                {
                    if (fileCount == 0)
                    {
                        scanindexRangeRef[radar_position].push_back(scanindex);

                        double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.ran[i]);
                        if (rangevalue >= -1 && rangevalue <= 500)
                        {
                            rangevalue = Gen7_round_value(rangevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                        }

                        double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.vel[i]);
                        if (rangeratevalue >= -30 && rangeratevalue <= 30)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                        }


                        double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.amp[i]);
                        if (ampvalue >= -100 && ampvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                        }


                        double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.rcs[i]);
                        if (rcsvalue >= -100 && rcsvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                        }


                        double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.snr[i]);
                        if (snrvalue >= -100 && snrvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                        }

                        double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Standalone.af_data.theta[i]));
                        if (azimuth >= -75 && azimuth <= 75)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                            (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                        }

                        double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Standalone.af_data.phi[i]));
                        if (elevation >= -20 && elevation <= 20)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                            (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                        }

                        double az_conf_value = double(SRR7p_Dets_info_GEN7_Standalone.af_data.az_conf[i]);
                        double el_conf_value = double(SRR7p_Dets_info_GEN7_Standalone.af_data.el_conf[i]);
                        (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                        (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);




                        if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_bistatic)
                        {
                            isbistatic_count_veh++;
                            //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                        }

                        if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_single_target) {
                            issingletarget_count_veh++;
                            //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                        }

                        if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_superres_target) {
                            issuperrestarget_count_veh++;
                            //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                        }




                        if ((rangevalue >= -1 && rangevalue <= 500) ||
                            (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                            (ampvalue >= -100 && ampvalue <= 100) ||
                            (snrvalue >= -100 && snrvalue <= 100) ||
                            (azimuth >= -75 && azimuth <= 75) ||
                            elevation >= -20 && elevation <= 20)
                        {

                            (sptr_plotparam_gen7.get() + radar_position)->si_veh.push_back(scanindex);


                        }

                    }
                    if (fileCount == 1)
                    {
                        if (!scanindexRangeRef[radar_position].empty())
                        {
                            if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                            {
                                double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.ran[i]);
                                if (rangevalue >= -1 && rangevalue <= 500)
                                {
                                    rangevalue = Gen7_round_value(rangevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                                }

                                double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.vel[i]);
                                if (rangeratevalue >= -30 && rangeratevalue <= 30)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                                }


                                double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.amp[i]);
                                if (ampvalue >= -100 && ampvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);

                                }

                                double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.rcs[i]);
                                if (rcsvalue >= -100 && rcsvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                                }

                                double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Standalone.af_data.snr[i]);
                                if (snrvalue >= -100 && snrvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                                }

                                double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Standalone.af_data.theta[i]));
                                if (azimuth >= -75 && azimuth <= 75)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                                    (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                                }

                                double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Standalone.af_data.phi[i]));
                                if (elevation >= -20 && elevation <= 20)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                                    (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                                }

                                double az_conf_value = double(SRR7p_Dets_info_GEN7_Standalone.af_data.az_conf[i]);
                                double el_conf_value = double(SRR7p_Dets_info_GEN7_Standalone.af_data.el_conf[i]);
                                (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                                (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);


                                if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_bistatic)
                                {
                                    isbistatic_count_resim++;
                                    //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                                }

                                if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_single_target) {
                                    issingletarget_count_resim++;
                                    //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                                }

                                if (SRR7p_Dets_info_GEN7_Standalone.af_data.f_superres_target) {
                                    issuperrestarget_count_resim++;
                                    //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                                }


                                if ((rangevalue >= -1 && rangevalue <= 500) ||
                                    (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                    (ampvalue >= -100 && ampvalue <= 100) ||
                                    (snrvalue >= -100 && snrvalue <= 100) ||
                                    (azimuth >= -75 && azimuth <= 75) ||
                                    elevation >= -20 && elevation <= 20)
                                {

                                    (sptr_plotparam_gen7.get() + radar_position)->si_resim.push_back(scanindex);

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

                    //       std::cout << "\n f1 isbistatic_count_veh " << isbistatic_count_veh;
                      //     std::cout << "\n f1 issingletarget_count_veh " << issingletarget_count_veh;
                     //      std::cout << "\n f1 issuperrestarget_count_veh " << issuperrestarget_count_veh;

                    if (isbistatic_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                        isbistatic_count_veh = 0;
                    }

                    if (issingletarget_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                        issingletarget_count_veh = 0;
                    }

                    if (issuperrestarget_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                        issuperrestarget_count_veh = 0;
                    }

                }
                if (fileCount == 1)
                {
                    if (isbistatic_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                        isbistatic_count_resim = 0;
                    }
                    if (issingletarget_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                        issingletarget_count_resim = 0;
                    }

                    if (issuperrestarget_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                        issuperrestarget_count_resim = 0;
                    }
                }



                if ((sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_range[fileCount][scanindex] = (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount];
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount]));
                }

                if ((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].size() != 0)
                {
                    std::vector<double>::iterator itrmax_value;


                    itrmax_value = std::max_element((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].begin(),
                        (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].end());

                    (sptr_plotparam_gen7.get() + radar_position)->range_max[fileCount].push_back(*itrmax_value);
                    (sptr_plotparam_gen7.get() + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);




                }

                (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].clear();

            }

        }

    }

    void GpoGen7DataCollector::collect_detection_data_srr7p_Satellite(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {


        if (m_radar_plugin != nullptr && sptr_plotparam_gen7.get() != nullptr)
        {
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_Dets_info_GEN7_Satellite, GEN7_DETECTION_STREAM);
            int radar_position = int(Radar_Posn);

            float32_T Self_Dets[AF_MAX_NUM_DET] = { 0 };
            for (int i = 0; i < AF_MAX_NUM_DET; i++)
            {
                Self_Dets[i] = Self_Dets[i] = SRR7p_Dets_info_GEN7_Satellite.af_data.ran[i];
            }
            unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
            unsigned8_T index[AF_MAX_NUM_DET] = { 0 };
            for (unsigned8_T i = 0; i < size; i++)
            {
                index[i] = i;
            }
            array_data_ptr_gpo_gen7 = Self_Dets;
            qsort(index, size, sizeof(*index), comparedata_gpo_gen7);


            /*print sorted self dets*/
            unsigned8_T count = 1;
            for (unsigned16_T i = 0; i < AF_MAX_NUM_DET; i++)
            {
                if ((SRR7p_Dets_info_GEN7_Satellite.af_data.ran[i] != 0) && (count == 1))
                {
                    if (fileCount == 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->si_veh_ref_scale.push_back(scanindex);
                        (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7_Satellite.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.detection_log_data.num_af_stage_one)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.detection_log_data.num_af_stage_two)));
                        (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.det_list_property.rest_count)));
                    }
                    if (fileCount == 1)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->si_resim_ref_scale.push_back(scanindex);
                        (sptr_plotparam_gen7.get() + radar_position)->valid_detection_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(SRR7p_Dets_info_GEN7_Satellite.af_data.num_af_det)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_fp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.detection_log_data.num_af_stage_one)));
                        (sptr_plotparam_gen7.get() + radar_position)->num_sp_detections[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.detection_log_data.num_af_stage_two)));
                        (sptr_plotparam_gen7.get() + radar_position)->rest_count[fileCount].push_back(double((SRR7p_Dets_info_GEN7_Satellite.det_list_property.rest_count)));
                    }
                    count = 0;
                }
                if (SRR7p_Dets_info_GEN7_Satellite.af_data.ran[i] != 0)
                {
                    if (fileCount == 0)
                    {
                        scanindexRangeRef[radar_position].push_back(scanindex);

                        double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.ran[i]);
                        if (rangevalue >= -1 && rangevalue <= 500)
                        {
                            rangevalue = Gen7_round_value(rangevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                        }

                        double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.vel[i]);
                        if (rangeratevalue >= -30 && rangeratevalue <= 30)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                            (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                        }


                        double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.amp[i]);
                        if (ampvalue >= -100 && ampvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                        }


                        double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.rcs[i]);
                        if (rcsvalue >= -100 && rcsvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                        }


                        double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.snr[i]);
                        if (snrvalue >= -100 && snrvalue <= 100)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                            (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                        }

                        double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Satellite.af_data.theta[i]));
                        if (azimuth >= -75 && azimuth <= 75)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                            (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                        }

                        double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Satellite.af_data.phi[i]));
                        if (elevation >= -20 && elevation <= 20)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                            (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                        }

                        double az_conf_value = double(SRR7p_Dets_info_GEN7_Satellite.af_data.az_conf[i]);
                        double el_conf_value = double(SRR7p_Dets_info_GEN7_Satellite.af_data.el_conf[i]);
                        (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                        (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);




                        if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_bistatic)
                        {
                            isbistatic_count_veh++;
                            //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                        }

                        if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_single_target) {
                            issingletarget_count_veh++;
                            //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                        }

                        if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_superres_target) {
                            issuperrestarget_count_veh++;
                            //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                        }




                        if ((rangevalue >= -1 && rangevalue <= 500) ||
                            (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                            (ampvalue >= -100 && ampvalue <= 100) ||
                            (snrvalue >= -100 && snrvalue <= 100) ||
                            (azimuth >= -75 && azimuth <= 75) ||
                            elevation >= -20 && elevation <= 20)
                        {

                            (sptr_plotparam_gen7.get() + radar_position)->si_veh.push_back(scanindex);


                        }

                    }
                    if (fileCount == 1)
                    {
                        if (!scanindexRangeRef[radar_position].empty())
                        {
                            if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                            {
                                double rangevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.ran[i]);
                                if (rangevalue >= -1 && rangevalue <= 500)
                                {
                                    rangevalue = Gen7_round_value(rangevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].push_back(rangevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                                }

                                double rangeratevalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.vel[i]);
                                if (rangeratevalue >= -30 && rangeratevalue <= 30)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                                }


                                double ampvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.amp[i]);
                                if (ampvalue >= -100 && ampvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);

                                }

                                double rcsvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.rcs[i]);
                                if (rcsvalue >= -100 && rcsvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                                }

                                double snrvalue = flt_to_dbl(SRR7p_Dets_info_GEN7_Satellite.af_data.snr[i]);
                                if (snrvalue >= -100 && snrvalue <= 100)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                                    (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                                }

                                double azimuth = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Satellite.af_data.theta[i]));
                                if (azimuth >= -75 && azimuth <= 75)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                                    (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                                }

                                double elevation = flt_to_dbl(RAD2DEG(SRR7p_Dets_info_GEN7_Satellite.af_data.phi[i]));
                                if (elevation >= -20 && elevation <= 20)
                                {
                                    (sptr_plotparam_gen7.get() + radar_position)->elevation[fileCount].push_back(elevation);
                                    (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                                }

                                double az_conf_value = double(SRR7p_Dets_info_GEN7_Satellite.af_data.az_conf[i]);
                                double el_conf_value = double(SRR7p_Dets_info_GEN7_Satellite.af_data.el_conf[i]);
                                (sptr_plotparam_gen7.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                                (sptr_plotparam_gen7.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);


                                if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_bistatic)
                                {
                                    isbistatic_count_resim++;
                                    //  (sptr_plotparam_gen7.get() + radar_position)->cum_isbistatic_count_veh++;
                                }

                                if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_single_target) {
                                    issingletarget_count_resim++;
                                    //  (sptr_plotparam_gen7.get() + radar_position)->cum_issingletarget_count_resim++;
                                }

                                if (SRR7p_Dets_info_GEN7_Satellite.af_data.f_superres_target) {
                                    issuperrestarget_count_resim++;
                                    //   (sptr_plotparam_gen7.get() + radar_position)->cum_issuperrestarget_count_veh++;
                                }


                                if ((rangevalue >= -1 && rangevalue <= 500) ||
                                    (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                    (ampvalue >= -100 && ampvalue <= 100) ||
                                    (snrvalue >= -100 && snrvalue <= 100) ||
                                    (azimuth >= -75 && azimuth <= 75) ||
                                    elevation >= -20 && elevation <= 20)
                                {

                                    (sptr_plotparam_gen7.get() + radar_position)->si_resim.push_back(scanindex);

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

                    //       std::cout << "\n f1 isbistatic_count_veh " << isbistatic_count_veh;
                      //     std::cout << "\n f1 issingletarget_count_veh " << issingletarget_count_veh;
                     //      std::cout << "\n f1 issuperrestarget_count_veh " << issuperrestarget_count_veh;

                    if (isbistatic_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                        isbistatic_count_veh = 0;
                    }

                    if (issingletarget_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                        issingletarget_count_veh = 0;
                    }

                    if (issuperrestarget_count_veh > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_veh);
                        (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                        issuperrestarget_count_veh = 0;
                    }

                }
                if (fileCount == 1)
                {
                    if (isbistatic_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isBistatic[fileCount].push_back(isbistatic_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->bistatic_scanindex[fileCount].push_back(scanindex);
                        isbistatic_count_resim = 0;
                    }
                    if (issingletarget_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->isSingleTarget[fileCount].push_back(issingletarget_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->issingletarget_scanindex[fileCount].push_back(scanindex);
                        issingletarget_count_resim = 0;
                    }

                    if (issuperrestarget_count_resim > 0)
                    {
                        (sptr_plotparam_gen7.get() + radar_position)->super_res_target[fileCount].push_back(issuperrestarget_count_resim);
                        (sptr_plotparam_gen7.get() + radar_position)->superrestarget_scanindex[fileCount].push_back(scanindex);
                        issuperrestarget_count_resim = 0;
                    }
                }



                if ((sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_range[fileCount][scanindex] = (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount];
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount]));
                }
                if ((sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].size() != 0)
                {
                    (sptr_plotparam_gen7.get() + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount]));
                }

                if ((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].size() != 0)
                {
                    std::vector<double>::iterator itrmax_value;


                    itrmax_value = std::max_element((sptr_plotparam_gen7.get() + radar_position)->range[fileCount].begin(),
                        (sptr_plotparam_gen7.get() + radar_position)->range[fileCount].end());

                    (sptr_plotparam_gen7.get() + radar_position)->range_max[fileCount].push_back(*itrmax_value);
                    (sptr_plotparam_gen7.get() + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);




                }

                (sptr_plotparam_gen7.get() + radar_position)->range_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->range_rate_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->azimuth_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->elevation_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->rcs_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->amp_scaleplot[fileCount].clear();
                (sptr_plotparam_gen7.get() + radar_position)->snr_scaleplot[fileCount].clear();

            }

        }

    }

void GpoGen7DataCollector::collect_tracker_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
  {
    m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&SRR7p_tracks_info_GEN7, GEN7_ROT_OBJECT_STREAM);
    int radar_position = int(Radar_Posn);
    
    DVSU_RECORD_T record = { 0 };

    if (custId == PLATFORM_GEN7)
    {
        /*sort by range*/
        float32_T Track_Objs[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
        for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
        {
            Track_Objs[i] = SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yposn;
        }
        unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
        unsigned8_T index[NUMBER_OF_PARTNER_DETECTIONS_L] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptr_gpo_gen7 = Track_Objs;
        qsort(index, size, sizeof(*index), comparedata_gpo_gen7);

        /*print sorted Opp dets*/
        unsigned8_T count = 1;
        for (unsigned8_T i = 0; i < NUMBER_OF_PARTNER_DETECTIONS_L; i++)
        {
            /*check for valid Track status level*/
            if (((SRR7p_tracks_info_GEN7.rot_output.obj[index[i]].status) > 0) && (count == 1))
            {
                count = 0;
            }
            if ((SRR7p_tracks_info_GEN7.rot_output.obj[index[i]].status) > 0)
            {
               
                    if (fileCount == 0)
                    {
                        scanindexRangeRef_tracker[radar_position].push_back(scanindex);
                        (sptr_plotparam_gen7.get() + radar_position)->vcs_si_veh.push_back(scanindex);

                        double xposvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xposn);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos[fileCount].push_back(xposvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos_scaleplot[fileCount].push_back(xposvalue);


                        double yposvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yposn);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos[fileCount].push_back(yposvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos_scaleplot[fileCount].push_back(yposvalue);


                        double xvelvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xvel);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel[fileCount].push_back(xvelvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel_scaleplot[fileCount].push_back(xvelvalue);

                        double yvelvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yvel);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel[fileCount].push_back(yvelvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel_scaleplot[fileCount].push_back(yvelvalue);


                        double xaccvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xaccel);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc[fileCount].push_back(xaccvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc_scaleplot[fileCount].push_back(xaccvalue);

                        double yaccvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yaccel);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc[fileCount].push_back(yaccvalue);
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc_scaleplot[fileCount].push_back(yaccvalue);


                    }

                    if (fileCount == 1)
                    {
                        if (!scanindexRangeRef_tracker[radar_position].empty())
                        {
                            if (inRange(scanindexRangeRef_tracker[radar_position].front(), scanindexRangeRef_tracker[radar_position].back(), scanindex))
                            {
                                (sptr_plotparam_gen7.get() + radar_position)->vcs_si_resim.push_back(scanindex);

                                double xposvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xposn);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos[fileCount].push_back(xposvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos_scaleplot[fileCount].push_back(xposvalue);

                                double yposvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yposn);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos[fileCount].push_back(yposvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos_scaleplot[fileCount].push_back(yposvalue);

                                double xvelvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xvel);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel[fileCount].push_back(xvelvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel_scaleplot[fileCount].push_back(xvelvalue);

                                double yvelvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yvel);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel[fileCount].push_back(yvelvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel_scaleplot[fileCount].push_back(yvelvalue);


                                double xaccvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_xaccel);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc[fileCount].push_back(xaccvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc_scaleplot[fileCount].push_back(xaccvalue);

                                double yaccvalue = flt_to_dbl(SRR7p_tracks_info_GEN7.rot_output.obj[i].vcs_yaccel);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc[fileCount].push_back(yaccvalue);
                                (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc_scaleplot[fileCount].push_back(yaccvalue);

                            }
                        }
                    }


                    if (count == 0)
                    {

                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_xpos_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_xpos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos_scaleplot[fileCount]));
                        }

                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_ypos_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_ypos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos_scaleplot[fileCount]));
                        }


                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_xvel_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_xvel[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel_scaleplot[fileCount]));
                        }

                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_yvel_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_ypos[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel_scaleplot[fileCount]));
                        }

                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_xacc_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_xacc[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc_scaleplot[fileCount]));
                        }

                        if ((sptr_plotparam_gen7.get() + radar_position)->vsc_yacc_scaleplot[fileCount].size() != 0)
                        {
                            (sptr_plotparam_gen7.get() + radar_position)->map_vcs_yacc[fileCount].insert(pair<double, std::vector<double>>(scanindex, (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc_scaleplot[fileCount]));
                        }


                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xpos_scaleplot[fileCount].clear();
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_ypos_scaleplot[fileCount].clear();

                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xvel_scaleplot[fileCount].clear();
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yvel_scaleplot[fileCount].clear();

                        (sptr_plotparam_gen7.get() + radar_position)->vsc_xacc_scaleplot[fileCount].clear();
                        (sptr_plotparam_gen7.get() + radar_position)->vsc_yacc_scaleplot[fileCount].clear();
                    }

                }
            }
           
        }


    }

    void  GpoGen7DataCollector::collect_boresightangle_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {
        if (sptr_plotparam_gen7.get() != nullptr && m_radar_plugin != nullptr)
        {
            int radar_position = int(Radar_Posn);
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_status_GEN7, GEN7_STATUS_STREAM);

            boresight_el_angle = flt_to_dbl((p_status_GEN7.sensor_mount_ornt_pitch) * 57.2958F)*-1.0f;
            boresight_az_angle = flt_to_dbl((p_status_GEN7.sensor_mount_ornt_yaw) * 57.2958F)* -1.0f;
           

        }


    }



    void GpoGen7DataCollector::collect_alignment_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {

        if (sptr_plotparam_gen7.get() != nullptr && m_radar_plugin != nullptr)
        {
            int radar_position = int(Radar_Posn);
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_DYalignment_GEN7, GEN7_DYNAMIC_ALIGNMENT_STREAM);

            //vehicle data
            (sptr_plotparam_gen7.get() + radar_position)->yaw_rate[fileCount].push_back(static_cast<double>(p_DYalignment_GEN7.raw_yawrate));
            (sptr_plotparam_gen7.get() + radar_position)->abs_speed[fileCount].push_back(static_cast<double>((p_DYalignment_GEN7.raw_speed)) * 3.6);
            (sptr_plotparam_gen7.get() + radar_position)->scanidex[fileCount].push_back(scanindex);

            //alignment data
           double  align_angle_az = flt_to_dbl((p_DYalignment_GEN7.vacs_boresight_az_estimated));//RAD2DEG()
           double  align_angle_el = flt_to_dbl((p_DYalignment_GEN7.vacs_boresight_el_estimated));
           
            double align_angle_az_initial = flt_to_dbl((p_DYalignment_GEN7.vacs_boresight_az_initial));
            double align_angle_el_initial = flt_to_dbl((p_DYalignment_GEN7.vacs_boresight_el_initial));

            double  az_misalign_angle = align_angle_az - boresight_az_angle;
            double el_misalign_angle = align_angle_el - boresight_el_angle;

            (sptr_plotparam_gen7.get() + radar_position)->vec_align_angle_az_initial[fileCount].push_back(align_angle_az_initial);
            (sptr_plotparam_gen7.get() + radar_position)->vec_align_angle_el_initial[fileCount].push_back(align_angle_el_initial);
            (sptr_plotparam_gen7.get() + radar_position)->vec_align_angle_az[fileCount].push_back(align_angle_az);
            (sptr_plotparam_gen7.get() + radar_position)->vec_align_angle_el[fileCount].push_back(align_angle_el);

            (sptr_plotparam_gen7.get() + radar_position)->vec_misalign_angle_az[fileCount].push_back(az_misalign_angle);
            (sptr_plotparam_gen7.get() + radar_position)->vec_misalign_angle_el[fileCount].push_back(el_misalign_angle);

            (sptr_plotparam_gen7.get() + radar_position)->alignment_scanidex[fileCount].push_back(scanindex);
        }

    }

    void GpoGen7DataCollector::collect_mnrblockage_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {
        if (sptr_plotparam_gen7.get() != nullptr && m_radar_plugin != nullptr)
        {
            int radar_position = int(Radar_Posn);
            uint8_t blockage_threshold[4]{};         
          
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_blockage_GEN7, GEN7_BLOCKAGE_STREAM);

            for (int i = 0; i < 4; i++)
            {
                 blockage_threshold[i] = (p_blockage_GEN7.blockage_th[i]);
                 (sptr_plotparam_gen7.get() + radar_position)->blockage_thres[fileCount].push_back(blockage_threshold[i]);
            }
            (sptr_plotparam_gen7.get() + radar_position)->map_mnr[fileCount][scanindex] = (sptr_plotparam_gen7.get() + radar_position)->blockage_thres[fileCount];
           
            
            double blockage_status = flt_to_dbl(p_blockage_GEN7.blockage_status);
            double blockage_active = flt_to_dbl(p_blockage_GEN7.blockage_active);

            (sptr_plotparam_gen7.get() + radar_position)->blockage_status[fileCount].push_back(blockage_status);
            (sptr_plotparam_gen7.get() + radar_position)->blockage_active[fileCount].push_back(blockage_active);
           
            (sptr_plotparam_gen7.get() + radar_position)->mnr_scanidex[fileCount].push_back(scanindex);
        }

    }

    void GpoGen7DataCollector::collect_interference_data_srr7p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
    {
        if (sptr_plotparam_gen7.get() != nullptr && m_radar_plugin != nullptr)
        {
            int radar_position = int(Radar_Posn);
            m_radar_plugin->Gen7RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_interference_GEN7, GEN7_ID_STREAM);

         
            double detected_interference = flt_to_dbl(p_interference_GEN7.interference_detected);


            (sptr_plotparam_gen7.get() + radar_position)->interference_detected[fileCount].push_back(detected_interference);

            (sptr_plotparam_gen7.get() + radar_position)->interference_scanidex[fileCount].push_back(scanindex);
        }

    }



