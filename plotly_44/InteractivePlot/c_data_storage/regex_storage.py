# Dictionary of signal patterns with their aliases, data types, and plot types
SIGNAL_PATTERNS = {
    'stream_hdr': {
        'aliases': ['stream_header', 'stream_hdr'],
        'plot_types': ['table']
    },
    'vel': {
        'aliases': ['vel', 'velocity'],
        'plot_types': ['scatter', 'histogram']
    },
    'num_af_det': {
        'aliases': ['num_af_det', 'af_det'],
        'plot_types': ['scatter', 'box', 'histogram'],
        'description': 'Number of scanindex detection count veh vs resim'
    },
    'f_bistatic': {
        'aliases': ['f_bistatic', 'bistatic_count'],
        'plot_types': ['scatter', 'bar'],
        'description': 'Count number of 1s in each scanindex for bistatic detection'
    },
    'f_superres': {
        'aliases': ['f_superres', 'super_res_target'],
        'plot_types': ['scatter'],
        'description': 'Super resolution target detection'
    },
    'f_signal_target': {
        'aliases': ['f_signal_target', 'isinletar'],
        'plot_types': ['scatter'],
        'description': 'Signal target identification'
    },
    'ran': {
        'aliases': ['ran', 'detection_range'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Detection range plot 2D to 1D'
    },
    'rangerate': {
        'aliases': ['rangerate', 'range_rate'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Range rate measurements'
    },
    'theta': {
        'aliases': ['theta', 'azimution'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Azimuth angle measurements'
    },
    'phi': {
        'aliases': ['phi', 'elivation'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Elevation angle measurements'
    },
    'amplitude': {
        'aliases': ['amplitude', 'amplitude_val'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Amplitude measurements'
    },
    'snr': {
        'aliases': ['snr', 'signal_to_noise_ratio'],
        'plot_types': ['scatter', 'histogram', 'box'],
        'description': 'Signal-to-noise ratio measurements'
    },
    'rcs': {
        'aliases': ['rcs', 'radar_cross_section'],
        'plot_types': ['scatter', 'histogram'],
        'description': 'Radar cross-section measurements'
    },
}