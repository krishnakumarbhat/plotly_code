import copy

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PEObjectsEvaluationOutputSingleLog, PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline

class PEPipeline50k(PEPipeline):
    def __init__(self):
        super().__init__()


    def evaluate(self, estimated_objs: ExtractedData, reference_objs: ExtractedData,
                 *args, **kwargs) -> PEObjectsEvaluationOutputSingleLog:
        """
        This is a single log pipeline for radar objects evaluation

        :param estimated_data: data with estimated objects
        :type estimated_data: ExtractedData
        :param reference_data: data with reference objects
        :type reference_data: ExtractedData
        :return: PEObjectsEvaluationOutputSingleLog: performance evaluation data for single log
        """
        self.init_data_structures(estimated_objs, reference_objs)
        self.mark_relevant_records()
        self.calculate_association_distances()
        self.associate_objects()
        self.binary_classification()

    def init_data_structures(self, estimated_data, reference_data):
        """
        Prepare data structures for further processing:
        - copy extracted reference and estimated data and assign to class attribute
        - copy extracted reference and estimated objects datasets and assign to class attribute
        - create data structure which represents objects pairs
        - create data structures for performance evaluation results for estimated and reference objects
        """

        self.extracted_est_objs = copy.deepcopy(estimated_data)
        self.extracted_ref_objs = copy.deepcopy(reference_data)

        pairs_builder_results = self.pairs_builder.build(self.extracted_est_objs, self.extracted_ref_objs)
        self.pairs_est_ref, self.pairs_est, self.pairs_ref = pairs_builder_results

        self.pe_results_est = PESingleObjects().base_signals_update(self.extracted_est_objs.signals)
        self.pe_results_ref = PESingleObjects().base_signals_update(self.extracted_ref_objs.signals)

        self.kpis_binary_class = None
        self.kpis_pairs_features = None
        self.log_features = None

