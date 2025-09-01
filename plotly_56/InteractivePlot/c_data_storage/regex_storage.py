# Dictionary of signal patterns with their aliases and plot types
Gen7V1_v2 = {
    'ran': {
        'aliases': ['ran', 'detection_range'],
        'call':['Range'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        'unit':'meters',
        'range_to_be_accepted':[0,500],
        'grp':'Vse_stream'
    },
    # 'num_af_det': {
    #     'aliases': ['num_af_det', 'af_det'],
    #     'call':['num_af_det'],
    #     'plot_types': ['scatter_with_in_out'],
    #     # 'plot_types': ['scatter_with_in_out','scatter_mismatch_scanindex','create_diff_box_plot','create_diff_scatter_plot']    ,
    #     'unit':'meters',
    #     'range_to_be_accepted':[0,500]
    # },
    'theta': {
        'aliases': ['theta', 'azimuth'],
        'call':['Azimuth'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        'unit':'radiance',
        'range_to_be_accepted':[-80,80]
    },
    'phi': {
        'aliases': ['phi', 'elevation'],
        'call':['elevation'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        # 'plot_types': ['create_match_mismatch_pie','scatter_with_in_out','scatter_mismatch_scanindex','histogram_with_count'],
        'unit':'radiance',
        'range_to_be_accepted':[-120,120]
    
    },
    # 'amplitude': {
    #     'aliases': ['amplitude', 'amplitude_val'],
    #     'call':['amplitude'],
    #     'plot_types': ['scatter_with_in_out'],
    #     # 'plot_types': ['create_match_mismatch_pie','scatter_with_in_out','scatter_mismatch_scanindex','histogram_with_count'],
    #     'unit':'meters',
    #     'range_to_be_accepted':[0,1000]
    # },
    'snr': {
        'aliases': ['snr', 'signal_to_noise_ratio'],
        'call':['signal_to_noise_ratio'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        # 'plot_types': ['create_match_mismatch_pie','scatter_with_in_out','scatter_mismatch_scanindex','histogram_with_count'],
        'unit':'DB',
        'range_to_be_accepted':[0,100]
    },
    'rcs': {
        'aliases': ['rcs', 'radar_cross_section'],
        'call':['radar_cross_section'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        # 'plot_types': ['create_match_mismatch_pie','scatter_with_in_out','scatter_mismatch_scanindex','histogram_with_count'],
    },
    # 'f_bistatic': {
    #     'aliases': ['f_bistatic', 'bistatic_count'],
    #     'plot_types': ['cal_num_af_det_with_bfstat_hist:']
    # },
    # 'f_superres': {
    #     'aliases': ['f_superres', 'super_res_target'],
    #     'plot_types': ['scatter_with_in_out','scatter_mismatch_scanindex', 'cal_f_superres']
    # },
    'vel': {
        'aliases': ['velocity'],
        'call':['Rangerate'],
        'plot_types': ['histogram_with_count','scatter_with_in_out'],
        # 'plot_types': ['create_match_mismatch_pie','scatter_with_in_out','scatter_mismatch_scanindex','histogram_with_count']
    },
    'veh_yaw': {
        'aliases': ['yaw'],
        'call':['Yaw'],
        'plot_types': ['scatter_with_in_out']
    },
    'veh_speed': {
        'aliases': ['speed'],
        'call':['Speed'],
        'plot_types': ['scatter_with_in_out']
    },
    'veh_steering_angle': {
        'aliases': ['steering_angle'],
        'call':['Steering_angle'],
        'plot_types': ['scatter_with_in_out']
    },
    'veh_pitch': {
        'aliases': ['pitch'],
        'call':['Veh_pitch'],
        'plot_types': ['scatter_with_in_out']
    },
    'veh_roll': {
        'aliases': ['Roll'],
        'call':['Roll'],
        'plot_types':['scatter_with_in_out']
    },

}
Gen7V1_v2_streams = {
				'VSE_STREAM':None,
                'DOWN_SELECTION_STREAM':None, 
                'DETECTION_STREAM':None}

sequance_of_plot = [
            'Range:create_match_mismatch_pie',
            'Rangerate:create_match_mismatch_pie',
            'Azimuth:create_match_mismatch_pie',
            'elevation:create_match_mismatch_pie',
            'amplitude:create_match_mismatch_pie',
            'radar_cross_section:create_match_mismatch_pie',
            'f_superres:create_match_mismatch_pie',
            'signal_to_noise_ratio:create_match_mismatch_pie',

            'Range:scatter_with_in_out',
            'Range:scatter_mismatch_scanindex',

            'Rangerate:scatter_with_in_out',
            'Rangerate:scatter_mismatch_scanindex',

            'Azimuth:scatter_with_in_out',
            'Azimuth:scatter_mismatch_scanindex',

            'elevation:scatter_with_in_out',
            'elevation:scatter_mismatch_scanindex',

            'amplitude:scatter_with_in_out',
            'amplitude:scatter_mismatch_scanindex',

            'radar_cross_section:scatter_with_in_out',
            'radar_cross_section:scatter_mismatch_scanindex',

            'f_superres:scatter_with_in_out',
            'f_superres:scatter_mismatch_scanindex',

            'signal_to_noise_ratio:scatter_with_in_out',
            'signal_to_noise_ratio:scatter_mismatch_scanindex',

            'Range:histogram_with_count',
            'Rangerate:histogram_with_count',
            'Azimuth:histogram_with_count',
            'elevation:histogram_with_count',
            'amplitude:histogram_with_count',
            'radar_cross_section:histogram_with_count',
            'signal_to_noise_ratio:histogram_with_count',

            'f_bistatic:cal_num_af_det_with_bfstat_hist',
            'f_superres:cal_f_superres',

            'Range:scatter_with_scanindex'
            
            #order of vse_stream

            'veh_roll:create_match_mismatch_pie',
            'veh_pitch:create_match_mismatch_pie',
            
            # speed,yawreate,string anlw 
            # float32_t   veh_speed;
            # float32_t   veh_steering_angle;
            # float32_t   veh_yaw;
            
            # ----------------
            # float32_t   veh_pitch;
            # float32_t   veh_roll;
            ]