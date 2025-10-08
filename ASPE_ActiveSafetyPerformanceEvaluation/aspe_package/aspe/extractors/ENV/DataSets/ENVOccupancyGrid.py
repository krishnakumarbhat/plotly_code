"""
ENV Occupancy Grid Data Set
"""
import pandas as pd

from aspe.extractors.ENV.DataSets.SignalDescription import ENVCoreSignals
from aspe.extractors.Interfaces.IObjects import IDataSet
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals


class ENVOccupancyGrid(IDataSet):
    """
    Class which represents ENV occupancy grid data.
    """
    # TODO: FZE-400 try to handle properties like signals
    # TODO create something like ENVOCGProperties
    def __init__(self):
        super().__init__()
        signal_names = [
            ENVCoreSignals.underdrivability_status,
        ]

        self.per_scan_index = pd.DataFrame(columns=['scan_index',
                                                    'cell_length',
                                                    'cell_width',
                                                    'x_n_cells_close',
                                                    'x_n_cells_mid',
                                                    'x_n_cells_far',
                                                    'y_n_cells',
                                                    'curvature',
                                                    'width_extension_factor'])

        self.update_signals_definition(signal_names)

    def get_base_name(self):
        """
        Get base name of internal objects
        :return: str
        """
        return 'occupancy_grid'
