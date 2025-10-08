import copy

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PEObjectsEvaluationOutputSingleLog, PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline

class PEPipeline50k(PEPipeline):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


    def evaluate(self, *args, **kwargs) -> PEObjectsEvaluationOutputSingleLog:
        """
        This is a single log pipeline for radar objects evaluation

        :param estimated_data: data with estimated objects
        :type estimated_data: ExtractedData
        :param reference_data: data with reference objects
        :type reference_data: ExtractedData
        :return: PEObjectsEvaluationOutputSingleLog: performance evaluation data for single log
        """
        # self.init_data_structures(estimated_objs, reference_objs)
        self.mark_relevant_records()
        # self.calculate_association_distances()
        self.associate_objects()
        self.binary_classification()
        self.calculate_kpis()
        return self.create_single_log_output_structure()

    def init_data_structures(self, pe_output):
        self.extracted_est_objs = copy.deepcopy(pe_output.extracted_estimated_objects)
        self.extracted_ref_objs = copy.deepcopy(pe_output.extracted_reference_objects)
        self.pe_results_est = copy.deepcopy(pe_output.pe_results_obj_est)
        self.pe_results_ref = copy.deepcopy(pe_output.pe_results_obj_ref)
        self.pairs_est_ref = copy.deepcopy(pe_output.pe_results_obj_pairs)
        kpis_binary_class = None

    def create_single_log_output_structure(self):
        output = PEObjectsEvaluationOutputSingleLog(
            extr_est_objs=self.extracted_est_objs,
            extr_ref_objs=self.extracted_ref_objs,
            paired_objs=self.pairs_est_ref,
            est_objs_results=self.pe_results_est,
            ref_objs_results=self.pe_results_ref,
            objects_kpis_per_log=self.kpis_binary_class,
        )
        return output

    def calculate_kpis(self):
        self.kpis_binary_class = self.kpi_manager_bin_class.calculate_kpis(self.pe_results_ref, self.pe_results_est)
