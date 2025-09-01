import h5py
import os
from typing import Dict, List, Optional
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
from InteractivePlot.b_persistence_layer.prerun_all_hdf_parser import PreRun
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
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

    def parse(self) -> List[DataPrep]:
        """
        Parse input and output HDF5 files based on the address map.
        
        Returns:
            List[DataPrep]: List of DataPrep objects for each sensor and stream
        """
        data_containers = []
        for input_file, output_file in self.address_map.items():
            # Run pre-processing to get sensor and stream information
            print(f"\nProcessing input file: {os.path.basename(input_file)}")
            prerun_result = PreRun(input_file, output_file)
            missing_data = prerun_result.missing_data
            sensor_list = prerun_result.sensor_list
            streams = prerun_result.streams

            if missing_data:
                print(f"Warning: Missing data if in input not in output or viceversa : {missing_data}")
            
            print(f"Found {len(sensor_list)} sensors: {', '.join(sensor_list)}")
            print(f"Found {len(streams)} streams: {', '.join(streams)}")
            
            total_items = len(sensor_list) * len(streams)
            processed_items = 0

            for sensor in sensor_list:
                # Create new storage instances for each sensor
                input_data = DataModelStorage()
                output_data = DataModelStorage()
                
                for stream in streams:
                    processed_items += 1
                    progress = (processed_items / total_items) * 100
                    print(f"\rProcessing sensor: {sensor}, stream: {stream} [{progress:.1f}%]", end="")
                    
                    input_data.init_parent(stream)
                    output_data.init_parent(stream)
                    
                    # Generate HTML name using base method from parent class, adding sensor info
                    # base_html_name = self.generate_html_name(input_file, output_file)
                    base_name = os.path.basename(input_file).split('.')[0]
                    self.html_name = f"{base_name}_{sensor}.html"
                    
                    # Process input file
                    with h5py.File(input_file, 'r') as hdf_file:
                        sensor_stream_path = f"{sensor}/{stream}"
                        
                        if sensor_stream_path in hdf_file:
                            data_group = hdf_file[sensor_stream_path]
                            
                            # Define possible header names
                            header_variants = ['Stream_Hdr', 'stream_hdr', 'StreamHdr', 'STREAM_HDR', 'streamheader', 'stream_header']
                            
                            # Find the correct header name if it exists
                            header_path = None
                            for variant in header_variants:
                                if variant in data_group:
                                    header_path = variant
                                    break
                            
                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                input_data.initialize(scan_index)
                                input_data = hdf_parser.HDF5Parser.parse(data_group, input_data, scan_index)
                            else:
                                # Handle case where no header is found
                                print(f"No stream header found in {sensor_stream_path} (input)")

                    # Process output file
                    with h5py.File(output_file, 'r') as hdf_file_out:
                        sensor_stream_path = f"{sensor}/{stream}"
                        
                        if sensor_stream_path in hdf_file_out:
                            data_group = hdf_file_out[sensor_stream_path]
                            
                            # Define possible header names
                            header_variants = ['Stream_Hdr', 'stream_hdr', 'StreamHdr', 'STREAM_HDR', 'streamheader', 'stream_header']
                            
                            # Find the correct header name if it exists
                            header_path = None
                            for variant in header_variants:
                                if variant in data_group:
                                    header_path = variant
                                    break
                            
                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                output_data.initialize(scan_index)
                                output_data = hdf_parser.HDF5Parser.parse(data_group, output_data, scan_index)
                            else:
                                # Handle case where no header is found
                                print(f"No stream header found in {sensor_stream_path} (output)")

                    # Create data container for visualization
                    data_container = DataPrep(
                        input_data,
                        output_data,
                        self.html_name,
                        self.output_dir,
                        stream
                    )
                    data_containers.append(data_container)
                    
        return data_containers
