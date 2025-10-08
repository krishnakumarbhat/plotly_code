# coding=utf-8
"""
F360 MUDP internal objects builder (version 16)
"""
import typing
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360ReferencePoint import F360ReferencePoint
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.F360.Enums.f360_objects import F360ObjectClass, F360ObjectsFilterType, F360ObjectStatus
from aspe.extractors.F360.Mudp.Builders.InternalObjects.IF360MudpInternalObjectsBuilder import (
    IF360MudpInternalObjectsBuilder,
)
from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.MathFunctions import (
    calc_position_in_bounding_box,
    calculate_jacobian_matrix_values_polar_to_cartesian,
    calculate_rotation_matrix_values,
)
from aspe.utilities.objects_lifespan_status_based_creator import ObjectLifespanStatusBasedCreator
from aspe.utilities.SupportingFunctions import get_nested_array
from aspe.utilities.UncertaintyPropagation import propagate_uncertainty_2d


# noinspection PyProtectedMember
class F360MudpInternalObjectsBuilderV16(IF360MudpInternalObjectsBuilder):
    """
    supports objects extraction for stream definitions:
    ch: 70
        - strdef_src035_str070_ver001.txt
        - strdef_src035_str070_ver002.txt
        - strdef_src035_str070_ver003.txt
        - strdef_src035_str070_ver004.txt
        - strdef_src035_str070_ver005.txt
    """
    required_stream_definitions = [['strdef_src035_str070_ver001', 'strdef_src035_str070_ver002',
                                    'strdef_src035_str070_ver003', 'strdef_src035_str070_ver004',
                                    'strdef_src035_str070_ver005']]
    main_object_stream: int = 70
    object_key = [main_object_stream]
    signal_mapper = {
        # ASPE signature                                  F360 signature
        # common data sets section
        # unified object interface section
        # coordinate system defined in properties
        GeneralSignals.id:                               'trkID',
        PhysicalSignals.position_x:                      'vcs_xposn',
        PhysicalSignals.position_y:                      'vcs_yposn',
        PhysicalSignals.velocity_otg_x:                  'vcs_xvel',
        PhysicalSignals.velocity_otg_y:                  'vcs_yvel',
        PhysicalSignals.acceleration_otg_x:              'vcs_xaccel',
        PhysicalSignals.acceleration_otg_y:              'vcs_yaccel',
        PhysicalSignals.bounding_box_orientation:        'vcs_pointing',
        GeneralSignals.motion_model_type:                'trk_fltr_type',
        GeneralSignals.motion_model_variances_suppl:     'state_variance',
        GeneralSignals.motion_model_covariances_suppl:   'supplemental_state_covariance',
        GeneralSignals.existence_indicator:              'existence_probability',
        # helpers section
        PhysicalSignals.speed:                           'speed',
        # F360 specific section
        F360CoreSignals.reduced_id:                      'reducedID',
        F360CoreSignals.status:                          'status',
        PhysicalSignals.curvature:                       'curvature',
        F360CoreSignals.confidence_level:                'confidenceLevel',
        F360CoreSignals.n_dets:                          'ndets',
        F360CoreSignals.f_moving:                        'f_moving',
        F360CoreSignals.f_moveable:                      'f_moveable',
        F360CoreSignals.reference_point:                 'reference_point',
    }
    auxiliary_signals = {
        'timestamp_us': [main_object_stream, 'data_timestamp_us'],
        'scan_index': [main_object_stream, 'stream_ref_index'],
    }
    object_class = {
        F360ObjectClass.UNDETERMINED: ObjectClass.UNDEFINED,
        F360ObjectClass.CAR: ObjectClass.CAR,
        F360ObjectClass.MOTORCYCLE: ObjectClass.TWOWHEEL,
        F360ObjectClass.TRUCK: ObjectClass.TRUCK,
        F360ObjectClass.PEDESTRIAN: ObjectClass.PEDESTRIAN,
        F360ObjectClass.POLE: ObjectClass.UNDEFINED,
        F360ObjectClass.TREE: ObjectClass.UNDEFINED,
        F360ObjectClass.ANIMAL: ObjectClass.UNDEFINED,
        F360ObjectClass.GOD: ObjectClass.UNDEFINED,
        F360ObjectClass.BICYCLE: ObjectClass.TWOWHEEL,
        F360ObjectClass.UNIDENTIFIED_VEHICLE: ObjectClass.UNDEFINED,
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._lifespan_mask = None
        self._parsed_data_shape = None

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._set_objects_status()
        self._set_parsed_data_shape()
        self._create_lifespan_mask()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _set_objects_status(self):
        """
        Sets _objects_status property.
        :return:
        """
        self._objects_status = get_nested_array(self._parsed_data['parsed_data'], self.object_key + ['status'])

    def _set_parsed_data_shape(self):
        """
        Sets _parsed_data_shape property.
        :return:
        """
        self._parsed_data_shape = self._objects_status.shape

    def _create_lifespan_mask(self):
        """
        Function which creates lifespan mask - DataFrame which 3 columns - row - column - obj_id. Each row is single
        object sample within single time instance.
        :return:
        """
        self._lifespan_mask = ObjectLifespanStatusBasedCreator.create_lifespan(self._objects_status,
                                                                               F360ObjectStatus.NEW_UPDATED.value,
                                                                               F360ObjectStatus.INVALID.value)

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        lifespan_mask = (self._lifespan_mask['row'], self._lifespan_mask['column'])
        raw_signals_parsed_dict = get_nested_array(self._parsed_data['parsed_data'], self.object_key)

        for raw_signal_name in raw_signals_parsed_dict.keys():
            signal_values = raw_signals_parsed_dict[raw_signal_name]
            obj_count = self._parsed_data_shape[1]

            if len(signal_values.shape) == 2:
                if signal_values.shape[1] == obj_count:
                    converted_signal = signal_values[lifespan_mask]
                    self._raw_signals[raw_signal_name] = converted_signal
                else:
                    warn('Invalid number of columns in signal values, signal will not be extracted')
            elif len(signal_values.shape) > 2:
                if signal_values.shape[1] == obj_count:
                    converted_signal = signal_values[lifespan_mask]
                    self._raw_signals[raw_signal_name] = list(converted_signal)
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
        self.data_set.signals['unique_id'] = self._lifespan_mask['unique_obj_id']
        self.data_set.signals['slot_id'] = self._lifespan_mask['column']
        self._extract_scan_index()
        self._extract_timestamp()
        self._extract_bbox_signals()
        self._calc_bbox_center_positions()
        self._extract_filter_type()
        self._extract_f360_object_class()
        self._extract_object_class()
        self._extract_object_class_probability()
        self._extract_movement_status()
        self._extract_yaw_rate()
        self._extract_motion_model_state()
        self._extract_variances_and_covariances()

    def _extract_scan_index(self):
        """
        Extract scan_index signal from tracker_info structure.
        :return:
        """
        scan_idx_vector = get_nested_array(self._parsed_data['parsed_data'],
                                           self.auxiliary_signals['scan_index'])
        if scan_idx_vector is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_idx_vector = np.arange(self._parsed_data_shape[0])

        obj_count = self._parsed_data_shape[1]
        scan_idx_matrix = np.tile(scan_idx_vector, (obj_count, 1)).T
        self.data_set.signals['scan_index'] = scan_idx_matrix[(self._lifespan_mask['row'],
                                                               self._lifespan_mask['column'])]

    def _extract_timestamp(self):
        """
        Auxiliary implementation of timestamp extraction, currently only for F360MudpInternalObjectsBuilderV14.
        For F360MudpInternalObjectsBuilderV12 timestamp is extracted as a mappable signal.
        Timestamp is returned in [s] (converted from [us] to [s]).
        :return:
        """
        object_list_timestamp_us = get_nested_array(self._parsed_data['parsed_data'],
                                                    self.auxiliary_signals['timestamp_us'])
        if object_list_timestamp_us is None:
            warn('Objectlist timestamp signal not found, timestamp will not be extracted')
            return

        object_list_timestamp = object_list_timestamp_us * 1e-6
        lifespan_mask = self._lifespan_mask['row']
        object_list_timestamp_signal = object_list_timestamp[lifespan_mask]
        self.data_set.signals['timestamp'] = object_list_timestamp_signal

    def _extract_bbox_signals(self):
        """
        Wrapper for selecting bounding box signal extraction method.
        """
        f_legacy = not all(signal in self._raw_signals for signal in ('length', 'width', 'reference_point'))
        if f_legacy:
            warn(f'Bounding box signals length/width/reference_point not found. Using legacy signals')
            try:
                self._extract_bbox_signals_legacy()
            except KeyError:
                warn(f'Extracting bounding boxes failed, signals len1/len2/wid1/wid2 not found')
        else:
            self._extract_bbox_signals_reference_point()

    def _extract_bbox_signals_reference_point(self):
        """
        Extract signals: length, width, reference_point_lateral, reference_point_longitudinal
        Reference point is logged as integer enum, which is mapped to the corresponding lat, lon ratio reference
        """
        f360_reference_point_integer = self._raw_signals['reference_point']
        reference_point_lat = np.empty(f360_reference_point_integer.shape[0])
        reference_point_lon = np.empty(f360_reference_point_integer.shape[0])

        for ref_point_int, ref_point_tuple in F360ReferencePoint.get_reference_point_map_items():
            f360_key_mask = f360_reference_point_integer == ref_point_int.value
            reference_point_lat[f360_key_mask] = ref_point_tuple[1]
            reference_point_lat[f360_key_mask] = ref_point_tuple[0]

        self.data_set.signals['bounding_box_dimensions_x'] = self._raw_signals['length']
        self.data_set.signals['bounding_box_dimensions_y'] = self._raw_signals['width']

        self.data_set.signals['bounding_box_refpoint_lat_offset_ratio'] = reference_point_lat
        self.data_set.signals['bounding_box_refpoint_long_offset_ratio'] = reference_point_lon

    def _extract_bbox_signals_legacy(self):
        """
            Extract signals: length, width, reference_point_lateral, reference_point_longitudinal
            pre 2023-01-25 F360 bounding box reference point update
        """
        len1 = self._raw_signals['len1']
        len2 = self._raw_signals['len2']
        length = len1 + len2
        reference_point_longitudinal = np.divide(len1, length, where=(length != 0))
        reference_point_longitudinal[length == 0] = np.nan
        self.data_set.signals['bounding_box_dimensions_x'] = length
        self.data_set.signals['bounding_box_refpoint_long_offset_ratio'] = reference_point_longitudinal

        wid1 = self._raw_signals['wid1']
        wid2 = self._raw_signals['wid2']
        width = wid1 + wid2
        reference_point_lateral = np.divide(wid1, width, where=(width != 0))
        reference_point_lateral[width == 0] = np.nan
        self.data_set.signals['bounding_box_dimensions_y'] = width
        self.data_set.signals['bounding_box_refpoint_lat_offset_ratio'] = reference_point_lateral

    def _calc_bbox_center_positions(self):
        """
        Calculate center position of bounding box.
        :return:
        """
        pos_x = self.data_set.signals['position_x']
        pos_y = self.data_set.signals['position_y']

        bbox_length = self.data_set.signals['bounding_box_dimensions_x']
        bbox_width = self.data_set.signals['bounding_box_dimensions_y']

        ref_point_lon = self.data_set.signals['bounding_box_refpoint_long_offset_ratio']
        ref_point_lat = self.data_set.signals['bounding_box_refpoint_lat_offset_ratio']

        bounding_box_orientation = self.data_set.signals['bounding_box_orientation']

        new_ref_point_lon = 0.5  # center is represented by point (0.5, 0.5)
        new_ref_point_lat = 0.5

        # FIXME add exception handling in case of missing data (nan coming). Should be solved in CEA-241
        center_lon, center_lat = calc_position_in_bounding_box(pos_x, pos_y, bbox_length, bbox_width,
                                                               bounding_box_orientation,
                                                               ref_point_lon, ref_point_lat,
                                                               new_ref_point_lon, new_ref_point_lat)
        self.data_set.signals['center_x'] = center_lon
        self.data_set.signals['center_y'] = center_lat

    def _extract_filter_type(self):
        """
        Extract F360 filter type signal. Map tracker filter type signal values to enum
        :return:
        """
        # TODO: develop function to map int array to enum array and use it here
        trk_fltr_type = self._raw_signals['trk_fltr_type']

        ccv_mask = trk_fltr_type == F360ObjectsFilterType.CCV.value
        ctca_mask = trk_fltr_type == F360ObjectsFilterType.CTCA.value

        filter_type = np.empty(trk_fltr_type.shape[0], dtype=F360ObjectsFilterType)
        filter_type[:] = F360ObjectsFilterType.INVALID

        filter_type[ccv_mask] = F360ObjectsFilterType.CCV
        filter_type[ctca_mask] = F360ObjectsFilterType.CTCA
        self.data_set.signals['filter_type'] = filter_type

    def _extract_f360_object_class(self):
        """
        Extract F360 object class signal. Map tracker class raw signal values to enum
        :return:
        """
        object_class = self._raw_signals['object_class']
        # TODO: develop function to map int array to enum array and use it here
        self.data_set.signals['f360_object_class'] = object_class

    def _extract_object_class(self):
        """
        Extract F360 object class signal. Map tracker object class signal values to enum
        :return:
        """
        f360_objects_class = self.data_set.signals['f360_object_class']
        object_class_map = self.object_class
        # TODO: develop function to map int array to enum array and use it here
        object_class = np.empty(f360_objects_class.shape[0], dtype=ObjectClass)
        object_class[:] = ObjectClass.UNDEFINED
        for f360_key, out_key in object_class_map.items():
            f360_key_mask = f360_objects_class == f360_key.value
            object_class[f360_key_mask] = out_key
        self.data_set.signals['object_class'] = object_class

    def _extract_object_class_probability(self):
        """
        Extract object_class_probability_1 signal. Basing on object_class signal fill probability signal with value from
        proper probability array.
        """
        f360_object_class = self.data_set.signals['f360_object_class']
        object_class_probability = np.full(f360_object_class.shape[0], np.nan)  # init with array of NaN

        mapping = {
            F360ObjectClass.PEDESTRIAN: 'probability_pedestrian',
            F360ObjectClass.TRUCK: 'probability_truck',
            F360ObjectClass.CAR: 'probability_car',
            F360ObjectClass.BICYCLE: 'probability_bicycle',
            F360ObjectClass.MOTORCYCLE: 'probability_motorcycle',
            F360ObjectClass.UNDETERMINED: 'probability_undet',
        }
        for object_class_enum, raw_signal_name in mapping.items():
            mask = (f360_object_class == object_class_enum.value)
            probability_signal = self._raw_signals[raw_signal_name]
            object_class_probability[mask] = probability_signal[mask]

        self.data_set.signals['object_class_probability'] = object_class_probability

    def _extract_movement_status(self):
        """
        Extract objects movement status
        """
        f_moving = self.data_set.signals['f_moving'].astype(bool)
        f_moveable = self.data_set.signals['f_moveable'].astype(bool)
        f_stationary = np.logical_and(np.logical_not(f_moving), np.logical_not(f_moveable))
        f_stopped = np.logical_and(np.logical_not(f_moving), f_moveable)

        movement_status = np.empty(f_moving.shape[0], dtype=MovementStatus)
        movement_status[:] = MovementStatus.UNDEFINED

        movement_status[f_stationary] = MovementStatus.STATIONARY
        movement_status[f_stopped] = MovementStatus.STOPPED
        movement_status[f_moving] = MovementStatus.MOVING
        self.data_set.signals['movement_status'] = movement_status

    def _extract_yaw_rate(self):
        """
        Extracts yaw rate from curvature and speed.
        :return:
        """
        curvature = self.data_set.signals['curvature']
        speed = self.data_set.signals['speed']

        yaw_rate = curvature * speed

        self.data_set.signals['yaw_rate'] = yaw_rate

    def _extract_motion_model_state(self) -> None:
        """
        Extracts motion_model_state_suppl from parsed data into signals dataframe.
        :return:
        """
        filter_type = self.data_set.signals['filter_type']

        ccv_mask = (filter_type == F360ObjectsFilterType.CCV)
        ctca_mask = (filter_type == F360ObjectsFilterType.CTCA)
        xposn = self._raw_signals['vcs_xposn']
        xvel = self._raw_signals['vcs_xvel']
        xaccel = self._raw_signals['vcs_xaccel']
        yposn = self._raw_signals['vcs_yposn']
        yvel = self._raw_signals['vcs_yvel']
        yaccel = self._raw_signals['vcs_yaccel']
        heading = self._raw_signals['vcs_heading']
        tang_accel = self._raw_signals['tang_accel']

        curvature = self.data_set.signals['curvature']
        speed = self.data_set.signals['speed']

        motion_model_state_suppl = np.empty((len(self.data_set.signals), 6))
        motion_model_state_suppl.fill(np.nan)

        def _fill_motion_model_state_suppl(filter_type_mask: pd.Series,
                                           series_in_order: typing.Iterable[pd.Series]) -> None:
            """
            Fills each column of motion_model_state_suppl with values from specified series.
            :param filter_type_mask: ctca_mask or cca_mask
            :param series_in_order: an iterable of series that will be put into corresponding columns
            :return:
            """
            for col, series in enumerate(series_in_order):
                motion_model_state_suppl[:, col][filter_type_mask] = series[filter_type_mask]

        _fill_motion_model_state_suppl(ccv_mask, (xposn, xvel, xaccel, yposn, yvel, yaccel))
        _fill_motion_model_state_suppl(ctca_mask, (xposn, yposn, heading, curvature, speed, tang_accel))

        self.data_set.signals['motion_model_state_suppl'] = list(motion_model_state_suppl)

    def _extract_variances_and_covariances(self):
        """
        Extracts position and velocity variances and covariances.
        :return:
        """
        filter_type = self.data_set.signals['filter_type']

        ccv_mask = (filter_type == F360ObjectsFilterType.CCV)
        ctca_mask = (filter_type == F360ObjectsFilterType.CTCA)

        motion_model_state_suppl_signal = self.data_set.signals['motion_model_state_suppl']
        motion_model_variances_suppl_signal = self.data_set.signals['motion_model_variances_suppl']
        motion_model_covariances_suppl_signal = self.data_set.signals['motion_model_covariances_suppl']

        motion_model_state_suppl_array = np.stack(motion_model_state_suppl_signal.to_numpy())
        motion_model_variances_suppl_array = np.stack(motion_model_variances_suppl_signal.to_numpy())
        motion_model_covariances_suppl_array = np.stack(motion_model_covariances_suppl_signal.to_numpy())

        position_variance_x = np.empty(len(self.data_set.signals))
        position_variance_y = np.empty(len(self.data_set.signals))
        position_covariance = np.empty(len(self.data_set.signals))

        position_variance_x.fill(np.nan)
        position_variance_y.fill(np.nan)
        position_covariance.fill(np.nan)

        position_variance_x[ccv_mask] = motion_model_variances_suppl_array[ccv_mask, 0]
        position_variance_y[ccv_mask] = motion_model_variances_suppl_array[ccv_mask, 3]
        position_covariance[ccv_mask] = motion_model_covariances_suppl_array[ccv_mask, 0]

        position_variance_x[ctca_mask] = motion_model_variances_suppl_array[ctca_mask, 0]
        position_variance_y[ctca_mask] = motion_model_variances_suppl_array[ctca_mask, 1]
        position_covariance[ctca_mask] = motion_model_covariances_suppl_array[ctca_mask, 0]

        velocity_otg_variance_x = np.empty(len(self.data_set.signals))
        velocity_otg_variance_y = np.empty(len(self.data_set.signals))
        velocity_otg_covariance = np.empty(len(self.data_set.signals))

        velocity_otg_variance_x.fill(np.nan)
        velocity_otg_variance_y.fill(np.nan)
        velocity_otg_covariance.fill(np.nan)

        velocity_otg_variance_x[ccv_mask] = motion_model_variances_suppl_array[ccv_mask, 1]
        velocity_otg_variance_y[ccv_mask] = motion_model_variances_suppl_array[ccv_mask, 4]
        velocity_otg_covariance[ccv_mask] = motion_model_covariances_suppl_array[ccv_mask, 1]

        speed_ctca = motion_model_state_suppl_array[ctca_mask, 4]
        heading_ctca = motion_model_state_suppl_array[ctca_mask, 2]
        speed_variance_ctca = motion_model_variances_suppl_array[ctca_mask, 4]
        heading_variance_ctca = motion_model_variances_suppl_array[ctca_mask, 2]
        speed_heading_covariance_ctca = motion_model_covariances_suppl_array[ctca_mask, 1]

        j_11, j_12, j_21, j_22 = calculate_jacobian_matrix_values_polar_to_cartesian(speed_ctca, heading_ctca)

        velocity_otg_variance_x_ctca, velocity_otg_variance_y_ctca, velocity_otg_covariance_ctca = \
            propagate_uncertainty_2d(speed_variance_ctca, heading_variance_ctca, speed_heading_covariance_ctca,
                                     j_11, j_12, j_21, j_22)

        velocity_otg_variance_x[ctca_mask] = velocity_otg_variance_x_ctca
        velocity_otg_variance_y[ctca_mask] = velocity_otg_variance_y_ctca
        velocity_otg_covariance[ctca_mask] = velocity_otg_covariance_ctca

        self.data_set.signals.loc[:, 'position_variance_x'] = position_variance_x
        self.data_set.signals.loc[:, 'position_variance_y'] = position_variance_y
        self.data_set.signals.loc[:, 'position_covariance'] = position_covariance

        self.data_set.signals.loc[:, 'velocity_otg_variance_x'] = velocity_otg_variance_x
        self.data_set.signals.loc[:, 'velocity_otg_variance_y'] = velocity_otg_variance_y
        self.data_set.signals.loc[:, 'velocity_otg_covariance'] = velocity_otg_covariance

    def _extract_properties(self):
        """
        Extract objects default properties and add additional properties
        """
        self.data_set.max_possible_obj_count = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'
