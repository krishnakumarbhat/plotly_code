from warnings import warn

import numpy as np

from aspe.extractors.F360.Mudp.Builders.Objects.IF360MudpObjectsBuilder import IF360MudpObjectsBuilder
from aspe.extractors.Interfaces.Enums.Object import MeasurementStatus, MovementStatus, ObjectClass
from aspe.utilities.MathFunctions import calc_position_in_bounding_box
from aspe.utilities.objects_lifespan_age_id_based_mask import create_obj_lifespan_mask_based_on_id_age
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, map_array_values, recursive_dict_extraction


class F360MudpOALObjectsBuilderV4(IF360MudpObjectsBuilder):
    required_stream_definitions = [["strdef_src035_str033_ver004", "strdef_src035_str033_ver005",
                                    "strdef_src035_str033_ver006", "strdef_src035_str033_ver007",
                                    "strdef_src035_str033_ver008"]]
    obj_list_map = ['parsed_data', 33, 'bmw_object_list', 'object_list']

    signal_mapper = {
        # BWM signature                                 ASPE signature
        'summary_id':                                   'id',
        'position_x':                                   'position_x',
        'position_y':                                   'position_y',
        'position_covariance_xy':                       'position_covariance',
        'dynamics_velocity_absolute_x':                 'velocity_otg_x',
        'dynamics_velocity_absolute_y':                 'velocity_otg_y',
        'dynamics_velocity_relative_x':                 'velocity_rel_x',
        'dynamics_velocity_relative_y':                 'velocity_rel_y',
        'dynamics_velocity_absolute_covariance_xy':     'velocity_otg_covariance',
        'dynamics_acceleration_absolute_x':             'acceleration_otg_x',
        'dynamics_acceleration_absolute_y':             'acceleration_otg_y',
        'dynamics_acceleration_absolute_covariance_xy': 'acceleration_otg_covariance',
        'dynamics_orientation_rate_mean':               'yaw_rate',
        'shape_length_edge_mean':                       'bounding_box_dimensions_x',
        'shape_width_edge_mean':                        'bounding_box_dimensions_y',
        'position_orientation':                         'bounding_box_orientation',
        'existence_existence_probability':              'existence_indicator',
        # signals which were taken from lifespan mask
        'unique_id':                                    'unique_id',
        'column':                                       'slot_id',
        'row':                                          'log_data_row',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._object_signals_flat = None

    def build(self):
        obj_list = get_from_dict_by_map(self._parsed_data, self.obj_list_map)
        self._object_signals_flat, _ = recursive_dict_extraction(obj_list)

        self._create_lifespan_mask()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
        return self.data_set

    def _create_lifespan_mask(self):
        age_array = self._object_signals_flat['summary_age']
        ids_array = self._object_signals_flat['summary_id']
        self._lifespan_mask = create_obj_lifespan_mask_based_on_id_age(ids_array, age_array)

    def _extract_raw_signals(self):
        rows = self._lifespan_mask['row']
        columns = self._lifespan_mask['column']
        unique_id = self._lifespan_mask['unique_id']

        self._raw_signals['unique_id'] = unique_id
        self._raw_signals['row'] = rows
        self._raw_signals['column'] = columns

        for signal_signature, array in self._object_signals_flat.items():
            self._raw_signals[signal_signature] = array[rows, columns]

    def _extract_mappable_signals(self):
        for bmw_signature, aspe_signature in self.signal_mapper.items():
            try:
                self.data_set.signals.loc[:, aspe_signature] = self._raw_signals.loc[:, bmw_signature].to_numpy()
            except KeyError:
                warn(f'Object list extraction warning! There is no signal {bmw_signature} inside raw_signals '
                     f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_non_mappable_signals(self):
        self.data_set.coordinate_system = 'ISO_VRACS'
        self._extract_bbox_signals()
        self._extract_variances()
        self._extract_classification()
        self._extract_movement_status()
        self._extract_measurement_status()

    def _extract_bbox_signals(self):
        # this is performed for ISO CS standard! not Aptiv VCS
        ref_point_map = {
            0: (1.0, 1.0),      # front left
            1: (1.0, 0.5),      # middle front
            2: (1.0, 0.0),      # front right
            3: (0.5, 0.0),      # side right
            4: (0.0, 0.0),      # rear right
            5: (0.0, 0.5),      # middle rear
            6: (0.0, 1.0),      # rear left
            7: (0.5, 1.0),      # middle left
        }
        ref_points = self._raw_signals.position_reference_point.to_numpy()
        ref_x = np.zeros(self._raw_signals.shape[0])
        ref_y = np.zeros(self._raw_signals.shape[0])
        for corner, (val_x, val_y) in ref_point_map.items():
            ref_x = np.where(ref_points == corner, val_x, ref_x)
            ref_y = np.where(ref_points == corner, val_y, ref_y)

        pos_x = self._raw_signals.position_x.to_numpy()
        pos_y = self._raw_signals.position_y.to_numpy()
        len = self._raw_signals.shape_length_edge_mean.to_numpy()
        wid = self._raw_signals.shape_width_edge_mean.to_numpy()
        ori = self._raw_signals.position_orientation.to_numpy()
        center_ref_x = np.array([0.5] * pos_x.shape[0])
        center_ref_y = np.array([0.5] * pos_x.shape[0])
        center_x, center_y = calc_position_in_bounding_box(pos_x, pos_y, len, wid, ori, ref_x, ref_y, center_ref_x,
                                                           center_ref_y)
        self.data_set.signals.loc[:, 'center_x'] = center_x
        self.data_set.signals.loc[:, 'center_y'] = center_y
        self.data_set.signals.loc[:, 'bounding_box_refpoint_long_offset_ratio'] = ref_x
        self.data_set.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = ref_y

    def _extract_variances(self):
        std_to_var_map = {
            # ASPE signature                 # BMW signature
            'position_variance_x':          'position_x_std_dev',
            'position_variance_y':          'position_y_std_dev',
            'velocity_otg_variance_x':      'dynamics_velocity_absolute_x_std_dev',
            'velocity_otg_variance_y':      'dynamics_velocity_absolute_y_std_dev',
            'acceleration_otg_variance_x':  'dynamics_acceleration_absolute_x_std_dev',
            'acceleration_otg_variance_y':  'dynamics_acceleration_absolute_y_std_dev',
            'yaw_rate_variance':            'dynamics_orientation_rate_std_dev',
        }

        for aspe_signature, bmw_signature in std_to_var_map.items():
            try:
                std_vals = self._raw_signals.loc[:, bmw_signature].to_numpy()
                var_vals = np.square(std_vals)
                self.data_set.signals.loc[:, aspe_signature] = var_vals
            except KeyError:
                warn(f'Object list extraction warning! There is no signal {bmw_signature} inside raw_signals '
                              f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_classification(self):
        class_map = {
            # BMW class                           # ASPE class
            'classification_class_car':           ObjectClass.CAR,
            'classification_class_truck':         ObjectClass.TRUCK,
            'classification_class_motorcycle':    ObjectClass.TWOWHEEL,
            'classification_class_bicycle':       ObjectClass.TWOWHEEL,
            'classification_class_pedestrian':    ObjectClass.PEDESTRIAN,
            'classification_class_animal':        ObjectClass.UNDEFINED,
            'classification_class_hazard':        ObjectClass.UNDEFINED,
            'classification_class_unknown':       ObjectClass.UNKNOWN,
        }
        class_signatures = [col for col in self._raw_signals.columns if
                            'classification' in col and 'drivable_' not in col]
        classes_arr = self._raw_signals.loc[:, class_signatures].to_numpy()
        max_idxs = np.argmax(classes_arr, axis=1)
        classes_bmw = np.array(class_signatures)[max_idxs]
        self.data_set.signals.loc[:, 'object_class'] = map_array_values(classes_bmw, class_map, ObjectClass.UNDEFINED)

        zero_to_n_idxs = np.arange(0, len(max_idxs))
        prob_vals = classes_arr[zero_to_n_idxs, max_idxs]
        self.data_set.signals.loc[:, 'object_class_probability'] = prob_vals

    def _extract_movement_status(self):
        movement_status_raw = self._raw_signals.loc[:, 'summary_status_movement'].to_numpy()
        mapper = {
            1: MovementStatus.STATIONARY,
            0: MovementStatus.MOVING,
        }
        self.data_set.signals.loc[:, 'movement_status'] = map_array_values(movement_status_raw, mapper,
                                                                           MovementStatus.UNDEFINED)

    def _extract_measurement_status(self):
        measurement_status_raw = self._raw_signals.loc[:, 'summary_status_measurement'].to_numpy()
        mapper = {
            0: MeasurementStatus.MEASURED,
            1: MeasurementStatus.NEW,
            2: MeasurementStatus.PREDICTED,
        }
        self.data_set.signals.loc[:, 'measurement_status'] = map_array_values(measurement_status_raw, mapper,
                                                                              MeasurementStatus.INVALID)
