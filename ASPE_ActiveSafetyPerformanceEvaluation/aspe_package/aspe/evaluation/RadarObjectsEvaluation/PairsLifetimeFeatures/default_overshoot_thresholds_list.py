DEFAULT_OVERSHOOT_THRESHOLD = {
    "acceleration_otg_x": {'search_start_thr': -5.0,
                           'overshoot_start_thr': 0.0,
                           'allowed_relative_oscillation': 0.035,
                           'sign': 1,
                           'scope': 3},
    "velocity_otg_x":     {'search_start_thr': -0.5,
                           'overshoot_start_thr': 0.0,
                           'allowed_relative_oscillation': 0.025,
                           'sign': -1,
                           'scope': 3},

}

"""
search_start_thr: value, beyond which it starts looking for overshooting
allowed_relative_oscillation: allowed oscillation value
sign: takes the value {-1 , 1} is used to invert overshoot along the time axis
scope: range (idx-scope, idx+scope) in which the average value of the signal is counted for comparison 
with a fixed value 
"""
