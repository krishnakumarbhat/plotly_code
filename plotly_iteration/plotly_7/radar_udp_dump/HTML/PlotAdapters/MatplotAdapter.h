#ifndef MATPLOT_ADAPTERS
#define MATPLOT_ADAPTERS

#include<map>
#include<iostream>
#include<memory>
#include<set>
#include<vector>
#include <fstream>
#include "../HTMLInterface/HTMLInterface.h"
#include "../../Common/IRadarStream.h"
#include "../../Common/Z_Logging.h"
#include "../DataFactory/DataFactoryInterface.h"
#include "../SharedData/SharedPlotData.h"
#include "../PlotAdapters/IPlotAdapter.h"
#include "../PlotAdapters/IPlotAdapterObserver.h"
#include "../PlotGenerator/MatplotNonScaleplot.h"
#include "../PlotGenerator/MatplotScaleplot.h"
#include "../InputParser/JsonInputParser.h"
#include<algorithm>
#include<memory>
#include<set>
class MatplotAdapter:public IPlotAdapter
{ 
	

public:

	MatplotAdapter(std::shared_ptr<PlotParameter[]>& sptr_plotparam);
	MatplotAdapter(MatplotAdapter&) = delete;
	MatplotAdapter& operator=(MatplotAdapter&) = delete;
	virtual ~MatplotAdapter();
	
	virtual void generate_plot()override;
	virtual void trigger_plot_generation(std::set<int> radarpositions, const char* f_name, const char* pltfolder, int filecount, std::string reportpath)override;
	inline void remove_old_html_reports(const char* f_name, const char* pltfolder, std::string reportpath);
	void adapter_forscaled_scatterplot(std::map<double, std::vector<double>>& map_property_f1,
										std::map<double, std::vector<double>>& map_property_f2,
										int filecount, const char* f_name, std::string plottitle,
										const char* foldname, std::string plotparam,
										std::string reportpath, int radarPosition);


	void adapter_for_mismatch_property_scatterplot(std::map<double, std::vector<double>> map_property_f1,
												   std::map<double, std::vector<double>> map_property_f2,
												   int filecount, const char* f_name, std::string plottitle,
												   const char* foldname, std::string plotparam,
												   std::string reportpath, int radarPosition,
		                                           int tolerance,int radarpos, int property_file1_count);


	void calculate_data_mismatch_property_scatterplot(std::map<double, std::vector<double>> map_property_f1,
		std::map<double, std::vector<double>> map_property_f2,
		int filecount, const char* f_name, std::string plottitle,
		const char* foldname, std::string plotparam,
		std::string reportpath, int radarPosition,
		int tolerance, int radarpos, int property_file1_count);

	void calculate_data_match_property_scatterplot(std::map<double, std::vector<double>> map_property_f1,
		std::map<double, std::vector<double>> map_property_f2,
		int filecount, const char* f_name, std::string plottitle,
		const char* foldname, std::string plotparam,
		std::string reportpath, int radarPosition,
		int tolerance, int radarpos, int property_file1_count);


	void calculate_match_mismatch_percentage(double totalcount, double mismatchcount,
		                                     int filecount, const char* f_name, std::string plottitle,
		                                     const char* foldname, std::string plotparam,
		                                     std::string reportpath, int radarPosition
		                                     );

	void adapter_for_additional_property_scatterplot(std::map<double, std::vector<double>> map_property_f1,
													std::map<double, std::vector<double>> map_property_f2,
													int filecount, const char* f_name, std::string plottitle,
													const char* foldname, std::string plotparam,
													std::string reportpath, int radarPosition,
		                                            int tolerance);

	int getindex(std::vector<double>&vec,double &value);
	int check_key(std::map<double, double> &map_start_end_scanindexf1,double &key);

	std::set<int> mset_radarposition;
	std::shared_ptr<PlotParameter[]>& sptr_plotparam_plotadapter;
	//ofstream ffile1, ffile2;

	std::vector<double> vec_range_f0_veh, vec_range_f1_resim;
	std::vector<double> vec_range_match, vec_range_mismatch[2], vec_range_additional;
	std::vector<double> vec_si_range_match, vec_si_range_mismatch, vec_si_range_additional;


	std::vector<double> vec_property_f0_veh, vec_property_f1_resim;
	std::vector<double> vec_property_match, vec_property_mismatch[2], vec_property_additional;
	std::vector<double> vec_si_property_match, vec_si_property_mismatch, vec_si_property_additional;


	ofstream file1, file2;
};
#endif