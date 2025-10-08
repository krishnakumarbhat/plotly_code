import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature


class MahalanobisDistance(IPairsFeature):
    def __init__(self, pos_x_signal_name='position_x', pos_y_signal_name='position_y',
                 orientation_signal_name='bounding_box_orientation', scale_x=1, scale_y=1, *args, **kwargs):
        """
        @param scale_x: scale at which lateral distance is calculated
        @param scale_y: scale at which longitudinal distance is calculated
        """
        super().__init__(*args, **kwargs)
        self.pos_x_signal_name = pos_x_signal_name
        self.pos_y_signal_name = pos_y_signal_name
        self.orientation_signal_name = orientation_signal_name
        self.scale_matrix = np.diag([scale_x, scale_y])

    def calculate(self, data_a: pd.DataFrame, data_b: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        pos_x_a = data_a.loc[:, self.pos_x_signal_name].values.astype('float32')
        pos_y_a = data_a.loc[:, self.pos_y_signal_name].values.astype('float32')
        pos_x_b = data_b.loc[:, self.pos_x_signal_name].values.astype('float32')
        pos_y_b = data_b.loc[:, self.pos_y_signal_name].values.astype('float32')
        angle = data_a.loc[:, self.orientation_signal_name].values.astype('float32')

        diff_x = pos_x_a - pos_x_b
        diff_y = pos_y_a - pos_y_b
        diff_vector = np.array([diff_x, diff_y]).transpose().reshape([len(diff_x), 1, 2])

        rotation_matrix = np.array([
            [np.cos(angle), -np.sin(angle)],
            [np.sin(angle), np.cos(angle)],
        ]).transpose([2, 0, 1])
        trans_matrix = rotation_matrix @ self.scale_matrix
        trans_vector = diff_vector @ trans_matrix

        dist_array = np.sqrt(trans_vector @ trans_vector.transpose(0, 2, 1))
        return pd.DataFrame({
            'mahalanobis_distance_2d': dist_array.reshape(len(dist_array))},
            index=data_a.index)
