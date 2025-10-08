import pickle

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PESingleObjects import PESingleObjects
from aspe.extractors.Interfaces.IHost import IHost
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


class PEObjectsEvaluationOutputSingleLog:
    def __init__(self,
                 extr_est_objs=IObjects(),
                 extr_ref_objs=IObjects(),
                 extr_ref_host=IHost(),
                 extr_radar_sensors=IRadarSensors(),
                 extr_radar_detections=IRadarDetections(),
                 paired_objs=PEPairedObjects(),
                 est_objs_results=PESingleObjects(),
                 ref_objs_results=PESingleObjects(),
                 pairs_lifetime_features=pd.DataFrame(),
                 objects_kpis_per_log=pd.DataFrame(),
                 pairs_kpis_per_log=pd.DataFrame(),
                 pairs_lifetime_kpis_per_log=pd.DataFrame(),
                 logs_features=pd.DataFrame()):
        self.extracted_estimated_objects = extr_est_objs
        self.extracted_reference_objects = extr_ref_objs
        self.extracted_reference_host = extr_ref_host
        self.extracted_radar_sensors = extr_radar_sensors
        self.extracted_radar_detections = extr_radar_detections
        self.pe_results_obj_pairs = paired_objs
        self.pe_results_obj_est = est_objs_results
        self.pe_results_obj_ref = ref_objs_results
        self.pe_results_pairs_lifetime_features = pairs_lifetime_features

        self.kpis_binary_class_per_log = objects_kpis_per_log
        self.kpis_pairs_features_per_log = pairs_kpis_per_log
        self.kpis_pairs_lifetime_per_log = pairs_lifetime_kpis_per_log

        self.logs_features_per_log = logs_features

    def print_kpis_and_log_features(self):
        print('\nSingle log results:')
        print('\nAssociated pairs KPIs:')
        print(self.kpis_pairs_features_per_log.to_string())
        print('\n')
        print(self.kpis_pairs_lifetime_per_log.to_string())
        print('\nObjects features KPIs:')
        print(self.kpis_binary_class_per_log.to_string())
        print('\nLog features:')
        print(self.logs_features_per_log.to_string())

    def save_to_pickle(self, pickle_path):
        with open(pickle_path, 'wb') as f:
            pickle.dump(self, f)


class PEObjectsEvaluationOutputMultiLog(PEObjectsEvaluationOutputSingleLog):
    def __init__(self):
        super().__init__(extr_est_objs=IObjects(),
                         extr_ref_objs=IObjects(),
                         extr_ref_host=IHost(),
                         extr_radar_sensors=IRadarSensors(),
                         extr_radar_detections=IRadarDetections(),
                         paired_objs=PEPairedObjects(),
                         est_objs_results=PESingleObjects(),
                         ref_objs_results=PESingleObjects(),
                         objects_kpis_per_log=pd.DataFrame(),
                         pairs_kpis_per_log=pd.DataFrame(),
                         logs_features=pd.DataFrame())

        self.logs_paths = pd.DataFrame()
        self.kpis_binary_class_aggregated = pd.DataFrame()
        self.kpis_pairs_features_aggregated = pd.DataFrame()
        self.kpis_pairs_lifetime_aggregated = pd.DataFrame()
        self.logs_features_aggregated = pd.DataFrame()
        self.logs_count = 0

    def update(self, log_path: str, single_log_results: PEObjectsEvaluationOutputSingleLog):
        # TODO handle sensor properties aggregation - DFT-94
        self.logs_paths = self.logs_paths.append({'log_index': self.logs_count, 'log_path': log_path},
                                                 ignore_index=True, sort=False)
        self._increment_log_index_for_single_results(single_log_results)
        self.extracted_estimated_objects.signals = self.extracted_estimated_objects.signals.append(
            single_log_results.extracted_estimated_objects.signals, sort=False)
        self.extracted_reference_objects.signals = self.extracted_reference_objects.signals.append(
            single_log_results.extracted_reference_objects.signals, sort=False)
        if single_log_results.extracted_reference_host is not None:
            self.extracted_reference_host.signals = self.extracted_reference_host.signals.append(
                single_log_results.extracted_reference_host.signals, sort=False)
        if single_log_results.extracted_radar_sensors is not None:
            self.extracted_radar_sensors.signals = self.extracted_radar_sensors.signals.append(
                single_log_results.extracted_radar_sensors.signals, sort=False)
            self.extracted_radar_sensors.per_sensor = single_log_results.extracted_radar_sensors.per_sensor
            self.extracted_radar_sensors.per_look = single_log_results.extracted_radar_sensors.per_look
        if single_log_results.extracted_radar_detections is not None:
            self.extracted_radar_detections.signals = self.extracted_radar_detections.signals.append(
                single_log_results.extracted_radar_detections.signals, sort=False)

        obj_est_len = len(self.pe_results_obj_est.signals)
        obj_ref_len = len(self.pe_results_obj_ref.signals)
        single_log_results.pe_results_obj_pairs.signals.loc[:, 'index_est'] += obj_est_len
        single_log_results.pe_results_obj_pairs.signals.loc[:, 'index_ref'] += obj_ref_len

        self.pe_results_obj_pairs.signals = self.pe_results_obj_pairs.signals.append(
            single_log_results.pe_results_obj_pairs.signals, sort=False)
        self.pe_results_obj_est.signals = self.pe_results_obj_est.signals.append(
            single_log_results.pe_results_obj_est.signals, sort=False)
        self.pe_results_obj_ref.signals = self.pe_results_obj_ref.signals.append(
            single_log_results.pe_results_obj_ref.signals, sort=False)

        self.pe_results_pairs_lifetime_features = self.pe_results_pairs_lifetime_features.append(
            single_log_results.pe_results_pairs_lifetime_features, sort=False)
        self.kpis_binary_class_per_log = self.kpis_binary_class_per_log.append(
            single_log_results.kpis_binary_class_per_log, sort=False)
        self.kpis_pairs_features_per_log = self.kpis_pairs_features_per_log.append(
            single_log_results.kpis_pairs_features_per_log, sort=False)
        self.kpis_pairs_lifetime_per_log = self.kpis_pairs_lifetime_per_log.append(
            single_log_results.kpis_pairs_lifetime_per_log, sort=False)
        self.logs_features_per_log = self.logs_features_per_log.append(single_log_results.logs_features_per_log,
                                                                       sort=False)
        self._reset_indexes()

    def _reset_indexes(self):
        self.logs_paths.reset_index(drop=True)
        self.extracted_estimated_objects.signals.reset_index(inplace=True, drop=True)
        self.extracted_reference_objects.signals.reset_index(inplace=True, drop=True)
        self.extracted_reference_host.signals.reset_index(inplace=True, drop=True)
        self.extracted_radar_sensors.signals.reset_index(inplace=True, drop=True)
        self.extracted_radar_detections.signals.reset_index(inplace=True, drop=True)
        self.pe_results_obj_pairs.signals.reset_index(inplace=True, drop=True)
        self.pe_results_obj_est.signals.reset_index(inplace=True, drop=True)
        self.pe_results_obj_ref.signals.reset_index(inplace=True, drop=True)
        self.pe_results_pairs_lifetime_features.reset_index(inplace=True, drop=True)
        self.kpis_binary_class_per_log.reset_index(inplace=True, drop=True)
        self.kpis_pairs_features_per_log.reset_index(inplace=True, drop=True)
        self.kpis_pairs_lifetime_per_log.reset_index(inplace=True, drop=True)
        self.logs_features_per_log.reset_index(inplace=True, drop=True)

    def _increment_log_index_for_single_results(self, single_log_results):
        single_log_results.extracted_estimated_objects.signals['log_index'] = self.logs_count
        single_log_results.extracted_reference_objects.signals['log_index'] = self.logs_count
        if single_log_results.extracted_reference_host is not None:
            single_log_results.extracted_reference_host.signals['log_index'] = self.logs_count
        if single_log_results.extracted_radar_sensors is not None:
            single_log_results.extracted_radar_sensors.signals['log_index'] = self.logs_count
        if single_log_results.extracted_radar_detections is not None:
            single_log_results.extracted_radar_detections.signals['log_index'] = self.logs_count

        single_log_results.pe_results_obj_pairs.signals['log_index'] = self.logs_count
        single_log_results.pe_results_obj_est.signals['log_index'] = self.logs_count
        single_log_results.pe_results_obj_ref.signals['log_index'] = self.logs_count
        single_log_results.pe_results_pairs_lifetime_features['log_index'] = self.logs_count

        single_log_results.kpis_binary_class_per_log['log_index'] = self.logs_count
        single_log_results.kpis_pairs_features_per_log['log_index'] = self.logs_count
        single_log_results.kpis_pairs_lifetime_per_log['log_index'] = self.logs_count

        single_log_results.logs_features_per_log['log_index'] = self.logs_count
        self.logs_count = self.logs_count + 1

    def print_kpis_and_log_features(self):
        print(f'\nAggregated KPIs from {self.logs_count} logs:\n')
        print('Associated pairs KPIs:')
        print(self.kpis_pairs_features_aggregated.to_string())
        print('\n')
        print(self.kpis_pairs_lifetime_aggregated.to_string())
        print('\nObjects features KPIs:')
        print(self.kpis_binary_class_aggregated.to_string())
        print('\nLogs features:')
        print(self.logs_features_aggregated.to_string())

    @staticmethod
    def _recalculate_scan_index_for_single_df(signals: pd.DataFrame, signals_map: pd.DataFrame):
        """
        Static function to recalculate scan indexes for single data frame
        :param signals: signals for which scan indexes should be recalculated
        :type signals: pd.DataFrame
        :param signals_map: signal map which is base for scan index recalculation
        :type signals_map: pd.DataFrame
        :return:
        """
        signals.set_index(['log_index', 'scan_index'], False, inplace=True)
        signals.scan_index = signals_map.loc[signals.index][:]
        signals.reset_index(drop=True, inplace=True)

    @staticmethod
    def _calculate_map(signals_ref: pd.DataFrame, signals_est: pd.DataFrame, diff_si_for_si_change=10):
        """
        Static function to calculate map of scan index with log index into new scan indexes which are
        increasing monotonically and have smaller gaps
        :param signals_ref: estimated signals which contain all needed scan indexes (can be with duplicates)
        :type signals_ref: pd.DataFrame
        :param signals_est: reference signals which contain all needed scan indexes (can be with duplicates)
        :type signals_est: pd.DataFrame
        :param diff_si_for_si_change:
        :return: si_map: pd.DataFrame with new scan indexes
        """
        mapping_index_ref = pd.MultiIndex.from_arrays([signals_ref.log_index, signals_ref.scan_index]).unique()
        mapping_index_est = pd.MultiIndex.from_arrays([signals_est.log_index, signals_est.scan_index]).unique()
        mapping_index = mapping_index_ref.union(mapping_index_est)
        si = mapping_index.to_frame().scan_index

        # Check for breaks in scan indexes
        si_diff = si.diff()
        si_diff[0] = 1  # needed since si_diff[0] is nan
        si_diff[si_diff < 1] = diff_si_for_si_change  # different logs may start with completely different SI
        si_diff[si_diff > diff_si_for_si_change] = diff_si_for_si_change  # May happen also in case of long drop
        si_map = si.iloc[0] + si_diff.cumsum()  # New scan indexes
        si_map = si_map.astype('int64')
        return si_map

    def recalculate_scan_indexes(self, diff_si_for_si_change=10):
        """
        Method to recalculate scan indexes to avoid scan indexes duplication for several logs.
        Log index together with scan index is utilized to perform such a transformation.
        New scan index is monotonically increasing. To discriminate between different logs predefined scan index gap
        between logs is defined (diff_si_for_si_change)
        :param diff_si_for_si_change: gap to detect that log ended and new one should be started
        :return: none, structure is modified.
        """

        si_map = self._calculate_map(self.pe_results_obj_ref.signals,
                                     self.pe_results_obj_est.signals,
                                     diff_si_for_si_change)

        self._recalculate_scan_index_for_single_df(self.extracted_reference_host.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.pe_results_obj_est.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.pe_results_obj_ref.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.pe_results_obj_pairs.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.extracted_estimated_objects.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.extracted_reference_objects.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.extracted_radar_sensors.signals, si_map)
        self._recalculate_scan_index_for_single_df(self.extracted_radar_detections.signals, si_map)
