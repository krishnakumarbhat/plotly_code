import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.Gating.IDetEvalGating import IDetEvalGating
from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class DistanceToCenterGating(IDetEvalGating):
    def __init__(self, threshold_bb_diagonal_scale: float = 1.0, threshold_offset: float = 2.0):
        """
        Gating of detection-object pairs. Gating is done based on cartesian distance between center of object and
        detection. Threshold is calculated based on diagonal of bounding box as:
            thr = diagonal * scale + offset
        :param threshold_bb_diagonal_scale: scale of the bounding box diagonal value for threshold
        :param threshold_offset: offset of threshold
        """
        self.threshold_bb_diagonal_scale = threshold_bb_diagonal_scale
        self.threshold_offset = threshold_offset

    def gating(self, estimated_data: ExtractedData, reference_data: ExtractedData) \
            -> pd.DataFrame:
        """
        Perform gating
        :param estimated_data:
        :param reference_data:
        :return: pd.DataFrame, valid paris DataFrame with linkage to estimated detections and reference objects
                 valid_pairs['scan_index']
                 valid_pairs['unique_id_obj']
                 valid_pairs['unique_id_det']
                 valid_pairs['gating_distance']
        """

        rel_det_signals = estimated_data.detections.signals[['scan_index', 'unique_id', 'position_x', 'position_y']]
        rel_ref_signals = reference_data.objects.signals[['scan_index', 'unique_id', 'center_x', 'center_y',
                                                          'bounding_box_dimensions_x', 'bounding_box_dimensions_y']]

        pairs = rel_det_signals.merge(rel_ref_signals, on='scan_index', suffixes=('_det', '_obj'))
        pairs['gating_distance'] = np.hypot(
            pairs['position_x'].values.astype(np.float32) - pairs['center_x'].values.astype(np.float32),
            pairs['position_y'].values.astype(np.float32) - pairs['center_y'].values.astype(np.float32))
        pairs['bounding_box_diagonal'] = np.hypot(pairs['bounding_box_dimensions_x'].values.astype(np.float32),
                                                  pairs['bounding_box_dimensions_y'].values.astype(np.float32))
        pairs['gating_threshold'] = pairs['bounding_box_diagonal'] * self.threshold_bb_diagonal_scale \
                                    + self.threshold_offset
        pairs['f_is_associated'] = pairs['gating_distance'] < pairs['gating_threshold']
        valid_pairs = pairs[pairs['f_is_associated']].reset_index(drop=True)
        valid_pairs = valid_pairs[['scan_index', 'unique_id_obj', 'unique_id_det', 'gating_distance']]
        return valid_pairs
