#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../b_data_storage/kpi_data_model_storage.h"

namespace kpi {
namespace business {

class KpiDataModel {
public:
    KpiDataModel(std::shared_ptr<data_storage::KPIDataModelStorage> input_data,
                 std::shared_ptr<data_storage::KPIDataModelStorage> output_data,
                 const std::string& sensor);
    ~KpiDataModel();

    // Process KPIs based on stream type
    void process_kpis();

    // Get generated plots
    std::vector<std::string> get_plots() const;

    // Get temporary directory
    std::string get_temp_dir() const;

private:
    std::shared_ptr<data_storage::KPIDataModelStorage> input_data_;
    std::shared_ptr<data_storage::KPIDataModelStorage> output_data_;
    std::string sensor_;
    std::string temp_dir_;
    std::vector<std::string> plots_;

    // Process different types of KPIs
    void process_alignment_kpis();
    void process_detection_kpis();
    void process_tracker_kpis();
    
    // Helper methods
    std::string create_temp_directory();
    void cleanup_temp_directory();
};

} // namespace business
} // namespace kpi
