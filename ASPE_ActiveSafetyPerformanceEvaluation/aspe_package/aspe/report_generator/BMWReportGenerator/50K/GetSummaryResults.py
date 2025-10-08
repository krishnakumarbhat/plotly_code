import os
import pickle
from pprint import pprint

import numpy as np
from tqdm import tqdm
from copy import deepcopy

from aspe.extractors.Interfaces.Enums.Object import ObjectClass, MovementStatus

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BCType, BasicBinaryClassifier

from aspe.evaluation.RadarObjectsEvaluation.Flags import IsVisibleFlag, SignalInBoundsFlag, IsMovableFlag, SignalEqualityFlag
from aspe.evaluation.RadarObjectsEvaluation.Metrics.ComplexMahalanobisDistance import ComplexMahalanobisDistance
from aspe.report_generator.BMWReportGenerator import PEPipeline50k
import pandas as pd


class GetSummaryResults():
    class_mapper = {'veh': ObjectClass.CAR,
                    'ped': ObjectClass.PEDESTRIAN}

    stats = ['TP', 'FP', 'FN', 'number_of_si']
    classes_for_stats = ['veh', 'ped']
    def __init__(self, pe_pipeline, input_path, existence_thresholds=[0.5, 0.8, 0.9, 0.95, 0.999], output_path=""):
        self.output_path = output_path
        self.pe_pipeline = pe_pipeline
        self.existence_thresholds = existence_thresholds
        self.input_path = input_path
        self.output = self.init_output_structure()

    def init_output_structure(self):
        existence_labels = self.existence_thresholds.copy()
        existence_labels.extend(self.classes_for_stats)
        columns = [f'{x}_{str(y)}' for x in self.stats for y in existence_labels]
        return pd.DataFrame(columns=columns)

    def get_stat_for_class_type(self, pe_output, class_name):
        self.pe_pipeline.init_data_structures(pe_output)
        binary_classifier = BasicBinaryClassifier()

        relevancy_ref_objs = deepcopy(self.relevancy_ref_objs)
        relevancy_ref_objs.extend([SignalEqualityFlag(signal_name='object_class', signal_expected_value=class_name)])
        self.pe_pipeline.relevancy_ref_objs = relevancy_ref_objs
        self.pe_pipeline.relevancy_est_objs = self.relevancy_est_objs
        ref_filtered = self.pe_pipeline.extracted_ref_objs.signals[self.pe_pipeline.extracted_ref_objs.signals.object_class ==  class_name]
        ref_results = self.pe_pipeline.pe_results_ref.signals[self.pe_pipeline.pe_results_ref.signals.unique_id.isin(ref_filtered.unique_id) & self.pe_pipeline.pe_results_ref.signals.scan_index.isin(ref_filtered.scan_index)]

        # self.pe_pipeline.pe_results_ref.signals = ref_results
        output = self.pe_pipeline.evaluate()
        est = output.pe_results_obj_est
        ref = output.pe_results_obj_ref

        ref.signals = ref.signals[ref.signals.index.isin(ref_results.index)]
        est_binary_classification = est.signals.loc[:, 'binary_classification'].values
        ref_binary_classification = ref.signals.loc[:, 'binary_classification'].values


        tp_count = np.sum(ref_binary_classification == BCType.TruePositive)
        fn_count = np.sum(ref_binary_classification == BCType.FalseNegative)
        # fp_count = np.sum(est_binary_classification == BCType.FalsePositive)
        number_of_si = est.signals.loc[:, 'scan_index'].drop_duplicates().shape[0]

        return {'TP': tp_count, 'FN': fn_count, 'number_of_si': number_of_si}

    def get_relevancy_flags(self):
        self.relevancy_est_objs = deepcopy(list(self.pe_pipeline.relevancy_est_objs))
        self.relevancy_ref_objs = deepcopy(list(self.pe_pipeline.relevancy_ref_objs))


    def get_stat_for_existence_threshold(self, pe_output, thresh):
        self.pe_pipeline.init_data_structures(pe_output)
        binary_classifier = BasicBinaryClassifier()
        relevancy_est_objs = deepcopy(self.relevancy_est_objs)
        relevancy_est_objs.extend([SignalInBoundsFlag('existence_indicator', min_value=thresh)])
        self.pe_pipeline.relevancy_est_objs = relevancy_est_objs
        self.pe_pipeline.relevancy_ref_objs = self.relevancy_ref_objs
        output = self.pe_pipeline.evaluate()


        est = output.pe_results_obj_est
        ref = output.pe_results_obj_ref


        est_binary_classification = est.signals.loc[:, 'binary_classification'].values
        ref_binary_classification = ref.signals.loc[:, 'binary_classification'].values
        tp_count = np.sum(est_binary_classification == BCType.TruePositive)
        fn_count = np.sum(ref_binary_classification == BCType.FalseNegative)
        fp_count = np.sum(est_binary_classification == BCType.FalsePositive)
        number_of_si = est.signals.loc[:, 'scan_index'].drop_duplicates().shape[0]
        return {'TP':tp_count, 'FP': fp_count, 'FN': fn_count, 'number_of_si': number_of_si}

    # def get_stat_for_class_type(self, pe_output, class_name):

    def aggregate_stats(self):
        for file in os.listdir(self.input_path):
            with open(os.path.join(self.input_path, file), 'rb') as file:
                pe_output = pickle.load(file)
            row = {}
            self.get_relevancy_flags()
            for filtering_stat in tqdm(self.existence_thresholds + self.classes_for_stats):
                # filtering_stat = col[0].split('_')[-1]
                if filtering_stat not in ['ped', 'veh']:
                    stats = self.get_stat_for_existence_threshold(pe_output, float(filtering_stat))
                else:
                    stats = self.get_stat_for_class_type(pe_output, self.class_mapper[filtering_stat])
                for k,v in stats.items():
                    row.update({f'{k}_{filtering_stat}':v})

            self.output = self.output.append(row, ignore_index=True)

            self.output.to_excel(os.path.join(self.output_path, '50K_summary_results.xlsx'), engine='openpyxl')
        return self.output

    def stats_from_output(self, output):
        df = pd.DataFrame(columns=['TPR', 'PPV', 'FP_per_sc'])
        sum = output.sum()
        for thresh in self.existence_thresholds:
            TP = sum[f'TP_{thresh}']
            FP = sum[f'FP_{thresh}']
            FN = sum[f'FN_{thresh}']

            TPR = TP/(TP+FN)
            PPV = TP/(TP+FP)
            FP_per_sc = FP/sum[f'number_of_si_{thresh}']
            df.loc[thresh] = {'TPR': TPR, 'PPV': PPV, 'FP_per_sc': FP_per_sc}
        return df

if __name__ == "__main__":
    visibility_threshold = 0.5
    vel_threshold = 0.2

    pe_pipeline = PEPipeline50k(
        relevancy_reference_objects=[
            IsVisibleFlag(visibility_threshold),
            SignalEqualityFlag(signal_name='movement_status', signal_expected_value=MovementStatus.MOVING),],
        relevancy_estimated_objects=[IsMovableFlag()],
        association_distance=ComplexMahalanobisDistance(scale_x=1 / 2),)

    results_generator = GetSummaryResults(pe_pipeline, r"C:\Users\zj9lvp\Downloads\temp_pe_out", output_path=r"C:\Users\zj9lvp\Documents\aspe_bmw_data\pandora")
    stats = results_generator.aggregate_stats()
    general_stats = results_generator.stats_from_output(stats)
    pprint(stats)
    with open(r'C:\Users\zj9lvp\Documents\aspe_bmw_data\pandora\output_test.pkl', 'wb') as file:
        pickle.dump(stats, file)
    pass

