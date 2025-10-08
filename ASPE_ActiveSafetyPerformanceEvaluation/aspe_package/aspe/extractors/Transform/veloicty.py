import numpy as np

from aspe.utilities.MathFunctions import calc_velocity_in_position


def otg_to_rel_vel(velocity_otg_x, velocity_otg_y,
                   cs_origin_velocity_otg_x, cs_origin_velocity_otg_y, cs_yaw_rate,
                   position_x, position_y):
    """
    Calculate relative (rel) velocity in coordinate system (cs) based on over-the-ground (otg) velocity using rigid
    body theory
    :param velocity_otg_x:
    :param velocity_otg_y:
    :param cs_origin_velocity_otg_x:
    :param cs_origin_velocity_otg_y:
    :param cs_yaw_rate:
    :param position_x:
    :param position_y:
    :return:
    """
    cs_yr_influence_x, cs_yr_influence_y = calc_velocity_in_position(0.0, 0.0, 0.0, 0.0, cs_yaw_rate,
                                                                     position_x, position_y)

    velocity_rel_x = velocity_otg_x - cs_origin_velocity_otg_x - cs_yr_influence_x
    velocity_rel_y = velocity_otg_y - cs_origin_velocity_otg_y - cs_yr_influence_y

    return velocity_rel_x, velocity_rel_y
