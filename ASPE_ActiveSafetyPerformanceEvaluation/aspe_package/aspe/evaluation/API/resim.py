from datetime import datetime
from os import getlogin
from pathlib import Path
from typing import List, Optional, Set, Union

import jinja2
import pdfkit

from aspe.evaluation.RadarObjectsEvaluation.Association.AssociateAll import AssociateAll
from aspe.evaluation.RadarObjectsEvaluation.Flags import AllTrueFlag
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.PPVObjectsKPI import PPVBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.KPI.BinaryClassificationKPI.TPRObjectsKPI import TPRBinaryClassificationKPI
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import PEPairedObjectsPreBuilderGating
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.extractors.API.mudp import parse_mudp
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.parsers.testing import assert_mudp_parsed_data_equal
from aspe.utilities.SupportingFunctions import get_f360_mudp_resim_log_path


def evaluate_resim_single_log(parsed_data_before: dict, parsed_data_after: dict,
                              distance_threshold: float = 0.01,
                              optimize_memory_usage: bool = True,
                              compare_internal_objects: bool = False) -> dict:
    """
    Executes extraction and PEPipeline for provided instances of parsed data.
    :param parsed_data_before: parsed data
    :param parsed_data_after: parsed data
    :param distance_threshold: distance threshold to match two objects from before and after [m]
    :param optimize_memory_usage: memory optimization flag for building pairs of objects from before and after
           It is recommended to use the default optimize_memory_usage=True to avoid swap memory usage or MemoryError.
    :param compare_internal_objects: if True internal_objects (all) are evaluated instead of objects (reduced)
    :return: resim evaluation results
        * pairs_features: deviations calculated for paired objects
        * kpis_pairs_features: KPIs of deviations calculated for paired objects
        * kpis_binary_class: binary classification results (TPR and PPV)
        * log_features: coundts of TP, FP, FN, all records and relevant records only
        * pairs_signals_diff: paired objects signal values (before and after)
        * before_objects_signals_not_associated: signals of non-associated objects (before)
        * before_objects_raw_signals_not_associated: raw signals of non-associated objects (before)
        * after_objects_signals_not_associated: signals of non-associated objects (after)
        * after_objects_raw_signals_not_associated: raw signals of non-associated objects (before)
    """
    extractor = F360MUDPExtractor(f_extract_internal_objects=compare_internal_objects,
                                  f_extract_sensors=False,
                                  f_extract_detections=False)

    extracted_data_before = extractor.extract_data(parsed_data_before)
    extracted_data_after = extractor.extract_data(parsed_data_after)

    gating = PEPairedObjectsPreBuilderGating(distance_threshold=distance_threshold,
                                             f_optimize_memory_usage=optimize_memory_usage)
    relevancy = AllTrueFlag()
    association = AssociateAll()
    objects_features_kpis = [TPRBinaryClassificationKPI(), PPVBinaryClassificationKPI()]

    if compare_internal_objects:
        est_from_internal_objects, ref_from_internal_objects = True, True
        evaluated_objects_before = extracted_data_before.internal_objects
        evaluated_objects_after = extracted_data_after.internal_objects
    else:
        est_from_internal_objects, ref_from_internal_objects = False, False
        evaluated_objects_before = extracted_data_before.objects
        evaluated_objects_after = extracted_data_after.objects

    if evaluated_objects_before is None or evaluated_objects_after is None:
        raise AttributeError('Objects to evaluate are None. Check data after extraction or change input arguments')

    pe = PEPipeline(pairs_builder=gating,
                    relevancy_estimated_objects=relevancy,
                    relevancy_reference_objects=relevancy,
                    association=association,
                    pairs_lifetime_features=[],
                    objects_features_kpis=objects_features_kpis,
                    evaluate_internal_objects=est_from_internal_objects,
                    ref_from_internal_objects=ref_from_internal_objects)

    output = pe.evaluate(extracted_data_after, extracted_data_before)  # estimated, reference

    pairs_ref_before = pe.pairs_ref.add_suffix('_before')
    pairs_est_after = pe.pairs_est.add_suffix('_after')

    columns = list(pe.pairs_est_ref.signals.columns)
    for signal_name in evaluated_objects_before.signals.columns:
        columns.append(signal_name + '_before')
        columns.append(signal_name + '_after')

    pairs_signals_diff = pe.pairs_est_ref.signals \
        .merge(pairs_ref_before, left_on='index_ref', right_on='index_ref_before') \
        .merge(pairs_est_after, left_on='index_est', right_on='index_est_after') \
        [columns]

    not_associated_before_mask = ~output.pe_results_obj_ref.signals['is_associated']
    not_associated_after_mask = ~output.pe_results_obj_est.signals['is_associated']

    before_objects_signals_not_associated = evaluated_objects_before.signals[not_associated_before_mask]
    before_objects_raw_signals_not_associated = evaluated_objects_before.raw_signals[not_associated_before_mask]

    after_objects_signals_not_associated = evaluated_objects_after.signals[not_associated_after_mask]
    after_objects_raw_signals_not_associated = evaluated_objects_after.raw_signals[not_associated_after_mask]

    return {
        'pairs_features': output.pe_results_obj_pairs.signals,
        'kpis_pairs_features': output.kpis_pairs_features_per_log,
        'kpis_binary_class': output.kpis_binary_class_per_log,
        'log_features': output.logs_features_per_log,
        'pairs_signals_diff': pairs_signals_diff,
        'before_objects_signals_not_associated': before_objects_signals_not_associated,
        'before_objects_raw_signals_not_associated': before_objects_raw_signals_not_associated,
        'after_objects_signals_not_associated': after_objects_signals_not_associated,
        'after_objects_raw_signals_not_associated': after_objects_raw_signals_not_associated,
    }


def f360_resim_comparision(log_list: List[str], resim_extension_before: str, resim_extension_after: str,
                           mudp_stream_def_path: str, *,
                           streams_to_compare: Set[int] = {6},
                           distance_threshold: float = 0.01,
                           optimize_memory_usage: bool = True,
                           break_on_first_fail: bool = True,
                           compare_internal_objects: bool = False,
                           ) -> dict:
    """
    Executes resim comparision for log from provided lists.
    :param log_list: list of paths to original logs
    :param resim_extension_before: resim extension before
    :param resim_extension_after: resim extension after
    :param mudp_stream_def_path: path to MUDP stream definitions directory
    :param streams_to_compare: set of streams to compare
    :param distance_threshold: distance threshold to match two objects from before and after [m]
    :param optimize_memory_usage: memory optimization flag for building pairs of objects from before and after
           It is recommended to use the default optimize_memory_usage=True to avoid swap memory usage or MemoryError.
    :param break_on_first_fail: whether to stop refactoring verification on first difference
    :param compare_internal_objects: if True internal_objects (all) are evaluated instead of objects (reduced)
    :return: dictionary of performance evaluation results for each individual log
             For more information about output dictionary for single log, please check evaluate_resim_single_log.
    """
    results = {}
    for log_path in log_list:
        log_name = Path(log_path).stem
        print(f'{log_name} in progress...')

        data_before_path = get_f360_mudp_resim_log_path(log_path, resim_extension_before)
        data_after_path = get_f360_mudp_resim_log_path(log_path, resim_extension_after)

        parsed_data_before = parse_mudp(data_before_path, mudp_stream_def_path)
        parsed_data_after = parse_mudp(data_after_path, mudp_stream_def_path)

        try:
            assert_mudp_parsed_data_equal(parsed_data_before, parsed_data_after, streams_to_compare=streams_to_compare)
            print('Data equal.')
            results[log_name] = {'equal': True}

        except AssertionError as err:
            print(err)
            print('Data not equal, detailed comparision in progress...')

            result = evaluate_resim_single_log(parsed_data_before, parsed_data_after,
                                               distance_threshold=distance_threshold,
                                               optimize_memory_usage=optimize_memory_usage,
                                               compare_internal_objects=compare_internal_objects)
            results[log_name] = {'equal': False, **result}

            if break_on_first_fail:
                break

    return results


def generate_resim_comparison_report(*,
                                     subtitle: Optional[str] = None,
                                     resim_extension_before: str,
                                     resim_extension_after: str,
                                     commit_id_before: str,
                                     commit_id_after: str,
                                     log_list: List[str],
                                     results: dict,
                                     comments: Optional[str] = None,
                                     output_path: Union[str, Path]) -> None:
    """
    Exports refactoring verification report as PDF.
    :param subtitle: an optional subtitle of the report, e.g. identifier and name of refactoring task (or None)
    :param resim_extension_before: resim extension before
    :param resim_extension_after: resim extension after
    :param commit_id_before: commit id before
    :param commit_id_after: commit id after
    :param log_list: list of paths to original logs
    :param results: output of f360_resim_comparision function
    :param comments: optional comments
    :param output_path: path to output PDF file
    :return:
    """
    report_generator_dir = Path(__file__).parents[2] / 'report_generator'
    templates_path = str(report_generator_dir / 'RefactoringVerification' / 'templates')
    aptiv_logo_path = str(report_generator_dir / 'templates' / 'aptiv_logo.png')
    css_path = str(report_generator_dir / 'RefactoringVerification' / 'css' / 'refactoring_verification_report.css')
    wkhtmltopdf_path = str(report_generator_dir / 'wkhtmltopdf' / 'bin' / 'wkhtmltopdf.exe')

    template_loader = jinja2.FileSystemLoader(searchpath=templates_path)
    template_env = jinja2.Environment(loader=template_loader)

    template = template_env.get_template('refactoring_verification_report.html')
    output_text = template.render(aptiv_logo_path=aptiv_logo_path,
                                  subtitle=subtitle,
                                  resim_extension_before=resim_extension_before,
                                  resim_extension_after=resim_extension_after,
                                  commit_id_before=commit_id_before,
                                  commit_id_after=commit_id_after,
                                  log_list=log_list,
                                  results=results,
                                  comments=comments,
                                  generated_by=getlogin(),
                                  generated_at=datetime.now())

    configuration = pdfkit.configuration(wkhtmltopdf=wkhtmltopdf_path)
    while True:
        try:
            pdfkit.from_string(output_text, output_path, configuration=configuration, css=css_path)
            break
        except IOError:
            input('The output file is open in another application. Please close the file and press Enter.')
