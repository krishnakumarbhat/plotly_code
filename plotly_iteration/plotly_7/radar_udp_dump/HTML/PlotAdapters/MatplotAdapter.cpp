#include "MatplotAdapter.h"

namespace fs = std::filesystem;
MatplotAdapter::MatplotAdapter(std::shared_ptr<PlotParameter[]>& sptr_plotparam):sptr_plotparam_plotadapter(sptr_plotparam)
{
	
	//std::cout << "\MatplotAdapter::MatplotAdapter()";
	/*ffile1.open("file1adapterdump.txt", ios::out | ios::app);
	ffile2.open("file2adapterdump.txt", ios::out | ios::app);
	ffile1 << "\n align_angle_az ";
	ffile2 << "\n align_angle_az";*/

	file1.open("mismatch.txt", ios::out | ios::app);
	file2.open("match.txt", ios::out | ios::app);
	
}

MatplotAdapter::~MatplotAdapter()
{
	//std::cout << "\nMatplotAdapter::~MatplotAdapter()";
	file1.close();
	file2.close();
}



void MatplotAdapter::generate_plot()
{
	//std::cout << "\nMatplotAdapter::generate_plot";

	


}

void MatplotAdapter::trigger_plot_generation(std::set<int> radarpositions, const char* f_name, const char* pltfolder, int filecount, std::string reportpath)
{
	mset_radarposition = radarpositions;

	JsonInputParser& jsonparser = JsonInputParser::getInstance();
	MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();
	
	remove_old_html_reports(f_name, pltfolder, reportpath);

	//1 func call for common property scatter plot
	//     vehicle info
	//     timing info
	//     error info
	//     other detection prop
	//     alignment angle
	//     mnr info
	//2 func call for detection property Histogram plot
	//3 func call for detection property scatter plot
	    // 3.1 func call for detection mismatch value plot
		// 3.2 func call for detection mismatch percentange plot (pie chart)
		
	//4 func call for tracker property plot

	if (1)//Common plots for scaled and non scaled mode
	{
		for (auto& radarpos : mset_radarposition)
		{
			string plottitle1;
			std::map<int, string> titleinfomap;
			matplotnonscale.PrintReportInfoinHTML(f_name, "html", pltfolder, reportpath, radarpos);



			//-----------------------------------------Speed--------------------------------------------------------//
			if ((sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0].size() != 0)
			{
				titleinfomap[0] = "RL Vehicle Speed";
				titleinfomap[1] = "RR Vehicle Speed";
				titleinfomap[2] = "FR Vehicle Speed";
				titleinfomap[3] = "FL Vehicle Speed";
				titleinfomap[5] = "FC Vehicle Speed";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[1],
																(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[1],
																filecount, f_name, plottitle1.c_str(), pltfolder,
																"speed km/hr", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[1].clear();

			}

			//-----------------------------------------Yaw Rate--------------------------------------------------------//
			if ((sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0].size() != 0)
			{
				titleinfomap[0] = "RL Yawrate";
				titleinfomap[1] = "RR Yawrate";
				titleinfomap[2] = "FR Yawrate";
				titleinfomap[3] = "FL Yawrate";
				titleinfomap[5] = "FC Yawrate";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[1],
																(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[1],
																filecount, f_name, plottitle1.c_str(),pltfolder, 
																"yaw rate", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[1].clear();
			}

			////-----------------------------------------C2 Timing Info--------------------------------------------------------//

			if ((sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0].size() != 0 && 
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size()!=0)
			{
				titleinfomap[0] = "RL C2 Timing info[9]";
				titleinfomap[1] = "RR C2 Timing info[9]";
				titleinfomap[2] = "FR C2 Timing info[9]";
				titleinfomap[3] = "FL C2 Timing info[9]";
				titleinfomap[5] = "FC C2 Timing info[9]";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1],
					                                           filecount, f_name, plottitle1.c_str(), pltfolder,
					                                           "c2 timing info", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1].clear();
			}


			//-----------------------------------------error info--------------------------------------------------------//


			

			if ((sptr_plotparam_plotadapter.get() + radarpos)->c0_c2_ipc_err_cntr[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size() != 0)
			{
				titleinfomap[0] = "RL c0 c2 IPC Error count";
				titleinfomap[1] = "RR c0 c2 IPC Error count";
				titleinfomap[2] = "FR c0 c2 IPC Error count";
				titleinfomap[3] = "FL c0 c2 IPC Error count";
				titleinfomap[5] = "FC c0 c2 IPC Error count";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->c0_c2_ipc_err_cntr[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
																(sptr_plotparam_plotadapter.get() + radarpos)->c0_c2_ipc_err_cntr[1],
																filecount, f_name, plottitle1.c_str(), pltfolder,
																"c0 c2 ipc error", reportpath, radarpos);

				
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->c1_c2_ipc_err_cntr[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size() != 0)
			{
				titleinfomap[0] = "RL C1 C2 IPC Error count";
				titleinfomap[1] = "RR C1 C2 IPC Error count";
				titleinfomap[2] = "FR C1 C2 IPC Error count";
				titleinfomap[3] = "FL C1 C2 IPC Error count";
				titleinfomap[5] = "FC C1 C2 IPC Error count";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->c1_c2_ipc_err_cntr[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->c1_c2_ipc_err_cntr[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"c1 c2 ipc error", reportpath, radarpos);


			}


			if ((sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_Cust_Core[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size() != 0)
			{
				titleinfomap[0] = "RL Float Exception CountCustCore";
				titleinfomap[1] = "RR Float Exception CountCustCore";
				titleinfomap[2] = "FR Float Exception CountCustCore";
				titleinfomap[3] = "FL Float Exception CountCustCore";
				titleinfomap[5] = "FC Float Exception CountCustCore";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_Cust_Core[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_Cust_Core[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"float exception", reportpath, radarpos);


			}


			if ((sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_RDD_Core[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size() != 0)
			{
				titleinfomap[0] = "RL Float Exception Count RDD Core";
				titleinfomap[1] = "RR Float Exception Count RDD Core";
				titleinfomap[2] = "FR Float Exception Count RDD Core";
				titleinfomap[3] = "FL Float Exception Count RDD Core";
				titleinfomap[5] = "FC Float Exception Count RDD Core";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_RDD_Core[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_RDD_Core[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"float excep rdd", reportpath, radarpos);


			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_FF_Core[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].size() != 0)
			{
				titleinfomap[0] = "RL Float Exception Count FF Core";
				titleinfomap[1] = "RR Float Exception Count FF Core";
				titleinfomap[2] = "FR Float Exception Count FF Core";
				titleinfomap[3] = "FL Float Exception Count FF Core";
				titleinfomap[5] = "FC Float Exception Count FF Core";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_FF_Core[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->Float_Exception_Count_FF_Core[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"Float Exception Count FF Core", reportpath, radarpos);


			}

			


			//-----------------------------------------Difference in Valid Detection Count Bar--------------------------------------------------------//
			if (1)
			{		
				std::vector<double> vec_diff_detec_scanindex;
				std::vector<double> file1_diff_count, file2_diff_count;
				std::vector<double> vec_diff_count_value;

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[0].size() != 0||
					(sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].size() != 0)
				{

					titleinfomap[0] = "RL Scatter Detection count Veh VS Resim";
					titleinfomap[1] = "RR Scatter Detection count Veh VS Resim";
					titleinfomap[2] = "FR Scatter Detection count Veh VS Resim";
					titleinfomap[3] = "FL Scatter Detection count Veh VS Resim";
					titleinfomap[5] = "FC Scatter Detection count Veh VS Resim";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					//filling file2 map with missing scan index and value zero: Equalising the map of valid detection count
					for (auto& file1_data : (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[0])
					{
						auto map2itr = (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].find(file1_data.first);
						if (map2itr == (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].end())
						{
							(sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].insert(std::pair<int, int>(file1_data.first, 0));
						}
					}

					

					vec_diff_detec_scanindex.clear();
					file1_diff_count.clear();
					file2_diff_count.clear();
					vec_diff_count_value.clear();
					

					//logic to get difference count number and corresponding scanindex
					for (auto& file1_data : (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[0])
					{
						auto map2itr = (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].find(file1_data.first);
						if (map2itr != (sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].end())
						{
							int diff_count = file1_data.second-map2itr->second;
							
														
								vec_diff_detec_scanindex.push_back(file1_data.first);
								vec_diff_count_value.push_back(diff_count);

								file1_diff_count.push_back(file1_data.second);
								file2_diff_count.push_back(map2itr->second);
							

						}

					}
					
					matplotnonscale.generate_detection_scatterplot(vec_diff_detec_scanindex,
						                                           file1_diff_count,
						                                           vec_diff_detec_scanindex,
						                                           file2_diff_count,
						                                           filecount, f_name, plottitle1.c_str(), 
						                                           pltfolder, "scatter detection count", reportpath, radarpos);				

				}



				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[0].size() != 0 ||
					(sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].size() != 0)
				{
					titleinfomap[0] = "RL Box plot Diff Detection value";
					titleinfomap[1] = "RR Box plot Diff Detection value";
					titleinfomap[2] = "FR Box plot Diff Detection value";
					titleinfomap[3] = "FL Box plot Diff Detection value";
					titleinfomap[5] = "FC Box plot Diff Detection value";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}


					matplotnonscale.generate_boxplot(vec_diff_count_value,
													file1_diff_count,
													filecount, f_name, plottitle1.c_str(),
													pltfolder, "diff detection count", reportpath, radarpos);

				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[0].size() != 0 ||
					(sptr_plotparam_plotadapter.get() + radarpos)->map_si_valid_detection_count[1].size() != 0)
				{
					titleinfomap[0] = "RL scatter Diff Detection value ";
					titleinfomap[1] = "RR scatter Diff Detection value";
					titleinfomap[2] = "FR scatter Diff Detection value";
					titleinfomap[3] = "FL scatter Diff Detection value";
					titleinfomap[5] = "FC scatter Diff Detection value";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					file2_diff_count.clear();
					matplotnonscale.generate_detection_scatterplot(vec_diff_detec_scanindex,
																   vec_diff_count_value,
																   vec_diff_detec_scanindex,
																   file2_diff_count,
																   filecount, f_name, plottitle1.c_str(),
																   pltfolder, "diff detection count", reportpath, radarpos);

				}


				

				
			}

			//-----------------------------------------other Detection properties --------------------------------//
			
			

			if ((sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[0].size() != 0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[1].size() != 0)
			{
				titleinfomap[0] = "RL IsBistatic";
				titleinfomap[1] = "RR IsBistatic";
				titleinfomap[2] = "FR IsBistatic";
				titleinfomap[3] = "FL IsBistatic";
				titleinfomap[5] = "FC IsBistatic";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				/*std::cout << std::endl << "## isBistatic[0].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[0].size();

				std::cout << std::endl << "## isBistatic[1].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[1].size();*/

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->bistatic_scanindex[0],
					                                          (sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[0],
					                                          (sptr_plotparam_plotadapter.get() + radarpos)->bistatic_scanindex[1],
					                                          (sptr_plotparam_plotadapter.get() + radarpos)->isBistatic[1],
					                                           filecount, f_name, plottitle1.c_str(), pltfolder,
					                                           "bistatic count", reportpath, radarpos);
				
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_veh != 0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_resim != 0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_veh !=0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_resim !=0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_veh !=0 ||
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_resim !=0)
			{
				titleinfomap[0] = "RL diff detection count Bar";
				titleinfomap[1] = "RR diff detection count Bar";
				titleinfomap[2] = "FR diff detection count Bar";
				titleinfomap[3] = "FL diff detection count Bar";
				titleinfomap[5] = "FC diff detection count Bar";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

			
				std::vector<double> x = { 1, 2, 3 };
				std::vector<std::vector<double>> y;

				std::vector<double> y1, y2;
				
				

				y1.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_veh));
				y1.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_veh));
				y1.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_veh));

				y2.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_resim));
				y2.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_resim));
				y2.push_back(abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_resim));

				y.push_back(y1);
				y.push_back(y2);
				
				std::cout << std::endl << " cum_isbistatic_count_veh "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_veh);
				std::cout << std::endl << " cum_isbistatic_count_resim "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_resim);
				std::cout << std::endl << " cum_issingletarget_count_veh "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_veh);
				std::cout << std::endl << " cum_issingletarget_count_resim "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_resim);
				std::cout << std::endl << " cum_issuperrestarget_count_veh "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_veh);
				std::cout << std::endl << " cum_issuperrestarget_count_resim "<< abs((sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_resim);
				

				matplotnonscale.generate_barplot(x,y,
					                             filecount, f_name, plottitle1.c_str(), pltfolder,
					                             "detection diff count", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_veh = 0;
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_veh = 0;
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_veh = 0;

				(sptr_plotparam_plotadapter.get() + radarpos)->cum_isbistatic_count_resim = 0;
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issingletarget_count_resim = 0;
				(sptr_plotparam_plotadapter.get() + radarpos)->cum_issuperrestarget_count_resim = 0;

			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[0].size() != 0 || 
				(sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[1].size() != 0)
			{
				titleinfomap[0] = "RL IsSingleTarget";
				titleinfomap[1] = "RR IsSingleTarget";
				titleinfomap[2] = "FR IsSingleTarget";
				titleinfomap[3] = "FL IsSingleTarget";
				titleinfomap[5] = "FC IsSingleTarget";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				/*std::cout << std::endl << " isSingleTarget[0].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[0].size();
				std::cout << std::endl << " isSingleTarget[1].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[1].size();*/

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->issingletarget_scanindex[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->issingletarget_scanindex[1],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->isSingleTarget[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder,
					                                            "issingletarget count", reportpath, radarpos);

			}

		


			if ((sptr_plotparam_plotadapter.get() + radarpos)->super_res_target[0].size() != 0||
				(sptr_plotparam_plotadapter.get() + radarpos)->super_res_target[1].size() != 0)
			{
				titleinfomap[0] = "RL Super Res Target";
				titleinfomap[1] = "RR Super Res Target";
				titleinfomap[2] = "FR Super Res Target";
				titleinfomap[3] = "FL Super Res Target";
				titleinfomap[5] = "FC Super Res Target";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				/*std::cout << std::endl << "### superrestarget_scanindex[0].size() "<<(sptr_plotparam_plotadapter.get() + radarpos)->superrestarget_scanindex[0].size();
				std::cout << std::endl << "### superrestarget_scanindex[1].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->superrestarget_scanindex[1].size();*/
				matplotnonscale.generate_detection_scatterplot(
					(sptr_plotparam_plotadapter.get() + radarpos)->superrestarget_scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->super_res_target[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->superrestarget_scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->super_res_target[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"Super Res Target", reportpath, radarpos);

			}


			if ((sptr_plotparam_plotadapter.get() + radarpos)->num_fp_detections[0].size() != 0)
			{
				titleinfomap[0] = "RL First Pass RDD";
				titleinfomap[1] = "RR First Pass RDD";
				titleinfomap[2] = "FR First Pass RDD";
				titleinfomap[3] = "FL First Pass RDD";
				titleinfomap[5] = "FC First Pass RDD";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->num_fp_detections[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->si_resim_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->num_fp_detections[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"first pass rdd", reportpath, radarpos);

			}


			if ((sptr_plotparam_plotadapter.get() + radarpos)->num_sp_detections[0].size() != 0)
			{
				titleinfomap[0] = "RL Second Pass RDD";
				titleinfomap[1] = "RR Second Pass RDD";
				titleinfomap[2] = "FR Second Pass RDD";
				titleinfomap[3] = "FL Second Pass RDD";
				titleinfomap[5] = "FC Second Pass RDD";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->num_sp_detections[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->si_resim_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->num_sp_detections[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"second pass rdd", reportpath, radarpos);

			}


			if ((sptr_plotparam_plotadapter.get() + radarpos)->rest_count[0].size() != 0)
			{
				titleinfomap[0] = "RL Rest Count";
				titleinfomap[1] = "RR Rest Count";
				titleinfomap[2] = "FR Rest Count";
				titleinfomap[3] = "FL Rest Count";
				titleinfomap[5] = "FC Rest Count";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->rest_count[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->si_resim_ref_scale,
					(sptr_plotparam_plotadapter.get() + radarpos)->rest_count[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"rest count", reportpath, radarpos);

			}

			/*if ((sptr_plotparam_plotadapter.get() + radarpos)->af_counters[0].size() != 0)
			{
				titleinfomap[0] = "RL AF Counters";
				titleinfomap[1] = "RR AF Counters";
				titleinfomap[2] = "FR AF Counters";
				titleinfomap[3] = "FL AF Counters";
				titleinfomap[5] = "FC AF Counters";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
					(sptr_plotparam_plotadapter.get() + radarpos)->af_counters[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
					(sptr_plotparam_plotadapter.get() + radarpos)->af_counters[1],
					filecount, f_name, plottitle1.c_str(), pltfolder,
					"AF Counters", reportpath, radarpos);

			}*/

			/*(sptr_plotparam_stla.get() + radar_position)->isBistatic[fileCount].push_back(double((Flr4p_Dets_info_s3.target_report[index[i]].isBistatic)));
			(sptr_plotparam_stla.get() + radar_position)->isSingleTarget[fileCount].push_back(double((Flr4p_Dets_info_s3.target_report[index[i]].isSingleTarget)));
			(sptr_plotparam_stla.get() + radar_position)->isSingleTarget_azimuth[fileCount].push_back(double((Flr4p_Dets_info_s3.target_report[index[i]].isSingleTarget_azimuth)));
			(sptr_plotparam_stla.get() + radar_position)->super_res_target[fileCount].push_back(double((Flr4p_Dets_info_s3.target_report[index[i]].super_res_target)));*/

			//-----------------------------------------initial align_angle_az Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0  &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az_initial[0].size()!=0)
			{
				titleinfomap[0] = "RL Initial Align Angle az ";
				titleinfomap[1] = "RR Initial Align Angle az";
				titleinfomap[2] = "FR Initial Align Angle az";
				titleinfomap[3] = "FL Initial Align Angle az";
				titleinfomap[5] = "FC Initial Align Angle az";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}



				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az_initial[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az_initial[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "initial alignangle az (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az_initial[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az_initial[1].clear();
			}

			//-----------------------------------------estimated align_angle_az Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az[0].size()!=0)
			{
				titleinfomap[0] = "RL Estimated Align Angle az ";
				titleinfomap[1] = "RR Estimated Align Angle az";
				titleinfomap[2] = "FR Estimated Align Angle az";
				titleinfomap[3] = "FL Estimated Align Angle az";
				titleinfomap[5] = "FC Estimated Align Angle az";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "esti alignangle az (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_az[1].clear();

			}


			//-----------------------------------------n_updates_azimuth Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_azimuth[0].size()!=0)
			{
				titleinfomap[0] = "RL n updates_azimuth";
				titleinfomap[1] = "RR n updates_azimuth";
				titleinfomap[2] = "FR n updates_azimuth";
				titleinfomap[3] = "FL n updates_azimuth";
				titleinfomap[5] = "FC n updates_azimuth";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_azimuth[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_azimuth[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "n updates azimuth", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_azimuth[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_azimuth[1].clear();

			}

	

			//-----------------------------------------initial align_angle_el Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el_initial[0].size()!=0)
			{
				titleinfomap[0] = "RL Initial Align el angl ";
				titleinfomap[1] = "RR Initial Align el angl ";
				titleinfomap[2] = "FR Initial Align el angl ";
				titleinfomap[3] = "FL Initial Align el angl ";
				titleinfomap[5] = "FC Initial Align el angl ";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el_initial[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el_initial[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "initial alignang el (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el_initial[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el_initial[1].clear();
			}


			//-----------------------------------------estimated align_angle_el Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el[0].size()!=0)
			{
				titleinfomap[0] = "RL Estimated align angl el";
				titleinfomap[1] = "RR Estimated align angl el";
				titleinfomap[2] = "FR Estimated align angl el";
				titleinfomap[3] = "FL Estimated align angl el";
				titleinfomap[5] = "FC Estimated align angl el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "estimated alignang el (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_el[1].clear();

			}

			//-----------------------------------------n_updates_elevation Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_elevation[0].size()!=0)
			{
				titleinfomap[0] = "RL n updates elevation";
				titleinfomap[1] = "RR n updates elevation";
				titleinfomap[2] = "FR n updates elevation";
				titleinfomap[3] = "FL n updates elevation";
				titleinfomap[5] = "FC n updates elevation";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_elevation[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_elevation[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "n updates elevation", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_elevation[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_n_updates_elevation[1].clear();

			}

			//-----------------------------------------align_quality_factor_az Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_az[0].size()!=0)
			{
				titleinfomap[0] = "RL align quality factor az";
				titleinfomap[1] = "RR align quality factor az";
				titleinfomap[2] = "FR align quality factor az";
				titleinfomap[3] = "FL align quality factor az";
				titleinfomap[5] = "FC align quality factor az";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_az[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_az[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "align quality factor az", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_az[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_az[1].clear();

			}
			//-----------------------------------------align_quality_factor_el Scatter--------------------------------------------------------//  
			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[0].size()!=0)
			{
				titleinfomap[0] = "RL align quality factor el";
				titleinfomap[1] = "RR align quality factor el";
				titleinfomap[2] = "FR align quality factor el";
				titleinfomap[3] = "FL align quality factor el";
				titleinfomap[5] = "FC align quality factor el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "align quality factor el", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[1].clear();

			}
			//Added for opp_align_quality_factor_elev
			
			//-----------------------------------------opp_align_quality_factor_elev Scatter--------------------------------------------------------// 
			
			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[0].size() != 0)
			{
				titleinfomap[0] = "RL opp align quality factor el";
				titleinfomap[1] = "RR opp align quality factor el";
				titleinfomap[2] = "FR opp align quality factor el";
				titleinfomap[3] = "FL opp align quality factor el";
				titleinfomap[5] = "FC opp align quality factor el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_opp_align_quality_factor_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_opp_align_quality_factor_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "opp align quality factor el", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_quality_factor_el[1].clear();

			}
			//Added for own_align_angle_raw_elev
			//-----------------------------------------own_align_angle_raw_elev Scatter--------------------------------------------------------// 

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_raw_el[0].size() != 0)
			{
				titleinfomap[0] = "RL own_align_angle_raw_el";
				titleinfomap[1] = "RR own_align_angle_raw_el";
				titleinfomap[2] = "FR own_align_angle_raw_el";
				titleinfomap[3] = "FL own_align_angle_raw_el";
				titleinfomap[5] = "FC own_align_angle_raw_el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_raw_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_raw_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "own align angle raw el", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_raw_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_angle_raw_el[1].clear();

			}
			
			//-----------------------------------------align_numpts_el Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[0].size()!=0)
			{
				titleinfomap[0] = "RL align numpts el";
				titleinfomap[1] = "RR align numpts el";
				titleinfomap[2] = "FR align numpts el";
				titleinfomap[3] = "FL align numpts el";
				titleinfomap[5] = "FC align numpts el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				std::cout << std::endl << "@@@ vec_align_numpts_el[0]" << (sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[0].size();
				std::cout << std::endl << "@@@ vec_align_numpts_el[1]" << (sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[1].size();

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "align numpts el", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_el[1].clear();

			}
			//-----------------------------------------align_numpts_az Scatter--------------------------------------------------------//  
			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_az[0].size()!=0)
			{
				titleinfomap[0] = "RL align numpts az";
				titleinfomap[1] = "RR align numpts az";
				titleinfomap[2] = "FR align numpts az";
				titleinfomap[3] = "FL align numpts az";
				titleinfomap[5] = "FC align numpts az";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_az[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_az[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "align numpts az", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_az[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_align_numpts_az[1].clear();
			}


			//-----------------------------------------misalignment_angle_az Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_az[0].size() != 0)
			{
				titleinfomap[0] = "RL MisAlign Angle az ";
				titleinfomap[1] = "RR MisAlign Angle az";
				titleinfomap[2] = "FR MisAlign Angle az";
				titleinfomap[3] = "FL MisAlign Angle az";
				titleinfomap[5] = "FC MisAlign Angle az";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}



				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_az[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_az[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "misalign angle az (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_az[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_az[1].clear();
			}

			//-----------------------------------------misalignment_angle_el Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_el[0].size() != 0)
			{
				titleinfomap[0] = "RL MisAlign Angle el ";
				titleinfomap[1] = "RR MisAlign Angle el";
				titleinfomap[2] = "FR MisAlign Angle el";
				titleinfomap[3] = "FL MisAlign Angle el";
				titleinfomap[5] = "FC MisAlign Angle el";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}



				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_el[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_el[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "misalign angle el (degree)", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_el[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_misalign_angle_el[1].clear();
			}



			//----------------------------------------Deetected_Interference Scatter(GPO_GEN7)--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->interference_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->interference_detected[0].size() != 0)
			{
				titleinfomap[0] = "RL Interference Detected";
				titleinfomap[1] = "RR Interference Detected";
				titleinfomap[2] = "FR Interference Detected";
				titleinfomap[3] = "FL Interference Detected";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->interference_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->interference_detected[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->interference_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->interference_detected[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "Intereference Detected", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->interference_detected[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->interference_detected[1].clear();

			}


			//-----------------------------------------AvgRm_mnr_avg Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgRm_mnr_avg[0].size()!=0)
			{
				titleinfomap[0] = "RL AvgRange mnr avg";
				titleinfomap[1] = "RR AvgRange mnr avg";
				titleinfomap[2] = "FR AvgRange mnr avg";
				titleinfomap[3] = "FL AvgRange mnr avg";
				titleinfomap[5] = "FC AvgRange mnr avg";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgRm_mnr_avg[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgRm_mnr_avg[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "AvgRm mnr avg", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgRm_mnr_avg[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgRm_mnr_avg[1].clear();

			}
			//-----------------------------------------MaxRm_mnr_avg Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxRm_mnr_avg[0].size()!=0)
			{
				titleinfomap[0] = "RL MaxRange mnr avg";
				titleinfomap[1] = "RR MaxRange mnr avg";
				titleinfomap[2] = "FR MaxRange mnr avg";
				titleinfomap[3] = "FL MaxRange mnr avg";
				titleinfomap[5] = "FC MaxRange mnr avg";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxRm_mnr_avg[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxRm_mnr_avg[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "MaxRm mnr avg", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxRm_mnr_avg[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxRm_mnr_avg[1].clear();

			}

			//-----------------------------------------AvgPm_mnr_avg Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0&&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgPm_mnr_avg[0].size()!=0)
			{
				titleinfomap[0] = "RL AvgPower mnr avg";
				titleinfomap[1] = "RR AvgPower mnr avg";
				titleinfomap[2] = "FR AvgPower mnr avg";
				titleinfomap[3] = "FL AvgPower mnr avg";
				titleinfomap[5] = "FC AvgPower mnr avg";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgPm_mnr_avg[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgPm_mnr_avg[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "AvgPm mnr avg", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgPm_mnr_avg[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_AvgPm_mnr_avg[1].clear();

			}




			//-----------------------------------------MaxPm_mnr_avg Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxPm_mnr_avg[0].size()!=0)
			{
				titleinfomap[0] = "RL MaxPower mnr avg";
				titleinfomap[1] = "RR MaxPower mnr avg";
				titleinfomap[2] = "FR MaxPower mnr avg";
				titleinfomap[3] = "FL MaxPower mnr avg";
				titleinfomap[5] = "FC MaxPower mnr avg";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxPm_mnr_avg[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxPm_mnr_avg[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "MaxPm mnr avg", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxPm_mnr_avg[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_MaxPm_mnr_avg[1].clear();

			}
			//-----------------------------------------Degraded Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Degraded[0].size()!=0)
			{
				titleinfomap[0] = "RL Degraded";
				titleinfomap[1] = "RR Degraded";
				titleinfomap[2] = "FR Degraded";
				titleinfomap[3] = "FL Degraded";
				titleinfomap[5] = "FC Degraded";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_Degraded[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_Degraded[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "degraded", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Degraded[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Degraded[1].clear();

			}
			//-----------------------------------------vec_Blocked Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size()!=0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Blocked[0].size()!=0)
			{


				titleinfomap[0] = "RL Blocked";
				titleinfomap[1] = "RR Blocked";
				titleinfomap[2] = "FR Blocked";
				titleinfomap[3] = "FL Blocked";
				titleinfomap[5] = "FC Blocked";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_Blocked[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->vec_Blocked[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "Blocked", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Blocked[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->vec_Blocked[1].clear();

			}

			//-----------------------------------------Blockage_Status Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_status[0].size() != 0)
			{
				titleinfomap[0] = "RL MnrBlockage Status";
				titleinfomap[1] = "RR MnrBlockage Status";
				titleinfomap[2] = "FR MnrBlockage Status";
				titleinfomap[3] = "FL MnrBlockage Status";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->blockage_status[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->blockage_status[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "MnrBlockage Status", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_status[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_status[1].clear();

			}

			//-----------------------------------------Blockage_Active Scatter--------------------------------------------------------//  

			if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size() != 0 &&
				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_active[0].size() != 0)
			{
				titleinfomap[0] = "RL MnrBlockage Active";
				titleinfomap[1] = "RR MnrBlockage Active";
				titleinfomap[2] = "FR MnrBlockage Active";
				titleinfomap[3] = "FL MnrBlockage Active";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->blockage_active[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->blockage_active[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "MnrBlockage Active", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_active[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->blockage_active[1].clear();

			}


			if (/*jsonparser.bdetectionplot*/1)
			{
				
				//-----------------------------------------Valid Detection Count--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[0].size() != 0)
				{
					titleinfomap[0] = "RL Valid  Detection Count";
					titleinfomap[1] = "RR Valid  Detection Count";
					titleinfomap[2] = "FR Valid  Detection Count";
					titleinfomap[3] = "FL Valid  Detection Count";
					titleinfomap[5] = "FC Valid  Detection Count";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh_ref_scale,
						(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim_ref_scale,
						(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "valid detection count", reportpath, radarpos);

					(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[0].clear();
					(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[1].clear();
				}
				//-----------------------------------------Maximum Range--------------------------------------------------------//

				if((sptr_plotparam_plotadapter.get() + radarpos)->range_max[0].size()!=0)
				{ 
				titleinfomap[0] = "RL Max Range  ";
				titleinfomap[1] = "RR Max Range  ";
				titleinfomap[2] = "FR Max Range  ";
				titleinfomap[3] = "FL Max Range  ";
				titleinfomap[5] = "FC Max Range  ";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}


				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->range_max[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[1],
																(sptr_plotparam_plotadapter.get() + radarpos)->range_max[1],
																filecount, f_name, plottitle1.c_str(), 
					                                            pltfolder, "max range value", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->range_max[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->range_max[1].clear();

				}
				//-----------------------------------------Range Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->range[0].size() != 0)
				{


					titleinfomap[0] = "RL Range Histogram  ";
					titleinfomap[1] = "RR Range Histogram ";
					titleinfomap[2] = "FR Range Histogram ";
					titleinfomap[3] = "FL Range Histogram ";
					titleinfomap[5] = "FC Range Histogram ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->range[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->range[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "range", reportpath, radarpos);

				}
				//-----------------------------------------Range Rate Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0].size() != 0) {
					titleinfomap[0] = "RL RangeRate Histogram  ";
					titleinfomap[1] = "RR RangeRate Histogram ";
					titleinfomap[2] = "FR RangeRate Histogram ";
					titleinfomap[3] = "FL RangeRate Histogram ";
					titleinfomap[5] = "FC RangeRate Histogram ";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0],
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->range_rate[1],
					                                         filecount, f_name, plottitle1.c_str(), 
					                                         pltfolder, "rangerate", reportpath, radarpos);


				}
				//-----------------------------------------Azimuthal Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0].size() != 0)
				{
					titleinfomap[0] = "RL Azimuth Histogram";
					titleinfomap[1] = "RR Azimuth Histogram";
					titleinfomap[2] = "FR Azimuth Histogram";
					titleinfomap[3] = "FL Azimuth Histogram";
					titleinfomap[5] = "FC Azimuth Histogram";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->azimuth[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "azimuth", reportpath, radarpos);

				}

				//-----------------------------------------Elevation Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->elevation[0].size()!=0)
				{
					titleinfomap[0] = "RL Elevation Histogram ";
					titleinfomap[1] = "RR Elevation Histogram";
					titleinfomap[2] = "FR Elevation Histogram";
					titleinfomap[3] = "FL Elevation Histogram";
					titleinfomap[5] = "FC Elevation Histogram";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->elevation[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->elevation[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "elevation", reportpath, radarpos);
				}
				//-----------------------------------------RCS Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->rcs[0].size()!=0)
				{
					titleinfomap[0] = "RL RCS Histogram";
					titleinfomap[1] = "RR RCS Histogram";
					titleinfomap[2] = "FR RCS Histogram";
					titleinfomap[3] = "FL RCS Histogram";
					titleinfomap[5] = "FC RCS Histogram";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->rcs[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->rcs[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "rcs", reportpath, radarpos);
				}
				//-----------------------------------------Amplitude Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->amp[0].size()!=0)
				{
					titleinfomap[0] = "RL Amplitude Histogram";
					titleinfomap[1] = "RR Amplitude Histogram";
					titleinfomap[2] = "FR Amplitude Histogram";
					titleinfomap[3] = "FL Amplitude Histogram";
					titleinfomap[5] = "FC Amplitude Histogram";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->amp[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->amp[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "amplitude", reportpath, radarpos);
				}
				//-----------------------------------------SNR Histogram--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->snr[0].size()!=0)
				{
					titleinfomap[0] = "RL SNR Histogram";
					titleinfomap[1] = "RR SNR Histogram";
					titleinfomap[2] = "FR SNR Histogram";
					titleinfomap[3] = "FL SNR Histogram";
					titleinfomap[5] = "FC SNR Histogram";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->snr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->snr[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "snr", reportpath, radarpos);
				}
			}
			
		}
	}

	if (jsonparser.bscanindex_scaling == true)
	{
		std::cout << "\n scaling plot enabled";
		for (auto& radarpos : mset_radarposition)
		{		
			if (/*jsonparser.bdetectionplot*/1)
			{

				string plottitle1{};
				std::map<int, string> titleinfomap{};
				std::string plotparam{};

				//matplotnonscale.PrintReportInfoinHTML(f_name, "html", pltfolder, reportpath, radarpos);

				//-----------------------------------------C2 Timing Info--------------------------------------------------------//
				/*titleinfomap[0] = "RL c2 timing info[9]";
				titleinfomap[1] = "RR c2 timing info[9]";
				titleinfomap[2] = "FR c2 timing info[9]";
				titleinfomap[3] = "FL c2 timing info[9]";
				titleinfomap[5] = "FC c2 timing info[9]";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1],
					(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "c2 timing info", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1].clear();*/

				//-----------------------------------------Range Mismatch Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_range[1].size() != 0)
				{
					titleinfomap[0] = "RL Detection Mismatch range ";
					titleinfomap[1] = "RR Detection Mismatch range";
					titleinfomap[2] = "FR Detection Mismatch range";
					titleinfomap[3] = "FL Detection Mismatch range";
					titleinfomap[5] = "FC Detection Mismatch range";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_range[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, " Range",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->range[0].size());
				}

				//-----------------------------------------Range Rate Mismatch Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[1].size() != 0)
				{


					titleinfomap[0] = "RL Detection Mismatch range rate ";
					titleinfomap[1] = "RR Detection Mismatch range rate";
					titleinfomap[2] = "FR Detection Mismatch range rate";
					titleinfomap[3] = "FL Detection Mismatch range rate";
					titleinfomap[5] = "FC Detection Mismatch range rate";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Range rate",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0].size());
				}

				//-----------------------------------------Azimuth Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[1].size() != 0)
				{

					titleinfomap[0] = "RL Azimuth Mismatch  ";
					titleinfomap[1] = "RR Azimuth Mismatch  ";
					titleinfomap[2] = "FR Azimuth Mismatch  ";
					titleinfomap[3] = "FL Azimuth Mismatch  ";
					titleinfomap[5] = "FC Azimuth Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Azimuth",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0].size());
				}

				//-----------------------------------------Elevation Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[1].size() != 0)
				{
					titleinfomap[0] = "RL Elevation Mismatch  ";
					titleinfomap[1] = "RR Elevation Mismatch  ";
					titleinfomap[2] = "FR Elevation Mismatch  ";
					titleinfomap[3] = "FL Elevation Mismatch  ";
					titleinfomap[5] = "FC Elevation Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Elevation",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->elevation[0].size());
				}


				//-----------------------------------------RCS Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[1].size() != 0)
				{
					titleinfomap[0] = "RL RCS Mismatch  ";
					titleinfomap[1] = "RR RCS Mismatch  ";
					titleinfomap[2] = "FR RCS Mismatch  ";
					titleinfomap[3] = "FL RCS Mismatch  ";
					titleinfomap[5] = "FC RCS Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "RCS",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->rcs[0].size());
				}

				//-----------------------------------------snr Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_snr[1].size() != 0)
				{
					titleinfomap[0] = "RL SNR Mismatch  ";
					titleinfomap[1] = "RR SNR Mismatch  ";
					titleinfomap[2] = "FR SNR Mismatch  ";
					titleinfomap[3] = "FL SNR Mismatch  ";
					titleinfomap[5] = "FC SNR Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_snr[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "SNR",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->snr[0].size());
				}


				//-----------------------------------------Scaled Blockage_Threshold(for GPO_GEN7)--------------------------------------------------------//  

				if ((sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].size() != 0 &&
					(sptr_plotparam_plotadapter.get() + radarpos)->blockage_thres[0].size() != 0)
				{
					titleinfomap[0] = "RL MnrBlockage Threshold";
					titleinfomap[1] = "RR MnrBlockage Threshold";
					titleinfomap[2] = "FR MnrBlockage Threshold";
					titleinfomap[3] = "FL MnrBlockage Threshold";
					titleinfomap[5] = "FC MnrBlockage Threshold";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}


					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_mnr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_mnr[1],
						filecount, f_name, plottitle1,
						pltfolder, "MnrBlockage Threshold",
						reportpath, radarpos);

					(sptr_plotparam_plotadapter.get() + radarpos)->map_mnr[0].clear();
					(sptr_plotparam_plotadapter.get() + radarpos)->map_mnr[1].clear();

				}


				//-----------------------------------------scaled detection range--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0].size()!=0)
				{
					plotparam = "range";
					titleinfomap[0] = "RL Detection range";
					titleinfomap[1] = "RR Detection range";
					titleinfomap[2] = "FR Detection range";
					titleinfomap[3] = "FL Detection range";
					titleinfomap[5] = "FC Detection range";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_range[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}


			


				

				//-----------------------------------------scaled detection range rate--------------------------------------------------------//
				
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0].size() != 0)
				{
					plotparam = "range rate";
					titleinfomap[0] = "RL Detection range rate";
					titleinfomap[1] = "RR Detection range rate";
					titleinfomap[2] = "FR Detection range rate";
					titleinfomap[3] = "FL Detection range rate";
					titleinfomap[5] = "FC Detection range rate";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}

			
				

				//-----------------------------------------scaled detection azimuth--------------------------------------------------------//
				
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0].size() != 0)
				{
					plotparam = "azimuth";
					titleinfomap[0] = "RL Detection azimuth";
					titleinfomap[1] = "RR Detection azimuth";
					titleinfomap[2] = "FR Detection azimuth";
					titleinfomap[3] = "FL Detection azimuth";
					titleinfomap[5] = "FC Detection azimuth";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}

					}
					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}

			
				

				

				//-----------------------------------------scaled detection elevation--------------------------------------------------------//
				
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0].size() != 0)
				{
					plotparam = "elevation";
					titleinfomap[0] = "RL Detection elevation";
					titleinfomap[1] = "RR Detection elevation";
					titleinfomap[2] = "FR Detection elevation";
					titleinfomap[3] = "FL Detection elevation";
					titleinfomap[5] = "FC Detection elevation";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}

			
				

				//-----------------------------------------scaled RCS Scatter--------------------------------------------------------//   
				
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0].size() != 0)
				{
					plotparam = "rcs";
					titleinfomap[0] = "RL Detection rcs";
					titleinfomap[1] = "RR Detection rcs";
					titleinfomap[2] = "FR Detection rcs";
					titleinfomap[3] = "FL Detection rcs";
					titleinfomap[5] = "FC Detection rcs";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}


				

				

				//-----------------------------------------SNR Scatter--------------------------------------------------------//  
				
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0].size() != 0)
				{
					plotparam = "snr";
					titleinfomap[0] = "RL Detection snr";
					titleinfomap[1] = "RR Detection snr";
					titleinfomap[2] = "FR Detection snr";
					titleinfomap[3] = "FL Detection snr";
					titleinfomap[5] = "FC Detection snr";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_snr[1],
						filecount, f_name, plottitle1,
						pltfolder, plotparam,
						reportpath, radarpos);
				}


			
				

			}

			if (/*jsonparser.btrackerplot*/1)
			{
				string plottitle1{};
				std::map<int, string> titleinfomap{};
				std::string plotparam{};

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xpos[0].size() != 0)
				{
					plotparam = "xpos";
					titleinfomap[0] = "RL tracker xpos";
					titleinfomap[1] = "RR tracker xpos";
					titleinfomap[2] = "FR tracker xpos";
					titleinfomap[3] = "FL tracker xpos";
					titleinfomap[5] = "FC tracker xpos";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xpos[0],
						                         (sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xpos[1],
						                          filecount, f_name, plottitle1,
						                          pltfolder, plotparam,
						                          reportpath, radarpos);
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_ypos[0].size() != 0)
				{
					plotparam = "ypos";
					titleinfomap[0] = "RL tracker ypos";
					titleinfomap[1] = "RR tracker ypos";
					titleinfomap[2] = "FR tracker ypos";
					titleinfomap[3] = "FL tracker ypos";
					titleinfomap[5] = "FC tracker ypos";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_ypos[0],
													(sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_ypos[1],
													filecount, f_name, plottitle1,
													pltfolder, plotparam,
													reportpath, radarpos);
				}


				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xvel[0].size() != 0)
				{
					plotparam = "xvel";
					titleinfomap[0] = "RL tracker xvel";
					titleinfomap[1] = "RR tracker xvel";
					titleinfomap[2] = "FR tracker xvel";
					titleinfomap[3] = "FL tracker xvel";
					titleinfomap[5] = "FC tracker xvel";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xvel[0],
													(sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xvel[1],
													filecount, f_name, plottitle1,
													pltfolder, plotparam,
													reportpath, radarpos);
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yvel[0].size() != 0)
				{
					plotparam = "yvel";
					titleinfomap[0] = "RL tracker yvel";
					titleinfomap[1] = "RR tracker yvel";
					titleinfomap[2] = "FR tracker yvel";
					titleinfomap[3] = "FL tracker yvel";
					titleinfomap[5] = "FC tracker yvel";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yvel[0],
						                          (sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yvel[1],
													filecount, f_name, plottitle1,
													pltfolder, plotparam,
													reportpath, radarpos);
				}


				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xacc[0].size() != 0)
				{
					plotparam = "xacc";
					titleinfomap[0] = "RL tracker xacc";
					titleinfomap[1] = "RR tracker xacc";
					titleinfomap[2] = "FR tracker xacc";
					titleinfomap[3] = "FL tracker xacc";
					titleinfomap[5] = "FC tracker xacc";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xacc[0],
													(sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_xacc[1],
													filecount, f_name, plottitle1,
													pltfolder, plotparam,
													reportpath, radarpos);
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yacc[0].size() != 0)
				{
					plotparam = "yacc";
					titleinfomap[0] = "RL tracker yacc";
					titleinfomap[1] = "RR tracker yacc";
					titleinfomap[2] = "FR tracker yacc";
					titleinfomap[3] = "FL tracker yacc";
					titleinfomap[5] = "FC tracker yacc";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					adapter_forscaled_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yacc[0],
													(sptr_plotparam_plotadapter.get() + radarpos)->map_vcs_yacc[1],
													filecount, f_name, plottitle1,
													pltfolder, plotparam,
													reportpath, radarpos);
				}

			}

			
			
		}

		for (auto& radarpos : mset_radarposition)
		{
			(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1].clear();
		}
		
	}
	

	if (jsonparser.bscanindex_scaling == false)
	{
		std::cout << "\n Non scaling plot enabled";
		for (auto& radarpos : mset_radarposition)
		{
			string plottitle1;
			std::map<int, string> titleinfomap;
			/*string plottitle1;
			std::map<int, string> titleinfomap;
			matplotnonscale.PrintReportInfoinHTML(f_name, "html", pltfolder, reportpath, radarpos);
			//-----------------------------------------C2 Timing Info--------------------------------------------------------//
			titleinfomap[0] = "RL c2 timing info[9]";
			titleinfomap[1] = "RR c2 timing info[9]";
			titleinfomap[2] = "FR c2 timing info[9]";
			titleinfomap[3] = "FL c2 timing info[9]";
			titleinfomap[5] = "FC c2 timing info[9]";
			for (auto val : titleinfomap)
			{
				if (val.first == radarpos)
				{
					plottitle1 = val.second;
				}
			}
			matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex[0], 
				                                           (sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0],
				                                           (sptr_plotparam_plotadapter.get() + radarpos)->scanindex[1], 
				                                           (sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1],
				                                            filecount, f_name, plottitle1.c_str(), pltfolder, "c2 timing info", reportpath, radarpos);

			(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->c2timinginfo_index23[1].clear();*/

			//----------------------------------------- Detection Plots --------------------------------------------------------//
			if (/*jsonparser.bdetectionplot*/1)
			{
				//-----------------------------------------Speed--------------------------------------------------------//
				/*titleinfomap[0] = "rl vehicle info speed";
				titleinfomap[1] = "rr vehicle info speed";
				titleinfomap[2] = "fr vehicle info speed";
				titleinfomap[3] = "fl vehicle info speed";
				titleinfomap[5] = "fc vehicle info speed";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->scanidex[1], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "speed km/hr", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->abs_speed[1].clear();

				//-----------------------------------------Yaw Rate--------------------------------------------------------//
				titleinfomap[0] = "rl vehicle info yawrate";
				titleinfomap[1] = "rr vehicle info yawrate";
				titleinfomap[2] = "fr vehicle info yawrate";
				titleinfomap[3] = "fl vehicle info yawrate";
				titleinfomap[5] = "fc vehicle info yawrate";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->scanidex[1], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "yaw rate", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->yaw_rate[1].clear();*/

				//-----------------------------------------Valid Detection Count--------------------------------------------------------//
				/*titleinfomap[0] = "RL Valid  Detection Count";
				titleinfomap[1] = "RR Valid  Detection Count";
				titleinfomap[2] = "FR Valid  Detection Count";
				titleinfomap[3] = "FL Valid  Detection Count";
				titleinfomap[5] = "FC Valid  Detection Count";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh_ref_scale, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->si_resim_ref_scale, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "valid detection count", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->valid_detection_count[1].clear();

				//-----------------------------------------Maximum Range--------------------------------------------------------//
				titleinfomap[0] = "RL Max Range  ";
				titleinfomap[1] = "RR Max Range  ";
				titleinfomap[2] = "FR Max Range  ";
				titleinfomap[3] = "FL Max Range  ";
				titleinfomap[5] = "FC Max Range  ";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[0], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->range_max[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[1], 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->range_max[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "max range value", reportpath, radarpos);

				(sptr_plotparam_plotadapter.get() + radarpos)->range_max[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->range_max[1].clear();


				//-----------------------------------------Range Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL Range Histogram  ";
				titleinfomap[1] = "RR Range Histogram ";
				titleinfomap[2] = "FR Range Histogram ";
				titleinfomap[3] = "FL Range Histogram ";
				titleinfomap[5] = "FC Range Histogram ";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->range[0],
					                                        (sptr_plotparam_plotadapter.get() + radarpos)->range[1],
					                                         filecount, f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);


				//-----------------------------------------Range Rate Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL RangeRate Histogram  ";
				titleinfomap[1] = "RR RangeRate Histogram ";
				titleinfomap[2] = "FR RangeRate Histogram ";
				titleinfomap[3] = "FL RangeRate Histogram ";
				titleinfomap[5] = "FC RangeRate Histogram ";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0],
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->range_rate[1],
					                                          filecount, f_name, plottitle1.c_str(), pltfolder, "rangerate", reportpath, radarpos);



				//-----------------------------------------Azimuthal Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL Azimuth Histogram";
				titleinfomap[1] = "RR Azimuth Histogram";
				titleinfomap[2] = "FR Azimuth Histogram";
				titleinfomap[3] = "FL Azimuth Histogram";
				titleinfomap[5] = "FC Azimuth Histogram";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0], 
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->azimuth[1],
					                                         filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth", reportpath, radarpos);



				//-----------------------------------------Elevation Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL Elevation Histogram ";
				titleinfomap[1] = "RR Elevation Histogram";
				titleinfomap[2] = "FR Elevation Histogram";
				titleinfomap[3] = "FL Elevation Histogram";
				titleinfomap[5] = "FC Elevation Histogram";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->elevation[0], 
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->elevation[1],
					                                         filecount, f_name, plottitle1.c_str(), pltfolder, "elevation", reportpath, radarpos);

				//-----------------------------------------RCS Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL RCS Histogram";
				titleinfomap[1] = "RR RCS Histogram";
				titleinfomap[2] = "FR RCS Histogram";
				titleinfomap[3] = "FL RCS Histogram";
				titleinfomap[5] = "FC RCS Histogram";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->rcs[0], 
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->rcs[1],
					                                         filecount, f_name, plottitle1.c_str(), pltfolder, "rcs", reportpath, radarpos);

				//-----------------------------------------Amplitude Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL Amplitude Histogram";
				titleinfomap[1] = "RR Amplitude Histogram";
				titleinfomap[2] = "FR Amplitude Histogram";
				titleinfomap[3] = "FL Amplitude Histogram";
				titleinfomap[5] = "FC Amplitude Histogram";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->amp[0],
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->amp[1],
					                                          filecount, f_name, plottitle1.c_str(), pltfolder, "amplitude", reportpath, radarpos);

				//-----------------------------------------SNR Histogram--------------------------------------------------------//
				titleinfomap[0] = "RL SNR Histogram ";
				titleinfomap[1] = "RR SNR Histogram";
				titleinfomap[2] = "FR SNR Histogram";
				titleinfomap[3] = "FL SNR Histogram";
				titleinfomap[5] = "FC SNR Histogram";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_histogram((sptr_plotparam_plotadapter.get() + radarpos)->snr[0], 
					                                         (sptr_plotparam_plotadapter.get() + radarpos)->snr[1],
					                                          filecount, f_name, plottitle1.c_str(), pltfolder, "snr", reportpath, radarpos);*/

				//-----------------------------------------Range Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->range[0].size() != 0)
				{
					titleinfomap[0] = "RL Detection range";
					titleinfomap[1] = "RR Detection range";
					titleinfomap[2] = "FR Detection range";
					titleinfomap[3] = "FL Detection range";
					titleinfomap[5] = "FC Detection range";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}


					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->range[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->range[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "range", reportpath, radarpos);
				}

			/*	std::cout << "\n-------------Range (Match + Mismatch)-------------radar------" << radarpos;
				std::cout << "\n (sptr_plotparam_plotadapter.get() + radarpos)->si_veh.size() " << (sptr_plotparam_plotadapter.get() + radarpos)->si_veh.size();
				std::cout << "\n (sptr_plotparam_plotadapter.get() + radarpos)->range[0].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->range[0].size();
				std::cout << "\n (sptr_plotparam_plotadapter.get() + radarpos)->range[1].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->range[1].size();*/

				



				//-----------------------------------------Range Mismatch Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0].size() != 0 
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_range[1].size() != 0)
				{
					/*adapter_for_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0],
															  (sptr_plotparam_plotadapter.get() + radarpos)->map_range[1],
															   filecount, f_name, plottitle1.c_str(),
															   pltfolder, "range",
															   reportpath, radarpos,
															   .2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->range[0].size());*/

					titleinfomap[0] = "RL Detection Mismatch range ";
					titleinfomap[1] = "RR Detection Mismatch range";
					titleinfomap[2] = "FR Detection Mismatch range";
					titleinfomap[3] = "FL Detection Mismatch range";
					titleinfomap[5] = "FC Detection Mismatch range";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_range[0],
																(sptr_plotparam_plotadapter.get() + radarpos)->map_range[1],
																filecount, f_name, plottitle1.c_str(),
																pltfolder, " Range",
																reportpath, radarpos,
																.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->range[0].size());
				}


				(sptr_plotparam_plotadapter.get() + radarpos)->range[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->range[1].clear();


				//-----------------------------------------Range Rate Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0].size() != 0)
				{
					titleinfomap[0] = "RL Detection range rate";
					titleinfomap[1] = "RR Detection range rate";
					titleinfomap[2] = "FR Detection range rate";
					titleinfomap[3] = "FL Detection range rate";
					titleinfomap[5] = "FC Detection range rate";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
																	(sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0],
																	(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
																	(sptr_plotparam_plotadapter.get() + radarpos)->range_rate[1],
																	filecount, f_name, plottitle1.c_str(), pltfolder, "rangerate", reportpath, radarpos);


				}

				//-----------------------------------------Range Rate Mismatch Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[1].size() != 0)
				{

					titleinfomap[0] = "RL Detection Mismatch range rate ";
					titleinfomap[1] = "RR Detection Mismatch range rate";
					titleinfomap[2] = "FR Detection Mismatch range rate";
					titleinfomap[3] = "FL Detection Mismatch range rate";
					titleinfomap[5] = "FC Detection Mismatch range rate";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[0],
																	(sptr_plotparam_plotadapter.get() + radarpos)->map_rangerate[1],
																	filecount, f_name, plottitle1.c_str(),
																	pltfolder, "Range rate",
																	reportpath, radarpos,
																	.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0].size());
				}
				(sptr_plotparam_plotadapter.get() + radarpos)->range_rate[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->range_rate[1].clear();
				//-----------------------------------------Azimuth Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0].size() != 0)
				{


				titleinfomap[0] = "RL Detection azimuth";
				titleinfomap[1] = "RR Detection azimuth";
				titleinfomap[2] = "FR Detection azimuth";
				titleinfomap[3] = "FL Detection azimuth";
				titleinfomap[5] = "FC Detection azimuth";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->si_resim, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->azimuth[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth", reportpath, radarpos);


				}
				//-----------------------------------------Azimuth Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[1].size() != 0)
				{

					titleinfomap[0] = "RL Azimuth Mismatch  ";
					titleinfomap[1] = "RR Azimuth Mismatch  ";
					titleinfomap[2] = "FR Azimuth Mismatch  ";
					titleinfomap[3] = "FL Azimuth Mismatch  ";
					titleinfomap[5] = "FC Azimuth Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_azimuth[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Azimuth",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0].size());
				}
				(sptr_plotparam_plotadapter.get() + radarpos)->azimuth[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->azimuth[1].clear();


				//-----------------------------------------Azimuth confidense Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->az_conf[0].size() != 0)
				{
					titleinfomap[0] = "RL Detection azimuth Confidence";
					titleinfomap[1] = "RR Detection azimuth Confidence";
					titleinfomap[2] = "FR Detection azimuth Confidence";
					titleinfomap[3] = "FL Detection azimuth Confidence";
					titleinfomap[5] = "FC Detection azimuth Confidence";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->az_conf[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->az_conf[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "azimuth confi", reportpath, radarpos);

					(sptr_plotparam_plotadapter.get() + radarpos)->az_conf[0].clear();
					(sptr_plotparam_plotadapter.get() + radarpos)->az_conf[1].clear();
				}


				//-----------------------------------------Elevation Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->elevation[0].size()!=0)
				{
					titleinfomap[0] = "RL Detection elevation";
					titleinfomap[1] = "RR Detection elevation";
					titleinfomap[2] = "FR Detection elevation";
					titleinfomap[3] = "FL Detection elevation";
					titleinfomap[5] = "FC Detection elevation";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->elevation[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->elevation[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "elevation", reportpath, radarpos);


				}

				//-----------------------------------------Elevation Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[1].size() != 0)
				{

					titleinfomap[0] = "RL Elevation Mismatch  ";
					titleinfomap[1] = "RR Elevation Mismatch  ";
					titleinfomap[2] = "FR Elevation Mismatch  ";
					titleinfomap[3] = "FL Elevation Mismatch  ";
					titleinfomap[5] = "FC Elevation Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_elevation[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Elevation",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->elevation[0].size());
				}

				(sptr_plotparam_plotadapter.get() + radarpos)->elevation[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->elevation[1].clear();


				//-----------------------------------------Elevation Confidense Scatter--------------------------------------------------------//
				if ((sptr_plotparam_plotadapter.get() + radarpos)->el_conf[0].size() != 0)
				{
					titleinfomap[0] = "RL Detection elevation Confidense";
					titleinfomap[1] = "RR Detection elevation Confidense";
					titleinfomap[2] = "FR Detection elevation Confidense";
					titleinfomap[3] = "FL Detection elevation Confidense";
					titleinfomap[5] = "FC Detection elevation Confidense";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->el_conf[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->el_conf[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "elevation confidense", reportpath, radarpos);

					(sptr_plotparam_plotadapter.get() + radarpos)->el_conf[0].clear();
					(sptr_plotparam_plotadapter.get() + radarpos)->el_conf[1].clear();

				}

				//-----------------------------------------RCS Scatter--------------------------------------------------------// 
				if ((sptr_plotparam_plotadapter.get() + radarpos)->rcs[0].size()!=0) {
					titleinfomap[0] = "RL Detection rcs";
					titleinfomap[1] = "RR Detection rcs";
					titleinfomap[2] = "FR Detection rcs";
					titleinfomap[3] = "FL Detection rcs";
					titleinfomap[5] = "FC Detection rcs";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->rcs[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->rcs[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "rcs", reportpath, radarpos);


				}

				//-----------------------------------------RCS Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[1].size() != 0)
				{

					titleinfomap[0] = "RL RCS Mismatch  ";
					titleinfomap[1] = "RR RCS Mismatch  ";
					titleinfomap[2] = "FR RCS Mismatch  ";
					titleinfomap[3] = "FL RCS Mismatch  ";
					titleinfomap[5] = "FC RCS Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_rcs[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "RCS",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->rcs[0].size());
				}

				(sptr_plotparam_plotadapter.get() + radarpos)->rcs[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->rcs[1].clear();
				//-----------------------------------------Amplitude Scatter--------------------------------------------------------//  
				if ((sptr_plotparam_plotadapter.get() + radarpos)->amp[0].size()!=0)
				{
					titleinfomap[0] = "RL Detection amplitude";
					titleinfomap[1] = "RR Detection amplitude";
					titleinfomap[2] = "FR Detection amplitude";
					titleinfomap[3] = "FL Detection amplitude";
					titleinfomap[5] = "FC Detection amplitude";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}


				matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->amp[0],
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->si_resim, 
					                                           (sptr_plotparam_plotadapter.get() + radarpos)->amp[1],
					                                            filecount, f_name, plottitle1.c_str(), pltfolder, "amp", reportpath, radarpos);


				}

				//-----------------------------------------Amplitude Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_amp[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_amp[1].size() != 0)
				{

					titleinfomap[0] = "RL Amplitude Mismatch  ";
					titleinfomap[1] = "RR Amplitude Mismatch  ";
					titleinfomap[2] = "FR Amplitude Mismatch  ";
					titleinfomap[3] = "FL Amplitude Mismatch  ";
					titleinfomap[5] = "FC Amplitude Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}

					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_amp[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_amp[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "Amp",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->amp[0].size());

				}
				(sptr_plotparam_plotadapter.get() + radarpos)->amp[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->amp[1].clear();
				//-----------------------------------------SNR Scatter--------------------------------------------------------//  
				if ((sptr_plotparam_plotadapter.get() + radarpos)->snr[0].size()!=0)
				{
					titleinfomap[0] = "RL Detection snr";
					titleinfomap[1] = "RR Detection snr";
					titleinfomap[2] = "FR Detection snr";
					titleinfomap[3] = "FL Detection snr";
					titleinfomap[5] = "FC Detection snr";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->si_veh,
						(sptr_plotparam_plotadapter.get() + radarpos)->snr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->si_resim,
						(sptr_plotparam_plotadapter.get() + radarpos)->snr[1],
						filecount, f_name, plottitle1.c_str(), pltfolder, "snr", reportpath, radarpos);

				}

				//-----------------------------------------Amplitude Mismatch Scatter--------------------------------------------------------//

				if ((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0].size() != 0
					&& (sptr_plotparam_plotadapter.get() + radarpos)->map_snr[1].size() != 0)
				{

					titleinfomap[0] = "RL SNR Mismatch  ";
					titleinfomap[1] = "RR SNR Mismatch  ";
					titleinfomap[2] = "FR SNR Mismatch  ";
					titleinfomap[3] = "FL SNR Mismatch  ";
					titleinfomap[5] = "FC SNR Mismatch  ";
					for (auto val : titleinfomap)
					{
						if (val.first == radarpos)
						{
							plottitle1 = val.second;
						}
					}
					calculate_data_mismatch_property_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->map_snr[0],
						(sptr_plotparam_plotadapter.get() + radarpos)->map_snr[1],
						filecount, f_name, plottitle1.c_str(),
						pltfolder, "SNR",
						reportpath, radarpos,
						.2, radarpos, (sptr_plotparam_plotadapter.get() + radarpos)->snr[0].size());
				}
				(sptr_plotparam_plotadapter.get() + radarpos)->snr[0].clear();
				(sptr_plotparam_plotadapter.get() + radarpos)->snr[1].clear();

				
			
			
			}


			


			//-----------------------------------------Tracker Plots--------------------------------------------------------//

			if (/*jsonparser.btrackerplot */ 1)
			{
				std::cout << "\n tracker plot enaled (non scaled)";
				//-----------------------------------------xpos Scatter--------------------------------------------------------//  
				titleinfomap[0] = "RL tracker info xposition";
				titleinfomap[1] = "RR tracker info xposition";
				titleinfomap[2] = "FR tracker info xposition";
				titleinfomap[3] = "FL tracker info xposition";
				titleinfomap[5] = "FC tracker info xposition";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				//std::cout << "\nsize of (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xpos[0].size() " << (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xpos[0].size();
				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_xpos[0].size()!=0)
				{

					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh,
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xpos[0],
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xpos[1],
						                                            filecount, f_name, plottitle1.c_str(), pltfolder, "XPOS", reportpath, radarpos);
				}
				

				//-----------------------------------------ypos Scatter--------------------------------------------------------//
				titleinfomap[0] = "RL tracker info yposition";
				titleinfomap[1] = "RR tracker info yposition";
				titleinfomap[2] = "FR tracker info yposition";
				titleinfomap[3] = "FL tracker info yposition";
				titleinfomap[5] = "FC tracker info yposition";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_ypos[0].size()!=0)
				{
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_ypos[0],
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_ypos[1],
						                                           filecount, f_name, plottitle1.c_str(), pltfolder, "YPOS", reportpath, radarpos);
				}
				

				//-----------------------------------------xvelocity Scatter--------------------------------------------------------//
				titleinfomap[0] = "RL tracker info xvelocity";
				titleinfomap[1] = "RR tracker info xvelocity";
				titleinfomap[2] = "FR tracker info xvelocity";
				titleinfomap[3] = "FL tracker info xvelocity";
				titleinfomap[5] = "FC tracker info xvelocity";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_xvel[0].size()!=0)
				{
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xvel[0],
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xvel[1],
						                                            filecount, f_name, plottitle1.c_str(), pltfolder, "XVEL", reportpath, radarpos);
				}
				

				//-----------------------------------------yvelocity Scatter--------------------------------------------------------//
				titleinfomap[0] = "RL tracker info yvelocity";
				titleinfomap[1] = "RR tracker info yvelocity";
				titleinfomap[2] = "FR tracker info yvelocity";
				titleinfomap[3] = "FL tracker info yvelocity";
				titleinfomap[5] = "FC tracker info yvelocity";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_yvel[0].size()!=0)
				{
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_yvel[0],
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_yvel[1],
						                                           filecount, f_name, plottitle1.c_str(), pltfolder, "YVEL", reportpath, radarpos);

				}
				

				//-----------------------------------------xacceleration Scatter--------------------------------------------------------//
				titleinfomap[0] = "RL tracker info xacceleration";
				titleinfomap[1] = "RR tracker info xacceleration";
				titleinfomap[2] = "FR tracker info xacceleration";
				titleinfomap[3] = "FL tracker info xacceleration";
				titleinfomap[5] = "FC tracker info xacceleration";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}

				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_xacc[0].size()!=0)
				{
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xacc[0],
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                           (sptr_plotparam_plotadapter.get() + radarpos)->vsc_xacc[1],
						                                           filecount, f_name, plottitle1.c_str(), pltfolder, "XACC", reportpath, radarpos);
				}
				

				//-----------------------------------------yacceleration Scatter--------------------------------------------------------//
				titleinfomap[0] = "RL tracker info yacceleration";
				titleinfomap[1] = "RR tracker info yacceleration";
				titleinfomap[2] = "FR tracker info yacceleration";
				titleinfomap[3] = "FL tracker info yacceleration";
				titleinfomap[5] = "FC tracker info yacceleration";
				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}


				if ((sptr_plotparam_plotadapter.get() + radarpos)->vsc_yacc[0].size()!=0)
				{
					matplotnonscale.generate_detection_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_veh, 
						                                            (sptr_plotparam_plotadapter.get() + radarpos)->vsc_yacc[0],
						                                            (sptr_plotparam_plotadapter.get() + radarpos)->vcs_si_resim, 
						                                            (sptr_plotparam_plotadapter.get() + radarpos)->vsc_yacc[1],
						                                            filecount, f_name, plottitle1.c_str(), pltfolder, "YACC", reportpath, radarpos);
				}
				
			}

			//-----------------------------------------FF Alerts Scatter--------------------------------------------------------//

			if ((sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_left[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_left[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info BswAlert Left";
				titleinfomap[1] = "RR Feature info BswAlert Left";
				titleinfomap[2] = "FR Feature info BswAlert Left";
				titleinfomap[3] = "FL Feature info BswAlert Left";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->bsw_left_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_left[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->bsw_left_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_left[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "BSWLEFT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_right[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_right[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info BswAlert Right";
				titleinfomap[1] = "RR Feature info BswAlert Right";
				titleinfomap[2] = "FR Feature info BswAlert Right";
				titleinfomap[3] = "FL Feature info BswAlert Right";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->bsw_right_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_right[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->bsw_right_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->bsw_alert_right[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "BSWRIGHT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_left[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_left[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CvwAlert Left";
				titleinfomap[1] = "RR Feature info CvwAlert Left";
				titleinfomap[2] = "FR Feature info CvwAlert Left";
				titleinfomap[3] = "FL Feature info CvwAlert Left";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->cvw_left_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_left[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->cvw_left_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_left[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CVWLEFT", reportpath, radarpos);
			}
			if ((sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_right[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_right[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CvwAlert Right";
				titleinfomap[1] = "RR Feature info CvwAlert Right";
				titleinfomap[2] = "FR Feature info CvwAlert Right";
				titleinfomap[3] = "FL Feature info CvwAlert Right";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->cvw_right_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_right[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->cvw_right_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->cvw_alert_right[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CVWRIGHT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_left[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_left[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CtaAlert Left";
				titleinfomap[1] = "RR Feature info CtaAlert Left";
				titleinfomap[2] = "FR Feature info CtaAlert Left";
				titleinfomap[3] = "FL Feature info CtaAlert Left";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->cta_left_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_left[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->cta_left_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_left[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CTALEFT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_right[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_right[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CtaAlert Right";
				titleinfomap[1] = "RR Feature info CtaAlert Right";
				titleinfomap[2] = "FR Feature info CtaAlert Right";
				titleinfomap[3] = "FL Feature info CtaAlert Right";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->cta_right_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_right[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->cta_right_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->cta_alert_right[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CTARIGHT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_left[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_left[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CedAlert Left";
				titleinfomap[1] = "RR Feature info CedAlert Left";
				titleinfomap[2] = "FR Feature info CedAlert Left";
				titleinfomap[3] = "FL Feature info CedAlert Left";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->ced_left_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_left[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->ced_left_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_left[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CEDLEFT", reportpath, radarpos);
			}

			if ((sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_right[0].size() != 0 || (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_right[1].size() != 0)
			{
				titleinfomap[0] = "RL Feature info CedAlert Right";
				titleinfomap[1] = "RR Feature info CedAlert Right";
				titleinfomap[2] = "FR Feature info CedAlert Right";
				titleinfomap[3] = "FL Feature info CedAlert Right";

				for (auto val : titleinfomap)
				{
					if (val.first == radarpos)
					{
						plottitle1 = val.second;
					}
				}
				matplotnonscale.generate_features_scatterplot((sptr_plotparam_plotadapter.get() + radarpos)->ced_right_si[0], (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_right[0],
					(sptr_plotparam_plotadapter.get() + radarpos)->ced_right_si[1], (sptr_plotparam_plotadapter.get() + radarpos)->ced_alert_right[1],
					filecount, f_name, plottitle1.c_str(), pltfolder, "CEDRIGHT", reportpath, radarpos);
			}
		}


		for (auto& radarpos : mset_radarposition)
		{
			(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->scanidex[1].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->scanindex_maxrange[1].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->si_veh.clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->si_resim.clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->alignment_scanidex[1].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[0].clear();
			(sptr_plotparam_plotadapter.get() + radarpos)->mnr_scanidex[1].clear();
		}
		


	}
}

inline void MatplotAdapter::remove_old_html_reports(const char* f_name, const char* pltfolder, std::string reportpath)
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

void MatplotAdapter::adapter_forscaled_scatterplot(std::map<double, std::vector<double>>&map_property_f1,
	                                               std::map<double, std::vector<double>>& map_property_f2,
	                                               int filecount, const char* f_name, std::string plottitle,
	                                               const char* foldname, std::string plotparam,
	                                               std::string reportpath, int radarPosition)
{

	//std::cout << "\n MatplotAdapter::adapter_forscaled_scatterplot radar "<< radarPosition<<" "<< plottitle;
	
	MatplotScale& matplotscale = MatplotScale::getInstance();
	
	std::map<double, std::vector<double>> map_prop_f1;
	std::map<double, std::vector<double>> map_prop_f2;

	map_prop_f1.clear();
	map_prop_f2.clear();

	map_prop_f1 = std::move(map_property_f1);
	map_prop_f2 = std::move(map_property_f2);

	//std::cout << "\n map_prop_f1.size()" << map_prop_f1.size();
	//std::cout << "\n map_prop_f2.size()" << map_prop_f2.size();

	std::map<double, double> map_start_end_scanindexf1;
	map_start_end_scanindexf1.clear();
	
	std::vector<double> vec_scanindex_ref;
	vec_scanindex_ref.clear();
	std::size_t scanindex_scaling_interval{ 0 }; 

	if (map_prop_f1.size() > map_prop_f2.size())
	{
		scanindex_scaling_interval = map_prop_f2.size() / 10;
		for (auto& val : map_prop_f2)
		{
			vec_scanindex_ref.push_back(val.first);
		}
	}
	else if (map_prop_f2.size() > map_prop_f1.size())
	{
		scanindex_scaling_interval = map_prop_f1.size() / 10;
		for (auto& val : map_prop_f1)
		{
			vec_scanindex_ref.push_back(val.first);
		}
	}
	else if (map_prop_f2.size() == map_prop_f1.size())
	{
		scanindex_scaling_interval = map_prop_f1.size() / 10;
		for (auto& val : map_prop_f1)
		{
			vec_scanindex_ref.push_back(val.first);
		}
	}


	int index = 0;		
	std::size_t plotimageindex_scaling = 0;
	

	for (auto& val : vec_scanindex_ref)
	{		
		if (index < vec_scanindex_ref.size())
		{
			if ((vec_scanindex_ref.size())- index>= scanindex_scaling_interval)
			{
				map_start_end_scanindexf1[vec_scanindex_ref[index]] = vec_scanindex_ref[index] + scanindex_scaling_interval;
				index = getindex(vec_scanindex_ref, vec_scanindex_ref[index]);
				index = index + scanindex_scaling_interval;
				//std::cout << "\n index "<<index;
			}		
		}
	}
	
	//std::cout << "\n map_start_end_scanindexf1.size() " << map_start_end_scanindexf1.size();

	std::vector<double> xvaluef1, yvaluef1, xvaluef2, yvaluef2;
	for (auto& val : map_start_end_scanindexf1)
	{
		xvaluef1.clear();
		yvaluef1.clear();
		xvaluef2.clear();
		yvaluef2.clear();
	
		//---------------------------scaling data for file1-with refeance to f1scale index----------------------------------//
			for (auto it = map_prop_f1.find(val.first); it != map_prop_f1.find(val.second); it++)
			{
				
				for (int i = 0; i < it->second.size(); i++)
				{
					xvaluef1.push_back(it->first);
				}
				for (auto& value_range : it->second)
				{
					yvaluef1.push_back(value_range);
				}

			}


			//---------------------------scaling data for file2-with refeance to f1scale index----------------------------------//
			for (auto it = map_prop_f2.find(val.first); it != map_prop_f2.find(val.second); it++)
			{				
				for (int i = 0; i < it->second.size(); i++)
				{
					xvaluef2.push_back(it->first);
				}
				for (auto& value_range : it->second)
				{
					yvaluef2.push_back(value_range);
				}		

			}


		/*	std::cout << "\n xvaluef1.size() " << xvaluef1.size();
			std::cout << "\n yvaluef1.size() " << yvaluef1.size();
			std::cout << "\n xvaluef2.size() " << xvaluef2.size();
			std::cout << "\n yvaluef2.size() " << yvaluef2.size();*/

			matplotscale.generate_scaled_scatterplot(xvaluef1,
													yvaluef1,
													xvaluef2,
													yvaluef2,
													plotimageindex_scaling, f_name, plottitle,
													foldname, plotparam,
													reportpath, radarPosition);


		/*	std::cout << "\n After gen xvaluef1.size() " << xvaluef1.size();
			std::cout << "\n After gen yvaluef1.size() " << yvaluef1.size();
			std::cout << "\n After gen xvaluef2.size() " << xvaluef2.size();
			std::cout << "\n After gen yvaluef2.size() " << yvaluef2.size();*/

			
			plotimageindex_scaling++;

		
	}

	
	map_prop_f1.clear();
	map_prop_f2.clear();
	
}

void MatplotAdapter::adapter_for_mismatch_property_scatterplot(std::map<double, std::vector<double>> map_property_f1, 
	                                                           std::map<double, std::vector<double>> map_property_f2, 
	                                                           int filecount, const char* f_name, std::string plottitle, 
	                                                           const char* foldname, std::string plotparam, 
	                                                           std::string reportpath, int radarPosition, int tolerance,int radarpos, int property_file1_count)
{
	string plottitle1;
	std::map<int, string> titleinfomap;

	MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

	//std::cout << "\n adapter_for_mismatch_property_scatterplot " << property_file1_count;
	if (1)
	{
		titleinfomap[0] = "RL Mismatch "+ plotparam;
		titleinfomap[1] = "RR Mismatch "+ plotparam;
		titleinfomap[2] = "FR Mismatch "+ plotparam;
		titleinfomap[3] = "FL Mismatch "+ plotparam;
		titleinfomap[5] = "FC Mismatch "+ plotparam;
		for (auto val : titleinfomap)
		{
			if (val.first == radarpos)
			{
				plottitle1 = val.second;
			}
		}

		for (auto map_range_file0 : map_property_f1)
		{

			auto file1_resim_itr = map_property_f2.find(map_range_file0.first);

			if (file1_resim_itr != map_property_f2.end())
			{
				vec_property_f0_veh = map_range_file0.second;
				vec_property_f1_resim = file1_resim_itr->second;

				//sort vehcile and resim data
				std::sort(vec_property_f0_veh.begin(), vec_property_f0_veh.end());
				std::sort(vec_property_f1_resim.begin(), vec_property_f1_resim.end());

				//Normalizing range vector of vehicle data and resim data
				//case2: vec_range_f0_veh > vec_range_f1_resim (missing range in resim)
				if (vec_property_f0_veh.size() > vec_property_f1_resim.size())
				{
					auto diff = vec_property_f0_veh.size() - vec_property_f1_resim.size();

					for (int i = 0; i < diff; i++)
					{
						vec_property_f1_resim.push_back(0);
					}

				}



			//	if (vec_property_f0_veh.size() == vec_property_f1_resim.size())
			//	{
					if (std::equal(vec_property_f0_veh.begin(), vec_property_f0_veh.end(), vec_property_f1_resim.begin()))
					{

						continue;
					}
					else
					{
						auto range_mismatchpair = std::mismatch(vec_property_f0_veh.begin(), vec_property_f0_veh.end(), vec_property_f1_resim.begin());

					vec_property_mismatch[0].push_back(*range_mismatchpair.first);
					vec_property_mismatch[1].push_back(*range_mismatchpair.second);
					vec_si_property_mismatch.push_back(map_range_file0.first);

					for (; range_mismatchpair.first != vec_property_f0_veh.end() && range_mismatchpair.second != vec_property_f1_resim.end();)
					{
						++range_mismatchpair.first;
						++range_mismatchpair.second;

						if ((range_mismatchpair.first != vec_property_f0_veh.end() && range_mismatchpair.second != vec_property_f1_resim.end()))
						{
							range_mismatchpair = std::mismatch(range_mismatchpair.first, vec_property_f0_veh.end(), range_mismatchpair.second);
							if ((range_mismatchpair.first != vec_property_f0_veh.end() && range_mismatchpair.second != vec_property_f1_resim.end()))
							{
								//std::cout << "\n" << *range_mismatchpair.first << " " << *range_mismatchpair.second;
								vec_property_mismatch[0].push_back(*range_mismatchpair.first);
								vec_property_mismatch[1].push_back(*range_mismatchpair.second);
								vec_si_property_mismatch.push_back(map_range_file0.first);
							}
						}


						}
				//	}
					


				}
			}

		}

		//vec_si_range_mismatch
		std::vector<double> vec_mismatch_file1, vec_mismatch_file2, vec_mismatch_si, vec_diff;

		std::transform(begin(vec_property_mismatch[0]), end(vec_property_mismatch[0]),
			begin(vec_property_mismatch[1]), std::back_inserter(vec_diff),
			[&](double value_f1, double value_f2) {

				if (abs(value_f1 - value_f2) >= 0.2)//tolerance should be calibration
				{
					vec_mismatch_file1.push_back(value_f1);
					vec_mismatch_file2.push_back(value_f2);

					int index;
					auto itr = std::find(begin(vec_property_mismatch[0]), end(vec_property_mismatch[0]), value_f1);
					if (itr != vec_property_mismatch[0].end())
					{
						index = itr - vec_property_mismatch[0].begin();
						vec_mismatch_si.push_back(vec_si_property_mismatch.at(index));
					}

					return abs(value_f1 - value_f2);

				}

			});




		matplotnonscale.generate_detection_scatterplot(vec_mismatch_si,
														vec_mismatch_file1,
														vec_mismatch_si,
														vec_mismatch_file2,
														filecount, f_name, plottitle1.c_str(),
														foldname, "mismatch", reportpath, radarpos);



		/*std::cout << "\n-------------Mismatch-------------radar------" << radarpos;
		std::cout << "\n vec_mismatch_si.size() " << vec_mismatch_si.size();
		std::cout << "\n vec_mismatch_file1.size() " << vec_mismatch_file1.size();
		std::cout << "\n property_file1_count.size() " << property_file1_count;
		std::cout << "\n vec_mismatch_file2.size() " << vec_mismatch_file2.size();
		std::cout << "\n---------------------------------------------" << radarpos;*/

		


		titleinfomap[0] = "RL"+ plotparam +  " mismatch match  pie";
		titleinfomap[1] = "RR" + plotparam + " mismatch match  pie";
		titleinfomap[2] = "FR" + plotparam + " mismatch match  pie";
		titleinfomap[3] = "FL" + plotparam + " mismatch match  pie";
		titleinfomap[5] = "FC" + plotparam + " mismatch match  pie";
		for (auto val : titleinfomap)
		{
			if (val.first == radarpos)
			{
				plottitle1 = val.second;
			}
		}

		calculate_match_mismatch_percentage(property_file1_count, vec_mismatch_file1.size(),
											filecount, f_name, plottitle1.c_str(),
											foldname, "mismatch match", reportpath, radarpos);

		vec_mismatch_file1.clear();
		vec_mismatch_file2.clear();
		vec_mismatch_si.clear();
		vec_diff.clear();
		
		vec_si_property_mismatch.clear();
		vec_property_mismatch[0].clear();
		vec_property_mismatch[1].clear();




	}
}

void MatplotAdapter::calculate_data_mismatch_property_scatterplot(std::map<double, std::vector<double>> map_property_f1,
	std::map<double, std::vector<double>> map_property_f2,
	int filecount, const char* f_name, std::string plottitle,
	const char* foldname, std::string plotparam, std::string reportpath,
	int radarPosition, int tolerance, int radarpos, int property_file1_count)
{


	MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

	std::vector<double>  diff, scanindex, differencevalue;
	string plottitle1;
	std::map<int, string> titleinfomap;

	
	diff.clear();
	scanindex.clear();
	differencevalue.clear();

	
	    file1 <<"\n Radar " << to_string(radarPosition);
	
		file1 <<" ------------ - plotparam---------------- "<< plotparam;

		for (auto [firstfile1, secondfile1] : map_property_f1)
		{

			for (auto [firstfile2, secondfile2] : map_property_f2)
			{

				if (firstfile1 == firstfile2)
				{
					sort(secondfile1.begin(), secondfile1.end());
					sort(secondfile2.begin(), secondfile2.end());
					std::set_difference(secondfile1.begin(), secondfile1.end(),
										secondfile2.begin(), secondfile2.end(),
										std::back_inserter(diff));

					 
					if (diff.size() != 0) 
					{
						
						file1 << "\n ----------------------------- ";
						file1 << "\n Mismatch scanindex " << firstfile1;

						for (int i=1;i<= diff.size();i++)
						{
							scanindex.push_back(firstfile1);
						}

					    file1 << "\n Mismatch value ";

						for (auto val:diff)
						{
							differencevalue.push_back(val);
							file1 << "\n" << val;
						}
						diff.clear();


					}


				}
			}




			
		}

		for (auto [firstfile1, secondfile1] : map_property_f1)
		{
			if (map_property_f2.count(firstfile1)==0)//key not present in file2
			{

				file1 << "\n ----------------------------- ";
				file1 << "\n Missing  scanindex  " << firstfile1;

				for (int i = 1; i <= secondfile1.size(); i++)
				{
					scanindex.push_back(firstfile1);
				}

				file1 << "\n Missing scanindex value  ";
				for (auto val : secondfile1)
				{
					differencevalue.push_back(val);
					file1 << "\n" << val;
				}
			}

		}

		if (differencevalue.size() != 0)
		{
			matplotnonscale.generate_mismmatch_scatterplot(scanindex, differencevalue, filecount, f_name, plottitle.c_str(),
				foldname, plotparam, reportpath, radarPosition);
		}
		double mismatchcount = differencevalue.size();
		double  mismatch_percentage =(mismatchcount/(double)property_file1_count);

		/*std::cout << std::endl << "#######mismatchcount " << mismatchcount;
		std::cout << std::endl << "#######property_file1_count " << property_file1_count;
		std::cout << std::endl << "#######mismatch_percentage " << mismatch_percentage;*/
		
		
		//int mismatch_percentage_final = ceil(mismatch_percentage * 100);
		int mismatch_percentage_final = mismatch_percentage * 100;
		int match_percentage = 100 - mismatch_percentage_final;

		/*std::cout << std::endl << "#######match_percentage " << match_percentage;
		std::cout << std::endl << "#######mismatch_percentage_final " << mismatch_percentage_final;*/

		file1 << "\n mismatchcount " << to_string(mismatchcount);
		file1 << "\n property_file1_count " << to_string(property_file1_count);
		file1 << "\n mismatch_percentage " << to_string(mismatch_percentage);
		file1 << "\n match_percentage " << to_string(match_percentage);
		file1 << "\n mismatch_percentage_final " << to_string(mismatch_percentage_final);
		

		if (match_percentage != 100 && mismatch_percentage_final !=100)
		{
			std::vector<double> match_mismatch_percentage;
			match_mismatch_percentage.push_back(mismatch_percentage_final);
			match_mismatch_percentage.push_back(match_percentage);

			std::vector<std::string> mat_mismat_txt;
			mat_mismat_txt.push_back("mismatch");
			mat_mismat_txt.push_back("match");


			titleinfomap[0] = "RL " + plotparam + " Mismatch  pie";
			titleinfomap[1] = "RR " + plotparam + " Mismatch  pie";
			titleinfomap[2] = "FR" + plotparam + " Mismatch  pie";
			titleinfomap[3] = "FL " + plotparam + " Mismatch  pie";
			titleinfomap[5] = "FC " + plotparam + " Mismatch  pie";
			for (auto val : titleinfomap)
			{
				if (val.first == radarpos)
				{
					plottitle1 = val.second;
				}
			}


			if (match_mismatch_percentage.size() != 0)
			{
				matplotnonscale.generate_pieplot(match_mismatch_percentage, mat_mismat_txt,
					filecount, f_name, plottitle1.c_str(),
					foldname, plotparam, reportpath, radarPosition);
			}
		}
		else if(match_percentage==100)
		{
			/*std::vector<double> v{ 100,0 };
			matplotnonscale.generate_barplot_matchmistach(v, filecount, f_name, plottitle1.c_str(),
				foldname, plotparam, reportpath, radarPosition);*/
			std::cout << std::endl << "match_percentage == 100";

			std::vector<double> match_mismatch_percentage;
			match_mismatch_percentage.push_back(1.0);
			match_mismatch_percentage.push_back(99.99);

			std::vector<std::string> mat_mismat_txt;
			mat_mismat_txt.push_back("mismatch");
			mat_mismat_txt.push_back("match");


			titleinfomap[0] = "RL" + plotparam + " mismatch match  pie";
			titleinfomap[1] = "RR" + plotparam + " mismatch match  pie";
			titleinfomap[2] = "FR" + plotparam + " mismatch match  pie";
			titleinfomap[3] = "FL" + plotparam + " mismatch match  pie";
			titleinfomap[5] = "FC" + plotparam + " mismatch match  pie";
			for (auto val : titleinfomap)
			{
				if (val.first == radarpos)
				{
					plottitle1 = val.second;
				}
			}


			if (match_mismatch_percentage.size() != 0)
			{
				matplotnonscale.generate_pieplot(match_mismatch_percentage, mat_mismat_txt,
					filecount, f_name, plottitle1.c_str(),
					foldname, plotparam, reportpath, radarPosition);
			}
		}
		else if (mismatch_percentage_final == 100)
		{
			/*std::vector<double> v{ 100,0 };
			matplotnonscale.generate_barplot_matchmistach(v, filecount, f_name, plottitle1.c_str(),
				foldname, plotparam, reportpath, radarPosition);*/

			std::cout << std::endl << "mismatch_percentage == 100";
			std::vector<double> match_mismatch_percentage;
			match_mismatch_percentage.push_back(99.99);
			match_mismatch_percentage.push_back(1.0);

			std::vector<std::string> mat_mismat_txt;
			mat_mismat_txt.push_back("mismatch");
			mat_mismat_txt.push_back("match");


			titleinfomap[0] = "RL" + plotparam + " mismatch match  pie";
			titleinfomap[1] = "RR" + plotparam + " mismatch match  pie";
			titleinfomap[2] = "FR" + plotparam + " mismatch match  pie";
			titleinfomap[3] = "FL" + plotparam + " mismatch match  pie";
			titleinfomap[5] = "FC" + plotparam + " mismatch match  pie";
			for (auto val : titleinfomap)
			{
				if (val.first == radarpos)
				{
					plottitle1 = val.second;
				}
			}


			if (match_mismatch_percentage.size() != 0)
			{
				matplotnonscale.generate_pieplot(match_mismatch_percentage, mat_mismat_txt,
					filecount, f_name, plottitle1.c_str(),
					foldname, plotparam, reportpath, radarPosition);
			}
		}

		
		
	
}





void MatplotAdapter::calculate_data_match_property_scatterplot(std::map<double, std::vector<double>> map_property_f1, std::map<double, std::vector<double>> map_property_f2, int filecount, const char* f_name, std::string plottitle, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition, int tolerance, int radarpos, int property_file1_count)
{


}

void MatplotAdapter::calculate_match_mismatch_percentage(double totalcount, double mismatchcount, int filecount, const char* f_name, std::string plottitle, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
	MatplotNonScale& matplotnonscale = MatplotNonScale::getInstance();

	//std::cout << "\n calculate_match_mismatch_percentage " << totalcount << " " << mismatchcount;
	if (totalcount!=0 && mismatchcount!=0)
	{
		
		double  mismatch_percentage = mismatchcount / totalcount;
		//std::cout << "\n Radar " << radarPosition << "  ##mismatch_percentage " << mismatch_percentage;
		mismatch_percentage =ceil(mismatch_percentage * 100);
		double match_percentage = 100 - mismatch_percentage;

		//std::cout <<"\n Radar "<< radarPosition<< "  mismatch_percentage " << mismatch_percentage;
		//std::cout <<"\n Radar "<< radarPosition << " match_percentage" << match_percentage;
		std::vector<double> match_mismatch_percentage;
		match_mismatch_percentage.push_back(mismatch_percentage);
		match_mismatch_percentage.push_back(match_percentage);

		std::vector<std::string> mat_mismat_txt;
		mat_mismat_txt.push_back("mismatch");
		mat_mismat_txt.push_back("match");

		
		if (match_mismatch_percentage.size()!=0)
		{
			matplotnonscale.generate_pieplot(match_mismatch_percentage, mat_mismat_txt,
				filecount, f_name, plottitle.c_str(),
				foldname, plotparam, reportpath, radarPosition);
		}
		
	}
	


}

void MatplotAdapter::adapter_for_additional_property_scatterplot(std::map<double, std::vector<double>> map_property_f1, std::map<double, std::vector<double>> map_property_f2, int filecount, const char* f_name, std::string plottitle, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition, int tolerance)
{
}







int MatplotAdapter::getindex(std::vector<double>& vec, double& value)
{
	int index{ 0 };
	auto it = find(vec.begin(), vec.end(), value);

	if (it != vec.end())
	{
		index = it - vec.begin();
	}
	else
	{
		index = -1;
	}

	return index;
}

int MatplotAdapter::check_key(std::map<double, double> &map_start_end_scanindexf1, double& key)
{
	if (map_start_end_scanindexf1.count(key) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
	
}



	

	

	

	
	


















