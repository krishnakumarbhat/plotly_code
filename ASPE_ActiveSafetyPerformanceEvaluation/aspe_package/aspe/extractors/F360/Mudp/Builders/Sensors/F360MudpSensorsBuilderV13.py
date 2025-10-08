# coding=utf-8
"""
F360 MUDP sensors builder (version 13)
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.Enums.f360_radars import F360RadarSensorMountingLocation, F360RadarSensorType
from aspe.extractors.F360.Mudp.Builders.Sensors.IF360MudpSensorsBuilder import IF360MudpSensorsBuilder
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts


class F360MudpSensorsBuilderV13(IF360MudpSensorsBuilder):
    """
    supports host extraction for stream definitions:
    ch: 3 - strdef_src035_str003_ver013.txt
    """
    required_stream_definitions = {'strdef_src035_str003_ver013'}
    main_stream = 3
    tracker_info_stream = 7
    sensors_key = [main_stream, 'sensorInfoLog']
    signal_mapper = {
        # ASPE signature            F360 signature
        'timestamp':                'timestamp',
        'unique_id':                'sensorID',
        'sensor_id':                'sensorID',
        'look_id':                  'look_id',
        'look_index':               'look_index',
        'sensor_valid':             'f_sens_valid',
        'new_measurement_update':   'new_measurement_update',
        'velocity_otg_x':           'vcs_long_vel',
        'velocity_otg_y':           'vcs_lat_vel',

    }
    auxiliary_signals = {
        'scan_index': [tracker_info_stream, 'tracker_index'],
        'fov': [main_stream, 'sensorInfoLog', 'useful_fov'],
        'look_id': [main_stream, 'sensorInfoLog', 'look_id'],
        'range_rate_interval_width': [main_stream, 'sensorInfoLog', 'range_rate_interval_width'],
    }
    properties = {
        'sensor_id': [main_stream, 'sensorInfoLog', 'sensorID'],
        'sensor_type': [main_stream, 'sensorInfoLog', 'sensor_type'],
        'polarity': [main_stream, 'sensorInfoLog', 'radar_polarity'],
        'mount_location': [main_stream, 'sensorInfoLog', 'mount_location'],
        'boresight_az_angle': [main_stream, 'sensorInfoLog', 'vcs_boresight_az_angle'],
        'boresight_elev_angle': [main_stream, 'sensorInfoLog', 'vcs_boresight_elev_angle'],
        'position_x': [main_stream, 'sensorInfoLog', 'vcs_long_posn'],
        'position_y': [main_stream, 'sensorInfoLog', 'vcs_lat_posn'],
        'position_z': [main_stream, 'sensorInfoLog', 'vcs_height_offset_m'],
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
        'LIDAR': -1,
        'VISION': -1,
        'VEHICLE': -1,
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
        'LIDAR': None,
        'VISION': None,
        'VEHICLE': None,
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._valid_sensors_ids = None

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._get_valid_sensors_idxs()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        return self.data_set

    def _get_valid_sensors_idxs(self):
        """
        Check which sensors are valid
        :return: list with idxs of valid sensors
        """
        sensors_valid = get_nested_array(self._parsed_data['parsed_data'], self.sensors_key + ['f_sens_valid'])
        idxs = []
        for i, idx in enumerate(sensors_valid.T):
            if np.all(idx == 1):
                idxs.append(i)
        self._valid_sensors_ids = idxs

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        sensors_valid = get_nested_array(self._parsed_data['parsed_data'], self.sensors_key + ['f_sens_valid'])
        signals_1d_default_shape = sensors_valid.shape[1]

        raw_signals_parsed_dict = get_nested_array(self._parsed_data['parsed_data'], self.sensors_key)
        for name, values in raw_signals_parsed_dict.items():
            if values.shape[1] == signals_1d_default_shape:
                converted_signal = values[:, self._valid_sensors_ids]
                if len(values.shape) == 2:
                    self._raw_signals[name] = converted_signal.flatten('F')
                elif len(values.shape) > 2:
                    # TODO: CEA-148; due to disagreement of handling - skip this
                    # converted_signal = nlr.unstructured_to_structured(array[:, self._valid_sensors_ids]).astype('O')
                    continue
            else:
                warn('Invalid number of columns in signal values, signal will not be extracted')

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

    def _extract_non_mappable_signals(self):
        """
        Extract signals which can not be strictly mapped using Mapper object.
        :return:
        """
        self._extract_scan_index()

    def _extract_scan_index(self):
        """
        Extracts scan index.
        :return:
        """
        scan_idx = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['scan_index']).flatten('F')
        self.data_set.signals['scan_index'] = np.hstack([scan_idx for _ in self._valid_sensors_ids])

    def _extract_properties(self):
        """
        Extraction of default properties from parsed_data by modifying parameter->properties with extracted data
        """
        self._extract_properties_per_sensor()
        self._extract_properties_per_look()
        self.data_set.coordinate_system = 'VCS'

    def _extract_properties_per_sensor(self):
        """
        Extract properties per sensor, sensor type and mount location are being mapped into their enum class
        counterparts
        """
        props = self.data_set.per_sensor
        for prop in props:
            property_values = get_nested_array(self._parsed_data['parsed_data'], self.properties[prop])
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

    def _extract_properties_per_look(self):
        """
        Extract properties per look, per sensor, such as:
        - hard coded for now: max range, min and max elevation
        - 3d signal FOV split into two signals min and max azimuth
        """
        sensors_type = self.data_set.per_sensor['sensor_type']
        sensors_ids = self.data_set.per_sensor['sensor_id']
        fov_array = get_nested_array(self._parsed_data['parsed_data'],
                                     self.auxiliary_signals['fov'])
        range_rate_interval_width_array = get_nested_array(self._parsed_data['parsed_data'],
                                                           self.auxiliary_signals['range_rate_interval_width'])
        look_id_array = get_nested_array(self._parsed_data['parsed_data'],
                                         self.auxiliary_signals['look_id'])

        props_per_look = dict.fromkeys(self.data_set.per_look.columns)
        for key in props_per_look.keys():
            props_per_look[key] = []

        for sensor_id in self._valid_sensors_ids:
            #   helpful variables for min_max azimuth calculation
            fov_per_sensor = fov_array[:, 2*sensor_id:2*sensor_id+2]
            look_per_sensor_2d = np.c_[look_id_array[:, sensor_id], look_id_array[:, sensor_id]]
            #   helpful variables for range_rate_interval_width calculation
            rng_rate_int_w_per_sensor = range_rate_interval_width_array[:, sensor_id]
            look_per_sensor_1d = look_id_array[:, sensor_id]

            look_ids = np.unique(look_id_array[:, sensor_id]).tolist()
            for look_id in look_ids:
                #   Mappable signals
                props_per_look['sensor_id'].append(sensors_ids[sensor_id])
                props_per_look['look_id'].append(look_id)

                #   Non-mappable signals
                azimuth_per_look = fov_per_sensor[np.where(look_per_sensor_2d == look_id)]
                props_per_look['min_azimuth'].append(azimuth_per_look.min())
                props_per_look['max_azimuth'].append(azimuth_per_look.max())

                rng_rate_int_w_per_look = rng_rate_int_w_per_sensor[np.where(look_per_sensor_1d == look_id)]
                unique_v = get_unique_value_with_most_counts(rng_rate_int_w_per_look, 'range_rate_interval_width')
                props_per_look['range_rate_interval_width'].append(unique_v)

                #   Hard coded signals
                props_per_look['max_range'].append(self.max_range[sensors_type[sensor_id]])
                props_per_look['min_elevation'].append(
                    self.min_max_elevation[sensors_type[sensor_id]][0])
                props_per_look['max_elevation'].append(
                    self.min_max_elevation[sensors_type[sensor_id]][1])
        self.data_set.per_look = pd.DataFrame.from_dict(props_per_look)
