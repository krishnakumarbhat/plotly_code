#include "config_storage.h"
#include <iostream>

namespace interactive_plot {
namespace data_storage {

ConfigStorage::ConfigStorage() = default;
ConfigStorage::~ConfigStorage() = default;

void ConfigStorage::store_plot_config(const std::string& config_id, const PlotConfig& config) {
    plot_configs_[config_id] = config;
    std::cout << "Stored plot configuration: " << config_id << std::endl;
}

PlotConfig ConfigStorage::get_plot_config(const std::string& config_id) const {
    auto it = plot_configs_.find(config_id);
    if (it != plot_configs_.end()) {
        return it->second;
    }
    return PlotConfig{}; // Return empty config if not found
}

bool ConfigStorage::has_config(const std::string& config_id) const {
    return plot_configs_.find(config_id) != plot_configs_.end();
}

std::vector<std::string> ConfigStorage::get_all_config_ids() const {
    std::vector<std::string> config_ids;
    for (const auto& pair : plot_configs_) {
        config_ids.push_back(pair.first);
    }
    return config_ids;
}

void ConfigStorage::clear() {
    plot_configs_.clear();
}

} // namespace data_storage
} // namespace interactive_plot
