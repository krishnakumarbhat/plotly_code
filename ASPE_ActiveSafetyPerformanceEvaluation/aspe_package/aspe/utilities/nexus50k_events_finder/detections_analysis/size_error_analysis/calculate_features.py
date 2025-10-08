import numpy as np
import pandas as pd

from aspe.utilities.MathFunctions import normalize_angle_vector


def calculate_aspect_angle(df: pd.DataFrame, use_degrees=False):
    angular_pos = np.arctan2(df.center_y.to_numpy(), df.center_x.to_numpy())
    asp_angle = normalize_angle_vector(angular_pos - df.bounding_box_orientation.to_numpy())
    df['aspect_angle'] = asp_angle

    if use_degrees:
        df['aspect_angle'] = np.rad2deg(df['aspect_angle'])


def calculate_speed(df: pd.DataFrame):
    df['speed'] = np.hypot(df['velocity_otg_x'].to_numpy(),
                           df['velocity_otg_y'].to_numpy())


def calculate_range(df: pd.DataFrame):
    df['range'] = np.hypot(df['center_x'].to_numpy(), df['center_y'].to_numpy())
