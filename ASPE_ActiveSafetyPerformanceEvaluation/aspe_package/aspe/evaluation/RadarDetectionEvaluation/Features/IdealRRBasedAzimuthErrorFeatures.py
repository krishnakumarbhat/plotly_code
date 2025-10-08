import numpy as np
import pandas as pd
from tqdm import tqdm

from aspe.evaluation.RadarDetectionEvaluation.Features.IDetEvalPairsFeatures import IDetEvalPairsFeatures
from aspe.evaluation.RadarDetectionEvaluation.Utilities.azimuth_deviation import (
    calc_azimuth_deviation_for_given_rr_and_vel,
)
from aspe.evaluation.RadarDetectionEvaluation.Utilities.bb_support import is_point_in_bb
from aspe.extractors.Transform.veloicty import otg_to_rel_vel
from aspe.utilities.MathFunctions import pol2cart


class IdealRRBasedAzimuthErrorFeatures(IDetEvalPairsFeatures):
    """
    Calculating Features for each pair (detection, object)
    Ideal Range is assumed and based on that azimuth error is calculated.
    Based on azimuth error, corrected position of detection is calculated and so corrected detection is checked if
    it fits into bounding box

    Note:
        * There are two hypotheses of azimuth for given range rate and velocity vector - azimuth with lower error
          is chosen
        * In case of target yawing, each point of the target has different velocity - that may produce unreliable
          results. However that can be controlled by target_yaw_rate_to_ignore signal

    """

    def __init__(self, bb_length_extension=0.0, bb_width_extension=0.0, target_yaw_rate_to_ignore=np.inf):
        self._bb_length_extension = bb_length_extension
        self._bb_width_extension = bb_width_extension
        self._target_yaw_rate_to_ignore = target_yaw_rate_to_ignore

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

        # Host yaw rate don't have any impact on range rate calculation, so it can be ignored for relative velocity calc
        velocity_rel_x, velocity_rel_y = otg_to_rel_vel(valid_pairs_synch_objects['velocity_otg_x'],
                                                        valid_pairs_synch_objects['velocity_otg_y'],
                                                        valid_pairs_synch_sensor['velocity_otg_x'],
                                                        valid_pairs_synch_sensor['velocity_otg_y'],
                                                        0.0, 0.0, 0.0)

        azimuth_error = calc_azimuth_deviation_for_given_rr_and_vel(valid_pairs_synch_dets['azimuth'],
                                                                    valid_pairs_synch_dets['range_rate'],
                                                                    velocity_rel_x, velocity_rel_y)

        # Calculate corrected signals
        corrected_azimuth = valid_pairs_synch_dets['azimuth'] - azimuth_error
        corrected_x, corrected_y = pol2cart(valid_pairs_synch_dets['range'], corrected_azimuth)

        extended_length = valid_pairs_synch_objects['bounding_box_dimensions_x'] + self._bb_length_extension
        extended_width = valid_pairs_synch_objects['bounding_box_dimensions_y'] + self._bb_width_extension
        f_corrected_pos_in_bb = is_point_in_bb(corrected_x, corrected_y,
                                               valid_pairs_synch_objects['center_x'],
                                               valid_pairs_synch_objects['center_y'],
                                               extended_length,
                                               extended_width,
                                               valid_pairs_synch_objects['bounding_box_orientation'])

        f_original_pos_in_bb = is_point_in_bb(valid_pairs_synch_dets['position_x'],
                                              valid_pairs_synch_dets['position_y'],
                                              valid_pairs_synch_objects['center_x'],
                                              valid_pairs_synch_objects['center_y'],
                                              extended_length,
                                              extended_width,
                                              valid_pairs_synch_objects['bounding_box_orientation'])

        features = pd.DataFrame()
        features['ideal_rr_based_azimuth_error'] = azimuth_error
        features['ideal_rr_based_corrected_azimuth'] = corrected_azimuth
        features['ideal_rr_based_corrected_x'] = corrected_x
        features['ideal_rr_based_corrected_y'] = corrected_y
        features['ideal_rr_based_f_original_pos_in_bb'] = f_original_pos_in_bb
        features['ideal_rr_based_f_corrected_pos_in_bb'] = f_corrected_pos_in_bb

        f_ignore = np.abs(valid_pairs_synch_objects['yaw_rate']) > self._target_yaw_rate_to_ignore
        features.loc[f_ignore, :] = np.nan

        return features
