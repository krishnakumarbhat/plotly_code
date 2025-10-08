#pragma once

#include <string>
#include <memory>

namespace interactive_plot {
namespace config {

class JsonParser {
public:
    virtual ~JsonParser() = default;
    virtual bool parse(const std::string& json_content) = 0;
    virtual std::string get_value(const std::string& key) const = 0;
};

class JsonParserFactory {
public:
    static std::unique_ptr<JsonParser> create_parser(const std::string& parser_type);
};

class AllSensorJsonParser : public JsonParser {
public:
    bool parse(const std::string& json_content) override;
    std::string get_value(const std::string& key) const override;

private:
    std::unordered_map<std::string, std::string> parsed_data_;
};

} // namespace config
} // namespace interactive_plot
