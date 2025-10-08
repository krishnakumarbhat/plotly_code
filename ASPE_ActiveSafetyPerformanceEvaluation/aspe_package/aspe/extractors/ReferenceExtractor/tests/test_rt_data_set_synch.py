# coding utf-8
"""
Main testing file for Rt DVL Extraction smoke tests
"""
from datetime import datetime

import numpy as np
import pandas as pd

from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtDataSetSynch import RtDataSetSynch
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects


def test_general_data_output_check_with_single_target():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.53, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.54, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.55, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, 1000.56, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.57, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = RtObjects()
    expected_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    expected_slave_data = RtObjects()
    expected_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)


def test_general_data_output_check_with_two_targets():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.53, 0, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.54, 0, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.55, 0, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, 1000.56, 0, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.57, 0, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [1, 1000.63, 1, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [2, 1000.64, 1, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [3, 1000.65, 1, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [4, 1000.66, 1, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [5, 1000.67, 1, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 22.34]
        ],
        columns=['scan_index', 'timestamp', 'unique_obj_id', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = RtObjects()
    expected_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    expected_slave_data = RtObjects()
    expected_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, 0, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.44, 0, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.45, 0, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [1, 1000.43, 1, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [2, 1000.44, 1, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 22.34],
            [3, 1000.45, 1, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 22.34]
        ],
        columns=['scan_index', 'timestamp', 'unique_obj_id', 'utc_timestamp', 'center_x']
    )

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)


def test_output_check_with_nan_record_in_si_and_ts():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.53, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.54, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.55, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [np.nan, 1000.56, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, np.nan, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, 1000.58, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [7, 1000.59, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [8, 1000.60, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = RtObjects()
    expected_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    expected_slave_data = RtObjects()
    expected_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)


def test_output_check_with_target_timestamp_as_nan():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, np.nan, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, np.nan, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, np.nan, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, np.nan, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, np.nan, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, np.nan, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [7, np.nan, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [8, np.nan, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = RtObjects()
    expected_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    expected_slave_data = RtObjects()
    expected_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)


def test_output_check_with_target_utc_as_nan():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.53, np.nan, 12.34],
            [2, 1000.54, np.nan, 12.34],
            [3, 1000.55, np.nan, 12.34],
            [4, 1000.56, np.nan, 12.34],
            [5, 1000.57, np.nan, 12.34],
            [6, 1000.58, np.nan, 12.34],
            [7, 1000.59, np.nan, 12.34],
            [8, 1000.60, np.nan, 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = input_master_data

    expected_slave_data = input_slave_data

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)


def test_output_check_with_timestamp_nan_record_in_host():
    # Setup
    input_master_data = RtObjects()
    input_master_data.max_possible_obj_count = 1
    input_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [4,  np.nan, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 112.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    input_slave_data = RtObjects()
    input_slave_data.max_possible_obj_count = 4
    input_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.53, datetime.strptime('2019-07-22 00:15:00.13', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.54, datetime.strptime('2019-07-22 00:15:00.14', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.55, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [4, 1000.56, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.57, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, 1000.58, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [7, 1000.59, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [8, 1000.60, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34]
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Expected
    expected_master_data = RtObjects()
    expected_master_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 112.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    expected_slave_data = RtObjects()
    expected_slave_data.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 12.34],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'center_x']
    )

    # Results
    rt_synch = RtDataSetSynch(f_clear_nans=True, f_crop_master_slave=True)
    result_master_data, result_slave_data = rt_synch.synch(master=input_master_data, slave=input_slave_data)

    # Verify
    pd.testing.assert_frame_equal(expected_master_data.signals, result_master_data.signals, check_like=True,
                                  check_dtype=False)
    pd.testing.assert_frame_equal(expected_slave_data.signals, result_slave_data.signals, check_like=True,
                                  check_dtype=False)
