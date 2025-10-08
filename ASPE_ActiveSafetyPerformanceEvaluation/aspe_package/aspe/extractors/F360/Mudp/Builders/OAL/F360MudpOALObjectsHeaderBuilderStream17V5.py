from warnings import warn

import numpy as np

from aspe.extractors.F360.DataSets import F360OALObjectListHeader
from aspe.extractors.F360.DataSets.F360OALObjectListHeader import F360OALObjectsHeader
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals
from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, recursive_dict_extraction


class F360MudpOALObjectsHeaderBuilderStream17V5(IMudpBuilder):
    required_stream_definitions = [["strdef_src035_str017_ver002", "strdef_src035_str017_ver003",
                                    "strdef_src035_str017_ver004", "strdef_src035_str017_ver005",
                                    "strdef_src035_str017_ver006", "strdef_src035_str017_ver007",
                                    "strdef_src035_str017_ver008", "strdef_src035_str017_ver009"]]
    obj_list_header_map = ['parsed_data', 17, 'adapted_output', 'bmw_object_list']

    signal_mapper = {
        # ASPE signature                              # BWM signature
        GeneralSignals.scan_index:                    'counter',
        F360OALObjectListHeader.number_of_objects:    'number_of_objects',
    }

    aux_signals = {
        'ts_detections_s': ['parsed_data', 17, 'adapted_output', 'bmw_object_list', 'timestamp_detections', 'seconds'],
        'ts_detections_frac': ['parsed_data', 17, 'adapted_output', 'bmw_object_list', 'timestamp_detections',
                               'fractional_seconds'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._obj_list_header_signals_flat = None
        self.data_set = F360OALObjectsHeader()

    def build(self):
        obj_list_header = get_from_dict_by_map(self._parsed_data, self.obj_list_header_map)
        obj_list_header.pop('object_list')
        self._obj_list_header_signals_flat, _ = recursive_dict_extraction(obj_list_header)
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
        self._convert_dtypes()
        return self.data_set

    def _extract_raw_signals(self):
        for signal_signature, array in self._obj_list_header_signals_flat.items():
            if len(array.shape) == 1:
                self._raw_signals[signal_signature] = array
            elif len(array.shape) == 2:
                for column_number in range(0, array.shape[1]):
                    self._raw_signals[signal_signature + f'_{column_number}'] = array[:, column_number]

    def _extract_non_mappable_signals(self):
        self.data_set.signals.loc[:, 'slot_id'] = np.arange(self._raw_signals.shape[0])
        self.data_set.signals.loc[:, 'unique_id'] = 0
        self._extract_timestamp()
        self._extract_sensor_timestamps()

    def _extract_timestamp(self):
        ts_secs_arr = self._raw_signals.loc[:, 'timestamp_seconds'].to_numpy()
        ts_frac_secs_arr = self._raw_signals.loc[:, 'timestamp_fractional_seconds'].to_numpy()
        ts_arr = ts_secs_arr + ts_frac_secs_arr * 1e-9
        self.data_set.signals.loc[:, 'timestamp'] = ts_arr

    def _extract_sensor_timestamps(self):
        sensor_ts_s = get_from_dict_by_map(self._parsed_data, self.aux_signals['ts_detections_s'])
        sensor_ts_frac = get_from_dict_by_map(self._parsed_data, self.aux_signals['ts_detections_frac'])
        sensor_ts = sensor_ts_s + sensor_ts_frac * 1e-9

        self.data_set.signals.loc[:, 'min_sensor_timestamp'] = np.min(sensor_ts, axis=1)
        self.data_set.signals.loc[:, 'max_sensor_timestamp'] = np.max(sensor_ts, axis=1)
        self.data_set.signals.loc[:, 'mean_sensor_timestamp'] = np.mean(sensor_ts, axis=1)
