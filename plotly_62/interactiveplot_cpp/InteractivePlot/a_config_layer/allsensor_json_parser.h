#pragma once

#include "json_parser_factory.h"
#include <unordered_map>
#include <vector>

namespace interactive_plot {
namespace config {

class AllSensorJsonParser : public JsonParser {
public:
    AllSensorJsonParser();
    ~AllSensorJsonParser();

    bool parse(const std::string& json_content) override;
    std::string get_value(const std::string& key) const override;
    
    // Additional methods specific to all-sensor parsing
    std::vector<std::string> get_sensor_list() const;
    std::unordered_map<std::string, std::string> get_sensor_parameters(const std::string& sensor_id) const;

private:
    std::unordered_map<std::string, std::string> global_config_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sensor_configs_;
    
    void parse_global_section(const std::string& section);
    void parse_sensor_section(const std::string& sensor_id, const std::string& section);
};

} // namespace config
} // namespace interactive_plot
