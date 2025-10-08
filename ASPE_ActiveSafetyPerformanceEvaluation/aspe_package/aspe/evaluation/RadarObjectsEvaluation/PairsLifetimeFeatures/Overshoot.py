import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.default_overshoot_thresholds_list import (
    DEFAULT_OVERSHOOT_THRESHOLD,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.IPairsLifetimeFeature import IPairsLifetimeFeature


class Overshoot(IPairsLifetimeFeature):
    def __init__(self, thresholds: dict = DEFAULT_OVERSHOOT_THRESHOLD, objects_to_evaluate: list = []):
        """
        Class responsible for acceleration, velocity overshoot calculation. For each signal specified in 'thresholds'
        overshoot (magnitude, duration) is calculated. Based on the acceleration signal, the module detects the scan
        indexes in which the object is braking sharply. In turn, it counts overshoot at the designated scan indexes,
        in which the first index to analyze is the one in which the acceleration falls below the threshold.
        :param thresholds: dict in form {signal name: threshold value}, contains threshold for marking most varying part
        of signal
        :param objects_to_evaluate: list of unique_id of objects to be analyzed
        """
        self.thresholds = thresholds
        self.objects_to_evaluate = objects_to_evaluate
        self.signals_to_evaluate = list(thresholds.keys())
        self.object_recognition_signal = 'object_id'

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, pairs_est_ref: pd.DataFrame,
                  *args, **kwargs) -> pd.DataFrame:
        # Check if current object is in objects to evaluate list
        object_id = pairs_est_ref.reset_index().loc[0, 'unique_id_ref']
        data_is_empty = True

        # Evaluate for objects from the list or for all objects if the list is empty
        if object_id in self.objects_to_evaluate or not self.objects_to_evaluate:
            filtered_objects_data = self.prepare_object_data(estimated_data)
            analysis_start_idxs = self.find_analysis_start_indexes(filtered_objects_data)
            overshoot_results = self.provide_overshoot_results()

            if len(analysis_start_idxs):
                data_is_empty = False
                # Check overshoot for every falling slope below acceleration threshold
                for signal_to_evaluate in self.signals_to_evaluate:
                    magnitude, duration, scans = self.evaluate_signal(signal_to_evaluate,
                                                                      filtered_objects_data,
                                                                      analysis_start_idxs)
                    overshoot_results[f'overshoot_{signal_to_evaluate}_magnitude'] = magnitude
                    overshoot_results[f'overshoot_{signal_to_evaluate}_duration'] = duration
                    overshoot_results[f'overshoot_{signal_to_evaluate}_scans'] = scans

            else:
                overshoot_results = self.provide_overshoot_results(np.nan)
        else:
            overshoot_results = self.provide_overshoot_results(np.nan)

        overshoot_results[self.object_recognition_signal] = object_id
        return pd.DataFrame(overshoot_results, index=[0])

    def prepare_object_data(self, objects_data):
        signals_to_filter = ['scan_index', 'timestamp'] + self.signals_to_evaluate
        filtered_objects_data = objects_data.filter(signals_to_filter, axis=1)
        filtered_objects_data = filtered_objects_data.sort_values(by=['scan_index'])
        filtered_objects_data = filtered_objects_data.reset_index().drop(columns=['index'])
        return filtered_objects_data

    def find_analysis_start_indexes(self, filtered_objects_data, signal_to_evaluate = 'acceleration_otg_x'):
        init_mask = filtered_objects_data[signal_to_evaluate] < self.thresholds[signal_to_evaluate]['search_start_thr']
        # selects any initial scan index of the interval below the threshold
        mask = init_mask ^ np.roll(init_mask, +1) & init_mask
        analysis_start_idxs = filtered_objects_data.index[mask].tolist()
        return analysis_start_idxs

    def evaluate_signal(self, signal_to_evaluate, filtered_objects_data, analysis_start_idxs):
        filtered_objects_data[signal_to_evaluate] = \
            filtered_objects_data[signal_to_evaluate] * self.thresholds[signal_to_evaluate]['sign']
        overshoot_magnitude, overshoot_duration, overshoot_scans = [], [], []
        overshoot_start_threshold = self.thresholds[signal_to_evaluate]['overshoot_start_thr']
        overshoot_end_threshold = self.thresholds[signal_to_evaluate]['allowed_relative_oscillation']
        scope = self.thresholds[signal_to_evaluate]['scope']

        for analysis_start_idx in analysis_start_idxs:
            # find first zero crossing idx from negative to positive
            signal = filtered_objects_data.loc[analysis_start_idx::, signal_to_evaluate]
            zero_crossing = \
                np.bitwise_and(signal > overshoot_start_threshold, np.roll(signal, 1) <= overshoot_start_threshold)
            if zero_crossing.any():
                overshoot_start_idx = zero_crossing[zero_crossing].index.values[0]
                for idx in range(overshoot_start_idx, len(filtered_objects_data)):
                    # If the average value of the signal over a range of a few samples backward and forward exceeds
                    # the calculated fixed value then save the data
                    steady_state_samples = filtered_objects_data.loc[idx-scope:idx+scope, signal_to_evaluate]
                    if np.mean(np.abs(np.diff(steady_state_samples))) < overshoot_end_threshold:
                        overshoot_magnitude.\
                            append(np.max(filtered_objects_data.loc[analysis_start_idx:idx, signal_to_evaluate]))
                        overshoot_duration.\
                            append(filtered_objects_data.loc[idx, 'timestamp']
                                   - filtered_objects_data.loc[overshoot_start_idx, 'timestamp'])
                        overshoot_scans.\
                            append([filtered_objects_data.loc[overshoot_start_idx, 'scan_index'],
                                    filtered_objects_data.loc[idx, 'scan_index']])
                        break

        if not overshoot_magnitude:
            overshoot_magnitude, overshoot_duration, overshoot_scans = np.nan, np.nan, np.nan

        return overshoot_magnitude, overshoot_duration, overshoot_scans

    def provide_overshoot_results(self, value_to_assign=[]):
        overshoot_results = {}
        for signal_to_evaluate in self.signals_to_evaluate:
            overshoot_results[f'overshoot_{signal_to_evaluate}_magnitude'] = value_to_assign
            overshoot_results[f'overshoot_{signal_to_evaluate}_duration'] = value_to_assign
            overshoot_results[f'overshoot_{signal_to_evaluate}_scans'] = value_to_assign
        return overshoot_results
