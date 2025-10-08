import pytest
import numpy as np

from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.utilities.MathFunctions import calc_position_in_bounding_box
from aspe.extractors.GDSR.builders.gdsr_bin_objects_builder import GdsrBinObjectsBuilder


@pytest.fixture
def parsed_gdsr_output_objects() -> dict:
    """Stub for GDSR tracker output data as parsed from a bin file.

    The data includes two timesteps and two objects.

    :return: a dictionary of all the signals stored in the GDSR tracker output bin file.
    """
    return {
        "parsed_data": {
            "rolling_count_fusionTracker": np.array([95.0, 96.0]),
            "errors": np.array([2.781342323134e-309, 2.781342323134e-309]),
            "log_timestamp": np.array([17468.0, 17518.0]),
            "scan_index": np.array([350.0, 351.0]),
            "culminated_existence_probability": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "id": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "distinct_id": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "age": np.array([[72.0, 0.0], [73.0, 0.0]]),
            "stage_age": np.array([[63.0, 0.0], [64.0, 0.0]]),
            "status": np.array([[2.0, 0.0], [2.0, 0.0]]),
            "speed": np.array([[27.72228240966797, 0.0], [27.712942123413086, 0.0]]),
            "tangential_accel": np.array([[-0.0003603061195462942, 0.0], [-0.0033974209800362587, 0.0]]),
            "vcs_long_posn": np.array([[15.117459297180176, 0.0], [15.398212432861328, 0.0]]),
            "vcs_lat_posn": np.array([[-3.9493677616119385, 0.0], [-4.003518104553223, 0.0]]),
            "vcs_long_vel": np.array([[27.69455337524414, 0.0], [27.6756534576416, 0.0]]),
            "vcs_lat_vel": np.array([[-0.2727387249469757, 0.0], [-0.3149105906486511, 0.0]]),
            "vcs_long_accel": np.array([[-0.056940022855997086, 0.0], [-0.0740421712398529, 0.0]]),
            "vcs_lat_accel": np.array([[-0.3461242914199829, 0.0], [-0.38555440306663513, 0.0]]),
            "vcs_long_vel_rel": np.array([[5.523006439208984, 0.0], [5.481254577636719, 0.0]]),
            "vcs_lat_vel_rel": np.array([[-0.27273863554000854, 0.0], [-0.31491050124168396, 0.0]]),
            "heading": np.array([[-0.009386301040649414, 0.0], [-0.011028289794921875, 0.0]]),
            "heading_rate": np.array([[-0.002506471239030361, 0.0], [-0.0029205733444541693, 0.0]]),
            "width": np.array([[1.7630383968353271, 0.0], [1.7547663450241089, 0.0]]),
            "length": np.array([[3.4538891315460205, 0.0], [3.4545772075653076, 0.0]]),
            "object_class": np.array([[3.0, 0.0], [3.0, 0.0]]),
            "object_distance": np.array([[13.55757999420166, -1.0], [13.844759941101074, -1.0]]),
            "ref_position": np.array([[5.0, 8.0], [5.0, 8.0]]),
            "ref_long_posn": np.array([[13.39058780670166, 0.0], [13.671032905578613, 0.0]]),
            "ref_lat_posn": np.array([[-3.9334733486175537, 0.0], [-3.984308958053589, 0.0]]),
            "ref_long_vel": np.array([[27.69455337524414, 0.0], [27.6756534576416, 0.0]]),
            "ref_lat_vel": np.array([[-0.2727387249469757, 0.0], [-0.3149105906486511, 0.0]]),
            "ref_long_accel": np.array([[-0.056940022855997086, 0.0], [-0.0740421712398529, 0.0]]),
            "ref_lat_accel": np.array([[-0.3461242914199829, 0.0], [-0.38555440306663513, 0.0]]),
            "curvi_heading": np.array([[-0.00938628800213337, 0.0], [-0.01102827675640583, 0.0]]),
            "curvi_long_posn": np.array([[15.117459297180176, 0.0], [15.398212432861328, 0.0]]),
            "curvi_lat_posn": np.array([[-3.9493677616119385, 0.0], [-4.003518104553223, 0.0]]),
            "curvi_long_vel": np.array([[27.69455337524414, 0.0], [27.6756534576416, 0.0]]),
            "curvi_lat_vel": np.array([[-0.2727387249469757, 0.0], [-0.3149105906486511, 0.0]]),
            "curvi_long_accel": np.array([[-0.056940022855997086, 0.0], [-0.0740421712398529, 0.0]]),
            "curvi_lat_accel": np.array([[-0.3461242914199829, 0.0], [-0.38555440306663513, 0.0]]),
            "curvi_long_vel_rel": np.array([[5.523006439208984, 0.0], [5.481254577636719, 0.0]]),
            "curvi_lat_vel_rel": np.array([[-0.27273863554000854, 0.0], [-0.31491050124168396, 0.0]]),
            "curvi_coord_calc_method": np.array([[3.0, 0.0], [3.0, 0.0]]),
            "existence_probability": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "eclipse_value": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "obstruction_probability": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "priority_number": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "priority_value": np.array([[13.55757999420166, 0.0], [13.844759941101074, 0.0]]),
            "probability_2wheel": np.array([[0.0003852082008961588, 0.0], [0.0004299802239984274, 0.0]]),
            "probability_car": np.array([[0.9996147751808167, 0.0], [0.9995700120925903, 0.0]]),
            "probability_pedestrian": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "probability_truck": np.array([[1.7760360293550548e-08, 0.0], [1.590962561692777e-08, 0.0]]),
            "probability_unknown": np.array([[0.0, 1.0], [0.0, 1.0]]),
            "f_just_merged": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "f_just_merged_with": np.array([[-1.0, -1.0], [-1.0, -1.0]]),
            "f_moveable": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "f_stationary": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "f_reflection": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "f_ref_updated": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "f_behind_guardrail": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "f_just_redivided": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "f_intersects_guardrail": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "innovation_pos_lon": np.array([[0.19006715714931488, 0.0], [0.14993984997272491, 0.0]]),
            "innovation_pos_lat": np.array([[0.2414567470550537, 0.0], [0.24932558834552765, 0.0]]),
            "innovation_size_x": np.array([[0.46121612191200256, 0.0], [0.4622012674808502, 0.0]]),
            "innovation_size_y": np.array([[0.3021963834762573, 0.0], [0.354763925075531, 0.0]]),
            "innovation_vel_lon": np.array([[0.022983616217970848, 0.0], [0.023566149175167084, 0.0]]),
            "innovation_vel_lat": np.array([[0.09556646645069122, 0.0], [0.04001076519489288, 0.0]]),
            "innovation_acc_lon": np.array([[0.04023648425936699, 0.0], [0.038250841200351715, 0.0]]),
            "innovation_acc_lat": np.array([[0.10052414238452911, 0.0], [0.04571632295846939, 0.0]]),
            "innovation_heading": np.array([[0.16688428819179535, 0.0], [0.16690224409103394, 0.0]]),
            "innovation_heading_rate": np.array([[0.7185155749320984, 0.0], [0.7195305228233337, 0.0]]),
            "variance_vel_lon_filt": np.array([[0.08076247572898865, 0.0], [0.059228744357824326, 0.0]]),
            "variance_vel_lat_filt": np.array([[1.0333592891693115, 0.0], [0.36934930086135864, 0.0]]),
            "variance_pos_lon_filt": np.array([[0.16256806254386902, 0.0], [0.1437976360321045, 0.0]]),
            "variance_pos_lat_filt": np.array([[0.19152656197547913, 0.0], [0.16406859457492828, 0.0]]),
            "variance_size_filtered_x": np.array([[0.2706991136074066, 0.0], [0.26938021183013916, 0.0]]),
            "variance_size_filtered_y": np.array([[0.40178292989730835, 0.0], [0.4987146854400635, 0.0]]),
            "variance_heading": np.array([[0.001347043551504612, 0.0], [0.00048210102249868214, 0.0]]),
            "variance_covariances_velocities_filtered": np.array(
                [[0.26336371898651123, 0.0], [0.11400911211967468, 0.0]]
            ),
            "variance_covariances_positions_filtered": np.array(
                [[0.1390264928340912, 0.0], [0.12731854617595673, 0.0]]
            ),
            "accuracy_pos_lon": np.array([[0.5166053771972656, 0.0], [0.49693840742111206, 0.0]]),
            "accuracy_pos_lat": np.array([[0.7200373411178589, 0.0], [0.6987918615341187, 0.0]]),
            "accuracy_size_x": np.array([[0.45661669969558716, 0.0], [0.45829206705093384, 0.0]]),
            "accuracy_size_y": np.array([[0.23099030554294586, 0.0], [0.2681224048137665, 0.0]]),
            "accuracy_vel_lon": np.array([[0.3556591272354126, 0.0], [0.3223470449447632, 0.0]]),
            "accuracy_vel_lat": np.array([[1.3574409484863281, 0.0], [1.2790040969848633, 0.0]]),
            "accuracy_acceleration_lon": np.array([[0.042376916855573654, 0.0], [0.04113909602165222, 0.0]]),
            "accuracy_acceleration_lat": np.array([[0.06121277064085007, 0.0], [0.056563835591077805, 0.0]]),
            "accuracy_heading": np.array([[0.04879003390669823, 0.0], [0.04597589746117592, 0.0]]),
            "accuracy_heading_rate": np.array([[0.7158992886543274, 0.0], [0.7169886827468872, 0.0]]),
            "filtered_amplitude": np.array([[7.218466281890869, -100.0], [7.704865455627441, -100.0]]),
            "origin_sensor_front_left": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "origin_sensor_front_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "origin_sensor_front_right": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "origin_sensor_right_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "origin_sensor_rear_right": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "origin_sensor_rear_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "origin_sensor_rear_left": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "origin_sensor_left_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_front_left": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "in_sensor_FOV_front_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_front_right": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_right_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_rear_right": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_rear_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_rear_left": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "in_sensor_FOV_left_center": np.array([[0.0, 0.0], [0.0, 0.0]]),
            "guardrail_f_active": np.array([[1.0, 1.0], [1.0, 1.0]]),
            "guardrail_lateral_position": np.array([[-5.999999523162842, 0.0], [-5.999999523162842, 0.0]]),
            "guardrail_status": np.array([[2.0, 0.0], [2.0, 0.0]]),
            "track_age": np.array([[255.0, 0.0], [255.0, 0.0]]),
            "guardrail_existence_probability": np.array([[1.0, 0.0], [1.0, 0.0]]),
            "f_concrete_guardrail_present": np.array([[0.0, 0.0], [0.0, 0.0]]),
        },
        "path": "",
    }


def test_shape(parsed_gdsr_output_objects):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted_objects = objects_builder.build()

    # There's one object tracked for 2 timesteps, therefore the dataframe should have two rows
    assert extracted_objects.signals.shape[0] == 2


def test_coordinate_system(parsed_gdsr_output_objects):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted_objects = objects_builder.build()

    assert extracted_objects.coordinate_system == "VCS"


@pytest.mark.parametrize(
    "aspe_signal_name, gdsr_signal_name",
    [
        ("id", "distinct_id"),
        ("position_x", "ref_long_posn"),
        ("position_y", "ref_lat_posn"),
        ("velocity_otg_x", "ref_long_vel"),
        ("velocity_otg_y", "ref_lat_vel"),
        ("velocity_rel_x", "vcs_long_vel_rel"),
        ("velocity_rel_y", "vcs_lat_vel_rel"),
        ("acceleration_otg_x", "ref_long_accel"),
        ("acceleration_otg_y", "ref_lat_accel"),
        ("yaw_rate", "heading_rate"),
        ("bounding_box_dimensions_x", "length"),
        ("bounding_box_dimensions_y", "width"),
        ("bounding_box_orientation", "heading"),
        ("existence_indicator", "existence_probability"),
        ("velocity_otg_variance_x", "variance_vel_lon_filt"),
        ("velocity_otg_variance_y", "variance_vel_lat_filt"),
        ("position_variance_x", "variance_pos_lon_filt"),
        ("position_variance_y", "variance_pos_lat_filt"),
        ("bounding_box_orientation_variance", "variance_heading"),
        ("velocity_otg_covariance", "variance_covariances_velocities_filtered"),
        ("position_covariance", "variance_covariances_positions_filtered"),
        ("speed", "speed"),
    ],
)
def test_mapped_signals(parsed_gdsr_output_objects: dict, aspe_signal_name: str, gdsr_signal_name: str):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    # Extract the values of the parameterized signal
    extracted_signal_values = extracted.signals[aspe_signal_name]

    # Only expect the extracted signals to contain actual objects - in parsed_gdsr_output_objects there's only one
    # object, in column 0
    expected_signal_values = parsed_gdsr_output_objects["parsed_data"][gdsr_signal_name][:, 0]

    assert np.all(extracted_signal_values == expected_signal_values)


def test_bounding_box_refpoint(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    long_bbox_refpoint_ratio = extracted.signals["bounding_box_refpoint_long_offset_ratio"].to_list()
    assert long_bbox_refpoint_ratio == [0.0, 0.0]

    lat_bbox_refpoint_ratio = extracted.signals["bounding_box_refpoint_lat_offset_ratio"].to_list()
    assert lat_bbox_refpoint_ratio == [0.5, 0.5]


def test_bbox_center(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    # Compute the expected position of the center of the bounding box
    pos_x = parsed_gdsr_output_objects["parsed_data"]["ref_long_posn"][:, 0]
    pos_y = parsed_gdsr_output_objects["parsed_data"]["ref_lat_posn"][:, 0]
    length = parsed_gdsr_output_objects["parsed_data"]["length"][:, 0]
    width = parsed_gdsr_output_objects["parsed_data"]["width"][:, 0]
    heading = parsed_gdsr_output_objects["parsed_data"]["heading"][:, 0]
    ref_x = np.array([0.0, 0.0])
    ref_y = np.array([0.5, 0.5])
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


def test_movement_status(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    assert extracted.signals["movement_status"].to_list() == [MovementStatus.MOVING, MovementStatus.MOVING]


def test_scan_index(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    # All scan indices in parsed_gdsr_output_objects have at least one object; expect all scan indices to be extracted
    assert np.all(extracted.signals["scan_index"] == parsed_gdsr_output_objects["parsed_data"]["scan_index"])


def test_timestamp(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    # All timestamps in parsed_gdsr_output_objects have at least one object; expect all timestamps to be extracted and
    # converted from milliseconds to seconds
    expected_timestamp = parsed_gdsr_output_objects["parsed_data"]["log_timestamp"] * 1e-3
    assert np.all(extracted.signals["timestamp"] == expected_timestamp)


def test_object_classification(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    assert extracted.signals["object_class"].to_list() == [ObjectClass.CAR, ObjectClass.CAR]


def test_object_classification_probability(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects)
    extracted = objects_builder.build()

    expected_signal_values = parsed_gdsr_output_objects["parsed_data"]["probability_car"][:, 0]

    assert np.all(extracted.signals["object_class_probability"] == expected_signal_values)


def test_extract_raw_signals_disabled(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects, f_extract_raw_signals=False)
    extracted = objects_builder.build()

    assert extracted.raw_signals is None


def test_extract_raw_signals_enabled(parsed_gdsr_output_objects: dict):
    objects_builder = GdsrBinObjectsBuilder(parsed_gdsr_output_objects, f_extract_raw_signals=True)
    extracted = objects_builder.build()

    assert extracted.raw_signals is not None
    # There's one object tracked for 2 timesteps, therefore the dataframe should have two rows
    assert extracted.raw_signals.shape[0] == 2
    # All signals that were parsed must be also contained in the raw_signals dataframe
    # All signal data for each object in parsed_gdsr_output_objects must be exactly the same in raw_signals -
    for signal_name, signal_data in parsed_gdsr_output_objects["parsed_data"].items():
        assert signal_name in extracted.raw_signals.columns

        if signal_data.ndim == 1:
            expected_data = signal_data
        elif signal_data.ndim == 2:
            # There's only one object, in column 0
            expected_data = signal_data[:, 0]
        else:
            assert False, f"signal `{signal_name}` has invalid shape: {signal_data.ndim}; expected 1 or 2"

        assert np.all(
            extracted.raw_signals[signal_name] == expected_data
        ), f"the data of `{signal_name}` doesn't match the parsed data"
