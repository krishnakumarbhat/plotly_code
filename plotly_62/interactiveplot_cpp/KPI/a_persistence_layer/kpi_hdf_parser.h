#pragma once

#include <hdf5.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include "../b_data_storage/kpi_data_model_storage.h"

namespace kpi {
namespace persistence {

class KPIHDFParser {
public:
    KPIHDFParser(hid_t group, std::shared_ptr<data_storage::KPIDataModelStorage> storage, 
                 const std::vector<int>& scan_index);
    ~KPIHDFParser();

    // Static parse method
    static std::shared_ptr<data_storage::KPIDataModelStorage> parse(
        hid_t group,
        std::shared_ptr<data_storage::KPIDataModelStorage> storage,
        const std::vector<int>& scan_index,
        const std::vector<std::string>& header_names
    );

    // Cleanup method
    void cleanup();

private:
    hid_t root_group_;
    std::shared_ptr<data_storage::KPIDataModelStorage> storage_;
    std::vector<int> scan_index_;
    std::unordered_set<std::string> processed_groups_;
    std::vector<hid_t> grp_list_;
    std::vector<std::pair<std::string, hid_t>> datasets_;

    // Parse group recursively
    void parse_grp(hid_t group, const std::vector<std::string>& header_names);
    
    // Helper functions
    std::string get_group_name(hid_t group);
    int get_group_depth(hid_t group);
    bool is_dataset(hid_t obj_id);
    bool is_group(hid_t obj_id);
    std::vector<std::string> get_all_kpi_keys();
};

} // namespace persistence
} // namespace kpi
