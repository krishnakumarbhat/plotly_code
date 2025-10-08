import warnings

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360BmwObjectList import F360BmwObjectList
from aspe.extractors.Interfaces.Enums.Object import MeasurementStatus, MovementStatus, ObjectClass
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.utilities.flatten_someip_object_list_data import flatten_someip_signals
from aspe.utilities.MathFunctions import calc_position_in_bounding_box
from aspe.utilities.objects_lifespan_age_id_based_mask import create_obj_lifespan_mask_based_on_id_age
from aspe.utilities.SupportingFunctions import get_from_dict_by_map, map_array_values


class F360Mdf4BmwObjectListBuilder(IBuilder):
    """
    Class for extracting someIP object list from parsed mf4 file.
    """

    object_list_data_keys_map = ['RecogSideRadarObjectlist', 'Objectlist']

    signal_mapper = {
        # BWM signature                                 ASPE signature
        'summary_id':                                   'id',
        'position_x_value':                             'position_x',
        'position_y_value':                             'position_y',
        'position_covariance_xy':                       'position_covariance',
        'dynamics_velocity_absolute_x_value':           'velocity_otg_x',
        'dynamics_velocity_absolute_y_value':           'velocity_otg_y',
        'dynamics_velocity_absolute_covariance_xy':     'velocity_otg_covariance',
        'dynamics_velocity_relative_x_value':           'velocity_rel_x',
        'dynamics_velocity_relative_y_value':           'velocity_rel_y',
        'dynamics_acceleration_absolute_x_value':       'acceleration_otg_x',
        'dynamics_acceleration_absolute_y_value':       'acceleration_otg_y',
        'dynamics_acceleration_relative_x_value':       'acceleration_rel_x',
        'dynamics_acceleration_relative_y_value':       'acceleration_rel_y',
        'dynamics_acceleration_absolute_covariance_xy': 'acceleration_otg_covariance',
        'dynamics_orientation_rate_mean':               'yaw_rate',
        'shape_length_edge_mean':                       'bounding_box_dimensions_x',
        'shape_width_edge_mean':                        'bounding_box_dimensions_y',
        'position_orientation_value':                   'bounding_box_orientation',
        'existence_existence_probability':              'existence_indicator',
        # signals which were taken from lifespan mask
        'unique_id':                                    'unique_id',
        'column':                                       'slot_id',
        'row':                                          'log_data_row',
    }

    def __init__(self, parsed_data: dict, f_extract_raw_signals: bool = True):
        object_list_parsed_data = get_from_dict_by_map(parsed_data, self.object_list_data_keys_map)
        super().__init__(object_list_parsed_data)
        self.data_set = F360BmwObjectList()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()
        self._object_signals_flat = {}
        self._lifespan_mask = pd.DataFrame()

    def build(self):
        """
        Create and fill F360BMWObjectList with data.
        :return: filled F360BmwObjectList data set object
        """
        self._flatten_data()
        self._create_lifespan_mask()
        self._extract_raw_signals()
        self._set_coordinate_system()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
        return self.data_set

    def _flatten_data(self):
        object_signals_flat = flatten_someip_signals(self._parsed_data, data_key='object_list')
        self._object_signals_flat.update(object_signals_flat)

    def _create_lifespan_mask(self):
        age_array = self._object_signals_flat['summary_age']
        ids_array = self._object_signals_flat['summary_id']
        self._lifespan_mask = create_obj_lifespan_mask_based_on_id_age(ids_array, age_array)

    def _set_coordinate_system(self):
        self.data_set.coordinate_system = 'ISO_VRACS'

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
                self.data_set.signals[aspe_signature] = self._raw_signals.loc[:, bmw_signature].to_numpy()
            except KeyError:
                warnings.warn(f'Object list extraction warning! There is no signal {bmw_signature} inside raw_signals '
                              f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_non_mappable_signals(self):
        self._extract_bbox_signals()
        self._extract_variances()
        self._extract_classification()
        self._extract_movement_status()
        self._extract_measurement_status()
        self._extract_speed()

    def _extract_bbox_signals(self):
        # this is performed for ISO CS standard! not Aptiv VCS
        ref_point_map = {
            'Corner_Rear_Left':   (0.0, 1.0),
            'Middle_Rear':        (0.0, 0.5),
            'Corner_Rear_Right':  (0.0, 0.0),
            'Middle_Side_Right':  (0.5, 0.0),
            'Corner_Front_Right': (1.0, 0.0),
            'Middle_Front':       (1.0, 0.5),
            'Corner_Front_Left':  (1.0, 1.0),
            'Middle_Side_Left':   (0.5, 1.0),
        }
        ref_points = self._raw_signals.position_reference_point.to_numpy()
        ref_x = np.zeros(self._raw_signals.shape[0])
        ref_y = np.zeros(self._raw_signals.shape[0])
        for corner, (val_x, val_y) in ref_point_map.items():
            ref_x = np.where(ref_points == corner, val_x, ref_x)
            ref_y = np.where(ref_points == corner, val_y, ref_y)

        pos_x = self._raw_signals.position_x_value.to_numpy()
        pos_y = self._raw_signals.position_y_value.to_numpy()
        len = self._raw_signals.shape_length_edge_mean.to_numpy()
        wid = self._raw_signals.shape_width_edge_mean.to_numpy()
        ori = self._raw_signals.position_orientation_value.to_numpy()
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
            'position_variance_x':         'position_x_std_dev_value',
            'position_variance_y':         'position_y_std_dev_value',
            'velocity_otg_variance_x':     'dynamics_velocity_absolute_x_std_dev_value',
            'velocity_otg_variance_y':     'dynamics_velocity_absolute_y_std_dev_value',
            'acceleration_otg_variance_x': 'dynamics_acceleration_absolute_x_std_dev_value',
            'acceleration_otg_variance_y': 'dynamics_acceleration_absolute_y_std_dev_value',
            'yaw_rate_variance':           'dynamics_orientation_rate_std_dev',
        }

        for aspe_signature, bmw_signature in std_to_var_map.items():
            try:
                std_vals = self._raw_signals.loc[:, bmw_signature].to_numpy()
                var_vals = np.square(std_vals)
                self.data_set.signals[aspe_signature] = var_vals
            except KeyError:
                warnings.warn(f'Object list extraction warning! There is no signal {bmw_signature} inside raw_signals '
                              f'DataFrame. Signal {aspe_signature} will be not filled.')

    def _extract_classification(self):
        # TODO: fix mapper
        mapper = {
            # BMW class                                     # ASPE class
            'classification_class_car_EmPercent':        ObjectClass.CAR,
            'classification_class_truck_EmPercent':      ObjectClass.TRUCK,
            'classification_class_motorcycle_EmPercent': ObjectClass.TWOWHEEL,
            'classification_class_bicycle_EmPercent':    ObjectClass.TWOWHEEL,
            'classification_class_pedestrian_EmPercent': ObjectClass.PEDESTRIAN,
            'classification_class_animal_EmPercent':     ObjectClass.UNDEFINED,
            'classification_class_hazard_EmPercent':     ObjectClass.UNDEFINED,
            'classification_class_unknown_EmPercent':    ObjectClass.UNKNOWN,
        }
        class_signatures = [col for col in self._raw_signals.columns if
                            'classification' in col and 'drivable_' not in col]
        classes_arr = self._raw_signals.loc[:, class_signatures].to_numpy()
        max_idxs = np.argmax(classes_arr, axis=1)
        classes_bmw = np.array(class_signatures)[max_idxs]
        self.data_set.signals.loc[:, 'object_class'] = map_array_values(classes_bmw, mapper, ObjectClass.UNDEFINED)

        zero_to_n_idxs = np.arange(0, len(max_idxs))
        prob_vals = classes_arr[zero_to_n_idxs, max_idxs]
        self.data_set.signals.loc[:, 'object_class_probability'] = prob_vals

    def _extract_movement_status(self):
        movement_status_raw = self._raw_signals.loc[:, 'summary_status_movement'].to_numpy()
        mapper = {
            'kMovementStatus_Moved':      MovementStatus.MOVING,
            'kMovementStatus_Stationary': MovementStatus.STATIONARY,
        }
        self.data_set.signals.loc[:, 'movement_status'] = map_array_values(movement_status_raw, mapper,
                                                                           MovementStatus.UNDEFINED)

    def _extract_measurement_status(self):
        measurement_status_raw = self._raw_signals.loc[:, 'summary_status_measurement'].to_numpy()
        mapper = {
            'kMeasurementStatus_Measured':  MeasurementStatus.MEASURED,
            'kMeasurementStatus_New':       MeasurementStatus.NEW,
            'kMeasurementStatus_Predicted': MeasurementStatus.PREDICTED,
        }
        self.data_set.signals.loc[:, 'measurement_status'] = map_array_values(measurement_status_raw, mapper,
                                                                              MeasurementStatus.INVALID)

    def _extract_speed(self):
        vel_x = self.data_set.signals.loc[:, 'velocity_otg_x'].to_numpy()
        vel_y = self.data_set.signals.loc[:, 'velocity_otg_y'].to_numpy()
        self.data_set.signals.loc[:, 'speed'] = np.hypot(vel_x, vel_y)
