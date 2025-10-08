#ifndef MATPLOT_NONSCALE
#define MATPLOT_NONSCALE

#include<matplot/matplot.h>
#include<chrono>
#include <filesystem>

#include "../SharedData/SharedPlotData.h"
#include "../../CommonFiles/inc/ctml.hpp"


using namespace std;
namespace fs = std::filesystem;
using namespace matplot;

#ifdef _WIN32
#include <direct.h>// for _mkdir
#endif
#ifdef __GNUC__
#include <sys/stat.h>
#endif


class MatplotNonScale
{
	MatplotNonScale();
public:
	MatplotNonScale(MatplotNonScale&) = delete;
	MatplotNonScale& operator=(MatplotNonScale&) = delete;
	virtual ~MatplotNonScale();
	static MatplotNonScale& getInstance();


	


	void prepare_data_generate_plot();
	void prepare_c2timinginfo_generate_nonscaled_c2timing_scatterplot();


	int PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt);
	int print_reporttoolrun_timing_info(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt);


	//scatter plots for vehicle information
	void generate_nonscaled_vehinfo_speed_scatterplot(Vehicle_Plot_Param_T *pveh_plot_info,int fileindex,
		                                            const char* f_name,const char* current_radar,const char* foldname,
		                                            std::string plotparam,std::string reportpath, int radarPosition);

	void generate_nonscaled_vehinfo_yawrate_scatterplot(Vehicle_Plot_Param_T* pveh_plot_info, int fileindex,
		                                                const char* f_name, const char* current_radar, const char* foldname,
		                                                std::string plotparam, std::string reportpath,int radarPosition);


	//scatter plots for detection properties
	void generate_nonscaled_detection_range_scatterplot(SelfDetection_Plot_Parameters_T*pselfdetection, int fileindex,
		                                          const char* f_name, const char* current_radar, const char* foldname,
		                                          std::string plotparam, std::string reportpath, int radarPosition);



	void generate_barplot(const std::vector<double>& f1_xvalue,
		                                const std::vector<std::vector<double>>& f1_yvalue,
	                                    int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		                                std::string plotparam, std::string reportpath, int radarPosition);


	void generate_barplot_matchmistach(const std::vector<double>& f1_xvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);


	void generate_boxplot(const std::vector<double>& f1_xvalue,
		                  const std::vector<double>& f1_yvalue,
		                  int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		                  std::string plotparam, std::string reportpath, int radarPosition);


	void generate_pieplot(const std::vector<double> f1_xvalue,
							const std::vector<std::string> f1_yvalue,
							int fileindex, const char* f_name, const char* current_radar, const char* foldname,
							std::string plotparam, std::string reportpath, int radarPosition);

	template<class  type1,class type2,class type3,class type4>
	void generate_detection_scatterplot(const std::vector<type1> &f1_xvalue,
		                                const std::vector<type2> &f1_yvalue,
										const std::vector<type3> &f2_xvalue,
		                                const std::vector<type4> &f2_yvalue,
										int fileindex,const char* f_name, const char* current_radar, const char* foldname,
										std::string plotparam, std::string reportpath, int radarPosition);


	template<class  type1, class type2, class type3, class type4>
	void generate_detectionProperty_scatterplot(const std::vector<type1>& f1_xvalue,
		const std::vector<type2>& f1_yvalue,
		const std::vector<type3>& f2_xvalue,
		const std::vector<type4>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);



	template<class  type1, class type2, class type3, class type4>
	void generate_trackerProperty_scatterplot(const std::vector<type1>& f1_xvalue,
		const std::vector<type2>& f1_yvalue,
		const std::vector<type3>& f2_xvalue,
		const std::vector<type4>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);


	/*template<class  type1, class type2, class type3, class type4>
	void generate_barplot(const std::vector<type1>& f1_xvalue,
		const std::vector<type2>& f1_yvalue,
		const std::vector<type3>& f2_xvalue,
		const std::vector<type4>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);*/


	void generate_nonscaled_detection_maxrange_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
														const char* f_name, const char* current_radar, const char* foldname,
														std::string plotparam, std::string reportpath, int radarPosition);

	
	void generate_nonscaled_detection_rangerate_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection,int fileindex,
													const char* f_name, const char* current_radar, const char* foldname,
													std::string plotparam, std::string reportpath, int radarPosition);

	void generate_nonscaled_detection_azimuth_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
														const char* f_name, const char* current_radar, const char* foldname,
														std::string plotparam, std::string reportpath, int radarPosition);

	void generate_nonscaled_detection_elevation_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
															const char* f_name, const char* current_radar, const char* foldname,
															std::string plotparam, std::string reportpath, int radarPosition);

	void generate_nonscaled_detection_rcs_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
													  const char* f_name, const char* current_radar, const char* foldname,
													  std::string plotparam, std::string reportpath, int radarPosition);

	void generate_nonscaled_detection_amp_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
													  const char* f_name, const char* current_radar, const char* foldname,
													  std::string plotparam, std::string reportpath, int radarPosition);

	void generate_nonscaled_detection_snr_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
													  const char* f_name, const char* current_radar, const char* foldname,
													  std::string plotparam, std::string reportpath, int radarPosition);

	void generate_valid_detectioncount_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
													const char* f_name, const char* current_radar, const char* foldname,
													std::string plotparam, std::string reportpath, int radarPosition);

	void generate_el_conf_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
									const char* f_name, const char* current_radar, const char* foldname,
									std::string plotparam, std::string reportpath, int radarPosition);

	void generate_az_conf_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
									const char* f_name, const char* current_radar, const char* foldname,
									std::string plotparam, std::string reportpath, int radarPosition);


	void generate_isbistatic_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
									const char* f_name, const char* current_radar, const char* foldname,
									std::string plotparam, std::string reportpath, int radarPosition);

	void generate_issingletarget_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
										const char* f_name, const char* current_radar, const char* foldname,
										std::string plotparam, std::string reportpath, int radarPosition);

	void generate_super_res_target_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
										const char* f_name, const char* current_radar, const char* foldname,
										std::string plotparam, std::string reportpath, int radarPosition);

	void generate_super_res_target_type_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
										const char* f_name, const char* current_radar, const char* foldname,
										std::string plotparam, std::string reportpath, int radarPosition);

	void generate_bf_type_el_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
									const char* f_name, const char* current_radar, const char* foldname,
									std::string plotparam, std::string reportpath, int radarPosition);

	void generate_bf_type_az_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
										const char* f_name, const char* current_radar, const char* foldname,
										std::string plotparam, std::string reportpath, int radarPosition);


	//scatter plot for timing/through put info

	void generate_c2timinginfo_scatterplot(Timing_Info_T* ptiminginfo, int fileindex,
		const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);

	//histogram plot for detection properties



	void generate_detection_range_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_histogram(const std::vector<double>&f1_value,const std::vector<double>&f2_value, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_rangerate_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_azimuth_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_elevation_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_rcs_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_amp_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_snr_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_isbistatic_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_issingletarget_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_super_res_target_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_super_res_target_type_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_bf_type_el_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
											const char* f_name, const char* current_radar, const char* foldname,
											std::string plotparam, std::string reportpath, int radarPosition);

	void generate_detection_bf_type_az_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex,
												const char* f_name, const char* current_radar, const char* foldname,
												std::string plotparam, std::string reportpath, int radarPosition);

	//generate bar graph

	void generate_diff_valid_detection_count_bargraph(std::map<double,double> &map_valid_detection_count_file1, 
		                                              std::map<double, double>& map_valid_detection_count_file2,int fileindex,
														const char* f_name, const char* current_radar, const char* foldname,
														std::string plotparam, std::string reportpath, int radarPosition);

	void preparedata_diff_valid_detection_count(std::map<double, double>& map_valid_detection_count_file1,
												std::map<double, double>& map_valid_detection_count_file2
																	);



	std::vector<double> data_prep_vec_valid_det_diff_count;
	std::vector<double> data_prep_vec_valid_det_diff_count_scanindex;
	
//scatter plot for  Tracker  

	void generate_tracker_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
		const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);


	void generate_match_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
		const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);


	void generate_mismmatch_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
		
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);

	//scatter plot for  Feature Function 

	void generate_features_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
		const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue,
		int fileindex, const char* f_name, const char* current_radar, const char* foldname,
		std::string plotparam, std::string reportpath, int radarPosition);

	
	ofstream file1, file2;

};

template<class type1,class type2,class type3,class type4>
void MatplotNonScale::generate_detection_scatterplot(const std::vector<type1>& f1_xvalue,
	                                                 const std::vector<type2>& f1_yvalue,
	                                                 const std::vector<type3>& f2_xvalue,
	                                                 const std::vector<type4>& f2_yvalue,
	                                                 int fileindex, const char* f_name, const char* current_radar, const char* foldname,
	                                                 std::string plotparam, std::string reportpath, int radarPosition)
{


	//std::cout << std::endl << "generate_detection_scatterplot";
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

	if (int(radarPosition) == 0)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
	}
	else if (int(radarPosition) == 1)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
	}
	else if (int(radarPosition) == 2)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
	}
	else if (int(radarPosition) == 3)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
	}
	else if (int(radarPosition) == 5)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
	}

	else if (int(radarPosition) == 19)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_DC.html";
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

		//auto tracks_fig2 = figure(true);
		sgtitle(radar_position, "blue");
		std::vector<std::string> legdetails;
		tiledlayout(1, 1);

		auto ax1 = nexttile();
		grid(false);
		ax1->ylabel(plotparam);
		ax1->xlabel("scan index");

		if (1)
		{

			if (!f1_xvalue.empty() && !f1_yvalue.empty())
			{
				auto pfl11 = scatter(f1_xvalue, f1_yvalue);
				pfl11->color("blue");
				pfl11->fill(true);
				pfl11->marker_color("b");
				pfl11->marker_size(1.5);
				pfl11->marker("O");
				pfl11->marker_face(true);
				legdetails.push_back("file1");
				hold(true);
			}

			if (!f2_xvalue.empty() && !f2_yvalue.empty())
			{
				auto pfl2 = scatter(f2_xvalue, f2_yvalue);
				pfl2->color("red");
				pfl2->fill(true);
				pfl2->marker_color("r");
				pfl2->marker_size(1.5);
				pfl2->marker("O");
				pfl2->marker_face(true);
				legdetails.push_back("file2");
			}

			ax1->legend(legdetails);
			radar_position.append(std::to_string(radarPosition));


			htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + ".png";
			save((char*)htmlreport_image.c_str(), "png");

			htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + ".png";


			//std::cout << std::endl << "htmlreport_image "<< htmlreport_image;
			//std::cout << std::endl << "htmlreport_imagepath " << htmlreport_imagepath;
			//htmlreport_folder
			//std::cout << std::endl << "htmlreport_folder " << htmlreport_folder;

			/*<!--Tab links-->
				<div class = "tab">
				<button class = "tablinks" onclick = "openCity(event, 'London')">London< / button>
				<button class = "tablinks" onclick = "openCity(event, 'Paris')">Paris< / button>
				<button class = "tablinks" onclick = "openCity(event, 'Tokyo')">Tokyo< / button>
				< / div>

				<!--Tab content-->
				<div id = "London" class = "tabcontent">
				<h3>London< / h3>
				<p>London is the capital city of England.< / p>
				< / div>

				<div id = "Paris" class = "tabcontent">
				<h3>Paris< / h3>
				<p>Paris is the capital of France.< / p>
				< / div>*/

			/*<img src = "img_chania.jpg" alt = "Flowers in Chania">*/


			CTML::Node image_trks("img");
			image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
				                  .UseClosingTag(false);


			document.AppendNodeToBody(image_trks);
		}

	}
	log_report << document.ToString() << endl;

	if (!log_report.is_open())
		log_report.close();




}



template<class type1, class type2, class type3, class type4>
void MatplotNonScale::generate_detectionProperty_scatterplot(const std::vector<type1>& f1_xvalue,
	const std::vector<type2>& f1_yvalue,
	const std::vector<type3>& f2_xvalue,
	const std::vector<type4>& f2_yvalue,
	int fileindex, const char* f_name, const char* current_radar, const char* foldname,
	std::string plotparam, std::string reportpath, int radarPosition)
{


	//std::cout << std::endl << "generate_detectionProperty_scatterplot";
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

	if (int(radarPosition) == 0)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
	}
	else if (int(radarPosition) == 1)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
	}
	else if (int(radarPosition) == 2)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
	}
	else if (int(radarPosition) == 3)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
	}
	else if (int(radarPosition) == 5)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
	}

	else if (int(radarPosition) == 19)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_DC.html";
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

		//auto tracks_fig2 = figure(true);
		sgtitle(radar_position, "blue");
		std::vector<std::string> legdetails;
		tiledlayout(1, 1);

		auto ax1 = nexttile();
		grid(false);
		ax1->ylabel(plotparam);
		ax1->xlabel("scan index");

		if (1)
		{

			if (!f1_xvalue.empty() && !f1_yvalue.empty())
			{
				auto pfl11 = scatter(f1_xvalue, f1_yvalue);
				pfl11->color("blue");
				pfl11->fill(true);
				pfl11->marker_color("b");
				pfl11->marker_size(1.5);
				pfl11->marker("O");
				pfl11->marker_face(true);
				legdetails.push_back("file1");
				hold(true);
			}

			if (!f2_xvalue.empty() && !f2_yvalue.empty())
			{
				auto pfl2 = scatter(f2_xvalue, f2_yvalue);
				pfl2->color("red");
				pfl2->fill(true);
				pfl2->marker_color("r");
				pfl2->marker_size(1.5);
				pfl2->marker("O");
				pfl2->marker_face(true);
				legdetails.push_back("file2");
			}

			ax1->legend(legdetails);
			radar_position.append(std::to_string(radarPosition));


			htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + ".png";
			save((char*)htmlreport_image.c_str(), "png");

			htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + ".png";


			//std::cout << std::endl << "htmlreport_image " << htmlreport_image;
			//std::cout << std::endl << "htmlreport_imagepath " << htmlreport_imagepath;
			//htmlreport_folder
			//std::cout << std::endl << "htmlreport_folder " << htmlreport_folder;

			


			CTML::Node image_trks("img");
			image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
				.UseClosingTag(false);


			document.AppendNodeToBody(image_trks);
		}

	}
	log_report << document.ToString() << endl;

	if (!log_report.is_open())
		log_report.close();




}



template<class type1, class type2, class type3, class type4>
void MatplotNonScale::generate_trackerProperty_scatterplot(const std::vector<type1>& f1_xvalue,
	const std::vector<type2>& f1_yvalue,
	const std::vector<type3>& f2_xvalue,
	const std::vector<type4>& f2_yvalue,
	int fileindex, const char* f_name, const char* current_radar, const char* foldname,
	std::string plotparam, std::string reportpath, int radarPosition)
{


	//std::cout << std::endl << "generate_detectionProperty_scatterplot";
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

	if (int(radarPosition) == 0)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
	}
	else if (int(radarPosition) == 1)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
	}
	else if (int(radarPosition) == 2)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
	}
	else if (int(radarPosition) == 3)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
	}
	else if (int(radarPosition) == 5)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
	}

	else if (int(radarPosition) == 19)
	{
		htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_DC.html";
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

		//auto tracks_fig2 = figure(true);
		sgtitle(radar_position, "blue");
		std::vector<std::string> legdetails;
		tiledlayout(1, 1);

		auto ax1 = nexttile();
		grid(false);
		ax1->ylabel(plotparam);
		ax1->xlabel("scan index");

		if (1)
		{

			if (!f1_xvalue.empty() && !f1_yvalue.empty())
			{
				auto pfl11 = scatter(f1_xvalue, f1_yvalue);
				pfl11->color("blue");
				pfl11->fill(true);
				pfl11->marker_color("b");
				pfl11->marker_size(1.5);
				pfl11->marker("O");
				pfl11->marker_face(true);
				legdetails.push_back("file1");
				hold(true);
			}

			if (!f2_xvalue.empty() && !f2_yvalue.empty())
			{
				auto pfl2 = scatter(f2_xvalue, f2_yvalue);
				pfl2->color("red");
				pfl2->fill(true);
				pfl2->marker_color("r");
				pfl2->marker_size(1.5);
				pfl2->marker("O");
				pfl2->marker_face(true);
				legdetails.push_back("file2");
			}

			ax1->legend(legdetails);
			radar_position.append(std::to_string(radarPosition));


			htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + ".png";
			save((char*)htmlreport_image.c_str(), "png");

			htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + ".png";


			//std::cout << std::endl << "htmlreport_image " << htmlreport_image;
			//std::cout << std::endl << "htmlreport_imagepath " << htmlreport_imagepath;
			//htmlreport_folder
			//std::cout << std::endl << "htmlreport_folder " << htmlreport_folder;

			/*<!--Tab links-->
				<div class = "tab">
				<button class = "tablinks" onclick = "openCity(event, 'London')">London< / button>
				<button class = "tablinks" onclick = "openCity(event, 'Paris')">Paris< / button>
				<button class = "tablinks" onclick = "openCity(event, 'Tokyo')">Tokyo< / button>
				< / div>

				<!--Tab content-->
				<div id = "London" class = "tabcontent">
				<h3>London< / h3>
				<p>London is the capital city of England.< / p>
				< / div>

				<div id = "Paris" class = "tabcontent">
				<h3>Paris< / h3>
				<p>Paris is the capital of France.< / p>
				< / div>*/

				/*<img src = "img_chania.jpg" alt = "Flowers in Chania">*/


			CTML::Node image_trks("img");
			image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
				.UseClosingTag(false);


			document.AppendNodeToBody(image_trks);
		}

	}
	log_report << document.ToString() << endl;

	if (!log_report.is_open())
		log_report.close();




}

#endif