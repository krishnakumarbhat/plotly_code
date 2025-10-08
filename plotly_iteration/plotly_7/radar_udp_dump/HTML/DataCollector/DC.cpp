
#include<filesystem>

#include "DC.h"
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
float32_T* array_data_ptr_dc;

DETECTIONS_T Detdata_dc = { 0 };
VEHICLE_DATA_T p_veh_dc = { 0 };

double round_value_dc(double var)
{
    
    double value = floor(var * 10) / 10;
    return value;
}
int comparedata_dc(const void* a, const void* b)
{
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptr_dc[value_of_a] < array_data_ptr_dc[value_of_b]) {
        return -1;
    }
    else if (array_data_ptr_dc[value_of_a] == array_data_ptr_dc[value_of_b])
    {

    }
    else
    {
        return (array_data_ptr_dc[value_of_a] > array_data_ptr_dc[value_of_b]);
    }
}

DCDataCollector::DCDataCollector(std::shared_ptr<PlotParameter[]> &sptr_plotparam):sptr_plotparam_dc(sptr_plotparam)
{
	

    
}

DCDataCollector::~DCDataCollector()
{
	
	
}



void DCDataCollector::collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
	

  /*  if (fileCount == 0)
    {
        std::cout << std::endl << "\n DCDataCollector::collect_parsedmf4_data";
    }
    if (fileCount == 1)
    {
        std::cout << std::endl << "\n DCDataCollector::collect_parsedmf4_data";
    }*/

    JsonInputParser& json = JsonInputParser::getInstance();
	
	
		if (stream_num == DSPACE_CUSTOMER_DATA)
		{

			if (custId == BMW_SP25_L2)
			{

         
                collect_vehicleinfo(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
                collect_detection_data_dc(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
               
               
			}
			
        
			
		}

	
}


void DCDataCollector::generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{


}

void DCDataCollector::calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    JsonInputParser& jsonpar = JsonInputParser::getInstance();
    MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start,log2_scanindex_end;
    long long log1_duration,log2_duration;
    int tool_duration_func_inputlog;
    int log1_duration_fin{ 0 }, log2_duration_fin{ 0 };

    if ((sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.size() != 0)
    {
        log1_scanindex_start = (sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.front();
        log1_scanindex_end = (sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.back();
        log1_duration = log1_scanindex_end - log1_scanindex_start;
        log1_duration_fin = log1_duration * .05;

        if (sptr_plotparam_dc.get() != nullptr)
        {
            jsonpar.Log1_duration = to_string(log1_duration_fin);

        }
    }

    if ((sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.size() != 0)
    {
        log2_scanindex_start = (sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.front();
        log2_scanindex_end = (sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.back();
        log2_duration = log2_scanindex_end - log2_scanindex_start;
        log2_duration_fin = log2_duration * .05;

        if (sptr_plotparam_dc.get() != nullptr)
        {
            jsonpar.Log2_duration = to_string(log2_duration_fin);
        }
    }
   
    


  
    if (sptr_plotparam_dc != nullptr)
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

void DCDataCollector::calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    //std::cout << "\ncalculate_Logduration";  
    JsonInputParser& jsonparser = JsonInputParser::getInstance();
  

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start, log2_scanindex_end;
    long long log1_duration, log2_duration;
    double tool_duration_func_inputlog;
    long long log1_duration_fin, log2_duration_fin;


    if (sptr_plotparam_dc.get() !=nullptr)
    {

        if ((sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.size() != 0)
        {
            log1_scanindex_start = (sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.front();
            log1_scanindex_end = (sptr_plotparam_dc.get() + radarposition)->si_veh_ref_scale.back();
            log1_duration = log1_scanindex_end - log1_scanindex_start;
            log1_duration_fin = log1_duration * .05;

            if (sptr_plotparam_dc.get() != nullptr)
            {
                jsonparser.Log1_duration = to_string(log1_duration_fin);
                
            }
        }
     
       
        if ((sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.size()!=0)
        {
            log2_scanindex_start = (sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.front();
            log2_scanindex_end = (sptr_plotparam_dc.get() + radarposition)->si_resim_ref_scale.back();
            log2_duration = log2_scanindex_end - log2_scanindex_start;
            log2_duration_fin = log2_duration * .05;

            if (sptr_plotparam_dc.get() != nullptr)
            {              
                jsonparser.Log2_duration = to_string(log2_duration_fin);
            }
        }
           

        
     
    }
    

}



double DCDataCollector::flt_to_dbl(float32_T con_value)
{
	char chr_fValue[32];//32
	sprintf(chr_fValue, "%9.8f", con_value);//9.8
	double db_val = strtod(chr_fValue, NULL);
	return db_val;
}

bool DCDataCollector::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
{
	return ((resimsi - vehiclesi_end) * (resimsi - vehiclesi_start) <= 0);
}

inline void DCDataCollector::remove_old_html_reports(const char* f_name, const char* pltfolder,std::string reportpath)
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











void DCDataCollector::collect_vehicleinfo(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    int radar_position = int(Radar_Posn);

    if (radar_position == 0)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_RL);
    }
    if (radar_position == 1)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_RR);
    }
    if (radar_position == 2)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FR);
    }
    if (radar_position == 3)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FL);
    }
    if (radar_position == 5)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FC);
    }

    m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_dc, DSPACE_VEH);

    

     (sptr_plotparam_dc.get() + radar_position)->yaw_rate[fileCount].push_back(static_cast<double>(p_veh_dc.yawrate));
     (sptr_plotparam_dc.get() + radar_position)->abs_speed[fileCount].push_back(static_cast<double>((p_veh_dc.abs_speed)) * 3.6);
     (sptr_plotparam_dc.get() + radar_position)->scanidex[fileCount].push_back(Detdata_dc.det_info.ScanIndex);
    

   
  

   
}








void DCDataCollector::collect_detection_data_dc(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    
   
        int radar_position = int(Radar_Posn);
  
        if (radar_position == 0)
        {
            m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_RL);
        }
        if (radar_position == 1)
        {
            m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_RR);
        }
        if (radar_position == 2)
        {
            m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FR);
        }
        if (radar_position == 3)
        {
            m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FL);
        }
        if (radar_position == 5)
        {
            m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Detdata_dc, DSPACE_DETECTION_FC);
        }


   

        ////detections
        if(fileCount == 0)
        {
            unsigned8_T count = 1;

            for (unsigned16_T i = 0; i < 250; i++)
            {
                if ((Detdata_dc.det[i].range != 0) && (count == 1))
                {
                    //std::cout <<"\n F0 scan index " <<Detdata_dc.det_info.ScanIndex <<" Radar " << GetRadarPosName(Radar_Posn);
                    (sptr_plotparam_dc.get() + radar_position)->si_veh_ref_scale.push_back(Detdata_dc.det_info.ScanIndex);           
                    count = 0;
                }

                if (Detdata_dc.det[i].range != 0)
                {
                    //std::cout << std::endl << i <<" Range " << Detdata_dc.det[i].range;

                    scanindexRangeRef[radar_position].push_back(Detdata_dc.det_info.ScanIndex);

                    double rangevalue = flt_to_dbl(Detdata_dc.det[i].range);
                    if (rangevalue >= -1 && rangevalue <= 500)
                    {
                        
                        rangevalue = round_value_dc(rangevalue);
                        (sptr_plotparam_dc.get() + radar_position)->range[fileCount].push_back(rangevalue);
                        (sptr_plotparam_dc.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                    }

                    double rangeratevalue = flt_to_dbl(Detdata_dc.det[i].range_rate);
                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        rangeratevalue = round_value_dc(rangeratevalue);
                        (sptr_plotparam_dc.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                        (sptr_plotparam_dc.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }

                    double ampvalue = flt_to_dbl(Detdata_dc.det[i].amplitude);
                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        ampvalue = round_value_dc(ampvalue);
                        (sptr_plotparam_dc.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                        (sptr_plotparam_dc.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }

                    double rcsvalue = flt_to_dbl(Detdata_dc.det[i].std_rcs);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        rcsvalue = round_value_dc(rcsvalue);
                        (sptr_plotparam_dc.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                        (sptr_plotparam_dc.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }

                    double snrvalue = flt_to_dbl(Detdata_dc.det[i].detection_snr);
                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        snrvalue = round_value_dc(snrvalue);
                        (sptr_plotparam_dc.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                        (sptr_plotparam_dc.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }

                    double azimuth = flt_to_dbl(RAD2DEG(Detdata_dc.det[i].azimuth));
                    if (azimuth >= -90 && azimuth <= 90)
                    {
                        azimuth = round_value_dc(azimuth);
                        (sptr_plotparam_dc.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                        (sptr_plotparam_dc.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }

                    double elevation = flt_to_dbl(RAD2DEG(Detdata_dc.det[i].elevation));
                    if (elevation >= -40 && elevation <= 40)
                    {
                        elevation = round_value_dc(elevation);
                        (sptr_plotparam_dc.get() + radar_position)->elevation[fileCount].push_back(elevation);
                        (sptr_plotparam_dc.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                    }

                    double az_conf_value = double(Detdata_dc.det[i].azimuth_confidence);
                    double el_conf_value = double(Detdata_dc.det[i].elevation_confidence);
                    (sptr_plotparam_dc.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                    (sptr_plotparam_dc.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);


                    if (int(Detdata_dc.det[i].bistatic))
                    {
                        isbistatic_count_veh++;
                        (sptr_plotparam_dc.get() + radar_position)->cum_isbistatic_count_veh++;
                    }
                   
                    if (int(Detdata_dc.det[i].super_res_target)) {
                        issuperrestarget_count_veh++;
                        (sptr_plotparam_dc.get() + radar_position)->cum_issuperrestarget_count_veh++;
                    }

                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -90 && azimuth <= 90) ||
                        elevation >= -40 && elevation <= 40)
                    {

                        (sptr_plotparam_dc.get() + radar_position)->si_veh.push_back(Detdata_dc.det_info.ScanIndex);

                    }

                    
                }

            }

            if (count == 0)
            {
                //std::cout << "\n /F0 scan index " << Detdata_dc.det_info.ScanIndex << " Radar " << GetRadarPosName(Radar_Posn);
            }
        }



        if (fileCount == 1)
        {


            if (!scanindexRangeRef[radar_position].empty())
            {
                if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), Detdata_dc.det_info.ScanIndex))
                {
                    unsigned8_T count = 1;

                    for (unsigned16_T i = 251; i < 499; i++)
                    {
                        if ((Detdata_dc.det[i].range != 0) && (count == 1))
                        {
                            //std::cout << "\n F1 scan index " << Detdata_dc.det_info.ScanIndex << " Radar " << GetRadarPosName(Radar_Posn);
                            (sptr_plotparam_dc.get() + radar_position)->si_resim_ref_scale.push_back(Detdata_dc.det_info.ScanIndex);
                            count = 0;
                        }

                        if (Detdata_dc.det[i].range != 0)
                        {
                            //std::cout << std::endl << i<< " Range " << Detdata_dc.det[i].range;

                            double rangevalue = flt_to_dbl(Detdata_dc.det[i].range);
                            if (rangevalue >= -1 && rangevalue <= 500)
                            {

                                rangevalue = round_value_dc(rangevalue);
                                (sptr_plotparam_dc.get() + radar_position)->range[fileCount].push_back(rangevalue);
                                (sptr_plotparam_dc.get() + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                            }

                            double rangeratevalue = flt_to_dbl(Detdata_dc.det[i].range_rate);
                            if (rangeratevalue >= -30 && rangeratevalue <= 30)
                            {
                                rangeratevalue = round_value_dc(rangeratevalue);
                                (sptr_plotparam_dc.get() + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                (sptr_plotparam_dc.get() + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                            }

                            double ampvalue = flt_to_dbl(Detdata_dc.det[i].amplitude);
                            if (ampvalue >= -100 && ampvalue <= 100)
                            {
                                ampvalue = round_value_dc(ampvalue);
                                (sptr_plotparam_dc.get() + radar_position)->amp[fileCount].push_back(ampvalue);
                                (sptr_plotparam_dc.get() + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                            }

                            double rcsvalue = flt_to_dbl(Detdata_dc.det[i].std_rcs);
                            if (rcsvalue >= -100 && rcsvalue <= 100)
                            {
                                rcsvalue = round_value_dc(rcsvalue);
                                (sptr_plotparam_dc.get() + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                (sptr_plotparam_dc.get() + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                            }

                            double snrvalue = flt_to_dbl(Detdata_dc.det[i].detection_snr);
                            if (snrvalue >= -100 && snrvalue <= 100)
                            {
                                snrvalue = round_value_dc(snrvalue);
                                (sptr_plotparam_dc.get() + radar_position)->snr[fileCount].push_back(snrvalue);
                                (sptr_plotparam_dc.get() + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                            }

                            double azimuth = flt_to_dbl(RAD2DEG(Detdata_dc.det[i].azimuth));
                            if (azimuth >= -90 && azimuth <= 90)
                            {
                                azimuth = round_value_dc(azimuth);
                                (sptr_plotparam_dc.get() + radar_position)->azimuth[fileCount].push_back(azimuth);
                                (sptr_plotparam_dc.get() + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                            }

                            double elevation = flt_to_dbl(RAD2DEG(Detdata_dc.det[i].elevation));
                            if (elevation >= -40 && elevation <= 40)
                            {
                                elevation = round_value_dc(elevation);
                                (sptr_plotparam_dc.get() + radar_position)->elevation[fileCount].push_back(elevation);
                                (sptr_plotparam_dc.get() + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                            }

                            double az_conf_value = double(Detdata_dc.det[i].azimuth_confidence);
                            double el_conf_value = double(Detdata_dc.det[i].elevation_confidence);
                            (sptr_plotparam_dc.get() + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                            (sptr_plotparam_dc.get() + radar_position)->el_conf[fileCount].push_back(el_conf_value);


                            if (int(Detdata_dc.det[i].bistatic))
                            {
                                isbistatic_count_veh++;
                                (sptr_plotparam_dc.get() + radar_position)->cum_isbistatic_count_veh++;
                            }

                            if (int(Detdata_dc.det[i].super_res_target)) {
                                issuperrestarget_count_veh++;
                                (sptr_plotparam_dc.get() + radar_position)->cum_issuperrestarget_count_veh++;
                            }


                            if ((rangevalue >= -1 && rangevalue <= 500) ||
                                (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                (ampvalue >= -100 && ampvalue <= 100) ||
                                (snrvalue >= -100 && snrvalue <= 100) ||
                                (azimuth >= -90 && azimuth <= 90) ||
                                elevation >= -40 && elevation <= 40)
                            {

                                (sptr_plotparam_dc.get() + radar_position)->si_resim.push_back(Detdata_dc.det_info.ScanIndex);

                            }



                        }

                    }

                    if (count == 0)
                    {
                        //std::cout << "\n /F1 scan index " << Detdata_dc.det_info.ScanIndex << " Radar " << GetRadarPosName(Radar_Posn);
                    }

                }
            }
        }


        
   
      
    
            /*if (Detdata_dc.det[i].range != 0)
            {
                fprintf(SIL_XML_out, "\t<DETECTION_%d>", i);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].range);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].range_rate);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].azimuth);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].elevation);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].detection_snr);
                fprintf(SIL_XML_out, "\t\t\t%d", Detdata.det[i].det_id);
                fprintf(SIL_XML_out, "\t%d", Detdata.det[i].valid);
                fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].amplitude);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_rcs);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_azimuth);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].existence_probability);
                fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].multi_target_probability);
                fprintf(SIL_XML_out, "\t\t\t\t%f", Detdata.det[i].Mixer_Bias);
                fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].std_range);
                fprintf(SIL_XML_out, "\t%f", Detdata.det[i].std_range_rate);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].std_elevation);
                fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].classification);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].ambiguity_id);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].rdd_fp_det_idx);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].bistatic);
                fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].host_veh_clutter);
                fprintf(SIL_XML_out, "\t\t%f", Detdata.det[i].azimuth_confidence);
                fprintf(SIL_XML_out, "\t\t\t%f", Detdata.det[i].super_res_target);
                fprintf(SIL_XML_out, "\t\t</Detection>\n");
            }*/
       
       
    

}















void DCDataCollector::collect_tracker_data_dc(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
 


}



