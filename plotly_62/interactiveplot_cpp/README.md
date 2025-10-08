# InteractivePlot C++ Implementation

This is a complete C++ implementation of the KPI and InteractivePlot system, mirroring the Python structure with equivalent functionality.

## Project Structure

```
interactiveplot_cpp/
├── CMakeLists.txt              # Main build configuration
├── main.cpp                    # Server entry point
├── README.md                   # This file
├── KPI/                        # KPI processing module
│   ├── CMakeLists.txt
│   ├── a_persistence_layer/    # HDF5 data parsing
│   │   ├── hdf_wrapper.h/cpp
│   │   └── kpi_hdf_parser.h/cpp
│   ├── b_data_storage/         # Data model and configuration
│   │   ├── kpi_data_model_storage.h/cpp
│   │   └── kpi_config_storage.h/cpp
│   ├── c_business_layer/       # KPI processing logic
│   │   ├── kpi_factory.h/cpp
│   │   ├── alignment_matching_kpi.h/cpp
│   │   ├── detection_matching_kpi.h/cpp
│   │   └── tracker_matching_kpi.h/cpp
│   └── d_presentation_layer/   # HTML report generation
│       └── kpi_html_gen.h/cpp
└── InteractivePlot/            # Interactive plotting module
    ├── CMakeLists.txt
    ├── a_config_layer/         # Configuration management
    │   ├── config_manager.h/cpp
    │   ├── json_parser_factory.h/cpp
    │   └── allsensor_json_parser.h/cpp
    ├── b_persistence_layer/    # HDF5 data access
    │   └── hdf_parser.h/cpp
    ├── c_data_storage/         # Data storage and configuration
    │   ├── data_model_storage.h/cpp
    │   └── config_storage.h/cpp
    ├── d_business_layer/       # Data processing and calculations
    │   ├── data_prep.h/cpp
    │   ├── data_cal.h/cpp
    │   └── utils.h/cpp
    └── e_presentation_layer/   # HTML generation
        └── html_generator.h/cpp
```

## Dependencies

- **HDF5**: For reading HDF5 data files
- **ZeroMQ**: For network communication
- **CMake**: Build system (version 3.16+)
- **C++17**: Modern C++ standard

## Building

1. Install dependencies:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libhdf5-dev libzmq3-dev cmake

   # CentOS/RHEL
   sudo yum install hdf5-devel zeromq-devel cmake
   ```

2. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j$(nproc)
   ```

3. Run the server:
   ```bash
   ./kpi_server
   ```

## Features

### KPI Module
- **Data Model Storage**: Hierarchical data storage with bidirectional signal mapping
- **HDF Parser**: Efficient HDF5 file parsing with stream filtering
- **Business Logic**: Parallel processing of alignment, detection, and tracker KPIs
- **HTML Generation**: Interactive Plotly-based reports

### InteractivePlot Module
- **Configuration Management**: JSON-based sensor configuration
- **Data Processing**: Statistical calculations, filtering, and signal processing
- **Visualization**: HTML-based interactive plots using Plotly.js
- **Utilities**: Time measurement, file operations, and mathematical functions

## Key Improvements over Python Version

1. **Performance**: Native C++ performance for large datasets
2. **Memory Management**: Explicit memory control and RAII patterns
3. **Type Safety**: Compile-time type checking
4. **Concurrency**: Native threading support for parallel KPI processing
5. **Resource Management**: Automatic cleanup of HDF5 resources

## Usage

The server listens on port 5555 for ZMQ requests. Send JSON requests with:

```json
{
  "sensor_id": "sensor1",
  "input_file": "/path/to/input.h5",
  "output_file": "/path/to/output.h5",
  "output_dir": "/path/to/reports",
  "base_name": "test_run"
}
```

Response format:
```json
{
  "status": "success",
  "html_report": "/path/to/generated/report.html"
}
```

## Architecture

The C++ implementation follows the same layered architecture as the Python version:

1. **Persistence Layer**: HDF5 file I/O and data parsing
2. **Data Storage Layer**: In-memory data models and configuration
3. **Business Layer**: Core processing logic and calculations
4. **Presentation Layer**: HTML report generation

## Thread Safety

- KPI processing uses parallel execution for different stream types
- HDF5 operations are synchronized per file handle
- Data models are designed for single-threaded access per instance

## Error Handling

- Comprehensive exception handling with detailed error messages
- Resource cleanup using RAII patterns
- Graceful degradation for missing data or configuration

## Testing

Build and run tests:
```bash
make test
```

## Contributing

Follow the existing code structure and naming conventions. All new features should include appropriate error handling and documentation.
