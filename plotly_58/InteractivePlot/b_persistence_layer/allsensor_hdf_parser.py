import h5py
import os
import logging
import time
import gc
from typing import Dict, List, Optional, Tuple
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
from InteractivePlot.b_persistence_layer.prerun_hdf_parser import PreRun
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator
from InteractivePlot.c_data_storage.regex_storage import Gen7V1_v2_streams , header_names

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
        self.start_time_parsing = time.time()

    def parse(self) -> List[DataPrep]:
        """
        Parse input and output HDF5 files based on the address map.
        Uses sequential processing to avoid serialization issues.
        
        Returns:
            List[DataPrep]: List of DataPrep objects for each sensor and stream
        """
        parsing_time_start = time.time()
        for input_file, output_file in self.address_map.items():
            logging.info(f"\nProcessing input file: {os.path.basename(input_file)}")
            logging.info(f"Processing output file: {os.path.basename(output_file)} \n")
            prerun_result = PreRun(input_file, output_file)
            missing_data = prerun_result.missing_data
            sensor_list = prerun_result.sensor_list
            streams = prerun_result.streams

            if missing_data:
                logging.debug(f"Warning: Missing data if in input not in output or viceversa : {missing_data}")
            
            print(f"Found {len(sensor_list)} sensors: {', '.join(sensor_list)}")
            print(f"Found {len(streams)} streams: {', '.join(streams)}")

            total_combinations = len(sensor_list) * len(streams)
            processed = 0
            global hdf_file_in , hdf_file_out
            try:
                hdf_file_in = h5py.File(input_file, 'r')
                    
            except Exception as e:
                print(f"\nError processing input file for {sensor}/{stream}: {str(e)}")
        
            try:
                hdf_file_out = h5py.File(output_file, 'r')
                    
            except Exception as e:
                print(f"\nError processing output file for {sensor}/{stream}: {str(e)}")


            for sensor in sensor_list:
                base_name = os.path.basename(input_file).split('.')[0]
                sensor_plots_hash = {}
                sensor_kpi_plots = {}
                for stream in streams:
                    html_name = f"{base_name}_{sensor}.html"
                    if stream in Gen7V1_v2_streams:
                        processed += 1
                        progress = (processed / total_combinations) * 100
                        print(f"\rProcessing...... [{progress:.1f}%]", end="")
                        
                        # Create storage instances for this combination
                        input_data = DataModelStorage()
                        output_data = DataModelStorage()
                        
                        input_data.init_parent(stream)
                        output_data.init_parent(stream)
                        
                        # Process input file
                        sensor_stream_path = f"{sensor}/{stream}"
                        
                        if sensor_stream_path in hdf_file_in:
                            data_group = hdf_file_in[sensor_stream_path]
                            
                            # Find header using next() with generator expression for both inp and out
                            header_path = next((variant for variant in header_names if variant in data_group), None)
                            
                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                num_of_scanidx = len(scan_index)

                            split_factors = [
                                (500, 1),
                                (1000, 2),
                                (1500, 3),
                                (2000, 4),
                                (2500, 5),
                                (3000, 6),
                                (float('inf'), 8)
                            ]

                            for threshold, factor in split_factors:
                                if num_of_scanidx < threshold:
                                    split_num = factor

                                input_data.initialize(scan_index,sensor,stream,split_num)
                                a= time.time()
                                input_data = hdf_parser.HDF5Parser.parse(data_group, input_data, scan_index,header_names)
                                b = time.time()
                                print(f"Time taken by input parsing {b-a} in {stream}")
                        
                        # Process output file
                        sensor_stream_path = f"{sensor}/{stream}"
                        
                        if sensor_stream_path in hdf_file_out:
                            data_group = hdf_file_out[sensor_stream_path]
                            
                            header_path = next((variant for variant in header_names if variant in data_group), None)
                            
                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                
                                output_data.initialize(scan_index,sensor,stream)
                                a= time.time()
                                output_data = hdf_parser.HDF5Parser.parse(data_group, output_data, scan_index,header_names)
                                b = time.time()
                                print(f"Time taken by output parsing {b-a} in {stream}")
                        parsing_time_end = time.time()
                        logging.info(f"time taken to parse completly {parsing_time_end-parsing_time_start}")
                        
                        # Create data container if we have data
                        if input_data._data_container or output_data._data_container:
                            a= time.time()
                            # try:
                            data_prep = DataPrep(
                                input_data,
                                output_data,
                                html_name,
                                sensor,
                                stream,
                                self.output_dir,
                                generate_html=False
                            )
                            b= time.time()
                            print(f"Time taken by dataprep parsing {b-a} in {stream}")
                            if stream not in sensor_plots_hash:
                                sensor_plots_hash[stream] = data_prep.plots_hash

                            # Store KPI plots if any
                            if data_prep.kpi_plots:
                                sensor_kpi_plots['KPI'] = data_prep.kpi_plots
                            logging.debug(f"  Successfully created DataPrep for {sensor}/{stream}")
                        
                            # Clean memory to free up resources
                            # Clear large data objects to reduce memory usage
                            if hasattr(data_prep, 'cleanup') and callable(data_prep.cleanup):
                                data_prep.cleanup()
                            
                            # Clear data containers as they're no longer needed after plots are generated
                            if hasattr(input_data, '_data_container'):
                                input_data.clear()
                            
                            if hasattr(output_data, '_data_container'):
                                output_data.clear()
                                
                            # Force garbage collection
                            gc.collect()
                            
                            # except Exception as e:
                                # print(f"\nError creating DataPrep for {sensor}/{stream}: {str(e)}")
                                            # Store the plots from this stream
                            # Generate HTML for this sensor with all streams as tabs
                if sensor_plots_hash and html_name:
                    end_time_each_parsing = time.time()
                    parsing_time = end_time_each_parsing - self.start_time_parsing
                    print(f"{parsing_time}")

                    HtmlGenerator(
                        sensor_plots_hash, 
                        sensor_kpi_plots, 
                        html_name, 
                        self.output_dir
                    )

            print(f"Generated HTML report for sensor {sensor} with {len(sensor_plots_hash)} streams")
            print("\nCompleted processing all sensor/stream combinations")
                    
