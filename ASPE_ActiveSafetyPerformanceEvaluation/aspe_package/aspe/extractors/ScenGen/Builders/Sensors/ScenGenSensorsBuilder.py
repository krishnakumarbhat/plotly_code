"""
Scenario Generator sensors builder
"""

from warnings import warn

import numpy as np
import pandas as pd
from scipy.io.matlab.mio5_params import MatlabOpaque

from aspe.extractors.ScenGen.Builders.ScenGenBuilder import ScenGenBuilder
from aspe.extractors.ScenGen.DataSets.ScenGenRadarSensors import ScenGenRadarSensors
from aspe.extractors.ScenGen.Enums.ScenGenRadarsMountingLocation import ScenGenRadarsMountingLocation


class MappableSignalsExtractionWarning(UserWarning):
    """Custom warning for mappable signals extraction."""
    pass


class NonMappableSignalsExtractionWarning(UserWarning):
    """Custom warning for non-mappable signals extraction."""
    pass


class ScenGenSensorsBuilder(ScenGenBuilder):
    per_sensor_properties = {
        'sensor_id': 'sensor_id',
        'boresight_az_angle': 'bsa_vcs',
        'position_x': 'xposn_vcs',
        'position_y': 'yposn_vcs',
    }

    per_look_properties = {
        'min_azimuth': 'minazim',
        'max_azimuth': 'maxazim',
        'max_range': 'maxrng',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super(ScenGenSensorsBuilder, self).__init__(parsed_data)

        self._raw_signals = None
        self._valid_senors_ids = None
        self.data_set = ScenGenRadarSensors()

        self._f_extract_raw_signals = f_extract_raw_signals

    def build(self):
        """Main extraction function."""

        self._get_valid_senors_idx()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()

        return self.data_set

    def _get_valid_senors_idx(self):
        """Get valid sensor ids from vehicle data."""
        self._valid_senors_ids = self._parsed_data['vehicle_data']['common']['active_sensors']

    def _extract_raw_signals(self):
        sensors_config = self._parsed_data['sensors_config']
        valid_sensors_config = [sensor for idx, sensor in enumerate(sensors_config) if self._valid_senors_ids[idx] != 0]

        df_index = pd.RangeIndex(0, len(valid_sensors_config))
        df_columns = list(valid_sensors_config[0].keys())
        df_columns.append('sensor_id')
        self._raw_signals = pd.DataFrame(index=df_index, columns=df_columns)

        for sensor_idx, sensor in enumerate(valid_sensors_config):
            self._raw_signals['sensor_id'].iloc[sensor_idx] = sensor_idx
            for name, value in sensor.items():
                if isinstance(value, MatlabOpaque):
                    warn(f'{name} saved as Matlab object. Cannot be extracted.')
                else:
                    self._raw_signals[name].iloc[sensor_idx] = value

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_mappable_signals(self):
        warn('No time dependent sensors signals in Scenario Generator data. Signals structure will not be filled with '
             'mappable signals.', MappableSignalsExtractionWarning)

    def _extract_non_mappable_signals(self):
        warn('No time dependent sensors signals in Scenario Generator data. Signals structure will not be filled with '
             'non-mappable signals.', NonMappableSignalsExtractionWarning)

    def _extract_properties(self):
        self._extract_properties_per_sensor()
        self._extract_properties_per_look()
        self.coordinate_system = 'VCS'

    def _extract_properties_per_sensor(self, ):
        """
        Extract properties per sensor.

        Properties per sensor signals:
            - sensor_id - extracted from raw signals
            - sensor_type - not present
            - polarity - not present
            - mount_location - extracted from detections, mapped with enum
            - boresight_az_angle - extracted from raw signals
            - boresight_elev_angle - not present
            - position_x - extracted from raw signals
            - position_y - extracted from raw signals
            - position_z - not present
        """
        props = self.data_set.per_sensor

        dets = self._parsed_data['detections']

        for prop in props:
            if prop in self.per_sensor_properties:
                key = self.per_sensor_properties[prop]
                props[prop] = self._raw_signals[key]
            elif prop == 'mount_location':
                val, idx = np.unique(dets['sensor_mount_loc'], return_index=True)
                mount_loc_values = val[idx.argsort()]

                get_sensors_mounting_location = np.vectorize(
                    lambda x: ScenGenRadarsMountingLocation(x).name)
                props[prop] = get_sensors_mounting_location(mount_loc_values)

    def _extract_properties_per_look(self):
        """
        Extract properties per look.

        Properties per look signals:
            - sensor_id - extracted from per_sensor
            - look_id - extracted from detections
            - max_range - extracted from raw signals
            - min_elevation - not present
            - max_elevation - not present
            - min_azimuth - extracted from raw signals
            - max_azimuth - extracted from raw signals
            - range_rate_interval_width - not present
        """

        def get_look_ids() -> list:
            """
            Get look ids for given sensor id.
            @return: list of look ids
            """
            val, idx = np.unique(dets['sensor_mount_loc'], return_index=True)
            mount_loc_values = val[idx.argsort()]
            mask = dets['sensor_mount_loc'] == mount_loc_values[sensor_id]
            return np.unique(dets['sensor_short_long_look'][mask]).astype(int).tolist()

        def get_property():
            """Gets single property and adds it to dictionary depending on the certain conditions."""
            if prop in self.per_look_properties.keys():
                key = self.per_look_properties[prop]
                val = self._raw_signals[key][sensor_id]
                props_per_look[prop].append(val)
            elif prop == 'sensor_id':
                props_per_look[prop].append(sensors_ids[sensor_id])
            elif prop == 'look_id':
                props_per_look['look_id'].append(look_id)
            else:
                props_per_look[prop].append(np.nan)

        dets = self._parsed_data['detections']
        sensors_ids = self.data_set.per_sensor['sensor_id']

        props_per_look = dict.fromkeys(self.data_set.per_look.columns)
        for key in props_per_look.keys():
            props_per_look[key] = []

        for sensor_id in sensors_ids:
            look_ids = get_look_ids()
            for look_id in look_ids:
                for prop in props_per_look.keys():
                    get_property()
        self.data_set.per_look = pd.DataFrame.from_dict(props_per_look)
