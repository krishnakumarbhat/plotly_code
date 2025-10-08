import numpy as np
from aspe.utilities.MathFunctions import calc_position_in_bounding_box, calc_velocity_in_position


class TestsCalcPositionInBoundingBox:
    """
    Class for testing utilities.math_functions.calc_position_in_bounding_box.
    """
    def test_ref_point_corner_cases_for_center_position_calculation(self):
        position_lateral = np.array([10, 10, 10, 10, 10])
        position_longitudinal = np.array([10, 10, 10, 10, 10])
        ref_point_lateral = np.array([0, 0, 1, 1, 0.5])
        ref_point_longitudinal = np.array([0, 1, 0, 1, 0.5])
        bbox_width = np.array([2, 2, 2, 2, 2])
        bbox_length = np.array([4, 4, 4, 4, 4])
        pointing_angle = np.array([0, 0, 0, 0, 0])

        point_to_calculate_lon = np.array([0.5, 0.5, 0.5, 0.5, 0.5])
        point_to_calculate_lat = np.array([0.5, 0.5, 0.5, 0.5, 0.5])

        expected_lateral = np.array([11, 11, 9, 9, 10])
        expected_longitudinal = np.array([12, 8, 12, 8, 10])

        new_lon, new_lat = calc_position_in_bounding_box(position_longitudinal, position_lateral,
                                                         bbox_length, bbox_width, pointing_angle,
                                                         ref_point_longitudinal, ref_point_lateral,
                                                         point_to_calculate_lon, point_to_calculate_lat)
        np.testing.assert_array_equal((expected_lateral, expected_longitudinal), (new_lat, new_lon))

    def test_90_deg_rotation_for_center_position_calculation(self):
        position_lateral = np.array([10, 10, 10, 10, 10])
        position_longitudinal = np.array([10, 10, 10, 10, 10])
        ref_point_lateral = np.array([0, 0, 1, 1, 0.5])
        ref_point_longitudinal = np.array([0, 1, 0, 1, 0.5])
        bbox_width = np.array([2, 2, 2, 2, 2])
        bbox_length = np.array([4, 4, 4, 4, 4])
        pointing_angle = np.array([np.pi/2, np.pi/2, np.pi/2, np.pi/2, np.pi/2])

        point_to_calculate_lon = np.array([0.5, 0.5, 0.5, 0.5, 0.5])
        point_to_calculate_lat = np.array([0.5, 0.5, 0.5, 0.5, 0.5])

        expected_lateral = np.array([12, 8, 12, 8, 10])
        expected_longitudinal = np.array([9, 9, 11, 11, 10])

        new_lon, new_lat = calc_position_in_bounding_box(position_longitudinal, position_lateral,
                                                         bbox_length, bbox_width, pointing_angle,
                                                         ref_point_longitudinal, ref_point_lateral,
                                                         point_to_calculate_lon, point_to_calculate_lat)
        np.testing.assert_array_equal((expected_lateral, expected_longitudinal), (new_lat, new_lon))

    def test_negative_90_deg_rotation_for_center_position_calculation(self):
        position_lateral = np.array([10, 10, 10, 10, 10])
        position_longitudinal = np.array([10, 10, 10, 10, 10])
        ref_point_lateral = np.array([0, 0, 1, 1, 0.5])
        ref_point_longitudinal = np.array([0, 1, 0, 1, 0.5])
        bbox_width = np.array([2, 2, 2, 2, 2])
        bbox_length = np.array([4, 4, 4, 4, 4])
        pointing_angle = np.array([-np.pi/2, -np.pi/2, -np.pi/2, -np.pi/2, -np.pi/2])

        point_to_calculate_lon = np.array([0.5, 0.5, 0.5, 0.5, 0.5])
        point_to_calculate_lat = np.array([0.5, 0.5, 0.5, 0.5, 0.5])

        expected_lateral = np.array([8, 12, 8, 12, 10])
        expected_longitudinal = np.array([11, 11, 9, 9, 10])

        new_lon, new_lat = calc_position_in_bounding_box(position_longitudinal, position_lateral,
                                                         bbox_length, bbox_width, pointing_angle,
                                                         ref_point_longitudinal, ref_point_lateral,
                                                         point_to_calculate_lon, point_to_calculate_lat)
        np.testing.assert_array_equal((expected_lateral, expected_longitudinal), (new_lat, new_lon))


class TestCalcVelocityInPosition:

    def test_check_for_the_same_position(self):
        pos_x = np.array([10.0])
        pos_y = np.array([15.0])

        vel_x = np.array([15.0])
        vel_y = np.array([15.0])

        yaw_rate = np.array([3.0])  # rad/s

        vel_out_x, vel_out_y = calc_velocity_in_position(pos_x, pos_y, vel_x, vel_y, yaw_rate, pos_x, pos_y)

        assert np.allclose(vel_out_x, vel_x)
        assert np.allclose(vel_out_y, vel_y)

    def test_check_for_center_to_corner_with_no_yaw_rate(self):
        pos_x = np.array([10.0, 10.0, 10.0, 10.0])
        pos_y = np.array([15.0, 15.0, 15.0, 15.0])

        vel_x = np.array([5.0, 5.0, 5.0, 5.0])
        vel_y = np.array([2.0, 2.0, 2.0, 2.0])

        yaw_rate = np.array([0.0])  # rad/s

        pos_x_out = np.array([8.0, 12.0, 12.0, 8.0])
        pos_y_out = np.array([14.0, 14.0, 15.0, 15.0])

        vel_out_x, vel_out_y = calc_velocity_in_position(pos_x, pos_y, vel_x, vel_y, yaw_rate, pos_x_out, pos_y_out)

        assert np.allclose(vel_out_x, vel_x)
        assert np.allclose(vel_out_y, vel_y)

    def test_check_for_center_to_corner_with_positive_yaw_rate(self):
        pos_x = np.array([10.0, 10.0, 10.0, 10.0])
        pos_y = np.array([15.0, 15.0, 15.0, 15.0])

        vel_x = np.array([5.0, 5.0, 5.0, 5.0])
        vel_y = np.array([2.0, 2.0, 2.0, 2.0])

        yaw_rate = np.array([2.0])  # rad/s

        pos_x_out = np.array([8.0, 12.0, 12.0, 8.0])
        pos_y_out = np.array([14.0, 14.0, 16.0, 16.0])

        expected_vel_x = np.array([7.0, 7.0, 3.0, 3.0])
        expected_vel_y = np.array([-2.0, 6.0, 6.0, -2.0])

        vel_out_x, vel_out_y = calc_velocity_in_position(pos_x, pos_y, vel_x, vel_y, yaw_rate, pos_x_out, pos_y_out)

        assert np.allclose(vel_out_x, expected_vel_x)
        assert np.allclose(vel_out_y, expected_vel_y)

    def test_check_for_center_to_corner_with_negative_yaw_rate(self):
        pos_x = np.array([10.0, 10.0, 10.0, 10.0])
        pos_y = np.array([15.0, 15.0, 15.0, 15.0])

        vel_x = np.array([5.0, 5.0, 5.0, 5.0])
        vel_y = np.array([2.0, 2.0, 2.0, 2.0])

        yaw_rate = np.array([-1.0])  # rad/s

        pos_x_out = np.array([8.0, 12.0, 12.0, 8.0])
        pos_y_out = np.array([14.0, 14.0, 16.0, 16.0])

        expected_vel_x = np.array([4.0, 4.0, 6.0, 6.0])
        expected_vel_y = np.array([4.0, 0.0, 0.0, 4.0])

        vel_out_x, vel_out_y = calc_velocity_in_position(pos_x, pos_y, vel_x, vel_y, yaw_rate, pos_x_out, pos_y_out)

        assert np.allclose(vel_out_x, expected_vel_x)
        assert np.allclose(vel_out_y, expected_vel_y)


