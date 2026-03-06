"""
File Name: poi.py
Author: Bharanidharan Subramani
Email : Bharanidharan.s@aptiv.com
Description:
This module hold dictionary where point of interested signal for
plotting to updated for the same.
"""

datasource = None
poi_data = {
    "project": "GEN7V2",
    "stream_version": "v15",
    "streams": ["DETECTION_STREAM/AF_Det", "DETECTION_STREAM/Stream_Hdr",
                '04_OLP', 'DOWN_SELECTION_STREAM/AF_DS_Det', 'DOWN_SELECTION_STREAM/Stream_Hdr'
                ],

    "sensors": ["RL", "RR", "FL", "FR"],

    "DETECTION_STREAM/AF_Det": [
        {"name": "ran", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vel", "pname": "rangerate", "unit": "m/s", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "phi", "pname": "phi", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "theta", "pname": "theta", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "snr", "pname": "snr", "unit": "dB", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "rcs", "pname": "rcs", "unit": "m2", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "DETECTION_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC"]},

    ],

    '04_OLP': [
        {'name': 'vcs_pos_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_pos_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_vel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_vel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_accel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_accel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'f_moveable', 'pname': 'f_moveable', 'unit': 'm', 'plots': ['SC']},
        {'name': 'f_stationary', 'pname': 'f_stationary', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_pos_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_pos_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_vel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_vel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'age', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_2wheel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_car', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_pedestrian', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_truck', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_heading', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'eclipse_value', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},

    ],

    "DOWN_SELECTION_STREAM/AF_DS_Det": [
        {"name": "ran", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vel", "pname": "rangerate", "unit": "m/s", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "phi", "pname": "phi", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "theta", "pname": "theta", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "snr", "pname": "snr", "unit": "dB", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "rcs", "pname": "rcs", "unit": "m2", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "DOWN_SELECTION_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC"]},

    ],

}

poi_data_DC = {

    'streams': ['02_Input_DC_Data/Detections/Detection_Info', '04_OLP', '03_TrackerInfo/Tracker_Input/VSE',
                '03_TrackerInfo/Tracker_Output/ROT', '00_metadata', '03_TrackerInfo/Tracker_Output/AllObjects'],

    '02_Input_DC_Data/Detections/Detection_Info': [

        {'name': 'range', 'pname': 'range', 'unit': 'm', 'plots': ['SC', 'HS', 'BPMP']},
        {'name': 'range_rate', 'pname': 'range_rate', 'unit': 'm/s', 'plots': ['SC']},
        {'name': 'snr', 'pname': 'range_rate', 'unit': 'db', 'plots': ['SC']},
        {'name': 'azimuth', 'pname': 'range_rate', 'unit': 'rad', 'plots': ['SC']},
        {'name': 'elevation', 'pname': 'range_rate', 'unit': 'rad', 'plots': ['SC']},
        {'name': 'amplitude', 'pname': 'range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'valid', 'pname': 'range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'azimuth_confidence', 'pname': 'range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'elevation_confidence', 'pname': 'range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'std_range', 'pname': 'std_range', 'unit': 'm', 'plots': ['SC']},
        {'name': 'std_range_rate', 'pname': 'std_range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'std_rcs', 'pname': 'std_range', 'unit': 'm', 'plots': ['SC']},
        {'name': 'std_azimuth', 'pname': 'std_range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'std_elevation', 'pname': 'std_range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'bistatic', 'pname': 'std_range_rate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'existence_probability', 'pname': 'std_range_rate', 'unit': 'm', 'plots': ['SC']}

    ],

    '04_OLP': [
        {'name': 'vcs_pos_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_pos_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_vel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_vel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_accel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_accel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'f_moveable', 'pname': 'f_moveable', 'unit': 'm', 'plots': ['SC']},
        {'name': 'f_stationary', 'pname': 'f_stationary', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_pos_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_pos_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_vel_x', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_vel_y', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'age', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_2wheel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_car', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_pedestrian', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'class_prob_truck', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'curvi_heading', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'eclipse_value', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},

    ],

    '03_TrackerInfo/Tracker_Input/VSE': [
        {'name': 'raw_speed_mps', 'pname': 'raw_speed_mps', 'unit': 'm', 'plots': ['SC']},
        {'name': 'raw_speed_qf', 'pname': 'raw_speed_qf', 'unit': 'm', 'plots': ['SC']},
        {'name': 'raw_steering_angle_deg', 'pname': 'raw_steering_angle_deg', 'unit': 'm', 'plots': ['SC']},
        {'name': 'raw_steering_angle_qf', 'pname': 'raw_steering_angle_qf', 'unit': 'm', 'plots': ['SC']},
        {'name': 'raw_yaw_rate_qf', 'pname': 'raw_steering_angle_qf', 'unit': 'm', 'plots': ['SC']},
        {'name': 'raw_yaw_rate_rps', 'pname': 'raw_steering_angle_qf', 'unit': 'm', 'plots': ['SC']},
        {'name': 'road_wheel_angle_deg', 'pname': 'road_wheel_angle_deg', 'unit': 'm', 'plots': ['SC']},
        {'name': 'road_wheel_angle_qf', 'pname': 'road_wheel_angle_qf', 'unit': 'm', 'plots': ['SC']},
        {'name': 'sensor_sideslip', 'pname': 'sensor_sideslip', 'unit': 'm', 'plots': ['SC']},

    ],

    '03_TrackerInfo/Tracker_Output/ROT': [
        {'name': 'length', 'pname': 'length', 'unit': 'm', 'plots': ['SC']}

    ],
    '00_metadata': [
        {'name': 'Created_datetime', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},
        {'name': 'DC_version', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},
        {'name': 'OCG_version', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},
        {'name': 'OLP_version', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},
        {'name': 'SFL_version', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},
        {'name': 'Tracker_version', 'pname': 'length', 'unit': 'm', 'plots': ['SC']},

    ],

    '03_TrackerInfo/Tracker_Output/AllObjects': [
        {'name': 'vcs_xposn', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_yposn', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_xvel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_yvel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_xaccel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},
        {'name': 'vcs_yaccel', 'pname': 'yawrate', 'unit': 'm', 'plots': ['SC']},

    ],

}
poi_data_MCIP_CAN = {
    "project": "GEN7V2",
    "stream_version": "v15",
    "streams": ["SRR_FR_DETECTION", "SRR_FL_DETECTION", "SRR_RR_DETECTION", "SRR_RL_DETECTION", "FLR_DETECTION"

                ],

    "sensors": ["MCIP_FR", "MCIP_FL", "MCIP_RR", "MCIP_RL", "MCIP_FLR"],

    "SRR_FR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_FR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_FL_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_FL_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_RR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_RR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_RL_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_RL_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
    ],
    "FLR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_FLR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ]

}

poi_data_CEER_CAN = {
    "project": "GEN7V2",
    "stream_version": "v15",
    "streams": ["SRR_FR_DETECTION", "SRR_FL_DETECTION", "SRR_RR_DETECTION", "SRR_RL_DETECTION", "FLR_DETECTION"

                ],

    "sensors": ["CEER_FR", "CEER_FL", "CEER_RR", "CEER_RL", "CEER_FLR"],

    "SRR_FR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_FR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_FL_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_FL_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_RR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_RR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ],
    "SRR_RL_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_SRR_RL_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
    ],
    "FLR_DETECTION": [
        {"name": "DET_RANGE", "pname": "Range", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},
        {"name": "DET_RCS", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_AZIMUTH", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_ELEVATION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_RANGE_VELOCITY", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "DET_SNR", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "timestamp_FLR_DETECTION", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},

    ]

}
