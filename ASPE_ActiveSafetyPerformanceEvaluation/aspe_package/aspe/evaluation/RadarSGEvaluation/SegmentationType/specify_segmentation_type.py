import pandas as pd

from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType


def specify_segmentation_type(samples_df: pd.DataFrame, proper_type: SegmentationType) -> pd.Series:
    """
    Function replaces multiple_segmentation value with proper SegmentationType.
    Returns pd.Series with SegmentationTypes.

    :param samples_df: samples used in SG evaluation
    :type samples_df: pd.DataFrame
    :param proper_type: type which should be assigned
    :type proper_type: SegmentationType

    :return: column with segmentation type
    :rtype: pd.Series
    """
    segmentation_type = samples_df.loc[:, 'multiple_segmentation'].rename('segmentation_type')

    f_true_positives_mask = samples_df.loc[:, 'classification'].values

    mask = segmentation_type.values
    segmentation_type.iloc[:] = SegmentationType.NotAssigned
    segmentation_type.iloc[mask] = proper_type
    segmentation_type.iloc[~f_true_positives_mask] = SegmentationType.NotAssigned
    return segmentation_type
