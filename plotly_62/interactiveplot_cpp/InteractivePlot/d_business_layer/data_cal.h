#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../c_data_storage/data_model_storage.h"

namespace interactive_plot {
namespace business {

class DataCalculator {
public:
    DataCalculator();
    ~DataCalculator();

    // Perform calculations on data
    std::vector<double> calculate_moving_average(const std::vector<double>& data, size_t window_size);
    std::vector<double> calculate_derivative(const std::vector<double>& data);
    std::vector<double> calculate_integral(const std::vector<double>& data, double dt = 1.0);
    
    // Statistical calculations
    double calculate_correlation(const std::vector<double>& x, const std::vector<double>& y);
    double calculate_rms(const std::vector<double>& data);
    std::vector<double> calculate_fft_magnitude(const std::vector<double>& data);
    
    // Signal processing
    std::vector<double> apply_butterworth_filter(const std::vector<double>& data, 
                                               double cutoff_freq, 
                                               double sampling_freq,
                                               bool is_lowpass = true);
    
    // Data interpolation
    std::vector<double> interpolate_linear(const std::vector<double>& x_old,
                                         const std::vector<double>& y_old,
                                         const std::vector<double>& x_new);

private:
    // Helper methods
    double linear_interpolate(double x0, double y0, double x1, double y1, double x);
    std::vector<std::complex<double>> fft(const std::vector<double>& data);
};

} // namespace business
} // namespace interactive_plot
