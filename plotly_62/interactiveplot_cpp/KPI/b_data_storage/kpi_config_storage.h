#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace kpi {
namespace data_storage {

// KPI Configuration structures
struct KPIConfig {
    std::unordered_map<std::string, std::vector<std::string>> signals;
};

// Global KPI configuration maps
extern std::unordered_map<std::string, KPIConfig> KPI_ALIGNMENT_CONFIG;
extern std::unordered_map<std::string, KPIConfig> KPI_DETECTION_CONFIG;
extern std::unordered_map<std::string, KPIConfig> KPI_TRACKER_CONFIG;

// Configuration loader functions
void load_kpi_alignment_config();
void load_kpi_detection_config();
void load_kpi_tracker_config();
void initialize_all_kpi_configs();

} // namespace data_storage
} // namespace kpi
