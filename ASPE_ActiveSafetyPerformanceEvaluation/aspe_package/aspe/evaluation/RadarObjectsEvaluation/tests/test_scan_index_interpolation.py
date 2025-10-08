import numpy as np
import pytest
import pandas as pd
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.providers.Preprocessing.ScanIndexSynch import ScanIndexInterpolationSynch
from enum import Enum


@pytest.fixture()
def interpolation_object():
    interpolation = ScanIndexInterpolationSynch(angle_signals=['azimuth'])
    return interpolation


@pytest.fixture()
def master_ds():
    master_signals = pd.DataFrame({
        'timestamp': (np.arange(5) + 1).astype(float),  # 1..5
        'scan_index': (np.arange(5) + 10).astype(float)
    })
    master_ds = IObjects()
    master_ds.signals = master_signals
    return master_ds


def test_single_object_floats_interpolation_single_signal(interpolation_object, master_ds):
    slave_signals = pd.DataFrame({
        'timestamp': np.array([0, 1.5, 2.5, 3.5, 6.5]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'signal': np.array([0, 15, 25, 35, 65])
    })

    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    np.testing.assert_array_equal(slave_ds.signals['signal'].to_numpy(), np.array([10, 20, 30, 40, 50]))


def test_single_object_floats_interpolation_two_signals(interpolation_object, master_ds):
    slave_signals = pd.DataFrame({
        'timestamp': np.array([0, 1.5, 2.5, 3.5, 6.5]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'signal_A': np.array([0, 15, 25, 35, 65]),
        'signal_B': np.array([0, 150, 250, 350, 650])
    })

    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    np.testing.assert_array_equal(slave_ds.signals['signal_A'].to_numpy(), np.array([10, 20, 30, 40, 50]))
    np.testing.assert_array_equal(slave_ds.signals['signal_B'].to_numpy(), np.array([100, 200, 300, 400, 500]))


def test_floats_interp_slave_ts_not_in_master_ts_range(interpolation_object, master_ds):
    slave_signals = pd.DataFrame({
        'timestamp': np.array([100, 101, 102, 103, 104]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'signal': np.array([0, 15, 25, 35, 65])
    })

    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    assert len(slave_ds.signals) == 0


def test_two_objects_floats_interpolation(interpolation_object, master_ds):
    slave_signals = pd.DataFrame({
        'timestamp': np.array([0, 1.5, 2.5, 3.5, 6.5] +  # first obj
                              [1.1, 2.1, 3.1]),       # second obj
        'scan_index': np.array([1, 2, 3, 4, 5] +
                               [2, 3, 4]),
        'unique_id': np.array([0] * 5 +
                              [1] * 3),
        'signal': np.array([0, 15, 25, 35, 65] +
                           [11, 21, 31])
    })

    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    expected = np.array([10, 20, 30, 40, 50] + [20, 30])  # expected first obj + expected second obj
    np.testing.assert_array_equal(slave_ds.signals['signal'].to_numpy(), expected)


def test_enum_interpolation(interpolation_object, master_ds):
    class SomeEnum(Enum):
        A = 0
        B = 1
        C = 2

    slave_signals = pd.DataFrame({
        'timestamp': np.array([0, 1.6, 2.6, 3.6, 6.6]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'signal': np.array([SomeEnum.C, SomeEnum.B, SomeEnum.A, SomeEnum.A, SomeEnum.B])
    })
    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    expected = np.array([SomeEnum.B, SomeEnum.B, SomeEnum.A, SomeEnum.A, SomeEnum.A])
    np.testing.assert_array_equal(slave_ds.signals['signal'].to_numpy(), expected)


def test_bool_interpolation(interpolation_object, master_ds):

    slave_signals = pd.DataFrame({
        'timestamp': np.array([0, 1.6, 2.6, 3.6, 6.6]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'signal': np.array([True, False, False, True, True])
    })
    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    expected = np.array([False, False, False, True, True])
    np.testing.assert_array_equal(slave_ds.signals['signal'].to_numpy(), expected)


def test_angle_interpolation(interpolation_object, master_ds):
    slave_signals = pd.DataFrame({
        'timestamp': np.array([0.5, 1.5, 2.5, 3.5, 4.5]),
        'scan_index': np.array([1, 2, 3, 4, 5]),
        'unique_id': np.array([0] * 5),
        'azimuth': np.array([-0.05, 0.15, 3.05, -3.13, -3.01])
    })
    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    interpolation_object.synch(None, None, master_ds, slave_ds)
    expected = np.array([0.05, 1.6, 3.1, -3.07])
    np.testing.assert_array_almost_equal(slave_ds.signals['azimuth'].to_numpy(), expected, 2)