import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

#TODO: FZE-400 When interface will be known, change/delete/provide expected signals below
cell_length = SignalDescription(signature='cell_length',
                                dtype=np.int16,
                                description='Value of length of the cell',
                                unit='-')

cell_width = SignalDescription(signature='cell_width',
                    dtype=np.int16,
                    description='Value of width of the cell',
                    unit='-')

width_extension_factor = SignalDescription(signature='width_extension_factor',
                    dtype=np.float32,
                    description='Scale factor of upper bound of the grid.',
                    unit='-')

x_offset = SignalDescription(signature='x_offset',
                    dtype=np.float32,
                    description='Longitudinal offset of occupancy grid from the VCS',
                    unit='m')

y_offset = SignalDescription(signature='y_offset',
                    dtype=np.float32,
                    description='Lateral offset of occupancy grid from the VCS',
                    unit='m')

x_n_cells_close = SignalDescription(signature='x_n_cells_close',
                    dtype=np.int16,
                    description='Number of longitudinal cells in x axis (close range)',
                    unit='-')

x_n_cells_mid = SignalDescription(signature='x_n_cells_mid',
                    dtype=np.int16,
                    description='Number of longitudinal cells in x axis (mid range)',
                    unit='-')

x_n_cells_far = SignalDescription(signature='x_n_cells_far',
                    dtype=np.int16,
                    description='Number of longitudinal cells in x axis (far range)',
                    unit='-')

y_n_cells = SignalDescription(signature='y_n_cells',
                    dtype=np.int16,
                    description='Number of lateral cells in y axis',
                    unit='-')

underdrivability_status = SignalDescription(signature='underdrivability_status',
                    dtype='category',
                    description='Underdrivability representation status, see OCGUnderdrivableStatus',
                    unit='-')

confidence_level = SignalDescription(signature='confidence_level',
                    dtype=np.float32,
                    description='Confidence of drivability classification',
                    unit='-')

grid_orientation = SignalDescription(signature='grid_orientation',
                    dtype=np.float32,
                    description='Orientation of occupancy grid',
                    unit='rad')

