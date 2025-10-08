import numpy as np
import pandas as pd
from scipy.stats import chi2
from tqdm import tqdm

from aspe.evaluation.RadarDetectionEvaluation.Features.IDetEvalPairsFeatures import IDetEvalPairsFeatures
from aspe.evaluation.RadarDetectionEvaluation.NEES.optimizers import nearest_det_3d_nees_constant_obj_cov


class Ness3dFeatures(IDetEvalPairsFeatures):
    """
    NEES based features for detections. 3D case considering range azimuth and range rate.
    """

    def __init__(self,
                 optimizer=nearest_det_3d_nees_constant_obj_cov):
        """

        :param optimizer: function which is wrapping
        """
        self._optimizer = optimizer

    def calc_features(self, valid_pairs_synch_dets: pd.DataFrame,
                      valid_pairs_synch_objects: pd.DataFrame,
                      valid_pairs_synch_sensor: pd.DataFrame) \
            -> pd.DataFrame:
        """
        Calculating Features for each pair (detection, object)
        :param valid_pairs_synch_dets: detection signals synchronized in size with other inputs
        :type valid_pairs_synch_dets: pd.DataFrame
        :param valid_pairs_synch_objects: objects signals synchronized in size with other inputs
        :type valid_pairs_synch_objects: pd.DataFrame
        :param valid_pairs_synch_sensor: sensor signals synchronized in size with other inputs
        :type valid_pairs_synch_sensor: pd.DataFrame
        :return: pd.DataFrame DataFrame with features
        """

        print('Optimization based reference point finding')
        nees_list = []
        for slot_idx, single_det in tqdm(valid_pairs_synch_dets.iterrows(), total=len(valid_pairs_synch_dets)):
            single_obj = valid_pairs_synch_objects.iloc[slot_idx, :]
            single_sensor_data = valid_pairs_synch_sensor.iloc[slot_idx, :]

            nees_list.append(self._optimizer(single_obj, single_sensor_data, single_det))
        nees = pd.DataFrame(nees_list)

        nees['dof'] = self._calc_dof(nees)
        nees['p_value'] = 1.0 - chi2.cdf(nees['nees_3d'], nees['dof'])

        return nees

    @staticmethod
    def _calc_dof(nees):
        f_inside_x = np.logical_and(nees.ref_point_x > 0.001, nees.ref_point_x < 0.999)
        f_inside_y = np.logical_and(nees.ref_point_y > 0.001, nees.ref_point_y < 0.999)
        f_corner = np.logical_not(np.logical_or(f_inside_x, f_inside_y))
        f_side = np.logical_xor(np.logical_not(f_inside_x), np.logical_not(f_inside_y))
        dof = np.full_like(nees.ref_point_x, 1.0)
        dof[f_side] = 2.0  # On side of bounding box
        dof[f_corner] = 3.0  # Outside of bounding box
        return dof

