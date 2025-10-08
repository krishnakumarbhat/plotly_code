import pandas as pd

from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType


def specify_bc_type(samples_df: pd.DataFrame, proper_type: BCType) -> pd.Series:
    """
    Function replaces classification value with proper BCType value.
    Returns pd.Series with BCTypes.

    :param samples_df: samples used in SG evaluation
    :type samples_df:  pd.DataFrame
    :param proper_type: type which should be specified
    :type proper_type: BCType

    :return: column with binary classification types
    :rtype: pd.Series
    """
    classification_type = samples_df.loc[:, 'classification'].rename('binary_classification')

    mask = ~classification_type.values
    classification_type.iloc[:] = BCType.TruePositive
    classification_type.iloc[mask] = proper_type

    return classification_type
