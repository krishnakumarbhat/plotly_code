import numpy as np
import pandas as pd

from aspe.utilities.MathFunctions import normalize_angle_vector


def calc_aspect_angle(auto_gt: pd.DataFrame):
    def wrap_asp_angle_to_90(asp_angle: np.ndarray):
        abs_angle = np.abs(asp_angle)
        over_90 = abs_angle >= (np.pi)
        out = np.where(over_90, abs_angle - 2*(abs_angle % (np.pi)), abs_angle)
        return out

    angular_pos = np.arctan2(auto_gt.center_y.to_numpy(), (auto_gt.center_x.to_numpy() + 2.6))
    asp_angle_raw = normalize_angle_vector((angular_pos - auto_gt.bounding_box_orientation.to_numpy()))

    auto_gt['aspect_angle'] = np.rad2deg(wrap_asp_angle_to_90(asp_angle_raw))


def filter_only_cars(auto_gt: pd.DataFrame):
    return auto_gt.loc[auto_gt.object_class == 'ObjectClass.CAR', :]


def filter_only_same_dir(auto_gt):
    auto_gt.query('velocity_otg_x > 0.0', inplace=True)


def filter_only_moving(auto_gt: pd.DataFrame):
    return auto_gt.loc[auto_gt.movement_status == 'MovementStatus.MOVING', :]


def calc_speed(auto_gt: pd.DataFrame):
    auto_gt.loc[:, 'speed'] = np.hypot(auto_gt.loc[:, 'velocity_otg_x'].to_numpy(),
                                       auto_gt.loc[:, 'velocity_otg_y'].to_numpy())


def calc_range(auto_gt):
    """Calc range from center of host instead of center of front bumper"""
    auto_gt.loc[:, 'range'] = \
        np.hypot(auto_gt.loc[:, 'center_x'].to_numpy() + 2.6, auto_gt.loc[:, 'center_y'].to_numpy())