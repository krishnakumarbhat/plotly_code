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
#include<set>
#include <iterator>
#include<algorithm>
class FLR4P
{

    FLR4P();
    ~FLR4P();
public:
    FLR4P(FLR4P&) = delete;
    FLR4P& operator=(FLR4P&) = delete;
    static FLR4P& getInstance();

    void PlotParaMemAllocation();
    void PlotParaMemDeallocation();

    
    double flt_to_dbl(float32_T con_value);
    bool inRange(double vehiclesi_start, double vehiclesi_end, double resimsi);

    //Data extraction and Data collection


    void CollectTrackerData_SRR6_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    void CollectDetectionData_SRR6_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    void CollectDetectionData_FLR4_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    void CollectTrackerData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    void CollectDetectionData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);
    
    void CollectTrackerData_BMW_MID(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform);


    //DataPreparation
    void DataPrep_SlicedTracker_plot(Tracker_Plot_Parameter_T *ptrackerplot,const char* f_name, const char* foldname, std::string reportpath,int radarpos);  
    void DataPreparation_Slicing_Detection_Scatterplot(const char* f_name, const char* foldname, std::string reportpath,int radarpos);
   
    
    //Plotting Data 
    void Generateplot(const char* inputVehFileNameBMW, const char* pltfoldername, std::string reportpath);  
    
    //Different plots
    int GenerateScatterPlot(std::vector<double> file1_xdata,
                            std::vector<double> file1_ydata,
                            std::vector<double> file2_xdata,
                            std::vector<double> file2_ydata,
                            int fileindex,
                            const char* f_name,
                            const char* current_radar,
                            const char* foldname,
                            std::string plotparam,
                            std::string reportpath,
                            int arraycnt);

    int GenerateBoxPlot(std::vector<double> file1_xdata,
                       std::vector<double> file1_ydata,
        std::vector<double> file2_xdata,
        std::vector<double> file2_ydata,
        int fileindex,
        const char* f_name,
        const char* current_radar,
        const char* foldname,
        std::string plotparam,
        std::string reportpath,
        int arraycnt);

    int GenerateBoxPlot_New(std::vector<std::vector<double>>file1_xdata,     
        int fileindex,
        const char* f_name,
        const char* current_radar,
        const char* foldname,
        std::string plotparam,
        std::string reportpath,
        int arraycnt);


    int GenerateBarPlots( std::vector<string> xdata,
                          std::vector<std::vector<int>> ydata,
                          const char* f_name,
                          const char* current_radar,
                          const char* foldname,
                          std::string plotparam,
                          std::string reportpath,
                          int arraycnt);

    int GenerateHistogramPlot(std::vector<double> f1_detection_data,
                              std::vector<double> f2_detection_data,
                              const char* f_name,
                              const char* current_radar,
                              const char* foldname,
                              std::string plotparam,
                              std::string reportpath,
                              int arraycnt,
                              int zerodatafilenumber);



    int PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt);
   

    Self_Detections_Param_T *ptr_gpo_selfdetection_RL;
    Tracker_Plot_Parameter_T* pTrackerPlotParamDC;
    Tracker_Plot_Parameter_T* pTrackerPlotParamFC_FLR4P;
    Tracker_Plot_Parameter_T* pTrackerPlotParam[4];
    SelfDetection_Plot_Parameter_T*pSelfDetectionPlotParameterFC;

    //SelfDetection_Plot_Parameter* pSelfDetectionPlotParameterFC[7];
    Tracker_Plot_Parameter_T* pTrackerPlotParamFC_FLR4P_file1, * pTrackerPlotParamFC_FLR4P_file2;

    char inputVehFileNameHonda[_MAX_PATH] = { 0 };

    std::vector<double> FC_scanindex;
    std::vector<double> Scanindex[4];
    

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
    ofstream logfile, logfile1,file1_data,file2_data,debugdata;

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


    //Mismatch azimuth detection scanindex and value between veh and resim
    std::vector<double> scanindex_mismatch_azimuth_range_detection_inVeh[4];
    std::vector<double> scanindex_mismatch_azimuth_range_detection_inResim[4];
    std::vector<double> mismatch_azimuth_detection_inVeh[4];
    std::vector<double> mismatch_azimuth_detection_inResim[4];


    //Mismatch elevation detection scanindex and value between veh and resim
    std::vector<double> scanindex_mismatch_elevation_range_detection_inVeh[4];
    std::vector<double> scanindex_mismatch_elevation_range_detection_inResim[4];
    std::vector<double> mismatch_elevation_detection_inVeh[4];
    std::vector<double> mismatch_elevation_detection_inResim[4];


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


    std::vector<double> tempdata[NUMBER_OF_FILES];

    std::vector<std::vector<int>> vec_object_classification;
    std::vector<int> file1_object_classification[5];
    std::vector<int> file2_object_classification[5];

    int file1_car_count, file1_truck_count, file1_ped_count, file1_2wheeler_count;
    int file2_car_count, file2_truck_count, file2_ped_count, file2_2wheeler_count;


    std::vector<std::vector<int>> vec_object_status;
    std::vector<int> file1_object_status[5];
    std::vector<int> file2_object_status[5];

    int file1_newstatus_count, file1_maturestatus_count, file1_coastedstatus_count;
    int file2_newstatus_count, file2_maturestatus_count, file2_coastedstatus_count;


    std::vector<std::vector<int>> vec_moving_objects;
    std::vector<int> vec_file1_moving_objects;
    std::vector<int> vec_file2_moving_objects;

    int file1_moving_objects, file2_moving_objects;


    std::vector<std::vector<int>> vec_bistatic_det,vec_singletarget_det, vec_singletargetazimuth_det, vec_dopplermixedinterval_det,vec_dopplerunfolding_det,vec_bf_type_az, vec_bf_type_el, vec_super_res_target,vec_outside_sector;
    std::vector<int> vec_file1_singletarget_det, vec_file1_singletargetazimuth_det, vec_file1_dopplermixedinterval_det, vec_file1_dopplerunfolding_det, vec_file1_bf_type_az, vec_file1_bf_type_el, vec_file1_super_res_target, vec_file1_outside_sector;
    std::vector<int> vec_file2_singletarget_det, vec_file2_singletargetazimuth_det, vec_file2_dopplermixedinterval_det, vec_file2_dopplerunfolding_det, vec_file2_bf_type_az, vec_file2_bf_type_el, vec_file2_super_res_target, vec_file2_outside_sector;

    std::vector<double> vec_file1_bistatic_det, vec_file2_bistatic_det;

    int file1_bistatic_det, file1_singletarget_det, file1_singletargetazimuth_det, file1_dopplermixedinterval_det, file1_dopplerunfolding_det, file1_bf_type_az, file1_bf_type_el, file1_super_res_target, file1_outside_sector;
    int file2_bistatic_det, file2_singletarget_det, file2_singletargetazimuth_det, file2_dopplermixedinterval_det, file2_dopplerunfolding_det, file2_bf_type_az, file2_bf_type_el, file2_super_res_target, file2_outside_sector;

    int azimuth_out_of_range_count[2];
    int elevation_out_of_range_count[2];

    

};