import warnings
from collections import defaultdict
from datetime import datetime

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, recursive_dict_extraction


class NexusDetectionListBuilder(IBuilder):
    signal_mapper = {
        # ASPE signature             'nexus signature
        'range_rate':                'rangeRate_value',
        'azimuth':                   'azimuth_value',
        'amplitude':                 'amplitude',
        'range':                     'range_value',
        'scan_index':                'scan_index',
        'sensor_id':                 'sensor_id',
    }

    def __init__(self, parsed_data, save_raw_signals=False, sensor_data=None):
        super().__init__(parsed_data)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._raw_signals = None
        self.data_set = IRadarDetections()
        self.save_raw_signals = save_raw_signals
        self.sensor_data = sensor_data

    def build(self):
        self.extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._set_scan_indexes()
        return self.data_set

    def extract_raw_signals(self):

        radar_detections_samples = self._parsed_data['radar_detections']['radar_detections_samples']
        radar_detections_info = self._parsed_data['radar_detections']['radar_detections_info']

        detection_flat, nested_dict_map = recursive_dict_extraction(radar_detections_samples[0][0].detections[0])
        raw_signals_dict = defaultdict(list)
        for dets_info, detections_per_radar in zip(radar_detections_info, radar_detections_samples):
            sensor_nexus_id = dets_info.sensor
            sensor_id = self.sensor_data.loc[self.sensor_data['nexus_id'] == sensor_nexus_id, 'sensor_id'].to_numpy()[0]
            for sample in detections_per_radar:
                detections_per_radar = sample.detections
                for detection in detections_per_radar:
                    raw_signals_dict['timestamp'].append(sample.timestamp)
                    raw_signals_dict['sensor_id'].append(sensor_id)
                    raw_signals_dict['scan_index'].append(sample.scanIndex)
                    for k, v in nested_dict_map.items():
                        raw_signals_dict[k].append(get_from_dict_by_map(detection, v))

        self._raw_signals = pd.DataFrame(raw_signals_dict)

        if self.save_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self._extract_position()
        self._extract_variances()
        self._extract_unique_id()

    def _extract_variances(self):
        self.data_set.signals['range_variance'] = self._raw_signals['range_stddev'] ** 2
        self.data_set.signals['azimuth_variance'] = self._raw_signals['azimuth_stddev'] ** 2
        self.data_set.signals['elevation_variance'] = self._raw_signals['elevation_stddev'] ** 2
        self.data_set.signals['range_rate_variance'] = self._raw_signals['range_stddev'] ** 2

    def _extract_unique_id(self):
        self.data_set.signals['unique_id'] = self.data_set.signals.index[:]

    def _extract_position(self):
        for sensor_id, sensor_dets in self.data_set.signals.groupby(by='sensor_id'):
            sensors = self.sensor_data.set_index('sensor_id')
            bsa = sensors.loc[sensor_id, 'boresight_az_angle']
            sensor_x = sensors.loc[sensor_id, 'position_x']
            sensor_y = sensors.loc[sensor_id, 'position_y']
            polarity = sensors.loc[sensor_id, 'polarity']

            sensor_dets['azimuth_vcs'] = polarity * sensor_dets['azimuth'] + bsa
            position_y = np.sin(sensor_dets['azimuth_vcs']) * sensor_dets['range']
            position_x = np.cos(sensor_dets['azimuth_vcs']) * sensor_dets['range']

            self.data_set.signals.loc[sensor_dets.index, 'position_y'] = position_y + sensor_y
            self.data_set.signals.loc[sensor_dets.index, 'position_x'] = position_x + sensor_x
            self.data_set.signals.loc[sensor_dets.index, 'azimuth_vcs'] = sensor_dets['azimuth_vcs']

    def _extract_mappable_signals(self):
        for aspe_signature, nexus_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, nexus_signature].to_numpy()
            except KeyError:
                warnings.warn(
                    f'Object list extraction warning! There is no signal {nexus_signature} inside raw_signals '
                    f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_timestamp(self):
        def timestamp_str_to_unix_time(ts: str):
            return datetime.fromisoformat(ts).timestamp()

        timestamps = self._raw_signals.loc[:, 'timestamp'].to_numpy().astype('str')
        timestamps = np.char.replace(timestamps, 'Z', '')

        self.data_set.signals.loc[:, 'timestamp'] = np.array([timestamp_str_to_unix_time(t) for t in timestamps])

    def _set_scan_indexes(self):
        radar_time_scan_info = self.data_set.signals[['scan_index', 'sensor_id']]
        sensors = radar_time_scan_info['sensor_id'].unique()
        for id in sensors:
            dets_per_sensor = self.data_set.signals['scan_index'][self.data_set.signals['sensor_id'] == id]
            min_scan_index = dets_per_sensor.min()
            max_scan_index = dets_per_sensor.max()
            n = max_scan_index - min_scan_index + 1
            self.data_set.signals.loc[dets_per_sensor.index, 'scan_index'] = dets_per_sensor.replace(
                np.arange(min_scan_index, max_scan_index + 1), np.arange(n))
