import numpy as np
import pandas as pd
from scipy.io import loadmat

from aspe.utilities.MathFunctions import calc_position_in_bounding_box, normalize_angle_vector


def transform_rt_range_data_mat2data_frame(mat_path, rt_ref_x=1.0, rt_ref_y=0.5, ref_length=5.0, ref_width=2.0, include_host_yawing = True):
    rt_data = loadmat(mat_path)
    rt_out = {}
    for key, value in rt_data.items():
        if isinstance(value, np.ndarray):
            rt_out[key] = value.reshape(-1)
    rt_df = pd.DataFrame(rt_out)

    translate_dict = {
        'Range1Forward__Range1PosForward': 'position_x',
        'Range1Lateral__Range1PosLateral': 'position_y',
        'meas_time': 'timestamp',
    }

    for rt_signature, aspe_signature in translate_dict.items():
        rt_df[aspe_signature] = rt_df[rt_signature]

    rt_df['position_y'] = -rt_df['position_y']
    orientation = rt_df['Target1HeadingPitchRoll__Target1AngleHeading'] - \
                  rt_df['HunterHeadingPitchRoll__HunterAngleHeading']

    orientation_norm = normalize_angle_vector(orientation)
    rt_df['bounding_box_orientation'] = -orientation_norm

    rt_df['bounding_box_dimensions_x'] = ref_length
    rt_df['bounding_box_dimensions_y'] = ref_width

    host_vel_x = rt_df['HunterVelocityLevel__HunterVelForward']
    host_vel_y = -rt_df['HunterVelocityLevel__HunterVelLateral']

    target_vel_x_relative = rt_df['Range1Forward__Range1VelForward']
    target_vel_y_relative = -rt_df['Range1Lateral__Range1VelLateral']

    host_yaw_rate = -rt_df['HunterRateVehicle__HunterAngRateZ']
    target_yaw_rate = -rt_df['Target1RateVehicle__Target1AngRateZ']
    center_x, center_y = calc_position_in_bounding_box(rt_df['position_x'].to_numpy(),
                                                       rt_df['position_y'].to_numpy(),
                                                       rt_df['bounding_box_dimensions_x'].to_numpy(),
                                                       rt_df['bounding_box_dimensions_y'].to_numpy(),
                                                       rt_df['bounding_box_orientation'].to_numpy(),
                                                       np.full(len(rt_df), rt_ref_x),
                                                       np.full(len(rt_df), rt_ref_y),
                                                       np.full(len(rt_df), 0.5),
                                                       np.full(len(rt_df), 0.5))
    rt_df['center_x'] = center_x
    rt_df['center_y'] = center_y

    if include_host_yawing:
        host_yaw_infl_y = (rt_df['position_x']) * host_yaw_rate
        host_yaw_infl_x = -rt_df['position_y'] * host_yaw_rate

        rt_df['velocity_otg_x'] = host_vel_x + target_vel_x_relative + host_yaw_infl_x
        rt_df['velocity_otg_y'] = host_vel_y + target_vel_y_relative + host_yaw_infl_y

        rt_df['host_yaw_infl_y'] = host_yaw_infl_y
        rt_df['host_yaw_infl_x'] = host_yaw_infl_x
    else:
        rt_df['velocity_otg_x'] = host_vel_x + target_vel_x_relative
        rt_df['velocity_otg_y'] = host_vel_y + target_vel_y_relative

        rt_df['host_yaw_infl_y'] = 0.0
        rt_df['host_yaw_infl_x'] = 0.0

    rt_df['host_vel_abs_x'] = host_vel_x
    rt_df['host_vel_abs_y'] = host_vel_y
    rt_df['target_vel_rel_x'] = target_vel_x_relative
    rt_df['target_vel_rel_y'] = target_vel_y_relative
    rt_df['yaw_rate'] = target_yaw_rate
    rt_df['speed'] = np.sqrt(np.square(rt_df['velocity_otg_x']) + np.square(rt_df['velocity_otg_y']))

    return rt_df


if __name__ == '__main__':
    rt_data = transform_rt_range_data_mat2data_frame(r"C:\logs\BYK-589_DEX-530\ANALYSIS_362\rt_range_data.mat")

