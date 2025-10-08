from datetime import datetime

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.SDB.DataSets.SDB_ObjectList import SDB_ObjectList
from aspe.utilities.SupportingFunctions import map_array_values, recursive_dict_extraction


class SDB_ObjectListBuilder(IBuilder):
    def __init__(self, parsed_data: dict, stationary_threshold: float, f_extract_raw_signals: bool = True):
        super().__init__(parsed_data)
        self.stationary_threshold = stationary_threshold
        self.data_set = SDB_ObjectList()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    def build(self):
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._set_coordinate_system()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals
        return self.data_set

    def _set_coordinate_system(self):
        self.data_set.coordinate_system = 'PANCS'

    def _extract_raw_signals(self):
        output = {}
        for timestamp, samples in self._parsed_data.items():
            for sample in samples:
                # Temporary solution
                # for attr in sample['attributes']:
                #     if attr['type'] in ('category', 'tag') and attr['identifier'] in ('stationary', 'oncoming'):
                #         sample.attributes.remove(attr)
                ###
                flat_cuboid, _ = recursive_dict_extraction(sample)
                flat_cuboid['timestamp'] = timestamp
                for signal, value in flat_cuboid.items():
                    try:
                        output[signal].append(value)
                    except KeyError:
                        output[signal] = [value]
        self._raw_signals = pd.DataFrame(output)

    def _extract_mappable_signals(self):
        signals_mapper = {
            # ASPE signature             'pandora signature
            'bounding_box_dimensions_x': 'size_y',
            'bounding_box_dimensions_y': 'size_x',
            'existence_indicator':       'confidence',
        }

        for aspe_signature, pandora_signature in signals_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[pandora_signature]
            except KeyError:
                print(f'Signal {pandora_signature} not found in raw_signals, signal will be not extracted')

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self._extract_scan_index()
        self._extract_position_velocity()
        self._extract_unique_id()
        self._extract_orientation()
        self._extract_objectclass()
        self._extract_occlusion()
        self._extract_refpoint_offset_ratio()
        self._extract_movement_status()
        self._correct_timestamp_according_to_azimuth()

    def _extract_timestamp(self):
        timestamps = self._raw_signals.loc[:, 'timestamp'].to_numpy()
        utc_timestamp = []
        for t in timestamps:
            t_part1, t_part2 = t.split('.')
            timestamp_iso_format = t_part1 + '+00:00'
            nanosecond_part = float(t_part2.rstrip('Z')) * 1e-9
            timestamp = datetime.fromisoformat(timestamp_iso_format).timestamp() + nanosecond_part
            utc_timestamp.append(timestamp)

        self.data_set.signals.loc[:, 'timestamp'] = utc_timestamp
        self.data_set.signals.loc[:, 'vigem_timestamp'] = utc_timestamp

    def _extract_scan_index(self):
        ts = self.data_set.signals.loc[:, 'timestamp'].to_numpy()
        unq_ts = np.unique(ts)
        ts_to_scan_index_map = {t: index for index, t in enumerate(unq_ts, start=1)}
        scan_index = map_array_values(ts, ts_to_scan_index_map, -1)
        self.data_set.signals.loc[:, 'scan_index'] = scan_index

    def _extract_position_velocity(self):
        self.data_set.signals['position_x'] = -self._raw_signals['center_y']
        self.data_set.signals['position_y'] = -self._raw_signals['center_x']
        self.data_set.signals['position_z'] = -self._raw_signals['center_z']

        self.data_set.signals['center_x'] = -self._raw_signals['center_y']
        self.data_set.signals['center_y'] = -self._raw_signals['center_x']
        self.data_set.signals['center_z'] = -self._raw_signals['center_z']

        self.data_set.signals['velocity_otg_x'] = -self._raw_signals['velocity_y']
        self.data_set.signals['velocity_otg_y'] = -self._raw_signals['velocity_x']
        self.data_set.signals['velocity_otg_z'] = -self._raw_signals['velocity_z']
        self.data_set.signals['speed'] = \
            np.sqrt(self._raw_signals['velocity_y'] ** 2 + self._raw_signals['velocity_x'] ** 2)

    def _extract_unique_id(self):
        uuid = self._raw_signals.loc[:, 'uuid'].to_numpy()
        unq_uuid = np.unique(uuid)
        unq_id_mapper = {uid: index for index, uid in enumerate(unq_uuid)}
        unq_id = map_array_values(uuid, unq_id_mapper, -1)
        self.data_set.signals.loc[:, 'unique_id'] = unq_id

    def _extract_orientation(self):
        q_w = self._raw_signals['quaternion_w'].to_numpy()
        q_z = self._raw_signals['quaternion_z'].to_numpy()
        magnitude = np.hypot(q_w, q_z)
        q_w_norm = q_w / magnitude
        orientation = 2.0 * np.arccos(q_w_norm) * -np.sign(q_z) + np.pi  # shift with Pi due to reversed orientation
        normalized_orientation = (orientation + np.pi) % (2 * np.pi) - np.pi
        self.data_set.signals.loc[:, 'bounding_box_orientation'] = normalized_orientation

    def _extract_occlusion(self):
        attributes_identifiers_columns = [col for col in self._raw_signals.columns if
                                          'attributes' in col and 'identifier' in col]
        attributes_values_columns = [col for col in self._raw_signals.columns if
                                     'attributes' in col and ('value' in col or 'category' in col)]

        attributes_identifiers = self._raw_signals.loc[:, attributes_identifiers_columns].to_numpy()
        attributes_values = self._raw_signals.loc[:, attributes_values_columns].to_numpy()

        df_len = len(self._raw_signals)
        for signal_name in self.data_set.occlusion_signals:
            mask = signal_name == attributes_identifiers
            signal_values = attributes_values[mask]

            mask_rows = np.argwhere(np.any(mask, axis=1)).reshape(-1)  # handle situation when some attribute is skipped
            signal_arr = np.full(df_len, np.nan)
            signal_arr[mask_rows] = signal_values
            self.data_set.signals.loc[:, signal_name] = signal_values

            if np.isnan(signal_arr).all():
                print(f'Signal {signal_name} not found in raw_signals, signal will not be extracted')
            elif np.isnan(signal_arr).any():
                print(f'Signal {signal_name} not found in at least one row of data')

    def _extract_refpoint_offset_ratio(self):
        filler = np.full(len(self.data_set.signals), 0.5)
        self.data_set.signals['bounding_box_refpoint_lat_offset_ratio'] = filler
        self.data_set.signals['bounding_box_refpoint_long_offset_ratio'] = filler

    def _correct_timestamp_according_to_azimuth(self):
        azimuth = np.arctan2(self.data_set.signals.center_y, self.data_set.signals.center_x)
        azimuth_2pi = np.where(azimuth < 0.0, 2 * np.pi + azimuth, azimuth)  # [-pi, +pi] -> [0, 2pi]
        azimuth_normalized = azimuth_2pi / (2 * np.pi)  # [0, 1] range
        lidar_rotation_cycle = 0.1  # [s]
        dt = azimuth_normalized * lidar_rotation_cycle
        self.data_set.signals.loc[:, 'timestamp'] += dt

    def _extract_movement_status(self):
        def func(df):
            # TODO: fix stationary detection
            is_moving = pd.Series(df['speed'] > self.stationary_threshold)
            is_movable = is_moving.index > is_moving.idxmax()

            df['movement_status'] = np.full(len(is_moving), MovementStatus.STATIONARY)
            df['movement_status'][is_movable] = np.full(len(is_moving), MovementStatus.STOPPED)
            df['movement_status'][is_moving] = np.full(len(is_moving), MovementStatus.MOVING)
            return df

        id_groups = self.data_set.signals.groupby('unique_id')
        self.data_set.signals = id_groups.apply(func)

    def _extract_objectclass(self):
        # TODO: undefined or unknown class shall be added
        class_mapper = {
            'car_or_van_or_suv_or_pickup': ObjectClass.CAR,
            'truck_or_bus':                ObjectClass.TRUCK,
            'pedestrian':                  ObjectClass.PEDESTRIAN,
            'motorcycle_or_bicycle':       ObjectClass.TWOWHEEL,
        }
        sdb_oc = self._raw_signals['annotation']
        self.data_set.signals['object_class'] = np.vectorize(class_mapper.get)(sdb_oc)
