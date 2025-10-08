#include "data_model_storage.h"
#include <iostream>
#include <algorithm>

namespace interactive_plot {
namespace data_storage {

DataModelStorage::DataModelStorage() 
    : parent_counter_(-1), child_counter_(-1) {
}

DataModelStorage::~DataModelStorage() {
    clear();
}

void DataModelStorage::initialize(const std::vector<int>& scan_index, const std::string& sensor) {
    data_container_.clear();
    for (int idx : scan_index) {
        data_container_[idx] = std::vector<std::vector<double>>();
    }
    std::cout << "Initialized data storage for sensor " << sensor << " with " 
              << scan_index.size() << " scan indices" << std::endl;
}

void DataModelStorage::init_parent(const std::string& parent_name) {
    parent_counter_++;
    child_counter_ = -1;
    current_parent_ = parent_name;
}

std::string DataModelStorage::set_value(const std::any& value, const std::string& signal_name, const std::string& group_name) {
    child_counter_++;
    std::string key = std::to_string(parent_counter_) + "_" + std::to_string(child_counter_);
    
    // Update mappings
    value_to_signal_[key] = signal_name;
    signal_to_value_[signal_name] = value;
    
    return key;
}

std::unordered_map<std::string, std::any> DataModelStorage::get_data() const {
    return signal_to_value_;
}

void DataModelStorage::clear() {
    value_to_signal_.clear();
    signal_to_value_.clear();
    data_container_.clear();
    parent_counter_ = -1;
    child_counter_ = -1;
}

} // namespace data_storage
} // namespace interactive_plot
