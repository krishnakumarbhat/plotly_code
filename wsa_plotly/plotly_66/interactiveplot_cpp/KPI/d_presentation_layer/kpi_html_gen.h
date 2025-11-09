#pragma once

#include <string>
#include <vector>

namespace kpi {
namespace presentation {

class KPIHtmlGenerator {
public:
    KPIHtmlGenerator();
    ~KPIHtmlGenerator();

    // Generate HTML report from KPI plots
    std::string generate_report(
        const std::vector<std::string>& plot_files,
        const std::string& sensor_id,
        const std::string& base_name,
        const std::string& output_path
    );

private:
    // Helper methods
    std::string generate_header(const std::string& title);
    std::string generate_footer();
    std::string embed_plot_file(const std::string& plot_file);
    std::string get_plot_type_from_filename(const std::string& filename);
};

} // namespace presentation
} // namespace kpi
