#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../c_data_storage/data_model_storage.h"

namespace interactive_plot {
namespace business {

class DataPrep {
public:
    DataPrep();
    ~DataPrep();

    // Prepare data for visualization
    bool prepare_data(std::shared_ptr<data_storage::DataModelStorage> storage,
                     const std::string& sensor_id);
    
    // Filter data based on criteria
    std::vector<double> filter_data(const std::vector<double>& input_data,
                                   const std::string& filter_type);
    
    // Normalize data
    std::vector<double> normalize_data(const std::vector<double>& input_data);
    
    // Calculate statistics
    struct DataStats {
        double mean;
        double std_dev;
        double min_val;
        double max_val;
        size_t count;
    };
    
    DataStats calculate_statistics(const std::vector<double>& data);

private:
    // Helper methods
    double calculate_mean(const std::vector<double>& data);
    double calculate_std_dev(const std::vector<double>& data, double mean);
};

} // namespace business
} // namespace interactive_plot
