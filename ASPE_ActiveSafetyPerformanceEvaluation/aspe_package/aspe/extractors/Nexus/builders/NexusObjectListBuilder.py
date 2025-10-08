from collections import defaultdict
from datetime import datetime

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, map_array_values, recursive_dict_extraction


class NexusObjectListBuilder(IBuilder):

    signal_mapper = {
        # ASPE signature             pandora signature
        'bounding_box_dimensions_x': 's_y',
        'bounding_box_dimensions_y': 's_x',
        'existence_indicator':       'confidence',
        'nexus_id':                  'uuid',
        'scan_index':                'scan_index'}

    def __init__(self, parsed_data, save_raw_signals=False):
        super().__init__(parsed_data)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._raw_signals = None
        self.data_set = IObjects()
        self.save_raw_signals = save_raw_signals

    def build(self):
        self.extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()

        return self.data_set

    def _extract_non_mappable_signals(self):
        self._extract_timestamp()
        self._extract_position_velocity()
        self._extract_id()
        self._extract_unique_id()
        self._extract_orientation()
        self._extract_objectclass()
        self._extract_refpoint_offset_ratio()

    def extract_raw_signals(self):
        tracker_objects = self._parsed_data['bwm_tracker_objects']['tracker_objects_samples']

        # flatten first cuboid to get info about structure
        cuboid_flat, nested_dict_map = recursive_dict_extraction(tracker_objects[0].cuboids[0])

        raw_signals_dict = defaultdict(list)
        for sample_index, sample in enumerate(tracker_objects):
            cuboids = sample.cuboids

            for cuboid in cuboids:
                raw_signals_dict['timestamp'].append(sample.timestamp)
                raw_signals_dict['scan_index'].append(sample_index)

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

    def _extract_position_velocity(self):
        self.data_set.signals['position_x'] = self._raw_signals['c_x']
        self.data_set.signals['position_y'] = self._raw_signals['c_y']
        self.data_set.signals['position_z'] = self._raw_signals['c_z']

        self.data_set.signals['center_x'] = self._raw_signals['c_x']
        self.data_set.signals['center_y'] = self._raw_signals['c_y']
        self.data_set.signals['center_z'] = self._raw_signals['c_z']

        self.data_set.signals['velocity_otg_x'] = self._raw_signals['v_x']
        self.data_set.signals['velocity_otg_y'] = self._raw_signals['v_y']
        self.data_set.signals['velocity_otg_z'] = self._raw_signals['v_z']
        self.data_set.signals['speed'] = np.sqrt(self._raw_signals['v_y'] ** 2 + self._raw_signals['v_x'] ** 2)

    def _extract_id(self):
        object_id = (self._raw_signals['uuid']).astype(int)
        self.data_set.signals['id'] = object_id

    def _extract_unique_id(self):
        next_free_unique_id = 1
        for _, object_track in self.data_set.signals.groupby('id'):
            scan_index_diff = object_track.scan_index.diff().dropna()

            if (1 == scan_index_diff).all():
                id_increments =[]
            else:
                #need to find places where there is a scan_index_diff
                #take indexes wih are related to different objects
                id_increments = (scan_index_diff.where(scan_index_diff != 1)).dropna()
                id_increments = list(id_increments.index)

            filter_other_objects = self._get_large_position_and_velocity_jumps_indexes(object_track)
            id_increments.extend(filter_other_objects)
            id_increments.append(object_track.index[-1]+1)
            unique_id_increments = sorted(set(id_increments))
            first_index = object_track.index[0]
            for index in unique_id_increments:
                # it maybe one object further checks needs
                object_track.loc[first_index:index-1, 'unique_id'] = next_free_unique_id
                next_free_unique_id = next_free_unique_id + 1
                first_index = index
            self.data_set.signals.loc[object_track.index, 'unique_id'] = object_track['unique_id']
        self.data_set.signals.unique_id = self.data_set.signals.unique_id.astype(int)

    def _get_large_position_and_velocity_jumps_indexes(self, object_track):
        x_diff = object_track.center_x.diff().dropna()
        y_diff = object_track.center_y.diff().dropna()
        pos_diff_sq = x_diff**2 + y_diff**2
        speed_diff = object_track.speed.diff().dropna()
        scan_index_diff_mask = (pos_diff_sq > 100.0) | (abs(speed_diff) > 3.0)
        unique_id_increments = []
        if any(scan_index_diff_mask):
            #there is probably another object
            unique_id_increments = scan_index_diff_mask.where(scan_index_diff_mask == True).dropna()
            unique_id_increments = list(unique_id_increments.index)
        return unique_id_increments

    def _extract_orientation(self):
        q_w = self._raw_signals['r_w'].to_numpy()
        q_z = self._raw_signals['r_z'].to_numpy()
        magnitude = np.hypot(q_w, q_z)
        q_w_norm = q_w / magnitude
        orientation = 2.0 * np.arccos(q_w_norm) * -np.sign(q_z) + np.pi/2  # shift with Pi due to reversed orientation
        normalized_orientation = (orientation + np.pi) % (2 * np.pi) - np.pi
        self.data_set.signals.loc[:, 'bounding_box_orientation'] = normalized_orientation

    def _extract_refpoint_offset_ratio(self):
        filler = np.full(len(self.data_set.signals), 0.5)
        self.data_set.signals['bounding_box_refpoint_lat_offset_ratio'] = filler
        self.data_set.signals['bounding_box_refpoint_long_offset_ratio'] = filler

    def _extract_objectclass(self):
        class_mapper = {
            'Car':                 ObjectClass.CAR,
            'Truck':               ObjectClass.TRUCK,
            'Pedestrian':          ObjectClass.PEDESTRIAN,
            'MotorCycle':          ObjectClass.TWOWHEEL,
            'Object Undetermined': ObjectClass.UNKNOWN,
        }
        sdb_oc = self._raw_signals['annotation'].to_numpy()
        self.data_set.signals['object_class'] = map_array_values(sdb_oc, class_mapper, ObjectClass.UNDEFINED)

