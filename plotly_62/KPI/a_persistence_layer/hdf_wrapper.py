import h5py
import os
import logging
import time
from typing import Dict, Any, Optional, List
from dataclasses import dataclass

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from InteractivePlot.d_business_layer.utils import time_taken
from KPI.b_data_storage.kpi_config_storage import KPI_ALIGNMENT_CONFIG, KPI_DETECTION_CONFIG, KPI_TRACKER_CONFIG
from KPI.a_persistence_layer.kpi_hdf_parser import KPIHDFParser
from KPI.c_business_layer.kpi_factory import KpiDataModel

logger = logging.getLogger(__name__)


@dataclass
class KPIProcessingConfig:
    sensor_id: str
    input_file_path: str
    output_file_path: str
    output_dir: str
    base_name: str
    kpi_subdir: str = "kpi"


class KPIHDFWrapper:
    """Parses KPI-related streams from HDF5 input/output and forwards to KPI factory."""

    def __init__(self, config: KPIProcessingConfig):
        self.config = config
        self.start_time_parsing = time.time()
        self.header_variants = [
            "Stream_Hdr",
            "stream_hdr",
            "StreamHdr",
            "STREAM_HDR",
            "streamheader",
            "stream_header",
            "HEADER_STREAM",
        ]
        self.stream_input_model = KPI_DataModelStorage()
        self.stream_output_model = KPI_DataModelStorage()

    def parse(self) -> Dict[str, Any]:
        """Parse configured KPI streams from input/output HDF5 files and forward to KPI factory."""
        results: Dict[str, Any] = {
            "sensor_id": self.config.sensor_id,
            "base_name": self.config.base_name,
            "processing_time": 0.0,
            "available_streams": [],
            "input_data": {},
            "output_data": {},
            "streams_processed": {},
            "html_report_path": "",
        }

        # Build output directory for HTML report path
        kpi_dir = os.path.join(
            self.config.output_dir,
            self.config.base_name,
            self.config.sensor_id,
            self.config.kpi_subdir,
        )
        os.makedirs(kpi_dir, exist_ok=True)
        html_report_path = os.path.join(
            kpi_dir, f"{self.config.base_name}_{self.config.sensor_id}_kpi_report.html"
        )
        results["html_report_path"] = html_report_path

        # Collect streams from KPI config
        streams: List[str] = []
        streams.extend(list(KPI_ALIGNMENT_CONFIG.keys())) if KPI_ALIGNMENT_CONFIG else None
        streams.extend(list(KPI_DETECTION_CONFIG.keys())) if KPI_DETECTION_CONFIG else None
        streams.extend(list(KPI_TRACKER_CONFIG.keys())) if KPI_TRACKER_CONFIG else None
        results["available_streams"] = streams

        sensor = self.config.sensor_id
        input_path = self.config.input_file_path
        output_path = self.config.output_file_path

        # Open HDF5 files (if present)
        hdf_in: Optional[h5py.File] = None
        hdf_out: Optional[h5py.File] = None

        if os.path.exists(input_path):
            hdf_in = h5py.File(input_path, "r")
        else:
            logger.error(f"Input HDF5 not found: {input_path}")

        if os.path.exists(output_path):
            hdf_out = h5py.File(output_path, "r")
        else:
            logger.warning(f"Output HDF5 not found: {output_path}")
        
        # Initialize stream-specific models dictionary
        self.stream_models = {}

        for stream_idx, stream in enumerate(streams):
            logger.info(f"Processing stream [{stream_idx}] {stream}")
            group_path = f"{sensor}/{stream}"

            # Skip if OD stream is missing
            if hdf_in is not None and group_path not in hdf_in:
                logger.warning(f"Skipping stream {stream} - OD data not found")
                continue

            # For each KPI stream, attempt to parse from input and output
            scan_index = None
            if hdf_in is not None and streams:
                data_group_in = hdf_in[group_path]
                header_path_in = next((v for v in self.header_variants if v in data_group_in), None)
                if header_path_in and f"{header_path_in}/scan_index" in data_group_in:
                    scan_index = data_group_in[f"{header_path_in}/scan_index"][()]
            
            # Skip if scan_index is None
            if scan_index is None:
                logger.warning(f"Skipping stream {stream} - No valid scan index found")
                continue
                
            # Initialize stream-specific models only if we have valid scan_index
            self.stream_models[stream] = {
                'input': KPI_DataModelStorage(),
                'output': KPI_DataModelStorage()
            }
            
            # Initialize models with scan index
            self.stream_models[stream]['input'].initialize(scan_index, sensor)
            self.stream_models[stream]['output'].initialize(scan_index, sensor)

            # Set stream-specific parent
            self.stream_models[stream]['input'].init_parent(stream)
            self.stream_models[stream]['output'].init_parent(stream)


            # # Expose per-stream storages on the wrapper for downstream access/debugging
            # safe_attr = stream.replace("/", "_").replace(" ", "_")
            # setattr(self, f"{safe_attr}_input_storage", stream_input_storage)
            # setattr(self, f"{safe_attr}_output_storage", stream_output_storage)
            # # Also store in a single dict for easy access by stream name
            # self.per_stream_storages[stream] = {
            #     "input": stream_input_storage,
            #     "output": stream_output_storage,
            # }

            # # Update stream processing status
            # results["streams_processed"][stream] = {
            #     "input_available": bool(scan_ind is not None and group_path in hdf_in),
            #     "output_available": bool(scan_ind is not None and group_path in hdf_out),
            #     "both_available": bool(scan_ind is not None and group_path in hdf_in and group_path in hdf_out),
            # }

            # # Set stream-specific parent with bracket notation for stream separation
            # self.stream_input_model.init_parent(stream)
            # self.stream_output_model.init_parent(stream)
            # # Also set parent on per-stream storages
            # stream_input_storage.init_parent(stream)
            # stream_output_storage.init_parent(stream)





            # Parse input and output files for this specific stream
            if hdf_in is not None and hdf_out is not None and group_path in hdf_in and group_path in hdf_out:
                # Parse input stream
                data_group_in = hdf_in[group_path]
                header_path_in = next((v for v in self.header_variants if v in data_group_in), None)
                if header_path_in:
                    try:
                        a = time.time()
                        # Parse into stream-specific model
                        self.stream_models[stream]['input'] = KPIHDFParser.parse(
                            data_group_in, self.stream_models[stream]['input'], self.header_variants
                        )
                        b = time.time()
                        logger.debug(f"Parsed input stream {stream} in {b - a:.3f}s")
                    except Exception as e:
                        logger.error(f"Error parsing input stream {stream}: {e}")
                
                # Parse output stream
                data_group_out = hdf_out[group_path]
                header_path_out = next((v for v in self.header_variants if v in data_group_out), None)
                if header_path_out:
                    try:
                        a = time.time()
                        # Parse into stream-specific model
                        self.stream_models[stream]['output'] = KPIHDFParser.parse(
                            data_group_out, self.stream_models[stream]['output'], self.header_variants
                        )
                        b = time.time()
                        logger.debug(f"Parsed output stream {stream} in {b - a:.3f}s")
                    except Exception as e:
                        logger.error(f"Error parsing output stream {stream}: {e}")




        # After parsing all streams, combine them into the global models
        self.stream_input_model = KPI_DataModelStorage()
        self.stream_output_model = KPI_DataModelStorage()
        
        for stream, models in self.stream_models.items():
            # Here you would need to implement a way to combine models
            # This is a simplified example - you'll need to adapt this to your specific needs
            if hasattr(self.stream_input_model, '_data_container') and hasattr(models['input'], '_data_container'):
                self.stream_input_model._data_container.update(models['input']._data_container)
            if hasattr(self.stream_output_model, '_data_container') and hasattr(models['output'], '_data_container'):
                self.stream_output_model._data_container.update(models['output']._data_container)


        KpiDataModel(
                    self.stream_models,
                    sensor,
                    )

        if hdf_in is not None:
            try:
                hdf_in.close()
            except Exception:
                pass
        if hdf_out is not None:
            try:
                hdf_out.close()
            except Exception:
                pass

        results["processing_time"] = time.time() - self.start_time_parsing
        logger.info(
            "KPI parsing complete for sensor=%s base=%s in %.3fs",
            self.config.sensor_id,
            self.config.base_name,
            results["processing_time"],
        )
        return results


@time_taken
def parse_for_kpi(
    sensor_id: str,
    input_file_path: str,
    output_dir: str,
    base_name: str,
    kpi_subdir: str,
    output_file_path: str,
) -> str:
    if not all([sensor_id, input_file_path, output_file_path, output_dir, base_name]):
        raise ValueError(
            "Missing required fields: sensor_id, input_file_path, output_file_path, output_dir, base_name"
        )

    wrapper = KPIHDFWrapper(
        KPIProcessingConfig(
            sensor_id=sensor_id,
            input_file_path=input_file_path,
            output_file_path=output_file_path,
            output_dir=output_dir,
            base_name=base_name,
            kpi_subdir=kpi_subdir,
        )
    )
    results = wrapper.parse()
    return results.get("html_report_path", "")
