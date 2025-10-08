#pragma once

#include <hdf5.h>
#include <string>
#include <vector>
#include <memory>
#include "../c_data_storage/data_model_storage.h"

namespace interactive_plot {
namespace persistence {

class HDFParser {
public:
    HDFParser();
    ~HDFParser();

    // Parse HDF5 file and populate data storage
    bool parse_file(const std::string& file_path, 
                   std::shared_ptr<data_storage::DataModelStorage> storage);
    
    // Parse specific group in HDF5 file
    bool parse_group(hid_t group_id, 
                    std::shared_ptr<data_storage::DataModelStorage> storage,
                    const std::string& group_name = "");
    
    // Get available groups in HDF5 file
    std::vector<std::string> get_groups(const std::string& file_path);
    
    // Get datasets in a specific group
    std::vector<std::string> get_datasets(hid_t group_id);

private:
    // Helper methods
    bool is_valid_hdf5_file(const std::string& file_path);
    std::vector<double> read_dataset(hid_t dataset_id);
    std::string get_group_name(hid_t group_id);
    
    // HDF5 iteration callbacks
    static herr_t group_iterator(hid_t group_id, const char* name, const H5L_info_t* info, void* op_data);
    static herr_t dataset_iterator(hid_t group_id, const char* name, const H5L_info_t* info, void* op_data);
};

} // namespace persistence
} // namespace interactive_plot
