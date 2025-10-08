#include "MatplotNonScaleplot.h"
#include "../HTMLReportManager/HTMLReportManager.h"
#include "../InputParser//JsonInputParser.h"
#include <cmath>


MatplotNonScale::MatplotNonScale()
{

   /* file1.open("mismatch.txt", ios::out | ios::app);
    file2.open("match.txt", ios::out | ios::app);*/
}

MatplotNonScale::~MatplotNonScale()
{
  /* file1.close();
   file2.close();*/
}

MatplotNonScale& MatplotNonScale::getInstance()
{
	static MatplotNonScale instance;
	return instance;
}



void MatplotNonScale::prepare_data_generate_plot()
{

}

void MatplotNonScale::prepare_c2timinginfo_generate_nonscaled_c2timing_scatterplot()
{
}

int MatplotNonScale::PrintReportInfoinHTML(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt)
{
    JsonInputParser& jsonparser = JsonInputParser::getInstance();

    CTML::Document document;
    //HtmlReportManager& report = HtmlReportManager::getInstance();
    //DataProxy& datapxy = DataProxy::getInstance();
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
    htmlreport_folder = reportpath + "/" + foldname;

    //cout << "\n htmlreport_file" << htmlreport_file;
    //cout << "\n htmlreport_folder" << htmlreport_folder;



    std::string cs_info = "4537 ";
    std::string sourcecode_cs_info = "23757";
    std::string os_info;
    std::string tool_version = "2.3";
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

#ifdef _WIN32
    os_info = "Windows ";
#endif
#ifdef __GNUC__
    os_info = "Linux ";
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

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Binary Change Set Info  : " << cs_info << " CS Release Date :Jan 06 2025 " << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Source code  Change Set Info  : " << sourcecode_cs_info;
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Tool run on  " << os_info <<" Operating Sysem  "<< "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Tool Version  : " << tool_version << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " HTML Report Generation Date Time Info  : " << std::ctime(&end_time) << "</p>\n";


        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Kindly note below color code for Interpretations " << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File1:blue)  :: " << jsonparser.vehiclefilename <<" ( Log duration "<< jsonparser.Log1_duration <<" sec )" << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File2:red) :: " << jsonparser.resimfilename << " ( Log duration " << jsonparser.Log2_duration<< " sec )" << "</p>\n";

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File1:blue)  :: " << jsonparser.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File2:orange)  :: " << jsonparser.resimfilename << "</p>\n";

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

int MatplotNonScale::print_reporttoolrun_timing_info(const char* f_name, const char* current_radar, const char* foldname, std::string reportpath, int arraycnt)
{
    JsonInputParser& jsonpar = JsonInputParser::getInstance();
    CTML::Document document;
   // HtmlReportManager& report = HtmlReportManager::getInstance();
    //DataProxy& datapxy = DataProxy::getInstance();
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
    htmlreport_folder = reportpath + "/" + foldname;

    //cout << "\n htmlreport_file" << htmlreport_file;
    //cout << "\n htmlreport_folder" << htmlreport_folder;



    std::string cs_info = "4537";
    std::string os_info;
    std::string tool_version = "2.3";
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
        log_report << "<h1> HTML Report Run Time Information  </h1>\n" << endl;

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Binary Change Set Info  : " << cs_info << " CS Release Date : Jan 06 2025 " << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Log1 " << jsonpar.vehiclefilename << " ( Log duration " << jsonpar.Log1_duration << " sec )" << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Log2 " << jsonpar.resimfilename << " ( Log duration " << jsonpar.Log2_duration << " sec )" << "</p>\n";
        //log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Log1 Duration  : " << report.Log1_duration <<" sec" << "</p>\t";
        //log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Log2 Duration  : " << report.Log2_duration <<" sec" << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " HTML Run time(sec)  : " << jsonpar.html_runtime_sec <<"  sec in OS "<< os_info << "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " HTML Run time(min)  : " << jsonpar.html_runtime_min << "  min in OS " << os_info<< "</p>\t";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Run time as function of input  : " << jsonpar.tool_runtime_func_inputtime << " times of input file duration in OS"<< os_info <<"</p>\n";


       /* log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Kindly note below color code for Interpretations " << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File1:blue)  :: " << report.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Scatter Plot (color File2:red) :: " << report.resimfilename << "</p>\n";

        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File1:blue)  :: " << report.vehiclefilename << "</p>\n";
        log_report << "<p style=\"font-size: " << 16 << "px;\">" << " Histogram (color File2:orange)  :: " << report.resimfilename << "</p>\n";*/

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

void MatplotNonScale::generate_nonscaled_vehinfo_speed_scatterplot(Vehicle_Plot_Param_T* pveh_plot_info, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath,int radarPosition)
{

     
    if ((pveh_plot_info->scanidex[0].size() == 0) && (pveh_plot_info->scanidex[1].size() == 0))
    {
        return ;
    }
		
    //std::cout << "\ngenerate_nonscaled_vehinfo_speed_scatterplot radar"<< radarPosition;

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
           
                sgtitle(radar_position, "blue");
                tiledlayout(1, 1);
                auto ax1 = nexttile();
                grid(false);
                ax1->ylabel(plotparam);
                ax1->xlabel("scan index");


                std::vector<std::string> legdetails;
                
                
               

                if (pveh_plot_info != nullptr)
                {
                    if ((pveh_plot_info + radarPosition)->abs_speed[0].size() != 0)
                    {
                        auto pfl11 = scatter((pveh_plot_info + radarPosition)->scanidex[0], (pveh_plot_info + radarPosition)->abs_speed[0]);
                        pfl11->color("blue");
                        pfl11->fill(true);
                        pfl11->marker_color("b");
                        pfl11->marker_size(1.5);
                        pfl11->marker("O");
                        pfl11->marker_face(true);
                        legdetails.push_back("file1");

                        hold(true);
                        //std::cout << "\ngenerate_nonscaled_vehinfo_speed_scatterplot fo radarPosition"<< radarPosition;
                    }

                    if ((pveh_plot_info + radarPosition)->abs_speed[1].size() != 0)
                    {
                        auto pfl2 = scatter((pveh_plot_info + radarPosition)->scanidex[1], (pveh_plot_info + radarPosition)->abs_speed[1]);
                        pfl2->color("red");
                        pfl2->fill(true);
                        pfl2->marker_color("r");
                        pfl2->marker_size(1.5);
                        pfl2->marker("O");
                        pfl2->marker_face(true);
                        legdetails.push_back("file2");
                        //std::cout << "\ngenerate_nonscaled_vehinfo_speed_scatterplot f1 radarPosition"<< radarPosition;
                    }
                    
                    //std::cout << "\ngenerate_nonscaled_vehinfo_speed_scatterplot fo f1 radarPosition "<< radarPosition;
                    ax1->legend(legdetails);
                    radar_position.append(std::to_string(radarPosition));


                    htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
                    save((char*)htmlreport_image.c_str(), "png");

                    htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_vehinfo_yawrate_scatterplot(Vehicle_Plot_Param_T* pveh_plot_info, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\ngenerate_nonscaled_vehinfo_yawrate_scatterplot";
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
          
                sgtitle(radar_position, "blue");
                tiledlayout(1, 1);
                auto ax1 = nexttile();
                grid(false);
                ax1->ylabel(plotparam);
                ax1->xlabel("scan index");


                std::vector<std::string> legdetails;
                         

                if (pveh_plot_info != nullptr)
                {
                    if ((pveh_plot_info + radarPosition)->yaw_rate[0].size() != 0)
                    {
                        auto pfl11 = scatter((pveh_plot_info + radarPosition)->scanidex[0], (pveh_plot_info + radarPosition)->yaw_rate[0]);
                        pfl11->color("blue");
                        pfl11->fill(true);
                        pfl11->marker_color("b");
                        pfl11->marker_size(1.5);
                        pfl11->marker("O");
                        pfl11->marker_face(true);
                        legdetails.push_back("file1");

                        hold(true);
                    }

                    if ((pveh_plot_info + radarPosition)->yaw_rate[1].size() != 0)
                    {
                        auto pfl2 = scatter((pveh_plot_info + radarPosition)->scanidex[1], (pveh_plot_info + radarPosition)->yaw_rate[1]);
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


                    htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
                    save((char*)htmlreport_image.c_str(), "png");

                    htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_range_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, /*std::vector<double>& file1_ydata, std::vector<double>& file2_ydata,*/ int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    std::cout << "\n generate_nonscaled_detection_range_scatterplot"; 
    auto start_time = std::chrono::high_resolution_clock::now();
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
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);

        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->range[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->range[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");
                hold(true);

            }

            if ((pselfdetection + radarPosition)->range[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->range[1]);
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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
            CTML::Node image_trks("img");
            image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
                .UseClosingTag(false);
            document.AppendNodeToBody(image_trks);
        }
       

       
    }
    log_report << document.ToString() << endl;

    if (!log_report.is_open())
        log_report.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << "\n total_duration " << total_duration;

}

void MatplotNonScale::generate_barplot(const std::vector<double>& f1_xvalue, 
                                       const std::vector<std::vector<double>>& f1_yvalue, 
                                       int fileindex, const char* f_name, const char* current_radar, 
                                       const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

               
               
                auto pfl11 = bar(f1_xvalue, f1_yvalue);

                std::vector<double> label_x;
                std::vector<double> label_y;
                std::vector<std::string> labels;
                for (size_t i = 0; i < f1_yvalue.size(); ++i) {
                    for (size_t j = 0; j < f1_xvalue.size(); ++j) {
                        label_x.emplace_back(pfl11->x_end_point(i, j));
                        label_y.emplace_back(f1_yvalue[i][j] + 1);
                        labels.emplace_back(num2str(f1_yvalue[i][j]));
                    }
                }
                hold(on);
                //text(label_x, label_y, labels);
                gca()->x_axis().ticklabels({ "isbistatic", "issingletar","issuperestar"});
                matplot::text(label_x, label_y, labels)->font("Times New Roman"); //Specify the font


               /* pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);*/
                //legdetails.push_back("1. isbistatic 2. issingletarget");
               
            }
            if (!f1_xvalue.empty())
            {
                //auto pfl11 = bar(f1_xvalue);
                //legdetails.push_back("file1");
            }

         

            //ax1->legend(legdetails);
            radar_position.append(std::to_string(radarPosition));


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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
void MatplotNonScale::generate_barplot_matchmistach(const std::vector<double>& f1_xvalue, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

        radar_position.append(std::to_string(radarPosition));


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
}





void MatplotNonScale::generate_boxplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

            if (!f1_xvalue.empty())
            {
                auto pfl11 = boxplot(f1_xvalue);
                /* pfl11->color("blue");
                 pfl11->fill(true);
                 pfl11->marker_color("b");
                 pfl11->marker_size(1.5);
                 pfl11->marker("O");
                 pfl11->marker_face(true);*/
                legdetails.push_back("diff");

            }



            ax1->legend(legdetails);
            radar_position.append(std::to_string(radarPosition));


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_pieplot(const std::vector<double> f1_xvalue, const std::vector<std::string> f1_yvalue, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        //std::vector<std::string> legdetails;
        tiledlayout(1, 1);

       auto ax1 = nexttile();
       // grid(false);
        //ax1->ylabel(plotparam);
        

        if (1)
        {

            if (!f1_xvalue.empty())
            {
                pie(f1_xvalue);
                std::string label = to_string(int(f1_xvalue.at(0))) + "% mismatch & " + to_string(int(f1_xvalue.at(1))) + "% match";
                ax1->xlabel(label.c_str());
               // matplot::text(f1_xvalue,f1_yvalue)->font("Times New Roman"); //Specify the font
                /* pfl11->color("blue");
                 pfl11->fill(true);
                 pfl11->marker_color("b");
                 pfl11->marker_size(1.5);
                 pfl11->marker("O");
                 pfl11->marker_face(true);*/
                //legdetails.push_back("diff");

            }



            //ax1->legend(legdetails);
            radar_position.append(std::to_string(radarPosition));


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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



//void MatplotNonScale::generate_detection_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
//                                                     const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue, 
//                                                     int fileindex, const char* f_name, const char* current_radar, const char* foldname, 
//                                                     std::string plotparam, std::string reportpath, int radarPosition)
//{
//    //std::cout << "\n generate_nonscaled_detection_scatterplot_generic";
//    //auto start_time = std::chrono::high_resolution_clock::now();
//
//    CTML::Document document;
//    std::fstream log_report;
//    std::string htmlreport_file;
//    std::string htmlreport_folder;
//    std::string htmlreport_image;
//
//    std::string htmlreport_imagepath(f_name);
//    std::string radar_position(current_radar);
//
//    auto mf4_filenamefrom_path = fs::path(f_name).filename();
//    std::string mf4_filename{ mf4_filenamefrom_path.u8string() };
//    string imagefoldername = "GeneratedImages";
//
//    if (int(radarPosition) == 0)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
//    }
//    else if (int(radarPosition) == 1)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
//    }
//    else if (int(radarPosition) == 2)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
//    }
//    else if (int(radarPosition) == 3)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
//    }
//    else if (int(radarPosition) == 5)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FC.html";
//    }
//
//    else if (int(radarPosition) == 19)
//    {
//        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_DC.html";
//    }
//    htmlreport_folder = reportpath + "/" + foldname;
//
//    
//
//    if (!std::filesystem::exists(htmlreport_folder.c_str())) {
//#ifdef _WIN32
//        _mkdir(htmlreport_folder.c_str());
//#endif
//#ifdef __GNUC__
//        mkdir(htmlreport_folder.c_str(), ACCESSPERMS);
//#endif
//    }
//    if (!log_report.is_open())
//
//        log_report.open(htmlreport_file.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app);
//    {
//
//        auto tracks_fig2 = figure(true);
//        sgtitle(radar_position, "blue");
//        std::vector<std::string> legdetails;
//        tiledlayout(1, 1);
//
//        auto ax1 = nexttile();
//        grid(false);
//        ax1->ylabel(plotparam);
//        ax1->xlabel("scan index");
//
//        if (1)
//        {
//
//            if (!f1_xvalue.empty() && !f1_yvalue.empty())
//            {
//                auto pfl11 = scatter(f1_xvalue, f1_yvalue);
//                pfl11->color("blue");
//                pfl11->fill(true);
//                pfl11->marker_color("b");
//                pfl11->marker_size(1.5);
//                pfl11->marker("O");
//                pfl11->marker_face(true);
//                legdetails.push_back("file1");
//                hold(true);
//            }
//
//            if (!f2_xvalue.empty() && !f2_yvalue.empty())
//            {
//                auto pfl2 = scatter(f2_xvalue, f2_yvalue);
//                pfl2->color("red");
//                pfl2->fill(true);
//                pfl2->marker_color("r");
//                pfl2->marker_size(1.5);
//                pfl2->marker("O");
//                pfl2->marker_face(true);
//                legdetails.push_back("file2");
//            }
//
//            ax1->legend(legdetails);
//            radar_position.append(std::to_string(radarPosition));
//
//
//            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
//            save((char*)htmlreport_image.c_str(), "png");
//
//            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
//            CTML::Node image_trks("img");
//            image_trks.SetAttribute("src", htmlreport_imagepath.c_str())
//                .UseClosingTag(false);
//            document.AppendNodeToBody(image_trks);
//        }
//        
//    }
//    log_report << document.ToString() << endl;
//
//    if (!log_report.is_open())
//        log_report.close();
//
//  
//
//
//    //auto end_time = std::chrono::high_resolution_clock::now();
//    //auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
//    //std::cout << "\n total_duration " << total_duration;
//
//
//}



void MatplotNonScale::generate_nonscaled_detection_maxrange_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\n generate_nonscaled_detection_maxrange_scatterplot";
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
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);

        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->range_max[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->scanindex_maxrange[0], (pselfdetection + radarPosition)->range_max[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");
                hold(true);

            }

            if ((pselfdetection + radarPosition)->range_max[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->scanindex_maxrange[1], (pselfdetection + radarPosition)->range_max[1]);
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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_rangerate_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
      
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        
        
      

        if (pselfdetection != nullptr)
        {

            if ((pselfdetection + radarPosition)->range_rate[0].size()!=0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->range_rate[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->range_rate[1].size()!=0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->range_rate[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_azimuth_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
      
        
        

        if (pselfdetection != nullptr)
        {

            if ((pselfdetection + radarPosition)->azimuth[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->azimuth[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }
          
            if ((pselfdetection + radarPosition)->azimuth[1].size()!=0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->azimuth[1]);
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





            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_elevation_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        
       
        

        if (pselfdetection != nullptr)
        {

            if ((pselfdetection + radarPosition)->elevation[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->elevation[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }
           
            if ((pselfdetection + radarPosition)->elevation[1].size()!=0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->elevation[1]);
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





            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_rcs_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        
        
        

        if (pselfdetection != nullptr)
        {

            if ((pselfdetection + radarPosition)->rcs[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->rcs[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->rcs[1].size()!=0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->rcs[1]);
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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_amp_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
      
        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->amp[0].size() != 0)
            {
                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->amp[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }
           
            if ((pselfdetection + radarPosition)->amp[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->amp[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_nonscaled_detection_snr_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;
        
        
        

        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->snr[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->snr[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->snr[1].size()!=0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->snr[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_valid_detectioncount_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->snr[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh_ref_scale, (pselfdetection + radarPosition)->valid_detection_count[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->snr[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim_ref_scale, (pselfdetection + radarPosition)->valid_detection_count[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_el_conf_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{

    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->el_conf[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->el_conf[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->el_conf[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->el_conf[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_az_conf_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{

    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->az_conf[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->az_conf[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->el_conf[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->az_conf[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_isbistatic_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{

    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->isBistatic[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->isBistatic[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->isBistatic[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->isBistatic[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_issingletarget_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->isSingleTarget[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->isSingleTarget[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->isSingleTarget[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->isSingleTarget[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_super_res_target_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{


    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->super_res_target[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->super_res_target[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->super_res_target[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->super_res_target[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_super_res_target_type_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{



    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->super_res_target_type[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->super_res_target_type[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->super_res_target_type[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->super_res_target_type[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_bf_type_el_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{

    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->bf_type_el[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->bf_type_el[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->bf_type_el[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->bf_type_el[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_bf_type_az_scatterplot(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{

    //std::cout << "\n generate_c2timinginfo_scatterplot";
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
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (pselfdetection != nullptr)
        {
            if ((pselfdetection + radarPosition)->bf_type_az[0].size() != 0)
            {

                auto pfl11 = scatter((pselfdetection + radarPosition)->si_veh, (pselfdetection + radarPosition)->bf_type_az[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
                legdetails.push_back("file1");

                hold(true);
            }

            if ((pselfdetection + radarPosition)->bf_type_az[1].size() != 0)
            {
                auto pfl2 = scatter((pselfdetection + radarPosition)->si_resim, (pselfdetection + radarPosition)->bf_type_az[1]);
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

            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_c2timinginfo_scatterplot(Timing_Info_T* ptiminginfo, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\ngenerate_c2timinginfo_scatterplot";
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

        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");

        if (ptiminginfo != nullptr)
        {
            if ((ptiminginfo + radarPosition)->c2timinginfo_index23[0].size() != 0)
            {
                auto pfl11 = scatter((ptiminginfo + radarPosition)->scanindex[0], (ptiminginfo + radarPosition)->c2timinginfo_index23[0]);
                pfl11->color("blue");
                pfl11->fill(true);
                pfl11->marker_color("b");
                pfl11->marker_size(1.5);
                pfl11->marker("O");
                pfl11->marker_face(true);
            }
        }

        radar_position.append(std::to_string(radarPosition));

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

}

void MatplotNonScale::generate_detection_range_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\n generate_detection_range_histogram";
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            if (pselfdetection != nullptr)
            {
                if ((pselfdetection + radarPosition)->range[0].size() != 0)
                {
                    auto h1 = hist(ax12, (pselfdetection + radarPosition)->range[0], a, histogram::normalization::count);
                    h1->face_color("b");
                    legdetails1.push_back("f1");
                    hold(on);
                    
                }
                if ((pselfdetection + radarPosition)->range[1].size() != 0)
                {
                    auto h2 = hist(ax12, (pselfdetection + radarPosition)->range[1], a, histogram::normalization::count);
                    h2->face_color("r");
                    legdetails1.push_back("f2");
                }

                                   
                ax12->legend(legdetails1);
                ylabel("count");
                xlabel(plotparam);
            }
              
                   

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
    
}

void MatplotNonScale::generate_detection_histogram(const std::vector<double>& f1_value, const std::vector<double>& f2_value, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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
            sgtitle(radar_position, "blue");
            tiledlayout(1, 1);

            auto ax12 = nexttile();
            std::vector<std::string> legdetails1;
            auto a = histogram::binning_algorithm::automatic;

            if (!f1_value.empty())
            {
                auto h1 = hist(ax12,f1_value, a, histogram::normalization::count);
                h1->face_color("b");
                h1->face_alpha(.5f);
                legdetails1.push_back("f1");
                hold(on);
            }
            
            if (!f2_value.empty())
            {
                auto h2 = hist(ax12, f2_value, a, histogram::normalization::count);
                h2->face_color("r");
                h2->face_alpha(.5f);
                legdetails1.push_back("f2");
            }

            ax12->legend(legdetails1);
            ylabel("count");
            xlabel(plotparam);
           
        
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

}



void MatplotNonScale::generate_detection_rangerate_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
    //std::cout << "\n generate_detection_rangerate_histogram";
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            if (pselfdetection != nullptr)
            {
                if ((pselfdetection + radarPosition)->range_rate[0].size() != 0)
                {
                    auto h1 = hist(ax12, (pselfdetection + radarPosition)->range_rate[0], a, histogram::normalization::count);
                    h1->face_color("b");
                    legdetails1.push_back("f1");
                    hold(on);
                    //std::cout << "\n generate_detection_rangerate_histogram 88";

                }
               
                if ((pselfdetection + radarPosition)->range_rate[1].size()!=0)
                {
                    auto h2 = hist(ax12, (pselfdetection + radarPosition)->range_rate[1], a, histogram::normalization::count);
                    h2->face_color("r");
                    legdetails1.push_back("f2");
                    //std::cout << "\n generate_detection_rangerate_histogram 77";
                }
                

               
            
                ax12->legend(legdetails1);
                ylabel("count");
                xlabel(plotparam);

            }
           

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
}

void MatplotNonScale::generate_detection_azimuth_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

           

            if (pselfdetection != nullptr)
            {
                if ((pselfdetection + radarPosition)->azimuth[0].size() != 0)
                {
                    auto h1 = hist(ax12, (pselfdetection + radarPosition)->azimuth[0], a, histogram::normalization::count);
                    h1->face_color("b");
                    legdetails1.push_back("f1");
                    hold(on);


                }
                if ((pselfdetection + radarPosition)->azimuth[1].size() != 0)
                {
                    auto h2 = hist(ax12, (pselfdetection + radarPosition)->azimuth[1], a, histogram::normalization::count);
                    h2->face_color("r");
                    legdetails1.push_back("f2");
                }
                
                          
                ax12->legend(legdetails1);
                ylabel("count");
                xlabel(plotparam);
            }

         

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
}

void MatplotNonScale::generate_detection_elevation_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

           
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->elevation[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->elevation[1], a, histogram::normalization::count);
            h2->face_color("r");
           
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            


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
}

void MatplotNonScale::generate_detection_rcs_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->rcs[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->rcs[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_amp_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->amp[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->amp[1], a, histogram::normalization::count);
            h2->face_color("r"); 


            //auto h1 = hist((pselfdetection + radarPosition)->amp[0]);
            //hold(on);
            //auto h2 = hist((pselfdetection + radarPosition)->amp[1]);

            //h1->normalization(histogram::normalization::count);
            ////h1->face_color("b");
            ////h1->bin_width(0.25);
            //h2->normalization(histogram::normalization::count);
            ////h2->bin_width(0.25);
            ////h2->face_color("r");


            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
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
}

void MatplotNonScale::generate_detection_snr_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->snr[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->snr[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_isbistatic_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
    htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
    htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->isBistatic[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->isBistatic[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_issingletarget_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->isSingleTarget[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->isSingleTarget[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_super_res_target_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->super_res_target[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->super_res_target[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_super_res_target_type_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->super_res_target_type[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->super_res_target_type[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_bf_type_el_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->bf_type_el[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->bf_type_el[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_detection_bf_type_az_histogram(SelfDetection_Plot_Parameters_T* pselfdetection, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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

            /*  if (zerodatafilenumber == 0)
              {*/
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->bf_type_az[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->bf_type_az[1], a, histogram::normalization::count);
            h2->face_color("r");
            //h1->face_color({ 0., 0., 0.5, 0.5 });
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);
            /*  }*/

           /*   if (zerodatafilenumber == 1)
              {
                  auto h2 = hist(ax12, f2_detection_data, a, histogram::normalization::count);
                  h2->face_color("r");
                  legdetails1.push_back("f2");

                  ax12->legend(legdetails1);

              }
              if (zerodatafilenumber == 2)
              {
                  auto h1 = hist(ax12, f1_detection_data, a, histogram::normalization::count);
                  h1->face_color("b");
                  legdetails1.push_back("f1");
                  ax12->legend(legdetails1);
              }*/


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
}

void MatplotNonScale::generate_diff_valid_detection_count_bargraph(std::map<double, double>& map_valid_detection_count_file1, 
                                                                   std::map<double, double>& map_valid_detection_count_file2, 
                                                                   int fileindex, const char* f_name, const char* current_radar, 
                                                                   const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{
  

   // std::cout << "\n generate_diff_valid_detection_count_bargraph";
    preparedata_diff_valid_detection_count(map_valid_detection_count_file1, map_valid_detection_count_file2);

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

    if (radarPosition == 0)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RL.html";
    }
    else if (radarPosition == 1)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_RR.html";
    }
    else if (radarPosition == 2)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FR.html";
    }
    else if (radarPosition == 3)
    {
        htmlreport_file = reportpath + "/" + foldname + "/" + mf4_filename + "-REPORT_FL.html";
    }
    else if (radarPosition == 5)
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
       
            sgtitle(radar_position, "blue");
            tiledlayout(1, 1);

            auto ax12 = nexttile();
            std::vector<std::string> legdetails1;



           /* auto a = histogram::binning_algorithm::automatic;

          
            auto h1 = hist(ax12, (pselfdetection + radarPosition)->bf_type_az[0], a, histogram::normalization::count);
            h1->face_color("b");
            hold(on);
            auto h2 = hist(ax12, (pselfdetection + radarPosition)->bf_type_az[1], a, histogram::normalization::count);
            h2->face_color("r");
          
            legdetails1.push_back("f1");
            legdetails1.push_back("f2");
            ax12->legend(legdetails1);*/

            bar(data_prep_vec_valid_det_diff_count_scanindex, data_prep_vec_valid_det_diff_count);
          


            ylabel("valid differance count");
            xlabel(plotparam);
            //ax1->legend(legdetails);
            radar_position.append(std::to_string(radarPosition));
        



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

   
}

void MatplotNonScale::preparedata_diff_valid_detection_count(std::map<double, double>& map_valid_detection_count_file1, std::map<double, double>& map_valid_detection_count_file2)
{
    //std::cout << "\n preparedata_diff_valid_detection_count";
    //std::cout << "\n scan index valid difference";

    if (!map_valid_detection_count_file1.empty() && !map_valid_detection_count_file2.empty())
    {
        for (auto& file1_si_validdet_pair : map_valid_detection_count_file1)
        {
            for (auto& file2_si_validdet_pair : map_valid_detection_count_file2)
            {
                if (file1_si_validdet_pair.first == file2_si_validdet_pair.first)
                {
                    if ((file1_si_validdet_pair.second - file2_si_validdet_pair.second) != 0)
                    {

                        //cout  << "\n"<<file1_si_validdet_pair.first << " " << (file1_si_validdet_pair.second - file2_si_validdet_pair.second);
                        data_prep_vec_valid_det_diff_count.push_back(file1_si_validdet_pair.second - file2_si_validdet_pair.second);
                        data_prep_vec_valid_det_diff_count_scanindex.push_back(file1_si_validdet_pair.first);
                    }

                }
            }
        }
    }

    /*std::cout << "\n preparedata_diff_valid_detection_count value";

    for (auto &value: data_prep_vec_valid_det_diff_count)
    {
        std::cout << " " << value;
    }*/

}

void MatplotNonScale::generate_tracker_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
    const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue,
    int fileindex, const char* f_name, const char* current_radar, const char* foldname,
    std::string plotparam, std::string reportpath, int radarPosition)
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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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

void MatplotNonScale::generate_match_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue, const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
{


}

void MatplotNonScale::generate_mismmatch_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue, int fileindex, const char* f_name, const char* current_radar, const char* foldname, std::string plotparam, std::string reportpath, int radarPosition)
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

        //auto tracks_fig2 = figure(true);
        /* if (fileindex == 0)
         {*/
        sgtitle(radar_position, "blue");
        tiledlayout(1, 1);
        auto ax1 = nexttile();
        grid(false);
        ax1->ylabel(plotparam);
        ax1->xlabel("scan index");


        std::vector<std::string> legdetails;




        if (1)
        {

            auto pfl11 = scatter(f1_xvalue, f1_yvalue);
            pfl11->color("blue");
            pfl11->fill(true);
            pfl11->marker_color("b");
            pfl11->marker_size(1.5);
            pfl11->marker("O");
            pfl11->marker_face(true);
            legdetails.push_back("Mismatch data bet file1 and file2");

        }


        ax1->legend(legdetails);
        radar_position.append(std::to_string(radarPosition));

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

}




void MatplotNonScale::generate_features_scatterplot(const std::vector<double>& f1_xvalue, const std::vector<double>& f1_yvalue,
    const std::vector<double>& f2_xvalue, const std::vector<double>& f2_yvalue,
    int fileindex, const char* f_name, const char* current_radar, const char* foldname,
    std::string plotparam, std::string reportpath, int radarPosition)
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
       tracks_fig2->width(tracks_fig2->width() * 2);
       tracks_fig2->height(tracks_fig2->height() * 1); 

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


            htmlreport_image = reportpath + "/" + foldname + "/" + imagefoldername + "/" + radar_position + "-TRK.png";
            save((char*)htmlreport_image.c_str(), "png");

            htmlreport_imagepath = imagefoldername + "/" + radar_position.c_str() + "-TRK.png";
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


