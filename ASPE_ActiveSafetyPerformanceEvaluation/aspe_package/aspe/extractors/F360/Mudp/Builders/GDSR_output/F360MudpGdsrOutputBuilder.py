from warnings import warn

import numpy as np

from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.F360.Mudp.Builders.Objects.IF360MudpObjectsBuilder import IF360MudpObjectsBuilder
from aspe.extractors.Interfaces.Enums.Object import MeasurementStatus, MovementStatus, ObjectClass
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.MathFunctions import calc_position_in_bounding_box
from aspe.utilities.objects_lifespan_status_based_creator import ObjectLifespanStatusBasedCreator
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, map_array_values, recursive_dict_extraction


class F360MudpGdsrOutput(IF360MudpObjectsBuilder):
    required_stream_definitions = ["strdef_src035_str033_ver004"]
    obj_list_map = ['parsed_data', 33, 'gdsr_tracker_output']

    signal_mapper = {
        # ASPE signature                               # BWM signature
        GeneralSignals.id:                            'id',
        PhysicalSignals.position_x:                   'ref_long_posn',
        PhysicalSignals.position_y:                   'ref_lat_posn',
        PhysicalSignals.velocity_otg_x:               'ref_long_vel',
        PhysicalSignals.velocity_otg_y:               'ref_lat_vel',
        PhysicalSignals.velocity_rel_x:               'vcs_long_vel_rel',
        PhysicalSignals.velocity_rel_y:               'vcs_lat_vel_rel',
        PhysicalSignals.acceleration_otg_x:           'ref_long_accel',
        PhysicalSignals.acceleration_otg_y:           'ref_lat_accel',
        PhysicalSignals.yaw_rate:                     'heading_rate',
        PhysicalSignals.bounding_box_dimensions_x:    'length',
        PhysicalSignals.bounding_box_dimensions_y:    'width',
        PhysicalSignals.bounding_box_orientation:     'heading',
        GeneralSignals.existence_indicator:           'existence_probability',
        F360CoreSignals.status:                       'status',
    # signals which were taken from lifespan mask
        GeneralSignals.unique_id:                     'unique_id',
        GeneralSignals.slot_id:                       'column',
        GeneralSignals.log_data_row:                  'row',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._object_signals_flat = None

    def build(self):
        obj_data = get_from_dict_by_map(self._parsed_data, self.obj_list_map)
        self._object_signals_flat, _ = recursive_dict_extraction(obj_data)

        self._create_lifespan_mask()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
        self._convert_dtypes()
        return self.data_set

    def _create_lifespan_mask(self):
        status_array = self._object_signals_flat['status'].copy()
        status_array_extended = np.vstack([np.zeros(shape=(1, status_array.shape[1]), dtype=int), status_array])
        status_diff = np.diff(status_array_extended, axis=0)
        obj_was_born_mask = status_diff == 2
        new_enum_for_new_obj_status = 1
        status_array[obj_was_born_mask] = new_enum_for_new_obj_status

        self._lifespan_mask = \
            ObjectLifespanStatusBasedCreator.create_lifespan(status_array,
                                                             obj_status_new_val=new_enum_for_new_obj_status,
                                                             obj_status_invalid_val=0)

    def _extract_raw_signals(self):
        rows = self._lifespan_mask['row']
        columns = self._lifespan_mask['column']
        unique_id = self._lifespan_mask['unique_obj_id']

        self._raw_signals['unique_id'] = unique_id
        self._raw_signals['row'] = rows
        self._raw_signals['column'] = columns

        for signal_signature, array in self._object_signals_flat.items():
            if 'guardrails' in signal_signature:
                continue
            if len(array.shape) == 2:
                self._raw_signals[signal_signature] = array[rows, columns]

    def _extract_non_mappable_signals(self):
        self.data_set.coordinate_system = 'VCS'
        self._extract_bbox_signals()
        self._extract_movement_status()
        self._extract_measurement_status()
        self._extract_scan_index()
        self._extract_timestamp()

    def _extract_bbox_signals(self):
        ref_point_map = {
            0: (1.0, 0.0),      # front left
            1: (1.0, 0.5),      # middle front
            2: (1.0, 1.0),      # front right
            3: (0.5, 1.0),      # side right
            4: (0.0, 1.0),      # rear right
            5: (0.0, 0.5),      # middle rear
            6: (0.0, 0.0),      # rear left
            7: (0.5, 0.0),      # middle left
        }
        ref_points = self._raw_signals.loc[:, 'ref_position'].to_numpy().astype(int)
        ref_x = np.zeros(self._raw_signals.shape[0])
        ref_y = np.zeros(self._raw_signals.shape[0])
        for corner, (val_x, val_y) in ref_point_map.items():
            ref_x = np.where(ref_points == corner, val_x, ref_x)
            ref_y = np.where(ref_points == corner, val_y, ref_y)

        pos_x = self._raw_signals.loc[:, 'ref_long_posn'].to_numpy()
        pos_y = self._raw_signals.loc[:, 'ref_lat_posn'].to_numpy()
        len = self._raw_signals.loc[:, 'length'].to_numpy()
        wid = self._raw_signals.loc[:, 'width'].to_numpy()
        ori = self._raw_signals.loc[:, 'heading'].to_numpy()

        center_ref_x = np.array([0.5] * pos_x.shape[0])
        center_ref_y = np.array([0.5] * pos_x.shape[0])
        center_x, center_y = calc_position_in_bounding_box(pos_x, pos_y, len, wid, ori, ref_x, ref_y, center_ref_x,
                                                           center_ref_y)
        self.data_set.signals.loc[:, 'center_x'] = center_x
        self.data_set.signals.loc[:, 'center_y'] = center_y
        self.data_set.signals.loc[:, 'bounding_box_refpoint_long_offset_ratio'] = ref_x
        self.data_set.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = ref_y

    def _extract_movement_status(self):
        movement_status_raw = self._raw_signals.loc[:, 'f_stationary'].to_numpy()
        mapper = {
            1: MovementStatus.STATIONARY,
            0: MovementStatus.MOVING,
        }
        self.data_set.signals.loc[:, 'movement_status'] = map_array_values(movement_status_raw, mapper,
                                                                           MovementStatus.UNDEFINED)

    def _extract_measurement_status(self):
        measurement_status_raw = self._raw_signals.loc[:, 'status'].to_numpy()
        mapper = {
            2: MeasurementStatus.MEASURED,
            3: MeasurementStatus.PREDICTED,
        }
        self.data_set.signals.loc[:, 'movement_status'] = map_array_values(measurement_status_raw, mapper,
                                                                           MeasurementStatus.INVALID)

    def _extract_scan_index(self):
        scan_index_arr = self._parsed_data['parsed_data'][33]['bmw_object_list']['counter']
        rows = self._lifespan_mask['row']
        scan_index = scan_index_arr[rows]
        self.data_set.signals['scan_index'] = scan_index

    def _extract_timestamp(self):
        ts_dict = self._parsed_data['parsed_data'][33]['bmw_object_list']['timestamp']
        ts_seconds = ts_dict['seconds']
        ts_nano = ts_dict['fractional_seconds']
        timestamp_arr = ts_seconds + ts_nano * 1e-9
        rows = self._lifespan_mask['row']
        timestamp = timestamp_arr[rows]
        self.data_set.signals['timestamp'] = timestamp
