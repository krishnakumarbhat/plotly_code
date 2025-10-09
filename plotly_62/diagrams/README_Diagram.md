# DataPrep Flow Diagram Documentation

## Overview
This directory contains a comprehensive block diagram that illustrates the data preparation (`data_prep`) and HTML generation (`html_gen`) processes in the Interactive Plot application. The diagram shows the complete flow of data from input parameters to final HTML output.

## Files Included

1. **`data_prep_flow_diagram.dot`** - Graphviz DOT file containing the diagram definition
2. **`generate_diagram.py`** - Python script to generate the diagram
3. **`data_prep_parameter_analysis.md`** - Detailed analysis of each parameter's flow
4. **`README_Diagram.md`** - This documentation file

## Diagram Features

### 🎯 **What the Diagram Shows:**
- **Input Parameters:** All 9 parameters passed to `DataPrep.__init__()`
- **Data Flow:** How data moves through the system with directional arrows
- **Process Steps:** Each major function and method in the processing pipeline
- **Conditional Logic:** Different execution paths based on parameters
- **Output Generation:** HTML file creation and organization
- **Memory Management:** Memory tracking and cleanup processes

### 🎨 **Visual Elements:**
- **Color-coded clusters:** Different colors for different process phases
- **Shape coding:** Cylinders for data storage, boxes for processes
- **Arrow types:** 
  - Solid arrows: Direct data flow
  - Dashed arrows: Data dependencies
  - Dotted arrows: Conditional execution paths
- **Legend:** Explains all visual elements

## Generating the Diagram

### Prerequisites
You need to have Graphviz installed on your system:

#### Windows:
```bash
# Download from https://graphviz.org/download/
# Or use Chocolatey:
choco install graphviz
```

#### macOS:
```bash
brew install graphviz
```

#### Ubuntu/Debian:
```bash
sudo apt-get install graphviz
```

#### CentOS/RHEL:
```bash
sudo yum install graphviz
```

### Generation Commands

#### Option 1: Using the Python Script (Recommended)
```bash
python generate_diagram.py
```

This will:
- Check if Graphviz is installed
- Generate both PNG and SVG versions
- Provide detailed feedback on the process

#### Option 2: Direct Graphviz Commands
```bash
# Generate PNG version
dot -Tpng -o data_prep_flow_diagram.png data_prep_flow_diagram.dot

# Generate SVG version (scalable)
dot -Tsvg -o data_prep_flow_diagram.svg data_prep_flow_diagram.dot

# Generate PDF version
dot -Tpdf -o data_prep_flow_diagram.pdf data_prep_flow_diagram.dot
```

## Understanding the Diagram

### 🔄 **Process Flow Overview:**

1. **Input Parameters** (Light Blue Cluster)
   - All 9 parameters enter the system
   - Parameters are validated and stored

2. **DataPrep Class** (Light Green Cluster)
   - Initialization and memory tracking
   - Main plot generation orchestration

3. **Plot Generation Process** (Light Coral Cluster)
   - Data calculations import
   - Multiprocessing setup
   - Signal configuration retrieval
   - Parallel/sequential processing

4. **Individual Signal Processing** (Light Steel Blue Cluster)
   - Cached data retrieval
   - Alias checking and fallback
   - Plot type creation
   - JSON file generation

5. **HTML Generation** (Light Pink Cluster)
   - Folder structure creation
   - Plot categorization
   - HTML file generation
   - Index creation

6. **Generated Outputs** (Light Goldenrod Cluster)
   - Category-specific HTML files
   - Main navigation index
   - Master index with dropdowns
   - Temporary file cleanup

### 📊 **Key Data Transformations:**

| Parameter | Transformation |
|-----------|---------------|
| `input_data` | Raw HDF5 → Structured data → Plot arrays |
| `output_data` | Processed data → Comparison data → Overlay plots |
| `html_name` | Filename → Stem → Category filenames |
| `sensor` | Sensor ID → Directory structure → Navigation |
| `stream_name` | Stream ID → Config lookup → Directory structure |
| `input_file_name` | Filename → Template placeholder → User display |
| `output_file_name` | Filename → Template placeholder → User display |
| `output_dir` | Path → Base directory → File structure |
| `generate_html` | Boolean → Conditional logic → Resource management |

## Performance Insights

The diagram reveals several performance optimizations:

1. **LRU Caching:** `_get_data_cached()` prevents redundant data access
2. **Multiprocessing:** Parallel signal processing for large datasets
3. **Memory Monitoring:** Continuous tracking of memory usage
4. **Temporary Files:** JSON storage to manage memory efficiently
5. **Garbage Collection:** Explicit cleanup after plot generation

## Error Handling

The diagram shows error handling mechanisms:

1. **Import Errors:** Graceful handling of missing modules
2. **Multiprocessing Fallback:** Sequential processing if parallel fails
3. **File I/O Errors:** Error logging for failed operations
4. **Memory Errors:** Monitoring and logging of memory issues
5. **Configuration Errors:** Fallback for missing signal configs

## Customization

### Modifying the Diagram

To modify the diagram:

1. Edit `data_prep_flow_diagram.dot`
2. Regenerate using the commands above
3. The DOT file uses standard Graphviz syntax

### Adding New Elements

```dot
// Add a new process node
new_process [label="New Process\nDescription", fillcolor=lightblue];

// Add connections
existing_node -> new_process;
new_process -> output_node;
```

### Changing Colors

```dot
// Change cluster color
subgraph cluster_name {
    label="Cluster Label";
    style=filled;
    color=lightblue;  // Change this color
}

// Change node color
node_name [fillcolor=lightyellow];  // Change this color
```

## Troubleshooting

### Common Issues:

1. **Graphviz not found:**
   - Install Graphviz using the commands above
   - Ensure it's in your system PATH

2. **Diagram too large:**
   - Use SVG format for better scaling
   - Adjust font sizes in the DOT file

3. **Overlapping elements:**
   - Graphviz automatically layouts the diagram
   - Adjust node positioning if needed

4. **Missing dependencies:**
   - Ensure all required Python packages are installed
   - Check file paths are correct

## Related Documentation

- **`data_prep_parameter_analysis.md`** - Detailed parameter flow analysis
- **`InteractivePlot/d_business_layer/data_prep.py`** - Source code
- **`InteractivePlot/e_presentation_layer/html_generator.py`** - HTML generation code

## Support

If you encounter issues with diagram generation:

1. Check Graphviz installation: `dot -V`
2. Verify file permissions
3. Check Python dependencies
4. Review error messages in the console output

The diagram provides a comprehensive visual understanding of how data flows through the Interactive Plot application's data preparation and HTML generation processes. 