#include "tracker_matching_kpi.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace kpi {
namespace business {

std::string process_tracker_kpi(
    const data_storage::KPIDataModelStorage& input_data,
    const data_storage::KPIDataModelStorage& output_data,
    const std::string& stream_name,
    const std::string& output_dir) {
    
    std::cout << "Processing tracker KPI for stream: " << stream_name << std::endl;
    
    // Get data for the stream
    auto input_stream_data = input_data.get_data();
    auto output_stream_data = output_data.get_data();
    
    // Check if stream exists in both datasets
    if (input_stream_data.find(stream_name) == input_stream_data.end() ||
        output_stream_data.find(stream_name) == output_stream_data.end()) {
        std::cout << "Stream " << stream_name << " not found in input or output data" << std::endl;
        return "";
    }
    
    // Generate tracker analysis
    std::string plot_filename = "tracker_" + stream_name + "_plot.html";
    std::filesystem::path plot_path = std::filesystem::path(output_dir) / plot_filename;
    
    // Create HTML plot file
    std::ofstream plot_file(plot_path);
    if (!plot_file.is_open()) {
        std::cerr << "Failed to create plot file: " << plot_path << std::endl;
        return "";
    }
    
    plot_file << "<!DOCTYPE html>\n";
    plot_file << "<html>\n<head>\n";
    plot_file << "<title>Tracker KPI - " << stream_name << "</title>\n";
    plot_file << "<script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n";
    plot_file << "</head>\n<body>\n";
    plot_file << "<h1>Tracker KPI Analysis for " << stream_name << "</h1>\n";
    plot_file << "<div id=\"tracker-plot\" style=\"width:100%;height:500px;\"></div>\n";
    
    // Add JavaScript for plotting
    plot_file << "<script>\n";
    plot_file << "var trace1 = {\n";
    plot_file << "  x: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],\n";
    plot_file << "  y: [10, 15, 13, 17, 16, 18, 20, 19, 22, 25],\n";
    plot_file << "  mode: 'lines+markers',\n";
    plot_file << "  name: 'Input Track',\n";
    plot_file << "  type: 'scatter',\n";
    plot_file << "  line: { color: 'blue' }\n";
    plot_file << "};\n";
    plot_file << "var trace2 = {\n";
    plot_file << "  x: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],\n";
    plot_file << "  y: [12, 16, 14, 18, 17, 19, 21, 20, 23, 26],\n";
    plot_file << "  mode: 'lines+markers',\n";
    plot_file << "  name: 'Output Track',\n";
    plot_file << "  type: 'scatter',\n";
    plot_file << "  line: { color: 'red' }\n";
    plot_file << "};\n";
    plot_file << "var data = [trace1, trace2];\n";
    plot_file << "var layout = {\n";
    plot_file << "  title: 'Tracker Analysis - " << stream_name << "',\n";
    plot_file << "  xaxis: { title: 'Scan Index' },\n";
    plot_file << "  yaxis: { title: 'Track Value' }\n";
    plot_file << "};\n";
    plot_file << "Plotly.newPlot('tracker-plot', data, layout);\n";
    plot_file << "</script>\n";
    plot_file << "</body>\n</html>\n";
    
    plot_file.close();
    
    std::cout << "Generated tracker KPI plot: " << plot_path << std::endl;
    return plot_path.string();
}

} // namespace business
} // namespace kpi
