import numpy as np
from scipy.stats import gamma, norm

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.supporting_functions import cdf_to_two_tailed_p_value


class NEESMeanTestPValueKPI(ISeriesKPI):
    def __init__(self, state_dimension: int, test_type: str = 'mean Gamma'):
        if test_type not in ('mean Normal', 'mean Gamma'):
            raise ValueError('Invalid test type')

        super().__init__()
        self.state_dimension = state_dimension
        self.test_type = test_type
        self.kpi_signature = f'NEES {self.test_type} test p-value'

    def calculate_kpi(self, series):
        nees_test_variable = series.mean()
        n = len(series)
        k = self.state_dimension

        if self.test_type == 'mean Gamma':
            alpha = n * k / 2
            theta = 2 / n
            cdf = gamma.cdf(nees_test_variable, a=alpha, scale=theta)

        elif self.test_type == 'mean Normal':
            mean = k
            sigma = np.sqrt(2 * k / n)
            cdf = norm.cdf(nees_test_variable, mean, sigma)

        p_value = cdf_to_two_tailed_p_value(cdf)
        return p_value
