#include<iostream>
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../SharedData/SharedPlotData.h"
#include "../helper/genutil.h"
#include "../../Common/IRadarStream.h"

#include<matplot/matplot.h>
#include <filesystem>
#include"../DataProxy/DataProxy.h"
#ifdef _WIN32
#include <direct.h>// for _mkdir
#endif
#include "../../CommonFiles/inc/ctml.hpp"

#include <fstream>
#include <iostream>
class Gen5PlatformDataCollector
{

    Gen5PlatformDataCollector();
    ~Gen5PlatformDataCollector();
public:
    Gen5PlatformDataCollector(Gen5PlatformDataCollector&) = delete;
    Gen5PlatformDataCollector& operator=(Gen5PlatformDataCollector&) = delete;
    static Gen5PlatformDataCollector& getInstance();

    void PlotParaMemAllocation();
    void PlotParaMemDeallocation();


    double flt_to_dbl(float32_T con_value);
    bool inRange(double vehiclesi_start, double vehiclesi_end, double resimsi);

    //Data extraction and Data collection
    void CollectData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    void Collect_RangeMisMatch_inDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data);

    void Collect_Addition_MissingRangeDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data);
    void Collect_Addition_MissingAzimuthDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data);
    void Collect_Addition_MissingElevationDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data);
    void Collect_Addition_MissingAmplitudeDetections_RL(int filecnt, const std::map<double, std::vector<double>>& veh_det_data, const std::map<double, std::vector<double>>& resim_det_data);

    void CollectDetectionDataRL(Self_Detections_Param_T* selfdetecion, int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex);
    void CheckCollectedDetectionvalues();

   //Data Preparation
    void PlotDataPrep_Missing_Additional_Detection_count_RL();

    //Plotting Data
    void Generateplot(const char* inputVehFileNameBMW, const char* pltfoldername, std::string reportpath);
    int GenerateMissingDetectionCountPlot(Self_Detections_Param_T* selfdetect, const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt);
    int GenerateDetectionPlots(Self_Detections_Param_T* selfdetec, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int arraycnt);
    

    int PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt);
   

    Self_Detections_Param_T *ptr_gpo_selfdetection_RL;
    char inputVehFileNameHonda[_MAX_PATH] = { 0 };

    

    bool IsMissingDetection;
    bool IsAdditionalDetection;

    vector<double> vFLscanindex, vFRscanindex, vRRscanindex, vRLscanindex;

    std::vector<std::string> plotparams;
    std::vector<std::string> trackdimension;

    //mature track
    std::vector<double> track_mismatch_cnt_RL_veh;
    std::vector<double> track_mismatch_cnt_RL_resim;

    std::vector<double> arr_track_mismatch_cnt_RL_veh[600];
    std::vector<double> arr_track_mismatch_cnt_RL_resim[600];

    std::vector<double> track_mismatch_si;
    std::vector<double> arr_track_mismatch_si[10000];
    std::vector<std::vector<double>> vv_track_mismatch_cnt;
    std::vector<std::string> trackinfo;
    std::vector<int> scanindex;

    std::vector<int> arr_scanindex[600];

    int arr_cnt;


    //mature track RR

    std::vector<double> track_mismatch_cnt_RR_veh;
    std::vector<double> track_mismatch_cnt_RR_resim;

    std::vector<double> arr_track_mismatch_cnt_RR_veh[600];
    std::vector<double> arr_track_mismatch_cnt_RR_resim[600];

    std::vector<double> track_mismatch_si_RR;
    std::vector<double> arr_track_mismatch_si_RR[600];
    std::vector<std::vector<double>> vv_track_mismatch_cnt_RR;
    std::vector<std::string> trackinfo_RR;
    std::vector<std::string> scanindex_RR;

    std::vector<std::string> arr_scanindex_RR[600];

    int arr_cnt_RR;


    //mature track FL

    std::vector<double> track_mismatch_cnt_FL_veh;
    std::vector<double> track_mismatch_cnt_FL_resim;

    std::vector<double> arr_track_mismatch_cnt_FL_veh[600];
    std::vector<double> arr_track_mismatch_cnt_FL_resim[600];

    std::vector<double> track_mismatch_si_FL;
    std::vector<double> arr_track_mismatch_si_FL[600];
    std::vector<std::vector<double>> vv_track_mismatch_cnt_FL;
    std::vector<std::string> trackinfo_FL;
    std::vector<std::string> scanindex_FL;

    std::vector<std::string> arr_scanindex_FL[600];

    int arr_cnt_FL;


    //mature track FR


    std::vector<double> track_mismatch_cnt_FR_veh;
    std::vector<double> track_mismatch_cnt_FR_resim;

    std::vector<double> arr_track_mismatch_cnt_FR_veh[600];
    std::vector<double> arr_track_mismatch_cnt_FR_resim[600];

    std::vector<double> track_mismatch_si_FR;
    std::vector<double> arr_track_mismatch_si_FR[600];
    std::vector<std::vector<double>> vv_track_mismatch_cnt_FR;
    std::vector<std::string> trackinfo_FR;
    std::vector<std::string> scanindex_FR;

    std::vector<std::string> arr_scanindex_FR[600];

    int arr_cnt_FR;

    std::map<std::vector<double>, std::vector<double>> RLmapmissing_detectionVeh;

   
    //int tracks_fig;
   
    string outsvg_commmon;
    ofstream logfile;

    std::vector<double> RL_detection_range_mismatch_scanindex;
    std::vector<double> RL_detection_range_mismatch_value;
    bool veh_sil;
    bool veh_hil;
    bool sil_hil;

    //Missing range detection value and its scanindex in resim 
    std::vector<double> Scanindex_missing_range_detection[4];
    std::vector<double> missing_range_detection_value_inResim[4];

    //Additional range detection value and its scanindex in resim 
    std::vector<double> Scanindex_additional_range_detection[4];  
    std::vector<double> additional_range_detection_value_inResim[4];

    //Mismatch range detection scanindex and value between veh and resim
    std::vector<double> scanindex_mismatch_range_detection_inVeh[4];
    std::vector<double> scanindex_mismatch_range_detection_inResim[4];
    std::vector<double> mismatch_range_detection_inVeh[4];
    std::vector<double> mismatch_range_detection_inResim[4];


    //Missing azimuth detection value and its scanindex in resim 
    std::vector<double> Scanindex_missing_azimuth_detection[4];
    std::vector<double> missing_azimuth_detection_value_inResim[4];

    //Additional azimuth detection value and its scanindex in resim 
    std::vector<double> Scanindex_additional_azimuth_detection[4];
    std::vector<double> additional_azimuth_detection_value_inResim[4];


    //Missing amplitude detection value and its scanindex in resim 
    std::vector<double> Scanindex_missing_amplitude_detection[4];
    std::vector<double> missing_amplitude_detection_value_inResim[4];

    //Additional amplitude detection value and its scanindex in resim 
    std::vector<double> Scanindex_additional_amplitude_detection[4];
    std::vector<double> additional_amplitude_detection_value_inResim[4];


    //Missing elevation detection value and its scanindex in resim 
    std::vector<double> Scanindex_missing_elevation_detection[4];
    std::vector<double> missing_elevation_detection_value_inResim[4];

    //Additional elevation detection value and its scanindex in resim 
    std::vector<double> Scanindex_additional_elevation_detection[4];
    std::vector<double> additional_elevation_detection_value_inResim[4];
};