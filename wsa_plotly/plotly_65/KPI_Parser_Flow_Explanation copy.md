# KPI Parser Flow and State Transitions Explanation

## Overview
The KPI parsing system processes multiple streams of HDF5 data incrementally, using bracket notation to maintain stream separation while storing all data in a single model.

## Data Flow Architecture

### 1. **Entry Point: kpi_server.py**
- Receives ZMQ requests with sensor data information
- Calls `parse_for_kpi()` function with parameters:
  - `sensor_id`: Target sensor identifier
  - `input_file_path`: Path to input HDF5 file
  - `output_file_path`: Path to output HDF5 file
  - `output_dir`: Directory for HTML reports
  - `base_name`: Base name for output files

### 2. **Wrapper Layer: hdf_wrapper.py**
- Creates `KPIHDFWrapper` instance with `KPIProcessingConfig`
- Manages HDF5 file operations and stream coordination
- Maintains single `stream_input_model` and `stream_output_model` instances
- Coordinates parsing across multiple streams with bracket notation

### 3. **Parser Layer: kpi_hdf_parser.py**
- `KPIHDFParser.parse()` performs depth-first traversal of HDF5 groups
- Filters data based on KPI configuration (alignment, detection, tracker)
- Processes datasets and stores them in `KPI_DataModelStorage`

### 4. **Storage Layer: kpi_data_model_storage.py**
- `KPI_DataModelStorage` manages bidirectional signal mappings
- Stores data in hierarchical structure with scan indices
- Supports bracket notation for stream identification

### 5. **Business Logic: kpi_factory.py**
- `KpiDataModel` processes stored data for KPI generation
- Runs alignment, detection, and tracker KPIs in parallel
- Generates HTML reports

## Parser Loop Sequence Flow

### Phase 1: Initialization
```
1. Start parse() method
2. Extract scan_index from first available stream
3. Initialize both models with scan_index and sensor_id
   - stream_input_model.initialize(scan_index, sensor)
   - stream_output_model.initialize(scan_index, sensor)
```

### Phase 2: Stream Processing Loop
```
For each stream in [alignment, detection, tracker] streams:
  1. Set stream index: stream_idx = 0, 1, 2, ...
  2. Log: "Processing stream [{stream_idx}] {stream}"
  3. Initialize parent with bracket notation:
     - stream_input_model.init_parent(f"[{stream_idx}_{stream}]")
     - stream_output_model.init_parent(f"[{stream_idx}_{stream}]")
  4. Check if HDF5 group exists for sensor/stream
  5. If exists:
     a. Parse input stream: KPIHDFParser.parse(input_group, stream_input_model)
     b. Parse output stream: KPIHDFParser.parse(output_group, stream_output_model)
  6. Continue to next stream
```

### Phase 3: Data Storage Operations
```
For each dataset in stream:
  1. Call set_value(dataset, signal_name, group_name)
  2. Check if new parent group (first signal in stream)
  3. If new parent:
     - Generate keys: parent_counter_None, parent_counter_child_counter
     - Process entire dataset with _process_dataset()
     - Store with bracket notation: [stream_idx_stream_name]_signal_name
  4. If child item:
     - Increment child_counter
     - Append data to existing parent's data structure
     - Update signal mappings
```

### Phase 4: Completion
```
1. Create KpiDataModel with both populated models
2. Close HDF5 files
3. Return results with HTML report path
```

## State Transitions

### Key States:
1. **INIT** → **SCAN_EXTRACTED**: Extract scan indices from first stream
2. **SCAN_EXTRACTED** → **MODELS_INITIALIZED**: Initialize data containers
3. **MODELS_INITIALIZED** → **STREAM_PROCESSING**: Begin stream loop
4. **STREAM_PROCESSING** → **PARENT_SET**: Set bracket notation parent
5. **PARENT_SET** → **INPUT_PARSING** / **OUTPUT_PARSING**: Parse HDF5 data
6. **PARSING** → **DATA_STORED**: Store in KPI_DataModelStorage
7. **DATA_STORED** → **STREAM_COMPLETE**: Finish current stream
8. **STREAM_COMPLETE** → **STREAM_PROCESSING**: Next stream (loop)
9. **STREAM_COMPLETE** → **ALL_STREAMS_DONE**: All streams processed
10. **ALL_STREAMS_DONE** → **KPI_GENERATED**: Create KpiDataModel
11. **KPI_GENERATED** → **COMPLETE**: Return results

### Error Handling:
- Parse errors are caught and logged
- Processing continues with next stream
- Failed streams are marked as unavailable in results

## Key Data Structures

### Bracket Notation System:
- Stream identification: `[0_detection]`, `[1_alignment]`, `[2_tracker]`
- Signal names: `[0_detection]_signal_name`
- Parent keys: `0_None`, `1_None`, `2_None`
- Child keys: `0_0`, `0_1`, `1_0`, `1_1`, etc.

### Storage Mappings:
- `_signal_to_value`: Maps signal names to storage keys
- `_value_to_signal`: Maps storage keys to signal names  
- `_data_container`: Maps scan indices to data lists

### Counter System:
- `_parent_counter`: Increments for each new stream (-1 → 0 → 1 → 2)
- `_child_counter`: Resets for each stream, increments for each signal

## Memory Management
- Single model instances shared across all streams
- Bracket notation prevents data overwrites
- Stream data accumulated incrementally
- HDF5 files closed after processing
- Garbage collection in parser cleanup

This architecture ensures proper stream separation while maintaining memory efficiency through shared model instances and incremental processing.
