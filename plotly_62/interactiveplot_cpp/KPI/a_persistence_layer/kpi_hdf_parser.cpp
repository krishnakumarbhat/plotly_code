#include "kpi_hdf_parser.h"
#include "../b_data_storage/kpi_config_storage.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace kpi {
namespace persistence {

KPIHDFParser::KPIHDFParser(hid_t group, std::shared_ptr<data_storage::KPIDataModelStorage> storage, 
                           const std::vector<int>& scan_index)
    : root_group_(group), storage_(storage), scan_index_(scan_index) {
}

KPIHDFParser::~KPIHDFParser() {
    cleanup();
}

std::shared_ptr<data_storage::KPIDataModelStorage> KPIHDFParser::parse(
    hid_t group,
    std::shared_ptr<data_storage::KPIDataModelStorage> storage,
    const std::vector<int>& scan_index,
    const std::vector<std::string>& header_names) {
    
    KPIHDFParser parser(group, storage, scan_index);
    parser.parse_grp(parser.root_group_, header_names);
    parser.cleanup();
    
    return storage;
}

void KPIHDFParser::parse_grp(hid_t group, const std::vector<std::string>& header_names) {
    // Get group ID as string for tracking
    char group_name[256];
    H5Iget_name(group, group_name, sizeof(group_name));
    std::string group_id = std::string(group_name);
    
    // Skip if already processed
    if (processed_groups_.find(group_id) != processed_groups_.end()) {
        return;
    }
    processed_groups_.insert(group_id);
    
    std::string current_group_name = get_group_name(group);
    int current_depth = get_group_depth(group);
    
    // Get all KPI keys
    std::vector<std::string> all_keys = get_all_kpi_keys();
    
    bool is_second_layer = (current_depth == 2);
    if (is_second_layer) {
        bool found_in_keys = std::find(all_keys.begin(), all_keys.end(), current_group_name) != all_keys.end();
        if (!found_in_keys) {
            return; // Skip this group and all its children
        }
    }
    
    // Get group info
    hsize_t num_objs;
    H5Gget_num_objs(group, &num_objs);
    
    std::vector<std::pair<std::string, hid_t>> datasets_to_process;
    std::vector<hid_t> child_groups;
    
    // Extract stream name from group path
    std::string current_stream = "";
    std::string group_path = std::string(group_name);
    size_t second_slash = group_path.find('/', 1);
    if (second_slash != std::string::npos) {
        size_t third_slash = group_path.find('/', second_slash + 1);
        if (third_slash != std::string::npos) {
            current_stream = group_path.substr(second_slash + 1, third_slash - second_slash - 1);
        }
    }
    
    // Iterate through group members
    for (hsize_t i = 0; i < num_objs; i++) {
        char member_name[256];
        H5Gget_objname_by_idx(group, i, member_name, sizeof(member_name));
        std::string item_name = std::string(member_name);
        
        hid_t obj_id = H5Gopen2(group, member_name, H5P_DEFAULT);
        if (obj_id < 0) {
            obj_id = H5Dopen2(group, member_name, H5P_DEFAULT);
        }
        
        if (obj_id >= 0) {
            if (is_dataset(obj_id)) {
                // Check if this dataset is in KPI config
                bool found_in_config = false;
                if (!current_stream.empty()) {
                    auto& alignment_config = data_storage::KPI_ALIGNMENT_CONFIG;
                    auto& detection_config = data_storage::KPI_DETECTION_CONFIG;
                    auto& tracker_config = data_storage::KPI_TRACKER_CONFIG;
                    
                    if (alignment_config.find(current_stream) != alignment_config.end() ||
                        detection_config.find(current_stream) != detection_config.end() ||
                        tracker_config.find(current_stream) != tracker_config.end()) {
                        found_in_config = true;
                    }
                }
                
                if (found_in_config) {
                    datasets_to_process.push_back({item_name, obj_id});
                }
            } else if (is_group(obj_id)) {
                // Check if not in header names
                bool is_header = std::find(header_names.begin(), header_names.end(), item_name) != header_names.end();
                if (!is_header) {
                    child_groups.push_back(obj_id);
                }
            }
        }
    }
    
    // Process datasets immediately
    for (const auto& dataset_pair : datasets_to_process) {
        storage_->set_value(dataset_pair.second, dataset_pair.first, current_group_name);
        H5Dclose(dataset_pair.second);
    }
    
    // Process child groups recursively
    for (hid_t child_group : child_groups) {
        parse_grp(child_group, header_names);
        H5Gclose(child_group);
    }
}

std::string KPIHDFParser::get_group_name(hid_t group) {
    char name[256];
    H5Iget_name(group, name, sizeof(name));
    std::string full_name = std::string(name);
    
    // Extract last part after final '/'
    size_t last_slash = full_name.find_last_of('/');
    if (last_slash != std::string::npos) {
        return full_name.substr(last_slash + 1);
    }
    return full_name;
}

int KPIHDFParser::get_group_depth(hid_t group) {
    char name[256];
    H5Iget_name(group, name, sizeof(name));
    std::string full_name = std::string(name);
    
    // Count slashes to determine depth
    int depth = 0;
    for (char c : full_name) {
        if (c == '/') depth++;
    }
    return depth - 1; // Root is depth 0
}

bool KPIHDFParser::is_dataset(hid_t obj_id) {
    H5I_type_t obj_type = H5Iget_type(obj_id);
    return obj_type == H5I_DATASET;
}

bool KPIHDFParser::is_group(hid_t obj_id) {
    H5I_type_t obj_type = H5Iget_type(obj_id);
    return obj_type == H5I_GROUP;
}

std::vector<std::string> KPIHDFParser::get_all_kpi_keys() {
    std::vector<std::string> all_keys;
    
    // Collect keys from all KPI configs
    for (const auto& pair : data_storage::KPI_ALIGNMENT_CONFIG) {
        all_keys.push_back(pair.first);
    }
    for (const auto& pair : data_storage::KPI_DETECTION_CONFIG) {
        all_keys.push_back(pair.first);
    }
    for (const auto& pair : data_storage::KPI_TRACKER_CONFIG) {
        all_keys.push_back(pair.first);
    }
    
    return all_keys;
}

void KPIHDFParser::cleanup() {
    grp_list_.clear();
    datasets_.clear();
    processed_groups_.clear();
}

} // namespace persistence
} // namespace kpi
