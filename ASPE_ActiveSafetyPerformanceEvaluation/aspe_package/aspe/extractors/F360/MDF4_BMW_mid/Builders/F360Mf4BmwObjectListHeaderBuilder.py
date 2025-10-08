import warnings

import pandas as pd

from aspe.extractors.F360.DataSets.F360BmwObjectListHeader import F360BmwObjectListHeader
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.utilities.flatten_someip_object_list_data import flatten_someip_headers
from aspe.utilities.SupportingFunctions import get_from_dict_by_map


class F360Mdf4BmwObjectListHeaderBuilder(IBuilder):
    """
    Class for extracting someIP object list header data from parsed mf4 file.
    """

    object_list_data_keys_map = ['RecogSideRadarObjectlist', 'Objectlist']

    signal_mapper = {
        # BWM signature                                 ASPE signature
        'vigem_ts':                                     'vigem_timestamp',
        'counter_datatypeRef':                          'scan_index',
    }

    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        object_list_parsed_data = get_from_dict_by_map(parsed_data, self.object_list_data_keys_map)
        super().__init__(object_list_parsed_data)
        self.data_set = F360BmwObjectListHeader()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()
        self._object_headers_flat = {}

    def build(self):
        self._flatten_data()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals
        return self.data_set

    def _flatten_data(self):
        object_headers_flat = flatten_someip_headers(self._parsed_data, keys_to_skip=('object_list', ))
        self._object_headers_flat.update(object_headers_flat)

    def _extract_raw_signals(self):
        for key, array in self._object_headers_flat.items():
            if len(array.shape) > 1:
                warnings.warn(
                    f"object list header extraction: dimension of signal array: {key} is above 1. Skipping it")
            else:
                self._raw_signals[key] = array

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
                        warnings.warn('Missing scan index information - scan_index signal will be not extracted')
                else:
                    warnings.warn(f'Object list header extraction warning! There is no signal {bmw_signature} inside '
                              f'raw_signals DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self.data_set.signals.loc[:, 'unique_id'] = 0

    def _extract_timestamp(self):
        ts_seconds = self._object_headers_flat['timestamp_seconds']
        ts_nanoseconds = self._object_headers_flat['timestamp_nanoseconds']
        self.data_set.signals.loc[:, 'timestamp'] = ts_seconds + ts_nanoseconds * 1e-9
