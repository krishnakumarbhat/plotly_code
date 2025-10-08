#include "hdf_parser.h"
#include <iostream>
#include <filesystem>

namespace interactive_plot {
namespace persistence {

HDFParser::HDFParser() = default;
HDFParser::~HDFParser() = default;

bool HDFParser::parse_file(const std::string& file_path, 
                          std::shared_ptr<data_storage::DataModelStorage> storage) {
    if (!is_valid_hdf5_file(file_path)) {
        std::cerr << "Invalid HDF5 file: " << file_path << std::endl;
        return false;
    }
    
    hid_t file_id = H5Fopen(file_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) {
        std::cerr << "Failed to open HDF5 file: " << file_path << std::endl;
        return false;
    }
    
    bool success = parse_group(file_id, storage, "/");
    
    H5Fclose(file_id);
    return success;
}

bool HDFParser::parse_group(hid_t group_id, 
                           std::shared_ptr<data_storage::DataModelStorage> storage,
                           const std::string& group_name) {
    if (!storage) {
        std::cerr << "Invalid storage pointer" << std::endl;
        return false;
    }
    
    std::cout << "Parsing HDF5 group: " << group_name << std::endl;
    
    // Initialize parent for this group
    storage->init_parent(group_name);
    
    // Iterate through all objects in the group
    herr_t status = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, 
                              nullptr, group_iterator, storage.get());
    
    return status >= 0;
}

std::vector<std::string> HDFParser::get_groups(const std::string& file_path) {
    std::vector<std::string> groups;
    
    if (!is_valid_hdf5_file(file_path)) {
        return groups;
    }
    
    hid_t file_id = H5Fopen(file_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file_id < 0) {
        return groups;
    }
    
    // Use H5Literate to find all groups
    H5Literate(file_id, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, group_iterator, &groups);
    
    H5Fclose(file_id);
    return groups;
}

std::vector<std::string> HDFParser::get_datasets(hid_t group_id) {
    std::vector<std::string> datasets;
    
    H5Literate(group_id, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, dataset_iterator, &datasets);
    
    return datasets;
}

bool HDFParser::is_valid_hdf5_file(const std::string& file_path) {
    if (!std::filesystem::exists(file_path)) {
        return false;
    }
    
    // Check if file is a valid HDF5 file
    htri_t is_hdf5 = H5Fis_hdf5(file_path.c_str());
    return is_hdf5 > 0;
}

std::vector<double> HDFParser::read_dataset(hid_t dataset_id) {
    std::vector<double> data;
    
    // Get dataspace
    hid_t dataspace = H5Dget_space(dataset_id);
    if (dataspace < 0) {
        return data;
    }
    
    // Get dimensions
    int ndims = H5Sget_simple_extent_ndims(dataspace);
    if (ndims <= 0) {
        H5Sclose(dataspace);
        return data;
    }
    
    std::vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(dataspace, dims.data(), nullptr);
    
    // Calculate total number of elements
    hsize_t total_elements = 1;
    for (int i = 0; i < ndims; ++i) {
        total_elements *= dims[i];
    }
    
    // Read data
    data.resize(total_elements);
    herr_t status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
    
    H5Sclose(dataspace);
    
    if (status < 0) {
        data.clear();
    }
    
    return data;
}

std::string HDFParser::get_group_name(hid_t group_id) {
    char name[256];
    H5Iget_name(group_id, name, sizeof(name));
    return std::string(name);
}

herr_t HDFParser::group_iterator(hid_t group_id, const char* name, const H5L_info_t* info, void* op_data) {
    H5O_info_t obj_info;
    H5Oget_info_by_name(group_id, name, &obj_info, H5P_DEFAULT);
    
    if (obj_info.type == H5O_TYPE_GROUP) {
        auto* groups = static_cast<std::vector<std::string>*>(op_data);
        if (groups) {
            groups->push_back(std::string(name));
        }
    }
    
    return 0; // Continue iteration
}

herr_t HDFParser::dataset_iterator(hid_t group_id, const char* name, const H5L_info_t* info, void* op_data) {
    H5O_info_t obj_info;
    H5Oget_info_by_name(group_id, name, &obj_info, H5P_DEFAULT);
    
    if (obj_info.type == H5O_TYPE_DATASET) {
        auto* datasets = static_cast<std::vector<std::string>*>(op_data);
        if (datasets) {
            datasets->push_back(std::string(name));
        }
    }
    
    return 0; // Continue iteration
}

} // namespace persistence
} // namespace interactive_plot
