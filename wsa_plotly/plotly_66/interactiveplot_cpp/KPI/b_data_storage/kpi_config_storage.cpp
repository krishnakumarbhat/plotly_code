#include "kpi_config_storage.h"
#include <iostream>

namespace kpi {
namespace data_storage {

// Global KPI configuration maps
std::unordered_map<std::string, KPIConfig> KPI_ALIGNMENT_CONFIG;
std::unordered_map<std::string, KPIConfig> KPI_DETECTION_CONFIG;
std::unordered_map<std::string, KPIConfig> KPI_TRACKER_CONFIG;

void load_kpi_alignment_config() {
    // Example alignment configuration
    KPIConfig alignment_config;
    alignment_config.signals = {
        {"alignment_stream1", {"signal1", "signal2", "signal3"}},
        {"alignment_stream2", {"signal4", "signal5"}}
    };
    
    KPI_ALIGNMENT_CONFIG["alignment_stream1"] = alignment_config;
    KPI_ALIGNMENT_CONFIG["alignment_stream2"] = alignment_config;
}

void load_kpi_detection_config() {
    // Example detection configuration
    KPIConfig detection_config;
    detection_config.signals = {
        {"detection_stream1", {"det_signal1", "det_signal2"}},
        {"detection_stream2", {"det_signal3", "det_signal4"}}
    };
    
    KPI_DETECTION_CONFIG["detection_stream1"] = detection_config;
    KPI_DETECTION_CONFIG["detection_stream2"] = detection_config;
}

void load_kpi_tracker_config() {
    // Example tracker configuration
    KPIConfig tracker_config;
    tracker_config.signals = {
        {"tracker_stream1", {"track_signal1", "track_signal2"}},
        {"tracker_stream2", {"track_signal3"}}
    };
    
    KPI_TRACKER_CONFIG["tracker_stream1"] = tracker_config;
    KPI_TRACKER_CONFIG["tracker_stream2"] = tracker_config;
}

void initialize_all_kpi_configs() {
    load_kpi_alignment_config();
    load_kpi_detection_config();
    load_kpi_tracker_config();
    
    std::cout << "Initialized KPI configurations:" << std::endl;
    std::cout << "  Alignment streams: " << KPI_ALIGNMENT_CONFIG.size() << std::endl;
    std::cout << "  Detection streams: " << KPI_DETECTION_CONFIG.size() << std::endl;
    std::cout << "  Tracker streams: " << KPI_TRACKER_CONFIG.size() << std::endl;
}

} // namespace data_storage
} // namespace kpi
