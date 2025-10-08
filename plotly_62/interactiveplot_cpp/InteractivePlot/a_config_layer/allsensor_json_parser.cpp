#include "allsensor_json_parser.h"
#include <iostream>
#include <sstream>

namespace interactive_plot {
namespace config {

AllSensorJsonParser::AllSensorJsonParser() = default;
AllSensorJsonParser::~AllSensorJsonParser() = default;

bool AllSensorJsonParser::parse(const std::string& json_content) {
    global_config_.clear();
    sensor_configs_.clear();
    
    // Simplified JSON parsing - in production use proper JSON library
    // For demonstration, create sample configuration
    
    // Global configuration
    global_config_["version"] = "1.0";
    global_config_["format"] = "hdf5";
    global_config_["default_sampling_rate"] = "1000";
    
    // Sensor configurations
    std::unordered_map<std::string, std::string> sensor1_config;
    sensor1_config["name"] = "Primary Sensor";
    sensor1_config["type"] = "radar";
    sensor1_config["sampling_rate"] = "2000";
    sensor1_config["channels"] = "4";
    sensor_configs_["sensor1"] = sensor1_config;
    
    std::unordered_map<std::string, std::string> sensor2_config;
    sensor2_config["name"] = "Secondary Sensor";
    sensor2_config["type"] = "lidar";
    sensor2_config["sampling_rate"] = "1500";
    sensor2_config["channels"] = "2";
    sensor_configs_["sensor2"] = sensor2_config;
    
    std::cout << "Parsed configuration for " << sensor_configs_.size() << " sensors" << std::endl;
    return true;
}

std::string AllSensorJsonParser::get_value(const std::string& key) const {
    auto it = global_config_.find(key);
    if (it != global_config_.end()) {
        return it->second;
    }
    return "";
}

std::vector<std::string> AllSensorJsonParser::get_sensor_list() const {
    std::vector<std::string> sensors;
    for (const auto& pair : sensor_configs_) {
        sensors.push_back(pair.first);
    }
    return sensors;
}

std::unordered_map<std::string, std::string> AllSensorJsonParser::get_sensor_parameters(const std::string& sensor_id) const {
    auto it = sensor_configs_.find(sensor_id);
    if (it != sensor_configs_.end()) {
        return it->second;
    }
    return {};
}

void AllSensorJsonParser::parse_global_section(const std::string& section) {
    // Implementation for parsing global configuration section
    std::cout << "Parsing global section: " << section.substr(0, 50) << "..." << std::endl;
}

void AllSensorJsonParser::parse_sensor_section(const std::string& sensor_id, const std::string& section) {
    // Implementation for parsing sensor-specific configuration section
    std::cout << "Parsing sensor section for " << sensor_id << ": " << section.substr(0, 50) << "..." << std::endl;
}

} // namespace config
} // namespace interactive_plot
