import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.supporting_functions import (
    adjust_pos_by_reference_point_2_reference_point,
    moving_window,
    saturate_downwards,
    saturate_upwards,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.default_latency_thresholds_list import (
    DEFAULT_LATENCY_THRESHOLD,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.IPairsLifetimeFeature import IPairsLifetimeFeature
from aspe.utilities.MathFunctions import calc_velocity_in_position


class Latency(IPairsLifetimeFeature):
    def __init__(self, thresholds: dict = DEFAULT_LATENCY_THRESHOLD, moving_std_window_size: int = 20,
                 min_delay: float = 0.0, max_delay: float = 0.5,
                 grid_search_nodes: int = 6,
                 grid_search_iterations: int = 5,
                 min_pair_lifetime: float = 2.5,
                 min_signal_time_above_threshold: float = 0.25):
        """
        Class responsible for latency calculation. For each signal specified in 'thresholds' dict latency is estimated
        by calculating RMS + 3STD of difference between reference and estimate while reference signal is being delayed
        by DT in specified range. DT with lowest RMS + 3STD is considered as latency.
        Only varying signals which met certain conditions are taken into latency evaluation:
        - min/max spread of signal is above given threshold
        - at least one sample of std of signal diff calculated in moving window is above given threshold
        - if these two conditions are not met - latency for given signal is np.nan
        :param thresholds: dict in form {signal name: threshold value}, contains threshold for marking most varying part
        of signal
        :param moving_std_window_size: size of moving window which calculates std of signal diff, which is needed to
        mark signal part taken for latency calculation
        :param min_delay: minimal considered latency
        :param max_delay: maximal considered latency
        :param grid_search_nodes: optimization parameter - how many grid nodes are checked as latency candidate - first
        grid search iteration is within range min_delay, max_delay
        :param grid_search_iterations: how many grid search optimization is done - each iteration returns latency
        candidate and new left and right boundaries for next grid search iteration
        :param min_pair_lifetime: lifetime of ref-est objects pair must be above this value to calculate latency,
        otherwise all latency signals will be equal to np.nan
        :param min_signal_time_above_threshold:
        """
        self.thresholds = thresholds
        self.moving_std_window_size = moving_std_window_size
        self.min_delay = min_delay
        self.max_delay = max_delay
        self.grid_search_nodes = grid_search_nodes
        self.grid_search_iterations = grid_search_iterations
        self.min_pair_lifetime = min_pair_lifetime
        self.min_signal_time_above_std_diff_threshold = min_signal_time_above_threshold

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, pairs_est_ref: pd.DataFrame,
                  *args, **kwargs) -> pd.DataFrame:
        ref_adjusted = self.adjust_reference_point_of_ref_data(estimated_data, reference_data)
        latency_results = {}
        pair_life_time_is_above_threshold = self.check_if_pair_lifetime_is_above_thr(estimated_data)
        if pair_life_time_is_above_threshold:
            for signal_name in self.thresholds.keys():
                latency = self.calculate_latency_for_single_signal(estimated_data, ref_adjusted, signal_name)
                latency_results[f'latency_{signal_name}'] = latency
        else:
            latency_results = {f'latency_{signal_name}': np.nan for signal_name in self.thresholds.keys()}
        return pd.DataFrame(latency_results, index=[0])

    @staticmethod
    def adjust_reference_point_of_ref_data(estimated_data: pd.DataFrame, reference_data: pd.DataFrame):
        ref_adjusted = reference_data.copy()
        adjust_pos_by_reference_point_2_reference_point(estimated_data, ref_adjusted)

        ref_vel_x, ref_vel_y = calc_velocity_in_position(reference_data['position_x'], reference_data['position_y'],
                                                         ref_adjusted['velocity_otg_x'], ref_adjusted['velocity_otg_y'],
                                                         ref_adjusted['yaw_rate'],
                                                         ref_adjusted['position_x'], ref_adjusted['position_y'])
        ref_adjusted.loc[:, 'velocity_otg_x'] = ref_vel_x
        ref_adjusted.loc[:, 'velocity_otg_y'] = ref_vel_y
        return ref_adjusted

    def check_if_pair_lifetime_is_above_thr(self, estimated_data: pd.DataFrame):
        ts = estimated_data.loc[:, 'timestamp'].to_numpy()
        born_ts, died_ts = ts[0], ts[-1]
        lifespan = died_ts - born_ts
        return lifespan > self.min_pair_lifetime

    def calculate_latency_for_single_signal(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame,
                                            signal_name: str):
        latency = np.nan  # initial value
        try:
            est_signal = estimated_data.loc[:, ['timestamp', signal_name]]
            ref_signal = reference_data.loc[:, ['timestamp', signal_name]]
            if self.signal_meets_conditions_for_calculations(ref_signal, signal_name):
                latency = self.minimize_cost_function_by_grid_search(ref_signal, est_signal, signal_name)
        except KeyError:
            pass
        return latency

    def signal_meets_conditions_for_calculations(self, ref_signal: pd.DataFrame, signal_name: str):
        conditions_met = False
        thresholds = self.thresholds[signal_name]
        min_max_spread_thr, std_thr = thresholds['min_max_spread'], thresholds['diff_std_peak']

        signal_vals = ref_signal.loc[:, signal_name].to_numpy()
        signal_spread = signal_vals.max() - signal_vals.min()
        if min_max_spread_thr < signal_spread:
            signal_diff = np.diff(signal_vals)
            signal_diff = np.hstack([signal_diff[0], signal_diff])
            std_windowed = moving_window(signal_diff, win_size=20, win_fun=np.std)
            if np.any(std_thr < std_windowed):
                conditions_met = True
        return conditions_met

    def minimize_cost_function_by_grid_search(self, ref_signal, est_signal, signal_name):
        lowest_dt, dt_before_min, dt_after_min = self.find_cost_fun_min_for_given_range(ref_signal, est_signal,
                                                                                        self.min_delay, self.max_delay,
                                                                                        signal_name)
        for n in range(self.grid_search_iterations):
            if lowest_dt is np.nan:
                break
            lowest_dt, dt_before_min, dt_after_min = self.find_cost_fun_min_for_given_range(ref_signal, est_signal,
                                                                                            dt_before_min, dt_after_min,
                                                                                            signal_name)
        return lowest_dt

    def find_cost_fun_min_for_given_range(self, ref_signal, est_signal, min_ts, max_ts, signal_name):
        dt_vec = np.linspace(min_ts, max_ts, self.grid_search_nodes)
        dt_to_cost_fun_value = {}
        for dt in dt_vec:
            cost_function_value = self.calc_cost_function_for_given_dt(dt, est_signal, ref_signal, signal_name)
            dt_to_cost_fun_value[dt] = cost_function_value
        cost_fun_to_dt = {v: k for k, v in dt_to_cost_fun_value.items() if v is not np.nan}

        lowest_dt, dt_before_min, dt_after_min = np.nan, np.nan, np.nan  # initial values
        if len(cost_fun_to_dt) > 0:
            lowest_cost_fun_val = min(cost_fun_to_dt.keys())
            if lowest_cost_fun_val < self.thresholds[signal_name]['cost_fun_max']:
                lowest_dt = cost_fun_to_dt[lowest_cost_fun_val]
                dt_step = dt_vec[1] - dt_vec[0]
                dt_before_min = saturate_downwards(lowest_dt - dt_step, self.min_delay)
                dt_after_min = saturate_upwards(lowest_dt + dt_step, self.max_delay)
        return lowest_dt, dt_before_min, dt_after_min

    @staticmethod
    def calc_cost_function_for_given_dt(dt: float, est_signal: pd.DataFrame, ref_signal: pd.DataFrame,
                                        signal_name: str):
        delayed_ref = Latency.delay_signal(ref_signal, dt)
        est_signal_synced, ref_signal_synced = Latency.sync_signal_pair(est_signal, delayed_ref, signal_name)
        cost_fun_val = Latency.calc_cost_function_between_synced_signals(est_signal_synced, ref_signal_synced,
                                                                         signal_name)
        return cost_fun_val

    @staticmethod
    def delay_signal(ref_signal: pd.DataFrame, dt: float):
        ref_signal_delayed = ref_signal.copy()
        ref_signal_delayed.loc[:, 'timestamp'] = ref_signal_delayed.loc[:, 'timestamp'] + dt
        return ref_signal_delayed

    @staticmethod
    def sync_signal_pair(est_signal: pd.DataFrame, delayed_ref: pd.DataFrame, signal_name: str):
        ref_ts = delayed_ref.loc[:, 'timestamp'].to_numpy()
        est_ts = est_signal.loc[:, 'timestamp'].to_numpy()

        ref_signal_vals = delayed_ref.loc[:, signal_name].to_numpy()
        ref_signal_vals_synced = np.interp(x=est_ts, xp=ref_ts, fp=ref_signal_vals, left=np.nan, right=np.nan)

        ref_signal_synced = pd.DataFrame({'timestamp': est_ts, signal_name: ref_signal_vals_synced})
        return est_signal, ref_signal_synced

    @staticmethod
    def calc_cost_function_between_synced_signals(est_signal: pd.DataFrame, ref_signal: pd.DataFrame, signal_name: str):
        est_signal_vals = est_signal.loc[:, signal_name].to_numpy().astype(float)
        ref_signal_vals = ref_signal.loc[:, signal_name].to_numpy().astype(float)
        signal_diff = ref_signal_vals - est_signal_vals
        signal_diff = signal_diff[~np.isnan(signal_diff)]  # remove nans
        if len(signal_diff) > 0:
            cost_fun_val = np.sqrt(np.mean(np.square(signal_diff))) + 3 * np.std(signal_diff)
        else:
            cost_fun_val = np.nan
        return cost_fun_val
