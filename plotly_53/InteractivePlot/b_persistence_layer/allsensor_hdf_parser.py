import h5py
import os
from typing import Dict, List, Optional, Tuple
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
from InteractivePlot.b_persistence_layer.prerun_all_hdf_parser import PreRun
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

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
        Uses sequential processing to avoid serialization issues.
        
        Returns:
            List[DataPrep]: List of DataPrep objects for each sensor and stream
        """

        for input_file, output_file in self.address_map.items():
            print(f"\nProcessing input file: {os.path.basename(input_file)}")
            prerun_result = PreRun(input_file, output_file)
            missing_data = prerun_result.missing_data
            sensor_list = prerun_result.sensor_list
            streams = prerun_result.streams

            if missing_data:
                print(f"Warning: Missing data if in input not in output or viceversa : {missing_data}")
            
            print(f"Found {len(sensor_list)} sensors: {', '.join(sensor_list)}")
            print(f"Found {len(streams)} streams: {', '.join(streams)}")

            total_combinations = len(sensor_list) * len(streams)
            processed = 0
            
            for sensor in sensor_list:
                base_name = os.path.basename(input_file).split('.')[0]
                html_name = f"{base_name}_{sensor}.html"
                
                for stream in streams:
                    processed += 1
                    progress = (processed / total_combinations) * 100
                    print(f"\rProcessing...... [{progress:.1f}%]", end="")
                    
                    # Create storage instances for this combination
                    input_data = DataModelStorage()
                    output_data = DataModelStorage()
                    
                    input_data.init_parent(stream)
                    output_data.init_parent(stream)
                    
                    # Process input file
                    try:
                        with h5py.File(input_file, 'r') as hdf_file:
                            sensor_stream_path = f"{sensor}/{stream}"
                            
                            if sensor_stream_path in hdf_file:
                                data_group = hdf_file[sensor_stream_path]
                                
                                # Find header using next() with generator expression
                                header_variants = ['Stream_Hdr', 'stream_hdr', 'StreamHdr', 'STREAM_HDR', 'streamheader', 'stream_header']
                                header_path = next((variant for variant in header_variants if variant in data_group), None)
                                
                                if header_path:
                                    scan_index = data_group[f"{header_path}/scan_index"][()]
                                    input_data.initialize(scan_index)
                                    input_data = hdf_parser.HDF5Parser.parse(data_group, input_data, scan_index)
                    except Exception as e:
                        print(f"\nError processing input file for {sensor}/{stream}: {str(e)}")
                    
                    # Process output file
                    try:
                        with h5py.File(output_file, 'r') as hdf_file:
                            sensor_stream_path = f"{sensor}/{stream}"
                            
                            if sensor_stream_path in hdf_file:
                                data_group = hdf_file[sensor_stream_path]
                                
                                # Find header using next() with generator expression
                                header_variants = ['Stream_Hdr', 'stream_hdr', 'StreamHdr', 'STREAM_HDR', 'streamheader', 'stream_header']
                                header_path = next((variant for variant in header_variants if variant in data_group), None)
                                
                                if header_path:
                                    scan_index = data_group[f"{header_path}/scan_index"][()]
                                    output_data.initialize(scan_index)
                                    output_data = hdf_parser.HDF5Parser.parse(data_group, output_data, scan_index)
                    except Exception as e:
                        print(f"\nError processing output file for {sensor}/{stream}: {str(e)}")
                    
                    # Create data container if we have data
                    if input_data._data_container or output_data._data_container:
                        try:
                            DataPrep(
                                input_data,
                                output_data,
                                html_name,
                                self.output_dir,
                                stream
                            )
                        except Exception as e:
                            print(f"\nError creating DataPrep for {sensor}/{stream}: {str(e)}")
            
            print("\nCompleted processing all sensor/stream combinations")
                    
