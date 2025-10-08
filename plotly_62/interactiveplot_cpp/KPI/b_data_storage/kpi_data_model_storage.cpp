#include "kpi_data_model_storage.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <set>

namespace kpi {
namespace data_storage {

KPIDataModelStorage::KPIDataModelStorage() 
    : parent_counter_(-1), child_counter_(-1) {
}

KPIDataModelStorage::~KPIDataModelStorage() {
    clear();
}

void KPIDataModelStorage::initialize(const std::vector<int>& scan_index, const std::string& sensor) {
    if (!scan_index.empty()) {
        // Check if scan_index is sorted and sequential
        auto min_idx = *std::min_element(scan_index.begin(), scan_index.end());
        auto max_idx = *std::max_element(scan_index.begin(), scan_index.end());
        
        std::vector<int> expected_scan_index;
        for (int i = min_idx; i <= max_idx; ++i) {
            expected_scan_index.push_back(i);
        }
        
        // Find duplicates
        std::unordered_map<int, int> freq;
        std::vector<int> duplicates;
        for (int idx : scan_index) {
            freq[idx]++;
        }
        for (const auto& pair : freq) {
            if (pair.second > 1) {
                duplicates.push_back(pair.first);
            }
        }
        
        // Find missing indices
        std::set<int> scan_set(scan_index.begin(), scan_index.end());
        std::vector<int> missing_indices;
        for (int expected : expected_scan_index) {
            if (scan_set.find(expected) == scan_set.end()) {
                missing_indices.push_back(expected);
            }
        }
        
        if (!missing_indices.empty()) {
            std::cout << "Missing scan indices at sensor " << sensor << ": ";
            for (int idx : missing_indices) std::cout << idx << " ";
            std::cout << std::endl;
        }
        
        if (!duplicates.empty()) {
            std::cout << "Duplicate scan indices at " << sensor << ": ";
            for (int idx : duplicates) std::cout << idx << " ";
            std::cout << std::endl;
        }
    }
    
    // Initialize data container
    data_container_.clear();
    for (int idx : scan_index) {
        data_container_[idx] = std::vector<std::vector<double>>();
    }
}

void KPIDataModelStorage::init_parent(const std::string& stream_name) {
    parent_counter_++;
    child_counter_ = -1;
    stream_name_ = stream_name;
}

std::string KPIDataModelStorage::set_value(hid_t dataset, const std::string& signal_name, const std::string& grp_name) {
    // Check if this is a new parent group
    bool is_new_parent = (signal_to_value_.find(grp_name) == signal_to_value_.end() && child_counter_ == -1);
    
    if (is_new_parent) {
        // Handle new parent group
        std::string key_grp = std::to_string(parent_counter_) + "_None";
        child_counter_++;
        std::string key_stream = std::to_string(parent_counter_) + "_" + std::to_string(child_counter_);
        
        // Process and store the data
        process_dataset(dataset, key_stream, signal_name, key_grp);
        return key_stream;
    } else {
        // Handle child item
        child_counter_++;
        std::string key = std::to_string(parent_counter_) + "_" + std::to_string(child_counter_);
        
        // Read dataset
        std::vector<double> dataset_values = read_hdf5_dataset(dataset);
        size_t dataset_len = dataset_values.size();
        size_t container_len = data_container_.size();
        
        // Skip if lengths don't match
        if (dataset_len != container_len) {
            std::cout << "Skipping child processing for " << signal_name << " in " << grp_name 
                     << ": dataset length (" << dataset_len << ") does not match scan indices length (" 
                     << container_len << ")" << std::endl;
            return key;
        }
        
        // Process data for child
        size_t idx = 0;
        for (auto& container_pair : data_container_) {
            if (idx < dataset_values.size()) {
                double rounded_value = std::round(dataset_values[idx] * 100.0) / 100.0;
                if (!container_pair.second.empty()) {
                    container_pair.second.back().push_back(rounded_value);
                }
            }
            idx++;
        }
        
        // Update mappings
        value_to_signal_[key] = signal_name;
        
        // Update signal-to-value mapping
        if (signal_to_value_.find(signal_name) == signal_to_value_.end()) {
            std::unordered_map<std::string, std::string> grp_map;
            grp_map[grp_name] = key;
            signal_to_value_[signal_name] = grp_map;
        }
        
        return key;
    }
}

void KPIDataModelStorage::process_dataset(hid_t dataset, const std::string& key_stream, 
                                        const std::string& signal_name, const std::string& key_grp) {
    std::vector<double> dataset_values = read_hdf5_dataset(dataset);
    size_t dataset_len = dataset_values.size();
    size_t container_len = data_container_.size();
    
    // Skip if lengths don't match
    if (dataset_len != container_len) {
        std::cout << "Skipping plot for " << signal_name << ": dataset length (" << dataset_len 
                 << ") does not match scan indices length (" << container_len << ")" << std::endl;
        return;
    }
    
    // Process all rows in the dataset and store them
    size_t idx = 0;
    for (auto& container_pair : data_container_) {
        if (idx < dataset_values.size()) {
            double rounded_value = std::round(dataset_values[idx] * 100.0) / 100.0;
            container_pair.second.push_back(std::vector<double>{rounded_value});
        }
        idx++;
    }
    
    // Update mappings
    value_to_signal_[key_stream] = signal_name;
    value_to_signal_[key_grp] = stream_name_;
    signal_to_value_[signal_name] = key_stream;
    signal_to_value_[stream_name_] = key_grp;
}

std::vector<double> KPIDataModelStorage::read_hdf5_dataset(hid_t dataset) {
    std::vector<double> values;
    
    // Get dataspace
    hid_t dataspace = H5Dget_space(dataset);
    if (dataspace < 0) return values;
    
    // Get dimensions
    int ndims = H5Sget_simple_extent_ndims(dataspace);
    if (ndims <= 0) {
        H5Sclose(dataspace);
        return values;
    }
    
    std::vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace, dims.data(), nullptr);
    
    // Calculate total number of elements
    hsize_t total_elements = 1;
    for (int i = 0; i < ndims; ++i) {
        total_elements *= dims[i];
    }
    
    // Read data
    values.resize(total_elements);
    herr_t status = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data());
    
    H5Sclose(dataspace);
    
    if (status < 0) {
        values.clear();
    }
    
    return values;
}

void KPIDataModelStorage::clear() {
    value_to_signal_.clear();
    signal_to_value_.clear();
    data_container_.clear();
    parent_counter_ = -1;
    child_counter_ = -1;
}

std::unordered_map<std::string, std::any> KPIDataModelStorage::get_data() const {
    std::unordered_map<std::string, std::any> result;
    
    // Convert internal data structure to return format
    for (const auto& pair : signal_to_value_) {
        result[pair.first] = pair.second;
    }
    
    return result;
}

std::unordered_map<std::string, std::any> KPIDataModelStorage::get_data(
    const KPIDataModelStorage& input_data,
    const KPIDataModelStorage& output_data,
    const std::string& signal_name,
    const std::string& grp_name) {
    
    std::unordered_map<std::string, std::any> data_dict;
    
    // Initialize result structure
    data_dict["SI"] = std::vector<int>();
    data_dict["I"] = std::vector<double>();
    data_dict["O"] = std::vector<double>();
    data_dict["MI"] = std::vector<std::vector<double>>(2);
    data_dict["MO"] = std::vector<std::vector<double>>(2);
    data_dict["match"] = 0;
    data_dict["mismatch"] = 0;
    
    // Implementation would continue with data matching logic similar to Python version
    // This is a simplified version for demonstration
    
    return data_dict;
}

} // namespace data_storage
} // namespace kpi
