datasource = None
poi_data = {
    "project": "GEN7V2",
    "stream_version": "v15",
    "streams": ["DETECTION_STREAM/Stream_Hdr", "DETECTION_STREAM/AF_Det",
                "VSE_STREAM/Veh_Info", "VSE_STREAM/Stream_Hdr",
                "TOI_STREAM/TOI_Data", "TOI_STREAM/Stream_Hdr",
                "HEADER_STREAM/Stream_Hdr", "HEADER_STREAM",
                "DOWN_SELECTION_STREAM/Stream_Hdr", "DOWN_SELECTION_STREAM/AF_DS_Det",
                "STATUS_STREAM",
                "ALIGNMENT_STREAM/Stream_Hdr", "ALIGNMENT_STREAM/"

                ],

    "sensors": ["FC", "FL", "FR", "RL", "RR"],

    "DETECTION_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},

    ],

    "DETECTION_STREAM/AF_Det": [
        {"name": "ran", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vel", "pname": "rangerate", "unit": "m/s", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "phi", "pname": "phi", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "theta", "pname": "theta", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "snr", "pname": "snr", "unit": "dB", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "rcs", "pname": "rcs", "unit": "m2", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "VSE_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC"]},

    ],

    "VSE_STREAM/Veh_Info": [
        {"name": "veh_speed", "pname": "speed", "unit": "km/s", "plots": ["SC"]},
        {"name": "veh_steering_angle", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "veh_yaw", "pname": "yaw", "unit": "deg", "plots": ["SC"]},
        {"name": "veh_pitch", "pname": "pitch", "unit": "deg", "plots": ["SC"]},
        {"name": "veh_roll", "pname": "roll", "unit": "deg", "plots": ["SC"]}

    ],

    "TOI_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC"]},

    ],

    "TOI_STREAM/TOI_Data": [
        {"name": "std_ran", "pname": "speed", "unit": "m", "plots": ["SC"]},
        {"name": "std_phi_1", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "std_vel_1", "pname": "yaw", "unit": "m/s", "plots": ["SC"]},
        {"name": "std_theta_1", "pname": "pitch", "unit": "deg", "plots": ["SC"]},

    ],
    "HEADER_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC"]},

    ],

    "HEADER_STREAM": [
        {"name": "det_range_max", "pname": "speed", "unit": "m", "plots": ["SC"]},
        {"name": "det_range_min", "pname": "steering angle", "unit": "m", "plots": ["SC"]},
        {"name": "sensor_fov_hor_max", "pname": "speed", "unit": "fov_hor_max", "plots": ["SC"]},
        {"name": "sensor_fov_hor_min", "pname": "steering angle", "unit": "fov_hor_min", "plots": ["SC"]},
        {"name": "speed_range_max", "pname": "speed", "unit": "km/s", "plots": ["SC"]},
        {"name": "speed_range_min", "pname": "steering angle", "unit": "km/s", "plots": ["SC"]},

    ],

    "DOWN_SELECTION_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},

    ],

    "DOWN_SELECTION_STREAM/AF_DS_Det": [
        {"name": "ran", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vel", "pname": "rangerate", "unit": "m/s", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "phi", "pname": "phi", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "theta", "pname": "theta", "unit": "deg", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "snr", "pname": "snr", "unit": "dB", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "rcs", "pname": "rcs", "unit": "m2", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "STATUS_STREAM": [
        {"name": "sensor_mount_ornt_pitch", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "sensor_mount_ornt_yaw", "pname": "rangerate", "unit": "m/s",
         "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},

    ],

    "ALIGNMENT_STREAM/Stream_Hdr": [
        {"name": "scan_index", "pname": "scan_index", "unit": "index", "plots": ["SC", "SC_MM", "Bar_MM"]},

    ],

    "ALIGNMENT_STREAM/Veh_Info": [
        {"name": "vacs_boresight_az_initial", "pname": "speed", "unit": "km/s", "plots": ["SC"]},
        {"name": "vacs_boresight_el_initial", "pname": "steering angle", "unit": "deg", "plots": ["SC"]},
        {"name": "vacs_boresight_az_estimated", "pname": "yaw", "unit": "deg", "plots": ["SC"]},
        {"name": "vacs_boresight_el_estimated", "pname": "pitch", "unit": "deg", "plots": ["SC"]},

    ]

}

poi_data_DC = {

    "streams": ["Tracker_Information/OLP", "OSI_Ground_Truth/Object", "Raw_Detection_Information"
                ],

    "Tracker_Information/OLP": [
        {"name": "vcs_pos_x", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_pos_y", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_vel_x", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_vel_y", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_accel_x", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_accel_y", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "OSI_Ground_Truth/Object": [
        {"name": "vcs_long_posn", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_lat_posn", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_long_vel", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_lat_vel", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_long_accel", "pname": "vcs_pos_x", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "vcs_lat_accel", "pname": "vcs_pos_y", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

    ],

    "Raw_Detection_Information": [
        {"name": "range", "pname": "range", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "range_rate", "pname": "range_rate", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "azimuth", "pname": "azimuth", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "elevation", "pname": "elevation", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "snr", "pname": "snr", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]},
        {"name": "amplitude", "pname": "amplitude", "unit": "m", "plots": ["SC", "HIS", "SC_MM", "Bar_MM"]}

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
