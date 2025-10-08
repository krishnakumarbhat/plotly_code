#include "kpi_factory.h"
#include "alignment_matching_kpi.h"
#include "detection_matching_kpi.h"
#include "tracker_matching_kpi.h"
#include "../b_data_storage/kpi_config_storage.h"
#include <iostream>
#include <filesystem>
#include <random>
#include <sstream>
#include <thread>
#include <future>

namespace kpi {
namespace business {

KpiDataModel::KpiDataModel(std::shared_ptr<data_storage::KPIDataModelStorage> input_data,
                           std::shared_ptr<data_storage::KPIDataModelStorage> output_data,
                           const std::string& sensor)
    : input_data_(input_data), output_data_(output_data), sensor_(sensor) {
    
    temp_dir_ = create_temp_directory();
    process_kpis();
}

KpiDataModel::~KpiDataModel() {
    cleanup_temp_directory();
}

void KpiDataModel::process_kpis() {
    std::cout << "Processing KPIs in parallel using multithreading" << std::endl;
    
    // Launch KPI processing tasks in parallel
    std::vector<std::future<void>> futures;
    
    futures.push_back(std::async(std::launch::async, [this]() {
        process_alignment_kpis();
    }));
    
    futures.push_back(std::async(std::launch::async, [this]() {
        process_detection_kpis();
    }));
    
    futures.push_back(std::async(std::launch::async, [this]() {
        process_tracker_kpis();
    }));
    
    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    std::cout << "All KPI processing completed" << std::endl;
}

void KpiDataModel::process_alignment_kpis() {
    auto input_data = input_data_->get_data();
    auto output_data = output_data_->get_data();
    
    for (const auto& config_pair : data_storage::KPI_ALIGNMENT_CONFIG) {
        const std::string& stream_name = config_pair.first;
        
        // Check if stream exists in both input and output data
        if (input_data.find(stream_name) != input_data.end() && 
            output_data.find(stream_name) != output_data.end()) {
            
            std::cout << "Processing alignment KPI for stream: " << stream_name << std::endl;
            
            try {
                std::string plot_path = process_alignment_kpi(*input_data_, *output_data_, 
                                                           stream_name, temp_dir_);
                if (!plot_path.empty()) {
                    plots_.push_back(plot_path);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing alignment KPI for " << stream_name 
                         << ": " << e.what() << std::endl;
            }
        }
    }
}

void KpiDataModel::process_detection_kpis() {
    auto input_data = input_data_->get_data();
    auto output_data = output_data_->get_data();
    
    for (const auto& config_pair : data_storage::KPI_DETECTION_CONFIG) {
        const std::string& stream_name = config_pair.first;
        
        // Check if stream exists in both input and output data
        if (input_data.find(stream_name) != input_data.end() && 
            output_data.find(stream_name) != output_data.end()) {
            
            std::cout << "Processing detection KPI for stream: " << stream_name << std::endl;
            
            try {
                std::string plot_path = process_detection_kpi(*input_data_, *output_data_, 
                                                            stream_name, temp_dir_);
                if (!plot_path.empty()) {
                    plots_.push_back(plot_path);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing detection KPI for " << stream_name 
                         << ": " << e.what() << std::endl;
            }
        }
    }
}

void KpiDataModel::process_tracker_kpis() {
    auto input_data = input_data_->get_data();
    auto output_data = output_data_->get_data();
    
    for (const auto& config_pair : data_storage::KPI_TRACKER_CONFIG) {
        const std::string& stream_name = config_pair.first;
        
        // Check if stream exists in both input and output data
        if (input_data.find(stream_name) != input_data.end() && 
            output_data.find(stream_name) != output_data.end()) {
            
            std::cout << "Processing tracker KPI for stream: " << stream_name << std::endl;
            
            try {
                std::string plot_path = process_tracker_kpi(*input_data_, *output_data_, 
                                                          stream_name, temp_dir_);
                if (!plot_path.empty()) {
                    plots_.push_back(plot_path);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing tracker KPI for " << stream_name 
                         << ": " << e.what() << std::endl;
            }
        }
    }
}

std::string KpiDataModel::create_temp_directory() {
    // Generate random directory name
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::stringstream ss;
    ss << "kpi_plots_temp_" << dis(gen);
    
    std::filesystem::path temp_path = std::filesystem::temp_directory_path() / ss.str();
    std::filesystem::create_directories(temp_path);
    
    return temp_path.string();
}

void KpiDataModel::cleanup_temp_directory() {
    if (!temp_dir_.empty() && std::filesystem::exists(temp_dir_)) {
        try {
            std::filesystem::remove_all(temp_dir_);
        } catch (const std::exception& e) {
            std::cerr << "Error cleaning up temp directory: " << e.what() << std::endl;
        }
    }
}

std::vector<std::string> KpiDataModel::get_plots() const {
    return plots_;
}

std::string KpiDataModel::get_temp_dir() const {
    return temp_dir_;
}

} // namespace business
} // namespace kpi
