#include "data_cal.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <complex>
#include <iostream>

namespace interactive_plot {
namespace business {

DataCalculator::DataCalculator() = default;
DataCalculator::~DataCalculator() = default;

std::vector<double> DataCalculator::calculate_moving_average(const std::vector<double>& data, size_t window_size) {
    if (data.empty() || window_size == 0) {
        return data;
    }
    
    std::vector<double> result;
    result.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        size_t start = (i >= window_size - 1) ? i - window_size + 1 : 0;
        size_t end = i + 1;
        
        double sum = 0.0;
        for (size_t j = start; j < end; ++j) {
            sum += data[j];
        }
        
        result.push_back(sum / (end - start));
    }
    
    return result;
}

std::vector<double> DataCalculator::calculate_derivative(const std::vector<double>& data) {
    if (data.size() < 2) {
        return data;
    }
    
    std::vector<double> derivative;
    derivative.reserve(data.size() - 1);
    
    for (size_t i = 1; i < data.size(); ++i) {
        derivative.push_back(data[i] - data[i-1]);
    }
    
    return derivative;
}

std::vector<double> DataCalculator::calculate_integral(const std::vector<double>& data, double dt) {
    if (data.empty()) {
        return data;
    }
    
    std::vector<double> integral;
    integral.reserve(data.size());
    
    double sum = 0.0;
    for (double value : data) {
        sum += value * dt;
        integral.push_back(sum);
    }
    
    return integral;
}

double DataCalculator::calculate_correlation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.empty()) {
        return 0.0;
    }
    
    double mean_x = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double mean_y = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    
    double numerator = 0.0;
    double sum_sq_x = 0.0;
    double sum_sq_y = 0.0;
    
    for (size_t i = 0; i < x.size(); ++i) {
        double dx = x[i] - mean_x;
        double dy = y[i] - mean_y;
        
        numerator += dx * dy;
        sum_sq_x += dx * dx;
        sum_sq_y += dy * dy;
    }
    
    double denominator = std::sqrt(sum_sq_x * sum_sq_y);
    return (denominator > 0) ? numerator / denominator : 0.0;
}

double DataCalculator::calculate_rms(const std::vector<double>& data) {
    if (data.empty()) {
        return 0.0;
    }
    
    double sum_squares = 0.0;
    for (double value : data) {
        sum_squares += value * value;
    }
    
    return std::sqrt(sum_squares / data.size());
}

std::vector<double> DataCalculator::calculate_fft_magnitude(const std::vector<double>& data) {
    auto fft_result = fft(data);
    std::vector<double> magnitude;
    magnitude.reserve(fft_result.size());
    
    for (const auto& complex_val : fft_result) {
        magnitude.push_back(std::abs(complex_val));
    }
    
    return magnitude;
}

std::vector<double> DataCalculator::apply_butterworth_filter(const std::vector<double>& data, 
                                                           double cutoff_freq, 
                                                           double sampling_freq,
                                                           bool is_lowpass) {
    // Simplified Butterworth filter implementation
    // In production, use a proper DSP library
    
    double rc = 1.0 / (2.0 * M_PI * cutoff_freq);
    double dt = 1.0 / sampling_freq;
    double alpha = dt / (rc + dt);
    
    std::vector<double> filtered = data;
    
    if (is_lowpass) {
        // Low-pass filter
        for (size_t i = 1; i < filtered.size(); ++i) {
            filtered[i] = alpha * data[i] + (1.0 - alpha) * filtered[i-1];
        }
    } else {
        // High-pass filter (simplified)
        for (size_t i = 1; i < filtered.size(); ++i) {
            filtered[i] = alpha * (filtered[i-1] + data[i] - data[i-1]);
        }
    }
    
    return filtered;
}

std::vector<double> DataCalculator::interpolate_linear(const std::vector<double>& x_old,
                                                     const std::vector<double>& y_old,
                                                     const std::vector<double>& x_new) {
    if (x_old.size() != y_old.size() || x_old.empty()) {
        return {};
    }
    
    std::vector<double> y_new;
    y_new.reserve(x_new.size());
    
    for (double x : x_new) {
        // Find the two points to interpolate between
        auto it = std::lower_bound(x_old.begin(), x_old.end(), x);
        
        if (it == x_old.begin()) {
            y_new.push_back(y_old[0]);
        } else if (it == x_old.end()) {
            y_new.push_back(y_old.back());
        } else {
            size_t i1 = std::distance(x_old.begin(), it);
            size_t i0 = i1 - 1;
            
            double y_interp = linear_interpolate(x_old[i0], y_old[i0], x_old[i1], y_old[i1], x);
            y_new.push_back(y_interp);
        }
    }
    
    return y_new;
}

double DataCalculator::linear_interpolate(double x0, double y0, double x1, double y1, double x) {
    if (x1 == x0) {
        return y0;
    }
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

std::vector<std::complex<double>> DataCalculator::fft(const std::vector<double>& data) {
    // Simplified FFT implementation - in production use FFTW or similar
    size_t n = data.size();
    std::vector<std::complex<double>> result;
    result.reserve(n);
    
    for (size_t k = 0; k < n; ++k) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * k * j / n;
            sum += data[j] * std::complex<double>(std::cos(angle), std::sin(angle));
        }
        result.push_back(sum);
    }
    
    return result;
}

} // namespace business
} // namespace interactive_plot
