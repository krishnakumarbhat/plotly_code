import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import adjust_pos_by_reference_point_2_reference_point
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.MahalanobisDistance import (
    MahalanobisDistance,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.gui.utilities.calc_bbox_corners import calc_bbox_corners


class ComplexMahalanobisDistance(IPairsFeature):
    def __init__(self, scale_x=1, scale_y=1, mirp_weight=2, *args, **kwargs):
        """
        @param scale_x: scale at which lateral distance is calculated
        @param scale_y: scale at which longitudinal distance is calculated
        @param mirp_weight: weight of most important reference point
        """
        super().__init__(*args, **kwargs)
        self.corners = [
            MahalanobisDistance('corner_x_RL', 'corner_y_RL', scale_x=scale_x, scale_y=scale_y, *args, **kwargs),
            MahalanobisDistance('corner_x_RR', 'corner_y_RR', scale_x=scale_x, scale_y=scale_y, *args, **kwargs),
            MahalanobisDistance('corner_x_FL', 'corner_y_FL', scale_x=scale_x, scale_y=scale_y, *args, **kwargs),
            MahalanobisDistance('corner_x_FR', 'corner_y_FR', scale_x=scale_x, scale_y=scale_y, *args, **kwargs),
        ]
        self.mirp = MahalanobisDistance(scale_x=scale_x, scale_y=scale_y, *args, **kwargs)
        self.mirp_weight = mirp_weight

    def calculate(self, data_a: pd.DataFrame, data_b: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        adjust_pos_by_reference_point_2_reference_point(data_b, data_a)
        ref_bb_corners = calc_bbox_corners(
            data_a['center_x'],
            data_a['center_y'],
            data_a['bounding_box_orientation'],
            data_a['bounding_box_dimensions_x'],
            data_a['bounding_box_dimensions_y'],
        )
        ref_bb_corners['bounding_box_orientation'] = data_a['bounding_box_orientation']
        est_bb_corners = calc_bbox_corners(
            data_b['center_x'],
            data_b['center_y'],
            data_b['bounding_box_orientation'],
            data_b['bounding_box_dimensions_x'],
            data_b['bounding_box_dimensions_y'],
        )
        est_bb_corners['bounding_box_orientation'] = data_b['bounding_box_orientation']
        m_distances = np.full(len(data_a), 0.)
        for corner in self.corners:
            m_distances += corner.calculate(ref_bb_corners, est_bb_corners).values
        m_distances += self.mirp.calculate(data_a, data_b).values * self.mirp_weight
        m_distances /= len(self.corners) + self.mirp_weight
        return pd.DataFrame({
            'complex_mahalanobis_distance_2d': m_distances,
        })
