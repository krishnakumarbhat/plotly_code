# Dictionary of signal patterns with their aliases and plot types
Gen7V1_v2 = {

    'ran': {
        'aliases': ['ran', 'detection_range'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],
        # 'pos':[1 ,22,15]
        'unit':'meters',
        'range_to_be_accepted':[0,500]
    },
    'num_af_det': {
        'aliases': ['num_af_det', 'af_det'],
        'plot_types': ['scatter_with_scanindex','create_diff_box_plot','create_diff_scatter_plot']    ,
        'unit':'meters',
        'range_to_be_accepted':[0,500]
    },
    # 'histogram', 'call: cal_ran'
    'rangerate': {
        'aliases': ['rangerate', 'range_rate'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count',],
        # 'pos':[2 ,23,16]
        'unit':'meters',
        'range_to_be_accepted':[0,500]
    },
    'theta': {
        'aliases': ['theta', 'azimuth'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],
        'unit':'radiance',
        'range_to_be_accepted':[-80,80]
    },
    # , 'histogram', 'call: cal_theta'
    'phi': {
        'aliases': ['phi', 'elevation'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],
        'unit':'radiance',
        'range_to_be_accepted':[-120,120]
    
    },
    # , 'histogram', 'call: cal_phi'
    'amplitude': {
        'aliases': ['amplitude', 'amplitude_val'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],
        'unit':'meters',
        'range_to_be_accepted':[0,1000]
    },
    # , 'histogram', 'call: cal_amplitude'
    'snr': {
        'aliases': ['snr', 'signal_to_noise_ratio'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],
        'unit':'DB',
        'range_to_be_accepted':[0,100]
    },
    # , 'histogram', 'box_with_value', 'call: cal_snr'
    'rcs': {
        'aliases': ['rcs', 'radar_cross_section'],
        'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex','histogram_with_count'],

    },
    # 'vel': {
    #     'aliases': ['vel', 'velocity'],
    #     'plot_types': ['create_match_mismatch_pie','scatter_with_scanindex']
    # },
    
    # , 'histogram','call: cal_rcs'
    # 'histogram', 'call: cal_rangerate'
    # 'box_with_value', 'cal_num_af_det'
    # 'f_bistatic': {
    #     'aliases': ['f_bistatic', 'bistatic_count'],
    #     'plot_types': ['cal_num_af_det_with_bfstat_hist']
    # },
    # 'f_superres': {
    #     'aliases': ['f_superres', 'super_res_target'],
    #     'plot_types': ['scatter_with_scanindex', 'call: cal_f_superres']
    # },
    # 'f_signal_target': {
    #     'aliases': ['f_signal_target', 'isinletar'],
    #     'plot_types': ['scatter_with_scanindex', 'call: cal_f_signal_target']
    # },


}

# sequance_plot:{}