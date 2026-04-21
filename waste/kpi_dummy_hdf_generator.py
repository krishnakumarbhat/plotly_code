import os
from typing import Tuple

import h5py
import numpy as np

from KPI.b_data_storage.kpi_config_storage import (
    KPI_ALIGNMENT_CONFIG,
    KPI_DETECTION_CONFIG,
    KPI_TRACKER_CONFIG,
)
from KPI.a_persistence_layer.hdf_wrapper import parse_for_kpi


SENSOR_ID = "SENSOR1"
NUM_SCANS = 10
NUM_DETS = 5

# Default directory where HDFs will be written.
# Adjust this if you want them somewhere else.
BASE_DIR = r"C:/Users/ouymc2/Desktop/hdf_db"

PAIR1_INPUT = os.path.join(BASE_DIR, "kpi_dummy_pair1_input.h5")
PAIR1_OUTPUT = os.path.join(BASE_DIR, "kpi_dummy_pair1_output.h5")
PAIR2_INPUT = os.path.join(BASE_DIR, "kpi_dummy_pair2_input.h5")
PAIR2_OUTPUT = os.path.join(BASE_DIR, "kpi_dummy_pair2_output.h5")

# Separate configurations for detection stream rdd_idx
RDD_IDX_CONFIG_INPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        # Customize input (vehicle) rdd_idx patterns
        0: [1, 2, 3, 4, 0],  # First scan has shifted pattern
        1: [0, 2, 1, 3, 4],  # Third scan swaps positions
    }
}

RDD_IDX_CONFIG_OUTPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        # Customize output (simulation) rdd_idx patterns
        0: [1, 1, 3, 4, 0],  # First scan has shifted pattern
        1: [1, 2, 0, 3, 3],  # Second scan has mixed pattern
        # 7: [4, 3, 2, 1, 0],  # Eighth scan is reversed
    }
}

# Separate configurations for RDD_STREAM rdd1_rindx and rdd1_dindx
RDD1_RINDX_CONFIG_INPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        # Customize input rdd1_rindx patterns
        0: [1, 2, 3, 4, 0],  # First scan has shifted pattern
        1: [0, 2, 1, 3, 4],  # Third scan swaps positions
    }
}

RDD1_RINDX_CONFIG_OUTPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        0: [1, 3, 3, 4, 0],  # First scan has shifted pattern
        1: [0, 2, 1, 3, 4],  # Third scan swaps positions
        # Customize output rdd1_rindx patterns
    }
}

RDD1_DINDX_CONFIG_INPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        # Customize input rdd1_dindx patterns
        0: [1, 2, 3, 4, 0],  # First scan has shifted pattern
        1: [0, 2, 1, 3, 4],  # Third scan swaps positions
    }
}

RDD1_DINDX_CONFIG_OUTPUT = {
    "default_pattern": list(range(NUM_DETS)),  # [0, 1, 2, 3, 4]
    "custom_rows": {
        # Customize output rdd1_dindx patterns
        0: [1, 4, 3, 2, 0],  # First scan has shifted pattern
        1: [0, 2, 1, 3, 4],  # Third scan swaps positions
    }
}


def _create_common_header(group: h5py.Group, num_scans: int, add_extra_scan: bool = False) -> np.ndarray:
    """Create Stream_Hdr/scan_index for a stream and return the scan_index array.
    
    Args:
        group: HDF5 group to add header to
        num_scans: Base number of scans
        add_extra_scan: If True, add one extra scan index (for testing mismatches)
    """
    hdr = group.create_group("Stream_Hdr")
    if add_extra_scan:
        # Add an extra scan index (e.g., 0-10 instead of 0-9)
        scan_index = np.arange(num_scans + 3, dtype=np.int32)
    else:
        scan_index = np.arange(num_scans, dtype=np.int32)
    hdr.create_dataset("scan_index", data=scan_index)
    return scan_index


def _populate_alignment_stream(stream_group: h5py.Group, value_offset: float) -> None:
    # Alignment signals are all 1D per-scan.
    stream_name = os.path.basename(stream_group.name)
    cfg = KPI_ALIGNMENT_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    for i, signal_name in enumerate(cfg.keys()):
        values = np.linspace(-1.0, 1.0, num_scans, dtype=np.float32) + value_offset + i
        stream_group.create_dataset(signal_name, data=values)


def _populate_detection_stream(stream_group: h5py.Group, value_offset: float, is_input: bool = True) -> None:
    stream_name = os.path.basename(stream_group.name)
    det_cfg = KPI_DETECTION_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    # 1D per-scan arrays
    if "num_af_det" in det_cfg:
        num_af_det = np.full(num_scans, NUM_DETS, dtype=np.int32)
        stream_group.create_dataset("num_af_det", data=num_af_det)

    if "scan_index" in det_cfg:
        scan_index = stream_group["Stream_Hdr/scan_index"][()]
        stream_group.create_dataset("scan_index", data=scan_index)

    # 2D [num_scans x NUM_DETS] arrays for detection-wise data
    def mk2d(base: float) -> np.ndarray:
        grid = np.arange(num_scans * NUM_DETS, dtype=np.float32).reshape(num_scans, NUM_DETS)
        return grid + base + value_offset

    for idx, signal_name in enumerate([
        "rdd_idx",
        "ran",
        "vel",
        "theta",
        "phi",
        "f_single_target",
        "f_superres_target",
        "f_bistatic",
    ]):
        if signal_name in det_cfg:
            # Special handling for rdd_idx: use separate config for input/output
            if signal_name == "rdd_idx":
                config = RDD_IDX_CONFIG_INPUT if is_input else RDD_IDX_CONFIG_OUTPUT
                data = _generate_rdd_idx_matrix(num_scans, NUM_DETS, config)
            else:
                data = mk2d(base=float(idx))
            stream_group.create_dataset(signal_name, data=data)


def _populate_rdd_stream(stream_group: h5py.Group, value_offset: float, is_input: bool = True) -> None:
    stream_name = os.path.basename(stream_group.name)
    rdd_cfg = KPI_DETECTION_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    if "rdd1_num_detect" in rdd_cfg:
        num_det = np.full(num_scans, NUM_DETS, dtype=np.int32)
        stream_group.create_dataset("rdd1_num_detect", data=num_det)

    if "scan_index" in rdd_cfg:
        scan_index = stream_group["Stream_Hdr/scan_index"][()]
        stream_group.create_dataset("scan_index", data=scan_index)

    def mk2d(base: float) -> np.ndarray:
        grid = np.arange(num_scans * NUM_DETS, dtype=np.float32).reshape(num_scans, NUM_DETS)
        return grid + base + value_offset

    for idx, signal_name in enumerate([
        "rdd1_rindx",
        "rdd1_dindx",
        "rdd2_range",
        "rdd2_range_rate",
    ]):
        if signal_name in rdd_cfg:
            # Special handling for rdd1_rindx and rdd1_dindx: use separate configs
            if signal_name == "rdd1_rindx":
                config = RDD1_RINDX_CONFIG_INPUT if is_input else RDD1_RINDX_CONFIG_OUTPUT
                data = _generate_rdd_idx_matrix(num_scans, NUM_DETS, config)
            elif signal_name == "rdd1_dindx":
                config = RDD1_DINDX_CONFIG_INPUT if is_input else RDD1_DINDX_CONFIG_OUTPUT
                data = _generate_rdd_idx_matrix(num_scans, NUM_DETS, config)
            else:
                data = mk2d(base=float(idx))
            stream_group.create_dataset(signal_name, data=data)
 
def _generate_rdd_idx_matrix(num_scans: int, num_dets: int, config: dict) -> np.ndarray:
    """
    Generate rdd_idx matrix with customizable patterns per row.
    
    Args:
        num_scans: Number of scan indices (rows)
        num_dets: Number of detections per scan (columns)
        config: Dictionary with 'default_pattern' and 'custom_rows'
    
    Returns:
        np.ndarray: Matrix of shape (num_scans, num_dets)
    """
    # Initialize with default pattern for all rows
    default_pattern = config.get("default_pattern", list(range(num_dets)))
    data = np.tile(default_pattern, (num_scans, 1)).astype(np.float32)
    
    # Apply custom patterns for specific rows
    custom_rows = config.get("custom_rows", {})
    for row_idx, custom_pattern in custom_rows.items():
        if 0 <= row_idx < num_scans:
            if len(custom_pattern) == num_dets:
                data[row_idx, :] = custom_pattern
            else:
                print(f"Warning: Custom pattern for row {row_idx} has length {len(custom_pattern)}, expected {num_dets}. Skipping.")
        else:
            print(f"Warning: Row index {row_idx} out of range [0, {num_scans-1}]. Skipping.")
    
    return data


def _populate_cdc_stream(stream_group: h5py.Group, value_offset: float) -> None:
    stream_name = os.path.basename(stream_group.name)
    cdc_cfg = KPI_DETECTION_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    if "num_cdc_records" in cdc_cfg:
        data = (np.arange(num_scans, dtype=np.int32) + 1).astype(np.float32) + value_offset
        stream_group.create_dataset("num_cdc_records", data=data)


def _populate_vse_stream(stream_group: h5py.Group, value_offset: float) -> None:
    stream_name = os.path.basename(stream_group.name)
    vse_cfg = KPI_DETECTION_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    if "veh_speed" in vse_cfg:
        data = np.linspace(0.0, 30.0, num_scans, dtype=np.float32) + value_offset
        stream_group.create_dataset("veh_speed", data=data)


def _populate_tracker_stream(stream_group: h5py.Group, value_offset: float) -> None:
    stream_name = os.path.basename(stream_group.name)
    trk_cfg = KPI_TRACKER_CONFIG.get(stream_name, {})
    # Use actual number of scans from header (includes extra scans if added)
    num_scans = stream_group["Stream_Hdr/scan_index"].shape[0]

    if not trk_cfg:
        return

    # Simple 1D per-scan values for all tracker fields
    for idx, signal_name in enumerate(trk_cfg.keys()):
        base = float(idx)
        if signal_name == "trkID":
            data = np.arange(num_scans, dtype=np.int32)
        else:
            data = np.linspace(0.0, 10.0, num_scans, dtype=np.float32) + base + value_offset
        stream_group.create_dataset(signal_name, data=data)


def create_kpi_dummy_hdf(file_path: str, value_offset: float, is_input: bool = True, add_extra_scan: bool = False) -> None:
    """Create a single dummy KPI HDF file at the given path.

    value_offset controls how different this file's numeric values are from others.
    is_input: True for input (vehicle) files, False for output (simulation) files
    add_extra_scan: If True, add an extra scan index (for testing mismatches)
    """
    os.makedirs(os.path.dirname(file_path), exist_ok=True)

    with h5py.File(file_path, "w") as h5:
        # Alignment stream
        if "DYNAMIC_ALIGNMENT_STREAM" in KPI_ALIGNMENT_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/DYNAMIC_ALIGNMENT_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_alignment_stream(grp, value_offset)

        # Detection-related streams
        if "DETECTION_STREAM" in KPI_DETECTION_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/DETECTION_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_detection_stream(grp, value_offset, is_input)

        if "RDD_STREAM" in KPI_DETECTION_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/RDD_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_rdd_stream(grp, value_offset, is_input)

        if "CDC_STREAM" in KPI_DETECTION_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/CDC_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_cdc_stream(grp, value_offset)

        if "VSE_STREAM" in KPI_DETECTION_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/VSE_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_vse_stream(grp, value_offset)

        # Tracker stream
        if "TRACKER_STREAM" in KPI_TRACKER_CONFIG:
            grp = h5.create_group(f"{SENSOR_ID}/TRACKER_STREAM")
            _create_common_header(grp, NUM_SCANS, add_extra_scan)
            _populate_tracker_stream(grp, value_offset)


def generate_all_pairs() -> Tuple[str, str, str, str]:
    """Generate 4 HDFs: 2 input and 2 output.

    Pair 1: input/output have identical data (100% match).
    Pair 2: output uses a different value offset AND an extra scan index to create mismatches.
    """
    # Pair 1 - perfect match
    create_kpi_dummy_hdf(PAIR1_INPUT, value_offset=0.0, is_input=True, add_extra_scan=False)
    create_kpi_dummy_hdf(PAIR1_OUTPUT, value_offset=0.0, is_input=False, add_extra_scan=False)

    # Pair 2 - introduce differences via non-zero offset AND extra scan in output
    create_kpi_dummy_hdf(PAIR2_INPUT, value_offset=0.0, is_input=True, add_extra_scan=False)
    create_kpi_dummy_hdf(PAIR2_OUTPUT, value_offset=1.5, is_input=False, add_extra_scan=True)

    return PAIR1_INPUT, PAIR1_OUTPUT, PAIR2_INPUT, PAIR2_OUTPUT


def self_test_parse(pair_input: str, pair_output: str) -> None:
    """Run your existing KPI parser on a given input/output pair to ensure
    the generated HDF structure is compatible.
    """
    base_name = os.path.splitext(os.path.basename(pair_input))[0]
    out_dir = BASE_DIR
    kpi_subdir = "kpi"

    print(f"Running KPI parse_for_kpi on:\n  input={pair_input}\n  output={pair_output}")

    try:
        html_path = parse_for_kpi(
            sensor_id=SENSOR_ID,
            input_file_path=pair_input,
            output_dir=out_dir,
            base_name=base_name,
            kpi_subdir=kpi_subdir,
            output_file_path=pair_output,
        )
        print(f"KPI parsing completed. HTML report (if generated): {html_path}")
    except Exception as exc:
        print(f"KPI parsing raised an exception: {exc}")


if __name__ == "__main__":
    print("Generating dummy KPI HDF pairs in:", BASE_DIR)
    p1_in, p1_out, p2_in, p2_out = generate_all_pairs()
    print("Created files:")
    print("  Pair1 input:", p1_in)
    print("  Pair1 output:", p1_out)
    print("  Pair2 input:", p2_in)
    print("  Pair2 output:", p2_out)

    # Optional quick test: run the KPI parser on the 100% match pair
    self_test_parse(p1_in, p1_out)
