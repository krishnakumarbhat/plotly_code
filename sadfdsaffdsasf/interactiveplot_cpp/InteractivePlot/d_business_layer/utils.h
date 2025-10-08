#pragma once

#include <string>
#include <chrono>
#include <functional>

namespace interactive_plot {
namespace business {

class Utils {
public:
    // Time measurement utilities
    static double time_taken(std::function<void()> func);
    
    // String utilities
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);
    
    // File utilities
    static bool file_exists(const std::string& path);
    static std::string get_file_extension(const std::string& path);
    static std::string get_filename_without_extension(const std::string& path);
    
    // Math utilities
    static double round_to_decimals(double value, int decimals);
    static bool is_approximately_equal(double a, double b, double epsilon = 1e-9);
    
    // Memory utilities
    static void force_garbage_collection();
    static size_t get_memory_usage();
};

// RAII timer class for automatic time measurement
class Timer {
public:
    Timer(const std::string& operation_name);
    ~Timer();
    
    double elapsed_seconds() const;
    void reset();

private:
    std::string operation_name_;
    std::chrono::steady_clock::time_point start_time_;
};

} // namespace business
} // namespace interactive_plot
