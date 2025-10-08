# coding=utf-8
"""
F360 MUDP sensors builder (version 15)
"""
from collections import defaultdict
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.Enums.f360_radars import F360RadarSensorMountingLocation, F360RadarSensorType
from aspe.extractors.F360.Mudp.Builders.Sensors.IF360MudpSensorsBuilder import IF360MudpSensorsBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts


class F360MudpSensorsBuilderV15(IF360MudpSensorsBuilder):
    """
    supports host extraction for stream definitions:
    ch: 72
        - strdef_src035_str072_ver001

    requires:
    ch: 71
        - strdef_src035_str071_ver001
    """
    required_stream_definitions = [['strdef_src035_str072_ver001', 'strdef_src035_str072_ver002'],
                                   ['strdef_src035_str071_ver001', 'strdef_src035_str071_ver002']]
    main_stream = 72
    sensors_key = [main_stream]

    # Used to get data for extrapolation
    detections_stream = 71
    detections_key = [detections_stream]

    signal_mapper = {
        # ASPE signature                         F360 signature
        GeneralSignals.unique_id:                'id',
        GeneralSignals.sensor_id:                'id',
    }
    auxiliary_signals = {
        'scan_index': [main_stream, 'stream_ref_index'],
        'data_timestamp_us': [main_stream, 'data_timestamp_us'],
        'f360log_version': [main_stream, 'f360log_version'],
        'num_elements': [main_stream, 'num_elements'],
        'fov_min_az_rad': [main_stream, 'fov_min_az_rad'],
        'fov_max_az_rad': [main_stream, 'fov_max_az_rad'],
        'fov_min_el_rad': [main_stream, 'fov_min_el_rad'],
        'fov_max_el_rad': [main_stream, 'fov_max_el_rad'],
        'reference_scan_index': [detections_stream, 'stream_ref_index'],
    }
    properties = {
        'sensor_id': [main_stream, 'id'],
        'sensor_type': [main_stream, 'sensor_type'],
        'polarity': [main_stream, 'polarity'],
        'mount_location': [main_stream, 'mounting_location'],
        'boresight_az_angle': [main_stream, 'vcs_boresight_azimuth_angle'],
        'boresight_elev_angle': [main_stream, 'vcs_boresight_elevation_angle'],
        'position_x': [main_stream, 'vcs_position_longitudinal'],
        'position_y': [main_stream, 'vcs_position_lateral'],
        'position_z': [main_stream, 'vcs_position_height'],
        'min_azimuth': [main_stream, 'fov_min_az_rad'],
        'max_azimuth': [main_stream, 'fov_max_az_rad'],
        'v_wrapping': [main_stream, 'v_wrapping'],
        'r_wrapping': [main_stream, 'r_wrapping'],
    }

    max_range = {
        'UNKNOWN': -1,
        'SRR2_RADAR': 90,
        'SRR4_RADAR': 90,
        'SRR4_MM_RADAR': 90,
        'SRR5_RADAR': 90,
        'SRR6_PLUS_RADAR': 200,
        'ESR_RADAR': 220,
        'MRR1_RADAR': 220,
        'MRR2_RADAR': 220,
        'MRR3_RADAR': 220,
        'MRR360_RADAR': 180,
        'FLR4_PLUS_RADAR': 320,
        'SRR7_PLUS_RADAR': 250,
        'FLR7_RADAR': 300,
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
        'SRR6_PLUS_RADAR': elevation,
        'ESR_RADAR': elevation,
        'MRR1_RADAR': elevation,
        'MRR2_RADAR': elevation,
        'MRR3_RADAR': elevation,
        'MRR360_RADAR': elevation,
        'FLR4_PLUS_RADAR': (-10, 10),
        'SRR7_PLUS_RADAR': elevation,
        'FLR7_RADAR': (-14.5, 14.5),
        'LIDAR': None,
        'VISION': None,
        'VEHICLE': None,
        'DEFAULT': elevation,
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals, f_reduced_sensor_stream=True)
        self._valid_sensors_ids = None

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._extract_valid_sensors_idxs()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._filter_valid_sensors()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _extract_valid_sensors_idxs(self):
        """
        Extract valid sensor index and indicate sensor configuration
        """
        sensors_valid_arr = get_nested_array(self._parsed_data['parsed_data'], self.properties['sensor_id'])
        self._f_single_sensor_config = True if sensors_valid_arr.ndim == 1 else False
        self._valid_sensors_ids = np.where(np.sum(sensors_valid_arr, axis=0) != 0)[0]

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        signals_1d_default_shape = len(self._valid_sensors_ids)
        raw_signals_parsed_dict = get_nested_array(self._parsed_data['parsed_data'], self.sensors_key)

        for name, values in raw_signals_parsed_dict.items():
            if len(values.shape) == 1:
                converted_signal = np.hstack([values for _ in self._valid_sensors_ids])
            elif values.shape[1] == signals_1d_default_shape:
                converted_signal = values[:, self._valid_sensors_ids]

            if len(values.shape) <= 2:
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

    def _extract_non_mappable_signals(self):
        """
        Extract signals which can not be strictly mapped using Mapper object.
        :return:
        """
        self._extract_scan_index()
        self._extract_timestamp()

    def _extract_scan_index(self):
        """
        Extracts scan index.
        :return:
        """
        scan_idx = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['scan_index']).flatten('F')
        self.data_set.signals['scan_index'] = np.hstack([scan_idx for _ in self._valid_sensors_ids])

    def _extract_timestamp(self):
        """
        Extracts timestamp.
        :return:
        """
        timestamp = get_nested_array(self._parsed_data['parsed_data'],
                                     self.auxiliary_signals['data_timestamp_us']).flatten('F')
        self.data_set.signals['timestamp'] = np.hstack([timestamp for _ in self._valid_sensors_ids])

    def _extract_properties(self):
        """
        Extraction of default properties from parsed_data by modifying parameter->properties with extracted data
        """
        self._extract_properties_per_sensor()
        self.data_set.coordinate_system = 'VCS'

    def _extract_properties_per_sensor(self):
        """
        Extract properties per sensor, sensor type and mount location are being mapped into their enum class
        counterparts
        """
        props = self.data_set.per_sensor
        stream_num = self.properties['v_wrapping'][0]

        if 'v_wrapping' not in self._parsed_data['parsed_data'][stream_num]:
            self.properties['v_wrapping'][1] = 'range_rate_interval_width'

        for prop in props:
            # make sure prop not in hard coded values (previously handled in 'per_look')
            if prop in ['max_range', 'max_elevation', 'min_elevation', 'range_rate_interval_width']:
                continue

            property_values = get_nested_array(self._parsed_data['parsed_data'], self.properties[prop])

            if prop == 'r_wrapping' and self.properties['v_wrapping'][1] == 'range_rate_interval_width':
                v_wrapping_array = self._parsed_data['parsed_data'][stream_num]['range_rate_interval_width']
                property_values = np.zeros_like(v_wrapping_array)


            valid_values = property_values[:, self._valid_sensors_ids]\
                if not self._f_single_sensor_config else property_values

            values_with_most_counts = []
            for sensor_id in self._valid_sensors_ids:

                value_with_most_counts = get_unique_value_with_most_counts(valid_values[:, sensor_id], prop)\
                    if not self._f_single_sensor_config else get_unique_value_with_most_counts(valid_values, prop)
                values_with_most_counts.append(value_with_most_counts)

            if prop == 'sensor_type':
                get_sensors_type = np.vectorize(lambda x: F360RadarSensorType(x).name)
                props[prop] = get_sensors_type(values_with_most_counts)
            elif prop == 'mount_location':
                get_sensors_mounting_location = np.vectorize(lambda x: F360RadarSensorMountingLocation(x).name)
                props[prop] = get_sensors_mounting_location(values_with_most_counts)
            else:
                props[prop] = values_with_most_counts

        # Deal with hard coded values
        for sensor_id in self._valid_sensors_ids:
            sensor_type = props['sensor_type'][sensor_id]
            if sensor_type in self.max_range.keys() and sensor_type in self.min_max_elevation.keys():
                props.loc[:, ('max_range', sensor_id)] = self.max_range[sensor_type]
                props['min_elevation'] = self.min_max_elevation[sensor_type][0] if self.min_max_elevation[sensor_type][0] is not None else np.nan
                props['max_elevation'] = self.min_max_elevation[sensor_type][1] if self.min_max_elevation[sensor_type][1] is not None else np.nan
            else:
                props.loc[:, ('max_range', sensor_id)] = np.nan
                props['min_elevation'] = np.nan
                props['max_elevation'] = np.nan

    def _filter_valid_sensors(self):
        valid_id_map = self._raw_signals.loc[:, 'id'] > 0
        self._raw_signals = self._raw_signals.loc[valid_id_map, :].reset_index(drop=True)
        self.data_set.signals = self.data_set.signals.loc[valid_id_map, :].reset_index(drop=True)
