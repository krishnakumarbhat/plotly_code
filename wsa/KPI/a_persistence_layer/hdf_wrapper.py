import h5py
import os
import logging
import time
from typing import Dict, Any, Optional, List
from dataclasses import dataclass

from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
from InteractivePlot.d_business_layer.utils import time_taken
from KPI.b_data_storage.kpi_config_storage import KPI_ALIGNMENT_CONFIG, KPI_DETECTION_CONFIG, KPI_TRACKER_CONFIG
from KPI.a_persistence_layer.kpi_hdf_parser import KPIHDFParser

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
        try:
            if os.path.exists(input_path):
                hdf_in = h5py.File(input_path, "r")
            else:
                logger.error(f"Input HDF5 not found: {input_path}")

            if os.path.exists(output_path):
                hdf_out = h5py.File(output_path, "r")
            else:
                logger.warning(f"Output HDF5 not found: {output_path}")

            # For each KPI stream, attempt to parse from input and output
            for stream in streams:
                results["streams_processed"][stream] = {
                    "input_available": False,
                    "output_available": False,
                    "both_available": False,
                }

                # Prepare data models
                input_model = DataModelStorage()
                output_model = DataModelStorage()
                input_model.init_parent(stream)
                output_model.init_parent(stream)

                # Parse input file stream
                if hdf_in is not None:
                    group_path = f"{sensor}/{stream}"
                    if group_path in hdf_in:
                        data_group = hdf_in[group_path]
                        header_path = next((v for v in self.header_variants if v in data_group), None)
                        if header_path:
                            try:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                input_model.initialize(scan_index, sensor, stream)
                                a = time.time()
                                # Parse using KPIHDFParser
                                input_model = KPIHDFParser.parse(
                                    data_group, input_model, scan_index, self.header_variants
                                )
                                b = time.time()
                                logger.debug(f"Parsed input stream {stream} in {b - a:.3f}s")
                                results["input_data"][stream] = input_model
                                results["streams_processed"][stream]["input_available"] = True
                            except Exception as e:
                                logger.error(f"Error parsing input stream {stream}: {e}")
                        else:
                            logger.debug(f"No header found for input stream {stream} at {group_path}")
                    else:
                        logger.debug(f"Input stream group not found: {group_path}")

                # Parse output file stream
                if hdf_out is not None:
                    group_path = f"{sensor}/{stream}"
                    if group_path in hdf_out:
                        data_group = hdf_out[group_path]
                        header_path = next((v for v in self.header_variants if v in data_group), None)
                        if header_path:
                            try:
                                scan_index = data_group[f"{header_path}/scan_index"][()]
                                output_model.initialize(scan_index, sensor, stream)
                                a = time.time()
                                # Parse using KPIHDFParser
                                output_model = KPIHDFParser.parse(
                                    data_group, output_model, scan_index, self.header_variants
                                )
                                b = time.time()
                                logger.debug(f"Parsed output stream {stream} in {b - a:.3f}s")
                                results["output_data"][stream] = output_model
                                results["streams_processed"][stream]["output_available"] = True
                            except Exception as e:
                                logger.error(f"Error parsing output stream {stream}: {e}")
                        else:
                            logger.debug(f"No header found for output stream {stream} at {group_path}")
                    else:
                        logger.debug(f"Output stream group not found: {group_path}")

                # Mark both available flag
                sp = results["streams_processed"][stream]
                sp["both_available"] = bool(sp["input_available"] and sp["output_available"])

                # Forward to KPI factory if both available
                if sp["both_available"]:
                    try:
                        from KPI.c_business_layer.kpi_factory import KpiDataModel
                        _ = KpiDataModel(
                            results["input_data"][stream],
                            results["output_data"][stream],
                            sensor,
                            stream,
                        )
                    except Exception as e:
                        # Keep parsing results even if KPI visualization is not ready
                        logger.warning(f"KPI factory processing failed for stream {stream}: {e}")

        finally:
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
