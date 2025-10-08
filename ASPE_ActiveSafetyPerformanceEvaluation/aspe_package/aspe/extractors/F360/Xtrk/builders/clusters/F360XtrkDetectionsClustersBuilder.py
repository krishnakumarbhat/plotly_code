# coding=utf-8
"""
F360 XTRK detections clusters builder
"""
import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360DetectionsClusters import F360DetectionsClusters
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.F360.Xtrk.builders.F360XtrkBuilder import F360XtrkBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.objects_lifespan_status_based_creator import ObjectLifespanStatusBasedCreator
from aspe.utilities.SupportingFunctions import get_nested_array


class F360XtrkDetectionsClustersBuilder(F360XtrkBuilder):
    """
    Supports F360 detections clusters signals extraction from .xtrk file.
    """

    signal_mapper = {
            # ASPE signature                        F360 signature
            PhysicalSignals.position_x:             ['vcs_position.longitudinal', 'vcs_position.x'],
            PhysicalSignals.position_y:             ['vcs_position.lateral', 'vcs_position.y'],
            PhysicalSignals.range_rate:             'rep_rdotcomp',
            PhysicalSignals.azimuth_vcs_aligned:    'rep_vcs_az',
            F360CoreSignals.tracker_id:             'id',
            GeneralSignals.existence_indicator:     'exist_prob',
            F360CoreSignals.n_dets:                 'ndets',
    }

    auxiliary_signals = {
        'scan_index': ['trkrInfo', 'cnt_loops'],
        'execution_timestamp': ['trkrInfo', 'time_us'],
        'time_since_created': ['clusters', 'time_since_created'],
        'time_since_measurement': ['clusters', 'time_since_measurement'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._raw_signals = pd.DataFrame()
        self.data_set = F360DetectionsClusters()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._lifespan_mask = None

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._calc_mask()
        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _calc_mask(self):
        """
        Get information about indexes of cluster data. Filter out not used slots and get information about unique_id.
        Logic is based on 'time_since_created' signal, which is equal to -1.0 if cluster slot is empty. Cluster init
        time is found by looking on difference of this signal - places where difference is greater than
        1.0 are marked as cluster init time.
        """
        time_since_created = get_nested_array(self._parsed_data, self.auxiliary_signals['time_since_created'])
        first_row = np.full(shape=(1, time_since_created.shape[1]), fill_value=-1.0)
        time_since_created_exp = np.vstack([first_row, time_since_created])  # to handle diff of first row
        time_diff = np.diff(time_since_created_exp, axis=0)

        cluster_born_value, cluster_invalid_value = 1, -1
        status_mat = np.full(time_since_created.shape, 0)
        status_mat[time_diff >= 1.0] = cluster_born_value
        status_mat[time_since_created == -1.0] = cluster_invalid_value

        self._lifespan_mask = ObjectLifespanStatusBasedCreator.create_lifespan(status_mat,
                                                                               obj_status_new_val=cluster_born_value,
                                                                               obj_status_invalid_val=cluster_invalid_value)

    def _set_parsed_data_shape(self):
        time_since_created = get_nested_array(self._parsed_data, self.auxiliary_signals['time_since_created'])
        self._parsed_data_shape = time_since_created.shape

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        """
        raw_signals = self._parsed_data['clusters']

        for name, values in raw_signals.items():
            if len(values.shape) == 2:
                converted_signal = values[self._lifespan_mask['row'], self._lifespan_mask['column']]
                self._raw_signals[name] = converted_signal
        self._raw_signals['unique_id'] = self._lifespan_mask['unique_obj_id']
        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Extract signals which cannot be strictly mapped using mapper object.
        """
        self._extract_scan_index()
        self._extract_timestamp()
        self.data_set.signals['slot_id'] = self._lifespan_mask['column']
        self.data_set.signals['unique_id'] = self._lifespan_mask['unique_obj_id']

    def _extract_scan_index(self):
        """
        Extract scan index data
        """
        rows = self._lifespan_mask['row'].to_numpy()
        scan_idx_vector = get_nested_array(self._parsed_data, self.auxiliary_signals['scan_index'])
        self.data_set.signals['scan_index'] = scan_idx_vector[rows]

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'

    def _extract_timestamp(self):
        '''
        Extract measurement timestamp, which is defined as tracker execution timestamp + time_since_measurement
        '''
        time_since_measurement = get_nested_array(self._parsed_data, self.auxiliary_signals['time_since_measurement'])
        exec_timestamp = get_nested_array(self._parsed_data, self.auxiliary_signals['execution_timestamp']) * 1e-6
        timestamp = exec_timestamp - time_since_measurement
        self.data_set.signals['timestamp'] = timestamp[self._lifespan_mask['row']]
