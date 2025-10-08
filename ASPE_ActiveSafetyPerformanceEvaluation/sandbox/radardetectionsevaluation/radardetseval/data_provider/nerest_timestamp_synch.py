from copy import deepcopy
from warnings import warn

import numpy as np
import pandas as pd
from AptivDataExtractors.Interfaces.ExtractedData import ExtractedData
from AptivDataExtractors.Interfaces.IDataSet import IDataSet


class RtF360NearestTimestampSynchronization:
    def __init__(self, max_ts_diff=0.05, inplace=False, *args, **kwargs):
        """
        Init function realise class configuration: all filter conf should be set here

        :param args: any arguments needed for configuration
        :param kwargs: same as above
        """
        super().__init__(*args, **kwargs)
        self.ref_data = None
        self.est_data = None
        self._max_ts_diff = max_ts_diff
        self.inplace = inplace

    def synch(self,
              estimated_data: ExtractedData,
              reference_data: ExtractedData,
              master_data_set: IDataSet,
              slave_data_set: IDataSet,
              *args, **kwargs) -> (ExtractedData, ExtractedData):
        """
        :param estimated_data: extracted estimation object
        :type estimated_data: ExtractedData
        :param reference_data: extracted reference object
        :type reference_data: ExtractedData
        :param master_data_set: on which data set data should be synchronized - master
        :type master_data_set: IDataSet
        :param slave_data_set: on which data set data should be synchronized - slave
        :type slave_data_set: IDataSet
        :param args:
        :param kwargs:
        :return: (ExtractedData, ExtractedData) -> synchronized_reference_data, synchronized_estimated_data
        """
        if self.inplace:
            self.est_data = estimated_data
            self.ref_data = reference_data
        else:
            self.ref_data = deepcopy(reference_data)
            self.est_data = deepcopy(estimated_data)

        corr_si_df = self._get_corresponding_scan_indexes_dataframe(master=master_data_set,
                                                                    slave=slave_data_set)

        if corr_si_df is None:
            pass
        else:
            self._check_mid_nans(corr_si_df)

            corr_si_df = self._corr_array_drop_nan_set_index(corr_si_df)
            for data_set in self.ref_data.__dict__.values():
                if isinstance(data_set, IDataSet):
                    data_set.signals = self._get_slave_si_synched_dataset(data_set, corr_si_df)

            for data_set in self.est_data.__dict__.values():
                if isinstance(data_set, IDataSet):
                    data_set.signals = self._get_master_si_synched_dataset(data_set, corr_si_df)

        return self.est_data, self.ref_data

    def _get_corresponding_scan_indexes_dataframe(self, master, slave) -> pd.DataFrame():
        """
        For providing a matching scan_index structure this function takes timestamp from est_data and looking for
        nearest timestamp in ref_data
        :return: dataframe with columns as: ['master_data_scan_index', 'slave_data_scan_index']
        """

        master_si_ts = master.signals[['scan_index', 'timestamp']].reset_index(drop=True)
        slave_si_ts = slave.signals[['scan_index', 'timestamp']].reset_index(drop=True)

        master_ts_array = np.array(master_si_ts['timestamp'])
        slave_ts_array = np.array(slave_si_ts['timestamp'])

        # each row represents diff for single host's ts and all target's ts
        diff_ts_matrix = abs(np.subtract(master_ts_array.reshape(-1, 1), slave_ts_array))

        if diff_ts_matrix.size == 0:
            corr_array = None
        else:
            # finding min values for each row, looking for min ts diff in all target's ts for each host's ts
            min_diff_values = diff_ts_matrix.min(axis=1)
            min_diff_indexes = diff_ts_matrix.argmin(axis=1)

            min_diff_valid_indexes = np.where(min_diff_values <= self._max_ts_diff, min_diff_indexes, np.nan)

            slave_data_scan_index = slave_si_ts['scan_index'][min_diff_valid_indexes].reset_index(drop=True)
            slave_data_duplicated_mask = slave_data_scan_index.duplicated(keep='first')
            slave_data_scan_index[slave_data_duplicated_mask] = np.nan  # filling duplicates with nans

            corr_array = pd.concat([master_si_ts['scan_index'], slave_data_scan_index], axis=1)
            corr_array.columns = ['master_data_scan_index', 'slave_data_scan_index']

        return corr_array

    def _check_mid_nans(self, corresponding_scan_indexes):
        """
        Checking in there is any nan other than at the beginning or end or array
        :param corresponding_scan_indexes:
        :return:
        """
        first_valid_idx = corresponding_scan_indexes['slave_data_scan_index'].first_valid_index()
        last_valid_idx = corresponding_scan_indexes['slave_data_scan_index'].last_valid_index()

        mid_series = corresponding_scan_indexes['slave_data_scan_index'][first_valid_idx:last_valid_idx + 1]

        mid_series_nan_mask = mid_series.isnull()

        if mid_series_nan_mask.any():
            warn('nan occurs in mid of data')

    def _corr_array_drop_nan_set_index(self, corresponding_scan_indexes: pd.DataFrame()) -> pd.DataFrame():
        corr_array = corresponding_scan_indexes.dropna(axis=0)

        corr_array = corr_array.set_index('slave_data_scan_index')

        return corr_array

    def _get_slave_si_synched_dataset(self, data_set,
                                      corresponding_scan_indexes: pd.DataFrame()) -> pd.DataFrame():
        """
        Getting from ref data rows with corresponding scan_indexes and returns them in new dataframe
        :param corresponding_scan_indexes: dataframe: ['est_data_scan_index', 'ref_data_scan_index']
        :return: scan index synchronized ref data by nearest timestamp
        """
        si_synch_mask = data_set.signals['scan_index'].isin(corresponding_scan_indexes.index)

        signals_si_filtered = data_set.signals[si_synch_mask].reset_index(drop=True)

        signals_si_filtered_with_master_si = signals_si_filtered.join(corresponding_scan_indexes, on='scan_index')

        signals_si_filtered_with_master_si['scan_index'] = signals_si_filtered_with_master_si['master_data_scan_index']

        si_synched_signals = signals_si_filtered_with_master_si.drop(columns='master_data_scan_index')

        return si_synched_signals

    def _get_master_si_synched_dataset(self, data_set, corresponding_scan_indexes: pd.DataFrame()) -> pd.DataFrame():
        """
        Filtering masters scan indexes by ones in corresponding_scan_indexes dataframe
        :param data_set:
        :param corresponding_scan_indexes:
        :return:
        """
        si_synch_mask = data_set.signals['scan_index'].isin(corresponding_scan_indexes['master_data_scan_index'])

        signals_si_filtered = data_set.signals[si_synch_mask].reset_index(drop=True)

        return signals_si_filtered
