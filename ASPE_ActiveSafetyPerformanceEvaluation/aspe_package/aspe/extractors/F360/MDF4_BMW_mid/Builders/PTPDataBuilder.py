import warnings

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.PTPData import PTPData
from aspe.extractors.Interfaces.IBuilder import IBuilder


class PTPDataBuilder(IBuilder):
    """
    Class for extracting someIP object list from parsed mf4 file.
    """

    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        super().__init__(parsed_data['PTPMessages'])
        self.data_set = PTPData()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    def build(self):
        self.extract_raw_signals()
        sync_msgs, follow_msgs = self.split_to_sync_follow()
        self.update_sync_with_follow_timestamp(sync_msgs, follow_msgs)
        self.extract_mappable_signals(sync_msgs)
        self.extract_non_mappable_signals()
        self.filter_out_nans()
        self.extract_attributes()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals
        return self.data_set

    def extract_raw_signals(self):
        output = {'vigem_timestamp': []}
        for vigem_ts, frame in self._parsed_data.items():
            output['vigem_timestamp'].append(vigem_ts)
            for k, v in frame.items():
                if 'header' in k:
                    continue
                try:
                    output[k].append(v)
                except KeyError:
                    output[k] = [v]
        ptp_df = pd.DataFrame(output).sort_index(axis=1)
        ptp_df.loc[:, 'ethernet_timestamp'] = \
            ptp_df.loc[:, 'ethernetTimestampSecond'] + ptp_df.loc[:, 'ethernetTimestampNanosecond'] * 1e-9
        ptp_df.loc[:, 'timestamp'] = ptp_df.loc[:, 'timestampSecond'] + ptp_df.loc[:, 'timestampNanosecond'] * 1e-9
        self._raw_signals = ptp_df

    def split_to_sync_follow(self):
        is_sync = self._raw_signals.loc[:, 'messageType'] == 'Sync'
        sync_msgs, follow_msgs = self._raw_signals.loc[is_sync, :], self._raw_signals.loc[~is_sync, :]
        return sync_msgs, follow_msgs

    def update_sync_with_follow_timestamp(self, sync_msgs: pd.DataFrame, follow_msgs: pd.DataFrame):
        sync_ts, follow_ts = \
            sync_msgs.loc[:, 'ethernet_timestamp'].to_numpy(), follow_msgs.loc[:, 'ethernet_timestamp'].to_numpy()
        diff_mat = sync_ts - follow_ts.reshape((-1, 1))
        min_diffs = np.abs(diff_mat).min(axis=0)
        is_above_thr_mask = min_diffs > 0.05
        min_diffs_idxs = np.abs(diff_mat).argmin(axis=0)
        min_diffs_idxs[is_above_thr_mask] = -1

        follow_system_ts = follow_msgs.loc[:, 'timestamp'].to_numpy()
        sync_system_ts = [follow_system_ts[idx] if idx > -1 else np.nan for idx in min_diffs_idxs]
        sync_msgs.loc[:, 'timestamp'] = sync_system_ts

    def extract_mappable_signals(self, sync_msgs: pd.DataFrame):
        mapper = {
            'timestamp': 'timestamp',
            'ethernet_timestamp': 'ethernet_timestamp',
            'vigem_timestamp': 'vigem_timestamp',
        }
        for ptp_signature, aspe_signature in mapper.items():
            try:
                self.data_set.signals[aspe_signature] = sync_msgs.loc[:, ptp_signature].to_numpy()
            except KeyError:
                warnings.warn(f'PTP Data extraction warning! There is no signal {ptp_signature}. '
                              f'Signal {aspe_signature} will be not filled.')

    def extract_non_mappable_signals(self):
        self.data_set.signals['vigem_to_global_time_diff'] = \
            self.data_set.signals.loc[:, 'vigem_timestamp'] - self.data_set.signals.loc[:, 'timestamp']
        self.data_set.signals.loc[:, 'unique_id'] = 0

    def filter_out_nans(self):
        isnan = self.data_set.signals.timestamp.isna()
        self.data_set.signals = self.data_set.signals.loc[~isnan, :].reset_index(drop=True)

    def extract_attributes(self):
        self.data_set.vigem_to_global_time_diff_median = \
            self.data_set.signals.loc[:, 'vigem_to_global_time_diff'].median()
        vtgtd_min = self.data_set.signals.loc[:, 'vigem_to_global_time_diff'].min()
        vtgtd_max = self.data_set.signals.loc[:, 'vigem_to_global_time_diff'].max()
        self.data_set.vigem_to_global_time_diff_spread = vtgtd_max - vtgtd_min
