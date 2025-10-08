
#include "STLA_FLR4P.h"
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
#define PI 3.14159265359
#define RAD2DEG(x) ((x * 180.00) / PI)

using namespace std;
namespace fs = std::filesystem;
using namespace matplot;
extern RADAR_Plugin_Interface_T* m_radar_plugin;
STLA_Detection_Log_Data_T GPO_Dets_info_t1 = { 0 };
Tracker_Log_Data_FLR4P_T Flr4p_Tracker_info1 = { 0 };
//STLA_Detection_Log_Data_T STLA_Dets_info = { 0 };

Detection_Log_Data_FLR4P_T Flr4p_Dets_info1 = { 0 };
STLA_Detection_Log_Data_T STLA_Dets_info1 = { 0 };
STLA_Detection_Log_Data_T STLA_FLR4_Dets_info1= { 0 };


STLA_SRR6P_Alignment_Log_Data_T p_veh_stla1 = { 0 };
STLA_FLR4_Alignment_Log_Data_T p_veh_stla_flr41 = { 0 };

STLA_C2_Diagnostics_Logging_Data_T Diagnostics1 = { 0 };
//STLA_Header_Log_Data_T 
//Header_Log_Data_T
VEHICLE_DATA_T p_veh1 = { 0 };
C2_Tracker_Logging_Data_GEN5_T gen5_curvi_track_XML1 = { 0 };

Tracker_Info_Log_XML_T p_g_tracker_OAL_ping_xml1 = { 0 };
All_Objects_Log_XML_T ALL_object_xml1 = { 0 };

float32_T* array_data_ptrFLR4P, * array_data_ptrFLR4;
auto tracks_fig2 = figure(true);

//Start of GenericFunctions
float roundv(float var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

inline static const string roundDouble(const double input, const int decimal_places)
{
    ostringstream str;
    str << fixed << setprecision(decimal_places);
    str << input;
    return str.str();
}
int comparedataFLR4P(const void* a, const void* b)
{ 
    unsigned8_T value_of_a = *(unsigned8_T*)a;
    unsigned8_T value_of_b = *(unsigned8_T*)b;
    if (array_data_ptrFLR4P[value_of_a] < array_data_ptrFLR4P[value_of_b]) {
        return -1;
    }
    else if (array_data_ptrFLR4P[value_of_a] == array_data_ptrFLR4P[value_of_b])
    {

    }
    else
    {
        return (array_data_ptrFLR4P[value_of_a] > array_data_ptrFLR4P[value_of_b]);
    }
}

string set_out_html_pathFLR4P(const char* f_name)
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







double FLR4P::flt_to_dbl(float32_T con_value)
{
    char chr_fValue[32];//32
    sprintf(chr_fValue, "%9.8f", con_value);//9.8
    double db_val = strtod(chr_fValue, NULL);
    return db_val;
}
bool FLR4P::inRange(double vehiclesi_start, double vehiclesi_end, double resimsi)
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
FLR4P::FLR4P()
{

    logfile.open("logfile.txt", ios::out);
    logfile1.open("logfile1.txt", ios::out);

    debugdata.open("debugdata.txt", ios::out);



    file1_data.open("file1_data.txt", ios::out);
    file2_data.open("file2_data.txt", ios::out);

    PlotParaMemAllocation();
}

FLR4P::~FLR4P()
{
    logfile.close();
    logfile1.close();
    debugdata.close();
    file1_data.close();
    file2_data.close();

}

FLR4P& FLR4P::getInstance()
{
	static FLR4P instance;
	return instance;
}
void FLR4P::PlotParaMemAllocation()
{


  /*  pTrackerPlotParamFC_FLR4P = new Tracker_Plot_Parameter_T();*/

    

   /* for (int sensor_pos=0; sensor_pos <7; sensor_pos++)
    {
        pSelfDetectionPlotParameterFC[sensor_pos] = new SelfDetection_Plot_Parameter_T();
    }*/

    pSelfDetectionPlotParameterFC = new SelfDetection_Plot_Parameter_T[7];
    pTrackerPlotParamDC = new Tracker_Plot_Parameter_T;
    

   /*  for (int sensor_pos = 0; sensor_pos < 4; sensor_pos++)
     {
            pTrackerPlotParam[sensor_pos] = new Tracker_Plot_Parameter_T();
     }*/

 

}
void FLR4P::PlotParaMemDeallocation()
{
    //cout << "\nPlotParaMemDeallocation";
	/*if (pTrackerPlotParamFC_FLR4P != nullptr)
		delete pTrackerPlotParamFC_FLR4P;*/

    //if(pSelfDetectionPlotParameterFC!=nullptr)
       delete[] pSelfDetectionPlotParameterFC;
       delete pTrackerPlotParamDC;

   /* if(pTrackerPlotParam!=nullptr)
        delete[] pTrackerPlotParam;*/

	
}
void FLR4P::CollectTrackerData_SRR6_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{

  
    int radar_postion = int(Radar_Posn);
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&gen5_curvi_track_XML1, CURVI_TRACK_DATA);

    DVSU_RECORD_T record = { 0 };
   

    if (custId == PLATFORM_GEN5 || custId == NISSAN_GEN5 || custId == HONDA_GEN5 || custId == STLA_SRR6P)
    {
       
        /*sort by range*/
        float32_T Track_Objs[GEN5_NUMBER_OF_OBJECTS_L] = { 0 };
        for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
        {
            Track_Objs[i] = gen5_curvi_track_XML1.CurviTracks[i].vcs_long_posn;
        }
        unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
        unsigned8_T index[GEN5_NUMBER_OF_OBJECTS_L] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptrFLR4P = Track_Objs;
        qsort(index, size, sizeof(*index), comparedataFLR4P);

        /*print sorted Opp dets*/
        unsigned8_T count = 1;
        for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
        {
            /*check for valid Track status level*/
            if (((gen5_curvi_track_XML1.CurviTracks[index[i]].status) > 0) && (count == 1))
            {
                count = 0;
                if (fileCount == 0)
                {
                    pTrackerPlotParam[radar_postion]->vcs_si_veh_ref_scale.push_back(scanindex);
                }
                if (fileCount == 1)
                {
                    pTrackerPlotParam[radar_postion]->vcs_si_resim_ref_scale.push_back(scanindex);
                }
              
                
            }
            if ((gen5_curvi_track_XML1.CurviTracks[index[i]].status) > 0)
            {
                if (fileCount == 0)
                {
                   
                    
                        pTrackerPlotParam[radar_postion]->vcs_si_veh.push_back(scanindex);
                        FC_scanindex.push_back(scanindex);

                        double vcs_xposvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].vcs_long_posn);
                        pTrackerPlotParam[radar_postion]->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                        pTrackerPlotParam[radar_postion]->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                        double vcs_yposvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].vcs_lat_posn);
                        pTrackerPlotParam[radar_postion]->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                        pTrackerPlotParam[radar_postion]->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                        double vcs_xvelvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].curvi_long_vel);
                        pTrackerPlotParam[radar_postion]->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                        pTrackerPlotParam[radar_postion]->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                        double vcs_yvelvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].curvi_lat_vel);
                        pTrackerPlotParam[radar_postion]->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                        pTrackerPlotParam[radar_postion]->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                        double vcs_xaccvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].ref_long_accel);
                        pTrackerPlotParam[radar_postion]->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                        pTrackerPlotParam[radar_postion]->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                        double vcs_yaccvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].ref_lat_accel);
                        pTrackerPlotParam[radar_postion]->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                        pTrackerPlotParam[radar_postion]->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);

                    
                   

                    
                }

                if (fileCount == 1)
                {

                    if (inRange(FC_scanindex.front(), FC_scanindex.back(), scanindex))
                    {


                       

                            pTrackerPlotParam[radar_postion]->vcs_si_resim.push_back(scanindex);
                           

                            double vcs_xposvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].vcs_long_posn);
                            pTrackerPlotParam[radar_postion]->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                            pTrackerPlotParam[radar_postion]->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                            double vcs_yposvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].vcs_lat_posn);
                            pTrackerPlotParam[radar_postion]->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                            pTrackerPlotParam[radar_postion]->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                            double vcs_xvelvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].curvi_long_vel);
                            pTrackerPlotParam[radar_postion]->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                            pTrackerPlotParam[radar_postion]->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                            double vcs_yvelvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].curvi_lat_vel);
                            pTrackerPlotParam[radar_postion]->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                            pTrackerPlotParam[radar_postion]->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                            double vcs_xaccvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].ref_long_accel);
                            pTrackerPlotParam[radar_postion]->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                            pTrackerPlotParam[radar_postion]->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                            double vcs_yaccvalue = flt_to_dbl(gen5_curvi_track_XML1.CurviTracks[index[i]].ref_lat_accel);
                            pTrackerPlotParam[radar_postion]->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                            pTrackerPlotParam[radar_postion]->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);
                        
                    }

                }




               
                
               
            }
        }
        if (count == 0)
        {
          
            pTrackerPlotParam[radar_postion]->map_xpos[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_xpos_scaleplot[fileCount];
            pTrackerPlotParam[radar_postion]->map_ypos[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_ypos_scaleplot[fileCount];

            pTrackerPlotParam[radar_postion]->map_xvel[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_xvel_scaleplot[fileCount];
            pTrackerPlotParam[radar_postion]->map_yvel[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_yvel_scaleplot[fileCount];

            pTrackerPlotParam[radar_postion]->map_xacc[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_xacc_scaleplot[fileCount];
            pTrackerPlotParam[radar_postion]->map_yacc[fileCount][scanindex] = pTrackerPlotParam[radar_postion]->vsc_yacc_scaleplot[fileCount];



            pTrackerPlotParam[radar_postion]->vsc_xpos_scaleplot[fileCount].clear();
            pTrackerPlotParam[radar_postion]->vsc_ypos_scaleplot[fileCount].clear();
            pTrackerPlotParam[radar_postion]->vsc_xvel_scaleplot[fileCount].clear();
            pTrackerPlotParam[radar_postion]->vsc_yvel_scaleplot[fileCount].clear();
            pTrackerPlotParam[radar_postion]->vsc_xacc_scaleplot[fileCount].clear();
            pTrackerPlotParam[radar_postion]->vsc_yacc_scaleplot[fileCount].clear();
        }

    }
}
void FLR4P::CollectDetectionData_SRR6_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
  

    
    std::vector<double> vtemp_c2_timing_info; 
    int radar_postion = int(Radar_Posn);

  
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Diagnostics1, Diagnostics_Data);     
    for (int i = 0; i < 24; i++)
    {

       
        vtemp_c2_timing_info.push_back(double(Diagnostics1.Timing_Info_c2[i]));
    }
    

    (pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[fileCount].insert(pair<double, std::vector<double>>(scanindex, vtemp_c2_timing_info));
     vtemp_c2_timing_info.clear();


     m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla1, Alignment_Data);

     (pSelfDetectionPlotParameterFC + radar_postion)->speed[fileCount].push_back(flt_to_dbl(p_veh_stla1.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed));
     (pSelfDetectionPlotParameterFC + radar_postion)->yawrate[fileCount].push_back(flt_to_dbl(p_veh_stla1.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate));
    
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&STLA_Dets_info1, DETS_COMP);




    



   /* fprintf(SIL_XML_out, "\n\t<VEH_DATA_INFO_%d_%s>\n", g_scanindex, GetRadarPosName(Radar_Posn));
    fprintf(SIL_XML_out, "\tabs_speed \t%f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed);
    fprintf(SIL_XML_out, "\tyawrate	\t %f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate);
    fprintf(SIL_XML_out, "\tf_reverse\t %f\n", p_veh_stla.Align_Input.Dyn_Align_input.Dyn_Vehicle.f_reverse);*/


    //m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh1,DSPACE_VEH);//VEH
    /*sort by range*/
    float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
    for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    {
        Self_Dets[i] = STLA_Dets_info1.target_report[i].range;
    }
    unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
    unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
    for (unsigned8_T i = 0; i < size; i++)
    {
        index[i] = i;
    }
    array_data_ptrFLR4P = Self_Dets;
    qsort(index, size, sizeof(*index), comparedataFLR4P);
   
   
    /*print sorted self dets*/
    unsigned8_T count = 1;
    for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    {
        /*check for  non zero range*/

        if ((STLA_Dets_info1.target_report[index[i]].range != 0) && (count == 1))
        {

            if (fileCount == 0)
            {
                (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[fileCount].push_back(double(STLA_Dets_info1.target_count));
                (pSelfDetectionPlotParameterFC + radar_postion)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(STLA_Dets_info1.target_count)));

               
            }
            if (fileCount == 1)
            {
                (pSelfDetectionPlotParameterFC + radar_postion)->si_resim_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[fileCount].push_back(double(STLA_Dets_info1.target_count));
                (pSelfDetectionPlotParameterFC + radar_postion)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(STLA_Dets_info1.target_count)));
               
               
            }

            count = 0;
           
        }
        if (STLA_Dets_info1.target_report[index[i]].range != 0)
        {
            if (fileCount == 0)
            {
                    //cout << "\n F1 SI " << scanindex << "veh speed " << p_veh1.abs_speed;

                    FC_scanindex.push_back(scanindex);
                    Scanindex[radar_postion].push_back(scanindex);
               
                    double rangevalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].range);
                    float rangefloat = STLA_Dets_info1.target_report[index[i]].range;
                    float rangeresults = roundv(rangefloat);
                    double testrange = round(rangevalue * 100.0) / 100.0;
                    
                    string rangestr=roundDouble(rangevalue, 2);
                    double rangestrTodouble = stod(rangestr);

                   
                    if(rangevalue>=-1 && rangevalue<=500)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->range[fileCount].push_back(rangevalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->rangef[fileCount].push_back(rangevalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);
                    }

                   

                    double rangeratevalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].range_rate);

                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }


                    double ampvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].amp);

                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->amp[fileCount].push_back(ampvalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }
                    

                    double rcsvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].rcs);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->rcs[fileCount].push_back(rcsvalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }
                   

                    double snrvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].snr);

                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->snr[fileCount].push_back(snrvalue);
                        (pSelfDetectionPlotParameterFC + radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }
                    


                    double azimuth = flt_to_dbl(RAD2DEG(STLA_Dets_info1.target_report[index[i]].azimuth));
                    if (azimuth >= -90 && azimuth <= 90)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[fileCount].push_back(azimuth);
                        (pSelfDetectionPlotParameterFC + radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }
                   

                    if (trunc((azimuth * -1)) > 65)
                    {
                       
                        azimuth_out_of_range_count[fileCount]++;
                    }

                    double elevation = flt_to_dbl(RAD2DEG(STLA_Dets_info1.target_report[index[i]].elevation));
                    if (elevation >= -40 && elevation <= 40)
                    {
                        (pSelfDetectionPlotParameterFC + radar_postion)->elevation[fileCount].push_back(elevation);
                        (pSelfDetectionPlotParameterFC + radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);
                    }
                    

                    double az_conf_value = double(STLA_Dets_info1.target_report[index[i]].az_conf);
                    double el_conf_value = double(STLA_Dets_info1.target_report[index[i]].el_conf);

                    (pSelfDetectionPlotParameterFC + radar_postion)->az_conf[fileCount].push_back(az_conf_value);
                    (pSelfDetectionPlotParameterFC + radar_postion)->el_conf[fileCount].push_back(el_conf_value);

                    if (trunc((elevation * -1)) > 10)
                    {
                        
                        elevation_out_of_range_count[fileCount]++;
                    }

                    uint8_t isBistatic = STLA_Dets_info1.target_report[index[i]].isBistatic;

                    if (isBistatic == 1)
                    {
                        
                        file1_bistatic_det++;
                        vec_file1_bistatic_det.push_back(file1_bistatic_det);
                    }

                    uint8_t isSingleTarget = STLA_Dets_info1.target_report[index[i]].isSingleTarget;
                    if (isSingleTarget == 1)
                    {
                        file1_singletarget_det++;
                        vec_file1_singletarget_det.push_back(file1_singletarget_det);

                    }

                    uint8_t isSingleTarget_azimuth = STLA_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                    if (isSingleTarget_azimuth == 1)
                    {
                        file1_singletargetazimuth_det++;
                        vec_file1_singletargetazimuth_det.push_back(file1_singletargetazimuth_det);
                    }

                    uint8_t f_Doppler_mixed_interval = STLA_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                    if (f_Doppler_mixed_interval == 1)
                    {
                        file1_dopplermixedinterval_det++;
                        vec_file1_dopplermixedinterval_det.push_back(file1_dopplermixedinterval_det);
                    }

                    uint8_t f_valid_Doppler_unfolding = STLA_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                    if (f_valid_Doppler_unfolding == 1)
                    {
                        file1_dopplerunfolding_det++;
                        vec_file1_dopplerunfolding_det.push_back(file1_dopplerunfolding_det);
                    }

                    uint8_t bf_type_az = STLA_Dets_info1.target_report[index[i]].bf_type_az;
                    if (bf_type_az == 1)
                    {
                        file1_bf_type_az++;
                        vec_file1_bf_type_az.push_back(file1_bf_type_az);

                    }

                    uint8_t bf_type_el = STLA_Dets_info1.target_report[index[i]].bf_type_el;
                    if (bf_type_el == 1)
                    {
                        file1_bf_type_el++;
                        vec_file1_bf_type_el.push_back(file1_bf_type_el);

                    }

                    uint8_t super_res_target = STLA_Dets_info1.target_report[index[i]].super_res_target;
                    if (super_res_target == 1)
                    {
                        file1_super_res_target++;
                        vec_file1_super_res_target.push_back(file1_super_res_target);
                    }

                    uint8_t super_res_target_type = STLA_Dets_info1.target_report[index[i]].super_res_target_type;
                    if (super_res_target_type == 1)
                    {

                    }

                    uint8_t outside_sector = STLA_Dets_info1.target_report[index[i]].outside_sector;
                    if (outside_sector == 1)
                    {
                        file1_outside_sector++;
                        vec_file1_outside_sector.push_back(file1_outside_sector);
                    }




                    


                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -90 && azimuth <= 90) ||
                        elevation >= -40 && elevation <= 40)
                    {

                        (pSelfDetectionPlotParameterFC + radar_postion)->si_veh.push_back(scanindex);
                       
                    }

                    
               
                


            }

            if (fileCount == 1)
            {

               
                    if (inRange(Scanindex[radar_postion].front(), Scanindex[radar_postion].back(), scanindex))
                    {
                        
                        //cout << "\n F2 SI " << scanindex << "veh speed " << p_veh1.abs_speed;

                        double rangevalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].range);
                    
                        float rangefloat = STLA_Dets_info1.target_report[index[i]].range;
                        float rangeresults = roundv(rangefloat);
                        double testrange = round(rangevalue * 100.0) / 100.0;

                        string rangestr = roundDouble(rangevalue, 2);
                        double rangestrTodouble = stod(rangestr);

                        

                        if (rangevalue >= -1 && rangevalue <= 500)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->range[fileCount].push_back(rangevalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->rangef[fileCount].push_back(rangevalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);
                        }

                        double rangeratevalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].range_rate);
                        if (rangeratevalue >= -30 && rangeratevalue <=30)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                        }
                      
                   


                        double ampvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].amp);
                        if (ampvalue >= -100 && ampvalue<=100)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->amp[fileCount].push_back(ampvalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);
                        }
                      

                        double rcsvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].rcs);
                        if (rcsvalue >= -100 && rcsvalue <= 100)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->rcs[fileCount].push_back(rcsvalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                        }
                      

                        double snrvalue = flt_to_dbl(STLA_Dets_info1.target_report[index[i]].snr);
                        if (snrvalue >=-100 && snrvalue <= 100)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->snr[fileCount].push_back(snrvalue);
                            (pSelfDetectionPlotParameterFC + radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);
                        }
                       


                        double azimuth = flt_to_dbl(RAD2DEG(STLA_Dets_info1.target_report[index[i]].azimuth));
                        if (azimuth >= -90 && azimuth <=90)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[fileCount].push_back(azimuth);
                            (pSelfDetectionPlotParameterFC + radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);
                        }
                      


                        if (trunc((azimuth * -1)) > 65)
                        {
                            
                            azimuth_out_of_range_count[fileCount]++;
                        }
                        double elevation = flt_to_dbl(RAD2DEG(STLA_Dets_info1.target_report[index[i]].elevation));
                        if (elevation>=-40 && elevation<=40)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->elevation[fileCount].push_back(elevation);
                            (pSelfDetectionPlotParameterFC + radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);
                            
                        }

                        double az_conf_value = double(STLA_Dets_info1.target_report[index[i]].az_conf);
                        double el_conf_value = double(STLA_Dets_info1.target_report[index[i]].el_conf);

                        (pSelfDetectionPlotParameterFC + radar_postion)->az_conf[fileCount].push_back(az_conf_value);
                        (pSelfDetectionPlotParameterFC + radar_postion)->el_conf[fileCount].push_back(el_conf_value);
                     

                        if (trunc((elevation * -1)) > 10)
                        {
                           
                            elevation_out_of_range_count[fileCount]++;
                        }

                        uint8_t isBistatic = STLA_Dets_info1.target_report[index[i]].isBistatic;

                        if (isBistatic == 1)
                        {
                          
                            file2_bistatic_det++;
                            vec_file2_bistatic_det.push_back(file2_bistatic_det);
                        }

                        uint8_t isSingleTarget = STLA_Dets_info1.target_report[index[i]].isSingleTarget;
                        if (isSingleTarget == 1)
                        {
                            file2_singletarget_det++;
                            vec_file2_singletarget_det.push_back(file2_singletarget_det);

                        }

                        uint8_t isSingleTarget_azimuth = STLA_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                        if (isSingleTarget_azimuth == 1)
                        {
                            file2_singletargetazimuth_det++;
                            vec_file2_singletargetazimuth_det.push_back(file2_singletargetazimuth_det);
                        }

                        uint8_t f_Doppler_mixed_interval = STLA_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                        if (f_Doppler_mixed_interval == 1)
                        {
                            file2_dopplermixedinterval_det++;
                            vec_file2_dopplermixedinterval_det.push_back(file2_dopplermixedinterval_det);
                        }

                        uint8_t f_valid_Doppler_unfolding = STLA_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                        if (f_valid_Doppler_unfolding == 1)
                        {
                            file2_dopplerunfolding_det++;
                            vec_file2_dopplerunfolding_det.push_back(file2_dopplerunfolding_det);
                        }

                        uint8_t bf_type_az = STLA_Dets_info1.target_report[index[i]].bf_type_az;
                        if (bf_type_az == 1)
                        {
                            file2_bf_type_az++;
                            vec_file2_bf_type_az.push_back(file2_bf_type_az);

                        }

                        uint8_t bf_type_el = STLA_Dets_info1.target_report[index[i]].bf_type_el;
                        if (bf_type_el == 1)
                        {
                            file2_bf_type_el++;
                            vec_file2_bf_type_el.push_back(file2_bf_type_el);

                        }

                        uint8_t super_res_target = STLA_Dets_info1.target_report[index[i]].super_res_target;
                        if (super_res_target == 1)
                        {
                            file2_super_res_target++;
                            vec_file2_super_res_target.push_back(file2_super_res_target);
                        }

                        uint8_t super_res_target_type = STLA_Dets_info1.target_report[index[i]].super_res_target_type;
                        if (super_res_target_type == 1)
                        {

                        }

                        uint8_t outside_sector = STLA_Dets_info1.target_report[index[i]].outside_sector;
                        if (outside_sector == 1)
                        {
                            file2_outside_sector++;
                            vec_file2_outside_sector.push_back(file2_outside_sector);
                        }


                      

                        if ((rangevalue >= -1 && rangevalue <= 500)||
                            (rangeratevalue >= -100 && rangeratevalue <= 100)||
                            (ampvalue >= -100 && ampvalue <= 100)||
                            (snrvalue >= -100 && snrvalue <= 100)||
                            (azimuth >= -90 && azimuth <= 90)||
                            elevation >= -40 && elevation <= 40)
                        {
                            (pSelfDetectionPlotParameterFC + radar_postion)->si_resim.push_back(scanindex);
                        }

                        
                    }


                
               
            }




        }
    }
    if (count == 0)
    {
        
        (pSelfDetectionPlotParameterFC + radar_postion)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC + radar_postion)->map_rangef[fileCount].insert(pair<float, std::vector<float>>((float)scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount]));
        (pSelfDetectionPlotParameterFC + radar_postion)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC + radar_postion)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC + radar_postion)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC + radar_postion)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount]));
            (pSelfDetectionPlotParameterFC+radar_postion)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount]));
            (pSelfDetectionPlotParameterFC+radar_postion)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount]));


       
           /* std::vector<double>::iterator itr_maxValue;

            double max_value_temp;
            max_value_temp = *std::max_element((pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].begin(),
                                              (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].end());*/

            //cout << "\n FILE  " << fileCount << "SI " << scanindex << "radar " << radar_postion << " max range " << max_value_temp;
            //(pSelfDetectionPlotParameterFC+radar_postion)->range_max[fileCount].push_back(max_value_temp);


            (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].clear();
            (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount].clear(); 
            
            
    }





}
void FLR4P::CollectDetectionData_FLR4_Platform(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{

    std::vector<double> vtemp_c2_timing_info;

    int radar_postion = int(Radar_Posn);


   
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Diagnostics1, Diagnostics_Data);
    for (int i = 0; i < 24; i++)
    {

       
        vtemp_c2_timing_info.push_back(double(Diagnostics1.Timing_Info_c2[i]));
    }


    (pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[fileCount].insert(pair<double, std::vector<double>>(scanindex, vtemp_c2_timing_info));
    vtemp_c2_timing_info.clear();


    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_veh_stla_flr41, Alignment_Data);

    (pSelfDetectionPlotParameterFC + radar_postion)->speed[fileCount].push_back(flt_to_dbl(p_veh_stla_flr41.Align_Input.Dyn_Align_input.Dyn_Vehicle.speed));
    (pSelfDetectionPlotParameterFC + radar_postion)->yawrate[fileCount].push_back(flt_to_dbl(p_veh_stla_flr41.Align_Input.Dyn_Align_input.Dyn_Vehicle.yawrate));

    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&STLA_FLR4_Dets_info1, DETS_COMP);
    /*sort by range*/
    float32_T Self_Dets[AF_DET_MAX_COUNT_FLR4] = { 0 };
    for (unsigned16_T i = 0; i < AF_DET_MAX_COUNT_FLR4; i++)
    {
        Self_Dets[i] = STLA_FLR4_Dets_info1.target_report[i].range;
    }
    unsigned16_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
    unsigned16_T index[AF_DET_MAX_COUNT_FLR4] = { 0 };
    for (unsigned16_T i = 0; i < size; i++)
    {
        index[i] = i;
    }
    array_data_ptrFLR4P = Self_Dets;
    qsort(index, size, sizeof(*index), comparedataFLR4P);

    //cout << "\n sorting done";

    /*print sorted self dets*/
    unsigned8_T count = 1;
    for (unsigned16_T i = 0; i < AF_DET_MAX_COUNT_FLR4; i++)
    {
        /*check for  non zero range*/

        if ((STLA_FLR4_Dets_info1.target_report[index[i]].range != 0) && (count == 1))
        {
          
            if (fileCount == 0)
            {
                (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC+radar_postion)->valid_detection_count[fileCount].push_back(double(STLA_FLR4_Dets_info1.target_count));
                (pSelfDetectionPlotParameterFC+radar_postion)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(STLA_Dets_info1.target_count)));

            }
            if (fileCount == 1)
            {
                (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC+radar_postion)->valid_detection_count[fileCount].push_back(double(STLA_FLR4_Dets_info1.target_count));
                (pSelfDetectionPlotParameterFC+radar_postion)->map_valid_detection_count[fileCount].insert(pair<double, std::vector<double>>(scanindex, double(STLA_Dets_info1.target_count)));

            }

            count = 0;

        }
        if (STLA_FLR4_Dets_info1.target_report[index[i]].range != 0)
        {
            if (fileCount == 0)
            {

                FC_scanindex.push_back(scanindex);
                Scanindex[radar_postion].push_back(scanindex);

                double rangevalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].range);
                float rangefloat = STLA_FLR4_Dets_info1.target_report[index[i]].range;
                float rangeresults = roundv(rangefloat);
                double testrange = round(rangevalue * 100.0) / 100.0;

                string rangestr = roundDouble(rangevalue, 2);
                double rangestrTodouble = stod(rangestr);

                //pSelfDetectionPlotParameterFC->range[fileCount].push_back(testrange);
                
                //cout << "\n F1 Range " << rangevalue;
                if (rangevalue >= -1 && rangevalue <= 500)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].push_back(rangevalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount].push_back(rangevalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);
                }



                double rangeratevalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].range_rate);

                if (rangeratevalue >= -30 && rangeratevalue <= 30)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                }


                double ampvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].amp);

                if (ampvalue >= -100 && ampvalue <= 100)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->amp[fileCount].push_back(ampvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);
                }


                double rcsvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].rcs);
                if (rcsvalue >= -100 && rcsvalue <= 100)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->rcs[fileCount].push_back(rcsvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                }


                double snrvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].snr);

                if (snrvalue >= -100 && snrvalue <= 100)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->snr[fileCount].push_back(snrvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);
                }



                double azimuth = flt_to_dbl(RAD2DEG(STLA_FLR4_Dets_info1.target_report[index[i]].azimuth));
                if (azimuth >= -90 && azimuth <= 90)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->azimuth[fileCount].push_back(azimuth);
                    (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);
                }


                if (trunc((azimuth * -1)) > 65)
                {
                    //cout << "\n f1 azimuth out of range  " << azimuth << " scan index " << scanindex;
                    azimuth_out_of_range_count[fileCount]++;
                }

                double elevation = flt_to_dbl(RAD2DEG(STLA_FLR4_Dets_info1.target_report[index[i]].elevation));
                if (elevation >= -40 && elevation <= 40)
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->elevation[fileCount].push_back(elevation);
                    (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);
                }


                //cout << "\n SI " << scanindex << "AZ Conf" << double(STLA_FLR4_Dets_info1.target_report[index[i]].az_conf);

                double az_conf_value = double(STLA_FLR4_Dets_info1.target_report[index[i]].az_conf);
                double el_conf_value = double(STLA_FLR4_Dets_info1.target_report[index[i]].el_conf);

                (pSelfDetectionPlotParameterFC + radar_postion)->az_conf[fileCount].push_back(az_conf_value);
                (pSelfDetectionPlotParameterFC + radar_postion)->el_conf[fileCount].push_back(el_conf_value);



                if (trunc((elevation * -1)) > 10)
                {
                    //cout << "\n f1 elevation out of range  " << elevation << " scan index " << scanindex;
                    elevation_out_of_range_count[fileCount]++;
                }

                uint8_t isBistatic = STLA_FLR4_Dets_info1.target_report[index[i]].isBistatic;

                (pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[fileCount].push_back(double(isBistatic));

                if (isBistatic == 1)
                {
                    //cout << "\n f1 isBistatic " << int(isBistatic)<<" scan index "<<scanindex;
                    file1_bistatic_det++;
                    vec_file1_bistatic_det.push_back(double(isBistatic));
                }

                uint8_t isSingleTarget = STLA_FLR4_Dets_info1.target_report[index[i]].isSingleTarget;
                if (isSingleTarget == 1)
                {
                    file1_singletarget_det++;
                    vec_file1_singletarget_det.push_back(file1_singletarget_det);

                }

                uint8_t isSingleTarget_azimuth = STLA_FLR4_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                if (isSingleTarget_azimuth == 1)
                {
                    file1_singletargetazimuth_det++;
                    vec_file1_singletargetazimuth_det.push_back(file1_singletargetazimuth_det);
                }

                uint8_t f_Doppler_mixed_interval = STLA_FLR4_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                if (f_Doppler_mixed_interval == 1)
                {
                    file1_dopplermixedinterval_det++;
                    vec_file1_dopplermixedinterval_det.push_back(file1_dopplermixedinterval_det);
                }

                uint8_t f_valid_Doppler_unfolding = STLA_FLR4_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                if (f_valid_Doppler_unfolding == 1)
                {
                    file1_dopplerunfolding_det++;
                    vec_file1_dopplerunfolding_det.push_back(file1_dopplerunfolding_det);
                }

                uint8_t bf_type_az = STLA_FLR4_Dets_info1.target_report[index[i]].bf_type_az;
                if (bf_type_az == 1)
                {
                    file1_bf_type_az++;
                    vec_file1_bf_type_az.push_back(file1_bf_type_az);

                }

                uint8_t bf_type_el = STLA_FLR4_Dets_info1.target_report[index[i]].bf_type_el;
                if (bf_type_el == 1)
                {
                    file1_bf_type_el++;
                    vec_file1_bf_type_el.push_back(file1_bf_type_el);

                }

                uint8_t super_res_target = STLA_FLR4_Dets_info1.target_report[index[i]].super_res_target;
                if (super_res_target == 1)
                {
                    file1_super_res_target++;
                    vec_file1_super_res_target.push_back(file1_super_res_target);
                }

                uint8_t super_res_target_type = STLA_FLR4_Dets_info1.target_report[index[i]].super_res_target_type;
                if (super_res_target_type == 1)
                {

                }

                uint8_t outside_sector = STLA_FLR4_Dets_info1.target_report[index[i]].outside_sector;
                if (outside_sector == 1)
                {
                    file1_outside_sector++;
                    vec_file1_outside_sector.push_back(file1_outside_sector);
                }




                file1_data << "\n F1 range value " << round(rangevalue * 100.0) / 100.0;


                if ((rangevalue >= -1 && rangevalue <= 500) ||
                    (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                    (ampvalue >= -100 && ampvalue <= 100) ||
                    (snrvalue >= -100 && snrvalue <= 100) ||
                    (azimuth >= -90 && azimuth <= 90) ||
                    elevation >= -40 && elevation <= 40)
                {

                    (pSelfDetectionPlotParameterFC+radar_postion)->si_veh.push_back(scanindex);

                }







            }

            if (fileCount == 1)
            {


                if (inRange(Scanindex[radar_postion].front(), Scanindex[radar_postion].back(), scanindex))
                {
                    

                    double rangevalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].range);

                    float rangefloat = STLA_FLR4_Dets_info1.target_report[index[i]].range;
                    float rangeresults = roundv(rangefloat);
                    double testrange = round(rangevalue * 100.0) / 100.0;

                    string rangestr = roundDouble(rangevalue, 2);
                    double rangestrTodouble = stod(rangestr);

                    //pSelfDetectionPlotParameterFC->range[fileCount].push_back(testrange);
                    //cout << "\n F2 Range " << rangevalue;
                    if (rangevalue >= -1 && rangevalue <= 500)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].push_back(rangevalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount].push_back(rangevalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);
                    }

                    double rangeratevalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].range_rate);
                    if (rangeratevalue >= -30 && rangeratevalue <= 30)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);
                    }




                    double ampvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].amp);
                    if (ampvalue >= -100 && ampvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->amp[fileCount].push_back(ampvalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);
                    }


                    double rcsvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].rcs);
                    if (rcsvalue >= -100 && rcsvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->rcs[fileCount].push_back(rcsvalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);
                    }


                    double snrvalue = flt_to_dbl(STLA_FLR4_Dets_info1.target_report[index[i]].snr);
                    if (snrvalue >= -100 && snrvalue <= 100)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->snr[fileCount].push_back(snrvalue);
                        (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);
                    }



                    double azimuth = flt_to_dbl(RAD2DEG(STLA_FLR4_Dets_info1.target_report[index[i]].azimuth));
                    if (azimuth >= -90 && azimuth <= 90)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->azimuth[fileCount].push_back(azimuth);
                        (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);
                    }



                    if (trunc((azimuth * -1)) > 65)
                    {
                        // cout << "\n f2 azimuth out of range  " << azimuth << " scan index " << scanindex;
                        azimuth_out_of_range_count[fileCount]++;
                    }
                    double elevation = flt_to_dbl(RAD2DEG(STLA_FLR4_Dets_info1.target_report[index[i]].elevation));
                    if (elevation >= -40 && elevation <= 40)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->elevation[fileCount].push_back(elevation);
                        (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);

                    }


                    //cout << "\n SI " << scanindex << "AZ Conf" << double(STLA_FLR4_Dets_info1.target_report[index[i]].az_conf);

                    double az_conf_value = double(STLA_FLR4_Dets_info1.target_report[index[i]].az_conf);
                    double el_conf_value = double(STLA_FLR4_Dets_info1.target_report[index[i]].el_conf);

                    (pSelfDetectionPlotParameterFC + radar_postion)->az_conf[fileCount].push_back(az_conf_value);
                    (pSelfDetectionPlotParameterFC + radar_postion)->el_conf[fileCount].push_back(el_conf_value);


                    if (trunc((elevation * -1)) > 10)
                    {
                        //cout << "\n f2 elevation out of range  " << elevation << " scan index " << scanindex;
                        elevation_out_of_range_count[fileCount]++;
                    }

                    uint8_t isBistatic = STLA_FLR4_Dets_info1.target_report[index[i]].isBistatic;

                    (pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[fileCount].push_back(double(isBistatic));

                    if (isBistatic == 1)
                    {
                        // cout << "\n f2 isBistatic " << int(isBistatic) << " scan index " << scanindex;
                        file2_bistatic_det++;
                        vec_file2_bistatic_det.push_back(double(isBistatic));
                    }

                    uint8_t isSingleTarget = STLA_FLR4_Dets_info1.target_report[index[i]].isSingleTarget;
                    if (isSingleTarget == 1)
                    {
                        file2_singletarget_det++;
                        vec_file2_singletarget_det.push_back(file2_singletarget_det);

                    }

                    uint8_t isSingleTarget_azimuth = STLA_FLR4_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                    if (isSingleTarget_azimuth == 1)
                    {
                        file2_singletargetazimuth_det++;
                        vec_file2_singletargetazimuth_det.push_back(file2_singletargetazimuth_det);
                    }

                    uint8_t f_Doppler_mixed_interval = STLA_FLR4_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                    if (f_Doppler_mixed_interval == 1)
                    {
                        file2_dopplermixedinterval_det++;
                        vec_file2_dopplermixedinterval_det.push_back(file2_dopplermixedinterval_det);
                    }

                    uint8_t f_valid_Doppler_unfolding = STLA_FLR4_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                    if (f_valid_Doppler_unfolding == 1)
                    {
                        file2_dopplerunfolding_det++;
                        vec_file2_dopplerunfolding_det.push_back(file2_dopplerunfolding_det);
                    }

                    uint8_t bf_type_az = STLA_FLR4_Dets_info1.target_report[index[i]].bf_type_az;
                    if (bf_type_az == 1)
                    {
                        file2_bf_type_az++;
                        vec_file2_bf_type_az.push_back(file2_bf_type_az);

                    }

                    uint8_t bf_type_el = STLA_FLR4_Dets_info1.target_report[index[i]].bf_type_el;
                    if (bf_type_el == 1)
                    {
                        file2_bf_type_el++;
                        vec_file2_bf_type_el.push_back(file2_bf_type_el);

                    }

                    uint8_t super_res_target = STLA_FLR4_Dets_info1.target_report[index[i]].super_res_target;
                    if (super_res_target == 1)
                    {
                        file2_super_res_target++;
                        vec_file2_super_res_target.push_back(file2_super_res_target);
                    }

                    uint8_t super_res_target_type = STLA_FLR4_Dets_info1.target_report[index[i]].super_res_target_type;
                    if (super_res_target_type == 1)
                    {

                    }

                    uint8_t outside_sector = STLA_FLR4_Dets_info1.target_report[index[i]].outside_sector;
                    if (outside_sector == 1)
                    {
                        file2_outside_sector++;
                        vec_file2_outside_sector.push_back(file2_outside_sector);
                    }


                    /* file2_data << "\n rangeratevalue " << rangeratevalue;
                     file2_data << "\n azimuth " << azimuth;
                     file2_data << "\n elevation " << elevation;

                     file2_data << "\n rcsvalue " << rcsvalue;
                     file2_data << "\n snrvalue " << snrvalue;*/

                    if ((rangevalue >= -1 && rangevalue <= 500) ||
                        (rangeratevalue >= -100 && rangeratevalue <= 100) ||
                        (ampvalue >= -100 && ampvalue <= 100) ||
                        (snrvalue >= -100 && snrvalue <= 100) ||
                        (azimuth >= -90 && azimuth <= 90) ||
                        elevation >= -40 && elevation <= 40)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->si_resim.push_back(scanindex);
                    }


                }




            }




        }
    }
    if (count == 0)
    {

        (pSelfDetectionPlotParameterFC+radar_postion)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_rangef[fileCount].insert(pair<float, std::vector<float>>((float)scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount]));


        /*std::vector<double>::iterator itr_maxValue;

        double max_value_temp;
        max_value_temp = *std::max_element((pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].begin(),
                                    (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].end());*/


        //cout << "\n FILE  " << fileCount << "SI " << scanindex << "radar "<< radar_postion<<" max range " << max_value_temp;
        //cout << "\n SI " << scanindex << " max range " << max_value_temp;
        ////(pSelfDetectionPlotParameterFC+radar_postion)->range_max[fileCount].push_back(max_value_temp);

        (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->rangef[fileCount].clear();


    }





}
void FLR4P::CollectTrackerData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{
    
    DataProxy& datapxy = DataProxy::getInstance();
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Flr4p_Tracker_info1, CURVI_TRACK_DATA);
    DVSU_RECORD_T record = { 0 };
    if (custId == PLATFORM_GEN5 || custId == STLA_FLR4P)
    {
#ifndef DISABLE_TRACKER
        /*sort by range*/
        float32_T Track_Objs[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < GEN5_NUMBER_OF_OBJECTS_L; i++)
        {
            Track_Objs[i] = Flr4p_Tracker_info1.object[i].vcs_xposn;
        }
        unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
        unsigned8_T index[NUMBER_OF_OBJECT_TRACKS_L] = { 0 };
        for (unsigned8_T i = 0; i < size; i++)
        {
            index[i] = i;
        }
        array_data_ptrFLR4P = Track_Objs;
        qsort(index, size, sizeof(*index), comparedataFLR4P);

        /*print sorted Opp dets*/
        unsigned8_T count = 1;
        for (unsigned8_T i = 0; i < NUMBER_OF_OBJECT_TRACKS_L; i++)
        {
            /*check for valid Track status level*/
            if (((Flr4p_Tracker_info1.object[i].status) > 0) && (count == 1))
            {
                count = 0;
                if (fileCount == 0 )
                {
                    pTrackerPlotParamFC_FLR4P->vcs_si_veh_ref_scale.push_back(scanindex);
                }
                if (fileCount == 1)
                {                  
                    pTrackerPlotParamFC_FLR4P->vcs_si_resim_ref_scale.push_back(scanindex);
                }
                
            }
            if ((Flr4p_Tracker_info1.object[i].status) > 0)
            {
                if (fileCount == 0)
                {
                    unsigned8_T status = Flr4p_Tracker_info1.object[i].status;

                    if (status!=0)
                    {
                        if (Flr4p_Tracker_info1.object[i].f_moving)
                        {                        
                            file1_moving_objects++;
                            vec_file1_moving_objects.push_back(file1_moving_objects);

                            pTrackerPlotParamFC_FLR4P->vcs_si_veh.push_back(scanindex);
                            FC_scanindex.push_back(scanindex);

                            double vcs_xposvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xposn);
                            pTrackerPlotParamFC_FLR4P->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                            pTrackerPlotParamFC_FLR4P->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                            double vcs_yposvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yposn);
                            pTrackerPlotParamFC_FLR4P->vsc_ypos[fileCount].push_back(vcs_yposvalue);  
                            pTrackerPlotParamFC_FLR4P->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                            double vcs_xvelvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xvel);
                            pTrackerPlotParamFC_FLR4P->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                            pTrackerPlotParamFC_FLR4P->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                            double vcs_yvelvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yvel);
                            pTrackerPlotParamFC_FLR4P->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                            pTrackerPlotParamFC_FLR4P->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                            double vcs_xaccvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xaccel);
                            pTrackerPlotParamFC_FLR4P->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                            pTrackerPlotParamFC_FLR4P->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                            double vcs_yaccvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yaccel);
                            pTrackerPlotParamFC_FLR4P->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                            pTrackerPlotParamFC_FLR4P->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);


                            unsigned8_T object_class = Flr4p_Tracker_info1.object[i].object_class;

                           
                            if (object_class==0)
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
                                
                            unsigned8_T status = Flr4p_Tracker_info1.object[i].status;

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
                            }

                        }
                    }
                   
                }

                if (fileCount == 1)
                {
                    if (inRange(FC_scanindex.front(),FC_scanindex.back(),scanindex))
                    {
                      
                        unsigned8_T status = Flr4p_Tracker_info1.object[i].status;

                        if (status!=0)
                        {
                            if (Flr4p_Tracker_info1.object[i].f_moving)
                            {                            
                                file2_moving_objects++;
                                vec_file2_moving_objects.push_back(file2_moving_objects);

                                pTrackerPlotParamFC_FLR4P->vcs_si_resim.push_back(scanindex);
                                
                                double vcs_xposvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xposn);
                                pTrackerPlotParamFC_FLR4P->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                                double vcs_yposvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yposn);
                                pTrackerPlotParamFC_FLR4P->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);

                                double vcs_xvelvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xvel);
                                pTrackerPlotParamFC_FLR4P->vsc_xvel[fileCount].push_back(vcs_xvelvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_xvel_scaleplot[fileCount].push_back(vcs_xvelvalue);

                                double vcs_yvelvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yvel);
                                pTrackerPlotParamFC_FLR4P->vsc_yvel[fileCount].push_back(vcs_yvelvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_yvel_scaleplot[fileCount].push_back(vcs_yvelvalue);

                                double vcs_xaccvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_xaccel);
                                pTrackerPlotParamFC_FLR4P->vsc_xacc[fileCount].push_back(vcs_xaccvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_xacc_scaleplot[fileCount].push_back(vcs_xaccvalue);

                                double vcs_yaccvalue = flt_to_dbl(Flr4p_Tracker_info1.object[i].vcs_yaccel);
                                pTrackerPlotParamFC_FLR4P->vsc_yacc[fileCount].push_back(vcs_yaccvalue);
                                pTrackerPlotParamFC_FLR4P->vsc_yacc_scaleplot[fileCount].push_back(vcs_yaccvalue);


                                unsigned8_T object_class = Flr4p_Tracker_info1.object[i].object_class;

                              
                                if (object_class == 0)
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
                                }



                                unsigned8_T status = Flr4p_Tracker_info1.object[i].status;

                               
                                if (status == 1)
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
                                } 



                            }
                        }
                   }
                   
                }             
            }
        }
        if (count == 0)
        {
            pTrackerPlotParamFC_FLR4P->map_xpos[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_xpos_scaleplot[fileCount];
            pTrackerPlotParamFC_FLR4P->map_ypos[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_ypos_scaleplot[fileCount];

            pTrackerPlotParamFC_FLR4P->map_xvel[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_xvel_scaleplot[fileCount];
            pTrackerPlotParamFC_FLR4P->map_yvel[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_yvel_scaleplot[fileCount];

            pTrackerPlotParamFC_FLR4P->map_xacc[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_xacc_scaleplot[fileCount];
            pTrackerPlotParamFC_FLR4P->map_yacc[fileCount][scanindex] = pTrackerPlotParamFC_FLR4P->vsc_yacc_scaleplot[fileCount];



            pTrackerPlotParamFC_FLR4P->vsc_xpos_scaleplot[fileCount].clear();
            pTrackerPlotParamFC_FLR4P->vsc_ypos_scaleplot[fileCount].clear();
            pTrackerPlotParamFC_FLR4P->vsc_xvel_scaleplot[fileCount].clear();
            pTrackerPlotParamFC_FLR4P->vsc_yvel_scaleplot[fileCount].clear();
            pTrackerPlotParamFC_FLR4P->vsc_xacc_scaleplot[fileCount].clear();
            pTrackerPlotParamFC_FLR4P->vsc_yacc_scaleplot[fileCount].clear();

          
                     
        }
#endif
    }
}

void FLR4P::CollectDetectionData(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{

    int radar_postion = int(Radar_Posn);
    DataProxy& datapxy = DataProxy::getInstance();
    m_radar_plugin->Gen5RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&Flr4p_Dets_info1, DETS_COMP);
    
    
    //detection data
    /*sort by range*/
    float32_T Self_Dets[NUMBER_OF_DETECTIONS] = { 0 };
    for (unsigned8_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    {
        Self_Dets[i] = Flr4p_Dets_info1.target_report[i].range;
    }
    unsigned8_T size = sizeof(Self_Dets) / sizeof(*Self_Dets);
    unsigned8_T index[NUMBER_OF_DETECTIONS] = { 0 };
    for (unsigned8_T i = 0; i < size; i++)
    {
        index[i] = i;
    }

    array_data_ptrFLR4P = Self_Dets;
    qsort(index, size, sizeof(*index), comparedataFLR4P);


    /*print sorted self dets*/
    unsigned8_T count = 1;
    for (unsigned16_T i = 0; i < NUMBER_OF_DETECTIONS; i++)
    {
        /*check for  non zero range*/

        if ((Flr4p_Dets_info1.target_report[index[i]].range != 0) && (count == 1))
        {
            if (fileCount == 0)
            {             
                (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC+radar_postion)->valid_detection_count[fileCount].push_back(double(Flr4p_Dets_info1.target_count));
               
            }
            if (fileCount == 1)
            {             
                (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.push_back(scanindex);
                (pSelfDetectionPlotParameterFC+radar_postion)->valid_detection_count[fileCount].push_back(double(Flr4p_Dets_info1.target_count));
               
            }
           

            count = 0;
        }
        if (Flr4p_Dets_info1.target_report[index[i]].range != 0)
        {
            if (fileCount == 0)
            {
                
                (pSelfDetectionPlotParameterFC+radar_postion)->si_veh.push_back(scanindex);
                FC_scanindex.push_back(scanindex);
                Scanindex[radar_postion].push_back(scanindex);
                double rangevalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].range);
                (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].push_back(round(rangevalue * 100.0) / 100.0);
                (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);


               

                double rangeratevalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].range_rate);
                (pSelfDetectionPlotParameterFC+radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);

                
                double ampvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].amp);
                (pSelfDetectionPlotParameterFC+radar_postion)->amp[fileCount].push_back(ampvalue);
                (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);

                double rcsvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].rcs);
                (pSelfDetectionPlotParameterFC+radar_postion)->rcs[fileCount].push_back(rcsvalue);
                (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);

                double snrvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].snr);
                (pSelfDetectionPlotParameterFC+radar_postion)->snr[fileCount].push_back(snrvalue);
                (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);


                double azimuth = flt_to_dbl(RAD2DEG(Flr4p_Dets_info1.target_report[index[i]].azimuth));
                (pSelfDetectionPlotParameterFC+radar_postion)->azimuth[fileCount].push_back(azimuth);
                (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);

                if (trunc((azimuth * -1)) > 65)
                {
                    
                    azimuth_out_of_range_count[fileCount]++;
                }

                double elevation = flt_to_dbl(RAD2DEG(Flr4p_Dets_info1.target_report[index[i]].elevation));
                (pSelfDetectionPlotParameterFC+radar_postion)->elevation[fileCount].push_back(elevation);
                (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);

                double heightvalue = rangevalue * sin(Flr4p_Dets_info1.target_report[index[i]].elevation);
                (pSelfDetectionPlotParameterFC+radar_postion)->height[fileCount].push_back((heightvalue * 100.0) / 100.0);
                (pSelfDetectionPlotParameterFC+radar_postion)->height_scaleplot[fileCount].push_back((heightvalue * 100.0) / 100.0);

                if (trunc((elevation * -1)) > 10)
                {
                    
                    elevation_out_of_range_count[fileCount]++;
                }

                uint8_t isBistatic = Flr4p_Dets_info1.target_report[index[i]].isBistatic;

                if (isBistatic==1)
                {
                   
                    file1_bistatic_det++;
                    vec_file1_bistatic_det.push_back(file1_bistatic_det);
                }

                uint8_t isSingleTarget = Flr4p_Dets_info1.target_report[index[i]].isSingleTarget;
                if (isSingleTarget == 1)
                {
                    file1_singletarget_det++;
                    vec_file1_singletarget_det.push_back(file1_singletarget_det);
                    
                }

                uint8_t isSingleTarget_azimuth = Flr4p_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                if (isSingleTarget_azimuth == 1)
                {
                    file1_singletargetazimuth_det++;
                    vec_file1_singletargetazimuth_det.push_back(file1_singletargetazimuth_det);
                }

                uint8_t f_Doppler_mixed_interval = Flr4p_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                if (f_Doppler_mixed_interval == 1)
                {
                    file1_dopplermixedinterval_det++;
                    vec_file1_dopplermixedinterval_det.push_back(file1_dopplermixedinterval_det);
                }

                uint8_t f_valid_Doppler_unfolding = Flr4p_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                if (f_valid_Doppler_unfolding == 1)
                {
                    file1_dopplerunfolding_det++;
                    vec_file1_dopplerunfolding_det.push_back(file1_dopplerunfolding_det);
                }

                uint8_t bf_type_az = Flr4p_Dets_info1.target_report[index[i]].bf_type_az;
                if (bf_type_az == 1)
                {
                    file1_bf_type_az++;
                    vec_file1_bf_type_az.push_back(file1_bf_type_az);

                }

                uint8_t bf_type_el = Flr4p_Dets_info1.target_report[index[i]].bf_type_el;
                if (bf_type_el == 1)
                {
                    file1_bf_type_el++;
                    vec_file1_bf_type_el.push_back(file1_bf_type_el);

                }

                uint8_t super_res_target = Flr4p_Dets_info1.target_report[index[i]].super_res_target;
                if (super_res_target == 1)
                {
                    file1_super_res_target++;
                    vec_file1_super_res_target.push_back(file1_super_res_target);
                }

                uint8_t super_res_target_type = Flr4p_Dets_info1.target_report[index[i]].super_res_target_type;
                if (super_res_target_type == 1)
                {

                }

                uint8_t outside_sector = Flr4p_Dets_info1.target_report[index[i]].outside_sector;
                if (outside_sector == 1)
                {
                    file1_outside_sector++;
                    vec_file1_outside_sector.push_back(file1_outside_sector);
                }

               
                


            }

            if (fileCount == 1)
            {
                if (inRange(Scanindex[radar_postion].front(), Scanindex[radar_postion].back(), scanindex))
                {
                    (pSelfDetectionPlotParameterFC+radar_postion)->si_resim.push_back(scanindex);

                    double rangevalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].range);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range[fileCount].push_back(round(rangevalue * 100.0) / 100.0);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].push_back(rangevalue);


                    double rangeratevalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].range_rate);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_rate[fileCount].push_back(rangeratevalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].push_back(rangeratevalue);

                    


                    double ampvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].amp);
                    (pSelfDetectionPlotParameterFC+radar_postion)->amp[fileCount].push_back(ampvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].push_back(ampvalue);

                    double rcsvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].rcs);
                    (pSelfDetectionPlotParameterFC+radar_postion)->rcs[fileCount].push_back(rcsvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].push_back(rcsvalue);

                    double snrvalue = flt_to_dbl(Flr4p_Dets_info1.target_report[index[i]].snr);
                    (pSelfDetectionPlotParameterFC+radar_postion)->snr[fileCount].push_back(snrvalue);
                    (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].push_back(snrvalue);



                    double azimuth = flt_to_dbl(RAD2DEG(Flr4p_Dets_info1.target_report[index[i]].azimuth));
                    (pSelfDetectionPlotParameterFC+radar_postion)->azimuth[fileCount].push_back(azimuth);
                    (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].push_back(azimuth);


                    if (trunc((azimuth*-1)) > 65)
                    {
                      
                        azimuth_out_of_range_count[fileCount]++;
                    }
                    double elevation = flt_to_dbl(RAD2DEG(Flr4p_Dets_info1.target_report[index[i]].elevation));
                    (pSelfDetectionPlotParameterFC+radar_postion)->elevation[fileCount].push_back(elevation);
                    (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].push_back(elevation);


                    double heightvalue = rangevalue * sin(Flr4p_Dets_info1.target_report[index[i]].elevation);
                    (pSelfDetectionPlotParameterFC+radar_postion)->height[fileCount].push_back((heightvalue * 100.0) / 100.0);
                    (pSelfDetectionPlotParameterFC+radar_postion)->height_scaleplot[fileCount].push_back((heightvalue * 100.0) / 100.0);

                    if (trunc((elevation*-1)) > 10)
                    {
                        
                        elevation_out_of_range_count[fileCount]++;
                    }

                    uint8_t isBistatic = Flr4p_Dets_info1.target_report[index[i]].isBistatic;

                    if (isBistatic == 1)
                    {
                       
                        file2_bistatic_det++;
                        vec_file2_bistatic_det.push_back(file2_bistatic_det);
                    }

                    uint8_t isSingleTarget = Flr4p_Dets_info1.target_report[index[i]].isSingleTarget;
                    if (isSingleTarget == 1)
                    {
                        file2_singletarget_det++;
                        vec_file2_singletarget_det.push_back(file2_singletarget_det);

                    }

                    uint8_t isSingleTarget_azimuth = Flr4p_Dets_info1.target_report[index[i]].isSingleTarget_azimuth;
                    if (isSingleTarget_azimuth == 1)
                    {
                        file2_singletargetazimuth_det++;
                        vec_file2_singletargetazimuth_det.push_back(file2_singletargetazimuth_det);
                    }

                    uint8_t f_Doppler_mixed_interval = Flr4p_Dets_info1.target_report[index[i]].f_Doppler_mixed_interval;
                    if (f_Doppler_mixed_interval == 1)
                    {
                        file2_dopplermixedinterval_det++;
                        vec_file2_dopplermixedinterval_det.push_back(file2_dopplermixedinterval_det);
                    }

                    uint8_t f_valid_Doppler_unfolding = Flr4p_Dets_info1.target_report[index[i]].f_valid_Doppler_unfolding;
                    if (f_valid_Doppler_unfolding == 1)
                    {
                        file2_dopplerunfolding_det++;
                        vec_file2_dopplerunfolding_det.push_back(file2_dopplerunfolding_det);
                    }

                    uint8_t bf_type_az = Flr4p_Dets_info1.target_report[index[i]].bf_type_az;
                    if (bf_type_az == 1)
                    {
                        file2_bf_type_az++;
                        vec_file2_bf_type_az.push_back(file2_bf_type_az);

                    }

                    uint8_t bf_type_el = Flr4p_Dets_info1.target_report[index[i]].bf_type_el;
                    if (bf_type_el == 1)
                    {
                        file2_bf_type_el++;
                        vec_file2_bf_type_el.push_back(file2_bf_type_el);

                    }

                    uint8_t super_res_target = Flr4p_Dets_info1.target_report[index[i]].super_res_target;
                    if (super_res_target == 1)
                    {
                        file2_super_res_target++;
                        vec_file2_super_res_target.push_back(file2_super_res_target);
                    }

                    uint8_t super_res_target_type = Flr4p_Dets_info1.target_report[index[i]].super_res_target_type;
                    if (super_res_target_type == 1)
                    {

                    }

                    uint8_t outside_sector = Flr4p_Dets_info1.target_report[index[i]].outside_sector;
                    if (outside_sector == 1)
                    {
                        file2_outside_sector++;
                        vec_file2_outside_sector.push_back(file2_outside_sector);
                    }


                    
                }

                
            }


            
           
        }
    }
    if (count == 0)
    {
        


        (pSelfDetectionPlotParameterFC+radar_postion)->map_range[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_amp[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_snr[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount]));
        (pSelfDetectionPlotParameterFC+radar_postion)->map_height[fileCount].insert(pair<double, std::vector<double>>(scanindex, (pSelfDetectionPlotParameterFC+radar_postion)->height_scaleplot[fileCount]));


        (pSelfDetectionPlotParameterFC+radar_postion)->range_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->range_rate_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->azimuth_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->elevation_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->rcs_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->amp_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->snr_scaleplot[fileCount].clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->height_scaleplot[fileCount].clear();

      


       
    

    
    }

}

void FLR4P::CollectTrackerData_BMW_MID(int fileCount, const char* LogFname, IRadarStream* g_pIRadarStrm, short stream_num, unsigned8_T Radar_Posn, Customer_T custId, unsigned int scanindex, unsigned int platform)
{

    int radar_postion = int(Radar_Posn);
    if (stream_num == 12)
    {
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&p_g_tracker_OAL_ping_xml1, Tracker_Info_XML_DATA);
        m_radar_plugin->RadarMudpGetData(g_pIRadarStrm, (unsigned char*)&ALL_object_xml1, ALL_OBJECTXML_DATA);

      
    }
    unsigned8_T count = 1, ped_count = 1, truck_count = 1, car_count = 1, wheel_count = 1;
    char strData[32]; //size of float32
    /*sort by longitudinal_velocity*/
    float32_T Track_Objs[MAX_F360_OBJECTS] = { 0 };
    for (unsigned8_T i = 0; i < MAX_F360_OBJECTS; i++)
    {
        Track_Objs[i] = ALL_object_xml1.obj[i].vcs_xposn;
    }
    unsigned8_T size = sizeof(Track_Objs) / sizeof(*Track_Objs);
    unsigned8_T indexe[MAX_F360_OBJECTS] = { 0 };
    for (unsigned8_T i = 0; i < size; i++)
    {
        indexe[i] = i;
    }
    array_data_ptrFLR4P = Track_Objs;
    qsort(indexe, size, sizeof(*indexe), comparedataFLR4P);

    static unsigned8_T INVALID;
    static unsigned8_T NEW;
    static unsigned8_T NEW_COASTED;
    static unsigned8_T NEW_UPDATED;
    static unsigned8_T UPDATED;
    static unsigned8_T COASTED;

    for (unsigned8_T i = 0; i < MAX_F360_OBJECTS; i++)
    {
        unsigned8_T status = ALL_object_xml1.obj[indexe[i]].status;
        if (status == 0)
        {
            INVALID++;
        }
        if (status == 1)
        {
            NEW++;
        }
        if (status == 2)
        {
            NEW_COASTED++;
        }
        if (status == 3)
        {
            NEW_UPDATED++;
        }
        if (status == 4)
        {
            UPDATED++;
        }
        if (status == 5)
        {
            COASTED++;
        }
    }
    INVALID = NEW = NEW_COASTED = NEW_UPDATED = COASTED = UPDATED = 0;

    if (stream_num == 12)
    {
        for (uint8_t Track_index = 0; Track_index < MAX_F360_OBJECTS; Track_index++)
        {

            if (((ALL_object_xml1.obj[indexe[Track_index]].status) > 0) && (count == 1))
            {

                if (fileCount == 0)
                {
                    //pTrackerPlotParamDC->vcs_si_veh.push_back(p_g_tracker_OAL_ping_xml1.tracker_index);
                }
                if (fileCount == 1)
                {
                    //pTrackerPlotParamDC->vcs_si_resim.push_back(p_g_tracker_OAL_ping_xml1.tracker_index);
                }

                //cout << "\nDC scan index " << p_g_tracker_OAL_ping_xml1.tracker_index;
                //fprintf(SIL_XML_out, "\n<TRACK_DESCRIPTION_TRACK_START_%d>", p_g_tracker_OAL_ping_xml.tracker_index);
                //fprintf(SIL_XML_out, "\n<TRACK_INFO>\tlong_Postion,\tlat_Postion,\tObject_Class,\tStatus,\t\tObject_Length,\tObject_Width,\tVCS_lateral_velocity,\tVCS_longitudinal_velocity,\tHeading,\t</TRACK_INFO>");
                count = 0;
            }
            if ((ALL_object_xml1.obj[indexe[Track_index]].status) > 0)
            {
                
                
                
                
                if (fileCount == 0)
                {

                    pTrackerPlotParamDC->vcs_si_veh.push_back(double(p_g_tracker_OAL_ping_xml1.tracker_index));
                    FC_scanindex.push_back(double(p_g_tracker_OAL_ping_xml1.tracker_index));

                    double vcs_xposvalue = flt_to_dbl(ALL_object_xml1.obj[indexe[Track_index]].vcs_xposn);
                    pTrackerPlotParamDC->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                    pTrackerPlotParamDC->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                    double vcs_yposvalue = flt_to_dbl(ALL_object_xml1.obj[indexe[Track_index]].vcs_yposn);
                    pTrackerPlotParamDC->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                    pTrackerPlotParamDC->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);
                }

                if (fileCount == 1)
                {

                    if (inRange(FC_scanindex.front(), FC_scanindex.back(), p_g_tracker_OAL_ping_xml1.tracker_index))
                    {




                        pTrackerPlotParamDC->vcs_si_resim.push_back(double(p_g_tracker_OAL_ping_xml1.tracker_index));


                        double vcs_xposvalue = flt_to_dbl(ALL_object_xml1.obj[indexe[Track_index]].vcs_xposn);
                        pTrackerPlotParamDC->vsc_xpos[fileCount].push_back(vcs_xposvalue);
                        pTrackerPlotParamDC->vsc_xpos_scaleplot[fileCount].push_back(vcs_xposvalue);

                        double vcs_yposvalue = flt_to_dbl(ALL_object_xml1.obj[indexe[Track_index]].vcs_yposn);
                        pTrackerPlotParamDC->vsc_ypos[fileCount].push_back(vcs_yposvalue);
                        pTrackerPlotParamDC->vsc_ypos_scaleplot[fileCount].push_back(vcs_yposvalue);
                    }
                }
                
                
                
                //fprintf(SIL_XML_out, "\n  <TRACKS>");

                
                //fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xposn);
                //cout << "\n vcs_xposn" << ALL_object_xml1.obj[indexe[Track_index]].vcs_xposn;
                //cout << "\n vcs_yposn" << ALL_object_xml1.obj[indexe[Track_index]].vcs_yposn;
                //fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yposn);

                /*switch (ALL_object_xml.obj[indexe[Track_index]].object_class)
                {
                case 0:fprintf(SIL_XML_out, "\tUNKNOWN\t"); break;
                case 1:fprintf(SIL_XML_out, "\tPEDESTRIAN"); break;
                case 2:fprintf(SIL_XML_out, "\t2WHEEL\t"); break;
                case 3:fprintf(SIL_XML_out, "\tCAR\t"); break;
                case 4:fprintf(SIL_XML_out, "\tTRUCK\t"); break;
                default:fprintf(SIL_XML_out, "\tINVALID\t"); break;
                }

                switch (ALL_object_xml.obj[indexe[Track_index]].status)
                {
                case 0:fprintf(SIL_XML_out, "\tINVALID\t"); break;
                case 1:fprintf(SIL_XML_out, "\tNEW\t"); break;
                case 2:fprintf(SIL_XML_out, "\tNEW_COASTED\t"); break;
                case 3:fprintf(SIL_XML_out, "\tNEW_UPDATED\t"); break;
                case 4:fprintf(SIL_XML_out, "\tUPDATED\t"); break;
                case 5:fprintf(SIL_XML_out, "\tCOASTED\t"); break;
                }*/

                
                //fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].len1);

             
                //fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].wid1);

               
                //fprintf(SIL_XML_out, "\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_xvel);

                
                //fprintf(SIL_XML_out, "\t\t%f", ALL_object_xml.obj[indexe[Track_index]].vcs_yvel);

               
                //fprintf(SIL_XML_out, "\t\t\t%f", ALL_object_xml.obj[indexe[Track_index]].heading);

                //fprintf(SIL_XML_out, "\t</TRACKS>");
            }
        }
        if (count == 0)
        {
            //fprintf(SIL_XML_out, "\n</TRACK_DESCRIPTION_TRACK_END_%d>\n", p_g_tracker_OAL_ping_xml.tracker_index);
        }

    }


}








int FLR4P::GenerateScatterPlot(std::vector<double> file1_xdata,
                                       std::vector<double> file1_ydata,
                                       std::vector<double> file2_xdata,
                                       std::vector<double> file2_ydata,
                                        int fileindex,
                                        const char* f_name, 
                                        const char* current_radar, 
                                        const char* foldname, 
                                        std::string plotparam, 
                                        std::string reportpath, int arraycnt)
{


   //cout << "\nFLR4P::GenerateScatterPlot radar"<<int(arraycnt);
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

    if (int(arraycnt) == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (int(arraycnt) == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (int(arraycnt) == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (int(arraycnt) == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (int(arraycnt) == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }

    else if (int(arraycnt) == 19)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_DC.html";
    }
    htmlreport_folder = reportpath + "/" + foldname;

    //cout << "\n htmlreport_file " << htmlreport_file << endl;
    //cout << "\n htmlreport_folder " << htmlreport_folder << endl;

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

        auto tracks_fig2 = figure(true);
        if (fileindex==0)
        {
            sgtitle(radar_position ,"blue"); 
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            grid(false);
            ax1->ylabel(plotparam);
            ax1->xlabel("scan index");


            std::vector<std::string> legdetails;
            legdetails.push_back("file1");
            legdetails.push_back("file2");
            ax1->legend(legdetails);
            
          

            auto pfl11 = scatter(file1_xdata, file1_ydata);           
            pfl11->color("blue");
            pfl11->fill(true);
            pfl11->marker_color("b");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);
                      
       
            hold(true);

            auto pfl2 = scatter(file2_xdata, file2_ydata);           
            pfl2->color("red");
            pfl2->fill(true);
            pfl2->marker_color("r");
            pfl2->marker_size(1.5);
            pfl2->marker("O");
            pfl2->marker_face(true);
            

            radar_position.append(std::to_string(fileindex));
        }

        if (fileindex==1)
        {
            //cout << "\n scatter for timing info";
            sgtitle(radar_position, "blue");
            tiledlayout(1, 1);
            auto ax1 = nexttile();
            grid(false);
            ax1->ylabel(plotparam);
            ax1->xlabel("scan index");


            //std::vector<std::string> legdetails;
            //legdetails.push_back("file1");
            ////legdetails.push_back("file2");
            //ax1->legend(legdetails);



            auto pfl11 = scatter(file1_xdata, file1_ydata);
            pfl11->color("blue");
            pfl11->fill(true);
            pfl11->marker_color("b");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);


            //hold(true);

           /* auto pfl2 = scatter(file2_xdata, file2_ydata);
            pfl2->color("red");
            pfl2->fill(true);
            pfl2->marker_color("r");
            pfl2->marker_size(1.5);
            pfl2->marker("O");
            pfl2->marker_face(true);*/


            radar_position.append(std::to_string(fileindex));
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

int FLR4P::GenerateBoxPlot(std::vector<double> file1_xdata,std::vector<double> file1_ydata, std::vector<double> file2_xdata,
    std::vector<double> file2_ydata, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int arraycnt)
{
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

    if (arraycnt == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (arraycnt == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (arraycnt == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (arraycnt == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }

    else if (arraycnt == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }

    else if (arraycnt == 6)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-Overall_BOXREPORT.html";
    }
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

        auto tracks_fig2 = figure(true);
        if (1)
        {
            sgtitle(" Radar " + radar_position, "blue");
            tiledlayout(1, 1);
            auto barplot_title = nexttile();

          /*  std::vector<std::string> barplotlegend;
            barplotlegend.push_back("file1");
            barplotlegend.push_back("file2");
            barplot_title->legend(barplotlegend);*/



            //std::vector<double> boxplot_xdata = file1_ydata;
            //std::vector<std::string> boxplot_ydata = file1_xdata;
            //boxplot(file1_xdata, file1_ydata);
            boxplot(file1_xdata);
            xlabel(plotparam);
            //hold(true);
            //boxplot(file2_xdata, file2_ydata);
            //gca()->x_axis().ticklabels(xdata);

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

int FLR4P::GenerateBoxPlot_New(std::vector<std::vector<double>> file1_xdata, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int arraycnt)
{

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

    if (arraycnt == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (arraycnt == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (arraycnt == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (arraycnt == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }

    else if (arraycnt == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }

    else if (arraycnt == 6)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-Overall_BOXREPORT.html";
    }
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

        auto tracks_fig2 = figure(true);
        if (1)
        {
            sgtitle(" Radar " + radar_position, "blue");
            tiledlayout(1, 1);
            auto barplot_title = nexttile();

           /* std::vector<std::string> barplotlegend;
            barplotlegend.push_back("file1");
            barplotlegend.push_back("file2");
            barplot_title->legend(barplotlegend);*/



            //std::vector<double> boxplot_xdata = file1_ydata;
            //std::vector<std::string> boxplot_ydata = file1_xdata;
            //boxplot(file1_xdata, file1_ydata);

           /* radar_rangehismap[0] = "RL Range Boxplot";
            radar_rangehismap[1] = "RR Range Boxplot";
            radar_rangehismap[2] = "FR Range Boxplot";
            radar_rangehismap[3] = "FL Range Boxplot";
            radar_rangehismap[5] = "FC Range Boxplot";*/

            boxplot(file1_xdata);


            gca()->x_axis().label_font_size(.2);
            xlabel("1:RLf1 2:RLf2 3:RRf1 4:RRf2 5:FRf1 6:FRf2 7:FLf1 8:FLf2 9:FCf1 10:FCf2");
            
            //hold(true);
            //boxplot(file2_xdata, file2_ydata);
            //gca()->x_axis().ticklabels(xdata);

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


 int FLR4P::GenerateBarPlots(std::vector<string> xdata, std::vector<std::vector<int>> ydata, 
                                   const char* f_name, const char* current_radar, const char* foldname, 
                                   std::string plotparam, std::string reportpath, int arraycnt)
{
     
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

    if (arraycnt == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (arraycnt == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (arraycnt == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (arraycnt == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }

    else if (arraycnt == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }
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

        auto tracks_fig2 = figure(true);
        if (1)
        {
            sgtitle(" Radar " + radar_position, "blue");
            tiledlayout(1, 1);
            auto barplot_title = nexttile();

            std::vector<std::string> barplotlegend;
            barplotlegend.push_back("file1");
            barplotlegend.push_back("file2");
            barplot_title->legend(barplotlegend);

                        
          
            std::vector<std::vector<int>> barplot_ydata = ydata;    
            auto b = bar(barplot_title, barplot_ydata);
            
            
            gca()->x_axis().ticklabels(xdata);    
           
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




int FLR4P::GenerateHistogramPlot(std::vector<double> f1_detection_data,std::vector<double> f2_detection_data,  
                                                  const char* f_name, const char* current_radar, const char* foldname, 
                                                  std::string plotparam, std::string reportpath, int arraycnt, int zerodatafilenumber)
{

    //cout << "\n Detection Histogram plot radar "<< arraycnt;
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

    if (arraycnt == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (arraycnt == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (arraycnt == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (arraycnt == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (arraycnt == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }
    
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

        auto tracks_fig2 = figure(true);
        if (1)
        {
            sgtitle(radar_position, "blue"); 
            tiledlayout(1, 1);
           
            auto ax12 = nexttile();
            std::vector<std::string> legdetails1;
            
           

            auto a = histogram::binning_algorithm::automatic;

            if (zerodatafilenumber == 0)
            {
                auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                h1->face_color("b");
                hold(on);
                auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                h2->face_color("r");
                //h1->face_color({ 0., 0., 0.5, 0.5 });
                legdetails1.push_back("f1");
                legdetails1.push_back("f2");
                ax12->legend(legdetails1);
            }

            if (zerodatafilenumber==1)
            {
                auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                h2->face_color("r");
                legdetails1.push_back("f2");
                
                ax12->legend(legdetails1);

            }
            if (zerodatafilenumber==2)
            {
                auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                h1->face_color("b");
                legdetails1.push_back("f1");
                ax12->legend(legdetails1);
            }
           

            ylabel("count");
            xlabel(plotparam);

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

int FLR4P::PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt)
{
   //cout << "\nPrintReportInfoinHTML radar \n"<< arraycnt;
    
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
    if (arraycnt == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (arraycnt == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (arraycnt == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (arraycnt == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (arraycnt == 5)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
    }

    else if (arraycnt == 6)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-Overall_BOXREPORT.html";
    }

    else if (arraycnt == 19)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-Report_DC.html";
    }
    htmlreport_folder = reportpath + "/"+ foldname;

    //cout << "\n htmlreport_file" << htmlreport_file;
    //cout << "\n htmlreport_folder" << htmlreport_folder;



    std::string cs_info = "3713"; 
    std::string os_info;
    std::string tool_version = "1.2";
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

#ifdef _WIN32
    os_info = "Windows OS";
#endif
#ifdef __GNUC__
    os_info = "Linux OS";
#endif
 
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

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Change Set Info  : " << cs_info << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " || Operating Sysem  : " << os_info << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " || Tool Version  : " << tool_version << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " || Date Time Info  : " << std::ctime(&end_time) << "</p>\n";


        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Kindly note below color code for Interpretations " <<"</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File1:blue)  :: " <<datapxy.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File2:red) :: " << datapxy.resimfilename << "</p>\n";

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File1:blue)  :: " << datapxy.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File2:orange)  :: " << datapxy.resimfilename << "</p>\n";
      
    }

    radar_position.append("ReportInfo");
    htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
    save((char*)htmlreport_image.c_str(), "png");

    
    htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png"; 
    CTML::Node image_trks("img");
    image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
              .UseClosingTag(false);
    document.AppendNodeToBody(image_trks);

    return 0;
}


void FLR4P::Generateplot(const char* inputVehFileNameBMW, const char* pltfoldername, std::string reportpath)
{

    DataProxy& datapxy = DataProxy::getInstance();

    std::vector<double> c2timing_info_scanindexfile1, c2timing_info_scanindexfile2;
    std::vector<double> c2timing_info_valuefile1, c2timing_info_valuefile2;


    std::vector<double> c2timing_info_scanindexfile1_arrindex3, c2timing_info_scanindexfile1_arrindex9,
                        c2timing_info_scanindexfile1_arrindex17, c2timing_info_scanindexfile1_arrindex21,
                        c2timing_info_scanindexfile1_arrindex22, c2timing_info_scanindexfile1_arrindex23;


    std::vector<double> c2timing_info_valuefile1_arrindex3, c2timing_info_valuefile1_arrindex9,
                        c2timing_info_valuefile1_arrindex17, c2timing_info_valuefile1_arrindex21,
                        c2timing_info_valuefile1_arrindex22, c2timing_info_valuefile1_arrindex23, boxplot_c2timing_info_valuefile1_arrindex23;



  




   
    //cout << "\nGenerateplot ";
    




   

    std::vector<double> file1range_boxplot;
    std::vector<std::string> file1range_sensor_file_info;

    std::vector<double> file2range_boxplot;
    std::vector<std::string> file2range_sensor_file_info;
    file1range_boxplot.clear();
    file1range_sensor_file_info.clear();

    file2range_boxplot.clear();
    file2range_sensor_file_info.clear();

    std::vector<std::vector<double>> vrangeboxplot(10);
    std::vector<std::vector<double>> vrangeRateboxplot(10);
    std::vector<std::vector<double>> vazimuthboxplot(10);
    std::vector<std::vector<double>> velevationboxplot(10);
    std::vector<std::vector<double>> vamplitudeboxplot(10);
    std::vector<std::vector<double>> vsnrboxplot(10);
    std::vector<std::vector<double>> vvalidDetectionCountboxplot(10);


    auto mf4_filenamefrom_path = fs::path(inputVehFileNameBMW).filename();
    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };
    std::string htmlreport_file;
    std::string htmlreport_image;

    std::vector<string> sensor_report_extension;
    sensor_report_extension.push_back("-REPORT_RL.html");
    sensor_report_extension.push_back("-REPORT_RR.html");
    sensor_report_extension.push_back("-REPORT_FL.html");
    sensor_report_extension.push_back("-REPORT_FR.html");
    sensor_report_extension.push_back("-REPORT_FC.html");
    sensor_report_extension.push_back("-Overall_BOXREPORT.html");

    for (auto file_extension: sensor_report_extension)
    {
        htmlreport_file = reportpath + "/" + pltfoldername + "/" + mf4_filename + file_extension;


        if (std::filesystem::exists(htmlreport_file))
        {
            int result = remove(htmlreport_file.c_str());
        }
    }

  
    
    htmlreport_image = reportpath + "/" + pltfoldername + "/" + "GeneratedImages";
    
    

    if (std::filesystem::exists(htmlreport_image))
    {
      
        int result = remove(htmlreport_image.c_str());

        if (result)
        {
            //cout << "\nhtmlreport_image removed";
        }
        else
        {
            //cout << "\nhtmlreport_image notremoved";
        }
    }
    else
    {
        //cout << "\nhtmlreport_image not exists";
    }

    if (pTrackerPlotParamDC != nullptr)
    {
        if (datapxy.customerid == BMW_SAT)//for Honda
        {
            if (pTrackerPlotParamDC->vsc_xpos[0].size() != 0)
            {
                GenerateScatterPlot(pTrackerPlotParamDC->vcs_si_veh, pTrackerPlotParamDC->vsc_xpos[0],
                                    pTrackerPlotParamDC->vcs_si_resim, pTrackerPlotParamDC->vsc_xpos[1], 0,
                                    inputVehFileNameBMW, "DC XPosition",
                                     pltfoldername, "xposition", reportpath, 19);
            }

            if (pTrackerPlotParamDC->vsc_ypos[0].size() != 0)
            {           
                GenerateScatterPlot(pTrackerPlotParamDC->vcs_si_veh, pTrackerPlotParamDC->vsc_ypos[0],
                                    pTrackerPlotParamDC->vcs_si_resim, pTrackerPlotParamDC->vsc_ypos[1], 0,
                                    inputVehFileNameBMW, "DC YPosition",
                                    pltfoldername, "yposition", reportpath, 19);
            }

        }
    }
       
   /* if (pSelfDetectionPlotParameterFC != nullptr)
    {
        string plottitle_box;
        PrintReportInfoinHTML(inputVehFileNameBMW, "HTML", pltfoldername, reportpath, 6);
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0].size() != 0)
            {
                for (auto& vvaliddetectcnt : vvalidDetectionCountboxplot) {

                    for (auto validdetectval : (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0])
                    {
                        vvaliddetectcnt.push_back(validdetectval);
                    }

                    for (auto validdetectval : (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[1])
                    {
                        vvaliddetectcnt.push_back(validdetectval);
                    }



                }

            }

        }
        if (vvalidDetectionCountboxplot.size() != 0)
        {


            GenerateBoxPlot_New(vvalidDetectionCountboxplot,
                0,
                inputVehFileNameBMW, "Box Plot Valid Detection Count",
                pltfoldername, "detection count", reportpath, 6);

            vvalidDetectionCountboxplot.clear();

        }

        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size() != 0)
            {
                for (auto& vrange : vrangeboxplot) {

                    for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[0])
                    {
                        vrange.push_back(rangeval);
                    }

                    for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[1])
                    {
                        vrange.push_back(rangeval);
                    }



                }

            }

        }
        if (vrangeboxplot.size() != 0)
        {
            GenerateBoxPlot_New(vrangeboxplot,
                0,
                inputVehFileNameBMW, "Box Plot Range",
                pltfoldername, "range", reportpath, 6);

            vrangeboxplot.clear();
        }
       
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0].size() != 0)
            {
                for (auto& vrangerate : vrangeRateboxplot) {

                    for (auto rangerateval : (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0])
                    {
                        vrangerate.push_back(rangerateval);
                    }

                    for (auto rangerateval : (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1])
                    {
                        vrangerate.push_back(rangerateval);
                    }



                }

            }

        }
        if (vrangeRateboxplot.size() != 0)
        {
            GenerateBoxPlot_New(vrangeRateboxplot,
                0,
                inputVehFileNameBMW, "Box Plot RangeRate",
                pltfoldername, "range rate", reportpath, 6);

            vrangeRateboxplot.clear();
        }
       
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->elevation[0].size() != 0)
            {
                for (auto& velevation : velevationboxplot) {

                    for (auto elevationval : (pSelfDetectionPlotParameterFC + radar_postion)->elevation[0])
                    {
                        velevation.push_back(elevationval);
                    }

                    for (auto elevationval : (pSelfDetectionPlotParameterFC + radar_postion)->elevation[1])
                    {
                        velevation.push_back(elevationval);
                    }
                }

            }

        }
        if (velevationboxplot.size()!=0)
        {
            GenerateBoxPlot_New(velevationboxplot,
                0,
                inputVehFileNameBMW, "Box Plot Elevation",
                pltfoldername, "elevation", reportpath, 6);

            velevationboxplot.clear();
        }
      
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0].size() != 0)
            {
                for (auto& vazimuth : vazimuthboxplot) {

                    for (auto azimuthval : (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0])
                    {
                        vazimuth.push_back(azimuthval);
                    }

                    for (auto azimuthval : (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1])
                    {
                        vazimuth.push_back(azimuthval);
                    }
                }

            }

        }
        if (vazimuthboxplot.size() != 0)
        {
            GenerateBoxPlot_New(vazimuthboxplot,
                0,
                inputVehFileNameBMW, "Box Plot azimuth",
                pltfoldername, "azimuth", reportpath, 6);

            vazimuthboxplot.clear();
        }
      
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->amp[0].size() != 0)
            {
                for (auto& vamplitude : vamplitudeboxplot) {

                    for (auto amplitudeval : (pSelfDetectionPlotParameterFC + radar_postion)->amp[0])
                    {
                        vamplitude.push_back(amplitudeval);
                    }

                    for (auto amplitudeval : (pSelfDetectionPlotParameterFC + radar_postion)->amp[1])
                    {
                        vamplitude.push_back(amplitudeval);
                    }
                }

            }

        }
        if (vamplitudeboxplot.size() != 0)
        {
            GenerateBoxPlot_New(vamplitudeboxplot,
                0,
                inputVehFileNameBMW, "Box Plot Amplitude",
                pltfoldername, "amplitude", reportpath, 6);

            vamplitudeboxplot.clear();
        }
       
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->snr[0].size() != 0)
            {
                for (auto& vsnr : vsnrboxplot) {

                    for (auto snrval : (pSelfDetectionPlotParameterFC + radar_postion)->snr[0])
                    {
                        vsnr.push_back(snrval);
                    }

                    for (auto snrval : (pSelfDetectionPlotParameterFC + radar_postion)->snr[1])
                    {
                        vsnr.push_back(snrval);
                    }
                }

            }

        }
        if (vsnrboxplot.size() != 0)
        {
            GenerateBoxPlot_New(vsnrboxplot,
                0,
                inputVehFileNameBMW, "Box Plot SNR",
                pltfoldername, "snr", reportpath, 6);

            vsnrboxplot.clear();

        }
               
        for (auto radar_postion : datapxy.set_radarposition)
        {
            if ((pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[0].size() != 0)
            {
                string plottitle1;
                std::map<int, string> boxplotmap;
                boxplotmap[0] = "RL Box plot c2 timing info at arr(23)";
                boxplotmap[1] = "RR Box plot c2 timing info at arr(23)";
                boxplotmap[2] = "FR Box plot c2 timing info at arr(23)";
                boxplotmap[3] = "FL Box plot c2 timing info at arr(23)";
                boxplotmap[5] = "FC Box plot c2 timing info at arr(23)";

                for (auto val : boxplotmap)
                {
                    if (val.first == radar_postion)
                    {
                        plottitle1 = val.second;
                    }
                }

                for (auto val : (pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[0])
                {
                    c2timing_info_scanindexfile1_arrindex23.push_back(val.first);
                    c2timing_info_valuefile1_arrindex23.push_back(val.second.back());
                }

                c2timing_info_scanindexfile1.clear();

                GenerateBoxPlot(c2timing_info_valuefile1_arrindex23, c2timing_info_scanindexfile1, c2timing_info_scanindexfile1, c2timing_info_scanindexfile1,
                    0,
                    inputVehFileNameBMW, plottitle1.c_str(),
                    pltfoldername, "c2 timing info arr(23)", reportpath, 6);
            }

            }
           

    }*/

    //if (pSelfDetectionPlotParameterFC != nullptr)
    //{
    //    for (auto radar_postion : datapxy.set_radarposition)
    //    {

    //        PrintReportInfoinHTML(inputVehFileNameBMW, "HTML", pltfoldername, reportpath, radar_postion);

    //        if (pSelfDetectionPlotParameterFC != nullptr)
    //        {
    //            if ((pSelfDetectionPlotParameterFC + radar_postion)->speed[0].size() != 0)
    //            {
    //                string plottitle;
    //                std::map<int, string> radar_vehmap;
    //                radar_vehmap[0] = "RL Vehicle speed";
    //                radar_vehmap[1] = "RR Vehicle speed";
    //                radar_vehmap[2] = "FR Vehicle speed";
    //                radar_vehmap[3] = "FL Vehicle speed";
    //                radar_vehmap[5] = "FC Vehicle speed";


    //                for (auto val : radar_vehmap)
    //                {
    //                    if (val.first == radar_postion)
    //                    {
    //                        plottitle = val.second;
    //                    }
    //                }

    //                c2timing_info_scanindexfile1.clear();

    //                GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->speed[0],
    //                    c2timing_info_scanindexfile1, c2timing_info_scanindexfile1, 1,
    //                    inputVehFileNameBMW, plottitle.c_str(),
    //                    pltfoldername, "speed", reportpath, radar_postion);

    //            }


    //        }
    //        if (pSelfDetectionPlotParameterFC != nullptr)
    //        {
    //            if ((pSelfDetectionPlotParameterFC + radar_postion)->yawrate[0].size() != 0)
    //            {
    //                string plottitle;
    //                std::map<int, string> radar_vehmap;
    //                radar_vehmap[0] = "RL yaw rate";
    //                radar_vehmap[1] = "RR yaw rate";
    //                radar_vehmap[2] = "FR yaw rate";
    //                radar_vehmap[3] = "FL yaw rate";
    //                radar_vehmap[5] = "FC yaw rate";


    //                for (auto val : radar_vehmap)
    //                {
    //                    if (val.first == radar_postion)
    //                    {
    //                        plottitle = val.second;
    //                    }
    //                }

    //                c2timing_info_scanindexfile1.clear();

    //                GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->yawrate[0],
    //                    c2timing_info_scanindexfile1, c2timing_info_scanindexfile1, 1,
    //                    inputVehFileNameBMW, plottitle.c_str(),
    //                    pltfoldername, "yaw rate", reportpath, radar_postion);
    //            }


    //        }
    //        if (pSelfDetectionPlotParameterFC != nullptr)
    //        {
    //            c2timing_info_scanindexfile1_arrindex23.clear();
    //            c2timing_info_valuefile1_arrindex23.clear();


    //            if ((pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[0].size() != 0)
    //            {
    //                string plottitle;
    //                std::map<int, string> radar_rcsmap;
    //                radar_rcsmap[0] = "RL c2 timing info at arr(23)";
    //                radar_rcsmap[1] = "RR c2 timing info at arr(23)";
    //                radar_rcsmap[2] = "FR c2 timing info at arr(23)";
    //                radar_rcsmap[3] = "FL c2 timing info at arr(23)";
    //                radar_rcsmap[5] = "FC c2 timing info at arr(23)";

    //                for (auto val : radar_rcsmap)
    //                {
    //                    if (val.first == radar_postion)
    //                    {
    //                        plottitle = val.second;
    //                    }
    //                }

    //                for (auto val : (pSelfDetectionPlotParameterFC + radar_postion)->map_c2timing_info[0])
    //                {
    //                    c2timing_info_scanindexfile1_arrindex23.push_back(val.first);
    //                    c2timing_info_valuefile1_arrindex23.push_back(val.second.back());
    //                }
    //                c2timing_info_scanindexfile1.clear();
    //                c2timing_info_valuefile2.clear();

    //                GenerateScatterPlot(c2timing_info_scanindexfile1_arrindex23, c2timing_info_valuefile1_arrindex23,
    //                    c2timing_info_scanindexfile1, c2timing_info_valuefile2, 1,
    //                    inputVehFileNameBMW, plottitle.c_str(),
    //                    pltfoldername, "c2 timing", reportpath, radar_postion);

    //                //c2timing_info_scanindexfile1_arrindex23.clear();
    //                //c2timing_info_valuefile1_arrindex23.clear();
    //            }

    //        }

    //        if (datapxy.scanindex_scaling == 1 || datapxy.scanindex_scaling == 0)
    //        {
    //            if (datapxy.detectionplot == 1)
    //            {
    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcsmap;
    //                    radar_rcsmap[0] = "RL VALID DETECTION COUNT";
    //                    radar_rcsmap[1] = "RR VALID DETECTION COUNT";
    //                    radar_rcsmap[2] = "FR VALID DETECTION COUNT";
    //                    radar_rcsmap[3] = "FL VALID DETECTION COUNT";
    //                    radar_rcsmap[5] = "FC VALID DETECTION COUNT";

    //                    for (auto val : radar_rcsmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "valid detection count", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcsmap;
    //                    radar_rcsmap[0] = "RL Range BOX ";
    //                    radar_rcsmap[1] = "RR Range BOX";
    //                    radar_rcsmap[2] = "FR Range BOX";
    //                    radar_rcsmap[3] = "FL Range BOX";
    //                    radar_rcsmap[5] = "FC Range BOX";

    //                    for (auto val : radar_rcsmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    /* GenerateBoxPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh_ref_scale,(pSelfDetectionPlotParameterFC + radar_postion)->valid_detection_count[0],
    //                         0,
    //                         inputVehFileNameBMW, plottitle.c_str(),
    //                         pltfoldername, "box valid detection", reportpath, radar_postion);*/


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size() != 0)
    //                {


    //                    string plottitle;
    //                    std::map<int, string> radar_rangehismap;
    //                    radar_rangehismap[0] = "RL Range Boxplot";
    //                    radar_rangehismap[1] = "RR Range Boxplot";
    //                    radar_rangehismap[2] = "FR Range Boxplot";
    //                    radar_rangehismap[3] = "FL Range Boxplot";
    //                    radar_rangehismap[5] = "FC Range Boxplot";


    //                    for (auto& vrange : vrangeboxplot) {

    //                        for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[0])
    //                        {
    //                            vrange.push_back(rangeval);
    //                        }

    //                        for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[1])
    //                        {
    //                            vrange.push_back(rangeval);
    //                        }



    //                    }
    //                    /* GenerateBoxPlot_New(vrangeboxplot,
    //                         0,
    //                         inputVehFileNameBMW, plottitle.c_str(),
    //                         pltfoldername, "range", reportpath, radar_postion);*/






    //                    for (auto val : radar_rangehismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }
    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size() != 0)
    //                    {



    //                        if (radar_postion == 0)
    //                        {
    //                            for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[0])
    //                            {
    //                                file1range_boxplot.push_back(rangeval);
    //                                file1range_sensor_file_info.push_back("RLVeh");
    //                            }
    //                            //range_sensor_file_info.assign((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size(), "RLveh");
    //                        }




    //                    }
    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->range[1].size() != 0)
    //                    {



    //                        if (radar_postion == 0)
    //                        {
    //                            for (auto rangeval : (pSelfDetectionPlotParameterFC + radar_postion)->range[1])
    //                            {
    //                                file2range_boxplot.push_back(rangeval);
    //                                file2range_sensor_file_info.push_back("RLResim");
    //                            }
    //                            //range_sensor_file_info.assign((pSelfDetectionPlotParameterFC + radar_postion)->range[1].size(), "RLRes");
    //                        }




    //                    }

    //                    if (radar_postion == 0)
    //                    {


    //                        //GenerateBoxPlot(file1range_boxplot, file1range_sensor_file_info, file2range_boxplot, file2range_sensor_file_info,
    //                        //  0,
    //                        //  inputVehFileNameBMW, "Range Box"/*plottitle.c_str()*/,
    //                        //  pltfoldername, "range", reportpath, radar_postion);
    //                    }

    //                }


    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range_max[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcsmap;
    //                    radar_rcsmap[0] = "RL MAX RANGE";
    //                    radar_rcsmap[1] = "RR MAX RANGE";
    //                    radar_rcsmap[2] = "FR MAX RANGE";
    //                    radar_rcsmap[3] = "FL MAX RANGE";
    //                    radar_rcsmap[5] = "FC MAX RANGE";

    //                    for (auto val : radar_rcsmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->range_max[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim_ref_scale, (pSelfDetectionPlotParameterFC + radar_postion)->range_max[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "max range", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_rangehismap;
    //                    radar_rangehismap[0] = "RL Range Histogram";
    //                    radar_rangehismap[1] = "RR Range Histogram";
    //                    radar_rangehismap[2] = "FR Range Histogram";
    //                    radar_rangehismap[3] = "FL Range Histogram";
    //                    radar_rangehismap[5] = "FC Range Histogram";

    //                    for (auto val : radar_rangehismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }
    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->range[1].size() == 0)
    //                    {

    //                        (pSelfDetectionPlotParameterFC + radar_postion)->range[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->range[1].size(), 0);

    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->range[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->range[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "range", reportpath, radar_postion, 2);

    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->range[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->range[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "range", reportpath, radar_postion, 0);
    //                    }



    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rangeratehismap;
    //                    radar_rangeratehismap[0] = "RL Range Rate Histogram";
    //                    radar_rangeratehismap[1] = "RR Range Rate Histogram";
    //                    radar_rangeratehismap[2] = "FR Range Rate Histogram";
    //                    radar_rangeratehismap[3] = "FL Range Rate Histogram";
    //                    radar_rangeratehismap[5] = "FC Range Rate Histogram";

    //                    for (auto val : radar_rangeratehismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1].size() == 0)
    //                    {

    //                        (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1].size(), 0);

    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "rangerate", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "rangerate", reportpath, radar_postion, 0);
    //                    }


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_azimuth_hismap;
    //                    radar_azimuth_hismap[0] = "RL Azimuth Histogram";
    //                    radar_azimuth_hismap[1] = "RR Azimuth Histogram";
    //                    radar_azimuth_hismap[2] = "FR Azimuth Histogram";
    //                    radar_azimuth_hismap[3] = "FL Azimuth Histogram";
    //                    radar_azimuth_hismap[5] = "FC Azimuth Histogram";

    //                    for (auto val : radar_azimuth_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1].size() == 0)
    //                    {
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1].size(), 0);
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "azimuth", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "azimuth", reportpath, radar_postion, 0);
    //                    }




    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->elevation[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_elevation_hismap;
    //                    radar_elevation_hismap[0] = "RL Elevation Histogram";
    //                    radar_elevation_hismap[1] = "RR Elevation Histogram";
    //                    radar_elevation_hismap[2] = "FR Elevation Histogram";
    //                    radar_elevation_hismap[3] = "FL Elevation Histogram";
    //                    radar_elevation_hismap[5] = "FC Elevation Histogram";

    //                    for (auto val : radar_elevation_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->elevation[1].size() == 0)
    //                    {
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->elevation[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->elevation[1].size(), 0);

    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->elevation[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->elevation[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "elevation", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->elevation[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->elevation[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "elevation", reportpath, radar_postion, 0);

    //                    }




    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->rcs[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcs_hismap;
    //                    radar_rcs_hismap[0] = "RL RCS Histogram";
    //                    radar_rcs_hismap[1] = "RR RCS Histogram";
    //                    radar_rcs_hismap[2] = "FR RCS Histogram";
    //                    radar_rcs_hismap[3] = "FL RCS Histogram";
    //                    radar_rcs_hismap[5] = "FC RCS Histogram";

    //                    for (auto val : radar_rcs_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->rcs[1].size() == 0)
    //                    {
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->rcs[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->rcs[1].size(), 0);
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->rcs[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->rcs[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "rcs", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->rcs[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->rcs[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "rcs", reportpath, radar_postion, 0);
    //                    }




    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->snr[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_snr_hismap;
    //                    radar_snr_hismap[0] = "RL SNR Histogram";
    //                    radar_snr_hismap[1] = "RR SNR Histogram";
    //                    radar_snr_hismap[2] = "FR SNR Histogram";
    //                    radar_snr_hismap[3] = "FL SNR Histogram";
    //                    radar_snr_hismap[5] = "FC SNR Histogram";

    //                    for (auto val : radar_snr_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->snr[1].size() == 0)
    //                    {
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->snr[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->snr[1].size(), 0);
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->snr[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->snr[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "snr", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->snr[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->snr[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "snr", reportpath, radar_postion, 0);
    //                    }





    //                }

    //                /*if (vec_file1_bistatic_det.size() != 0)
    //                {


    //                    string plottitle;
    //                    std::map<int, string> radar_bistatic_hismap;
    //                    radar_bistatic_hismap[0] = "RL Bistatic Histogram";
    //                    radar_bistatic_hismap[1] = "RR Bistatic Histogram";
    //                    radar_bistatic_hismap[2] = "FR Bistatic Histogram";
    //                    radar_bistatic_hismap[3] = "FL Bistatic Histogram";
    //                    radar_bistatic_hismap[5] = "FC Bistatic Histogram";

    //                    for (auto val : radar_bistatic_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if (vec_file1_bistatic_det.size() != 0)
    //                    {
    //                        GenerateHistogramPlot(vec_file1_bistatic_det,
    //                            vec_file2_bistatic_det,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "bistatic", reportpath, radar_postion, 0);
    //                    }






    //                }*/


    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->az_conf[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_azconf_hismap;
    //                    radar_azconf_hismap[0] = "RL azimuth confidence Histogram";
    //                    radar_azconf_hismap[1] = "RR azimuth confidence Histogram";
    //                    radar_azconf_hismap[2] = "FR azimuth confidence Histogram";
    //                    radar_azconf_hismap[3] = "FL azimuth confidence Histogram";
    //                    radar_azconf_hismap[5] = "FC azimuth confidence Histogram";

    //                    for (auto val : radar_azconf_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->az_conf[0].size() != 0)
    //                    {
    //                        cout << "\n az conf histogram";
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->az_conf[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->az_conf[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "az conf", reportpath, radar_postion, 0);
    //                    }


    //                }


    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->el_conf[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_elconf_hismap;
    //                    radar_elconf_hismap[0] = "RL elevation confidence Histogram";
    //                    radar_elconf_hismap[1] = "RR elevation confidence Histogram";
    //                    radar_elconf_hismap[2] = "FR elevation confidence Histogram";
    //                    radar_elconf_hismap[3] = "FL elevation confidence Histogram";
    //                    radar_elconf_hismap[5] = "FC elevation confidence Histogram";

    //                    for (auto val : radar_elconf_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->el_conf[0].size() != 0)
    //                    {
    //                        //cout << "\n az conf histogram";
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->el_conf[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->el_conf[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "el conf", reportpath, radar_postion, 0);
    //                    }


    //                }


    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_isBistatic_hismap;
    //                    radar_isBistatic_hismap[0] = "RL isBistatic Histogram";
    //                    radar_isBistatic_hismap[1] = "RR isBistatic Histogram";
    //                    radar_isBistatic_hismap[2] = "FR isBistatic Histogram";
    //                    radar_isBistatic_hismap[3] = "FL isBistatic Histogram";
    //                    radar_isBistatic_hismap[5] = "FC isBistatic Histogram";

    //                    for (auto val : radar_isBistatic_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[0].size() != 0)
    //                    {
    //                        //cout << "\n az conf histogram";
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->isBistatic[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "isBistatic", reportpath, radar_postion, 0);
    //                    }


    //                }



    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->height[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_snr_hismap;
    //                    radar_snr_hismap[0] = "RL Height Histogram";
    //                    radar_snr_hismap[1] = "RR Height Histogram";
    //                    radar_snr_hismap[2] = "FR Height Histogram";
    //                    radar_snr_hismap[3] = "FL Height Histogram";
    //                    radar_snr_hismap[5] = "FC Height Histogram";

    //                    for (auto val : radar_snr_hismap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    if ((pSelfDetectionPlotParameterFC + radar_postion)->height[1].size() == 0)
    //                    {
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->height[1].assign((pSelfDetectionPlotParameterFC + radar_postion)->height[1].size(), 0);
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->height[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->height[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "height", reportpath, radar_postion, 2);
    //                    }
    //                    else
    //                    {
    //                        GenerateHistogramPlot((pSelfDetectionPlotParameterFC + radar_postion)->height[0],
    //                            (pSelfDetectionPlotParameterFC + radar_postion)->height[1],
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "height", reportpath, radar_postion, 0);
    //                    }





    //                }

    //            }

    //            if (datapxy.detectionplot == 1)
    //            {

    //                string plottitle;
    //                std::map<int, string> radar_rangeratehismap;
    //                radar_rangeratehismap[0] = "RL MaxRange";
    //                radar_rangeratehismap[1] = "RR MaxRange";
    //                radar_rangeratehismap[2] = "FR MaxRange";
    //                radar_rangeratehismap[3] = "FL MaxRange";
    //                radar_rangeratehismap[5] = "FC MaxRange";


    //                for (auto val : radar_rangeratehismap)
    //                {
    //                    if (val.first == radar_postion)
    //                    {
    //                        plottitle = val.second;
    //                    }
    //                }

    //                std::vector<double> f1_xdata, f1_ydata, f2_xdata, f2_ydata;
    //                f1_xdata.clear();
    //                f1_ydata.clear();
    //                f2_xdata.clear();
    //                f2_ydata.clear();

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->map_range[0].size() != 0)
    //                {
    //                    for (auto val : (pSelfDetectionPlotParameterFC + radar_postion)->map_range[0])
    //                    {
    //                        if (int(val.first) != 0 && val.second.size() != 0)
    //                            f1_xdata.push_back(val.first);
    //                        double max_rangevalue;
    //                        if (val.second.size() != 0)
    //                        {
    //                            max_rangevalue = *std::max_element(val.second.begin(), val.second.end());
    //                            f1_ydata.push_back(max_rangevalue);

    //                        }

    //                    }
    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->map_range[1].size() != 0)
    //                {
    //                    for (auto val : (pSelfDetectionPlotParameterFC + radar_postion)->map_range[1])
    //                    {
    //                        if (int(val.first) != 0 && val.second.size() != 0)
    //                            f2_xdata.push_back(val.first);
    //                        double max_rangevalue;

    //                        if (val.second.size() != 0)
    //                        {
    //                            max_rangevalue = *std::max_element(val.second.begin(), val.second.end());
    //                            f2_ydata.push_back(max_rangevalue);
    //                        }

    //                    }
    //                }

    //                if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
    //                {
    //                    GenerateScatterPlot(f1_xdata, f1_ydata,
    //                        f2_xdata, f2_ydata, 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "max range/si", reportpath, radar_postion);
    //                }

    //            }
    //        }
    //        if (datapxy.scanindex_scaling == 1)
    //        {
    //            if (datapxy.trackerplot == 1)
    //            {
    //                if (datapxy.customerid == STLA_FLR4P)
    //                {
    //                    DataPrep_SlicedTracker_plot(pTrackerPlotParamFC_FLR4P, inputVehFileNameBMW, pltfoldername, reportpath, radar_postion);
    //                }
    //                if (datapxy.customerid == HONDA_GEN5)
    //                {
    //                    DataPrep_SlicedTracker_plot(pTrackerPlotParam[radar_postion], inputVehFileNameBMW, pltfoldername, reportpath, radar_postion);

    //                }

    //            }

    //            if (datapxy.trackerplot == 1)
    //            {
    //                //Bar plot Data preparation : FC Object classification
    //                std::vector<int> file1_obj_class, file2_obj_class;
    //                std::vector<string> obj_class_xdata = { "ped", "2wheeler","car","truck" };


    //                if (file1_object_classification[1].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[1].back());
    //                }

    //                if (file1_object_classification[2].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[2].back());
    //                }

    //                if (file1_object_classification[3].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[3].back());
    //                }

    //                if (file1_object_classification[4].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[4].back());
    //                }


    //                if (file2_object_classification[1].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[1].back());
    //                }

    //                if (file2_object_classification[2].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[2].back());
    //                }

    //                if (file2_object_classification[3].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[3].back());
    //                }

    //                if (file2_object_classification[4].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[4].back());
    //                }


    //                if (file1_obj_class.size() != 0 && file2_obj_class.size() != 0)
    //                {
    //                    vec_object_classification.push_back(file1_obj_class);
    //                    vec_object_classification.push_back(file2_obj_class);

    //                }


    //                if (vec_object_classification.size() != 0)
    //                {/*
    //                    GenerateBarPlots(obj_class_xdata, vec_object_classification,
    //                        inputVehFileNameBMW, "Object classification",
    //                        pltfoldername, "xpos", reportpath, radarpos);*/
    //                }




    //                //Bar plot : "FC moving object count
    //                std::vector<string> obj_mvgobjects_xdata = { "moving objects" };
    //                std::vector<int> file1_moving_objects, file2_moving_objects;

    //                if (vec_file1_moving_objects.size() != 0)
    //                {
    //                    file1_moving_objects.push_back(vec_file1_moving_objects.back());
    //                }


    //                if (vec_file2_moving_objects.size() != 0)
    //                {
    //                    file2_moving_objects.push_back(vec_file2_moving_objects.back());
    //                }


    //                if (file1_moving_objects.size() != 0 && file2_moving_objects.size() != 0)
    //                {
    //                    vec_moving_objects.push_back(file1_moving_objects);
    //                    vec_moving_objects.push_back(file2_moving_objects);
    //                }



    //                if (vec_moving_objects.size() != 0)
    //                {
    //                    /*GenerateBarPlots(obj_mvgobjects_xdata, vec_moving_objects,
    //                        inputVehFileNameBMW, "moving object count",
    //                        pltfoldername, "xpos", reportpath, radarpos);*/

    //                }





    //            }
    //            if (datapxy.detectionplot == 1)
    //            {

    //                DataPreparation_Slicing_Detection_Scatterplot(inputVehFileNameBMW, pltfoldername, reportpath, radar_postion);

    //            }

    //            if (datapxy.detectionplot == 1) //fixed crash in this blocked hence changed from 1 to 0
    //            {


    //                //Bar plot Data preparation : FC Detection Properties count
    //                std::vector<string> det_xdata1 = { "bistatic","singletarget","singletargetazimuth" };
    //                std::vector<string> det_xdata;
    //                std::vector<int> f1_det_prop, f2_det_prop;

    //                if (vec_file1_bistatic_det.size() != 0 && vec_file2_bistatic_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_bistatic_det.back());
    //                    f2_det_prop.push_back(vec_file2_bistatic_det.back());
    //                    det_xdata.push_back("bistatic");
    //                }
    //                else if (vec_file1_bistatic_det.size() != 0 && vec_file2_bistatic_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_bistatic_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("bistatic");
    //                }
    //                else if (vec_file1_bistatic_det.size() == 0 && vec_file2_bistatic_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_bistatic_det.back());
    //                    det_xdata.push_back("bistatic");
    //                }





    //                if (vec_file1_singletarget_det.size() != 0 && vec_file2_singletarget_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletarget_det.back());
    //                    f2_det_prop.push_back(vec_file2_singletarget_det.back());
    //                    det_xdata.push_back("singletarget");
    //                }
    //                else if (vec_file1_singletarget_det.size() != 0 && vec_file2_singletarget_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletarget_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("singletarget");
    //                }
    //                else if (vec_file1_singletarget_det.size() == 0 && vec_file2_singletarget_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_singletarget_det.back());
    //                    det_xdata.push_back("singletarget");
    //                }




    //                if (vec_file1_singletargetazimuth_det.size() != 0 && vec_file2_singletargetazimuth_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletargetazimuth_det.back());
    //                    f2_det_prop.push_back(vec_file2_singletargetazimuth_det.back());
    //                    det_xdata.push_back("singletargetazimuth");
    //                }
    //                else if (vec_file1_singletargetazimuth_det.size() != 0 && vec_file2_singletargetazimuth_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletargetazimuth_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("singletargetazimuth");
    //                }
    //                else if (vec_file1_singletargetazimuth_det.size() == 0 && vec_file2_singletargetazimuth_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_singletargetazimuth_det.back());
    //                    det_xdata.push_back("singletargetazimuth");
    //                }




    //                if (vec_file1_dopplermixedinterval_det.size() != 0 && vec_file2_dopplermixedinterval_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplermixedinterval_det.back());
    //                    f2_det_prop.push_back(vec_file2_dopplermixedinterval_det.back());
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }
    //                else if (vec_file1_dopplermixedinterval_det.size() != 0 && vec_file2_dopplermixedinterval_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplermixedinterval_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }
    //                else if (vec_file1_dopplermixedinterval_det.size() == 0 && vec_file2_dopplermixedinterval_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_dopplermixedinterval_det.back());
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }




    //                if (vec_file1_dopplerunfolding_det.size() != 0 && vec_file2_dopplerunfolding_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplerunfolding_det.back());
    //                    f2_det_prop.push_back(vec_file2_dopplerunfolding_det.back());
    //                    det_xdata.push_back("dopplerunfolding");
    //                }
    //                else if (vec_file1_dopplerunfolding_det.size() != 0 && vec_file2_dopplerunfolding_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplerunfolding_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("dopplerunfolding");
    //                }
    //                else if (vec_file1_dopplerunfolding_det.size() == 0 && vec_file2_dopplerunfolding_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_dopplerunfolding_det.back());
    //                    det_xdata.push_back("dopplerunfolding");
    //                }


    //                if (f1_det_prop.size() != 0 && f2_det_prop.size() != 0)
    //                {
    //                    vec_bistatic_det.push_back(f1_det_prop);
    //                    vec_bistatic_det.push_back(f2_det_prop);
    //                }

    //                if (vec_bistatic_det.size() != 0)
    //                {
    //                    /*GenerateBarPlots(det_xdata, vec_bistatic_det,
    //                        inputVehFileNameBMW, " Detection Properties count",
    //                        pltfoldername, "xpos", reportpath, radarpos);*/
    //                }

    //            }
    //        }

    //        if (datapxy.scanindex_scaling == 0)
    //        {
    //            if (datapxy.customerid == HONDA_GEN5)//for Honda
    //            {

    //                if (pTrackerPlotParam[radar_postion]->vsc_xpos[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL XPosition";
    //                    radar_xpositionmap[1] = "RR XPosition";
    //                    radar_xpositionmap[2] = "FR XPosition";
    //                    radar_xpositionmap[3] = "FL XPosition";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_xpos[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_xpos[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "xposition", reportpath, radar_postion);
    //                }



    //                if (pTrackerPlotParam[radar_postion]->vsc_ypos[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL YPosition";
    //                    radar_xpositionmap[1] = "RR YPosition";
    //                    radar_xpositionmap[2] = "FR YPosition";
    //                    radar_xpositionmap[3] = "FL YPosition";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_ypos[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_ypos[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "yposition", reportpath, radar_postion);
    //                }



    //                if (pTrackerPlotParam[radar_postion]->vsc_xvel[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL XVELOCITY";
    //                    radar_xpositionmap[1] = "RR XVELOCITY";
    //                    radar_xpositionmap[2] = "FR XVELOCITY";
    //                    radar_xpositionmap[3] = "FL XVELOCITY";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_xvel[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_xvel[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "xvelocity", reportpath, radar_postion);
    //                }


    //                if (pTrackerPlotParam[radar_postion]->vsc_yvel[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL YVELOCITY";
    //                    radar_xpositionmap[1] = "RR YVELOCITY";
    //                    radar_xpositionmap[2] = "FR YVELOCITY";
    //                    radar_xpositionmap[3] = "FL YVELOCITY";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_yvel[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_yvel[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "yvelocity", reportpath, radar_postion);
    //                }


    //                if (pTrackerPlotParam[radar_postion]->vsc_xacc[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL XACCELERATION";
    //                    radar_xpositionmap[1] = "RR XACCELERATION";
    //                    radar_xpositionmap[2] = "FR XACCELERATION";
    //                    radar_xpositionmap[3] = "FL XACCELERATION";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_xacc[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_xacc[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "xacceleration", reportpath, radar_postion);
    //                }


    //                if (pTrackerPlotParam[radar_postion]->vsc_yacc[0].size() != 0)
    //                {

    //                    string plottitle;
    //                    std::map<int, string> radar_xpositionmap;
    //                    radar_xpositionmap[0] = "RL YACCELERATION";
    //                    radar_xpositionmap[1] = "RR YACCELERATION";
    //                    radar_xpositionmap[2] = "FR YACCELERATION";
    //                    radar_xpositionmap[3] = "FL YACCELERATION";

    //                    for (auto val : radar_xpositionmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }


    //                    GenerateScatterPlot(pTrackerPlotParam[radar_postion]->vcs_si_veh, pTrackerPlotParam[radar_postion]->vsc_yacc[0],
    //                        pTrackerPlotParam[radar_postion]->vcs_si_resim, pTrackerPlotParam[radar_postion]->vsc_yacc[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "yacceleration", reportpath, radar_postion);
    //                }
    //            }
    //        }
    //        if (datapxy.scanindex_scaling == 0)
    //        {

    //            if (datapxy.trackerplot == 1)
    //            {
    //                //Tracker plot

    //                if (pTrackerPlotParamFC_FLR4P != nullptr)
    //                {
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_xpos[0].size() != 0)
    //                    {

    //                        string plottitle;
    //                        std::map<int, string> radar_xpositionmap;
    //                        radar_xpositionmap[0] = "RL XPosition";
    //                        radar_xpositionmap[1] = "RR XPosition";
    //                        radar_xpositionmap[2] = "FR XPosition";
    //                        radar_xpositionmap[3] = "FL XPosition";
    //                        radar_xpositionmap[5] = "FC XPosition";
    //                        radar_xpositionmap[19] = "DC XPosition";

    //                        for (auto val : radar_xpositionmap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }


    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_xpos[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_xpos[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "xposition", reportpath, radar_postion);
    //                    }
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_ypos[0].size() != 0)
    //                    {
    //                        string plottitle;
    //                        std::map<int, string> radar_ypositionmap;
    //                        radar_ypositionmap[0] = "RL YPosition";
    //                        radar_ypositionmap[1] = "RR YPosition";
    //                        radar_ypositionmap[2] = "FR YPosition";
    //                        radar_ypositionmap[3] = "FL YPosition";
    //                        radar_ypositionmap[5] = "FC YPosition";
    //                        radar_ypositionmap[19] = "DC YPosition";

    //                        for (auto val : radar_ypositionmap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }



    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_ypos[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_ypos[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "yposition", reportpath, radar_postion);

    //                    }
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_xvel[0].size() != 0)
    //                    {

    //                        string plottitle;
    //                        std::map<int, string> radar_xvelocitymap;
    //                        radar_xvelocitymap[0] = "RL XVelocity";
    //                        radar_xvelocitymap[1] = "RR XVelocity";
    //                        radar_xvelocitymap[2] = "FR XVelocity";
    //                        radar_xvelocitymap[3] = "FL XVelocity";
    //                        radar_xvelocitymap[5] = "FC XVelocity";

    //                        for (auto val : radar_xvelocitymap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }


    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_xvel[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_xvel[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "xvelocity", reportpath, radar_postion);
    //                    }
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_yvel[0].size() != 0)
    //                    {

    //                        string plottitle;
    //                        std::map<int, string> radar_yvelocitymap;
    //                        radar_yvelocitymap[0] = "RL YVelocity";
    //                        radar_yvelocitymap[1] = "RR YVelocity";
    //                        radar_yvelocitymap[2] = "FR YVelocity";
    //                        radar_yvelocitymap[3] = "FL YVelocity";
    //                        radar_yvelocitymap[5] = "FC YVelocity";

    //                        for (auto val : radar_yvelocitymap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }



    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_yvel[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_yvel[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "yvelocity", reportpath, radar_postion);
    //                    }
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_xacc[0].size() != 0)
    //                    {
    //                        string plottitle;
    //                        std::map<int, string> radar_xacclerationmap;
    //                        radar_xacclerationmap[0] = "RL XAcceleration";
    //                        radar_xacclerationmap[1] = "RR XAcceleration";
    //                        radar_xacclerationmap[2] = "FR XAcceleration";
    //                        radar_xacclerationmap[3] = "FL XAcceleration";
    //                        radar_xacclerationmap[5] = "FC XAcceleration";

    //                        for (auto val : radar_xacclerationmap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }


    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_xacc[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_xacc[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "xacceleration", reportpath, radar_postion);

    //                    }
    //                    if (pTrackerPlotParamFC_FLR4P->vsc_yacc[0].size() != 0)
    //                    {
    //                        string plottitle;
    //                        std::map<int, string> radar_yacclerationmap;
    //                        radar_yacclerationmap[0] = "RL YAcceleration";
    //                        radar_yacclerationmap[1] = "RR YAcceleration";
    //                        radar_yacclerationmap[2] = "FR YAcceleration";
    //                        radar_yacclerationmap[3] = "FL YAcceleration";
    //                        radar_yacclerationmap[5] = "FC YAcceleration";

    //                        for (auto val : radar_yacclerationmap)
    //                        {
    //                            if (val.first == radar_postion)
    //                            {
    //                                plottitle = val.second;
    //                            }
    //                        }


    //                        GenerateScatterPlot(pTrackerPlotParamFC_FLR4P->vcs_si_veh, pTrackerPlotParamFC_FLR4P->vsc_yacc[0],
    //                            pTrackerPlotParamFC_FLR4P->vcs_si_resim, pTrackerPlotParamFC_FLR4P->vsc_yacc[1], 0,
    //                            inputVehFileNameBMW, plottitle.c_str(),
    //                            pltfoldername, "yacceleration", reportpath, radar_postion);

    //                    }
    //                }




    //            }

    //            if (datapxy.trackerplot == 1)
    //            {
    //                //Bar plot Data preparation : FC Object classification
    //                std::vector<int> file1_obj_class, file2_obj_class;
    //                std::vector<string> obj_class_xdata = { "ped", "2wheeler","car","truck" };


    //                if (file1_object_classification[1].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[1].back());
    //                }

    //                if (file1_object_classification[2].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[2].back());
    //                }

    //                if (file1_object_classification[3].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[3].back());
    //                }

    //                if (file1_object_classification[4].size() != 0)
    //                {
    //                    file1_obj_class.push_back(file1_object_classification[4].back());
    //                }


    //                if (file2_object_classification[1].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[1].back());
    //                }

    //                if (file2_object_classification[2].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[2].back());
    //                }

    //                if (file2_object_classification[3].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[3].back());
    //                }

    //                if (file2_object_classification[4].size() != 0)
    //                {
    //                    file2_obj_class.push_back(file2_object_classification[4].back());
    //                }


    //                if (file1_obj_class.size() != 0 && file2_obj_class.size() != 0)
    //                {
    //                    vec_object_classification.push_back(file1_obj_class);
    //                    vec_object_classification.push_back(file2_obj_class);

    //                }


    //                if (vec_object_classification.size() != 0)
    //                {
    //                    /* GenerateBarPlots(obj_class_xdata, vec_object_classification,
    //                         inputVehFileNameBMW, "Object classification",
    //                         pltfoldername, "xpos", reportpath, radarpos);*/
    //                }




    //                //Bar plot Data preparation : "FC moving object count
    //                std::vector<string> obj_mvgobjects_xdata = { "moving objects" };
    //                std::vector<int> file1_moving_objects, file2_moving_objects;

    //                if (vec_file1_moving_objects.size() != 0)
    //                {
    //                    file1_moving_objects.push_back(vec_file1_moving_objects.back());
    //                }


    //                if (vec_file2_moving_objects.size() != 0)
    //                {
    //                    file2_moving_objects.push_back(vec_file2_moving_objects.back());
    //                }


    //                if (file1_moving_objects.size() != 0 && file2_moving_objects.size() != 0)
    //                {
    //                    vec_moving_objects.push_back(file1_moving_objects);
    //                    vec_moving_objects.push_back(file2_moving_objects);
    //                }



    //                if (vec_moving_objects.size() != 0)
    //                {
    //                    /*  GenerateBarPlots(obj_mvgobjects_xdata, vec_moving_objects,
    //                                       inputVehFileNameBMW, "moving object count",
    //                                       pltfoldername, "xpos", reportpath, radarpos);*/

    //                }





    //            }

    //            if (datapxy.detectionplot == 1)
    //            {
    //                //Detection plots
    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rangemap;
    //                    radar_rangemap[0] = "RL Range";
    //                    radar_rangemap[1] = "RR Range";
    //                    radar_rangemap[2] = "FR Range";
    //                    radar_rangemap[3] = "FL Range";
    //                    radar_rangemap[5] = "FC Range";

    //                    for (auto val : radar_rangemap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->range[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->range[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "range", reportpath, radar_postion);

    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_range_ratemap;
    //                    radar_range_ratemap[0] = "RL RangeRate";
    //                    radar_range_ratemap[1] = "RR RangeRate";
    //                    radar_range_ratemap[2] = "FR RangeRate";
    //                    radar_range_ratemap[3] = "FL RangeRate";
    //                    radar_range_ratemap[5] = "FC RangeRate";

    //                    for (auto val : radar_range_ratemap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->range_rate[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "rangerate", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_azimuthmap;
    //                    radar_azimuthmap[0] = "RL Azimuth";
    //                    radar_azimuthmap[1] = "RR Azimuth";
    //                    radar_azimuthmap[2] = "FR Azimuth";
    //                    radar_azimuthmap[3] = "FL Azimuth";
    //                    radar_azimuthmap[5] = "FC Azimuth";

    //                    for (auto val : radar_azimuthmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->azimuth[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "azimuth", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->elevation[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_elevationmap;
    //                    radar_elevationmap[0] = "RL Elevation";
    //                    radar_elevationmap[1] = "RR Elevation";
    //                    radar_elevationmap[2] = "FR Elevation";
    //                    radar_elevationmap[3] = "FL Elevation";
    //                    radar_elevationmap[5] = "FC Elevation";

    //                    for (auto val : radar_elevationmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->elevation[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->elevation[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "elevation", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->amp[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_amplitudemap;
    //                    radar_amplitudemap[0] = "RL Amplitude";
    //                    radar_amplitudemap[1] = "RR Amplitude";
    //                    radar_amplitudemap[2] = "FR Amplitude";
    //                    radar_amplitudemap[3] = "FL Amplitude";
    //                    radar_amplitudemap[5] = "FC Amplitude";

    //                    for (auto val : radar_amplitudemap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }
    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->amp[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->amp[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "amplitude", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->snr[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_snrmap;
    //                    radar_snrmap[0] = "RL SNR";
    //                    radar_snrmap[1] = "RR SNR";
    //                    radar_snrmap[2] = "FR SNR";
    //                    radar_snrmap[3] = "FL SNR";
    //                    radar_snrmap[5] = "FC SNR";

    //                    for (auto val : radar_snrmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->snr[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->snr[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "snr", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->rcs[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcsmap;
    //                    radar_rcsmap[0] = "RL RCS";
    //                    radar_rcsmap[1] = "RR RCS";
    //                    radar_rcsmap[2] = "FR RCS";
    //                    radar_rcsmap[3] = "FL RCS";
    //                    radar_rcsmap[5] = "FC RCS";

    //                    for (auto val : radar_rcsmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->rcs[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->rcs[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "rcs", reportpath, radar_postion);


    //                }

    //                if ((pSelfDetectionPlotParameterFC + radar_postion)->height[0].size() != 0)
    //                {
    //                    string plottitle;
    //                    std::map<int, string> radar_rcsmap;
    //                    radar_rcsmap[0] = "RL HEIGHT";
    //                    radar_rcsmap[1] = "RR HEIGHT";
    //                    radar_rcsmap[2] = "FR HEIGHT";
    //                    radar_rcsmap[3] = "FL HEIGHT";
    //                    radar_rcsmap[5] = "FC HEIGHT";

    //                    for (auto val : radar_rcsmap)
    //                    {
    //                        if (val.first == radar_postion)
    //                        {
    //                            plottitle = val.second;
    //                        }
    //                    }

    //                    GenerateScatterPlot((pSelfDetectionPlotParameterFC + radar_postion)->si_veh, (pSelfDetectionPlotParameterFC + radar_postion)->height[0],
    //                        (pSelfDetectionPlotParameterFC + radar_postion)->si_resim, (pSelfDetectionPlotParameterFC + radar_postion)->height[1], 0,
    //                        inputVehFileNameBMW, plottitle.c_str(),
    //                        pltfoldername, "height", reportpath, radar_postion);


    //                }







    //            }

    //            if (datapxy.detectionplot == 1) //fixed crash in this blocked hence changed from 1 to 0
    //            {


    //                //Bar plot Data preparation : FC Detection Properties count
    //                std::vector<string> det_xdata1 = { "bistatic","singletarget","singletargetazimuth" };
    //                std::vector<string> det_xdata;
    //                std::vector<int> f1_det_prop, f2_det_prop;

    //                if (vec_file1_bistatic_det.size() != 0 && vec_file2_bistatic_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_bistatic_det.back());
    //                    f2_det_prop.push_back(vec_file2_bistatic_det.back());
    //                    det_xdata.push_back("bistatic");
    //                }
    //                else if (vec_file1_bistatic_det.size() != 0 && vec_file2_bistatic_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_bistatic_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("bistatic");
    //                }
    //                else if (vec_file1_bistatic_det.size() == 0 && vec_file2_bistatic_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_bistatic_det.back());
    //                    det_xdata.push_back("bistatic");
    //                }





    //                if (vec_file1_singletarget_det.size() != 0 && vec_file2_singletarget_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletarget_det.back());
    //                    f2_det_prop.push_back(vec_file2_singletarget_det.back());
    //                    det_xdata.push_back("singletarget");
    //                }
    //                else if (vec_file1_singletarget_det.size() != 0 && vec_file2_singletarget_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletarget_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("singletarget");
    //                }
    //                else if (vec_file1_singletarget_det.size() == 0 && vec_file2_singletarget_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_singletarget_det.back());
    //                    det_xdata.push_back("singletarget");
    //                }




    //                if (vec_file1_singletargetazimuth_det.size() != 0 && vec_file2_singletargetazimuth_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletargetazimuth_det.back());
    //                    f2_det_prop.push_back(vec_file2_singletargetazimuth_det.back());
    //                    det_xdata.push_back("singletargetazimuth");
    //                }
    //                else if (vec_file1_singletargetazimuth_det.size() != 0 && vec_file2_singletargetazimuth_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_singletargetazimuth_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("singletargetazimuth");
    //                }
    //                else if (vec_file1_singletargetazimuth_det.size() == 0 && vec_file2_singletargetazimuth_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_singletargetazimuth_det.back());
    //                    det_xdata.push_back("singletargetazimuth");
    //                }




    //                if (vec_file1_dopplermixedinterval_det.size() != 0 && vec_file2_dopplermixedinterval_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplermixedinterval_det.back());
    //                    f2_det_prop.push_back(vec_file2_dopplermixedinterval_det.back());
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }
    //                else if (vec_file1_dopplermixedinterval_det.size() != 0 && vec_file2_dopplermixedinterval_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplermixedinterval_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }
    //                else if (vec_file1_dopplermixedinterval_det.size() == 0 && vec_file2_dopplermixedinterval_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_dopplermixedinterval_det.back());
    //                    det_xdata.push_back("dopplermixedinterval");
    //                }




    //                if (vec_file1_dopplerunfolding_det.size() != 0 && vec_file2_dopplerunfolding_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplerunfolding_det.back());
    //                    f2_det_prop.push_back(vec_file2_dopplerunfolding_det.back());
    //                    det_xdata.push_back("dopplerunfolding");
    //                }
    //                else if (vec_file1_dopplerunfolding_det.size() != 0 && vec_file2_dopplerunfolding_det.size() == 0)
    //                {
    //                    f1_det_prop.push_back(vec_file1_dopplerunfolding_det.back());
    //                    f2_det_prop.push_back(0);
    //                    det_xdata.push_back("dopplerunfolding");
    //                }
    //                else if (vec_file1_dopplerunfolding_det.size() == 0 && vec_file2_dopplerunfolding_det.size() != 0)
    //                {
    //                    f1_det_prop.push_back(0);
    //                    f2_det_prop.push_back(vec_file2_dopplerunfolding_det.back());
    //                    det_xdata.push_back("dopplerunfolding");
    //                }


    //                if (f1_det_prop.size() != 0 && f2_det_prop.size() != 0)
    //                {
    //                    vec_bistatic_det.push_back(f1_det_prop);
    //                    vec_bistatic_det.push_back(f2_det_prop);
    //                }

    //                if (vec_bistatic_det.size() != 0)
    //                {
    //                    /* GenerateBarPlots(det_xdata, vec_bistatic_det,
    //                         inputVehFileNameBMW, "Detection Properties count",
    //                         pltfoldername, "xpos", reportpath, radarpos);*/
    //                }


    //            }

    //        }

    //    }
    //}
   
 



    
}






 
void FLR4P::DataPrep_SlicedTracker_plot(Tracker_Plot_Parameter_T* ptrackerplot,const char* f_name, const char* pltfoldername, std::string reportpath,int radarpos)
{
    std::size_t half_size = 0;
    std::size_t remainder;
    double start_scanindex;
    std::size_t current_size;
    double end_scanindex;
    std::size_t  intialsize = 1;


    if (ptrackerplot->vcs_si_veh_ref_scale.size()!=0 || ptrackerplot->vcs_si_resim_ref_scale.size() !=0)
    {
        if (ptrackerplot->vcs_si_veh_ref_scale.size() >= ptrackerplot->vcs_si_resim_ref_scale.size())
        {
            half_size = ptrackerplot->vcs_si_resim_ref_scale.size() / 10;
            remainder = ptrackerplot->vcs_si_resim_ref_scale.size() % 10;
        }
        else if (ptrackerplot->vcs_si_resim_ref_scale.size() >= ptrackerplot->vcs_si_veh_ref_scale.size())
        {
            half_size = ptrackerplot->vcs_si_veh_ref_scale.size() / 10;
            remainder = ptrackerplot->vcs_si_veh_ref_scale.size() % 10;
        }
    }



    for (int i = 0; i < 10; i++)
    {
        vector<double> file1_xdata;
        vector<double> file1_ydata;
        vector<double> file2_xdata;
        vector<double> file2_ydata;
        file1_xdata.clear();
        file1_ydata.clear();
        file2_xdata.clear();
        file2_ydata.clear();


        vector<double> file1_xdata_ypos_scanindex;
        vector<double> file1_ydata_ypos_pos;
        vector<double> file2_xdata_ypos_scanindex;
        vector<double> file2_ydata_ypos_pos;
        file1_xdata_ypos_scanindex.clear();
        file1_ydata_ypos_pos.clear();
        file2_xdata_ypos_scanindex.clear();
        file2_ydata_ypos_pos.clear();



        vector<double> file1_xdata_xvel_scanindex;
        vector<double> file1_ydata_xvel_vel;
        vector<double> file2_xdata_xvel_scanindex;
        vector<double> file2_ydata_xvel_vel;
        file1_xdata_xvel_scanindex.clear();
        file1_ydata_xvel_vel.clear();
        file2_xdata_xvel_scanindex.clear();
        file2_ydata_xvel_vel.clear();

        vector<double> file1_xdata_yvel_scanindex;
        vector<double> file1_ydata_yvel_vel;
        vector<double> file2_xdata_yvel_scanindex;
        vector<double> file2_ydata_yvel_vel;
        file1_xdata_yvel_scanindex.clear();
        file1_ydata_yvel_vel.clear();
        file2_xdata_yvel_scanindex.clear();
        file2_ydata_yvel_vel.clear();



        vector<double> file1_xdata_xacc_scanindex;
        vector<double> file1_ydata_xacc_acc;
        vector<double> file2_xdata_xacc_scanindex;
        vector<double> file2_ydata_xacc_acc;
        file1_xdata_xacc_scanindex.clear();
        file1_ydata_xacc_acc.clear();
        file2_xdata_xacc_scanindex.clear();
        file2_ydata_xacc_acc.clear();

        vector<double> file1_xdata_yacc_scanindex;
        vector<double> file1_ydata_yacc_acc;
        vector<double> file2_xdata_yacc_scanindex;
        vector<double> file2_ydata_yacc_acc;
        file1_xdata_yacc_scanindex.clear();
        file1_ydata_yacc_acc.clear();
        file2_xdata_yacc_scanindex.clear();
        file2_ydata_yacc_acc.clear();


        if (ptrackerplot->vcs_si_veh_ref_scale.size()!=0 || ptrackerplot->vcs_si_resim_ref_scale.size()!=0)
        {


            if (ptrackerplot->vcs_si_veh_ref_scale.size() >= ptrackerplot->vcs_si_resim_ref_scale.size())
            {
                start_scanindex = ptrackerplot->vcs_si_resim_ref_scale.at(intialsize - 1);
                current_size = intialsize + half_size;
                end_scanindex = ptrackerplot->vcs_si_resim_ref_scale.at(current_size - 1);
            }
            else if (ptrackerplot->vcs_si_resim_ref_scale.size() >= ptrackerplot->vcs_si_veh_ref_scale.size())
            {
                start_scanindex = ptrackerplot->vcs_si_veh_ref_scale.at(intialsize - 1);
                current_size = intialsize + half_size;
                end_scanindex = ptrackerplot->vcs_si_veh_ref_scale.at(current_size - 1);
            }
        }




        logfile << "\n---------F1-########--------------------\n ";

        if (ptrackerplot->map_xpos[0].size() != 0)
        {
            for (auto val : ptrackerplot->map_xpos[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    logfile << " \n@ SI  " << val.first << endl;
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        file1_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        file1_ydata.push_back(val);
                    }
                    logfile << "\n----------------------------";
                }
                else
                {
                    //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
                }
            }
        }


        logfile1 << "\n---------F2---########------------------\n ";
        if (ptrackerplot->map_xpos[1].size() != 0)
        {
            for (auto val : ptrackerplot->map_xpos[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    logfile1 << "\n @ SI  " << val.first << endl;
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        file2_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        file2_ydata.push_back(val);
                    }
                    logfile1 << "\n----------------------------";

                }
                else
                {
                    //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
                }
            }
        }

        if (file1_xdata.size() != 0 && file1_ydata.size() != 0)
        {


            string plottitle;
            std::map<int, string> radar_xpositionmap;
            radar_xpositionmap[0] = "RL XPosition";
            radar_xpositionmap[1] = "RR XPosition";
            radar_xpositionmap[2] = "FR XPosition";
            radar_xpositionmap[3] = "FL XPosition";
            radar_xpositionmap[5] = "FC XPosition";

            for (auto val : radar_xpositionmap)
            {
                if (val.first == radarpos)
                {
                    plottitle = val.second;
                }
            }

            GenerateScatterPlot(file1_xdata, file1_ydata,
                                file2_xdata, file2_ydata, i,
                                f_name, plottitle.c_str(),
                                pltfoldername, "xpos", reportpath, radarpos);

            file1_xdata.clear();
            file1_ydata.clear();
            file2_xdata.clear();
            file2_ydata.clear();

        }

        if (ptrackerplot->map_ypos[0].size()!=0)
        {
            for (auto val : ptrackerplot->map_ypos[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        file1_xdata_ypos_scanindex.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        file1_ydata_ypos_pos.push_back(val);
                    }
                }
                else
                {
                    //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
                }
            }
        }

        if (ptrackerplot->map_ypos[1].size() !=0)
        {
            for (auto val : ptrackerplot->map_ypos[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        file2_xdata_ypos_scanindex.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        file2_ydata_ypos_pos.push_back(val);

                    }

                }
                else
                {
                    //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
                }
            }
        }


        if (file1_xdata_ypos_scanindex.size() != 0 && file1_ydata_ypos_pos.size() != 0)
        {

            string plottitle;
            std::map<int, string> radar_xpositionmap;
            radar_xpositionmap[0] = "RL YPosition";
            radar_xpositionmap[1] = "RR YPosition";
            radar_xpositionmap[2] = "FR YPosition";
            radar_xpositionmap[3] = "FL YPosition";
            radar_xpositionmap[5] = "FC YPosition";

            for (auto val : radar_xpositionmap)
            {
                if (val.first == radarpos)
                {
                    plottitle = val.second;
                }
            }


            GenerateScatterPlot(file1_xdata_ypos_scanindex, file1_ydata_ypos_pos,
                                file2_xdata_ypos_scanindex, file2_ydata_ypos_pos, i,
                                f_name, plottitle.c_str(),
                                pltfoldername, "ypos", reportpath, radarpos);


            file1_xdata_ypos_scanindex.clear();
            file1_ydata_ypos_pos.clear();
            file2_xdata_ypos_scanindex.clear();
            file2_ydata_ypos_pos.clear();


        }



        //for (auto val : pTrackerPlotParamFC_FLR4P->map_xvel[0])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file1_xdata_xvel_scanindex.push_back(val.first);                            
        //            }

        //            for (auto val : val.second)
        //            {
        //                file1_ydata_xvel_vel.push_back(val);
        //               
        //            }                   
        //    }
        //    else
        //    {
        //        //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
        //    }
        //}

        //for (auto val : pTrackerPlotParamFC_FLR4P->map_xvel[1])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file2_xdata_xvel_scanindex.push_back(val.first);
        //            }
        //            for (auto val : val.second)
        //            {
        //                file2_ydata_xvel_vel.push_back(val);                          
        //            }                      
        //    }

        //    else
        //    {
        //        //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
        //    }
        //}


        //if (file1_xdata_xvel_scanindex.size() != 0 && file1_ydata_xvel_vel.size() != 0)
        //{
        //    GenerateScatterPlot(file1_xdata_xvel_scanindex,file1_ydata_xvel_vel,
        //        file2_xdata_xvel_scanindex,file2_ydata_xvel_vel, i,
        //        inputVehFileNameBMW, "FC VCS XVELOCITY", pltfoldername, "xvel", reportpath, 0);


        //}


        //for (auto val : pTrackerPlotParamFC_FLR4P->map_yvel[0])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {                     
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file1_xdata_yvel_scanindex.push_back(val.first);                            
        //            }

        //            for (auto val : val.second)
        //            {
        //                file1_ydata_yvel_vel.push_back(val);                              
        //            }
        //    }
        //    else
        //    {
        //        //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
        //    }
        //}


        //for (auto val : pTrackerPlotParamFC_FLR4P->map_yvel[1])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file2_xdata_yvel_scanindex.push_back(val.first);
        //                
        //            }
        //            for (auto val : val.second)
        //            {
        //                file2_ydata_yvel_vel.push_back(val);
        //               
        //            }                        

        //    }

        //    else
        //    {
        //        //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
        //    }
        //}


        //if (file1_xdata_yvel_scanindex.size() != 0 && file1_ydata_yvel_vel.size() != 0)
        //{
        //    GenerateScatterPlot(file1_xdata_yvel_scanindex,
        //        file1_ydata_yvel_vel,
        //        file2_xdata_yvel_scanindex,
        //        file2_ydata_yvel_vel, i,
        //        inputVehFileNameBMW, "FC VCS YVELOCITY", pltfoldername, "yvel", reportpath, 0);


        //}



        //for (auto val : pTrackerPlotParamFC_FLR4P->map_xacc[0])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file1_xdata_xacc_scanindex.push_back(val.first);
        //            
        //            }

        //            for (auto val : val.second)
        //            {
        //                file1_ydata_xacc_acc.push_back(val);
        //               
        //            }
        //           
        //    }
        //    else
        //    {
        //        //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
        //    }
        //}


        //for (auto val : pTrackerPlotParamFC_FLR4P->map_xacc[1])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        //        
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file2_xdata_xacc_scanindex.push_back(val.first);
        //               
        //            }
        //            for (auto val : val.second)
        //            {
        //                file2_ydata_xacc_acc.push_back(val);
        //               
        //            }                    

        //    }

        //    else
        //    {
        //        //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
        //    }
        //}


        //if (file1_xdata_xacc_scanindex.size() != 0 && file1_ydata_xacc_acc.size() != 0)
        //{
        //    GenerateScatterPlot(file1_xdata_xacc_scanindex,
        //        file1_ydata_xacc_acc,
        //        file2_xdata_xacc_scanindex,
        //        file2_ydata_xacc_acc, i,
        //        inputVehFileNameBMW, "FC VCS XACCELERATION", pltfoldername, "xacc", reportpath, 0);


        //}


        //for (auto val : pTrackerPlotParamFC_FLR4P->map_yacc[0])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {
        // 
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file1_xdata_yacc_scanindex.push_back(val.first);                             
        //            }
        //            for (auto val : val.second)
        //            {
        //                file1_ydata_yacc_acc.push_back(val);                               
        //            }
        //         
        //    }
        //    else
        //    {
        //        //cout << "\n {f1 check }Mising scan index in file 2 " << val.first;
        //    }
        //}


        //for (auto val : pTrackerPlotParamFC_FLR4P->map_yacc[1])
        //{
        //    if ((val.first >= start_scanindex && val.first <= end_scanindex))
        //    {                
        //            for (int i = 0; i < val.second.size(); i++)
        //            {
        //                file2_xdata_yacc_scanindex.push_back(val.first);
        //               
        //            }
        //            for (auto val : val.second)
        //            {
        //                file2_ydata_yacc_acc.push_back(val);
        //                
        //            }                     
        //    }

        //    else
        //    {
        //        //cout << "\n {f2} check Mising scan index in file 1 " << val.first;
        //    }
        //}



        //if (file1_xdata_yacc_scanindex.size() != 0 && file1_ydata_yacc_acc.size() != 0)
        //{
        //    GenerateScatterPlot(file1_xdata_yacc_scanindex,
        //        file1_ydata_yacc_acc,
        //        file2_xdata_yacc_scanindex,
        //        file2_ydata_yacc_acc, i,
        //        inputVehFileNameBMW, "FC VCS YACCELERATION", pltfoldername, "yacc", reportpath, 0);


        //}






        intialsize = current_size - 1;





    }
}

void FLR4P::DataPreparation_Slicing_Detection_Scatterplot(const char* f_name, const char* foldname, std::string reportpath,int radar_postion)
{

    //cout << "\nDataPreparation_Slicing_Detection_Scatterplot radarpos " << radarpos;
    std::size_t half_size = 0;
    std::size_t remainder;
    std::size_t current_size;
    std::size_t  intialsize = 1;
    double start_scanindex;
    double end_scanindex;


    std::vector<double> f1_xdata;
    std::vector<double> f1_ydata;
    std::vector<double> f2_xdata;
    std::vector<double> f2_ydata;

    f1_xdata.clear();
    f1_ydata.clear();
    f2_xdata.clear();
    f2_ydata.clear();

    if ((pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() != 0 || (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() != 0)
    {
        if ((pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() >= (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size())
        {
            half_size = (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() / 10;
            remainder = (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() % 10;
        }
        else if ((pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() >= (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size())
        {
            

            half_size = (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() / 10;
            remainder = (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() % 10;
        }
    }


    //cout << "\nstart before scaling";

    for (int i = 0; i < 10; i++)
    {

        if ((pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() != 0 || (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() != 0)
        {

            if ((pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size() >= (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size())
            {
                //cout << "\nveh > resim";
                //cout<<"si_resim_ref_scale @0 "<< (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.at(0);

                start_scanindex = (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.at(intialsize - 1);
                current_size = intialsize + half_size;
                end_scanindex = (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.at(current_size - 1);

                //cout << "\nstart_scanindex " << start_scanindex;
                //cout << "\nend_scanindex " << end_scanindex;
            }
            else if ((pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.size() >= (pSelfDetectionPlotParameterFC+radar_postion)->si_veh_ref_scale.size())
            {
                //cout << "\nresim > veh";
            


                start_scanindex = (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.at(intialsize - 1);
                current_size = intialsize + half_size;
                end_scanindex = (pSelfDetectionPlotParameterFC+radar_postion)->si_resim_ref_scale.at(current_size - 1);

                //cout << "\nstart_scanindex " << start_scanindex;
                //cout << "\nend_scanindex " << end_scanindex;
            }

           
        }



      
      


        //cout << "\nstart scaling map_range[0]";
        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_range[0].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_range[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        //(pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);  
                        f1_xdata.push_back(val.first);
                    }

                    for (auto val : val.second)
                    {
                        //(pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);  
                        f1_ydata.push_back(val);
                    }
                }

            }
        }
      


        //cout << "\nstart scaling map_range[1]";

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_range[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_range[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        //(pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first); 
                        f2_xdata.push_back(val.first);
                    }

                    for (auto val : val.second)
                    {
                        //(pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);   
                        f2_ydata.push_back(val);
                    }

                }

            }
        }
       

        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {
            string plottitle;
            std::map<int, string> radar_rangemap;
            radar_rangemap[0] = "RL Range";
            radar_rangemap[1] = "RR Range";
            radar_rangemap[2] = "FR Range";
            radar_rangemap[3] = "FL Range";
            radar_rangemap[5] = "FC Range";

            for (auto val: radar_rangemap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }
            
           
            GenerateScatterPlot(f1_xdata, f1_ydata,
                                f2_xdata, f2_ydata, i,
                                f_name, plottitle.c_str(),
                                foldname, "range", reportpath, radar_postion);



            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();

        /*    GeneratePlots_Sliced(i,
                f_name, "Range",
                foldname, "range", reportpath, 0);*/

        }

      /*  (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[0].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);
                    }
                }

            }
        }
        
        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_rangerate[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);

                    }

                }

            }
        }

   


        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {
            
            string plottitle;
            std::map<int, string> radar_rangeratemap;
            radar_rangeratemap[0] = "RL RangeRate";
            radar_rangeratemap[1] = "RR RangeRate";
            radar_rangeratemap[2] = "FR RangeRate";
            radar_rangeratemap[3] = "FL RangeRate";
            radar_rangeratemap[5] = "FC RangeRate";

            for (auto val : radar_rangeratemap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }
            
            
            GenerateScatterPlot(f1_xdata, f1_ydata,
                                f2_xdata, f2_ydata, i,
                                f_name, plottitle.c_str(),
                                foldname, "range rate", reportpath, radar_postion);


            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();

                /*GeneratePlots_Sliced(i,
                    f_name, "Range Rate",
                    foldname, "range rate", reportpath, 0);*/

        }

       /* (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[0].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);
                    }

                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);
                    }
                }

            }

        }
       
        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_azimuth[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);
                    }
                }


            }

        }
        


        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {

            string plottitle;
            std::map<int, string> radar_azimuthmap;
            radar_azimuthmap[0] = "RL Azimuth";
            radar_azimuthmap[1] = "RR Azimuth";
            radar_azimuthmap[2] = "FR Azimuth";
            radar_azimuthmap[3] = "FL Azimuth";
            radar_azimuthmap[5] = "FC Azimuth";

            for (auto val : radar_azimuthmap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }


            GenerateScatterPlot(f1_xdata, f1_ydata,
                                f2_xdata, f2_ydata, i,
                                f_name, plottitle.c_str(), foldname, 
                                "azimuth", reportpath, radar_postion);

            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();


                /*  GeneratePlots_Sliced(i,
                      f_name, "Azimuth",
                      foldname, "azimuth", reportpath, 0);*/

        }

     /*   (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[0].size()!=0)
        {

            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);
                    }

                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);
                    }
                }

            }
        }
       

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_elevation[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);

                    }

                }


            }
        }
        


        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {

            string plottitle;
            std::map<int, string> radar_elevationmap;
            radar_elevationmap[0] = "RL Elevation";
            radar_elevationmap[1] = "RR Elevation";
            radar_elevationmap[2] = "FR Elevation";
            radar_elevationmap[3] = "FL Elevation";
            radar_elevationmap[5] = "FC Elevation";


            for (auto val : radar_elevationmap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }

            //cout << "Elevation Scatter plot\n";

            GenerateScatterPlot(f1_xdata,f1_ydata,
                                f2_xdata,f2_ydata, i,
                                f_name, plottitle.c_str(), foldname, 
                                "elevation", reportpath, radar_postion);


            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();

                /*GeneratePlots_Sliced(i,
                    f_name, "elevation",
                    foldname, "elevation", reportpath, 0);*/

        }

       /* (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[0].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);

                    }

                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);

                    }

                }

            }
        }
     

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_rcs[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {

                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);
                    }

                }

                else
                {
                    //cout << "\n {f2} check mising scan index in file 1 " << val.first;
                }
            }
        }
        


        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {
            string plottitle;
            std::map<int, string> radar_rcsmap;
            radar_rcsmap[0] = "RL RCS";
            radar_rcsmap[1] = "RR RCS";
            radar_rcsmap[2] = "FR RCS";
            radar_rcsmap[3] = "FL RCS";
            radar_rcsmap[5] = "FC RCS";


            for (auto val : radar_rcsmap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }

             GenerateScatterPlot(f1_xdata,
                 f1_ydata,
                 f2_xdata,
                 f2_ydata, i,
                 f_name, plottitle.c_str()
                 , foldname, "rcs", reportpath, radar_postion);


             f1_xdata.clear();
             f1_ydata.clear();
             f2_xdata.clear();
             f2_ydata.clear();

                 /* GeneratePlots_Sliced(i,
                      f_name, "RCS",
                      foldname, "rcs", reportpath, 0);*/

        }

   /*     (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_snr[0].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_snr[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {

                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);
                    }

                }

            }
        }
        

        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_snr[1].size()!=0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_snr[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);

                    }
                }


            }
        }
        



        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {

            string plottitle;
            std::map<int, string> radar_snrmap;
            radar_snrmap[0] = "RL SNR";
            radar_snrmap[1] = "RR SNR";
            radar_snrmap[2] = "FR SNR";
            radar_snrmap[3] = "FL SNR";
            radar_snrmap[5] = "FC SNR";


            for (auto val : radar_snrmap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }
            GenerateScatterPlot(f1_xdata,
                 f1_ydata,
                 f2_xdata,
                 f2_ydata, i,
                 f_name, plottitle.c_str(), foldname, "snr", reportpath, radar_postion);


            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();

                 /* GeneratePlots_Sliced(i,
                      f_name, "SNR",
                      foldname, "snr", reportpath, 0);*/


        }
        /*(pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.clear();
        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.clear();*/


        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_height[0].size() != 0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_height[0])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {

                    for (int i = 0; i < val.second.size(); i++) 
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_xdata_sliced.push_back(val.first);
                        f1_xdata.push_back(val.first);
                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f1_ydata_sliced.push_back(val);
                        f1_ydata.push_back(val);
                    }

                }

            }
        }


        if ((pSelfDetectionPlotParameterFC+radar_postion)->map_height[1].size() != 0)
        {
            for (auto val : (pSelfDetectionPlotParameterFC+radar_postion)->map_height[1])
            {
                if ((val.first >= start_scanindex && val.first <= end_scanindex))
                {
                    for (int i = 0; i < val.second.size(); i++)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_xdata_sliced.push_back(val.first);
                        f2_xdata.push_back(val.first);

                    }
                    for (auto val : val.second)
                    {
                        (pSelfDetectionPlotParameterFC+radar_postion)->f2_ydata_sliced.push_back(val);
                        f2_ydata.push_back(val);

                    }
                }


            }
        }




        if (f1_xdata.size() != 0 && f1_ydata.size() != 0)
        {

            string plottitle;
            std::map<int, string> radar_snrmap;
            radar_snrmap[0] = "RL HEIGHT";
            radar_snrmap[1] = "RR HEIGHT";
            radar_snrmap[2] = "FR HEIGHT";
            radar_snrmap[3] = "FL HEIGHT";
            radar_snrmap[5] = "FC HEIGHT";


            for (auto val : radar_snrmap)
            {
                if (val.first == radar_postion)
                {
                    plottitle = val.second;
                }
            }
            GenerateScatterPlot(f1_xdata,
                f1_ydata,
                f2_xdata,
                f2_ydata, i,
                f_name, plottitle.c_str(), foldname, "height", reportpath, radar_postion);


            f1_xdata.clear();
            f1_ydata.clear();
            f2_xdata.clear();
            f2_ydata.clear();

            /* GeneratePlots_Sliced(i,
                 f_name, "SNR",
                 foldname, "snr", reportpath, 0);*/


        }









        intialsize = current_size - 1;





    }
}















//End of Gen5PlatformDataCollector Functions















