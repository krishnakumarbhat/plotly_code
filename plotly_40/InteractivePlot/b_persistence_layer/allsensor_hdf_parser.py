import h5py
import os
import multiprocessing as mp
from functools import partial
from typing import Dict, List, Optional, Tuple
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
from InteractivePlot.b_persistence_layer.prerun_all_hdf_parser import PreRun
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
import concurrent.futures

import re
class AllsensorHdfParser(PersensorHdfParser):
    """Parser for HDF5 files based on an address map and customer type."""

    def __init__(self, address_map: Dict[str, str], output_dir=None):
        """
        Initialize the AllsensorHdfParser.
        
        Args:
            address_map: Dictionary mapping input file paths to output file paths
            output_dir: Directory to save HTML reports
        """
        super().__init__(address_map, output_dir)
        # Default number of processes - will be updated via set_num_processes if needed
        self.num_processes = min(os.cpu_count() or 4, 8)  # Default to min(CPU count, 8)
        # Flag to control nested multiprocessing
        self.nested_mp_enabled = True
    
    def set_num_processes(self, num_processes: int):
        """
        Set the number of processes to use for parallel processing.
        
        Args:
            num_processes: Number of processes to use
        """
        try:
            num_processes = int(num_processes)
            if num_processes > 0:
                self.num_processes = num_processes
                print(f"Set number of processes for AllsensorHdfParser to {self.num_processes}")
            else:
                print(f"Invalid number of processes ({num_processes}), using default: {self.num_processes}")
        except (ValueError, TypeError):
            print(f"Invalid number of processes ({num_processes}), using default: {self.num_processes}")
    
    def set_nested_mp_enabled(self, enabled: bool):
        """
        Enable or disable nested multiprocessing.
        
        Args:
            enabled: True to enable nested multiprocessing, False to disable
        """
        self.nested_mp_enabled = enabled

    def _process_sensor_data(self, process_args) -> Optional[DataPrep]:
        """
        Process data for a single sensor in a file pair.
        This function can be run in parallel for different sensors.
        
        Args:
            process_args: Tuple containing (
                sensor,
                streams,
                input_file_path,
                output_file_path,
                html_base_name,
                nested_mp_enabled
            )
            
        Returns:
            DataPrep object or None if processing failed
        """
        sensor, streams, input_file_path, output_file_path, html_base_name, nested_mp_enabled = process_args
        
        # Set the environment variable to control nested multiprocessing in DataPrep
        if not nested_mp_enabled:
            os.environ['DISABLE_NESTED_MP'] = 'true'
        else:
            os.environ.pop('DISABLE_NESTED_MP', None)
        
        # Create data storage instances for this sensor
        input_data = DataModelStorage()
        output_data = DataModelStorage()
        
        # Open the HDF files here inside the worker process
        try:
            # Open files in readonly mode
            with h5py.File(input_file_path, 'r') as input_file_content, h5py.File(output_file_path, 'r') as output_file_content:
                # Process all streams for this sensor
                for stream in streams:
                    input_data.init_parent(stream)
                    output_data.init_parent(stream)
                    
                    # Generate HTML name for this sensor
                    html_name = f"{html_base_name}_{sensor}.html"
                    
                    # Process input file data for this sensor/stream
                    sensor_stream_path = f"{sensor}/{stream}"
                    
                    # Process input file for this sensor/stream
                    if sensor_stream_path in input_file_content:
                        try:
                            scan_index = input_file_content[f"{sensor_stream_path}/Stream_Hdr/scan_index"][()]
                            input_data.initialize(scan_index)
                            data_group = input_file_content[sensor_stream_path]
                            hdf_parser.HDF5Parser.parse(data_group, input_data, scan_index)
                        except KeyError as e:
                            print(f"\nWarning: Could not process input data for {sensor}/{stream}: {e}")
                            continue
                    else:
                        print(f"\nWarning: Path {sensor_stream_path} not found in input file")
                        continue
        
                    # Process output file for this sensor/stream
                    if sensor_stream_path in output_file_content:
                        try:
                            scan_index = output_file_content[f"{sensor_stream_path}/Stream_Hdr/scan_index"][()]
                            output_data.initialize(scan_index)
                            data_group = output_file_content[sensor_stream_path]
                            hdf_parser.HDF5Parser.parse(data_group, output_data, scan_index)
                        except KeyError as e:
                            print(f"\nWarning: Could not process output data for {sensor}/{stream}: {e}")
                            continue
                    else:
                        print(f"\nWarning: Path {sensor_stream_path} not found in output file")
                        continue
                
                # Create data container for visualization
                data_container = DataPrep(
                    input_data,
                    output_data,
                    html_name,
                    self.output_dir
                )
                
                return data_container
        except Exception as e:
            print(f"\nError processing sensor {sensor}: {e}")
            return None

    def parse(self) -> List[DataPrep]:
        """
        Parse input and output HDF5 files based on the address map,
        using parallel processing to handle multiple sensors.
        
        Returns:
            List[DataPrep]: List of DataPrep objects for each sensor and stream
        """
        data_containers = []
        
        # Process each input/output file pair
        for input_file, output_file in self.address_map.items():
            # Run pre-processing to get sensor and stream information
            print(f"\nProcessing input file: {os.path.basename(input_file)}")
            prerun_result = PreRun(input_file, output_file)
            missing_data = prerun_result.missing_data
            sensor_list = prerun_result.sensor_list
            streams = prerun_result.streams

            if missing_data and (missing_data.get('missing_groups') or missing_data.get('missing_datasets')):
                print(f"Warning: Missing data in input or output: {missing_data}")
            
            if not sensor_list or not streams:
                print(f"Warning: No valid sensors or streams found in {os.path.basename(input_file)}")
                continue
                
            print(f"Found {len(sensor_list)} sensors: {', '.join(sensor_list)}")
            print(f"Found {len(streams)} streams: {', '.join(streams)}")
            
            # Pre-generate HTML names for reuse
            base_html_name = self.generate_html_name(input_file, output_file)
            base_name = os.path.splitext(base_html_name)[0]
            
            # Prepare tasks for parallel processing - one task per sensor
            # Pass file paths instead of h5py objects to avoid pickling errors
            process_tasks = []
            for sensor in sensor_list:
                process_tasks.append((
                    sensor,
                    streams,
                    input_file,  # Pass file path instead of h5py.File object
                    output_file, # Pass file path instead of h5py.File object
                    base_name,
                    self.nested_mp_enabled  # Pass the nested MP flag
                ))
            
            # Determine the number of processes - use at most the number of CPU cores or sensors, whichever is smaller
            num_processes = min(self.num_processes, len(sensor_list))
            
            # Process sensors in parallel if we have multiple sensors
            if len(sensor_list) > 1 and num_processes > 1:
                print(f"Processing {len(sensor_list)} sensors in parallel using {num_processes} processes...")
                
                # Create a multiprocessing context that uses 'spawn' method for better cross-platform compatibility
                mp_ctx = mp.get_context('spawn')
                
                # Create a process pool with non-daemon processes
                with mp_ctx.Pool(processes=num_processes) as pool:
                    # Process all sensor data in parallel
                    results = pool.map(self._process_sensor_data, process_tasks)
                    
                    # Filter out None results and add to data_containers
                    data_containers.extend([result for result in results if result is not None])
            else:
                # Process sensors sequentially for single sensor or if multiprocessing is not beneficial
                print(f"Processing {len(sensor_list)} sensors sequentially...")
                for task in process_tasks:
                    result = self._process_sensor_data(task)
                    if result is not None:
                        data_containers.append(result)
                    
        return data_containers
