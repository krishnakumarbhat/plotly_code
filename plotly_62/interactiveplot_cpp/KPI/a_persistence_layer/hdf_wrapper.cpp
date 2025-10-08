#include "hdf_wrapper.h"
#include "kpi_hdf_parser.h"
#include "../b_data_storage/kpi_config_storage.h"
#include "../c_business_layer/kpi_factory.h"
#include <iostream>
#include <chrono>
#include <filesystem>
#include <any>

namespace kpi {
namespace persistence {

KPIHDFWrapper::KPIHDFWrapper(const KPIProcessingConfig& config) 
    : config_(config), start_time_parsing_(std::chrono::steady_clock::now()) {
    
    header_variants_ = {
        "Stream_Hdr",
        "stream_hdr", 
        "StreamHdr",
        "STREAM_HDR",
        "streamheader",
        "stream_header",
        "HEADER_STREAM"
    };
    
    stream_input_model_ = std::make_shared<data_storage::KPIDataModelStorage>();
    stream_output_model_ = std::make_shared<data_storage::KPIDataModelStorage>();
}

KPIHDFWrapper::~KPIHDFWrapper() = default;

std::unordered_map<std::string, std::any> KPIHDFWrapper::parse() {
    std::unordered_map<std::string, std::any> results;
    results["sensor_id"] = config_.sensor_id;
    results["base_name"] = config_.base_name;
    results["processing_time"] = 0.0;
    results["available_streams"] = std::vector<std::string>();
    results["input_data"] = std::unordered_map<std::string, std::any>();
    results["output_data"] = std::unordered_map<std::string, std::any>();
    results["streams_processed"] = std::unordered_map<std::string, std::any>();
    results["html_report_path"] = std::string();

    // Build output directory for HTML report path
    std::filesystem::path kpi_dir = std::filesystem::path(config_.output_dir) / 
                                   config_.base_name / config_.sensor_id / config_.kpi_subdir;
    std::filesystem::create_directories(kpi_dir);
    
    std::string html_report_path = (kpi_dir / (config_.base_name + "_" + config_.sensor_id + "_kpi_report.html")).string();
    results["html_report_path"] = html_report_path;

    // Collect streams from KPI config
    std::vector<std::string> streams = collect_streams();
    results["available_streams"] = streams;

    std::string sensor = config_.sensor_id;
    std::string input_path = config_.input_file_path;
    std::string output_path = config_.output_file_path;

    // Open HDF5 files
    hid_t hdf_in = -1;
    hid_t hdf_out = -1;

    if (file_exists(input_path)) {
        hdf_in = H5Fopen(input_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    } else {
        std::cerr << "Input HDF5 not found: " << input_path << std::endl;
    }

    if (file_exists(output_path)) {
        hdf_out = H5Fopen(output_path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    } else {
        std::cout << "Output HDF5 not found: " << output_path << std::endl;
    }

    // Get scan index from first available stream
    std::vector<int> scan_index;
    if (hdf_in >= 0 && !streams.empty()) {
        scan_index = get_scan_index(hdf_in, streams, sensor);
    }

    // Initialize models once with scan index
    if (!scan_index.empty()) {
        stream_input_model_->initialize(scan_index, sensor);
        stream_output_model_->initialize(scan_index, sensor);
    } else {
        std::cout << "No scan index found, initializing with empty data" << std::endl;
        stream_input_model_->initialize({}, sensor);
        stream_output_model_->initialize({}, sensor);
    }

    // Process each KPI stream
    for (size_t stream_idx = 0; stream_idx < streams.size(); ++stream_idx) {
        const std::string& stream = streams[stream_idx];
        std::cout << "Processing stream [" << stream_idx << "] " << stream << std::endl;

        std::unordered_map<std::string, bool> stream_results;
        stream_results["input_available"] = false;
        stream_results["output_available"] = false;
        stream_results["both_available"] = false;
        results["streams_processed"][stream] = stream_results;

        // Set stream-specific parent
        stream_input_model_->init_parent(stream);
        stream_output_model_->init_parent(stream);

        // Parse input and output files for this specific stream
        std::string group_path = sensor + "/" + stream;
        
        if (hdf_in >= 0 && hdf_out >= 0) {
            // Check if group exists in both files
            htri_t input_exists = H5Lexists(hdf_in, group_path.c_str(), H5P_DEFAULT);
            htri_t output_exists = H5Lexists(hdf_out, group_path.c_str(), H5P_DEFAULT);
            
            if (input_exists > 0 && output_exists > 0) {
                // Parse input stream
                hid_t data_group_in = H5Gopen2(hdf_in, group_path.c_str(), H5P_DEFAULT);
                if (data_group_in >= 0) {
                    // Check for header
                    bool header_found = false;
                    for (const std::string& header_variant : header_variants_) {
                        htri_t header_exists = H5Lexists(data_group_in, header_variant.c_str(), H5P_DEFAULT);
                        if (header_exists > 0) {
                            header_found = true;
                            break;
                        }
                    }
                    
                    if (header_found) {
                        try {
                            auto start_time = std::chrono::steady_clock::now();
                            stream_input_model_ = KPIHDFParser::parse(
                                data_group_in, stream_input_model_, scan_index, header_variants_
                            );
                            auto end_time = std::chrono::steady_clock::now();
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            std::cout << "Parsed input stream " << stream << " in " << duration.count() << "ms" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing input stream " << stream << ": " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "No header found for input stream " << stream << " at " << group_path << std::endl;
                    }
                    H5Gclose(data_group_in);
                }

                // Parse output stream
                hid_t data_group_out = H5Gopen2(hdf_out, group_path.c_str(), H5P_DEFAULT);
                if (data_group_out >= 0) {
                    // Check for header
                    bool header_found = false;
                    for (const std::string& header_variant : header_variants_) {
                        htri_t header_exists = H5Lexists(data_group_out, header_variant.c_str(), H5P_DEFAULT);
                        if (header_exists > 0) {
                            header_found = true;
                            break;
                        }
                    }
                    
                    if (header_found) {
                        try {
                            auto start_time = std::chrono::steady_clock::now();
                            stream_output_model_ = KPIHDFParser::parse(
                                data_group_out, stream_output_model_, scan_index, header_variants_
                            );
                            auto end_time = std::chrono::steady_clock::now();
                            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                            std::cout << "Parsed output stream " << stream << " in " << duration.count() << "ms" << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing output stream " << stream << ": " << e.what() << std::endl;
                        }
                    } else {
                        std::cout << "No header found for output stream " << stream << " at " << group_path << std::endl;
                    }
                    H5Gclose(data_group_out);
                }
            } else {
                std::cout << "Stream group not found in both HDF files: " << group_path << std::endl;
            }
        }
    }

    // Create KPI data model
    business::KpiDataModel kpi_model(stream_input_model_, stream_output_model_, sensor);

    // Close HDF5 files
    if (hdf_in >= 0) {
        H5Fclose(hdf_in);
    }
    if (hdf_out >= 0) {
        H5Fclose(hdf_out);
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_parsing_);
    results["processing_time"] = duration.count() / 1000.0;

    std::cout << "KPI parsing complete for sensor=" << config_.sensor_id 
              << " base=" << config_.base_name << " in " << duration.count() / 1000.0 << "s" << std::endl;

    return results;
}

std::vector<std::string> KPIHDFWrapper::collect_streams() {
    std::vector<std::string> streams;
    
    for (const auto& pair : data_storage::KPI_ALIGNMENT_CONFIG) {
        streams.push_back(pair.first);
    }
    for (const auto& pair : data_storage::KPI_DETECTION_CONFIG) {
        streams.push_back(pair.first);
    }
    for (const auto& pair : data_storage::KPI_TRACKER_CONFIG) {
        streams.push_back(pair.first);
    }
    
    return streams;
}

std::vector<int> KPIHDFWrapper::get_scan_index(hid_t hdf_file, const std::vector<std::string>& streams, const std::string& sensor) {
    std::vector<int> scan_index;
    
    for (const std::string& stream : streams) {
        std::string group_path = sensor + "/" + stream;
        htri_t group_exists = H5Lexists(hdf_file, group_path.c_str(), H5P_DEFAULT);
        
        if (group_exists > 0) {
            hid_t data_group = H5Gopen2(hdf_file, group_path.c_str(), H5P_DEFAULT);
            if (data_group >= 0) {
                for (const std::string& header_variant : header_variants_) {
                    htri_t header_exists = H5Lexists(data_group, header_variant.c_str(), H5P_DEFAULT);
                    if (header_exists > 0) {
                        std::string scan_index_path = header_variant + "/scan_index";
                        htri_t scan_index_exists = H5Lexists(data_group, scan_index_path.c_str(), H5P_DEFAULT);
                        if (scan_index_exists > 0) {
                            hid_t scan_index_dataset = H5Dopen2(data_group, scan_index_path.c_str(), H5P_DEFAULT);
                            if (scan_index_dataset >= 0) {
                                // Read scan index data
                                hid_t dataspace = H5Dget_space(scan_index_dataset);
                                hsize_t dims[1];
                                H5Sget_simple_extent_dims(dataspace, dims, nullptr);
                                
                                scan_index.resize(dims[0]);
                                H5Dread(scan_index_dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, scan_index.data());
                                
                                H5Sclose(dataspace);
                                H5Dclose(scan_index_dataset);
                                H5Gclose(data_group);
                                return scan_index;
                            }
                        }
                    }
                }
                H5Gclose(data_group);
            }
        }
    }
    
    return scan_index;
}

bool KPIHDFWrapper::file_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string parse_for_kpi(
    const std::string& sensor_id,
    const std::string& input_file_path,
    const std::string& output_dir,
    const std::string& base_name,
    const std::string& kpi_subdir,
    const std::string& output_file_path) {
    
    if (sensor_id.empty() || input_file_path.empty() || output_file_path.empty() || 
        output_dir.empty() || base_name.empty()) {
        throw std::invalid_argument("Missing required fields: sensor_id, input_file_path, output_file_path, output_dir, base_name");
    }

    KPIProcessingConfig config;
    config.sensor_id = sensor_id;
    config.input_file_path = input_file_path;
    config.output_file_path = output_file_path;
    config.output_dir = output_dir;
    config.base_name = base_name;
    config.kpi_subdir = kpi_subdir;

    KPIHDFWrapper wrapper(config);
    auto results = wrapper.parse();
    
    return std::any_cast<std::string>(results["html_report_path"]);
}

} // namespace persistence
} // namespace kpi
