from typing import List
import pytest
import numpy as np

from aspe.extractors.ReferenceExtractor.dSpace.builders.dspace_bin_objects_builder import DspaceBinObjectsBuilder
from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.utilities.MathFunctions import calc_position_in_bounding_box


@pytest.fixture
def parsed_dspace_data() -> dict:
    """Stub for dSpace data output by the resim as parsed from a bin file.

    The data includes two timesteps and up to two objects.
    The stub data is from the REAR_LEFT sensor intentionally because for some unknown reason only the REAR_LEFT sensor
    outputs the dSpace reference data - the other sensors output the file but it's empty. The tests and implementation
    of the builder assume that data has been parsed from the REAR_LEFT.

    :return: a dictionary of all the signals stored in the dSpace output bin file.
    """
    return {
        "parsed_data": {
            "log_timestamp": np.array([17468.0, 17518.0]),
            "scan_index": np.array([350.0, 351.0]),
            "OSIGTInput_RL_vcs_long_posn": np.array([[0.0, 14.94510269165039], [0.0, 15.222877502441406]]),
            "OSIGTInput_RL_vcs_long_vel": np.array([[0.0, 27.77777862548828], [0.0, 27.77777862548828]]),
            "OSIGTInput_RL_vcs_long_vel_rel": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "OSIGTInput_RL_vcs_long_accel": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "OSIGTInput_RL_vcs_lat_posn": np.array([[0.0, -3.498321056365967], [0.0, -3.498321056365967]]),
            "OSIGTInput_RL_vcs_lat_vel": np.array([[0.0, -1.0590607502081184e-07], [0.0, -7.847522454085265e-08]]),
            "OSIGTInput_RL_vcs_lat_vel_rel": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "OSIGTInput_RL_vcs_lat_accel": np.array([[0.0, -0.0], [0.0, -0.0]]),
            "OSIGTInput_RL_heading": np.array([[0.0, -3.8126186652220895e-09], [0.0, -2.8251079253749367e-09]]),
            "OSIGTInput_RL_length": np.array([[0.0, 4.362773895263672], [0.0, 4.362773895263672]]),
            "OSIGTInput_RL_speed": np.array([[0.0, 27.77777862548828], [0.0, 27.77777862548828]]),
            "OSIGTInput_RL_width": np.array([[0.0, 1.9765260219573975], [0.0, 1.9765260219573975]]),
        },
        "path": "",
    }


def test_shape(parsed_dspace_data):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted_objects = objects_builder.build()

    # There's one object tracked for 2 timesteps, therefore the dataframe should have two rows
    assert extracted_objects.signals.shape[0] == 2


def test_coordinate_system(parsed_dspace_data):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted_objects = objects_builder.build()

    assert extracted_objects.coordinate_system == "VCS"


@pytest.mark.parametrize(
    "aspe_signal_name, dspace_signal_name",
    [
        ("position_x", "OSIGTInput_RL_vcs_long_posn"),
        ("position_y", "OSIGTInput_RL_vcs_lat_posn"),
        ("velocity_otg_x", "OSIGTInput_RL_vcs_long_vel"),
        ("velocity_otg_y", "OSIGTInput_RL_vcs_lat_vel"),
        ("velocity_rel_x", "OSIGTInput_RL_vcs_long_vel_rel"),
        ("velocity_rel_y", "OSIGTInput_RL_vcs_lat_vel_rel"),
        ("acceleration_otg_x", "OSIGTInput_RL_vcs_long_accel"),
        ("acceleration_otg_y", "OSIGTInput_RL_vcs_lat_accel"),
        ("bounding_box_dimensions_x", "OSIGTInput_RL_length"),
        ("bounding_box_dimensions_y", "OSIGTInput_RL_width"),
        ("bounding_box_orientation", "OSIGTInput_RL_heading"),
        ("speed", "OSIGTInput_RL_speed"),
    ],
)
def test_mapped_signals(parsed_dspace_data: dict, aspe_signal_name: str, dspace_signal_name: str):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # Extract the values of the parameterized signal
    extracted_signal_values = extracted.signals[aspe_signal_name]

    # Only expect the extracted signals to contain actual objects - in parsed_dspace_data there's only one
    # object, in column 1
    expected_signal_values = parsed_dspace_data["parsed_data"][dspace_signal_name][:, 1]

    assert np.all(extracted_signal_values == expected_signal_values)


def test_bbox_center(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # Compute the expected position of the center of the bounding box - in parsed_dspace_data there's only one object,
    # in column 1
    pos_x = parsed_dspace_data["parsed_data"]["OSIGTInput_RL_vcs_long_posn"][:, 1]
    pos_y = parsed_dspace_data["parsed_data"]["OSIGTInput_RL_vcs_lat_posn"][:, 1]
    length = parsed_dspace_data["parsed_data"]["OSIGTInput_RL_length"][:, 1]
    width = parsed_dspace_data["parsed_data"]["OSIGTInput_RL_width"][:, 1]
    heading = parsed_dspace_data["parsed_data"]["OSIGTInput_RL_heading"][:, 1]
    ref_x = np.array([1.0, 1.0])  # Reference point is VCS origin: middle front
    ref_y = np.array([0.5, 0.5])  # Reference point is VCS origin: middle front
    center_ref_x = np.array([0.5] * pos_x.shape[0])
    center_ref_y = np.array([0.5] * pos_x.shape[0])
    expected_center_x, expected_center_y = calc_position_in_bounding_box(
        pos_x,
        pos_y,
        length,
        width,
        heading,
        ref_x,
        ref_y,
        center_ref_x,
        center_ref_y,
    )

    assert np.all(expected_center_x == extracted.signals["center_x"])
    assert np.all(expected_center_y == extracted.signals["center_y"])


def test_movement_status_with_moving_object(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # In parsed_dspace_data there's only one object in column 1. The object has a speed different than 0 m/s thus we
    # expect it to have the status MOVING
    assert np.all(extracted.signals["movement_status"] == MovementStatus.MOVING)


def test_movement_status_with_stationary_object(parsed_dspace_data: dict):
    # Change speeds of all objects in all scan indices to 0 m/s
    parsed_dspace_data["parsed_data"]["OSIGTInput_RL_speed"] = np.zeros_like(parsed_dspace_data["parsed_data"]["OSIGTInput_RL_speed"])

    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # In parsed_dspace_data there's only one object in column 1. The object has a speed equal to 0 m/s thus we expect it
    # to have the status STATIONARY
    assert np.all(extracted.signals["movement_status"] == MovementStatus.STATIONARY)


def test_scan_index(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # All scan indices in parsed_dspace_data have at least one object; expect all scan indices to be extracted
    assert np.all(extracted.signals["scan_index"] == parsed_dspace_data["parsed_data"]["scan_index"])


def test_timestamp(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data)
    extracted = objects_builder.build()

    # All timestamps in parsed_dspace_data have at least one object; expect all timestamps to be extracted and
    # converted from miliseconds to seconds
    expected_timestamp = parsed_dspace_data["parsed_data"]["log_timestamp"] * 1e-3
    assert np.all(extracted.signals["timestamp"] == expected_timestamp)


@pytest.mark.parametrize(
    "length_offset, expected_class",
    [
        (0.0, ObjectClass.CAR),  # No offset from stub data
        (0.8, ObjectClass.CAR),  # Positive deviation still inside the allowed range
        (-0.8, ObjectClass.CAR),  # Negative deviation still inside the allowed range
        (-0.9, ObjectClass.UNKNOWN),  # Outside allowed deviation
        (0.9, ObjectClass.UNKNOWN),  # Outside allowed deviation
    ],
)
def test_object_classification(parsed_dspace_data: dict, length_offset: float, expected_class: List[ObjectClass]):
    parsed_dspace_data["parsed_data"]["OSIGTInput_RL_length"][:, 1] += length_offset

    # allowed_length_relative_deviation controls the range of lengths that correspond to a object class
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data, allowed_length_relative_deviation=0.2)
    extracted = objects_builder.build()

    assert extracted.signals["object_class"].to_list() == [expected_class, expected_class]


def test_extract_raw_signals_disabled(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data, f_extract_raw_signals=False)
    extracted = objects_builder.build()

    assert extracted.raw_signals is None


def test_extract_raw_signals_enabled(parsed_dspace_data: dict):
    objects_builder = DspaceBinObjectsBuilder(parsed_dspace_data, f_extract_raw_signals=True)
    extracted = objects_builder.build()

    assert extracted.raw_signals is not None
    # There's one object tracked for 2 timesteps, therefore the dataframe should have two rows
    assert extracted.raw_signals.shape[0] == 2
    # All signals that were parsed must be also contained in the raw_signals dataframe
    # All signal data for each object in parsed_dspace_data must be exactly the same in raw_signals -
    for signal_name, signal_data in parsed_dspace_data["parsed_data"].items():
        assert signal_name in extracted.raw_signals.columns

        if signal_data.ndim == 1:
            expected_data = signal_data
        elif signal_data.ndim == 2:
            # There's only one object, in column 1
            expected_data = signal_data[:, 1]
        else:
            assert False, f"invalid signal shape: {signal_data.ndim}; expected 1 or 2"

        assert np.all(extracted.raw_signals[signal_name] == expected_data)
