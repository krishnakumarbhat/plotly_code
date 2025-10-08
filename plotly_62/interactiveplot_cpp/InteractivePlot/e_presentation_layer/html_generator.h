#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace interactive_plot {
namespace presentation {

struct PlotData {
    std::string plot_id;
    std::string title;
    std::vector<double> x_data;
    std::vector<double> y_data;
    std::string plot_type; // "line", "scatter", "bar", etc.
    std::unordered_map<std::string, std::string> styling;
};

class HtmlGenerator {
public:
    HtmlGenerator();
    ~HtmlGenerator();

    // Generate complete HTML report
    std::string generate_html_report(
        const std::vector<PlotData>& plots,
        const std::string& title,
        const std::string& output_path
    );

    // Generate individual plot HTML
    std::string generate_plot_html(const PlotData& plot);

    // Generate navigation menu
    std::string generate_navigation(const std::vector<PlotData>& plots);

private:
    // Helper methods
    std::string generate_html_header(const std::string& title);
    std::string generate_html_footer();
    std::string generate_plotly_script(const PlotData& plot);
    std::string generate_css_styles();
};

} // namespace presentation
} // namespace interactive_plot
