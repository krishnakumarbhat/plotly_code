cmake_minimum_required(VERSION 3.16)
project(InteractivePlotCpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(HDF5 REQUIRED COMPONENTS CXX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${HDF5_INCLUDE_DIRS})

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Add subdirectories
add_subdirectory(KPI)
add_subdirectory(InteractivePlot)

# Main executable
add_executable(kpi_server
    main.cpp
)

target_link_libraries(kpi_server
    kpi_lib
    interactive_plot_lib
    ${HDF5_LIBRARIES}
    ${ZMQ_LIBRARIES}
)

# Install targets
install(TARGETS kpi_server DESTINATION bin)
