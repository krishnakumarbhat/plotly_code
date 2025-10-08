#include "json_parser_factory.h"
#include <unordered_map>
#include <iostream>

namespace interactive_plot {
namespace config {

std::unique_ptr<JsonParser> JsonParserFactory::create_parser(const std::string& parser_type) {
    if (parser_type == "allsensor") {
        return std::make_unique<AllSensorJsonParser>();
    }
    return nullptr;
}

bool AllSensorJsonParser::parse(const std::string& json_content) {
    // Simplified JSON parsing - in production use proper JSON library
    parsed_data_.clear();
    
    // Sample parsing logic
    parsed_data_["sensor_count"] = "2";
    parsed_data_["default_sampling_rate"] = "1000";
    parsed_data_["data_format"] = "hdf5";
    
    std::cout << "Parsed JSON configuration with " << parsed_data_.size() << " parameters" << std::endl;
    return true;
}

std::string AllSensorJsonParser::get_value(const std::string& key) const {
    auto it = parsed_data_.find(key);
    if (it != parsed_data_.end()) {
        return it->second;
    }
    return "";
}

} // namespace config
} // namespace interactive_plot
