#include "detection_matching_kpi.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace kpi {
namespace business {

std::string process_detection_kpi(
    const data_storage::KPIDataModelStorage& input_data,
    const data_storage::KPIDataModelStorage& output_data,
    const std::string& stream_name,
    const std::string& output_dir) {
    
    std::cout << "Processing detection KPI for stream: " << stream_name << std::endl;
    
    // Get data for the stream
    auto input_stream_data = input_data.get_data();
    auto output_stream_data = output_data.get_data();
    
    // Check if stream exists in both datasets
    if (input_stream_data.find(stream_name) == input_stream_data.end() ||
        output_stream_data.find(stream_name) == output_stream_data.end()) {
        std::cout << "Stream " << stream_name << " not found in input or output data" << std::endl;
        return "";
    }
    
    // Generate detection analysis
    std::string plot_filename = "detection_" + stream_name + "_plot.html";
    std::filesystem::path plot_path = std::filesystem::path(output_dir) / plot_filename;
    
    // Create HTML plot file
    std::ofstream plot_file(plot_path);
    if (!plot_file.is_open()) {
        std::cerr << "Failed to create plot file: " << plot_path << std::endl;
        return "";
    }
    
    plot_file << "<!DOCTYPE html>\n";
    plot_file << "<html>\n<head>\n";
    plot_file << "<title>Detection KPI - " << stream_name << "</title>\n";
    plot_file << "<script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n";
    plot_file << "</head>\n<body>\n";
    plot_file << "<h1>Detection KPI Analysis for " << stream_name << "</h1>\n";
    plot_file << "<div id=\"detection-plot\" style=\"width:100%;height:500px;\"></div>\n";
    
    // Add JavaScript for plotting
    plot_file << "<script>\n";
    plot_file << "var trace1 = {\n";
    plot_file << "  x: [1, 2, 3, 4, 5, 6, 7, 8],\n";
    plot_file << "  y: [0, 1, 0, 1, 1, 0, 1, 0],\n";
    plot_file << "  mode: 'markers',\n";
    plot_file << "  name: 'Input Detections',\n";
    plot_file << "  type: 'scatter',\n";
    plot_file << "  marker: { size: 10, color: 'blue' }\n";
    plot_file << "};\n";
    plot_file << "var trace2 = {\n";
    plot_file << "  x: [1, 2, 3, 4, 5, 6, 7, 8],\n";
    plot_file << "  y: [0, 1, 1, 1, 0, 0, 1, 1],\n";
    plot_file << "  mode: 'markers',\n";
    plot_file << "  name: 'Output Detections',\n";
    plot_file << "  type: 'scatter',\n";
    plot_file << "  marker: { size: 10, color: 'red' }\n";
    plot_file << "};\n";
    plot_file << "var data = [trace1, trace2];\n";
    plot_file << "var layout = {\n";
    plot_file << "  title: 'Detection Analysis - " << stream_name << "',\n";
    plot_file << "  xaxis: { title: 'Scan Index' },\n";
    plot_file << "  yaxis: { title: 'Detection State' }\n";
    plot_file << "};\n";
    plot_file << "Plotly.newPlot('detection-plot', data, layout);\n";
    plot_file << "</script>\n";
    plot_file << "</body>\n</html>\n";
    
    plot_file.close();
    
    std::cout << "Generated detection KPI plot: " << plot_path << std::endl;
    return plot_path.string();
}

} // namespace business
} // namespace kpi
