from typing import Union

import numpy as np
import pandas as pd

from aspe.extractors.Transform.veloicty import otg_to_rel_vel
from aspe.utilities.MathFunctions import calc_position_in_bounding_box, calc_velocity_in_position


def calc_object_rel_state_at_ref_point(object_signals: Union[pd.Series, pd.DataFrame],
                                       cs_motion: Union[pd.Series, pd.DataFrame],
                                       ref_point_x: np.array,
                                       ref_point_y: np.array) \
        -> Union[pd.Series, pd.DataFrame]:
    """
    Transforming object state (position and velocity) from Over-The-Ground frame to relative frame.
    It gives possibility to do it for any given relative reference point on object.
    Note:
        velocity_otg_x and velocity_otg_y are required and as an output from transformation
        velocity_rel_x and velocity_rel_y is returned
    :param object_signals: Object signals containing minimum:
           .position_x
           .position_y
           .bounding_box_dimensions_x
           .bounding_box_dimensions_y
           .bounding_box_orientation
           .bounding_box_refpoint_long_offset_ratio
           .bounding_box_refpoint_lat_offset_ratio
           .velocity_otg_x
           .velocity_otg_y
           .yaw_rate

    :type object_signals: Union[pd.Series, pd.DataFrame]
    :param cs_motion: motion of coordinate system frame containing minimum:
           .velocity_otg_x
           .velocity_otg_y
           .yaw_rate
    :type cs_motion: Union[pd.Series, pd.DataFrame]
    :param ref_point_x: array of x reference point for which state should be calculated - can be single value
    :type ref_point_x: np.array
    :param ref_point_y: array of y reference point for which state should be calculated - can be single value
    :type ref_point_y: np.array
    :return: Union[pd.Series, pd.DataFrame] relative state containing:
             .position_x
             .position_y
             .velocity_rel_x
             .velocity_rel_y
    """
    position_x, position_y = calc_position_in_bounding_box(object_signals.position_x, object_signals.position_y,
                                                           object_signals.bounding_box_dimensions_x,
                                                           object_signals.bounding_box_dimensions_y,
                                                           object_signals.bounding_box_orientation,
                                                           object_signals.bounding_box_refpoint_long_offset_ratio,
                                                           object_signals.bounding_box_refpoint_lat_offset_ratio,
                                                           ref_point_x, ref_point_y)

    # Target yaw rate influence
    velocity_otg_x, velocity_otg_y = calc_velocity_in_position(object_signals.position_x,
                                                               object_signals.position_y,
                                                               object_signals.velocity_otg_x,
                                                               object_signals.velocity_otg_y,
                                                               object_signals.yaw_rate,
                                                               position_x, position_y)

    # Relative velocity calculation
    velocity_rel_x, velocity_rel_y = otg_to_rel_vel(velocity_otg_x, velocity_otg_y,
                                                    cs_motion.velocity_otg_x, cs_motion.velocity_otg_y,
                                                    cs_motion.yaw_rate,
                                                    position_x, position_y)

    data = {'position_x': position_x,
            'position_y': position_y,
            'velocity_rel_x': velocity_rel_x,
            'velocity_rel_y': velocity_rel_y}

    if isinstance(object_signals, pd.DataFrame):
        rel_state = pd.DataFrame(data)
    else:
        rel_state = pd.Series(data)

    return rel_state
