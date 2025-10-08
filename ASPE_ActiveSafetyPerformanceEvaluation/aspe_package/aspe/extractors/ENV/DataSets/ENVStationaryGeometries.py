"""
ENV Stationary Geometries Data Set
"""
from aspe.extractors.ENV.DataSets.SignalDescription import SGCoreSignals
from aspe.extractors.Interfaces.IObjects import IDataSet
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals


class ENVStationaryGeometries(IDataSet):
    """Class which represents ENV Stationary Geometries data."""

    def __init__(self):
        super().__init__()
        signal_names = [
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            SGCoreSignals.contour_id,
            SGCoreSignals.num_contours,
            SGCoreSignals.num_vertices,
            SGCoreSignals.f_valid,
            SGCoreSignals.drivability_class,
            SGCoreSignals.contour_type,
            PhysicalSignals.position_variance_y,
            PhysicalSignals.position_variance_x,
            PhysicalSignals.position_covariance,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self) -> str:

        """
        Get base name of SG data

        :return: str
        """

        return 'stationary_geometries'
