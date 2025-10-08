from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder
from aspe.extractors.F360.DataSets.F360RadarSensors import F360RadarSensors
from aspe.extractors.F360.Enums.f360_radars import F360RadarSensorMountingLocation, F360RadarSensorType
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts


class ENVKegRadarSensorsBuilder(ENVKegBuilder):
    # Mapping of radar sensor signals between ASPE and ENV. This is only concerned with signals that can be directly
    # mapped (no conversion needed)
    signal_mapper = {
        # ASPE signature:               ENV bin signature:
        GeneralSignals.unique_id:       'constant.id',
        GeneralSignals.sensor_id:       'constant.id',
        GeneralSignals.look_id:         'variable.look_id',
        GeneralSignals.look_index:      'variable.look_index',
        GeneralSignals.sensor_valid:    'variable.is_valid',
        PhysicalSignals.velocity_otg_x: 'variable.vcs_velocity.longitudinal',
        PhysicalSignals.velocity_otg_y: 'variable.vcs_velocity.lateral',
    }

    properties = {
        'sensor_id': ['RADAR_PARAMS', 'sensors', 'constant.id'],
        'sensor_type': ['RADAR_PARAMS', 'sensors', 'constant.sensor_type'],
        'polarity': ['RADAR_PARAMS', 'sensors', 'constant.polarity'],
        'mount_location': ['RADAR_PARAMS', 'sensors', 'constant.mounting_location'],
        'boresight_az_angle': ['RADAR_PARAMS', 'sensors', 'constant.mounting_position.vcs_boresight_azimuth_angle'],
        'boresight_elev_angle': ['RADAR_PARAMS', 'sensors', 'constant.mounting_position.vcs_boresight_elevation_angle'],
        'position_x': ['RADAR_PARAMS', 'sensors', 'constant.mounting_position.vcs_position.longitudinal'],
        'position_y': ['RADAR_PARAMS', 'sensors', 'constant.mounting_position.vcs_position.lateral'],
        'position_z': ['RADAR_PARAMS', 'sensors', 'constant.mounting_position.vcs_position.height'],
        'min_fov_azimuth': ['RADAR_PARAMS', 'sensors', 'constant.fov_min_az_rad'],
        'max_fov_azimuth': ['RADAR_PARAMS', 'sensors', 'constant.fov_max_az_rad'],
    }

    auxiliary_signals = {
        'is_valid': ['RADAR_PARAMS', 'sensors', 'variable.is_valid'],
        'look_id': ['RADAR_PARAMS', 'sensors', 'variable.look_id'],
        'v_wrapping': [['RADAR_PARAMS', 'sensors', 'constant.rng_rate_interval_widths'],
                       ['RADAR_PARAMS', 'sensors', 'constant.v_wrapping']],
        'r_wrapping': ['RADAR_PARAMS', 'sensors', 'constant.r_wrapping'],
    }

    max_range = {
        'UNKNOWN': -1,
        'SRR2_RADAR': 90,
        'SRR4_RADAR': 90,
        'SRR4_MM_RADAR': 90,
        'SRR5_RADAR': 90,
        'ESR_RADAR': 220,
        'MRR1_RADAR': 220,
        'MRR2_RADAR': 220,
        'MRR3_RADAR': 220,
        'MRR360_RADAR': 220,
        'FLR4_PLUS_RADAR': 350,
        'LIDAR': -1,
        'VISION': -1,
        'VEHICLE': -1,
        'DEFAULT': 90,
    }
    elevation = (-5, 5)
    min_max_elevation = {
        'UNKNOWN': None,
        'SRR2_RADAR': elevation,
        'SRR4_RADAR': elevation,
        'SRR4_MM_RADAR': elevation,
        'SRR5_RADAR': elevation,
        'ESR_RADAR': elevation,
        'MRR1_RADAR': elevation,
        'MRR2_RADAR': elevation,
        'MRR3_RADAR': elevation,
        'MRR360_RADAR': elevation,
        'FLR4_PLUS_RADAR': elevation,
        'LIDAR': None,
        'VISION': None,
        'VEHICLE': None,
        'DEFAULT': elevation,
    }

    def __init__(self, parsed_data, timestamps, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self.data_set = F360RadarSensors()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()
        self._parsed_data = parsed_data
        self._timestamps = timestamps

    def build(self) -> F360RadarSensors:
        """
        Extracts radar sensors from parsed data.

        :return: an instance of F360RadarSensors with extracted signals.
        """
        self._get_valid_sensors_idxs()
        self._extract_raw_signals()
        self._extract_radar_sensor_signals()
        self._extract_properties()
        self._filter_valid_sensors()

        return self.data_set

    def _get_valid_sensors_idxs(self):
        """
        Check which sensors are valid

        :return: list with idxs of valid sensors
        """
        sensors_valid_arr = get_nested_array(self._parsed_data, self.auxiliary_signals['is_valid'])
        self._valid_sensors_ids = np.where(np.sum(sensors_valid_arr, axis=0) != 0)[0]

    def _extract_raw_signals(self):  # TODO FZE-400 correct description
        """Extract raw ENV radar sensor signals into a dataframe."""

        sensors_valid = get_nested_array(self._parsed_data, self.auxiliary_signals['is_valid'])
        signals_1d_default_shape = sensors_valid.shape[1]

        sensor_signals = self._parsed_data['RADAR_PARAMS']

        for value in sensor_signals.values():
            for signal_signature, array in value.items():
                if array.shape[1] == signals_1d_default_shape:
                    converted_signal = array[:, self._valid_sensors_ids]
                    if len(array.shape) == 2:
                        self._raw_signals[signal_signature] = converted_signal.flatten('F')
                else:
                    warn('Invalid number of columns in signal values, signal will not be extracted')

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_radar_sensor_signals(self):
        self._extract_mappable_signals()
        self._extract_timestamp()
        self._extract_scan_index()
        self._extract_sensor_velocity_otg_variance()

    def _extract_properties(self):
        self.data_set.coordinate_system = 'VCS'
        self._extract_properties_per_sensor()
        self._extract_properties_per_look()

    def _extract_properties_per_sensor(self):
        """
        Extract properties per sensor, sensor type and mount location are being mapped into their enum class
        counterparts
        """
        props = self.data_set.per_sensor
        for prop in props:
            property_values = get_nested_array(self._parsed_data, self.properties[prop])
            valid_values = property_values[:, self._valid_sensors_ids]

            values_with_most_counts = []
            for sensor_id in self._valid_sensors_ids:
                value_with_most_counts = get_unique_value_with_most_counts(valid_values[:, sensor_id], prop)
                values_with_most_counts.append(value_with_most_counts)

            if prop == 'sensor_type':
                get_sensors_type = np.vectorize(lambda x: F360RadarSensorType(x).name)
                props[prop] = get_sensors_type(values_with_most_counts)
            elif prop == 'mount_location':
                get_sensors_mounting_location = np.vectorize(lambda x: F360RadarSensorMountingLocation(x).name)
                props[prop] = get_sensors_mounting_location(values_with_most_counts)
            else:
                props[prop] = values_with_most_counts
        self.data_set.per_sensor.reset_index(drop=True, inplace=True)

    def _extract_properties_per_look(self):
        """
        Extract properties per look, per sensor, such as:
        - hard coded for now: max range, min and max elevation
        - 3d signal FOV split into two signals min and max azimuth
        """
        sensors_type = self.data_set.per_sensor['sensor_type']
        sensors_ids = self.data_set.per_sensor['sensor_id']
        min_fov_az = get_nested_array(self._parsed_data, self.properties['min_fov_azimuth'])
        max_fov_az = get_nested_array(self._parsed_data, self.properties['max_fov_azimuth'])

        if 'constant.v_wrapping' in self._parsed_data['RADAR_PARAMS']['sensors']:
            v_wrapping_array = get_nested_array(self._parsed_data,
                                                self.auxiliary_signals['v_wrapping'][1])
        else:
            v_wrapping_array = get_nested_array(self._parsed_data,
                                                self.auxiliary_signals['v_wrapping'][0])
        r_wrapping_array = get_nested_array(self._parsed_data,
                                            self.auxiliary_signals['r_wrapping'])
        if r_wrapping_array is None:
            r_wrapping_array = np.zeros_like(v_wrapping_array)

        look_id_array = get_nested_array(self._parsed_data, self.auxiliary_signals['look_id'])

        if look_id_array is not None:
            props_per_look = dict.fromkeys(self.data_set.per_look.columns)
            for key in props_per_look:
                props_per_look[key] = []

            for sensor_id in self._valid_sensors_ids:
                #   useful variables for min_max azimuth calculation
                look_per_sensor_2d = np.c_[look_id_array[:, sensor_id], look_id_array[:, sensor_id]]
                #   useful variables for range_rate_interval_width calculation
                sensor_range_rate_intervals = v_wrapping_array[:, sensor_id, :]
                sensor_range_offsets = r_wrapping_array[:, sensor_id, :]

                look_ids = np.unique(look_id_array[:, sensor_id]).tolist()
                for look_id in look_ids:
                    #   Mappable signals
                    props_per_look['sensor_id'].append(sensors_ids[sensor_id])
                    props_per_look['look_id'].append(look_id)

                    #   Non-mappable signals
                    props_per_look['min_azimuth'].append(np.median(min_fov_az[np.where(look_per_sensor_2d == look_id)]))
                    props_per_look['max_azimuth'].append(np.median(max_fov_az[np.where(look_per_sensor_2d == look_id)]))

                    rng_rate_int_w_per_look = sensor_range_rate_intervals[:, look_id]
                    rng_offset_per_look = sensor_range_offsets[:, look_id]
                    unique_v = get_unique_value_with_most_counts(rng_rate_int_w_per_look, 'v_wrapping')
                    unique_r = get_unique_value_with_most_counts(rng_offset_per_look, 'r_wrapping')
                    props_per_look['v_wrapping'].append(unique_v)
                    props_per_look['r_wrapping'].append(unique_r)

                    #   Hard coded signals
                    sensor_type = sensors_type[sensor_id]
                    if sensor_type in self.max_range:
                        max_range = self.max_range[sensor_type]
                    else:
                        max_range = self.max_range['DEFAULT']
                        warn(f'SensorBuilder: Max range for {sensor_type} not found. Using default value.')

                    if sensor_type in self.min_max_elevation:
                        min_max_elevation = self.min_max_elevation[sensor_type]
                    else:
                        min_max_elevation = self.min_max_elevation['DEFAULT']
                        warn(f'SensorBuilder: Min/max elevation for {sensor_type} not found. Using default value.')

                    min_elevation = min_max_elevation[0] if min_max_elevation is not None else np.nan
                    max_elevation = min_max_elevation[1] if min_max_elevation is not None else np.nan

                    props_per_look['max_range'].append(max_range)
                    props_per_look['min_elevation'].append(min_elevation)
                    props_per_look['max_elevation'].append(max_elevation)
            self.data_set.per_look = pd.DataFrame.from_dict(props_per_look)
            self.data_set.per_look.reset_index(drop=True, inplace=True)

    def _extract_timestamp(self):
        try:
            self.data_set.signals['timestamp'] = self._raw_signals['variable.timestamp_us'] * 1e-6
        except KeyError:
            warn('Signal timestamp_us is not found, timestamp will not be extracted')

    def _extract_sensor_velocity_otg_variance(self):
        # Workaround for extraction of sensor velocity variance
        # TODO: Write this signal to binary output and map to ASPE signature
        self.data_set.signals['velocity_otg_variance_x'] = 0.0
        self.data_set.signals['velocity_otg_variance_y'] = 0.0
        self.data_set.signals['velocity_otg_covariance'] = 0.0

    def _extract_fov_azimuth(self, raw_looks_df):
        # Extract minimum and maximum field of view azimuth
        min_az_nx = raw_looks_df['fov_minaz_nx'].to_numpy()
        min_az_ny = raw_looks_df['fov_minaz_ny'].to_numpy()
        min_az_n = np.arctan2(min_az_ny, min_az_nx)

        max_az_nx = raw_looks_df['fov_maxaz_nx'].to_numpy()
        max_az_ny = raw_looks_df['fov_maxaz_ny'].to_numpy()
        # Add Pi to the maximum azimuth angle because the normal vector, after being calculated, is reversed to point
        # the boresight
        max_az_n = np.arctan2(max_az_ny, max_az_nx) + np.pi

        fov_half_angles = 0.5 * (max_az_n - min_az_n)
        self.data_set.per_look['max_azimuth'] = fov_half_angles
        self.data_set.per_look['min_azimuth'] = -fov_half_angles

    def _extract_scan_index(self):
        # TODO FZE-400 BEFORE MERGE check if this warning is needed, maybe handle it differently.
        # TODO If trackerInfo will be present in parsed data use it.
        warn('Tracker info stream or tracker_index signal not present in'
             ' parsed data, scan_index will start from 0')
        scan_idx = np.arange(self._timestamps.shape[0])
        self.data_set.signals['scan_index'] = np.hstack([scan_idx for _ in self._valid_sensors_ids])

    def _filter_valid_sensors(self):
        valid_id_map = self._raw_signals.loc[:, 'variable.is_valid'] > 0
        self._raw_signals = self._raw_signals.loc[valid_id_map, :].reset_index(drop=True)
        self.data_set.signals = self.data_set.signals.loc[valid_id_map, :].reset_index(drop=True)


