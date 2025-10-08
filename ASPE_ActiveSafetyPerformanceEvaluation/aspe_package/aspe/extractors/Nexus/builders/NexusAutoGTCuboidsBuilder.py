from collections import defaultdict
from datetime import datetime

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Nexus.data_sets.AutoGtCuboid import AutoGTCuboid
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, map_array_values, recursive_dict_extraction


class NexusAutoGTCuboidsBuilder(IBuilder):
    signal_mapper = {
        # ASPE signature             'pandora signature
        'bounding_box_dimensions_x': 's_y',
        'bounding_box_dimensions_y': 's_x',
        'existence_indicator': 'confidence',
        'nexus_id': 'uuid',
    }

    def __init__(self, parsed_data, save_raw_signals=False, sensors_data=None):
        super().__init__(parsed_data)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._raw_signals = None
        self.data_set = AutoGTCuboid()
        self.save_raw_signals = save_raw_signals
        self.sensors_data = sensors_data

    def build(self):
        self.extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self.data_set.update_signals_dtypes()
        self.data_set.coordinate_system = 'PANCS'
        self.data_set.auto_gt_version = self._parsed_data['auto_gt_lidar_cuboids']['auto_gt_version']
        return self.data_set

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self._extract_scan_index()
        self._extract_position_velocity()
        self._extract_unique_id()
        self._extract_orientation()
        self._extract_objectclass()
        self._extract_refpoint_offset_ratio()
        self._extract_movement_status()
        self._correct_timestamp_according_to_azimuth()
        self._extract_visibility_rate()

    def extract_raw_signals(self):
        cuboids = self._parsed_data['auto_gt_lidar_cuboids']['auto_gt_samples']

        # flatten first cuboid to get info about structure
        cuboid_flat, nested_dict_map = recursive_dict_extraction(cuboids[0].cuboids[0])

        raw_signals_dict = defaultdict(list)
        for sample_index, sample in enumerate(cuboids):
            sample_dict = sample.toDict()
            cuboids = sample_dict['cuboids']

            for cuboid in cuboids:
                raw_signals_dict['timestamp'].append(sample_dict['timestamp'])
                raw_signals_dict['sample_index'].append(sample_index)

                for k, v in nested_dict_map.items():
                    raw_signals_dict[k].append(get_from_dict_by_map(cuboid, v))

        self._raw_signals = pd.DataFrame(raw_signals_dict)
        if self.save_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_timestamp(self):
        def timestamp_str_to_unix_time(ts: str):
            return datetime.fromisoformat(ts).timestamp()

        timestamps = self._raw_signals.loc[:, 'timestamp'].to_numpy().astype('str')
        timestamps = np.char.replace(timestamps, 'Z', '')

        self.data_set.signals.loc[:, 'timestamp'] = np.array([timestamp_str_to_unix_time(t) for t in timestamps])

    def _extract_scan_index(self):
        self.data_set.signals.loc[:, 'scan_index'] = self._raw_signals.loc[:, 'sample_index']

    def _extract_position_velocity(self):
        self.data_set.signals['position_x'] = -self._raw_signals['c_y']
        self.data_set.signals['position_y'] = -self._raw_signals['c_x']
        self.data_set.signals['position_z'] = -self._raw_signals['c_z']

        self.data_set.signals['center_x'] = -self._raw_signals['c_y']
        self.data_set.signals['center_y'] = -self._raw_signals['c_x']
        self.data_set.signals['center_z'] = -self._raw_signals['c_z']

        self.data_set.signals['velocity_otg_x'] = -self._raw_signals['v_y']
        self.data_set.signals['velocity_otg_y'] = -self._raw_signals['v_x']
        self.data_set.signals['velocity_otg_z'] = -self._raw_signals['v_z']
        self.data_set.signals['speed'] = np.sqrt(self._raw_signals['v_y'] ** 2 + self._raw_signals['v_x'] ** 2)

    def _extract_unique_id(self):
        uuid = self._raw_signals.loc[:, 'uuid'].to_numpy()
        unq_uuid = set(self._raw_signals.loc[:, 'uuid'])
        unq_id_mapper = {uid: index for index, uid in enumerate(unq_uuid)}
        unq_id = map_array_values(uuid, unq_id_mapper, -1)
        self.data_set.signals.loc[:, 'unique_id'] = unq_id

    def _extract_orientation(self):
        q_w = self._raw_signals['r_w'].to_numpy()
        q_z = self._raw_signals['r_z'].to_numpy()
        magnitude = np.hypot(q_w, q_z)
        q_w_norm = q_w / magnitude
        orientation = 2.0 * np.arccos(q_w_norm) * -np.sign(q_z) + np.pi  # shift with Pi due to reversed orientation
        normalized_orientation = (orientation + np.pi) % (2 * np.pi) - np.pi
        self.data_set.signals.loc[:, 'bounding_box_orientation'] = normalized_orientation

    def _extract_refpoint_offset_ratio(self):
        filler = np.full(len(self.data_set.signals), 0.5)
        self.data_set.signals['bounding_box_refpoint_lat_offset_ratio'] = filler
        self.data_set.signals['bounding_box_refpoint_long_offset_ratio'] = filler

    def _correct_timestamp_according_to_azimuth(self):
        boresight_az_angle = self.sensors_data.boresight_az_angle[self.sensors_data.sensor_type == 'lidar'].iloc[0]

        azimuth = np.arctan2(self.data_set.signals.center_y, self.data_set.signals.center_x)

        azimuth = azimuth - boresight_az_angle
        azimuth_2pi = np.where(azimuth < 0.0, 2 * np.pi + azimuth, azimuth)  # [-pi, +pi] -> [0, 2pi]
        azimuth_normalized = azimuth_2pi / (2 * np.pi)  # [0, 1] range
        lidar_rotation_cycle = 0.1  # [s]
        dt = azimuth_normalized * lidar_rotation_cycle
        self.data_set.signals.loc[:, 'timestamp'] = self.data_set.signals.loc[:, 'timestamp'] + dt

    def _extract_objectclass(self):
        class_mapper = {
            'car_or_van_or_suv_or_pickup': ObjectClass.CAR,
            'truck_or_bus': ObjectClass.TRUCK,
            'pedestrian': ObjectClass.PEDESTRIAN,
            'motorcycle_or_bicycle': ObjectClass.TWOWHEEL,
        }
        sdb_oc = self._raw_signals['annotation'].to_numpy()
        self.data_set.signals['object_class'] = map_array_values(sdb_oc, class_mapper, ObjectClass.UNDEFINED)

    def _extract_movement_status(self):
        is_moving = ~self._raw_signals.loc[:, 'attribs_b_stationary']
        self.data_set.signals.loc[:, 'movement_status'] = MovementStatus.STOPPED
        self.data_set.signals.loc[is_moving, 'movement_status'] = MovementStatus.MOVING

    def _extract_visibility_rate(self):
        def get_visibility_ratio_for_sensor(sensor_str):
            total_area_signal_name = [s for s in total_area_signals if sensor_str in s]
            visible_area_signal_name = [s for s in visible_area_signals if sensor_str in s]

            if total_area_signal_name and visible_area_signal_name:
                total_area = np.array(self._raw_signals.loc[:, total_area_signal_name[0]])
                visible_area = np.array(self._raw_signals.loc[:, visible_area_signal_name[0]])

                occlusion_ratio = np.full(len(total_area), 0.0)
                valid_idxs = total_area > 0  # div by 0 handling
                occlusion_ratio[valid_idxs] = visible_area[valid_idxs] / total_area[valid_idxs]

                return occlusion_ratio
        total_area_signals = [c for c in self._raw_signals.columns if '_total_area' in c]
        visible_area_signals = [c for c in self._raw_signals.columns if '_visible_area' in c]

        front_right = get_visibility_ratio_for_sensor('-FR-')
        rear_right = get_visibility_ratio_for_sensor('-RR-')
        front_left = get_visibility_ratio_for_sensor('-FL-')
        rear_left = get_visibility_ratio_for_sensor('-RL-')

        occlusion_combined = np.vstack([front_right, rear_right, front_left, rear_left])
        max_occlusion = np.max(occlusion_combined, 0)
        self.data_set.signals['visibility_rate'] = max_occlusion
