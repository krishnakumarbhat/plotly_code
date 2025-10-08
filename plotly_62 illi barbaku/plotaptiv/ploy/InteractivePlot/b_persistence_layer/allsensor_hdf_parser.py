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
                print(f"Warning: Missing data detected: {missing_data}")
            
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
                    base_html_name = self.generate_html_name(input_file, output_file)
                    base_name = os.path.splitext(base_html_name)[0]
                    self.html_name = f"{base_name}_{sensor}.html"
                    
                    # Process input file
                    with h5py.File(input_file, 'r') as hdf_file:
                        sensor_stream_path = f"{sensor}/{stream}"
                        # b = r'\bstream[_ ]?hdr\b|\bStream[_ ]?Hdr\b|\bstreamheader\b|\bstream[_ ]?header\b'
                        # # Iterate through the keys in hdf_file_out
                        # for key in hdf_file_out.keys():
                        #     # Check if the key matches the regex pattern
                        #     if re.search(b, key):
                        #         # If a match is found, construct the path and retrieve scan_index
                        #         scan_index = hdf_file_out[key + '/scan_index']

                        if sensor_stream_path in hdf_file:
                            scan_index = hdf_file[f"{sensor_stream_path}/Stream_Hdr/scan_index"][()]
                            input_data.initialize(scan_index)
                            data_group = hdf_file[sensor_stream_path]
                            hdf_parser.HDF5Parser.parse(data_group, input_data, scan_index)


                    # Process output file
                    with h5py.File(output_file, 'r') as hdf_file_out:
                        sensor_stream_path = f"{sensor}/{stream}"
                        if sensor_stream_path in hdf_file_out:
                            scan_index = hdf_file_out[f"{sensor_stream_path}/Stream_Hdr/scan_index"][()]
                            output_data.initialize(scan_index)
                            data_group = hdf_file_out[sensor_stream_path]
                            hdf_parser.HDF5Parser.parse(data_group, output_data, scan_index)


                    # Create data container for visualization
                    data_container = DataPrep(
                        input_data,
                        output_data,
                        self.html_name,
                        self.output_dir
                    )
                    data_containers.append(data_container)
                print()  # Add a newline after processing all streams for a sensor
                    
        return data_containers
