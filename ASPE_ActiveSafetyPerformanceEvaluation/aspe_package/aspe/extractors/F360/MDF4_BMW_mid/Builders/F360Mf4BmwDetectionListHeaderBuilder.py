import warnings

import pandas as pd

from aspe.extractors.F360.DataSets.F360BmwDetectionListHeader import F360BmwDetectionListHeader
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.utilities.flatten_someip_object_list_data import flatten_someip_headers


class F360Mdf4BmwDetectionListHeaderBuilder(IBuilder):
    """
    Class for extracting someIP detection list header data from parsed mf4 file.
    """
    detection_list_key = 'RecogSideRadarDetectionList'

    signal_mapper = {
        # BWM signature         ASPE signature
        'vigem_ts': 'vigem_timestamp',
        'counter_datatypeRef': 'scan_index',
        'sensor_id': 'sensor_id',
    }

    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        super().__init__(parsed_data)
        self.data_set = F360BmwDetectionListHeader()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()
        self._detection_headers_flat = {}

    def build(self):
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals
        return self.data_set

    def _extract_raw_signals(self):
        detections_dict = self._parsed_data[self.detection_list_key]
        header_dfs = []
        for sensor_id, sensor_data in enumerate(detections_dict.values()):
            header_data_flat = flatten_someip_headers(sensor_data, keys_to_skip=('detections',))
            header_df = pd.DataFrame()

            for key, array in header_data_flat.items():
                if len(array.shape) > 1:
                    warnings.warn(
                        f"object list header extraction: dimension of signal array: {key} is above 1. Skipping it")
                else:
                    header_df[key] = array

            header_df['sensor_id'] = sensor_id
            header_dfs.append(header_df)

        self._raw_signals = pd.concat(header_dfs).reset_index(drop=True)

    def _extract_mappable_signals(self):
        for bmw_signature, aspe_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, bmw_signature]
            except KeyError:
                if aspe_signature == 'scan_index':  # TODO this is temporary solution!
                    if 'counter_CommonCRCCounterP7' in self._raw_signals:
                        self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, 'counter_CommonCRCCounterP7']
                    elif 'counter' in self._raw_signals:
                        self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, 'counter']
                else:
                    warnings.warn(f'Detection list header extraction warning! There is no signal {bmw_signature} inside'
                                  f' raw_signals DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self.data_set.signals.loc[:, 'unique_id'] = 0

    def _extract_timestamp(self):
        ts_seconds = self._raw_signals['header_time_stamp_seconds_value']
        ts_nanoseconds = self._raw_signals['header_time_stamp_fractional_seconds_value']
        self.data_set.signals.loc[:, 'timestamp'] = ts_seconds + ts_nanoseconds * 1e-9
