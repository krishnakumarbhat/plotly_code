#sensor
# coding=utf-8
"""
F360 XTRK sensors builder
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360RadarSensors import F360RadarSensors
from aspe.extractors.F360.Enums.f360_radars import F360RadarSensorMountingLocation, F360RadarSensorType
from aspe.extractors.F360.Xtrk.builders.F360XtrkBuilder import F360XtrkBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts


class F360XtrkSensorsBuilder(F360XtrkBuilder):
    """
    Supports sensor extraction from .xtrk file.
    """

    signal_mapper = {
        # ASPE signature                         # f360 signature - if it's list builder ckecks if any of
                                                 # listed signals is in f360 signals list - multiple version handling
        GeneralSignals.unique_id:                ['id', 'constant.id'],
        GeneralSignals.sensor_id:                ['id', 'constant.id'],
        GeneralSignals.look_id:                  ['look_id', 'variable.look_id'],
        GeneralSignals.look_index:               ['look_index', 'variable.look_index'],
        GeneralSignals.sensor_valid:             ['is_valid', 'variable.is_valid'],
        PhysicalSignals.velocity_otg_x:          ['vcs_velocity.longitudinal', 'variable.vcs_velocity.longitudinal'],
        PhysicalSignals.velocity_otg_y:          ['vcs_velocity.lateral', 'variable.vcs_velocity.lateral'],
    }

    # Signal mapping, plain list corresponds to nested location in structure, list of lists can be used to handle
    # multiple possible signal locations
    auxiliary_signals = {
        'scan_index': ['trkrInfo', 'cnt_loops'],
        'is_valid': [['sensor_calibs', 'is_valid'], ['sensors', 'variable.is_valid']],
        'timestamp_us': [['sensors', 'timestamp_us'], ['sensors', 'variable.timestamp_us']],
        'look_id': [['sensors', 'look_id'], ['sensors', 'variable.look_id']],
        'v_wrapping': [['sensor_calibs', 'rng_rate_interval_widths'],
                       ['sensors', 'constant.rng_rate_interval_widths'],
                       ['sensors', 'constant.v_wrapping']],
        'r_wrapping': ['sensors', 'constant.r_wrapping'],
    }

    # Properties mapping, plain list corresponds to nested location in structure, list of lists can be used to handle
    # multiple possible signal locations
    properties = {
        'sensor_id': [['sensor_calibs', 'id'], ['sensors', 'constant.id']],
        'sensor_type': [['sensor_calibs', 'sensor_type'], ['sensors', 'constant.sensor_type']],
        'polarity': [['sensor_calibs', 'polarity'], ['sensors', 'constant.polarity']],
        'mount_location': [['sensor_calibs', 'mounting_location'], ['sensors', 'constant.mounting_location']],
        'boresight_az_angle': [['sensor_calibs', 'mounting_position.vcs_boresight_azimuth_angle'],
                               ['sensors', 'constant.mounting_position.vcs_boresight_azimuth_angle']],
        'boresight_elev_angle': [['sensor_calibs', 'mounting_position.vcs_boresight_elevation_angle'],
                                 ['sensors', 'constant.mounting_position.vcs_boresight_elevation_angle']],
        'position_x': [['sensor_calibs', 'mounting_position.vcs_position.longitudinal'],
                       ['sensors', 'constant.mounting_position.vcs_position.longitudinal']],
        'position_y': [['sensor_calibs', 'mounting_position.vcs_position.lateral'],
                       ['sensors', 'constant.mounting_position.vcs_position.lateral']],
        'position_z': [['sensor_calibs', 'mounting_position.vcs_position.height'],
                       ['sensors', 'constant.mounting_position.vcs_position.height']],
        'min_fov_azimuth': [['sensor_calibs', 'fov_min_az_rad'],
                            ['sensors', 'constant.fov_min_az_rad']],
        'max_fov_azimuth': [['sensors', 'constant.fov_max_az_rad'],
                            ['sensor_calibs', 'fov_max_az_rad']],
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
        super().__init__(parsed_data)
        self._f_extract_raw_signals = f_extract_raw_signals
        self._valid_sensors_ids = None
        self.data_set = F360RadarSensors()
        self._raw_signals = pd.DataFrame()
        self.auxiliary_raw_signals = self._get_auxiliary_raw_signals_mapping(self.auxiliary_signals)

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._get_valid_sensors_idxs()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._filter_valid_sensors()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _get_valid_sensors_idxs(self):
        """
        Check which sensors are valid
        :return: list with idxs of valid sensors
        """
        sensors_valid_arr = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                             self.auxiliary_signals['is_valid'])
        self._valid_sensors_ids = np.where(np.sum(sensors_valid_arr, axis=0) != 0)[0]

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        sensors_valid = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                         self.auxiliary_signals['is_valid'])
        signals_1d_default_shape = sensors_valid.shape[1]

        sensors = self._parsed_data.get('sensors', pd.DataFrame())
        sensor_props = self._parsed_data.get('sensor_props', pd.DataFrame())
        sensor_calibs = self._parsed_data.get('sensor_calibs', pd.DataFrame())
        sensor_signals = {**sensors, **sensor_props, **sensor_calibs}

        for name, values in sensor_signals.items():
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

        scan_idx = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                    self.auxiliary_signals['scan_index']).flatten('F')
        if scan_idx is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_idx = np.arange(self._parsed_data_shape[0])
        self.data_set.signals['scan_index'] = np.hstack([scan_idx for _ in self._valid_sensors_ids])

    def _extract_timestamp(self):
        """
        Extracts timestamp.
        :return:
        """
        try:
            self.data_set.signals['timestamp'] = \
                self._get_nested_array_using_possible_signatures(self._raw_signals,
                                                                 self.auxiliary_raw_signals['timestamp_us']) * 1e-6
        except KeyError:
            warn('Signal timestamp_us is not found, timestamp will not be extracted')

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
            property_values = self._get_nested_array_using_possible_signatures(self._parsed_data, self.properties[prop])
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
        min_fov_az = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                      self.properties['min_fov_azimuth'])
        max_fov_az = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                      self.properties['max_fov_azimuth'])

        v_wrapping_array = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                                           self.auxiliary_signals['v_wrapping'])
        r_wrapping_array = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                            self.auxiliary_signals['r_wrapping'])
        if r_wrapping_array is None:
            r_wrapping_array = np.zeros_like(v_wrapping_array)

        look_id_array = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                         self.auxiliary_signals['look_id'])

        props_per_look = dict.fromkeys(self.data_set.per_look.columns)
        for key in props_per_look:
            props_per_look[key] = []

        for sensor_id in self._valid_sensors_ids:
            #   helpful variables for min_max azimuth calculation
            look_per_sensor_2d = np.c_[look_id_array[:, sensor_id], look_id_array[:, sensor_id]]
            #   helpful variables for range_rate_interval_width calculation
            #sensor_range_rate_intervals = v_wrapping_array[:, sensor_id, :]
            #sensor_range_offsets = r_wrapping_array[:, sensor_id, :]

            look_ids = np.unique(look_id_array[:, sensor_id]).tolist()
            for look_id in look_ids:
                #   Mappable signals
                props_per_look['sensor_id'].append(sensors_ids[sensor_id])
                props_per_look['look_id'].append(look_id)

                #   Non-mappable signals
                props_per_look['min_azimuth'].append(np.median(min_fov_az[np.where(look_per_sensor_2d == look_id)]))
                props_per_look['max_azimuth'].append(np.median(max_fov_az[np.where(look_per_sensor_2d == look_id)]))

                rng_rate_int_w_per_look = -1  #sensor_range_rate_intervals[:, look_id]
                rng_offset_per_look = -1  #sensor_range_offsets[:, look_id]
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

    def _filter_valid_sensors(self):
        valid_id_map = self._get_nested_array_using_possible_signatures(self._raw_signals,
                                                                        self.auxiliary_raw_signals['is_valid']) > 0
        self._raw_signals = self._raw_signals.loc[valid_id_map, :].reset_index(drop=True)
        self.data_set.signals = self.data_set.signals.loc[valid_id_map, :].reset_index(drop=True)