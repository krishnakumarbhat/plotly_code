from aspe.extractors.GDSR.builders.enums.gdsr_sensor import GdsrMountingLocation, GdsrRadarSensorType
import pytest
import numpy as np

from aspe.extractors.GDSR.builders.gdsr_bin_radar_sensors_builder import GdsrBinRadarSensorsBuilder


@pytest.fixture
def parsed_gdsr_radar_sensors() -> dict:
    """Stub for GDSR radar parameters data as parsed from a bin file.

    The data includes two timesteps.

    :return: a dictionary of all the signals stored in the GDSR radar parameters bin file.
    """
    return {
        "parsed_data": {
            "index": np.array([0.0, 1.0, 0.0, 1.0]),
            "id": np.array([1.0, 2.0, 1.0, 2.0]),
            "f_active": np.array([1.0, 1.0, 1.0, 1.0]),
            "mount_loc": np.array([153.0, 155.0, 153.0, 155.0]),
            "alignment": np.array([-0.0, -0.0, -0.0, -0.0]),
            "alignment_quality_factor": np.array([1.0, 0.0, 1.0, 0.0]),
            "azimuth_polarity": np.array([-1.0, 1.0, -1.0, 1.0]),
            "boresight_angle": np.array([-2.146754503250122, 2.146754503250122, -2.146754503250122, 2.146754503250122]),
            "cos_boresight_angle": np.array(
                [
                    -0.5446386337280273,
                    -0.5446386337280273,
                    -0.5446386337280273,
                    -0.5446386337280273,
                ]
            ),
            "sin_boresight_angle": np.array(
                [
                    -0.8386708498001099,
                    0.8386708498001099,
                    -0.8386708498001099,
                    0.8386708498001099,
                ]
            ),
            "long_posn": np.array([-4.681000232696533, -4.681000232696533, -4.681000232696533, -4.681000232696533]),
            "lat_posn": np.array([-0.8529999852180481, 0.8529999852180481, -0.8529999852180481, 0.8529999852180481]),
            "long_vel": np.array([13.890110969543457, 13.861041069030762, 13.87668514251709, 13.8907470703125]),
            "lat_vel": np.array(
                [
                    8.534798208949712e-16,
                    8.517006811288228e-16,
                    8.588348690666313e-16,
                    8.597035013540406e-16,
                ]
            ),
            "local_long_vel": np.array(
                [
                    -7.565091133117676,
                    -7.549258232116699,
                    -7.557778835296631,
                    -7.565437316894531,
                ]
            ),
            "local_lat_vel": np.array([11.64923095703125, -11.62485122680664, 11.637970924377441, -11.649765014648438]),
            "sensor_sideslip": np.array([0.0, 0.0, 0.0, 0.0]),
            "host_speed_raw": np.array(
                [
                    13.888641357421875,
                    13.888641357421875,
                    13.888656616210938,
                    13.888656616210938,
                ]
            ),
            "host_speed_estimated": np.array(
                [
                    13.890110969543457,
                    13.861041069030762,
                    13.87668514251709,
                    13.8907470703125,
                ]
            ),
            "look_id": np.array([0.0, 2.0, 3.0, 0.0]),
            "dwell_type": np.array([1.0, 1.0, 0.0, 1.0]),
            "v_un": np.array([69.78799438476562, 69.78799438476562, 59.582881927490234, 69.78799438476562]),
            "max_range_current_look": np.array([98.0, 40.0, 43.0, 98.0]),
            "max_range_rate_current_look": np.array(
                [
                    69.78799438476562,
                    69.78799438476562,
                    59.582881927490234,
                    69.78799438476562,
                ]
            ),
            "min_range_rate_current_look": np.array([0.0, 0.0, 0.0, 0.0]),
            "range_coverage": np.array([105.0, 105.0, 105.0, 105.0]),
            "rolling_count_transformDetections": np.array([176.0, 176.0, 177.0, 177.0]),
            "rolling_count_processDetections": np.array([176.0, 176.0, 177.0, 177.0]),
            "rolling_count_trackletTracker": np.array([176.0, 176.0, 177.0, 177.0]),
            "scan_index_radar_parameter": np.array([175.0, 175.0, 176.0, 176.0]),
            "time_stamp": np.array([851.0, 851.0, 901.0, 901.0]),
            "fov_minaz_d": np.array([1.8847293853759766, 4.251688480377197, 1.8847293853759766, 4.251688480377197]),
            "fov_minaz_nx": np.array(
                [
                    -0.22508391737937927,
                    -0.7986149787902832,
                    -0.22508391737937927,
                    -0.7986149787902832,
                ]
            ),
            "fov_minaz_ny": np.array(
                [
                    -0.9743393659591675,
                    0.6018422245979309,
                    -0.9743393659591675,
                    0.6018422245979309,
                ]
            ),
            "fov_maxaz_d": np.array([4.251688480377197, 1.8847293853759766, 4.251688480377197, 1.8847293853759766]),
            "fov_maxaz_nx": np.array(
                [
                    -0.7986149787902832,
                    -0.22508391737937927,
                    -0.7986149787902832,
                    -0.22508391737937927,
                ]
            ),
            "fov_maxaz_ny": np.array(
                [
                    -0.6018422245979309,
                    0.9743393659591675,
                    -0.6018422245979309,
                    0.9743393659591675,
                ]
            ),
            "ex_zone_at_max_az": np.array([0.0, 1.0, 0.0, 1.0]),
            "ex_zone_at_min_az": np.array([1.0, 0.0, 1.0, 0.0]),
            "max_az_non_ex_zone": np.array(
                [
                    1.0471975803375244,
                    1.0471975803375244,
                    1.0471975803375244,
                    1.0471975803375244,
                ]
            ),
            "log_timestamp": np.array([8779.0, 8779.0, 8829.0, 8829.0]),
            "scan_index": np.array([175.0, 175.0, 176.0, 176.0]),
            "is_in_other_sensor_fov": np.array([[0.0, 1.0], [1.0, 0.0], [0.0, 1.0], [1.0, 0.0]]),
        },
        "path": "",
    }


def test_coordinate_system(parsed_gdsr_radar_sensors):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted_sensors = sensors_builder.build()

    assert extracted_sensors.coordinate_system == "VCS"


def test_extract_raw_signals_disabled(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors, f_extract_raw_signals=False)
    extracted = sensors_builder.build()

    assert extracted.raw_signals is None


def test_shape(parsed_gdsr_radar_sensors):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted_sensors = sensors_builder.build()

    # The dataframe should have four rows because we have two sensors in two timesteps
    assert extracted_sensors.signals.shape[0] == 4


def test_extract_raw_signals_enabled(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors, f_extract_raw_signals=True)
    extracted = sensors_builder.build()

    assert extracted.raw_signals is not None
    # There's two time steps and two radar sensors
    assert extracted.raw_signals.shape[0] == 4
    # All signals that were parsed must also be contained in the raw_signals dataframe
    # All signal data in parsed_gdsr_radar_sensors must match exactly to raw_signals
    for signal_name, signal_data in parsed_gdsr_radar_sensors["parsed_data"].items():
        assert signal_name in extracted.raw_signals.columns

        # Transform the extracted data into a numpy array that we can use to compare to parsed_gdsr_radar_sensors
        if signal_data.ndim == 1:
            extracted_data = extracted.raw_signals[signal_name]
        elif signal_data.ndim == 2:
            # This looks weird but all we are doing is taking the 1D arrays in each row from the column `signal_name`
            # and transform them into a 2D numpy array for easier comparison
            extracted_data = np.array(extracted.raw_signals[signal_name].values.tolist())
        else:
            assert False, f"signal `{signal_name}` has invalid shape: {signal_data.ndim}; expected 1 or 2"

        assert np.all(extracted_data == signal_data), f"the data of `{signal_name}` doesn't match the parsed data"


@pytest.mark.parametrize(
    "aspe_signal_name, gdsr_signal_name",
    [
        ("sensor_id", "id"),
        ("look_id", "look_id"),
        ("sensor_valid", "f_active"),
        ("velocity_otg_x", "long_vel"),
        ("velocity_otg_y", "lat_vel"),
    ],
)
def test_mapped_signals(parsed_gdsr_radar_sensors: dict, aspe_signal_name: str, gdsr_signal_name: str):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # Data for each signal must match exactly
    assert np.all(extracted.signals[aspe_signal_name] == parsed_gdsr_radar_sensors["parsed_data"][gdsr_signal_name])


def test_timestamp(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # All timestamps are converted from milliseconds to seconds
    expected_timestamp = parsed_gdsr_radar_sensors["parsed_data"]["log_timestamp"] * 1e-3
    assert np.all(extracted.signals["timestamp"] == expected_timestamp)


def test_signals_per_sensor_shape(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # The dataframe should have 2 rows because there are 2 sensors
    assert extracted.per_sensor.shape[0] == 2


@pytest.mark.parametrize(
    "aspe_signal_name, gdsr_signal_name",
    [
        ("sensor_id", "id"),
        ("polarity", "azimuth_polarity"),
        ("boresight_az_angle", "boresight_angle"),
        ("position_x", "long_posn"),
        ("position_y", "lat_posn"),
    ],
)
def test_mapped_signals_per_sensor(parsed_gdsr_radar_sensors: dict, aspe_signal_name: str, gdsr_signal_name: str):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # Parsed data has 2 unique sensors in the first 2 indices of the array. Check signal mapping.
    expected = parsed_gdsr_radar_sensors["parsed_data"][gdsr_signal_name][:2]
    data = extracted.per_sensor[aspe_signal_name]
    assert np.all(data == expected)


def test_mounting_location(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # Parsed data has two unique sensors in the first two indices of the array. Check the extracted mounting location
    expected = [GdsrMountingLocation(parsed_gdsr_radar_sensors["parsed_data"]["mount_loc"][0])]
    expected.append(GdsrMountingLocation(parsed_gdsr_radar_sensors["parsed_data"]["mount_loc"][1]))
    data = extracted.per_sensor["mount_location"]
    assert all(data == expected)


def test_sensor_type(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # The expected mounting is unknown since the log files don't provide such information
    expected = np.full(extracted.per_sensor.shape[0], GdsrRadarSensorType.UNKNOWN)
    assert np.all(extracted.per_sensor["sensor_type"] == expected)


def test_signals_per_look_shape(parsed_gdsr_radar_sensors: dict):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # The dataframe should have 4 rows because there are 2 unique look IDs for each of the 2 sensors
    assert extracted.per_look.shape[0] == 4


@pytest.mark.parametrize(
    "aspe_signal_name, gdsr_signal_name",
    [
        ("sensor_id", "id"),
        ("look_id", "look_id"),
        ("max_range", "max_range_current_look"),
        ("range_rate_interval_width", "v_un"),
    ],
)
def test_mapped_signals_per_look(parsed_gdsr_radar_sensors: dict, aspe_signal_name: str, gdsr_signal_name: str):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # Parsed data has 3 unique sensors in the first 3 indices of the array. Check signal mapping.
    expected = parsed_gdsr_radar_sensors["parsed_data"][gdsr_signal_name]
    data = extracted.per_look[aspe_signal_name]
    assert np.all(data == expected)


def test_per_look_azimuth(parsed_gdsr_radar_sensors):
    sensors_builder = GdsrBinRadarSensorsBuilder(parsed_gdsr_radar_sensors)
    extracted = sensors_builder.build()

    # Parsed data has 3 unique sensors in the first 3 indices of the array. Check conversion between Hesse form to
    # radians
    min_az_nx = parsed_gdsr_radar_sensors["parsed_data"]["fov_minaz_nx"]
    min_az_ny = parsed_gdsr_radar_sensors["parsed_data"]["fov_minaz_ny"]
    min_az_n = np.arctan2(min_az_ny, min_az_nx)

    max_az_nx = parsed_gdsr_radar_sensors["parsed_data"]["fov_maxaz_nx"]
    max_az_ny = parsed_gdsr_radar_sensors["parsed_data"]["fov_maxaz_ny"]
    # Sum Pi to the maximum azimuth angle because the normal vector, after being calculated, is reversed to point
    # the boresight
    max_az_n = np.arctan2(max_az_ny, max_az_nx) + np.pi

    fov_angle = max_az_n - min_az_n
    boresight_angle = parsed_gdsr_radar_sensors["parsed_data"]["boresight_angle"]
    expected_min_az = boresight_angle - (0.5 * fov_angle)
    expected_max_az = boresight_angle + (0.5 * fov_angle)

    assert np.all(extracted.per_look["min_azimuth"] == expected_min_az)
    assert np.all(extracted.per_look["max_azimuth"] == expected_max_az)
