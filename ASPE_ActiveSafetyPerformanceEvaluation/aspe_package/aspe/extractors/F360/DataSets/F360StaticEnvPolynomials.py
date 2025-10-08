# coding=utf-8
"""
F360 Static Environment Polynomials Data Set
"""
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class F360StaticEnvPolynomials(IDataSet):
    """
    Data set for F360 static environment polynomials representation
    """

    def __init__(self):
        super().__init__()
        signal_names = [
            F360CoreSignals.polynomial_p0,
            F360CoreSignals.polynomial_p1,
            F360CoreSignals.polynomial_p2,
            F360CoreSignals.x_min_limit,
            F360CoreSignals.x_max_limit,
            F360CoreSignals.stat_env_type,
            F360CoreSignals.status,
            F360CoreSignals.confidence_level,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'static_environment'
