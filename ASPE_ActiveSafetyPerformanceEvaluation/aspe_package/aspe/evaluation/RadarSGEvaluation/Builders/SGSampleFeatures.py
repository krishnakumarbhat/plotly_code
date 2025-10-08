import pandas as pd

from aspe.evaluation.RadarSGEvaluation.BinaryClassification.specify_bc_type import specify_bc_type
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSampleFeatures import SGSampleFeatures
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.SegmentationType.specify_segmentation_type import specify_segmentation_type


def get_sample_features(samples_df: pd.DataFrame, bc_type: BCType,
                        segmentation_type: SegmentationType) -> SGSampleFeatures:
    """
    Function takes paired segments and return samples
    with projection samples (associated) on paired segments.

    :param samples_df: frame with samples
    :type samples_df: pd.DataFrame
    :param bc_type:  FalsePositive or FalseNegative enum value
    :type bc_type: BCType
    :param segmentation_type: under or over segmentation ratio
    :type segmentation_type: SegmentationType

    :rtype: SGSampleFeatures
    """

    sample_features_df = samples_df.loc[:, ['scan_index',
                                            'timestamp',
                                            'unique_id',
                                            'distance',
                                            'deviation_x',
                                            'deviation_y']]
    sample_features_df['binary_classification'] = specify_bc_type(samples_df, bc_type)
    sample_features_df['segmentation_type'] = specify_segmentation_type(samples_df, segmentation_type)
    sample_features = SGSampleFeatures()
    sample_features.insert_feature(sample_features_df)
    return sample_features
