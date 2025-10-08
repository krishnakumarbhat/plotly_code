import pandas as pd
import numpy as np

defualt_sensor_id = 1
front_right_sensor_id = 2

sensor_series_zero_motion = pd.Series({
    'scan_index': 0,
    'timestamp': 0,
    'slot_id': 0,
    'unique_id': 0,
    'sensor_id': defualt_sensor_id,
    'look_id': 0,
    'look_index': 0,
    'velocity_otg_x': 0.0,
    'velocity_otg_y': 0.0,
    'velocity_otg_variance_x': 0.01,
    'velocity_otg_variance_y': 0.01,
    'velocity_otg_covariance': 0.0,
    'yaw_rate': 0.0,  # [rad/s]
    'yaw_rate_variance': np.deg2rad(1)  # [rad^2/s^2]
})

default_sensor_per_sensor = pd.DataFrame([{
    'sensor_id': defualt_sensor_id,
    'sensor_type': 0,
    'polarity': 0,
    'mount_location': 0.0,
    'boresight_az_angle': 0.0,
    'boresight_elev_angle': 0.0,
    'position_x': 0.0,
    'position_y': 0.0
}])

front_right_sensor_per_sensor = pd.DataFrame([{
    'sensor_id': front_right_sensor_id,
    'sensor_type': 0,
    'polarity': 0,
    'mount_location': 0.0,
    'boresight_az_angle': np.deg2rad(60.0),
    'boresight_elev_angle': 0.0,
    'position_x': -0.5,
    'position_y': 0.8
}])

default_sensor_per_look = pd.DataFrame([{
    'sensor_id': defualt_sensor_id,
    'look_id': 0,
    'max_range': 15.0,
    'min_elevation': np.deg2rad(-10.0),
    'max_elevation': np.deg2rad(10.0),
    'min_azimuth': np.deg2rad(-75.0),
    'max_azimuth': np.deg2rad(75.0),
    'range_rate_interval_width': 50.0,
}])

default_detection = pd.Series({
    'scan_index': 0,
    'timestamp': 0.0,
    'slot_id': 0,
    'unique_id': 0,
    'sensor_id': defualt_sensor_id,
    'range': 0.0,
    'range_variance': 0.1**2,
    'azimuth': 0.0,
    'azimuth_variance': np.deg2rad(0.333)**2,
    'elevation': 0.00,
    'elevation_variance': np.deg2rad(1.0)**2,
    'range_rate': 0.0,
    'range_rate_variance': 0.05**2,
    'amplitude': 10.0,
    'position_x': 0.0,
    'position_y': 0.0,
})

default_object = pd.Series({
    'scan_index': 0,
    'timestamp': 0.0,
    'slot_id': 0,
    'unique_id': 0,
    'position_x': 0.0,
    'position_y': 0.0,
    'position_variance_x': 0.01,
    'position_variance_y': 0.01,
    'position_covariance': 0.0,
    'velocity_otg_x': 0.0,
    'velocity_otg_y': 0.0,
    'velocity_otg_variance_x': 0.01,
    'velocity_otg_variance_y': 0.01,
    'velocity_otg_covariance': 0.0,
    'bounding_box_dimensions_x': 4.0,
    'bounding_box_dimensions_y': 2.0,
    'bounding_box_orientation': np.deg2rad(0.0),
    'bounding_box_refpoint_long_offset_ratio': 0.5,
    'bounding_box_refpoint_lat_offset_ratio': 0.5,
    'yaw_rate': np.deg2rad(0.0),  # [rad/s]
    'yaw_rate_variance': np.deg2rad(1),  # [rad^2/s^2]
    'center_x': 0.0,
    'center_y': 0.0,
})

