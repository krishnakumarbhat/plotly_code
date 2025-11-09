# InteractivePlot Library CMakeLists.txt
add_library(interactive_plot_lib STATIC
    # Config Layer
    a_config_layer/allsensor_json_parser.cpp
    a_config_layer/json_parser_factory.cpp
    a_config_layer/config_manager.cpp
    
    # Persistence Layer
    b_persistence_layer/allsensor_hdf_parser.cpp
    b_persistence_layer/persensor_hdf_parser.cpp
    b_persistence_layer/hdf_parser.cpp
    
    # Data Storage Layer
    c_data_storage/config_storage.cpp
    c_data_storage/data_model_storage.cpp
    
    # Business Layer
    d_business_layer/data_cal.cpp
    d_business_layer/data_prep.cpp
    d_business_layer/utils.cpp
    
    # Presentation Layer
    e_presentation_layer/html_generator.cpp
    e_presentation_layer/plot_generator.cpp
)

target_include_directories(interactive_plot_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(interactive_plot_lib
    ${HDF5_LIBRARIES}
)
