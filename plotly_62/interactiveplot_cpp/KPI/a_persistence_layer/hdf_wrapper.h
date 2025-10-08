#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <hdf5.h>
#include "../b_data_storage/kpi_data_model_storage.h"

namespace kpi {
namespace persistence {

struct KPIProcessingConfig {
    std::string sensor_id;
    std::string input_file_path;
    std::string output_file_path;
    std::string output_dir;
    std::string base_name;
    std::string kpi_subdir = "kpi";
};

class KPIHDFWrapper {
public:
    explicit KPIHDFWrapper(const KPIProcessingConfig& config);
    ~KPIHDFWrapper();

    // Parse configured KPI streams from input/output HDF5 files
    std::unordered_map<std::string, std::any> parse();

private:
    KPIProcessingConfig config_;
    std::chrono::steady_clock::time_point start_time_parsing_;
    std::vector<std::string> header_variants_;
    std::shared_ptr<data_storage::KPIDataModelStorage> stream_input_model_;
    std::shared_ptr<data_storage::KPIDataModelStorage> stream_output_model_;

    // Helper methods
    std::vector<std::string> collect_streams();
    std::vector<int> get_scan_index(hid_t hdf_file, const std::vector<std::string>& streams, const std::string& sensor);
    bool file_exists(const std::string& path);
};

// Utility function
std::string parse_for_kpi(
    const std::string& sensor_id,
    const std::string& input_file_path,
    const std::string& output_dir,
    const std::string& base_name,
    const std::string& kpi_subdir,
    const std::string& output_file_path
);

} // namespace persistence
} // namespace kpi
