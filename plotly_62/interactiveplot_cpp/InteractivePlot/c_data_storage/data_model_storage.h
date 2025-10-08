#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <any>

namespace interactive_plot {
namespace data_storage {

class DataModelStorage {
public:
    DataModelStorage();
    ~DataModelStorage();

    // Initialize storage with scan indices
    void initialize(const std::vector<int>& scan_index, const std::string& sensor);
    
    // Set value in storage
    std::string set_value(const std::any& value, const std::string& signal_name, const std::string& group_name);
    
    // Get stored data
    std::unordered_map<std::string, std::any> get_data() const;
    
    // Clear all data
    void clear();
    
    // Initialize parent group
    void init_parent(const std::string& parent_name);

private:
    std::unordered_map<std::string, std::string> value_to_signal_;
    std::unordered_map<std::string, std::any> signal_to_value_;
    std::unordered_map<int, std::vector<std::vector<double>>> data_container_;
    
    int parent_counter_;
    int child_counter_;
    std::string current_parent_;
};

} // namespace data_storage
} // namespace interactive_plot
