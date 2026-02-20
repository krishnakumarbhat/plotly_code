import logging
from typing import Any, Dict
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

try:
    from KPI.c_business_layer.alignment_matching_kpi import process_alignment_kpi
except Exception:
    process_alignment_kpi = None  # type: ignore

try:
    from KPI.c_business_layer.detection_matching_kpi import main as process_detection_kpi
except Exception:
    process_detection_kpi = None  # type: ignore

try:
    from KPI.c_business_layer.tracker_matching_kpi import main as process_tracker_kpi
except Exception:
    process_tracker_kpi = None  # type: ignore


class KPIFactory:
    """Routes parsed stream data to the appropriate KPI processor."""

    @staticmethod
    def process_stream(input_data: DataModelStorage,
                       output_data: DataModelStorage,
                       sensor: str,
                       stream: str) -> Dict[str, Any]:
        stream_upper = (stream or "").upper()

        if "ALIGNMENT_STREAM" in stream_upper:
            return KPIFactory._run_alignment(input_data, output_data)

        if ("DETECTION" in stream_upper) or ("RDD" in stream_upper) or ("AF" in stream_upper) or ("DET" in stream_upper):
            return KPIFactory._run_detection(input_data, output_data)

        if ("TRACKER" in stream_upper) or ("ROT_OBJECT" in stream_upper) or ("ROT_OBJ" in stream_upper):
            return KPIFactory._run_tracker(input_data, output_data)

        logging.info(f"KPI: No processor mapped for stream {stream}")
        return {}

    @staticmethod
    def _run_alignment(input_data: DataModelStorage, output_data: DataModelStorage) -> Dict[str, Any]:
        if process_alignment_kpi is None:
            logging.warning("alignment_matching_kpi not available; skipping")
            return {}

        data_dict = {
            'vacs_boresight_az_nominal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_az_nominal'),
            'vacs_boresight_az_estimated': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_az_estimated'),
            'vacs_boresight_az_kf_internal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_az_kf_internal'),
            'vacs_boresight_el_nominal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_el_nominal'),
            'vacs_boresight_el_estimated': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_el_estimated'),
            'vacs_boresight_el_kf_internal': DataModelStorage.get_data(input_data, output_data, 'vacs_boresight_el_kf_internal'),
        }
        status = process_alignment_kpi(data_dict)
        return {"status": bool(status)}

    @staticmethod
    def _run_detection(input_data: DataModelStorage, output_data: DataModelStorage) -> Dict[str, Any]:
        if process_detection_kpi is None:
            logging.info("detection_matching_kpi not implemented; skipping")
            return {}
        try:
            # detection module expects file paths in legacy main(); skipping wiring for now
            return {"status": False}
        except Exception:
            logging.exception("Detection KPI failed")
            return {"status": False}

    @staticmethod
    def _run_tracker(input_data: DataModelStorage, output_data: DataModelStorage) -> Dict[str, Any]:
        if process_tracker_kpi is None:
            logging.info("tracker_matching_kpi not implemented; skipping")
            return {}
        try:
            # tracker module expects file paths in legacy main(); skipping wiring for now
            return {"status": False}
        except Exception:
            logging.exception("Tracker KPI failed")
            return {"status": False}
