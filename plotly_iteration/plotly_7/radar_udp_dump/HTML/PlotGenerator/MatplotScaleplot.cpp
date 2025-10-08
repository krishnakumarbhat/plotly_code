#include "MatplotScaleplot.h"
#include "../HTMLReportManager/HTMLReportManager.h"
#include "../InputParser//JsonInputParser.h"
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;
using namespace matplot;
MatplotScale::MatplotScale()
{
}

MatplotScale::~MatplotScale()
{
}

MatplotScale& MatplotScale::getInstance()
{
	static MatplotScale instance;
	return instance;
}

void MatplotScale::generate_scaled_scatterplot(const std::vector<double>& f1_xvalue, 
                                               const std::vector<double>& f1_yvalue, 
                                               const std::vector<double>& f2_xvalue, 
                                               const std::vector<double>& f2_yvalue, 
                                               int fileindex, const char* f_name, std::string plottile, 
                                               const char* foldname, std::string plotparam, 
                                               std::string reportpath, int radarPosition)
{

    CTML::Document document;
    std::fstream log_report;
    std::string htmlreport_file;
    std::string htmlreport_folder;
    std::string htmlreport_image;

    std::string htmlreport_imagepath(f_name);
    std::string radar_position(plottile);

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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + std::to_string(fileindex)+"-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + std::to_string(fileindex) + "-TRK.png";
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
