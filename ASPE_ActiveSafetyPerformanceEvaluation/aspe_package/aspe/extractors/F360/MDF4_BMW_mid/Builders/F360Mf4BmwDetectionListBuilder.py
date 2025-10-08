import warnings

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360BmwDetectionList import F360BmwDetectionList
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.utilities.flatten_someip_object_list_data import filter_out_empty_slots, flatten_someip_signals


class F360Mdf4BmwDetectionListBuilder(IBuilder):
    """
    Class for extracting someIP detection list from parsed mf4 file.
    """
    detection_list_key = 'RecogSideRadarDetectionList'

    signal_mapper = {
        # BWM signature             ASPE signature
        'distance_value': 'range',
        'angle_azimuth_value': 'azimuth',
        'angle_elevation_value': 'elevation',
        'radial_velocity_value': 'range_rate',
        # signals which were taken from mask
        'sensor_id': 'sensor_id',
        'column': 'slot_id',
        'row': 'log_data_row',
    }

    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        super().__init__(parsed_data)
        self.data_set = F360BmwDetectionList()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = None

    def build(self):
        """
        Create and fill F360BmwDetectionList with data.
        :return: filled F360BmwDetectionList data set object
        """
        self._set_coordinate_system()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
        return self.data_set

    def _set_coordinate_system(self):
        self.data_set.coordinate_system = 'ISO_VRACS'

    def _extract_raw_signals(self):
        detections_dict = self._parsed_data[self.detection_list_key]
        sensor_dfs = []
        for sensor_id, sensor_data in enumerate(detections_dict.values()):
            sensor_data_flat = flatten_someip_signals(sensor_data, data_key='detections')
            number_of_detections = [scan['number_of_detections'] for scan in sensor_data.values()]
            sensor_df = filter_out_empty_slots(sensor_data_flat, number_of_detections)
            sensor_df['sensor_id'] = sensor_id
            sensor_dfs.append(sensor_df)
        self._raw_signals = pd.concat(sensor_dfs).reset_index(drop=True)

    def _extract_mappable_signals(self):
        for bmw_signature, aspe_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, bmw_signature].to_numpy()
            except KeyError:
                warnings.warn(f'Object list extraction warning! There is no signal {bmw_signature} inside raw_signals '
                              f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_non_mappable_signals(self):
        self._extract_unique_id()
        self._extract_variances()

    def _extract_unique_id(self):
        self.data_set.signals['unique_id'] = np.arange(len(self.data_set.signals))

    def _extract_variances(self):
        self.data_set.signals['range_variance'] = self._raw_signals['distance_std_dev_value'] ** 2
        self.data_set.signals['azimuth_variance'] = self._raw_signals['angle_azimuth_std_dev_value'] ** 2
        self.data_set.signals['elevation_variance'] = self._raw_signals['angle_elevation_std_dev_value'] ** 2
        self.data_set.signals['range_rate_variance'] = self._raw_signals[
                                                           'radial_velocity_std_dev'] ** 2  # without "_value"
