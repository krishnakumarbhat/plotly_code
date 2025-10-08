import pytest
import numpy as np
import pandas as pd
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.conversions.objects_into_polyline import object_into_polyline


@pytest.fixture
def allowed_diff():
    return 1.0e-6


class TestObjectIntoPolyline:

    def test_check_for_object_0_degree_orientation(self, allowed_diff):
        single_row_dict = {
            'id': 0,
            'position_x': 6.0,
            'position_y': 5.0,
            'position_variance_x': 1.0,
            'position_variance_y': 2.0,
            'position_covariance': 0.4,
            'velocity_otg_x': 4.0,
            'velocity_otg_y': 0.0,
            'velocity_otg_variance_x': 0.2,
            'velocity_otg_variance_y': 0.4,
            'velocity_otg_covariance': 0.1,
            'bounding_box_dimensions_x': 4.0,
            'bounding_box_dimensions_y': 2.0,
            'bounding_box_orientation': np.deg2rad(0.0),
            'bounding_box_refpoint_long_offset_ratio': 0.5,
            'bounding_box_refpoint_lat_offset_ratio': 0.5,
            'yaw_rate': np.deg2rad(0.0),  # [rad/s]
        }

        expected_cov = [[1.0, 0.4, 0.0, 0.0],
                        [0.4, 2.0, 0.0, 0.0],
                        [0.0, 0.0, 0.2, 0.1],
                        [0.0, 0.0, 0.1, 0.4]]
        expected_rect = PolyLineDS()
        expected_rect.add_point(4.0, 4.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(8.0, 4.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(8.0, 6.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(4.0, 6.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(4.0, 4.0, 4.0, 0.0, expected_cov)

        single_obj_series = pd.Series(single_row_dict)
        observed_rect = object_into_polyline(single_obj_series)

        pd.testing.assert_frame_equal(expected_rect.signals, observed_rect.signals)

    def test_check_for_object_90_degree_orientation(self, allowed_diff):
        single_row_dict = {
            'id': 0,
            'position_x': 6.0,
            'position_y': 5.0,
            'position_variance_x': 1.0,
            'position_variance_y': 2.0,
            'position_covariance': 0.4,
            'velocity_otg_x': 0.0,
            'velocity_otg_y': 4.0,
            'velocity_otg_variance_x': 0.2,
            'velocity_otg_variance_y': 0.4,
            'velocity_otg_covariance': 0.1,
            'bounding_box_dimensions_x': 4.0,
            'bounding_box_dimensions_y': 2.0,
            'bounding_box_orientation': np.deg2rad(90.0),
            'bounding_box_refpoint_long_offset_ratio': 0.5,
            'bounding_box_refpoint_lat_offset_ratio': 0.5,
            'yaw_rate': np.deg2rad(0.0),  # [rad/s]
        }

        expected_cov = [[1.0, 0.4, 0.0, 0.0],
                        [0.4, 2.0, 0.0, 0.0],
                        [0.0, 0.0, 0.2, 0.1],
                        [0.0, 0.0, 0.1, 0.4]]
        expected_rect = PolyLineDS()
        expected_rect.add_point(7.0, 3.0, 0.0, 4.0, expected_cov)
        expected_rect.add_point(7.0, 7.0, 0.0, 4.0, expected_cov)
        expected_rect.add_point(5.0, 7.0, 0.0, 4.0, expected_cov)
        expected_rect.add_point(5.0, 3.0, 0.0, 4.0, expected_cov)
        expected_rect.add_point(7.0, 3.0, 0.0, 4.0, expected_cov)

        single_obj_series = pd.Series(single_row_dict)
        observed_rect = object_into_polyline(single_obj_series)

        pd.testing.assert_frame_equal(expected_rect.signals, observed_rect.signals)

    def test_check_for_object_non_zero_yaw_rate(self, allowed_diff):
        single_row_dict = {
            'id': 0,
            'position_x': 6.0,
            'position_y': 5.0,
            'position_variance_x': 1.0,
            'position_variance_y': 2.0,
            'position_covariance': 0.4,
            'velocity_otg_x': 0.0,
            'velocity_otg_y': 4.0,
            'velocity_otg_variance_x': 0.2,
            'velocity_otg_variance_y': 0.4,
            'velocity_otg_covariance': 0.1,
            'bounding_box_dimensions_x': 4.0,
            'bounding_box_dimensions_y': 2.0,
            'bounding_box_orientation': np.deg2rad(90.0),
            'bounding_box_refpoint_long_offset_ratio': 0.5,
            'bounding_box_refpoint_lat_offset_ratio': 0.5,
            'yaw_rate': 1.5,  # [rad/s]
        }

        expected_cov = [[1.0, 0.4, 0.0, 0.0],
                        [0.4, 2.0, 0.0, 0.0],
                        [0.0, 0.0, 0.2, 0.1],
                        [0.0, 0.0, 0.1, 0.4]]
        expected_rect = PolyLineDS()
        expected_rect.add_point(7.0, 3.0, 3.0,  5.5, expected_cov)
        expected_rect.add_point(7.0, 7.0, -3.0, 5.5, expected_cov)
        expected_rect.add_point(5.0, 7.0, -3.0, 2.5, expected_cov)
        expected_rect.add_point(5.0, 3.0, 3.0,  2.5, expected_cov)
        expected_rect.add_point(7.0, 3.0, 3.0,  5.5, expected_cov)

        single_obj_series = pd.Series(single_row_dict)
        observed_rect = object_into_polyline(single_obj_series)

        pd.testing.assert_frame_equal(expected_rect.signals, observed_rect.signals)

    def test_check_for_object_with_ref_point_in_corner(self, allowed_diff):
        single_row_dict = {
            'id': 0,
            'position_x': 6.0,
            'position_y': 5.0,
            'position_variance_x': 1.0,
            'position_variance_y': 2.0,
            'position_covariance': 0.4,
            'velocity_otg_x': 4.0,
            'velocity_otg_y': 0.0,
            'velocity_otg_variance_x': 0.2,
            'velocity_otg_variance_y': 0.4,
            'velocity_otg_covariance': 0.1,
            'bounding_box_dimensions_x': 4.0,
            'bounding_box_dimensions_y': 2.0,
            'bounding_box_orientation': np.deg2rad(0.0),
            'bounding_box_refpoint_long_offset_ratio': 1.0,
            'bounding_box_refpoint_lat_offset_ratio': 0.0,
            'yaw_rate': np.deg2rad(0.0),  # [rad/s]
        }

        expected_cov = [[1.0, 0.4, 0.0, 0.0],
                        [0.4, 2.0, 0.0, 0.0],
                        [0.0, 0.0, 0.2, 0.1],
                        [0.0, 0.0, 0.1, 0.4]]
        expected_rect = PolyLineDS()
        expected_rect.add_point(2.0, 5.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(6.0, 5.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(6.0, 7.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(2.0, 7.0, 4.0, 0.0, expected_cov)
        expected_rect.add_point(2.0, 5.0, 4.0, 0.0, expected_cov)

        single_obj_series = pd.Series(single_row_dict)
        observed_rect = object_into_polyline(single_obj_series)

        pd.testing.assert_frame_equal(expected_rect.signals, observed_rect.signals)