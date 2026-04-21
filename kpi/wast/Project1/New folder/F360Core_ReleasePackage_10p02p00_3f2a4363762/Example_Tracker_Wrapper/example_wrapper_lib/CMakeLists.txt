cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(example_wrapper_lib)

#import trackerlib, ocg and rspp lib(the libraries below are built from our jenkinsjob and copy to the path)

add_library(ot_tracker STATIC IMPORTED)
set_target_properties(ot_tracker PROPERTIES
             IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/ot_tracker/Debug/f360_variant_A.lib
             IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/ot_tracker/Release/f360_variant_A.lib)
add_library(rspp STATIC IMPORTED)
set_target_properties(rspp PROPERTIES
             IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/rspp/Debug/rspp_variant_A.lib
             IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/rspp/Release/rspp_variant_A.lib)
add_library(occupancy_grid STATIC IMPORTED)
set_target_properties(occupancy_grid PROPERTIES
             IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Debug/occupancy_grid.lib
             IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Release/occupancy_grid.lib)
add_library(ocg_common STATIC IMPORTED)
set_target_properties(ocg_common PROPERTIES
             IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Debug/ocg_common.lib
             IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Release/ocg_common.lib)
add_library(ocg_underdrivability STATIC IMPORTED)
set_target_properties(ocg_underdrivability PROPERTIES
             IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Debug/ocg_underdrivability.lib
             IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/module_libs/occupancy_grid/Release/ocg_underdrivability.lib)

target_sources(example_wrapper_lib
              PRIVATE
              source/Example_Wrapper_F360Tracker.cpp
              PUBLIC
              include/Example_Wrapper_F360Tracker.h)

#link rspp, ocg and tracker to the wrapperlib             
target_link_libraries(
    example_wrapper_lib
        PUBLIC ot_tracker rspp occupancy_grid ocg_common ocg_underdrivability
)

target_include_directories(
    example_wrapper_lib
    PUBLIC
        include
        module_libs/ot_tracker/iface_variant_A/core
        module_libs/ot_tracker/iface_variant_A/Logging
        module_libs/ot_tracker/iface_variant_A/Types
        module_libs/ot_tracker/iface_variant_A/Types/ocg
        module_libs/rspp/iface_variant_A
        module_libs/occupancy_grid/iface_variant_A
)
