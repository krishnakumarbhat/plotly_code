# DataPrep Parameter Analysis and Process Flow

## Overview
The `DataPrep` class is responsible for transforming raw sensor data into interactive HTML visualizations. This document provides a detailed analysis of how each input parameter flows through the data preparation and HTML generation processes.

## Input Parameters Analysis

### 1. `input_data` (DataModelStorage)
**Type:** DataModelStorage object  
**Purpose:** Contains raw input signal data from HDF5 files

**Flow through the process:**
- **Initialization:** Stored as `self.input_data` in DataPrep instance
- **Data Retrieval:** Used in `_get_data_cached()` method for LRU-cached data access
- **Signal Processing:** Passed to `DataModelStorage.get_data()` along with `output_data`
- **Plot Generation:** Provides input signal values for plot creation
- **Memory Management:** Monitored for memory usage during processing

**Key transformations:**
- Raw HDF5 data → Structured signal data → Plot-ready data arrays
- Data is cached using `@lru_cache` decorator for performance optimization

### 2. `output_data` (DataModelStorage)
**Type:** DataModelStorage object  
**Purpose:** Contains processed output signal data from HDF5 files

**Flow through the process:**
- **Initialization:** Stored as `self.output_data` in DataPrep instance
- **Data Retrieval:** Used in `_get_data_cached()` method alongside `input_data`
- **Comparison Analysis:** Provides reference data for mismatch calculations
- **Plot Generation:** Supplies output signal values for overlay plots

**Key transformations:**
- Processed HDF5 data → Comparison data → Plot visualization data
- Used for generating mismatch plots and overlay visualizations

### 3. `html_name` (String)
**Type:** String  
**Purpose:** Base name for generated HTML files

**Flow through the process:**
- **Initialization:** Stored as `self.html_name` and converted to stem (removes extension)
- **File Naming:** Used to create category-specific HTML filenames
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Output Files:** Forms basis for filenames like `{html_name}_histogram.html`

**Key transformations:**
- Raw filename → Stem (without extension) → Category-specific filenames

### 4. `sensor` (String)
**Type:** String  
**Purpose:** Identifies the sensor position (e.g., "FL", "FR", "RL", "RR")

**Flow through the process:**
- **Initialization:** Stored as `self.sensor` in DataPrep instance
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Folder Structure:** Used in `_create_folder_structure()` to create sensor-specific directories
- **Master Index:** Contributes to nested dropdown structure organization

**Key transformations:**
- Sensor identifier → Directory structure → Dropdown navigation element

### 5. `stream_name` (String)
**Type:** String  
**Purpose:** Identifies the data stream type (e.g., "DETECTION_STREAM", "VSE_STREAM")

**Flow through the process:**
- **Initialization:** Stored as `self.stream_name` in DataPrep instance
- **Configuration Lookup:** Used to retrieve signal configurations from `Gen7V1_V2`
- **Data Calculations:** Set as context in `DataCalculations` instance
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Folder Structure:** Used to create stream-specific directories

**Key transformations:**
- Stream identifier → Signal configuration → Directory structure → Navigation element

### 6. `input_file_name` (String)
**Type:** String  
**Purpose:** Name of the input HDF5 file

**Flow through the process:**
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Metadata Display:** Used in HTML template to show input filename
- **Template Replacement:** Replaces `{{INPUT_FILENAME}}` placeholder in HTML

**Key transformations:**
- Filename → HTML metadata → User-visible information

### 7. `output_file_name` (String)
**Type:** String  
**Purpose:** Name of the output HDF5 file

**Flow through the process:**
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Metadata Display:** Used in HTML template to show output filename
- **Template Replacement:** Replaces `{{OUTPUT_FILENAME}}` placeholder in HTML

**Key transformations:**
- Filename → HTML metadata → User-visible information

### 8. `output_dir` (String, optional)
**Type:** String, defaults to "html"  
**Purpose:** Directory where HTML files will be saved

**Flow through the process:**
- **Initialization:** Stored as `self.output_dir` with default value "html"
- **HTML Generation:** Passed to `HtmlGenerator` constructor
- **Folder Creation:** Used as base directory for all generated files
- **Master Index:** Used as root directory for master index creation

**Key transformations:**
- Directory path → Base path for all output files → File system structure

### 9. `generate_html` (Boolean, default: True)
**Type:** Boolean  
**Purpose:** Controls whether HTML files should be generated

**Flow through the process:**
- **Conditional Processing:** Determines if `HtmlGenerator` is instantiated
- **Memory Management:** If False, skips HTML generation and cleans up temp files
- **Resource Cleanup:** Controls whether temporary directory is removed

**Key transformations:**
- Boolean flag → Conditional execution path → Resource management decision

## Detailed Process Flow

### Phase 1: Initialization and Setup
1. **Parameter Storage:** All parameters are stored as instance variables
2. **Memory Tracking:** `_track_memory_usage()` monitors initial memory consumption
3. **Logging Setup:** Configures logging for the DataPrep instance

### Phase 2: Plot Generation
1. **Data Calculations Import:** Dynamically imports `DataCalculations` class
2. **Multiprocessing Setup:** Creates temporary directory and shared data structures
3. **Signal Configuration:** Retrieves signal configurations from `Gen7V1_V2`
4. **Parallel Processing:** Processes signals using multiprocessing or sequential execution
5. **JSON Generation:** Saves plot data as JSON files in temporary directory

### Phase 3: HTML Generation (if generate_html=True)
1. **HtmlGenerator Instantiation:** Creates HtmlGenerator with all relevant parameters
2. **Folder Structure Creation:** Organizes directories by sensor and stream
3. **Plot Categorization:** Groups plots by type (histogram, mismatch, kpi, etc.)
4. **HTML File Generation:** Creates category-specific HTML files
5. **Main Index Creation:** Generates navigation index for the sensor-stream combination
6. **Master Index Update:** Updates the master index with nested dropdown structure

### Phase 4: Cleanup
1. **Temporary File Removal:** Deletes temporary JSON files and directory
2. **Memory Cleanup:** Removes plot data from memory
3. **Resource Release:** Frees up system resources

## Data Transformations Summary

| Parameter | Input Type | Output Type | Key Transformation |
|-----------|------------|-------------|-------------------|
| `input_data` | DataModelStorage | Plot-ready arrays | Raw data → Structured data → Visualization data |
| `output_data` | DataModelStorage | Comparison data | Processed data → Reference data → Overlay plots |
| `html_name` | String | HTML filenames | Base name → Category-specific filenames |
| `sensor` | String | Directory structure | Sensor ID → Folder path → Navigation element |
| `stream_name` | String | Configuration lookup | Stream ID → Signal config → Directory structure |
| `input_file_name` | String | HTML metadata | Filename → Template placeholder → User display |
| `output_file_name` | String | HTML metadata | Filename → Template placeholder → User display |
| `output_dir` | String | File system path | Directory path → Base path → File structure |
| `generate_html` | Boolean | Execution path | Flag → Conditional logic → Resource management |

## Performance Considerations

1. **LRU Caching:** `_get_data_cached()` uses `@lru_cache` for repeated data access
2. **Multiprocessing:** Parallel signal processing for large datasets
3. **Memory Monitoring:** Continuous memory usage tracking
4. **Temporary Files:** JSON storage for plot data to manage memory
5. **Garbage Collection:** Explicit memory cleanup after plot generation

## Error Handling

1. **Import Errors:** Graceful handling of missing `DataCalculations` module
2. **Multiprocessing Fallback:** Sequential processing if parallel processing fails
3. **File I/O Errors:** Error logging for failed plot saves
4. **Memory Errors:** Monitoring and logging of memory issues
5. **Configuration Errors:** Fallback mechanisms for missing signal configurations 