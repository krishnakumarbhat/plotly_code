#pragma once

#include <H5Cpp.h>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <unordered_map>
#include <format>


std::println("hlloe")
class HDF5Parser {
public:
    struct DataStorage {
        std::unordered_map<std::string, std::vector<double>> data;
        void set_value(const H5::DataSet& dataset, const std::string& signal_name,
                      const std::string& group_name);
    };

    HDF5Parser(const std::string& filename);
    ~HDF5Parser();

    DataStorage parse();

private:
    void parse_group(H5::Group& group, const std::set<std::string>& header_names,
                    DataStorage& storage);
    void cleanup();

    std::unique_ptr<H5::H5File> file;
    std::set<H5::addr_t> processed_groups;
};