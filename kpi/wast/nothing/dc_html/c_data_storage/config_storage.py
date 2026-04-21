# Dictionary of signal patterns with their aliases and plot types
Gen7V1_V2 = {
    "Tracker_Information": {
        "vcs_accel_x": {},
        "vcs_accel_y": {},
        "vcs_pos_x": {},
        "vcs_pos_y": {},
        "vcs_vel_x": {},
        "vcs_vel_y": {}
    },
    "OSI_Ground_Truth": {
        "length": {},
        "speed": {},
        "vcs_lat_accel": {},
        "vcs_lat_posn": {},
        "vcs_lat_vel": {},
        "vcs_long_accel": {},
        "vcs_long_posn": {},
        "vcs_long_vel": {}
    }
}

Gen7V1_v2_KPI = {
   ("DETECTION_STREAM",): {
       "rdd1_dindx": {},
       "rdd1_rindx": {}
   }
}



sequence_of_plot = {
    "scatter": [
        "all_signal;bar_mismatch_plots_all",
        "fbi_sup_sig;bar_plots_fbi_sup_sig",
        "Range;scatter_plot",
        "Range;scatter_with_mismatch",
        "Rangerate;scatter_plot",
        "Rangerate;scatter_with_mismatch",
        "Azimuth;scatter_plot",
        "Azimuth;scatter_with_mismatch",
        "elevation;scatter_plot",
        "elevation;scatter_with_mismatch",
        "amplitude;scatter_plot",
        "amplitude;scatter_with_mismatch",
        "radar_cross_section;scatter_plot",
        "radar_cross_section;scatter_with_mismatch"
    ],
    "scatter_mismatch": [
        "f_superres;scatter_plot",
        "f_superres;scatter_with_mismatch",
        "f_single_target;scatter_plot",
        "f_single_target;scatter_with_mismatch",
        "num_af_det;scatter_plot",
        "f_bistatic;scatter_plot",
        "signal_to_noise_ratio;scatter_plot"
    ],
    "histogram": [
        "Range;histogram_with_count",
        "Rangerate;histogram_with_count",
        "Azimuth;histogram_with_radtodeg",
        "elevation;histogram_with_radtodeg",
        "amplitude;histogram_with_count",
        "radar_cross_section;histogram_with_count",
        "signal_to_noise_ratio;histogram_with_count",
        "Range;scatter_with_scanindex"
    ]
}
