*Interactive Plot Generation Project
This project processes configuration and JSON files to produce interactive plots. The main workflow involves parsing an XML configuration file, reading JSON mappings, processing HDF5 files, and outputting HTML files with Plotly visualizations.
<!-- ---------------------------------------------------------------------------------------------------------------- -->
*Project Structure
├── ConfigInteractivePlots.xml                # XML configuration file
├── InputsInteractivePlot.json                # JSON configuration for all sensors
├── InputsPerSensorInteractivePlot.json       # JSON configuration for per-sensor processing
├── InteractivePlot                           # Main package containing project modules
├── KPI                                       # Additional KPI calculations
│   └── detection_matching_kpi.py
└── tests                                     # Unit tests for the project

<!-- ---------------------------------------------------------------------------------------------------------------- -->
*Prerequisites
Python 3.6 or above
Required Python packages (see below)
Install the required packages using pip. For example, if all necessary packages are listed in a requirements file:

pip install -r requirements.txt

<!-- ---------------------------------------------------------------------------------------------------------------- -->
*How to Run
You can run the project from the command line by specifying the XML configuration file and the JSON input file. For example:

python main.py ConfigInteractivePlots.xml InputsInteractivePlot.json

This command will:
Parse the XML configuration with an instance of XmlConfigParser.
Determine the HDF5 file type.
Use JSONParserFactory to obtain the appropriate JSON parser to generate an input/output map.
Process the HDF5 files using the HdfProcessorFactory.


<!-- ---------------------------------------------------------------------------------------------------------------- -->
*Testing
Unit tests are provided in the tests directory. To run all the tests, use your preferred Python testing framework (e.g., pytest). For example:

pytest tests/
Make sure to install pytest if you haven't already:

pip install pytest


<!-- ---------------------------------------------------------------------------------------------------------------- -->

Development Tools

Visual Studio Code is configured using files in .vscode.
Diagrams and documentation images can be found in the Readme folder for an overview of the project architecture.
Additional Information

The project uses a layered approach to separate configuration parsing (a_config_layer), persistence (b_persistence_layer), business logic (c_business_layer), and presentation (d_presentation_layer).


The main entry point of the application is main.py.
