from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.PairsFeaturesKPIManager import (
    EARLY_OBJECT_EXTENDED_PAIRS_KPI_DICT,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import EXT_NO_ACC_REL_PAIRS_FEATURES_LIST
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.default_pairs_lifetime_kpis import (
    DEF_NO_ACC_REL_LIFETIME_FEATURES_KPIS,
)
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.providers.F360MudpRtRangeMdfDataProvider import F360MudpRtRangeMdfDataProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.report_generator.Plotter import Plotter
from aspe.report_generator.ReportGenerator import ReportGenerator
from aspe.utilities.SupportingFunctions import save_to_pkl

"""
Run this script as 'Run File in Console'
Example of single log evaluation:
pe - performance evaluation pipeline is configured by defaults strategies
"""

save_dir_path = r"C:\Users\mj2snl\Documents\logs\aspe_eg\2\SRR_DEBUG\results"
logs_dir = r'C:\Users\mj2snl\Documents\logs\aspe_eg\2\SRR_DEBUG\rRf360t4280309v205p50_2_33_00'
logger_path = r'C:\Users\mj2snl\Documents\logs\aspe_eg\2\SRR_DEBUG\rRf360t4280309v205p50_2_33_00'
mudp_stream_defs_dir = r"C:\Users\mj2snl\gitrepos\F360Core\F360Core\sw\zResimSupport\stream_definitions"
sw_version = 'A370'  # 'A' step
host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
host_length = 4.7
host_width = 1.9
cs_system = 'VCS'  # coordinate system in which evaluation will be performed
rt_f360_dt = 0.047  # time delay between RtRange and F360 which must be compensated

provider = F360MudpRtRangeMdfDataProvider(
    mudp_stream_defs_path=mudp_stream_defs_dir,
    host_rear_axle_to_front_bumper_dist=5,
    host_width=host_width,
    host_length=host_length,
    rt_hunter_target_shift=10,
    reference_time_shift=rt_f360_dt,
    coordinate_system=cs_system,
    save_to_file=True,
    force_extract=True,
)

pep = PEPipeline(pairs_features_list=EXT_NO_ACC_REL_PAIRS_FEATURES_LIST,
                 pairs_features_kpis=EARLY_OBJECT_EXTENDED_PAIRS_KPI_DICT,
                 evaluate_internal_objects=True)

multi_log_eval = PEMultiLogEvaluation(single_log_pipeline=pep,
                                      data_provider=provider,
                                      logging_file_folder=logger_path,
                                      pairs_features_kpis=EARLY_OBJECT_EXTENDED_PAIRS_KPI_DICT,
                                      pairs_lifetime_kpis=DEF_NO_ACC_REL_LIFETIME_FEATURES_KPIS)

test_case_signature = '2_33_00'
log_list = get_log_list_from_path(logs_dir, required_sub_strings=[test_case_signature], req_ext='.mudp',
                                  level=1)  # filter TestCaseX

pe_output = multi_log_eval.process_data(log_list)
save_to_pkl(pe_output, f'{logs_dir}\\{test_case_signature}_pe_output.pickle')

sw_1_load_path = pe_output
report_save_path = save_dir_path + '\\report_earlyobj.pdf'
A370_path = logs_dir + f"\\{test_case_signature}_pe_output.pickle"

# Create plotter object
plotter_config = {'plot_style': 'default',
                  'default_plot_size': (10, 3.75),
                  'figures_save_path': save_dir_path,
                  'host_size': (2, 5),
                  'trace_plots_lims': (-75, 75)}
plotter = Plotter(plotter_config)

# set main report configuration
report_gen_config = {'title': 'Active Safety Performance Evaluation report',
                     'subtitle': 'F360 radar tracker',
                     'tc_description': 'this is test scenario description'}

# prepare data
sw_names = ['output']
sw_descriptions = ['this is output description', ]

data_paths = [A370_path]

# generate report and save it
report_input_data = ReportGenerator.prepare_input_data(data_paths, sw_names, sw_descriptions)
rep_gen = ReportGenerator(report_input_data, report_gen_config, plotter)
rep_gen.save_pdf(report_save_path)
