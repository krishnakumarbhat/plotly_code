from typing import List

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PESingleObjects import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.F1ScoreObjectsKPI import (
    F1ScoreBinaryClassificationKPI,
)
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.PPVObjectsKPI import PPVBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.TPRObjectsKPI import TPRBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.IBinaryClassificationKPI import IBinaryClassificationKPI

DEFAULT_BIN_CLASS_KPI_LIST = [TPRBinaryClassificationKPI(), PPVBinaryClassificationKPI(),
                              F1ScoreBinaryClassificationKPI()]


class BinaryClassificationKPIManager:
    def __init__(self, list_of_kpis: List[IBinaryClassificationKPI]):
        self._kpis_to_calculate = list_of_kpis

    def calculate_kpis(self, reference_objects: PESingleObjects, estimated_objects: PESingleObjects):
        kpi_data = {'signature': [], 'value': []}
        for kpi in self._kpis_to_calculate:
            kpi_value = kpi.calculate_kpi(reference_objects, estimated_objects)
            kpi_data['signature'].append(kpi.kpi_signature)
            kpi_data['value'].append(kpi_value)
        self.kpi_results = pd.DataFrame(kpi_data)
        return self.kpi_results.copy()

    def _create_empty_results_df(self):
        return pd.DataFrame(columns=[kpi.kpi_signature for kpi in self._kpis_to_calculate])

    def reset_results(self):
        self.kpi_results = self._create_empty_results_df()
