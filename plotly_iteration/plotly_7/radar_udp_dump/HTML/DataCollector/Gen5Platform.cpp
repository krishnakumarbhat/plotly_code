
#include<filesystem>

#include "Gen5Platform.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../PlotGenerator/MatplotNonScaleplot.h"


#define AF_DET_MAX_COUNT    (256U) 
#define NUMBER_OF_DETECTIONS (200)
#define PI 3.14159265359
#define RAD2DEG(x) ((x * 180.00) / PI)

//const unsigned float PI = 3.14159265359;
namespace fs = std::filesystem;
extern RADAR_Plugin_Interface_T* m_radar_plugin;
float32_T* array_data_ptr_gpo;

//STLA_SRR6P_Alignment_Log_Data_T p_veh_stla_srr6p_s1 = { 0 };
//STLA_FLR4_Alignment_Log_Data_T p_veh_stla_flr4_s1 = { 0 };
//STLA_C2_Diagnostics_Logging_Data_T p_c2timing_stla_srr6p_s1 = { 0 };
//STLA_Detection_Log_Data_T p_selfdetection_flr4p_gpo = { 0 };
//STLA_Detection_Log_Data_T p_selfdetection_flr4p_gpo = { 0 };
Detection_Log_Data_FLR4P_T p_selfdetection_flr4p_gpo = { 0 };
Tracker_Log_Data_FLR4P_T p_tracker_flr4p_gpo = { 0 };


int comparedata_gpo(const void* a, const void* b)
{
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptr_gpo[value_of_a] < array_data_ptr_gpo[value_of_b]) {
        return -1;
    }
    else if (array_data_ptr_gpo[value_of_a] == array_data_ptr_gpo[value_of_b])
    {

    }
    else
    {
        return (array_data_ptr_gpo[value_of_a] > array_data_ptr_gpo[value_of_b]);
    }
}

Gen5PlatformDataCollector::Gen5PlatformDataCollector()
{
	//std::cout << "\nStlaScale1DataCollector::StlaScale1DataCollector()";
	
	pvehicle_info = new Vehicle_Plot_Param_T[7];
	ptiming_info = new Timing_Info_T[7];
	pselfdetection = new SelfDetection_Plot_Parameters_T[7];
    ptoolruntime = new Tool_RunTime_info_T;
    ptracker = new Tracker_Plot_Parameter_T[7];
	
	
}

Gen5PlatformDataCollector::~Gen5PlatformDataCollector()
{
	//std::cout << "\nStlaScale1DataCollector::~StlaScale1DataCollector()";
	delete[] pvehicle_info;
	delete[] ptiming_info;
	delete[] pselfdetection;
    delete ptoolruntime;
    delete[] ptracker;
}

Gen5PlatformDataCollector& Gen5PlatformDataCollector::getInstance()
{
	static Gen5PlatformDataCollector instance;
	return instance;
	
}

void Gen5PlatformDataCollector::collect_parsedmf4_data(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
	

	/*if (platform == UDP_PLATFORM_SRR6 || platform == UDP_PLATFORM_SRR6_PLUS || platform == UDP_PLATFORM_SRR6_PLUS_1X_CAN || platform == UDP_PLATFORM_SRR6_PLUS_1X_ETH || platform == UDP_PLATFORM_SRR6_PLUS_1X_1GBPS_ETH || platform == UDP_PLATFORM_STLA_FLR4)
	{

		if (stream_num == C2_CORE_MASTER_STREAM)
		{

			if (custId == STLA_SRR6P)
			{
				collect_vehicle_data_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
                collect_detection_data_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
			}
			else if (custId == STLA_FLR4)
			{
				collect_vehicle_data_flr4(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
                collect_detection_data_flr4(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
			}

			if (custId == STLA_SRR6P|| custId == STLA_FLR4)
			{
				collect_c2timing_info_srr6p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
			}
		}

	}*/


    if (platform == UDP_PLATFORM_FLR4_PLUS || platform == UDP_PLATFORM_FLR4_PLUS_STANDALONE)
    {

        if (stream_num == e_TRACKER_LOGGING_STREAM)
        {
            collect_tracker_data_gpo(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
        }

        if (stream_num == e_DET_LOGGING_STREAM)
        {
            collect_detection_data_gpo_flr4p(fileCount, LogFname, g_pIRadarStrm, stream_num, Radar_Posn, custId, scanindex, platform, IsRECU);
        }

    }
	
	
}

void Gen5PlatformDataCollector::generate_plot(const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{
	//MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();
	//HtmlReportManager& reportman = HtmlReportManager::getInstance();

 //   remove_old_html_reports(f_name,pltfolder,reportpath);

	//for (int radarpos : reportman.set_radar_positions)
	//{

	//	string plottitle1;
	//	std::map<int, string> titleinfomap;
	//	
	//	matplotnonscale.PrintReportInfoinHTML(f_name, "HTML", pltfolder, reportpath, radarpos);

 //       
 //       titleinfomap[0] = "RL vehicle info speed";
 //       titleinfomap[1] = "RR vehicle info speed";
 //       titleinfomap[2] = "FR vehicle info speed";
 //       titleinfomap[3] = "FL vehicle info speed";
 //       titleinfomap[5] = "FC vehicle info speed";

	//	for (auto val : titleinfomap)
	//	{
	//		if (val.first == radarpos)
	//		{
	//			plottitle1 = val.second;
	//		}
	//	}
	//	//matplotnonscale.generate_nonscaled_vehinfo_speed_scatterplot(pvehicle_info, filecount, f_name, plottitle1.c_str(), pltfolder, "speed km/hr", reportpath, radarpos);

 //       titleinfomap[0] = "RL vehicle info yawrate";
 //       titleinfomap[1] = "RR vehicle info yawrate";
 //       titleinfomap[2] = "FR vehicle info yawrate";
 //       titleinfomap[3] = "FL vehicle info yawrate";
 //       titleinfomap[5] = "FC vehicle info yawrate";

	//	for (auto val : titleinfomap)
	//	{
	//		if (val.first == radarpos)
	//		{
	//			plottitle1 = val.second;
	//		}
	//	}
	//	//matplotnonscale.generate_nonscaled_vehinfo_yawrate_scatterplot(pvehicle_info, filecount, f_name, plottitle1.c_str(), pltfolder, "yaw", reportpath, radarpos);


 //       titleinfomap[0] = "RL c2 timing info[23]";
 //       titleinfomap[1] = "RR c2 timing info[23]";
 //       titleinfomap[2] = "FR c2 timing info[23]";
 //       titleinfomap[3] = "FL c2 timing info[23]";
 //       titleinfomap[5] = "FC c2 timing info[23]";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }


 //       //matplotnonscale.generate_c2timinginfo_scatterplot(ptiming_info, filecount, f_name, plottitle1.c_str(), pltfolder, "c2 timing info", reportpath, radarpos);


 //       titleinfomap[0] = "RL Valid  Detection Count";
 //       titleinfomap[1] = "RR Valid  Detection Count";
 //       titleinfomap[2] = "FR Valid  Detection Count";
 //       titleinfomap[3] = "FL Valid  Detection Count";
 //       titleinfomap[5] = "FC Valid  Detection Count";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       matplotnonscale.generate_valid_detectioncount_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "valid detection count", reportpath, radarpos);


 //       titleinfomap[0] = "RL Diffence in Valid  Detection Count";
 //       titleinfomap[1] = "RR Diffence in Valid  Detection Count";
 //       titleinfomap[2] = "FR Diffence in Valid  Detection Count";
 //       titleinfomap[3] = "FL Diffence in Valid  Detection Count";
 //       titleinfomap[5] = "FC Diffence in Valid  Detection Count";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }


 //       matplotnonscale.generate_diff_valid_detection_count_bargraph((pselfdetection+radarpos)->map_si_valid_detection_count[0],
 //                                                                    (pselfdetection + radarpos)->map_si_valid_detection_count[1],
 //                                                                     filecount, f_name, plottitle1.c_str(), pltfolder, 
 //                                                                     "scan index", reportpath, radarpos);

 //       titleinfomap[0] = "RL Max Range  ";
 //       titleinfomap[1] = "RR Max Range  ";
 //       titleinfomap[2] = "FR Max Range  ";
 //       titleinfomap[3] = "FL Max Range  ";
 //       titleinfomap[5] = "FC Max Range  ";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_nonscaled_detection_maxrange_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "max range value", reportpath, radarpos);
 //       titleinfomap[0] = "RL Range Histogram  ";
 //       titleinfomap[1] = "RR Range Histogram ";
 //       titleinfomap[2] = "FR Range Histogram ";
 //       titleinfomap[3] = "FL Range Histogram ";
 //       titleinfomap[5] = "FC Range Histogram ";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }


 //      

 //      //matplotnonscale.generate_detection_range_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);


 //      matplotnonscale.generate_detection_histogram((pselfdetection+radarpos)->range[0], (pselfdetection + radarpos)->range[1], filecount, f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);

 //       titleinfomap[0] = "RL RangeRate Histogram  ";
 //       titleinfomap[1] = "RR RangeRate Histogram ";
 //       titleinfomap[2] = "FR RangeRate Histogram ";
 //       titleinfomap[3] = "FL RangeRate Histogram ";
 //       titleinfomap[5] = "FC RangeRate Histogram ";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //      matplotnonscale.generate_detection_rangerate_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "rangerate", reportpath, radarpos);


 //       titleinfomap[0] = "RL Azimuth Histogram";
 //       titleinfomap[1] = "RR Azimuth Histogram";
 //       titleinfomap[2] = "FR Azimuth Histogram";
 //       titleinfomap[3] = "FL Azimuth Histogram";
 //       titleinfomap[5] = "FC Azimuth Histogram";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       matplotnonscale.generate_detection_azimuth_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth", reportpath, radarpos);

 //       titleinfomap[0] = "RL Elevation Histogram ";
 //       titleinfomap[1] = "RR Elevation Histogram";
 //       titleinfomap[2] = "FR Elevation Histogram";
 //       titleinfomap[3] = "FL Elevation Histogram";
 //       titleinfomap[5] = "FC Elevation Histogram";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_detection_elevation_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "elevation", reportpath, radarpos);


 //       titleinfomap[0] = "RL RCS Histogram";
 //       titleinfomap[1] = "RR RCS Histogram";
 //       titleinfomap[2] = "FR RCS Histogram";
 //       titleinfomap[3] = "FL RCS Histogram";
 //       titleinfomap[5] = "FC RCS Histogram";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_detection_rcs_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "rcs", reportpath, radarpos);

 //       titleinfomap[0] = "RL Amplitude Histogram";
 //       titleinfomap[1] = "RR Amplitude Histogram";
 //       titleinfomap[2] = "FR Amplitude Histogram";
 //       titleinfomap[3] = "FL Amplitude Histogram";
 //       titleinfomap[5] = "FC Amplitude Histogram";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //      matplotnonscale.generate_detection_amp_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "amplitude", reportpath, radarpos);


 //       titleinfomap[0] = "RL SNR Histogram ";
 //       titleinfomap[1] = "RR SNR Histogram";
 //       titleinfomap[2] = "FR SNR Histogram";
 //       titleinfomap[3] = "FL SNR Histogram";
 //       titleinfomap[5] = "FC SNR Histogram";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //      matplotnonscale.generate_detection_snr_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "snr", reportpath, radarpos);


 //      titleinfomap[0] = "RL IsBistatic Histogram ";
 //      titleinfomap[1] = "RR IsBistatic Histogram";
 //      titleinfomap[2] = "FR IsBistatic Histogram";
 //      titleinfomap[3] = "FL IsBistatic Histogram";
 //      titleinfomap[5] = "FC IsBistatic Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }

 //     // matplotnonscale.generate_detection_isbistatic_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "isbistatic", reportpath, radarpos);

 //      titleinfomap[0] = "RL IsSingle target Histogram ";
 //      titleinfomap[1] = "RR IsSingle target Histogram";
 //      titleinfomap[2] = "FR IsSingle target Histogram";
 //      titleinfomap[3] = "FL IsSingle target Histogram";
 //      titleinfomap[5] = "FC IsSingle target Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }

 //      //matplotnonscale.generate_detection_issingletarget_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "issingletarget", reportpath, radarpos);


 //      titleinfomap[0] = "RL super res target Histogram ";
 //      titleinfomap[1] = "RR super res target  Histogram";
 //      titleinfomap[2] = "FR super res target  Histogram";
 //      titleinfomap[3] = "FL super res target  Histogram";
 //      titleinfomap[5] = "FC super res target  Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }
 //     // matplotnonscale.generate_detection_super_res_target_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "super res target", reportpath, radarpos);

 //      titleinfomap[0] = "RL super res target type Histogram ";
 //      titleinfomap[1] = "RR super res target type Histogram";
 //      titleinfomap[2] = "FR super res target type Histogram";
 //      titleinfomap[3] = "FL super res target type Histogram";
 //      titleinfomap[5] = "FC super res target type Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }

 //      //matplotnonscale.generate_detection_super_res_target_type_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "super res target type", reportpath, radarpos);

 //      titleinfomap[0] = "RL bf type el Histogram ";
 //      titleinfomap[1] = "RR bf type el Histogram";
 //      titleinfomap[2] = "FR bf type el Histogram";
 //      titleinfomap[3] = "FL bf type el Histogram";
 //      titleinfomap[5] = "FC bf type el Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }
 //      //matplotnonscale.generate_detection_bf_type_el_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "bf type el", reportpath, radarpos);

 //      titleinfomap[0] = "RL bf type az Histogram ";
 //      titleinfomap[1] = "RR bf type az Histogram";
 //      titleinfomap[2] = "FR bf type az Histogram";
 //      titleinfomap[3] = "FL bf type az Histogram";
 //      titleinfomap[5] = "FC bf type az Histogram";

 //      for (auto val : titleinfomap)
 //      {
 //          if (val.first == radarpos)
 //          {
 //              plottitle1 = val.second;
 //          }
 //      }

 //     // matplotnonscale.generate_detection_bf_type_az_histogram(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "bf type az", reportpath, radarpos);
 //       titleinfomap[0] = "RL Detection range";
 //       titleinfomap[1] = "RR Detection range";
 //       titleinfomap[2] = "FR Detection range";
 //       titleinfomap[3] = "FL Detection range";
 //       titleinfomap[5] = "FC Detection range";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       matplotnonscale.generate_nonscaled_detection_range_scatterplot(pselfdetection,
 //                                                               filecount, f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);

 //      // matplotnonscale.generate_nonscaled_detection_scatterplot((pselfdetection+ radarpos)->si_veh, (pselfdetection + radarpos)->range[0],
 //        //                                                                      (pselfdetection + radarpos)->si_resim, (pselfdetection + radarpos)->range[1], filecount, 
 //        //                                                                       f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);

 //       titleinfomap[0] = "RL Detection range rate";
 //       titleinfomap[1] = "RR Detection range rate";
 //       titleinfomap[2] = "FR Detection range rate";
 //       titleinfomap[3] = "FL Detection range rate";
 //       titleinfomap[5] = "FC Detection range rate";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }


 //       matplotnonscale.generate_nonscaled_detection_rangerate_scatterplot(pselfdetection,filecount, f_name, plottitle1.c_str(), pltfolder, "rangerate", reportpath, radarpos);

 //        titleinfomap[0] = "RL Detection azimuth";
 //       titleinfomap[1] = "RR Detection azimuth";
 //       titleinfomap[2] = "FR Detection azimuth";
 //       titleinfomap[3] = "FL Detection azimuth";
 //       titleinfomap[5] = "FC Detection azimuth";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       
 //       matplotnonscale.generate_nonscaled_detection_azimuth_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth", reportpath, radarpos);

 //       titleinfomap[0] = "RL azimuth Confidence";
 //       titleinfomap[1] = "RR azimuth Confidence";
 //       titleinfomap[2] = "FR azimuth Confidence";
 //       titleinfomap[3] = "FL azimuth Confidence";
 //       titleinfomap[5] = "FC azimuth Confidence";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       //matplotnonscale.generate_az_conf_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth confidence", reportpath, radarpos);


 //        titleinfomap[0] = "RL Detection elevation";
 //       titleinfomap[1] = "RR Detection elevation";
 //       titleinfomap[2] = "FR Detection elevation";
 //       titleinfomap[3] = "FL Detection elevation";
 //       titleinfomap[5] = "FC Detection elevation";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_nonscaled_detection_elevation_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "elevation", reportpath, radarpos);



 //       titleinfomap[0] = "RL Elevation Confidence";
 //       titleinfomap[1] = "RR Elevation Confidence";
 //       titleinfomap[2] = "FR Elevation Confidence";
 //       titleinfomap[3] = "FL Elevation Confidence";
 //       titleinfomap[5] = "FC Elevation Confidence";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       //matplotnonscale.generate_el_conf_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "el conf", reportpath, radarpos);
 //       
 //       titleinfomap[0] = "RL Detection rcs";
 //       titleinfomap[1] = "RR Detection rcs";
 //       titleinfomap[2] = "FR Detection rcs";
 //       titleinfomap[3] = "FL Detection rcs";
 //       titleinfomap[5] = "FC Detection rcs";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_nonscaled_detection_rcs_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "rcs", reportpath, radarpos);

 //       titleinfomap[0] = "RL Detection amplitude";
 //       titleinfomap[1] = "RR Detection amplitude";
 //       titleinfomap[2] = "FR Detection amplitude";
 //       titleinfomap[3] = "FL Detection amplitude";
 //       titleinfomap[5] = "FC Detection amplitude";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_nonscaled_detection_amp_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "amp", reportpath, radarpos);


 //       titleinfomap[0] = "RL Detection snr";
 //       titleinfomap[1] = "RR Detection snr";
 //       titleinfomap[2] = "FR Detection snr";
 //       titleinfomap[3] = "FL Detection snr";
 //       titleinfomap[5] = "FC Detection snr";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       matplotnonscale.generate_nonscaled_detection_snr_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "snr", reportpath, radarpos);

 //       titleinfomap[0] = "RL IsBistatic detection";
 //       titleinfomap[1] = "RR IsBistatic detection";
 //       titleinfomap[2] = "FR IsBistatic detection";
 //       titleinfomap[3] = "FL IsBistatic detection";
 //       titleinfomap[5] = "FC IsBistatic detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       //matplotnonscale.generate_isbistatic_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "isbistatic", reportpath, radarpos);


 //       titleinfomap[0] = "RL IsSingle taregt detection";
 //       titleinfomap[1] = "RR IsSingle taregt detection";
 //       titleinfomap[2] = "FR IsSingle taregt detection";
 //       titleinfomap[3] = "FL IsSingle taregt detection";
 //       titleinfomap[5] = "FC IsSingle taregt detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       //matplotnonscale.generate_issingletarget_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "issingletarget", reportpath, radarpos);


 //       titleinfomap[0] = "RL super res taregt detection";
 //       titleinfomap[1] = "RR super res taregt detection";
 //       titleinfomap[2] = "FR super res taregt detection";
 //       titleinfomap[3] = "FL super res taregt detection";
 //       titleinfomap[5] = "FC super res taregt detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //       //matplotnonscale.generate_super_res_target_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "super res target", reportpath, radarpos);

 //       titleinfomap[0] = "RL super res taregt type detection";
 //       titleinfomap[1] = "RR super res taregt type detection";
 //       titleinfomap[2] = "FR super res taregt type detection";
 //       titleinfomap[3] = "FL super res taregt type detection";
 //       titleinfomap[5] = "FC super res taregt type detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       //matplotnonscale.generate_super_res_target_type_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "super res target type", reportpath, radarpos);


 //       titleinfomap[0] = "RL bf type el detection";
 //       titleinfomap[1] = "RR bf type el detection";
 //       titleinfomap[2] = "FR bf type el detection";
 //       titleinfomap[3] = "FL bf type el detection";
 //       titleinfomap[5] = "FC bf type el detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }
 //      // matplotnonscale.generate_bf_type_el_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "bf type el detection", reportpath, radarpos);

 //       titleinfomap[0] = "RL bf type az detection";
 //       titleinfomap[1] = "RR bf type az detection";
 //       titleinfomap[2] = "FR bf type az detection";
 //       titleinfomap[3] = "FL bf type az detection";
 //       titleinfomap[5] = "FC bf type az detection";

 //       for (auto val : titleinfomap)
 //       {
 //           if (val.first == radarpos)
 //           {
 //               plottitle1 = val.second;
 //           }
 //       }

 //       //matplotnonscale.generate_bf_type_az_scatterplot(pselfdetection, filecount, f_name, plottitle1.c_str(), pltfolder, "bf type el detection", reportpath, radarpos);
 //       
	//}
}

void Gen5PlatformDataCollector::calculate_tool_runtime(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
   /* HtmlReportManager& repman = HtmlReportManager::getInstance();
    MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start,log2_scanindex_end;
    long long log1_duration,log2_duration;
    int tool_duration_func_inputlog;
    int log1_duration_fin{ 0 }, log2_duration_fin{ 0 };

    if ((pselfdetection + radarposition)->si_veh_ref_scale.size() != 0)
    {
        log1_scanindex_start = (pselfdetection + radarposition)->si_veh_ref_scale.front();
        log1_scanindex_end = (pselfdetection + radarposition)->si_veh_ref_scale.back();
        log1_duration = log1_scanindex_end - log1_scanindex_start;
        log1_duration_fin = log1_duration * .05;

        if (ptoolruntime != nullptr)
        {
            repman.Log1_duration = to_string(log1_duration_fin);

        }
    }

    if ((pselfdetection + radarposition)->si_resim_ref_scale.size() != 0)
    {
        log2_scanindex_start = (pselfdetection + radarposition)->si_resim_ref_scale.front();
        log2_scanindex_end = (pselfdetection + radarposition)->si_resim_ref_scale.back();
        log2_duration = log2_scanindex_end - log2_scanindex_start;
        log2_duration_fin = log2_duration * .05;

        if (ptoolruntime != nullptr)
        {
            repman.Log2_duration = to_string(log2_duration_fin);
        }
    }
   
    



        if (ptoolruntime != nullptr)
        {
            if (log1_duration_fin!=0 || log2_duration_fin!=0)
            {
                tool_duration_func_inputlog = (stoi(repman.html_runtime_sec)) / (log1_duration_fin + log2_duration_fin);
                repman.Log1_duration = to_string(log1_duration_fin);
                repman.Log2_duration = to_string(log2_duration_fin);
                repman.tool_runtime_func_inputtime = to_string(tool_duration_func_inputlog);
                matplotnonscale.print_reporttoolrun_timing_info(ptoolruntime, f_name, "HTML", pltfolder, reportpath, radarposition);
            }
            else
            {
                repman.Log1_duration = "Log duration not available";
                repman.Log2_duration = "Log duration not available";
                repman.tool_runtime_func_inputtime = "Run time not calculates as Log duration not available";
                matplotnonscale.print_reporttoolrun_timing_info(ptoolruntime, f_name, "HTML", pltfolder, reportpath, radarposition);

            }
           
        }
    */

}

void Gen5PlatformDataCollector::calculate_Logduration(const char* f_name, const char* pltfolder, int filecount, std::string reportpath, int radarposition)
{
    /*std::cout << "\ncalculate_Logduration";
    HtmlReportManager& repman = HtmlReportManager::getInstance();
    MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

    long long log1_scanindex_start, log1_scanindex_end, log2_scanindex_start, log2_scanindex_end;
    long long log1_duration, log2_duration;
    double tool_duration_func_inputlog;
    long long log1_duration_fin, log2_duration_fin;


    if (pselfdetection!=nullptr)
    {

        if ((pselfdetection + radarposition)->si_veh_ref_scale.size() != 0)
        {
            log1_scanindex_start = (pselfdetection + radarposition)->si_veh_ref_scale.front();
            log1_scanindex_end = (pselfdetection + radarposition)->si_veh_ref_scale.back();
            log1_duration = log1_scanindex_end - log1_scanindex_start;
            log1_duration_fin = log1_duration * .05;

            if (ptoolruntime != nullptr)
            {
                repman.Log1_duration = to_string(log1_duration_fin);
                
            }
        }
     
       
        if ((pselfdetection + radarposition)->si_resim_ref_scale.size()!=0)
        {
            log2_scanindex_start = (pselfdetection + radarposition)->si_resim_ref_scale.front();
            log2_scanindex_end = (pselfdetection + radarposition)->si_resim_ref_scale.back();
            log2_duration = log2_scanindex_end - log2_scanindex_start;
            log2_duration_fin = log2_duration * .05;

            if (ptoolruntime != nullptr)
            {              
                repman.Log2_duration = to_string(log2_duration_fin);
            }
        }
           

        
     
    }
    */

}

//void Gen5PlatformDataCollector::receive_master_updateonDatafactory(std::map<int, DataFactory*> map_datacollector_details)
//{
//
//}

double Gen5PlatformDataCollector::flt_to_dbl(float32_T con_value)
{
	char chr_fValue[32];//32
	sprintf(chr_fValue, "%9.8f", con_value);//9.8
	double db_val = strtod(chr_fValue, NULL);
	return db_val;
}

bool Gen5PlatformDataCollector::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
{
	return ((resimsi - vehiclesi_end) * (resimsi - vehiclesi_start) <= 0);
}

inline void Gen5PlatformDataCollector::remove_old_html_reports(const char* f_name, const char* pltfolder,std::string reportpath)
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







void Gen5PlatformDataCollector::collect_vehicle_data_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    /*if (pvehicle_info != nullptr && m_radar_plugin != nullptr)
    {
        int radar_position = int(Radar_Posn);
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla_srr6p_s1, Alignment_Data);
        (pvehicle_info + radar_position)->yaw_rate[fileCount].push_back(static_cast<double>(p_veh_stla_srr6p_s1.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate));
        (pvehicle_info + radar_position)->abs_speed[fileCount].push_back(static_cast<double>((p_veh_stla_srr6p_s1.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed)) * 3.6);
        (pvehicle_info + radar_position)->scanidex[fileCount].push_back(scanindex);
    }*/
   
	
}

void Gen5PlatformDataCollector::collect_vehicle_data_flr4(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
 /*   if (pvehicle_info != nullptr && m_radar_plugin != nullptr)
    {
        int radar_position = int(Radar_Posn);
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla_flr4_s1, Alignment_Data);
        (pvehicle_info + radar_position)->abs_speed[fileCount].push_back(static_cast<double>((p_veh_stla_flr4_s1.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed)) * 3.6);
        (pvehicle_info + radar_position)->yaw_rate[fileCount].push_back(static_cast<double>(p_veh_stla_flr4_s1.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate));
        (pvehicle_info + radar_position)->scanidex[fileCount].push_back(scanindex);
    }*/
}

void Gen5PlatformDataCollector::collect_c2timing_info_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
   /* if (ptiming_info != nullptr && m_radar_plugin != nullptr)
    {
        if (fileCount == 0)
        {
            int radar_position = int(Radar_Posn);
            m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_c2timing_stla_srr6p_s1, Diagnostics_Data);
            (ptiming_info + radar_position)->c2timinginfo_index23[fileCount].push_back(double(p_c2timing_stla_srr6p_s1.Timing_Info_c2[23]));
            (ptiming_info + radar_position)->scanindex[fileCount].push_back(scanindex);
        }
    }*/
 
}

void Gen5PlatformDataCollector::collect_c2timing_info_flr4(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
}



void Gen5PlatformDataCollector::collect_detection_data_srr6p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{

    //if (m_radar_plugin != nullptr && pselfdetection != nullptr)
    //{
    //    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_selfdetection_flr4p_gpo, DETS_COMP);
    //    int radar_position = int(Radar_Posn);

    //    float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
    //    for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    //    {
    //        Self_Dets[i] = p_selfdetection_flr4p_gpo.target_report[i].range;
    //    }
    //    unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
    //    unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
    //    for (unsigned8_T i = 0; i < size; i++)
    //    {
    //        index[i] = i;
    //    }
    //    array_data_ptr_gpo = Self_Dets;
    //    qsort(index, size, sizeof(*index), comparedata_gpo);


    //    /*print sorted self dets*/
    //    unsigned8_T count = 1;
    //    for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    //    {
    //        if ((p_selfdetection_flr4p_gpo.target_report[index[i]].range != 0) && (count == 1))
    //        {
    //            if (fileCount == 0)
    //            {
    //                (pselfdetection + radar_position)->si_veh_ref_scale.push_back(scanindex);
    //                (pselfdetection + radar_position)->valid_detection_count[fileCount].push_back(double(p_selfdetection_flr4p_gpo.target_count));
    //                (pselfdetection + radar_position)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
    //            }
    //            if (fileCount == 1)
    //            {
    //                (pselfdetection + radar_position)->si_resim_ref_scale.push_back(scanindex);
    //                (pselfdetection + radar_position)->valid_detection_count[fileCount].push_back(double(p_selfdetection_flr4p_gpo.target_count));
    //                (pselfdetection + radar_position)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
    //            }
    //            count = 0;
    //        }
    //        if (p_selfdetection_flr4p_gpo.target_report[index[i]].range != 0)
    //        {
    //            if (fileCount == 0)
    //            {
    //                scanindexRangeRef[radar_position].push_back(scanindex);

    //                double rangevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range);
    //                if (rangevalue >= -1 && rangevalue <= 500)
    //                {
    //                    (pselfdetection + radar_position)->range[fileCount].push_back(rangevalue);
    //                    (pselfdetection + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
    //                }

    //                double rangeratevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range_rate);
    //                if (rangeratevalue >= -30 && rangeratevalue <= 30)
    //                {
    //                    (pselfdetection + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
    //                    (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
    //                }

    //                double ampvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].amp);
    //                if (ampvalue >= -100 && ampvalue <= 100)
    //                {
    //                    (pselfdetection + radar_position)->amp[fileCount].push_back(ampvalue);
    //                    (pselfdetection + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
    //                }

    //                double rcsvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].rcs);
    //                if (rcsvalue >= -100 && rcsvalue <= 100)
    //                {
    //                    (pselfdetection + radar_position)->rcs[fileCount].push_back(rcsvalue);
    //                    (pselfdetection + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
    //                }

    //                double snrvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].snr);
    //                if (snrvalue >= -100 && snrvalue <= 100)
    //                {
    //                    (pselfdetection + radar_position)->snr[fileCount].push_back(snrvalue);
    //                    (pselfdetection + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
    //                }

    //                double azimuth = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].azimuth));
    //                if (azimuth >= -90 && azimuth <= 90)
    //                {
    //                    (pselfdetection + radar_position)->azimuth[fileCount].push_back(azimuth);
    //                    (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
    //                }

    //                double elevation = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].elevation));
    //                if (elevation >= -40 && elevation <= 40)
    //                {
    //                    (pselfdetection + radar_position)->elevation[fileCount].push_back(elevation);
    //                    (pselfdetection + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
    //                }

    //                double az_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].az_conf);
    //                double el_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].el_conf);
    //                (pselfdetection + radar_position)->az_conf[fileCount].push_back(az_conf_value);
    //                (pselfdetection + radar_position)->el_conf[fileCount].push_back(el_conf_value);


    //                double isbistatic = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isBistatic);               
    //                (pselfdetection + radar_position)->isBistatic[fileCount].push_back(isbistatic);

    //                double issingletarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isSingleTarget);
    //                (pselfdetection + radar_position)->isSingleTarget[fileCount].push_back(issingletarget);

    //                double superrestarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target);
    //                (pselfdetection + radar_position)->super_res_target[fileCount].push_back(superrestarget);

    //                double superrestarget_type = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target_type);
    //                (pselfdetection + radar_position)->super_res_target_type[fileCount].push_back(superrestarget_type);

    //                double bftype_el = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_el);
    //                (pselfdetection + radar_position)->bf_type_el[fileCount].push_back(bftype_el);

    //                double bftype_az = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_az);
    //                (pselfdetection + radar_position)->bf_type_az[fileCount].push_back(bftype_az);
    //                



    //                if ((rangevalue >= -1 && rangevalue <= 500) ||
    //                    (rangeratevalue >= -100 && rangeratevalue <= 100) ||
    //                    (ampvalue >= -100 && ampvalue <= 100) ||
    //                    (snrvalue >= -100 && snrvalue <= 100) ||
    //                    (azimuth >= -90 && azimuth <= 90) ||
    //                    elevation >= -40 && elevation <= 40)
    //                {

    //                    (pselfdetection + radar_position)->si_veh.push_back(scanindex);


    //                }

    //            }

    //            if (fileCount == 1)
    //            {
    //                if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
    //                {
    //                    double rangevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range);
    //                    if (rangevalue >= -1 && rangevalue <= 500)
    //                    {
    //                        (pselfdetection + radar_position)->range[fileCount].push_back(rangevalue);
    //                        (pselfdetection + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
    //                    }

    //                    double rangeratevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range_rate);
    //                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
    //                    {
    //                        (pselfdetection + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
    //                        (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
    //                    }

    //                    double ampvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].amp);
    //                    if (ampvalue >= -100 && ampvalue <= 100)
    //                    {
    //                        (pselfdetection + radar_position)->amp[fileCount].push_back(ampvalue);
    //                        (pselfdetection + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
    //                    }

    //                    double rcsvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].rcs);
    //                    if (rcsvalue >= -100 && rcsvalue <= 100)
    //                    {
    //                        (pselfdetection + radar_position)->rcs[fileCount].push_back(rcsvalue);
    //                        (pselfdetection + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
    //                    }

    //                    double snrvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].snr);
    //                    if (snrvalue >= -100 && snrvalue <= 100)
    //                    {
    //                        (pselfdetection + radar_position)->snr[fileCount].push_back(snrvalue);
    //                        (pselfdetection + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
    //                    }

    //                    double azimuth = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].azimuth));
    //                    if (azimuth >= -90 && azimuth <= 90)
    //                    {
    //                        (pselfdetection + radar_position)->azimuth[fileCount].push_back(azimuth);
    //                        (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
    //                    }

    //                    double elevation = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].elevation));
    //                    if (elevation >= -40 && elevation <= 40)
    //                    {
    //                        (pselfdetection + radar_position)->elevation[fileCount].push_back(elevation);
    //                        (pselfdetection + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
    //                    }

    //                    double az_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].az_conf);
    //                    double el_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].el_conf);
    //                    (pselfdetection + radar_position)->az_conf[fileCount].push_back(az_conf_value);
    //                    (pselfdetection + radar_position)->el_conf[fileCount].push_back(el_conf_value);



    //                    double isbistatic = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isBistatic);
    //                    (pselfdetection + radar_position)->isBistatic[fileCount].push_back(isbistatic);

    //                    double issingletarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isSingleTarget);
    //                    (pselfdetection + radar_position)->isSingleTarget[fileCount].push_back(issingletarget);

    //                    double superrestarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target);
    //                    (pselfdetection + radar_position)->super_res_target[fileCount].push_back(superrestarget);

    //                    double superrestarget_type = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target_type);
    //                    (pselfdetection + radar_position)->super_res_target_type[fileCount].push_back(superrestarget_type);

    //                    double bftype_el = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_el);
    //                    (pselfdetection + radar_position)->bf_type_el[fileCount].push_back(bftype_el);

    //                    double bftype_az = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_az);
    //                    (pselfdetection + radar_position)->bf_type_az[fileCount].push_back(bftype_az);


    //                    if ((rangevalue >= -1 && rangevalue <= 500) ||
    //                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
    //                        (ampvalue >= -100 && ampvalue <= 100) ||
    //                        (snrvalue >= -100 && snrvalue <= 100) ||
    //                        (azimuth >= -90 && azimuth <= 90) ||
    //                        elevation >= -40 && elevation <= 40)
    //                    {

    //                        (pselfdetection + radar_position)->si_resim.push_back(scanindex);
    //                      
    //                    }

    //                   
    //                }

    //            }

    //        }
    //    }
    //    if (count == 0)
    //    {
    //        if ((pselfdetection + radar_position)->range_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->range_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->range_rate_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->range_rate_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->azimuth_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->azimuth_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->elevation_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->elevation_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->rcs_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->rcs_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->amp_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->amp_scaleplot[fileCount]));
    //        }
    //        if ((pselfdetection + radar_position)->snr_scaleplot[fileCount].size() != 0)
    //        {
    //            (pselfdetection + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->snr_scaleplot[fileCount]));
    //        }

    //        if ((pselfdetection + radar_position)->range[fileCount].size() != 0)
    //        {
    //            std::vector<double>::iterator itrmax_value;

    //           
    //                itrmax_value = std::max_element((pselfdetection + radar_position)->range[fileCount].begin(),
    //                    (pselfdetection + radar_position)->range[fileCount].end());
    //                (pselfdetection + radar_position)->range_max[fileCount].push_back(*itrmax_value);
    //                (pselfdetection + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);

    //            
    //           
    //            
    //        }

    //        (pselfdetection + radar_position)->range_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->elevation_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->rcs_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->amp_scaleplot[fileCount].clear();
    //        (pselfdetection + radar_position)->snr_scaleplot[fileCount].clear();

    //    }

    //}

}

void Gen5PlatformDataCollector::collect_detection_data_gpo_flr4p(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
  
    if (m_radar_plugin != nullptr && pselfdetection != nullptr)
    {
        m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_selfdetection_flr4p_gpo, DETS_COMP);
        /*sort by range*/

        int radar_position = int(Radar_Posn);
        float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
            Self_Dets[i] = p_selfdetection_flr4p_gpo.target_report[i].range;
        }
        unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
        unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptr_gpo = Self_Dets;
        qsort(index, size, sizeof(*index), comparedata_gpo);

       

        /*print sorted self dets*/
        unsigned8_T count = 1;
        for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
        {
            /*check for  non zero range*/
            if ((p_selfdetection_flr4p_gpo.target_report[index[i]].range != 0) && (count == 1))
            {
                if (fileCount == 0)
                {
                    (pselfdetection + radar_position)->si_veh_ref_scale.push_back(scanindex);
                    (pselfdetection + radar_position)->valid_detection_count[fileCount].push_back(double(p_selfdetection_flr4p_gpo.target_count));
                    (pselfdetection + radar_position)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
                    (pselfdetection + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double,double>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
                }
                if (fileCount == 1)
                {
                    (pselfdetection + radar_position)->si_resim_ref_scale.push_back(scanindex);
                    (pselfdetection + radar_position)->valid_detection_count[fileCount].push_back(double(p_selfdetection_flr4p_gpo.target_count));
                    (pselfdetection + radar_position)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
                    (pselfdetection + radar_position)->map_si_valid_detection_count[fileCount].insert(pair<double, double>(scanindex, double(p_selfdetection_flr4p_gpo.target_count)));
                }

                
                count = 0;

            }
            if (p_selfdetection_flr4p_gpo.target_report[index[i]].range != 0)
            {
                if (fileCount == 0)
                {
                    scanindexRangeRef[radar_position].push_back(scanindex);

                    double rangevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range);
                    if (rangevalue >= -1 && rangevalue <= 500)
                    {
                        (pselfdetection + radar_position)->range[fileCount].push_back(rangevalue);
                        (pselfdetection + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                    }
                    double rangeratevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range_rate);
                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        (pselfdetection + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                        (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }
                    double ampvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].amp);
                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        (pselfdetection + radar_position)->amp[fileCount].push_back(ampvalue);
                        (pselfdetection + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }
                    double rcsvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].rcs);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        (pselfdetection + radar_position)->rcs[fileCount].push_back(rcsvalue);
                        (pselfdetection + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }
                    double snrvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].snr);
                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        (pselfdetection + radar_position)->snr[fileCount].push_back(snrvalue);
                        (pselfdetection + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }
                    double azimuth = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].azimuth));
                    if (azimuth >= -90 && azimuth <= 90)
                    {
                        (pselfdetection + radar_position)->azimuth[fileCount].push_back(azimuth);
                        (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }
                    double elevation = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].elevation));
                    if (elevation >= -40 && elevation <= 40)
                    {
                        (pselfdetection + radar_position)->elevation[fileCount].push_back(elevation);
                        (pselfdetection + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                    }
                    double az_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].az_conf);
                    double el_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].el_conf);
                    (pselfdetection + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                    (pselfdetection + radar_position)->el_conf[fileCount].push_back(el_conf_value);



                    double isbistatic = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isBistatic);
                    (pselfdetection + radar_position)->isBistatic[fileCount].push_back(isbistatic);

                    double issingletarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isSingleTarget);
                    (pselfdetection + radar_position)->isSingleTarget[fileCount].push_back(issingletarget);

                    double superrestarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target);
                    (pselfdetection + radar_position)->super_res_target[fileCount].push_back(superrestarget);

                    double superrestarget_type = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target_type);
                    (pselfdetection + radar_position)->super_res_target_type[fileCount].push_back(superrestarget_type);

                    double bftype_el = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_el);
                    (pselfdetection + radar_position)->bf_type_el[fileCount].push_back(bftype_el);

                    double bftype_az = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_az);
                    (pselfdetection + radar_position)->bf_type_az[fileCount].push_back(bftype_az);

                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -90 && azimuth <= 90) ||
                        elevation >= -40 && elevation <= 40)
                    {

                        (pselfdetection + radar_position)->si_veh.push_back(scanindex);

                    }
                }

                if (fileCount == 1)
                {
                    if (!scanindexRangeRef[radar_position].empty())
                    {
                        if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                        {
                            double rangevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range);
                            if (rangevalue >= -1 && rangevalue <= 500)
                            {
                                (pselfdetection + radar_position)->range[fileCount].push_back(rangevalue);
                                (pselfdetection + radar_position)->range_scaleplot[fileCount].push_back(rangevalue);
                            }
                            double rangeratevalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].range_rate);
                            if (rangeratevalue >= -30 && rangeratevalue <= 30)
                            {
                                (pselfdetection + radar_position)->range_rate[fileCount].push_back(rangeratevalue);
                                (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                            }
                            double ampvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].amp);
                            if (ampvalue >= -100 && ampvalue <= 100)
                            {
                                (pselfdetection + radar_position)->amp[fileCount].push_back(ampvalue);
                                (pselfdetection + radar_position)->amp_scaleplot[fileCount].push_back(ampvalue);
                            }
                            double rcsvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].rcs);
                            if (rcsvalue >= -100 && rcsvalue <= 100)
                            {
                                (pselfdetection + radar_position)->rcs[fileCount].push_back(rcsvalue);
                                (pselfdetection + radar_position)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                            }
                            double snrvalue = flt_to_dbl(p_selfdetection_flr4p_gpo.target_report[index[i]].snr);
                            if (snrvalue >= -100 && snrvalue <= 100)
                            {
                                (pselfdetection + radar_position)->snr[fileCount].push_back(snrvalue);
                                (pselfdetection + radar_position)->snr_scaleplot[fileCount].push_back(snrvalue);
                            }
                            double azimuth = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].azimuth));
                            if (azimuth >= -90 && azimuth <= 90)
                            {
                                (pselfdetection + radar_position)->azimuth[fileCount].push_back(azimuth);
                                (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].push_back(azimuth);
                            }
                            double elevation = flt_to_dbl(RAD2DEG(p_selfdetection_flr4p_gpo.target_report[index[i]].elevation));
                            if (elevation >= -40 && elevation <= 40)
                            {
                                (pselfdetection + radar_position)->elevation[fileCount].push_back(elevation);
                                (pselfdetection + radar_position)->elevation_scaleplot[fileCount].push_back(elevation);
                            }
                            double az_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].az_conf);
                            double el_conf_value = double(p_selfdetection_flr4p_gpo.target_report[index[i]].el_conf);
                            (pselfdetection + radar_position)->az_conf[fileCount].push_back(az_conf_value);
                            (pselfdetection + radar_position)->el_conf[fileCount].push_back(el_conf_value);



                            double isbistatic = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isBistatic);
                            (pselfdetection + radar_position)->isBistatic[fileCount].push_back(isbistatic);

                            double issingletarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].isSingleTarget);
                            (pselfdetection + radar_position)->isSingleTarget[fileCount].push_back(issingletarget);

                            double superrestarget = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target);
                            (pselfdetection + radar_position)->super_res_target[fileCount].push_back(superrestarget);

                            double superrestarget_type = double(p_selfdetection_flr4p_gpo.target_report[index[i]].super_res_target_type);
                            (pselfdetection + radar_position)->super_res_target_type[fileCount].push_back(superrestarget_type);

                            double bftype_el = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_el);
                            (pselfdetection + radar_position)->bf_type_el[fileCount].push_back(bftype_el);

                            double bftype_az = double(p_selfdetection_flr4p_gpo.target_report[index[i]].bf_type_az);
                            (pselfdetection + radar_position)->bf_type_az[fileCount].push_back(bftype_az);

                            if ((rangevalue >= -1 && rangevalue <= 500) ||
                                (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                                (ampvalue >= -100 && ampvalue <= 100) ||
                                (snrvalue >= -100 && snrvalue <= 100) ||
                                (azimuth >= -90 && azimuth <= 90) ||
                                elevation >= -40 && elevation <= 40)
                            {

                                (pselfdetection + radar_position)->si_resim.push_back(scanindex);


                            }

                        }
                    }
                }
            }
        }
        if (count == 0)
        {
            if ((pselfdetection + radar_position)->range_scaleplot[fileCount].size() != 0)
            {
                (pselfdetection + radar_position)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->range_scaleplot[fileCount]));
            }
            if ((pselfdetection + radar_position)->range_rate_scaleplot[fileCount].size()!=0)
            {
                (pselfdetection + radar_position)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->range_rate_scaleplot[fileCount]));
            }           
            if ((pselfdetection + radar_position)->azimuth_scaleplot[fileCount].size()!=0)
            {
                (pselfdetection + radar_position)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->azimuth_scaleplot[fileCount]));
            }
            if ((pselfdetection + radar_position)->elevation_scaleplot[fileCount].size() != 0)
            {
                (pselfdetection + radar_position)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->elevation_scaleplot[fileCount]));
            }
            if ((pselfdetection + radar_position)->rcs_scaleplot[fileCount].size()!=0)
            {
                (pselfdetection + radar_position)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->rcs_scaleplot[fileCount]));
            }
            if ((pselfdetection + radar_position)->amp_scaleplot[fileCount].size()!=0)
            {
                (pselfdetection + radar_position)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->amp_scaleplot[fileCount]));
            }
            if ((pselfdetection + radar_position)->snr_scaleplot[fileCount].size()!=0)
            {
                (pselfdetection + radar_position)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pselfdetection + radar_position)->snr_scaleplot[fileCount]));
            }          
            if ((pselfdetection + radar_position)->range[fileCount].size() != 0)
            {
                std::vector<double>::iterator itrmax_value;
                itrmax_value = std::max_element((pselfdetection + radar_position)->range[fileCount].begin(),
                                                (pselfdetection + radar_position)->range[fileCount].end());  

                (pselfdetection + radar_position)->range_max[fileCount].push_back(*itrmax_value);
                (pselfdetection + radar_position)->scanindex_maxrange[fileCount].push_back(scanindex);
            }
            (pselfdetection + radar_position)->range_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->range_rate_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->azimuth_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->elevation_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->rcs_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->amp_scaleplot[fileCount].clear();
            (pselfdetection + radar_position)->snr_scaleplot[fileCount].clear();

        }
    }
}

void Gen5PlatformDataCollector::collect_tracker_data_gpo(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform, bool IsRECU)
{
    //std::cout << "\nGen5PlatformDataCollector::collect_tracker_data_gpo";
    int radar_position = int(Radar_Posn);
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_tracker_flr4p_gpo, CURVI_TRACK_DATA);
    DVSU_RECORD_T record = { 0 };
    if (custId == PLATFORM_GEN5 || custId == STLA_FLR4P)
    {
#ifndef DISABLE_TRACKER
        /*sort by range*/
        float32_T Track_Objs[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
        {
            Track_Objs[i] = p_tracker_flr4p_gpo.object[i].vcs_xposn;
        }
        unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
        unsigned8_T index[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }

        array_data_ptr_gpo = Track_Objs;
        qsort(index, size, sizeof(*index), comparedata_gpo);

        /*print sorted Opp dets*/
        unsigned8_T count = 1;
        for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
        {
            /*check for valid Track status level*/
            if (((p_tracker_flr4p_gpo.object[i].status) > 0) && (count == 1))
            {
                count = 0;
                if (fileCount == 0)
                {
                    (ptracker+radar_position)->vcs_si_veh_ref_scale.push_back(scanindex);
                }
                if (fileCount == 1)
                {
                    (ptracker+radar_position)->vcs_si_resim_ref_scale.push_back(scanindex);
                }

            }
            if ((p_tracker_flr4p_gpo.object[i].status) > 0)
            {
                if (fileCount == 0)
                {
                    unsigned8_T status = p_tracker_flr4p_gpo.object[i].status;

                    if (status != 0)
                    {
                        if (p_tracker_flr4p_gpo.object[i].f_moving)
                        {
                            //file1_moving_objects++;
                            //vec_file1_moving_objects.push_back(file1_moving_objects);
                            scanindexRangeRef[radar_position].push_back(scanindex);
                            (ptracker+radar_position)->vcs_si_veh.push_back(scanindex);
                            //FC_scanindex.push_back(scanindex);

                            double vcs_xposvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xposn);
                            (ptracker+radar_position)->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                            (ptracker+radar_position)->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                            double vcs_yposvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yposn);
                            (ptracker+radar_position)->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                            (ptracker+radar_position)->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                            double vcs_xvelvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xvel);
                            (ptracker+radar_position)->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                            (ptracker+radar_position)->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                            double vcs_yvelvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yvel);
                            (ptracker+radar_position)->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                            (ptracker+radar_position)->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                            double vcs_xaccvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xaccel);
                            (ptracker+radar_position)->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                            (ptracker+radar_position)->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                            double vcs_yaccvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yaccel);
                            (ptracker+radar_position)->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                            (ptracker+radar_position)->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);


                            unsigned8_T object_class = p_tracker_flr4p_gpo.object[i].object_class;


                           /* if (object_class == 0)
                            {
                                file1_object_classification[object_class].push_back(1);
                            }
                            else if (object_class == 1)
                            {
                                file1_ped_count++;
                                file1_object_classification[object_class].push_back(file1_ped_count);
                            }
                            else if (object_class == 2)
                            {
                                file1_2wheeler_count++;
                                file1_object_classification[object_class].push_back(file1_2wheeler_count);
                            }
                            else if (object_class == 3)
                            {
                                file1_car_count++;
                                file1_object_classification[object_class].push_back(file1_car_count);
                            }
                            else if (object_class == 4)
                            {
                                file1_truck_count++;
                                file1_object_classification[object_class].push_back(file1_truck_count);
                            }

                            unsigned8_T status = p_tracker_flr4p_gpo.object[i].status;

                            if (status == 1)
                            {
                                file1_newstatus_count++;
                                file1_object_status[status].push_back(file1_newstatus_count);
                            }
                            else if (status == 2)
                            {
                                file1_maturestatus_count++;
                                file1_object_status[status].push_back(file1_maturestatus_count);
                            }
                            else if (status == 3)
                            {
                                file1_coastedstatus_count++;
                                file1_object_status[status].push_back(file1_coastedstatus_count);
                            }*/

                        }
                    }

                }

                if (fileCount == 1)
                {
                    if (inRange(scanindexRangeRef[radar_position].front(), scanindexRangeRef[radar_position].back(), scanindex))
                    {

                        unsigned8_T status = p_tracker_flr4p_gpo.object[i].status;

                        if (status != 0)
                        {
                            if (p_tracker_flr4p_gpo.object[i].f_moving)
                            {
                                //file2_moving_objects++;
                                //vec_file2_moving_objects.push_back(file2_moving_objects);

                                (ptracker+radar_position)->vcs_si_resim.push_back(scanindex);

                                double vcs_xposvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xposn);
                                (ptracker+radar_position)->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                                (ptracker+radar_position)->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                                double vcs_yposvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yposn);
                                (ptracker+radar_position)->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                                (ptracker+radar_position)->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                                double vcs_xvelvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xvel);
                                (ptracker+radar_position)->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                                (ptracker+radar_position)->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                                double vcs_yvelvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yvel);
                                (ptracker+radar_position)->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                                (ptracker+radar_position)->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                                double vcs_xaccvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_xaccel);
                                (ptracker+radar_position)->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                                (ptracker+radar_position)->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                                double vcs_yaccvalue = flt_to_dbl(p_tracker_flr4p_gpo.object[i].vcs_yaccel);
                                (ptracker+radar_position)->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                                (ptracker+radar_position)->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);


                                unsigned8_T object_class = p_tracker_flr4p_gpo.object[i].object_class;


                               /* if (object_class == 0)
                                {
                                    file2_object_classification[object_class].push_back(1);
                                }
                                else if (object_class == 1)
                                {
                                    file2_ped_count++;
                                    file2_object_classification[object_class].push_back(file2_ped_count);
                                }
                                else if (object_class == 2)
                                {
                                    file2_2wheeler_count++;
                                    file2_object_classification[object_class].push_back(file2_2wheeler_count);
                                }
                                else if (object_class == 3)
                                {
                                    file2_car_count++;
                                    file2_object_classification[object_class].push_back(file2_car_count);
                                }
                                else if (object_class == 4)
                                {
                                    file2_truck_count++;
                                    file2_object_classification[object_class].push_back(file2_truck_count);
                                }*/



                                unsigned8_T status = p_tracker_flr4p_gpo.object[i].status;


                              /*  if (status == 1)
                                {
                                    file2_newstatus_count++;
                                    file2_object_status[status].push_back(file2_newstatus_count);
                                }
                                else if (status == 2)
                                {
                                    file2_maturestatus_count++;
                                    file2_object_status[status].push_back(file2_maturestatus_count);
                                }
                                else if (status == 3)
                                {
                                    file2_coastedstatus_count++;
                                    file2_object_status[status].push_back(file2_coastedstatus_count);
                                }*/



                            }
                        }
                    }

                }
            }
        }
        if (count == 0)
        {
            (ptracker+radar_position)->map_xpos[fileCount][scanindex] = (ptracker+radar_position)->vsc_xpos_scaleplot[fileCount];
            (ptracker+radar_position)->map_ypos[fileCount][scanindex] = (ptracker+radar_position)->vsc_ypos_scaleplot[fileCount];

            (ptracker+radar_position)->map_xvel[fileCount][scanindex] = (ptracker+radar_position)->vsc_xvel_scaleplot[fileCount];
            (ptracker+radar_position)->map_yvel[fileCount][scanindex] = (ptracker+radar_position)->vsc_yvel_scaleplot[fileCount];

            (ptracker+radar_position)->map_xacc[fileCount][scanindex] = (ptracker+radar_position)->vsc_xacc_scaleplot[fileCount];
            (ptracker+radar_position)->map_yacc[fileCount][scanindex] = (ptracker+radar_position)->vsc_yacc_scaleplot[fileCount];



            (ptracker+radar_position)->vsc_xpos_scaleplot[fileCount].clear();
            (ptracker+radar_position)->vsc_ypos_scaleplot[fileCount].clear();
            (ptracker+radar_position)->vsc_xvel_scaleplot[fileCount].clear();
            (ptracker+radar_position)->vsc_yvel_scaleplot[fileCount].clear();
            (ptracker+radar_position)->vsc_xacc_scaleplot[fileCount].clear();
            (ptracker+radar_position)->vsc_yacc_scaleplot[fileCount].clear();



        }
#endif
    }

}



