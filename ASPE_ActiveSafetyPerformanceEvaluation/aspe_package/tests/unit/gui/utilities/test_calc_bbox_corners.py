import numpy as np
from aspe.gui.utilities.calc_bbox_corners import calc_bbox_corners_for_front_center_position


def test_calc_bbox_corners_for_front_center_position():
    position_x = np.array([-5])
    position_y = np.array([0])
    orientation = np.array([0.5])
    length = np.array([10])
    width = np.array([5])

    corners_x, corners_y = calc_bbox_corners_for_front_center_position(
        position_x, position_y, orientation, length, width
    )

    # Assert the shape of the output arrays
    assert corners_x.shape == (6,)
    assert corners_y.shape == (6,)

    # Assert the values of the output arrays
    expected_corners_x = np.array([-12.57726177, - 14.97438947, - 6.19856385, - 3.80143615, - 12.57726177,  np.nan])
    expected_corners_y = np.array([-6.98821179, -2.60029898,  2.1939564,  -2.1939564,  -6.98821179,         np.nan])

    diff = corners_y - expected_corners_y
    print("Difference between the arrays:")
    print(diff)

    assert np.allclose(corners_x[:5], expected_corners_x[:5],  atol=0.01)
    assert np.allclose(corners_y[:5], expected_corners_y[:5], atol=0.01)


