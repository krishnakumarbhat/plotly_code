import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGKPIOutput import SGKPIOutput
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.KPI.F1ScoreKPI import F1ScoreKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.BiasKPI import BiasKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalseNegativeKPI import FalseNegativeKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalsePositiveKPI import FalsePositiveKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.MeanAbsKPI import MeanAbsKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.OverSegmentationRatioKPI import OverSegmentationRatioKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.PredictivePositiveValueKPI import PPVCalculationKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.QuantileKPI import Quantile25KPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.QuantileKPI import Quantile50KPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.QuantileKPI import Quantile75KPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.QuantileKPI import Quantile99KPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.StdKPI import StdKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveKPI import TruePositiveKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveRatioKPI import TPRCalculationKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.UnderSegmentationRatioKPI import UnderSegmentationRatioKPI

LIST_OF_DEVIATION_KPI = [BiasKPI, MeanAbsKPI, StdKPI, Quantile99KPI]
LIST_OF_DISTANCE_KPI = [Quantile25KPI, Quantile50KPI, Quantile75KPI, Quantile99KPI]
LIST_OF_ESTIMATED_KPIS = [TruePositiveKPI, FalsePositiveKPI, PPVCalculationKPI]
LIST_OF_REFERENCE_KPIS = [TruePositiveKPI, FalseNegativeKPI, TPRCalculationKPI]

DICT_OF_SAMPLES_KPIS = {'distance': LIST_OF_DISTANCE_KPI,
                        'deviation_x': LIST_OF_DEVIATION_KPI,
                        'deviation_y': LIST_OF_DEVIATION_KPI}

DICT_OF_ESTIMATED_KPIS = {'binary_classification': LIST_OF_ESTIMATED_KPIS,
                          'segmentation_type': [UnderSegmentationRatioKPI]}

DICT_OF_REFERENCE_KPIS = {'binary_classification': LIST_OF_REFERENCE_KPIS,
                          'segmentation_type': [OverSegmentationRatioKPI]}


def calculate_kpis(estimated_samples: pd.DataFrame, reference_samples: pd.DataFrame) -> SGKPIOutput:
    """
    Function for calculation of KPIs given estimated and reference samples.

    :param estimated_samples: samples from estimated contours
    :type estimated_samples: pd.DataFrame
    :param reference_samples: samples from reference contours
    :type reference_samples: pd.DataFrame

    :return: SGKPIOutput
    """

    f1_score_kpi = F1ScoreKPI()
    f1_score_value = f1_score_kpi.calculate(estimated_samples['binary_classification'],
                                            reference_samples['binary_classification'])
    general_results = {f'{f1_score_kpi.kpi_signature}': f1_score_value}

    _get_kpi_series_values(general_results, estimated_samples, DICT_OF_ESTIMATED_KPIS)
    general_results['estimated_TPs'] = general_results['TPs'].copy()

    _get_kpi_series_values(general_results, reference_samples, DICT_OF_REFERENCE_KPIS)
    general_results['reference_TPs'] = general_results['TPs'].copy()
    general_results.pop('TPs')

    estimated_results = _get_kpi_metrics_results(estimated_samples, DICT_OF_SAMPLES_KPIS)

    reference_results = _get_kpi_metrics_results(reference_samples, DICT_OF_SAMPLES_KPIS)

    return SGKPIOutput(general_results, estimated_results, reference_results)


def _get_kpi_series_values(general_results: dict, samples: pd.DataFrame, kpi_dict: dict):
    """
    Function calculates values of given set of KPIs on a given dataset.

    :param general_results: general KPI metrics
    :type general_results: dict
    :param samples: frame of samples
    :type samples: pd.DataFrame
    :param kpi_dict: dictionary of KPIs to be calculated
    :type: dict

    :return: dict
    """
    for param, item in kpi_dict.items():
        output = ({f'{value().kpi_signature}': value().calculate(samples[param]) for value in item})
        general_results.update(**output)


def _get_kpi_metrics_results(samples: pd.DataFrame, kpi_dict: dict) -> dict:
    """
    Function for calculation given set of KPI's  metrics on a given dataset.

    :param samples: frame of samples
    :type samples: pd.DataFrame
    :param kpi_dict: dictionary of KPIs to be calculated
    :type: dict

    :rtype: dict
    """

    true_positives_samples = (samples.loc[samples['binary_classification'] == BCType.TruePositive, :]).copy()
    output = {}
    for param, item in kpi_dict.items():
        output.update(
            **{f'{value().kpi_signature}_{param}': value().calculate(true_positives_samples[param]) for value in item})

    return output
