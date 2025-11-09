#include "hdf5_parser.hpp"
#include <iostream>

// Define Gen7V1_V2 as a placeholder
const std::set<std::string> Gen7V1_V2 = {"Stream_0", "Stream_1"};

HDF5Parser::HDF5Parser(const std::string& filename) {
    try {
        file = std::make_unique<H5::H5File>(filename, H5F_ACC_RDONLY);
    } catch (H5::FileIException& e) {
        std::cerr << "Error opening HDF5 file: " << e.getCDetailMsg() << std::endl;
        throw;
    }
}

HDF5Parser::~HDF5Parser() {
    cleanup();
}

HDF5Parser::DataStorage HDF5Parser::parse() {
    DataStorage storage;
    std::set<std::string> header_names = {"header"};
    try {
        H5::Group root_group = file->openGroup("/");
        parse_group(root_group, header_names, storage);
    } catch (H5::Exception& e) {
        std::cerr << "Error parsing HDF5 file: " << e.getCDetailMsg() << std::endl;
    }
    return storage;
}

void HDF5Parser::parse_group(H5::Group& group, const std::set<std::string>& header_names,
                           DataStorage& storage) {
    if (processed_groups.count(group.getObjAddr())) {
        return;
    }
    processed_groups.insert(group.getObjAddr());

    for (hsize_t i = 0; i < group.getNumObjs(); ++i) {
        std::string obj_name = group.getObjnameByIdx(i);
        if (group.getObjTypeByIdx(i) == H5G_GROUP) {
            if (header_names.find(obj_name) == header_names.end()) {
                H5::Group child_group = group.openGroup(obj_name);
                parse_group(child_group, header_names, storage);
            }
        } else if (group.getObjTypeByIdx(i) == H5G_DATASET) {
            H5::DataSet dataset = group.openDataSet(obj_name);
            std::string group_name = group.getObjName();
            storage.set_value(dataset, obj_name, group_name);
        }
    }
}

void HDF5Parser::cleanup() {
    processed_groups.clear();
    if (file) {
        file->close();
    }
}

void HDF5Parser::DataStorage::set_value(const H5::DataSet& dataset,
                                       const std::string& signal_name,
                                       const std::string& group_name) {
    H5::DataSpace dataspace = dataset.getSpace();
    if (dataspace.getSimpleExtentNdims() != 1) {
        return; // Only handle 1D datasets for simplicity
    }

    hsize_t dims;
    dataspace.getSimpleExtentDims(&dims, nullptr);

    std::vector<double> data_vec(dims);
    dataset.read(data_vec.data(), H5::PredType::NATIVE_DOUBLE);

    data[group_name + "/" + signal_name] = data_vec;
}