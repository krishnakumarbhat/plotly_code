#include "hdf5_parser.hpp"
#include "python_plotter.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <hdf5_file>" << std::endl;
        return 1;
    }

    try {
        HDF5Parser parser(argv[1]);
        auto data_storage = parser.parse();

        // Example of how to access data and plot
        // This assumes you know the names of the datasets you want to plot
        if (data_storage.data.count("/Stream_0/x_data") && data_storage.data.count("/Stream_0/y_data")) {
            const auto& x_data = data_storage.data.at("/Stream_0/x_data");
            const auto& y_data = data_storage.data.at("/Stream_0/y_data");
            create_scatter_plot(x_data, y_data, "My Scatter Plot");
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}