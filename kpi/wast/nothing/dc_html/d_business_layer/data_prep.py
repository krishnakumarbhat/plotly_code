import importlib
from c_data_storage.config_storage import Gen7V1_V2
from c_data_storage.data_model_storage import DataModelStorage
from d_business_layer.data_dumper import DataDumper
from d_business_layer.utils import time_taken
import multiprocessing as mp
from functools import lru_cache
import logging
import time
import gc
import os
import tempfile
import shutil
from memory_profiler import profile ,memory_usage
import psutil

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling plot data.
    Acts as a business layer between data storage and presentation.

    This class is responsible for:
    1. Retrieving data from storage layers (input and output)
    2. Processing and transforming data for visualization
    3. Generating appropriate plot types based on signal configuration
    4. Implementing multiprocessing for efficient data processing
    5. Handling KPI calculations.
    6. Passing visualization data to the presentation layer
    7. Dumping specific datasets to text files
    """
    # @profile
    def __init__(
        self,
        input_data,
        output_data,
        html_name,
        sensor,
        stream_name,
        input_file_name,
        output_file_name,
        output_dir=None,
        generate_html=True,
    ):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage containing signal data
            output_data: Output data storage containing processed signal data
            html_name: Name for the HTML file to be generated
            output_dir: Directory to save HTML reports (defaults to "html")
            stream_name: Type of data stream being processed (e.g., 'Radar', 'DETECTION_STREAM')
            generate_html: Whether to generate HTML file or just return plot data
        """
        self.input_data = input_data
        self.output_data = output_data
        self.html_name = html_name
        self.sensor = sensor
        self.stream_name = stream_name
        self.input_file_name = input_file_name
        self.output_file_name = output_file_name
        self.output_dir = output_dir or "html"
        self.generate_html = generate_html
        
        # Set up logging
        self.logger = logging.getLogger(__name__)
        
        # Dump data after acquisition and before preparation
        self._dump_data_after_acquisition()
        
        # Exit after dumping as requested
        self.logger.info("Data dumping completed. Exiting as requested.")
        return
        
    def _dump_data_after_acquisition(self):
        """
        Dump specific datasets from HDF files after data acquisition is complete.
        This method is called after data has been parsed and stored.
        """
        try:
            self.logger.info("Starting data dumping process...")
            
            # Determine the HDF file path from the input file name
            # The input file name should be the HDF file path
            hdf_file_path = self.input_file_name
            if not os.path.exists(hdf_file_path):
                # Try with .h5 extension if not already present
                if not hdf_file_path.endswith('.h5'):
                    hdf_file_path = f"{hdf_file_path}.h5"
                if not os.path.exists(hdf_file_path):
                    self.logger.warning(f"HDF file not found: {hdf_file_path}")
                    return
            
            # Create data dumper and dump all data
            dumper = DataDumper(hdf_file_path, self.output_dir)
            results = dumper.dump_all_data()
            
            # Log results
            for data_type, success in results.items():
                if success:
                    self.logger.info(f"Successfully dumped {data_type} data")
                else:
                    self.logger.error(f"Failed to dump {data_type} data")
                    
        except Exception as e:
            self.logger.error(f"Error during data dumping: {e}")
       