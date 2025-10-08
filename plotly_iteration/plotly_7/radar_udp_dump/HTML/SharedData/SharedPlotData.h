
#ifndef SHAREDPLOT_DATA_H
#define SHAREDPLOT_DATA_H

#define NUMBER_OF_FILES (2)
#define FILE_COUNT (2)

#include<vector>
#include<iostream>
#include<map>
#include "matplot/matplot.h"
#include "../helper/genutil.h"
#include "../../Common/IRadarStream.h"


#define RL 0
#define RR 1
#define FR 2
#define FL 3


class PlotParameter
{
public:
	PlotParameter() { //std::cout << "\n PlotParameter";
	}
	~PlotParameter() { //std::cout << "\n ~PlotParameter";
	}

	std::vector<double> yaw_rate[NUMBER_OF_FILES];
	std::vector<double> abs_speed[NUMBER_OF_FILES];
	std::vector<double>	steering_angle[NUMBER_OF_FILES];
	std::vector<double> scanidex[NUMBER_OF_FILES];

	std::map<double, std::vector<double>> map_c2timing_info[NUMBER_OF_FILES];
	std::vector<double> c2timinginfo_index23[NUMBER_OF_FILES];
	std::vector<double> scanindex[NUMBER_OF_FILES];

	std::vector<double> si_veh;
	std::vector<double> si_resim;
	std::vector<double> si_veh_ref_scale;

	std::vector<double> si_resim_ref_scale;

	std::vector<double> scanindex_maxrange[NUMBER_OF_FILES];
	std::map<double, vector<double>>map_mnr[NUMBER_OF_FILES];


	std::vector<double> valid_detection_count[NUMBER_OF_FILES];
	//std::vector<double> valid_detection_differance[];
	//std::vector<double> valid_detection_differance_scanindex;
	std::vector<double> range[NUMBER_OF_FILES];
	std::vector<double> range_rate[NUMBER_OF_FILES];
	std::vector<double> vel[NUMBER_OF_FILES];
	std::vector<double> azimuth[NUMBER_OF_FILES];
	std::vector<double> elevation[NUMBER_OF_FILES];
	std::vector<double> rcs[NUMBER_OF_FILES];
	std::vector<double> amp[NUMBER_OF_FILES];
	std::vector<double> snr[NUMBER_OF_FILES];
	std::vector<double> height[NUMBER_OF_FILES];
	std::vector<double> bi_static_count[NUMBER_OF_FILES];
	std::vector<double> target_count[NUMBER_OF_FILES];
	std::vector<double> super_res_target[NUMBER_OF_FILES];
	std::vector<double> super_res_target_type[NUMBER_OF_FILES];
	std::vector<double> isBistatic[NUMBER_OF_FILES];
	std::vector<double> el_conf[NUMBER_OF_FILES];
	std::vector<double> az_conf[NUMBER_OF_FILES];
	std::vector<double> bf_type_az[NUMBER_OF_FILES];
	std::vector<double> bf_type_el[NUMBER_OF_FILES];
	std::vector<double> range_max[NUMBER_OF_FILES];
	std::vector<double> isSingleTarget[NUMBER_OF_FILES];
	std::vector<double> bistatic_scanindex[NUMBER_OF_FILES];
	std::vector<double> superrestarget_scanindex[NUMBER_OF_FILES];
	std::vector<double> issingletarget_scanindex[NUMBER_OF_FILES];



	std::vector<double> range_scaleplot[NUMBER_OF_FILES];//splot:scale/sclicing plot
	std::vector<double> range_rate_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> azimuth_scaleplot[NUMBER_OF_FILES];
	std::vector<double> elevation_scaleplot[NUMBER_OF_FILES];
	std::vector<double> rcs_scaleplot[NUMBER_OF_FILES];
	std::vector<double> amp_scaleplot[NUMBER_OF_FILES];
	std::vector<double> snr_scaleplot[NUMBER_OF_FILES];


	std::vector<double> num_fp_detections[NUMBER_OF_FILES];
	std::vector<double> num_sp_detections[NUMBER_OF_FILES];
	std::vector<double> num_af_detections[NUMBER_OF_FILES];
	std::vector<double> rest_count[NUMBER_OF_FILES];
	std::vector<double> af_counters[NUMBER_OF_FILES];
	//std::vector<double> isBistatic[NUMBER_OF_FILES];
	//std::vector<double> isSingleTarget[NUMBER_OF_FILES];
	std::vector<double> isSingleTarget_azimuth[NUMBER_OF_FILES]; 
	//std::vector<double> super_res_target[NUMBER_OF_FILES];

	int cum_isbistatic_count_veh, cum_isbistatic_count_resim;
	int cum_issingletarget_count_veh, cum_issingletarget_count_resim;
	int cum_issuperrestarget_count_veh, cum_issuperrestarget_count_resim;
	
	
	std::vector<double> c0_c2_ipc_err_cntr[NUMBER_OF_FILES];
	std::vector<double> c1_c2_ipc_err_cntr[NUMBER_OF_FILES];
	std::vector<double> Float_Exception_Count_FF_Core[NUMBER_OF_FILES];
	std::vector<double> Float_Exception_Count_RDD_Core[NUMBER_OF_FILES];
	std::vector<double> Float_Exception_Count_Cust_Core[NUMBER_OF_FILES];
	


	std::map<double, double> map_si_valid_detection_count[2];
	std::map<double, std::vector<double>> map_valid_detection_count[2];
	std::map<double, std::vector<double>> map_range[2];
	std::map<double, std::vector<double>> map_rangerate[2];
	std::map<double, std::vector<double>> map_vel[2];
	std::map<double, std::vector<double>> map_azimuth[2];
	std::map<double, std::vector<double>> map_elevation[2];
	std::map<double, std::vector<double>> map_rcs[2];
	std::map<double, std::vector<double>> map_amp[2];
	std::map<double, std::vector<double>> map_snr[2];
	std::map<double, std::vector<double>> map_height[2];

	//Tracker Data
	std::vector<double> vcs_si_veh;
	std::vector<double> vcs_si_resim;
	std::vector<double> vcs_si_veh_ref_scale;
	std::vector<double> vcs_si_resim_ref_scale;
	std::vector<double> vsc_xpos[NUMBER_OF_FILES];
	std::vector<double> vsc_ypos[NUMBER_OF_FILES];
	std::vector<double> vsc_xvel[NUMBER_OF_FILES];
	std::vector<double> vsc_yvel[NUMBER_OF_FILES];
	std::vector<double> vsc_xacc[NUMBER_OF_FILES];
	std::vector<double> vsc_yacc[NUMBER_OF_FILES];
	std::vector<double> vsc_xpos_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_ypos_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_xvel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_yvel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_xacc_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_yacc_scaleplot[NUMBER_OF_FILES];
	std::map<double, std::vector<double>> map_xpos[2];
	std::map<double, std::vector<double>> map_ypos[2];
    std::map<double, std::vector<double>> map_xvel[2];
	std::map<double, std::vector<double>> map_yvel[2];
	std::map<double, std::vector<double>> map_xacc[2];
	std::map<double, std::vector<double>> map_yacc[2];

	//Feature Functions Data
	std::vector<double>bsw_left_si[NUMBER_OF_FILES];
	std::vector<double>bsw_right_si[NUMBER_OF_FILES];
	std::vector<double>bsw_alert_left[NUMBER_OF_FILES];
	std::vector<double>bsw_alert_right[NUMBER_OF_FILES];

	std::vector<double> cvw_left_si[NUMBER_OF_FILES];
	std::vector<double> cvw_right_si[NUMBER_OF_FILES];
	std::vector<double>cvw_alert_left[NUMBER_OF_FILES];
	std::vector<double>cvw_alert_right[NUMBER_OF_FILES];

	std::vector<double>cta_left_si[NUMBER_OF_FILES];
	std::vector<double>cta_right_si[NUMBER_OF_FILES];
	std::vector<double>cta_alert_left[NUMBER_OF_FILES];
	std::vector<double>cta_alert_right[NUMBER_OF_FILES];

	std::vector<double> ced_left_si[NUMBER_OF_FILES];
	std::vector<double> ced_right_si[NUMBER_OF_FILES];
	std::vector<double>ced_alert_left[NUMBER_OF_FILES];
	std::vector<double>ced_alert_right[NUMBER_OF_FILES];


	std::vector<double> vec_align_angle_az[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_el[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_az_initial[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_el_initial[NUMBER_OF_FILES];
	std::vector<double> vec_n_updates_azimuth[NUMBER_OF_FILES];
	std::vector<double> vec_n_updates_elevation[NUMBER_OF_FILES];

	
	std::vector<double> vec_misalign_angle_az[NUMBER_OF_FILES];
	std::vector<double> vec_misalign_angle_el[NUMBER_OF_FILES];

	std::vector<double> vec_align_quality_factor_az[NUMBER_OF_FILES];
	std::vector<double> vec_align_quality_factor_el[NUMBER_OF_FILES];
	std::vector<double> vec_align_numpts_el[NUMBER_OF_FILES];
	std::vector<double> vec_align_numpts_az[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_raw_az[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_ref_az[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_raw_el[NUMBER_OF_FILES];
	std::vector<double> vec_align_angle_ref_el[NUMBER_OF_FILES];
	std::vector<double> alignment_scanidex[NUMBER_OF_FILES];

	// Added vectors for other structure fields
	std::vector<double> vec_opp_align_quality_factor_el[NUMBER_OF_FILES];


	std::vector<double> vec_AvgRm_mnr_avg[NUMBER_OF_FILES];
	std::vector<double> vec_MaxRm_mnr_avg[NUMBER_OF_FILES];

	std::vector<double>blockage_status[NUMBER_OF_FILES];
	std::vector<double>blockage_active[NUMBER_OF_FILES];
	std::vector<double>blockage_thres[NUMBER_OF_FILES];

	std::vector<double> vec_range_check_blockage[NUMBER_OF_FILES];


	std::vector<double> vec_AvgPm_mnr_avg[NUMBER_OF_FILES];
	std::vector<double> vec_MaxPm_mnr_avg[NUMBER_OF_FILES];

	std::vector<double> vec_Degraded[NUMBER_OF_FILES];
	std::vector<double> vec_Blocked[NUMBER_OF_FILES];
	std::vector<double> mnr_scanidex[NUMBER_OF_FILES];

	std::vector<double>interference_detected[NUMBER_OF_FILES];
	std::vector<double> interference_scanidex[NUMBER_OF_FILES];

	std::vector<double> vec_vcs_xpos[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_ypos[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_xvel[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_yvel[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_xacc[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_yacc[NUMBER_OF_FILES];
	std::vector<double> vec_vcs_heading[NUMBER_OF_FILES];

	std::vector<double> vcs_xpos_scaleplot[NUMBER_OF_FILES];//splot:scale/sclicing plot
	std::vector<double> vcs_ypos_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vcs_xvel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vcs_yvel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vcs_xacc_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vcs_yacc_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vcs_heading_scaleplot[NUMBER_OF_FILES];
	
	std::map<double, std::vector<double>> map_vcs_xpos[2];
	std::map<double, std::vector<double>> map_vcs_ypos[2];
	std::map<double, std::vector<double>> map_vcs_xvel[2];
	std::map<double, std::vector<double>> map_vcs_yvel[2];
	std::map<double, std::vector<double>> map_vcs_xacc[2];
	std::map<double, std::vector<double>> map_vcs_yacc[2];
	std::map<double, std::vector<double>> map_vcs_heading[2];

		

};
typedef struct Tool_RunTime_info
{
	std::string Log1_duration, Log2_duration;
	std::string  tool_runtime_sec;
	std::string  tool_runtime_min;
	std::string  tool_runtime_func_inputtime;

}Tool_RunTime_info_T;
typedef struct Vehicle_Plot_Param
{
	Vehicle_Plot_Param() {  }
	~Vehicle_Plot_Param() {  }

	std::vector<double> yaw_rate[NUMBER_OF_FILES];
	std::vector<double> abs_speed[NUMBER_OF_FILES];
	std::vector<double>	steering_angle[NUMBER_OF_FILES];
	std::vector<double> scanidex[NUMBER_OF_FILES];
	
}Vehicle_Plot_Param_T;

typedef struct Timing_Info
{
	std::map<double, std::vector<double>> map_c2timing_info[NUMBER_OF_FILES];
	std::vector<double> c2timinginfo_index23[NUMBER_OF_FILES];
	std::vector<double> scanindex[NUMBER_OF_FILES];

}Timing_Info_T;

typedef struct SelfDetection_Plot_Parameters
{
	std::vector<double> si_veh;
	std::vector<double> si_resim;
	std::vector<double> si_veh_ref_scale;
	std::vector<double> si_resim_ref_scale;

	std::vector<double> scanindex_maxrange[NUMBER_OF_FILES];

	std::vector<double> scanindex[NUMBER_OF_FILES];

	std::vector<double> valid_detection_count[NUMBER_OF_FILES];
	//std::vector<double> valid_detection_differance[];
	//std::vector<double> valid_detection_differance_scanindex;
	std::vector<double> range[NUMBER_OF_FILES];
	std::vector<double> range_rate[NUMBER_OF_FILES];
	std::vector<double> azimuth[NUMBER_OF_FILES];
	std::vector<double> elevation[NUMBER_OF_FILES];
	std::vector<double> rcs[NUMBER_OF_FILES];
	std::vector<double> amp[NUMBER_OF_FILES];
	std::vector<double> snr[NUMBER_OF_FILES];
	std::vector<double> height[NUMBER_OF_FILES];
	std::vector<double> bi_static_count[NUMBER_OF_FILES];
	std::vector<double> target_count[NUMBER_OF_FILES];
	std::vector<double> super_res_target[NUMBER_OF_FILES];
	std::vector<double> super_res_target_type[NUMBER_OF_FILES];
	std::vector<double> isBistatic[NUMBER_OF_FILES];
	std::vector<double> el_conf[NUMBER_OF_FILES];
	std::vector<double> az_conf[NUMBER_OF_FILES];
	std::vector<double> bf_type_az[NUMBER_OF_FILES];
	std::vector<double> bf_type_el[NUMBER_OF_FILES];		
	std::vector<double> range_max[NUMBER_OF_FILES];
	std::vector<double> isSingleTarget[NUMBER_OF_FILES];
	

	std::vector<double> range_scaleplot[NUMBER_OF_FILES];//splot:scale/sclicing plot
	std::vector<double> range_rate_scaleplot[NUMBER_OF_FILES];
	std::vector<double> azimuth_scaleplot[NUMBER_OF_FILES];
	std::vector<double> elevation_scaleplot[NUMBER_OF_FILES];
	std::vector<double> rcs_scaleplot[NUMBER_OF_FILES];
	std::vector<double> amp_scaleplot[NUMBER_OF_FILES];
	std::vector<double> snr_scaleplot[NUMBER_OF_FILES];


	std::map<double, double> map_si_valid_detection_count[2];
	std::map<double, std::vector<double>> map_valid_detection_count[2];
	std::map<double, std::vector<double>> map_range[2];
	std::map<double, std::vector<double>> map_rangerate[2];
	std::map<double, std::vector<double>> map_azimuth[2];
	std::map<double, std::vector<double>> map_elevation[2];
	std::map<double, std::vector<double>> map_rcs[2];
	std::map<double, std::vector<double>> map_amp[2];
	std::map<double, std::vector<double>> map_snr[2];
	std::map<double, std::vector<double>> map_height[2];


}SelfDetection_Plot_Parameters_T;


typedef struct SelfDetection_Plot_Parameter
{

	std::vector<double> si_veh;
	std::vector<double> si_resim;
	std::vector<double> si_veh_ref_scale;
	std::vector<double> si_resim_ref_scale;

	std::vector<double> valid_detection_count[NUMBER_OF_FILES];

	std::vector<double> range[NUMBER_OF_FILES];
	std::vector<float> rangef[NUMBER_OF_FILES];
	std::vector<double> range_rate[NUMBER_OF_FILES];
	std::vector<double> azimuth[NUMBER_OF_FILES];
	std::vector<double> elevation[NUMBER_OF_FILES];
	std::vector<double> rcs[NUMBER_OF_FILES];
	std::vector<double> amp[NUMBER_OF_FILES];
	std::vector<double> snr[NUMBER_OF_FILES];
	std::vector<double> height[NUMBER_OF_FILES];


	std::vector<double> range_scaleplot[NUMBER_OF_FILES];//splot:scale/sclicing plot
	std::vector<double> range_rate_scaleplot[NUMBER_OF_FILES];
	std::vector<double> azimuth_scaleplot[NUMBER_OF_FILES];
	std::vector<double> elevation_scaleplot[NUMBER_OF_FILES];
	std::vector<double> rcs_scaleplot[NUMBER_OF_FILES];
	std::vector<double> amp_scaleplot[NUMBER_OF_FILES];
	std::vector<double> snr_scaleplot[NUMBER_OF_FILES];


	//std::vector<double> az_conf[NUMBER_OF_FILES];
	//std::vector<double> el_conf[NUMBER_OF_FILES];

	std::vector<double> height_scaleplot[NUMBER_OF_FILES];
	std::vector<double> valid_detection_count_scaleplot[NUMBER_OF_FILES];

	std::vector<double> speed[NUMBER_OF_FILES];
	std::vector<double> yawrate[NUMBER_OF_FILES];
	std::vector<double> range_max[NUMBER_OF_FILES];



	std::vector<double> bi_static_count[NUMBER_OF_FILES];
	std::vector<double> target_count[NUMBER_OF_FILES];
	std::vector<double> super_res_target[NUMBER_OF_FILES];
	std::vector<double> isBistatic[NUMBER_OF_FILES];
	std::vector<double> el_conf[NUMBER_OF_FILES];
	std::vector<double> az_conf[NUMBER_OF_FILES];

	


	std::map<double, std::vector<double>> map_range[2];
	std::map<float, std::vector<float>> map_rangef[2];
	std::map<double, std::vector<double>> map_rangerate[2];
	std::map<double, std::vector<double>> map_azimuth[2];
	std::map<double, std::vector<double>> map_elevation[2];
	std::map<double, std::vector<double>> map_rcs[2];
	std::map<double, std::vector<double>> map_amp[2];
	std::map<double, std::vector<double>> map_snr[2];
	std::map<double, std::vector<double>> map_height[2];
	std::map<double, std::vector<double>> map_valid_detection_count[2];

	std::map<double, std::vector<double>> map_c2timing_info[2];



	vector<double> file1_xdata;
	vector<double> file1_ydata;
	vector<double> file2_xdata;
	vector<double> file2_ydata;

	vector<double> file1_xdata_ypos_scanindex;
	vector<double> file1_ydata_ypos_pos;
	vector<double> file2_xdata_ypos_scanindex;
	vector<double> file2_ydata_ypos_pos;

	vector<double> tempdata[NUMBER_OF_FILES];

	std::map<double, int> map_azimuth_out_of_range[2];
	std::map<double, int> map_elevation_out_of_range[2];


	std::vector<double> f1_xdata_sliced;
	std::vector<double> f1_ydata_sliced;

	std::vector<double> f2_xdata_sliced;
	std::vector<double> f2_ydata_sliced;

	std::vector<float> f1_xdata_slicedf;
	std::vector<float> f1_ydata_slicedf;

	std::vector<float> f2_xdata_slicedf;
	std::vector<float> f2_ydata_slicedf;


		
}SelfDetection_Plot_Parameter_T;


typedef struct Tracker_Plot_Parameter
{
	std::vector<double> vcs_si_veh;
	std::vector<double> vcs_si_resim;

	std::vector<double> vcs_si_veh_ref_scale;
	std::vector<double> vcs_si_resim_ref_scale;

	std::vector<double> vsc_xpos[NUMBER_OF_FILES];
	std::vector<double> vsc_ypos[NUMBER_OF_FILES];

	std::vector<double> vsc_xvel[NUMBER_OF_FILES];
	std::vector<double> vsc_yvel[NUMBER_OF_FILES];


	std::vector<double> vsc_xacc[NUMBER_OF_FILES];
	std::vector<double> vsc_yacc[NUMBER_OF_FILES];


	std::vector<double> vsc_xpos_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_ypos_scaleplot[NUMBER_OF_FILES];

	std::vector<double> vsc_xvel_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_yvel_scaleplot[NUMBER_OF_FILES];


	std::vector<double> vsc_xacc_scaleplot[NUMBER_OF_FILES];
	std::vector<double> vsc_yacc_scaleplot[NUMBER_OF_FILES];

	std::map<double, std::vector<double>> map_xpos[2];
	std::map<double, std::vector<double>> map_ypos[2];


	std::map<double, std::vector<double>> map_xvel[2];
	std::map<double, std::vector<double>> map_yvel[2];

	std::map<double, std::vector<double>> map_xacc[2];
	std::map<double, std::vector<double>> map_yacc[2];

}Tracker_Plot_Parameter_T;

typedef struct Tracker_Plot_Param
{
	std::vector<double> vcs_long_posn[NUMBER_OF_FILES];
	std::vector<double> vcs_lat_posn[NUMBER_OF_FILES];
	std::vector<double> vcs_long_vel[NUMBER_OF_FILES];
	std::vector<double> vcs_lat_vel[NUMBER_OF_FILES];
	std::vector<double> vcs_long_acc[NUMBER_OF_FILES];
	std::vector<double> vcs_lat_acc[NUMBER_OF_FILES];
	std::vector<double> vcs_si;
	std::vector<double> vcs_si_veh;
	std::vector<double> vcs_si_resim;
	std::vector<double> valid_tracks[NUMBER_OF_FILES];
	std::vector<double> valid_tracks_input1;
	std::vector<double> valid_tracks_input2;
	std::vector<double> vcs_lat_posn_input;
	std::vector<double> trackwidth[NUMBER_OF_FILES];
	std::vector<double> trackLenght[NUMBER_OF_FILES];

	//FLR4P
	std::vector<double> vsc_xpos[NUMBER_OF_FILES];
	std::vector<double> vsc_ypos[NUMBER_OF_FILES];
	std::vector<unsigned int> vcs_scanindex[NUMBER_OF_FILES];

	std::vector<double> vsc_xvel[NUMBER_OF_FILES];
	std::vector<double> vsc_yvel[NUMBER_OF_FILES];


	std::vector<double> vsc_xacc[NUMBER_OF_FILES];
	std::vector<double> vsc_yacc[NUMBER_OF_FILES];


	//Mature tracks
	std::vector<double> vcs_si_maturetracks;

	std::vector<double> maturetrack_count_perSI_RL[2];
	std::vector<double> maturetrack_count_perSI_RR[2];
	std::vector<double> maturetrack_count_perSI_FL[2];
	std::vector<double> maturetrack_count_perSI_FR[2];

	std::map<double,int> map_maturetrack_count_perSI_RL_veh;
	std::map<double, int> map_maturetrack_count_perSI_RL_resim;


	std::map<double, int> map_maturetrack_count_perSI_RR_veh;
	std::map<double, int> map_maturetrack_count_perSI_RR_resim;
	

}Tracker_Plot_Param_T;


typedef struct Feature_Plot_Param
{
	//CTA
	std::vector<double> f_cta_alert_left[NUMBER_OF_SIDES];
	std::vector<double> f_cta_alert_right[NUMBER_OF_SIDES];
	std::vector<double> f_cta_alert_left_ttc[NUMBER_OF_SIDES];
	std::vector<double> f_cta_alert_right_ttc[NUMBER_OF_SIDES];
	std::vector<double>	cta_si;


	//CED

	std::vector<double> f_ced_alert_left[NUMBER_OF_SIDES];
	std::vector<double> f_ced_alert_right[NUMBER_OF_SIDES];
	std::vector<double> f_ced_alert_left_ttc[NUMBER_OF_SIDES];
	std::vector<double> f_ced_alert_right_ttc[NUMBER_OF_SIDES];
	std::vector<double>	ced_si;


	//RECW
	std::vector<double> recw_alert_left[NUMBER_OF_SIDES];
	std::vector<double> recw_alert_left_ttc[NUMBER_OF_SIDES];
	std::vector<double> recw_alert_right[NUMBER_OF_SIDES];
	std::vector<double> recw_alert_right_ttc[NUMBER_OF_SIDES];
	std::vector<double>	recw_si;

	//LCDA


//	std::vector<double>	cvw_alert_left[NUMBER_OF_SIDES];
	std::vector<double>	cvw_alert_left_ttc[NUMBER_OF_SIDES];
//	std::vector<double>	cvw_alert_right[NUMBER_OF_SIDES];
	std::vector<double>	cvw_alert_right_ttc[NUMBER_OF_SIDES];
	std::vector<double>	cvw_si;


	//std::vector<double>	bsw_alert_left[NUMBER_OF_SIDES];
	//std::vector<double>	bsw_alert_right[NUMBER_OF_SIDES];
	std::vector<double>	bsw_si;

	//HONDA FF Data
	std::vector<double>bsw_left_si[NUMBER_OF_FILES];
	std::vector<double>bsw_right_si[NUMBER_OF_FILES];
	std::vector<double>bsw_alert_left[NUMBER_OF_FILES];
	std::vector<double>bsw_alert_right[NUMBER_OF_FILES];

	std::vector<double> cvw_left_si[NUMBER_OF_FILES];
	std::vector<double> cvw_right_si[NUMBER_OF_FILES];
	std::vector<double>cvw_alert_left[NUMBER_OF_FILES];
	std::vector<double>cvw_alert_right[NUMBER_OF_FILES];

	std::vector<double>cta_left_si[NUMBER_OF_FILES];
	std::vector<double>cta_right_si[NUMBER_OF_FILES];
	std::vector<double>cta_alert_left[NUMBER_OF_FILES];
	std::vector<double>cta_alert_right[NUMBER_OF_FILES];

	std::vector<double> ced_left_si[NUMBER_OF_FILES];
	std::vector<double> ced_right_si[NUMBER_OF_FILES];
	std::vector<double>ced_alert_left[NUMBER_OF_FILES];
	std::vector<double>ced_alert_right[NUMBER_OF_FILES];
	

}Feature_Plot_Param_T;

typedef struct SelfDetections
{

	std::map<double, std::vector<double>> veh_range_detections[4];
	std::map<double, std::vector<double>> resim_range_detections[4];

	std::map<double, std::vector<double>> veh_rangerate_detections[4];
	std::map<double, std::vector<double>> resim_rangerate_detections[4];

	std::map<double, std::vector<double>> veh_azimuth_detections[4];
	std::map<double, std::vector<double>> resim_azimuth_detections[4];

	std::map<double, std::vector<double>> veh_elevation_detections[4];
	std::map<double, std::vector<double>> resim_elevation_detections[4];

	std::map<double, std::vector<double>> veh_snr_detections[4];
	std::map<double, std::vector<double>> resim_snr_detections[4];

	std::map<double, std::vector<double>> veh_amp_detections[4];
	std::map<double, std::vector<double>> resim_amp_detections[4];


	//Ignore
	std::vector<double> vehiclescanindex[4];
	std::vector<double> vehicledetecttioncount[4];
	//Ignore
	std::vector<double> resimscanindex[4];
	std::vector<double> resimdetecttioncount[4];
	

	std::map<double, double> vehmap_SI_detectcount[4];
	std::map<double, double> resimmap_SI_detectcount[4];


	std::map<unsigned int, int> veh_valid_detectcount[4];
	std::map<unsigned int, int> resim_valid_detectcount[4];

	std::vector<double> missed_detect_cnt_scanindex[4];
	std::vector<int> missed_detect_count_value[4];
	std::vector<std::vector<double>> missed_detect_cnt_value[4];

	std::vector<double> additional_detect_cnt_scanindex[4];
	std::vector<int> additional_detect_count_value[4];
	std::vector<std::vector<double>> additional_detect_cnt_value[4];

}Self_Detections_Param_T;
#endif

