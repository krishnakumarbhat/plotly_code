#include "utils.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <algorithm>

namespace interactive_plot {
namespace business {

double Utils::time_taken(std::function<void()> func) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000000.0; // Convert to seconds
}

std::string Utils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string Utils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::stringstream ss;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) ss << delimiter;
        ss << strings[i];
    }
    
    return ss.str();
}

bool Utils::file_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string Utils::get_file_extension(const std::string& path) {
    std::filesystem::path p(path);
    return p.extension().string();
}

std::string Utils::get_filename_without_extension(const std::string& path) {
    std::filesystem::path p(path);
    return p.stem().string();
}

double Utils::round_to_decimals(double value, int decimals) {
    double multiplier = std::pow(10.0, decimals);
    return std::round(value * multiplier) / multiplier;
}

bool Utils::is_approximately_equal(double a, double b, double epsilon) {
    return std::abs(a - b) < epsilon;
}

void Utils::force_garbage_collection() {
    // C++ doesn't have automatic garbage collection like Python
    // This is a placeholder for any cleanup operations
    std::cout << "Memory cleanup requested" << std::endl;
}

size_t Utils::get_memory_usage() {
    // Platform-specific memory usage calculation would go here
    // For now, return 0 as placeholder
    return 0;
}

// Timer implementation
Timer::Timer(const std::string& operation_name) 
    : operation_name_(operation_name), start_time_(std::chrono::steady_clock::now()) {
    std::cout << "Starting timer for: " << operation_name_ << std::endl;
}

Timer::~Timer() {
    double elapsed = elapsed_seconds();
    std::cout << "Timer [" << operation_name_ << "] completed in " << elapsed << "s" << std::endl;
}

double Timer::elapsed_seconds() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_);
    return duration.count() / 1000000.0;
}

void Timer::reset() {
    start_time_ = std::chrono::steady_clock::now();
}

} // namespace business
} // namespace interactive_plot
