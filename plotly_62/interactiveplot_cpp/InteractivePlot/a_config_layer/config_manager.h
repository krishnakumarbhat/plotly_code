#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace interactive_plot {
namespace config {

struct SensorConfig {
    std::string sensor_id;
    std::vector<std::string> streams;
    std::unordered_map<std::string, std::string> parameters;
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Load configuration from JSON file
    bool load_config(const std::string& config_file_path);
    
    // Get sensor configuration
    std::shared_ptr<SensorConfig> get_sensor_config(const std::string& sensor_id) const;
    
    // Get all sensor IDs
    std::vector<std::string> get_all_sensor_ids() const;
    
    // Check if sensor exists
    bool has_sensor(const std::string& sensor_id) const;

private:
    std::unordered_map<std::string, std::shared_ptr<SensorConfig>> sensor_configs_;
    
    // Helper methods
    void parse_json_config(const std::string& json_content);
};

} // namespace config
} // namespace interactive_plot
