from typing import Dict, List

import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.Features import default_feature_list
from aspe.evaluation.RadarDetectionEvaluation.Features.IDetEvalPairsFeatures import IDetEvalPairsFeatures
from aspe.evaluation.RadarDetectionEvaluation.Gating.DistanceToCenterGating import DistanceToCenterGating
from aspe.evaluation.RadarDetectionEvaluation.Gating.IDetEvalGating import IDetEvalGating
from aspe.evaluation.RadarDetectionEvaluation.PreProcesors.IPreProcessor import IPreProcessor
from aspe.evaluation.RadarDetectionEvaluation.PreProcesors.RTBasedPreProcessor import RTBasedPreProcessor
from aspe.evaluation.RadarDetectionEvaluation.Utilities.support_functions import dict_based_df_concat
from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag, SignalInBoundsFlag
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.utilities.data_filters import slice_df_based_on_other


class DetectionEvaluation:
    """
    Detection evaluation class which can be configured covering various use cases with different methods for:
    - pre-processing - custom preprocesing handling different type of data.
                       More details can be found in IPreProcessor
    - gating - gating method to create pairs object-detection for which further features will be calculated.
               there may be many-to-many linkage after gating
               More details can be found in IDetEvalGating
    - paris features - feature list which will be calculated for each detection-object pair
    - association - paris feature based association. In general is method to calculate additional feature 'f_associated'
    """

    def __init__(self,
                 pre_processor: IPreProcessor = RTBasedPreProcessor(),
                 gating_method: IDetEvalGating = DistanceToCenterGating(),
                 pairs_features_list: List[
                     IDetEvalPairsFeatures] = default_feature_list.DEFAULT_DET_EVAL_PAIRS_FEATURES_LIST,
                 association: IFlag = SignalInBoundsFlag('p_value', min_value=0.01)):
        """

        :param pre_processor: pre-processor class
                              default: RTBasedPreProcessor()
        :type pre_processor: IPreProcessor
        :param gating_method: gating class
                              default: RTBasedPreProcessor()
        :type pre_processor: IDetEvalGating
        :param pairs_features_list: list of features for pairs
                                    default: default_feature_list.DEFAULT_DET_EVAL_PAIRS_FEATURES_LIST
        :type pre_processor: List[IDetEvalPairsFeatures]
        :param association: association method
                            default: SignalInBoundsFlag('p_value', min_value=0.001))
        :type pre_processor: IFlag
        """
        self.pre_processor = pre_processor
        self.gating_method = gating_method
        self.pairs_features_list = pairs_features_list
        self.association = association

    def evaluate_single_sensor(self,
                               estimated_data: ExtractedData,
                               reference_data: ExtractedData,
                               sensor_id: int) \
            -> Dict:
        """
        Method for evaluating single sensor data
        Note: there is no need to filter out single sensor data - that will be done by evaluation pre-processing
        :param estimated_data: Extracted estimated data congaing at least:
                               .detections
                               + all needed data by pre_processor (usually sensors)
        :type estimated_data: ExtractedData
        :param reference_data: Extracted reference data congaing at least:
                               .objects
                               .sensors
                               + all needed data by pre_processor (usually host)
        :type reference_data: ExtractedData
        :param sensor_id: ID of sensor which should be evaluated
        :type sensor_id: int
        :return: Dict with DataFrames:
                 'pairs_df' - pairs features DataFrame
                 'pairs_dets' - detections signals (estimated) synchronized with pairs_df
                 'pairs_ref' - objects signals (reference) synchronized with pairs_df
        """
        print(f'Evaluation of sensor with ID: {sensor_id}')
        est_synch, ref_synch = self.pre_processor.pre_process_data(estimated_data, reference_data, sensor_id)

        valid_pairs = self.gating_method.gating(est_synch, ref_synch)

        valid_pairs_synch_dets = slice_df_based_on_other(est_synch.detections.signals, valid_pairs,
                                                         ['scan_index', 'unique_id'], ['scan_index', 'unique_id_det'])

        valid_pairs_synch_objects = slice_df_based_on_other(ref_synch.objects.signals, valid_pairs,
                                                            ['scan_index', 'unique_id'],
                                                            ['scan_index', 'unique_id_obj'])

        valid_pairs_synch_sensor = slice_df_based_on_other(ref_synch.sensors.signals, valid_pairs,
                                                           ['scan_index'], ['scan_index'])
        if not valid_pairs.empty:
            valid_pairs_with_features = self._calc_features(valid_pairs_synch_dets,
                                                            valid_pairs_synch_objects,
                                                            valid_pairs_synch_sensor)

            f_associated_series = self.association.calc_flag(valid_pairs_with_features)
            f_associated = f_associated_series.to_frame('f_associated')

            pairs_df = pd.concat([valid_pairs, valid_pairs_with_features, f_associated], axis=1)
        else:
            pairs_df = valid_pairs
        output = {
            'pairs_df': pairs_df,
            'pairs_dets': valid_pairs_synch_dets,
            'pairs_ref': valid_pairs_synch_objects,
        }
        print(f'Single sensor evaluation done')
        return output

    def evaluate_single_data(self, estimated_data, reference_data):
        """
        Method for evaluating single data (can be multiple sensors)
        :param estimated_data: Extracted estimated data congaing at least:
                               .detections
                               + all needed data by pre_processor (usually sensors)
        :type estimated_data: ExtractedData
        :param reference_data: Extracted reference data congaing at least:
                               .objects
                               .sensors
                               + all needed data by pre_processor (usually host)
        :type reference_data: ExtractedData
        :return: Dict with DataFrames from evaluate_single_sensor +
                 'sensor_id_key' - Data frame with ids of sensors
        """
        output_dict = dict()
        sensors_ids = reference_data.sensors.per_sensor.sensor_id
        for single_sensor_id in sensors_ids:
            output_dict[single_sensor_id] = self.evaluate_single_sensor(estimated_data, reference_data,
                                                                        single_sensor_id)
        print(f'Accumulating different sensor data')
        flatten_dict_of_df = dict_based_df_concat(output_dict, 'sensor_id_key')
        print('Single log evaluation done')
        return flatten_dict_of_df

    def evaluate_multi_log(self, list_of_log: List[str], data_provider):
        """
        Method for evaluating many logs (can be multiple sensors)
        :param list_of_log: log list (paths to logs which can be consumed by data provider)
        :type list_of_log: List[str]
        :param data_provider: class for providing data:
                              estimated_data
                              reference_data
        :return: Dict with DataFrames from evaluate_single_data +
                 'logs' - DataFrame with paths to logs
                 'log_id_key' - DataFrame with log id
        """
        output_dict = dict()
        for idx, log_path in enumerate(list_of_log):
            print(f'Evaluation of log: {log_path}')
            estimated_data, reference_data = data_provider.get_single_log_data(log_path)
            output_dict[idx] = self.evaluate_single_data(estimated_data, reference_data)
            output_dict[idx]['logs'] = pd.DataFrame([log_path], columns=['log_path'])

        print(f'Accumulating different log data')
        flatten_dict_of_df = dict_based_df_concat(output_dict, 'log_id_key')
        print('Multi-log evaluation done')
        return flatten_dict_of_df

    def _calc_features(self, valid_pairs_synch_dets, valid_pairs_synch_objects, valid_pairs_synch_sensor):
        """
        Method for calculating features for pairs based on defined feature list
        :param valid_pairs_synch_dets:
        :param valid_pairs_synch_objects:
        :param valid_pairs_synch_sensor:
        :return:
        """
        pairs_features = []
        for feature in self.pairs_features_list:
            pairs_features.append(feature.calc_features(valid_pairs_synch_dets,
                                                        valid_pairs_synch_objects,
                                                        valid_pairs_synch_sensor))
        flat_pairs_features = pd.concat(pairs_features, axis=1)

        return flat_pairs_features
