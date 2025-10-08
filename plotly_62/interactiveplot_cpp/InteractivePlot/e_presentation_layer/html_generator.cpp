#include "html_generator.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace interactive_plot {
namespace presentation {

HtmlGenerator::HtmlGenerator() = default;
HtmlGenerator::~HtmlGenerator() = default;

std::string HtmlGenerator::generate_html_report(
    const std::vector<PlotData>& plots,
    const std::string& title,
    const std::string& output_path) {
    
    std::ofstream html_file(output_path);
    if (!html_file.is_open()) {
        std::cerr << "Failed to create HTML file: " << output_path << std::endl;
        return "";
    }
    
    // Write HTML structure
    html_file << generate_html_header(title);
    html_file << "<body>\n";
    html_file << "<div class=\"container\">\n";
    html_file << "<h1>" << title << "</h1>\n";
    
    // Add navigation
    html_file << generate_navigation(plots);
    
    // Add plots
    html_file << "<div class=\"plots-container\">\n";
    for (const auto& plot : plots) {
        html_file << generate_plot_html(plot);
    }
    html_file << "</div>\n";
    
    html_file << "</div>\n";
    html_file << generate_html_footer();
    
    html_file.close();
    
    std::cout << "Generated HTML report: " << output_path << std::endl;
    return output_path;
}

std::string HtmlGenerator::generate_plot_html(const PlotData& plot) {
    std::stringstream ss;
    
    ss << "<div class=\"plot-section\" id=\"" << plot.plot_id << "\">\n";
    ss << "  <h2>" << plot.title << "</h2>\n";
    ss << "  <div id=\"plot-" << plot.plot_id << "\" class=\"plot-container\"></div>\n";
    ss << "  " << generate_plotly_script(plot) << "\n";
    ss << "</div>\n";
    
    return ss.str();
}

std::string HtmlGenerator::generate_navigation(const std::vector<PlotData>& plots) {
    std::stringstream ss;
    
    ss << "<nav class=\"navigation\">\n";
    ss << "  <h2>Plots</h2>\n";
    ss << "  <ul>\n";
    
    for (const auto& plot : plots) {
        ss << "    <li><a href=\"#" << plot.plot_id << "\">" << plot.title << "</a></li>\n";
    }
    
    ss << "  </ul>\n";
    ss << "</nav>\n";
    
    return ss.str();
}

std::string HtmlGenerator::generate_html_header(const std::string& title) {
    std::stringstream ss;
    
    ss << "<!DOCTYPE html>\n";
    ss << "<html lang=\"en\">\n";
    ss << "<head>\n";
    ss << "  <meta charset=\"UTF-8\">\n";
    ss << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << "  <title>" << title << "</title>\n";
    ss << "  <script src=\"https://cdn.plot.ly/plotly-latest.min.js\"></script>\n";
    ss << "  <style>\n" << generate_css_styles() << "  </style>\n";
    ss << "</head>\n";
    
    return ss.str();
}

std::string HtmlGenerator::generate_html_footer() {
    return "</body>\n</html>\n";
}

std::string HtmlGenerator::generate_plotly_script(const PlotData& plot) {
    std::stringstream ss;
    
    ss << "<script>\n";
    ss << "  var trace = {\n";
    ss << "    x: [";
    for (size_t i = 0; i < plot.x_data.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << plot.x_data[i];
    }
    ss << "],\n";
    
    ss << "    y: [";
    for (size_t i = 0; i < plot.y_data.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << plot.y_data[i];
    }
    ss << "],\n";
    
    ss << "    type: '" << plot.plot_type << "',\n";
    ss << "    name: '" << plot.title << "'\n";
    ss << "  };\n";
    
    ss << "  var layout = {\n";
    ss << "    title: '" << plot.title << "',\n";
    ss << "    xaxis: { title: 'X Axis' },\n";
    ss << "    yaxis: { title: 'Y Axis' }\n";
    ss << "  };\n";
    
    ss << "  Plotly.newPlot('plot-" << plot.plot_id << "', [trace], layout);\n";
    ss << "</script>\n";
    
    return ss.str();
}

std::string HtmlGenerator::generate_css_styles() {
    return R"(
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f5f5f5;
    }
    .container {
      max-width: 1200px;
      margin: 0 auto;
      background-color: white;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    h1 {
      color: #333;
      border-bottom: 2px solid #007bff;
      padding-bottom: 10px;
    }
    .navigation {
      background-color: #f8f9fa;
      padding: 15px;
      border-radius: 5px;
      margin: 20px 0;
    }
    .navigation ul {
      list-style-type: none;
      padding: 0;
    }
    .navigation li {
      margin: 5px 0;
    }
    .navigation a {
      color: #007bff;
      text-decoration: none;
    }
    .navigation a:hover {
      text-decoration: underline;
    }
    .plot-section {
      margin: 30px 0;
      padding: 20px;
      border: 1px solid #ddd;
      border-radius: 5px;
    }
    .plot-container {
      width: 100%;
      height: 500px;
    }
)";
}

} // namespace presentation
} // namespace interactive_plot
