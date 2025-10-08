import pytest

from aspe.providers.Preprocessing.objects_in_sensors_fov import are_objects_in_fov
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


@pytest.fixture
def single_sensor():
    sensors = IRadarSensors()
    sensors.coordinate_system = "VCS"
    signals_d = {
        "scan_index": 1,
        "sensor_id": 0,
        "look_id": 0,
        "sensor_valid": 1,
    }
    sensors.signals = sensors.signals.append(signals_d, ignore_index=True)

    per_sensor_d = {
        "sensor_id": 0,
        "boresight_az_angle": 0,
        "position_x": 0,
        "position_y": 0,
    }
    sensors.per_sensor = sensors.per_sensor.append(per_sensor_d, ignore_index=True)

    per_look_d = {
        "sensor_id": 0,
        "look_id": 0,
        "max_range": 10,
        "min_azimuth": -1,
        "max_azimuth": 1,
    }
    sensors.per_look = sensors.per_look.append(per_look_d, ignore_index=True)

    return sensors


@pytest.fixture
def single_object():
    objects = IObjects()
    objects.coordinate_system = "VCS"

    # Create object completely inside the FoV
    signals_d = {
        "scan_index": 1,
        "position_x": 5,
        "position_y": 0,
        "bounding_box_dimensions_x": 1,
        "bounding_box_dimensions_y": 1,
        "bounding_box_orientation": 0,
        # Rear left reference point
        "bounding_box_refpoint_long_offset_ratio": 0,
        "bounding_box_refpoint_lat_offset_ratio": 0,
    }
    objects.signals = objects.signals.append(signals_d, ignore_index=True)

    return objects


def test_single_object_completely_inside_fov(single_object, single_sensor):
    out = are_objects_in_fov(single_object, single_sensor)

    assert out.shape[0] == 1
    assert out["f_any_ref_points_in_fov"].iloc[0]
    assert out["f_all_ref_points_in_fov"].iloc[0]


def test_single_object_completely_outside_fov_range(single_object, single_sensor):
    # The object's range is 50 which is outside the FoV range
    single_object.signals["position_x"] = 50

    out = are_objects_in_fov(single_object, single_sensor)

    assert out.shape[0] == 1
    assert not out["f_any_ref_points_in_fov"].iloc[0]
    assert not out["f_all_ref_points_in_fov"].iloc[0]


def test_single_object_partially_outside_fov_range(single_object, single_sensor):
    # The object's range is 50 which is outside the FoV range
    single_object.signals["position_x"] = 9.6

    out = are_objects_in_fov(single_object, single_sensor)

    assert out.shape[0] == 1
    assert out["f_any_ref_points_in_fov"].iloc[0]
    assert not out["f_all_ref_points_in_fov"].iloc[0]


def test_single_object_completely_outside_fov_azimuth_range(single_object, single_sensor):
    # The object's azimuth is pi/2 which is outside the FoV azimuth range
    single_object.signals["position_x"] = 0
    single_object.signals["position_y"] = 5

    out = are_objects_in_fov(single_object, single_sensor)

    assert out.shape[0] == 1
    assert not out["f_any_ref_points_in_fov"].iloc[0]
    assert not out["f_all_ref_points_in_fov"].iloc[0]


def test_invalid_sensor(single_object, single_sensor):
    single_sensor.signals["sensor_valid"] = 0
    out = are_objects_in_fov(single_object, single_sensor)

    assert out.shape[0] == 1
    assert not out["f_any_ref_points_in_fov"].iloc[0]
    assert not out["f_all_ref_points_in_fov"].iloc[0]
