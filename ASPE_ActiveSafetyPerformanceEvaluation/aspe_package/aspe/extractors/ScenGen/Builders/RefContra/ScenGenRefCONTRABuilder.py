"""
Scenario Generator static environment (RefContra) builder
"""
import warnings

import numpy as np
import pandas as pd

from aspe.extractors.ScenGen.Builders.ScenGenBuilder import ScenGenBuilder
from aspe.extractors.ScenGen.DataSets.ScenGenRefCONTRA import ScenGenRefCONTRA


class ScenGenRefCONTRABuilder(ScenGenBuilder):
    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)

        self._f_extract_raw_signals = f_extract_raw_signals

        self.data_set = ScenGenRefCONTRA()
        self._raw_signals = pd.DataFrame()
        self._parsed_data_shape = None
        self._log_length = None

    def build(self) -> ScenGenRefCONTRA:
        """Main extraction function."""

        self._set_parsed_data_shape()
        self._set_log_length()
        self._extract_raw_signals()
        self._extract_non_mappable_signals()
        self._sort_and_reset_dataset()
        self._convert_dtypes()

        return self.data_set

    def _set_parsed_data_shape(self):
        self._parsed_data_shape = self._parsed_data['ref']['vtx_x'].shape[0]

    def _set_log_length(self):
        """
        Set log length based on host data signals.
        Reference contours data are not time dependant, so it is needed to get time information from different source.
        """
        host_data = self._parsed_data['vehicle_data']
        self._log_length = host_data['vv'].shape[0]

    def _extract_raw_signals(self):
        ref_data = self._parsed_data['ref']

        for name, values in ref_data.items():
            if values.shape[0] == self._parsed_data_shape:
                self._raw_signals[name] = values
            else:
                # fancy way to match raw signals dimensions
                # subtract 1 to start indexing from 0 instead of matlab style 1
                self._raw_signals[name] = np.repeat(values,
                                                    np.array(np.append(values, 0) - np.insert(values, 0, 0))[:-1]) - 1

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        self._extract_scan_index()
        self._extract_timestamp()
        self._extract_vertices_positions()
        self._extract_vertices_ids()
        self._extract_contours_ids()
        self._extract_unique_id()

    def _extract_scan_index(self):
        """
        Extract scan index based on log length.
        Scan index is not present in Scenario Generator logs, so it needs to be arranged.
        """
        scan_idx = np.arange(self._log_length)
        scan_idx_vector = np.tile(scan_idx, self._parsed_data_shape)
        self.data_set.signals['scan_index'] = scan_idx_vector

    def _extract_timestamp(self):
        """
        Extract timestamp based on scan indexes.
        Timestamp is not present in Scenario Generator logs, so it needs to be arranged.
        To do that, 'dt' value given in host data is used, if 'dt' is missing default value is used.
        """
        host_data = self._parsed_data['vehicle_data']
        if 'dt' in host_data.keys():
            dt = host_data['dt']
        else:
            dt = 0.05  # set default value
            warnings.warn("'dt' signal is missing in host data, default value set.")
        timestamp_vector = self.data_set.signals['scan_index'] * dt
        self.data_set.signals['timestamp'] = timestamp_vector

    def _extract_vertices_positions(self):
        """
        Extract vertices positions from raw signals data.
        Due to the fact that positions are not time dependant (constant during whole log).
        """
        position_x = self._raw_signals['vtx_x'].to_numpy()
        position_y = self._raw_signals['vtx_y'].to_numpy()

        position_x_vector = np.repeat(position_x, self._log_length)
        position_y_vector = np.repeat(position_y, self._log_length)

        self.data_set.signals['position_x'] = position_x_vector
        self.data_set.signals['position_y'] = position_y_vector

    def _extract_vertices_ids(self):
        """
        Extract vertices ids.
        Related signals are not present in parsed data. They are arranged based on data shape and log length.
        """
        vertices_ids = np.arange(self._parsed_data_shape)
        vertices_ids_vector = np.repeat(vertices_ids, self._log_length)

        self.data_set.signals['vertex_id'] = vertices_ids_vector

    def _extract_contours_ids(self):
        """
        Extract contours ids.
        Contours ids are not present in parsed data. They are extracted based on 'last_vtx_idx' signals and reshaped
        to match dataset.signals shape.
        """
        last_vtx_idx = self._raw_signals['last_vtx_idx']
        uniques = np.unique(last_vtx_idx)

        contours_ids = np.full(self._parsed_data_shape, -1)

        it = np.nditer(uniques, flags=['f_index'])
        for el in it:
            mask = last_vtx_idx == el
            contours_ids[mask] = it.index

        contours_ids_vector = np.repeat(contours_ids, self._log_length)

        self.data_set.signals['contour_id'] = contours_ids_vector

    def _extract_unique_id(self):
        """
        Extract unique ids.
        Because reference contours exists thorough whole log, every contour id is unique. So they can be copied.
        """
        self.data_set.signals['unique_id'] = self.data_set.signals['contour_id']

    def _sort_and_reset_dataset(self):
        self.data_set.signals.sort_values(['scan_index', 'unique_id'], inplace=True)
        self.data_set.signals.reset_index(inplace=True, drop='index')
