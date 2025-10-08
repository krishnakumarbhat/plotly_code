# KPI Library CMakeLists.txt
add_library(kpi_lib STATIC
    # Data Storage Layer
    b_data_storage/kpi_data_model_storage.cpp
    b_data_storage/kpi_config_storage.cpp
    
    # Persistence Layer
    a_persistence_layer/kpi_hdf_parser.cpp
    a_persistence_layer/hdf_wrapper.cpp
    
    # Business Layer
    c_business_layer/kpi_factory.cpp
    c_business_layer/alignment_matching_kpi.cpp
    c_business_layer/detection_matching_kpi.cpp
    c_business_layer/tracker_matching_kpi.cpp
    
    # Presentation Layer
    d_presentation_layer/kpi_html_gen.cpp
)

target_include_directories(kpi_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(kpi_lib
    ${HDF5_LIBRARIES}
)
