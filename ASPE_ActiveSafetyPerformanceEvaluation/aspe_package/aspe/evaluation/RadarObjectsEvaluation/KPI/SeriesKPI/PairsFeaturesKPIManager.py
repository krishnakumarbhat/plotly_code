import copy
from warnings import warn

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.Flags.TruePositiveFlag import TruePositiveFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.TruePositiveObjectWithinTimeline import (
    TruePositiveObjectWithinTimeline,
)
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.BiasKPI import BiasKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MaxValueKPI import MaxValueKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MeanKPI import MeanKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.MinValueKPI import MinValueKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.NEESMeanTestPValueKPI import NEESMeanTestPValueKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.Quantile99KPI import Quantile99KPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.RMSEKPI import RMSEKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.STDDeviationKPI import STDKPI

DEFAULT_FILTERED_DEVIATION_KPIS = [
    (BiasKPI(), TruePositiveFlag()),
    (STDKPI(), TruePositiveFlag()),
    (RMSEKPI(), TruePositiveFlag()),
    (MaxValueKPI(), TruePositiveFlag()),
    (Quantile99KPI(), TruePositiveFlag()),
]

DEFAULT_FILTERED_RATIO_KPIS = [(MeanKPI(), TruePositiveFlag()),
                               (STDKPI(), TruePositiveFlag()),
                               (RMSEKPI(), TruePositiveFlag()),
                               (MaxValueKPI(), TruePositiveFlag()),
                               (MinValueKPI(), TruePositiveFlag()),
                               ]

DEFAULT_FILTERED_1D_NEES_KPIS = [
    (MeanKPI(), TruePositiveFlag()),
    (NEESMeanTestPValueKPI(state_dimension=1), TruePositiveFlag()),
]

DEFAULT_FILTERED_2D_NEES_KPIS = [
    (MeanKPI(), TruePositiveFlag()),
    (NEESMeanTestPValueKPI(state_dimension=2), TruePositiveFlag()),
]

DEFAULT_PAIRS_KPI_DICT = {
    'dev_position_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_position_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_speed': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_orientation': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_rel_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_rel_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_otg_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_otg_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_rel_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_rel_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_bounding_box_dimensions_x': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_bounding_box_dimensions_y': DEFAULT_FILTERED_DEVIATION_KPIS,
    'dev_yaw_rate': DEFAULT_FILTERED_DEVIATION_KPIS,
    'intersection_over_union': DEFAULT_FILTERED_RATIO_KPIS,
}

EXTENDED_PAIRS_KPI_DICT = {
    **DEFAULT_PAIRS_KPI_DICT,
    'nees_value_position_x': DEFAULT_FILTERED_1D_NEES_KPIS,
    'nees_value_position_y': DEFAULT_FILTERED_1D_NEES_KPIS,
    'nees_value_position_xy': DEFAULT_FILTERED_2D_NEES_KPIS,
    'nees_value_velocity_x': DEFAULT_FILTERED_1D_NEES_KPIS,
    'nees_value_velocity_y': DEFAULT_FILTERED_1D_NEES_KPIS,
    'nees_value_velocity_xy': DEFAULT_FILTERED_2D_NEES_KPIS,
}

EARLY_OBJECT_FILTERED_DEVIATION_KPIS = [
    (BiasKPI(), TruePositiveObjectWithinTimeline()),
    (RMSEKPI(), TruePositiveObjectWithinTimeline()),
    (MaxValueKPI(), TruePositiveObjectWithinTimeline()),
]

EARLY_OBJECT_FILTERED_1D_NEES_KPIS = [
    (MeanKPI(), TruePositiveObjectWithinTimeline()),
    (NEESMeanTestPValueKPI(state_dimension=1), TruePositiveObjectWithinTimeline()),
]

EARLY_OBJECT_FILTERED_2D_NEES_KPIS = [
    (MeanKPI(), TruePositiveObjectWithinTimeline()),
    (NEESMeanTestPValueKPI(state_dimension=2), TruePositiveObjectWithinTimeline()),
]

EARLY_OBJECT_DEFAULT_PAIRS_KPI_DICT = {
    'dev_position_x': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_position_y': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_speed': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_orientation': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_x': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_y': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_rel_x': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_velocity_rel_y': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_otg_x': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_acceleration_otg_y': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_bounding_box_dimensions_x': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_bounding_box_dimensions_y': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'dev_yaw_rate': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
    'intersection_over_union': EARLY_OBJECT_FILTERED_DEVIATION_KPIS,
}

EARLY_OBJECT_EXTENDED_PAIRS_KPI_DICT = {
    **EARLY_OBJECT_DEFAULT_PAIRS_KPI_DICT,
    'nees_value_position_x': EARLY_OBJECT_FILTERED_1D_NEES_KPIS,
    'nees_value_position_y': EARLY_OBJECT_FILTERED_1D_NEES_KPIS,
    'nees_value_position_xy': EARLY_OBJECT_FILTERED_2D_NEES_KPIS,
    'nees_value_velocity_x': EARLY_OBJECT_FILTERED_1D_NEES_KPIS,
    'nees_value_velocity_y': EARLY_OBJECT_FILTERED_1D_NEES_KPIS,
    'nees_value_velocity_xy': EARLY_OBJECT_FILTERED_2D_NEES_KPIS,
}


class PairsFeaturesKPIManager:
    def __init__(self, list_of_kpis):
        self.kpis_to_calculate = list_of_kpis
        self.kpi_results = self._create_empty_results_df()

    def calculate_kpis(self, pe_paired_objects: PEPairedObjects):
        for feature_signature in self.kpis_to_calculate.keys():
            if feature_signature in pe_paired_objects.signals.columns:
                self._calculate_single_feature_kpis(pe_paired_objects, feature_signature)
            else:
                warn(f'{feature_signature} pairs feature not found, KPIs will not be calculated')
        results = self.kpi_results.copy()
        self.reset_results()
        return results

    @staticmethod
    def _filter_data(pe_paired_objects, flag):
        pe_paired_objects_local = copy.deepcopy(pe_paired_objects)
        mask = flag.calc_flag(pe_paired_objects_local.signals)
        pe_paired_objects_local.signals = pe_paired_objects_local.signals[mask]
        return pe_paired_objects_local

    def _calculate_single_feature_kpis(self, pe_paired_objects, feature_signature):
        for kpi, flag in self.kpis_to_calculate[feature_signature]:
            pe_paired_objects_filtered = self._filter_data(pe_paired_objects, flag)
            series = pe_paired_objects_filtered.signals[feature_signature]
            series = series.values.astype(np.float32)
            series = series[~np.isnan(series)]
            n_samples = len(series)
            kpi_value = kpi.calculate_kpi(series)
            self._add_record_to_dev_results(feature_signature, kpi.kpi_signature, flag.flag_signature, kpi_value,
                                            n_samples)

    def _add_record_to_dev_results(self, feature_signature, kpi_signature, flag_signature, kpi_value, n_samples):
        record = {
            'feature_signature': feature_signature,
            'kpi_signature': kpi_signature,
            'flag_signature': flag_signature,
            'kpi_value': kpi_value,
            'n_samples': n_samples,
        }
        self.kpi_results = self.kpi_results.append(record, ignore_index=True)

    def reset_results(self):
        self.kpi_results = self._create_empty_results_df()

    @staticmethod
    def _create_empty_results_df():
        return pd.DataFrame(columns=['feature_signature', 'kpi_signature', 'flag_signature', 'kpi_value', 'n_samples'])
