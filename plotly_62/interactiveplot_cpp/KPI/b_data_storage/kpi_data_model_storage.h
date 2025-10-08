#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <any>
#include <hdf5.h>

namespace kpi {
namespace data_storage {

class KPIDataModelStorage {
public:
    KPIDataModelStorage();
    ~KPIDataModelStorage();

    // Initialize the data container with scan indices
    void initialize(const std::vector<int>& scan_index, const std::string& sensor);
    
    // Reset child counter when starting a new parent group
    void init_parent(const std::string& stream_name);
    
    // Set a value in the storage with group relationship
    std::string set_value(hid_t dataset, const std::string& signal_name, const std::string& grp_name);
    
    // Clear all stored data and reset counters
    void clear();
    
    // Get data for specific signal
    std::unordered_map<std::string, std::any> get_data() const;
    
    // Static method to get data records for specified signal
    static std::unordered_map<std::string, std::any> get_data(
        const KPIDataModelStorage& input_data,
        const KPIDataModelStorage& output_data,
        const std::string& signal_name,
        const std::string& grp_name = ""
    );

private:
    // Bidirectional mapping between values and signals
    std::unordered_map<std::string, std::string> value_to_signal_;
    std::unordered_map<std::string, std::any> signal_to_value_;
    
    // Main data container for storing scan index data
    std::unordered_map<int, std::vector<std::vector<double>>> data_container_;
    
    // Counter for unique identifiers
    int parent_counter_;
    int child_counter_;
    
    // Current stream name
    std::string stream_name_;
    
    // Helper methods
    void process_dataset(hid_t dataset, const std::string& key_stream, 
                        const std::string& signal_name, const std::string& key_grp);
    std::vector<double> read_hdf5_dataset(hid_t dataset);
    std::tuple<double> round_to_2_decimals(const std::any& data);
};

} // namespace data_storage
} // namespace kpi
