#ifndef MATPLOT_SCALE
#define MATPLOT_SCALE

#include<matplot/matplot.h>
#include<chrono>
#include <filesystem>

#include "../SharedData/SharedPlotData.h"
#include "../../CommonFiles/inc/ctml.hpp"


#ifdef _WIN32
#include <direct.h>// for _mkdir
#endif
#ifdef __GNUC__
#include <sys/stat.h>
#endif
class MatplotScale
{
	MatplotScale();
public:
	MatplotScale(MatplotScale&) = delete;
	MatplotScale& operator=(MatplotScale&) = delete;
	virtual ~MatplotScale();
	static MatplotScale& getInstance();

	void generate_scaled_scatterplot(const std::vector<double>& f1_xvalue, 
		                             const std::vector<double>& f1_yvalue,
									 const std::vector<double>& f2_xvalue, 
		                             const std::vector<double>& f2_yvalue,
									 int fileindex, const char* f_name, std::string plottile,
		                             const char* foldname,std::string plotparam, 
		                             std::string reportpath, int radarPosition);

};

#endif