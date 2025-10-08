import copy
from typing import Dict, List, Tuple, Union

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Association import IAssociation, RelEstToRefNN
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BasicBinaryClassifier, BCType, IBinaryClassifier
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PEObjectsEvaluationOutputSingleLog, PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.Flags import DistanceToCSOriginBelowThrFlag, IFlag, IsMovableFlag
from aspe.evaluation.RadarObjectsEvaluation.KPI import (
    DEFAULT_BIN_CLASS_KPI_LIST,
    DEFAULT_PAIRS_KPI_DICT,
    BinaryClassificationKPIManager,
    IBinaryClassificationKPI,
    ISeriesKPI,
    PairsFeaturesKPIManager,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import DEFAULT_PAIRS_FEATURES_LIST, IPairsFeature
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures import IPairsLifetimeFeature
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.default_pairs_lifetime_kpis import (
    DEFAULT_LIFETIME_FEATURES_KPIS,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import IPEPairedObjectsPreBuilder
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders.PEPairedObjectsPreBuilderGating import (
    PEPairedObjectsPreBuilderGating,
)
from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class PEPipeline:
    def __init__(self,
                 pairs_builder: IPEPairedObjectsPreBuilder = PEPairedObjectsPreBuilderGating(),
                 relevancy_estimated_objects: Union[IFlag, List[IFlag]] = IsMovableFlag(),
                 relevancy_reference_objects: Union[IFlag, List[IFlag]] = DistanceToCSOriginBelowThrFlag(thr=90.0),
                 association: IAssociation = RelEstToRefNN(),
                 binary_classification: IBinaryClassifier = BasicBinaryClassifier(),
                 pairs_features_list: List[IPairsFeature] = DEFAULT_PAIRS_FEATURES_LIST,
                 pairs_lifetime_features: List[IPairsLifetimeFeature] = [],
                 pairs_features_kpis: Dict[str, List[Tuple[ISeriesKPI, IFlag]]] = DEFAULT_PAIRS_KPI_DICT,
                 objects_features_kpis: List[IBinaryClassificationKPI] = DEFAULT_BIN_CLASS_KPI_LIST,
                 pairs_lifetime_kpis: List[ISeriesKPI] = DEFAULT_LIFETIME_FEATURES_KPIS,
                 evaluate_internal_objects: bool = False,
                 evaluate_oal_objects: bool = False,
                 ref_from_internal_objects: bool = False,
                 ref_from_oal_objects: bool = False,
                 ):
        """
        Pipeline configuration happens here.

        :param relevancy_estimated_objects: object instance of the relevancy_estimated_objects filter
        :type relevancy_estimated_objects: RelevancyInterface
        :param relevancy_reference_objects: object instance of the relevancy_reference_objects filter
        :type relevancy_reference_objects: RelevancyInterface
        :param association: object of the association methodology module
        :type association: AssociationInterface
        :param binary_classification: object of the binary classification module
        :type binary_classification: BinaryClassificationInterface
        :param pairs_features_list:
        :type pairs_features_list: List[IPairsFeature]
        """
        self.relevancy_est_objs = relevancy_estimated_objects
        self.relevancy_ref_objs = relevancy_reference_objects
        self.association = association
        self.binary_classifier = binary_classification
        self.pairs_features_list = pairs_features_list
        self.pairs_lifetime_features = pairs_lifetime_features
        self.pairs_lifetime_features_results = pd.DataFrame(columns=['unique_id', 'feature', 'value'])
        self.pairs_builder = pairs_builder
        self.kpi_manager_pairs_features = PairsFeaturesKPIManager(pairs_features_kpis)
        self.kpi_manager_bin_class = BinaryClassificationKPIManager(objects_features_kpis)
        self.pairs_lifetime_kpis = pairs_lifetime_kpis
        self.evaluate_internal_objects = evaluate_internal_objects
        self.evaluate_oal_objects = evaluate_oal_objects
        self.ref_from_internal_objects = ref_from_internal_objects
        self.ref_from_oal_objects = ref_from_oal_objects

        self.extracted_est_data = None
        self.extracted_ref_data = None
        self.pe_results_est = None
        self.pe_results_ref = None
        self.pairs_est_ref = None
        self.pairs_est = None
        self.pairs_ref = None
        self.kpis_binary_class = None
        self.kpis_pairs_features = None
        self.log_features = None

    def evaluate(self, estimated_data: ExtractedData, reference_data: ExtractedData,
                 *args, **kwargs) -> PEObjectsEvaluationOutputSingleLog:
        """
        This is a single log pipeline for radar objects evaluation

        :param estimated_data: data with estimated objects
        :type estimated_data: ExtractedData
        :param reference_data: data with reference objects
        :type reference_data: ExtractedData
        :return: PEObjectsEvaluationOutputSingleLog: performance evaluation data for single log
        """
        self.init_data_structures(estimated_data, reference_data)
        self.mark_relevant_records()
        self.calculate_pairs_features()
        self.associate_objects()
        self.binary_classification()
        self.calculate_pairs_lifetime_features()
        self.calculate_kpis()
        self.calculate_log_features()
        output = self.create_single_log_output_structure()
        output.print_kpis_and_log_features()
        return output

    def init_data_structures(self, estimated_data, reference_data):
        """
        Prepare data structures for further processing:
        - copy extracted reference and estimated data and assign to class attribute
        - copy extracted reference and estimated objects datasets and assign to class attribute
        - create data structure which represents objects pairs
        - create data structures for performance evaluation results for estimated and reference objects
        """
        self.extracted_est_data = copy.deepcopy(estimated_data)
        self.extracted_ref_data = copy.deepcopy(reference_data)

        self._choose_estimated_objects(estimated_data)
        self._choose_reference_objects(reference_data)

        pairs_builder_results = self.pairs_builder.build(self.extracted_est_objs, self.extracted_ref_objs)
        self.pairs_est_ref, self.pairs_est, self.pairs_ref = pairs_builder_results

        self.pe_results_est = PESingleObjects().base_signals_update(self.extracted_est_objs.signals)
        self.pe_results_ref = PESingleObjects().base_signals_update(self.extracted_ref_objs.signals)

    def _choose_estimated_objects(self, estimated_data: ExtractedData):
        if self.evaluate_internal_objects:
            self.extracted_est_objs = copy.deepcopy(estimated_data.internal_objects)
        elif self.evaluate_oal_objects and hasattr(estimated_data, 'oal_objects'):
            self.extracted_est_objs = copy.deepcopy(estimated_data.oal_objects)
        else:
            if estimated_data.objects is not None:
                self.extracted_est_objs = copy.deepcopy(estimated_data.objects)
            else:
                self.extracted_est_objs = estimated_data.internal_objects
                self.extracted_est_objs.signals = self.extracted_est_objs.signals.query('reduced_id > 0').reset_index()

    def _choose_reference_objects(self, reference_data: ExtractedData):
        if self.ref_from_internal_objects:
            self.extracted_ref_objs = copy.deepcopy(reference_data.internal_objects)
        elif self.ref_from_oal_objects and hasattr(reference_data, 'oal_objects'):
            self.extracted_ref_objs = copy.deepcopy(reference_data.oal_objects)
        else:
            self.extracted_ref_objs = copy.deepcopy(reference_data.objects)

    def mark_relevant_records(self):
        """
        1. Apply relevancy filters on reference data from extraction -> should return series
        2. Apply relevancy filters on estimated data from extraction -> should return series
        3. Update pe_pairs, pe_est, pe_ref structures using both series
        """

        def get_relevancy(relevancy_flag, signals, name):
            if not isinstance(relevancy_flag, list):
                relevant = relevancy_flag.calc_flag(signals)
            else:
                relevant = pd.Series(np.full(len(signals), True))
                for flag in relevancy_flag:
                    relevant = relevant & flag.calc_flag(signals)
            relevant.name = name
            return relevant

        relevant_est = get_relevancy(self.relevancy_est_objs, self.extracted_est_objs.signals, 'relevancy_flag_est')
        relevant_ref = get_relevancy(self.relevancy_ref_objs, self.extracted_ref_objs.signals, 'relevancy_flag_ref')
        self.pe_results_est.signals.loc[:, 'relevancy_flag'] = relevant_est
        self.pe_results_ref.signals.loc[:, 'relevancy_flag'] = relevant_ref

        self.pairs_est_ref.signals.loc[:, 'relevancy_flag_ref'] = relevant_ref.loc[
            self.pairs_est_ref.signals.index_ref].to_numpy()
        self.pairs_est_ref.signals.loc[:, 'relevancy_flag_est'] = relevant_est.loc[
            self.pairs_est_ref.signals.index_est].to_numpy()

    def associate_objects(self):
        is_associated, additional_info = self.association.associate(self.pairs_est_ref.signals, self.pairs_est,
                                                                    self.pairs_ref)
        if additional_info is not None:
            self.pairs_est_ref.signals = pd.concat([self.pairs_est_ref.signals, additional_info], axis=1)

        self.pairs_est_ref.signals['is_associated'] = is_associated
        self.pe_results_ref.signals['is_associated'] = False
        self.pe_results_est.signals['is_associated'] = False

        ref_assoc_idxs = self.pairs_est_ref.signals.loc[is_associated, 'index_ref']
        est_assoc_idxs = self.pairs_est_ref.signals.loc[is_associated, 'index_est']

        self.pe_results_ref.signals.loc[ref_assoc_idxs, 'is_associated'] = True
        self.pe_results_est.signals.loc[est_assoc_idxs, 'is_associated'] = True

    def binary_classification(self):
        self.binary_classifier.initial_classification(self.pairs_est_ref)
        self.binary_classifier.initial_classification(self.pe_results_ref)
        self.binary_classifier.initial_classification(self.pe_results_est)

        self.binary_classifier.classify_pairs(self.pairs_est_ref)
        self.binary_classifier.classify_estimated_data(self.pe_results_est, self.pairs_est_ref)
        self.binary_classifier.classify_reference_data(self.pe_results_ref, self.pairs_est_ref)

    def calculate_pairs_features(self):
        """
        1. Calculate features on paired objects (deviations) - loop over defined feature list
        2. Every loop run add pe_pairs column
        """
        # TODO consider placing this in separate class (PairsFeaturesManager?)
        for feature in self.pairs_features_list:
            try:
                features_df = feature.calculate(self.pairs_est, self.pairs_ref)
                self.pairs_est_ref.signals = pd.concat([self.pairs_est_ref.signals, features_df], axis=1)
            except KeyError:
                print(f'Cannot calculate feature {feature.__class__.__name__}. '
                      f'Missing signals')

    def calculate_pairs_lifetime_features(self):
        results = []
        if not self.pairs_lifetime_features:
            self.pairs_lifetime_features_results = pd.DataFrame()
        else:
            for unique_id, pair in self.pairs_est_ref.signals.groupby(by='unique_id'):
                est_object = self.pairs_est.loc[pair.index, :]
                ref_object = self.pairs_ref.loc[pair.index, :]
                single_pair_temp_results = []
                for feature in self.pairs_lifetime_features:
                    pair_results = feature.calculate(est_object, ref_object, pair)
                    single_pair_temp_results.append(pair_results)
                single_pair_results = pd.concat(single_pair_temp_results, axis=1)
                single_pair_results['unique_id'] = unique_id
                results.append(single_pair_results)
            if len(results):
                self.pairs_lifetime_features_results = pd.concat(results).reset_index(drop=True)
            else:
                self.pairs_lifetime_features_results = pd.DataFrame()

    def calculate_kpis(self):
        self.kpis_pairs_features = self.kpi_manager_pairs_features.calculate_kpis(self.pairs_est_ref)
        self.kpis_binary_class = self.kpi_manager_bin_class.calculate_kpis(self.pe_results_ref, self.pe_results_est)
        self.calculate_pairs_lifetime_kpis(self.pairs_lifetime_features_results)

    def calculate_log_features(self):
        tp_count = np.sum(self.pairs_est_ref.signals.loc[:, 'binary_classification'] == BCType.TruePositive)
        fp_count = np.sum(self.pe_results_est.signals.loc[:, 'binary_classification'] == BCType.FalsePositive)
        fn_count = np.sum(self.pe_results_ref.signals.loc[:, 'binary_classification'] == BCType.FalseNegative)
        under_seg_count = np.sum(
            self.pe_results_ref.signals.loc[:, 'binary_classification'] == BCType.UnderSegmentation)
        over_seg_count = np.sum(self.pe_results_ref.signals.loc[:, 'binary_classification'] == BCType.OverSegmentation)

        rel_est_count = np.sum(self.pe_results_est.signals.loc[:, 'relevancy_flag'].values)
        rel_ref_count = np.sum(self.pe_results_ref.signals.loc[:, 'relevancy_flag'].values)

        est_records_count = self.pe_results_est.signals.shape[0]
        ref_records_count = self.pe_results_ref.signals.shape[0]

        self.log_features = pd.DataFrame({'signature': ['TP_count', 'FP_count', 'FN_count', 'under_segmentation_count',
                                                        'over_segmentation_count',
                                                        'relevant_est_records_count', 'relevant_ref_records_count',
                                                        'all_est_records_count', 'all_ref_records_count'],
                                          'value': [tp_count, fp_count, fn_count, under_seg_count, over_seg_count,
                                                    rel_est_count, rel_ref_count,
                                                    est_records_count, ref_records_count]})

    def create_single_log_output_structure(self):
        output = PEObjectsEvaluationOutputSingleLog(
            extr_est_objs=self.extracted_est_objs,
            extr_ref_objs=self.extracted_ref_objs,
            extr_ref_host=self.extracted_ref_data.host,
            extr_radar_sensors=self.extracted_est_data.sensors,
            extr_radar_detections=self.extracted_est_data.detections,
            paired_objs=self.pairs_est_ref,
            pairs_lifetime_features=self.pairs_lifetime_features_results,
            est_objs_results=self.pe_results_est,
            ref_objs_results=self.pe_results_ref,
            objects_kpis_per_log=self.kpis_binary_class,
            pairs_kpis_per_log=self.kpis_pairs_features,
            pairs_lifetime_kpis_per_log=self.kpis_pairs_lifetime,
            logs_features=self.log_features,
        )
        return output

    def calculate_pairs_lifetime_kpis(self, pairs_lifetime_features_results):
        results = {'feature_signature': [],
                   'kpi_signature': [],
                   'n_samples': [],
                   'kpi_value': [], }
        for feature_signature, kpis in self.pairs_lifetime_kpis.items():
            if feature_signature in pairs_lifetime_features_results:
                feature = pairs_lifetime_features_results.loc[:, feature_signature].to_numpy()
                feature = feature[~np.isnan(feature)]
                for kpi in kpis:
                    if len(feature) > 0:
                        kpi_value = kpi.calculate_kpi(feature)
                    else:
                        kpi_value = np.nan
                    results['feature_signature'].append(feature_signature)
                    results['kpi_signature'].append(kpi.kpi_signature)
                    results['n_samples'].append(len(feature))
                    results['kpi_value'].append(kpi_value)
        self.kpis_pairs_lifetime = pd.DataFrame(results)


if __name__ == '__main__':
    from aspe.providers.F360MudpRTRangeDataProvider import F360MudpRTRangeDataProvider

    log_path = [
        # r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
        # r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\rRf360t4060306v204p50\FTP402_TC1_90_151039_001_rRf360t4060306v204p50.mudp",
        # r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl",
        r"C:\logs\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA\rRf360t4060306v204p50\FTP402_TC1_90_150938_001_rRf360t4060306v204p50.dvl"]
    mudp_parser_config_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\parsers_config\mudp_data_parser_config.json"
    dvl_parser_config_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\parsers_config\dvl_data_parser_config.json"
    mudp_stream_def_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions"

    data_gen = F360MudpRTRangeDataProvider(mudp_parser_config_path, dvl_parser_config_path, mudp_stream_def_path)
    test_vector = data_gen.get_single_log_data(log_path[0])
    pe = PEPipeline()
    out = pe.evaluate(*test_vector)
