#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace interactive_plot {
namespace data_storage {

struct PlotConfig {
    std::string plot_type;
    std::string title;
    std::vector<std::string> signals;
    std::unordered_map<std::string, std::string> parameters;
};

class ConfigStorage {
public:
    ConfigStorage();
    ~ConfigStorage();

    // Store plot configuration
    void store_plot_config(const std::string& config_id, const PlotConfig& config);
    
    // Retrieve plot configuration
    PlotConfig get_plot_config(const std::string& config_id) const;
    
    // Check if configuration exists
    bool has_config(const std::string& config_id) const;
    
    // Get all configuration IDs
    std::vector<std::string> get_all_config_ids() const;
    
    // Clear all configurations
    void clear();

private:
    std::unordered_map<std::string, PlotConfig> plot_configs_;
};

} // namespace data_storage
} // namespace interactive_plot
