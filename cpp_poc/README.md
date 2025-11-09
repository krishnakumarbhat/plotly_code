# HDF5 Parser and Plotly Visualization

This project demonstrates how to read HDF5 files using C++ and create interactive scatter plots using Python's Plotly library through pybind11 bindings.

## Prerequisites

- CMake (>= 3.12)
- C++ compiler with C++17 support
- Python 3.x
- HDF5 library and development files
- pybind11

## Dependencies

### C++ Dependencies
- HDF5
- pybind11

### Python Dependencies
Install the required Python packages:
```bash
pip install -r requirements.txt
```

## Building the Project

1. Clone the pybind11 repository:
```bash
git clone https://github.com/pybind/pybind11.git
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Configure and build the project:
```bash
cmake ..
make
```

## Running the Project

### Using the C++ executable
```bash
./cpp_poc <path_to_hdf5_file>
```

### Using the Python script
```bash
python ../plot_script.py <path_to_hdf5_file>
```

## Project Structure

- `src/`
  - `main.cpp` - Main C++ entry point
  - `hdf5_parser.hpp/cpp` - HDF5 file parser implementation
  - `python_plotter.hpp/cpp` - Python bindings and plotting functionality
- `plot_script.py` - Python script for plotting
- `requirements.txt` - Python dependencies
- `CMakeLists.txt` - CMake build configuration

## Notes

- The parser expects HDF5 files with a specific structure containing datasets named 'x_data' and 'y_data' under a 'Stream_0' group
- The scatter plot is interactive and will be displayed in your default web browser using Plotly