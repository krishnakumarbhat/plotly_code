"""
Interface for ENV Keg Stationary Geometries builders
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.ENV.DataSets.ENVStationaryGeometries import ENVStationaryGeometries
from aspe.extractors.ENV.DataSets.SignalDescription import SGCoreSignals
from aspe.extractors.ENV.Enums.env_stationary_geometries import SGContourType, SGDrivabilityClass
from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals


class ENVKegStationaryGeometriesBuilder(ENVKegBuilder):
    """Class used for creating and filling ENVStationaryGeometries data set from ENV .keg log data."""
    signal_mapper = {  # TODO: DOH-200 update the dict values by adding keg file internals
        # ASPE signature                            # SG signature
        GeneralSignals.unique_id:                   'unique_id',
        GeneralSignals.timestamp:                   'timestamp',
        SGCoreSignals.num_contours:                 'num_contours',
        SGCoreSignals.f_valid:                      'f_valid',
        SGCoreSignals.drivability_class:            'drivability_class',
        SGCoreSignals.contour_id:                   'unique_id',
        SGCoreSignals.num_vertices:                 'num_vertices',
        SGCoreSignals.contour_type:                 'type',
        PhysicalSignals.position_x:                 'position_x',
        PhysicalSignals.position_y:                 'position_y',
        PhysicalSignals.position_variance_y:        'position_variance_y',
        PhysicalSignals.position_variance_x:        'position_variance_x',
        PhysicalSignals.position_covariance:        'position_covariance_xy',
    }

    signal_class = {  # TODO: DOH-200 update the dict values by adding keg file internals
        SGContourType.INVALID:                      '0',
        SGContourType.POINT:                        '1',
        SGContourType.POLYLINE:                     '2',
        SGContourType.POLYGON:                      '3',
        SGDrivabilityClass.UNKNOWN:                 '0',
        SGDrivabilityClass.NON_DRIVABLE:            '1',
        SGDrivabilityClass.UNDERDRIVABLE:           '2',
        SGDrivabilityClass.OVERDRIVABLE:            '3',
    }

    ARRAY_INPUT = 2
    ARRAY_1DIM = 1

    def __init__(self, parsed_data, timestamps, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._parsed_data = parsed_data
        self._raw_signals_nested = pd.DataFrame()
        self.data_set = ENVStationaryGeometries()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._timestamps = timestamps

    def build(self) -> ENVStationaryGeometries:
        """Main extraction function."""
        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._convert_dtypes()
        return self.data_set

    def _set_parsed_data_shape(self):
        max_num_reduced_vertices = self._parsed_data['SG']['output']['vertices']['position_x'].shape[1]
        max_num_reduced_contours = self._parsed_data['SG']['output']['contours']['unique_id'].shape[1]
        self._parsed_data_shape = np.array([max_num_reduced_vertices, max_num_reduced_contours])

    def _extract_raw_signals(self):

        self._extract_raw_output_signals()
        self._transfer_iso_to_vcs()
        self._unwrap_raw_output_signals()

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_raw_output_signals(self):
        contours = self._parsed_data['SG']['output']['contours']
        vertices = self._parsed_data['SG']['output']['vertices']
        num_contours = {'num_contours': self._parsed_data['SG']['output']['num_contours']}
        f_valid = {'f_valid': self._parsed_data['SG']['output']['f_valid']}
        timestamp = {'timestamp': self._parsed_data['SG']['output']['timestamp']}
        output_signals = {**contours, **vertices, **timestamp, **num_contours, **f_valid}
        for signal_signature, array in output_signals.items():
            if len(array.shape) == self.ARRAY_INPUT:
                if array.shape[1] == self.ARRAY_1DIM:
                    self._raw_signals_nested[signal_signature] = array.reshape(-1)
                else:
                    self._raw_signals_nested[signal_signature] = list(array)

    def _transfer_iso_to_vcs(self):
        dist_rear_axle_to_vcs = self._parsed_data['VEHICLE']['host']['dist_rear_axle_to_vcs_m'].reshape(-1)

        self._raw_signals_nested['position_x'] = self._raw_signals_nested['position_x'] - dist_rear_axle_to_vcs
        self._raw_signals_nested['position_y'] = -self._raw_signals_nested['position_y']

    def _unwrap_raw_output_signals(self):

        columns = list(self._raw_signals_nested.columns)
        height = self._raw_signals_nested.shape[0] * self._parsed_data_shape[0]
        width = self._raw_signals_nested.columns.shape[0]
        df_fill = np.full([height, width], np.nan)
        self._raw_signals_nested.apply(lambda row: self._flatten_nested_arrays(row, df_fill), axis=1)
        self._raw_signals = pd.DataFrame(data=df_fill, columns=columns)

    def _flatten_nested_arrays(self, row: pd.Series, df_fill: np.ndarray):
        column_idx = 0
        row_idx = np.int32(row.name)
        nb_cells = self._parsed_data_shape[0]
        for signal_signature, el in row.items():
            if isinstance(el, np.ndarray):
                if el.shape[0] == self._parsed_data_shape[0]:
                    df_fill[(row_idx * nb_cells): ((row_idx + 1) * nb_cells), column_idx] = \
                        np.concatenate((el, np.full(nb_cells - el.shape[0], 0)))
                elif el.shape[0] == self._parsed_data_shape[1]:
                    repeated = np.repeat(el, self._parsed_data['SG']['output']['contours']['num_vertices'][row_idx])
                    df_fill[(row_idx * nb_cells): ((row_idx + 1) * nb_cells), column_idx] = \
                        np.concatenate((repeated, np.full(nb_cells - repeated.shape[0], 0)))
                else:
                    warn(f'Array shape does not match grid shape. {signal_signature} will not be extracted.',
                         stacklevel=2)
            elif isinstance(el, (float, int)):
                df_fill[(row_idx * nb_cells): ((row_idx + 1) * nb_cells), column_idx] = np.repeat(el, nb_cells)
            column_idx += 1

    def _extract_non_mappable_signals(self):
        """Extract signals which can not be strictly mapped using Mapper object."""
        self._extract_scan_index()
        self._map_signal_to_category()
        self._remove_empty_rows()

    def _remove_empty_rows(self):
        """Remove df rows which are empty and preserve original df index."""
        self.data_set.signals = self.data_set.signals[self.data_set.signals['unique_id'] != 0]

    def _extract_scan_index(self):
        scan_idx_vector = np.arange(self._parsed_data['SG']['output']['timestamp'].shape[0])
        self.data_set.signals['scan_index'] = np.repeat(scan_idx_vector, self._parsed_data_shape[0])

    def _map_signal_to_category(self):
        """Convert category-type signals to corresponding Enum class type"""
        sg_contour_class = self.data_set.signals['contour_type']
        sg_drivability_class = self.data_set.signals['drivability_class']
        object_class_map = self.signal_class
        contour_class = np.empty(sg_contour_class.shape[0], dtype=SGContourType)
        contour_class[:] = SGContourType.INVALID
        drivability_class = np.empty(sg_drivability_class.shape[0], dtype=SGDrivabilityClass)
        drivability_class[:] = SGDrivabilityClass.UNKNOWN
        for sg_key, num in object_class_map.items():
            if isinstance(sg_key, SGContourType):
                sg_key_mask = sg_contour_class == pd.Series(int(num), range(sg_contour_class.shape[0]))
                contour_class[sg_key_mask] = sg_key
            elif isinstance(sg_key, SGDrivabilityClass):
                sg_key_mask = sg_drivability_class == pd.Series(int(num), range(sg_contour_class.shape[0]))
                drivability_class[sg_key_mask] = sg_key
        self.data_set.signals['contour_type'] = contour_class
        self.data_set.signals['drivability_class'] = drivability_class
