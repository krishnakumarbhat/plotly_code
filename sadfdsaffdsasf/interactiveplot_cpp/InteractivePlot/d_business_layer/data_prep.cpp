#include "data_prep.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>

namespace interactive_plot {
namespace business {

DataPrep::DataPrep() = default;
DataPrep::~DataPrep() = default;

bool DataPrep::prepare_data(std::shared_ptr<data_storage::DataModelStorage> storage,
                           const std::string& sensor_id) {
    if (!storage) {
        std::cerr << "Invalid storage pointer" << std::endl;
        return false;
    }
    
    auto data = storage->get_data();
    std::cout << "Preparing data for sensor " << sensor_id << " with " 
              << data.size() << " signals" << std::endl;
    
    // Process each signal in the storage
    for (const auto& signal_pair : data) {
        std::cout << "Processing signal: " << signal_pair.first << std::endl;
    }
    
    return true;
}

std::vector<double> DataPrep::filter_data(const std::vector<double>& input_data,
                                         const std::string& filter_type) {
    std::vector<double> filtered_data = input_data;
    
    if (filter_type == "lowpass") {
        // Simple moving average filter
        size_t window_size = 3;
        for (size_t i = window_size; i < filtered_data.size(); ++i) {
            double sum = 0.0;
            for (size_t j = i - window_size; j <= i; ++j) {
                sum += input_data[j];
            }
            filtered_data[i] = sum / (window_size + 1);
        }
    } else if (filter_type == "highpass") {
        // Simple difference filter
        for (size_t i = 1; i < filtered_data.size(); ++i) {
            filtered_data[i] = input_data[i] - input_data[i-1];
        }
    }
    
    return filtered_data;
}

std::vector<double> DataPrep::normalize_data(const std::vector<double>& input_data) {
    if (input_data.empty()) {
        return input_data;
    }
    
    auto min_it = std::min_element(input_data.begin(), input_data.end());
    auto max_it = std::max_element(input_data.begin(), input_data.end());
    
    double min_val = *min_it;
    double max_val = *max_it;
    double range = max_val - min_val;
    
    std::vector<double> normalized_data;
    normalized_data.reserve(input_data.size());
    
    if (range > 0) {
        for (double value : input_data) {
            normalized_data.push_back((value - min_val) / range);
        }
    } else {
        normalized_data = input_data; // All values are the same
    }
    
    return normalized_data;
}

DataPrep::DataStats DataPrep::calculate_statistics(const std::vector<double>& data) {
    DataStats stats = {};
    
    if (data.empty()) {
        return stats;
    }
    
    stats.count = data.size();
    stats.mean = calculate_mean(data);
    stats.std_dev = calculate_std_dev(data, stats.mean);
    
    auto minmax = std::minmax_element(data.begin(), data.end());
    stats.min_val = *minmax.first;
    stats.max_val = *minmax.second;
    
    return stats;
}

double DataPrep::calculate_mean(const std::vector<double>& data) {
    if (data.empty()) {
        return 0.0;
    }
    
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

double DataPrep::calculate_std_dev(const std::vector<double>& data, double mean) {
    if (data.size() <= 1) {
        return 0.0;
    }
    
    double sum_sq_diff = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sum_sq_diff += diff * diff;
    }
    
    return std::sqrt(sum_sq_diff / (data.size() - 1));
}

} // namespace business
} // namespace interactive_plot
