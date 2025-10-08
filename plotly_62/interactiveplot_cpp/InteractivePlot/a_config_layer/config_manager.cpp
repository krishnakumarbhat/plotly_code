#include "config_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace interactive_plot {
namespace config {

ConfigManager::ConfigManager() = default;
ConfigManager::~ConfigManager() = default;

bool ConfigManager::load_config(const std::string& config_file_path) {
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file_path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << config_file.rdbuf();
    std::string json_content = buffer.str();
    config_file.close();
    
    try {
        parse_json_config(json_content);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<SensorConfig> ConfigManager::get_sensor_config(const std::string& sensor_id) const {
    auto it = sensor_configs_.find(sensor_id);
    if (it != sensor_configs_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> ConfigManager::get_all_sensor_ids() const {
    std::vector<std::string> sensor_ids;
    for (const auto& pair : sensor_configs_) {
        sensor_ids.push_back(pair.first);
    }
    return sensor_ids;
}

bool ConfigManager::has_sensor(const std::string& sensor_id) const {
    return sensor_configs_.find(sensor_id) != sensor_configs_.end();
}

void ConfigManager::parse_json_config(const std::string& json_content) {
    // Simplified JSON parsing - in production, use a proper JSON library like nlohmann/json
    // For now, create sample configuration
    
    auto sensor1 = std::make_shared<SensorConfig>();
    sensor1->sensor_id = "sensor1";
    sensor1->streams = {"stream1", "stream2", "stream3"};
    sensor1->parameters["sampling_rate"] = "1000";
    sensor1->parameters["data_type"] = "float64";
    
    auto sensor2 = std::make_shared<SensorConfig>();
    sensor2->sensor_id = "sensor2";
    sensor2->streams = {"stream4", "stream5"};
    sensor2->parameters["sampling_rate"] = "500";
    sensor2->parameters["data_type"] = "int32";
    
    sensor_configs_["sensor1"] = sensor1;
    sensor_configs_["sensor2"] = sensor2;
    
    std::cout << "Loaded configuration for " << sensor_configs_.size() << " sensors" << std::endl;
}

} // namespace config
} // namespace interactive_plot
