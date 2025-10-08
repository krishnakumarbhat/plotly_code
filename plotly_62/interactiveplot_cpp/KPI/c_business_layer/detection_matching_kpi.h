#pragma once

#include <string>
#include "../b_data_storage/kpi_data_model_storage.h"

namespace kpi {
namespace business {

// Process detection KPI for a specific stream
std::string process_detection_kpi(
    const data_storage::KPIDataModelStorage& input_data,
    const data_storage::KPIDataModelStorage& output_data,
    const std::string& stream_name,
    const std::string& output_dir
);

} // namespace business
} // namespace kpi
