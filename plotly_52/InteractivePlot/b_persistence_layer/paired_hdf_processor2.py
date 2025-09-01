import h5py
import os
import plotly.graph_objects as go
from collections import defaultdict
from typing import Dict, List, Optional, Tuple, Set
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.b_persistence_layer.prerun_paired_hdf_parser import PreRunPaired
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from InteractivePlot.d_business_layer.data_prep import DataPrep

class PairedHdfProcessor:
    """
    Processor for paired HDF5 files that handles synchronization and processing of 
    sensors common to both input and output files. The processor simultaneously opens
    all files in a pair, identifies common sensors and streams, and aggregates
    the data into a unified data store for KPI calculations and HTML generation.
    """

    def __init__(self, paired_files_list, output_dir=None):
        """
        Initialize the PairedHdfProcessor.
        
        Args:
            paired_files_list: List of dictionaries, each containing 'input_files' and 'output_files' keys
                               where each value is a list of file paths
            output_dir: Directory to save HTML reports
        """
        self.paired_files_list = paired_files_list
        self.output_dir = output_dir or "html"
        
        # Create output directory if it doesn't exist
        os.makedirs(self.output_dir, exist_ok=True)

    def _process_file(self, file_path: str, sensor: str, stream: str, data_storage: DataModelStorage) -> bool:
        """
        Process a single HDF file for a sensor/stream combination.
        
        Args:
            file_path: Path to the HDF file
            sensor: Sensor name
            stream: Stream name
            data_storage: DataModelStorage to store the parsed data
            
        Returns:
            bool: True if processing was successful, False otherwise
        """
        try:
            with h5py.File(file_path, 'r') as hdf_file:
                sensor_stream_path = f"{sensor}/{stream}"
                
                if sensor_stream_path in hdf_file:
                    data_group = hdf_file[sensor_stream_path]
                    
                    # Define possible header names and find the correct one
                    header_variants = ['Stream_Hdr', 'stream_hdr', 'StreamHdr', 'STREAM_HDR', 'streamheader', 'stream_header']
                    header_path = next((variant for variant in header_variants if variant in data_group), None)
                    
                    if header_path:
                        scan_index = data_group[f"{header_path}/scan_index"][()]
                        data_storage.initialize(scan_index)
                        data_storage = hdf_parser.HDF5Parser.parse(data_group, data_storage, scan_index)
                        return True
                    else:
                        print(f"No stream header found in {sensor_stream_path}")
                        return False
                else:
                    print(f"Sensor/stream path {sensor_stream_path} not found in {os.path.basename(file_path)}")
                    return False
        except Exception as e:
            print(f"Error processing {file_path} for {sensor}/{stream}: {str(e)}")
            return False

    def parse(self):
        """
        Process all paired HDF5 files.
        
        For each file pair:
        1. Identifies sensors and streams common to both input and output files
        2. For each matching sensor/stream, processes each file in the pair
        3. Generates an HTML report with tabs for each sensor/stream
        
        Returns:
            List of DataPrep objects
        """
        all_data_preps = []
        
        # Loop through each pair set
        for pair_index, pair_data in enumerate(self.paired_files_list):
            input_files = pair_data.get('input_pair', [])
            output_files = pair_data.get('output_pair', [])
            
            # Ensure input and output file lists have the same length
            if len(input_files) != len(output_files):
                print(f"Skipping pair #{pair_index+1}: Number of input files ({len(input_files)}) " 
                      f"doesn't match number of output files ({len(output_files)})")
                continue
                
            print(f"\nProcessing file pair #{pair_index+1}:")
            print(f"Input files: {', '.join([os.path.basename(f) for f in input_files])}")
            print(f"Output files: {', '.join([os.path.basename(f) for f in output_files])}")
            
            # Use PreRunPaired to analyze files and find common sensors/streams
            prerun = PreRunPaired(input_files, output_files)
            
            # Process each file pair independently
            for pair_idx in prerun.pair_data:
                pair_info = prerun.pair_data[pair_idx]
                
                input_file = pair_info['input_file']
                output_file = pair_info['output_file']
                
                base_name = os.path.basename(input_file).split('.')[0]
                
                # Process each sensor found in this pair
                for sensor in pair_info['sensors']:
                    sensor_html_name = f"{base_name}_{sensor}.html"
                    
                    # Initialize a list to store DataPrep objects for each stream of this sensor
                    sensor_data_preps = []
                    
                    # Get streams available for this sensor
                    if sensor in pair_info['streams']:
                        streams = pair_info['streams'][sensor]
                        
                        print(f"\nProcessing sensor {sensor} with {len(streams)} streams")
                        
                        # Process each stream for this sensor
                        for stream in streams:
                            print(f"  Processing stream: {stream}")
                            
                            # Create data storage instances for input and output
                            input_data = DataModelStorage()
                            output_data = DataModelStorage()
                            
                            input_data.init_parent(stream)
                            output_data.init_parent(stream)
                            
                            # Process input file
                            input_success = self._process_file(input_file, sensor, stream, input_data)
                            
                            # Process output file
                            output_success = self._process_file(output_file, sensor, stream, output_data)
                            
                            # If either file was processed successfully, create a DataPrep
                            if input_success or output_success:
                                try:
                                    data_prep = DataPrep(
                                        input_data,
                                        output_data,
                                        sensor_html_name,
                                        self.output_dir,
                                        stream
                                    )
                                    sensor_data_preps.append(data_prep)
                                    all_data_preps.append(data_prep)
                                    print(f"  Successfully created DataPrep for {sensor}/{stream}")
                                except Exception as e:
                                    print(f"  Error creating DataPrep for {sensor}/{stream}: {str(e)}")
                        
                        print(f"Completed processing sensor {sensor} with {len(sensor_data_preps)} successful streams")
                
                # Now process the next pair if there are multiple pairs
        
        print(f"\nCompleted processing all file pairs. Generated {len(all_data_preps)} DataPrep objects.")
        return all_data_preps
