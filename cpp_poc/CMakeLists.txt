cmake_minimum_required(VERSION 3.12)
project(cpp_poc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find HDF5
find_package(HDF5 REQUIRED COMPONENTS C HL)
include_directories(${HDF5_INCLUDE_DIRS})

# Find Python and pybind11
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_subdirectory(pybind11)

# Add executable
add_executable(cpp_poc src/main.cpp src/hdf5_parser.cpp)

# Link libraries
target_link_libraries(cpp_poc PRIVATE ${HDF5_LIBRARIES} ${PYTHON_LIBRARIES})

# Create the pybind11 module
add_library(python_plotter MODULE src/python_plotter.cpp)
target_link_libraries(python_plotter PRIVATE pybind11::module ${HDF5_LIBRARIES})

# Add a custom target to run the plot script
add_custom_target(
    run_plot
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/plot_script.py
    DEPENDS python_plotter
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)