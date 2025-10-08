"""
Interface for ENV keg occupancy grid builders
"""

from typing import Union
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.ENV.DataSets.ENVOccupancyGrid import ENVOccupancyGrid
from aspe.extractors.ENV.DataSets.SignalDescription import ENVCoreSignals
from aspe.extractors.ENV.Enums.env_occupancy_grid import OCGUnderdrivabilityStatus
from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder


class ENVKegOccupancyGridBuilder(ENVKegBuilder):
    """Class used for creating and filling ENVOccupancyGrid data set from ENV .keg log data."""

    signal_mapper = {  # TODO: FZE-400 Redefine the dict values when they will be determined in Keg file or incase of interface update
        # ASPE signature                            ENV signature
        ENVCoreSignals.underdrivability_status:     'underdrivability_status',
    }

    properties = {
        # ASPE signature                            ENV signature
        'cell_length':                              'cell_length',
        'cell_width':                               'cell_width',
        'width_extension_factor':                   'cell_width_extension_factor',
        'x_n_cells_close':                          'num_cells_x_close',
        'x_n_cells_mid':                            'num_cells_x_mid',
        'x_n_cells_far':                            'num_cells_x_far',
        'y_n_cells':                                'num_cells_y',
        'x_offset':                                 'ogcs_host_rear_axle_position.x',
        'y_offset':                                 'ogcs_host_rear_axle_position.y',
        'grid_orientation':                         'ogcs_host_rear_axle_position.yaw',
        'curvature':                                'grid_curvature',
    }

    cell_class = { #TODO: FZE-400 Currently enum defined on Basis of F360 tracker,  redefine the dict values if they are updated
        OCGUnderdrivabilityStatus.CAN_NOT_PASS_UNDER:          '0',
        OCGUnderdrivabilityStatus.IS_LIKELY_TO_PASS_UNDER:     '1',
        OCGUnderdrivabilityStatus.CAN_PASS_UNDER:              '2',
        OCGUnderdrivabilityStatus.NOT_TO_CONSIDER:             '3',
    }

    def __init__(self, parsed_data, timestamps, f_extract_raw_signals=False):
        ocg_data = self._get_data_for_ocg_builder(parsed_data)
        super().__init__(ocg_data)
        self._raw_signals_nested = pd.DataFrame()
        self.data_set = ENVOccupancyGrid()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._parsed_data_shape = None
        self._timestamps = timestamps

    def build(self):
        """Main extraction function."""

        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        # TODO: FZE-400 consider adding convert_dtype() here
        return self.data_set

    def _get_data_for_ocg_builder(self, data: dict) -> Union[dict, None]:
        ocg_data = data.get('OCG', None)
        if ocg_data is not None:
            arr = self._get_dist_rear_axle_position(data)
            ocg_data.update([('dist_rear_axle_to_vcs', arr)])
        return ocg_data

    @staticmethod
    def _get_dist_rear_axle_position(data: dict) -> np.ndarray:
        return data.get('VEHICLE', {}).get('host', {}).get('dist_rear_axle_to_vcs_m', np.array([3.7]))

    def _set_parsed_data_shape(self):
        grid_definition = self._parsed_data['output']['grid_definition']
        num_cells_x = (grid_definition['num_cells_x_close'][0] + grid_definition['num_cells_x_mid'][0] +
                       grid_definition['num_cells_x_far'][0])
        num_cells_y = grid_definition['num_cells_y'][0]
        self._parsed_data_shape = (num_cells_x[0], num_cells_y[0], num_cells_x[0] * num_cells_y[0])

    def _extract_raw_signals(self):
        self._extract_raw_internal_signals()
        self._extract_raw_output_signals()
        self._extract_raw_signals_by_cell_idx()

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_raw_internal_signals(self):
        props = self._parsed_data['internals']['props']
        zones = self._parsed_data['internals']['cells']

        internal_signals = {**props, **zones}
        for signal_signature, array in internal_signals.items():
            if len(array.shape) == 2:
                if array.shape[1] == 1:
                    self._raw_signals_nested[signal_signature] = array.reshape(-1)
                else:
                    self._raw_signals_nested[signal_signature] = [x for x in array]
            elif len(array.shape) == 3:
                self._raw_signals_nested[signal_signature] = [x for x in array]

    def _extract_raw_output_signals(self):
        grid_definition = self._parsed_data['output']['grid_definition']
        underdrivability = self._parsed_data['output']['underdrivability']

        self._extract_host_rear_axle_position(underdrivability)
        output_signals = {**grid_definition, **underdrivability}

        for signal_signature, array in output_signals.items():
            if len(array.shape) == 2:
                if array.shape[1] == 1:
                    self._raw_signals_nested[signal_signature] = array.reshape(-1)
                else:
                    self._raw_signals_nested[signal_signature] = [x for x in array]
            elif len(array.shape) == 3:
                self._raw_signals_nested[signal_signature] = [x for x in array]

    def _extract_raw_signals_by_cell_idx(self):
        columns = list(self._raw_signals_nested.columns)
        columns.append('cell_id')
        height = self._raw_signals_nested.shape[0] * self._parsed_data_shape[2]
        width = len(columns)
        df_fill = np.full([height, width], np.nan)
        self._raw_signals_nested.apply(lambda row: self._flatten_nested_arrays(row, df_fill), axis=1)
        self._raw_signals = pd.DataFrame(data=df_fill, columns=columns)

    def _flatten_nested_arrays(self, row: pd.Series, df_fill: np.ndarray):
        column_idx = 0
        row_idx = int(row.name)
        nb_cells = self._parsed_data_shape[2]
        for signal_signature, el in row.items():
            if type(el) is np.ndarray:
                if len(el.shape) > 1:
                    f_shape_check = (el.shape[0] == self._parsed_data_shape[0]) and (
                                el.shape[1] == self._parsed_data_shape[1])
                    if f_shape_check:
                        df_fill[(row_idx * nb_cells) : ((row_idx + 1) * nb_cells), column_idx] = el.flatten()
                    else:
                        warn(f'Array shape does not match grid shape. {signal_signature} will not be extracted.')
                else:
                    warn(f'Array dimensions does not match grid dimensions. {signal_signature} will not be extracted.')
            elif (type(el) is float) or (type(el) is int):
                df_fill[(row_idx * nb_cells) : ((row_idx + 1) * nb_cells), column_idx] = np.repeat(el, nb_cells)
            column_idx += 1
        df_fill[(row_idx * nb_cells) : ((row_idx + 1) * nb_cells), column_idx] = np.arange(0, nb_cells)

    def _extract_non_mappable_signals(self):
        """Extract signals which can not be strictly mapped using Mapper object."""
        self.data_set.signals.loc[:, 'unique_id'] = self._raw_signals.loc[:, 'cell_id'].astype(int)
        self._raw_signals.loc[:, 'unique_id'] = self._raw_signals.loc[:, 'cell_id'].astype(int)
        self._extract_scan_index()
        self._extract_timestamps()
        self._extract_cell_classes()

    def _extract_scan_index(self):
        scan_idx_vector = np.arange(self._timestamps.shape[0])
        self.data_set.per_scan_index['scan_index'] = scan_idx_vector
        scan_idx_vector = np.repeat(scan_idx_vector, self._parsed_data_shape[2])
        self.data_set.signals['scan_index'] = scan_idx_vector

    def _extract_timestamps(self):
        self.data_set.signals['timestamp'] = np.repeat(self._timestamps, self._parsed_data_shape[2])

    def _extract_cell_classes(self):
        ocg_objects_class = self.data_set.signals['underdrivability_status']
        object_class_map = self.cell_class
        object_class = np.empty(ocg_objects_class.shape[0], dtype=OCGUnderdrivabilityStatus)
        object_class[:] = OCGUnderdrivabilityStatus.NOT_TO_CONSIDER
        for ocg_key, out_key in object_class_map.items():
            ocg_key_mask = ocg_objects_class == ocg_key.value
            object_class[ocg_key_mask] = ocg_key
        self.data_set.signals['cell_class'] = object_class

    def _extract_properties(self):
        props = self.data_set.per_scan_index
        for aspe_signature, env_signature in self.properties.items():
            props[aspe_signature] = self._raw_signals_nested[env_signature]
        props['x_offset'] = self._transfer_to_vcs(props['x_offset'].to_numpy())

    def _transfer_to_vcs(self, data: np.ndarray) -> np.ndarray:
        return data + self._parsed_data['dist_rear_axle_to_vcs'].reshape(-1)

    @staticmethod
    def _extract_host_rear_axle_position(underdrivability: dict):
        for signal_signature, array in underdrivability['ogcs_host_rear_axle_position'].items():
            key = '.'.join(['ogcs_host_rear_axle_position', signal_signature])
            underdrivability[key] = array
        del underdrivability['ogcs_host_rear_axle_position']
