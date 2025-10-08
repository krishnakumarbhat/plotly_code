from aspe.report_generator.PlotGenerator import PlotGenerator
from aspe.report_generator.ReportBuilder import ReportBuilder
from aspe.report_generator.perf_eval_report_input_preparation import load_perf_eval_results
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalEnvelopeInBounds import SignalEnvelopeInBoundsFlag
from pathlib import Path
import pandas as pd
from typing import List, Callable
import numpy as np
from aspe.utilities.SupportingFunctions import rms


class MinMaxFeaturesFilter:
    def __init__(self, pe_results_obj_pairs, min_value, max_value, signal_to_filter):
        self.min_value = min_value
        self.max_value = max_value
        self.signal_to_filter_name = signal_to_filter
        filter_signal = pe_results_obj_pairs.loc[:, signal_to_filter]
        self.filtered_features = pe_results_obj_pairs.loc[(min_value < filter_signal) & (filter_signal < max_value), :]

    def get_max_estimated_value_for_column(self, column_name: str, estimator: Callable):
        estimated_vals = []
        for _, group in self.filtered_features.groupby(by=['sw_version', 'subset']):
            estimated_vals.append(estimator(group.loc[:, column_name]))
        estimated_max = max(estimated_vals) * 1.05
        return estimated_max


def generate_overview_report(report_input: dict, pdf_output_path: str,
                             dist_to_host_ranges: tuple = ((0, 12), (12, 40), (40, 100)),
                             target_maneuvering_yaw_rate_thr: float = 0.02):
    """
    Generate pdf report which contains barplots of RMS errors.
    @param report_input: dictionary containing pe_output .pickle paths. It's form should be:
        report_input = {
            'sw_version_1': {
                'test_case_1': PATCH_TO_PE_OUTPUT_SW_1_TEST_CASE_1,
                'test_case_2': PATCH_TO_PE_OUTPUT_SW_1_TEST_CASE_2,
            },

            'sw_version_2': {
                'test_case_1': PATCH_TO_PE_OUTPUT_SW_2_TEST_CASE_1,
                'test_case_2': PATCH_TO_PE_OUTPUT_SW_2_TEST_CASE_2,
            },
        }
    @param pdf_output_path: path of output .pdf file (should include '.pdf' at the end)
    @param dist_to_host_ranges: tuples of to-host ranges which are analyzed separately. It's form should be:
        dist_to_host_ranges=((RANGE_1_DIST_MIN, RANGE_1_DIST_MAX), (RANGE_2_DIST_MIN, RANGE_2_DIST_MAX), ...)
    @param target_maneuvering_yaw_rate_thr: threshold in radians/sec for target yaw rate which is used for splitting to
    maneuvering/not maneuvering subsets
    """
    pe_out = load_perf_eval_results(report_input)
    pairs_features = pe_out['pe_results_obj_pairs']
    pairs_features = pairs_features.loc[pairs_features.binary_classification == BCType.TruePositive, :] # take only True Positives
    dist_to_host_slots = [MinMaxFeaturesFilter(pairs_features, min_value=min_dist, max_value=max_dist, signal_to_filter='distance_to_ref_obj') for min_dist, max_dist in dist_to_host_ranges]

    pe_results_ref = pe_out['pe_results_obj_ref']
    dist_to_host_ref_objs_results = [MinMaxFeaturesFilter(pe_results_ref, min_value=min_dist, max_value=max_dist, signal_to_filter='distance_to_ref_obj') for min_dist, max_dist in dist_to_host_ranges]

    add_is_maneuvering_flag_to_pairs_feats(pairs_features, target_maneuvering_yaw_rate_thr)
    pairs_feats_maneuvering = pairs_features.loc[pairs_features.is_maneuvering, :]
    pairs_feats_not_maneuvering = pairs_features.loc[~pairs_features.is_maneuvering.astype(bool), :]

    plot_gen = PlotGenerator('subset', 'sw_version', hue_order=list(report_input.keys()))
    report_gen = ReportBuilder()

    # main page
    report_gen.add_report_titles(titles=['Aptiv Active Safety KPI report', 'F360 Core Tracker'])
    report_gen.add_software_names_in_main_page_template(software_names=list(report_input.keys()))
    report_gen.add_main_page_description(tests_desc='Test cases description can be found: https://confluence.asux.aptiv.com/display/F360Core/Test+cases+description')

    create_samples_count_section(report_gen, plot_gen, pairs_features, dist_to_host_slots, pairs_feats_maneuvering, pairs_feats_not_maneuvering)
    create_binary_classification_section(report_gen, plot_gen, pe_out, dist_to_host_ref_objs_results)

    plot_gen.fig_size = (12, 4)
    create_feature_plots_with_distance_to_host_filter(report_gen, plot_gen, dist_to_host_slots, 'Position longitudinal error', 'dev_position_x', estimator=rms, estimator_name='RMSE')
    create_feature_plots_with_distance_to_host_filter(report_gen, plot_gen, dist_to_host_slots, 'Position lateral error', 'dev_position_y', estimator=rms, estimator_name='RMSE')
    create_feature_plots_with_distance_to_host_filter(report_gen, plot_gen, dist_to_host_slots, 'Bounding box IoU', 'intersection_over_union', estimator=np.mean, estimator_name='mean')

    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Velocity longitudinal error', 'dev_velocity_x')
    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Velocity lateral error', 'dev_velocity_y')
    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Acceleration longitudinal error', 'dev_acceleration_otg_x')
    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Acceleration lateral error', 'dev_acceleration_otg_y')
    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Orientation error', 'dev_orientation')
    create_feature_plots_with_target_maneuvering_filter(report_gen, plot_gen, pairs_feats_maneuvering, pairs_feats_not_maneuvering, 'Yaw rate error', 'dev_yaw_rate')

    report_gen.generate_pdf_report(
        output_filename=pdf_output_path,
        css_name='style.css',
    )


def add_is_maneuvering_flag_to_pairs_feats(pairs_features: pd.DataFrame, target_maneuvering_yaw_rate_thr: float):
    yaw_rate_in_bounds_flag = SignalEnvelopeInBoundsFlag('ref_yaw_rate', max_value=target_maneuvering_yaw_rate_thr,
                                                         env_decay_time=15,
                                                         columns_to_groupby=('log_index', 'unique_id', 'subset', 'sw_version'))
    is_not_maneuvering = yaw_rate_in_bounds_flag.calc_flag(pairs_features)
    pairs_features.loc[:, 'is_maneuvering'] = ~(is_not_maneuvering.to_numpy())


def create_binary_classification_section(report_gen: ReportBuilder, plot_gen: PlotGenerator, pe_out: dict, dist_to_host_ref_objs_results: list):
    report_gen.add_section(section_name="Binary classification")
    plot_gen.fig_size = (12, 4)

    bin_class = pe_out['kpis_binary_class_aggregated'].groupby(by='signature')
    fig = plot_gen.bar_plot(bin_class.get_group('TPR'), y_column='value', ylim=(0, 1.05))
    report_gen.add_plot(fig=fig, fig_name=f"TPR - all samples")

    for dist_to_host_slot in dist_to_host_ref_objs_results:
        tpr = calculate_tpr(dist_to_host_slot)
        fig = plot_gen.bar_plot(tpr, y_column='tpr_value', ylim=(0, 1.05))
        report_gen.add_plot(fig=fig, fig_name=f"TPR - range {dist_to_host_slot.min_value} - {dist_to_host_slot.max_value} [m] from host ")


def calculate_tpr(dist_to_host_ref_objs_result):
    tpr_results = {'subset': [], 'sw_version': [], 'tpr_value': []}
    for (scenario, sw), group in dist_to_host_ref_objs_result.filtered_features.groupby(by=['subset', 'sw_version']):
        tp_count = np.sum(group.binary_classification == BCType.TruePositive)
        fn_count = np.sum(group.binary_classification == BCType.FalseNegative)
        if (tp_count + fn_count) > 0:
            tpr = tp_count / (tp_count + fn_count)
        else:
            tpr = 0
        tpr_results['subset'].append(scenario)
        tpr_results['sw_version'].append(sw)
        tpr_results['tpr_value'].append(tpr)
    return pd.DataFrame(tpr_results)


def create_samples_count_section(report_gen: ReportBuilder,
                                 plot_gen: PlotGenerator,
                                 pairs_features: pd.DataFrame,
                                 dist_to_host_pairs_features_slots: List[MinMaxFeaturesFilter],
                                 pairs_feats_maneuvering, pairs_feats_not_maneuvering):
    report_gen.add_section(section_name='Samples count')
    all_samples_count = pairs_features.groupby(by=['subset', 'sw_version']).count().reset_index().rename(columns={'scan_index': 'samples count'})
    fig = plot_gen.bar_plot(all_samples_count, 'samples count')
    report_gen.add_plot(fig=fig, fig_name=f"All samples count")
    sws = np.unique(pairs_features.sw_version)
    testcases = np.unique(pairs_features.subset)
    for dist_to_host_slot in dist_to_host_pairs_features_slots:
        feats = dist_to_host_slot.filtered_features
        samples_count = {'sw_version': [], 'subset': [], 'samples_count': []}
        for sw in sws:
            for tc in testcases:
                samples_count['sw_version'].append(sw)
                samples_count['subset'].append(tc)
                sw_tc_samples = feats.loc[(feats['sw_version'] == sw) & (feats['subset'] == tc), :]
                samples_count['samples_count'].append(len(sw_tc_samples))
        samples_count = pd.DataFrame(samples_count)
        fig = plot_gen.bar_plot(samples_count, 'samples_count')
        report_gen.add_plot(fig=fig, fig_name=f"Samples count in range {dist_to_host_slot.min_value} - {dist_to_host_slot.max_value} [m] from host")

    samples_count = {'sw_version': [], 'subset': [], 'samples_count_maneuvering': [], 'samples_count_not_maneuvering': []}
    for sw in sws:
        for tc in testcases:
            samples_count['sw_version'].append(sw)
            samples_count['subset'].append(tc)
            manv_samples = len(pairs_feats_maneuvering.loc[(pairs_feats_maneuvering['sw_version'] == sw) & (pairs_feats_maneuvering['subset'] == tc), :])
            not_manv_samples = len(pairs_feats_not_maneuvering.loc[(pairs_feats_not_maneuvering['sw_version'] == sw) & (pairs_feats_not_maneuvering['subset'] == tc), :])
            samples_count['samples_count_maneuvering'].append(manv_samples)
            samples_count['samples_count_not_maneuvering'].append(not_manv_samples)
    samples_count = pd.DataFrame(samples_count)
    fig = plot_gen.bar_plot(samples_count, 'samples_count_not_maneuvering')
    report_gen.add_plot(fig=fig, fig_name=f"Samples count for not maneuvering target")
    fig = plot_gen.bar_plot(samples_count, 'samples_count_maneuvering')
    report_gen.add_plot(fig=fig, fig_name=f"Samples count for maneuvering target")


def create_feature_plots_with_distance_to_host_filter(report_gen: ReportBuilder, plot_gen: PlotGenerator,
                                                      dist_to_host_pairs_features_slots: List[MinMaxFeaturesFilter],
                                                      section_name: str, pair_feature_column_name: str,
                                                      estimator: Callable,
                                                      estimator_name: str):
    if pair_feature_column_name in dist_to_host_pairs_features_slots[0].filtered_features:
        y_max = max([s.get_max_estimated_value_for_column(pair_feature_column_name, estimator) for s in dist_to_host_pairs_features_slots])
        report_gen.add_section(section_name=section_name)
        for dist_to_host_slot in dist_to_host_pairs_features_slots:
            fig = plot_gen.bar_plot_with_estimator(dist_to_host_slot.filtered_features, pair_feature_column_name, estimator=estimator, ylim=(0, y_max))
            report_gen.add_plot(fig=fig, fig_name=f"Range {dist_to_host_slot.min_value} - {dist_to_host_slot.max_value} [m] from host, {estimator_name}")


def create_feature_plots_with_target_maneuvering_filter(report_gen: ReportBuilder, plot_gen: PlotGenerator,
                                                        target_maneuvering_features: pd.DataFrame,
                                                        target_not_maneuvering_features: pd.DataFrame,
                                                        section_name: str, pair_feature_column_name: str):
    def find_max_estimated_value():
        estimated_vals = []
        for _, group in target_maneuvering_features.groupby(by=['sw_version', 'subset']):
            estimated_vals.append(rms(group.loc[:, pair_feature_column_name]))
        for _, group in target_not_maneuvering_features.groupby(by=['sw_version', 'subset']):
            estimated_vals.append(rms(group.loc[:, pair_feature_column_name]))
        return max(estimated_vals)

    if pair_feature_column_name in target_maneuvering_features:
        y_max = find_max_estimated_value() * 1.05
        report_gen.add_section(section_name=section_name)
        fig = plot_gen.bar_plot_with_estimator(target_not_maneuvering_features, pair_feature_column_name, estimator=rms, ylim=(0, y_max))
        report_gen.add_plot(fig=fig, fig_name=f"Target not maneuvering")
        fig = plot_gen.bar_plot_with_estimator(target_maneuvering_features, pair_feature_column_name, estimator=rms, ylim=(0, y_max))
        report_gen.add_plot(fig=fig, fig_name=f"Target maneuvering")


def add_feature_plots(report_gen: ReportBuilder, plot_gen: PlotGenerator, pairs_features:pd.DataFrame,
                      section_name: str, pair_feature_column_name: str):
    if pair_feature_column_name in pairs_features:
        report_gen.add_section(section_name=section_name)
        fig = plot_gen.bar_plot_with_estimator(pairs_features, pair_feature_column_name, estimator=rms)
        report_gen.add_plot(fig=fig, fig_name=f"All samples RMSE")


if __name__ == '__main__':

    report_input = {
        'sw_version_1': {
            'test_case_1': r"C:\logs\bbox_managment\SRR_DEBUG\rRf360t4280309v205p50_resize_fix_B\DS_01_pe_output.pickle",
            'test_case_2': r"C:\logs\bbox_managment\SRR_DEBUG\rRf360t4280309v205p50_resize_fix_B\DS_07_pe_output.pickle",
        },

        'sw_version_2': {
            'test_case_1': r"C:\logs\bbox_managment\SRR_DEBUG\rRf360t4280309v205p50_resize_fix\DS_01_pe_output.pickle",
            'test_case_2': r"C:\logs\bbox_managment\SRR_DEBUG\rRf360t4280309v205p50_resize_fix\DS_07_pe_output.pickle",
        },
    }

    generate_overview_report(report_input, f'aspe_overview_report.pdf')
