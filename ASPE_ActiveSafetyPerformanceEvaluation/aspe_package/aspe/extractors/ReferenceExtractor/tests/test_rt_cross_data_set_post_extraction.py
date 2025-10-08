# coding utf-8
"""
Main testing file for Rt cross data set post extraction
"""
from copy import deepcopy

import pytest

import numpy as np
from numpy import nan, deg2rad, sqrt, sin, cos
import pandas as pd
from datetime import datetime, timedelta
import itertools
from logging import warn

from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtHost import RtHost
from aspe.extractors.Interfaces import IDataSet
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects
from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtCrossDataSetPostExtraction import \
    RtCrossDataSetPostExtraction


def make_sample_si_ts_utc_dataframe(
    length: int,
    timestamp_start: float = 1000.43,
    timestamp_step: float = 0.01,
    utc_timestamp_start: datetime = datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f')
) -> pd.DataFrame:
    """
    :param length: number of rows
    :param timestamp_start: first timestamp
    :param timestamp_step: difference between subsequent timestamps
    :param utc_timestamp_start: first utc_timetamp
    :return: pd.DataFrame

    >>> make_sample_si_ts_utc_dataframe(3)
       scan_index  timestamp           utc_timestamp
    0           1    1000.43 2019-07-22 00:15:00.150
    1           2    1000.44 2019-07-22 00:15:00.160
    2           3    1000.45 2019-07-22 00:15:00.170

    >>> make_sample_si_ts_utc_dataframe(0)
    Empty DataFrame
    Columns: [scan_index, timestamp, utc_timestamp]
    Index: []

    """
    if length < 0:
        warn('Negative length')

    return pd.DataFrame(
        columns=(
            'scan_index',
            'timestamp',
            'utc_timestamp'
        ),
        data=(
            (
                i+1,
                timestamp_start + i * timestamp_step,
                utc_timestamp_start + i * timedelta(seconds=timestamp_step)
            )
            for i in range(length)
        )
    )


def glue_si_ts_utc_to_dataframe(signals_df: pd.DataFrame, **kwargs) -> pd.DataFrame:
    """
    :param signals_df: dataframe with signal values but without scan_index, timestamp and utc_timestamp
    :param kwargs: optional kwargs for make_sample_si_ts_utc_dataframe function
    :return: dataframe with sample scan_index, timestamp and utc_timestamp

    >>> signals_df = pd.DataFrame(columns=('center_x', 'center_y'), data=((1,11),(2,22),(3,33)))
    >>> signals_df
       center_x  center_y
    0         1        11
    1         2        22
    2         3        33
    >>> glue_si_ts_utc_to_dataframe(signals_df)
       scan_index  timestamp           utc_timestamp  center_x  center_y
    0           1    1000.43 2019-07-22 00:15:00.150         1        11
    1           2    1000.44 2019-07-22 00:15:00.160         2        22
    2           3    1000.45 2019-07-22 00:15:00.170         3        33
    """

    length = len(signals_df)
    si_ts_utc_df = make_sample_si_ts_utc_dataframe(length, **kwargs)
    return pd.concat((si_ts_utc_df, signals_df), axis=1)  # horizontally glue two dataframes


def fill_dataset_signals(dataset: IDataSet, signals_df: pd.DataFrame):
    """
    :param dataset: IDataSet object, e.g. RtObjects()
    :param signals_df: dataframe with signal values (without scan_index, timestamp, utc_timestamp)
    :return: dataset with filled signals
    """
    append_df = glue_si_ts_utc_to_dataframe(signals_df)
    dataset.signals = pd.concat((dataset.signals, append_df), axis=0, sort=False)  # vertically append to IDataSet.signals
    return dataset


@pytest.mark.parametrize("data", (
    # casual values
    (
        (0, 0.1, 0.1),
        (0.01, 0.2, 0.19),
        (0.02, 0.3, 0.28),
        (0.03, 0.4, 0.37),
        (0.04, 0.5, 0.46),
    ),

    # NaNs
    (
        (0, 0, 0),
        (nan, 0, nan),
        (0, nan, nan),
        (nan, 123, nan),
        (123, nan, nan),
        (nan, nan, nan),
    ),

    # angle normalization
    deg2rad((
        (1, 3, 2),
        (100, -100, 160),
        (0, 180, 180),
        (0, -180, -180),
        (90, -90, -180),
        (-90, 90, 180),
        (-90, 91, -179),
        (91, -90, 179),
        (180, -180, 0),
        (-180, 180, 0),
    )),
))
def test_extract_objects_bounding_box_orientation(data):
    test_df = pd.DataFrame(data, columns=(
        ('input_host', 'wcs_bounding_box_orientation'),
        ('input_objects', 'wcs_bounding_box_orientation'),
        ('expected_objects', 'bounding_box_orientation'),
    ))

    # Setup
    input_host = fill_dataset_signals(RtHost(), test_df['input_host'])
    input_objects = fill_dataset_signals(RtObjects(), test_df['input_objects'])

    # Expected
    expected_objects_signals = test_df['expected_objects']

    # Results
    rtcross = RtCrossDataSetPostExtraction()
    rtcross._host = input_host
    rtcross._objects = input_objects
    rtcross._host_ts_index = rtcross._host.signals.set_index('utc_timestamp')
    rtcross._host_suffix = '_host'

    rtcross._extract_objects_bounding_box_orientation()

    result_objects_signals = rtcross._objects.signals
    expected_columns_to_compare = test_df['expected_objects'].columns

    # Verify
    pd.testing.assert_frame_equal(expected_objects_signals,
                                  result_objects_signals[expected_columns_to_compare],
                                  check_dtype=False)


@pytest.mark.parametrize("data", (
    # different positions
    (
        (0, 0, 11, 22, 0.5, 0.5, deg2rad(33), 0, 0),
        (77, 88, 11, 22, 0.5, 0.5, deg2rad(33), 77, 88),
    ),

    # different bbox refpoints
    (
        (0, 0, 10, 10, 0.5, 0.5, deg2rad(0), 0, 0),
        (0, 0, 10, 10, 1, 0.5, deg2rad(0), -5, 0),
        (0, 0, 10, 10, 1, 1, deg2rad(0), -5, -5),
        (0, 0, 10, 10, 0.5, 1, deg2rad(0), 0, -5),
        (0, 0, 10, 10, 0, 1, deg2rad(0), 5, -5),
        (0, 0, 10, 10, 0, 0.5, deg2rad(0), 5, 0),
        (0, 0, 10, 10, 0, 0, deg2rad(0), 5, 5),
        (0, 0, 10, 10, 0.5, 0, deg2rad(0), 0, 5),
        (0, 0, 10, 10, 1, 0, deg2rad(0), -5, 5),
        (0, 0, 10, 10, 1, 0, deg2rad(0), -5, 5),
    ),

    # different angles
    (
        (0, 0, 10, 10, 1, 1, deg2rad(0), -5, -5),
        (0, 0, 10, 10, 1, 1, deg2rad(90), 5, -5),
        (0, 0, 10, 10, 1, 1, deg2rad(180), 5, 5),
        (0, 0, 10, 10, 1, 1, deg2rad(270), -5, 5),
        (0, 0, 10, 10, 1, 1, deg2rad(45), 0, -5*sqrt(2)),
        (0, 0, 10, 10, 1, 1, deg2rad(135), 5*sqrt(2), 0),
        (0, 0, 10, 10, 1, 1, deg2rad(225), 0, 5*sqrt(2)),
        (0, 0, 10, 10, 1, 1, deg2rad(315), -5*sqrt(2), 0),
        (0, 0, 10, 10, 1, 1, deg2rad(30), -5*sqrt(2)*sin(deg2rad(45-30)), -5*sqrt(2)*cos(deg2rad(45-30))),
        (0, 0, 10, 10, 1, 1, deg2rad(60), -5*sqrt(2)*sin(deg2rad(45-60)), -5*sqrt(2)*cos(deg2rad(45-60))),
    ),

    # all above combined
    (
        (
            pos_x,
            pos_y,
            bbox_dim_x,
            bbox_dim_y,
            bbox_refpoint_long,
            bbox_refpoint_lat,
            bbox_orientation,
            pos_x + (np.complex((0.5-bbox_refpoint_lat)*bbox_dim_y, (0.5-bbox_refpoint_long)*bbox_dim_x) * np.exp(1j * -bbox_orientation)).imag,
            pos_y + (np.complex((0.5-bbox_refpoint_lat)*bbox_dim_y, (0.5-bbox_refpoint_long)*bbox_dim_x) * np.exp(1j * -bbox_orientation)).real

            # Complex numbers here are representations of 2D vectors.
            # First coordinate (real component) describes horizontal component (along y axis, also called lateral).
            # Second coordinate (imaginary component) describes vertical component (along x axis, also called longitudinal).
            #
            # p - vector of reference point position (from (0, 0) to reference point).
            #       p = [pos_y, pos_x]
            #
            # b - vector from reference point to object center (0.5, 0.5), converted from bbox ratio to real-world units
            #       b = [b_y, b_x]
            #
            # Horizontal component of b is the y-axis distance between reference point and center, multiplied by bbox width
            #       b_y = (0.5 - bbox_refpoint_lat) * bbox_width
            # Vertical component of b is the x-axis distance between reference point and center, multiplied by bbox length
            #       b_x = (0.5 - bbox_refpoint_long) * bbox_length
            #
            # Let alpha be the bounding box orientation in radians, from -pi to +pi
            #
            # Let c be the output vector of object's center position (from (0,0) to object center)
            #       c = [center_y, center_x]
            #
            # We can calculate object's center by taking its reference point position vector
            # and adding vector b rotated clockwise by angle alpha.
            #       c = p + rotate_clockwise(b, alpha)
            # Since e^(i*phi) represents counter-clockwise rotation by angle phi, we substitute phi = -alpha.
            #       c = p + b * e^(i * -alpha)
            # Finally, center_y = c.real and center_x = c.imag.
        )
        for pos_x, pos_y, bbox_dim_x, bbox_dim_y, bbox_refpoint_long, bbox_refpoint_lat, bbox_orientation in itertools.product(
            (0, 12),
            (0, 34),
            (10, 20),
            (15, 25),
            np.linspace(0, 1, 4),
            np.linspace(0, 1, 4),
            np.deg2rad(np.linspace(-180, 180, 10))
        )
    ),

    # NaNs
    (
        (
            *inputs,
            0 if all(~np.isnan((inputs[0], *inputs[2:]))) else nan,
            0 if all(~np.isnan(inputs[1:])) else nan
        )
        for inputs in itertools.product((0, nan), repeat=7)
    )
))
def test_extract_objects_center(data):
    test_df = pd.DataFrame(data, columns=(
        ('input_objects', 'position_x'),
        ('input_objects', 'position_y'),
        ('input_objects', 'bounding_box_dimensions_x'),
        ('input_objects', 'bounding_box_dimensions_y'),
        ('input_objects', 'bounding_box_refpoint_long_offset_ratio'),
        ('input_objects', 'bounding_box_refpoint_lat_offset_ratio'),
        ('input_objects', 'bounding_box_orientation'),
        ('expected_objects', 'center_x'),
        ('expected_objects', 'center_y'),
    ))

    # Setup
    input_host = RtHost()
    input_objects = fill_dataset_signals(RtObjects(), test_df['input_objects'])

    # Expected
    expected_objects_signals = test_df['expected_objects']

    # Results
    rtcross = RtCrossDataSetPostExtraction()
    rtcross._host = input_host
    rtcross._objects = input_objects
    rtcross._host_ts_index = rtcross._host.signals.set_index('utc_timestamp')
    rtcross._host_suffix = '_host'

    rtcross._extract_objects_center()

    result_objects_signals = rtcross._objects.signals
    expected_columns_to_compare = test_df['expected_objects'].columns

    # Verify
    pd.testing.assert_frame_equal(expected_objects_signals,
                                  result_objects_signals[expected_columns_to_compare],
                                  check_dtype=False)


@pytest.mark.parametrize("data", (
    # casual values
    (
        (0, 0, 0, 0, 0, 0, 0, 0, 0),
        (10, 20, 0.01, 30, 40, 5, 12, 10+5+40*0.01, 20+12-30*0.01),
        (-15, 27, 0.07, 12, 23, 6, -8, -15+6+23*0.07, 27+(-8)-12*0.07),
    ),
))
def test_extract_objects_velocity_otg(data):
    test_df = pd.DataFrame(data, columns=(
        ('input_host', 'velocity_otg_y'),
        ('input_host', 'velocity_otg_x'),
        ('input_host', 'yaw_rate'),
        ('input_objects', 'position_y'),
        ('input_objects', 'position_x'),
        ('input_objects', 'rel_velocity_y'),
        ('input_objects', 'rel_velocity_x'),
        ('expected_objects', 'velocity_otg_y'),
        ('expected_objects', 'velocity_otg_x'),
    ))

    # Setup
    input_host = fill_dataset_signals(RtHost(), test_df['input_host'])
    input_objects = fill_dataset_signals(RtObjects(), test_df['input_objects'])

    # Expected
    expected_objects_signals = test_df['expected_objects']

    # Results
    rtcross = RtCrossDataSetPostExtraction()
    rtcross._host = input_host
    rtcross._objects = input_objects
    rtcross._host_ts_index = rtcross._host.signals.set_index('utc_timestamp')
    rtcross._host_suffix = '_host'

    rtcross._extract_objects_velocity_otg()

    result_objects_signals = rtcross._objects.signals
    expected_columns_to_compare = test_df['expected_objects'].columns

    # Verify
    pd.testing.assert_frame_equal(expected_objects_signals,
                                  result_objects_signals[expected_columns_to_compare],
                                  check_dtype=False)


@pytest.mark.parametrize("data", (
    # casual values
    (
        (0, 0, 0),
        (3, 4, 5),
        (-5, 12, 13),
        (6, -8, 10),
        (-7, -24, 25),
        (123.4, 567.8, sqrt(123.4**2+567.8**2)),
    ),

    # NaNs
    (
        (0, 0, 0),
        (nan, 0, nan),
        (0, nan, nan),
        (nan, 123, nan),
        (123, nan, nan),
        (nan, nan, nan),
    ),
))
def test_extract_objects_speed(data):
    test_df = pd.DataFrame(data, columns=(
        ('input_objects', 'velocity_otg_y'),
        ('input_objects', 'velocity_otg_x'),
        ('expected_objects', 'speed')
    ))

    # Setup
    input_host = RtHost()
    input_objects = fill_dataset_signals(RtObjects(), test_df['input_objects'])

    # Expected
    expected_objects_signals = test_df['expected_objects']

    # Results
    rtcross = RtCrossDataSetPostExtraction()
    rtcross._host = input_host
    rtcross._objects = input_objects
    rtcross._host_ts_index = rtcross._host.signals.set_index('utc_timestamp')
    rtcross._host_suffix = '_host'

    rtcross._extract_objects_speed()

    result_objects_signals = rtcross._objects.signals
    expected_columns_to_compare = test_df['expected_objects'].columns

    # Verify
    pd.testing.assert_frame_equal(expected_objects_signals,
                                  result_objects_signals[expected_columns_to_compare],
                                  check_dtype=False)


def test_extract_objects_speed_simple():
    # Setup
    input_objects = RtObjects()
    input_objects.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 0, 0],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 3, 4],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), -5, 12],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 6, -8],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), -7, -24],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 123.4, -567.8],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), nan, 0],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 0, nan],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'velocity_otg_y', 'velocity_otg_x']
    )

    # Expected
    expected_objects = RtObjects()
    expected_objects.signals = pd.DataFrame(
        data=[
            [1, 1000.43, datetime.strptime('2019-07-22 00:15:00.15', '%Y-%m-%d %H:%M:%S.%f'), 0, 0, 0],
            [2, 1000.44, datetime.strptime('2019-07-22 00:15:00.16', '%Y-%m-%d %H:%M:%S.%f'), 3, 4, 5],
            [3, 1000.45, datetime.strptime('2019-07-22 00:15:00.17', '%Y-%m-%d %H:%M:%S.%f'), -5, 12, 13],
            [4, 1000.46, datetime.strptime('2019-07-22 00:15:00.18', '%Y-%m-%d %H:%M:%S.%f'), 6, -8, 10],
            [5, 1000.47, datetime.strptime('2019-07-22 00:15:00.19', '%Y-%m-%d %H:%M:%S.%f'), -7, -24, 25],
            [6, 1000.48, datetime.strptime('2019-07-22 00:15:00.20', '%Y-%m-%d %H:%M:%S.%f'), 123.4, -567.8, sqrt(123.4**2+567.8**2)],
            [7, 1000.49, datetime.strptime('2019-07-22 00:15:00.21', '%Y-%m-%d %H:%M:%S.%f'), nan, 0, nan],
            [8, 1000.50, datetime.strptime('2019-07-22 00:15:00.22', '%Y-%m-%d %H:%M:%S.%f'), 0, nan, nan],
        ],
        columns=['scan_index', 'timestamp', 'utc_timestamp', 'velocity_otg_y', 'velocity_otg_x', 'speed']
    )

    # Results
    rtcross = RtCrossDataSetPostExtraction()
    rtcross._objects = deepcopy(input_objects)
    rtcross._extract_objects_speed()
    result_objects = rtcross._objects

    # Verify
    pd.testing.assert_frame_equal(expected_objects.signals, result_objects.signals,
                                  check_like=True, check_dtype=False)
