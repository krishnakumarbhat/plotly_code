import h5py
import os
import logging
import time
from typing import Dict
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.d_business_layer.data_prep import DataPrep
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
from InteractivePlot.b_persistence_layer.prerun_hdf_parser import PreRun
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from InteractivePlot.c_data_storage.regex_storage import Gen7V1_v2_streams
import asyncio
from concurrent.futures import ThreadPoolExecutor


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

    async def async_parse_files(self):
        """Asynchronous file parsing for better I/O performance"""
        tasks = []
        
        for input_file, output_file in self.address_map.items():
            task = asyncio.create_task(
                self._process_file_pair_async(input_file, output_file)
            )
            tasks.append(task)
            
        results = await asyncio.gather(*tasks, return_exceptions=True)
        return results

    async def _process_file_pair_async(self, input_file, output_file):
        """Process a single file pair asynchronously"""
        loop = asyncio.get_event_loop()
        
        # Run CPU-intensive work in thread pool
        with ThreadPoolExecutor() as executor:
            result = await loop.run_in_executor(
                executor, self._process_sensors_threaded, input_file, output_file
            )
        return result

    def _process_sensors_threaded(self, input_file, output_file):
        """Thread-safe processing of sensors for a file pair"""
        global base_name
        base_name = os.path.basename(input_file).split(".")[0]
        base_name_out = os.path.basename(output_file).split(".")[0]
        logging.info(f"\nProcessing input file: {os.path.basename(input_file)}")
        logging.info(f"Processing output file: {os.path.basename(output_file)} \n")
        
        prerun_result = PreRun(input_file, output_file)
        missing_data = prerun_result.missing_data
        sensor_list = prerun_result.sensor_list
        streams = prerun_result.streams

        if missing_data:
            logging.info(
                f"Warning: Missing data if in input not in output or viceversa : {missing_data}"
            )

        logging.info(f"Found {len(sensor_list)} sensors: {', '.join(sensor_list)}")
        logging.info(f"Found {len(streams)} streams: {', '.join(streams)}")

        total_combinations = len(sensor_list) * len(streams)
        processed = 0
        global hdf_file_in, hdf_file_out

        try:
            hdf_file_in = h5py.File(input_file, "r")
        except Exception as e:
            print(f"\nError processing input file {str(e)}")
            return None

        try:
            hdf_file_out = h5py.File(output_file, "r")
        except Exception as e:
            print(f"\nError processing output file {str(e)}")
            return None

        try:
            for sensor in sensor_list:
                html_name = f"{base_name}_{sensor}.html"
                for stream in streams:
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

                            # Find header using next() with generator expression
                            header_variants = [
                                "Stream_Hdr",
                                "stream_hdr",
                                "StreamHdr",
                                "STREAM_HDR",
                                "streamheader",
                                "stream_header",
                                "HEADER_STREAM",
                            ]
                            header_path = next(
                                (
                                    variant
                                    for variant in header_variants
                                    if variant in data_group
                                ),
                                None,
                            )

                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                input_data.initialize(scan_index, sensor, stream)
                                a = time.time()
                                input_data = hdf_parser.HDF5Parser.parse(
                                    data_group, input_data, scan_index, header_variants
                                )
                                b = time.time()
                                logging.debug(
                                    f"Time taken by input parsing {b - a} in {stream}"
                                )

                        # Process output file
                        sensor_stream_path = f"{sensor}/{stream}"

                        if sensor_stream_path in hdf_file_out:
                            data_group = hdf_file_out[sensor_stream_path]

                            # Find header using next() with generator expression
                            header_variants = [
                                "Stream_Hdr",
                                "stream_hdr",
                                "StreamHdr",
                                "STREAM_HDR",
                                "streamheader",
                                "stream_header",
                                "HEADER_STREAM",
                            ]
                            header_path = next(
                                (
                                    variant
                                    for variant in header_variants
                                    if variant in data_group
                                ),
                                None,
                            )

                            if header_path:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                output_data.initialize(scan_index, sensor, stream)
                                a = time.time()
                                output_data = hdf_parser.HDF5Parser.parse(
                                    data_group, output_data, scan_index, header_variants
                                )
                                b = time.time()
                                logging.debug(
                                    f"Time taken by output parsing {b - a} in {stream}"
                                )

                        # Create data container if we have data
                        if input_data._data_container or output_data._data_container:
                            a = time.time()
                            DataPrep(
                                input_data,
                                output_data,
                                html_name,
                                sensor,
                                stream,
                                base_name,
                                base_name_out,
                                self.output_dir,
                                generate_html=True,
                            )
                            b = time.time()
                            logging.debug(
                                f"Time taken by dataprep parsing {b - a} in {stream}"
                            )

                logging.info(
                    f"Generated HTML report for sensor /{sensor}/ with /{len(streams)}/ streams"
                )
            print("\nCompleted processing all sensor/stream combinations")
            return f"Successfully processed {input_file} and {output_file}"
        
        finally:
            # Ensure files are closed
            if 'hdf_file_in' in locals():
                hdf_file_in.close()
            if 'hdf_file_out' in locals():
                hdf_file_out.close()
